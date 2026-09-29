// =============================================================================
//  st02_test_containment.h -- St02 tests: refuse to run outside ctest's redirect roots.
//
//  AI(W906-TEST-SAFE) 20260928 (St02-E helper).
//
//  WHY.  St01 09:0x: run BY HAND (no ctest environment), test_tcp_cmd_server.exe wrote the real
//  D:\HT9045_Log\SaveEventLog\HANDLER LOG__2026_09_28.csv.  Since cMyDB P4 the golden RecordProcess / MyDBIProcess /
//  NewRecordProcess / MyDBIProcessNew bodies (cMyDB.cpp) write through as9045LogPath / asSaveEventLogPath /
//  asProductionLogPath / sProductionInfoFilePath, and only ctest's redirect environment points them away from the
//  machine (tests/CMakeLists.txt: the global ENVIRONMENT block + _ht9045_env_extra, all under
//  <build>/tests/machine_log_scratch or machine_config_scratch).  JsonBridge LogAppend (every act.*) and the 7016 pump's
//  TCPIPCommunicationLog reach the same roots.
//
//  RULE (the same as test_tcp_cmd_server.cpp "0. containment first", b12ab375, and test_mydb_p4_containment.cpp step 0):
//  the four globals must contain "machine_log_scratch" and the six variables below must point at machine_log_scratch /
//  machine_config_scratch; otherwise print why and the caller returns 2 BEFORE any Handler code runs.  It does NOT test
//  "not under D:\HT9045": ctest's scratch itself may sit under D:\HT9045\...\build.
//
//  Use (first thing in main(), after std::setvbuf):
//      if (!W906TestInsideCtestRoots("<ctest name>"))
//          return 2;
//  Header-only; the test must link the machine archives (common.cpp owns the four globals).
// =============================================================================
#ifndef ST02_TEST_CONTAINMENT_H
#define ST02_TEST_CONTAINMENT_H

#include "common.h"   // as9045LogPath, asSaveEventLogPath, asProductionLogPath, sProductionInfoFilePath, asGeneralPath

#include <cstdio>
#include <cstdlib>
#include <string>

inline std::string W906TestSafeLower(const char* p)
{
    std::string s(p ? p : "");
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] >= 'A' && s[i] <= 'Z')
            s[i] = (char)(s[i] - 'A' + 'a');
        if (s[i] == '/')
            s[i] = '\\';
    }
    return s;
}

// true = every root is ctest's scratch.  false = it printed why; the caller must return 2 at once.
inline bool W906TestInsideCtestRoots(const char* suite)
{
    const AnsiString* const roots[] = { &as9045LogPath, &asSaveEventLogPath, &asProductionLogPath, &sProductionInfoFilePath };
    const char* const names[] = { "as9045LogPath", "asSaveEventLogPath", "asProductionLogPath", "sProductionInfoFilePath" };
    const char* const envs[] = { "W906_HT9045LOG_ROOT", "W906_SAVEEVENTLOG_ROOT", "W906_RMS_ROOT", "W906_PRODINFO_ROOT",
                                 "W906_EVENTLOG_ROOT", "W906_AUTH_PATH" };
    bool contained = true;
    for (int i = 0; i < 4; ++i)
    {
        std::printf("  %s = %s\n", names[i], roots[i]->c_str());
        if (W906TestSafeLower(roots[i]->c_str()).find("machine_log_scratch") == std::string::npos)
            contained = false;
    }
    for (size_t i = 0; i < sizeof(envs) / sizeof(envs[0]); ++i)
    {
        const char* v = std::getenv(envs[i]);
        const std::string lv = W906TestSafeLower(v);
        if (lv.find("machine_log_scratch") == std::string::npos && lv.find("machine_config_scratch") == std::string::npos)
        {
            std::printf("  %s = %s\n", envs[i], v ? v : "(unset)");
            contained = false;
        }
    }
    // AI(W906-TESTGUARD-ST02) 20260928 (St02-E): since the laptop's TESTGUARD (main 3fb74540) ctest also gives every test its own
    //   Gerneral.ini: W906_GENERAL_INI_PATH = <build>/tests/general_ini_scratch/<test>/Gerneral.ini (tests/CMakeLists.txt
    //   _w906_env_all_tests, the same APPEND as the six variables above), which asGeneralPath follows (common.cpp:155 / :393).
    //   Same root and same test as tests/w906_ctest_guard.h, so the two guards agree.
    {
        const char* gv = std::getenv("W906_GENERAL_INI_PATH");
        std::printf("  asGeneralPath = %s\n", asGeneralPath.c_str());
        if (W906TestSafeLower(gv).find("\\general_ini_scratch\\") == std::string::npos ||
            W906TestSafeLower(asGeneralPath.c_str()).find("\\general_ini_scratch\\") == std::string::npos)
        {
            std::printf("  W906_GENERAL_INI_PATH = %s\n", gv ? gv : "(unset)");
            contained = false;
        }
    }
    if (!contained)
        std::printf("  ABORT: not inside ctest's redirect roots (run it with ctest -R %s) -- nothing was called\n", suite);
    return contained;
}

#endif // ST02_TEST_CONTAINMENT_H
