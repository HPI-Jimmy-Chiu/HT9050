// =============================================================================
//  AI(W906-W188) 20261009 (NB2-1): W-188 #3 -- HasHotReadyIC_9045 is golden 913 (GitLab honprec/rd/rd5/ht9045_913
//  main e9908638, ainarm_SearchPickPlate.cpp:1638-1840; 0618 :1248-1397, which the port body was verbatim).
//    H1  (RogerYang 20260831) the gate uses Tray.HasRealIC(): a hot plate holding only HAS_NULL_IC placeholders counts as
//        empty -> the placeholders are cleared (ClearNullIC) with one RecordProcess line and the function returns false,
//        instead of letting the downstream pick search spin 500 times and stop with "No GetHPFirstTeamMotUse" (HHT-76)
//    H2  (20260827) a team with mixed IC / empty cells raises bNeedReconcileHPList (golden 913 consumer ainarm9045.cpp:4506
//        is a separate function, not part of this change)
//    H3  (20260831) orphan cells are listed to the Exception log before "No GetHPFirstTeamMotUse"
//    H4  (20260801) FreePoint2DList on every return (TList::Clear() does not free the uPoint2D items)
//  [1] behaviour: only a HAS_NULL_IC placeholder on plate 1 -> false, placeholder cleared, golden log line; truly empty
//      plates -> false, no log; a real IC is left alone by the H1 gate.
//  [2] source ratchets on ainarm_SearchPickPlate.cpp / ainarm_SearchPickPlate.h / aHotPlateSubstrate.h (argv[1] = root).
//  Use: only through ctest (NB2_W188HotReady).
// =============================================================================
#include "ainarm_SearchPickPlate.h"
#include "aHotPlateSubstrate.h"
#include "Motor/mymotor.h"
#include "cmydef.h"
#include "canary_support.h"
extern AnsiString ExString;          // cMyDB.cpp (RecordProcess keeps the last text); cMyDB.h clashes with canary_support.h defaults
#include "w906_ctest_guard.h"
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

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
static int Find(const std::vector<std::string>& L, const std::string& head, int from = 0)
{
    for (int i = from; i < (int)L.size(); ++i) if (Trim(L[i]).compare(0, head.size(), head) == 0) return i;
    return -1;
}
static int End(const std::vector<std::string>& L, int sig)
{
    for (int i = sig + 1; i < (int)L.size(); ++i) if (L[i] == "}") return i;
    return -1;
}
static int CountIn(const std::vector<std::string>& L, int a, int b, const std::string& n)
{
    int c = 0;
    for (int i = a; i >= 0 && i <= b && i < (int)L.size(); ++i) if (L[i].find(n) != std::string::npos) ++c;
    return c;
}
static bool Has(const AnsiString& s, const char* n) { return std::string(s.c_str()).find(n) != std::string::npos; }

struct PlateSnap { int x, y, d00; };
static PlateSnap SavePlate(int m) { PlateSnap s = { MOT[m].Tray.XItem, MOT[m].Tray.YItem, MOT[m].Tray.Data[0][0] }; return s; }
static void RestorePlate(int m, const PlateSnap& s) { MOT[m].Tray.XItem = s.x; MOT[m].Tray.YItem = s.y; MOT[m].Tray.Data[0][0] = s.d00; }

int main(int argc, char** argv)
{
    extern AnsiString DataPath;
    const char* const rt[] = { "DataPath", DataPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("NB2_W188HotReady", rt)) return 2;
    std::printf("==== W-188 #3 HasHotReadyIC_9045 -> golden 913 ====\n");

    // ---------------------------------------------------------------- [1] behaviour
    std::printf("-- [1] H1: placeholders only = empty plate --\n");
    const PlateSnap p1 = SavePlate(MMPlate1), p2 = SavePlate(MMPlate2);
    const int savePP = iPickPlate[0], savePX = iPickPlateX[0], savePY = iPickPlateY[0];
    const bool saveASM = bRunAutoSiteMapping;
    const int saveRSM = LastSet.iRunStartMode;
    const AnsiString saveEx = ExString;
    iPickPlate[0] = 0; iPickPlateX[0] = 0; iPickPlateY[0] = 0;
    bRunAutoSiteMapping = false;
    LastSet.iRunStartMode = 0;
    MOT[MMPlate2].Tray.XItem = 0; MOT[MMPlate2].Tray.YItem = 0;
    MOT[MMPlate1].Tray.XItem = 1; MOT[MMPlate1].Tray.YItem = 1;

    MOT[MMPlate1].Tray.Data[0][0] = HAS_NULL_IC;
    ExString = "";
    bool r = HasHotReadyIC_9045();
    CHECK(!r, "[1] only a HAS_NULL_IC placeholder -> not ready (golden 913 HasRealIC gate)");
    CHECK(MOT[MMPlate1].Tray.Data[0][0] == NULL_IC, "[1] the placeholder is cleared (ClearNullIC)");
    CHECK(Has(ExString, "plate has no real IC, clear HAS_NULL_IC residue"), "[1] golden 913 RecordProcess line written once");
    std::printf("     (%s)\n", ExString.c_str());

    ExString = "";
    r = HasHotReadyIC_9045();
    CHECK(!r && ExString.Length() == 0, "[1] plates truly empty -> not ready, no log (the residue log only fires when there is residue)");

    MOT[MMPlate1].Tray.Data[0][0] = HAS_IC;
    ExString = "";
    HasHotReadyIC_9045();
    CHECK(MOT[MMPlate1].Tray.Data[0][0] == HAS_IC && !Has(ExString, "plate has no real IC"),
          "[1] a real IC is left alone by the H1 gate (the 0618 path runs)");

    RestorePlate(MMPlate1, p1); RestorePlate(MMPlate2, p2);
    iPickPlate[0] = savePP; iPickPlateX[0] = savePX; iPickPlateY[0] = savePY;
    bRunAutoSiteMapping = saveASM; LastSet.iRunStartMode = saveRSM;
    ExString = saveEx;

    // ---------------------------------------------------------------- [2] source
    std::printf("-- [2] source ratchets --\n");
    const std::string root = argc > 1 ? argv[1] : ".";
    const std::vector<std::string> L = ReadLines(root + "/ainarm_SearchPickPlate.cpp");
    const int f0 = Find(L, "bool HasHotReadyIC_9045()"), f1 = f0 < 0 ? -1 : End(L, f0);
    CHECK(f0 >= 0 && f1 > f0, "[2] HasHotReadyIC_9045 found");
    CHECK(CountIn(L, f0, f1, "if(MOT[MMPlate1].Tray.HasRealIC()==false &&") == 1 && CountIn(L, f0, f1, "MOT[MMPlate1].Tray.ClearNullIC();") == 1,
          "[2] H1: HasRealIC gate + ClearNullIC");
    CHECK(CountIn(L, f0, f1, "bNeedReconcileHPList=true;") == 1 && CountIn(L, f0, f1, "bool bMixTeam=false;") == 1,
          "[2] H2: bMixTeam raises bNeedReconcileHPList");
    CHECK(CountIn(L, f0, f1, "HasHotReadyIC orphan cell :") == 1 && CountIn(L, f0, f1, "PickFromHPList->GetHPSuckGroupCount()") == 1,
          "[2] H3: orphan-cell Exception log + ledger group count");
    CHECK(CountIn(L, f0, f1, "FreePoint2DList(lsPoint2D);") == 3 && CountIn(L, f0, f1, "delete lsPoint2D;") == 0,
          "[2] H4: every return frees the uPoint2D list (no bare delete left)");
    const int fp = Find(L, "static void FreePoint2DList(TList* ls)");
    CHECK(fp >= 0 && fp < f0 && Find(L, "bool bNeedReconcileHPList=false;") >= 0 && Find(L, "bool bNeedReconcileHPList=false;") < f0,
          "[2] golden 913 :1463-1480 helpers sit ahead of the function");
    const std::vector<std::string> H = ReadLines(root + "/ainarm_SearchPickPlate.h");
    CHECK(CountIn(H, 0, (int)H.size() - 1, "extern bool bNeedReconcileHPList;") == 1, "[2] header: extern bNeedReconcileHPList (golden 913 .h:18)");
    const std::vector<std::string> F = ReadLines(root + "/aHotPlateSubstrate.h");
    int fc = -1;
    for (int i = 0; i < (int)F.size(); ++i) if (Trim(F[i]).find("void ClearGroupList();   int GetHPSuckGroupCount();") == 0) fc = i;
    CHECK(fc >= 0, "[2] facade uPlateInfo declares GetHPSuckGroupCount (definition only in Public/HTEditList.cpp)");

    std::printf("==== W-188 #3: %d passed, %d failed ====\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
