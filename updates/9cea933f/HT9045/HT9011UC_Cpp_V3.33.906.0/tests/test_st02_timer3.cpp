// =============================================================================
//  tests/test_st02_timer3.cpp -- S-14: golden TfMain::Timer3Timer (906_0625_Steven main.cpp:25184-25820) = MainTimer3.cpp, and
//  the dispatcher's Timer3 slot (MainTimersSt02.cpp: latched on by InitialOK, 1000 ms, no re-entry guard -- golden has no
//  running flag (NB2 R126 M4) --, close guard).  AI(W906-ST02-MRB) 20261004 (!70 MR-B): Timer3 is now fMain->Timer3 in the timer table
//  (enabled by the bind = golden FormShow :10180; the table skips it inside its own box, Q85 = A -- sections 1 / 11 / 12).
//  AI(W906-S14) 20261001 (St02-E).  Suite: St02_Timer3.  Built and run in both configurations.
//
//  CONTAINMENT FIRST (st02_test_containment.h): RecordProcess / NewRecordProcess write through ctest's redirect roots only;
//  asYieldRecordPath and the temperature log roots point at a %TEMP% sandbox before anything runs.  The 8 yield alarms run
//  on a counting stand-in (TfYieldMonitoring's Check* are virtual, forms/fYieldMonitoring.h:286-299) until section 13.
//  Timer3's own statics are one-shot (G2 once, G3 at the first call, G15's 15-call phase), so the sections run in this order:
//    1. InitialOK false -> the handler does nothing (golden :25194); the dispatcher does not start the slot while InitialOK is
//       false, latches it on the first pass with InitialOK true, first call 1000 ms later, then every 1000 ms.
//       G3 (golden: SHIP only): the first call with FIX3_FULL_PLACE == Fix3K_ShortShuttle and the sensor disabled -> one
//       "Please enable the sensor 'SnFix3FullPlace'." (SHIP), none in SIM; the second call -> no message (10 s delay).
//    2. G2: the 2nd call writes exactly one MES2108 "Program Start" ("CCD Enabled, " + version, [C02] + CCD-TCPIP on) and the
//       FTP record "<HandlerID> is connect with <type>-<name>"; never again.
//    3. G15 TTL Clear lines: until iOpenNewTTLBoardStratDelayCount reaches 15, SwClear2 / SwClear6 are forced Off every call;
//       then bChangeTTLFlag = LastSet.bUseNewTTLBoard once; then Clear2 / 6 On and Clear1 / 5 follow [AntiSignal].
//    4. G16: the 8 yield alarms once per call, in golden's order.   G4: the main grid vacuum cells.   G7: NUMBER_PANEL_TYPE 1
//       -> DoShowBinDigital runs (SwLoaderBin+0..11 On, task 100); type 2 -> not here (golden Timer1); type 3 -> its branch.
//    5. G10 [E73]: 59 calls nothing, the 60th -> "Input arm Z..." / "Output arm Z..." per counter, the three counters 0.
//    6. G13 [A37]: the three messages by iCurrent93KARTStep / timers / bSpecTrayCnt / run mode / [A68], each once per arm.
//    7. G18: the status bar text "....-..-.. hh:00:00" -> TimerRecordLoaderDate once; any other second / minute -> not;
//       the real text has golden's sAlarmTime shape.
//    8. G20 [RMS]: running -> the Lot page fields disabled; RMS off -> untouched.
//    9. G26: 1 -> WAR1684, 2 -> WAR16500 (K_SKIP, MMSystem, errPart = asChangeSetupFileName), back to 0; Note box up -> waits.
//   10. the gated parts with their flags on: KYEC barcode (golden's else runs: cleared), JCET (the hourly line only), UNISEM,
//       stop-time label, direction images, Renesas FT-CT (no message), CCD temperature over 55 (MNetLog, a no-op).
//   11. the dispatcher: a box inside Timer3 (G26) -> the modal-wait tick calls Timer3 again on its deadline, as golden VCL
//       re-enters Timer3Timer (no running flag, :25184-25195; AI(W906-R126) 20261002, NB2 R126 M4); G26 cleared its flag
//       before the box, so the nested call shows nothing; Timer8 fires inside it too; bSystemClose -> Timer3 does not fire.
//   12. the hook-up lines are code, not comment (argv[1] = the tree root; ctest passes it): MainTimersSt02.cpp calls
//       W906_Timer3Timer once, CMakeLists.txt lists MainTimer3.cpp after MainTimerESDFallback.cpp, the G25 lines are comment.
//   13. the real TfYieldMonitoring back in place: three calls, no crash (ctest defaults: nothing to alarm on).
//   14. G4 RecordTemp (golden main.cpp:2658-2689; AI(W906-R126) 20261002, NB2 R126 M5): on a SystemSec that is a multiple of
//       the [L10] interval, every installed channel's fObserver->dTempHistroy row shifts left and UN150Read lands in [59];
//       other seconds, the same second again and channels not installed leave it alone.  Memory only.
//   15. (AI(W906-ST02-0128) 20261002) the same with the Observer page shown: RecordTemp -> UpdateTempChart on the
//       static-init chart must not throw and must have its series (machine patch 0128 OBS-TEMPSERIES f09c5208).
// =============================================================================
#include "MachineType.h"
#include "cmydef.h"
#include "Config.h"
#include "CosFunction.h"
#include "cprod.h"
#include "LastSet.h"
#include "mysensor.h"
#include "myswitch.h"
#include "myTimer.h"
#include "aHotPlateSubstrate.h"
#include "canary_support.h"
#include "common.h"
#include "cpublic.h"           // GetTimeInfo
#include "csystem.h"           // hLotStartTimeOut / hLotEndTimeOut
#include "forms/fMain.h"
#include "forms/fNote.h"
#include "forms/fLotInfo.h"
#include "forms/fSCKART.h"
#include "forms/fShowBinSelect.h"
#include "forms/fYieldMonitoring.h"
#include "forms/fObserver.h"      // fObserver->dTempHistroy (section 14)   AI(W906-R126) 20261002
#include "st02_test_containment.h"

#include <windows.h>
#include <cstdio>
#include <string>
#include <vector>

extern AnsiString ExString;                  // cMyDB.cpp (cMyDB.h:143; not included: it redeclares canary_support.h's RecordProcess default)

namespace ht9045 {
void W906_Timer3Timer();
void W906_Timer3SetRecordLoaderDate(int (*fn)());
void W906_Timer3SetStatusBarTime(AnsiString (*fn)());
void W906_Timer3SetNewRecordProcess(void (*fn)(AnsiString, AnsiString, AnsiString));
AnsiString W906_Timer3StatusBarTimeNow();
void W906_St02TimersTickAt(unsigned long now);
void W906_St02TimersReset();
void W906_St02TimersCounts(unsigned long* t8, unsigned long* ts);
void W906_St02TimersCountsESD(unsigned long* esd, unsigned long* reentries);
void W906_St02TimersCountsT3(unsigned long* t3, bool* on);
}
void W906_St02TimersTickFromModal();

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, what); }
    else     std::printf("  ok: %s\n", what);
}
#define CHECK(c) check((c), #c, __LINE__)

// ---- seams ----
static int g_loaderDateCalls = 0;
static int FakeLoaderDate() { ++g_loaderDateCalls; return 0; }
static std::string g_bar = "2026-10-01 13:30:15";
static AnsiString FakeBar() { return AnsiString(g_bar.c_str()); }
struct Rec { std::string code, s, debug; };
static std::vector<Rec> g_recs;
static void FakeNewRecord(AnsiString code, AnsiString s, AnsiString debug)
{
    Rec r;
    r.code = code.c_str(); r.s = s.c_str(); r.debug = debug.c_str();
    g_recs.push_back(r);
}

// ---- the operator (W906_ShowErrorMessage_Hook) ----
static int g_lastPos = -1;
static unsigned long g_nestAt = 0;     // non-zero: the box runs the dispatcher once at this time (the modal-wait tick)
static unsigned long g_n8InBox = 0, g_reInBox = 0, g_n3InBox = 0;
static int Operator(const char* code, int kcode, int pos)
{
    (void)code; (void)kcode;
    g_lastPos = pos;
    if (g_nestAt)
    {
        const unsigned long n = g_nestAt;
        g_nestAt = 0;
        ht9045::W906_St02TimersTickAt(n);
        ht9045::W906_St02TimersCounts(&g_n8InBox, 0);
        ht9045::W906_St02TimersCountsESD(0, &g_reInBox);
        ht9045::W906_St02TimersCountsT3(&g_n3InBox, 0);
    }
    return K_SKIP;
}

// ---- the 8 yield alarms, counted (golden :25598-25605 order = 1..8) ----
static std::vector<int> g_ym;
class CountingYM final : public TfYieldMonitoring
{
public:
    void CheckBySiteYieldAlarm() override             { g_ym.push_back(1); }
    void CheckBySiteByArmYieldAlarm() override        { g_ym.push_back(2); }
    void CheckLowYieldAlarm() override                { g_ym.push_back(3); }
    void CheckLowYieldAlarmByTotal() override         { g_ym.push_back(4); }
    void CheckIntervalLowYieldAlarmBySite() override  { g_ym.push_back(5); }
    void CheckIntervalLowYieldAlarmByTotal() override { g_ym.push_back(6); }
    void CheckLowYieldAlarmSpecial() override         { g_ym.push_back(7); }
    void CheckByPickerYieldAlarm() override           { g_ym.push_back(8); }
};

static std::string S1() { return std::string(W906_ShowMyMessage_LastS1.c_str()); }
static bool Has(const std::string& s, const char* needle) { return s.find(needle) != std::string::npos; }
static void T3() { ht9045::W906_Timer3Timer(); }

static std::string ReadAll(const std::string& p)
{
    std::string s;
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return s;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) s.append(buf, n);
    std::fclose(f);
    return s;
}

// the code part of one source line: // and /* */ comments removed, string literals kept (as test_st02_timer_esd.cpp)
static std::string CodeOfLine(const std::string& l)
{
    std::string out;
    bool inStr = false, inBlock = false;
    for (size_t i = 0; i < l.size(); ++i)
    {
        const char c = l[i];
        const char n = i + 1 < l.size() ? l[i + 1] : '\0';
        if (inBlock) { if (c == '*' && n == '/') { inBlock = false; ++i; } continue; }
        if (inStr) { out += c; if (c == '\\' && n) { out += n; ++i; } else if (c == '"') inStr = false; continue; }
        if (c == '"') { inStr = true; out += c; continue; }
        if (c == '/' && n == '/') break;
        if (c == '/' && n == '*') { inBlock = true; ++i; continue; }
        out += c;
    }
    return out;
}

static std::vector<std::string> Lines(const std::string& text)   // CRLF or LF
{
    std::vector<std::string> v;
    size_t p = 0;
    while (p <= text.size())
    {
        size_t e = text.find('\n', p);
        if (e == std::string::npos) e = text.size();
        std::string l = text.substr(p, e - p);
        if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1);
        v.push_back(l);
        p = e + 1;
    }
    return v;
}

static int CodeCount(const std::string& text, const std::string& needle)   // lines whose code part has needle (#if 0 ... #endif skipped)
{
    int n = 0;
    bool gated = false;
    const std::vector<std::string> v = Lines(text);
    for (size_t i = 0; i < v.size(); ++i)
    {
        const std::string& l = v[i];
        if (l.compare(0, 5, "#if 0") == 0) { gated = true; continue; }
        if (gated) { if (l.compare(0, 6, "#endif") == 0) gated = false; continue; }
        if (CodeOfLine(l).find(needle) != std::string::npos) ++n;
    }
    return n;
}

static std::string LineWith(const std::string& text, const std::string& needle)
{
    const std::vector<std::string> v = Lines(text);
    for (size_t i = 0; i < v.size(); ++i)
        if (v[i].find(needle) != std::string::npos) return v[i];
    return std::string();
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("St02_Timer3\n");
    if (!W906TestInsideCtestRoots("St02_Timer3"))
        return 2;
    char tmp[MAX_PATH];
    ::GetTempPathA(MAX_PATH, tmp);
    char sb[MAX_PATH];
    std::snprintf(sb, sizeof(sb), "%sht9045_st02t3_%lu", tmp, (unsigned long)::GetTickCount());
    const std::string sandbox = sb;
    ::CreateDirectoryA(sandbox.c_str(), 0);
    asYieldRecordPath = (sandbox + "\\Yield").c_str();
    asHiSiLogPath     = (sandbox + "\\Temperature").c_str();
    asLbTempLogPath   = (sandbox + "\\LB_Temperature").c_str();
    LB_TEMP_UpDown    = false;
#ifdef SOFT_SIMULTE
    std::printf("  build: SIM (SOFT_SIMULTE defined)\n");
#else
    std::printf("  build: SHIP (SOFT_SIMULTE not defined)\n");
#endif

    GetTimeInfo();
    // the other St02 timers stay quiet (the dispatcher runs them too)
    IniConfig.bP29LoaderCheckIsFull   = false;
    IniConfig.bRecordJamRateByTime    = false;
    IniConfig.bL10IndexTestlogTemp    = true;
    IniConfig.bVTESTFunction          = false;
    IniConfig.bI29EnableYieldRecord   = false;
    IniConfig.bO06SaveLogTimePeriod   = false;
    IniConfig.bSPILFunction           = false;
    // Timer3's own options, all off unless a section turns one on
    IniConfig.bE73_InOutZStepMotorLossCheck = false;
    IniConfig.bA37LotStartLotEnd      = false;
    IniConfig.bEnableRms              = false;
    IniConfig.bShowTrayAndDeviceDir   = false;
    IniConfig.bA11BarcodeTime         = false;
    TestIF_File.bRENESAS_EnableFTCT   = false;
    CosFunction.bShowHandlerStopTime  = false;
    NUMBER_PANEL_TYPE = 0;
    dCCDTemperature = 0.0;
    iChangeFileHasErr = 0;
    SystemStart = false;
    const int ccSaved = CUSTOMER_CODE;
    CHECK(fMain != 0 && fNote != 0 && fLotInfo != 0 && fSCKART != 0 && fShowBinSelect != 0 && fYieldMonitoring != 0);
    if (fMain == 0 || fNote == 0 || fLotInfo == 0 || fSCKART == 0 || fShowBinSelect == 0 || fYieldMonitoring == 0) return 1;
    fNote->fShow = false;
    fShowBinSelect->bUpdateBinDigital = false;
    W906_ShowErrorMessage_Reset();
    W906_ShowErrorMessage_Hook = &Operator;
    ht9045::W906_Timer3SetRecordLoaderDate(&FakeLoaderDate);
    ht9045::W906_Timer3SetStatusBarTime(&FakeBar);
    ht9045::W906_Timer3SetNewRecordProcess(&FakeNewRecord);
    TfYieldMonitoring* const realYM = fYieldMonitoring;
    CountingYM* const counting = new CountingYM();       // heap: the form's arrays are not small
    fYieldMonitoring = counting;

    // G2 (section 2) runs at the 2nd call, G3 (section 1) at the 1st: set them up before any call
    CosFunction.bFTPFunction = true;
    IniConfig.bEnableFTP = true;
    IniConfig.SocketHandlerID = "H01";
    IniConfig.TasterType = "T93K";
    IniConfig.TasterName = "N1";
    AccessLevel = 0;
    IniConfig.bEnableCCDUSETCPIP = true;
    IniConfig.bC02InstallCCD = true;
    FIX3_FULL_PLACE = Fix3K_ShortShuttle;
    Sen[SnFix3FullPlace].Enable = false;
    iOpenNewTTLBoardStratDelayCount = 0;
    LastSet.bUseNewTTLBoard = false;

    std::printf(" 1. InitialOK, the slot, G3\n");
    {
        InitialOK = false;
        for (int i = 0; i < 3; ++i) T3();
        CHECK(g_ym.empty() && g_recs.empty() && iOpenNewTTLBoardStratDelayCount == 0);   // golden :25194 return
        const unsigned long t0 = 300000;
        unsigned long n3 = 0;
        bool on = true;
        ht9045::W906_St02TimersReset();
        for (unsigned long t = 0; t <= 3000; t += 500)
            ht9045::W906_St02TimersTickAt(t0 + t);
        ht9045::W906_St02TimersCountsT3(&n3, &on);
        CHECK(n3 == 0 && on == true && g_ym.empty());     // AI(W906-ST02-MRB) 20261004: on from the bind (golden FormShow :10180); its calls at +1000..+3000 returned at once (:25194)
        InitialOK = true;
        const int m0 = W906_ShowMyMessage_Count;
        ht9045::W906_St02TimersTickAt(t0 + 3500);         // between two grid points (the grid runs from the bind, +1000 each)
        ht9045::W906_St02TimersCountsT3(&n3, &on);
        CHECK(n3 == 0 && on == true);
        ht9045::W906_St02TimersTickAt(t0 + 4000);         // Timer3 call 1
        ht9045::W906_St02TimersCountsT3(&n3, 0);
        CHECK(n3 == 1 && g_ym.size() == 8);
#ifdef SOFT_SIMULTE
        CHECK(W906_ShowMyMessage_Count == m0);            // golden #ifndef SOFT_SIMULTE (:25273)
#else
        CHECK(W906_ShowMyMessage_Count == m0 + 1 && S1() == "Please enable the sensor 'SnFix3FullPlace'.");
#endif
        ht9045::W906_St02TimersTickAt(t0 + 4500);
        ht9045::W906_St02TimersTickAt(t0 + 5000);         // Timer3 call 2
        ht9045::W906_St02TimersCountsT3(&n3, 0);
        CHECK(n3 == 2 && g_ym.size() == 16);
#ifdef SOFT_SIMULTE
        CHECK(W906_ShowMyMessage_Count == m0);
#else
        CHECK(W906_ShowMyMessage_Count == m0 + 1);        // no second Fix3 message (10 s delay)
#endif
        FIX3_FULL_PLACE = 0;
    }

    std::printf(" 2. G2: one MES2108 at the 2nd call\n");
    {
        CHECK(g_recs.size() == 1);
        if (!g_recs.empty())
        {
            const std::string want = std::string("CCD Enabled, ") + asHandlerVersion.c_str() + "." + SVNRevision.c_str();
            std::printf("    MES2108 debug text: %s\n", g_recs[0].debug.c_str());
            CHECK(g_recs[0].code == "MES2108" && g_recs[0].s == "Program Start" && g_recs[0].debug == want);
        }
        CHECK(std::string(ExString.c_str()) == "H01 is connect with T93K-N1");
        for (int i = 0; i < 3; ++i) T3();
        CHECK(g_recs.size() == 1);                        // bOnceLangFlag: once
        CosFunction.bFTPFunction = false;
        IniConfig.bEnableFTP = false;
        IniConfig.bEnableCCDUSETCPIP = false;
        IniConfig.bC02InstallCCD = false;
    }

    std::printf(" 3. G15 TTL Clear lines\n");
    {
        CHECK(iOpenNewTTLBoardStratDelayCount == 5);      // 2 dispatcher calls + 3 calls
        bool forcedOff = true;
        int k = 0;
        while (iOpenNewTTLBoardStratDelayCount < 15 && k < 40)
        {
            SW[SwClear2].On();
            SW[SwClear6].On();
            T3();
            forcedOff = forcedOff && SW[SwClear2].OutValue == false && SW[SwClear6].OutValue == false;
            ++k;
        }
        CHECK(k == 10 && forcedOff && bChangeTTLFlag == true);   // bChangeTTLFlag untouched until the phase ends
        SW[SwClear2].On();
        T3();                                             // count 15: the phase ends, bChangeTTLFlag = bUseNewTTLBoard (false)
        CHECK(SW[SwClear2].OutValue == false && bChangeTTLFlag == false && iOpenNewTTLBoardStratDelayCount == 15);
        SW[SwClear2].Off(); SW[SwClear6].Off(); SW[SwClear1].On(); SW[SwClear5].On();
        T3();                                             // bChangeTTLFlag false: nothing written
        CHECK(SW[SwClear2].OutValue == false && SW[SwClear6].OutValue == false && SW[SwClear1].OutValue == true && SW[SwClear5].OutValue == true);
        bChangeTTLFlag = true;
        TestIF_File.bAntiSignal = true;
        SW[SwClear1].Off(); SW[SwClear5].Off();
        T3();
        CHECK(SW[SwClear2].OutValue && SW[SwClear6].OutValue && SW[SwClear1].OutValue && SW[SwClear5].OutValue);
        TestIF_File.bAntiSignal = false;
        T3();
        CHECK(SW[SwClear2].OutValue && SW[SwClear6].OutValue && !SW[SwClear1].OutValue && !SW[SwClear5].OutValue);
        bChangeTTLFlag = false;
    }

    std::printf(" 4. G16 order, G4 grid, G7 bin panels\n");
    {
        g_ym.clear();
        T3();
        const int want[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
        CHECK(g_ym.size() == 8 && std::vector<int>(want, want + 8) == g_ym);

        InArmSuck.Suck[1][2].VacuumOnTime = 321;
        CatchTraySuck.Suck[0][0].VacuumOffTime = 77;
        LastSet.iUnLoaderTraySimulateTime[2] = 9;
        T3();
        CHECK(std::string(fMain->StringGrid2->Cells[6][3].c_str()) == "321");
        CHECK(std::string(fMain->StringGrid2->Cells[4][21].c_str()) == "77");
        CHECK(std::string(fMain->StringGrid2->Cells[3][27].c_str()) == "9");

        NUMBER_PANEL_TYPE = 1;
        fShowBinSelect->bUpdateBinDigital = true;
        fShowBinSelect->iShowBinDigitalTask = 1;
        for (int i = 0; i < 12; ++i) SW[SwLoaderBin + i].Off();
        T3();
        bool allOn = true;
        for (int i = 0; i < 12; ++i) allOn = allOn && SW[SwLoaderBin + i].OutValue;
        CHECK(allOn && fShowBinSelect->iShowBinDigitalTask == 100);   // type 1: Timer3 drives it (case 1)
        NUMBER_PANEL_TYPE = 2;
        fShowBinSelect->iShowBinDigitalTask = 1;
        T3();
        CHECK(fShowBinSelect->iShowBinDigitalTask == 1);  // type 2: golden Timer1 (:3439-3440), not Timer3
        NUMBER_PANEL_TYPE = 3;
        fShowBinSelect->bUpdateBinDigital = true;
        T3();                                             // + ChangeBinDispStatus
        CHECK(fShowBinSelect->bUpdateBinDigital == false);   // the type 3 / 4 branch ran (its BinDisCtrl writes are gated D7 / D8)
        NUMBER_PANEL_TYPE = 0;
        fShowBinSelect->bUpdateBinDigital = false;
        for (int i = 0; i < 12; ++i) SW[SwLoaderBin + i].Off();
    }

    std::printf(" 5. G10 [E73]\n");
    {
        IniConfig.bE73_InOutZStepMotorLossCheck = true;
        IniConfig.iE73StepMotorCheckCnt = 5;
        iInZHomeCnt = 5;
        iOutZHomeCnt = 4;
        const int m0 = W906_ShowMyMessage_Count;
        for (int i = 0; i < 59; ++i) T3();
        CHECK(W906_ShowMyMessage_Count == m0 && iInZHomeCnt == 5 && iOutZHomeCnt == 4);
        T3();                                             // the 60th
        CHECK(W906_ShowMyMessage_Count == m0 + 1 && Has(S1(), "Input arm Z"));
        CHECK(iInZHomeCnt == 0 && iOutZHomeCnt == 0);
        iOutZHomeCnt = 7;
        for (int i = 0; i < 60; ++i) T3();
        CHECK(W906_ShowMyMessage_Count == m0 + 2 && Has(S1(), "Output arm Z") && iOutZHomeCnt == 0);
        IniConfig.bE73_InOutZStepMotorLossCheck = false;
    }

    std::printf(" 6. G13 [A37]\n");
    {
        IniConfig.bA37LotStartLotEnd = true;
        IniConfig.bA68_AutoLoadUnload = false;
        LastSet.bWaitStartLotAutoRetestGPIB = false;
        LastSet.bWaitEndLotAutoRetestGPIB = false;
        TestIF_File.bSCKART_RunARTWithoutCmd = false;
        TrayForm.bSpecTrayCnt = false;
        LastSet.iRunStartMode = rsmInitialStart;
        hLotEndTimeOut.SetSecAndOn(120);
        hLotStartTimeOut.SetSecAndOn(0);
        ::Sleep(2);
        fSCKART->iCurrent93KARTStep = 2;
        const int m0 = W906_ShowMyMessage_Count;
        T3();
        CHECK(W906_ShowMyMessage_Count == m0 + 1 && Has(S1(), "EAP2S Key in"));
        T3();
        CHECK(W906_ShowMyMessage_Count == m0 + 1);        // re-armed for 120 s
        fSCKART->iCurrent93KARTStep = 4;
        hLotEndTimeOut.SetSecAndOn(0);
        ::Sleep(2);
        T3();
        CHECK(W906_ShowMyMessage_Count == m0 + 2 && S1() == "SECS/GEM host Lot End timeout!");
        T3();
        CHECK(W906_ShowMyMessage_Count == m0 + 2);
        TrayForm.bSpecTrayCnt = true;
        hLotEndTimeOut.SetSecAndOn(0);
        ::Sleep(2);
        T3();
        CHECK(W906_ShowMyMessage_Count == m0 + 2);        // AUTO IN OUT: no message
        TrayForm.bSpecTrayCnt = false;
        // the third message: bWaitEndLot on, bShowMsg set by the calls above
        LastSet.bWaitEndLotAutoRetestGPIB = true;
        fSCKART->iCurrent93KARTStep = 12;
        T3();
        CHECK(W906_ShowMyMessage_Count == m0 + 3 && Has(S1(), "(1)Test summary lot end finished"));
        T3();
        CHECK(W906_ShowMyMessage_Count == m0 + 3);        // bShowMsg cleared
        // step 4 counts only in the continue / retest / QA modes
        fSCKART->iCurrent93KARTStep = 4;
        LastSet.bWaitEndLotAutoRetestGPIB = false;  T3();  LastSet.bWaitEndLotAutoRetestGPIB = true;   // bShowMsg again (timer armed: no message)
        LastSet.iRunStartMode = rsmInitialStart;
        T3();
        CHECK(W906_ShowMyMessage_Count == m0 + 3);
        LastSet.iRunStartMode = rsmContinuRetest;
        T3();
        CHECK(W906_ShowMyMessage_Count == m0 + 4 && Has(S1(), "(1)Test summary lot end finished"));
        LastSet.bWaitEndLotAutoRetestGPIB = false;  T3();  LastSet.bWaitEndLotAutoRetestGPIB = true;
        IniConfig.bA68_AutoLoadUnload = true;             // [A68]: no box
        T3();
        CHECK(W906_ShowMyMessage_Count == m0 + 4);
        IniConfig.bA68_AutoLoadUnload = false;
        SystemStart = true;                               // running: no box
        T3();
        CHECK(W906_ShowMyMessage_Count == m0 + 4);
        SystemStart = false;
        IniConfig.bA37LotStartLotEnd = false;
        LastSet.bWaitEndLotAutoRetestGPIB = false;
        LastSet.iRunStartMode = rsmContinuStart;
        fSCKART->iCurrent93KARTStep = 0;
    }

    std::printf(" 7. G18 the hourly TimerRecordLoaderDate\n");
    {
        g_loaderDateCalls = 0;
        g_bar = "2026-10-01 13:00:00";
        T3();
        CHECK(g_loaderDateCalls == 1);
        g_bar = "2026-10-01 13:00:01";
        T3();
        g_bar = "2026-10-01 13:10:00";
        T3();
        g_bar = "2026-10-01 13:59:59";
        T3();
        CHECK(g_loaderDateCalls == 1);
        g_bar = "2026-10-01 14:00:00";
        T3();
        CHECK(g_loaderDateCalls == 2);
        const std::string now = ht9045::W906_Timer3StatusBarTimeNow().c_str();
        std::printf("    status bar text now: %s\n", now.c_str());
        CHECK(now.size() == 19 && now[4] == '-' && now[7] == '-' && now[10] == ' ' && now[13] == ':' && now[16] == ':');
        g_bar = "2026-10-01 13:30:15";
    }

    std::printf(" 8. G20 [RMS]\n");
    {
        CosFunction.bDownloadRecipeLevelMode = true;
        fLotInfo->edDeviceName->Enabled = true;  fLotInfo->cbbDeviceName->Enabled = true;
        fLotInfo->edTemp->Enabled = true;        fLotInfo->coLevelMode->Enabled = true;
        SystemStart = true;
        IniConfig.bEnableRms = false;
        T3();
        CHECK(fLotInfo->edDeviceName->Enabled && fLotInfo->cbbDeviceName->Enabled && fLotInfo->edTemp->Enabled && fLotInfo->coLevelMode->Enabled);
        IniConfig.bEnableRms = true;
        T3();
        CHECK(!fLotInfo->edDeviceName->Enabled && !fLotInfo->cbbDeviceName->Enabled && !fLotInfo->edTemp->Enabled && !fLotInfo->coLevelMode->Enabled);
        IniConfig.bEnableRms = false;
        SystemStart = false;
        CosFunction.bDownloadRecipeLevelMode = false;
    }

    std::printf(" 9. G26 WAR1684 / WAR16500\n");
    {
        asChangeSetupFileName = "T1.Setup";
        const int c0 = W906_ShowErrorMessage_Count;
        iChangeFileHasErr = 1;
        T3();
        CHECK(W906_ShowErrorMessage_Count == c0 + 1 && std::string(W906_ShowErrorMessage_LastCode.c_str()) == "WAR1684");
        CHECK(W906_ShowErrorMessage_LastKCode == K_SKIP && g_lastPos == MMSystem && std::string(W906_ShowErrorMessage_LastErrPart.c_str()) == "T1.Setup");
        CHECK(iChangeFileHasErr == 0);
        iChangeFileHasErr = 2;
        T3();
        CHECK(W906_ShowErrorMessage_Count == c0 + 2 && std::string(W906_ShowErrorMessage_LastCode.c_str()) == "WAR16500" && iChangeFileHasErr == 0);
        T3();
        CHECK(W906_ShowErrorMessage_Count == c0 + 2);
        fNote->fShow = true;                              // the Note box is up: waits
        iChangeFileHasErr = 1;
        T3();
        CHECK(W906_ShowErrorMessage_Count == c0 + 2 && iChangeFileHasErr == 1);
        fNote->fShow = false;
        T3();
        CHECK(W906_ShowErrorMessage_Count == c0 + 3 && iChangeFileHasErr == 0);
    }

    std::printf(" 10. the gated parts\n");
    {
        const int m0 = W906_ShowMyMessage_Count, e0 = W906_ShowErrorMessage_Count;
        CUSTOMER_CODE = CC_KYEC_LEE;                      // G14: golden's else runs (the KYEC branch is gated)
        IniConfig.bA11BarcodeTime = true;
        IniConfig.iA11BarcodeTime = 100;
        ReEnterBarcode[3] = true;
        iBarcodeTimeCount[3] = 0;
        T3();
        CHECK(ReEnterBarcode[3] == false && iBarcodeTimeCount[3] == 0);
        IniConfig.bA11BarcodeTime = false;
        CUSTOMER_CODE = CC_JCET;                          // G18: the JCET 10-minute line is gated, the hourly one runs
        g_loaderDateCalls = 0;
        g_bar = "2026-10-01 13:10:00";
        T3();
        CHECK(g_loaderDateCalls == 0);
        g_bar = "2026-10-01 14:00:00";
        T3();
        CHECK(g_loaderDateCalls == 1);
        g_bar = "2026-10-01 13:30:15";
        CUSTOMER_CODE = CC_UNISEM_M;                      // G17
        T3();
        CUSTOMER_CODE = ccSaved;
        CosFunction.bShowHandlerStopTime = true;          // G1
        IniConfig.bShowTrayAndDeviceDir = true;           // G23
        TestIF_File.bRENESAS_EnableFTCT = true;           // G27
        dCCDTemperature = 60.0;                           // G5: MNetLog (its body is gated: a no-op)
        for (int i = 0; i < 3; ++i) T3();
        CHECK(W906_ShowMyMessage_Count == m0 && W906_ShowErrorMessage_Count == e0);
        CosFunction.bShowHandlerStopTime = false;
        IniConfig.bShowTrayAndDeviceDir = false;
        TestIF_File.bRENESAS_EnableFTCT = false;
        dCCDTemperature = 0.0;
    }

    std::printf(" 11. the dispatcher: a box inside Timer3, close\n");
    {
        const unsigned long t0 = 600000;
        unsigned long n8 = 0, nts = 0, nesd = 0, re = 0, n3 = 0;
        ht9045::W906_St02TimersReset();
        ht9045::W906_St02TimersTickAt(t0);                // latch + grids start
        iChangeFileHasErr = 2;                            // G26 box in Timer3's next call
        g_nestAt = t0 + 2000;
        g_n8InBox = 0; g_reInBox = 0; g_n3InBox = 0;
        ht9045::W906_St02TimersTickAt(t0 + 1000);        // AI(W906-ST02-MRB) 20261004: the table's order (forms/fMain_Timers.h): Timer2, Timer3 -> its box -> the nested tick
        ht9045::W906_St02TimersCounts(&n8, &nts);
        ht9045::W906_St02TimersCountsESD(&nesd, &re);
        ht9045::W906_St02TimersCountsT3(&n3, 0);
        CHECK(iChangeFileHasErr == 0 && g_nestAt == 0);
        CHECK(n3 == 1 && g_n3InBox == 1);                 // AI(W906-ST02-MRB) 20261004: inside its own box the table skips Timer3 (Q85 = A; golden VCL would re-enter, R126 M4: accepted deviation) ...
        CHECK(g_reInBox == 1 && re == 1);                 // ... that one skip is Timer3's
        CHECK(g_n8InBox == 1 && n8 == 1);                 // ... Timer8 (after Timer3 in the table) fires inside the box, late, once
        bSystemClose = true;
        ht9045::W906_St02TimersTickAt(t0 + 100000);
        W906_St02TimersTickFromModal();
        unsigned long n3b = 0;
        ht9045::W906_St02TimersCountsT3(&n3b, 0);
        CHECK(n3b == n3);
        bSystemClose = false;
    }

    std::printf(" 12. the hook-up lines are code (argv[1] = the tree root, read only)\n");
    if (argc > 1)
    {
        const std::string root = argv[1];
        const std::string ds = ReadAll(root + "/MainTimersSt02.cpp"), cm = ReadAll(root + "/CMakeLists.txt");
        const std::string mt3 = ReadAll(root + "/MainTimer3.cpp"), wt = ReadAll(root + "/WebBridgeTags.cpp");
        CHECK(!ds.empty() && !cm.empty() && !mt3.empty() && !wt.empty());
        CHECK(CodeCount(ds, "W906_Timer3Timer();") == 2 && CodeCount(ds, "void W906_Timer3Timer();") == 1);   // the declaration + one call
        CHECK(CodeCount(ds, "InTick busy(kT3)") == 0 && CodeCount(ds, "InTick busy(kTS)") == 0 &&
              CodeCount(ds, "Due(g_t3, now, 1000)") == 0 && CodeCount(ds, "Due(g_tTS, now, 1000)") == 0);   // AI(W906-ST02-MRB) 20261004: no dispatcher slot left for Timer3 / TS ...
        CHECK(CodeCount(ds, "&St02OnTimer3") == 1 && CodeCount(ds, "&St02OnTimerTS") == 1 &&
              CodeCount(ds, "fMain->Timer3->Enabled = true;") == 1);                                     // ... they are table OnTimers, Timer3 on from the bind
        CHECK(CodeCount(mt3, "RecordTemp();") == 1 && CodeCount(mt3, "fObserver->dTempHistroy[i][59]=UN150Read[i];") == 1);   // R126 M5: G4 open
        CHECK(CodeCount(wt, "{ extern void W906_St02TimersTick(); W906_St02TimersTick(); }") == 1);   // PumpTick (S-15)
        const std::string cl = LineWith(cm, "MainTimer3.cpp");
        CHECK(!cl.empty() && cl.find("MainTimer3.cpp") < cl.find("#") && cl.find("MainTimerESDFallback.cpp  MainTimer3.cpp") != std::string::npos);
        CHECK(CodeCount(mt3, "fYieldMonitoring->CheckByPickerYieldAlarm();") == 1 && CodeCount(mt3, "fShowBinSelect->DoShowBinDigital();") == 1);
        CHECK(CodeCount(mt3, "tSoakTimer") == 0 && mt3.find("G25 not in this batch (Ifor01 I-01)") != std::string::npos);
        CHECK(CodeCount(mt3, "TTLLog(") == 0 && CodeCount(mt3, "UpdateLanguage(") == 0);   // gated
    }
    else
        std::printf("  (skipped: no tree root given)\n");

    std::printf(" 13. the real TfYieldMonitoring\n");
    {
        fYieldMonitoring = realYM;
        g_ym.clear();
        for (int i = 0; i < 3; ++i) T3();
        CHECK(g_ym.empty());                              // the real ones ran, not the counting stand-in
        delete counting;
    }

    std::printf(" 14. G4 RecordTemp: the Observer temperature history\n");
    {
        CHECK(fObserver != 0);
        const bool inst0 = bUT150Install[0], inst1 = bUT150Install[1];
        const double read0 = UN150Read[0];
        const int ivSaved = IniConfig.iL10TempRecordInterval;
        const Word secSaved = SystemSec;
        double h0[60], h1[60];
        for (int j = 0; j < 60; ++j) { h0[j] = fObserver->dTempHistroy[0][j]; h1[j] = fObserver->dTempHistroy[1][j]; }
        for (int j = 0; j < 60; ++j) { fObserver->dTempHistroy[0][j] = j; fObserver->dTempHistroy[1][j] = -1.0; }
        bUT150Install[0] = true;
        bUT150Install[1] = false;
        IniConfig.iL10TempRecordInterval = 1;             // golden's else: every 15 s
        UN150Read[0] = 123.5;
        SystemSec = 7;
        T3();                                             // 7 % 15 != 0 -> nothing
        CHECK(fObserver->dTempHistroy[0][0] == 0.0 && fObserver->dTempHistroy[0][59] == 59.0);
        SystemSec = 15;
        T3();                                             // a multiple of 15 -> shift left, the reading at [59]
        CHECK(fObserver->dTempHistroy[0][0] == 1.0 && fObserver->dTempHistroy[0][58] == 59.0 && fObserver->dTempHistroy[0][59] == 123.5);
        CHECK(fObserver->dTempHistroy[1][0] == -1.0 && fObserver->dTempHistroy[1][59] == -1.0);   // channel 1 not installed
        UN150Read[0] = 200.0;
        T3();                                             // the same second again -> no second record (golden iOldMin)
        CHECK(fObserver->dTempHistroy[0][59] == 123.5);
        IniConfig.iL10TempRecordInterval = 0;             // every 5 s
        SystemSec = 20;
        T3();
        CHECK(fObserver->dTempHistroy[0][58] == 123.5 && fObserver->dTempHistroy[0][59] == 200.0);
        for (int j = 0; j < 60; ++j) { fObserver->dTempHistroy[0][j] = h0[j]; fObserver->dTempHistroy[1][j] = h1[j]; }
        bUT150Install[0] = inst0;
        bUT150Install[1] = inst1;
        UN150Read[0] = read0;
        IniConfig.iL10TempRecordInterval = ivSaved;
        SystemSec = secSaved;
    }

    // AI(W906-ST02-0128) 20261002 (St02-E): the test MR !90 should have had.  fObserver is a STATIC facade (cObserver.cpp)
    //   built before the ini layer, so its constructor skips golden's TempChart AddSeries loop (906_0625_Steven
    //   cObserver.cpp:339-344, the port's INIFileGeneral guard); RecordTemp -> UpdateTempChart with the Observer page shown
    //   then hit Series[0] on an empty chart -> std::out_of_range -> wb_serve died 10-20 s after start (machine patch 0128
    //   OBS-TEMPSERIES, package 125, f09c5208: the series are now created on first use).  This drives that exact path.
    std::printf(" 15. G4 RecordTemp with the Observer page shown: UpdateTempChart on the static-init chart (0128)\n");
    {
        CHECK(fObserver != 0 && fObserver->TempChart != 0 && fObserver->cbbTempChart != 0);
        const bool showSaved = fObserver->bShow;
        const bool inst0 = bUT150Install[0];
        const double read0 = UN150Read[0];
        const int ivSaved15 = IniConfig.iL10TempRecordInterval;
        const Word secSaved15 = SystemSec;
        double h0[60];
        for (int j = 0; j < 60; ++j) h0[j] = fObserver->dTempHistroy[0][j];
        fObserver->bShow = true;                          // golden fObserver->bShow (W906_FormShowing: member || page table)
        bUT150Install[0] = true;
        IniConfig.iL10TempRecordInterval = 1;             // every 15 s
        UN150Read[0] = 55.0;
        SystemSec = 45;                                   // a multiple of 15, not the second section 14 recorded last
        bool threw = false;
        try { T3(); } catch (...) { threw = true; }
        CHECK(!threw);                                    // before 0128: std::out_of_range from Series[0] (terminate in wb_serve)
        CHECK(fObserver->TempChart->SeriesCount() >= tcTotalCount);   // the series exist after the first use (0128)
        CHECK(fObserver->dTempHistroy[0][59] == 55.0);  // RecordTemp still recorded the reading
        fObserver->cbbTempChart->ItemIndex = 0;           // "All": golden UpdateTempChart's all-series arm
        UN150Read[0] = 66.0;
        SystemSec = 60;
        threw = false;
        try { T3(); } catch (...) { threw = true; }
        CHECK(!threw);
        CHECK(fObserver->TempChart->SeriesCount() >= tcTotalCount &&
              fObserver->TempChart->Series[0]->Points.size() == 60);   // channel 0 installed: its 60 samples are drawn
        fObserver->bShow = showSaved;
        for (int j = 0; j < 60; ++j) fObserver->dTempHistroy[0][j] = h0[j];
        bUT150Install[0] = inst0;
        UN150Read[0] = read0;
        IniConfig.iL10TempRecordInterval = ivSaved15;
        SystemSec = secSaved15;
    }

    W906_ShowErrorMessage_Hook = 0;
    ht9045::W906_Timer3SetRecordLoaderDate(0);
    ht9045::W906_Timer3SetStatusBarTime(0);
    ht9045::W906_Timer3SetNewRecordProcess(0);
    std::printf("%s: %d/%d checks passed (sandbox %s)\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total, sandbox.c_str());
    return g_fail ? 1 : 0;
}
