// =============================================================================
//  test_st02_w195_hanatraymap.cpp -- W-195 (3) HANA H5: the HANA TrayMap half of TMyStringList::MySaveToFileShareMode
//                                    follows golden 913 Public/MyStringList.cpp (RogerYang 20260904, AI(ht9045-art-flow)).
//
//  AI(W906-W195) 20261009 (St02-E).  Suite name (add_test): St02_W195HanaTrayMap.
//    golden 913 :255-258: the lot start time loses '-', ':' and ' ' (906.0's SetLotStartTime() writes "yyyy-mm-dd hh:nn:ss" and
//      a file name cannot hold ':'); :264: the TrayMap file name uses it; :295-298: a failed CreateFile is logged to MNetLog.
//    [W906] the TrayMap root is as9045LogPath (golden literal "D:\HT9045_Log"; Public/MyStringList.cpp same line), so this test
//    writes only under its own %TEMP%\ht9045_w195h5_<tick> (as9045LogPath set here; the test aborts if it is under D:\).
//    [1] bHanaTrayMap + the SCK ART captions + LotStartTime "2026-10-09 11:22:33" -> <root>\Hana_TrayMap\<year>\LOTH5\
//        M1_FT1_LOTH5_TRAY_20261009112233_1.txt is written with the header row + the data row;
//    [2] a directory already standing on the target name -> CreateFile fails -> MNetLog gets
//        "TrayMap create file fail : <path>, err=5" (golden 913 :297), nothing else is written;
//    [3] source: the MNetLog include and the stripped-time line are code (before the //).
//  Reverse check (recorded in the MR): put RunInfo.LotStartTime back as the file-name argument -> [1] red (no file: the ':' in
//  the name makes CreateFile fail); drop the else branch -> [2] red.
// =============================================================================
#include "Public/MyStringList.h"
#include "forms/fSCKART.h"
#include "LogObjects.h"
#include "cprod.h"
#include "cmydef.h"
#include "Config.h"
#include "common.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>

static int g_pass = 0, g_fail = 0;
static char g_got[900];
#define GOT(...) std::snprintf(g_got, sizeof(g_got), __VA_ARGS__)
#define CHECK(cond, msg)                                                                                   \
    do {                                                                                                   \
        if (cond) { printf("  PASS: %s   [%s]\n", msg, g_got); ++g_pass; }                                 \
        else      { printf("  FAIL: %s   [got %s]  (line %d)\n", msg, g_got, __LINE__); ++g_fail; }        \
        g_got[0] = 0;                                                                                      \
    } while (0)

static std::string g_root;
static std::string g_want2;      // [2]'s expected MNetLog line
static bool g_inList2 = false;   // seen in the MNetLog object's memory list before the flush

static std::string Slurp(const std::string& p)
{
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return std::string("<missing>");
    std::string s;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) s.append(buf, n);
    std::fclose(f);
    return s;
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

// every file under dir whose bytes contain `needle` (MNetLog may already have flushed its line to <root>\MNetLog\...)
static bool AnyFileHas(const std::string& dir, const std::string& needle)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    bool hit = false;
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (std::strcmp(fd.cFileName, ".") == 0 || std::strcmp(fd.cFileName, "..") == 0) continue;
            const std::string p = dir + "\\" + fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) hit = AnyFileHas(p, needle) || hit;
            else if (Slurp(p).find(needle) != std::string::npos) hit = true;
        } while (!hit && ::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    return hit;
}

static bool UnderD(const std::string& p)
{
    return p.size() >= 2 && (p[0] == 'D' || p[0] == 'd') && p[1] == ':';
}

static std::string LineWith(const std::string& text, const std::string& anchor)
{
    const size_t at = text.find(anchor);
    if (at == std::string::npos) return std::string();
    const size_t a = text.rfind('\n', at), b = text.find('\n', at);
    return text.substr(a == std::string::npos ? 0 : a + 1, (b == std::string::npos ? text.size() : b) - (a == std::string::npos ? 0 : a + 1));
}

int main(int argc, char** argv)
{
    printf("St02_W195HanaTrayMap\n");
    if (argc < 2) { printf("  ABORT: argv[1] (the tree root) missing\n"); return 2; }
    const std::string src = std::string(argv[1]) + "/";

    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    g_root = std::string(tmp) + "ht9045_w195h5_" + stamp;
    ::CreateDirectoryA(g_root.c_str(), 0);

    const AnsiString savLog = as9045LogPath;
    const AnsiString savMachine = IniConfig.sGPIBMachineID;
    const AnsiString savLotStart = RunInfo.LotStartTime;
    const Word savYear = SystemYear;
    as9045LogPath = AnsiString(g_root.c_str());
    if (UnderD(std::string(as9045LogPath.c_str())) || fSCKART == 0 || fSCKART->palLotNumber == 0 || fSCKART->pnlProcessCode == 0 ||
        fSCKART->palTestCnt == 0)
    {
        printf("  ABORT: log root %s on D:, or the SCK ART panels are missing -- nothing was called\n", as9045LogPath.c_str());
        return 2;
    }
    GOT("as9045LogPath=%s", as9045LogPath.c_str());
    CHECK(true, "0. the TrayMap root (as9045LogPath) is the test's own %TEMP% folder");
    W906_CreateLogObjects();                                   // MNetLog writes through W906_MNetLogObj() (MNetLog.cpp); null otherwise
    GOT("MNetLog object %p", (void*)W906_MNetLogObj());
    CHECK(W906_MNetLogObj() != 0, "0. the MNetLog object exists (LogObjects.cpp)");

    IniConfig.sGPIBMachineID = "M1";
    RunInfo.LotStartTime = "2026-10-09 11:22:33";               // 906.0's SetLotStartTime format (golden 913 :255 note)
    // the year in the path is the TMyStringList object's own SystemYear, which MySaveToFileShareMode refreshes from the clock
    // (TMyStringList::GetTimeInfo, :492 / :474-480) -- not the global SystemYear; the test takes it from GetLocalTime
    fSCKART->pnlProcessCode->Caption = "FT1";
    fSCKART->palTestCnt->Caption = "1";

    // ---- [1] the file is written with the stripped time in its name ------------------------------------------------------
    {
        fSCKART->palLotNumber->Caption = "LOTH5";
        TMyStringList sl(AnsiString(g_root.c_str()), "TRAY", "HDR");
        sl.MaxLineCount = 1000;
        sl.AutoSave = true;
        sl.bHanaTrayMap = true;
        sl.MyList->Add("ROW1");
        sl.MySaveToFileShareMode();
        SYSTEMTIME st1; ::GetLocalTime(&st1);
        const std::string want = g_root + "\\Hana_TrayMap\\" + std::to_string((int)st1.wYear) + "\\LOTH5\\M1_FT1_LOTH5_TRAY_20261009112233_1.txt";
        const std::string body = Slurp(want);
        GOT("sLastFileName=%s; file %s", sl.sLastFileName.c_str(), body == "<missing>" ? "<missing>" : "present");
        CHECK(std::string(sl.sLastFileName.c_str()) == want && body == "HDR\r\nROW1\r\n",
              "1. TrayMap file <root>\\Hana_TrayMap\\<year>\\LOTH5\\M1_FT1_LOTH5_TRAY_20261009112233_1.txt = header + row (golden 913 :255-264)");
        sl.MyList->Clear();                                     // the dtor's MySaveToFile must not write again
    }

    // ---- [2] CreateFile fails -> MNetLog -----------------------------------------------------------------------------------
    {
        fSCKART->palLotNumber->Caption = "LOTF";
        SYSTEMTIME st; ::GetLocalTime(&st);                       // the year MySaveToFileShareMode will use
        const std::string year = std::to_string((int)st.wYear);
        const std::string dir = g_root + "\\Hana_TrayMap\\" + year + "\\LOTF";
        const std::string target = dir + "\\M1_FT1_LOTF_TRAY_20261009112233_1.txt";
        ::CreateDirectoryA((g_root + "\\Hana_TrayMap").c_str(), 0);
        ::CreateDirectoryA((g_root + "\\Hana_TrayMap\\" + year).c_str(), 0);
        ::CreateDirectoryA(dir.c_str(), 0);
        ::CreateDirectoryA(target.c_str(), 0);                  // a directory where the file should go: CreateFile(GENERIC_WRITE) fails
        TMyStringList* const mnet = W906_MNetLogObj();
        const int before = mnet ? mnet->MyList->Count : -1;
        TMyStringList sl(AnsiString(g_root.c_str()), "TRAY", "HDR");
        sl.MaxLineCount = 1000;
        sl.AutoSave = true;
        sl.bHanaTrayMap = true;
        sl.MyList->Add("ROW1");
        sl.MySaveToFileShareMode();
        std::string added;
        if (mnet) for (int i = before; i < mnet->MyList->Count; ++i) added += std::string(AnsiString(mnet->MyList->Strings[i]).c_str()) + "\n";
        g_want2 = "TrayMap create file fail : " + target + ", err=5";
        g_inList2 = added.find(g_want2) != std::string::npos;
        GOT("MNetLog lines added: %s", added.empty() ? "<none in memory>" : added.c_str());
        CHECK(true, "2. (the MNetLog line is checked after the log objects are flushed, below)");
        sl.MyList->Clear();
    }

    // ---- [3] source ------------------------------------------------------------------------------------------------------
    {
        const std::string t = Slurp(src + "Public/MyStringList.cpp");
        const std::string inc = LineWith(t, "#include \"MNetLog.h\"");
        const std::string lot = LineWith(t, "AnsiString sLotStartTime=StringReplace(StringReplace(StringReplace(RunInfo.LotStartTime");
        GOT("include %s; time line %s", inc.empty() ? "<not found>" : "found", lot.empty() ? "<not found>" : "found");
        CHECK(inc.compare(0, 9, "#include ") == 0 && !lot.empty() && lot.find("//") > lot.find("rfReplaceAll);"),
              "3. Public/MyStringList.cpp: the MNetLog include and the stripped-time line are code, before any //");
    }

    W906_DestroyLogObjects();                                  // flushes MNetLog into <root>\MNetLog\...
    {
        const bool onDisk = AnyFileHas(g_root + "\\MNetLog", g_want2);
        GOT("in memory before the flush: %s; on disk under <root>\\MNetLog: %s", g_inList2 ? "yes" : "no", onDisk ? "yes" : "no");
        CHECK(g_inList2 || onDisk, "2. CreateFile fails -> MNetLog \"TrayMap create file fail : <path>, err=5\" (golden 913 :295-298)");
    }
    as9045LogPath = savLog;  IniConfig.sGPIBMachineID = savMachine;  RunInfo.LotStartTime = savLotStart;  SystemYear = savYear;
    printf("St02_W195HanaTrayMap: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0) RemoveTree(g_root);
    else printf("  sandbox kept: %s\n", g_root.c_str());
    return g_fail == 0 ? 0 : 1;
}
