// =============================================================================
//  test_st02_w175_lowyield.cpp -- W-175 LOWYIELD-ALARM: golden fMain->slLowYieldAlarm (the low-yield alarm list) wired as golden.
//
//  AI(W906-W175) 20261008 (St02-E).  Suite name (add_test): St02_W175LowYield.  argv[1] = the port tree (read-only source pins).
//    [1] W906_LowYieldAlarmList() (LogObjects.cpp EOF; golden 0618 main.cpp:1642 `new TStringList()`): non-null, one object, empty.
//    [2] DoLowYieldAlarm (atester_ProcessCount.cpp:302; golden 0618 atester_ProcessCount.cpp:291-300) with
//        CosFunction.bYieldAlmNeedOneCycle: "Code,ErrPart" added once (AnsiPos de-dup), bNeedOneCycleByYieldAlm set.
//    [3] TfMain::BtnOneCycleClick (cCleanOut.cpp:315-321; golden 0618 main.cpp:4349-4352): a pending alarm marks the One Cycle as a
//        yield one (bNeedOneCycleByYieldAlm = true); an empty list leaves it alone.
//    [4] DoOneCycleFinishCheck, both golden branches (0618 csystem.cpp:13265-13273 after an auto-clean One Cycle,
//        :13600-13610 a normal one): every pending alarm is shown -- ShowErrorMessage(code, 0 = a notice, ..., errPart) in list
//        order, golden's parse (incl. SubString(0, ...)) -- and the list is cleared; a second finish shows nothing.
//        Every axis gets a disabled HTMotor (CheckIndexIsNormal: Gali_ReadEncoderInRandge with Enable false -> true; the normal
//        finish calls SetInArmSpeed), both in-shuttles are docked left (InSHT1InLF / InSHT2InLF); else W7_C2_OneCycleFinish's empty machine.
//    [5] source pins: the three gates retired (live code), the csystem macro on the accessor, no live fMain->slLowYieldAlarm /
//        W7C2_fMain_slLowYieldAlarm left.
//  Writes only ctest's redirected roots (RecordProcess / event log of the One Cycle path); refuses to run outside ctest.
// =============================================================================
#include "csystem.h"               // DoOneCycleFinishCheck
#include "aHotPlateSubstrate.h"    // InArmSuck / OutArmSuck / TestSocket
#include "cmydef.h"                // iOneCycle / iOneCycleTask / IndexStatus / CUSTOMER_CODE / bNeedOneCycleByYieldAlm / bIsAutoOneCycle ...
#include "CosFunction.h"           // CosFunction.bYieldAlmNeedOneCycle
#include "LastSet.h"               // LastSet.iRunStartMode
#include "Motor/mymotor.h"         // MOT[], MTestZ1 / MTestZ2 / MTestY1
#include "Motor/HTMotor.h"
#include "forms/fMain.h"           // fMain->BtnOneCycleClick
#include "atester_ProcessCount.h"  // DoLowYieldAlarm
#include "LogObjects.h"            // W906_LowYieldAlarmList
#include "acarry.h"                // b1ShuttleMoveToLeft / b2ShuttleMoveToLeft / bSHTOfsChangeLeft (InSHT1InLF / InSHT2InLF)
#include <windows.h>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "w906_ctest_guard.h"

extern int (*W906_ShowErrorMessage_Hook)(const char* Code, int KCode, int Pos);   // canary_support.h (cannot share a TU with cMyDB.h)
extern AnsiString W906_ShowErrorMessage_LastErrPart;

namespace {
int g_pass = 0, g_fail = 0;
void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}
struct Shown { std::string code, part; int kcode; };
std::vector<Shown> g_shown;
int RecordShow(const char* code, int kcode, int)
{
    Shown s;
    s.code = code ? code : "";
    s.part = W906_ShowErrorMessage_LastErrPart.c_str();   // canary_support.cpp records errPart before it calls the hook
    s.kcode = kcode;
    g_shown.push_back(s);
    return 0;                                              // 0 = "no key": the sim answer (K_RETRY) stands
}
std::string Joined()
{
    std::string r;
    for (const Shown& s : g_shown) r += (r.empty() ? "" : " | ") + s.code + "/" + s.part + "/k" + std::to_string(s.kcode);
    return r.empty() ? "(none)" : r;
}
int CountCode(const char* c)
{
    int n = 0;
    for (const Shown& s : g_shown) if (s.code == c) ++n;
    return n;
}
std::string ListText()
{
    std::string r;
    for (int i = 0; i < W906_LowYieldAlarmList()->Count; ++i)
        r += (i ? " | " : "") + std::string(AnsiString(W906_LowYieldAlarmList()->Strings[i]).c_str());
    return r.empty() ? "(empty)" : r;
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
std::vector<std::string> Lines(const std::string& text)
{
    std::vector<std::string> v;
    std::istringstream in(text);
    std::string l;
    while (std::getline(in, l)) { if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1); v.push_back(l); }
    return v;
}
// live code of a line: no `#if 0 ... #endif` block (nesting tracked), no // comment (outside strings)
std::vector<std::string> LiveCode(const std::vector<std::string>& v)
{
    std::vector<std::string> out(v.size());
    std::vector<int> st;
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

// One finish: the empty machine of W7_C2 plus disabled index motors; DoOneCycleFinishCheck until it finishes (iOneCycle 0).
int RunFinish(bool autoOneCycle, std::string* trace)
{
    InArmSuck.ClearAll();
    OutArmSuck.ClearAll();
    TestSocket.ClearAll();
    IndexStatus = Z1_Z2_Normal;
    CUSTOMER_CODE = 0;
    bResetMode = false;
    bIsAutoOneCycle = autoOneCycle;
    b1ShuttleMoveToLeft = true;  b2ShuttleMoveToLeft = true;            // InSHT1InLF / InSHT2InLF (csystem.cpp:18652 / :18737): docked left,
    bSHTOfsChangeLeft[0] = false;  bSHTOfsChangeLeft[1] = false;        //   no offset change pending, in-position LED low-active
    MOT[MInShuttle1].Led[iInposLed] = false;  MOT[MInShuttle2].Led[iInposLed] = false;
    iOneCycle = 1;
    iOneCycleTask = 0;
    int n = 0;
    for (; n < 60 && iOneCycle != 0; ++n) {
        DoOneCycleFinishCheck();
        if (trace) *trace += (trace->empty() ? "" : ",") + std::to_string(iOneCycleTask);
    }
    return n;
}
}  // namespace

int main(int argc, char** argv)
{
    if (!W906TestRequireCtestRedirects("St02_W175LowYield")) return 2;
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W175LowYield -- W-175: golden slLowYieldAlarm (DoLowYieldAlarm -> BtnOneCycleClick -> DoOneCycleFinishCheck)\n");
    const std::string src = argc > 1 ? argv[1] : "";

    // ---------------------------------------------------------------- [1]
    std::printf("[1] the list\n");
    TStringList* const L = W906_LowYieldAlarmList();
    Check(L != nullptr && L == W906_LowYieldAlarmList() && L->Count == 0, "[1] W906_LowYieldAlarmList(): one object, empty at start");

    const bool saveNeed = CosFunction.bYieldAlmNeedOneCycle, saveDirect = CosFunction.bContinueFailNeedAlarmDirectly;
    const bool saveSmart = CosFunction.bSmartAutoClean, saveHome = fAllMotorHome;
    const int saveRsm = LastSet.iRunStartMode;
    CosFunction.bYieldAlmNeedOneCycle = true;
    CosFunction.bContinueFailNeedAlarmDirectly = false;
    CosFunction.bSmartAutoClean = false;
    W906_ShowErrorMessage_Hook = &RecordShow;

    // ---------------------------------------------------------------- [2]
    std::printf("[2] DoLowYieldAlarm (golden 0618 atester_ProcessCount.cpp:291-300)\n");
    bNeedOneCycleByYieldAlm = false;
    fAllMotorHome = false;                                   // BtnOneCycleClick returns at once ([3] drives it on its own)
    DoLowYieldAlarm("WAR0701", "Site1");
    Check(L->Count == 1 && ListText() == "WAR0701,Site1" && bNeedOneCycleByYieldAlm, "[2] the first alarm: \"WAR0701,Site1\" added, bNeedOneCycleByYieldAlm set (" + ListText() + ")");
    DoLowYieldAlarm("WAR0701", "Site1");
    Check(L->Count == 1, "[2] the same code again: not added twice (golden Text.AnsiPos(AlarmCode)==0) (" + ListText() + ")");
    DoLowYieldAlarm("WAR0702", "Arm2");
    Check(L->Count == 2 && ListText() == "WAR0701,Site1 | WAR0702,Arm2", "[2] another code: added after it (" + ListText() + ")");

    // ---------------------------------------------------------------- [3]
    std::printf("[3] TfMain::BtnOneCycleClick (golden 0618 main.cpp:4349-4352)\n");
    const int saveOneCycle = iOneCycle, saveTask = iOneCycleTask;
    fAllMotorHome = true;
    iOneCycle = 0;
    bSECSGEMAlarm = false;
    bEnableEmployeeIDCheck = false;
    bNeedOneCycleByYieldAlm = false;
    fMain->BtnOneCycleClick(fMain);
    Check(bNeedOneCycleByYieldAlm, "[3] pending alarms + One Cycle -> bNeedOneCycleByYieldAlm = true");
    TStringList keep;
    keep.Assign(L);
    L->Clear();
    iOneCycle = 0;
    bNeedOneCycleByYieldAlm = false;
    fMain->BtnOneCycleClick(fMain);
    Check(!bNeedOneCycleByYieldAlm, "[3] empty list -> bNeedOneCycleByYieldAlm stays false");
    L->Assign(&keep);
    fAllMotorHome = false;

    // ---------------------------------------------------------------- [4]
    std::printf("[4] DoOneCycleFinishCheck shows the pending alarms and clears the list\n");
    W906_CreateLogObjects();                                 // golden has them from the ctor; the finish writes the event log (MyDBIProcess) -- ctest roots
    std::vector<HTMotor*> saveMot(TOTAL_MOTOR);              // every axis gets a disabled HTMotor (golden always has the objects): CheckIndexIsNormal
    std::vector<HTMotor> off(TOTAL_MOTOR);                   //   passes (Enable false -> in range) and the normal finish's SetInArmSpeed etc. do not
    for (int i = 0; i < TOTAL_MOTOR; ++i) { saveMot[i] = MOT[i].Motor; off[i].Enable = false; MOT[i].Motor = &off[i]; MOT[i].MovFlag = false; }   //   dereference NULL
    const struct { bool autoOne; const char* name; } cases[2] = { {true, "after an auto-clean One Cycle (0618 csystem.cpp:13265-13273)"},
                                                              {false, "a normal One Cycle (0618 csystem.cpp:13600-13610)"} };
    for (const auto& c : cases) {
        LastSet.iRunStartMode = 0;                           // not rsmAutoSiteMap
        if (L->Count == 0) { L->Add("WAR0701,Site1"); L->Add("WAR0702,Arm2"); }
        bNeedOneCycleByYieldAlm = true;
        g_shown.clear();
        std::string trace;
        const int n = RunFinish(c.autoOne, &trace);
        Check(iOneCycle == 0, std::string("[4] ") + c.name + ": DoOneCycleFinishCheck reached the finish (" + std::to_string(n) + " calls, cursor " + trace + ")");
        Check(CountCode("WAR0701") == 1 && CountCode("WAR0702") == 1, std::string("[4] ") + c.name + ": each pending alarm shown once -- " + Joined());
        bool parsed = false;
        for (std::size_t i = 0; i + 1 < g_shown.size(); ++i)
            if (g_shown[i].code == "WAR0701" && g_shown[i].part == "Site1" && g_shown[i].kcode == 0 &&
                g_shown[i + 1].code == "WAR0702" && g_shown[i + 1].part == "Arm2" && g_shown[i + 1].kcode == 0) parsed = true;
        Check(parsed, std::string("[4] ") + c.name + ": in list order, golden parse (code / errPart), kcode 0 = a notice");
        Check(L->Count == 0, std::string("[4] ") + c.name + ": the list is cleared (" + ListText() + ")");
        g_shown.clear();
        bNeedOneCycleByYieldAlm = true;
        RunFinish(c.autoOne, nullptr);
        Check(CountCode("WAR0701") == 0 && CountCode("WAR0702") == 0, std::string("[4] ") + c.name + ": a second finish shows nothing -- " + Joined());
    }
    for (int i = 0; i < TOTAL_MOTOR; ++i) MOT[i].Motor = saveMot[i];
    W906_DestroyLogObjects();
    W906_ShowErrorMessage_Hook = 0;
    iOneCycle = saveOneCycle;
    iOneCycleTask = saveTask;
    CosFunction.bYieldAlmNeedOneCycle = saveNeed;
    CosFunction.bContinueFailNeedAlarmDirectly = saveDirect;
    CosFunction.bSmartAutoClean = saveSmart;
    fAllMotorHome = saveHome;
    LastSet.iRunStartMode = saveRsm;

    // ---------------------------------------------------------------- [5]
    std::printf("[5] source pins\n");
    {
        std::string pc, co, cs;
        const bool r = ReadAll(src + "/atester_ProcessCount.cpp", &pc) && ReadAll(src + "/cCleanOut.cpp", &co) && ReadAll(src + "/csystem.cpp", &cs);
        Check(r, "[5] read atester_ProcessCount.cpp / cCleanOut.cpp / csystem.cpp");
        const std::vector<std::string> lp = LiveCode(Lines(pc)), lc = LiveCode(Lines(co)), ls = LiveCode(Lines(cs));
        Check(LiveCount(lp, "W906_LowYieldAlarmList()->Add(Str);") == 1 && LiveCount(lp, "AnsiString(W906_LowYieldAlarmList()->Text).AnsiPos(AlarmCode)==0") == 1,
              "[5] atester_ProcessCount.cpp: the add and its de-dup are live (golden :295-297)");
        Check(LiveCount(lc, "W906_LowYieldAlarmList()->Count!=0)") == 1, "[5] cCleanOut.cpp: the BtnOneCycleClick re-arm is live (golden main.cpp:4349-4352)");
        Check(LiveCount(ls, "#define W7C2_FMAIN_SLLOWYIELD   (W906_LowYieldAlarmList())") == 1,
              "[5] csystem.cpp: W7C2_FMAIN_SLLOWYIELD is the real list (the define)");
        Check(LiveCount(ls, "AnsiString(W7C2_FMAIN_SLLOWYIELD->Strings[i]).SubString(0,") == 1 && LiveCount(ls, "AnsiString(W7C2_FMAIN_SLLOWYIELD->Strings[i]).SubString(1,") == 1,
              "[5] csystem.cpp: both golden readers live, with the AnsiString(...) cast");
        Check(LiveCount(lp, "fMain->slLowYieldAlarm") + LiveCount(lc, "fMain->slLowYieldAlarm") + LiveCount(ls, "W7C2_fMain_slLowYieldAlarm") == 0,
              "[5] no live fMain->slLowYieldAlarm / W7C2_fMain_slLowYieldAlarm left");
    }

    std::printf("St02_W175LowYield: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
