// =============================================================================
//  tests/test_fastclk_jobs.cpp -- AI(W906-FASTCLK) 20261003.  Suite: FastClk_Jobs (both configs).
//
//  The three jobs RULINGS_20261002 #7 (s0 #35 = A) puts on the serve loop's shared fast clock, and the wb_serve glue that
//  registers them (FastClockWbServe.cpp + FastClock.cpp + WebStreamStats.cpp compiled here; FastClockJobs.cpp and
//  MainTimersSt02.cpp come with ht9045_sm):
//    1. heater (golden THeaterThread, uHeaterThread.cpp:56-76): InitialOK false -> nothing.  SIM: the beat IS St02's
//       HeaterSimTick (fHeaterOK follows chkHeaterOk).  SHIP: golden's body -- with no heater controller (NoHeater, DoThermo
//       returns at its first line) at ambient, DoHeaterOn switches the relay off and the chamber fan on.
//    2. GM-2 (golden TfGroundMan::Timer1Timer GroundMan.cpp:551-575): InitialOK true -> labStatus = iGroundMasterTask;
//       false -> untouched; the facade's Timer1->Interval is golden's 30 (ctor :134).  DoGroundMasterMonitor stays GM-2 #if 0.
//    3. bin panel (golden TfMain::Timer1 NUMBER_PANEL_TYPE==2 segment, St02 E-T1-022): the fast entry has no deadline of its
//       own (the fast clock is the Interval) but keeps golden's guards -- the 20 ms dwOldTime rule (a call 15 ms after the
//       last one is dropped), type 2 only, the running flag.  With the fast clock owning it, St02's dispatcher skips it on the
//       PumpTick path and still runs it on the modal path (the fast clock is main-loop only).
//    4. the glue, end to end with real time (1.5 s of the serve loop's two calls): the jobs registered once each with their
//       periods (heater only when MachineType.h W906_FASTCLK_HEATER is defined), the two owner flags set, each job ran at
//       about its rate; prints "[timing]" (per-job avg / max body time in this build).
//    5. source pins (argv[1] = tree root, read only): the two serve-loop calls, the registrations, the MachineType.h switch,
//       the PumpTick and St02-dispatcher gates, the SHIP heater body = golden's five calls in order under if(InitialOK).
//       CONTROL (run 20261003): scratch roots with one hook removed each turn their 5x check red.
//  CONTAINED: refuses (exit 2) outside ctest's redirect roots (st02_test_containment.h).  Memory only.
// =============================================================================
#include "MachineType.h"
#include "cmydef.h"           // InitialOK, SystemInitialOK, NUMBER_PANEL_TYPE, bSystemClose, bSECSGEMAlarm, fHeaterOK, SystemStart
#include "cprod.h"
#include "csystem.h"
#include "LastSet.h"
#include "Config.h"           // IniConfig.bAmbRunChamberFanCanStop
#include "forms/fMain.h"      // fMain->chkHeaterOk
#include "forms/fGroundMan.h" // fGroundMan
#include "forms/fShowBinSelect.h"
#include "myswitch.h"
#include "st02_test_containment.h"

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

namespace ht9045 {
void W906_FastClockHeaterBeat();
void W906_FastClockGroundManBeat();
void W906_FastClockSetOwnsHeater(bool);
bool W906_FastClockOwnsHeater();
void W906_FastClockSetOwnsBin(bool);
bool W906_FastClockOwnsBin();
void W906_St02Timer1BinFastAt(unsigned long now);
void W906_St02TimersTickAt(unsigned long now);
void W906_St02TimersTickModalAt(unsigned long now);
void W906_St02TimersReset();
void W906_St02TimersCountsT1(unsigned long* t1);
}
void W906_FastClockPassBegin();                                                   // FastClockWbServe.cpp
unsigned long W906_FastClockClampNext(unsigned long nextTick, unsigned long nowTick);
bool W906_FastClockJob(const char* name, unsigned* periodMs, unsigned long long* runs, unsigned long long* busyUs, unsigned long long* maxUs);
extern int TC401HeaterControl;
extern const int NoHeater;

// FileRW/_ProxyTry.cpp is a wb_serve source; the glue reports its counter -- 0 here (no FileRW, nothing to skip)
unsigned long FileRW_ProxyTryBusyCount() { return 0; }

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, what); }
    else     std::printf("  ok: %s\n", what);
}
#define CHECK(c) check((c), #c, __LINE__)

static unsigned long BinCalls() { unsigned long n = 0; ht9045::W906_St02TimersCountsT1(&n); return n; }
static void BinBase()
{
    ht9045::W906_St02TimersReset();
    NUMBER_PANEL_TYPE = 2;  InitialOK = true;  SystemInitialOK = true;
    bSECSGEMAlarm = false;  bSECSGEM_NoteAlarm = false;  bSystemClose = false;
}

// ---- source helpers ------------------------------------------------------------------------------------------------------
static std::string ReadSource(const std::string& root, const char* rel)
{
    std::ifstream f((root + "/" + rel).c_str(), std::ios::binary);
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}
static std::string CodeOnly(const std::string& s)     // comments and literals blanked, newlines kept (test_sysinit_boot.cpp)
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
static std::string LineWith(const std::string& s, const std::string& needle)   // the whole line holding the first hit
{
    const size_t p = s.find(needle);
    if (p == std::string::npos) return std::string();
    const size_t a = s.rfind('\n', p);
    const size_t b = s.find('\n', p);
    return s.substr(a == std::string::npos ? 0 : a + 1, (b == std::string::npos ? s.size() : b) - (a == std::string::npos ? 0 : a + 1));
}
static size_t Col(const std::string& line, const std::string& needle) { return line.find(needle); }

int main(int argc, char** argv)
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
#ifdef SOFT_SIMULTE
    const char* const build = "SIM";
#else
    const char* const build = "SHIP";
#endif
    std::printf("FastClk_Jobs (%s)\n", build);
    if (!W906TestInsideCtestRoots("FastClk_Jobs"))
        return 2;
    CHECK(fMain != 0 && fMain->chkHeaterOk != 0 && fGroundMan != 0 && fShowBinSelect != 0);
    if (fMain == 0 || fMain->chkHeaterOk == 0 || fGroundMan == 0 || fShowBinSelect == 0) return 1;
    const bool sInit = InitialOK, sSysInit = SystemInitialOK, sStart = SystemStart, sClose = bSystemClose;
    const bool sOK = fHeaterOK, sChk = fMain->chkHeaterOk->Checked, sRelay = SW[SwHeaterRelay].OutValue, sFan = SW[SwHeaterFan].OutValue;
    const int  sTemp = LastSet.iTemperature, sCtrl = TC401HeaterControl, sType = NUMBER_PANEL_TYPE, sTask = fGroundMan->iGroundMasterTask;
    const bool sFanStop = IniConfig.bAmbRunChamberFanCanStop;

    std::printf(" 1. heater beat (golden THeaterThread::HeaterThreadProcess)\n");
    {
        SystemStart = false;  LastSet.iTemperature = Tempture_Ambient;  TC401HeaterControl = NoHeater;  IniConfig.bAmbRunChamberFanCanStop = false;
        fMain->chkHeaterOk->Checked = true;
        InitialOK = false;
        fHeaterOK = false;  SW[SwHeaterRelay].OutValue = true;  SW[SwHeaterFan].OutValue = false;
        ht9045::W906_FastClockHeaterBeat();
        CHECK(fHeaterOK == false && SW[SwHeaterRelay].OutValue == true && SW[SwHeaterFan].OutValue == false);   // golden :58
        InitialOK = true;
        ht9045::W906_FastClockHeaterBeat();
#ifdef SOFT_SIMULTE
        CHECK(fHeaterOK == true);                                                // HeaterSimTick's CheckHeater SIM branch (golden :138-144)
#else
        CHECK(SW[SwHeaterRelay].OutValue == false && SW[SwHeaterFan].OutValue == true);   // DoHeaterOn at ambient (golden csystem.cpp:1134 ff.)
#endif
    }

    std::printf(" 2. GM-2 beat (golden TfGroundMan::Timer1Timer)\n");
    {
        CHECK(fGroundMan->Timer1->Interval == 30);                               // golden GroundMan.cpp:134 / GroundMan.dfm:1877
        InitialOK = true;
        fGroundMan->iGroundMasterTask = 7;
        fGroundMan->labStatus->Caption = "x";
        ht9045::W906_FastClockGroundManBeat();
        CHECK(fGroundMan->labStatus->Caption == "7");                             // golden :572
        InitialOK = false;
        fGroundMan->labStatus->Caption = "x";
        ht9045::W906_FastClockGroundManBeat();
        CHECK(fGroundMan->labStatus->Caption == "x");                             // golden :555-559
        fGroundMan->iGroundMasterTask = sTask;
    }

    std::printf(" 3. bin panel fast entry + St02's dispatcher\n");
    {
        const unsigned long t0 = 700000;
        BinBase();
        ht9045::W906_St02Timer1BinFastAt(t0);
        CHECK(BinCalls() == 1);                                                  // no deadline of its own: runs at once
        ht9045::W906_St02Timer1BinFastAt(t0 + 15);
        CHECK(BinCalls() == 1);                                                  // golden :3163-3167 (now - dwOldTime < 20)
        ht9045::W906_St02Timer1BinFastAt(t0 + 30);
        CHECK(BinCalls() == 2);
        NUMBER_PANEL_TYPE = 0;
        ht9045::W906_St02Timer1BinFastAt(t0 + 90);
        CHECK(BinCalls() == 2);                                                  // :3439 type 2 only
        BinBase();
        ht9045::W906_FastClockSetOwnsBin(true);
        for (unsigned long t = t0; t <= t0 + 300; t += 10) ht9045::W906_St02TimersTickAt(t);
        CHECK(BinCalls() == 0);                                                  // the PumpTick path leaves it to the fast clock
        for (unsigned long t = t0 + 300; t <= t0 + 600; t += 10) ht9045::W906_St02TimersTickModalAt(t);
        CHECK(BinCalls() >= 8);                                                  // the modal path still runs it (30 ms deadline)
        ht9045::W906_FastClockSetOwnsBin(false);
        BinBase();
        for (unsigned long t = t0; t <= t0 + 300; t += 10) ht9045::W906_St02TimersTickAt(t);
        CHECK(BinCalls() >= 8);                                                  // not owned: St02's option A unchanged
        NUMBER_PANEL_TYPE = sType;
    }

    std::printf(" 4. the glue, end to end (1.5 s of real serve-loop calls)\n");
    {
#ifdef SOFT_SIMULTE
        InitialOK = true;                                                        // SIM bodies are memory only
#else
        InitialOK = false;                                                       // SHIP: the golden heater body stays inert here
#endif
        NUMBER_PANEL_TYPE = 0;
        const DWORD start = ::GetTickCount();
        unsigned long passes = 0;
        while (::GetTickCount() - start < 1500) {
            const DWORD n0 = ::GetTickCount();
            const unsigned long next = W906_FastClockClampNext(n0 + 50, n0);    // the loop's own 50 ms cap
            const long wait = (long)(next - n0);
            ::Sleep(wait > 0 ? (DWORD)wait : 1);
            W906_FastClockPassBegin();
            ++passes;
        }
        unsigned p = 0; unsigned long long runs = 0, busy = 0, mx = 0;
        const bool hasHeater = W906_FastClockJob("heater", &p, &runs, &busy, &mx);
#ifdef W906_FASTCLK_HEATER
        CHECK(hasHeater && p == 20 && ht9045::W906_FastClockOwnsHeater());
        CHECK(runs >= 20 && runs <= 76);                                         // 1.5 s / 20 ms = 75; coarse timers wake late, never early
        std::printf("  [timing] heater (%s body): %lu runs, avg %.3f ms, max %.3f ms\n", build, (unsigned long)runs, runs ? busy / 1000.0 / runs : 0.0, mx / 1000.0);
#else
        CHECK(!hasHeater && !ht9045::W906_FastClockOwnsHeater());
#endif
        CHECK(W906_FastClockJob("binpanel", &p, &runs, &busy, &mx) && p == 30 && ht9045::W906_FastClockOwnsBin());
        CHECK(runs >= 14 && runs <= 51);
        std::printf("  [timing] binpanel: %lu runs, avg %.3f ms, max %.3f ms\n", (unsigned long)runs, runs ? busy / 1000.0 / runs : 0.0, mx / 1000.0);
        CHECK(W906_FastClockJob("groundman", &p, &runs, &busy, &mx) && p == 30);
        CHECK(runs >= 14 && runs <= 51);
        std::printf("  [timing] groundman: %lu runs, avg %.3f ms, max %.3f ms; %lu serve-loop passes\n", (unsigned long)runs, runs ? busy / 1000.0 / runs : 0.0, mx / 1000.0, passes);
        CHECK(!W906_FastClockJob("nosuch", &p, &runs, &busy, &mx));
        ht9045::W906_FastClockSetOwnsHeater(false);  ht9045::W906_FastClockSetOwnsBin(false);
    }

    std::printf(" 5. source pins\n");
    if (argc > 1) {
        const std::string root = argv[1];
        const std::string ws = ReadSource(root, "tools/wb_serve.cpp"), wsc = CodeOnly(ws);
        const std::string gl = CodeOnly(ReadSource(root, "FastClockWbServe.cpp"));
        const std::string mt = CodeOnly(ReadSource(root, "MachineType.h"));
        const std::string bt = CodeOnly(ReadSource(root, "WebBridgeTags.cpp"));
        const std::string st = CodeOnly(ReadSource(root, "MainTimersSt02.cpp"));
        const std::string jb = CodeOnly(ReadSource(root, "FastClockJobs.cpp"));
        CHECK(!ws.empty() && !gl.empty() && !mt.empty() && !bt.empty() && !st.empty() && !jb.empty());   // 5a
        // 5b the serve loop: PassBegin once, first on the line that pumps the native forms right after the sleep
        const std::string pb = LineWith(wsc, "W906_FastClockPassBegin();");
        CHECK(CountOf(wsc, "W906_FastClockPassBegin();") == 2                                   // the extern + the call
              && Col(pb, "W906_FastClockPassBegin();") < Col(pb, "W906_NativeFormsPump(0);"));
        // 5c the sleep: ClampNext once, on the n0 line, before `wait` is computed
        const std::string cn = LineWith(wsc, "next = W906_FastClockClampNext(next, n0);");
        CHECK(CountOf(wsc, "W906_FastClockClampNext(") == 2 && Col(cn, "const DWORD n0 = ::GetTickCount();") != std::string::npos);
        // 5d the registrations (CodeOnly blanks the names, so count the bodies)
        CHECK(CountOf(gl, "&ht9045::W906_FastClockHeaterBeat)") == 1 && CountOf(gl, "&ht9045::W906_St02Timer1BinFast)") == 1
              && CountOf(gl, "&ht9045::W906_FastClockGroundManBeat)") == 1);
        CHECK(CountOf(gl, "#ifdef W906_FASTCLK_HEATER") == 1);
        // 5e the switch, live (not commented)
        CHECK(CountOf(mt, "#define W906_FASTCLK_HEATER") == 1);
        // 5f PumpTick: HeaterSimTick only while the fast clock does not own the heater
        CHECK(CountOf(bt, "if (!W906_FastClockOwnsHeater()) W906_HeaterSimTick();") == 1 && CountOf(bt, "W906_HeaterSimTick();") == 2);
        // 5g St02's dispatcher: the PumpTick path skips Timer1BinTick while owned, the modal path runs it
        CHECK(CountOf(st, "if (!W906_FastClockOwnsBin() || g_modalDepth > 0) Timer1BinTick(now);") == 1);
        // 5h the SHIP heater body = golden uHeaterThread.cpp:58-65, in order, under if(InitialOK)
        const size_t g = jb.find("if(InitialOK)");
        const size_t a = jb.find("CheckATC6System();", g), b = jb.find("DoThermo();", g), c = jb.find("HeaterDoorIsOpen();", g),
                     d = jb.find("CheckHeater();", g), e = jb.find("DoHeaterOn();", g);
        CHECK(g != std::string::npos && g < a && a < b && b < c && c < d && d < e && e != std::string::npos);
    } else
        std::printf("  (skipped: no tree root given)\n");

    InitialOK = sInit;  SystemInitialOK = sSysInit;  SystemStart = sStart;  bSystemClose = sClose;
    fHeaterOK = sOK;  fMain->chkHeaterOk->Checked = sChk;  SW[SwHeaterRelay].OutValue = sRelay;  SW[SwHeaterFan].OutValue = sFan;
    LastSet.iTemperature = sTemp;  TC401HeaterControl = sCtrl;  NUMBER_PANEL_TYPE = sType;  IniConfig.bAmbRunChamberFanCanStop = sFanStop;
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
