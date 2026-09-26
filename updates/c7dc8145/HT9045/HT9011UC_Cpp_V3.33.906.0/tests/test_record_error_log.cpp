// AI(W906-RECERR) 20260927: golden RecordErrorLog (cpublic.cpp EOF) writes <as9045LogPath>\<FilePth>\YYYY\MM\YYYYMMDDHH.txt.
//   Refuses to run unless as9045LogPath is the ctest sandbox (W906_HT9045LOG_ROOT, St02 D5 via _ht9045_env_extra) --
//   a test that could write the real D:\HT9045_Log is the failure mode this tree has paid for before.
#include "cpublic.h"
#include "cmydef.h"
#include "common.h"
#include <cstdio>
#include <cstring>
#include <string>

static int g_fail = 0, g_total = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL: %s  (line %d)\n", what, line); }
    else     { std::printf("  PASS: %s\n", what); }
}
#define CHECK(c) check((c), #c, __LINE__)

int main()
{
    std::printf("as9045LogPath = %s\n", as9045LogPath.c_str());
    const bool sandboxed = std::strstr(as9045LogPath.c_str(), "machine_log_scratch") != 0;
    CHECK(sandboxed);
    if (!sandboxed) {
        std::printf("FAIL: not sandboxed -- refusing to call RecordErrorLog (it would write the real log tree)\n");
        return 1;
    }
    const char* tag = "hello-W906-RECERR";
    RecordErrorLog(0, "W906_RecordErrorLogTest", tag);
    AnsiString path;
    path.sprintf("%s\\W906_RecordErrorLogTest\\%04d\\%02d\\%04d%02d%02d%02d.txt", as9045LogPath.c_str(),
                 SystemYear, SystemMonth, SystemYear, SystemMonth, SystemDate, SystemHour);
    std::printf("expect %s\n", path.c_str());
    FILE* f = std::fopen(path.c_str(), "rb");
    CHECK(f != 0);
    std::string s;
    if (f) { char buf[4096]; size_t n; while ((n = std::fread(buf, 1, sizeof buf, f)) > 0) s.append(buf, n); std::fclose(f); }
    CHECK(s.find(tag) != std::string::npos);
    CHECK(s.find(", :, ") != std::string::npos);   // golden line format "<time>, :, <msg>"
    std::printf("%s: %d / %d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
