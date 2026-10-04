// =============================================================================
//  test_e034_noop_eq.cpp -- AI(W906-E034) 20261003 (St01, todo E-034 = laptop card S-21)
//
//  Steven 1003 14:5x "Q82. A": the golden 0618 no-op `==` statements that 0625 / V912 fixed to `=`
//  are taken in the laptop's port files (#20 exception, Steven 1003 standing rule; Q82;
//  RULINGS_20261003 #1).  This test covers the sites that the existing core tests cannot reach:
//
//  [A] acatchtray.cpp DoLoadCarRotArmReadRFID x3 -- BEHAVIOUR (offline: MOT[MLdCarRotArm].Motor==NULL
//      makes MotorMove return -1 = true, cylinders disabled make OffSensor true, Sen[] forced on,
//      W906_ShowErrorMessage_SimReturn picks Skip).  0618 :8403 / :8444 / :8512 `=="NOREAD"` left the
//      tray ID ""; 0625 same lines / V912 :9088 / :9166 / :9245 set "NOREAD".  Port :8568 / :8609 /
//      :8677.  The function has no caller in the port today (golden caller cTrayMapping.cpp:5828 is
//      not ported), so this is the only place it runs.
//  [B] SOURCE checks (a behaviour test is impossible or unsafe), each with an in-test mutation:
//      the checker is re-run on a copy of the file with the fix turned back into `==` and must fail.
//      B1 csystem.cpp MainProc WAR0955 (port :32417; 0618 :18646, 0625 :18646, V912 :19623): the
//         line sits inside `#if 0` SAFETY-GATE(W906-T6-OCRSTART) -- not compiled, no runtime effect
//         until that gate opens; this check pins the fix.
//      B2 OCRInsp.cpp DoOCRFlow case 1 (port :819; 0618 / 0625 / V912 :408): reaching it needs a tray
//         map + OCR motor moves + the RS232 OCR facade; source check.
//      B3 forms/fTrayAssignment.cpp ReadFile (port :235; 0618 / 0625 cTrayAssignment.cpp:178, V912
//         :212): the body reads (and ReadIniData may write) the real DataPath Tray.Data, and wb_serve
//         routes ReadFile to FileRW TA_ReadFile (already `=false`) through
//         g_W906_TrayAssignmentReadFileHook; source check only.
//      B4 cShowBinSelect.cpp ChangeBinDispStatus (port :2315; 0618 :272, 0625 :272, V912 :295): the
//         fix is St02's (8db5c2c9, AI(W906-ST02-C14) 1002), not E-034's; St02's ctest C14_BinDispPane
//         (tests/test_c14_bindisp_pane.cpp:195) is the behaviour test.  E-034 only pins the line.
//
//  argv[1] = the port tree root (CMAKE_SOURCE_DIR).  Reads source files only; writes nothing.
//  CONTROL: W906_E034_ROOT (when set) replaces argv[1] for [B] -- point it at a copy with one fix turned
//  back into `==` and the run must go red (that is how the 20261003 reverse check was done, through ctest
//  so the ENV-ALL scratch roots still apply).
//  The other E-034 sites are behaviour-tested in YieldMonCore / ObserverCore / TemperFromCore.
// =============================================================================
#include "acatchtray.h"            // DoLoadCarRotArmReadRFID
#include "acatchtray_shims.h"      // fTrayMapping (ldRFID), iCoverTrayIDTask[], iReadCIDAction
#include "Motor/mymotor.h"         // MOT[]
#include "mycylin.h"               // Cylinder[]
#include "mysensor.h"              // Sen[] (TMySensor)
#include "cmydef.h"                // asTrayIDDataCorverLoader, SnLoadCarRFIDSW, MLdCarRotArm, C_LoadCarRFID*, K_*
#include "MachineType.h"           // iKeyenceCoverTrayID_LoaderCar
#include "Config.h"                // IniConfig.bEnable_SECS_GEM
#include "canary_support.h"        // W906_ShowErrorMessage_SimReturn / _LastCode / _Reset

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg) do { \
        if (cond) { ++g_pass; std::printf("  PASS: %s\n", msg); } \
        else      { ++g_fail; std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); } \
    } while (0)

// -----------------------------------------------------------------------------
//  [A] acatchtray.cpp DoLoadCarRotArmReadRFID -- behaviour
// -----------------------------------------------------------------------------
static void ForceSensorOn(TMySensor &s)    // same knob as tests/test_agv_portscan.cpp
{
    s.Enable  = true;
    s.ISABase = -1;
    s.Type    = 0;                          // IsOn() -> !ret, ret stays false -> true
}

static void Test_LoadCarRotArmReadRFID_NoRead()
{
    std::printf("\n[A] acatchtray DoLoadCarRotArmReadRFID: a failed / skipped RFID read sets \"NOREAD\"\n");
    int &Task = iCoverTrayIDTask[iKeyenceCoverTrayID_LoaderCar];
    TMySensor savedSen = Sen[SnLoadCarRFIDSW];
    bool savedSecs = IniConfig.bEnable_SECS_GEM;
    IniConfig.bEnable_SECS_GEM = false;
    MOT[MLdCarRotArm].Motor = NULL;         // offline: MotorMove returns -1 (true), StopMotor no-op
    Cylinder[C_LoadCarRFIDRotArmD].Enable = false;   // OffSensor() -> true
    Cylinder[C_LoadCarRFIDRotArmU].Enable = false;
    fTrayMapping->ldRFID->bReadUID1 = false;

    // case 3 resets the function's statics (bMotOk / bCynOk / bReadOK) and the ID.
    Task = 3;
    DoLoadCarRotArmReadRFID(false);
    CHECK(Task == 100 && asTrayIDDataCorverLoader == "", "case 3: statics reset, tray ID cleared, Task 100");

    // Site 1 (port :8568, 0618 :8403): no UID but the RFID switch is on -> "NOREAD", Task 4000.
    ForceSensorOn(Sen[SnLoadCarRFIDSW]);
    Task = 3000;
    DoLoadCarRotArmReadRFID(false);
    CHECK(Task == 4000, "site 1: switch on without a UID -> Task 4000");
    CHECK(asTrayIDDataCorverLoader == "NOREAD", "E-034 site 1 (:8568; 0618 :8403 no-op -> 0625 :8403 / V912 :9088 =\"NOREAD\")");
    Sen[SnLoadCarRFIDSW] = savedSen;

    // Site 2 (port :8609, 0618 :8444): arm back up, bReadOK false -> "NOREAD", Task 6000.
    asTrayIDDataCorverLoader = "";
    Task = 4000;
    DoLoadCarRotArmReadRFID(false);
    CHECK(Task == 6000, "site 2: motor + cylinder ok, read not ok -> Task 6000");
    CHECK(asTrayIDDataCorverLoader == "NOREAD", "E-034 site 2 (:8609; 0618 :8444 no-op -> 0625 :8444 / V912 :9166 =\"NOREAD\")");

    // Control: WAR16120 answered Retry -> back to case 3, ID untouched.
    asTrayIDDataCorverLoader = "";
    W906_ShowErrorMessage_Reset();
    W906_ShowErrorMessage_SimReturn = K_RETRY;
    Task = 6000;
    bool r = DoLoadCarRotArmReadRFID(true);
    CHECK(r == false && Task == 3 && asTrayIDDataCorverLoader == "", "control: WAR16120 Retry -> Task 3, tray ID untouched");

    // Site 3 (port :8677, 0618 :8512): WAR16120 answered Skip -> "NOREAD", returns true.
    W906_ShowErrorMessage_Reset();
    W906_ShowErrorMessage_SimReturn = K_SKIP;
    Task = 6000;
    r = DoLoadCarRotArmReadRFID(true);
    CHECK(W906_ShowErrorMessage_LastCode == "WAR16120", "site 3: WAR16120 asked (Retry | Skip)");
    CHECK(r == true, "site 3: Skip ends the read (returns true)");
    CHECK(asTrayIDDataCorverLoader == "NOREAD", "E-034 site 3 (:8677; 0618 :8512 no-op -> 0625 :8512 / V912 :9245 =\"NOREAD\")");

    W906_ShowErrorMessage_Reset();
    asTrayIDDataCorverLoader = "";
    Task = 1;
    IniConfig.bEnable_SECS_GEM = savedSecs;
}

// -----------------------------------------------------------------------------
//  [B] source checks
// -----------------------------------------------------------------------------
typedef std::vector<std::string> Lines;

static bool ReadLines(const std::string &path, Lines &out)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f) return false;
    std::stringstream ss; ss << f.rdbuf();
    std::string s = ss.str(), cur;
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '\n') { if (!cur.empty() && cur.back() == '\r') cur.pop_back(); out.push_back(cur); cur.clear(); }
        else cur += s[i];
    }
    if (!cur.empty()) out.push_back(cur);
    return true;
}

static std::string Trim(const std::string &s)
{
    size_t a = s.find_first_not_of(" \t"), b = s.find_last_not_of(" \t");
    return a == std::string::npos ? std::string() : s.substr(a, b - a + 1);
}

// code part: the line up to a `//` that is not inside a string literal, trimmed.
static std::string Code(const std::string &line)
{
    bool inStr = false;
    for (size_t i = 0; i < line.size(); ++i)
    {
        if (line[i] == '"' && (i == 0 || line[i - 1] != '\\')) inStr = !inStr;
        if (!inStr && line.compare(i, 2, "//") == 0) return Trim(line.substr(0, i));
    }
    return Trim(line);
}

static int FindCode(const Lines &L, int from, const std::string &startsWith)
{
    for (int i = from; i < (int)L.size(); ++i)
        if (Code(L[i]).compare(0, startsWith.size(), startsWith) == 0) return i;
    return -1;
}

static int NextCodeLine(const Lines &L, int from)
{
    for (int i = from; i < (int)L.size(); ++i)
        if (!Code(L[i]).empty()) return i;
    return -1;
}

// Each checker returns the 0-based index of the fixed line, or -1 when the fix is not there.
// Every checker compares the CODE PART only (Code() cuts at `//` first): the trailing comments may say
// anything, including `==` (St02-M 1003: St02 W-17 8374034c rewrites the comment on cShowBinSelect :2315).
static int CheckWar0955(const Lines &L)
{
    int hit = -1, n = 0;
    for (int i = 0; i < (int)L.size(); ++i)
        if (Code(L[i]).find("ShowErrorMessage(\"WAR0955\"") != std::string::npos) { hit = i; ++n; }
    if (n != 1) return -1;
    if (Code(L[hit]).compare(0, 21, "ret=ShowErrorMessage(") != 0) return -1;
    return hit;
}

static int CheckOcrInsp(const Lines &L)
{
    int fn = FindCode(L, 0, "int DoOCRFlow()");
    if (fn < 0) return -1;
    int send = FindCode(L, fn, "fOCR->SendOCR(fOCR->ocrLot);");
    if (send < 0) return -1;
    for (int i = send - 1; i > fn; --i)       // the code line right before SendOCR(ocrLot)
        if (!Code(L[i]).empty())
            return Code(L[i]) == "fOCR->bOcr_ReceiveOK[fOCR->ocrLot]=false;" ? i : -1;
    return -1;
}

static int CheckTrayAssignment(const Lines &L)
{
    int fn = FindCode(L, 0, "void TfTrayAssignment::ReadFile()");
    if (fn < 0) return -1;
    int cond = FindCode(L, fn, "if(Prod.iTrayType[eFix3]==tNotUse)");
    if (cond < 0) return -1;
    int stmt = NextCodeLine(L, cond + 1);
    if (stmt < 0 || Code(L[stmt]) != "TrayForm.bTrayUpDownSet[eFix3]=false;") return -1;
    return stmt;
}

static int CheckShowBinSelect(const Lines &L)
{
    int fn = FindCode(L, 0, "void TfShowBinSelect::ChangeBinDispStatus()");
    if (fn < 0) return -1;
    int hit = -1;
    for (int i = fn + 1; i < (int)L.size() && !(L[i].size() && L[i][0] == '}'); ++i)
    {
        std::string c = Code(L[i]);
        if (c.compare(0, 29, "PageControl1->ActivePageIndex") == 0 && c.find('3') != std::string::npos)
        {
            if (c != "PageControl1->ActivePageIndex=3;") return -1;
            hit = i;
        }
    }
    return hit;
}

struct SourceSite
{
    const char *file;
    int (*check)(const Lines &);
    const char *fixed;      // token in the fixed line
    const char *broken;     // the 0618 form the mutation puts back
    const char *what;
};

static const SourceSite kSites[] = {
    { "csystem.cpp",               CheckWar0955,        "ret=ShowErrorMessage(",                "ret==ShowErrorMessage(",
      "B1 csystem.cpp MainProc WAR0955 (port :32417; 0618 :18646 -> 0625 :18646 / V912 :19623 ret=; inside #if 0 OCRSTART, no runtime effect yet)" },
    { "OCRInsp.cpp",               CheckOcrInsp,        "bOcr_ReceiveOK[fOCR->ocrLot]=false;",  "bOcr_ReceiveOK[fOCR->ocrLot]==false;",
      "B2 OCRInsp.cpp DoOCRFlow (port :819; 0618 :408 -> 0625 / V912 :408 =false)" },
    { "forms/fTrayAssignment.cpp", CheckTrayAssignment, "bTrayUpDownSet[eFix3]=false;",         "bTrayUpDownSet[eFix3]==false;",
      "B3 fTrayAssignment.cpp ReadFile (port :235; 0618 cTrayAssignment.cpp:178 -> 0625 :178 / V912 :212 =false)" },
    { "cShowBinSelect.cpp",        CheckShowBinSelect,  "ActivePageIndex=3;",                   "ActivePageIndex==3;",
      "B4 cShowBinSelect.cpp ChangeBinDispStatus (port :2315; 0618 :272 -> 0625 :272 / V912 :295 =3; St02's fix 8db5c2c9)" },
};

static void Test_SourceSites(const std::string &root)
{
    std::printf("\n[B] source checks (+ in-test mutation: the 0618 `==` put back must be caught)\n");
    for (const SourceSite &s : kSites)
    {
        Lines L;
        bool ok = ReadLines(root + "/" + s.file, L);
        CHECK(ok, s.file);
        if (!ok) continue;
        int at = s.check(L);
        CHECK(at >= 0, s.what);
        if (at < 0) continue;
        std::printf("        (%s:%d)\n", s.file, at + 1);

        Lines M = L;                            // mutation: back to the 0618 no-op
        size_t p = M[at].find(s.fixed);
        CHECK(p != std::string::npos, "mutation: fixed token found on the checked line");
        if (p == std::string::npos) continue;
        M[at].replace(p, std::string(s.fixed).size(), s.broken);
        std::string msg = std::string("mutation caught: ") + s.file + " with the 0618 `==` is rejected";
        CHECK(s.check(M) < 0, msg.c_str());
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) { std::printf("usage: test_e034_noop_eq <port tree root>\n"); return 2; }
    Test_LoadCarRotArmReadRFID_NoRead();
    const char *ov = std::getenv("W906_E034_ROOT");
    std::string root = (ov && *ov) ? std::string(ov) : std::string(argv[1]);
    if (ov && *ov) std::printf("[B] root from W906_E034_ROOT: %s\n", ov);
    Test_SourceSites(root);
    std::printf("\n%d/%d checks passed (test_e034_noop_eq)\n", g_pass, g_pass + g_fail);
    return g_fail == 0 ? 0 : 1;
}
