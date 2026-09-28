// =============================================================================
//  w906_ctest_guard.h -- a test executable refuses to run outside ctest's redirect environment.
//
//  AI(W906-TESTGUARD) 20260928 (laptop; St02 FROM_STEVEN 0928 11:03 item 6 (a)).
//
//  WHY.  tests/CMakeLists.txt points the machine's data roots at <build dir>/tests/machine_log_scratch and
//  machine_config_scratch through ctest's ENVIRONMENT property -- and only through it.  Run the same .exe by hand (a
//  debugger, .vscode/launch.json's "debug a ctest binary" entry, an agent's shell) and every getenv seam falls back to
//  golden's literal: D:\HT9045_Log\..., D:\HT9045\config\, D:\HT9045\IniData\, D:\HT9045\system\Gerneral.ini (the
//  per-test general_ini_scratch copy exists only under ctest, see tests/test_bootstrap.cpp).  The test still passes
//  while it writes, which is why nobody notices (the MACHINE-DATA CONTAINMENT block in tests/CMakeLists.txt says it in
//  its own words: "This does NOT protect a test executable run BY HAND").
//  St02 found two laptop tests on that path:
//    SjsonChan  HandleAction -> act.main.stateRecord / act.main.testerConnect / JsonBridge/ChanAction.cpp:187
//               LogAppend -> RecordProcess
//    ChangeLog  InitialOK=true + the change-log hooks -> RecordChangeLogProcess -> MyDBIProcess("ChangeLog")
//  Both land in the cMyDB log roots once the golden cMyDB bodies are linked (cMyDB P4, St02, MR !3); before that they
//  reach stdout / counting stand-ins, so this guard is for then as much as for now.
//
//  RULE (the shape of St02's tests/st02_test_containment.h, test_record_error_log.cpp, test_memo_log.cpp): every
//  variable in the list below must be set and point into ctest's scratch (W906CtestGuardInScratch), and so must every
//  runtime path the caller passes; otherwise print which ones do not and the caller returns 2 BEFORE any Handler code
//  runs.  It does NOT test "not under D:\HT9045": the scratch itself sits under D:\HT9045\...\build_*\tests.
//
//  OPT-OUT, for a deliberate real-file run (you accept writes into the real D:\HT9045 / D:\HT9045_Log files):
//      set W906_TEST_ALLOW_REAL_FILES=1
//  Exactly "1".  The run then prints a WARNING and goes ahead.  tests/CMakeLists.txt never sets it.
//
//  OPT-IN PER TEST, deliberately NOT a global refusal in tests/test_bootstrap.cpp (which is compiled into every test):
//  that would also stop, when run by hand, the live-config readers config_db / config_loaders / GA1_ReadGeneralIni /
//  ini_helpers / IniFiles (they are meant to be run against the real machine config, read-only -- CopyFile to %TEMP% or
//  a vclcompat-only read), every test debugged through .vscode/launch.json's "Debug one ctest binary (MinGW/gdb)" entry
//  (its "environment" is empty), and the many executables that never open a machine file.
//
//  Use -- the first statement of main():
//      if (!W906TestRequireCtestRedirects("SjsonChan"))
//          return 2;
//  or, to also prove the seams took (the same AnsiString globals the writers use):
//      const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asGeneralPath", asGeneralPath.c_str(), 0 };
//      if (!W906TestRequireCtestRedirects("ChangeLog", rt))
//          return 2;
//  Header-only; no link dependency (the environment is read with getenv; runtime paths are passed in by the caller).
// =============================================================================
#ifndef W906_CTEST_GUARD_H
#define W906_CTEST_GUARD_H

#include <cstdio>
#include <cstdlib>
#include <string>

// true = the value points into ctest's scratch: <build dir>/tests/machine_log_scratch, machine_config_scratch (the
// containment block's two roots) or general_ini_scratch (the per-test Gerneral.ini sandbox).
inline bool W906CtestGuardInScratch(const char* value)
{
    std::string s(value ? value : "");
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] >= 'A' && s[i] <= 'Z')
            s[i] = (char)(s[i] - 'A' + 'a');
        if (s[i] == '/')
            s[i] = '\\';
    }
    return s.find("\\machine_log_scratch") != std::string::npos || s.find("\\machine_config_scratch") != std::string::npos ||
           s.find("\\general_ini_scratch\\") != std::string::npos;
}

// true = go on.  false = it printed why; main() must return 2 at once -- nothing of the Handler has run yet.
// runtimePaths: optional { "name", value, "name", value, ..., 0 }.
inline bool W906TestRequireCtestRedirects(const char* ctestName, const char* const* runtimePaths = 0)
{
    // What tests/CMakeLists.txt hands EVERY test (measured 20260928 in build_ship/tests/CTestTestfile.cmake: 219 of the
    // 220 tests carry the first twelve; the 220th is the Python START_SitesCensus).  Keep in step with:
    //   * the ONE merged ENVIRONMENT string of the MACHINE-DATA CONTAINMENT block (set_tests_properties(
    //     ${_ht9045_all_tests} ...)), copied to every later test by AI(W906-ENV-ALL) at the end of that file -- ten;
    //   * _ht9045_env_extra (AI(W906-CMYDB-P4-D5)), APPENDed to every test by ENV-ALL -- two;
    //   * the per-test Gerneral.ini sandbox, APPENDed by ENV-ALL on the same line (AI(W906-TESTGUARD)) -- one.
    // A variable dropped from CMake makes every opted-in test refuse under ctest too (exit 2, loud): update this list.
    static const char* const kVars[] = {
        "W906_E84DATA_ROOT", "W906_PRODLOG_ROOT", "W906_CLEANPADLOG_ROOT", "W906_TCPDATA_ROOT", "W906_SUMMARYLOT_ROOT",
        "W906_EVENTLOG_ROOT", "W906_INIDATA_ROOT", "W906_UNLOADERINFO_ROOT", "W906_AUTH_PATH", "W906_MACHINERECORD_DIR",
        "W906_HT9045LOG_ROOT", "W906_SAVEEVENTLOG_ROOT",
        "W906_GENERAL_INI_PATH",
    };
    const int nVars = (int)(sizeof(kVars) / sizeof(kVars[0]));
    const char* who = (ctestName && *ctestName) ? ctestName : "this test";
    int bad = 0, nRuntime = 0;
    for (int i = 0; i < nVars; ++i)
    {
        const char* v = std::getenv(kVars[i]);
        if (W906CtestGuardInScratch(v))
            continue;
        if (bad++ == 0)
            std::printf("[w906_ctest_guard] %s: not inside ctest's redirect environment:\n", who);
        std::printf("  %s = %s\n", kVars[i], v ? v : "(unset)");
    }
    for (int i = 0; runtimePaths != 0 && runtimePaths[i] != 0 && runtimePaths[i + 1] != 0; i += 2)
    {
        ++nRuntime;
        if (W906CtestGuardInScratch(runtimePaths[i + 1]))
            continue;
        if (bad++ == 0)
            std::printf("[w906_ctest_guard] %s: not inside ctest's redirect environment:\n", who);
        std::printf("  %s = %s   (runtime path)\n", runtimePaths[i], runtimePaths[i + 1]);
    }
    if (bad == 0)
    {
        std::printf("[w906_ctest_guard] %s: %d redirect variables and %d runtime paths are in ctest's scratch\n",
                    who, nVars, nRuntime);
        std::fflush(stdout);
        return true;
    }
    const char* allow = std::getenv("W906_TEST_ALLOW_REAL_FILES");
    if (allow != 0 && allow[0] == '1' && allow[1] == '\0')
    {
        std::printf("  WARNING: W906_TEST_ALLOW_REAL_FILES=1 -- running anyway; this run may write the REAL "
                    "D:\\HT9045 / D:\\HT9045_Log files named above\n");
        std::fflush(stdout);
        return true;
    }
    std::printf("  REFUSED (exit 2) -- nothing was called.  Run it through ctest: ctest --test-dir <build dir> -R \"^%s$\"\n"
                "  (a deliberate real-file run: set W906_TEST_ALLOW_REAL_FILES=1)\n", who);
    std::fflush(stdout);
    return false;
}

#endif // W906_CTEST_GUARD_H
