// =============================================================================
//  AI(W906-W188) 20261009 (NB2-1): W-188 #5 -- ReconcilePickFromHPList = golden 913 (GitLab honprec/rd/rd5/ht9045_913 main
//  e9908638; RogerYang 20260827) ainarm_SearchPickPlate.cpp:1481-1636 + its call ainarm9045.cpp:4506-4511, and
//  uHPSuckGroup::RemoveSuckTeam (golden 913 Public/HTEditList.cpp:2939-2946).
//  The hot-plate pick ledger (PickFromHPList) is only ever reduced: bookings that point at an empty plate cell (phantom) or
//  repeat a cell already booked (duplicate) are cleared, emptied teams / groups are deleted, an open group is re-added when
//  nothing is left, the ledger is saved and one RecordProcess line is written.  -1 = arm busy (flag kept for the next pass).
//  [1] behaviour through the real entry point (ainarm_SearchPickPlate.cpp; this TU sees only the real uPlateInfo):
//      phantom + duplicate ledger -> 3 fixes, 2 teams + 1 group deleted, live booking kept, file written, log line;
//      second call -> 0, nothing written; everything phantom -> open group re-added; bRunAutoSiteMapping -> -1, untouched.
//  [2] source ratchets (argv[1] = source root).
//  sHPPickRec is pointed at a %TEMP% file for the whole test (the real one is d:\HT9045\system\PickHPRec.json).
//  Use: only through ctest (NB2_W188Reconcile).
// =============================================================================
#include "Public/HTEditList.h"
#include "Motor/mymotor.h"
#include "cmydef.h"
#include "w906_ctest_guard.h"
#include <windows.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

extern AnsiString ExString;                       // cMyDB.cpp: RecordProcess keeps the last text
extern bool bRunAutoSiteMapping;
int ReconcilePickFromHPList(const char* szWho);   // ainarm_SearchPickPlate.cpp (facade TU)

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
static int FindLine(const std::vector<std::string>& L, const std::string& n)
{
    for (int i = 0; i < (int)L.size(); ++i) if (L[i].find(n) != std::string::npos) return i;
    return -1;
}
static bool Has(const AnsiString& s, const char* n) { return std::string(s.c_str()).find(n) != std::string::npos; }

static uHPSuckTeam* Team(int iP)
{
    uHPSuckTeam* t = new uHPSuckTeam();
    t->ClearHPSuckTeam();
    t->iP = iP;
    return t;
}
static void Book(uHPSuckTeam* t, int i, int j, int c, int r, int site)
{
    t->bSuck[i][j] = true; t->iPlateC[i][j] = c; t->iPlateR[i][j] = r; t->iSite[i][j] = site;
}

int main(int argc, char** argv)
{
    extern AnsiString DataPath;
    const char* const rt[] = { "DataPath", DataPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("NB2_W188Reconcile", rt)) return 2;
    std::printf("==== W-188 #5 ReconcilePickFromHPList -> golden 913 ====\n");

    char tmp[MAX_PATH] = { 0 };
    ::GetTempPathA(MAX_PATH, tmp);
    const std::string rec = std::string(tmp) + "nb2_w188e_PickHPRec.json";
    ::DeleteFileA(rec.c_str());
    const AnsiString saveRec = sHPPickRec;
    sHPPickRec = AnsiString(rec.c_str());
    uPlateInfo* const saveList = PickFromHPList;
    const bool saveASM = bRunAutoSiteMapping;
    bRunAutoSiteMapping = false;

    const int saveX = MOT[MMPlate1].Tray.XItem, saveY = MOT[MMPlate1].Tray.YItem;
    const int d11 = MOT[MMPlate1].Tray.Data[1][1], d22 = MOT[MMPlate1].Tray.Data[2][2];
    MOT[MMPlate1].Tray.XItem = 4; MOT[MMPlate1].Tray.YItem = 4;
    MOT[MMPlate1].Tray.Data[1][1] = HAS_IC;      // real IC at C1 R1
    MOT[MMPlate1].Tray.Data[2][2] = NULL_IC;     // empty cell C2 R2

    // ---------------------------------------------------------------- [1a] phantom + duplicate
    std::printf("-- [1a] phantom + duplicate ledger --\n");
    uPlateInfo* L = new uPlateInfo();
    uHPSuckGroup* g0 = new uHPSuckGroup();
    uHPSuckTeam* t00 = Team(0); Book(t00, 0, 0, 1, 1, 1); Book(t00, 0, 1, 2, 2, 2);   // live + phantom
    uHPSuckTeam* t01 = Team(0); Book(t01, 0, 0, 1, 1, 3);                            // duplicate of C1 R1 only
    g0->AddHPSuckTeam(t00); g0->AddHPSuckTeam(t01);
    uHPSuckGroup* g1 = new uHPSuckGroup();
    uHPSuckTeam* t10 = Team(0); Book(t10, 0, 0, 2, 2, 4);                            // phantom only
    g1->AddHPSuckTeam(t10);
    L->AddHPSuckGroup(g0); L->AddHPSuckGroup(g1);
    PickFromHPList = L;

    ExString = "";
    int r = ReconcilePickFromHPList("NB2T");
    char msg[200];
    std::snprintf(msg, sizeof msg, "returns phantom+dup = 3 (got %d)", r);
    CHECK(r == 3, msg);
    CHECK(L->GetHPSuckGroupCount() == 1, "phantom-only group deleted, one group left");
    uHPSuckGroup* left = L->ExtractSuckGroup(0);
    CHECK(left == g0 && left->GetTeamCount() == 1, "duplicate-only team deleted, first team kept");
    uHPSuckTeam* kept = left ? left->ExtractSuckTeam(0) : 0;
    CHECK(kept && kept->bSuck[0][0] && kept->iPlateC[0][0] == 1 && kept->iPlateR[0][0] == 1 && kept->iSite[0][0] == 1,
          "live booking (C1 R1, site 1) kept with its nozzle binding");
    CHECK(kept && !kept->bSuck[0][1] && kept->iPlateC[0][1] == -1 && kept->iPlateR[0][1] == -1 && kept->iSite[0][1] == -1,
          "phantom slot cleared to -1");
    CHECK(Has(ExString, "ReconcilePickFromHPList by NB2T : phantom=2 dup=1 delTeam=2 delGroup=1 groupLeft=1"),
          "golden 913 RecordProcess summary line");
    std::printf("     (%s)\n", ExString.c_str());
    CHECK(::GetFileAttributesA(rec.c_str()) != INVALID_FILE_ATTRIBUTES, "ledger saved to sHPPickRec");

    ::DeleteFileA(rec.c_str());
    ExString = "";
    r = ReconcilePickFromHPList("NB2T");
    CHECK(r == 0 && ExString.Length() == 0 && ::GetFileAttributesA(rec.c_str()) == INVALID_FILE_ATTRIBUTES,
          "clean ledger -> 0, no log, no save");

    // ---------------------------------------------------------------- [1b] everything phantom -> open group re-added
    std::printf("-- [1b] all phantom --\n");
    uPlateInfo* L2 = new uPlateInfo();
    uHPSuckGroup* h0 = new uHPSuckGroup();
    uHPSuckTeam* u0 = Team(0); Book(u0, 0, 0, 2, 2, 1);
    h0->AddHPSuckTeam(u0);
    L2->AddHPSuckGroup(h0);
    PickFromHPList = L2;
    r = ReconcilePickFromHPList("NB2T");
    CHECK(r == 1 && L2->GetHPSuckGroupCount() == 1 && L2->ExtractSuckGroup(0) != 0 && L2->ExtractSuckGroup(0)->GetTeamCount() == 0,
          "all bookings phantom -> group deleted and one open (empty) group re-added");

    // ---------------------------------------------------------------- [1c] busy gate
    std::printf("-- [1c] busy gate --\n");
    uPlateInfo* L3 = new uPlateInfo();
    uHPSuckGroup* k0 = new uHPSuckGroup();
    uHPSuckTeam* v0 = Team(0); Book(v0, 0, 0, 2, 2, 1);
    k0->AddHPSuckTeam(v0);
    L3->AddHPSuckGroup(k0);
    PickFromHPList = L3;
    bRunAutoSiteMapping = true;
    r = ReconcilePickFromHPList("NB2T");
    bRunAutoSiteMapping = false;
    CHECK(r == -1 && L3->GetHPSuckGroupCount() == 1 && v0->bSuck[0][0], "bRunAutoSiteMapping -> -1 and the ledger is untouched");
    PickFromHPList = 0;
    CHECK(ReconcilePickFromHPList("NB2T") == 0, "PickFromHPList==NULL -> 0");

    PickFromHPList = saveList;
    bRunAutoSiteMapping = saveASM;
    MOT[MMPlate1].Tray.XItem = saveX; MOT[MMPlate1].Tray.YItem = saveY;
    MOT[MMPlate1].Tray.Data[1][1] = d11; MOT[MMPlate1].Tray.Data[2][2] = d22;
    sHPPickRec = saveRec;
    ::DeleteFileA(rec.c_str());

    // ---------------------------------------------------------------- [2] source ratchets
    std::printf("-- [2] source ratchets --\n");
    const std::string root = argc > 1 ? argv[1] : ".";
    const std::vector<std::string> A = ReadLines(root + "/ainarm_SearchPickPlate.cpp");
    const int e = FindLine(A, "int ReconcilePickFromHPList(const char* szWho)");
    CHECK(e > 0 && FindLine(A, "bool bNeedReconcileHPList=false;") < e && e < FindLine(A, "bool HasHotReadyIC_9045()"),
          "entry point sits between bNeedReconcileHPList and HasHotReadyIC_9045 (golden 913 order)");
    CHECK(Count(A, "LastSet.iRunStartMode==rsmAutoSiteMap     )") == 1 && Count(A, "return W906_ReconcileHPLedger913(szWho);") == 1,
          "busy gate verbatim + delegation to the ledger passes");
    const std::vector<std::string> H = ReadLines(root + "/Public/HTEditList.cpp");
    const int w = FindLine(H, "int W906_ReconcileHPLedger913(const char* szWho)");
    CHECK(w > 0 && Count(H, "uHPSuckTeam* HPDel=HPGroup->RemoveSuckTeam(it);") >= 1 && Count(H, "PickFromHPList->SaveFile(sHPPickRec);") == 1
          && Count(H, "uHPSuckTeam* uHPSuckGroup::RemoveSuckTeam(int iIndex)") == 1, "ledger passes + RemoveSuckTeam in Public/HTEditList.cpp");
    const std::vector<std::string> I = ReadLines(root + "/ainarm9045.cpp");
    CHECK(Count(I, "if(bNeedReconcileHPList==true && bRunAutoSiteMapping==false) { if(ReconcilePickFromHPList(\"DoInArm_9045\")>=0) bNeedReconcileHPList=false; }") == 1,
          "DoInArm_9045 calls it after the ASM decision (golden 913 ainarm9045.cpp:4506-4511)");
    const std::vector<std::string> P = ReadLines(root + "/ainarm_SearchPickPlate.h");
    CHECK(Count(P, "extern int  ReconcilePickFromHPList(const char* szWho);") == 1, "header extern (golden 913 .h:19)");

    std::printf("==== W-188 #5: %d passed, %d failed ====\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
