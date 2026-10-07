// =============================================================================
//  tests/test_note_auth.cpp -- todo D-026: the alarm note's password layer (Alert.Note, WS dialog.auth).
//  WebLogin.cpp's appended D-026 block (WebNoteAuth.h) = golden TfNote::FormShow's password block (V912 note.cpp:1496-1502,
//  :1818-1941), TfNote::DoPassword (:5277-5429) and TfNote::DoUnlockPassword (:5431-5445) as the Start() / BtnPauseClick()
//  presses reach them (:3584-3607 / :3906-3929, KeyCode==0 :3716-3727 / :4084-4095).
//  AI(W906-D026) 20261001 (St01).  Suite name (add_test): D026_NoteAuth
//
//    0. containment first (exit 2 / 1, nothing called): ctest's redirect environment (w906_ctest_guard.h) and the four seams
//       W906_PWBOOK_PATH / W906_LOGINDAT_PATH / W906_LEVELSET_PATH / W906_ALARMUNLOCK_PATH in the build dir's note_auth_scratch.
//       The jam-level file (golden FileNameJam000, a PRIVATE TfSecurity member with no production seam) is pointed at the
//       scratch folder through the friend W906_LevelSetPut (forms/fSecurity.h:561) -- its real body is WebLevelSet.cpp, which
//       only wb_serve links; the stand-in below does nothing else.  Every account and password is made up at run time (random).
//       stdout is captured into the scratch folder for the whole run.  Real files stamped: D:\HT9045\system\login.dat /
//       levelset.dat / lastdata.dat / Gerneral.ini, D:\HT9045\Error\English\JAM0000.dat, C:\Windows\AlarmUnlock.ini.
//    1. arming (FormShow): sJamArea "%02d %s" (ShowErrorMessage :882-886), Level from the jam file, bNeedPassWord, TempCode;
//       not recorded -> no password; Level 0 -> no auth object (mailbox bytes unchanged); the auth object's fields.
//    2. blocking alarm, password book: no pass -> refused; wrong -> WAR1677 + Operator; right -> "Login for unlock alarm" row,
//       logged out to Operator, one-shot pass (same K / pressed only; any connection -- Steven Q64 (3)); too low; cancelled;
//       CC_PTI keeps the level;
//       bad payloads, wrong target, not offered, no pending dialog.
//    3. blocking alarm, drop-down: the note's stOperatorClick arm (Supervisor slot, then Engineer slot), sSuperVisorString;
//       no logout in this mode.
//    4. golden adjustments: F15 (JAM0508 / JAM0509 only SKIP asks), VTEST WAR16123 + RETRY, SCC list, KYEC_LEE + N07,
//       barcode WAR04217, O16 same-alarm count, statistics level-up, bWaitSecsGemReply.
//       //AI(W906-E030) 20261003 (St01): WAR04217 改成釘 golden 906（note.cpp FormShow :1922-1927、DoPassword :5276-5278 沒有 V912
//       //  :1937-1942 / :5318-5323 的 CSV Compare 規則）：開著 CSV Compare 也只看 JAM 等級表（0 級不問；2 級要 2 級，工程師不夠）。
//    5. DoUnlockPassword: the AlarmUnlock.ini seam, wrong / right, unlock + login in two steps, a missing file never matches.
//    6. kCode==0 notice: NoticeGate, ACKNOWLEDGE only, pass one-shot, the page's BtnPause (the ack carries no button), the
//       machine running again (no press).
//    7. panel keys (IoGate): refused while golden would ask; a refused panel key leaves the screen's pass alone.
//    8. mailbox JSON: AlarmRequestJson("" auth) is today's bytes; with the object it is spliced in.
//   8b. AI(W906-D034) 20261002: the SpecialPanel password (golden 906 FormShow :1564-1606, PanSpecialNoteClick :5442-5460; V912 :1574-1616 / :5489-5507) -- the
//       SpecialErrNote.ini copy in the scratch folder (asErrNotePath), the flags, the lock on every press / panel key / notice, the
//       "special-note" auth object, wrong / cancelled / right, the golden globals' stickiness, TestSuck != 1 and no file, WAR07352 +
//       CC_LINGSEN, then a login behind it.  The special password is random and never printed (step 10 scans for it too).
//    9. source ratchets (comments stripped, '\r' dropped): the call sites in tools/wb_serve.cpp, WebBridge/WebBridgeServer.cpp,
//       tools/wb_dialog_mailbox.h.  Control run: W906_D026_SRC_ROOT at a pre-change copy of those three files must go red.
//   10. no password in any output: replies, state JSON, the auth object, the captured stdout, the log files of the run.
//   11. the machine's own files: unchanged.
//  ht9045::formjson::FormLock / FormUnlock (JsonBridge/FormJson.cpp, wb_serve only) are empty stand-ins (as test_weblogin_reauth).
// =============================================================================
#include "WebLogin.h"
#include "WebNoteAuth.h"
#include "cMyDB.h"         // ExString
#include "cmydef.h"        // AccessLevel, pwPath, sSuperVisorString, CUSTOMER_CODE, K_*, bWaitSecsGemReply, asUnlockPassword
#include "common.h"        // OpenGeneralIniFile, CloseIniFile, log roots, asGeneralPath
#include "CosFunction.h"
#include "Config.h"
#include "MachineType.h"
#include "cprod.h"         // USER, LevelSet, TestIF_File
#include "forms/fMain.h"   // AlarmCodeMap
#include "forms/fNote.h"
#include "forms/fSecurity.h"
#include "Public/cJSON.h"
#pragma GCC diagnostic push                  // the header's own -Wall findings (its :60 comment, :495 / :522 %llu under MinGW 6.3) are not this test's
#pragma GCC diagnostic ignored "-Wcomment"
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#include "tools/wb_dialog_mailbox.h"
#pragma GCC diagnostic pop
#include "w906_ctest_guard.h"

#include <windows.h>
#include <io.h>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace ht9045 { namespace formjson {
void FormLock()   { }
void FormUnlock() { }
} }

// friend of TfSecurity (forms/fSecurity.h:561); the real body (WebLevelSet.cpp) is linked into wb_serve only.  Used here ONLY to point
// the private FileNameJam000 at the sandbox (step 0).
std::string W906_LevelSetPut(const std::string& tag, const std::string& payload, bool, bool* ok)
{
    if (tag == "d026-jamfile" && fSecurity) fSecurity->FileNameJam000 = AnsiString(payload.c_str());
    if (ok) *ok = true;
    return std::string();
}
extern bool W906_ShowErrorMessage_Recorded;          // forms/fNote_ShowError.cpp:73

static int g_fail = 0, g_total = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL: %s  (line %d)\n", what, line); }
    else     { std::printf("  PASS: %s\n", what); }
}
#define CHECK(c) check((c), #c, __LINE__)

static bool Has(const std::string& s, const std::string& sub) { return s.find(sub) != std::string::npos; }
static std::string Lower(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i) { if (s[i] == '/') s[i] = '\\'; s[i] = (char)std::tolower((unsigned char)s[i]); }
    return s;
}

// ---- made-up credentials (random per run) -------------------------------------------------------------------------
static unsigned g_seed = 0;
static std::string Rand(const char* prefix, int n)
{
    static const char kAl[] = "abcdefghjkmnpqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    std::string s(prefix);
    for (int i = 0; i < n; ++i) { g_seed = g_seed * 1103515245u + 12345u; s += kAl[(g_seed >> 16) % (sizeof(kAl) - 1)]; }
    return s;
}
static std::vector<std::string> g_secrets;
static std::vector<std::string> g_outputs;
static void Out(const std::string& s) { g_outputs.push_back(s); }

struct Stamp { bool exists; unsigned long long size; FILETIME write; };
static Stamp StampOf(const char* p)
{
    Stamp s = { false, 0, { 0, 0 } };
    WIN32_FILE_ATTRIBUTE_DATA a;
    if (::GetFileAttributesExA(p, GetFileExInfoStandard, &a)) {
        s.exists = true;
        s.size = ((unsigned long long)a.nFileSizeHigh << 32) | a.nFileSizeLow;
        s.write = a.ftLastWriteTime;
    }
    return s;
}
static bool Same(const Stamp& a, const Stamp& b)
{
    return a.exists == b.exists && a.size == b.size &&
           a.write.dwLowDateTime == b.write.dwLowDateTime && a.write.dwHighDateTime == b.write.dwHighDateTime;
}
static std::string ReadFile(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
static void WriteText(const std::string& p, const std::string& body)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (!f) return;
    std::fwrite(body.data(), 1, body.size(), f);
    std::fclose(f);
}
static std::string NoCR(std::string s) { std::string o; o.reserve(s.size()); for (char c : s) if (c != '\r') o += c; return o; }   // RULE 0b: gate checkout is CRLF

// ---- the scratch folder ---------------------------------------------------------------------------------------------
static std::string g_dir, g_book, g_jam, g_unlock;
static std::string g_eng, g_sup, g_hon;              // book passwords (levels 1 / 2 / 3)
static void WriteBook()
{
    WriteText(g_book, "d26eng 1 " + g_eng + "\r\nd26sup 2 " + g_sup + "\r\nd26hon 3 " + g_hon + "\r\n");
}
static void RemoveBook() { ::DeleteFileA(g_book.c_str()); }
static void WriteJam(const std::string& body)          // the INI cache first (CloseIniFile flushes it), then the new content
{
    CloseIniFile();
    WriteText(g_jam, body);
}

// ---- helpers --------------------------------------------------------------------------------------------------------
static void Arm(const char* qid, const char* code, int kcode, bool recorded = true)
{
    W906_ShowErrorMessage_Recorded = recorded;
    fMain->AlarmCodeMap[AnsiString(code).UpperCase()] = AnsiString("made-up message");
    fNote->edErrorCode->Text = AnsiString(code);           // golden ErrShowToForm (the record half does it)
    W906_NoteAuthArm(qid, code, kcode, false);
}
static std::string State(const char* qid) { const std::string s = W906_NoteAuthStateJson(qid); Out(s); return s; }
struct Reply { bool ok = false; std::string raw; bool accepted = false; bool asked = false; std::string stage, alarm, message; int level = -1; bool loggedOut = false; bool cancelled = false; };
static Reply Verify(const std::string& rid, const std::string& current, bool blocking, const char* action, const char* pressed,
                    const char* user, const std::string& pw, bool cancelled = false)
{
    std::string v = "{\"schemaVersion\":\"1.0.0\",\"channel\":\"dialog-auth\",\"authId\":\"a-" + rid + "\",\"state\":\"pending\","
                    "\"target\":{\"channel\":\"show-error-message\",\"requestId\":\"" + rid + "\",\"requestSeq\":1},"
                    "\"kind\":\"access-level\",\"level\":null,"
                    "\"pendingAction\":{\"name\":\"" + std::string(action) + "\",\"code\":1,\"pressedButton\":" +
                    (pressed ? "\"" + std::string(pressed) + "\"" : std::string("null")) + "}";
    if (cancelled) v += ",\"cancelled\":true}";
    else v += ",\"credentials\":{\"userId\":" + (user ? "\"" + std::string(user) + "\"" : std::string("null")) + ",\"password\":\"" + pw + "\"}}";
    Reply r;
    r.ok = W906_NoteAuthVerify("a-" + rid, v, current, blocking, &r.raw);
    Out(r.raw);
    cJSON* o = r.ok ? cJSON_Parse(r.raw.c_str()) : 0;
    if (o) {
        r.accepted = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(o, "accepted")) != 0;
        r.asked = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(o, "asked")) != 0;
        r.loggedOut = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(o, "loggedOut")) != 0;
        r.cancelled = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(o, "cancelled")) != 0;
        const cJSON* s = cJSON_GetObjectItemCaseSensitive(o, "stage");   if (cJSON_IsString(s)) r.stage = s->valuestring;
        const cJSON* a = cJSON_GetObjectItemCaseSensitive(o, "alarm");   if (cJSON_IsString(a)) r.alarm = a->valuestring;
        const cJSON* m = cJSON_GetObjectItemCaseSensitive(o, "message"); if (cJSON_IsString(m)) r.message = m->valuestring;
        const cJSON* l = cJSON_GetObjectItemCaseSensitive(o, "level");   if (cJSON_IsNumber(l)) r.level = l->valueint;
        cJSON_Delete(o);
    }
    std::printf("    verify %s %s:%s -> ok=%d accepted=%d asked=%d stage=%s level=%d\n", rid.c_str(), action, pressed ? pressed : "-",
                (int)r.ok, (int)r.accepted, (int)r.asked, r.stage.c_str(), r.level);
    return r;
}
static bool Gate(const char* qid, int k, const char* pressed, std::string* why = 0)
{
    std::string w;
    const bool g = W906_NoteAuthAnswerGate(qid, k, pressed ? pressed : "", &w);
    Out(w);
    if (why) *why = w;
    return g;
}
static std::string AuthJson(const char* qid) { const std::string s = W906_NoteAuthRequestJson(qid); Out(s); return s; }
static void Logout() { std::string m; WebLogin_Logout(&m); }

// ---- the code part of a C++ file (string literals kept, comments dropped) -------------------------------------------
static std::string CodeOf(const std::string& text)
{
    std::string out;
    bool block = false;
    for (size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        if (block) { if (c == '*' && i + 1 < text.size() && text[i + 1] == '/') { block = false; ++i; } continue; }
        if (c == '"' || c == '\'') {
            const char q = c;
            out += c;
            for (++i; i < text.size(); ++i) {
                out += text[i];
                if (text[i] == '\\' && i + 1 < text.size()) { out += text[++i]; continue; }
                if (text[i] == q || text[i] == '\n') break;
            }
            continue;
        }
        if (c == '/' && i + 1 < text.size() && text[i + 1] == '/') { while (i < text.size() && text[i] != '\n') ++i; out += '\n'; continue; }
        if (c == '/' && i + 1 < text.size() && text[i + 1] == '*') { block = true; ++i; continue; }
        out += c;
    }
    return out;
}

static void Walk(const std::string& dir, const FILETIME& since, std::vector<std::string>* out, int depth)
{
    if (depth > 6) return;
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        const std::string n = fd.cFileName;
        if (n == "." || n == "..") continue;
        const std::string p = dir + "\\" + n;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { Walk(p, since, out, depth + 1); continue; }
        if (::CompareFileTime(&fd.ftLastWriteTime, &since) >= 0 && fd.nFileSizeHigh == 0 && fd.nFileSizeLow < 8u * 1024u * 1024u)
            out->push_back(p);
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}

int main(int argc, char** argv)
{
    // ---- 0. containment first -------------------------------------------------------------------------------------
    std::printf("D026_NoteAuth\n");
    if (!W906TestRequireCtestRedirects("D026_NoteAuth"))
        return 2;
    const char* const seamNames[] = { "W906_PWBOOK_PATH", "W906_LOGINDAT_PATH", "W906_LEVELSET_PATH", "W906_ALARMUNLOCK_PATH" };
    bool sandboxed = true;
    for (int i = 0; i < 4; ++i) {
        const char* v = std::getenv(seamNames[i]);
        std::printf("  %s = %s\n", seamNames[i], v ? v : "(unset)");
        if (v == 0 || *v == 0 || !Has(Lower(v), "note_auth_scratch")) sandboxed = false;
    }
    const AnsiString* const roots[] = { &as9045LogPath, &asSaveEventLogPath, &asProductionLogPath, &sProductionInfoFilePath };
    for (int i = 0; i < 4; ++i)
        if (!Has(roots[i]->c_str(), "machine_log_scratch")) sandboxed = false;
    if (!Has(Lower(asGeneralPath.c_str()), "general_ini_scratch")) sandboxed = false;
    CHECK(sandboxed);
    if (!sandboxed) { std::printf("FAIL: not sandboxed -- refusing to run the note password code\n"); return 1; }
    static const char* const kReal[] = { "D:\\HT9045\\system\\login.dat", "D:\\HT9045\\system\\levelset.dat",
                                         "D:\\HT9045\\system\\lastdata.dat", "D:\\HT9045\\system\\Gerneral.ini",
                                         "D:\\HT9045\\Error\\English\\JAM0000.dat", "C:\\Windows\\AlarmUnlock.ini",
                                         "D:\\HT9045\\system\\SpecialErrNote.ini" };   // AI(W906-D034): golden asErrNotePath
    const int nReal = (int)(sizeof(kReal) / sizeof(kReal[0]));
    Stamp real0[7];
    for (int i = 0; i < nReal; ++i) real0[i] = StampOf(kReal[i]);

    g_book = std::getenv("W906_PWBOOK_PATH");
    g_unlock = std::getenv("W906_ALARMUNLOCK_PATH");
    for (size_t i = 0; i < g_book.size(); ++i) if (g_book[i] == '/') g_book[i] = '\\';
    for (size_t i = 0; i < g_unlock.size(); ++i) if (g_unlock[i] == '/') g_unlock[i] = '\\';
    g_dir = g_book.substr(0, g_book.find_last_of('\\'));
    ::CreateDirectoryA(g_dir.c_str(), 0);
    g_jam = g_dir + "\\JAM0000.dat";
    const std::string capture = g_dir + "\\stdout_capture.txt";
    FILETIME t0;
    ::GetSystemTimeAsFileTime(&t0);
    { ULARGE_INTEGER u; u.LowPart = t0.dwLowDateTime; u.HighPart = t0.dwHighDateTime; u.QuadPart -= 10000000ull; t0.dwLowDateTime = u.LowPart; t0.dwHighDateTime = u.HighPart; }

    LARGE_INTEGER qpc; ::QueryPerformanceCounter(&qpc);
    g_seed = (unsigned)qpc.LowPart ^ (unsigned)::GetCurrentProcessId() ^ (unsigned)::GetTickCount();
    g_eng = Rand("e", 9); g_sup = Rand("s", 9); g_hon = Rand("h", 9);
    const std::string selEng = Rand("E", 9), selSup = Rand("S", 9), superPw = Rand("V", 11), wrongPw = Rand("w", 9), unlockPw = Rand("U", 10);
    g_secrets.push_back(g_eng); g_secrets.push_back(g_sup); g_secrets.push_back(g_hon); g_secrets.push_back(selEng);
    g_secrets.push_back(selSup); g_secrets.push_back(superPw); g_secrets.push_back(wrongPw); g_secrets.push_back(unlockPw);

    std::fflush(stdout);
    const int savedOut = _dup(1);
    if (!std::freopen(capture.c_str(), "w", stdout)) { std::fprintf(stderr, "cannot capture stdout\n"); return 1; }

    // what the run changes; restored at the end
    const AnsiString savedPwPath = pwPath, savedSuper = sSuperVisorString;
    const bool savedBookFlag = CosFunction.bUseLoginDatToSetLevel, saved5 = CosFunction.bSecurityHave5Level, savedSecret = IniConfig.bPasswordSecret;
    const bool savedSpil = IniConfig.bSPILFunction, savedShowName = CosFunction.bLoginShowUserName, savedUnlockOn = CosFunction.bUseAlarmUnlockPassWord;
    const bool savedF15 = IniConfig.bF15OutShuttleLoseICNeedPWD, savedStat = CosFunction.bStatisticsJamCount, savedO16c = CosFunction.bConAlarmNeedKeyInPassword;
    const bool savedO16 = IniConfig.bO16ConAlarmNeedKeyInPasswordCT, savedO17c = CosFunction.bConAlarmInTimeLevelUp, savedO17 = IniConfig.bO17EnableLevelUpWhenContiAlarm;
    const bool savedN07 = IniConfig.bN07_EnableEmployeeIdCheak, savedBar = TestIF_File.bEnableBarCode, savedBarCsv = TestIF_File.bEnableBarcodeCSVCompare;
    const bool savedHrs = CosFunction.bEnableHandlerResultServer;
    const int  savedVtest = IniConfig.bVTESTFunction, savedO16n = IniConfig.iO16ConAlarmNeedKeyInPasswordCT;
    const int savedCustomer = CUSTOMER_CODE, savedLevel = AccessLevel, savedJcet = JCET_FOR_EVAN;
    const int saved35 = LevelSet.AccessLevel[35];
    const bool savedRecorded = W906_ShowErrorMessage_Recorded, savedWait = bWaitSecsGemReply, savedSys = SystemStart, savedSoft = SoftStart;
    const AnsiString savedErrNote = asErrNotePath;                                       // AI(W906-D034)
    const bool savedIdxPwd = IniConfig.bIndexDropNeedPwdByIni, savedTimeUpPwd = IniConfig.bTesterTimeUpErrorNeedPassword;
    const int savedContiR = iTestTimeUpErrContinueR;
    static PASS_WORD savedUser;
    std::memcpy(&savedUser, &USER, sizeof(PASS_WORD));

    pwPath = AnsiString((g_dir + "\\no_such_tech.com").c_str());
    CosFunction.bUseLoginDatToSetLevel = false;
    CosFunction.bSecurityHave5Level = false;
    CosFunction.bLoginShowUserName = false;
    CosFunction.bUseAlarmUnlockPassWord = false;
    CosFunction.bStatisticsJamCount = false;
    CosFunction.bConAlarmNeedKeyInPassword = false;
    CosFunction.bConAlarmInTimeLevelUp = false;
    CosFunction.bEnableHandlerResultServer = false;
    IniConfig.bPasswordSecret = false;
    IniConfig.bSPILFunction = false;
    IniConfig.bF15OutShuttleLoseICNeedPWD = false;
    IniConfig.bO16ConAlarmNeedKeyInPasswordCT = false;
    IniConfig.bO17EnableLevelUpWhenContiAlarm = false;
    IniConfig.bN07_EnableEmployeeIdCheak = false;
    IniConfig.bVTESTFunction = 0;
    IniConfig.bIndexDropNeedPwdByIni = false;                                    // AI(W906-D034): no SpecialPanel lock unless 8b turns it on
    IniConfig.bTesterTimeUpErrorNeedPassword = false;
    asErrNotePath = AnsiString((g_dir + "\\no_such_SpecialErrNote.ini").c_str());   // never the machine's D:\HT9045\system\SpecialErrNote.ini
    TestIF_File.bEnableBarCode = false;
    TestIF_File.bEnableBarcodeCSVCompare = false;
    CUSTOMER_CODE = CC_HONPREC_QC;
    JCET_FOR_EVAN = 0;
    bWaitSecsGemReply = false;
    SystemStart = false;
    SoftStart = false;
    sSuperVisorString = AnsiString(superPw.c_str());
    std::memset(&USER, 0, sizeof(PASS_WORD));
    std::strcpy(USER.PassWord[0], selEng.c_str());       // drop-down Engineer slot
    std::strcpy(USER.PassWord[1], selSup.c_str());       // drop-down Supervisor slot
    OpenGeneralIniFile();
    LevelSet.AccessLevel[35] = 0;                         // GetJamLevel raises a fixed code list to levelset item 35 for most customers (cSecurity.cpp default arm)
    { bool ok = false; W906_LevelSetPut("d026-jamfile", g_jam, false, &ok); }
    WriteJam("[03 Index Unit]\r\nJAM0301=2\r\nJAM0302=0\r\n[05 Output Shuttle]\r\nJAM0508=2\r\n[16 System]\r\nWAR16123=2\r\n");
    W906_NoteAuthResetForTest();

    // ---- 1. arming ------------------------------------------------------------------------------------------------
    std::printf("  -- 1. arming (golden TfNote::FormShow's password block)\n");
    {
        Arm("101", "jam0301", K_RETRY | K_SKIP);
        const std::string s = State("101");
        std::printf("    state: %s\n", s.c_str());
        CHECK(Has(s, "\"sJamArea\":\"03 Index Unit\"") && Has(s, "\"sJamCode\":\"JAM0301\"") && Has(s, "\"tempCode\":\"jam0301\""));
        CHECK(Has(s, "\"Level\":2") && Has(s, "\"bNeedPassWord\":true") && Has(s, "\"shown\":true"));
        CHECK(std::string(fNote->sJamArea.c_str()) == "03 Index Unit" && std::string(fNote->sJamCode.c_str()) == "JAM0301");   // golden :885-886
        const std::string j = AuthJson("101");
        std::printf("    auth (select mode): %s\n", j.c_str());
        CHECK(Has(j, "\"required\":true") && Has(j, "\"kind\":\"access-level\"") && Has(j, "\"level\":2") && Has(j, "\"userIdRequired\":false"));
        CHECK(Has(j, "\"mode\":\"select\"") && Has(j, "\"logoutAfter\":false") && Has(j, "\"actions\":[\"SKIP\",\"RETRY\"]"));
        WriteBook();
        const std::string jb = AuthJson("101");
        CHECK(Has(jb, "\"userIdRequired\":true") && Has(jb, "\"mode\":\"book\"") && Has(jb, "\"logoutAfter\":true"));
        RemoveBook();

        Arm("102", "JAM0302", K_RETRY);                       // level 0 in the file
        CHECK(Has(State("102"), "\"bNeedPassWord\":false"));
        CHECK(AuthJson("102").empty());                       // -> the mailbox keeps today's required:false bytes
        CHECK(Gate("102", K_RETRY, "BtnStart"));

        Arm("103", "JAM0301", K_RETRY, false);                // the record half returned early: golden shows no note
        CHECK(Has(State("103"), "\"shown\":false"));
        CHECK(AuthJson("103").empty());
        CHECK(Gate("103", K_RETRY, "BtnStart"));
        CHECK(Has(State("101"), "\"Level\":2"));             // an earlier note keeps its own state
        LevelSet.AccessLevel[35] = 3;                         // GetJamLevel's default customer arm: JAM0302 is on its item-35 list
        Arm("104", "JAM0302", K_RETRY);
        CHECK(Has(State("104"), "\"Level\":3") && Has(State("104"), "\"bNeedPassWord\":true"));
        LevelSet.AccessLevel[35] = 0;
    }

    // ---- 2. blocking alarm, password book ---------------------------------------------------------------------------
    std::printf("  -- 2. blocking alarm, password book\n");
    {
        W906_NoteAuthResetForTest();                                            // every case starts clean: no notes, no pass, O16 / O17 counters 0
        WriteBook();
        Logout();
        Arm("201", "JAM0301", K_RETRY | K_SKIP);
        std::string why;
        CHECK(!Gate("201", K_RETRY, "BtnStart", &why));
        CHECK(Has(why, "auth-required") && Has(why, "DoPassword"));

        Reply r = Verify("201", "201", true, "RETRY", "BtnStart", "d26sup", wrongPw);
        CHECK(r.ok && !r.accepted && r.asked && r.alarm == "WAR1677" && r.loggedOut && AccessLevel == 0);
        CHECK(Has(r.message, "Wrong ID or password"));
        CHECK(!Gate("201", K_RETRY, "BtnStart"));

        r = Verify("201", "201", true, "RETRY", "BtnStart", "d26sup", g_sup);
        CHECK(r.ok && r.accepted && r.asked && r.loggedOut && AccessLevel == 0 && r.level == 0);   // golden :5418-5424 logout after
        CHECK(Has(r.raw, "\"accessLevel\":2"));
        CHECK(Has(State("201"), "\"pass\":true"));
        CHECK(!Gate("201", K_RETRY, "BtnPause"));            // another press: the pass is dropped
        CHECK(Has(State("201"), "\"pass\":false"));
        CHECK(!Gate("201", K_RETRY, "BtnStart"));            // and gone
        r = Verify("201", "201", true, "RETRY", "BtnStart", "d26sup", g_sup);
        CHECK(r.accepted);
        CHECK(Gate("201", K_RETRY, "BtnStart"));          // Steven Q64 (3): not tied to a connection / token / user -- any answer for this press uses it
        CHECK(!Gate("201", K_RETRY, "BtnStart"));         // once
        r = Verify("201", "201", true, "SKIP", "BtnPause", "d26hon", g_hon);
        CHECK(r.accepted);
        CHECK(Gate("201", K_SKIP, "BtnPause"));              // the same key / button: once
        CHECK(!Gate("201", K_SKIP, "BtnPause"));

        r = Verify("201", "201", true, "RETRY", "BtnStart", "d26eng", g_eng);   // level 1 < 2
        CHECK(r.ok && !r.accepted && r.asked && r.loggedOut && AccessLevel == 0 && Has(r.raw, "\"accessLevel\":1"));
        r = Verify("201", "201", true, "RETRY", "BtnStart", 0, "", true);      // cancelled = blank = wrong (Q45-4)
        CHECK(r.ok && !r.accepted && r.cancelled && r.alarm == "WAR1677" && AccessLevel == 0);
        r = Verify("201", "201", true, "RETRY", "BtnStart", "D26SUP", g_sup);  // user name compare is case-insensitive (golden)
        CHECK(r.accepted);

        CUSTOMER_CODE = CC_PTI;                                                 // 力成: the level stays after the unlock
        r = Verify("201", "201", true, "RETRY", "BtnStart", "d26sup", g_sup);
        CHECK(r.accepted && !r.loggedOut && AccessLevel == 2);
        CHECK(Gate("201", K_RETRY, "BtnStart"));             // that pass is used here, so the refusals below start with none
        CUSTOMER_CODE = CC_HONPREC_QC;
        Logout();

        CHECK(!Verify("201", "", true, "RETRY", "BtnStart", "d26sup", g_sup).ok);          // nothing pending
        Reply rw = Verify("201", "999", true, "RETRY", "BtnStart", "d26sup", g_sup);       // not the current dialog
        CHECK(!rw.ok && Has(rw.raw, "not-the-current-dialog"));
        Reply rn = Verify("201", "201", true, "TRAY_END", "BtnStart", "d26sup", g_sup);    // not offered by kcode
        CHECK(!rn.ok && Has(rn.raw, "not-an-offered-option"));
        Reply ra = Verify("201", "201", true, "ACKNOWLEDGE", 0, "d26sup", g_sup);
        CHECK(!ra.ok);
        std::string bad;
        CHECK(!W906_NoteAuthVerify("a", "[]", "201", true, &bad) && Has(bad, "bad-payload")); Out(bad);
        CHECK(!W906_NoteAuthVerify("a", "{\"target\":{\"requestId\":\"201\"},\"pendingAction\":{\"name\":\"RETRY\"},\"credentials\":{\"userId\":\"x\"}}", "201", true, &bad) &&
              Has(bad, "credentials.password")); Out(bad);
        CHECK(!W906_NoteAuthVerify("a", "{\"target\":{\"requestId\":\"201\"},\"pendingAction\":{\"name\":\"RETRY\",\"pressedButton\":\"BtnX\"},\"credentials\":{\"password\":\"y\"}}", "201", true, &bad)); Out(bad);
        CHECK(Has(State("201"), "\"pass\":false"));           // refused payloads never leave a pass
    }

    // ---- 3. blocking alarm, drop-down (no book) ---------------------------------------------------------------------
    std::printf("  -- 3. blocking alarm, drop-down\n");
    {
        W906_NoteAuthResetForTest();                                            // every case starts clean: no notes, no pass, O16 / O17 counters 0
        RemoveBook();
        AccessLevel = 0;
        Arm("301", "JAM0301", K_RETRY);
        Reply r = Verify("301", "301", true, "RETRY", "BtnStart", 0, selSup);   // Supervisor's slot, whatever the drop-down shows
        CHECK(r.accepted && r.asked && !r.loggedOut && AccessLevel == 2);       // no logout without a book (golden)
        CHECK(Gate("301", K_RETRY, "BtnStart"));
        AccessLevel = 0;
        r = Verify("301", "301", true, "RETRY", "BtnStart", 0, selEng);         // Engineer's slot: 1 < 2
        CHECK(!r.accepted && AccessLevel == 1);
        r = Verify("301", "301", true, "RETRY", "BtnStart", 0, superPw);        // sSuperVisorString -> HonPrec
        CHECK(r.accepted && AccessLevel == 3);
        r = Verify("301", "301", true, "RETRY", "BtnStart", 0, wrongPw);
        CHECK(!r.accepted && AccessLevel == 0 && r.alarm.empty());
        AccessLevel = 0;
    }

    // ---- 4. golden adjustments --------------------------------------------------------------------------------------
    std::printf("  -- 4. golden adjustments\n");
    {
        W906_NoteAuthResetForTest();                                            // every case starts clean: no notes, no pass, O16 / O17 counters 0
        WriteBook();
        IniConfig.bF15OutShuttleLoseICNeedPWD = true;                            // only SKIP asks (TempCode JAM0508)
        Arm("401", "JAM0508", K_RETRY | K_SKIP);
        CHECK(Has(State("401"), "\"sJamArea\":\"05 Output Shuttle\"") && Has(State("401"), "\"Level\":2"));
        const std::string j = AuthJson("401");
        CHECK(Has(j, "\"actions\":[\"SKIP\"]"));
        Reply r = Verify("401", "401", true, "RETRY", "BtnStart", "d26sup", wrongPw);
        CHECK(r.accepted && !r.asked);                                           // golden sets bNeedPassWord=false and does not ask
        CHECK(Has(State("401"), "\"bNeedPassWord\":false"));                     // and it sticks (golden member)
        Arm("402", "JAM0508", K_RETRY | K_SKIP);
        CHECK(!Gate("402", K_SKIP, "BtnPause"));
        CHECK(Gate("402", K_RETRY, "BtnPause"));
        IniConfig.bF15OutShuttleLoseICNeedPWD = false;

        IniConfig.bVTESTFunction = 1;                                            // VTEST WAR16123 + RETRY: DoPassword returns true
        Arm("403", "WAR16123", K_RETRY | K_SKIP);
        CHECK(Has(State("403"), "\"bNeedPassWord\":true"));
        CHECK(Gate("403", K_RETRY, "BtnStart"));
        CHECK(!Gate("403", K_SKIP, "BtnStart"));
        IniConfig.bVTESTFunction = 0;

        LevelSet.AccessLevel[35] = 0;                                            // GetJamLevel's own SCC arm raises to [35]: keep it out
        CUSTOMER_CODE = CC_SCC;                                                  // SCC list: Level 1 even at 0 in the file
        WriteJam("[03 Index Unit]\r\nJAM0303=0\r\n");
        Arm("404", "JAM0303", K_RETRY);
        CHECK(Has(State("404"), "\"Level\":1") && Has(State("404"), "\"bNeedPassWord\":true"));
        Reply rs = Verify("404", "404", true, "RETRY", "BtnStart", "d26eng", g_eng);
        CHECK(rs.accepted && Has(rs.raw, "\"required\":1"));
        CUSTOMER_CODE = CC_HONPREC_QC;

        CUSTOMER_CODE = CC_KYEC_LEE; IniConfig.bN07_EnableEmployeeIdCheak = true; // every alarm asks, level 0: any login passes
        WriteJam("[03 Index Unit]\r\nJAM0302=0\r\n");
        Arm("405", "JAM0302", K_RETRY);
        CHECK(Has(State("405"), "\"bNeedPassWord\":true") && Has(State("405"), "\"Level\":0"));
        Reply rk = Verify("405", "405", true, "RETRY", "BtnStart", "d26eng", g_eng);
        CHECK(rk.accepted && rk.loggedOut);
        CUSTOMER_CODE = CC_HONPREC_QC; IniConfig.bN07_EnableEmployeeIdCheak = false;

        //AI(W906-E030) 20261003 (St01): WAR04217 照 golden 906（FormShow 906 :1922-1927、DoPassword 906 :5276-5278 沒有 V912 FormShow
        //  :1937-1942「一律要密碼」與 DoPassword :5318-5323「等級改 1」）：兩個旗標開著也只看 JAM 等級表。
        TestIF_File.bEnableBarCode = true; TestIF_File.bEnableBarcodeCSVCompare = true;
        Arm("406", "WAR04217", K_RETRY);                                        // "04 Input Shuttle" has no WAR04217 line: level 0
        CHECK(Has(State("406"), "\"sJamArea\":\"04 Input Shuttle\"") && Has(State("406"), "\"Level\":0"));
        CHECK(Has(State("406"), "\"bNeedPassWord\":false"));                    // 906: no password (V912 :1941 set it true)
        CHECK(AuthJson("406").empty());                                         // no auth object -> the mailbox keeps required:false
        CHECK(Gate("406", K_RETRY, "BtnStart"));                                // the answer closes the note
        WriteJam("[04 Input Shuttle]\r\nWAR04217=2\r\n");
        Arm("412", "WAR04217", K_RETRY);                                        // level 2 in the file
        CHECK(Has(State("412"), "\"Level\":2") && Has(State("412"), "\"bNeedPassWord\":true"));
        CHECK(Has(AuthJson("412"), "\"level\":2"));                             // V912's iLevel=1 would say 1
        Reply rb = Verify("412", "412", true, "RETRY", "BtnStart", "d26eng", g_eng);   // Engineer (1) < 2: refused under 906
        CHECK(rb.ok && !rb.accepted && rb.asked && Has(rb.raw, "\"required\":2") && Has(rb.raw, "\"accessLevel\":1"));
        CHECK(!Gate("412", K_RETRY, "BtnStart"));
        rb = Verify("412", "412", true, "RETRY", "BtnStart", "d26sup", g_sup);  // Supervisor (2): accepted
        CHECK(rb.accepted && Has(rb.raw, "\"required\":2"));
        CHECK(Gate("412", K_RETRY, "BtnStart"));
        TestIF_File.bEnableBarCode = false; TestIF_File.bEnableBarcodeCSVCompare = false;
        Arm("413", "WAR04217", K_RETRY);                                        // the two flags change nothing (906)
        CHECK(Has(State("413"), "\"Level\":2") && Has(State("413"), "\"bNeedPassWord\":true"));
        CHECK(Has(AuthJson("413"), "\"level\":2"));
        WriteJam("[03 Index Unit]\r\nJAM0302=0\r\n");                          // back to the file the cases below start from

        CosFunction.bConAlarmNeedKeyInPassword = true; IniConfig.bO16ConAlarmNeedKeyInPasswordCT = true;   // O16: 2nd same JAM asks
        IniConfig.iO16ConAlarmNeedKeyInPasswordCT = 2; LevelSet.AccessLevel[35] = 2;
        Arm("407", "JAM0305", K_RETRY);                                         // not in GetJamLevel's item-35 list
        CHECK(Has(State("407"), "\"bNeedPassWord\":false"));                    // first one: counted
        Arm("408", "JAM0305", K_RETRY);
        CHECK(Has(State("408"), "\"bNeedPassWord\":true"));                     // second: golden iSameAlarmCT>=2
        Reply ro = Verify("408", "408", true, "RETRY", "BtnStart", "d26eng", g_eng);
        CHECK(!ro.accepted && Has(ro.raw, "\"required\":2"));                   // iLevel<=0 -> LevelSet.AccessLevel[35]
        Arm("409", "JAM0305", K_RETRY);
        CHECK(Has(State("409"), "\"bNeedPassWord\":false"));                    // the count restarted
        CosFunction.bConAlarmNeedKeyInPassword = false; IniConfig.bO16ConAlarmNeedKeyInPasswordCT = false; LevelSet.AccessLevel[35] = 0;

        CosFunction.bStatisticsJamCount = true;                                  // statistics: the 5th same code raises the level by one
        WriteJam("[03 Index Unit]\r\nJAM0301=1\r\n");
        const int statRows0 = fSecurity->sgStatisticsJam->RowCount;             // fSecurity's jam counter: put back after this case
        fSecurity->sgStatisticsJam->RowCount = 2;
        const AnsiString statCode0 = fSecurity->sgStatisticsJam->Cells[2][1], statCount0 = fSecurity->sgStatisticsJam->Cells[7][1];
        fSecurity->sgStatisticsJam->Cells[2][1] = "JAM0301";
        fSecurity->sgStatisticsJam->Cells[7][1] = "4";
        Arm("410", "JAM0301", K_RETRY);
        CHECK(Has(State("410"), "\"bNeedHighLevelPassword\":true"));
        Reply rl = Verify("410", "410", true, "RETRY", "BtnStart", "d26eng", g_eng);
        CHECK(!rl.accepted && Has(rl.raw, "\"required\":2"));
        CosFunction.bStatisticsJamCount = false;
        fSecurity->sgStatisticsJam->Cells[2][1] = statCode0; fSecurity->sgStatisticsJam->Cells[7][1] = statCount0;
        fSecurity->sgStatisticsJam->RowCount = statRows0;

        bWaitSecsGemReply = true;                                               // golden DoPassword returns true at once
        Arm("411", "JAM0301", K_RETRY);
        CHECK(Gate("411", K_RETRY, "BtnStart"));
        bWaitSecsGemReply = false;
        WriteJam("[03 Index Unit]\r\nJAM0301=2\r\nJAM0302=0\r\n");
    }

    // ---- 5. DoUnlockPassword ------------------------------------------------------------------------------------------
    std::printf("  -- 5. DoUnlockPassword\n");
    {
        CosFunction.bUseAlarmUnlockPassWord = true;
        WriteText(g_unlock, unlockPw + "\r\n");
        W906_NoteAuthResetForTest();
        WriteJam("[03 Index Unit]\r\nJAM0302=0\r\nJAM0302 UnlockPassWord=1\r\nJAM0301=2\r\nJAM0301 UnlockPassWord=1\r\n");
        Arm("501", "JAM0302", K_RETRY);                                         // unlock only (level 0)
        CHECK(Has(State("501"), "\"bAlarmUnlockPassWord\":true") && Has(State("501"), "\"bNeedPassWord\":false"));
        std::string j = AuthJson("501");
        CHECK(Has(j, "\"kind\":\"unlock-password\"") && Has(j, "\"userIdRequired\":false") && Has(j, "\"unlock\":true") && Has(j, "\"login\":false"));
        CHECK(!Gate("501", K_RETRY, "BtnStart"));
        Reply r = Verify("501", "501", true, "RETRY", "BtnStart", 0, wrongPw);
        CHECK(r.ok && !r.accepted && Has(r.message, "unlock"));
        CHECK(Has(State("501"), "\"bAlarmUnlockPassWord\":true"));
        r = Verify("501", "501", true, "RETRY", "BtnStart", 0, "", true);       // cancel: blank never equals a real password
        CHECK(!r.accepted);
        r = Verify("501", "501", true, "RETRY", "BtnStart", 0, unlockPw);
        CHECK(r.accepted && Has(State("501"), "\"bAlarmUnlockPassWord\":false"));
        CHECK(Gate("501", K_RETRY, "BtnStart"));
        CHECK(Gate("501", K_RETRY, "BtnStart"));                                // cleared for this note: golden asks no more

        Arm("502", "JAM0301", K_RETRY);                                         // unlock + login (level 2), two steps
        j = AuthJson("502");
        CHECK(Has(j, "\"kind\":\"unlock-password\"") && Has(j, "\"login\":true") && Has(j, "\"userIdRequired\":true") && Has(j, "\"level\":2"));
        r = Verify("502", "502", true, "RETRY", "BtnStart", "d26sup", unlockPw);
        CHECK(r.ok && !r.accepted && r.stage == "login" && Has(r.message, "now log in"));
        CHECK(!Gate("502", K_RETRY, "BtnStart"));
        r = Verify("502", "502", true, "RETRY", "BtnStart", "d26sup", g_sup);
        CHECK(r.accepted && r.stage == "done");
        CHECK(Gate("502", K_RETRY, "BtnStart"));

        ::DeleteFileA(g_unlock.c_str());                                         // missing file: nothing matches (not even blank)
        W906_NoteAuthResetForTest();
        Arm("503", "JAM0302", K_RETRY);
        r = Verify("503", "503", true, "RETRY", "BtnStart", 0, "", true);
        CHECK(!r.accepted);
        r = Verify("503", "503", true, "RETRY", "BtnStart", 0, unlockPw);
        CHECK(!r.accepted);
        CHECK(!Gate("503", K_RETRY, "BtnStart"));
        CosFunction.bUseAlarmUnlockPassWord = false;
        Arm("504", "JAM0302", K_RETRY);                                         // off: never asks
        CHECK(Gate("504", K_RETRY, "BtnStart"));
        WriteJam("[03 Index Unit]\r\nJAM0301=2\r\nJAM0302=0\r\n");
    }

    // ---- 6. kCode==0 notice -------------------------------------------------------------------------------------------
    std::printf("  -- 6. kCode==0 notice\n");
    {
        W906_NoteAuthResetForTest();                                            // every case starts clean: no notes, no pass, O16 / O17 counters 0
        Logout();
        Arm("601", "JAM0301", 0);
        std::string why;
        CHECK(!W906_NoteAuthNoticeGate("601", &why) && Has(why, "auth-required")); Out(why);
        CHECK(Has(AuthJson("601"), "\"actions\":[\"ACKNOWLEDGE\"]"));
        Reply rb = Verify("601", "601", false, "RETRY", "BtnStart", "d26sup", g_sup);   // a notice is only acknowledged
        CHECK(!rb.ok);
        Reply r = Verify("601", "601", false, "ACKNOWLEDGE", "BtnPause", "d26sup", g_sup);   // what dialog-page.js sends: a notice's only button
        CHECK(r.ok && r.accepted && r.asked && r.loggedOut);
        CHECK(Has(State("601"), "\"pass\":true"));
        CHECK(W906_NoteAuthNoticeGate("601", &why));                            // the ack (no button) uses it
        CHECK(!W906_NoteAuthNoticeGate("601", &why));                           // once
        r = Verify("601", "601", false, "ACKNOWLEDGE", 0, "d26sup", g_sup);      // pressedButton null: the same
        CHECK(r.accepted && W906_NoteAuthNoticeGate("601", &why));
        Reply rx = Verify("601", "601", false, "ACKNOWLEDGE", "BtnX", "d26sup", g_sup);
        CHECK(!rx.ok && Has(rx.raw, "bad-payload") && Has(State("601"), "\"pass\":false"));
        SystemStart = true;                                                     // pause 2: not a PAUSE press, no DoPassword
        CHECK(W906_NoteAuthNoticeGate("601", &why));
        r = Verify("601", "601", false, "ACKNOWLEDGE", 0, "d26sup", wrongPw);
        CHECK(r.accepted && !r.asked);
        SystemStart = false;
        CHECK(W906_NoteAuthNoticeGate("999", &why));                            // not an armed notice: left to NotifyAckHandle
        W906_ShowErrorMessage_Recorded = true;
        fNote->sJamArea = "03 Index Unit";  fNote->sJamCode = "JAM0302";        // a motor note: ShowMotorErrorMessage set them (level 0)
        fNote->edErrorCode->Text = "JAM0302";
        W906_NoteAuthArm("602", "JAM0302", 0, true);
        CHECK(Has(State("602"), "\"sJamCode\":\"JAM0302\"") && Has(State("602"), "\"bNeedPassWord\":false"));
        CHECK(W906_NoteAuthNoticeGate("602", &why));
    }

    // ---- 7. panel keys --------------------------------------------------------------------------------------------------
    std::printf("  -- 7. panel keys\n");
    {
        W906_NoteAuthResetForTest();                                            // every case starts clean: no notes, no pass, O16 / O17 counters 0
        Arm("701", "JAM0301", K_RETRY);
        CHECK(!W906_NoteAuthIoGate("701", K_RETRY, "BtnStart"));
        CHECK(!W906_NoteAuthIoGate("701", K_RETRY, "BtnPause"));
        Reply r = Verify("701", "701", true, "RETRY", "BtnStart", "d26sup", g_sup);
        CHECK(r.accepted);
        CHECK(!W906_NoteAuthIoGate("701", K_RETRY, "BtnStart"));                // a web pass is not a panel press
        CHECK(Has(State("701"), "\"pass\":true"));                              // the refused panel press left the screen's pass
        CHECK(Gate("701", K_RETRY, "BtnStart"));                                // so the page's answer right after it still closes the note
        Arm("702", "JAM0302", K_RETRY);
        CHECK(W906_NoteAuthIoGate("702", K_RETRY, "BtnStart"));
    }

    // ---- 8. mailbox JSON ------------------------------------------------------------------------------------------------
    std::printf("  -- 8. mailbox JSON\n");
    {
        W906_NoteAuthResetForTest();                                            // every case starts clean: no notes, no pass, O16 / O17 counters 0
        const std::string def = w906dlg::AlarmRequestJson(5, "801", "JAM0301", K_RETRY, 3, "", "JAM0301", true);
        CHECK(Has(def, "\"flushPanel\":null},\"auth\":{\"required\":false,\"kind\":\"none\",\"level\":null,\"title\":\"Password\",\"prompt\":\"\",\"userIdRequired\":true,\"defaultUserId\":null},\"buttons\":[]"));
        CHECK(def == w906dlg::AlarmRequestJson(5, "801", "JAM0301", K_RETRY, 3, "", "JAM0301", true, std::string()));
        Arm("801", "JAM0301", K_RETRY);
        const std::string a = AuthJson("801");
        const std::string with = w906dlg::AlarmRequestJson(5, "801", "JAM0301", K_RETRY, 3, "", "JAM0301", true, a);
        Out(with);
        CHECK(Has(with, "\"flushPanel\":null},\"auth\":" + a + ",\"buttons\":[]"));
        cJSON* o = cJSON_Parse(with.c_str());
        const cJSON* au = o ? cJSON_GetObjectItemCaseSensitive(o, "auth") : 0;
        CHECK(au && cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(au, "required")));
        if (o) cJSON_Delete(o);
    }

    // ---- 8b. AI(W906-D034) 20261002: the SpecialPanel password (golden 906 FormShow :1564-1606, PanSpecialNoteClick :5442-5460; V912 :1574-1616 / :5489-5507) -----
    std::printf("  -- 8b. D-034 SpecialPanel\n");
    {
        W906_NoteAuthResetForTest();
        RemoveBook();
        Logout();
        AccessLevel = 0;
        const std::string spPw = Rand("P", 10);
        g_secrets.push_back(spPw);
        const std::string ini = g_dir + "\\SpecialErrNote.ini";
        asErrNotePath = AnsiString(ini.c_str());
        const auto WriteSp = [&](int testSuck) {
            CloseIniFile();                                                      // the INI cache first (as WriteJam)
            WriteText(ini, "[SUCK]\r\nTestSuck=" + std::to_string(testSuck) + "\r\n[MESSAGE]\r\nEn=made-up special note en\r\n"
                           "Ch=made-up special note ch\r\n[PASSWORD]\r\nPwd=" + spPw + "\r\n");
        };
        WriteSp(1);
        Arm("851", "JAM0303", K_RETRY | K_SKIP);                                 // both flags off: golden 906 :1564 (V912 :1574) outer if is false
        CHECK(Has(State("851"), "\"specialLocked\":false") && !W906_SpecialPanelLocked());
        CHECK(AuthJson("851").empty());

        IniConfig.bIndexDropNeedPwdByIni = true;                                 // [I] Index drop needs the SpecialErrNote.ini password
        Arm("852", "JAM0303", K_RETRY | K_SKIP);
        std::string s = State("852");
        CHECK(Has(s, "\"bErrPan_err\":true") && Has(s, "\"specialPwdSet\":true") && Has(s, "\"specialLocked\":true") && W906_SpecialPanelLocked());
        std::string j = AuthJson("852");
        std::printf("    auth (special): %s\n", j.c_str());
        CHECK(Has(j, "\"required\":true") && Has(j, "\"kind\":\"special-note\"") && Has(j, "\"special\":true") && Has(j, "\"actions\":[\"SKIP\",\"RETRY\"]"));
        CHECK(Has(j, "\"userIdRequired\":false") && Has(j, "made-up special note en") && Has(j, "\"login\":false"));
        std::string why;
        CHECK(!Gate("852", K_RETRY, "BtnStart", &why) && Has(why, "SpecialPanel"));   // golden BtnSkipClick 906 :2845 / BtnStartClick :3818 (V912 :2867 / :3858)
        CHECK(!Gate("852", K_SKIP, "BtnPause"));                                 // BtnPauseClick :3870
        CHECK(!W906_NoteAuthIoGate("852", K_RETRY, "BtnStart"));                 // ScanKey :2908-2913
        Reply r = Verify("852", "852", true, "RETRY", "BtnStart", 0, wrongPw);
        CHECK(r.ok && !r.accepted && r.stage == "special" && Has(r.raw, "\"specialAsked\":true") && Has(r.message, "special note"));
        CHECK(W906_SpecialPanelLocked());
        r = Verify("852", "852", true, "RETRY", "BtnStart", 0, "", true);       // cancelled = blank = no match
        CHECK(r.ok && !r.accepted && r.cancelled && r.stage == "special" && W906_SpecialPanelLocked());
        r = Verify("852", "852", true, "RETRY", "BtnStart", 0, spPw);
        CHECK(r.ok && r.accepted && r.stage == "done" && !W906_SpecialPanelLocked());
        CHECK(Has(State("852"), "\"specialLocked\":false"));
        CHECK(Gate("852", K_RETRY, "BtnStart"));                                 // level 0: no other password
        CHECK(W906_NoteAuthIoGate("852", K_RETRY, "BtnStart"));

        Arm("853", "JAM0304", K_RETRY);                                          // golden globals: locked again by the next drop
        CHECK(W906_SpecialPanelLocked());
        Arm("854", "JAM0301", K_RETRY);                                          // not a special code (level 2): still locked -- :1598 is never undone by FormShow
        j = AuthJson("854");
        CHECK(Has(j, "\"kind\":\"special-note\"") && Has(j, "\"login\":true") && Has(j, "\"level\":2"));
        CHECK(!Gate("854", K_RETRY, "BtnStart", &why) && Has(why, "SpecialPanel"));
        r = Verify("854", "854", true, "RETRY", "BtnStart", 0, spPw);            // the special password, then golden asks the login
        CHECK(r.ok && !r.accepted && r.stage == "login" && Has(r.message, "now log in") && !W906_SpecialPanelLocked());
        CHECK(!Gate("854", K_RETRY, "BtnStart", &why) && Has(why, "DoPassword"));
        r = Verify("854", "854", true, "RETRY", "BtnStart", 0, selSup);          // the note's stOperatorClick arm (drop-down mode)
        CHECK(r.accepted && AccessLevel == 2 && Gate("854", K_RETRY, "BtnStart"));
        AccessLevel = 0;

        Arm("855", "JAM0305", K_RETRY);
        CHECK(W906_SpecialPanelLocked());
        WriteSp(0);                                                              // TestSuck != 1: golden :1612-1616 clears the lock
        Arm("856", "JAM0306", K_RETRY);
        CHECK(!W906_SpecialPanelLocked() && Has(State("856"), "\"specialPwdSet\":false"));
        WriteSp(1);
        Arm("857", "JAM0314", K_RETRY);
        CHECK(W906_SpecialPanelLocked());
        CloseIniFile(); ::DeleteFileA(ini.c_str());                               // no file: cleared as well
        Arm("858", "JAM0315", K_RETRY);
        CHECK(!W906_SpecialPanelLocked());

        WriteSp(1);                                                              // Tester time up (WAR07352)
        IniConfig.bIndexDropNeedPwdByIni = false;
        IniConfig.bTesterTimeUpErrorNeedPassword = true;
        Arm("859", "JAM0303", K_RETRY);                                          // not the time-up code
        CHECK(!W906_SpecialPanelLocked());
        Arm("860", "WAR07352", K_RETRY);
        CHECK(W906_SpecialPanelLocked());
        CUSTOMER_CODE = CC_LINGSEN; iTestTimeUpErrContinueR = 0;
        r = Verify("860", "860", true, "RETRY", "BtnPause", 0, spPw);
        CHECK(r.accepted && iTestTimeUpErrContinueR == 2 && !W906_SpecialPanelLocked());   // golden 906 :5454-5458 (V912 :5501-5505)
        CUSTOMER_CODE = CC_HONPREC_QC; iTestTimeUpErrContinueR = 0;

        Arm("861", "WAR07352", 0);                                               // a kCode==0 notice
        CHECK(W906_SpecialPanelLocked());
        CHECK(Has(AuthJson("861"), "\"actions\":[\"ACKNOWLEDGE\"]"));
        CHECK(!W906_NoteAuthNoticeGate("861", &why) && Has(why, "SpecialPanel")); Out(why);
        SystemStart = true;                                                      // pause 2: not a PAUSE press
        CHECK(W906_NoteAuthNoticeGate("861", &why));
        SystemStart = false;
        r = Verify("861", "861", false, "ACKNOWLEDGE", "BtnPause", 0, spPw);
        CHECK(r.ok && r.accepted && !W906_SpecialPanelLocked() && W906_NoteAuthNoticeGate("861", &why));
        CHECK(!Has(j, spPw) && !Has(State("861"), spPw));
        CloseIniFile(); ::DeleteFileA(ini.c_str());
        IniConfig.bTesterTimeUpErrorNeedPassword = false;
        W906_NoteAuthResetForTest();
    }

    // ---- 9. source ratchets -------------------------------------------------------------------------------------------
    std::printf("  -- 9. call sites\n");
    {
        CHECK(!Has(CodeOf("x = 1;   // W906_NoteAuthArm(qidStr, code, kcode, false);\n"), "W906_NoteAuthArm"));
        CHECK(!Has(CodeOf("/* W906_NoteAuthAnswerGate(qidStr) */ y;\n"), "W906_NoteAuthAnswerGate"));
        const char* ov = std::getenv("W906_D026_SRC_ROOT");                     // control run: a pre-change copy
        std::string root = (ov && *ov) ? std::string(ov) : std::string(argc > 1 ? argv[1] : "");
        for (size_t i = 0; i < root.size(); ++i) if (root[i] == '/') root[i] = '\\';
        if (!root.empty() && root[root.size() - 1] != '\\') root += '\\';
        std::printf("    source root: %s%s\n", root.c_str(), (ov && *ov) ? " (W906_D026_SRC_ROOT)" : "");
        CHECK(!root.empty());
        const std::string wb  = NoCR(CodeOf(NoCR(ReadFile(root + "tools\\wb_serve.cpp"))));
        const std::string srv = NoCR(CodeOf(NoCR(ReadFile(root + "WebBridge\\WebBridgeServer.cpp"))));
        const std::string mb  = NoCR(CodeOf(NoCR(ReadFile(root + "tools\\wb_dialog_mailbox.h"))));
        CHECK(!wb.empty() && !srv.empty() && !mb.empty());
        const size_t fwd = wb.find("static int ForwardShowErrorMessage(const char* code, int kcode, int pos)");
        const size_t rec = (fwd == std::string::npos) ? fwd : wb.find("W906_ShowErrorMessageRecordLikeGolden(AnsiString(code ? code : \"\")", fwd);
        const size_t arm = (fwd == std::string::npos) ? fwd : wb.find("W906_NoteAuthArm(qidStr, code, kcode, g_w906MotorNoteMsg != 0);", fwd);
        const size_t kz  = (fwd == std::string::npos) ? fwd : wb.find("if (kcode == 0) {", fwd);
        CHECK(fwd != std::string::npos && rec != std::string::npos && arm != std::string::npos && kz != std::string::npos && rec < arm && arm < kz);
        CHECK(Has(wb, "blocking, W906_NoteAuthRequestJson(requestId.c_str())"));   //AI(W906-W142) 20261007 (St02-E): prefix -- W-142 appends ", w906Desc, w906Key" (display.description) after the auth object
        CHECK(Has(wb, "if (k != 0 && (k & kcode) != 0) {  { extern bool W906_NoteAuthAnswerGate(const char*, int, const std::string&, std::string*); std::string noteAuthWhy; if (!W906_NoteAuthAnswerGate(qidStr, k, pressed, &noteAuthWhy)) { g_modalServer->CompleteCommand((unsigned long long)wc.id, false, noteAuthWhy); continue; } }"));
        CHECK(Has(wb, "else if (wc.cmd == \"dialog.auth\") {  extern bool W906_NoteAuthVerify(") && Has(wb, "wc.value.asString() : std::string(), std::string(qidStr), true, &naReply); g_modalServer->CompleteCommand((unsigned long long)wc.id, naOk, naReply); } else if (wc.cmd == \"dialog.notifyAck\") { g_modalServer->CompleteCommand("));
        CHECK(Has(wb, "if (kio > 0 && (kio & kcode) != 0 && W906_NoteAuthIoGate(qidStr, kio, ioPressed)) {"));
        CHECK(Has(wb, "extern bool W906_SpecialPanelLocked(); const bool spLocked = W906_SpecialPanelLocked() && Key != SnFKAlarmReset && Key != SnRKAlarmReset;"));   // AI(W906-D034) C1
        CHECK(Has(wb, "bool blocked = spLocked;"));                              // AI(W906-D034) C2: golden ScanKey :2908-2913
        CHECK(Has(wb, "} else if (wc.cmd == \"dialog.notifyAck\" && [&]() -> bool { if (g_w906NoticeGatePassed.erase((unsigned long long)wc.id) != 0) return false;") && Has(wb, "extern bool W906_NoteAuthNoticeGate(const std::string&, std::string*); std::string nw; if (W906_NoteAuthNoticeGate(wc.hasTag ? wc.tag : std::string(), &nw)) return false; server.CompleteCommand((unsigned long long)wc.id, false, nw); return true; }()) {"));   // AI(W906-NOTICE-DEFER-5) 20261007: the main-loop gate is skipped ONLY for the command id MbWait already gated (wb_serve.cpp NOTICE-DEFER-4/5); the gate itself unchanged
        CHECK(Has(wb, "g_w906NoticeGatePassed.insert((unsigned long long)wc.id);") && !Has(wb, "g_w906NoticeGatePassed = tg;"));   // AI(W906-NOTICE-DEFER-5) 20261007: keyed by command id, never by tag
        CHECK(Has(wb, "} else if (wc.cmd == \"dialog.notifyAck\") { extern void W906_NoticeAckCommand(webbridge::WebBridgeServer&, const webbridge::WebCommand&); W906_NoticeAckCommand(server, wc);"));   // INBOX 119's pin still holds
        CHECK(Has(wb, "const std::string naCurrent = (g_alarmSlot.kind == w906dlg::AlarmSlot::kNotice) ? g_alarmSlot.requestId : std::string();"));
        CHECK(Has(wb, "naCurrent, false, &naReply);  server.CompleteCommand((unsigned long long)wc.id, naOk, naReply);"));
        CHECK(!Has(wb, "pressed, wc.connId") && !Has(wb, "wc.connId, std::string(qidStr)") && !Has(wb, "wc.connId, &nw") &&
              !Has(wb, "wc.connId, naCurrent"));                               // Steven Q64 (3): the password pass is not tied to the connection
        CHECK(!Has(wb, "dialog auth not wired"));
        CHECK(Has(srv, "cmdName != \"dialog.response\" && cmdName != \"dialog.notifyAck\" && cmdName != \"dialog.auth\"  && cmdName != \"motor.stop\""));
        CHECK(Has(mb, "bool blocking, const std::string& authJson = std::string()") && Has(mb, "AlarmRequestJson(seq, requestId, code, kcode, pos, unitName, message, blocking, authJson"));   //AI(W906-W142) 20261007 (St02-E): prefixes -- W-142 adds description / descriptionKey after authJson
    }

    // restore
    RemoveBook();
    ::DeleteFileA(g_unlock.c_str());
    CloseIniFile();
    ::DeleteFileA(g_jam.c_str());
    { bool ok = false; W906_LevelSetPut("d026-jamfile", "", false, &ok); }
    CloseGeneralIniFile();
    W906_NoteAuthResetForTest();
    std::memcpy(&USER, &savedUser, sizeof(PASS_WORD));
    pwPath = savedPwPath; sSuperVisorString = savedSuper;
    CosFunction.bUseLoginDatToSetLevel = savedBookFlag; CosFunction.bSecurityHave5Level = saved5; IniConfig.bPasswordSecret = savedSecret;
    IniConfig.bSPILFunction = savedSpil; CosFunction.bLoginShowUserName = savedShowName; CosFunction.bUseAlarmUnlockPassWord = savedUnlockOn;
    IniConfig.bF15OutShuttleLoseICNeedPWD = savedF15; CosFunction.bStatisticsJamCount = savedStat; CosFunction.bConAlarmNeedKeyInPassword = savedO16c;
    IniConfig.bO16ConAlarmNeedKeyInPasswordCT = savedO16; CosFunction.bConAlarmInTimeLevelUp = savedO17c; IniConfig.bO17EnableLevelUpWhenContiAlarm = savedO17;
    IniConfig.bN07_EnableEmployeeIdCheak = savedN07; TestIF_File.bEnableBarCode = savedBar; TestIF_File.bEnableBarcodeCSVCompare = savedBarCsv;
    CosFunction.bEnableHandlerResultServer = savedHrs; IniConfig.bVTESTFunction = savedVtest; IniConfig.iO16ConAlarmNeedKeyInPasswordCT = savedO16n;
    CUSTOMER_CODE = savedCustomer; AccessLevel = savedLevel; JCET_FOR_EVAN = savedJcet; LevelSet.AccessLevel[35] = saved35;
    W906_ShowErrorMessage_Recorded = savedRecorded; bWaitSecsGemReply = savedWait; SystemStart = savedSys; SoftStart = savedSoft;
    asErrNotePath = savedErrNote; IniConfig.bIndexDropNeedPwdByIni = savedIdxPwd; IniConfig.bTesterTimeUpErrorNeedPassword = savedTimeUpPwd;   // AI(W906-D034)
    iTestTimeUpErrContinueR = savedContiR;

    std::fflush(stdout);
    _dup2(savedOut, 1);
    _close(savedOut);
    const std::string captured = ReadFile(capture);
    std::fwrite(captured.data(), 1, captured.size(), stdout);

    // ---- 10. no password anywhere ---------------------------------------------------------------------------------------
    std::printf("  -- 10. no password in any output\n");
    {
        Out(std::string(ExString.c_str()));
        std::vector<std::string> files;
        Walk(as9045LogPath.c_str(), t0, &files, 0);
        Walk(asSaveEventLogPath.c_str(), t0, &files, 0);
        Walk(g_dir, t0, &files, 0);
        int hits = 0, scanned = 0;
        bool unlockRow = false;
        for (size_t s = 0; s < g_secrets.size(); ++s) {
            for (size_t i = 0; i < g_outputs.size(); ++i) if (Has(g_outputs[i], g_secrets[s])) ++hits;
            if (Has(captured, g_secrets[s])) ++hits;
        }
        for (size_t f = 0; f < files.size(); ++f) {
            const std::string body = ReadFile(files[f]);
            ++scanned;
            if (Has(body, "==Login for unlock alarm.==")) unlockRow = true;
            for (size_t s = 0; s < g_secrets.size(); ++s)
                if (Has(body, g_secrets[s])) { ++hits; std::printf("    secret found in %s\n", files[f].c_str()); }
        }
        std::printf("    %d outputs, %d captured bytes, %d log/scratch files scanned, %d hits\n",
                    (int)g_outputs.size(), (int)captured.size(), scanned, hits);
        CHECK(hits == 0);
        CHECK(!g_outputs.empty() && !captured.empty());
        CHECK(unlockRow);                                                       // golden :5399 RecordProcess reached the log
    }

    // ---- 11. the machine's own files ------------------------------------------------------------------------------------
    for (int i = 0; i < nReal; ++i) {
        const Stamp now = StampOf(kReal[i]);
        std::printf("  %s unchanged: %s\n", kReal[i], Same(real0[i], now) ? "yes" : "NO");
        CHECK(Same(real0[i], now));
    }
    ::DeleteFileA(capture.c_str());
    ::RemoveDirectoryA(g_dir.c_str());

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
