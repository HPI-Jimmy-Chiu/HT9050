// =============================================================================
//  test_st02_w143_mnetlog.cpp -- W-143 MNETLOG: golden MNetLog writes the MNetLog file again, as its own module.
//
//  AI(W906-W143) 20261007 (St02-E).  Suite name (add_test): St02_W143MNetLog.  argv[1] = the port tree root (read only).
//  Laptop card W-143 = the !296 proposal option A (skill hpi-mnetlog-split); Steven 1007 12:0x: mmoMNet "not displayed" + TODO.
//    [1] before the log objects exist (fMain->slMNetLog null, as in every ctest): MNetLog returns true and writes nothing;
//    [2] with an slMNetLog in the sandbox: MNetLog(text) -> the MNetLog file has "<date>, <time>, text" (golden
//        Motor/myMN200motor.cpp:2150 AddTextWithDateTime); "" adds nothing (golden :2148); returns true (:2155);
//    [3] source pins: one live MNetLog definition (MNetLog.cpp, not myMN200motor.cpp); the two file-local stubs the card
//        retired (AutoClean.cpp / TfFTP.cpp) are gone; the five hand-written declarations include MNetLog.h; MNetLog.cpp
//        does not include forms/fMain.h and carries the TODO(W906-LOGVIEW) for mmoMNet; cStateRecord.cpp G5 is lifted and
//        queues the active MNetLog file like the event log; W906_CreateLogObjects still builds it under as9045LogPath.
//  Files: only under %TEMP%\ht9045_w143_<tick> (the object is pointed there; the test refuses a path under D:\HT9045 that is
//  not a build dir -- the \obj\v906\ rule of d74e577e / 2b663dbd).  Every CHECK prints what it read.
// =============================================================================
#include "MNetLog.h"
#include "LogObjects.h"
#include "Public/MyStringList.h"
#include "forms/fMain.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>

static int g_pass = 0, g_fail = 0;
static std::string g_got;
#define CHECK(cond, msg)                                                                                         \
    do {                                                                                                         \
        if (cond) { printf("  PASS: %s   [%s]\n", msg, g_got.c_str()); ++g_pass; }                               \
        else      { printf("  FAIL: %s   [got %s]  (line %d)\n", msg, g_got.c_str(), __LINE__); ++g_fail; }      \
        g_got.clear();                                                                                           \
    } while (0)

static std::string Lower(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i) { if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a'); if (s[i] == '/') s[i] = '\\'; }
    return s;
}
static bool UnderMachineTree(const std::string& p)
{
    const std::string s = Lower(p);
    if (s.find("\\obj\\v906\\") != std::string::npos) return false;
    return s.compare(0, 9, "d:\\ht9045") == 0;
}
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
static bool Has(const std::string& s, const char* t) { return s.find(t) != std::string::npos; }
static int  Count(const std::string& s, const char* t) { int n = 0; for (size_t p = s.find(t); p != std::string::npos; p = s.find(t, p + 1)) ++n; return n; }

int main(int argc, char** argv)
{
    printf("St02_W143MNetLog\n");
    const std::string tree = argc > 1 ? argv[1] : "";
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    const std::string root = std::string(tmp) + "ht9045_w143_" + stamp;
    ::CreateDirectoryA(root.c_str(), 0);
    if (UnderMachineTree(root)) { printf("  ABORT: sandbox %s is under D:\\HT9045 -- nothing was called\n", root.c_str()); return 2; }

    // [1] no log object yet
    g_got = std::string("fMain=") + (fMain ? "set" : "null") + " slMNetLog=" + (fMain && fMain->slMNetLog ? "set" : "null") +
            " W906_MNetLogObj=" + (W906_MNetLogObj() ? "set" : "null");
    CHECK(fMain != 0 && fMain->slMNetLog == 0 && W906_MNetLogObj() == 0, "1. ctest: fMain exists, slMNetLog not created yet (wb_serve's W906_CreateLogObjects does that)");
    bool r = MNetLog("no object yet");
    g_got = r ? "true" : "false";
    CHECK(r, "1. MNetLog without the object -> true, no crash (golden returns true on every path)");

    // [2] an slMNetLog in the sandbox, as W906_CreateLogObjects builds it (LogObjects.cpp: path, "MNetLog", "")
    TMyStringList* sl = new TMyStringList(AnsiString((root + "\\MNetLog").c_str()), "MNetLog", "");
    fMain->slMNetLog = sl;
    const std::string file = std::string(sl->GetFileName().c_str());
    if (UnderMachineTree(file) || Lower(file).find(Lower(root)) != 0)
    {
        printf("  ABORT: the MNetLog file %s is not in the sandbox -- nothing more is called\n", file.c_str());
        fMain->slMNetLog = 0;
        return 2;
    }
    g_got = std::string(W906_MNetLogObj() == sl ? "same" : "other");
    CHECK(W906_MNetLogObj() == sl, "2. W906_MNetLogObj() returns fMain->slMNetLog (LogObjects.cpp EOF)");
    const std::string marker = std::string("W143 hello ") + stamp;
    r = MNetLog(AnsiString(marker.c_str()));
    MNetLog("");
    sl->MySaveToFile();
    const std::string body = Slurp(file);
    const size_t at = body.find(marker);
    g_got = "file=" + file + " bytes=" + std::to_string(body.size()) + " line=" + (at == std::string::npos ? std::string("-") : body.substr(at >= 26 ? at - 26 : 0, 26 + marker.size()));
    CHECK(r && at != std::string::npos, "2. MNetLog(text) -> the MNetLog file has the text (golden :2150 AddTextWithDateTime)");
    g_got = body.substr(0, body.find('\n') == std::string::npos ? body.size() : body.find('\n'));
    CHECK(at >= 26 && body[at - 2] == ',' && body.compare(at - 26, 2, "20") == 0, "2. ... with golden's \"YYYY-MM-DD, hh:mm:ss.mmm, \" prefix");
    g_got = "lines=" + std::to_string(Count(body, "\n"));
    CHECK(Count(body, "\n") == 1, "2. MNetLog(\"\") adds nothing (golden :2148) -- one line in the file");
    fMain->slMNetLog = 0;
    delete sl;

    // [3] source pins
    if (tree.empty()) { g_got = "no argv[1]"; CHECK(false, "3. argv[1] = the port tree root"); }
    else
    {
        const std::string mn = Slurp(tree + "/MNetLog.cpp"), mh = Slurp(tree + "/MNetLog.h"), mm = Slurp(tree + "/Motor/myMN200motor.cpp");
        g_got = "MNetLog.cpp defs=" + std::to_string(Count(mn, "\nbool MNetLog(AnsiString Message)")) + " myMN200motor live defs=" +
                std::to_string(Count(mm, "\nbool MNetLog(AnsiString Message)"));
        CHECK(Count(mn, "\nbool MNetLog(AnsiString Message)") == 1 && Count(mm, "\nbool MNetLog(AnsiString Message)") == 0 && Has(mm, "//W143 bool MNetLog(AnsiString Message)"),
              "3. one live MNetLog: MNetLog.cpp; myMN200motor.cpp keeps the old text commented out");
        g_got = std::string("fMain.h include=") + (Has(mn, "forms/fMain.h\"") ? "yes" : "no") + " TODO=" + (Has(mn, "TODO(W906-LOGVIEW)") ? "yes" : "no");
        CHECK(!Has(mn, "#include \"forms/fMain.h\"") && Has(mn, "W906_MNetLogObj()") &&
              Has(mn, "// TODO(W906-LOGVIEW): golden \xE9\xA1\xAF\xE7\xA4\xBA\xE5\x9C\xA8 fMain->mmoMNet") && Has(mh, "bool MNetLog(AnsiString Message);"),
              "3. MNetLog.cpp reaches slMNetLog without forms/fMain.h; mmoMNet = not displayed + TODO(W906-LOGVIEW) (Steven 12:0x)");
        const char* stubs[2] = { "AutoClean/AutoClean.cpp", "ProductionInfo/TfFTP.cpp" };   // cMyDB.cpp:186 keeps its stub (test_ga1_cmydb)
        std::string left;
        for (int i = 0; i < 2; ++i)
        {
            const std::string s = Slurp(tree + "/" + stubs[i]);
            if (Has(s, "\nstatic bool MNetLog(") || Has(s, "\nstatic void MNetLog(") || !Has(s, "#include \"MNetLog.h\"")) left += std::string(stubs[i]) + " ";
        }
        g_got = left.empty() ? "all retired" : left;
        CHECK(left.empty(), "3. the file-local stubs (AutoClean.cpp :189, TfFTP.cpp :179) are retired -> the real MNetLog");
        const char* decls[5] = { "Motor/myMN200motor.h", "EtherCAT/MyEtherCAT.cpp", "MyLaneIo.cpp", "Adam6024Pressure_St02.cpp", "MainTimer3.cpp" };
        left.clear();
        for (int i = 0; i < 5; ++i)
        {
            const std::string s = Slurp(tree + "/" + decls[i]);
            if (Has(s, "\nextern bool MNetLog(AnsiString Message);") || Has(s, "\nbool MNetLog(AnsiString Message);") || !Has(s, "#include \"MNetLog.h\"")) left += std::string(decls[i]) + " ";
        }
        g_got = left.empty() ? "all include MNetLog.h" : left;
        CHECK(left.empty(), "3. the hand-written declarations include MNetLog.h");
        const std::string sr = Slurp(tree + "/cStateRecord.cpp");
        g_got = std::string("gate=") + (Has(sr, "\n#if 0 // GATE(W906-STATEREC-G5)") ? "closed" : "open");
        CHECK(!Has(sr, "\n#if 0 // GATE(W906-STATEREC-G5)") && Has(sr, "str1=(slMNetLog!=NULL) ? slMNetLog->GetFileName() : AnsiString(\"\");") &&
              Has(sr, "job.copies.push_back(std::make_pair(std::string(str1.c_str()), std::string(sDestMNet.c_str())));"),
              "3. cStateRecord.cpp G5 lifted: the active MNetLog file is queued with the hang-up copies (golden main.cpp:26414-26420)");
        const std::string lo = Slurp(tree + "/LogObjects.cpp");
        g_got = std::string("create=") + (Has(lo, "self->slMNetLog =new TMyStringList(as9045LogPath+\"\\\\MNetLog\",") ? "yes" : "no");
        CHECK(Has(lo, "self->slMNetLog =new TMyStringList(as9045LogPath+\"\\\\MNetLog\","), "3. W906_CreateLogObjects builds slMNetLog under as9045LogPath (golden D:\\HT9045_Log\\MNetLog)");
    }

    printf("St02_W143MNetLog: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0) RemoveTree(root);
    else printf("  sandbox kept: %s\n", root.c_str());
    return g_fail ? 1 : 0;
}
