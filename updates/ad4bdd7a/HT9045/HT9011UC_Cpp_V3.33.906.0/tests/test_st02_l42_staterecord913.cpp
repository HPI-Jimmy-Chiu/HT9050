// =============================================================================
//  test_st02_l42_staterecord913.cpp -- card L42 (ST02_V912_VS_V913 s3 L42): golden 913 State Record diagnostics
//  (RogerYang 20260914 / 20260915 / 20260922, 偉測 HHT-17): bodies in StateRecord_L42_St02.cpp, called on single lines of
//  cStateRecord.cpp (the laptop's file; its line count is unchanged).
//
//  AI(W906-L42) 20261010 (St02).  Suite name (add_test): St02_L42StateRecord913.  argv[1] = the port tree (read-only source pins).
//    [1] W906_L42_SaveMachineMaterial (golden 913 main.cpp:27706-27727): MachineMaterial.txt in the given folder, exactly the
//        five golden lines -- SystemStart / HasICUnderMachine + detail / HasAnyICInMachine + detail -- for the idle state
//        and for a seeded one (SystemStart, a tray on Tray Z);
//    [2] TfMain::SaveTaskList -> Task_ListWithTime.csv: the Pattern #31 rows (golden 913 :6835-6863) right after
//        bCheckShuttle2Flag and right before b1ShuttleMoveToLeft, with seeded values; no ShtChkFlagEscapeCT row (gated);
//        FLCarryKit / BLCarryKit / TestSocket dumps (:7079-7116) between BTestSuck and CleanKitTime; the ATK rows (:7140-7166)
//        only on a seeded ATK machine (fAGV->IsATK_AMR()), the TrayArm row right before MainProcMonitor, the gated two absent;
//    [3] TfMain::SaveDecisionVariables -> DecisionVariables.csv: section 5b (:7310-7427, ASM arm selection / dispatch cursor
//        / dispatched table / hot-plate ledger) between sections 5 and 6, every row with seeded values; the hang-up watchdog
//        rows (:7474-7484) between bRunAutoClean and section 10;
//    [S] source pins: the MachineMaterial call sits on the DumpMainFormSnapshot line of W906_DoStateRecordBody, live once,
//        before the background zip job (W906_StateRecordWorkerBusy.store(true)); each MR-b call live once inside its golden
//        function of cStateRecord.cpp, which defines no W906_L42_* body; bodies + golden 913 citations in StateRecord_L42_St02.cpp
//        (on the cStateRecord.cpp line of the root CMakeLists.txt); the rows whose globals are not in the port stay behind
//        GATE(W906-L42-1..3).
//  DoStateRecord itself is not run (it writes D:\HT9045_StateRecord and starts 7z); the helper gets a sandbox folder.
//  Writes only a fresh l42_<tick> folder in ctest's log-root sandbox (W906_HT9045LOG_ROOT; removed when green); refuses a
//  root under D:\HT9045 (incl. D:\HT9045_Log) unless it is a build dir (\obj\v906\).  No card, no COM port, no network.
// =============================================================================
#include "forms/fMain.h"
#include "vclcompat/vcl_compat.h"
#include "cmydef.h"
#include "cprod.h"
#include "common.h"
#include "csystem.h"
#include "Motor/mymotor.h"
#include "Config.h"                 // IniConfig.bA65_BundleIDList
#include "forms/fAGV.h"             // fAGV->IsATK_AMR()
#include "aHotPlateSubstrate.h"     // FLCarryKit / BLCarryKit / TestSocket / CatchTraySuck / InArmSiteMapData / iPlacePlate / PickFromHPList (as cStateRecord.cpp)
#include "acarry.h"                 // b1ShuttleMoveToRight / b2ShuttleMoveToRight
#include "atester.h"                // HangTime
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "StateRecord_L42_St02.h"                       // W906_L42_SaveMachineMaterial (StateRecord_L42_St02.cpp)
extern bool IndexZCanMove[2];                            // golden ainarm2.h:48 (port: ainarm9045_w7_shims.h:58)

namespace {
int g_pass = 0, g_fail = 0;
void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}
std::string Lower(std::string s)
{
    for (std::size_t i = 0; i < s.size(); ++i) { if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a'); if (s[i] == '/') s[i] = '\\'; }
    return s;
}
bool UnderMachineTree(const std::string& p)
{
    const std::string s = Lower(p);
    if (s.find("\\obj\\v906\\") != std::string::npos) return false;   // a build dir under the main checkout is fine
    return s.compare(0, 9, "d:\\ht9045") == 0;                       // D:\HT9045, D:\HT9045_Log, D:\HT9045_StateRecord ...
}
bool ReadAll(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    *out = ss.str();
    return true;
}
bool Exists(const std::string& p) { return ::GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES; }
std::vector<std::string> Lines(const std::string& text)
{
    std::vector<std::string> v;
    std::istringstream in(text);
    std::string l;
    while (std::getline(in, l)) { if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1); v.push_back(l); }
    return v;
}
std::vector<std::string> FileLines(const std::string& p)
{
    std::string body;
    if (!ReadAll(p, &body)) return std::vector<std::string>();
    return Lines(body);
}
// live code of a line: no `#if 0 ... #endif` block (nesting tracked), no // comment (outside strings)
std::vector<std::string> LiveCode(const std::vector<std::string>& v)
{
    std::vector<std::string> out(v.size());
    std::vector<int> st;   // 1 = a dead #if 0 frame
    for (std::size_t i = 0; i < v.size(); ++i) {
        std::string t = v[i];
        std::size_t a = t.find_first_not_of(" \t");
        const std::string d = a == std::string::npos ? "" : t.substr(a);
        const bool dead = !st.empty() && st.back() == 1;
        if (d.compare(0, 3, "#if") == 0) { st.push_back(dead || d.compare(0, 5, "#if 0") == 0 ? 1 : 0); continue; }
        if (d.compare(0, 6, "#endif") == 0) { if (!st.empty()) st.pop_back(); continue; }
        if (d.compare(0, 5, "#else") == 0 || d.compare(0, 5, "#elif") == 0) {
            if (!st.empty()) { const bool parentDead = st.size() > 1 && st[st.size() - 2] == 1; st.back() = parentDead ? 1 : (st.back() == 1 ? 0 : 1); }
            continue;
        }
        if (dead) continue;
        bool inStr = false;
        for (std::size_t k = 0; k + 1 < t.size(); ++k) {
            if (t[k] == '"' && (k == 0 || t[k - 1] != '\\')) inStr = !inStr;
            if (!inStr && t[k] == '/' && t[k + 1] == '/') { t = t.substr(0, k); break; }
        }
        out[i] = t;
    }
    return out;
}
int LiveCount(const std::vector<std::string>& live, const std::string& needle)
{
    int n = 0;
    for (const std::string& l : live) if (l.find(needle) != std::string::npos) ++n;
    return n;
}
// 1-based line of the first live line holding `a` (and `b` when given), 0 = none
int LiveLine(const std::vector<std::string>& live, const std::string& a, const std::string& b = std::string())
{
    for (std::size_t i = 0; i < live.size(); ++i)
        if (live[i].find(a) != std::string::npos && (b.empty() || live[i].find(b) != std::string::npos)) return (int)i + 1;
    return 0;
}
void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            const std::string n = fd.cFileName;
            if (n == "." || n == "..") continue;
            const std::string p = dir + "\\" + n;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p); else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}
std::string TF(bool b) { return b ? "true" : "false"; }
std::string Zeros(int n) { std::string s; for (int i = 0; i < n; ++i) s += "0,"; return s; }
int Find(const std::vector<std::string>& v, const std::string& needle)        // first line holding needle, -1 = none
{
    for (std::size_t i = 0; i < v.size(); ++i) if (v[i].find(needle) != std::string::npos) return (int)i;
    return -1;
}
int FindExact(const std::vector<std::string>& v, const std::string& line)
{
    for (std::size_t i = 0; i < v.size(); ++i) if (v[i] == line) return (int)i;
    return -1;
}
std::vector<std::string> Slice(const std::vector<std::string>& v, int from, int n)
{
    std::vector<std::string> s;
    for (int i = from; i >= 0 && i < from + n && (std::size_t)i < v.size(); ++i) s.push_back(v[(std::size_t)i]);
    return s;
}
std::string Join(const std::vector<std::string>& v)
{
    std::string s;
    for (const std::string& l : v) s += "      | " + l + "\n";
    return s;
}
}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_L42StateRecord913 -- card L42: golden 913 State Record diagnostics (MachineMaterial.txt, Task_ListWithTime.csv, DecisionVariables.csv)\n");
    const std::string src = argc > 1 ? argv[1] : "";
    if (!std::getenv("W906_HT9045LOG_ROOT") || as9045LogPath.IsEmpty() || UnderMachineTree(as9045LogPath.c_str())) {
        std::printf("  ABORT: as9045LogPath = %s is not a sandbox (W906_HT9045LOG_ROOT %s) -- nothing was called\n",
                    as9045LogPath.c_str(), std::getenv("W906_HT9045LOG_ROOT") ? "set" : "not set (run under ctest)");
        return 2;
    }
    // a fresh folder per run under ctest's log root: a re-run (or the reverse round) must not see the last run's files
    const std::string root = std::string(as9045LogPath.c_str()) + "\\l42_" + std::to_string((unsigned long)::GetTickCount());
    if (UnderMachineTree(root)) { std::printf("  ABORT: %s is under D:\\HT9045 -- nothing was called\n", root.c_str()); return 2; }
    ::CreateDirectoryA(root.c_str(), 0);
    if (!Exists(root)) { std::printf("  ABORT: could not create %s\n", root.c_str()); return 2; }

    // ---------------------------------------------------------------- [1]
    std::printf("[1] W906_L42_SaveMachineMaterial (golden 913 main.cpp:27706-27727)\n");
    {
        const std::string d1 = root + "\\idle";
        ::CreateDirectoryA(d1.c_str(), 0);
        SystemStart = false;
        W906_L42_SaveMachineMaterial(d1.c_str());
        const std::vector<std::string> got = FileLines(d1 + "\\MachineMaterial.txt");
        std::vector<std::string> want;
        want.push_back("SystemStart        = false");
        want.push_back("HasICUnderMachine  = " + TF(HasICUnderMachine()));
        want.push_back("    Detail         : " + std::string(sHasICUnderMachine().c_str()));
        want.push_back("HasAnyICInMachine  = " + TF(HasAnyICInMachine()));
        want.push_back("    Detail         : " + std::string(sHasAnyICInMachine().c_str()));
        Check(Exists(d1 + "\\MachineMaterial.txt"), "[1] idle: MachineMaterial.txt written into the given folder");
        Check(got == want, "[1] idle: exactly the five golden lines, golden order\n" + Join(got));

        const std::string d2 = root + "\\seeded";
        ::CreateDirectoryA(d2.c_str(), 0);
        const bool trayZ = MOT[MMTrayZ].fHasTray;
        const std::string before = sHasICUnderMachine().c_str();
        SystemStart = true;
        MOT[MMTrayZ].fHasTray = true;                                // golden sHasICUnderMachine: "Tray Z, " first
        W906_L42_SaveMachineMaterial(d2.c_str());
        const std::vector<std::string> g2 = FileLines(d2 + "\\MachineMaterial.txt");
        Check(g2.size() == 5, "[1] seeded: five lines (" + std::to_string(g2.size()) + ")\n" + Join(g2));
        Check(g2.size() > 0 && g2[0] == "SystemStart        = true", "[1] seeded: SystemStart        = true");
        Check(g2.size() > 1 && g2[1] == "HasICUnderMachine  = true", "[1] seeded: HasICUnderMachine  = true (a tray on Tray Z)");
        Check(g2.size() > 2 && g2[2] == "    Detail         : Tray Z, " + before, "[1] seeded: the detail line names Tray Z first (golden csystem.cpp sHasICUnderMachine)");
        Check(g2.size() > 4 && g2[3] == "HasAnyICInMachine  = " + TF(HasAnyICInMachine()) &&
              g2[4] == "    Detail         : " + std::string(sHasAnyICInMachine().c_str()), "[1] seeded: HasAnyICInMachine + its detail");
        MOT[MMTrayZ].fHasTray = trayZ;
        SystemStart = false;
        Check(!Exists(root + "\\MachineMaterial.txt"), "[1] nothing written outside the folder it was given");
    }

    // ---------------------------------------------------------------- [2]
    std::printf("[2] TfMain::SaveTaskList (golden 913 main.cpp:6835-6863 / :7079-7116 / :7140-7166)\n");
    {
        Check(fMain != nullptr, "[2] fMain exists");
        b1ShuttleMoveToRight = true;
        b2ShuttleMoveToRight = false;
        IndexZCanMove[0] = true;
        IndexZCanMove[1] = false;
        Prod.TestZ1_Safe = 1111;
        Prod.TestZ2_Safe = 2222;
        MOT[MTestZ1].MovFlag = true;  MOT[MTestZ1].Led[iHomeLed] = false; MOT[MTestZ1].Led[iInposLed] = true;
        MOT[MTestZ2].MovFlag = false; MOT[MTestZ2].Led[iHomeLed] = true;  MOT[MTestZ2].Led[iInposLed] = false;
        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 8; ++j) {
                FLCarryKit.Item[i][j] = i * 10 + j;
                BLCarryKit.Item[i][j] = 100 + i * 10 + j;
                TestSocket.Item[i][j] = 200 + i * 10 + j;
            }
        const std::string p1 = std::to_string((int)MOT[MTestZ1].Gali_ReadPos());   // offline: Motor==NULL -> 0 (myGALILmotor.cpp)
        const std::string p2 = std::to_string((int)MOT[MTestZ2].Gali_ReadPos());

        const std::string d = root + "\\tasklist";
        ::CreateDirectoryA(d.c_str(), 0);
        fMain->SaveTaskList(d.c_str());
        const std::vector<std::string> t = FileLines(d + "\\Task_ListWithTime.csv");
        Check(!t.empty(), "[2] Task_ListWithTime.csv written (" + std::to_string(t.size()) + " lines)");

        // Pattern #31: right after bCheckShuttle2Flag, right before b1ShuttleMoveToLeft; the gated counter row is not there
        const int k = Find(t, "bCheckShuttle2Flag, ");
        std::vector<std::string> w31;
        w31.push_back("b1ShuttleMoveToRight, true");
        w31.push_back("b2ShuttleMoveToRight, false");
        w31.push_back("IndexZCanMove[0]/[1], 1 / 0");
        w31.push_back("MTestZ1 MovFlag/ReadPos/Safe/HomeLed/InPosLed, 1 / " + p1 + " / 1111 / 0 / 1");
        w31.push_back("MTestZ2 MovFlag/ReadPos/Safe/HomeLed/InPosLed, 0 / " + p2 + " / 2222 / 1 / 0");
        Check(k >= 0 && Slice(t, k + 1, 5) == w31, "[2] Pattern #31: the five golden rows right after bCheckShuttle2Flag\n" + Join(Slice(t, k + 1, 5)));
        Check(k >= 0 && (std::size_t)(k + 6) < t.size() && t[(std::size_t)k + 6].compare(0, 21, "b1ShuttleMoveToLeft, ") == 0,
              "[2] Pattern #31: then b1ShuttleMoveToLeft (golden order; the ShtChkFlagEscapeCT row is gated, GATE(W906-L42-1))");
        Check(Find(t, "ShtChkFlagEscapeCT") < 0, "[2] no ShtChkFlagEscapeCT row (iShtChkFlagEscapeCT is not in the port -- not invented)");

        // In Shuttle / TestSocket: after the two BTestSuck rows, before CleanKitTime
        const int b = FindExact(t, "BTestSuck");
        std::vector<std::string> wd;
        const char* const names[3] = {"FLCarryKit", "BLCarryKit", "TestSocket"};
        for (int n = 0; n < 3; ++n) {
            wd.push_back(names[n]);
            for (int i = 0; i < 2; ++i) {
                std::string row;
                for (int j = 0; j < 8; ++j) row += std::to_string(n * 100 + i * 10 + j) + ",";
                wd.push_back(row);
            }
        }
        Check(b >= 0 && Slice(t, b + 3, 9) == wd, "[2] FLCarryKit / BLCarryKit / TestSocket dumps right after BTestSuck\n" + Join(Slice(t, b + 3, 9)));
        Check(b >= 0 && (std::size_t)(b + 12) < t.size() && t[(std::size_t)b + 12] == "CleanKitTime", "[2] ... then CleanKitTime (golden order)");

        // ATK: golden's customer condition -- not ATK -> nothing; ATK -> the TrayArm row right before MainProcMonitor
        const int m = Find(t, "MainProcMonitor, ");
        Check(m > 0 && Find(t, "ATKFixFull") < 0, "[2] not an ATK machine (fAGV->IsATK_AMR() false): no ATKFixFull row");
        Check(fAGV != nullptr, "[2] fAGV exists");
        if (fAGV != nullptr) {
            const int cc = CUSTOMER_CODE, cover = USE_COVER_TRAYID;
            const bool bundle = IniConfig.bA65_BundleIDList;
            CUSTOMER_CODE = CC_AMKOR_Korea;
            USE_COVER_TRAYID = tCID_NFC;
            IniConfig.bA65_BundleIDList = true;
            const bool trayX = MOT[MTrayX].fHasTray;
            MOT[MTrayX].fHasTray = true;
            CatchTraySuck.iWhichTray = 4;
            CatchTraySuck.iWhichKit = 1;
            Check(fAGV->IsATK_AMR(), "[2] seeded ATK: CUSTOMER_CODE 971 + NFC cover tray ID + [A65] -> fAGV->IsATK_AMR()");
            const std::string da = root + "\\tasklist_atk";
            ::CreateDirectoryA(da.c_str(), 0);
            fMain->SaveTaskList(da.c_str());
            const std::vector<std::string> ta = FileLines(da + "\\Task_ListWithTime.csv");
            const int ma = Find(ta, "MainProcMonitor, ");
            Check(ma > 0 && ta[(std::size_t)ma - 1] == "ATKFixFull TrayArm, MOT[MTrayX].fHasTray=true, CatchTraySuck.iWhichTray=4, iWhichKit=1",
                  "[2] ATK: the TrayArm row right before MainProcMonitor (golden order)\n" + Join(Slice(ta, ma - 1, 1)));
            Check(Find(ta, "ATKFixFull, Task=") < 0 && Find(ta, "ATKFixFull Auto") < 0,
                  "[2] ATK: the Fix-Full task / per-Auto rows stay gated (GATE(W906-L42-2) / (W906-L42-3): their globals are not in the port)");
            CUSTOMER_CODE = cc;
            USE_COVER_TRAYID = cover;
            IniConfig.bA65_BundleIDList = bundle;
            MOT[MTrayX].fHasTray = trayX;
        }
    }

    // ---------------------------------------------------------------- [3]
    std::printf("[3] TfMain::SaveDecisionVariables (golden 913 main.cpp:7310-7427 / :7474-7484)\n");
    {
        iAutoSiteMapHPToSht = 1;
        for (int a = 0; a < 2; ++a)
            for (int i = 0; i < 2; ++i)
                for (int j = 0; j < 8; ++j) { Prod.fNeedToCheckASM[a][i][j] = false; Prod.fInArmSuck4x8[a][i][j] = false; Prod.fAlreadyCheckASM[a][i][j] = false; }
        Prod.fNeedToCheckASM[1][0][2] = true;
        Prod.fInArmSuck4x8[0][1][7] = true;
        Prod.fAlreadyCheckASM[0][1][3] = true;
        bAutoSiteMapWaitTestResult = true;
        bRunAutoSiteMapping = false;
        bSiteMappingCHKOK = true;
        iDoSiteMappingStep = 3;
        iAutoSiteCurrStep = 7;
        iAutoSiteMapInArmRow = 1; iAutoSiteMapInArmCol = 2; iAutoSiteMapInArmCol_8 = 3;
        iAutoSiteMapCount = 4;
        iAutoSiteMapHPToKit = 5;
        iAutoSiteMapSiteNo = 6;
        iResetSiteMappingStep = 8;
        Prod.bInitialAutoSiteMap = true;
        InArmSiteMapData.iP = 11; InArmSiteMapData.iPlateR = 12; InArmSiteMapData.iPlateC = 13; InArmSiteMapData.iSuckR = 14; InArmSiteMapData.iSuckC = 15;
        bAutoSiteMapHasPickHP = true;
        bAutoSiteMapHotplateSave = false;
        bAutoSiteMapHotplateReady = true;
        iPlacePlate[0] = 21; iPlacePlateX[0] = 22; iPlacePlateY[0] = 23;
        iPickPlate[0] = 24;  iPickPlateX[0] = 25;  iPickPlateY[0] = 26;
        bHangTimePause = true;
        TestISTimeOut = false;
        Prod.iHangupMaxTime = 600;

        const std::string d = root + "\\decision";
        ::CreateDirectoryA(d.c_str(), 0);
        fMain->SaveDecisionVariables(d.c_str());
        const std::vector<std::string> v = FileLines(d + "\\DecisionVariables.csv");
        Check(!v.empty(), "[3] DecisionVariables.csv written (" + std::to_string(v.size()) + " lines)");

        std::vector<std::string> w;
        const std::string z16 = Zeros(16);
        w.push_back("# === ASM Arm Selection ===");
        w.push_back("iAutoSiteMapHPToSht, 1");
        w.push_back("Prod.fNeedToCheckASM (1=需做 ASM)");
        w.push_back("  Arm1:" + z16);
        w.push_back("  Arm2:0,0,1," + Zeros(13));
        w.push_back("Prod.fInArmSuck4x8 (1=使用該站)");
        w.push_back("  Arm1:" + Zeros(15) + "1,");
        w.push_back("  Arm2:" + z16);
        w.push_back("bAutoSiteMapWaitTestResult, true");
        w.push_back("bRunAutoSiteMapping, false");
        w.push_back("bSiteMappingCHKOK, true");
        w.push_back("iDoSiteMappingStep, 3");
        w.push_back("iAutoSiteCurrStep, 7");
        w.push_back("iAutoSiteMapInArmRow/Col/Col_8, 1 / 2 / 3");
        w.push_back("iAutoSiteMapCount, 4");
        w.push_back("iAutoSiteMapHPToKit, 5");
        w.push_back("iAutoSiteMapSiteNo, 6");
        w.push_back("iResetSiteMappingStep, 8");
        w.push_back("Prod.bInitialAutoSiteMap, true");
        w.push_back("InArmSiteMapData P/R/C/SuckR/SuckC, 11 / 12 / 13 / 14 / 15");
        w.push_back("Prod.fAlreadyCheckASM (1=本輪已派工)");
        w.push_back("  Arm1:" + Zeros(11) + "1," + Zeros(4));
        w.push_back("  Arm2:" + z16);
        w.push_back("bAutoSiteMapHasPickHP, true");
        w.push_back("bAutoSiteMapHotplateSave, false");
        w.push_back("bAutoSiteMapHotplateReady, true");
        w.push_back("iPlacePlate[0]/X/Y, 21 / 22 / 23");
        w.push_back("iPickPlate[0]/X/Y, 24 / 25 / 26");
        w.push_back("PickFromHPList group count, " + std::to_string((PickFromHPList != NULL) ? PickFromHPList->GetHPSuckGroupCount() : -1));
        w.push_back("HP RealIC(HowManyIC) P0/P1, " + std::to_string(MOT[MMPlate1].Tray.HowManyIC()) + " / " + std::to_string(MOT[MMPlate2].Tray.HowManyIC()));
        w.push_back("HP HasIC(含HAS_NULL_IC佔位) P0/P1, " + TF(MOT[MMPlate1].Tray.HasIC()) + " / " + TF(MOT[MMPlate2].Tray.HasIC()));
        w.push_back("");
        const int a = FindExact(v, "# === ASM Arm Selection ===");
        Check(a >= 2 && v[(std::size_t)a - 2].compare(0, 25, "TestIF_File.iShuttle_Sel,") == 0 && v[(std::size_t)a - 1].empty(),
              "[3] 5b follows section 5 (TestIF_File.iShuttle_Sel, then its blank line)");
        Check(a >= 0 && Slice(v, a, (int)w.size()) == w, "[3] 5b ASM: the golden 913 rows, golden order\n" + Join(Slice(v, a, (int)w.size())));
        Check(a >= 0 && (std::size_t)a + w.size() < v.size() && v[(std::size_t)a + w.size()] == "# === Customer Code ===", "[3] ... then section 6 (Customer Code)");

        const int r = Find(v, "bRunAutoClean, ");
        Check(r >= 0 && Slice(v, r + 1, 2) == std::vector<std::string>({"bHangTimePause, true", "TestISTimeOut, false"}),
              "[3] HangTime: bHangTimePause / TestISTimeOut right after bRunAutoClean\n" + Join(Slice(v, r + 1, 5)));
        Check(r >= 0 && (std::size_t)(r + 3) < v.size() && (v[(std::size_t)r + 3] == "HangTime.Off(), true" || v[(std::size_t)r + 3] == "HangTime.Off(), false"),
              "[3] HangTime: HangTime.Off() row");
        Check(r >= 0 && Slice(v, r + 4, 3) == std::vector<std::string>({"Prod.iHangupMaxTime, 600", "", "# === Task State Snapshot ==="}),
              "[3] HangTime: Prod.iHangupMaxTime (%.0f), then the blank line and section 10 (golden order)");
    }

    // ---------------------------------------------------------------- [S]
    std::printf("[S] source pins (cStateRecord.cpp / StateRecord_L42_St02.cpp / CMakeLists.txt, read only)\n");
    {
        std::string text, ltext, cmtext;
        const bool r = !src.empty() && ReadAll(src + "\\cStateRecord.cpp", &text) && ReadAll(src + "\\StateRecord_L42_St02.cpp", &ltext) &&
                       ReadAll(src + "\\CMakeLists.txt", &cmtext);
        Check(r, "[S] read cStateRecord.cpp, StateRecord_L42_St02.cpp and CMakeLists.txt under " + src);
        const std::vector<std::string> raw = Lines(text);
        const std::vector<std::string> live = LiveCode(raw);
        const std::vector<std::string> llive = LiveCode(Lines(ltext));
        bool onLine = false;   // the new TU is built with cStateRecord.cpp: same library line (code part, before the #)
        for (const std::string& l : Lines(cmtext)) {
            const std::string code = l.substr(0, l.find('#'));
            if (code.find("cStateRecord.cpp") != std::string::npos && code.find("StateRecord_L42_St02.cpp") != std::string::npos) onLine = true;
        }
        Check(onLine, "[S] CMakeLists.txt: StateRecord_L42_St02.cpp on the cStateRecord.cpp line (ht9045_sm)");
        Check(LiveLine(live, "#include \"StateRecord_L42_St02.h\"") != 0 && LiveLine(live, "void W906_L42_") == 0,
              "[S] cStateRecord.cpp includes StateRecord_L42_St02.h and defines no W906_L42_* body (same-line calls only)");
        const int call = LiveLine(live, "DumpMainFormSnapshot(NewPath);", "W906_L42_SaveMachineMaterial(NewPath);");
        const int job = LiveLine(live, "W906_StateRecordWorkerBusy.store(true);");
        const int body = LiveLine(live, "void TfMain::W906_DoStateRecordBody(");
        Check(LiveCount(live, "W906_L42_SaveMachineMaterial(NewPath);") == 1, "[S] one live call of W906_L42_SaveMachineMaterial(NewPath)");
        Check(call != 0 && body != 0 && job != 0 && body < call && call < job,
              "[S] it sits on the DumpMainFormSnapshot line of W906_DoStateRecordBody (:" + std::to_string(call) +
              "), before the zip job (:" + std::to_string(job) + ")");
        Check(call != 0 && live[(std::size_t)call - 1].find("DumpMainFormSnapshot(NewPath);") < live[(std::size_t)call - 1].find("W906_L42_SaveMachineMaterial(NewPath);"),
              "[S] ... after golden's DumpMainFormSnapshot (golden 913 order)");
        Check(LiveLine(llive, "void W906_L42_SaveMachineMaterial(AnsiString NewPath)") != 0 &&
              ltext.find("// golden 913 main.cpp:27706-27727 (L42)") != std::string::npos, "[S] StateRecord_L42_St02.cpp: the body and its golden 913 citation");
        Check(LiveCount(llive, "MachineMaterial.txt") == 1 && LiveCount(live, "MachineMaterial.txt") == 0,
              "[S] MachineMaterial.txt written in one place, into NewPath");
        // MR-b: each golden 913 block is called from one line inside its golden function
        const int fTask = LiveLine(live, "void TfMain::SaveTaskList(");
        const int fDec = LiveLine(live, "void TfMain::SaveDecisionVariables(");
        const int fNext = LiveLine(live, "void TfMain::ProcessTimeUpdate(");
        struct Site { const char* call; const char* body; const char* cite; int lo, hi; };
        const Site sites[5] = {
            {"W906_L42_TaskListPattern31(sList);",       "void W906_L42_TaskListPattern31(TStringList *sList)",       "// golden 913 main.cpp:6835-6863 (L42)", fTask, fDec},
            {"W906_L42_TaskListInShuttleSocket(sList);", "void W906_L42_TaskListInShuttleSocket(TStringList *sList)", "// golden 913 main.cpp:7079-7116 (L42)", fTask, fDec},
            {"W906_L42_TaskListATKFixFull(sList);",      "void W906_L42_TaskListATKFixFull(TStringList *sList)",      "// golden 913 main.cpp:7140-7166 (L42)", fTask, fDec},
            {"W906_L42_DecisionASM(sList);",             "void W906_L42_DecisionASM(TStringList *sList)",             "// golden 913 main.cpp:7310-7427 (L42)", fDec, fNext},
            {"W906_L42_DecisionHangTime(sList);",        "void W906_L42_DecisionHangTime(TStringList *sList)",        "// golden 913 main.cpp:7474-7484 (L42)", fDec, fNext},
        };
        for (const Site& s : sites) {
            const int at = LiveLine(live, s.call);
            Check(LiveCount(live, s.call) == 1 && at > s.lo && at < s.hi && s.lo != 0 && s.hi != 0,
                  std::string("[S] ") + s.call + " live once, inside its golden function (:" + std::to_string(at) + ")");
            Check(LiveLine(llive, s.body) != 0 && ltext.find(s.cite) != std::string::npos && LiveLine(live, s.body) == 0,
                  std::string("[S] StateRecord_L42_St02.cpp: ") + s.body + " + its citation (not in cStateRecord.cpp)");
        }
        // the rows whose globals are not in the port stay gated -- not invented
        Check(ltext.find("GATE(W906-L42-1)") != std::string::npos && ltext.find("GATE(W906-L42-2)") != std::string::npos &&
              ltext.find("GATE(W906-L42-3)") != std::string::npos, "[S] the three GATE(W906-L42-n) blocks are there");
        Check(LiveCount(llive, "iShtChkFlagEscapeCT") + LiveCount(live, "iShtChkFlagEscapeCT") == 0 && LiveCount(llive, "iATKFixFullTask") == 0 &&
              LiveCount(llive, "bATKFixFullHold") == 0 && LiveCount(llive, "bAutoWaitAMR") == 0 && LiveCount(llive, "ATKFixFullMinFreeCells") == 0,
              "[S] iShtChkFlagEscapeCT / iATKFixFull* / bAutoWaitAMR* / ATKFixFullMinFreeCells: no live use");
    }

    if (g_fail == 0) RemoveTree(root);
    std::printf("%s: %d / %d checks passed%s\n", g_fail ? "FAIL" : "PASS", g_pass, g_pass + g_fail,
                g_fail ? (" (sandbox kept: " + root + ")").c_str() : "");
    return g_fail ? 1 : 0;
}
