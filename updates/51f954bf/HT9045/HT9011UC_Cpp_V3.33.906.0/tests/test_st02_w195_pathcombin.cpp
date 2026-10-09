// =============================================================================
//  test_st02_w195_pathcombin.cpp -- W-195 MR-A: a file part that already starts with "\" gets no second separator, and
//                                    TfBinSel::ReadFile's SavePath names carry no leading "\" (golden 913).
//
//  AI(W906-W195) 20261009 (St02-E).  Suite name (add_test): St02_W195PathCombin.  argv[1] = the tree root (read only).
//    golden 913 ProductionInfo/FileInfo.cpp:304-305 (Steven 20260708 : Add protection): the local-path branch of
//      FileInfo::PathCombin adds "\" only when the path does not end with one AND sFile.AnsiPos("\\")!=1.  The port has the
//      golden function three times: FileInfo::PathCombin (ProductionInfo/FileInfo.cpp:471), Common_PathCombin (common.cpp:2506,
//      behind GetRecipeFileName / CopyAndCompressFile) and MySL_PathCombin (Public/MyStringList.cpp:195).
//    golden 913 cBinSel.cpp:1128 / :1135 / :1139 / :1146 / :1150: ReadFile's SavePath names without the leading "\".
//    [1] FileInfo().PathCombin, the six cases (only the first one changes);
//    [2] GetRecipeFileName("\Binasgn.Data") = GetRecipePath() + "Binasgn.Data" (Common_PathCombin);
//    [3] fBinSel->ReadFile(false, false, "") in a sandbox: every Binasgn*.Data path its ReadFunctionData -> SaveFunctionData
//        writes hand the change-log hooks (common.cpp :1100 int, :1234 string) has one backslash before the name;
//    [4] source: the five SavePath lines in cBinSel.cpp and the three guards carry the golden 913 text.
//  Reverse checks (recorded in the MR): drop the second condition in FileInfo.cpp -> [1] red; in common.cpp -> [2] red; put
//  "\\" back on one SavePath line -> [4] red; revert both cBinSel.cpp and common.cpp -> [3] red.  Either change alone keeps
//  [3] green -- each already gives ReadFile one backslash; golden 913 has both, so [4] pins the names.
//  Every path is under %TEMP%\ht9045_w195_<tick> (DataPath / LastDataPath / asTeachPath set here; asGeneralPath is this test's
//  scratch copy, tests/test_bootstrap.cpp); the test aborts before ReadFile if one of them is under D:\HT9045.  The sandbox
//  is removed on a green run.
// =============================================================================
#include "ProductionInfo/FileInfo.h"
#include "database.h"
#include "csystem.h"
#include "cprod.h"
#include "common.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "MachineType.h"
#include "forms/fBinSel.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "forms/fTemp_Set.h"   // AI(W906-N1G5) 20261009 (laptop, batch 136 integration): fTemp_Set before ReadFile reaches ChangeSite (see main)

static int g_pass = 0, g_fail = 0;
static char g_got[700];
#define GOT(...) std::snprintf(g_got, sizeof(g_got), __VA_ARGS__)
#define CHECK(cond, msg)                                                                                   \
    do {                                                                                                   \
        if (cond) { printf("  PASS: %s   [%s]\n", msg, g_got); ++g_pass; }                                 \
        else      { printf("  FAIL: %s   [got %s]  (line %d)\n", msg, g_got, __LINE__); ++g_fail; }        \
        g_got[0] = 0;                                                                                      \
    } while (0)

static std::string g_root;

static std::string Slurp(const std::string& p)
{
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return std::string();
    std::string s;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) s.append(buf, n);
    std::fclose(f);
    return s;
}

static void WriteText(const std::string& p, const char* text)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (f) { std::fputs(text, f); std::fclose(f); }
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

static bool UnderMachineTree(const std::string& p)
{
    std::string s = p;
    for (size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    for (size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\';
    if (s.find("\\obj\\v906\\") != std::string::npos) return false;   // a build dir under D:\HT9045 is a sandbox (as St02_SecsEcFileWrites)
    return s.compare(0, 9, "d:\\ht9045") == 0;
}

// the line of `text` that contains `anchor` (first match), or "" -- source pins look at that one line only
static std::string LineWith(const std::string& text, const std::string& anchor)
{
    const size_t at = text.find(anchor);
    if (at == std::string::npos) return std::string();
    const size_t a = text.rfind('\n', at), b = text.find('\n', at);
    return text.substr(a == std::string::npos ? 0 : a + 1, (b == std::string::npos ? text.size() : b) - (a == std::string::npos ? 0 : a + 1));
}

// one backslash right before "Binasgn" and no "\\" pair anywhere (a UNC path never reaches here: the sandbox is %TEMP%)
static bool OneBackslash(const std::string& f)
{
    const size_t at = f.find("Binasgn");
    return at != std::string::npos && at >= 1 && f[at - 1] == '\\' && f.find("\\\\") == std::string::npos;
}

// every WriteIniData overload hands its file name to its change-log hook (common.cpp :1057 bool, :1100 int, :1146 double,
// :1234 string); the capture chains to whatever was installed before
static std::vector<std::string> g_files;
static W906_ChangeLogFn_Str g_prevStr = 0;
static W906_ChangeLogFn_Int g_prevInt = 0;
static W906_ChangeLogFn_Bool g_prevBool = 0;
static W906_ChangeLogFn_Double g_prevDouble = 0;
static void CaptureStr(AnsiString F, AnsiString G, AnsiString N, AnsiString ret, AnsiString V, AnsiString& s1, AnsiString& s2, bool& b)
{
    g_files.push_back(F.c_str());
    if (g_prevStr) g_prevStr(F, G, N, ret, V, s1, s2, b);
}
static void CaptureInt(AnsiString F, AnsiString G, AnsiString N, int ret, int V, AnsiString& s1, AnsiString& s2, bool& b)
{
    g_files.push_back(F.c_str());
    if (g_prevInt) g_prevInt(F, G, N, ret, V, s1, s2, b);
}
static void CaptureBool(AnsiString F, AnsiString G, AnsiString N, bool ret, bool V, AnsiString& s1, AnsiString& s2, bool& b)
{
    g_files.push_back(F.c_str());
    if (g_prevBool) g_prevBool(F, G, N, ret, V, s1, s2, b);
}
static void CaptureDouble(AnsiString F, AnsiString G, AnsiString N, double ret, double V, AnsiString S, AnsiString& s1, AnsiString& s2, bool& b)
{
    g_files.push_back(F.c_str());
    if (g_prevDouble) g_prevDouble(F, G, N, ret, V, S, s1, s2, b);
}

int main(int argc, char** argv)
{
    printf("St02_W195PathCombin\n");
    if (argc < 2) { printf("  ABORT: argv[1] (the tree root) missing\n"); return 2; }
    const std::string src = std::string(argv[1]) + "/";

    // ---- [1] FileInfo::PathCombin (golden 913 ProductionInfo/FileInfo.cpp:290-313) ---------------------------------------
    {
        struct Case { const char* path; const char* file; const char* want; const char* why; };
        const Case cs[] = {
            { "C:\\a",   "\\b", "C:\\a\\b",   "file part starts with \\ -> no second one (W-195, the only changed case)" },
            { "C:\\a",   "b",   "C:\\a\\b",   "plain name -> one separator added" },
            { "C:\\a\\", "b",   "C:\\a\\b",   "path ends with \\ -> nothing added" },
            { "C:\\a\\", "\\b", "C:\\a\\\\b", "path ends with \\ and file starts with \\ -> both kept, as golden 913" },
            { "",        "\\b", "\\b",        "empty path -> the file part alone" },
            { "x/y",     "\\b", "x/y/\\b",    "FTP path ('/') -> '/' added; golden 913 guards only the local branch" },
        };
        for (size_t i = 0; i < sizeof(cs) / sizeof(cs[0]); ++i)
        {
            const std::string got = FileInfo().PathCombin(AnsiString(cs[i].path), AnsiString(cs[i].file)).c_str();
            GOT("PathCombin(\"%s\", \"%s\") = \"%s\", want \"%s\"", cs[i].path, cs[i].file, got.c_str(), cs[i].want);
            char msg[200];
            std::snprintf(msg, sizeof(msg), "1. FileInfo::PathCombin: %s", cs[i].why);
            CHECK(got == cs[i].want, msg);
        }
    }

    // ---- sandbox -------------------------------------------------------------------------------------------------------
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    g_root = std::string(tmp) + "ht9045_w195_" + stamp;
    ::CreateDirectoryA(g_root.c_str(), 0);
    ::CreateDirectoryA((g_root + "\\Data").c_str(), 0);
    ::CreateDirectoryA((g_root + "\\Data\\W195R").c_str(), 0);
    WriteText(g_root + "\\SetUp.inf", "W195R\r\n");
    DataPath     = AnsiString((g_root + "\\Data\\").c_str());
    LastDataPath = AnsiString((g_root + "\\SetUp.inf").c_str());
    // ReadFile -> SetWorkParameter -> ReadTechData reads teach.ini and the general ini (as St02_SecsEcFileWrites)
    asTeachPath  = AnsiString((g_root + "\\teach.ini").c_str());
    const std::string recipe = std::string(GetRecipePath().c_str());
    if (UnderMachineTree(std::string(DataPath.c_str())) || UnderMachineTree(recipe) || UnderMachineTree(std::string(asTeachPath.c_str())) ||
        UnderMachineTree(std::string(asGeneralPath.c_str())) || recipe.compare(0, g_root.size(), g_root) != 0)
    {
        printf("  ABORT: a path is not in the sandbox (DataPath=%s recipe=%s teach=%s general=%s) -- ReadFile was not called\n",
               DataPath.c_str(), recipe.c_str(), asTeachPath.c_str(), asGeneralPath.c_str());
        return 2;
    }
    GOT("recipe=%s teach=%s general=%s", recipe.c_str(), asTeachPath.c_str(), asGeneralPath.c_str());
    CHECK(true, "0. recipe dir, teach.ini and the general ini are all outside the machine tree");

    // ---- [2] GetRecipeFileName -> Common_PathCombin (golden 913 common.cpp:2044-2048) ------------------------------------
    {
        const std::string want = recipe + "Binasgn.Data";   // GetRecipePath() ends with "\" (common.cpp:2536)
        const std::string lead = GetRecipeFileName(AnsiString("\\Binasgn.Data")).c_str();
        const std::string plain = GetRecipeFileName(AnsiString("Binasgn.Data")).c_str();
        GOT("GetRecipeFileName(\"\\Binasgn.Data\")=%s; (\"Binasgn.Data\")=%s; want %s", lead.c_str(), plain.c_str(), want.c_str());
        CHECK(lead == want && plain == want, "2. GetRecipeFileName: a name with or without a leading \\ gives <recipe>\\Binasgn.Data (Common_PathCombin guard)");
    }

    // ---- [3] TfBinSel::ReadFile in the sandbox ---------------------------------------------------------------------------
    OpenGeneralIniFile();
    GOT("fBinSel=%p", (void*)fBinSel);
    CHECK(fBinSel != 0, "0. the fBinSel object exists (cBinSel.cpp:159)");
    if (fBinSel != 0)
    {
        g_files.clear();
        g_prevStr = W906_ChangeLogHook_Str; W906_ChangeLogHook_Str = CaptureStr;
        g_prevInt = W906_ChangeLogHook_Int; W906_ChangeLogHook_Int = CaptureInt;
        g_prevBool = W906_ChangeLogHook_Bool; W906_ChangeLogHook_Bool = CaptureBool;
        g_prevDouble = W906_ChangeLogHook_Double; W906_ChangeLogHook_Double = CaptureDouble;
        if (fTemp_Set == 0) fTemp_Set = new TfTemp_Set();   // AI(W906-N1G5) 20261009 (laptop, batch 136 integration): golden boot creates TfTemp_Set (CreateForm HT9045.cpp:186) before anything reaches ChangeSite; since !369 ChangeSite calls fTemp_Set->InitialAddrToATC() and this ReadFile goes SetWorkParameter -> ChangeSite -- the same line !369 added to St02_SecsEcFileWrites and the other ChangeSite tests
        fBinSel->ReadFile(false, false, "");
        W906_ChangeLogHook_Str = g_prevStr;
        W906_ChangeLogHook_Int = g_prevInt;
        W906_ChangeLogHook_Bool = g_prevBool;
        W906_ChangeLogHook_Double = g_prevDouble;
        CloseIniFile();

        int binasgn = 0, bad = 0;
        std::string firstBad, last;
        for (size_t i = 0; i < g_files.size(); ++i)
        {
            const std::string& f = g_files[i];
            if (f.find("Binasgn") == std::string::npos) continue;
            ++binasgn; last = f;
            if (!OneBackslash(f)) { ++bad; if (firstBad.empty()) firstBad = f; }
        }
        GOT("%d writes seen, %d to Binasgn*.Data, %d of them not <recipe>\\Binasgn*; first bad %s; last %s", (int)g_files.size(), binasgn, bad,
            firstBad.empty() ? "-" : firstBad.c_str(), last.c_str());
        CHECK(binasgn > 0 && bad == 0, "3. ReadFile -> ReadFunctionData -> SaveFunctionData writes <recipe>\\Binasgn*.Data with one backslash (golden 913)");
    }

    // ---- [4] source: golden 913 text in the four port lines --------------------------------------------------------------
    {
        const std::string bin = Slurp(src + "cBinSel.cpp");
        // ReadFile's SavePath block only: from its signature to its first use of the names (GetRecipeFileName(SavePath[2])).
        // Save's own SavePath lines (cBinSel.cpp :891-910) are W-191 MR-5's, not this MR's.
        const size_t a = bin.find("void TfBinSel::ReadFile(");
        const size_t b = a == std::string::npos ? std::string::npos : bin.find("GetRecipeFileName(SavePath[2])", a);
        const std::string body = (a == std::string::npos || b == std::string::npos) ? std::string() : bin.substr(a, b - a);
        const std::string arrLine = LineWith(body, "AnsiString SavePath[]={");
        const std::string arr = arrLine.substr(0, arrLine.find("//"));   // code only: the note after // quotes the old "\\" names
        int names = 0, lead = 0;
        for (size_t at = body.find("SavePath[0]=\""); at != std::string::npos; at = body.find("SavePath[0]=\"", at + 1))
        {
            ++names;
            if (body.compare(at + 13, 2, "\\\\") == 0) ++lead;
        }
        const bool arrOk = !arr.empty() && arr.find("\"\\\\") == std::string::npos && arr.find("\"BinasgnOff.Data\", \"Binasgn.Data\"") != std::string::npos;
        GOT("ReadFile body %s; array line %s; SavePath[0]= lines %d, with a leading \\\\: %d", body.empty() ? "<not found>" : "found",
            arr.empty() ? "<not found>" : arr.c_str(), names, lead);
        CHECK(arrOk && names == 4 && lead == 0, "4. cBinSel.cpp ReadFile: SavePath names without a leading \\ (golden 913 :1128 / :1135 / :1139 / :1146 / :1150)");

        const char* files[3] = { "ProductionInfo/FileInfo.cpp", "common.cpp", "Public/MyStringList.cpp" };
        for (int i = 0; i < 3; ++i)
        {
            const std::string t = Slurp(src + files[i]);
            const std::string g = LineWith(t, "sFile.AnsiPos(\"\\\\\")!=1)");
            const size_t cut = g.find("//");
            const std::string code = g.substr(0, cut == std::string::npos ? g.size() : cut);
            const bool ok = !g.empty() && code.find("combinedPath[combinedPath.Length()]") != std::string::npos && code.find("'\\\\'") != std::string::npos &&
                            code.find("&& sFile.AnsiPos(\"\\\\\")!=1)") != std::string::npos;
            GOT("%s guard line %s", files[i], g.empty() ? "<not found>" : "found, both conditions before //");
            char msg[200];
            std::snprintf(msg, sizeof(msg), "4. %s: the local-path guard has golden 913's second condition on the same line (FileInfo.cpp:304-305)", files[i]);
            CHECK(ok, msg);
        }
    }

    printf("St02_W195PathCombin: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0) RemoveTree(g_root);
    else printf("  sandbox kept: %s\n", g_root.c_str());
    return g_fail == 0 ? 0 : 1;
}
