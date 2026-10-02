// =============================================================================
//  tests/test_weblogin_reauth.cpp -- Q45 "甲" re-login (#4 Configuration [M01], #6 / #7 SetUp RTC-off / OCR-off):
//  WebLogin.cpp's appended W906_Reauth block (WebReauth.h) = golden TfSetup::DoPassword (V912 cSetUp.cpp:4325-4362) and
//  TfConfiguration::DoPassword (cConfiguration.cpp:6482-6529), plus the editlist.save plumbing (take / ack / clear).
//  AI(W906-Q45-B5) 20260930 (St01).  Suite name (add_test): WebLogin_Reauth
//
//    0. containment first (exit 2 / 1, nothing called): ctest's redirect environment (w906_ctest_guard.h: cMyDB log roots,
//       per-test Gerneral.ini ...) and the three login seams W906_PWBOOK_PATH / W906_LOGINDAT_PATH / W906_LEVELSET_PATH in the
//       build dir's weblogin_reauth_scratch (tests/CMakeLists.txt DEFERred APPEND).  Then it stamps D:\HT9045\system\login.dat /
//       levelset.dat / lastdata.dat / Gerneral.ini.  Every account and password is made up at run time (random); USER, LevelSet,
//       REAL_TIME_CCD and sSuperVisorString are set in memory; the text password book is written into the scratch folder.
//       stdout is captured into the scratch folder for the whole run (and echoed at the end).
//    1. plumbing: W906_ReauthTake beside widgets / inside widgets (refused) / wrong point / page without a point / bad shapes,
//       the JSON strings are gone from root, W906_ReauthAck splices "reauth" into an ack, W906_ReauthClear.
//    2. SetUp, password book: pass (login changes, MES2144 "USER login"), wrong (Operator + WAR1677), cancelled (= blank = wrong),
//       level too low (login changes, fails), item 37 = 0 (still asked, always passes), no REAL_TIME_CCD (not asked), no answer
//       (handled=false, nothing changes), no logout after; the stash path W906_ReauthSetupDoPassword reports golden's reverts.
//    3. SetUp, drop-down (no book): the SetUp arm (bNeedPassword: Supervisor's then Engineer's slot, drop-down ignored), the
//       normal arm when bNeedPassword is false (OCR-only), sSuperVisorString -> HonPrec, cancel; the drop-down does not move.
//    4. Configuration, password book: pass / wrong / too low -> always logged out to Operator (btLogin Login, itemIndex 0);
//       the M01 revert callback runs for every changed cell only on failure; item 92 = 0 and no REAL_TIME_CCD: not asked, no logout;
//       no answer: handled=false, cells put back, no logout; already "Logout" on btLogin: still asked (not WebLogin_BookLogin).
//    5. Configuration, drop-down: only the current drop-down level's slot; wrong -> AccessLevel 0 but the drop-down stays and
//       there is no logout (golden logs out only with a book; Q45-5 = A).
//    6. editlist.get extra.auth (W906_ReauthOpenJson) for both pages; W906_ReauthControls.
//    7. the save-flow call sites exist in the four source files as code, not inside a comment (the FileRW side and wb_serve
//       cannot be linked into a ctest); the comment stripper is itself checked on a broken line.
//    8. no password in any output: every returned JSON, WebLogin_StateJson, ExString, the captured stdout, and the event-log
//       files under the redirected log roots written during the run.
//    9. the machine's own files: unchanged.
//  ht9045::formjson::FormLock / FormUnlock (JsonBridge/FormJson.cpp, wb_serve only) are empty stand-ins below (same as
//  test_weblogin_force_operator.cpp).  Run by hand (no ctest environment) it refuses at step 0 -- that is the point.
// =============================================================================
#include "WebLogin.h"
#include "WebReauth.h"
#include "cMyDB.h"         // ExString
#include "cmydef.h"        // AccessLevel, pwPath, REAL_TIME_CCD, sSuperVisorString, CUSTOMER_CODE, JCET_FOR_EVAN
#include "common.h"        // OpenGeneralIniFile, log roots, asGeneralPath
#include "CosFunction.h"
#include "Config.h"
#include "MachineType.h"   // CC_HONPREC_QC
#include "cprod.h"         // USER, LevelSet
#include "forms/fMain.h"
#include "Public/cJSON.h"
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

// ---- made-up credentials (random per run; never golden's, never the machine's) ------------------------------------
static unsigned g_seed = 0;
static std::string Rand(const char* prefix, int n)
{
    static const char kAl[] = "abcdefghjkmnpqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    std::string s(prefix);
    for (int i = 0; i < n; ++i) { g_seed = g_seed * 1103515245u + 12345u; s += kAl[(g_seed >> 16) % (sizeof(kAl) - 1)]; }
    return s;
}
static std::vector<std::string> g_secrets;   // every password the test typed (for step 8)
static std::vector<std::string> g_outputs;   // every string the code under test returned (for step 8)
static void Out(const std::string& s) { g_outputs.push_back(s); }

// ---- file stamps (step 9) ------------------------------------------------------------------------------------------
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

// ---- the scratch folder and the text password book -----------------------------------------------------------------
static std::string g_book;                   // = W906_PWBOOK_PATH (golden pwPath's test seam)
static std::string g_eng, g_sup, g_hon;      // book passwords (levels 1 / 2 / 3)
static void WriteBook()
{
    FILE* f = std::fopen(g_book.c_str(), "wb");
    if (!f) return;
    // golden text book: "<user> <level> <password>" per line (SplitStrByDotSpaceOnly, WebLogin.cpp :449-453)
    std::fprintf(f, "q45eng 1 %s\r\nq45sup 2 %s\r\nq45hon 3 %s\r\n", g_eng.c_str(), g_sup.c_str(), g_hon.c_str());
    std::fclose(f);
}
static void RemoveBook() { ::DeleteFileA(g_book.c_str()); }

// ---- small helpers -------------------------------------------------------------------------------------------------
static W906ReauthAnswer Ans(const char* point, const std::string& user, const std::string& pw)
{
    W906ReauthAnswer a;
    a.present = true;
    a.point = point;
    a.userId = user;
    a.password = pw;
    return a;
}
static W906ReauthAnswer Cancel(const char* point)
{
    W906ReauthAnswer a;
    a.present = true;
    a.cancelled = true;
    a.point = point;
    return a;
}
static std::string State() { const std::string s = WebLogin_StateJson(); Out(s); return s; }
static void Keep(const W906ReauthResult& r) { Out(r.reason); Out(r.login); Out(r.alarm); }
static void Logout() { std::string m; WebLogin_Logout(&m); }                    // Operator, btLogin "Login"
static bool BookAs(const char* user, const std::string& pw)                      // a starting login (golden cbUserSelectChange)
{
    std::string m;
    return WebLogin_BookCompare(AnsiString(user), AnsiString(pw.c_str()), AnsiString(g_book.c_str()), &m) == WEBLOGIN_OK;
}
static bool PickLevel(int idx)                                                   // drop-down mode: select idx (enough level first)
{
    AccessLevel = idx;
    std::string m;
    return WebLogin_Select(idx, false, AnsiString(""), &m) == WEBLOGIN_OK;
}
static std::string Take(const char* tag, const std::string& valueJson, std::string* printed)
{
    cJSON* root = cJSON_Parse(valueJson.c_str());
    const std::string r = W906_ReauthTake(root, tag);
    char* p = root ? cJSON_PrintUnformatted(root) : 0;
    *printed = p ? p : "";
    if (p) cJSON_free(p);
    if (root) cJSON_Delete(root);
    Out(r);
    Out(*printed);
    return r;
}

// ---- step 7: the code part of a C++ line (string literals kept, // and /* */ comments dropped) ---------------------
static std::string CodeOf(const std::string& text)
{
    std::string out;
    bool block = false;
    for (size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        if (block) { if (c == '*' && i + 1 < text.size() && text[i + 1] == '/') { block = false; ++i; } continue; }
        if (c == '"' || c == '\'') {                                             // copy a literal whole
            const char q = c;
            out += c;
            for (++i; i < text.size(); ++i) {
                out += text[i];
                if (text[i] == '\\' && i + 1 < text.size()) { out += text[++i]; continue; }
                if (text[i] == q || text[i] == '\n') break;
            }
            continue;
        }
        if (c == '/' && i + 1 < text.size() && text[i + 1] == '/') {            // to the end of the line
            while (i < text.size() && text[i] != '\n') ++i;
            out += '\n';
            continue;
        }
        if (c == '/' && i + 1 < text.size() && text[i + 1] == '*') { block = true; ++i; continue; }
        out += c;
    }
    return out;
}
static std::string ReadFile(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
static std::string TreeRoot()                                                    // from __FILE__ (tests/ -> the tree root)
{
    std::string f = __FILE__;
    for (size_t i = 0; i < f.size(); ++i) if (f[i] == '/') f[i] = '\\';
    const size_t k = Lower(f).rfind("\\tests\\");
    return k == std::string::npos ? std::string() : f.substr(0, k + 1);
}

// ---- step 8: files written under the log roots during the run ------------------------------------------------------
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

int main()
{
    // ---- 0. containment first ---------------------------------------------------------------------------------------
    std::printf("WebLogin_Reauth\n");
    if (!W906TestRequireCtestRedirects("WebLogin_Reauth"))
        return 2;
    const char* const seamNames[] = { "W906_PWBOOK_PATH", "W906_LOGINDAT_PATH", "W906_LEVELSET_PATH" };
    bool sandboxed = true;
    for (int i = 0; i < 3; ++i) {
        const char* v = std::getenv(seamNames[i]);
        std::printf("  %s = %s\n", seamNames[i], v ? v : "(unset)");
        if (v == 0 || *v == 0 || Lower(v).compare(0, 17, "d:\\ht9045\\system\\") == 0 || !Has(Lower(v), "weblogin_reauth_scratch"))
            sandboxed = false;
    }
    const AnsiString* const roots[] = { &as9045LogPath, &asSaveEventLogPath, &asProductionLogPath, &sProductionInfoFilePath };
    for (int i = 0; i < 4; ++i)
        if (!Has(roots[i]->c_str(), "machine_log_scratch")) sandboxed = false;
    if (!Has(Lower(asGeneralPath.c_str()), "general_ini_scratch")) sandboxed = false;
    CHECK(sandboxed);
    if (!sandboxed) {
        std::printf("FAIL: not sandboxed -- refusing to run the re-login code\n");
        return 1;
    }
    static const char* const kReal[] = { "D:\\HT9045\\system\\login.dat", "D:\\HT9045\\system\\levelset.dat",
                                         "D:\\HT9045\\system\\lastdata.dat", "D:\\HT9045\\system\\Gerneral.ini" };
    Stamp real0[4];
    for (int i = 0; i < 4; ++i) real0[i] = StampOf(kReal[i]);

    g_book = std::getenv("W906_PWBOOK_PATH");
    for (size_t i = 0; i < g_book.size(); ++i) if (g_book[i] == '/') g_book[i] = '\\';
    const std::string sandbox = g_book.substr(0, g_book.find_last_of('\\'));
    ::CreateDirectoryA(sandbox.c_str(), 0);
    const std::string capture = sandbox + "\\stdout_capture.txt";
    FILETIME t0;
    ::GetSystemTimeAsFileTime(&t0);
    {   // one second back: FAT-ish timestamp rounding must not drop a file written right at the start
        ULARGE_INTEGER u; u.LowPart = t0.dwLowDateTime; u.HighPart = t0.dwHighDateTime; u.QuadPart -= 10000000ull;
        t0.dwLowDateTime = u.LowPart; t0.dwHighDateTime = u.HighPart;
    }

    LARGE_INTEGER qpc; ::QueryPerformanceCounter(&qpc);
    g_seed = (unsigned)qpc.LowPart ^ (unsigned)::GetCurrentProcessId() ^ (unsigned)::GetTickCount();
    g_eng = Rand("e", 9); g_sup = Rand("s", 9); g_hon = Rand("h", 9);
    const std::string selEng = Rand("E", 9), selSup = Rand("S", 9), superPw = Rand("V", 11), wrongPw = Rand("w", 9);
    g_secrets.push_back(g_eng); g_secrets.push_back(g_sup); g_secrets.push_back(g_hon);
    g_secrets.push_back(selEng); g_secrets.push_back(selSup); g_secrets.push_back(superPw); g_secrets.push_back(wrongPw);

    // ---- capture stdout into the scratch folder from here on --------------------------------------------------------
    std::fflush(stdout);
    const int savedOut = _dup(1);                                          // fd 1 = stdout (MinGW.org hides _fileno under -std=c++17)
    if (!std::freopen(capture.c_str(), "w", stdout)) { std::fprintf(stderr, "cannot capture stdout\n"); return 1; }

    // what the run changes; restored at the end
    const AnsiString savedPwPath = pwPath, savedSuper = sSuperVisorString;
    const bool savedRtc = REAL_TIME_CCD, savedBookFlag = CosFunction.bUseLoginDatToSetLevel, saved5 = CosFunction.bSecurityHave5Level;
    const bool savedSecret = IniConfig.bPasswordSecret, savedSpil = IniConfig.bSPILFunction, savedShowName = CosFunction.bLoginShowUserName;
    const int savedCustomer = CUSTOMER_CODE, savedLevel = AccessLevel, savedJcet = JCET_FOR_EVAN;
    const int saved37 = LevelSet.AccessLevel[37], saved92 = LevelSet.AccessLevel[92];
    static PASS_WORD savedUser;
    std::memcpy(&savedUser, &USER, sizeof(PASS_WORD));

    pwPath = AnsiString((sandbox + "\\no_such_tech.com").c_str());               // golden pwPath: never the machine's
    CosFunction.bUseLoginDatToSetLevel = false;
    CosFunction.bSecurityHave5Level = false;
    CosFunction.bLoginShowUserName = false;
    IniConfig.bPasswordSecret = false;
    IniConfig.bSPILFunction = false;
    CUSTOMER_CODE = CC_HONPREC_QC;
    JCET_FOR_EVAN = 0;
    sSuperVisorString = AnsiString(superPw.c_str());
    std::memset(&USER, 0, sizeof(PASS_WORD));
    std::strcpy(USER.PassWord[0], selEng.c_str());                               // drop-down Engineer slot (stOperatorClick)
    std::strcpy(USER.PassWord[1], selSup.c_str());                               // drop-down Supervisor slot
    OpenGeneralIniFile();                                                        // stOperatorClick reads [VENDER] (sandbox copy)
    std::string msg;

    // ---- 1. plumbing ----------------------------------------------------------------------------------------------
    std::printf("  -- 1. editlist.save plumbing\n");
    {
        std::string printed;
        std::string v = "{\"widgets\":{\"cbEnableRealTimeCCD\":{\"checked\":false}},\"answers\":{},"
                        "\"reauth\":{\"point\":\"rtcOff\",\"userId\":\"q45hon\",\"password\":\"" + g_hon + "\"}}";
        CHECK(Take("TestIF_File_SetUp", v, &printed).empty());
        CHECK(W906_ReauthHasAnswer("TestIF_File_SetUp"));
        CHECK(!W906_ReauthHasAnswer("IniConfig"));
        CHECK(!Has(printed, "reauth") && !Has(printed, g_hon) && Has(printed, "cbEnableRealTimeCCD"));   // detached before anything prints it
        W906_ReauthClear();
        CHECK(!W906_ReauthHasAnswer("TestIF_File_SetUp"));

        v = "{\"widgets\":{\"reauth\":{\"point\":\"rtcOff\",\"password\":\"" + g_hon + "\"},\"cbOcrFunction\":{\"checked\":true}}}";
        const std::string r2 = Take("TestIF_File_SetUp", v, &printed);
        CHECK(Has(r2, "beside widgets"));
        CHECK(!W906_ReauthHasAnswer("TestIF_File_SetUp"));
        CHECK(!Has(printed, "reauth") && !Has(printed, g_hon));

        v = "{\"widgets\":{},\"reauth\":{\"point\":\"m01\",\"password\":\"" + g_hon + "\"}}";
        CHECK(Has(Take("TestIF_File_SetUp", v, &printed), "not a re-login point"));   // Configuration's point on the SetUp page
        CHECK(!W906_ReauthHasAnswer("TestIF_File_SetUp"));
        v = "{\"widgets\":{},\"reauth\":{\"point\":\"m01\",\"password\":\"" + g_hon + "\"}}";
        CHECK(Has(Take("HSys", v, &printed), "not a re-login point"));                // a page with no re-login point
        CHECK(!W906_ReauthHasAnswer("HSys"));
        CHECK(Has(Take("IniConfig", "{\"widgets\":{},\"reauth\":\"" + g_hon + "\"}", &printed), "must be an object"));
        CHECK(Has(Take("IniConfig", "{\"widgets\":{},\"reauth\":{\"point\":\"m01\",\"password\":7}}", &printed), "password must be a string"));
        CHECK(Has(Take("IniConfig", "{\"widgets\":{},\"reauth\":{\"point\":\"m01\",\"userId\":1,\"password\":\"x\"}}", &printed), "userId"));
        CHECK(Take("IniConfig", "{\"widgets\":{},\"reauth\":{\"point\":\"m01\",\"cancelled\":true}}", &printed).empty());
        CHECK(W906_ReauthHasAnswer("IniConfig"));
        CHECK(Take("IniConfig", "{\"widgets\":{},\"answers\":{}}", &printed).empty());   // no reauth: the old answer is gone too
        CHECK(!W906_ReauthHasAnswer("IniConfig"));

        std::string ack = "{\"struct\":\"TestIF_File_SetUp\",\"saved\":true}";
        W906_ReauthAck(&ack);                                                    // nothing brought, nothing asked -> unchanged
        CHECK(ack == "{\"struct\":\"TestIF_File_SetUp\",\"saved\":true}");
    }

    // ---- 2. SetUp, password book ------------------------------------------------------------------------------------
    std::printf("  -- 2. SetUp (golden cSetUp.cpp:4325-4362), password book\n");
    WriteBook();
    REAL_TIME_CCD = true;
    LevelSet.AccessLevel[37] = 3;
    {
        W906ReauthResult r;
        Logout();
        CHECK(BookAs("q45eng", g_eng) && AccessLevel == 1);
        ExString = "";
        bool ok = W906_Reauth("TestIF_File_SetUp", true, Ans("rtcOff", "Q45HON", g_hon), &r);   // user name in any case
        Keep(r);
        std::string st = State();
        std::printf("    pass: ok=%d level=%d %s\n", (int)ok, AccessLevel, r.reason.c_str());
        CHECK(ok && r.passed && r.asked && r.handled && r.answered && r.mode == "book");
        CHECK(AccessLevel == 3 && r.level == 3 && r.levelBefore == 1 && r.required == 3 && r.levelItem == 37);
        CHECK(!r.loggedOut && Has(st, "\"userCaption\":\"HonPrec\""));        // SetUp version: no logout
        CHECK(std::string(ExString.c_str()) == "======== USER login ========");   // golden MES2144 (inside the book compare)
        CHECK(Has(r.login, "\"level\":3"));

        ok = W906_Reauth("TestIF_File_SetUp", true, Ans("rtcOff", "q45hon", wrongPw), &r);
        Keep(r); st = State();
        std::printf("    wrong: ok=%d level=%d alarm=%s %s\n", (int)ok, AccessLevel, r.alarm.c_str(), r.reason.c_str());
        CHECK(!ok && !r.passed && AccessLevel == 0 && r.alarm == "WAR1677");
        CHECK(Has(st, "\"userCaption\":\"Operator\"") && Has(st, "\"btLogin\":\"Login\""));

        CHECK(BookAs("q45sup", g_sup) && AccessLevel == 2);
        ok = W906_Reauth("TestIF_File_SetUp", true, Cancel("rtcOff"), &r);
        Keep(r);
        std::printf("    cancelled: ok=%d level=%d %s\n", (int)ok, AccessLevel, r.reason.c_str());
        CHECK(!ok && r.cancelled && AccessLevel == 0 && r.alarm == "WAR1677");   // Q45-4 = A: blank = wrong -> Operator

        ok = W906_Reauth("TestIF_File_SetUp", true, Ans("ocrOff", "q45sup", g_sup), &r);
        Keep(r); st = State();
        std::printf("    too low: ok=%d level=%d %s\n", (int)ok, AccessLevel, r.reason.c_str());
        CHECK(!ok && AccessLevel == 2 && Has(r.reason, "level 2 < 3"));          // the login changed anyway (golden)
        CHECK(Has(st, "\"userCaption\":\"Supervisor\""));

        LevelSet.AccessLevel[37] = 0;                                            // SetUp has no "item 37 = 0" shortcut
        ok = W906_Reauth("TestIF_File_SetUp", true, Ans("rtcOff", "q45sup", wrongPw), &r);
        Keep(r);
        std::printf("    item 37 = 0: ok=%d asked=%d level=%d\n", (int)ok, (int)r.asked, AccessLevel);
        CHECK(ok && r.asked && AccessLevel == 0);                                // asked, wrong -> Operator, but 0 >= 0 passes
        LevelSet.AccessLevel[37] = 3;

        CHECK(BookAs("q45sup", g_sup));
        REAL_TIME_CCD = false;
        ok = W906_Reauth("TestIF_File_SetUp", true, Ans("rtcOff", "q45hon", g_hon), &r);
        Keep(r);
        CHECK(ok && !r.asked && r.handled && AccessLevel == 2);                  // not installed: golden does not ask
        REAL_TIME_CCD = true;

        W906ReauthAnswer none;
        ok = W906_Reauth("TestIF_File_SetUp", true, none, &r);
        Keep(r);
        CHECK(!ok && r.asked && !r.handled && AccessLevel == 2);                 // no answer: caller refuses, nothing changed

        // the stash path (what the generated SU_DoPassword calls): answer from W906_ReauthTake, reverts reported, ack spliced
        std::string printed;
        CHECK(Take("TestIF_File_SetUp", "{\"widgets\":{},\"reauth\":{\"point\":\"rtcOff\",\"userId\":\"q45eng\",\"password\":\"" +
                   g_eng + "\"}}", &printed).empty());
        bool handled = false;
        ok = W906_ReauthSetupDoPassword(true, true, false, true, &handled);        // RTC unchecked, OCR still checked
        CHECK(!ok && handled && AccessLevel == 1);                               // Engineer < 3
        CHECK(!W906_ReauthHasAnswer("TestIF_File_SetUp"));                       // used once
        std::string ack = "{\"struct\":\"TestIF_File_SetUp\",\"saved\":true}";
        W906_ReauthAck(&ack);
        Out(ack);
        std::printf("    ack: %s\n", ack.c_str());
        cJSON* aj = cJSON_Parse(ack.c_str());
        const cJSON* ra = aj ? cJSON_GetObjectItemCaseSensitive(aj, "reauth") : 0;
        const cJSON* rv = ra ? cJSON_GetObjectItemCaseSensitive(ra, "reverted") : 0;
        CHECK(ra && cJSON_IsFalse(cJSON_GetObjectItemCaseSensitive(ra, "passed")) && cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(ra, "asked")));
        CHECK(rv && cJSON_GetArraySize(rv) == 1 && std::string(cJSON_GetArrayItem(rv, 0)->valuestring) == "cbEnableRealTimeCCD");   // golden :3578-3579 only
        CHECK(ra && cJSON_IsObject(cJSON_GetObjectItemCaseSensitive(ra, "login")) && cJSON_GetObjectItemCaseSensitive(aj, "saved"));
        if (aj) cJSON_Delete(aj);
        W906_ReauthClear();

        handled = true;                                                          // no answer on the stash: handled=false (caller: ELPasswordRefused)
        ok = W906_ReauthSetupDoPassword(true, false, false, true, &handled);
        CHECK(!ok && !handled && AccessLevel == 1);
        ack = "{\"saved\":true}";
        W906_ReauthAck(&ack);
        Out(ack);
        CHECK(Has(ack, "\"answered\":false") && Has(ack, "\"handled\":false"));
        W906_ReauthClear();
    }

    // ---- 3. SetUp, drop-down mode -----------------------------------------------------------------------------------
    std::printf("  -- 3. SetUp, drop-down (golden stOperatorClick main.cpp:13193-13324)\n");
    RemoveBook();                                                                // no book -> golden bTechComExist false
    {
        W906ReauthResult r;
        LevelSet.AccessLevel[37] = 2;
        CHECK(PickLevel(1));                                                     // drop-down on Engineer
        bool ok = W906_Reauth("TestIF_File_SetUp", true, Ans("rtcOff", "", selSup), &r);   // bNeedPassword: SetUp arm
        Keep(r);
        std::string st = State();
        std::printf("    SetUp arm: ok=%d level=%d %s\n", (int)ok, AccessLevel, st.c_str());
        CHECK(ok && r.mode == "select" && AccessLevel == 2);                     // Supervisor's slot, drop-down ignored
        CHECK(Has(st, "\"itemIndex\":1") && Has(st, "\"levelName\":\"Engineer\""));   // golden: stOperatorClick leaves the drop-down
        CHECK(!g_W906SetupAsksPassword);                                         // the flag is only up during the call

        CHECK(PickLevel(1));
        ok = W906_Reauth("TestIF_File_SetUp", false, Ans("ocrOff", "", selSup), &r);      // OCR only (bNeedPassword false): normal arm
        Keep(r);
        std::printf("    normal arm: ok=%d level=%d\n", (int)ok, AccessLevel);
        CHECK(!ok && AccessLevel == 0);                                          // only the Engineer slot is compared

        CHECK(PickLevel(1));
        ok = W906_Reauth("TestIF_File_SetUp", true, Ans("rtcOff", "", selEng), &r);
        Keep(r);
        CHECK(!ok && AccessLevel == 1);                                          // SetUp arm finds Engineer's slot: 1 < 2 -> fail

        ok = W906_Reauth("TestIF_File_SetUp", true, Ans("rtcOff", "", superPw), &r);       // sSuperVisorString -> HonPrec
        Keep(r);
        CHECK(ok && AccessLevel == iDefHonPrecLevel);

        ok = W906_Reauth("TestIF_File_SetUp", true, Cancel("rtcOff"), &r);
        Keep(r);
        CHECK(!ok && AccessLevel == 0 && r.alarm.empty());                       // drop-down mode has no WAR1677 here (golden)
    }

    // ---- 4. Configuration, password book ----------------------------------------------------------------------------
    std::printf("  -- 4. Configuration (golden cConfiguration.cpp:6482-6529 + cbM01Click :6531-6544), password book\n");
    WriteBook();
    LevelSet.AccessLevel[92] = 2;
    {
        std::vector<std::string> reverted;
        const auto revert = [&reverted](const std::string& id) { reverted.push_back(id); };
        const std::vector<std::string> changed = { "cbM01_05", "cbM01_07" };
        std::string printed;
        bool handled = false;

        Logout();
        CHECK(BookAs("q45eng", g_eng));
        CHECK(Take("IniConfig", "{\"widgets\":{},\"reauth\":{\"point\":\"m01\",\"userId\":\"q45sup\",\"password\":\"" + g_sup + "\"}}", &printed).empty());
        bool ok = W906_ReauthConfigM01(changed, revert, &handled);
        std::string st = State();
        std::printf("    pass: ok=%d level=%d %s\n", (int)ok, AccessLevel, st.c_str());
        CHECK(ok && handled && reverted.empty());
        CHECK(AccessLevel == 0 && Has(st, "\"userCaption\":\"Operator\"") && Has(st, "\"itemIndex\":0") && Has(st, "\"btLogin\":\"Login\""));
        std::string ack = "{\"struct\":\"IniConfig\"}";
        W906_ReauthAck(&ack); Out(ack);
        CHECK(Has(ack, "\"loggedOut\":true") && Has(ack, "\"passed\":true") && Has(ack, "\"levelItem\":92"));
        W906_ReauthClear();

        CHECK(BookAs("q45sup", g_sup));
        CHECK(Take("IniConfig", "{\"widgets\":{},\"reauth\":{\"point\":\"m01\",\"userId\":\"q45sup\",\"password\":\"" + wrongPw + "\"}}", &printed).empty());
        ok = W906_ReauthConfigM01(changed, revert, &handled);
        std::printf("    wrong: ok=%d level=%d reverted=%d\n", (int)ok, AccessLevel, (int)reverted.size());
        CHECK(!ok && handled && reverted == changed && AccessLevel == 0);
        ack = "{\"struct\":\"IniConfig\"}"; W906_ReauthAck(&ack); Out(ack);
        CHECK(Has(ack, "\"alarm\":\"WAR1677\"") && Has(ack, "\"cbM01_05\"") && Has(ack, "\"loggedOut\":true"));
        W906_ReauthClear();

        reverted.clear();
        CHECK(BookAs("q45hon", g_hon));
        CHECK(Take("IniConfig", "{\"widgets\":{},\"reauth\":{\"point\":\"m01\",\"userId\":\"q45eng\",\"password\":\"" + g_eng + "\"}}", &printed).empty());
        ok = W906_ReauthConfigM01(changed, revert, &handled);
        CHECK(!ok && reverted == changed && AccessLevel == 0);                   // Engineer 1 < 2, then logged out anyway
        W906_ReauthClear();

        reverted.clear();                                                        // item 92 = 0: golden returns true, no ask, no logout
        CHECK(BookAs("q45sup", g_sup));
        LevelSet.AccessLevel[92] = 0;
        CHECK(Take("IniConfig", "{\"widgets\":{},\"reauth\":{\"point\":\"m01\",\"userId\":\"q45eng\",\"password\":\"" + wrongPw + "\"}}", &printed).empty());
        ok = W906_ReauthConfigM01(changed, revert, &handled);
        CHECK(ok && handled && reverted.empty() && AccessLevel == 2);
        ack = "{\"x\":1}"; W906_ReauthAck(&ack); Out(ack);
        CHECK(Has(ack, "\"asked\":false") && Has(ack, "\"loggedOut\":false"));
        W906_ReauthClear();
        LevelSet.AccessLevel[92] = 2;

        REAL_TIME_CCD = false;                                                   // not installed: no ask, no logout
        ok = W906_ReauthConfigM01(changed, revert, &handled);
        CHECK(ok && handled && reverted.empty() && AccessLevel == 2);
        REAL_TIME_CCD = true;

        ok = W906_ReauthConfigM01(changed, revert, &handled);                     // asked, no answer: put back, nothing else
        CHECK(!ok && !handled && reverted == changed && AccessLevel == 2 && Has(State(), "\"userCaption\":\"Supervisor\""));
        W906_ReauthClear();

        reverted.clear();                                                        // btLogin already "Logout": golden DoPassword skips btLoginClick
        Logout();
        CHECK(WebLogin_BookLogin(AnsiString("q45eng"), AnsiString(g_eng.c_str()), AnsiString(g_book.c_str()), &msg) == WEBLOGIN_OK);
        CHECK(Has(State(), "\"btLogin\":\"Logout\""));
        CHECK(Take("IniConfig", "{\"widgets\":{},\"reauth\":{\"point\":\"m01\",\"userId\":\"q45hon\",\"password\":\"" + g_hon + "\"}}", &printed).empty());
        ok = W906_ReauthConfigM01(changed, revert, &handled);
        std::printf("    already logged in: ok=%d level=%d\n", (int)ok, AccessLevel);
        CHECK(ok && reverted.empty() && AccessLevel == 0 && Has(State(), "\"btLogin\":\"Login\""));
        W906_ReauthClear();
    }

    // ---- 5. Configuration, drop-down mode ---------------------------------------------------------------------------
    std::printf("  -- 5. Configuration, drop-down\n");
    RemoveBook();
    {
        W906ReauthResult r;
        CHECK(PickLevel(1));
        bool ok = W906_Reauth("IniConfig", true /* ignored for Configuration */, Ans("m01", "", selSup), &r);
        Keep(r);
        std::string st = State();
        std::printf("    Engineer picked, Supervisor typed: ok=%d level=%d %s\n", (int)ok, AccessLevel, st.c_str());
        CHECK(!ok && AccessLevel == 0 && !r.loggedOut);                          // only the Engineer slot; no logout without a book
        CHECK(Has(st, "\"itemIndex\":1") && Has(st, "\"levelName\":\"Engineer\""));   // golden: the drop-down does not move (Q45-5)

        CHECK(PickLevel(2));
        ok = W906_Reauth("IniConfig", false, Ans("m01", "", selSup), &r);
        Keep(r);
        CHECK(ok && AccessLevel == 2 && !r.loggedOut);
    }

    // ---- 6. editlist.get extra.auth ---------------------------------------------------------------------------------
    std::printf("  -- 6. extra.auth\n");
    {
        LevelSet.AccessLevel[37] = 3;
        REAL_TIME_CCD = true;
        std::string j = W906_ReauthOpenJson("TestIF_File_SetUp", true, true, false, false);
        Out(j);
        std::printf("    SetUp: %s\n", j.c_str());
        cJSON* o = cJSON_Parse(j.c_str());
        const cJSON* pts = o ? cJSON_GetObjectItemCaseSensitive(o, "points") : 0;
        CHECK(o && std::string(cJSON_GetObjectItemCaseSensitive(o, "mode")->valuestring) == "select");   // book removed above
        CHECK(pts && cJSON_GetArraySize(pts) == 2);
        const cJSON* p0 = pts ? cJSON_GetArrayItem(pts, 0) : 0;
        const cJSON* p1 = pts ? cJSON_GetArrayItem(pts, 1) : 0;
        CHECK(p0 && std::string(cJSON_GetObjectItemCaseSensitive(p0, "id")->valuestring) == "rtcOff" &&
              cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(p0, "armed")) && cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(p0, "armedAtOpen")) &&
              cJSON_GetObjectItemCaseSensitive(p0, "level")->valueint == 3 && cJSON_GetObjectItemCaseSensitive(p0, "levelItem")->valueint == 37 &&
              cJSON_IsFalse(cJSON_GetObjectItemCaseSensitive(p0, "logoutAfter")));
        CHECK(p1 && std::string(cJSON_GetObjectItemCaseSensitive(p1, "id")->valuestring) == "ocrOff" && cJSON_IsFalse(cJSON_GetObjectItemCaseSensitive(p1, "armed")));
        if (o) cJSON_Delete(o);
        REAL_TIME_CCD = false;
        j = W906_ReauthOpenJson("TestIF_File_SetUp", true, true, true, true);
        CHECK(!Has(j, "\"armed\":true"));                                        // not installed: nothing armed
        REAL_TIME_CCD = true;

        WriteBook();
        LevelSet.AccessLevel[92] = 2;
        j = W906_ReauthOpenJson("IniConfig", false, false, false, false);
        Out(j);
        std::printf("    IniConfig: %s\n", j.c_str());
        o = cJSON_Parse(j.c_str());
        pts = o ? cJSON_GetObjectItemCaseSensitive(o, "points") : 0;
        CHECK(o && std::string(cJSON_GetObjectItemCaseSensitive(o, "mode")->valuestring) == "book");
        CHECK(pts && cJSON_GetArraySize(pts) == 5);
        p0 = pts ? cJSON_GetArrayItem(pts, 0) : 0;
        CHECK(p0 && std::string(cJSON_GetObjectItemCaseSensitive(p0, "id")->valuestring) == "m01" &&
              cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(p0, "armed")) && cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(p0, "logoutAfter")) &&
              cJSON_GetArraySize(cJSON_GetObjectItemCaseSensitive(p0, "controls")) == 16);
        int vendorArmed = 0;
        for (int i = 1; pts && i < cJSON_GetArraySize(pts); ++i)
            if (!cJSON_IsFalse(cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(pts, i), "armed"))) ++vendorArmed;
        CHECK(vendorArmed == 0 && Has(j, "網頁版不提供"));
        if (o) cJSON_Delete(o);
        LevelSet.AccessLevel[92] = 0;
        CHECK(Has(W906_ReauthOpenJson("IniConfig", false, false, false, false), "\"id\":\"m01\",\"controls\""));
        CHECK(!Has(W906_ReauthOpenJson("IniConfig", false, false, false, false), "\"armed\":true"));   // item 92 = 0
        int n = 0;
        const char* const* m01 = W906_ReauthControls("m01", &n);
        CHECK(m01 && n == 16 && std::string(m01[0]) == "cbM01" && std::string(m01[15]) == "cbM01_15");
        CHECK(W906_ReauthControls("nope", &n) == 0 && n == 0);
    }

    // ---- 7. the save-flow call sites are code, not comments ---------------------------------------------------------
    std::printf("  -- 7. call sites\n");
    {
        CHECK(!Has(CodeOf("x = 1;   // W906_ReauthAck(&ack);\n"), "W906_ReauthAck"));   // the stripper drops a call after //
        CHECK(!Has(CodeOf("/* W906_ReauthTake(root, wc.tag) */ y;\n"), "W906_ReauthTake"));
        CHECK(Has(CodeOf("s = \"a // b\"; W906_ReauthClear();\n"), "W906_ReauthClear"));   // a // inside a literal is not a comment
        const std::string root = TreeRoot();
        CHECK(!root.empty());
        const std::string gen = CodeOf(ReadFile(root + "FileRW\\TestIF_File_SetUp.gen.inc"));
        const std::string ini = CodeOf(ReadFile(root + "FileRW\\IniConfig.cpp"));
        const std::string su  = CodeOf(ReadFile(root + "FileRW\\TestIF_File_SetUp.cpp"));
        const std::string wb  = CodeOf(ReadFile(root + "tools\\wb_serve.cpp"));
        CHECK(Has(gen, "static bool SU_DoPassword() { bool W906_ReauthSetupDoPassword(bool, bool, bool, bool, bool*);"));
        CHECK(Has(gen, "return bHandled ? bFlag : filerw::ELPasswordRefused("));
        CHECK(Has(gen, "if(SU_DoPassword()==false)"));
        CHECK(Has(ini, "filerw::SessionBegin(answersJson);  IC_ReauthM01();"));
        CHECK(Has(ini, "!W906_ReauthHasAnswer(\"IniConfig\")"));
        CHECK(Has(ini, "W906_ReauthConfigM01(changed, revert, &handled);"));
        CHECK(Has(ini, "w.Key(\"auth\").RawValue(W906_ReauthOpenJson(\"IniConfig\""));
        CHECK(Has(su, "w.Key(\"auth\").RawValue(W906_ReauthOpenJson(\"TestIF_File_SetUp\""));
        CHECK(Has(wb, "const std::string reauthRefused = W906_ReauthTake(root, wc.tag);"));
        CHECK(Has(wb, "if (!reauthRefused.empty()) { perr = reauthRefused;"));
        CHECK(Has(wb, "if (st == 200) W906_ReauthAck(&ack);"));
        CHECK(Has(wb, "W906_ReauthClear(); } } reauthWipe;"));
    }

    // restore
    RemoveBook();
    CloseGeneralIniFile();
    std::memcpy(&USER, &savedUser, sizeof(PASS_WORD));
    pwPath = savedPwPath; sSuperVisorString = savedSuper;
    REAL_TIME_CCD = savedRtc; CosFunction.bUseLoginDatToSetLevel = savedBookFlag; CosFunction.bSecurityHave5Level = saved5;
    IniConfig.bPasswordSecret = savedSecret; IniConfig.bSPILFunction = savedSpil; CosFunction.bLoginShowUserName = savedShowName;
    CUSTOMER_CODE = savedCustomer; AccessLevel = savedLevel; JCET_FOR_EVAN = savedJcet;
    LevelSet.AccessLevel[37] = saved37; LevelSet.AccessLevel[92] = saved92;
    W906_ReauthClear();

    // ---- end the capture ------------------------------------------------------------------------------------------
    std::fflush(stdout);
    _dup2(savedOut, 1);
    _close(savedOut);
    const std::string captured = ReadFile(capture);
    std::fwrite(captured.data(), 1, captured.size(), stdout);                   // ctest's log shows the run

    // ---- 8. no password anywhere ----------------------------------------------------------------------------------
    std::printf("  -- 8. no password in any output\n");
    {
        Out(std::string(ExString.c_str()));
        std::vector<std::string> files;
        Walk(as9045LogPath.c_str(), t0, &files, 0);
        Walk(asSaveEventLogPath.c_str(), t0, &files, 0);
        Walk(sandbox, t0, &files, 0);                                            // includes the stdout capture
        int hits = 0, scanned = 0;
        for (size_t s = 0; s < g_secrets.size(); ++s) {
            for (size_t i = 0; i < g_outputs.size(); ++i) if (Has(g_outputs[i], g_secrets[s])) ++hits;
            if (Has(captured, g_secrets[s])) ++hits;
        }
        for (size_t f = 0; f < files.size(); ++f) {
            if (Lower(files[f]) == Lower(g_book)) continue;                      // the test's own book (already deleted)
            const std::string body = ReadFile(files[f]);
            ++scanned;
            for (size_t s = 0; s < g_secrets.size(); ++s)
                if (Has(body, g_secrets[s])) { ++hits; std::printf("    secret found in %s\n", files[f].c_str()); }
        }
        std::printf("    %d outputs, %d captured bytes, %d log/scratch files scanned, %d hits\n",
                    (int)g_outputs.size(), (int)captured.size(), scanned, hits);
        CHECK(hits == 0);
        CHECK(!g_outputs.empty() && !captured.empty());
    }

    // ---- 9. the machine's own files ---------------------------------------------------------------------------------
    for (int i = 0; i < 4; ++i) {
        const Stamp now = StampOf(kReal[i]);
        std::printf("  %s unchanged: %s\n", kReal[i], Same(real0[i], now) ? "yes" : "NO");
        CHECK(Same(real0[i], now));
    }
    ::DeleteFileA(capture.c_str());
    ::RemoveDirectoryA(sandbox.c_str());                                         // only when empty

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
