// =============================================================================
//  AI(W906-W188) 20261010 (NB2-1): W-188 #6 -- uPlateInfo::PurgePausedBookings = golden 913 (GitLab honprec/rd/rd5/ht9045_913
//  main e9908638; RogerYang 20260831f) Public/HTEditList.cpp:2820-2878, and the purge block at the head of golden 913
//  main.cpp TfMain::ReStartAutoSiteMapping (:29258-29275, 20260914 iResetSiteMappingStep gate), which the port's still-empty
//  TfMain::ReStartAutoSiteMapping (forms/fMain.cpp:248) now calls via W906_ReStartASMPurge913() (Public/HTEditList.cpp EOF).
//  A paused booking = bSuck false but coordinates still set (ASM borrowed the IC with SetPlateSuck(false) and never gave it
//  back).  At the ASM boundary those slots are wiped to -1, teams left empty are deleted, empty groups are deleted except the
//  tail open group; the purge is skipped while iResetSiteMappingStep>0 (One Cycle interrupted a borrow: the give-back is
//  still pending).
//  [1] behaviour on a real uPlateInfo ledger; [2] source ratchets (argv[1] = source root).
//  Use: only through ctest (NB2_W188PurgePaused).
// =============================================================================
#include "Public/HTEditList.h"
#include "Motor/mymotor.h"
#include "cmydef.h"
#include "w906_ctest_guard.h"
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

extern AnsiString ExString;                 // cMyDB.cpp: RecordProcess keeps the last text
extern int iResetSiteMappingStep;
void W906_ReStartASMPurge913();             // Public/HTEditList.cpp EOF

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
static int Count(const std::vector<std::string>& L, const std::string& n)
{
    int c = 0;
    for (size_t i = 0; i < L.size(); ++i) if (L[i].find(n) != std::string::npos) ++c;
    return c;
}
static bool Has(const AnsiString& s, const char* n) { return std::string(s.c_str()).find(n) != std::string::npos; }

static uHPSuckTeam* Team()
{
    uHPSuckTeam* t = new uHPSuckTeam();
    t->ClearHPSuckTeam();
    t->iP = 0;
    return t;
}
static void Slot(uHPSuckTeam* t, int i, int j, int c, int r, bool suck)
{
    t->bSuck[i][j] = suck; t->iPlateC[i][j] = c; t->iPlateR[i][j] = r; t->iSite[i][j] = 1;
}
// group 0: team A (one live slot + one paused slot) and team B (only paused); group 1: team C (only paused);
// group 2: open tail group with no team.  -> 3 slots wiped, B and C deleted, group 1 deleted, groups 0 and 2 left.
static uPlateInfo* Build(uHPSuckTeam** teamA)
{
    uPlateInfo* L = new uPlateInfo();
    uHPSuckGroup* g0 = new uHPSuckGroup();
    uHPSuckTeam* a = Team(); Slot(a, 0, 0, 1, 1, true); Slot(a, 0, 1, 2, 2, false);
    uHPSuckTeam* b = Team(); Slot(b, 0, 0, 3, 3, false);
    g0->AddHPSuckTeam(a); g0->AddHPSuckTeam(b);
    uHPSuckGroup* g1 = new uHPSuckGroup();
    uHPSuckTeam* c = Team(); Slot(c, 1, 0, 4, 4, false);
    g1->AddHPSuckTeam(c);
    uHPSuckGroup* g2 = new uHPSuckGroup();
    L->AddHPSuckGroup(g0); L->AddHPSuckGroup(g1); L->AddHPSuckGroup(g2);
    if (teamA) *teamA = a;
    return L;
}

int main(int argc, char** argv)
{
    extern AnsiString DataPath;
    const char* const rt[] = { "DataPath", DataPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("NB2_W188PurgePaused", rt)) return 2;
    std::printf("==== W-188 #6 PurgePausedBookings -> golden 913 ====\n");

    // ---------------------------------------------------------------- [1a] the method
    std::printf("-- [1a] uPlateInfo::PurgePausedBookings --\n");
    uHPSuckTeam* a = 0;
    uPlateInfo* L = Build(&a);
    const int n = L->PurgePausedBookings();
    char msg[160];
    std::snprintf(msg, sizeof msg, "returns 3 wiped paused slots (got %d)", n);
    CHECK(n == 3, msg);
    CHECK(L->GetHPSuckGroupCount() == 2, "middle group emptied -> deleted; tail open group kept even though empty");
    uHPSuckGroup* g0 = L->ExtractSuckGroup(0);
    CHECK(g0 && g0->GetTeamCount() == 1 && g0->ExtractSuckTeam(0) == a, "all-paused team deleted, team with a live slot kept");
    CHECK(a->bSuck[0][0] && a->iPlateC[0][0] == 1 && a->iPlateR[0][0] == 1 && a->iSite[0][0] == 1, "live booking untouched");
    CHECK(!a->bSuck[0][1] && a->iPlateC[0][1] == -1 && a->iPlateR[0][1] == -1 && a->iSite[0][1] == -1, "paused slot wiped to -1");
    CHECK(L->PurgePausedBookings() == 0 && L->GetHPSuckGroupCount() == 2, "second call: nothing left to purge (unused -1 slots are not counted)");

    // ---------------------------------------------------------------- [1b] the ReStartAutoSiteMapping purge block
    std::printf("-- [1b] ReStartAutoSiteMapping purge block (iResetSiteMappingStep gate) --\n");
    uPlateInfo* const saveList = PickFromHPList;
    const int saveStep = iResetSiteMappingStep;
    uPlateInfo* L2 = Build(0);
    PickFromHPList = L2;
    iResetSiteMappingStep = 1;
    ExString = "";
    W906_ReStartASMPurge913();
    CHECK(L2->GetHPSuckGroupCount() == 3 && ExString.Length() == 0, "iResetSiteMappingStep>0 (give-back pending) -> ledger untouched, no log");
    iResetSiteMappingStep = 0;
    W906_ReStartASMPurge913();
    CHECK(L2->GetHPSuckGroupCount() == 2, "iResetSiteMappingStep==0 -> purged");
    CHECK(Has(ExString, "ASM boundary : purge 3 paused booking slot(s), grp=2"), "golden 913 RecordProcess line");
    std::printf("     (%s)\n", ExString.c_str());
    ExString = "";
    W906_ReStartASMPurge913();
    CHECK(ExString.Length() == 0, "nothing purged -> no log line");
    PickFromHPList = 0;
    W906_ReStartASMPurge913();
    CHECK(true, "PickFromHPList==NULL -> no crash");
    PickFromHPList = saveList;
    iResetSiteMappingStep = saveStep;

    // ---------------------------------------------------------------- [2] source ratchets
    std::printf("-- [2] source ratchets --\n");
    const std::string root = argc > 1 ? argv[1] : ".";
    const std::vector<std::string> M = ReadLines(root + "/forms/fMain.cpp");
    CHECK(Count(M, "void TfMain::ReStartAutoSiteMapping(bool /*bStart*/) { void W906_ReStartASMPurge913(); W906_ReStartASMPurge913(); }") == 1,
          "forms/fMain.cpp ReStartAutoSiteMapping calls the 913 purge block");
    const std::vector<std::string> H = ReadLines(root + "/Public/HTEditList.cpp");
    CHECK(Count(H, "int uPlateInfo::PurgePausedBookings()") == 1 && Count(H, "void W906_ReStartASMPurge913()") == 1
          && Count(H, "iResetSiteMappingStep==0)") == 1 && Count(H, "ig!=HPSuckGroupList->Count-1)") == 1,
          "Public/HTEditList.cpp: method (tail open group kept) + purge block with the 20260914 gate");
    const std::vector<std::string> D = ReadLines(root + "/Public/HTEditList.h");
    CHECK(Count(D, "bool bSuck);   int  PurgePausedBookings();") == 1, "Public/HTEditList.h declaration in code, not after the // comment (golden 913 .h:214)");

    std::printf("==== W-188 #6: %d passed, %d failed ====\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
