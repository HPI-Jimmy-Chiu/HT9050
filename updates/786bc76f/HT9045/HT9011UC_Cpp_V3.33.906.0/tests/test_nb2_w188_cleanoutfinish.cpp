// =============================================================================
//  AI(W906-W188) 20261009 (NB2-1): W-188 #1 -- DoCleanOutFinishCheck brought to golden 913
//  (GitLab honprec/rd/rd5/ht9045_913 main e9908638, csystem.cpp:15591-16687; 0618 was :14713-15745).
//  Three 913 hunks, all on the same lines as the 0618 code they follow / replace (csystem.cpp line count of
//  DoCleanOutFinishCheck unchanged; the two longer bodies live at the end of csystem.cpp):
//    H1  golden 913 :15600-15639  clean-out watchdog -- diagnostic EventLog line only (RogerYang 20260811)
//    H2  golden 913 :15847-15854  AutoClean residual latch: AutoClean not running -> clear iCheckFinish_ByAutoClean
//                                 instead of parking on cursor 18 (RogerYang 20260708)
//    H3  golden 913 :15975-15989  Clean Out finish keeps PickHPRec while a hot plate still holds a REAL IC
//                                 (HAS_NULL_IC placeholders do not count) (RogerYang 20260831)
//
//  [1] H3 behaviour: W906_CleanOutResetPickHPRec913() called directly; the golden 913 decision is read back from the
//      RecordProcess line it writes (cMyDB.cpp RecordProcess keeps the last text in ExString):
//        real IC on plate 1             -> "HP still has real IC ... keep PickHPRec"
//        only a HAS_NULL_IC placeholder -> "HP empty, ResetFile PickHPRec"   (HasRealIC, not HasIC)
//        both plates empty              -> "HP empty, ResetFile PickHPRec"   (the 0618 behaviour)
//      NOTE: inside csystem.cpp `PickFromHPList` is still the W7C1 no-op seam (csystem.cpp:2498-2505, ResetFile does
//      nothing), so today H3 decides + logs; the real PickHPRec.json reset waits for that seam to be retired.
//  [2] source ratchets on csystem.cpp (argv[1] = port root): H1 call inside DoCleanOutFinishCheck ahead of
//      `if(iOneCycle)`; H2 line shape inside `if(iCheckFinish_ByAutoClean==1)`; the bare 0618
//      `PickFromHPList->ResetFile(sHPPickRec);` is gone from DoCleanOutFinishCheck and lives only in the H3 helper;
//      H1 helper carries the golden statics and the "CleanOutWatchdog" EventLog tag.
//  H2 is reached only after the cursor-1..17 gates of the clean-out drain (csystem.cpp DoCleanOutFinishCheck), which
//  the offline HAL stops at cursor 5 (see W7_C1_CleanOutFinish) -- so H2 is pinned by source only.
//  H1 fires after 300 s (hot: soak+300 s) of an unfinished clean-out on HTimer wall time -- source only as well.
//  Use: only through ctest (NB2_W188CleanOutFinish).
// =============================================================================
#include "csystem.h"
#include "MachineDefine.h"
#include "MachineType.h"
#include "cmydef.h"
#include "Motor/mymotor.h"
#include "cMyDB.h"                 // ExString (RecordProcess keeps the last line)
#include "w906_ctest_guard.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

void W906_CleanOutResetPickHPRec913();

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }               \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static std::vector<std::string> ReadLines(const std::string& path)
{
    std::vector<std::string> v;
    std::ifstream f(path.c_str(), std::ios::binary);
    std::string s;
    while (std::getline(f, s)) {
        if (!s.empty() && s[s.size() - 1] == '\r') s.erase(s.size() - 1);
        v.push_back(s);
    }
    return v;
}

static std::string Trim(const std::string& s)
{
    size_t a = s.find_first_not_of(" \t");
    return a == std::string::npos ? std::string() : s.substr(a);
}

// first line index whose trimmed text starts with `head`, from `from`; -1 if none
static int FindStart(const std::vector<std::string>& L, const std::string& head, int from = 0)
{
    for (int i = from; i < (int)L.size(); ++i)
        if (Trim(L[i]).compare(0, head.size(), head) == 0) return i;
    return -1;
}

// end line (the closing brace at column 0) of a function whose signature line is `sig`
static int FuncEnd(const std::vector<std::string>& L, int sig)
{
    for (int i = sig + 1; i < (int)L.size(); ++i)
        if (L[i] == "}") return i;
    return -1;
}

static int CountIn(const std::vector<std::string>& L, int a, int b, const std::string& needle)
{
    int n = 0;
    for (int i = a; i <= b && i < (int)L.size(); ++i)
        if (L[i].find(needle) != std::string::npos) ++n;
    return n;
}

struct PlateSnap { int x, y, d00; };
static PlateSnap SavePlate(int m) { PlateSnap s = { MOT[m].Tray.XItem, MOT[m].Tray.YItem, MOT[m].Tray.Data[0][0] }; return s; }
static void RestorePlate(int m, const PlateSnap& s) { MOT[m].Tray.XItem = s.x; MOT[m].Tray.YItem = s.y; MOT[m].Tray.Data[0][0] = s.d00; }

int main(int argc, char** argv)
{
    extern AnsiString DataPath;
    const char* const rt[] = { "DataPath", DataPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("NB2_W188CleanOutFinish", rt)) return 2;
    std::printf("==== W-188 #1 DoCleanOutFinishCheck -> golden 913 ====\n");

    // ---------------------------------------------------------------- [1] H3
    std::printf("-- [1] H3: Clean Out finish keeps PickHPRec while a hot plate holds a real IC --\n");
    const PlateSnap p1 = SavePlate(MMPlate1), p2 = SavePlate(MMPlate2);
    const AnsiString saveEx = ExString;
    MOT[MMPlate2].Tray.XItem = 0; MOT[MMPlate2].Tray.YItem = 0;

    MOT[MMPlate1].Tray.XItem = 1; MOT[MMPlate1].Tray.YItem = 1; MOT[MMPlate1].Tray.Data[0][0] = HAS_IC;
    ExString = "";
    W906_CleanOutResetPickHPRec913();
    CHECK(std::string(ExString.c_str()).find("HP still has real IC") != std::string::npos &&
          std::string(ExString.c_str()).find("keep PickHPRec") != std::string::npos,
          "[1] real IC on plate 1 -> keep PickHPRec (golden 913 HasRealIC guard)");
    std::printf("     (%s)\n", ExString.c_str());

    MOT[MMPlate1].Tray.Data[0][0] = HAS_NULL_IC;
    ExString = "";
    W906_CleanOutResetPickHPRec913();
    CHECK(std::string(ExString.c_str()).find("HP empty, ResetFile PickHPRec") != std::string::npos,
          "[1] only a HAS_NULL_IC placeholder -> reset branch (HasRealIC, not HasIC)");

    MOT[MMPlate1].Tray.Data[0][0] = 0;
    ExString = "";
    W906_CleanOutResetPickHPRec913();
    CHECK(std::string(ExString.c_str()).find("HP empty, ResetFile PickHPRec") != std::string::npos,
          "[1] both plates empty -> reset branch (the 0618 behaviour)");

    RestorePlate(MMPlate1, p1); RestorePlate(MMPlate2, p2);
    ExString = saveEx;

    // ---------------------------------------------------------------- [2] source
    std::printf("-- [2] source ratchets (csystem.cpp) --\n");
    const std::string root = argc > 1 ? argv[1] : ".";
    const std::vector<std::string> L = ReadLines(root + "/csystem.cpp");
    CHECK(!L.empty(), "[2] csystem.cpp readable");
    const int f0 = FindStart(L, "void DoCleanOutFinishCheck()");
    const int f1 = f0 < 0 ? -1 : FuncEnd(L, f0);
    CHECK(f0 >= 0 && f1 > f0, "[2] DoCleanOutFinishCheck found");
    const int io = f0 < 0 ? -1 : FindStart(L, "if(iOneCycle)", f0);
    const int h1 = f0 < 0 ? -1 : FindStart(L, "{ void W906_DoCleanOutWatchdog913(); W906_DoCleanOutWatchdog913(); }", f0);
    CHECK(h1 > f0 && io > h1 && io - h1 <= 2, "[2] H1: watchdog call is the first statement, right ahead of if(iOneCycle) (golden 913 :15600-15639 order)");
    const int lat = f0 < 0 ? -1 : FindStart(L, "if(iCheckFinish_ByAutoClean==1)", f0);
    CHECK(lat > f0 && lat < f1 && lat + 3 < (int)L.size() &&
          Trim(L[lat + 2]).find("if(bRunAutoClean==false) { iCheckFinish_ByAutoClean=0; } else { iCleanOutCycleTask=18;") == 0 &&
          Trim(L[lat + 3]) == "return; }",
          "[2] H2: latch -- AutoClean not running clears iCheckFinish_ByAutoClean, else park on 18 (golden 913 :15847-15854)");
    CHECK(CountIn(L, f0, f1, "PickFromHPList->ResetFile(sHPPickRec);") == 0 &&
          CountIn(L, f0, f1, "{ void W906_CleanOutResetPickHPRec913(); W906_CleanOutResetPickHPRec913(); }") == 1,
          "[2] H3: DoCleanOutFinishCheck no longer resets PickHPRec unconditionally; calls the golden 913 guard once");
    const int g3 = FindStart(L, "void W906_CleanOutResetPickHPRec913()");
    const int g3e = g3 < 0 ? -1 : FuncEnd(L, g3);
    CHECK(g3 > f1 && CountIn(L, g3, g3e, "MOT[MMPlate1].Tray.HasRealIC()==false") == 1 &&
          CountIn(L, g3, g3e, "PickFromHPList->ResetFile(sHPPickRec);") == 1,
          "[2] H3 helper: HasRealIC guard around the single ResetFile");
    const int g1 = FindStart(L, "void W906_DoCleanOutWatchdog913()");
    const int g1e = g1 < 0 ? -1 : FuncEnd(L, g1);
    CHECK(g1 > f1 && CountIn(L, g1, g1e, "static HTimer hCleanOutWD;") == 1 &&
          CountIn(L, g1, g1e, "static bool   bCleanOutWDArmed=false;") == 1 &&
          CountIn(L, g1, g1e, "\"CleanOutWatchdog\"") == 1 &&
          CountIn(L, g1, g1e, "Prod.iHotTime+300") == 1,
          "[2] H1 helper: golden statics, soak-based threshold, EventLog-only tag");

    std::printf("==== W-188 #1: %d passed, %d failed ====\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
