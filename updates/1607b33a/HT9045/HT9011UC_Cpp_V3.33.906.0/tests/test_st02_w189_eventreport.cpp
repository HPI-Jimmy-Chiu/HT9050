// =============================================================================
//  test_st02_w189_eventreport.cpp -- W-189: THGem's EventReport_CEID.def / EventReport_ReportID.def go to a ctest sandbox
//  (W906_SECSSYSTEM_ROOT, SECSGEM/uHGemEquipment.cpp W906_SecsSystemDir) instead of the golden literal
//  D:\HT9045\SECS\SECS\SYSTEM\ (golden 0618 uHGemEquipment.cpp:8202/:8212/:8657/:8661, 913 :8281/:8291/:8736/:8740).
//
//  AI(W906-W189) 20261009 (St02-E).  Suite name (add_test): St02_W189EventReport.
//    [0] containment: the ctest roots, W906_SECSSYSTEM_ROOT set and not under D:\HT9045\SECS; this test moves it to its own
//        sub-folder (ctest -j: uHGemEquipment [7] copies into the shared one); size + mtime of the two real .def files.
//    [1] READ probe first: known .def files in the sandbox -> ReadEventReportData shows THEIR rows.  If not, STOP before any
//        Save -- a broken seam can never write the real files.
//    [2] SaveEventReportData -> both files in the sandbox, line for line CopyStringGridAsTabFormat; the real files unchanged.
//    [3] a fresh THGem reads them back (round trip).
//    [4] golden 913 SetReportIDContent(.., bSaveNow=false) (uHGemEquipment.cpp:7521/:7561-7564, RogerYang 20260812): the row is
//        set, nothing saved; the default (true) saves as before.
//    [5] HT9045Gem::AddReprot (913 uHGemHT9045.cpp:460): exactly ONE save (0618: one per event + one).  No live caller in the
//        port (golden's only caller is the TFSECS ctor, 913 UsecegemMainFrom.cpp:206); HT9045Gem is built directly on a THGem
//        (no SystemModularInitial).
//  The real files are only stat'ed (GetFileAttributesEx), never opened.
// =============================================================================
#include "SECSGEM/uHGemEquipment.h"
#include "SECSGEM/uHGemHT9045.h"
#include "SECSGEM/SecsEventType.h"
#include "CosFunction.h"
#include "st02_test_containment.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

int W906_SecsSystemDirUses_St02();   // SECSGEM/uHGemEquipment.cpp file end

static int g_pass = 0, g_fail = 0;
static bool Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return true; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
    return false;
}

static const char* const kReal[2] = { "D:\\HT9045\\SECS\\SECS\\SYSTEM\\EventReport_CEID.def",
                                       "D:\\HT9045\\SECS\\SECS\\SYSTEM\\EventReport_ReportID.def" };
static const char* const kName[2] = { "EventReport_CEID.def", "EventReport_ReportID.def" };

struct Stamp { bool exists; unsigned long long size; unsigned long long mtime; };
static Stamp StampOf(const char* p)
{
    Stamp s = { false, 0, 0 };
    WIN32_FILE_ATTRIBUTE_DATA d;
    if (GetFileAttributesExA(p, GetFileExInfoStandard, &d))
    {
        s.exists = true;
        s.size = ((unsigned long long)d.nFileSizeHigh << 32) | d.nFileSizeLow;
        s.mtime = ((unsigned long long)d.ftLastWriteTime.dwHighDateTime << 32) | d.ftLastWriteTime.dwLowDateTime;
    }
    return s;
}
static std::string StampText(const Stamp& s)
{
    if (!s.exists) return "absent";
    return std::to_string(s.size) + " bytes, mtime " + std::to_string(s.mtime);   // no %llu: MinGW.org's printf is msvcrt's
}
static bool SameStamp(const Stamp& a, const Stamp& b) { return a.exists == b.exists && a.size == b.size && a.mtime == b.mtime; }

static bool WriteText(const std::string& path, const std::string& text)
{
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) return false;
    const bool ok = std::fwrite(text.data(), 1, text.size(), f) == text.size();
    std::fclose(f);
    return ok;
}
static std::vector<std::string> ReadLines(const std::string& path)
{
    std::vector<std::string> out;
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return out;
    std::string all;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) all.append(buf, n);
    std::fclose(f);
    std::string cur;
    for (size_t i = 0; i < all.size(); ++i)
    {
        if (all[i] == '\r') continue;
        if (all[i] == '\n') { out.push_back(cur); cur.clear(); continue; }
        cur += all[i];
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}
static bool SameAsGrid(THGem& g, TStringGrid* grid, const std::string& path, std::string& why)
{
    TStringList want;
    g.CopyStringGridAsTabFormat(grid, &want);
    const std::vector<std::string> got = ReadLines(path);
    if ((int)got.size() != want.Count) { why = "lines " + std::to_string(got.size()) + " vs grid " + std::to_string(want.Count); return false; }
    for (int i = 0; i < want.Count; ++i)
        if (got[i] != std::string(want.GetString(i).c_str())) { why = "line " + std::to_string(i) + " differs"; return false; }
    why = std::to_string(got.size()) + " lines";
    return true;
}

static char g_env[1024];

int main()
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W189EventReport -- THGem Read/SaveEventReportData follow W906_SECSSYSTEM_ROOT under ctest\n");
    if (!W906TestInsideCtestRoots("St02_W189EventReport"))
        return 2;

    // ---------------------------------------------------------------- [0]
    std::printf("[0] containment\n");
    const char* base = std::getenv("W906_SECSSYSTEM_ROOT");
    const std::string lbase = W906TestSafeLower(base);
    std::printf("  W906_SECSSYSTEM_ROOT = %s\n", base ? base : "(unset)");
    if (!Check(base != NULL && *base != 0 && lbase.find("machine_config_scratch") != std::string::npos &&
               lbase.find("\\ht9045\\secs\\") == std::string::npos,
               "[0] W906_SECSSYSTEM_ROOT is ctest's machine_config_scratch, not D:\\HT9045\\SECS"))
        return 2;
    const std::string own = std::string(base) + "\\St02_W189";
    std::snprintf(g_env, sizeof(g_env), "W906_SECSSYSTEM_ROOT=%s", own.c_str());
    HT9045_TEST_PUTENV(g_env);
    const char* now = std::getenv("W906_SECSSYSTEM_ROOT");
    if (!Check(now != NULL && own == now, "[0] moved to this test's own folder " + own))
        return 2;
    ForceDirectories(AnsiString(own.c_str()));
    std::string ownFile[2];
    for (int i = 0; i < 2; ++i) { ownFile[i] = own + "\\" + kName[i]; DeleteFileA(ownFile[i].c_str()); }
    Stamp before[2];
    for (int i = 0; i < 2; ++i)
    {
        before[i] = StampOf(kReal[i]);
        std::printf("  real %s: %s\n", kName[i], StampText(before[i]).c_str());
    }

    // ---------------------------------------------------------------- [1]
    std::printf("[1] READ probe (before any Save)\n");
    THGem g;
    Check(WriteText(ownFile[0], "CEID\tEnable\tAlias\tReportID\t\r\n77\t1\tW189Probe\t5\t\r\n") &&
          WriteText(ownFile[1], "ReportID\tType\t\r\n5\t2\t1027\t\r\n"), "[1] probe files written in the sandbox");
    int uses = W906_SecsSystemDirUses_St02();
    g.ReadEventReportData();
    const bool readOk = Check(g.strGrdCEID->Cells[0][1] == "77" && g.strGrdCEID->Cells[2][1] == "W189Probe" &&
                              g.stdGridReportID->Cells[0][1] == "5" && g.stdGridReportID->Cells[2][1] == "1027",
                              "[1] ReadEventReportData shows the sandbox rows (CEID row 1 = '" + std::string(g.strGrdCEID->Cells[0][1].c_str()) +
                              "', ReportID row 1 = '" + std::string(g.stdGridReportID->Cells[0][1].c_str()) + "')");
    Check(W906_SecsSystemDirUses_St02() == uses + 2, "[1] two file names built (" + std::to_string(W906_SecsSystemDirUses_St02() - uses) + ")");
    if (!readOk)
    {
        std::printf("  STOP: the read did not follow W906_SECSSYSTEM_ROOT -- no Save is attempted\n");
        std::printf("\nSt02_W189EventReport: %d passed, %d failed\n", g_pass, g_fail);
        return 1;
    }

    // ---------------------------------------------------------------- [2]
    std::printf("[2] SaveEventReportData\n");
    for (int i = 0; i < 2; ++i) DeleteFileA(ownFile[i].c_str());
    g.strGrdCEID->Cells[0][2] = 88;
    g.strGrdCEID->Cells[1][2] = 0;
    g.strGrdCEID->Cells[2][2] = "W189Saved";
    g.strGrdCEID->Cells[3][2] = 6;
    g.stdGridReportID->Cells[0][2] = 6;
    g.stdGridReportID->Cells[1][2] = 1;
    g.stdGridReportID->Cells[2][2] = 1028;
    uses = W906_SecsSystemDirUses_St02();
    g.SaveEventReportData();
    Check(W906_SecsSystemDirUses_St02() == uses + 2, "[2] two file names built");
    std::string why;
    bool same = SameAsGrid(g, g.strGrdCEID, ownFile[0], why);   // first, then the message (argument order is unspecified)
    Check(same, "[2] sandbox EventReport_CEID.def == the CEID grid (" + why + ")");
    same = SameAsGrid(g, g.stdGridReportID, ownFile[1], why);
    Check(same, "[2] sandbox EventReport_ReportID.def == the ReportID grid (" + why + ")");
    for (int i = 0; i < 2; ++i)
    {
        const Stamp s = StampOf(kReal[i]);
        Check(SameStamp(s, before[i]), std::string("[2] real ") + kName[i] + " unchanged (" + StampText(s) + ")");
    }

    // ---------------------------------------------------------------- [3]
    std::printf("[3] a fresh THGem reads the saved files\n");
    {
        THGem g2;
        g2.ReadEventReportData();
        Check(g2.strGrdCEID->Cells[0][1] == "77" && g2.strGrdCEID->Cells[0][2] == "88" && g2.strGrdCEID->Cells[2][2] == "W189Saved" &&
              g2.strGrdCEID->Cells[3][2] == "6", "[3] CEID rows 77 / 88 W189Saved -> 6 read back");
        Check(g2.stdGridReportID->Cells[0][2] == "6" && g2.stdGridReportID->Cells[2][2] == "1028", "[3] ReportID row 6 -> 1028 read back");
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] SetReportIDContent(.., bSaveNow=false) -- golden 913 uHGemEquipment.cpp:7521 / :7561-7564\n");
    for (int i = 0; i < 2; ++i) DeleteFileA(ownFile[i].c_str());
    {
        unsigned ids[1] = { 1029 };
        uses = W906_SecsSystemDirUses_St02();
        const bool r = g.SetReportIDContent(9, 1, ids, 1, false);
        int row = -1;
        for (int y = 1; y < g.stdGridReportID->RowCount; ++y)
            if (g.stdGridReportID->Cells[0][y] == "9") { row = y; break; }
        Check(r && row > 0 && g.stdGridReportID->Cells[2][row] == "1029", "[4] false: ReportID 9 -> 1029 set in the grid (row " + std::to_string(row) + ")");
        Check(W906_SecsSystemDirUses_St02() == uses && !StampOf(ownFile[0].c_str()).exists && !StampOf(ownFile[1].c_str()).exists,
              "[4] false: nothing saved (" + std::to_string(W906_SecsSystemDirUses_St02() - uses) + " file names built, files absent)");
        ids[0] = 1030;
        uses = W906_SecsSystemDirUses_St02();
        g.SetReportIDContent(9, 1, ids, 1);
        same = SameAsGrid(g, g.stdGridReportID, ownFile[1], why);
        Check(W906_SecsSystemDirUses_St02() == uses + 2 && same,
              "[4] default (true): saved, file == the grid (" + why + ")");
    }

    // ---------------------------------------------------------------- [5]
    std::printf("[5] HT9045Gem::AddReprot -- golden 913 uHGemHT9045.cpp:451-464\n");
    for (int i = 0; i < 2; ++i) DeleteFileA(ownFile[i].c_str());
    {
        const bool savedEnable = CosFunction.bEnable_SECS_GEM;
        THGem* const savedHGem = HGem;
        CosFunction.bEnable_SECS_GEM = true;
        HGem = &g;
        {
            HT9045Gem mg("HT9045", &g);
            const int n = SECS_EVENT.TotalEvent - SECS_EVENT.DoStart;
            uses = W906_SecsSystemDirUses_St02();
            mg.AddReprot();
            const int built = W906_SecsSystemDirUses_St02() - uses;
            Check(built == 2, "[5] exactly one save (" + std::to_string(built / 2) + " saves; 0618 would be " + std::to_string(n + 1) + " for " + std::to_string(n) + " events)");
            int row = -1;
            for (int y = 1; y < g.stdGridReportID->RowCount; ++y)
                if (g.stdGridReportID->Cells[0][y] == "1") { row = y; break; }
            Check(row > 0 && g.stdGridReportID->Cells[1][row] == "1" && g.stdGridReportID->Cells[2][row] == "1027" && g.stdGridReportID->Cells[3][row] == "",
                  "[5] ReportID 1 = Mode 1, {1027} (System Time) (row " + std::to_string(row) + ")");
            same = SameAsGrid(g, g.stdGridReportID, ownFile[1], why);
            Check(same, "[5] the saved ReportID file == the grid (" + why + ")");
        }
        HGem = savedHGem;
        CosFunction.bEnable_SECS_GEM = savedEnable;
    }

    // ---------------------------------------------------------------- end
    std::printf("[end] the real files\n");
    for (int i = 0; i < 2; ++i)
    {
        const Stamp s = StampOf(kReal[i]);
        Check(SameStamp(s, before[i]), std::string("[end] real ") + kName[i] + " unchanged (" + StampText(s) + ")");
    }
    if (g_fail == 0)
        for (int i = 0; i < 2; ++i) DeleteFileA(ownFile[i].c_str());
    std::printf("\nSt02_W189EventReport: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
