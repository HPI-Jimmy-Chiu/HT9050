// =============================================================================
//  tests/test_st02_timer_esd.cpp -- S-13: golden TfMain::TimerESDTimer (906_0625_Steven main.cpp:30911-31173) =
//  MainTimerESD.cpp + its tESDError part THandlerTesterSide::TimerESDDrainESDError (TesterComm/Handler/HandlerTesterSide.cpp),
//  and the dispatcher's TimerESD slot / per-timer re-entry guard / close guard (MainTimersSt02.cpp).
//  AI(W906-S13) 20261001 (St02-E).  Suite: St02_TimerESD.  Built and run in both configurations.
//
//  CONTAINMENT FIRST (st02_test_containment.h): RecordProcess / ProductionLog write through ctest's redirect roots only;
//  asYieldRecordPath and the temperature log roots point at a %TEMP% sandbox before anything runs.
//    1. SIM build only -- golden :30921-30923 `#ifdef SOFT_SIMULTE return;`: a queued code, tGPIBMsg text, [I29] and [O06-8]
//       on, 700 calls of W906_TimerESDTimer -> nothing moves (queue, list, boxes, files, TimerRecordLoaderDate).  Runs right
//       after 2a, which makes the queue object.
//    2. both builds -- the tESDError part through its hook (that body has no SOFT_SIMULTE branch; golden :30937-31026):
//       a. no TesterComm (fTesterSide NULL) -> nothing, no crash; an empty queue -> no box;
//       b. MES0731 + Skip: the box gets "MES0731", K_SKIP|K_ONECYCLE, MMSystem, "TimerESD"; the code is still queued while
//          the box waits; Index cells not empty and < TEST_PASS -> TEST_PASS+iTestBinCount with a "RESET" record, the
//          others unchanged; InitOneCycle("... user skip IC on index arm."); fMain->BtnOneCycle->Down;
//       c. MES0731 + One Cycle: no cell changes; InitOneCycle("... user perform one cycle."); Down;
//       d. WAR07326 with [I46] = 1 / 2 / 0 -> K_SKIP / K_RETRY / K_SKIP|K_RETRY; WAR07317 likewise; MES1713 -> K_RETRY|K_SKIP;
//          any other code -> K_RETRY;
//       e. the same code again within 1.5 s (alarm-age seam) -> no box, "The same Alarm occurs : <code>" recorded; a third
//          time -> a box; again after 1.5 s -> a box;
//       f. the Note box showing (fNote->fShow) -> the queue does not move.
//    3. SHIP build only -- the timer itself: E2 without / with the hook; golden bRun (a call from inside a waiting box
//       returns at once); E3 tGPIBMsg: one ShowMyMessage per call; E4 [I29] interval 3 -> the 3rd call writes the
//       "By Interval" line; E5 [O06-8] period 0 -> the 600th call writes the golden :31114-31146 line (fixed values) and
//       calls TimerRecordLoaderDate once; E1 / E6 gated (bSPILFunction + SystemStart on, 601 calls -> no box).
//    4. both builds -- the dispatcher: TimerESD fires 1000 ms after the start and then every 1000 ms; a waiting box inside
//       one timer -> the modal-wait tick skips that timer only (Timer8's MES0921 in both builds, TimerESD's own box in
//       SHIP); bSystemClose -> nothing fires (W906_St02TimersTickFromModal included).
//    5. the hook-up lines are code, not comment (argv[1] = the tree root; ctest passes it): wb_serve.cpp W906_ModalWaitTick's
//       first line calls W906_St02TimersTickFromModal (and still St01's A01 tick), WebBridgeTags.cpp PumpTick calls the
//       dispatcher, TesterCommWiring.cpp installs the hook, CMakeLists.txt lists MainTimerESD.cpp.
// =============================================================================
#include "MachineType.h"
#include "cmydef.h"
#include "Config.h"
#include "cprod.h"
#include "LastSet.h"
#include "mysensor.h"
#include "aHotPlateSubstrate.h"
#include "cSocket.h"
#include "canary_support.h"
#include "common.h"
#include "cpublic.h"           // GetTimeInfo
#include "forms/fMain.h"
#include "forms/fNote.h"
#include "Public/MyProductionRecord.h"
#include "TesterComm/Handler/HandlerTesterSide.h"
#include "st02_test_containment.h"

#include <windows.h>
#include <cstdio>
#include <string>
#include <vector>

extern TStringList W906ART_fMain_tGPIBMsg;   // AutoRetest.cpp:284 (golden fMain->tGPIBMsg)
extern AnsiString ExString;                  // cMyDB.cpp (cMyDB.h:143; not included: it redeclares canary_support.h's RecordProcess default)

namespace ht9045 {
void W906_TimerESDTimer();
void W906_TimerESDSetRecordLoaderDate(int (*fn)());
void W906_St02TimersTickAt(unsigned long now);
void W906_St02TimersReset();
void W906_St02TimersCounts(unsigned long* t8, unsigned long* ts);
void W906_St02TimersCountsESD(unsigned long* esd, unsigned long* reentries);
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

// ---- the operator (W906_ShowErrorMessage_Hook) ----
static int g_answer = 0;               // the key pressed; 0 = no answer -> W906_ShowErrorMessage_SimReturn (K_RETRY)
static int g_lastPos = -1;
static int g_queueAtBox = -1;          // fTesterSide->tESDError->Count while the box waits
static unsigned long g_nestAt = 0;     // non-zero: the box runs the dispatcher once at this time (the modal-wait tick)
static unsigned long g_n8InBox = 0, g_reInBox = 0;
static bool g_callEsdInBox = false;    // the box calls W906_TimerESDTimer directly (golden bRun)
static int g_smmInBox = -1;            // W906_ShowMyMessage_Count change caused by that call
static int Operator(const char* code, int kcode, int pos)
{
    (void)code; (void)kcode;
    g_lastPos = pos;
    g_queueAtBox = (fTesterSide && fTesterSide->tESDError) ? fTesterSide->tESDError->Count : -1;
    if (g_nestAt)
    {
        const unsigned long n = g_nestAt;
        g_nestAt = 0;
        ht9045::W906_St02TimersTickAt(n);
        ht9045::W906_St02TimersCounts(&g_n8InBox, 0);
        ht9045::W906_St02TimersCountsESD(0, &g_reInBox);
    }
    if (g_callEsdInBox)
    {
        g_callEsdInBox = false;
        const int c0 = W906_ShowMyMessage_Count;
        ht9045::W906_TimerESDTimer();
        g_smmInBox = W906_ShowMyMessage_Count - c0;
    }
    return g_answer;
}

static int g_age = 5000;               // the alarm age (ms) THandlerTesterSide sees instead of tESDAlarmTimer
static int FakeAge() { return g_age; }
static int g_loaderDateCalls = 0;
static int FakeLoaderDate() { ++g_loaderDateCalls; return 0; }

static void FixSensor(int idx, bool on)   // as tests/test_i115b_lifted.cpp:112-118: no IO read, Type decides
{
    Sen[idx].Enable  = true;
    Sen[idx].ISABase = 99;
    Sen[idx].Type    = on ? 0 : 1;
}

static bool DirExists(const std::string& p)
{
    const DWORD a = ::GetFileAttributesA(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
}

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

static std::string SiteCsvText(const std::string& dir)   // all *Site.csv files of the yield month folder
{
    std::string all;
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*Site.csv").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return all;
    do { all += ReadAll(dir + "\\" + fd.cFileName); } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
    return all;
}

static TStringList* Q() { return fTesterSide ? fTesterSide->tESDError : 0; }

static void ClearIndexGrids()
{
    for (int i = 0; i < MAX_Index_Row; i++)
        for (int j = 0; j < NEW_MAX_Index_Col; j++)
        {
            FTestSuck.Item[i][j]  = NULL_IC;
            BTestSuck.Item[i][j]  = NULL_IC;
            TestSocket.Item[i][j] = NULL_IC;
        }
}

static bool HasReset(TMyKitSuck& k, int i, int j)
{
    return std::string(k.PordRec[i][j].asBuffer->GetString(eErrorCode).c_str()).find("RESET") != std::string::npos;
}

// the code part of one source line: // and /* */ comments removed, string literals kept (as test_d015_a01_autologout.cpp)
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

static int CodeCount(const std::string& text, const std::string& needle)   // lines whose code part has needle (#if 0 ... #endif
{                                                                          //   skipped the same way as test_d015_a01_autologout.cpp)
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

static std::string LineAfter(const std::string& text, const std::string& exact)
{
    const std::vector<std::string> v = Lines(text);
    for (size_t i = 0; i + 1 < v.size(); ++i)
        if (v[i] == exact) return v[i + 1];
    return std::string();
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
    std::printf("St02_TimerESD\n");
    if (!W906TestInsideCtestRoots("St02_TimerESD"))
        return 2;
    char tmp[MAX_PATH];
    ::GetTempPathA(MAX_PATH, tmp);
    char sb[MAX_PATH];
    std::snprintf(sb, sizeof(sb), "%sht9045_st02esd_%lu", tmp, (unsigned long)::GetTickCount());
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

    InitialOK = true;
    GetTimeInfo();
    char month[16];
    std::snprintf(month, sizeof(month), "%04d%02d", (int)SystemYear, (int)SystemMonth);
    const std::string yieldMonth = sandbox + "\\Yield\\" + month;
    IniConfig.bP29LoaderCheckIsFull   = false;
    IniConfig.bRecordJamRateByTime    = false;
    IniConfig.bL10IndexTestlogTemp    = true;    // TemperatureStorageLog not called by the dispatcher (S-15)
    IniConfig.bVTESTFunction          = false;
    IniConfig.bI29EnableYieldRecord   = false;
    IniConfig.bI29_1SaveYieldBySocketByBin = false;
    IniConfig.bO06SaveLogTimePeriod   = false;
    IniConfig.bSPILFunction           = false;
    IniConfig.iI46_ActionWhenGpibFlowErr = 0;
    TestIF_File.iGpibMode = 0;
    iTestBinCount = 8;
    SystemStart = false;
    CHECK(fMain != 0 && fNote != 0 && fMain->BtnOneCycle != 0);
    if (fMain == 0 || fNote == 0 || fMain->BtnOneCycle == 0) return 1;
    fNote->fShow = false;
    W906_ShowErrorMessage_Reset();
    W906_ShowErrorMessage_Hook = &Operator;
    THandlerTesterSide::TimerESDSetAlarmAgeForTest(&FakeAge);
    ht9045::W906_TimerESDSetRecordLoaderDate(&FakeLoaderDate);
    ht9045::W906_ESDErrorDrainHook = 0;

    std::printf(" 2a. no TesterComm / empty queue\n");
    {
        CHECK(fTesterSide == 0);
        const int c0 = W906_ShowErrorMessage_Count;
        THandlerTesterSide::ESDErrorDrainHook();          // fTesterSide NULL: nothing
        CHECK(W906_ShowErrorMessage_Count == c0);
        fTesterSide = new THandlerTesterSide();           // not attached to the hub: no engine, only the queue
        CHECK(Q() != 0 && Q()->Count == 0);
        THandlerTesterSide::ESDErrorDrainHook();
        CHECK(W906_ShowErrorMessage_Count == c0);         // empty queue: no box
    }

#ifdef SOFT_SIMULTE
    std::printf(" 1. SIM build: W906_TimerESDTimer does nothing (golden :30921-30923)\n");
    {
        ht9045::W906_ESDErrorDrainHook = &THandlerTesterSide::ESDErrorDrainHook;
        Q()->Add("MES0731");
        W906ART_fMain_tGPIBMsg.Add("RCMD:S10F3 sim");
        IniConfig.bI29EnableYieldRecord = true;
        IniConfig.fI29YieldRecordInterval = 1;
        IniConfig.bO06SaveLogTimePeriod = true;
        IniConfig.iO06SaveLogTimePeriod = 0;
        const int e0 = W906_ShowErrorMessage_Count, m0 = W906_ShowMyMessage_Count;
        for (int i = 0; i < 700; ++i)
            ht9045::W906_TimerESDTimer();
        CHECK(Q()->Count == 1 && W906ART_fMain_tGPIBMsg.Count == 1);
        CHECK(W906_ShowErrorMessage_Count == e0 && W906_ShowMyMessage_Count == m0);
        CHECK(g_loaderDateCalls == 0 && !DirExists(yieldMonth));
        Q()->Clear();
        W906ART_fMain_tGPIBMsg.Clear();
        IniConfig.bI29EnableYieldRecord = false;
        IniConfig.bO06SaveLogTimePeriod = false;
        ht9045::W906_ESDErrorDrainHook = 0;
    }
#endif

    std::printf(" 2b. MES0731 + Skip\n");
    {
        ClearIndexGrids();
        FTestSuck.Item[0][0]  = HAS_IC;                   // -> TEST_PASS+iTestBinCount
        FTestSuck.Item[0][1]  = HAS_NULL_IC;              // unchanged
        BTestSuck.Item[1][NEW_MAX_Index_Col - 1] = HAS_IC;   // -> changed (last column)
        TestSocket.Item[1][1] = HAS_IC;                   // -> changed
        TestSocket.Item[0][2] = TEST_PASS + 3;            // already tested: unchanged
        iOneCycle = 7;
        bLampOneCycle = false;
        fMain->BtnOneCycle->Down = false;
        Q()->Add("MES0731");
        g_answer = K_SKIP;
        THandlerTesterSide::ESDErrorDrainHook();
        CHECK(std::string(W906_ShowErrorMessage_LastCode.c_str()) == "MES0731");
        CHECK(W906_ShowErrorMessage_LastKCode == (K_SKIP | K_ONECYCLE) && g_lastPos == MMSystem);
        CHECK(std::string(W906_ShowErrorMessage_LastErrPart.c_str()) == "TimerESD");
        CHECK(g_queueAtBox == 1 && Q()->Count == 0);      // deleted only after the answer
        CHECK(FTestSuck.Item[0][0] == TEST_PASS + iTestBinCount && HasReset(FTestSuck, 0, 0));
        CHECK(BTestSuck.Item[1][NEW_MAX_Index_Col - 1] == TEST_PASS + iTestBinCount && HasReset(BTestSuck, 1, NEW_MAX_Index_Col - 1));
        CHECK(TestSocket.Item[1][1] == TEST_PASS + iTestBinCount && HasReset(TestSocket, 1, 1));
        CHECK(FTestSuck.Item[0][1] == HAS_NULL_IC && TestSocket.Item[0][2] == TEST_PASS + 3 && FTestSuck.Item[1][0] == NULL_IC);
        CHECK(iOneCycle == 0 && bLampOneCycle == true);   // InitOneCycle ran
        CHECK(std::string(ExString.c_str()) == "Start ONE CYCLE by After Tester Pause handler, user skip IC on index arm.");
        CHECK(fMain->BtnOneCycle->Down == true);
    }

    std::printf(" 2c. MES0731 + One Cycle\n");
    {
        ClearIndexGrids();
        FTestSuck.Item[0][3] = HAS_IC;
        iOneCycle = 7;
        fMain->BtnOneCycle->Down = false;
        Q()->Add("MES0731");
        g_answer = K_ONECYCLE;
        THandlerTesterSide::ESDErrorDrainHook();
        CHECK(W906_ShowErrorMessage_LastKCode == (K_SKIP | K_ONECYCLE) && Q()->Count == 0);
        CHECK(FTestSuck.Item[0][3] == HAS_IC);            // no IC changed
        CHECK(iOneCycle == 0);
        CHECK(std::string(ExString.c_str()) == "Start ONE CYCLE by After Tester Pause handler, user perform one cycle.");
        CHECK(fMain->BtnOneCycle->Down == true);
        ClearIndexGrids();
    }

    std::printf(" 2d. the keys per code ([I46])\n");
    {
        g_answer = K_SKIP;                                // K_SKIP -> TestProcessSetToErr (an empty stand-in today): no crash
        const int i46[3] = { 1, 2, 0 };
        const int want[3] = { K_SKIP, K_RETRY, K_SKIP | K_RETRY };
        for (int k = 0; k < 3; ++k)
        {
            IniConfig.iI46_ActionWhenGpibFlowErr = i46[k];
            Q()->Add("WAR07326");
            THandlerTesterSide::ESDErrorDrainHook();
            CHECK(std::string(W906_ShowErrorMessage_LastCode.c_str()) == "WAR07326" && W906_ShowErrorMessage_LastKCode == want[k]);
        }
        IniConfig.iI46_ActionWhenGpibFlowErr = 0;
        Q()->Add("WAR07317");
        THandlerTesterSide::ESDErrorDrainHook();
        CHECK(W906_ShowErrorMessage_LastKCode == (K_SKIP | K_RETRY));
        Q()->Add("MES1713");
        THandlerTesterSide::ESDErrorDrainHook();
        CHECK(W906_ShowErrorMessage_LastKCode == (K_RETRY | K_SKIP));
        Q()->Add("WAR0711");
        THandlerTesterSide::ESDErrorDrainHook();
        CHECK(std::string(W906_ShowErrorMessage_LastCode.c_str()) == "WAR0711" && W906_ShowErrorMessage_LastKCode == K_RETRY);
        CHECK(Q()->Count == 0);
        g_answer = 0;
    }

    std::printf(" 2e. the same code within 1.5 s\n");
    {
        Q()->Add("WAR0704");
        Q()->Add("WAR0704");
        Q()->Add("WAR0704");
        g_age = 100;
        const int c0 = W906_ShowErrorMessage_Count;
        THandlerTesterSide::ESDErrorDrainHook();          // box (WAR0711 was the last one)
        CHECK(W906_ShowErrorMessage_Count == c0 + 1);
        THandlerTesterSide::ESDErrorDrainHook();          // same code, 100 ms -> recorded, no box
        CHECK(W906_ShowErrorMessage_Count == c0 + 1 && Q()->Count == 1);
        CHECK(std::string(ExString.c_str()) == "The same Alarm occurs : WAR0704");
        THandlerTesterSide::ESDErrorDrainHook();          // golden cleared asOldESDError -> box
        CHECK(W906_ShowErrorMessage_Count == c0 + 2 && Q()->Count == 0);
        Q()->Add("WAR0704");
        g_age = 2000;                                     // 1.5 s passed -> box
        THandlerTesterSide::ESDErrorDrainHook();
        CHECK(W906_ShowErrorMessage_Count == c0 + 3 && Q()->Count == 0);
        g_age = 5000;
    }

    std::printf(" 2f. the Note box showing -> the queue waits\n");
    {
        Q()->Add("WAR0711");
        fNote->fShow = true;
        const int c0 = W906_ShowErrorMessage_Count;
        THandlerTesterSide::ESDErrorDrainHook();
        CHECK(W906_ShowErrorMessage_Count == c0 && Q()->Count == 1);
        fNote->fShow = false;
        THandlerTesterSide::ESDErrorDrainHook();
        CHECK(W906_ShowErrorMessage_Count == c0 + 1 && Q()->Count == 0);
    }

#ifndef SOFT_SIMULTE
    std::printf(" 3a. SHIP: the timer -- E2 without / with the hook, golden bRun\n");
    {
        ht9045::W906_ESDErrorDrainHook = 0;
        Q()->Add("WAR0711");
        const int c0 = W906_ShowErrorMessage_Count;
        ht9045::W906_TimerESDTimer();                     // no hook: no queue for the timer
        CHECK(W906_ShowErrorMessage_Count == c0 && Q()->Count == 1);
        ht9045::W906_ESDErrorDrainHook = &THandlerTesterSide::ESDErrorDrainHook;
        W906ART_fMain_tGPIBMsg.Add("RCMD:TESTER_ERROR in box");
        g_callEsdInBox = true;                            // the box calls the timer again: golden bRun -> returns at once
        ht9045::W906_TimerESDTimer();
        CHECK(W906_ShowErrorMessage_Count == c0 + 1 && Q()->Count == 0);
        CHECK(g_smmInBox == 0);                           // the nested call did not reach E3
        CHECK(W906ART_fMain_tGPIBMsg.Count == 0);         // the outer call did, after the box (E3 follows E2)
        CHECK(std::string(W906_ShowMyMessage_LastS1.c_str()) == "RCMD:TESTER_ERROR in box");
    }

    std::printf(" 3b. SHIP: E3 tGPIBMsg -> one ShowMyMessage per call\n");
    {
        W906ART_fMain_tGPIBMsg.Add("MSG-A");
        W906ART_fMain_tGPIBMsg.Add("MSG-B");
        const int m0 = W906_ShowMyMessage_Count;
        ht9045::W906_TimerESDTimer();
        CHECK(W906_ShowMyMessage_Count == m0 + 1 && std::string(W906_ShowMyMessage_LastS1.c_str()) == "MSG-A" && W906ART_fMain_tGPIBMsg.Count == 1);
        ht9045::W906_TimerESDTimer();
        CHECK(W906_ShowMyMessage_Count == m0 + 2 && std::string(W906_ShowMyMessage_LastS1.c_str()) == "MSG-B" && W906ART_fMain_tGPIBMsg.Count == 0);
        ht9045::W906_TimerESDTimer();
        CHECK(W906_ShowMyMessage_Count == m0 + 2);
    }

    TestSocket.iShtRow = 2;                               // E4 / E5: one logged arm row (golden starts at row 1)
    TestSocket.iShtCol = 1;
    ArmData[0]->ArmSKET[1][0]->Pass = 3;  ArmData[0]->ArmSKET[1][0]->Total = 4;   // 75 %
    ArmData[1]->ArmSKET[1][0]->Pass = 1;  ArmData[1]->ArmSKET[1][0]->Total = 2;   // 50 %

    std::printf(" 3c. SHIP: E4 [I29] By Interval\n");
    {
        IniConfig.bI29EnableYieldRecord = true;
        IniConfig.fI29YieldRecordInterval = 3;
        ht9045::W906_TimerESDTimer();
        ht9045::W906_TimerESDTimer();
        CHECK(!DirExists(yieldMonth));                    // calls 1 and 2: not yet
        ht9045::W906_TimerESDTimer();
        CHECK(DirExists(yieldMonth));                     // call 3: SaveSiteYield("By Interval")
        CHECK(SiteCsvText(yieldMonth).find(",By Interval,") != std::string::npos);
        IniConfig.bI29EnableYieldRecord = false;
    }

    std::printf(" 3d. SHIP: E5 [O06-8] the production log line\n");
    {
        for (int i = 0; i < eTrayCount; ++i)
        {
            Prod.iTrayType[i] = tNotUse;
            Prod.iIsFailT6[i] = 0;
        }
        for (int i = 0; i < iTestBinCount; ++i)
            Prod.iT6PosCate[i] = 0;
        Prod.iT6PosCate[1] = 1;  Prod.iT6PosCate[3] = 1;  Prod.iT6PosCate[2] = 2;   // A1: bins 1,3   A2: bin 2
        Prod.iTrayType[0] = tTrayAuto;  Prod.iTrayType[1] = tTrayAuto;
        Prod.iIsFailT6[1] = 1;  Prod.iIfErrorT6 = 1;                                 // A1 P, A2 FE
        iTo3Unload[0] = 0;  iTo3Unload[1] = 1;
        LastSet.BinCT[0][0] = 10;  LastSet.BinCT[0][1] = 20;
        LastSet.SendCT[0] = 30;
        RunInfo.iUnloadCount = 29;
        TestIF_File.iShuttleMode = 0;
        LastSet.iTemperature = Tempture_Ambient;
        fMain->cbSetupFileName->Text = "T.Setup";
        IniConfig.bO06SaveLogTimePeriod = true;
        IniConfig.iO06SaveLogTimePeriod = 0;              // 0*1200+600 = 600 calls (10 minutes)
        g_loaderDateCalls = 0;
        for (int i = 0; i < 599; ++i)
            ht9045::W906_TimerESDTimer();
        CHECK(g_loaderDateCalls == 0);
        ht9045::W906_TimerESDTimer();                     // the 600th
        CHECK(g_loaderDateCalls == 1);
        TStringList* lines = fMain->MemoProductionLog->Lines;
        const std::string want = "Load:30;Total:29;A1:10;A2:20;A1(P):1,3;A2(FE):2;Aa:62.50%;SetupFile:T.Setup;Temperature:25.0;";
        const std::string last = (lines && lines->Count > 0) ? std::string(lines->GetString(lines->Count - 1).c_str()) : std::string();
        std::printf("    last production log line: %s\n", last.c_str());
        CHECK(last.size() >= want.size() && last.compare(last.size() - want.size(), want.size(), want) == 0);
        CHECK(last.find(" --> Load:") != std::string::npos);
        IniConfig.bO06SaveLogTimePeriod = false;
    }

    std::printf(" 3e. SHIP: E1 / E6 gated\n");
    {
        IniConfig.bSPILFunction = true;
        SystemStart = true;
        const int m0 = W906_ShowMyMessage_Count;
        for (int i = 0; i < 601; ++i)
            ht9045::W906_TimerESDTimer();
        CHECK(W906_ShowMyMessage_Count == m0);            // golden would check the event-log folder at call 600
        IniConfig.bSPILFunction = false;
        SystemStart = false;
    }
#endif

    std::printf(" 4. the dispatcher\n");
    {
        ht9045::W906_ESDErrorDrainHook = &THandlerTesterSide::ESDErrorDrainHook;
        const unsigned long t0 = 200000;
        unsigned long n8 = 0, nts = 0, nesd = 0, re = 0;
        ht9045::W906_St02TimersReset();
        ht9045::W906_St02TimersTickAt(t0);
        ht9045::W906_St02TimersTickAt(t0 + 500);
        ht9045::W906_St02TimersCountsESD(&nesd, &re);
        CHECK(nesd == 0);                                 // not before one Interval
        ht9045::W906_St02TimersTickAt(t0 + 1000);
        ht9045::W906_St02TimersTickAt(t0 + 1500);
        ht9045::W906_St02TimersCountsESD(&nesd, &re);
        CHECK(nesd == 1);
        ht9045::W906_St02TimersTickAt(t0 + 2000);
        ht9045::W906_St02TimersCountsESD(&nesd, &re);
        CHECK(nesd == 2 && re == 0);

        // Timer8's MES0921 box (both builds): the modal-wait tick inside it skips Timer8 only
        IniConfig.bP29LoaderCheckIsFull = true;
        IniConfig.dP29LoaderCheckIsFullInterval = 0;
        FixSensor(SnLoaderIsFull, true);
        unsigned long t = t0 + 2000;
        const int c0 = W906_ShowErrorMessage_Count;
        for (int k = 0; k < 5 && W906_ShowErrorMessage_Count == c0; ++k)
        {
            t += 1000;
            g_nestAt = t + 1000;                          // used by the first box only
            ::Sleep(5);                                   // the 0 s loader-full delay (as St02_MainTimers section 2)
            ht9045::W906_St02TimersTickAt(t);
        }
        CHECK(W906_ShowErrorMessage_Count > c0 && std::string(W906_ShowErrorMessage_LastCode.c_str()) == "MES0921");
        ht9045::W906_St02TimersCounts(&n8, &nts);
        ht9045::W906_St02TimersCountsESD(&nesd, &re);
        CHECK(g_nestAt == 0 && g_reInBox == 1);           // inside the box: Timer8 skipped once ...
        CHECK(n8 == g_n8InBox);                           // ... and not counted again
        CHECK(nts >= 2 && nesd >= 3);                     // the other two kept firing (inside the box too)
        g_nestAt = 0;
        IniConfig.bP29LoaderCheckIsFull = false;
        FixSensor(SnLoaderIsFull, false);
        Sen[SnLoaderIsFull].Enable = false;

#ifndef SOFT_SIMULTE
        // TimerESD's own box (SHIP): Timer8 fires inside it, TimerESD is skipped
        ht9045::W906_St02TimersReset();
        ht9045::W906_St02TimersTickAt(t0);
        Q()->Add("WAR0711");
        g_nestAt = t0 + 2000;
        g_n8InBox = 0;
        g_reInBox = 0;
        ht9045::W906_St02TimersTickAt(t0 + 1000);        // Timer8, TS, then TimerESD -> its box -> the nested tick
        ht9045::W906_St02TimersCounts(&n8, &nts);
        ht9045::W906_St02TimersCountsESD(&nesd, &re);
        CHECK(Q()->Count == 0 && nesd == 1);
        CHECK(g_n8InBox == 2 && g_reInBox == 1);          // Timer8 fired inside TimerESD's box; TimerESD skipped
        g_nestAt = 0;
#endif

        // close: nothing fires after bSystemClose (golden FormClose)
        ht9045::W906_St02TimersCounts(&n8, &nts);
        ht9045::W906_St02TimersCountsESD(&nesd, &re);
        bSystemClose = true;
        ht9045::W906_St02TimersTickAt(t0 + 100000);
        W906_St02TimersTickFromModal();
        unsigned long n8b = 0, ntsb = 0, nesdb = 0, reb = 0;
        ht9045::W906_St02TimersCounts(&n8b, &ntsb);
        ht9045::W906_St02TimersCountsESD(&nesdb, &reb);
        CHECK(n8b == n8 && ntsb == nts && nesdb == nesd);
        bSystemClose = false;
    }

    std::printf(" 5. the hook-up lines are code (argv[1] = the tree root, read only)\n");
    if (argc > 1)
    {
        const std::string root = argv[1];
        const std::string wb = ReadAll(root + "/tools/wb_serve.cpp"), wt = ReadAll(root + "/WebBridgeTags.cpp");
        const std::string cw = ReadAll(root + "/TesterComm/Handler/TesterCommWiring.cpp"), cm = ReadAll(root + "/CMakeLists.txt");
        CHECK(!wb.empty() && !wt.empty() && !cw.empty() && !cm.empty());
        const std::string modal = "{ extern void W906_St02TimersTickFromModal(); W906_St02TimersTickFromModal(); }";
        CHECK(CodeOfLine("x;  // " + modal).find(modal) == std::string::npos &&
              CodeOfLine("x;  /* " + modal + " */  y;").find(modal) == std::string::npos &&
              CodeOfLine("x;  " + modal + "  /* a */  // b").find(modal) != std::string::npos);   // the ratchet itself
        const std::string ml = LineAfter(wb, "void W906_ModalWaitTick(int kind, int kcode)");
        CHECK(CodeOfLine(ml).find(modal) != std::string::npos && CodeCount(wb, "W906_St02TimersTickFromModal();") == 1);
        CHECK(CodeOfLine(ml).find("{ extern void W906_A01AutoLogoutTick(); W906_A01AutoLogoutTick(); }") != std::string::npos);   // St01's call still there
        CHECK(CodeCount(wt, "{ extern void W906_St02TimersTick(); W906_St02TimersTick(); }") == 1);   // PumpTick :605 (S-15)
        CHECK(CodeCount(cw, "ht9045::W906_ESDErrorDrainHook = &THandlerTesterSide::ESDErrorDrainHook;") == 1);
        const std::string cl = LineWith(cm, "MainTimerESD.cpp");
        CHECK(!cl.empty() && cl.find("MainTimerESD.cpp") < cl.find("#") && cl.find("MainTimer8.cpp  MainTimerESD.cpp") != std::string::npos);
    }
    else
        std::printf("  (skipped: no tree root given)\n");

    W906_ShowErrorMessage_Hook = 0;
    THandlerTesterSide::TimerESDSetAlarmAgeForTest(0);
    ht9045::W906_TimerESDSetRecordLoaderDate(0);
    ht9045::W906_ESDErrorDrainHook = 0;
    delete fTesterSide;
    fTesterSide = 0;
    std::printf("%s: %d/%d checks passed (sandbox %s)\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total, sandbox.c_str());
    return g_fail ? 1 : 0;
}
