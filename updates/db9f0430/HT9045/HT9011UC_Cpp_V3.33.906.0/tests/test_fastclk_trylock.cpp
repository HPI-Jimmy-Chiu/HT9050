// =============================================================================
//  tests/test_fastclk_trylock.cpp -- AI(W906-FASTCLK) 20261003.  Suite: FastClk_TryLock (both configs).  Memory only.
//
//  RULINGS_20261002 #7 (s0 #35 = A): 「冷卻風扇拿 FormLock 的鎖先改（主迴圈不該等 HTTP 執行緒）」.
//  The two golden reads of the Configuration page in the heater body -- DoSwCoolingFan GATE H1-08 (csystem.cpp; golden 906
//  csystem.cpp:20442-20446) and DoThermo GATE G14a (bthermo.cpp; golden 906 bthermo.cpp:1004-1005) -- go through
//  FileRW_ProxyTryRead (FileRW/_ProxyTry.cpp, compiled here): TryEnterCriticalSection on the FormJson lock.
//
//  This test is the FormJson lock and the FileRW proxies:
//    * FormLock / FormUnlock / FormTryLock on a real CRITICAL_SECTION, as JsonBridge/FormJson.cpp;
//    * the seven symbols FileRW/_fallback.cpp defines (so that member is never extracted), the two READS taking FormLock exactly
//      as FileRW/_EditList.cpp's do -- so a caller that goes back to them blocks here as it would in wb_serve.
//    1. lock free, page table "fConfiguration" open, chkHeater checked + PageControl1 on tab 1 -> DoSwCoolingFan refuses
//       (golden H1-08: false, outputs untouched); both reads ran with this thread already holding the lock (one acquisition).
//    2. lock free, chkHeater unchecked -> not refused: the normal rule drives the fan (blower on, valve off).
//    3. ANOTHER thread holds the lock for 1.5 s (the socket thread's GET /api/editlist): DoSwCoolingFan returns false within
//       100 ms, outputs untouched (this beat skipped); DoThermo returns within 100 ms without entering DoThermoReal
//       (iThermoTask stays 1); FileRW_ProxyTryBusyCount +2.
//    4. still held, Configuration page closed: neither asks for the lock (golden's own condition order): the fan is driven,
//       DoThermoReal takes its first step (iThermoTask 1 -> 100).
//    5. released: DoSwCoolingFan gives the step-1 answer again; DoThermo with Configuration open on tab 1 returns at golden's
//       G14a (iThermoTask untouched), and on tab 0 goes on (1 -> 100).
//    6. source pins (argv[1] = tree root, read only): the H1-08 / G14a lines read through FileRW_ProxyTryRead, the blocking
//       readers are gone from both function bodies; FileRW/_ProxyTry.cpp takes FormTryLock and never FormLock; FormJson.cpp's
//       FormTryLock is TryEnterCriticalSection on the one g_lock.
//  CONTROL (run 20261003): a scratch root whose csystem.cpp has the 20261001 H1-08 line back turns 6b red; one whose
//  _ProxyTry.cpp calls FormLock() turns 6d red.
// =============================================================================
#include "MachineType.h"      // SOFT_SIMULTE, Type_HT9046_LS, ChamberOnly, tcChamber
#include "cmydef.h"           // InitialOK, MachineTypeChoice, USE_AIR_CONDITIONER, bCCDOverTemp, bHALTing, UN150Read, CUSTOMER_CODE, PauseUT150Polling
#include "cprod.h"            // Temperature
#include "csystem.h"          // DoSwCoolingFan, W906_FormFShowHook
#include "bthermo.h"          // DoThermo
#include "LastSet.h"          // LastSet.iTemperature
#include "atester_shims.h"    // fiosetview
#include "myswitch.h"         // SW[]
#include "mycylin.h"          // Cylinder[]
#include "vclcompat/AnsiString.h"
#include "st02_test_containment.h"   // refuse to run outside ctest's redirect roots (a golden body may log)

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

extern int iThermoTask;                       // bthermo.cpp:1261
extern int TC401HeaterControl;                // cmydef.cpp:231
extern const int KT4H;                        // cmydef.cpp:227
bool FileRW_ProxyTryRead(const char*, const char*, bool*, const char*, int*);   // FileRW/_ProxyTry.cpp (compiled here)
unsigned long FileRW_ProxyTryBusyCount();                                        // same

// ---- the FormJson lock (JsonBridge/FormJson.cpp is a wb_serve source) ---------------------------------------------------------
namespace {
CRITICAL_SECTION g_cs;
struct CsInit { CsInit() { ::InitializeCriticalSection(&g_cs); } } g_csInit;
DWORD g_owner = 0;            // the thread that holds it, 0 = free (written only while holding it)
int   g_depth = 0;
}
namespace ht9045 { namespace formjson {
void FormLock()    { ::EnterCriticalSection(&g_cs); g_owner = ::GetCurrentThreadId(); ++g_depth; }
void FormUnlock()  { if (--g_depth == 0) g_owner = 0; ::LeaveCriticalSection(&g_cs); }
bool FormTryLock() { if (!::TryEnterCriticalSection(&g_cs)) return false; g_owner = ::GetCurrentThreadId(); ++g_depth; return true; }
} }

// ---- FileRW/_fallback.cpp's seven, so its member is never pulled in -----------------------------------------------------------
namespace {
bool g_chk = false;           // the TfConfiguration chkHeater proxy
int  g_page = -1;             // the TfConfiguration PageControl1 proxy's ActivePageIndex
int  g_reads = 0, g_readsUnderOwnLock = 0;
void NoteRead() { ++g_reads; if (g_owner == ::GetCurrentThreadId() && g_depth >= 2) ++g_readsUnderOwnLock; }   // 2 = the caller's try-acquisition + this accessor's own
}
bool FileRW_ProxyChecked(const char* form, const char* name)                     // as FileRW/_EditList.cpp: takes FormLock
{
    ht9045::formjson::FormLock();
    const bool v = std::strcmp(form, "TfConfiguration") == 0 && std::strcmp(name, "chkHeater") == 0 && g_chk;
    NoteRead();
    ht9045::formjson::FormUnlock();
    return v;
}
int FileRW_ProxyPageIndex(const char* form, const char* name)                     // as FileRW/_EditList.cpp: takes FormLock
{
    ht9045::formjson::FormLock();
    const int v = (std::strcmp(form, "TfConfiguration") == 0 && std::strcmp(name, "PageControl1") == 0) ? g_page : -1;
    NoteRead();
    ht9045::formjson::FormUnlock();
    return v;
}
void FileRW_IniConfig_ChangeCBListProperty() {}
bool FileRW_ProxySetChecked(const char*, const char*, bool) { return false; }
bool FileRW_ProxySetText(const char*, const char*, const char*) { return false; }
bool FileRW_ProxySetItemIndex(const char*, const char*, int) { return false; }
vclcompat::AnsiString FileRW_HSys_CustomerName() { return vclcompat::AnsiString("HonPrec"); }

// ---- harness --------------------------------------------------------------------------------------------------------------
static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, what); }
    else     std::printf("  ok: %s\n", what);
}
#define CHECK(c) check((c), #c, __LINE__)

static const char* g_openPage = 0;
static bool FakePageTable(const char* goldenForm) { return g_openPage != 0 && std::strcmp(goldenForm, g_openPage) == 0; }

static bool Blower() { return SW[SwCoolingFan_Blower].OutValue; }
static bool Valve()  { return Cylinder[C_CoolingValve].Status; }
static void Preset(bool blower, bool valve) { SW[SwCoolingFan_Blower].OutValue = blower; Cylinder[C_CoolingValve].Status = valve; }

static double MsSince(LARGE_INTEGER a)
{
    LARGE_INTEGER f, b; ::QueryPerformanceFrequency(&f); ::QueryPerformanceCounter(&b);
    return (double)(b.QuadPart - a.QuadPart) * 1000.0 / (double)f.QuadPart;
}

// the other thread: holds the lock until told to let go
static volatile LONG g_holderHas = 0, g_holderGo = 0;
static DWORD WINAPI Holder(LPVOID)
{
    ht9045::formjson::FormLock();
    ::InterlockedExchange(&g_holderHas, 1);
    const DWORD t0 = ::GetTickCount();
    while (::InterlockedCompareExchange(&g_holderGo, 0, 0) == 0 && ::GetTickCount() - t0 < 1500)
        ::Sleep(1);
    ht9045::formjson::FormUnlock();
    ::InterlockedExchange(&g_holderHas, 0);
    return 0;
}

// ---- source helpers (test_sysinit_boot.cpp pattern) ------------------------------------------------------------------------
static std::string ReadSource(const std::string& root, const char* rel)
{
    std::ifstream f((root + "/" + rel).c_str(), std::ios::binary);
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}
static std::string CodeOnly(const std::string& s)     // comments and literals blanked, newlines kept
{
    std::string o(s);
    enum { CODE, LINE, BLOCK, STR, CHR } st = CODE;
    for (size_t i = 0; i < o.size(); ++i) {
        const char c = s[i];
        const char n = (i + 1 < s.size()) ? s[i + 1] : '\0';
        switch (st) {
        case CODE:
            if (c == '/' && n == '/') { st = LINE; o[i] = ' '; }
            else if (c == '/' && n == '*') { st = BLOCK; o[i] = ' '; o[i + 1] = ' '; ++i; }
            else if (c == '"') { st = STR; }
            else if (c == '\'') { st = CHR; }
            break;
        case LINE:  if (c == '\n') st = CODE; else if (c != '\r') o[i] = ' '; break;
        case BLOCK: if (c == '*' && n == '/') { st = CODE; o[i] = ' '; o[i + 1] = ' '; ++i; } else if (c != '\n' && c != '\r') o[i] = ' '; break;
        case STR: case CHR:
            if (c == '\\' && i + 1 < s.size()) { o[i] = ' '; if (s[i + 1] != '\n') o[i + 1] = ' '; ++i; }
            else if ((st == STR && c == '"') || (st == CHR && c == '\'')) st = CODE;
            else if (c != '\n' && c != '\r') o[i] = ' ';
            break;
        }
    }
    return o;
}
static int CountOf(const std::string& hay, const std::string& needle)
{
    int n = 0;
    for (size_t p = hay.find(needle); p != std::string::npos; p = hay.find(needle, p + needle.size())) ++n;
    return n;
}
// the text of a function: from its head to the first line that is a lone "}" (the tree's column-0 closing brace)
static std::string Body(const std::string& code, const char* head)
{
    const size_t a = code.find(head);
    if (a == std::string::npos) return std::string();
    size_t b = code.find("\n}", a);
    return b == std::string::npos ? std::string() : code.substr(a, b - a + 2);
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
#ifdef SOFT_SIMULTE
    std::printf("FastClk_TryLock (SIM)\n");
#else
    std::printf("FastClk_TryLock (SHIP)\n");
#endif
    if (!W906TestInsideCtestRoots("FastClk_TryLock"))
        return 2;
    const int    sMachine = MachineTypeChoice, sAirCon = USE_AIR_CONDITIONER, sCust = CUSTOMER_CODE, sTask = iThermoTask, sCtrl = TC401HeaterControl;
    const int    sTemp = LastSet.iTemperature, sMode = Temperature.iIndexHeatMode;
    const double sCool = Temperature.fChamberCoolTemp, sChamber = UN150Read[tcChamber];
    const bool   sCCD = bCCDOverTemp, sHalt = bHALTing, sInit = InitialOK, sPause = PauseUT150Polling;
    const bool   sBlower = Blower(), sValve = Valve();
    bool (*const sHook)(const char*) = W906_FormFShowHook;
    CHECK(fiosetview != 0);
    if (fiosetview == 0) return 1;
    const bool sIoShow = fiosetview->fShow;

    if (MachineTypeChoice == Type_HT9046_LS) MachineTypeChoice = 0;
    USE_AIR_CONDITIONER = 0;  CUSTOMER_CODE = 0;  bCCDOverTemp = false;  bHALTing = false;  fiosetview->fShow = false;
    Temperature.fChamberCoolTemp = 35.0;
    LastSet.iTemperature = Tempture_Ambient;  Temperature.iIndexHeatMode = ChamberOnly;  UN150Read[tcChamber] = 40.0;   // normal rule: blower on, valve off
    TC401HeaterControl = KT4H;  PauseUT150Polling = false;
    W906_FormFShowHook = &FakePageTable;

    std::printf(" 1. lock free, on the heater tab -> golden H1-08 refuses\n");
    g_openPage = "fConfiguration";  g_chk = true;  g_page = 1;  g_reads = 0;  g_readsUnderOwnLock = 0;
    Preset(false, true);
    CHECK(DoSwCoolingFan(false) == false);
    CHECK(Blower() == false && Valve() == true);
    CHECK(g_reads == 2 && g_readsUnderOwnLock == 2);        // chkHeater + PageControl1, inside the one try-acquisition
    CHECK(g_depth == 0 && g_owner == 0);                    // released

    std::printf(" 2. lock free, chkHeater unchecked -> the normal rule\n");
    g_chk = false;
    Preset(false, true);
    CHECK(DoSwCoolingFan(false) == true);
    CHECK(Blower() == true && Valve() == false);

    std::printf(" 3. another thread holds FormLock -> this beat is skipped, nothing waits\n");
    g_chk = false;  g_page = 0;
    const unsigned long busy0 = FileRW_ProxyTryBusyCount();
    ::InterlockedExchange(&g_holderGo, 0);
    HANDLE th = ::CreateThread(NULL, 0, &Holder, NULL, 0, NULL);
    CHECK(th != NULL);
    for (int i = 0; i < 2000 && ::InterlockedCompareExchange(&g_holderHas, 0, 0) == 0; ++i) ::Sleep(1);
    CHECK(::InterlockedCompareExchange(&g_holderHas, 0, 0) == 1);
    {
        Preset(false, true);
        LARGE_INTEGER t0; ::QueryPerformanceCounter(&t0);
        const bool r = DoSwCoolingFan(false);
        const double ms = MsSince(t0);
        std::printf("     DoSwCoolingFan with the lock held elsewhere: %.3f ms\n", ms);
        CHECK(r == false);
        CHECK(ms < 100.0);
        CHECK(Blower() == false && Valve() == true);         // untouched (the free answer would be blower on, valve off)
    }
    {
        iThermoTask = 1;
        LARGE_INTEGER t0; ::QueryPerformanceCounter(&t0);
        DoThermo();
        const double ms = MsSince(t0);
        std::printf("     DoThermo with the lock held elsewhere: %.3f ms\n", ms);
        CHECK(ms < 100.0);
        CHECK(iThermoTask == 1);                              // DoThermoReal not entered (golden G14a's early return)
    }
    CHECK(FileRW_ProxyTryBusyCount() - busy0 == 2);

    std::printf(" 4. still held, Configuration closed -> no lock asked, golden goes on\n");
    g_openPage = 0;
    {
        Preset(false, true);
        LARGE_INTEGER t0; ::QueryPerformanceCounter(&t0);
        const bool r = DoSwCoolingFan(false);
        const double ms = MsSince(t0);
        CHECK(r == true && ms < 100.0);
        CHECK(Blower() == true && Valve() == false);
        iThermoTask = 1;
        DoThermo();
        CHECK(iThermoTask == 100);                            // DoThermoReal case 1 (golden bthermo.cpp case 1 -> Task=100)
        CHECK(FileRW_ProxyTryBusyCount() - busy0 == 2);       // nothing more was tried
    }
    ::InterlockedExchange(&g_holderGo, 1);
    if (th) { ::WaitForSingleObject(th, 5000); ::CloseHandle(th); }

    std::printf(" 5. released -> step-1 answers again\n");
    g_openPage = "fConfiguration";  g_chk = true;  g_page = 1;
    Preset(false, true);
    CHECK(DoSwCoolingFan(false) == false && Blower() == false && Valve() == true);
    iThermoTask = 1;
    DoThermo();
    CHECK(iThermoTask == 1);                                  // golden G14a: on tsTempComm -> return
    g_page = 0;
    iThermoTask = 1;
    DoThermo();
    CHECK(iThermoTask == 100);                                // another tab -> DoThermoReal
    CHECK(g_depth == 0 && g_owner == 0);

    std::printf(" 6. source pins\n");
    if (argc > 1) {
        const std::string root = argv[1];
        const std::string cs = CodeOnly(ReadSource(root, "csystem.cpp"));
        const std::string bt = CodeOnly(ReadSource(root, "bthermo.cpp"));
        const std::string pt = CodeOnly(ReadSource(root, "FileRW/_ProxyTry.cpp"));
        const std::string fj = CodeOnly(ReadSource(root, "JsonBridge/FormJson.cpp"));
        CHECK(!cs.empty() && !bt.empty() && !pt.empty() && !fj.empty());   // 6a
        // CodeOnly blanks string literals, so the needles are names: one block-scope declaration + one call each
        const std::string fan = Body(cs, "bool DoSwCoolingFan(bool OnOff)");
        CHECK(!fan.empty() && CountOf(fan, "FileRW_ProxyTryRead(") == 2
              && CountOf(fan, "FileRW_ProxyChecked(") == 0 && CountOf(fan, "FileRW_ProxyPageIndex(") == 0);   // 6b H1-08
        const std::string thermo = Body(bt, "void DoThermo()");
        CHECK(!thermo.empty() && CountOf(thermo, "FileRW_ProxyTryRead(") == 2
              && CountOf(thermo, "FileRW_ProxyPageIndex(") == 0);                                              // 6c G14a
        CHECK(CountOf(pt, "FormTryLock()") >= 2 && CountOf(pt, "FormLock()") == 0
              && CountOf(pt, "EnterCriticalSection") == 0);                                                    // 6d never blocks
        CHECK(CountOf(fj, "bool FormTryLock() { return ::TryEnterCriticalSection(&g_lock) != 0; }") == 1);      // 6e the one lock
    } else
        std::printf("  (skipped: no tree root given)\n");

    // restore
    W906_FormFShowHook = sHook;  g_openPage = 0;
    MachineTypeChoice = sMachine;  USE_AIR_CONDITIONER = sAirCon;  CUSTOMER_CODE = sCust;  iThermoTask = sTask;  TC401HeaterControl = sCtrl;
    LastSet.iTemperature = sTemp;  Temperature.iIndexHeatMode = sMode;  Temperature.fChamberCoolTemp = sCool;  UN150Read[tcChamber] = sChamber;
    bCCDOverTemp = sCCD;  bHALTing = sHalt;  InitialOK = sInit;  PauseUT150Polling = sPause;  fiosetview->fShow = sIoShow;
    Preset(sBlower, sValve);
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
