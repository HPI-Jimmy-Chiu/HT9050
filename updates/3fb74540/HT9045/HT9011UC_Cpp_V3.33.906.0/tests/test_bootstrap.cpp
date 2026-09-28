// =============================================================================
//  test_bootstrap.cpp -- BATCH-SAFETY bootstrap linked into EVERY test target.
//
//  AI(W906-TestBootstrap) 20260728: added after an MSVC Debug-CRT test run
//  wedged the whole suite and spammed the developer's screen with modal
//  dialogs.
//
//  THE PROBLEM
//  -----------
//  These tests are run in batch (`ctest -jN`, 95 executables, often unattended
//  and often from an agent's background shell). On Windows, several failure
//  modes do NOT just return a non-zero exit code -- they raise a MODAL DIALOG
//  and block forever waiting for a human to click OK:
//
//    * MSVC **Debug CRT**: a failed assert() / _CrtDbg report pops the classic
//      "Debug Assertion Failed!" box.  Observed 20260728: an MSVC Debug build
//      of this suite reached 94/95 and then hung indefinitely on exactly this.
//      Nothing is written to the ctest log while it hangs, so the run just
//      looks stalled -- the log is actively misleading.
//    * MSVC CRT invalid-parameter handler: terminates via the same dialog path.
//    * abort(): the CRT's own abort message box, plus a WER report.
//    * Hard crashes (access violation, STATUS_STACK_OVERFLOW): the Windows
//      Error Reporting "program stopped working" box.  This is NOT MSVC-only --
//      this project has hit both a SegFault and a real STATUS_STACK_OVERFLOW
//      (the 16MB embedded THGem buffer, see DEVLOG 2026-07-27) under MinGW.
//
//  A blocking dialog in a batch run is strictly worse than a loud failure: it
//  converts "one test failed" into "the entire suite hangs and the machine
//  becomes unusable until someone dismisses N dialogs".
//
//  WHAT THIS DOES
//  --------------
//  Runs one static initializer, before main(), in every test executable, that
//  turns every one of those modal paths into stderr output + a non-zero exit.
//  It does NOT hide or suppress failures -- a failing test still fails, and now
//  it fails *visibly and immediately* with the diagnostic on stderr where ctest
//  captures it, instead of silently blocking.
//
//  This is TEST-ONLY infrastructure.  It is deliberately NOT linked into the
//  production libraries: the shipped application should keep Windows' default
//  error-reporting behaviour.
//
//  Wired in via `link_libraries(ht9045_test_bootstrap)` at the top of
//  tests/CMakeLists.txt, an INTERFACE library that carries this file as an
//  INTERFACE source -- so it is COMPILED INTO each test executable.  (It is
//  deliberately not a static library: an object whose only content is a static
//  initializer can be dropped by the linker when nothing references it.)
// =============================================================================
#ifdef _WIN32

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

#if defined(_MSC_VER)
#  include <crtdbg.h>
#  include <float.h>   // _controlfp_s / _PC_64 / _MCW_PC (FP fidelity, below)
#endif

// AI(W906-FW0) 20260817: -std=c++17 defines __STRICT_ANSI__, which hides
// MinGW's _putenv declaration. Declare it ourselves (msvcrt.dll exports it);
// NOT SetEnvironmentVariableA -- that writes the Win32 block only, and the
// CRT's getenv (what csystem.cpp reads) snapshots its own copy at startup.
#if defined(__MINGW32__)
extern "C" int _putenv(const char *);
#endif

namespace {

#if defined(_MSC_VER)
// Replaces the CRT's default invalid-parameter behaviour (which ends in the
// same modal dialog).  Reports to stderr and exits non-zero so ctest records a
// real failure.
void HT9045_TestInvalidParameterHandler(const wchar_t *expression,
                                        const wchar_t *function,
                                        const wchar_t *file,
                                        unsigned int   line,
                                        uintptr_t      /*pReserved*/)
{
    fprintf(stderr,
            "[test_bootstrap] FATAL: CRT invalid parameter -- file=%ls line=%u "
            "function=%ls expression=%ls\n",
            file       ? file       : L"<unknown>",
            line,
            function   ? function   : L"<unknown>",
            expression ? expression : L"<unknown>");
    fflush(stderr);
    _exit(3);
}
#endif // _MSC_VER

struct HT9045_DisableModalErrorDialogs
{
    HT9045_DisableModalErrorDialogs()
    {
        // --- Win32 level (applies to BOTH MinGW and MSVC builds) -------------
        // SEM_FAILCRITICALERRORS  : no "there is no disk in the drive" box.
        // SEM_NOGPFAULTERRORBOX   : no WER "program stopped working" box on an
        //                           access violation / stack overflow.
        // SEM_NOOPENFILEERRORBOX  : no "file not found" box.
        SetErrorMode(SEM_FAILCRITICALERRORS |
                     SEM_NOGPFAULTERRORBOX  |
                     SEM_NOOPENFILEERRORBOX);

        // AI(W906-FW0) 20260817: redirect the production BinCount/TrayID store
        // for EVERY test executable. csystem.cpp's ReadWriteBinCountMode /
        // ReadWriteTrayID hardcode D:\HT9045\system\BinCount.txt (golden
        // faithful), and W7_L1 tray tests were MEASURED rewriting that live
        // production file from ctest (tools/webprobe/system_guard, 20260817).
        // An env var, read at call time inside csystem.cpp, is deliberately
        // chosen over a link-level seam: this TU is compiled into every test
        // whether or not it links csystem.obj, so a symbol reference here
        // would break non-csystem tests, and a dynamic-init global in
        // csystem.cpp would race this initializer (static-init order).
        // Production behaviour is untouched -- the env var only ever exists
        // inside test processes. Relative path lands in the test's cwd
        // (build_*/tests), never in D:\HT9045\system.
        _putenv("W906_BINCOUNT_PATH=w906_bincount_scratch.txt");

#if defined(_MSC_VER)
        // --- MSVC Debug CRT --------------------------------------------------
        // Send assert / error / warn reports to stderr instead of a dialog.
        // Without this, ONE failed assert in a Debug build blocks `ctest -jN`
        // indefinitely (observed 20260728).
        const int reports[] = { _CRT_ASSERT, _CRT_ERROR, _CRT_WARN };
        for (int i = 0; i < 3; ++i)
        {
            _CrtSetReportMode(reports[i], _CRTDBG_MODE_FILE);
            _CrtSetReportFile(reports[i], _CRTDBG_FILE_STDERR);
        }

        // abort(): suppress both the CRT's own message box and the WER report,
        // so it just terminates with a non-zero code that ctest can record.
        _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

        _set_invalid_parameter_handler(&HT9045_TestInvalidParameterHandler);
#endif // _MSC_VER
    }
};

// Static init -- runs before main() in whichever test executable compiles
// this TU.  Inert unless something actually fails.
HT9045_DisableModalErrorDialogs g_ht9045_disable_modal_error_dialogs;

// AI(W906-W7-A1) 20260728: MSVC/x86-only FP-precision fixup so the MSVC
// second oracle reproduces BCB6's x87 80-bit intermediate precision instead
// of SSE2's 64-bit double precision. Per docs/W7_UI_ARCHITECTURE_PLAN.md
// SS4-V5 (independently re-verified there with a *runtime*, non-constant-
// folded input -- a compile-time constant such as (int)(1.234*1000.0) gives
// 1234 on BOTH compilers and hides the discrepancy): the root
// CMakeLists.txt's `if(CMAKE_SIZEOF_VOID_P EQUAL 4) add_compile_options(
// /arch:IA32) endif()` selects x87 codegen, but codegen alone is not
// sufficient -- the CRT's runtime FP control word still defaults to 53-bit
// (double) precision on entry, which silently narrows every intermediate
// even with x87 instructions selected. Only widening the control word via
// _controlfp_s(..., _PC_64, _MCW_PC) to 64-bit (extended) precision,
// together with /arch:IA32, reproduces MinGW/BCB6's runtime result (e.g.
// tests/test_cUnitConvert.cpp's CHECK_INT(iUnitMultiply1000(1.234), 1233) --
// MSVC without this fixup gives 1234). Only compiled for 32-bit MSVC
// (_M_IX86); this project's MSVC build is always 32-bit (D9/D10 + R11 in
// the plan -- the driver layer requires 32-bit), and MinGW needs no
// equivalent call since g++ 6.3 already reproduces the x87 result natively.
#if defined(_MSC_VER) && defined(_M_IX86)
struct HT9045_ControlFpX87Fidelity
{
    HT9045_ControlFpX87Fidelity()
    {
        unsigned int old_state = 0;
        _controlfp_s(&old_state, _PC_64, _MCW_PC);
    }
};

HT9045_ControlFpX87Fidelity g_ht9045_controlfp_x87_fidelity;
#endif // _MSC_VER && _M_IX86

// AI(W906-LASTDATA-TESTDIR) 20260926: 每個測試行程一個 lastdata*.dat 沙盒（cprod.cpp 檔尾 W906_LastDataPath）。
//   建在工作目錄底下（ctest 的工作目錄是 build dir，可重生；不寫 %TEMP% —— 這台的 EDR 會殺寫到 D:\HT9045 外的行程），
//   開始時把 D:\HT9045\system 的三個檔複製進來當種子（讀的測試看到的內容跟以前一樣；原檔只讀不寫），
//   WriteLastDataFile 寫的 config.ini 那四節也落在這裡（cprod.cpp:2085；不複製種子，它只寫不讀），
//   正常結束時刪掉。當掉的話資料夾會留在 build dir（無害，下一次 clean 就沒了）。
struct W906LastDataSandbox
{
    char dir[MAX_PATH];
    bool made;
    W906LastDataSandbox() : made(false)
    {
        char cwd[MAX_PATH];
        const DWORD n = GetCurrentDirectoryA(sizeof(cwd), cwd);
        if (n == 0 || n >= sizeof(cwd)) return;
        const unsigned long pid = static_cast<unsigned long>(GetCurrentProcessId());
        std::snprintf(dir, sizeof(dir), "%s\\w906_ctest_lastdata_%lu\\", cwd, pid);
        if (!CreateDirectoryA(dir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) return;
        made = true;
        static const char* const kLeaf[3] = { "lastdata.dat", "lastdata_backup.dat", "lastdata_backup2.dat" };
        for (int i = 0; i < 3; ++i) {
            char src[MAX_PATH], dst[MAX_PATH];
            std::snprintf(src, sizeof(src), "D:\\HT9045\\system\\%s", kLeaf[i]);
            std::snprintf(dst, sizeof(dst), "%s%s", dir, kLeaf[i]);
            CopyFileA(src, dst, FALSE);            // 原檔不存在就不複製（跟以前一樣讀不到）
        }
        char name[64];
        std::snprintf(name, sizeof(name), "W906_CTEST_LASTDATA_DIR_%lu", pid);
        SetEnvironmentVariableA(name, dir);
    }
    ~W906LastDataSandbox()
    {
        if (!made) return;
        static const char* const kLeaf[4] = { "lastdata.dat", "lastdata_backup.dat", "lastdata_backup2.dat", "config.ini" };   // config.ini：WriteLastDataFile 寫的（cprod.cpp:2085），不當種子
        for (int i = 0; i < 4; ++i) {
            char f[MAX_PATH];
            std::snprintf(f, sizeof(f), "%s%s", dir, kLeaf[i]);
            DeleteFileA(f);
        }
        RemoveDirectoryA(dir);
    }
};
W906LastDataSandbox g_w906_lastdata_sandbox;

// AI(W906-TESTGUARD) 20260928: the per-test Gerneral.ini sandbox (St02 FROM_STEVEN 0928 11:03 item 6 (b)).
//   asGeneralPath already has a seam -- W906_GENERAL_INI_PATH, read at BOTH sites (common.cpp :155 declaration, :393
//   InitCommonString) -- and tests/CMakeLists.txt (AI(W906-ENV-ALL) at the file end, on its _ht9045_env_extra APPEND
//   line) now hands every test W906_GENERAL_INI_PATH=<build dir>/tests/general_ini_scratch/<test name>/Gerneral.ini and
//   W906_GENERAL_INI_SEED=D:/HT9045/system/Gerneral.ini (golden's literal).  The path must come from ctest, not from a
//   _putenv here: asGeneralPath is a dynamic-init global in an archive member, and MinGW 6.3 runs the archives' .ctors
//   BEFORE a default-priority object of this TU (measured 20260928 on a 3-TU probe: plain .ctors in reverse link order;
//   an init_priority(101) object before all of them).  This object only makes the FILE: before main(), a fresh copy of
//   the seed over the per-test path, so every read sees the machine's bytes as before, and a missing-key write-back
//   (CheckAndReadIniDataGeneral through LoadMachineConfig / ReadTechData, WriteIniData(asGeneralPath, ...)) lands in the
//   build tree.  init_priority(101) under GCC so the copy exists before any archive initializer could read it (same
//   probe: without it an archive initializer saw no file, with it the file was there); MSVC orders .CRT$XCU by link
//   order and this TU is linked before the archives (documented behaviour, not measured here).
//   One path PER TEST because ctest -jN runs tests concurrently and the copy is remade at every start.  The copy is KEPT
//   after the run (the next run replaces it): diff it against the seed to see what a test wrote back.
//   Touches nothing unless BOTH variables are set, and never a target outside a ...\general_ini_scratch\ directory -- a
//   by-hand environment such as .vscode/launch.json's W906_GENERAL_INI_PATH=D:\HT9045\system\Gerneral.ini is left
//   alone.  A seed that does not exist leaves NO copy, so the test sees "no Gerneral.ini", as it did before.
//   (No <cstring> on purpose: an #include line above would move the lines this file is cited by.)
bool W906PathHasPart(const char* lowPath, const char* part)
{
    for (const char* p = lowPath; *p != '\0'; ++p)
    {
        const char* a = p;
        const char* b = part;
        while (*b != '\0' && *a == *b) { ++a; ++b; }
        if (*b == '\0') return true;
    }
    return false;
}
char* W906LastBackslash(char* path)
{
    char* last = NULL;
    for (char* p = path; *p != '\0'; ++p)
        if (*p == '\\') last = p;
    return last;
}
struct W906GeneralIniSandbox
{
    W906GeneralIniSandbox()
    {
        char dst[MAX_PATH], seed[MAX_PATH], low[MAX_PATH];
        const DWORD nd = GetEnvironmentVariableA("W906_GENERAL_INI_PATH", dst, sizeof(dst));
        const DWORD ns = GetEnvironmentVariableA("W906_GENERAL_INI_SEED", seed, sizeof(seed));
        if (ns == 0 || ns >= sizeof(seed) || nd == 0 || nd >= sizeof(dst)) return;   // not a ctest sandbox run: touch nothing
        for (DWORD i = 0; i <= nd; ++i)                                           // i == nd copies the terminating NUL
        {
            if (dst[i] == '/') dst[i] = '\\';
            low[i] = (dst[i] >= 'A' && dst[i] <= 'Z') ? static_cast<char>(dst[i] - 'A' + 'a') : dst[i];
        }
        if (!W906PathHasPart(low, "\\general_ini_scratch\\"))
        {
            std::fprintf(stderr, "[test_bootstrap] W906_GENERAL_INI_PATH=%s is not under general_ini_scratch -- not seeded\n", dst);
            return;
        }
        char* leaf = W906LastBackslash(dst);
        if (leaf == NULL) return;
        *leaf = '\0';                                                              // dst = ...\general_ini_scratch\<test>
        char* up = W906LastBackslash(dst);
        if (up != NULL) { *up = '\0'; CreateDirectoryA(dst, NULL); *up = '\\'; }  // ...\general_ini_scratch (ERROR_ALREADY_EXISTS is fine)
        CreateDirectoryA(dst, NULL);                                               // ...\general_ini_scratch\<test>
        *leaf = '\\';
        SetFileAttributesA(dst, FILE_ATTRIBUTE_NORMAL);
        DeleteFileA(dst);                                                          // a missing seed leaves no copy, as before
        if (CopyFileA(seed, dst, FALSE))
        {
            SetFileAttributesA(dst, FILE_ATTRIBUTE_NORMAL);                        // never inherit a read-only bit from the seed
            return;
        }
        const DWORD err = GetLastError();
        if (GetFileAttributesA(seed) != INVALID_FILE_ATTRIBUTES)                   // the seed is there, the copy failed: say so
            std::fprintf(stderr, "[test_bootstrap] could not copy %s to %s (error %lu) -- this test sees no Gerneral.ini\n",
                         seed, dst, static_cast<unsigned long>(err));
    }
};
#if defined(__GNUC__)
W906GeneralIniSandbox g_w906_general_ini_sandbox __attribute__((init_priority(101)));
#else
W906GeneralIniSandbox g_w906_general_ini_sandbox;
#endif

} // anonymous namespace

#endif // _WIN32
