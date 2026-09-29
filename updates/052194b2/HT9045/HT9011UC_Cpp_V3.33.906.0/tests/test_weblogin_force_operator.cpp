// =============================================================================
//  tests/test_weblogin_force_operator.cpp -- D4: golden TfMain::ChangeTesterConnect's Off-Line -> On-Line "forced back to
//  Operator" block (golden 906_0625_Steven main.cpp:12104-12131; 912 main.cpp:12621-12648, same body), ported as
//  WebLogin.cpp W906_WebLoginForceOperator and installed in W906_WebLoginForceOperatorHook (LogObjects.cpp EOF).
//  AI(W906-D4) 20260928 (St02-E).  Suite name (add_test): WebLogin_ForceOperator
//
//    0. containment first: refuses to run (exit 1, nothing called) unless
//         * the three login seams W906_LOGINDAT_PATH / W906_PWBOOK_PATH / W906_LEVELSET_PATH are set (tests/CMakeLists.txt,
//           a DEFERred APPEND into the build dir's weblogin_d4_scratch) and none of them is under D:\HT9045\system --
//           the body opens none of those files; the seams are set anyway so that a later change to it cannot reach
//           the machine's own login.dat / levelset.dat;
//         * the cMyDB log roots are the ctest sandbox (as test_mydb_p4_containment step 0): golden writes
//           NewRecordProcess("MES2140", "======== Operator login ========") on two of the three branches, and that
//           row goes to the event-log files under those roots (SaveEventLogInfo).
//       Then it records existence / size / write time of D:\HT9045\system\login.dat and levelset.dat.
//    1. the seat holds &W906_WebLoginForceOperator after static init (WebLogin.cpp's installer struct).
//    2. the three golden branches (906_0625_Steven line numbers), each from a raised state (Supervisor picked,
//       AccessLevel 2, btLogin "Logout"), called through the seat as forms/fMain.cpp calls it:
//         a. 4-level (CosFunction.bSecurityHave5Level false)          -> "Operator", MES2140 logged  (golden :12123-12128)
//         b. 5-level, CUSTOMER_CODE CC_HONPREC_QC (not CC_KYEC_LEE)    -> "Open",     nothing logged  (golden :12117-12121)
//         c. 5-level, CUSTOMER_CODE == CC_KYEC_LEE                     -> "Operator", MES2140 logged  (golden :12111-12116)
//       and every branch: AccessLevel 0 (:12106), WebLogin_StateJson itemIndex 0 (:12105) / levelName / userCaption /
//       btLogin "Login" (:12130); TfMain::TemperatureEditDisable (:12131) runs -- checked only as "no crash" (facade flags).
//       "Logged" is read from cMyDB.cpp's ExString (NewRecordProcess stores its message there, cMyDB.cpp:1877): cleared
//       before each call, it holds the golden message afterwards, or stays empty.  (Counting rows in an event-log file does
//       not work: TMyStringList::MySaveToFile appends its whole buffer each time, so a row can land twice.)
//    3. D:\HT9045\system\login.dat and levelset.dat: existence, size and write time unchanged.
//    4. AI(W906-Q45-B5) 20260929 (St02-E): the Q45 re-login pieces (St01 B5), run before 3:
//         a. stOperatorClick's SetUp arm (golden V912 main.cpp:13283-13301) through WebLogin_Select, flag
//            g_W906SetupAsksPassword on: Supervisor's password with Engineer picked -> level 2 (drop-down ignored,
//            case-insensitive); Engineer's -> 1; the same password in both slots -> 2 ([1] is tried first);
//            flag off -> the normal arm (:13304-13320) -> 0.  USER is saved and restored around it.
//         b. the book compare split with a 1-line text book in the sandbox: WebLogin_BookCompare logs in without
//            touching btLogin (golden cbUserSelectChange; btLoginClick :28091 sets "Logout" before calling it);
//            WebLogin_BookLogin (btLoginClick) then shows "Logout" and refuses a second login; a wrong password in
//            the compare -> Operator and btLogin "Login" (golden bError :15333-15344).
//  NOT here: the call site (forms/fMain.cpp TfMain::ChangeTesterConnect :1148).  That line is its own commit, and the path
//  through it runs fMain->ModifyTester(ON_LINE) first (LastSet), which is not a thing a ctest may do on a machine.
//  ht9045::formjson::FormLock / FormUnlock (JsonBridge/FormJson.cpp, wb_serve only) are empty stand-ins below; since 8c63dffc
//  W906_WebLoginForceOperator calls them on every run, so this test does NOT exercise the real FormJson lock (wb_serve does).
//  Run by hand (no ctest environment) it refuses at step 0 -- that is the point.
// =============================================================================
#include "WebLogin.h"
#include "cMyDB.h"         // ExString
#include "cmydef.h"        // AccessLevel, pwPath, CUSTOMER_CODE
#include "common.h"        // as9045LogPath, asSaveEventLogPath, asProductionLogPath, sProductionInfoFilePath
#include "CosFunction.h"
#include "Config.h"
#include "MachineType.h"   // CC_KYEC_LEE, CC_HONPREC_QC
#include "forms/fMain.h"
#include "cprod.h"         // USER (golden PASS_WORD)   AI(W906-Q45-B5)

#include <windows.h>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

extern void (*W906_WebLoginForceOperatorHook)();   // LogObjects.cpp EOF

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

static bool Has(const std::string& s, const char* sub) { return s.find(sub) != std::string::npos; }

static std::string Lower(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '/') s[i] = '\\';
        s[i] = (char)std::tolower((unsigned char)s[i]);
    }
    return s;
}

struct Stamp { bool exists; unsigned long long size; FILETIME write; };
static Stamp StampOf(const char* p)
{
    Stamp s = { false, 0, { 0, 0 } };
    WIN32_FILE_ATTRIBUTE_DATA a;
    if (::GetFileAttributesExA(p, GetFileExInfoStandard, &a))
    {
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

// Supervisor picked in the drop-down (golden cbUserSelectChange, AccessLevel already 2 -> no password box), btLogin "Logout".
static bool RaiseToSupervisor()
{
    CosFunction.bSecurityHave5Level = false;       // WebLogin_Select refuses 5-level (not ported); the branch flag is set after
    AccessLevel = 2;
    std::string msg;
    const int r = WebLogin_Select(2, false, AnsiString(""), &msg);
    WebLogin_LoggedIn();
    const std::string st = WebLogin_StateJson();
    std::printf("    raised: r=%d %s\n", r, st.c_str());
    return r == WEBLOGIN_OK && Has(st, "\"itemIndex\":2") && Has(st, "\"level\":2") && Has(st, "\"btLogin\":\"Logout\"");
}

static void Branch(const char* title, bool fiveLevel, int customer, const char* want, bool wantLogged)
{
    static const char* const kGoldenMsg = "======== Operator login ========";   // golden :12115 / :12127
    std::printf("  -- %s\n", title);
    CHECK(RaiseToSupervisor());
    CosFunction.bSecurityHave5Level = fiveLevel;
    CUSTOMER_CODE = customer;
    ExString = "";

    W906_WebLoginForceOperatorHook();                                  // as forms/fMain.cpp ChangeTesterConnect calls it

    const std::string logged = ExString.c_str();
    const std::string st = WebLogin_StateJson();
    const std::string text = fMain->cbUserSelect->Text.c_str();
    std::printf("    after: logged='%s' text=%s %s\n", logged.c_str(), text.c_str(), st.c_str());
    CHECK(AccessLevel == 0);
    CHECK(text == want);
    CHECK(Has(st, "\"itemIndex\":0"));
    CHECK(Has(st, "\"level\":0"));
    CHECK(Has(st, (std::string("\"levelName\":\"") + want + "\"").c_str()));
    CHECK(Has(st, (std::string("\"userCaption\":\"") + want + "\"").c_str()));
    CHECK(Has(st, "\"btLogin\":\"Login\""));
    CHECK(wantLogged ? (logged == kGoldenMsg) : logged.empty());
}

int main()
{
    std::printf("WebLogin_ForceOperator\n");

    // ---- 0. containment first --------------------------------------------------------------------------------
    const char* const seamNames[] = { "W906_LOGINDAT_PATH", "W906_PWBOOK_PATH", "W906_LEVELSET_PATH" };
    bool sandboxed = true;
    for (int i = 0; i < 3; ++i)
    {
        const char* v = std::getenv(seamNames[i]);
        std::printf("  %s = %s\n", seamNames[i], v ? v : "(unset)");
        if (v == 0 || *v == 0 || Lower(v).compare(0, 17, "d:\\ht9045\\system\\") == 0 || !Has(Lower(v), "weblogin_d4_scratch"))
            sandboxed = false;
    }
    const AnsiString* const roots[] = { &as9045LogPath, &asSaveEventLogPath, &asProductionLogPath, &sProductionInfoFilePath };
    const char* const rootNames[] = { "as9045LogPath", "asSaveEventLogPath", "asProductionLogPath", "sProductionInfoFilePath" };
    for (int i = 0; i < 4; ++i)
    {
        std::printf("  %s = %s\n", rootNames[i], roots[i]->c_str());
        if (!Has(roots[i]->c_str(), "machine_log_scratch"))
            sandboxed = false;
    }
    CHECK(sandboxed);
    if (!sandboxed)
    {
        std::printf("FAIL: not sandboxed -- refusing to call W906_WebLoginForceOperator\n");
        return 1;
    }

    static const char* const kReal[] = { "D:\\HT9045\\system\\login.dat", "d:\\HT9045\\system\\levelset.dat" };
    Stamp real0[2];
    for (int i = 0; i < 2; ++i)
    {
        real0[i] = StampOf(kReal[i]);
        std::printf("  %s: %s\n", kReal[i], real0[i].exists ? "exists" : "absent");
    }

    std::string sandbox = std::getenv("W906_LOGINDAT_PATH");
    for (size_t i = 0; i < sandbox.size(); ++i) if (sandbox[i] == '/') sandbox[i] = '\\';
    sandbox = sandbox.substr(0, sandbox.find_last_of('\\'));

    // what the three branches below change; restored at the end
    const AnsiString savedPwPath = pwPath;
    const bool savedUseLoginDat = CosFunction.bUseLoginDatToSetLevel;
    const bool saved5 = CosFunction.bSecurityHave5Level;
    const int savedCustomer = CUSTOMER_CODE;
    const int savedLevel = AccessLevel;

    pwPath = AnsiString((sandbox + "\\no_such_pwbook.com").c_str());     // drop-down mode (golden FormShow :10863 FileExists(pwPath))
    CosFunction.bUseLoginDatToSetLevel = false;
    IniConfig.bSPILFunction = false;

    // ---- 1. the seat -------------------------------------------------------------------------------------------
    CHECK(W906_WebLoginForceOperatorHook == &W906_WebLoginForceOperator);
    if (W906_WebLoginForceOperatorHook == 0)
    {
        std::printf("FAIL: the seat is empty -- WebLogin.cpp's installer did not run\n");
        return 1;
    }

    // ---- 2. the three golden branches ----------------------------------------------------------------------------
    Branch("a. 4-level", false, CC_HONPREC_QC, "Operator", true);
    Branch("b. 5-level, CC_HONPREC_QC", true, CC_HONPREC_QC, "Open", false);
    Branch("c. 5-level, CC_KYEC_LEE", true, CC_KYEC_LEE, "Operator", true);

    // ---- 4. Q45 re-login pieces (AI(W906-Q45-B5) 20260929, St02-E) ---------------------------------------------------
    {
        std::printf("  -- 4a. stOperatorClick's SetUp arm (golden V912 main.cpp:13283-13301)\n");
        static PASS_WORD savedUser;
        std::memcpy(&savedUser, &USER, sizeof(PASS_WORD));
        const bool savedSecret = IniConfig.bPasswordSecret;  const bool savedFive = CosFunction.bSecurityHave5Level; CosFunction.bSecurityHave5Level = false;  OpenGeneralIniFile();   //AI(W906-MERGE-0929b) 20260929: (+ OpenGeneralIniFile: stOperatorClick reads/writes Gerneral.ini [VENDER] (WebLogin.cpp:180-187) through INIFileGeneral, NULL in this test -> SIGSEGV in TIniFile::ValueExists; ctest redirects asGeneralPath to the sandbox copy, W906_GENERAL_INI_PATH) laptop gate: Branch("c. 5-level") above leaves bSecurityHave5Level=true (Branch never restores it), so WebLogin_Select returned WEBLOGIN_BAD_ARG (3, "5-level security is not ported yet") for every try -- 4a is a 4-level scenario
        std::string msg;
        struct Try { const char* p0; const char* p1; int pick; const char* typed; bool flag; int want; const char* what; };
        const Try tries[] = {
            { "EngPw", "SupPw", 1, "suppw", true,  2, "flag on: Supervisor's password, Engineer picked -> 2 (drop-down ignored, any case)" },
            { "EngPw", "SupPw", 2, "ENGPW", true,  1, "flag on: Engineer's password, Supervisor picked -> 1" },
            { "Same",  "Same",  1, "same",  true,  2, "flag on: the same password in both slots -> 2 ([1] first)" },
            { "EngPw", "SupPw", 1, "suppw", false, 0, "flag off: the normal arm needs the picked level -> 0" },
        };
        for (size_t t = 0; t < sizeof(tries) / sizeof(tries[0]); ++t)
        {
            std::strcpy(USER.PassWord[0], tries[t].p0);
            std::strcpy(USER.PassWord[1], tries[t].p1);
            AccessLevel = 0;
            g_W906SetupAsksPassword = tries[t].flag;
            const int r = WebLogin_Select(tries[t].pick, true, AnsiString(tries[t].typed), &msg);
            g_W906SetupAsksPassword = false;
            std::printf("    %s: r=%d level=%d\n", tries[t].what, r, AccessLevel);
            CHECK(AccessLevel == tries[t].want);
        }
        std::memcpy(&USER, &savedUser, sizeof(PASS_WORD));  CosFunction.bSecurityHave5Level = savedFive;  CloseGeneralIniFile();   //AI(W906-MERGE-0929b) 20260929: restore (+ close the sandbox Gerneral.ini)

        std::printf("  -- 4b. book compare split (golden cbUserSelectChange vs btLoginClick :28080-28094)\n");
        IniConfig.bPasswordSecret = false;
        const std::string book = sandbox + "\\q45_b5_book.txt";   // sandbox = weblogin_d4_scratch (step 0 checked it)
        CreateDirectoryA(sandbox.c_str(), 0);                      // ctest does not create it (the D4 body opens no file)
        FILE* bf = std::fopen(book.c_str(), "wb");
        CHECK(bf != 0);
        if (bf) { std::fputs("b5user 2 b5pass\r\n", bf); std::fclose(bf); }
        const AnsiString bk(book.c_str());
        CHECK(WebLogin_Logout(&msg));
        int r = WebLogin_BookCompare(AnsiString("B5USER"), AnsiString("b5pass"), bk, &msg);
        std::string st = WebLogin_StateJson();
        std::printf("    compare ok: r=%d %s\n", r, st.c_str());
        CHECK(r == WEBLOGIN_OK && AccessLevel == 2 && Has(st, "\"btLogin\":\"Login\""));
        CHECK(WebLogin_Logout(&msg));
        r = WebLogin_BookLogin(AnsiString("b5user"), AnsiString("B5PASS"), bk, &msg);
        st = WebLogin_StateJson();
        std::printf("    btLogin login: r=%d %s\n", r, st.c_str());
        CHECK(r == WEBLOGIN_OK && AccessLevel == 2 && Has(st, "\"btLogin\":\"Logout\""));
        r = WebLogin_BookLogin(AnsiString("b5user"), AnsiString("b5pass"), bk, &msg);
        std::printf("    second btLogin login: r=%d (%s)\n", r, msg.c_str());
        CHECK(r == WEBLOGIN_ALREADY_IN);
        r = WebLogin_BookCompare(AnsiString("b5user"), AnsiString("wrong"), bk, &msg);
        st = WebLogin_StateJson();
        std::printf("    compare wrong: r=%d %s\n", r, st.c_str());
        CHECK(r == WEBLOGIN_BAD_CREDENTIALS && AccessLevel == 0 && Has(st, "\"btLogin\":\"Login\"") && Has(st, "\"itemIndex\":0"));
        DeleteFileA(book.c_str());
        RemoveDirectoryA(sandbox.c_str());                         // only when empty
        IniConfig.bPasswordSecret = savedSecret;
    }

    pwPath = savedPwPath;
    CosFunction.bUseLoginDatToSetLevel = savedUseLoginDat;
    CosFunction.bSecurityHave5Level = saved5;
    CUSTOMER_CODE = savedCustomer;
    AccessLevel = savedLevel;

    // ---- 3. the machine's own files ------------------------------------------------------------------------------
    for (int i = 0; i < 2; ++i)
    {
        const Stamp now = StampOf(kReal[i]);
        std::printf("  %s unchanged: %s\n", kReal[i], Same(real0[i], now) ? "yes" : "NO");
        CHECK(Same(real0[i], now));
    }

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
