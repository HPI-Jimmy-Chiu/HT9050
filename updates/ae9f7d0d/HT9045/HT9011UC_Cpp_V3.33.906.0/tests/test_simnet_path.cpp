// =============================================================================
//  test_simnet_path.cpp -- W58 Q5 (phase 2, group A): W906_SimNetPathBlocked (common.cpp end of file).
//
//  AI(W906-W58) 20261001 (St02-E).  Suite name (add_test): SimNet_Path.  Built and run in both configurations:
//    SIM (`build`): a UNC / `//` path is blocked, a local drive path is not, W906_SIM_NET_PATHS=1 lets everything through
//                   (only exactly "1"), leading blanks are skipped, an empty path is not blocked;
//    SHIP (`build_ship`, W906_NO_SOFT_SIMULTE): never blocked, whatever the path or the variable.
//  A mapped drive letter (DRIVE_REMOTE) cannot be faked here: GetDriveTypeA asks the real machine.  The test only checks
//  that C: (the system drive, never remote) is not blocked.  It writes nothing and calls no Handler code.
// =============================================================================
#include "vclcompat/vcl_compat.h"
#include "common.h"

#include <cstdio>
#include <cstdlib>
#include <string>

// _putenv: MinGW.org 6.3 declares neither _putenv nor putenv in strict mode (CXX_EXTENSIONS OFF) -- the same guard as
// tests/test_agv_e84.cpp:143-162.
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                                     \
    do {                                                                                     \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }                            \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; }      \
        std::fflush(stdout);                                                                 \
    } while (0)

namespace {
bool Blocked(const char* path, const char* env)
{
    if (env) { std::string e = std::string("W906_SIM_NET_PATHS=") + env; HT9045_TEST_PUTENV(e.c_str()); }
    else HT9045_TEST_PUTENV("W906_SIM_NET_PATHS=");
    const bool r = W906_SimNetPathBlocked(AnsiString(path));
    HT9045_TEST_PUTENV("W906_SIM_NET_PATHS=");
    return r;
}
}  // namespace

int main()
{
#ifdef W906_NO_SOFT_SIMULTE
    const bool sim = false;
#else
    const bool sim = true;
#endif
    std::printf("SimNet_Path (W58 Q5, %s build)\n", sim ? "SIM" : "SHIP");

    const bool unc = Blocked("\\\\server\\share\\TestSummary", 0);
    const bool fwd = Blocked("//server/share", 0);
    const bool blanks = Blocked("  \\\\server\\share", 0);
    const bool local = Blocked("D:\\HT9045_Log\\TestSummary\\", 0);
    const bool sysdrv = Blocked("C:\\Windows", 0);
    const bool relative = Blocked("TestSummary", 0);
    const bool empty = Blocked("", 0);
    const bool allow1 = Blocked("\\\\server\\share", "1");
    const bool allow0 = Blocked("\\\\server\\share", "0");
    const bool allow11 = Blocked("\\\\server\\share", "11");

    if (sim)
    {
        CHECK(unc && fwd, "SIM: a UNC path and a // path are blocked");
        CHECK(blanks, "SIM: leading blanks are skipped (still a UNC path)");
        CHECK(!local && !sysdrv && !relative && !empty, "SIM: a local drive path, C:, a relative path and an empty path are not blocked");
        CHECK(!allow1, "SIM: W906_SIM_NET_PATHS=1 writes anyway");
        CHECK(allow0 && allow11, "SIM: only exactly \"1\" lets it through (\"0\" / \"11\" still block)");
    }
    else
    {
        CHECK(!unc && !fwd && !blanks && !local && !sysdrv && !relative && !empty && !allow1 && !allow0 && !allow11,
              "SHIP: never blocked (golden unchanged)");
    }
    std::printf("SimNet_Path: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
