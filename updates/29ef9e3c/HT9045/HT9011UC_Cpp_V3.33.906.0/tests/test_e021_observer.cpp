// =============================================================================
//  test_e021_observer.cpp -- todo E-021: Data.Observer's St01 rows (OB-1 / OB-2 / OB-3 / OB-4 / OB-6 / OB-8 / OB-9)
//
//  //AI(W906-E021-OB1) 20261002 [W906] St01 new file.  todo D:\HT9045\.claude\skills\ht9050-construction\references\todo.md E-021;
//    St02 inventory D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\E019_DATA_STATUS_EVENTS_20261001.md 3.2; Jimmy RULINGS_20261001 #0;
//    ST01-E 20261002 conditions (shadowed SAFETY gates, seams set and required, Q66 = B for record names, OB-8 year digits).
//  golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cObserver.cpp: FormClose :654-674, BtnExitClick :697-706,
//    SavePrecautionMemoInformation :4309-4383, SavePrecautionParameter :4385-4414, Load*LogMenu :4494-4540,
//    SaveMajorMaintenanceInformation :4542-4620, the Record handlers :4683-4984, pgcMessageChange :4997-5024,
//    lstTimeDataClick :5072-5075, btnBackupLogYearClick :5602-5622, btnClearTimeClick :5624-5630.
//  //AI(W906-E030) 20261003 (St01): 雙重註解（Jimmy RULINGS_20261002 #20：golden = 906）——golden 906 D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\cObserver.cpp [AI(W906-E032) 20261003] 的同一批函式與 V912 逐行相同：FormClose :654-674、BtnExitClick :697-706 同號；其餘 906 = V912 − 231：
//    SavePrecautionMemoInformation 906 :4078-4152、SavePrecautionParameter :4154-4183、Load*LogMenu :4263-4309、
//    SaveMajorMaintenanceInformation :4311-4389、Record handlers :4452-4753、pgcMessageChange :4766-4793、lstTimeDataClick :4841-4844、
//    btnBackupLogYearClick :5371-5391、btnClearTimeClick :5393-5399。本檔各 CHECK 訊息裡的 golden 行號已改成「:906（或 :906, V912 :N）」雙列（AI(W906-E030-CITE) 20261003）；V912 :1-768 兩棵同號的只寫一次。
//  Under test: the tail of cObserver.cpp (ht9045::sjson::W906_ObserverAct, W906_E021_*, FileRW_Observer_WindowEdge -- the objects
//    wb_serve links), reached through the REAL act.* dispatch JsonBridge/ChanAction.cpp HandleActionWithTag (its :344 same-line
//    dispatch); "ok" = the reply carries "executed":true, as wb_serve.cpp:4833.
//    [0] seams: W906_PRECAUTION_ROOT / W906_MAJORMAINT_ROOT / W906_BACKUPLOGBAT_PATH set by ctest into machine_log_scratch (else exit 2,
//        nothing called); OB-8's exec pointer is golden's ExecZipCommand by default
//    [1] refusals: not open, unknown op, bad payload (widgets: unknown name, a panel, a memo line with LF, a stale itemIndex)
//    [2] OB-2 pure handlers + widget sync (Add / Clear / dates / Show with golden's refusal and the flags)
//    [3] OB-2 Save: golden's refusal, the record file + mirror (exact lines), the form reset + log list; the port refusal for path
//        characters (nothing written anywhere, form untouched); B01 off
//    [4] OB-4 Precaution Log search (dfm captions)
//    [5] OB-3 Major Maintenance: dates, add / clear, Save (file, reset, log list), Search (round trip), path refusal, B02 off oddity
//    [6] OB-1 Exit guard; the close edge = golden FormClose -> PrecautionParameter.ini (keys), once per open, B01 off
//    [7] OB-6 Time Data: pgcMessage page change lists <TimeData>\<year>\*.CSV, the grid, a file click
//    [8] OB-8 Backup Log: the bat lines (13, December twice), digits only for any year text, the hook called with (bat, " ")
//    [9] OB-9 Clear Time Data
//    [10] observer.get's full reply carries dataRecord; source ratchets (comments and '\r' stripped): the dispatch, the hook default,
//         the name check before the first write, the window-edge row, the year path, the page files (argv[1] port tree, argv[2] web\page)
//    [11] real files: D:\PrecautionRecord (+ \system), D:\MajorMaintenanceRecord, D:\HT9045\system listing and config.ini unchanged
//    [12] //AI(W906-E021-B5) 20261002: Big5 as golden (ST01-M ruling "follow BCB"): a Chinese note round-trips (the Big5 bytes on disk,
//         the Unicode file name, Search reads it back), golden-written Big5 fixtures (Chinese names) are listed and parsed, a Chinese
//         note with ".." / "\" is still refused (Q66), a character with no Big5 form (simplified Chinese, emoji) is refused with
//         nothing written, PrecautionParameter.ini holds Big5 bytes
//  Never runs golden FormShow (jimmychiu's copy makes D:\PrecautionRecord\system on the real disk); sets bShow instead.
//  Never executes a command: W906_E021_ExecHook is replaced by a recorder before OB-8 runs.
// =============================================================================
#include "JsonBridge/ChanAction.h"
#include "forms/fObserver.h"
#include "forms/fLotInfo.h"
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "LastSet.h"
#include "Config.h"
#include "Public/cJSON.h"
#include "w906_ctest_guard.h"

#include <windows.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

extern AnsiString W906_ShowMyMessage_LastS1;   // canary_support.cpp:67 (its header cannot share a TU with cMyDB.h)
extern int W906_ShowMyMessage_Count;           // canary_support.cpp:68
extern AnsiString as9045LogPath;
void GetTimeInfo();                            // cpublic.cpp:450
bool ExecZipCommand(AnsiString Path, AnsiString Param);   // cpublic.h:47
extern bool (*W906_E021_ExecHook)(AnsiString Path, AnsiString Param);
AnsiString W906_E021_PrecautionRoot();
AnsiString W906_E021_MajorMaintRoot();
AnsiString W906_E021_BackupLogBatPath();
AnsiString W906_E021_TimeDataRoot();
void W906_E021_LoadPrecautionLogMenu(TfObserver* f);
void W906_E021_LoadMajorMaintenanceLogMenu(TfObserver* f);
const char* FileRW_Observer_WindowEdge(bool open);
std::string W906_ObserverJson(const std::string& act, int arg, const std::string& text);

namespace {

int g_pass = 0;
int g_fail = 0;

void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}

bool Has(const std::string& h, const char* n) { return h.find(n) != std::string::npos; }
bool AckOk(const std::string& r) { return Has(r, "\"executed\":true"); }

std::string Act(const char* op, const std::string& value)
{
    const std::string cmd = std::string("act.observer.") + op;
    const std::string r = ht9045::sjson::HandleActionWithTag(cmd, value, std::string());
    std::printf("    %s %s -> %s\n", cmd.c_str(), value.substr(0, 160).c_str(), r.substr(0, 220).c_str());
    return r;
}

// ---- tiny JSON reads -------------------------------------------------------------------------------------------------------
struct J {
    cJSON* root;
    explicit J(const std::string& s) : root(cJSON_Parse(s.c_str())) {}
    ~J() { if (root) cJSON_Delete(root); }
    const cJSON* at(const char* path) const {   // "a.b.c"
        const cJSON* n = root;
        std::string p = path, seg;
        std::size_t i = 0;
        while (n && i <= p.size()) {
            const std::size_t d = p.find('.', i);
            seg = p.substr(i, d == std::string::npos ? std::string::npos : d - i);
            n = cJSON_GetObjectItemCaseSensitive(n, seg.c_str());
            if (d == std::string::npos) break;
            i = d + 1;
        }
        return n;
    }
    std::string str(const char* path) const { const cJSON* n = at(path); return (n && cJSON_IsString(n)) ? n->valuestring : "<none>"; }
    int num(const char* path) const { const cJSON* n = at(path); return (n && cJSON_IsNumber(n)) ? (int)n->valuedouble : -999; }
    bool flag(const char* path) const { const cJSON* n = at(path); return n && cJSON_IsTrue(n); }
    std::vector<std::string> strs(const char* path) const {
        std::vector<std::string> v;
        const cJSON* n = at(path);
        if (n && cJSON_IsArray(n)) for (const cJSON* c = n->child; c; c = c->next) v.push_back(cJSON_IsString(c) ? c->valuestring : "<?>");
        return v;
    }
};

// ---- files -----------------------------------------------------------------------------------------------------------------
bool ReadAll(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    *out = ss.str();
    return true;
}
void WriteAll(const std::string& p, const std::string& s) { std::ofstream f(p.c_str(), std::ios::binary); f << s; }
bool Exists(const std::string& p) { return ::GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES; }

void ListDir(const std::string& root, std::map<std::string, std::string>* out)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((root + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) { (*out)["<absent>"] = "1"; return; }
    do {
        const std::string name = fd.cFileName;
        if (name == "." || name == "..") continue;
        char buf[96];
        std::snprintf(buf, sizeof(buf), "%lu/%lu/%lu/%lu", (unsigned long)fd.nFileSizeHigh, (unsigned long)fd.nFileSizeLow,
                      (unsigned long)fd.ftLastWriteTime.dwHighDateTime, (unsigned long)fd.ftLastWriteTime.dwLowDateTime);
        (*out)[name] = buf;
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}
std::vector<std::string> Files(const std::string& dir)   // plain files, sorted (FindFirstFile order is by name on NTFS)
{
    std::vector<std::string> v;
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return v;
    do { if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) v.push_back(fd.cFileName); } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
    return v;
}
void MakeDirs(const std::string& p)
{
    for (std::size_t i = 3; i <= p.size(); ++i)
        if (i == p.size() || p[i] == '\\' || p[i] == '/') ::CreateDirectoryA(p.substr(0, i).c_str(), 0);
}
// delete everything under dir (dir must be inside ctest's machine_log_scratch\e021)
void Wipe(const std::string& dir)
{
    if (!W906CtestGuardInScratch(dir.c_str()) || dir.find("e021") == std::string::npos) { std::printf("  (wipe refused: %s)\n", dir.c_str()); return; }
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        const std::string name = fd.cFileName;
        if (name == "." || name == "..") continue;
        const std::string p = dir + "\\" + name;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { Wipe(p); ::RemoveDirectoryA(p.c_str()); }
        else ::DeleteFileA(p.c_str());
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}
std::vector<std::string> SplitLines(const std::string& s)
{
    std::vector<std::string> v;
    std::string cur;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\r') continue;
        if (s[i] == '\n') { v.push_back(cur); cur.clear(); continue; }
        cur += s[i];
    }
    if (!cur.empty()) v.push_back(cur);
    return v;
}

// source without // and /* */ comments, string literals kept, '\r' dropped
std::string StripComments(const std::string& s)
{
    std::string o;
    o.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i) {
        const char c = s[i];
        if (c == '\r') continue;
        if (c == '"' || c == '\'') {
            const char q = c;
            o += c;
            for (++i; i < s.size(); ++i) {
                o += s[i];
                if (s[i] == '\\' && i + 1 < s.size()) { o += s[++i]; continue; }
                if (s[i] == q || s[i] == '\n') break;
            }
            continue;
        }
        if (c == '/' && i + 1 < s.size() && s[i + 1] == '/') { while (i < s.size() && s[i] != '\n') ++i; o += '\n'; continue; }
        if (c == '/' && i + 1 < s.size() && s[i + 1] == '*') { i += 2; while (i + 1 < s.size() && !(s[i] == '*' && s[i + 1] == '/')) ++i; ++i; continue; }
        o += c;
    }
    return o;
}
std::string StripHtmlComments(const std::string& s)
{
    std::string o;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s.compare(i, 4, "<!--") == 0) { const std::size_t e = s.find("-->", i + 4); if (e == std::string::npos) break; i = e + 2; continue; }
        if (s[i] != '\r') o += s[i];
    }
    return o;
}
// the body of a function (from "name(" at line start to the matching close brace), comments stripped
std::string Body(const std::string& src, const char* head)
{
    const std::size_t a = src.find(head);
    if (a == std::string::npos) return "";
    const std::size_t b = src.find('{', a);
    int depth = 0;
    for (std::size_t i = b; i < src.size(); ++i) {
        if (src[i] == '{') ++depth;
        else if (src[i] == '}' && --depth == 0) return src.substr(a, i - a + 1);
    }
    return "";
}

// ---- OB-8's command runner, replaced (a test never executes anything) ------------------------------------------------------
int g_execCalls = 0;
std::string g_execPath, g_execParam;
bool RecordExec(AnsiString Path, AnsiString Param) { ++g_execCalls; g_execPath = Path.c_str(); g_execParam = Param.c_str(); return true; }

// ---- Big5 helpers (this test's own, with the Win32 API directly -- not the code under test) --------------------------------------
std::wstring Wd(const std::string& u8)
{
    const int n = ::MultiByteToWideChar(CP_UTF8, 0, u8.data(), (int)u8.size(), 0, 0);
    std::wstring w(n > 0 ? (std::size_t)n : 0, L'\0');
    if (n > 0) ::MultiByteToWideChar(CP_UTF8, 0, u8.data(), (int)u8.size(), &w[0], n);
    return w;
}
std::string U8(const std::wstring& w)
{
    const int n = ::WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), 0, 0, 0, 0);
    std::string u(n > 0 ? (std::size_t)n : 0, '\0');
    if (n > 0) ::WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), &u[0], n, 0, 0);
    return u;
}
std::string FromBig5(const std::string& b)
{
    const int n = ::MultiByteToWideChar(950, 0, b.data(), (int)b.size(), 0, 0);
    std::wstring w(n > 0 ? (std::size_t)n : 0, L'\0');
    if (n > 0) ::MultiByteToWideChar(950, 0, b.data(), (int)b.size(), &w[0], n);
    return U8(w);
}
std::string ToBig5(const std::string& u8)
{
    const std::wstring w = Wd(u8);
    const int n = ::WideCharToMultiByte(950, 0, w.data(), (int)w.size(), 0, 0, 0, 0);
    std::string b(n > 0 ? (std::size_t)n : 0, '\0');
    if (n > 0) ::WideCharToMultiByte(950, 0, w.data(), (int)w.size(), &b[0], n, 0, 0);
    return b;
}
std::vector<std::string> FilesW(const std::string& dirU8)   // UTF-8 names, as NTFS holds them
{
    std::vector<std::string> v;
    WIN32_FIND_DATAW fd;
    HANDLE h = ::FindFirstFileW((Wd(dirU8) + L"\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return v;
    do { if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) v.push_back(U8(fd.cFileName)); } while (::FindNextFileW(h, &fd));
    ::FindClose(h);
    return v;
}
bool ReadAllW(const std::string& pathU8, std::string* out)
{
    out->clear();
    HANDLE h = ::CreateFileW(Wd(pathU8).c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (h == INVALID_HANDLE_VALUE) return false;
    char buf[4096];
    DWORD got = 0;
    while (::ReadFile(h, buf, sizeof(buf), &got, 0) && got > 0) out->append(buf, got);
    ::CloseHandle(h);
    return true;
}
void WriteAllW(const std::string& pathU8, const std::string& bytes)
{
    HANDLE h = ::CreateFileW(Wd(pathU8).c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD put = 0;
    ::WriteFile(h, bytes.data(), (DWORD)bytes.size(), &put, 0);
    ::CloseHandle(h);
}
int IndexOfItem(TComboBox* cb, const std::string& name)
{
    for (int i = 0; i < cb->Items->Count; ++i) if (std::string(AnsiString(cb->Items->Strings[i]).c_str()) == name) return i;
    return -1;
}

// what the page sends: every Record-tab input
std::string W(const std::string& extra) { return "{\"widgets\":{" + extra + "}}"; }

void Blank(TfObserver* f)
{
    f->edPrecautionRecordDocumentNo->Text = ""; f->edNoteContents->Text = ""; f->edApprovedManager->Text = "";
    f->edWatchmakers->Text = ""; f->edFinishName->Text = ""; f->edPromptDay->Text = "";
    f->pnPrecautionStartTime->Caption = ""; f->pnPrecautionEndTime->Caption = "";
    f->MemoHandlerPrecautionRecord->Lines->Clear();
}

const char* kCompletePr =
    "\"edPrecautionRecordDocumentNo\":\"DOC-7\",\"edNoteContents\":\"Note1\",\"edApprovedManager\":\"Boss\","
    "\"edWatchmakers\":\"Maker\",\"edFinishName\":\"Closer\",\"edPromptDay\":\"3\","
    "\"cobPRFinishType\":{\"itemIndex\":1},\"MemoHandlerPrecautionRecord\":[\"line A\",\"line B\"]";

}  // namespace

int main(int argc, char** argv)
{
    extern AnsiString asSaveEventLogPath, asGeneralPath, AuthPath;
    const char* e1 = getenv("W906_PRECAUTION_ROOT");
    const char* e2 = getenv("W906_MAJORMAINT_ROOT");
    const char* e3 = getenv("W906_BACKUPLOGBAT_PATH");
    const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asSaveEventLogPath", asSaveEventLogPath.c_str(),
                               "asGeneralPath", asGeneralPath.c_str(), "AuthPath", AuthPath.c_str(),
                               "W906_PRECAUTION_ROOT", e1 ? e1 : "(unset)", "W906_MAJORMAINT_ROOT", e2 ? e2 : "(unset)",
                               "W906_BACKUPLOGBAT_PATH", e3 ? e3 : "(unset)", 0 };
    if (!W906TestRequireCtestRedirects("E021_Observer", rt))
        return 2;
    const std::string root = argc > 1 ? argv[1] : "";
    const std::string web = argc > 2 ? argv[2] : "";

    // real machine paths: listings before (compared in [11])
    std::map<std::string, std::string> pr0, prs0, mm0, sys0, pr1, prs1, mm1, sys1;
    std::string cfg0, cfg1;
    ListDir("D:\\PrecautionRecord", &pr0);
    ListDir("D:\\PrecautionRecord\\system", &prs0);
    ListDir("D:\\MajorMaintenanceRecord", &mm0);
    ListDir("D:\\HT9045\\system", &sys0);
    ReadAll("D:\\HT9045\\config\\config.ini", &cfg0);

    const std::string PR = W906_E021_PrecautionRoot().c_str(), MM = W906_E021_MajorMaintRoot().c_str(), BAT = W906_E021_BackupLogBatPath().c_str();
    const std::string sand = PR.substr(0, PR.find_last_of("\\/"));   // ...\machine_log_scratch\e021 (ctest hands '/' paths)
    std::printf("[0] seams\n");
    {
        Check(PR == e1 && MM == e2 && BAT == e3, "the three seams are what ctest set (" + PR + " / " + MM + " / " + BAT + ")");
        Check(sand.find("e021") != std::string::npos && W906CtestGuardInScratch(sand.c_str()), "the sandbox is machine_log_scratch\\e021");
        Check(W906_E021_ExecHook == &ExecZipCommand, "OB-8's exec pointer defaults to golden's ExecZipCommand (cpublic.cpp:810)");
        Wipe(sand);
        MakeDirs(sand);
        MakeDirs(BAT.substr(0, BAT.find_last_of("\\/")));   // golden writes 2.bat into an existing D:\HT9045\system; SaveToFile makes no folder
        W906_E021_ExecHook = &RecordExec;
        Check(W906_E021_TimeDataRoot() == as9045LogPath + "\\TimeData", "TimeData root = as9045LogPath + \\TimeData (W906_HT9045LOG_ROOT)");
    }

    GetTimeInfo();
    TfObserver* f = fObserver;
    IniConfig.SocketHandlerID = "HTEST";
    IniConfig.bB01_UsePrecautionRecordFunction = true;
    IniConfig.bB02_HanderMajorMaintenanceRecordFunction = true;
    IniConfig.asB01_PrecautionRecordSavePath = "";
    IniConfig.asB02_HanderMajorMaintenanceRecordSavePath = "";
    f->cobNoteContents->Items->Clear(); f->cobNoteContents->Items->Add("NC1"); f->cobNoteContents->Items->Add("NC2");
    f->cobHandlerPrecautionRecord->Items->Clear(); f->cobHandlerPrecautionRecord->Items->Add("R1"); f->cobHandlerPrecautionRecord->Items->Add("R2");
    f->cobUndesirablePhenomenon->Items->Clear(); f->cobUndesirablePhenomenon->Items->Add("P1"); f->cobUndesirablePhenomenon->Items->Add("P2");
    f->cobCountermeasure->Items->Clear(); f->cobCountermeasure->Items->Add("C1");
    fLotInfo->ed_PIOEEMO->Text = "MO123";

    std::printf("[1] refusals\n");
    {
        f->bShow = false;
        std::string r = Act("sync", "{}");
        Check(!AckOk(r) && Has(r, "\"guard\":\"not-open\""), "before golden FormShow -> not-open");
        f->bShow = true;                                  // golden FormShow :350 (not run here: jimmychiu's copy writes D:\PrecautionRecord\system)
        r = ht9045::sjson::HandleActionWithTag("act.observer.nope", "{}", std::string());
        Check(!AckOk(r) && Has(r, "\"guard\":\"unknown-action\"") && Has(r, "prSave"), "unknown op -> unknown-action with the op list");
        r = Act("sync", "[1]");
        Check(Has(r, "\"guard\":\"bad-payload\""), "value not an object -> bad-payload");
        r = Act("sync", W("\"edNope\":\"x\""));
        Check(Has(r, "\"guard\":\"bad-payload\"") && Has(r, "unknown widget"), "unknown widget -> bad-payload");
        r = Act("sync", W("\"pnPrecautionStartTime\":\"2020/01/01\""));
        Check(Has(r, "\"guard\":\"bad-payload\"") && Has(r, "not an input"), "a panel (golden writes its Caption) -> bad-payload");
        r = Act("sync", W("\"MemoHandlerPrecautionRecord\":[\"a\\nb\"]"));
        Check(Has(r, "\"guard\":\"bad-payload\""), "a memo line with LF -> bad-payload");
        r = Act("sync", W("\"cobNoteContents\":{\"itemIndex\":5}"));
        Check(Has(r, "\"guard\":\"bad-payload\"") && Has(r, "stale"), "itemIndex outside Items -> bad-payload (stale list)");
        r = Act("sync", W("\"MemoNoteLog\":[\"x\"]"));
        Check(Has(r, "\"guard\":\"bad-payload\""), "MemoNoteLog is not an input -> bad-payload");
        r = Act("sync", W("\"edNoteContents\":\"keep\",\"edNope\":\"x\""));
        Check(f->edNoteContents->Text != "keep", "a refused payload applies nothing (checked first, applied second)");
    }

    std::printf("[2] OB-2 pure handlers\n");
    {
        Blank(f);
        std::string r = Act("sync", W("\"edNoteContents\":\"\",\"cobNoteContents\":{\"itemIndex\":1}"));
        J j(r);
        Check(AckOk(r) && f->cobNoteContents->ItemIndex == 1 && f->cobNoteContents->Text == "NC2", "sync: itemIndex 1 -> Text = Items[1] (VCL SetItemIndex)");
        Check(j.str("dataRecord.combos.cobNoteContents.text") == "NC2" && j.strs("dataRecord.combos.cobPRFinishType.items").size() == 2,
              "dataRecord: combos with items / text; cobPRFinishType has the dfm items (By MO, By Day)");
        r = Act("prNoteSet", W("\"edNoteContents\":\"\",\"cobNoteContents\":{\"itemIndex\":1}"));
        Check(AckOk(r) && f->edNoteContents->Text == "NC2", "prNoteSet: empty note -> the combo text (golden :4454-4457 (V912 :4685-4688))");
        r = Act("prNoteSet", W("\"edNoteContents\":\"mine\",\"cobNoteContents\":{\"itemIndex\":0}"));
        Check(f->edNoteContents->Text == "mine", "prNoteSet: a note already there stays (golden :4454 (V912 :4685))");
        Act("prRecordSet", W("\"cobHandlerPrecautionRecord\":{\"itemIndex\":0},\"MemoHandlerPrecautionRecord\":[]"));
        Act("prRecordSet", W("\"cobHandlerPrecautionRecord\":{\"itemIndex\":0}"));
        r = Act("prRecordSet", W("\"cobHandlerPrecautionRecord\":{\"itemIndex\":1}"));
        J j2(r);
        Check(f->MemoHandlerPrecautionRecord->Lines->Count == 2 && j2.strs("dataRecord.memos.MemoHandlerPrecautionRecord").size() == 2 &&
              j2.strs("dataRecord.memos.MemoHandlerPrecautionRecord")[0] == "R1", "prRecordSet: R1 once (IndexOf, golden :4463 (V912 :4694)), then R2");
        r = Act("prRecordSet", W("\"cobHandlerPrecautionRecord\":{\"itemIndex\":-1}"));
        Check(f->MemoHandlerPrecautionRecord->Lines->Count == 2, "prRecordSet with no selection (Text \"\") adds nothing (golden :4463 (V912 :4694))");
        Act("prRecordClear", "{}");
        Check(f->MemoHandlerPrecautionRecord->Lines->Count == 0, "prRecordClear: memo cleared (golden :4470 (V912 :4701))");
        Act("prStartDate", "{}");
        Act("prFinishDate", "{}");
        const std::string s1 = f->pnPrecautionStartTime->Caption.c_str(), s2 = f->pnPrecautionEndTime->Caption.c_str();
        Check(s1.size() == 10 && s1[4] == '/' && s1[7] == '/' && s2 == s1, "prStartDate / prFinishDate: yyyy/mm/dd now (golden :4525 (V912 :4756) / :4531 (V912 :4762)) = " + s1);

        f->bStartPrecautionRecord = false; f->bSavePrecautionRecordFinish = true; f->asStartPrecautionRecordMOId = "";
        const int n0 = W906_ShowMyMessage_Count;
        r = Act("prFormShow", W("\"edPrecautionRecordDocumentNo\":\"\""));
        Check(AckOk(r) && W906_ShowMyMessage_Count == n0 + 1 && W906_ShowMyMessage_LastS1 == "Precaution Memo Information Not Enter Complete, Please Check" &&
              !f->bStartPrecautionRecord, "prFormShow incomplete -> golden's message (:4481, V912 :4712), flags unchanged");
        r = Act("prFormShow", W(kCompletePr));
        Check(AckOk(r) && f->bStartPrecautionRecord && !f->bSavePrecautionRecordFinish && f->asStartPrecautionRecordMOId == "MO123" &&
              Has(r, "fPrecaution->Show()"), "prFormShow complete -> bStartPrecautionRecord, MO id from fLotInfo->ed_PIOEEMO (:4486-4491, V912 :4717-4722); popup listed as skipped");
        IniConfig.bB01_UsePrecautionRecordFunction = false;
        f->bStartPrecautionRecord = false;
        r = Act("prFormShow", W(kCompletePr));
        Check(AckOk(r) && !f->bStartPrecautionRecord && Has(r, "B01 off"), "prFormShow with B01 off -> golden returns (:4476-4477, V912 :4707-4708)");
        IniConfig.bB01_UsePrecautionRecordFunction = true;
    }

    std::printf("[3] OB-2 Save\n");
    {
        Blank(f);
        int n0 = W906_ShowMyMessage_Count;
        std::string r = Act("prSave", W("\"edNoteContents\":\"x\""));
        Check(AckOk(r) && W906_ShowMyMessage_Count == n0 + 1 && Files(PR).empty(), "incomplete -> golden refusal (:4498-4502, V912 :4729-4733), nothing written");
        Act("prStartDate", "{}");
        Act("prFinishDate", "{}");
        const std::string start = f->pnPrecautionStartTime->Caption.c_str();
        f->bStartPrecautionRecord = true; f->bSavePrecautionRecordFinish = false;
        IniConfig.asB01_PrecautionRecordSavePath = AnsiString((sand + "\\mirrorPR").c_str());
        r = Act("prSave", W(kCompletePr));
        const std::vector<std::string> pf = Files(PR);
        Check(AckOk(r) && pf.size() == 1, "complete -> one record file in " + PR);
        const std::string name = pf.empty() ? "" : pf[0];
        Check(name.compare(0, 6, "HTEST_") == 0 && name.size() > 10 && name.substr(name.size() - 10) == "_Note1.txt",
              "file name <SocketHandlerID>_<yyyy_mm_dd_hh_nn_ss>_<note>.txt (golden :4091-4092 (V912 :4322-4323)): " + name);
        std::string body;
        ReadAll(PR + "\\" + name, &body);
        const std::vector<std::string> L = SplitLines(FromBig5(body));   // //AI(W906-E021-B5): golden's Big5 file
        const char* const want[] = { "DOCUMENT NO.: DOC-7", "注意事項: Note1", "注意事項內容: ", "line A", "line B", "部門主管核准: Boss",
                                     "製表者: Maker", "結案者: Closer", 0 };
        bool same = L.size() == 12;
        for (int i = 0; same && want[i]; ++i) same = (L[i] == want[i]);
        Check(same && L[8] == "開始日期: " + start && L[9] == "結案日期: " + start && L[10] == "結案方式: By Day" && L[11] == "預計結案天數: 3",
              "record lines = golden :4094-4115 (V912 :4325-4346) (12 lines, read back from Big5)");
        Check(Files(sand + "\\mirrorPR").size() == 1 && Files(sand + "\\mirrorPR")[0] == name, "the customer mirror gets the same file (golden :4121-4139 (V912 :4352-4370))");
        J j(r);
        Check(f->edNoteContents->Text == "" && f->edPrecautionRecordDocumentNo->Text == "" && f->pnPrecautionStartTime->Caption == "" &&
              f->cobPRFinishType->ItemIndex == -1 && f->cobPRFinishType->Text == "" && f->cobNoteContents->Text == "" &&
              f->MemoHandlerPrecautionRecord->Lines->Count == 0, "form reset (golden :4505-4516 (V912 :4736-4747); combo Text follows ItemIndex -1)");
        Check(f->bSavePrecautionRecordFinish && !f->bStartPrecautionRecord, "flags: finish=true, start=false (:4518-4519, V912 :4749-4750)");
        Check(j.strs("dataRecord.combos.cobSearchPrecautionLog.items").size() == 1 && j.str("dataRecord.combos.cobSearchPrecautionLog.text") == name,
              "LoadPrecautionLogMenu (:4517, V912 :4748): the log list shows the new file, item 0");
        IniConfig.asB01_PrecautionRecordSavePath = "";

        // [W906] port-only refusal (Steven Q66 = B)
        Act("prStartDate", "{}");                       // golden's reset cleared both panels; CheckKeyIn(2) needs them
        Act("prFinishDate", "{}");
        std::map<std::string, std::string> sb0, sb1;
        ListDir(PR, &sb0);
        const char* const bad[] = { "..\\\\..\\\\HT9045\\\\system\\\\x", "a/b", "c:d", "x*y", "q?", "\\\"", "<a", "b>", "p|q", "..", 0 };
        bool allRefused = true, untouched = true;
        for (int i = 0; bad[i]; ++i) {
            n0 = W906_ShowMyMessage_Count;
            std::string v = std::string(kCompletePr);
            v.replace(v.find("Note1"), 5, bad[i]);
            r = Act("prSave", W(v));
            allRefused = allRefused && !AckOk(r) && Has(r, "\"guard\":\"record-name\"") && W906_ShowMyMessage_Count == n0 + 1;
            untouched = untouched && f->edPrecautionRecordDocumentNo->Text == "DOC-7";
        }
        ListDir(PR, &sb1);
        Check(allRefused, "path characters \\ / : * ? \" < > | and \"..\" in the note -> refused (record-name) + a message");
        Check(sb0 == sb1 && Files(PR).size() == 1, "refused: nothing written in the sandbox");
        Check(untouched, "refused: golden's form reset is not run (the operator's input stays)");
        IniConfig.bB01_UsePrecautionRecordFunction = false;
        r = Act("prSave", W(kCompletePr));
        Check(AckOk(r) && Has(r, "B01 off") && Files(PR).size() == 1, "B01 off -> golden returns (:4496-4497, V912 :4727-4728)");
        IniConfig.bB01_UsePrecautionRecordFunction = true;
    }

    std::printf("[4] OB-4 Precaution Log search\n");
    {
        f->MemoNoteLog->Lines->Clear();
        std::string r = Act("prLogSearch", W("\"cobSearchPrecautionLog\":{\"itemIndex\":0}"));
        J j(r);
        Check(AckOk(r) && j.str("dataRecord.panels.pnPrecautionLogDocumentNo") == "DOC-7" && j.str("dataRecord.panels.pnPrecautionLogNoteContents") == "Note1" &&
              j.str("dataRecord.panels.pnPrecautionLogApprovedManager") == "Boss" && j.str("dataRecord.panels.pnPrecautionLogWatchmakers") == "Maker" &&
              j.str("dataRecord.panels.pnPrecautionLogFinishName") == "Closer" && j.str("dataRecord.panels.pnPrecautionLogFinishType") == "By Day" &&
              j.str("dataRecord.panels.pnPrecautionLogPromptDay") == "3", "golden :4708-4746 (V912 :4939-4977) parse with the dfm captions: every panel");
        const std::vector<std::string> m = j.strs("dataRecord.memos.MemoNoteLog");
        Check(m.size() == 2 && m[0] == "line A" && m[1] == "line B", "MemoNoteLog = the note lines; the \"注意事項內容:\" header is swallowed (:4720, V912 :4951)");
    }

    std::printf("[5] OB-3 Major Maintenance\n");
    {
        Act("mmDate", "{}"); Act("mmStart", "{}"); Act("mmEnd", "{}");
        const std::string d = f->pnMajorMaintenanceDate->Caption.c_str(), t1 = f->pnMajorMaintenanceStartTime->Caption.c_str(), t2 = f->pnMajorMaintenanceEndTime->Caption.c_str();
        Check(d.size() == 10 && d[4] == '/' && t1.size() == 5 && t1[2] == ':' && t2.size() == 5, "mmDate yyyy/mm/dd, mmStart / mmEnd hh:nn (golden :4537 (V912 :4768) / :4543 (V912 :4774) / :4571 (V912 :4802))");
        Act("mmPhenAdd", W("\"cobUndesirablePhenomenon\":{\"itemIndex\":0},\"MemoUndesirablePhenomenon\":[]"));
        Act("mmPhenAdd", W("\"cobUndesirablePhenomenon\":{\"itemIndex\":0}"));
        Act("mmPhenAdd", W("\"cobUndesirablePhenomenon\":{\"itemIndex\":1}"));
        Act("mmCmAdd", W("\"cobCountermeasure\":{\"itemIndex\":0},\"MemoCountermeasure\":[]"));
        Check(f->MemoUndesirablePhenomenon->Lines->Count == 2 && f->MemoCountermeasure->Lines->Count == 1, "Add: P1 once, P2; C1 (golden :4548 (V912 :4779) / :4554 (V912 :4785))");
        Act("mmPhenClear", "{}"); Act("mmCmClear", "{}");
        Check(f->MemoUndesirablePhenomenon->Lines->Count == 0 && f->MemoCountermeasure->Lines->Count == 0, "Clear both (golden :4561 (V912 :4792) / :4566 (V912 :4797))");
        int n0 = W906_ShowMyMessage_Count;
        std::string r = Act("mmSave", W("\"edMajorMaintenanceCheckNo\":\"\""));
        Check(AckOk(r) && W906_ShowMyMessage_Count == n0 + 1 && W906_ShowMyMessage_LastS1 == "Major Maintenance Information Not Enter Complete, Please Check" &&
              Files(MM).empty(), "incomplete -> golden refusal (:4576-4580, V912 :4807-4811), nothing written");
        f->bChangeReciepeSaveMajorMaintenanceRecord = true;
        const std::string mmIn = "\"cobMajorMaintenanceClassType\":{\"itemIndex\":2},\"edMajorMaintenanceCheckNo\":\"LOT9\",\"edMajorMaintenancePersonnel\":\"Fixer\","
                                 "\"edMajorMaintenanceCheckPersonnel\":\"Checker\",\"MemoUndesirablePhenomenon\":[\"Jam at X\",\"second: line\"],\"MemoCountermeasure\":[\"cleaned\"]";
        r = Act("mmSave", W(mmIn));
        const std::vector<std::string> mf = Files(MM);
        Check(AckOk(r) && mf.size() == 1 && mf[0].size() > 13 && mf[0].substr(mf[0].size() - 13) == "_Jam at X.txt", "record file named after phenomenon line 0 (golden :4326-4327 (V912 :4557-4558))");
        std::string body;
        if (!mf.empty()) ReadAll(MM + "\\" + mf[0], &body);
        const std::vector<std::string> L = SplitLines(FromBig5(body));   // //AI(W906-E021-B5): golden's Big5 file
        Check(L.size() == 12 && L[0] == "班別: C" && L[1] == "日期: " + d && L[2] == "開始時間: " + t1 && L[3] == "結束時間: " + t2 &&
              L[4] == "確認批號: LOT9" && L[5] == "維修者: Fixer" && L[6] == "確認者: Checker" && L[7] == "不良現象: " && L[8] == "Jam at X" &&
              L[9] == "second: line" && L[10] == "處理對策: " && L[11] == "cleaned", "record lines = golden :4329-4353 (V912 :4560-4584) (read back from Big5)");
        J j(r);
        Check(f->edMajorMaintenanceCheckNo->Text == "" && f->pnMajorMaintenanceDate->Caption == "" && f->cobMajorMaintenanceClassType->ItemIndex == -1 &&
              f->MemoUndesirablePhenomenon->Lines->Count == 0 && !f->bChangeReciepeSaveMajorMaintenanceRecord &&
              j.strs("dataRecord.combos.cobMajorMaintenanceSearch.items").size() == 1, "reset + log list + bChangeReciepeSaveMajorMaintenanceRecord=false (golden :4583-4593 (V912 :4814-4824))");
        r = Act("mmSearch", W("\"cobMajorMaintenanceSearch\":{\"itemIndex\":0}"));
        J k(r);
        const std::vector<std::string> ph = k.strs("dataRecord.memos.MemoUndesirablePhenomenon"), cm = k.strs("dataRecord.memos.MemoCountermeasure");
        Check(AckOk(r) && k.num("dataRecord.combos.cobMajorMaintenanceClassType.itemIndex") == 2 && k.str("dataRecord.panels.pnMajorMaintenanceDate") == d &&
              k.str("dataRecord.panels.pnMajorMaintenanceStartTime") == t1 && k.str("dataRecord.edits.edMajorMaintenanceCheckNo") == "LOT9" &&
              k.str("dataRecord.edits.edMajorMaintenanceCheckPersonnel") == "Checker", "mmSearch restores the fields (golden :4626-4653 (V912 :4857-4884), dfm captions)");
        Check(ph.size() == 2 && ph[0] == "Jam at X" && ph[1] == "second: line" && cm.size() == 1 && cm[0] == "cleaned",
              "mmSearch: phenomenon / countermeasure lines between the headers (golden :4655-4675 (V912 :4886-4906))");

        std::map<std::string, std::string> sb0, sb1;
        ListDir(MM, &sb0);
        n0 = W906_ShowMyMessage_Count;
        std::string bad = mmIn;
        bad.replace(bad.find("Jam at X"), 8, "..\\\\..\\\\HT9045\\\\system\\\\evil");
        r = Act("mmSave", W(bad));
        ListDir(MM, &sb1);
        Check(!AckOk(r) && Has(r, "\"guard\":\"record-name\"") && W906_ShowMyMessage_Count == n0 + 1 && sb0 == sb1 &&
              f->edMajorMaintenanceCheckNo->Text == "LOT9", "phenomenon line 0 with path characters -> refused, nothing written, form kept");
        IniConfig.bB02_HanderMajorMaintenanceRecordFunction = false;
        r = Act("mmSave", W(mmIn));
        Check(AckOk(r) && Files(MM).size() == 1 && f->edMajorMaintenanceCheckNo->Text == "", "B02 off: golden still checks and wipes the form, writes nothing (:4319 (V912 :4550) is the only B02 test)");
        IniConfig.bB02_HanderMajorMaintenanceRecordFunction = true;
    }

    std::printf("[6] OB-1 Exit + close edge\n");
    {
        f->bChangeReciepeSaveMajorMaintenanceRecord = true;
        int n0 = W906_ShowMyMessage_Count;
        std::string r = Act("exit", "{}");
        Check(AckOk(r) && Has(r, "\"close\":false") && W906_ShowMyMessage_Count == n0 + 1 &&
              W906_ShowMyMessage_LastS1 == "Major Maintenance Information Not Enter Complete And Save, Please Check", "Exit with MM unsaved + B02 -> golden refusal (:699-704), no close");
        f->bChangeReciepeSaveMajorMaintenanceRecord = false;
        r = Act("exit", "{}");
        Check(AckOk(r) && Has(r, "\"close\":true"), "Exit otherwise -> close:true (golden Close() :705)");
        Check(std::string(FileRW_Observer_WindowEdge(true)).find("no open-edge action") != std::string::npos, "open edge: nothing");
        Act("sync", W(std::string(kCompletePr) + ",\"edNoteContents\":\"Kept\""));
        f->asStartPrecautionRecordMOId = "MO9";
        f->bShow = true;
        f->iShowYieldChart = 1;
        const std::string e = FileRW_Observer_WindowEdge(false);
        std::string ini;
        const bool got = ReadAll(PR + "\\system\\PrecautionParameter.ini", &ini);
        Check(Has(e, "FormClose") && !f->bShow && f->iShowYieldChart == 0, "close edge = golden FormClose: bShow=false, iShowYieldChart=0 (:656-657)");
        Check(got && Has(ini, "[Precaution]") && Has(ini, "DOCUMENT NO.=DOC-7") && Has(ini, "Note Contents=Kept") && Has(ini, "Finish Type=1") &&
              Has(ini, "MO ID=MO9") && Has(ini, "Note Count=2") && Has(ini, "Note0=line A") && Has(ini, "Note1=line B"),
              "PrecautionParameter.ini in the sandbox has golden's keys (:4165-4180, V912 :4396-4411)");
        const std::string e2s = FileRW_Observer_WindowEdge(false);
        Check(Has(e2s, "did not run"), "a second close edge without a new open -> golden FormClose not run again");
        Wipe(PR + "\\system");
        ::RemoveDirectoryA((PR + "\\system").c_str());
        IniConfig.bB01_UsePrecautionRecordFunction = false;
        f->bShow = true;
        const std::string e3s = FileRW_Observer_WindowEdge(false);
        Check(Exists(PR + "\\system") && !Exists(PR + "\\system\\PrecautionParameter.ini") && Has(e3s, "B01 off"),
              "B01 off: golden makes the folder first, writes no ini (:4157 (V912 :4388) / :4162 (V912 :4393))");
        IniConfig.bB01_UsePrecautionRecordFunction = true;
        f->bShow = true;
    }

    std::printf("[7] OB-6 Time Data\n");
    {
        char y[16];
        std::snprintf(y, sizeof(y), "%d", (int)SystemYear);
        const std::string tdDir = std::string(as9045LogPath.c_str()) + "\\TimeData\\" + y;   // shared scratch: other tests may have files here too
        const std::string probe = tdDir + "\\E021_probe.csv";
        MakeDirs(tdDir);
        WriteAll(probe, "Date,PowerOn,Run\r\n2026/10/02,10,20\r\n");
        std::string r = Act("msgTab", "{\"arg\":2}");
        J j(r);
        const std::vector<std::string> files = j.strs("timeData.files");
        int mine = -1;
        for (std::size_t i = 0; i < files.size(); ++i) if (files[i].find("E021_probe.csv") != std::string::npos) mine = (int)i;
        Check(AckOk(r) && mine >= 0 && j.num("timeData.itemIndex") == (int)files.size() - 1 && f->pgcMessage->ActivePageIndex == 2,
              "msgTab 2 -> pgcMessageChange lists <TimeData>\\<year>\\*.CSV, the last one selected (golden :4774-4783 (V912 :5005-5014))");
        char a[32];
        std::snprintf(a, sizeof(a), "{\"arg\":%d}", mine < 0 ? 0 : mine);
        r = Act("timeFile", a);
        J k(r);
        const cJSON* cells = k.at("timeData.grid.cells");
        bool gridOk = false;
        if (cells && mine >= 0) {
            const cJSON* r1 = cJSON_GetArrayItem(cells, 1);
            gridOk = r1 && cJSON_IsString(cJSON_GetArrayItem(r1, 2)) && std::string(cJSON_GetArrayItem(r1, 2)->valuestring) == "20";
        }
        Check(AckOk(r) && k.num("timeData.itemIndex") == mine && gridOk, "timeFile -> lstTimeDataClick (golden :4843 (V912 :5074)) -> GetTimeDataText: the grid holds that CSV");
        ::DeleteFileA(probe.c_str());
        r = Act("timeFile", "{\"arg\":99}");
        Check(Has(r, "\"guard\":\"bad-payload\""), "timeFile outside the list -> bad-payload");
        r = Act("msgTab", "{\"arg\":7}");
        Check(Has(r, "\"guard\":\"bad-payload\""), "msgTab outside 0..3 -> bad-payload");
    }

    std::printf("[8] OB-8 Backup Log\n");
    {
        const char* ev = getenv("W906_EVENTLOG_ROOT");
        const std::string evRoot = ev ? ev : "D:\\HT9045_Log\\EventLogTxt";
        const char* const years[] = { "2026", "2026\" & del /q C:\\x & \"", "abc", "..\\..\\x", 0 };
        const int want[] = { 2026, 2026, 0, 0 };
        for (int i = 0; years[i]; ++i) {
            g_execCalls = 0;
            ::DeleteFileA(BAT.c_str());
            std::string yv;
            for (const char* c = years[i]; *c; ++c) { if (*c == '"' || *c == '\\') yv += '\\'; yv += *c; }
            const std::string r = Act("backupLogYear", "{\"year\":\"" + yv + "\"}");
            std::string bat;
            ReadAll(BAT, &bat);
            const std::vector<std::string> L = SplitLines(bat);
            char yy[16];
            std::snprintf(yy, sizeof(yy), "%d", want[i]);
            bool ok = AckOk(r) && L.size() == 13;
            for (int m = 1; ok && m <= 12; ++m) {
                char mm[8];
                std::snprintf(mm, sizeof(mm), "%02d", m);
                ok = L[m - 1] == "XCopy /y/a/e/c/i/h/f/r \"" + evRoot + "\\" + yy + "\\" + mm + "\" \"" + evRoot + "\\" + yy + "\\" + yy + "\"";
            }
            ok = ok && L[12] == L[11];
            Check(ok, std::string("year text '") + years[i] + "' -> 13 XCopy lines with the integer " + yy + " only, December twice (golden :5377-5385 (V912 :5608-5616))");
            Check(g_execCalls == 1 && g_execPath == BAT && g_execParam == " ", "the bat is run once through the hook with (bat, \" \") (golden :5388 (V912 :5619))");
        }
        Check(W906_E021_ExecHook == &RecordExec, "the recorder stayed installed (no command ran)");
    }

    std::printf("[9] OB-9 Clear Time Data\n");
    {
        LastSet.iJamCount[0] = 5; LastSet.iJamCount[1] = 6; LastSet.iJamCount[2] = 7;
        for (int k = 0; k < 3; ++k) for (int i = 0; i < 8; ++i) LastSet.SystemAccSecond[k][i] = 9;
        const std::string r = Act("clearTime", "{}");
        bool z = LastSet.iJamCount[0] == 0 && LastSet.iJamCount[1] == 0 && LastSet.iJamCount[2] == 0;
        for (int k = 0; k < 2; ++k) for (int i = 0; i < 8; ++i) z = z && LastSet.SystemAccSecond[k][i] == 0;
        Check(AckOk(r) && z, "iJamCount[0..2]=0 (golden :5395-5397 (V912 :5626-5628)), ClearCount(ctTimeData): SystemAccSecond[0..1][*]=0");
        Check(LastSet.SystemAccSecond[2][0] == 9, "SystemAccSecond[2] untouched (golden ClearCount k<2, cCounterClear.cpp ctTimeData)");
    }

    std::printf("[10] observer.get + ratchets\n");
    {
        try {
            const std::string o = W906_ObserverJson("year", -1, "2026");
            Check(Has(o, "\"dataRecord\":{\"b01\":") && Has(o, "\"tabVisible\""), "observer.get (full reply) carries dataRecord");
        } catch (const std::exception& x) {
            Check(false, std::string("observer.get year threw: ") + x.what());
        }
        std::string chan, obs, edge, click, html, wire, ev;
        const bool r1 = ReadAll(root + "\\JsonBridge\\ChanAction.cpp", &chan) && ReadAll(root + "\\cObserver.cpp", &obs) &&
                        ReadAll(root + "\\FileRW\\WindowEdgeTails.h", &edge) && ReadAll(root + "\\FileRW\\MainClick.cpp", &click);
        const bool r2 = ReadAll(web + "\\Data.Observer.html", &html) && ReadAll(web + "\\ht9045_observer_wire.js", &wire) &&
                        ReadAll(web + "\\ht9045_observer_ev.js", &ev);
        Check(r1 && r2, "sources readable (argv[1] port tree, argv[2] web\\page)");
        chan = StripComments(chan); obs = StripComments(obs); edge = StripComments(edge); click = StripComments(click);
        html = StripHtmlComments(html); wire = StripComments(wire); ev = StripComments(ev);
        Check(Has(chan, "if (cmd.compare(0, 13, \"act.observer.\") == 0) { std::string W906_ObserverAct(const std::string&, const std::string&); return W906_ObserverAct(cmd, payloadJson); }"),
              "ChanAction.cpp dispatches act.observer.* to W906_ObserverAct");
        Check(Has(obs, "j += \",\\\"testInfo\\\":\" + W906Obs_TestInfoJson(f);   { std::string W906_E021_DataRecordJson(TfObserver*); j += \",\\\"dataRecord\\\":\" + W906_E021_DataRecordJson(f); }"),
              "W906_ObserverJson's full reply appends dataRecord (same line as testInfo)");
        Check(Has(obs, "bool (*W906_E021_ExecHook)(AnsiString Path, AnsiString Param) = &ExecZipCommand;"), "the exec hook's default is golden's ExecZipCommand (source)");
        const std::string s1 = Body(obs, "bool W906_E021_SavePrecautionMemoInformation("), s2 = Body(obs, "bool W906_E021_SaveMajorMaintenanceInformation(");
        const std::size_t a1 = s1.find("W906_E021_RecordNameRefused("), b1 = s1.find("MyForceDirectories("), c1 = s1.find("SaveToFile(");
        const std::size_t a2 = s2.find("W906_E021_RecordNameRefused("), b2 = s2.find("MyForceDirectories("), c2 = s2.find("SaveToFile(");
        Check(a1 != std::string::npos && a1 < b1 && b1 < c1 && a2 != std::string::npos && a2 < b2 && b2 < c2,
              "both record saves check the name before their first write (mkdir, SaveToFile)");
        const std::string bk = Body(obs, "std::string W906_E021_btnBackupLogYearClick(");
        Check(Has(bk, "iYear=atoi(f->cbbEventLogYear->Text.c_str());") && Has(bk, "str2.sprintf(\"%s\\\\%d\\\\%d\",evRoot.c_str(),iYear,iYear);") &&
              Has(bk, "str1.sprintf(\"%s\\\\%d\\\\%02d\",evRoot.c_str(),iYear,i);") && bk.find("cbbEventLogYear->Text") == bk.rfind("cbbEventLogYear->Text") &&
              Has(bk, "*ran = W906_E021_ExecHook(bat, \" \");"), "OB-8: only atoi's integer reaches the bat; the bat runs through the hook");
        Check(Has(edge, "{\"fObserver\",        \"cObserver.cpp:654-674 TfObserver::FormClose (906 = V912 same lines; + SavePrecautionParameter 906 :4154-4183, V912 :4385-4414)\", false, true,  false}") &&
              Has(edge, "const char* FileRW_Observer_WindowEdge(bool open);") && Has(click, "&FileRW_Observer_WindowEdge,"),
              "window-edge row fObserver (close only, runs while running too) + MainClick.cpp's function table");
        const std::size_t w1 = html.find("<script src=\"ht9045_observer_wire.js\"></script>"), w2 = html.find("<script src=\"ht9045_observer_ev.js\"></script>");
        Check(w1 != std::string::npos && w2 != std::string::npos && w1 < w2, "Data.Observer.html loads ht9045_observer_ev.js after ht9045_observer_wire.js");
        Check(Has(wire, "if (d.dataRecord && window.HT9045ObserverEv) block('dataRecord', d.dataRecord, window.HT9045ObserverEv.render);") &&
              Has(wire, "if (b && !window.HT9045ObserverEv) b.addEventListener('click', function () {"), "ht9045_observer_wire.js: the render hook and the Backup Log guard");
        const char* const ops[] = { "prNoteSet", "prRecordSet", "prRecordClear", "prFormShow", "prSave", "prStartDate", "prFinishDate", "mmDate", "mmStart",
                                    "mmEnd", "mmPhenAdd", "mmCmAdd", "mmPhenClear", "mmCmClear", "mmSave", "mmSearch", "prLogSearch", "clearTime",
                                    "backupLogYear", 0 };
        bool all = Has(ev, "'act.observer.' + op") && Has(ev, "send('exit'") && Has(ev, "act('msgTab'") && Has(ev, "act('timeFile'");
        for (int i = 0; ops[i]; ++i) all = all && Has(ev, (std::string("'") + ops[i] + "'").c_str());
        Check(all, "ht9045_observer_ev.js sends every act.observer.* op");
    }

    std::printf("[12] Big5 (cp950) as golden\n");
    {
        f->bShow = true;
        IniConfig.bB01_UsePrecautionRecordFunction = true;
        IniConfig.bB02_HanderMajorMaintenanceRecordFunction = true;
        IniConfig.asB01_PrecautionRecordSavePath = "";
        IniConfig.asB02_HanderMajorMaintenanceRecordSavePath = "";
        // (a) a Chinese note round-trips
        Act("prStartDate", "{}");
        Act("prFinishDate", "{}");
        const std::string note = "測試注意";                                   // Big5 B4FA B8D5 AA60 B74E
        const std::string pr = "\"edPrecautionRecordDocumentNo\":\"文件-1\",\"edNoteContents\":\"" + note + "\",\"edApprovedManager\":\"主管\","
                               "\"edWatchmakers\":\"製表\",\"edFinishName\":\"結案\",\"edPromptDay\":\"3\",\"cobPRFinishType\":{\"itemIndex\":0},"
                               "\"MemoHandlerPrecautionRecord\":[\"卡料 請注意\",\"碁銹裏\"]";   // F9D6-F9D8: cp950's ETen extension
        const std::vector<std::string> before = FilesW(PR);
        std::string r = Act("prSave", W(pr));
        std::vector<std::string> after = FilesW(PR);
        std::string mine;
        for (std::size_t i = 0; i < after.size(); ++i) {
            bool old = false;
            for (std::size_t k = 0; k < before.size(); ++k) old = old || before[k] == after[i];
            if (!old) mine = after[i];
        }
        Check(AckOk(r) && after.size() == before.size() + 1 && mine.compare(0, 6, "HTEST_") == 0 &&
              mine.size() > note.size() + 5 && mine.substr(mine.size() - note.size() - 5) == "_" + note + ".txt",
              "Chinese note -> one file whose Unicode name ends _" + note + ".txt (wide-char API = what golden's Big5 ANSI name becomes on NTFS)");
        std::string bytes;
        ReadAllW(PR + "\\" + mine, &bytes);
        const std::string kLabel = "\xAA\x60\xB7\x4E\xA8\xC6\xB6\xB5: \xB4\xFA\xB8\xD5\xAA\x60\xB7\x4E";   // "注意事項: 測試注意" in Big5
        Check(bytes.find(kLabel) != std::string::npos && bytes.find("\xF9\xD6\xF9\xD7\xF9\xD8") != std::string::npos &&
              bytes.find("\xE6\xB3\xA8") == std::string::npos, "the bytes on disk are Big5 (known label + note bytes, ETen F9D6-F9D8), no UTF-8 sequence");
        const std::vector<std::string> L = SplitLines(FromBig5(bytes));
        Check(L.size() == 12 && L[0] == "DOCUMENT NO.: 文件-1" && L[1] == "注意事項: " + note && L[3] == "卡料 請注意" && L[4] == "碁銹裏" &&
              L[5] == "部門主管核准: 主管" && L[10] == "結案方式: By MO" && L[11] == "預計結案天數: 3", "decoded as cp950 the lines are golden's (:4094-4115, V912 :4325-4346)");
        Check(L.size() == 12 && bytes.size() >= 2 && bytes.compare(bytes.size() - 2, 2, "\r\n") == 0, "CRLF after every line (BCB6 SaveToFile)");
        // Search reads it back
        const int ix = IndexOfItem(f->cobSearchPrecautionLog, mine);
        char a1[64];
        std::snprintf(a1, sizeof(a1), "{\"widgets\":{\"cobSearchPrecautionLog\":{\"itemIndex\":%d}}}", ix);
        r = Act("prLogSearch", a1);
        J j(r);
        const std::vector<std::string> m = j.strs("dataRecord.memos.MemoNoteLog");
        Check(ix >= 0 && AckOk(r) && j.str("dataRecord.panels.pnPrecautionLogDocumentNo") == "文件-1" &&
              j.str("dataRecord.panels.pnPrecautionLogNoteContents") == note && j.str("dataRecord.panels.pnPrecautionLogApprovedManager") == "主管" &&
              m.size() == 2 && m[0] == "卡料 請注意" && m[1] == "碁銹裏", "the log list holds the Unicode name; Search reads the Big5 file back exactly");

        // (b) golden-written Big5 fixtures with Chinese names, found and parsed
        const std::string fxPr = "HTEST_2025_01_02_03_04_05_舊紀錄.txt";
        WriteAllW(PR + "\\" + fxPr, ToBig5("DOCUMENT NO.: 舊-9\r\n注意事項: 舊紀錄\r\n注意事項內容: \r\n第一行\r\n部門主管核准: 王\r\n製表者: 李\r\n"
                                           "結案者: 陳\r\n開始日期: 2025/01/02\r\n結案日期: 2025/01/03\r\n結案方式: By Day\r\n預計結案天數: 7\r\n"));
        W906_E021_LoadPrecautionLogMenu(f);
        const int fx = IndexOfItem(f->cobSearchPrecautionLog, fxPr);
        std::snprintf(a1, sizeof(a1), "{\"widgets\":{\"cobSearchPrecautionLog\":{\"itemIndex\":%d}}}", fx);
        r = Act("prLogSearch", a1);
        J k(r);
        Check(fx >= 0 && AckOk(r) && k.str("dataRecord.panels.pnPrecautionLogNoteContents") == "舊紀錄" && k.str("dataRecord.panels.pnPrecautionLogWatchmakers") == "李" &&
              k.str("dataRecord.panels.pnPrecautionLogPromptDay") == "7" && k.strs("dataRecord.memos.MemoNoteLog").size() == 1 &&
              k.strs("dataRecord.memos.MemoNoteLog")[0] == "第一行", "a golden Big5 Precaution record (Chinese name) is listed and parsed");
        const std::string fxMm = "HTEST_2025_02_03_04_05_06_卡料.txt";
        MakeDirs(MM);
        WriteAllW(MM + "\\" + fxMm, ToBig5("班別: B\r\n日期: 2025/02/03\r\n開始時間: 08:00\r\n結束時間: 09:30\r\n確認批號: 批-1\r\n維修者: 張\r\n"
                                           "確認者: 林\r\n不良現象: \r\n卡料\r\n吸嘴: 破損\r\n處理對策: \r\n更換吸嘴\r\n"));
        W906_E021_LoadMajorMaintenanceLogMenu(f);
        const int fm = IndexOfItem(f->cobMajorMaintenanceSearch, fxMm);
        std::snprintf(a1, sizeof(a1), "{\"widgets\":{\"cobMajorMaintenanceSearch\":{\"itemIndex\":%d}}}", fm);
        r = Act("mmSearch", a1);
        J q(r);
        const std::vector<std::string> ph = q.strs("dataRecord.memos.MemoUndesirablePhenomenon"), cm = q.strs("dataRecord.memos.MemoCountermeasure");
        Check(fm >= 0 && AckOk(r) && q.num("dataRecord.combos.cobMajorMaintenanceClassType.itemIndex") == 1 && q.str("dataRecord.edits.edMajorMaintenanceCheckNo") == "批-1" &&
              q.str("dataRecord.edits.edMajorMaintenancePersonnel") == "張" && q.str("dataRecord.panels.pnMajorMaintenanceEndTime") == "09:30" &&
              ph.size() == 2 && ph[0] == "卡料" && ph[1] == "吸嘴: 破損" && cm.size() == 1 && cm[0] == "更換吸嘴",
              "a golden Big5 Major Maintenance record (Chinese name) is listed and parsed");

        Act("prStartDate", "{}");                       // the save in (a) cleared both panels (golden :4510-4511（V912 :4741-4742）)
        Act("prFinishDate", "{}");
        // (c) Q66: a Chinese note with ".." or "\" is still refused
        std::map<std::string, std::string> s0, s1;
        ListDir(PR, &s0);
        bool refused = true;
        const char* const q66[] = { "測試..檔", "測試\\\\檔", 0 };
        for (int i = 0; q66[i]; ++i) {
            std::string v = pr;
            v.replace(v.find(note), note.size(), q66[i]);
            r = Act("prSave", W(v));
            refused = refused && !AckOk(r) && Has(r, "\"guard\":\"record-name\"");
        }
        ListDir(PR, &s1);
        Check(refused && s0 == s1, "Q66: a Chinese note with \"..\" or \"\\\" -> record-name, nothing written");

        // (d) no Big5 form -> refused where it enters, nothing written anywhere
        std::map<std::string, std::string> m0, m1, sy0, sy1;
        ListDir(MM, &m0);
        ListDir(PR + "\\system", &sy0);
        const char* const nob5[] = { "简体", "😀", "x\xE2\x82\xAC" /* the euro sign is in cp950 (A3E1): accepted */, 0 };
        for (int i = 0; nob5[i]; ++i) {
            const int n0 = W906_ShowMyMessage_Count;
            std::string v = pr;
            v.replace(v.find(note), note.size(), nob5[i]);
            r = Act("prSave", W(v));
            if (i < 2)
                Check(!AckOk(r) && Has(r, "\"guard\":\"not-big5\"") && !Has(r, "\"dataRecord\"") && W906_ShowMyMessage_Count == n0 + 1 &&
                      std::string(f->edNoteContents->Text.c_str()) != nob5[i],
                      std::string("note '") + nob5[i] + "' -> not-big5 + a message, nothing applied, no dataRecord (the page keeps the text)");
            else
                Check(AckOk(r), "a character cp950 does have (the euro sign) is accepted");
        }
        r = Act("sync", W("\"MemoUndesirablePhenomenon\":[\"OK\",\"卡\xE4\xB8\xA2\"]"));   // U+4E22 "丢" (simplified)
        Check(!AckOk(r) && Has(r, "\"guard\":\"not-big5\""), "a memo line with a simplified character -> not-big5");
        ListDir(MM, &m1);
        ListDir(PR + "\\system", &sy1);
        std::vector<std::string> prNow = FilesW(PR);
        bool onlyEuro = prNow.size() == after.size() + 2;   // + the Precaution fixture + the euro note
        Check(onlyEuro && m0 == m1 && sy0 == sy1, "refused: nothing written (record folders, \\system)");

        // (e) PrecautionParameter.ini holds Big5 bytes
        Act("sync", W(pr));
        f->bShow = true;
        FileRW_Observer_WindowEdge(false);
        std::string ini;
        ReadAll(PR + "\\system\\PrecautionParameter.ini", &ini);
        Check(ini.find("Note Contents=\xB4\xFA\xB8\xD5\xAA\x60\xB7\x4E") != std::string::npos && ini.find("Note0=\xA5\x64\xAE\xC6") != std::string::npos,
              "PrecautionParameter.ini: Note Contents / Note0 are Big5 bytes (golden TIniFile = ANSI)");
        f->bShow = true;
    }

    std::printf("[11] real machine files\n");
    {
        ListDir("D:\\PrecautionRecord", &pr1);
        ListDir("D:\\PrecautionRecord\\system", &prs1);
        ListDir("D:\\MajorMaintenanceRecord", &mm1);
        ListDir("D:\\HT9045\\system", &sys1);
        ReadAll("D:\\HT9045\\config\\config.ini", &cfg1);
        Check(pr0 == pr1 && prs0 == prs1, "D:\\PrecautionRecord and \\system: listing unchanged");
        Check(mm0 == mm1, "D:\\MajorMaintenanceRecord: unchanged (absent stays absent)");
        Check(sys0 == sys1, "D:\\HT9045\\system: listing unchanged (no 2.bat written there)");
        Check(cfg0 == cfg1, "D:\\HT9045\\config\\config.ini unchanged");
    }

    std::printf("\nE021_Observer: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0) Wipe(sand);
    return g_fail == 0 ? 0 : 1;
}
