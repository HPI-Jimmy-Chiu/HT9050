// ===========================================================================
//  test_memo_log.cpp -- AI(W906-MEMO) 20260927
//
//  The TfMainMemo stand-in (forms/FormWidgets.h) now stores lines, with a defensive 4096-line cap; the Out Shuttle
//  sensor log paths run as in golden (cbShowShuttleSensor ticked by default, golden main.dfm:16056) and
//  OutShuttleLog (golden cpublic.cpp:534-558, now acarry_shims.cpp EOF) writes SH2_/SH1_ files.
//   [1] TfMainMemo: Add / Count / Get / Clear, the cap drops the OLDEST lines, SaveToFile bytes (CRLF, raw), a failed
//       open leaves no file and does not throw.
//   [2] TfMain::AddShuttleMessage: golden's >2048 clear (main.cpp:30232 / :30240).
//   [3] OutShuttleLog: golden gate (Checked && (Count>1024 || bFlag)); writes <asShtLogPath>\YYYYMM\SH2_<t>.logs then
//       SH1_<t>.logs, each ending with the timestamp line it adds, and clears both memos.
//  Refuses to run unless asShtLogPath is inside ctest's machine_log_scratch sandbox (as9045LogPath via the D5 seam).
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "forms/FormWidgets.h"
#include "forms/fMain.h"
#include "common.h"      // asShtLogPath, as9045LogPath
#include "cpublic.h"     // OutShuttleLog, GetTimeInfo
#include "cmydef.h"      // SystemYear..

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static int g_pass = 0, g_fail = 0;
#define CHECK(c) do { if (c) { ++g_pass; std::printf("  PASS: %s\n", #c); } \
                      else { ++g_fail; std::printf("  FAIL: %s  (line %d)\n", #c, __LINE__); } } while (0)

static std::string readAll(const std::string& p)
{
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return std::string("<none>");
    std::string s;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof buf, f)) > 0) s.append(buf, n);
    std::fclose(f);
    return s;
}

static std::vector<std::string> listDir(const std::string& dir, const char* pat)
{
    std::vector<std::string> out;
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA((dir + "\\" + pat).c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return out;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) out.push_back(dir + "\\" + fd.cFileName);
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return out;
}

int main()
{
    std::printf("asShtLogPath = %s\n", asShtLogPath.c_str());
    if (std::strstr(asShtLogPath.c_str(), "machine_log_scratch") == NULL)
    {
        std::printf("REFUSED: asShtLogPath is not the ctest machine_log_scratch sandbox\n");
        return 1;
    }

    std::printf("[1] TfMainMemo storage\n");
    {
        TfMainMemo m;
        CHECK(m.Lines->Count == 0);
        m.Lines->Add("a");
        m.Lines->Add("b");
        CHECK(m.Lines->Count == 2);
        CHECK(std::strcmp(m.Lines->Get(0).c_str(), "a") == 0 && std::strcmp(m.Lines->Get(1).c_str(), "b") == 0);
        CHECK(std::strcmp(m.Lines->Get(5).c_str(), "") == 0);
        char tmp[MAX_PATH];
        GetTempPathA(MAX_PATH, tmp);
        std::string f = std::string(tmp) + "ht9045_memo_save_test.txt";
        m.Lines->SaveToFile(f.c_str());
        CHECK(readAll(f) == std::string("a\r\nb\r\n"));          // every line + CRLF, like vclcompat TStringList
        DeleteFileA(f.c_str());
        std::string bad = std::string(tmp) + "ht9045_no_such_dir_xyz\\x.txt";
        m.Lines->SaveToFile(bad.c_str());                         // no throw
        CHECK(GetFileAttributesA(bad.c_str()) == INVALID_FILE_ATTRIBUTES);
        m.Clear();
        CHECK(m.Lines->Count == 0);
        for (int i = 0; i < TfMainMemoLines::kLineCap + 10; ++i)
        {
            AnsiString s;
            s.sprintf("L%d", i);
            m.Lines->Add(s);
        }
        CHECK(m.Lines->Count == TfMainMemoLines::kLineCap);      // capped
        CHECK(std::strcmp(m.Lines->Get(0).c_str(), "L10") == 0); // the 10 oldest dropped
        AnsiString last;
        last.sprintf("L%d", TfMainMemoLines::kLineCap + 9);
        CHECK(std::strcmp(m.Lines->Get(TfMainMemoLines::kLineCap - 1).c_str(), last.c_str()) == 0);
        CHECK(TfMainMemoLines::kLineCap > 2049);                  // above every golden threshold (2049-line csv)
    }

    std::printf("[2] AddShuttleMessage (golden main.cpp:30228-30246)\n");
    {
        CHECK(fMain != NULL && fMain->meShuttle1 != NULL && fMain->meShuttle2 != NULL);
        fMain->meShuttle1->Clear();
        for (int i = 0; i < 2049; ++i) fMain->AddShuttleMessage(0, "s1");
        CHECK(fMain->meShuttle1->Lines->Count == 2049);           // >2048 not yet reached before the 2049th add
        fMain->AddShuttleMessage(0, "next");
        CHECK(fMain->meShuttle1->Lines->Count == 1);              // Count 2049 > 2048 -> Clear, then Add
        fMain->meShuttle2->Clear();
        fMain->AddShuttleMessage(1, "s2");
        CHECK(fMain->meShuttle2->Lines->Count == 1);
        fMain->meShuttle1->Clear();
        fMain->meShuttle2->Clear();
    }

    std::printf("[3] OutShuttleLog (golden cpublic.cpp:534-558)\n");
    {
        CHECK(fMain->cbShowShuttleSensor->Checked == true);       // golden main.dfm:16056 Checked = True
        GetTimeInfo();
        char ym[16];
        std::snprintf(ym, sizeof ym, "%04d%02d", (int)SystemYear, (int)SystemMonth);
        std::string dir = std::string(asShtLogPath.c_str()) + "\\" + ym;
        std::vector<std::string> before = listDir(dir, "*.logs");

        fMain->meShuttle1->Lines->Add("x1");
        fMain->meShuttle2->Lines->Add("x2");
        OutShuttleLog(false);                                     // Count <= 1024 and !bFlag -> nothing
        CHECK(listDir(dir, "*.logs").size() == before.size());
        CHECK(fMain->meShuttle1->Lines->Count == 1 && fMain->meShuttle2->Lines->Count == 1);

        fMain->cbShowShuttleSensor->Checked = false;
        OutShuttleLog(true);                                      // unticked -> nothing even with bFlag
        CHECK(listDir(dir, "*.logs").size() == before.size());
        fMain->cbShowShuttleSensor->Checked = true;

        OutShuttleLog(true);                                      // bFlag -> save both, clear both
        std::vector<std::string> after = listDir(dir, "*.logs");
        std::vector<std::string> made;
        for (size_t i = 0; i < after.size(); ++i)
        {
            bool old = false;
            for (size_t k = 0; k < before.size(); ++k) old = old || before[k] == after[i];
            if (!old) made.push_back(after[i]);
        }
        CHECK(made.size() == 2);
        int sh1 = 0, sh2 = 0;
        for (size_t i = 0; i < made.size(); ++i)
        {
            std::string body = readAll(made[i]);
            if (made[i].find("\\SH1_") != std::string::npos) { ++sh1; CHECK(body.compare(0, 4, "x1\r\n") == 0); }
            if (made[i].find("\\SH2_") != std::string::npos) { ++sh2; CHECK(body.compare(0, 4, "x2\r\n") == 0); }
            CHECK(body.size() > 4 && body.substr(body.size() - 2) == "\r\n");   // ends with the timestamp line + CRLF
            DeleteFileA(made[i].c_str());
        }
        CHECK(sh1 == 1 && sh2 == 1);
        CHECK(fMain->meShuttle1->Lines->Count == 0 && fMain->meShuttle2->Lines->Count == 0);

        for (int i = 0; i < 1025; ++i) fMain->meShuttle1->Lines->Add("y");
        before = listDir(dir, "*.logs");
        OutShuttleLog(false);                                     // Count 1025 > 1024 -> save both
        after = listDir(dir, "*.logs");
        CHECK(after.size() == before.size() + 2);
        for (size_t i = 0; i < after.size(); ++i)
        {
            bool old = false;
            for (size_t k = 0; k < before.size(); ++k) old = old || before[k] == after[i];
            if (!old) DeleteFileA(after[i].c_str());
        }
        RemoveDirectoryA(dir.c_str());                            // only if empty
    }

    std::printf("%s: %d / %d checks passed\n", g_fail ? "FAIL" : "PASS", g_pass, g_pass + g_fail);
    return g_fail ? 1 : 0;
}
