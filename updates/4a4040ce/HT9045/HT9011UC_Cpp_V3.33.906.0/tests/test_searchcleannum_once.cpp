// =============================================================================
//  test_searchcleannum_once.cpp -- AI(W906-FLOW-5) 20260929
//  ctest: SearchCleanNumOnce
//
//  RULINGS_20260929 section 5 item 5 = A: "AutoClean count reached AlarmCount" keeps golden 906's WAR16313, and
//  the two SearchCleanNum copies become ONE -- the Cleaning page / boot copy (FileRW/TestIF_File_Cleaning.cpp
//  FileRW_Cleaning_SearchCleanNum, which was a V912 copy raising WAR1922 with its own "last alarmed" static)
//  now forwards to AutoClean/AutoClean.cpp's SearchCleanNum (golden AutoClean.cpp:1245-1304) through
//  W906_AutoClean_SearchCleanNum, so one count alarms once whichever path reaches it first.
//
//  PART A (behaviour, AutoClean side -- the one body both paths now reach; alarms counted BY CODE through
//  W906_ShowErrorMessage_Hook, canary_support.h:129):
//    A1  count 7 >= AlarmCount 5         -> returns 7, ONE WAR16313, pnlCleanCount red, the three text mirrors = "7"
//    A2  the same count again            -> no second alarm (golden :1289 iMin!=iLastAutoCleanAlarmCount)
//    A3  CheckCleaningCount (bCleanCountAlarmByMin) reaches the same body -> still no second alarm, returns false
//    A4  count 8                         -> one new WAR16313 (a new count alarms again)
//    A5  count 3 < 5                     -> no alarm, pnlCleanCount navy (golden :1300)
//    A6  back to 8                       -> NO alarm: golden's static is only written when it alarms (golden quirk,
//                                           pinned so a later "fix" is a visible decision, not an accident)
//    A7  iAutoClean_Function==0          -> returns atoi(edCleaningCount), no alarm (golden :1250-1253)
//    A8  WAR1922 (the V912 code) is never raised
//  PART B (source text -- FileRW/TestIF_File_Cleaning.cpp lives in wb_serve only, so this test cannot link it):
//    B1  FileRW_Cleaning_SearchCleanNum's live body is exactly `return W906_AutoClean_SearchCleanNum();`
//    B2  TestIF_File_Cleaning.cpp has no live WAR1922 and no live iLastAutoCleanAlarmCount (comments and #if 0
//        groups removed first)
//    B3  AutoClean.cpp has exactly ONE live `static int iLastAutoCleanAlarmCount` and the EOF wrapper
//    B4  TestIF_File_Cleaning.gen.inc still routes the page's SearchCleanNum() calls to the forwarder
//
//  Files touched: none of the machine's.  RecordProcess -> MyDBIProcess writes the event-log / production-log text
//  files, which the directory-wide redirect roots in tests/CMakeLists.txt (_w906_env_all_tests) send to the build dir.
// =============================================================================
#include "AutoClean/AutoClean.h"
#include "Motor/mymotor.h"
#include "cprod.h"
#include "cmydef.h"
#include "cpublic.h"
#include "CosFunction.h"
#include "FormsFacade.h"            // fMain (pnlCleanCount / pnlCleanCountFont / AutoCleanStringGrid), fCleaning
#include "forms/fShowBinSelect.h"   // fShowBinSelect->ed_AutoCleanCount
#include "canary_support.h"         // W906_ShowErrorMessage_Hook / _LastCode / _Reset
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

int W906_AutoClean_SearchCleanNum();   // AutoClean/AutoClean.cpp (end of file)

// Count the two codes by name through the answer hook, so an unrelated alarm raised on the way (none is expected)
// can neither be mistaken for the one under test nor hide it.  Returning 0 keeps the sim answer (K_RETRY).
static int g_war16313 = 0, g_war1922 = 0;
static int CountAlarm(const char* Code, int /*KCode*/, int /*Pos*/)
{
    if (std::strcmp(Code, "WAR16313") == 0) ++g_war16313;
    if (std::strcmp(Code, "WAR1922") == 0)  ++g_war1922;
    return 0;
}

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }               \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// ---------------------------------------------------------------------------
//  PART A helpers
// ---------------------------------------------------------------------------
static void SeedKit(int alarmCount)
{
    TestIF_File.iAutoClean_Function   = 1;
    TestIF_File.iAutoClean_AlarmCount = alarmCount;
    TestIF.iAutoClean_AlarmCount      = alarmCount;
    MOT[MMAutoCleanKit].Tray.SetXYItem(4, 1);
    MOT[MMAutoCleanKit].Tray.ClearData();
    fMain->AutoCleanStringGrid->ColCount = 8;
    fMain->AutoCleanStringGrid->RowCount = 8;
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            fMain->AutoCleanStringGrid->Cells[x][y] = AnsiString("0");
    MOT[MMAutoCleanKit].Tray.Data[0][0] = HAS_CLEAN_IC;   // one pad in use (golden :1260-1263 set)
}
static void SetPadCount(int n)
{
    char b[16];
    std::snprintf(b, sizeof(b), "%d", n);
    fMain->AutoCleanStringGrid->Cells[0][1] = AnsiString(b);   // golden :1265 reads Cells[X][Y+1]
}

// ---------------------------------------------------------------------------
//  PART B helpers: comment / string aware stripper, then #if 0 group removal
// ---------------------------------------------------------------------------
static std::string RootDir()
{
    std::string self(__FILE__);
    std::string::size_type a = self.find_last_of("/\\");
    if (a == std::string::npos) return std::string();
    std::string dir = self.substr(0, a);                 // .../tests
    std::string::size_type b = dir.find_last_of("/\\");
    if (b == std::string::npos) return std::string();
    return dir.substr(0, b);                              // the ported tree root
}
static bool ReadAll(const std::string& p, std::string& out)
{
    std::ifstream f(p.c_str(), std::ios::in | std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    out = ss.str();
    return !out.empty();
}
static std::string StripComments(const std::string& s)
{
    std::string o;
    o.reserve(s.size());
    size_t i = 0, n = s.size();
    while (i < n) {
        char c = s[i];
        if (c == '/' && i + 1 < n && s[i + 1] == '/') {
            while (i < n && s[i] != '\n') ++i;
        } else if (c == '/' && i + 1 < n && s[i + 1] == '*') {
            i += 2;
            while (i + 1 < n && !(s[i] == '*' && s[i + 1] == '/')) { if (s[i] == '\n') o += '\n'; ++i; }
            i += 2;
            o += ' ';
        } else if (c == '"' || c == '\'') {
            char q = c;
            o += c; ++i;
            while (i < n && s[i] != q && s[i] != '\n') {
                if (s[i] == '\\' && i + 1 < n) { o += s[i]; ++i; }
                o += s[i]; ++i;
            }
            if (i < n) { o += s[i]; ++i; }
        } else {
            o += c; ++i;
        }
    }
    return o;
}
static std::string Trim(const std::string& s)
{
    size_t a = s.find_first_not_of(" \t\r");
    if (a == std::string::npos) return std::string();
    size_t b = s.find_last_not_of(" \t\r");
    return s.substr(a, b - a + 1);
}
// Drops every `#if 0` group (up to its #else/#elif or matching #endif); keeps everything else.
static std::string DropIf0(const std::string& s)
{
    std::istringstream in(s);
    std::string line, o;
    int depth = 0, skipAt = -1;
    while (std::getline(in, line)) {
        std::string t = Trim(line);
        bool pp = !t.empty() && t[0] == '#';
        std::string d;
        if (pp) { d = Trim(t.substr(1)); }
        if (pp && (d.compare(0, 2, "if") == 0)) {
            ++depth;
            if (skipAt < 0) {
                std::string e = Trim(d.substr(2));
                if (d.compare(0, 5, "ifdef") != 0 && d.compare(0, 6, "ifndef") != 0 && e == "0") { skipAt = depth; continue; }
            }
        } else if (pp && d.compare(0, 5, "endif") == 0) {
            if (skipAt == depth) { skipAt = -1; --depth; continue; }
            --depth;
        } else if (pp && (d.compare(0, 4, "else") == 0 || d.compare(0, 4, "elif") == 0)) {
            if (skipAt == depth) { skipAt = -1; continue; }
        }
        if (skipAt < 0) { o += line; o += '\n'; }
    }
    return o;
}
static std::string Squash(const std::string& s)
{
    std::string o;
    bool ws = false;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') { ws = true; continue; }
        if (ws && !o.empty()) o += ' ';
        ws = false;
        o += c;
    }
    return o;
}
static int Count(const std::string& hay, const std::string& needle)
{
    int k = 0;
    for (size_t p = hay.find(needle); p != std::string::npos; p = hay.find(needle, p + needle.size())) ++k;
    return k;
}
static bool Live(const std::string& rel, std::string& out)
{
    std::string raw;
    if (!ReadAll(RootDir() + "/" + rel, raw)) return false;
    out = Squash(DropIf0(StripComments(raw)));
    return true;
}

int main()
{
    std::printf("==== FLOW-5 SearchCleanNumOnce (RULINGS_20260929 section 5 item 5 = A) ====\n");

    const bool saveByMin = CosFunction.bCleanCountAlarmByMin;
    int (*const saveHook)(const char*, int, int) = W906_ShowErrorMessage_Hook;
    W906_ShowErrorMessage_Reset();
    W906_ShowErrorMessage_Hook = CountAlarm;

    // ---- PART A -------------------------------------------------------------
    std::printf("[A1] count 7 reaches AlarmCount 5 -> one WAR16313\n");
    SeedKit(5);
    SetPadCount(7);
    int r = W906_AutoClean_SearchCleanNum();
    CHECK(r == 7, "A1a returns the minimum in-use pad count (7)");
    CHECK(g_war16313 == 1 && g_war1922 == 0, "A1b exactly one WAR16313 and no WAR1922 (golden 906 :1295, not V912's code)");
    CHECK(W906_ShowErrorMessage_LastCode == AnsiString("WAR16313"), "A1c it is the last alarm raised");
    CHECK(fMain->pnlCleanCountFont->Color == 0x000000FF, "A1d pnlCleanCount font clRed (golden :1284)");
    CHECK(fMain->pnlCleanCount->Caption == AnsiString("Cleaned Count 7 / 5"), "A1e pnlCleanCount caption (golden :1280-1281)");
    CHECK(fCleaning->edCleaningCount->Text == AnsiString("7"), "A1f fCleaning->edCleaningCount mirror (golden :1279)");
    CHECK(fShowBinSelect->ed_AutoCleanCount->Text == AnsiString("7"), "A1g fShowBinSelect->ed_AutoCleanCount mirror (golden :1278)");

    std::printf("[A2] same count again -> no second alarm\n");
    r = W906_AutoClean_SearchCleanNum();
    CHECK(r == 7 && g_war16313 == 1, "A2 same count 7 -> returns 7, still one WAR16313 (golden :1289)");

    std::printf("[A3] CheckCleaningCount reaches the same body -> still once\n");
    CosFunction.bCleanCountAlarmByMin = true;
    bool ok = CheckCleaningCount();
    CHECK(ok == false, "A3a CheckCleaningCount: 7 >= TestIF.iAutoClean_AlarmCount 5 -> false (AutoClean.cpp:1707-1715)");
    CHECK(g_war16313 == 1, "A3b the engine path shares the one static -> no second alarm for 7");
    CosFunction.bCleanCountAlarmByMin = saveByMin;

    std::printf("[A4] count 8 -> a new count alarms again\n");
    SetPadCount(8);
    r = W906_AutoClean_SearchCleanNum();
    CHECK(r == 8 && g_war16313 == 2, "A4 count 8 -> one more WAR16313");

    std::printf("[A5] count 3 < AlarmCount -> no alarm, navy\n");
    SetPadCount(3);
    r = W906_AutoClean_SearchCleanNum();
    CHECK(r == 3 && g_war16313 == 2, "A5a count 3 -> no alarm");
    CHECK(fMain->pnlCleanCountFont->Color == 0x00800000, "A5b pnlCleanCount font clNavy (golden :1300)");

    std::printf("[A6] back to 8 -> no alarm (golden quirk: the static moves only when it alarms)\n");
    SetPadCount(8);
    r = W906_AutoClean_SearchCleanNum();
    CHECK(r == 8 && g_war16313 == 2, "A6 count 8 again after 3 -> no alarm (iLastAutoCleanAlarmCount is still 8)");

    std::printf("[A7] iAutoClean_Function==0 -> the edit's number, no alarm\n");
    TestIF_File.iAutoClean_Function = 0;
    fCleaning->edCleaningCount->Text = AnsiString("42");
    r = W906_AutoClean_SearchCleanNum();
    CHECK(r == 42 && g_war16313 == 2, "A7 Function 0 -> atoi(edCleaningCount)=42, no alarm (golden :1250-1253)");
    TestIF_File.iAutoClean_Function = 1;
    CHECK(g_war1922 == 0, "A8 WAR1922 (the V912 code) was never raised");
    W906_ShowErrorMessage_Hook = saveHook;

    // ---- PART B -------------------------------------------------------------
    std::printf("[B] source text (root %s)\n", RootDir().c_str());
    std::string cl, ac, gi;
    bool rc = Live("FileRW/TestIF_File_Cleaning.cpp", cl);
    bool ra = Live("AutoClean/AutoClean.cpp", ac);
    bool rg = Live("FileRW/TestIF_File_Cleaning.gen.inc", gi);
    CHECK(rc && ra && rg, "B0 the three source files are readable (path from __FILE__)");
    CHECK(Count(cl, "int FileRW_Cleaning_SearchCleanNum() { return W906_AutoClean_SearchCleanNum(); }") == 1,
          "B1 FileRW_Cleaning_SearchCleanNum is one forwarding line");
    CHECK(Count(cl, "WAR1922") == 0 && Count(cl, "iLastAutoCleanAlarmCount") == 0,
          "B2 TestIF_File_Cleaning.cpp: no live WAR1922, no live second static");
    CHECK(Count(ac, "static int iLastAutoCleanAlarmCount") == 1,
          "B3a AutoClean.cpp: exactly one live 'last alarmed' static");
    CHECK(Count(ac, "int W906_AutoClean_SearchCleanNum() { return SearchCleanNum(); }") == 1,
          "B3b AutoClean.cpp: the external wrapper forwards to the file-static SearchCleanNum");
    CHECK(Count(gi, "#define SearchCleanNum FileRW_Cleaning_SearchCleanNum") == 1,
          "B4 TestIF_File_Cleaning.gen.inc: the page's SearchCleanNum() calls still go through the forwarder");

    std::printf("==== SearchCleanNumOnce: %d passed, %d failed ====\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
