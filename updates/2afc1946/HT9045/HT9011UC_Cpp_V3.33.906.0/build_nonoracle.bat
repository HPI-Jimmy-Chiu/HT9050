@echo off
REM ===========================================================================
REM  build_nonoracle.bat -- THE NON-ORACLE BUILD LANE.
REM  AI(W906-MachineImport) 20260825.
REM
REM  ---------------------------------------------------------------------------
REM  WHY THIS FILE EXISTS
REM  ---------------------------------------------------------------------------
REM  build.bat is the ORACLE lane. It hard-codes MINGW_BIN=C:\MinGW\bin and
REM  FATALs when g++ is not there (build.bat:37 and :57). That guard is correct
REM  and must stay -- but on a machine without that compiler it means the VS Code
REM  preLaunchTask 'V906: Build (MinGW, quick)' fails before anything launches,
REM  which is exactly the F5 error this file was written to fix:
REM
REM      Error exists after running preLaunchTask 'V906: Build (MinGW, quick)'
REM
REM  Measured on this machine 20260825: C:\MinGW exists but holds ZERO files.
REM
REM  This is the SECOND lane, kept deliberately separate:
REM    * different compiler  : WinLibs MinGW-w64 i686 g++ 16.2.0
REM    * different standard  : C++14  (-DHT9045_CXX_STANDARD=14, see below)
REM    * different build dir : build_nonoracle\   -- build\ is NEVER touched
REM
REM  *** NO NUMBER FROM THIS LANE IS COMPARABLE TO A BCB6 BASELINE. ***
REM  Different compiler, different C++ standard, different libstdc++. Use it to
REM  SEE the software run. Never quote a test count or a float from it.
REM
REM  ---------------------------------------------------------------------------
REM  WHY C++14 IS NOT OPTIONAL HERE
REM  ---------------------------------------------------------------------------
REM  The tree does `using namespace std;` (MachineType.h:7, MachineDefine.h:30
REM  and :67, database.h:57) AND relies on the w32api `typedef unsigned char
REM  byte` from rpcndr.h. Under C++17 a libstdc++ that HAS std::byte makes every
REM  unqualified `byte` ambiguous. Measured 20260825 with g++ 16.2.0:
REM      cmydef.h:127: error: reference to 'byte' is ambiguous
REM      Motor/HTMotor.h:92, EJ1N/TextProcess.h:107  -- same error
REM  The ORACLE g++ 6.3.0 never sees this: its libstdc++ predates std::byte,
REM  which GCC added in 7. So the clash is an artefact of building with a MODERN
REM  compiler, NOT a defect in the translation -- which is why the workaround is
REM  a build flag and not an edit to any translated source file.
REM
REM  CMakeLists.txt:4 carries the HT9045_CXX_STANDARD escape hatch. It DEFAULTS
REM  TO 17, so the oracle lane and every recorded baseline are bit-for-bit
REM  unaffected. It cannot be done from the command line instead: CMake emits
REM  CMAKE_CXX_FLAGS BEFORE the standard flag (measured: CXX_FLAGS = -std=c++14
REM  -std=c++17 ...), so an appended -std= loses.
REM
REM  ---------------------------------------------------------------------------
REM  WHY -static IS NOT COSMETIC
REM  ---------------------------------------------------------------------------
REM  The WinLibs build is thread model POSIX. Without -static every exe needs
REM  libwinpthread-1.dll on PATH and dies with 0xC0000135 (STATUS_DLL_NOT_FOUND)
REM  and NO message at all. Static also reproduces the documented import table:
REM  verified 20260825 with objdump -p -- KERNEL32 / msvcrt / USER32 / VERSION /
REM  WS2_32 only.
REM
REM  ---------------------------------------------------------------------------
REM  MODES
REM  ---------------------------------------------------------------------------
REM    build_nonoracle.bat             configure-if-needed + build the two F5
REM                                    exes (wb_publish, wb_gateway). This is
REM                                    what F5's preLaunchTask runs.
REM    build_nonoracle.bat all         ... plus wb_serve.exe (single-process)
REM    build_nonoracle.bat configure   force a re-configure of build_nonoracle\
REM    build_nonoracle.bat dbgexe      build_dbg_nonoracle\ with -g, but ONLY
REM                                    wb_publish + wb_gateway. This is what the
REM                                    debug F5 compound runs. Measured 2.4 s when
REM                                    already current, ~11 s incremental.
REM    build_nonoracle.bat dbg         SAME directory, SAME -g, but ALL targets --
REM                                    which INCLUDES the 137 ctest binaries. Only
REM                                    the ctest-debug launch config needs those
REM                                    (its program is tests\<name>.exe). ~7 GB and
REM                                    RAM-hungry. Prefer dbgexe unless you need a
REM                                    test exe.
REM    build_nonoracle.bat clean       delete build_nonoracle\ ONLY
REM    build_nonoracle.bat help
REM
REM  Run it from cmd.exe. Every VS Code task in .vscode\tasks.json pins the shell
REM  to cmd.exe for this reason -- PowerShell never resolves a bare command name
REM  from the current directory.
REM ===========================================================================
setlocal EnableExtensions

set "MODE=%~1"
if "%MODE%"=="" set "MODE=quick"
if /i "%MODE%"=="help"  goto :help
if /i "%MODE%"=="-h"    goto :help
if /i "%MODE%"=="/?"    goto :help

REM ===========================================================================
REM  BOTH GUARDS BELOW ARE WRITTEN WITH goto, NOT WITH `exit /b` INSIDE A
REM  PARENTHESISED BLOCK. AI(W906-MachineImport) 20260825 -- and that is not
REM  style, it is the difference between the guard working and not working.
REM
REM  When `exit /b N` sits inside a ( ) block that has MORE STATEMENTS AFTER IT,
REM  cmd.exe can leave the PROCESS exit code at 0. The cmake guard had exactly
REM  that shape: `exit /b 1` in the inner if, followed by
REM  `set "HT9045_CMAKE=cmake.exe"` still inside the outer block. This file is
REM  F5's preLaunchTask, and VS Code gates the launch on the task's exit code --
REM  so a 0 there means the debugger starts anyway. The ONE guard whose whole job
REM  is to stop F5 when cmake is missing would have failed to stop it, and the
REM  failure would then surface as an unrelated-looking cppdbg error.
REM
REM  It could never be caught on this machine by testing the happy path: cmake
REM  IS present here, so that branch is never taken. Found by an adversarial
REM  audit of this file, then reproduced by forcing HT9045_CMAKE to a bogus path.
REM ===========================================================================

REM ---- toolchain -----------------------------------------------------------
if not defined HT9045_NONORACLE_BIN set "HT9045_NONORACLE_BIN=%LOCALAPPDATA%\Programs\ht9045-nonoracle-toolchain\mingw32\bin"
if not exist "%HT9045_NONORACLE_BIN%\g++.exe" goto :no_gxx

REM ---- cmake ---------------------------------------------------------------
if not defined HT9045_CMAKE set "HT9045_CMAKE=%LOCALAPPDATA%\Programs\cmake-4.4.2-windows-x86_64\bin\cmake.exe"
if exist "%HT9045_CMAKE%" goto :cmake_ok
where cmake.exe >nul 2>nul
if errorlevel 1 goto :no_cmake
set "HT9045_CMAKE=cmake.exe"
:cmake_ok

set "PATH=%HT9045_NONORACLE_BIN%;%PATH%"
set "BINF=%HT9045_NONORACLE_BIN:\=/%"
if not defined HT9045_JOBS set "HT9045_JOBS=%NUMBER_OF_PROCESSORS%"
if not defined HT9045_JOBS set "HT9045_JOBS=4"

REM ---------------------------------------------------------------------------
REM  Debug-lane parallelism is capped by RAM, NOT by core count.
REM  AI(W906-GL-DbgJobs) 20260826.
REM
REM  Core count is the wrong budget for a Debug link. This machine has 28 cores
REM  and 7.68 GB of RAM; a Debug wb_publish.exe is 71.7 MB with 51.6 MB of
REM  .debug_info, and `ld` needs several hundred MB of working set to produce
REM  one. Measured 20260826: `dbg` at -j 28 drove free RAM to 0.28 GB, stalled
REM  at 89% for many minutes, and starved an unrelated gdb session on the same
REM  box into apparent hangs. It did not fail -- which is worse, because the
REM  operator sees "F5 is broken" with no error anywhere.
REM
REM  So: allow roughly 1.5 GB of RAM per Debug link job, floor 2, and never
REM  more than the core count. The probe is a PowerShell spawn costing ~0.4 s,
REM  so it is NOT run here -- it is called from :dbg / :dbgexe only. The quick
REM  lane is F5's preLaunchTask and has to stay at its measured ~2 s; paying
REM  for a Debug-only measurement on every single F5 would be a regression in
REM  the one path an operator waits on. See :calc_dbg_jobs at the bottom.
REM ---------------------------------------------------------------------------

echo [nonoracle] ====================================================================
echo [nonoracle]  NON-ORACLE LANE -- proves the software RUNS, never proves the
echo [nonoracle]  translation is FAITHFUL. Do not compare its numbers to BCB6.
echo [nonoracle] ====================================================================
echo [nonoracle] mode   : %MODE%
echo [nonoracle] g++    : %HT9045_NONORACLE_BIN%\g++.exe
echo [nonoracle] cmake  : %HT9045_CMAKE%
echo [nonoracle] jobs   : %HT9045_JOBS%

if /i "%MODE%"=="clean"     goto :clean
if /i "%MODE%"=="dbgexe"    goto :dbgexe
if /i "%MODE%"=="dbg"       goto :dbg
if /i "%MODE%"=="configure" goto :configure
if /i "%MODE%"=="all"       goto :quick
if /i "%MODE%"=="quick"     goto :quick

echo [nonoracle] unknown mode "%MODE%".
goto :help

REM ===========================================================================
:configure
call :do_configure "build_nonoracle" ""
if errorlevel 1 exit /b 1
echo [nonoracle] configure done: build_nonoracle\
exit /b 0

REM ===========================================================================
:quick
if not exist "build_nonoracle\CMakeCache.txt" (
    call :do_configure "build_nonoracle" ""
    if errorlevel 1 exit /b 1
)
REM AI(W906-T4-RETIRE) 20260919: 這裡本來是 `set "TARGETS=wb_publish wb_gateway"`，
REM   而 MODE=all 再加上 wb_serve。那兩個 target 已依使用者裁決退役
REM   （計畫書 §11.4 的 R2），所以照舊名字建置會**直接失敗** ——
REM   cmake --build --target 對一個不存在的名字是錯誤，不是 no-op。
REM   wb_serve 現在是唯一的執行檔，兩個模式因此收斂成同一個目標。
REM   MODE=all 保留成可接受的別名而不是移除，免得既有的呼叫開始報「未知模式」。
set "TARGETS=wb_serve"
echo [nonoracle] building: %TARGETS%
"%HT9045_CMAKE%" --build "build_nonoracle" --target %TARGETS% -j %HT9045_JOBS%
if errorlevel 1 (
    echo [nonoracle] BUILD FAILED. Nothing was launched.
    exit /b 1
)
echo [nonoracle] OK  build_nonoracle\wb_serve.exe
exit /b 0

REM ===========================================================================
REM  dbg: a SEPARATE dir with -g. build_nonoracle\ has no CMAKE_BUILD_TYPE, so
REM  CXX_FLAGS carries no -g and breakpoints show as hollow grey circles with no
REM  explanation from VS Code. Debug (not RelWithDebInfo) is deliberate: under
REM  GCC, Debug adds -g only (-O0 is already the default), so the generated code
REM  matches the plain build. RelWithDebInfo brings -O2 and optimises locals away.
:dbg
call :calc_dbg_jobs
if not exist "build_dbg_nonoracle\CMakeCache.txt" (
    call :do_configure "build_dbg_nonoracle" "-DCMAKE_BUILD_TYPE=Debug"
    if errorlevel 1 exit /b 1
)
REM  NO --target HERE, DELIBERATELY. AI(W906-MachineImport) 20260825: this used
REM  to say `--target wb_publish wb_gateway`, which made the launch config
REM  the launch config whose name ends "...(build_dbg_nonoracle)" impossible to
REM  start -- its program is build_dbg_nonoracle\tests\<name>.exe, and a
REM  two-exe target list never builds a single test binary. Measured: after a
REM  --target wb_publish wb_gateway build, build_nonoracle\tests\ held 0 exes.
REM  tests\CMakeLists.txt has ZERO EXCLUDE_FROM_ALL, so the default ALL target
REM  already covers libs + tests + exes -- the same thing build.bat's quick mode
REM  builds for the oracle lane. Cost: the FIRST dbg build is a long one.
REM
REM  KNOW WHAT ELSE "ALL" PULLS IN. Measured on this machine 20260825, ALL also
REM  produced pci1203_linkprobe.exe, which LINKS THE REAL ADVANTECH MOTION SDK
REM  (the CMake target only exists where find_library locates ADVMOT.lib -- it
REM  does here; this box is a real machine controller). The quick mode above
REM  names three targets explicitly and never builds it, so this is new with dbg.
REM
REM  It is safe to have built, and that is verifiable rather than assumed:
REM  tools/pci1203_linkprobe.cpp takes function POINTERS to Acm_GetAvailableDevs
REM  and Acm_GetErrorMessage, CALLS NEITHER, and prints their addresses. Its own
REM  header says "BUILD-ONLY BY DESIGN ... no motion, no device opened". Only the
REM  explicit `--enumerate` argument calls into the SDK, and even then it only
REM  enumerates -- it never opens a device. Without a card that call blocks 15+
REM  seconds (vendor behaviour, measured 20260818 by 1203HAL-3).
REM
REM  DO NOT run it with --enumerate as a casual smoke test on a live machine,
REM  and do not turn it into a ctest -- its own header asks for both.
REM
REM  Also note: the link succeeded here under the NON-ORACLE g++ 16.2.0. The
REM  deliverable 1203HAL-3 wanted is a link under the PINNED oracle g++ 6.3.0.
REM  This result is encouraging, not the deliverable. Do not record it as one.
REM
REM  Disk: build_dbg_nonoracle\ measured 7.18 GB (137 test exes, 6.66 GB of that).
echo [nonoracle] building Debug ^(-g^): ALL targets ^(libs + tests + exes^)
echo [nonoracle] the FIRST dbg build is a full one -- expect several minutes.
echo [nonoracle] NOTE: only the ctest-debug launch config needs the test exes.
echo [nonoracle]       To debug wb_serve, use `dbgexe` instead --
echo [nonoracle]       same directory, same -g, without 137 Debug test binaries.
"%HT9045_CMAKE%" --build "build_dbg_nonoracle" -j %HT9045_DBG_JOBS%
if errorlevel 1 (
    echo [nonoracle] BUILD FAILED.
    exit /b 1
)
echo [nonoracle] OK  build_dbg_nonoracle\ has -g -- breakpoints will bind.
echo [nonoracle]     exes  : build_dbg_nonoracle\wb_serve.exe
echo [nonoracle]     ctest : build_dbg_nonoracle\tests\*.exe
exit /b 0

REM ===========================================================================
REM  dbgexe: the SAME build_dbg_nonoracle\ directory and the SAME -g, but only
REM  the one exe the HMI debug session actually launches.
REM  AI(W906-T4-RETIRE) 20260919: was wb_publish + wb_gateway; both retired
REM  (plan 11.4 R2/R5). Building them by name would now FAIL, not no-op.
REM  AI(W906-GL-DbgExe) 20260826.
REM
REM  WHY THIS IS NOT A SHORTCUT. The `dbg` mode above must keep building ALL,
REM  and the comment block above says exactly why: the ctest-debug launch
REM  config's program is build_dbg_nonoracle\tests\<name>.exe, and a two-target
REM  build never produces one. That reasoning is about DEBUGGING A TEST. It
REM  does not apply to the "star" HMI debug compound, whose two programs are
REM  wb_publish.exe and wb_gateway.exe and nothing else.
REM
REM  Charging that compound for 137 Debug test binaries is not caution, it is a
REM  measured failure mode: 20260826 on this box (28 cores / 7.68 GB) a full
REM  dbg build exhausted RAM and hung at 89%, so pressing the debug F5 looked
REM  identical to "the debugger is broken". Same directory means this stays
REM  incremental with `dbg` -- run either, in any order, no reconfigure.
REM
REM  Use `dbg` when you need a test exe. Use `dbgexe` to debug the HMI.
:dbgexe
call :calc_dbg_jobs
if not exist "build_dbg_nonoracle\CMakeCache.txt" (
    call :do_configure "build_dbg_nonoracle" "-DCMAKE_BUILD_TYPE=Debug"
    if errorlevel 1 exit /b 1
)
echo [nonoracle] building Debug ^(-g^): wb_serve only
echo [nonoracle] jobs   : %HT9045_DBG_JOBS% ^(RAM-capped; -g links are memory-hungry^)
"%HT9045_CMAKE%" --build "build_dbg_nonoracle" --target wb_serve -j %HT9045_DBG_JOBS%
if errorlevel 1 (
    echo [nonoracle] BUILD FAILED. Nothing was launched.
    exit /b 1
)
echo [nonoracle] OK  build_dbg_nonoracle\wb_serve.exe  ^(-g, breakpoints bind^)
echo [nonoracle] NOTE: tests\*.exe are NOT built by this mode -- use `dbg` for those.
exit /b 0

REM ===========================================================================
:clean
if exist "build_nonoracle" (
    echo [nonoracle] deleting build_nonoracle\ ...
    rmdir /s /q "build_nonoracle"
)
echo [nonoracle] clean done. build\ and build_dbg_nonoracle\ were NOT touched.
exit /b 0

REM ===========================================================================
REM  Compute HT9045_DBG_JOBS. Called only from :dbg / :dbgexe -- see the long
REM  comment near the top for why this is RAM-derived and why the quick lane
REM  must not pay for it. Honours a caller-set HT9045_DBG_JOBS untouched.
:calc_dbg_jobs
if defined HT9045_DBG_JOBS exit /b 0
set "HT9045_DBG_JOBS=%HT9045_JOBS%"
for /f "usebackq delims=" %%J in (`powershell -NoProfile -ExecutionPolicy Bypass -Command "$g=(Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory/1GB; $n=[int][Math]::Floor($g/1.5); if($n -lt 2){$n=2}; if($n -gt %HT9045_JOBS%){$n=%HT9045_JOBS%}; $n" 2^>nul`) do set "HT9045_DBG_JOBS=%%J"
exit /b 0

REM ===========================================================================
REM  %1 = build dir, %2 = extra cmake args (may be empty)
:do_configure
REM  AI(W906-NINJA-ROLLOUT) 20261006 (NB2-2b A16): Ninja when ninja.exe is found -- FIRST the one the WinLibs toolchain
REM  ships next to g++ (16.2.0 zip: mingw32\bin\ninja.exe 1.13.2, so every PC with this lane already has it), then
REM  PATH and the winget package dir (the same lookup as build.bat:130-132) -- else "MinGW Makefiles" as before. A dir that already has a CMakeCache.txt KEEPS its generator and make program --
REM  CMake refuses to change a generator in place -- so `configure` on an old make dir still works; switch such a dir
REM  with tools\ninja_setup.ps1 -Switch <dir>. HT9045_GENERATOR=MinGW Makefiles forces make for a new dir.
REM  The builds below all use "cmake --build", which drives either generator.
set "NO_GEN=MinGW Makefiles"
set "NO_MAKE=%BINF%/mingw32-make.exe"
set "NO_NINJA="
if exist "%HT9045_NONORACLE_BIN%\ninja.exe" set "NO_NINJA=%HT9045_NONORACLE_BIN%\ninja.exe"
if not defined NO_NINJA for %%P in (ninja.exe) do if exist "%%~$PATH:P" set "NO_NINJA=%%~$PATH:P"
if not defined NO_NINJA for /d %%D in ("%LOCALAPPDATA%\Microsoft\WinGet\Packages\Ninja-build.Ninja_*") do if exist "%%~D\ninja.exe" set "NO_NINJA=%%~D\ninja.exe"
if defined NO_NINJA set "NO_GEN=Ninja"
if defined NO_NINJA set "NO_MAKE=%NO_NINJA:\=/%"
if /i "%HT9045_GENERATOR%"=="MinGW Makefiles" set "NO_GEN=MinGW Makefiles"
if /i "%HT9045_GENERATOR%"=="MinGW Makefiles" set "NO_MAKE=%BINF%/mingw32-make.exe"
if exist "%~1\CMakeCache.txt" for /f "tokens=2 delims==" %%G in ('findstr /b /c:"CMAKE_GENERATOR:INTERNAL=" "%~1\CMakeCache.txt"') do set "NO_GEN=%%G"
if exist "%~1\CMakeCache.txt" for /f "tokens=2 delims==" %%M in ('findstr /b /c:"CMAKE_MAKE_PROGRAM:" "%~1\CMakeCache.txt"') do set "NO_MAKE=%%M"
echo [nonoracle] configuring "%~1" %~2  ^(generator: %NO_GEN%^)
"%HT9045_CMAKE%" -S . -B "%~1" -G "%NO_GEN%" %~2 -DHT9045_CXX_STANDARD=14 -DCMAKE_EXE_LINKER_FLAGS=-static -DCMAKE_CXX_COMPILER="%BINF%/g++.exe" -DCMAKE_C_COMPILER="%BINF%/gcc.exe" -DCMAKE_MAKE_PROGRAM="%NO_MAKE%"
if errorlevel 1 (
    echo [nonoracle] FATAL: cmake configure failed for "%~1".
    exit /b 1
)
exit /b 0

REM ===========================================================================
REM  FATAL exits. Reached only by goto, so nothing can fall into them and nothing
REM  follows the exit inside a block -- the exit code always reaches the caller.
:no_gxx
echo [nonoracle] FATAL: g++ not found at "%HT9045_NONORACLE_BIN%\g++.exe"
echo [nonoracle]        That is the NON-ORACLE toolchain ^(WinLibs MinGW-w64 i686 g++ 16.2.0^).
echo [nonoracle]        Point HT9045_NONORACLE_BIN at its bin\ directory, or install it.
echo [nonoracle]        NOTE: the ORACLE lane is build.bat and wants C:\MinGW instead --
echo [nonoracle]        do NOT install this toolchain into C:\MinGW, that would satisfy
echo [nonoracle]        build.bat's guard with the WRONG compiler, which is the one failure
echo [nonoracle]        mode that produces silently-wrong numbers instead of an error.
exit /b 1

:no_cmake
echo [nonoracle] FATAL: cmake not found.
echo [nonoracle]        Tried "%HT9045_CMAKE%" and PATH.
echo [nonoracle]        Point HT9045_CMAKE at cmake.exe, or put it on PATH.
echo [nonoracle]        cmake and ctest ship together; if you add one, add both.
exit /b 1

REM ===========================================================================
:help
echo.
echo   build_nonoracle.bat             configure-if-needed + build wb_serve into
echo                                   build_nonoracle\  ^(F5 uses this^)
echo   build_nonoracle.bat all         ... plus wb_serve.exe
echo   build_nonoracle.bat configure   force a re-configure
echo   build_nonoracle.bat dbgexe      build_dbg_nonoracle\ with -g, ONLY the two
echo                                   HMI exes -- this is what the debug F5 wants
echo   build_nonoracle.bat dbg         build_dbg_nonoracle\ with -g, ALL targets
echo                                   ^(libs + tests + exes^) -- needed only to
echo                                   debug a ctest exe; ~7 GB and RAM-hungry
echo   build_nonoracle.bat clean       delete build_nonoracle\ only
echo.
echo   Env overrides: HT9045_NONORACLE_BIN, HT9045_CMAKE, HT9045_JOBS,
echo                  HT9045_DBG_JOBS ^(defaults to RAM/1.5GB, capped by cores^)
echo.
echo   The ORACLE lane is build.bat ^(needs MinGW.org g++ 6.3.0 at C:\MinGW^).
echo   Only the oracle lane produces numbers you may compare to BCB6.
echo.
exit /b 2
