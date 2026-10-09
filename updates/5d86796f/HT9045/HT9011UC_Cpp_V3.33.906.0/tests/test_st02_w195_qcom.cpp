// ===========================================================================
//  tests/test_st02_w195_qcom.cpp -- W-195 (3) Qualcomm MR-Q1: the by-count yield check of golden 913 (RogerYang
//  AI(rf360-yield-count) 0814 + the 0916 else-if fix).  AI(W906-W195) 20261009 (St02-E).  依 W-195 卡，S25 不適用（Steven 1009 13:2x）.
//
//  Every log / yield path the exercised code can write (as9045LogPath, asSaveEventLogPath, asYieldRecordPath) points into
//  %TEMP%\ht9045_w195q_<tick> BEFORE the first call; the test stops if one is under D:\HT9045.  argv[1] = the tree root, read
//  only (part 4).
//
//    1. TfYieldMonitoring::UpdateYieldCountCheckTick (golden 913 uYieldMonitoring.cpp:6101-6128): flag off -> never; N new fails
//       -> one tick and a new base; fewer fails (ClearYieldCount / lot change) -> re-base; every run-state guard freezes the base
//       and the tick comes once the guard clears ("恢復後補檢查"); N <= 0 -> never.
//    2. the 913 fix on CheckLowYieldAlarm (golden :4606-4614): by-count on + no tick -> nothing even with "no wait 1 min"; by-count on
//       + tick -> the check body runs inside the 60 s interval; by-count off -> today's time gate.
//    3. InitialCosFunction: only CC_QUALCOMM turns CosFunction.bYieldAlarmCheckByCount on (golden CosFunction.cpp:2626 / :4583);
//       CC_PTI (HT9050) keeps it off.
//    4. source pins: MainTimer3.cpp calls the tick once, before the 8 checks (golden main.cpp:26378); the 7 Check* gates.
// ===========================================================================
#include "forms/fYieldMonitoring.h"
#include "atester_shims.h"       // fContact (TfContactShim), as test_yieldmon_core.cpp
#include "aHotPlateSubstrate.h"  // TestSocket
#include "forms/fTemp_Set.h"   // the W-195 rule: create fTemp_Set first (see main)
#include "cSocket.h"
#include "cmydef.h"
#include "cprod.h"
#include "common.h"
#include "Config.h"
#include "LastSet.h"
#include "CosFunction.h"
#include "MachineType.h"

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static int g_fail = 0;
static int g_total = 0;
static void check(bool ok, const char* what)
{
    ++g_total;
    std::printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}

static std::string Read(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static bool UnderMachineTree(const std::string& p)
{
    std::string s = p;
    for (size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    for (size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\';
    return s.compare(0, 9, "d:\\ht9045") == 0;
}

static void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (std::strcmp(fd.cFileName, ".") == 0 || std::strcmp(fd.cFileName, "..") == 0) continue;
            const std::string p = dir + "\\" + fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p);
            else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}

// as test_yieldmon_core.cpp ResetCommonGuards: every Check* guard true
static void ResetCommonGuards()
{
    SystemStart = true;
    fContact->fShow = false;
    iHome = 0;
    bRunAutoClean = false;
    bZ1PickShuttle = false;
    bZ2PickShuttle = false;
    LastSet.iRunStartMode = 0;
    CUSTOMER_CODE = 0;
    IniConfig.bKoreaFunction = false;
    CosFunction.bSmartAutoClean = false;
    CosFunction.bYieldAlmNeedOneCycle = false;
    CosFunction.bLowYieldDoOneCycle = false;
    CosFunction.bSpecailLowYeild = false;
    Prod.bFailAlarmLowYieldSpecial = false;
    bLowYeildAlarmSpecial = false;
    bLowYeildAlarmSpecial1stPass = false;
}

static void SetFails(unsigned a0, unsigned a1)
{
    ArmData[0]->ArmSKET[0][0]->Fail = a0;
    ArmData[1]->ArmSKET[0][0]->Fail = a1;
}

// the low-yield seed of test_yieldmon_core.cpp Test_CheckLowYieldAlarm_SameNS (:306-329): 15 % < 20 % over 200 ICs
static void SeedLowYield()
{
    Prod.bSlidingWindowYield = false;
    IniConfig.bEnableAutoCleanFunction = false;
    TestIF.iAutoClean_Function = false;
    Prod.bFailAlarmLowYield = true;
    Prod.dLowYieldLimit = 20.0;
    IniConfig.bLowYieldAlarmSameNS = true;
    CosFunction.bYieldControlUseEACount = false;
    bUseTwoArm32Site = false;
    Prod.iLowYieldCount = 150;
    TestIF_File.iTestMode = 0;
    TestIF.iShuttleMode = 0;
    IniConfig.bA09_ByArmCloseSite = false;
    TestSocket.iShtRow = 1;
    TestSocket.iShtCol = 1;
    LastSet.bUseTestSocket[0][0][0] = true;
    LastSet.bUseTestSocket[1][0][0] = true;
    ArmData[0]->ArmSKET[0][0]->Pass = 20;
    ArmData[0]->ArmSKET[0][0]->Fail = 80;
    ArmData[0]->ArmSKET[0][0]->Total = 100;
    ArmData[1]->ArmSKET[0][0]->Pass = 10;
    ArmData[1]->ArmSKET[0][0]->Fail = 90;
    ArmData[1]->ArmSKET[0][0]->Total = 100;
}
static bool AlarmRan() { return ArmData[0]->ArmSKET[0][0]->Pass == 0 && ArmData[0]->ArmSKET[0][0]->Total == 0; }   // ClearData(0,0) ran

// the body of `name` in `text` (from its definition line to the next line that is exactly "}")
static std::string Body(const std::string& text, const char* name)
{
    const size_t a = text.find(name);
    if (a == std::string::npos) return "";
    const size_t b = text.find("\n}", a);
    return text.substr(a, b == std::string::npos ? std::string::npos : b - a);
}
static int Count(const std::string& s, const char* what)
{
    int n = 0;
    for (size_t at = s.find(what); at != std::string::npos; at = s.find(what, at + 1)) ++n;
    return n;
}

int main(int argc, char** argv)
{
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    const std::string tmpRoot = std::string(tmp) + "ht9045_w195q_";
    const std::string root = tmpRoot + stamp;
    ::CreateDirectoryA(root.c_str(), 0);
    as9045LogPath = AnsiString(root.c_str());
    asSaveEventLogPath = AnsiString((root + "\\SaveEventLog").c_str());
    asYieldRecordPath = AnsiString((root + "\\Yield").c_str());
    IniConfig.bO06SaveLogTimePeriod = false;
    IniConfig.bI29YieldRecordIntervalIC = false;
    if (UnderMachineTree(as9045LogPath.c_str()) || UnderMachineTree(asSaveEventLogPath.c_str()) || UnderMachineTree(asYieldRecordPath.c_str()))
    {
        std::printf("STOP: a log path is under D:\\HT9045 -- nothing called\n");
        return 1;
    }
    if (fTemp_Set == 0) fTemp_Set = new TfTemp_Set();   // W-195 rule (laptop batch 136): golden boot creates TfTemp_Set
    GetTimeInfo();

    // ---- 1. the tick ----------------------------------------------------------
    {
        TfYieldMonitoring y;
        ResetCommonGuards();
        TestSocket.iShtRow = 1; TestSocket.iShtCol = 1;
        SetFails(0, 0);
        TestIF_File.iYieldAlarmCheckIntervalByCount = 3;

        CosFunction.bYieldAlarmCheckByCount = false;
        SetFails(5, 5);
        y.UpdateYieldCountCheckTick();
        check(y.bYieldCountCheckTick == false && y.ulYieldFailBase == 0, "flag off: never a tick, the base untouched (golden :6107-6108)");

        CosFunction.bYieldAlarmCheckByCount = true;
        SetFails(0, 0);
        y.UpdateYieldCountCheckTick();
        check(y.bYieldCountCheckTick == false, "flag on, 0 fails: no tick");
        SetFails(1, 1);
        y.UpdateYieldCountCheckTick();
        check(y.bYieldCountCheckTick == false && y.ulYieldFailBase == 0, "2 new fails < N=3: no tick, base 0");
        SetFails(2, 1);
        y.UpdateYieldCountCheckTick();
        check(y.bYieldCountCheckTick == true && y.ulYieldFailBase == 3, "3 new fails = N: a tick, the base moves to 3 (:6123-6127)");
        y.UpdateYieldCountCheckTick();
        check(y.bYieldCountCheckTick == false && y.ulYieldFailBase == 3, "the next second without new fails: no tick (it is reset first, :6105)");
        SetFails(4, 1);
        y.UpdateYieldCountCheckTick();
        check(y.bYieldCountCheckTick == false, "2 more fails: no tick");
        SetFails(4, 2);
        y.UpdateYieldCountCheckTick();
        check(y.bYieldCountCheckTick == true && y.ulYieldFailBase == 6, "3 more fails: a tick, base 6");
        SetFails(1, 0);
        y.UpdateYieldCountCheckTick();
        check(y.bYieldCountCheckTick == false && y.ulYieldFailBase == 1, "fewer fails than the base (ClearYieldCount / lot change): re-base to 1, no tick (:6120-6121)");
        SetFails(2, 2);
        y.UpdateYieldCountCheckTick();
        check(y.bYieldCountCheckTick == true && y.ulYieldFailBase == 4, "3 new fails after the re-base: a tick");

        struct Guard { const char* what; int which; } guards[] = {
            { "SystemStart==false", 0 }, { "the Contact page shown", 1 }, { "iHome!=0", 2 }, { "bRunAutoClean", 3 },
            { "bZ1PickShuttle", 4 }, { "bZ2PickShuttle", 5 }, { "LastSet.iRunStartMode==rsmAutoSiteMap", 6 } };
        unsigned f = 4;
        for (size_t g = 0; g < sizeof(guards) / sizeof(guards[0]); ++g) {
            const unsigned long base = y.ulYieldFailBase;
            switch (guards[g].which) {
                case 0: SystemStart = false; break;
                case 1: fContact->fShow = true; break;
                case 2: iHome = 1; break;
                case 3: bRunAutoClean = true; break;
                case 4: bZ1PickShuttle = true; break;
                case 5: bZ2PickShuttle = true; break;
                default: LastSet.iRunStartMode = rsmAutoSiteMap; break;
            }
            f += 5;
            SetFails(f, 0);
            y.UpdateYieldCountCheckTick();
            const bool frozen = y.bYieldCountCheckTick == false && y.ulYieldFailBase == base;
            ResetCommonGuards();
            y.UpdateYieldCountCheckTick();
            const bool resumed = y.bYieldCountCheckTick == true && y.ulYieldFailBase == f;
            std::string msg = std::string(guards[g].what) + ": no tick and the base frozen; once it clears the tick comes (恢復後補檢查, :6113-6116)";
            check(frozen && resumed, msg.c_str());
        }
        TestIF_File.iYieldAlarmCheckIntervalByCount = 0;
        SetFails(f + 50, 0);
        y.UpdateYieldCountCheckTick();
        check(y.bYieldCountCheckTick == false, "N=0: never a tick (golden :6110 防呆)");
        TestIF_File.iYieldAlarmCheckIntervalByCount = 3;
    }

    // ---- 2. the 913 fix on CheckLowYieldAlarm ----------------------------------
    {
        TfYieldMonitoring y;
        ResetCommonGuards();
        CosFunction.bLowYieldAlarmIntervalTimeBySetting = false;   // the interval is 60 calls (:1090-1091); this part makes 5
        CosFunction.bYieldAlarmCheckByCount = true;
        CosFunction.bYieldAlarmNoWait1Min = true;
        SeedLowYield();
        y.bYieldCountCheckTick = false;
        y.CheckLowYieldAlarm();
        check(!AlarmRan(), "by-count on, no tick: CheckLowYieldAlarm leaves even with \"no wait 1 min\" (golden 913 :4606-4610)");

        CosFunction.bYieldAlarmNoWait1Min = false;
        SeedLowYield();
        y.bYieldCountCheckTick = true;
        y.CheckLowYieldAlarm();
        check(AlarmRan(), "by-count on + tick, inside the 60 s interval: the check body runs and raises the low-yield alarm (the 913 fix :4612-4614)");

        CosFunction.bYieldAlarmCheckByCount = false;
        SeedLowYield();
        y.bYieldCountCheckTick = true;
        y.CheckLowYieldAlarm();
        check(!AlarmRan(), "by-count off: the tick means nothing, the 60 s time gate holds as today");
        CosFunction.bYieldAlarmNoWait1Min = true;
        SeedLowYield();
        y.CheckLowYieldAlarm();
        check(AlarmRan(), "by-count off + \"no wait 1 min\": the alarm as today");
        CosFunction.bYieldAlarmNoWait1Min = false;
    }

    // ---- 3. only CC_QUALCOMM turns it on ---------------------------------------
    CUSTOMER_CODE = CC_QUALCOMM;
    CosFunction.bYieldAlarmCheckByCount = false;
    InitialCosFunction();
    check(CosFunction.bYieldAlarmCheckByCount == true, "InitialCosFunction with CC_QUALCOMM: by-count on (golden CosFunction.cpp:2626)");
    CUSTOMER_CODE = CC_PTI;
    InitialCosFunction();
    check(CosFunction.bYieldAlarmCheckByCount == false, "InitialCosFunction with CC_PTI (HT9050): by-count off again (the default, golden :4583)");
    CUSTOMER_CODE = 0;

    // ---- 4. source pins ---------------------------------------------------------
    if (argc > 1) {
        const std::string src = argv[1];
        const std::string t3 = Read(src + "/MainTimer3.cpp");
        const size_t tick = t3.find("\n    fYieldMonitoring->UpdateYieldCountCheckTick();");
        const size_t first = t3.find("\n    fYieldMonitoring->CheckBySiteYieldAlarm();");
        const size_t last = t3.find("\n    fYieldMonitoring->CheckByPickerYieldAlarm();");
        check(Count(t3, "fYieldMonitoring->UpdateYieldCountCheckTick();") == 1 && tick != std::string::npos && first != std::string::npos &&
              last != std::string::npos && tick < first && first < last && Count(t3.substr(tick + 1, first - tick - 1), "\n") == 0,
              "MainTimer3.cpp: the tick once, on the line right before CheckBySiteYieldAlarm, ahead of all 8 checks (golden 913 main.cpp:26378)");

        const std::string ym = Read(src + "/uYieldMonitoring.cpp");
        const char* d1[] = { "void TfYieldMonitoring::CheckBySiteYieldAlarm()", "void TfYieldMonitoring::CheckByPickerYieldAlarm()" };
        for (size_t i = 0; i < 2; ++i) {
            const std::string b = Body(ym, d1[i]);
            check(Count(b, "if(CosFunction.bYieldAlarmCheckByCount) { if(bYieldCountCheckTick==false) return; } else if(iCount2>=iAlarmTimeInterval ||") == 1,
                  i == 0 ? "CheckBySiteYieldAlarm: the 0814 gate (golden 913 :3872-3885)" : "CheckByPickerYieldAlarm: the 0814 gate (golden 913 :4103-4116)");
        }
        struct D2 { const char* fn; const char* head; const char* what; } d2[] = {
            { "void TfYieldMonitoring::CheckBySiteByArmYieldAlarm()", "if(CosFunction.bYieldAlarmCheckByCount || iCount3>=iAlarmTimeInterval ||", "CheckBySiteByArmYieldAlarm (golden 913 :4344-4352)" },
            { "void TfYieldMonitoring::CheckLowYieldAlarm()", "if(CosFunction.bYieldAlarmCheckByCount || iCount2>=iAlarmTimeInterval ||", "CheckLowYieldAlarm (:4606-4614)" },
            { "void TfYieldMonitoring::CheckLowYieldAlarmByTotal()", "if(CosFunction.bYieldAlarmCheckByCount || iCount4>=iAlarmTimeInterval ||", "CheckLowYieldAlarmByTotal (:5054-5062)" },
            { "void TfYieldMonitoring::CheckIntervalLowYieldAlarmBySite()", "if(CosFunction.bYieldAlarmCheckByCount || iCount5>=iAlarmTimeInterval ||", "CheckIntervalLowYieldAlarmBySite (:5586-5594)" },
            { "void TfYieldMonitoring::CheckIntervalLowYieldAlarmByTotal()", "if(CosFunction.bYieldAlarmCheckByCount || iCount6>=iAlarmTimeInterval ||", "CheckIntervalLowYieldAlarmByTotal (:5730-5738)" } };
        for (size_t i = 0; i < sizeof(d2) / sizeof(d2[0]); ++i) {
            const std::string b = Body(ym, d2[i].fn);
            const size_t ret = b.find("if(CosFunction.bYieldAlarmCheckByCount && bYieldCountCheckTick==false) return;");
            const size_t head = b.find(d2[i].head);
            std::string msg = std::string(d2[i].what) + ": the early return, then the || head (the 913 else-if fix)";
            check(ret != std::string::npos && head != std::string::npos && ret < head && Count(b, "bYieldAlarmCheckByCount") == 2, msg.c_str());
        }
    } else
        check(argc > 1, "argv[1] = the tree root (part 4)");

    if (g_fail == 0 && root.compare(0, tmpRoot.size(), tmpRoot) == 0)
        RemoveTree(root);
    std::printf("test_st02_w195_qcom: %d/%d passed%s\n", g_total - g_fail, g_total, g_fail ? (" (files kept under " + root + ")").c_str() : "");
    return g_fail == 0 ? 0 : 1;
}
