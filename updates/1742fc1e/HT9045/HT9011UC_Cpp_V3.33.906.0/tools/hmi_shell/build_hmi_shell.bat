@echo off
REM AI(W906-HMI-SHELL) 20260930: builds the HMI program window (tools\hmi_shell\hmi_shell.cpp) into build_hmi_shell\.
REM   Own script on purpose: the CMake source list belongs to the laptop (GitHub README notice), so this target is not in it.
REM   Toolchain = the i686 WinLibs of build_nonoracle (32-bit exe, 32-bit WebView2Loader.dll).  Takes a few seconds.
REM   Output: build_hmi_shell\ht9045_hmi.exe + WebView2Loader.dll + WebView2_LICENSE.txt
setlocal
set "HERE=%~dp0"
set "TC=%LOCALAPPDATA%\Programs\ht9045-nonoracle-toolchain\mingw32\bin"
if not exist "%TC%\g++.exe" goto :no_tc
set "PATH=%TC%;%PATH%"
set "OUT=%HERE%..\..\build_hmi_shell"
if not exist "%OUT%" mkdir "%OUT%"
windres --include-dir "%HERE%." -i "%HERE%hmi_shell.rc" -o "%OUT%\hmi_shell_res.o"
if errorlevel 1 goto :fail
g++ -std=c++14 -O2 -municode -mwindows -static -Wall -Wno-unknown-pragmas -I"%HERE%vendor\webview2\include" "%HERE%hmi_shell.cpp" "%OUT%\hmi_shell_res.o" -o "%OUT%\ht9045_hmi.exe" -lole32 -loleaut32 -luuid -lshell32 -luser32 -lgdi32
if errorlevel 1 goto :fail
copy /y "%HERE%vendor\webview2\x86\WebView2Loader.dll" "%OUT%\WebView2Loader.dll" >nul
copy /y "%HERE%vendor\webview2\LICENSE.txt" "%OUT%\WebView2_LICENSE.txt" >nul
echo built %OUT%\ht9045_hmi.exe
exit /b 0
:no_tc
echo FATAL: toolchain not found: %TC%
exit /b 1
:fail
echo FAILED: hmi_shell build
exit /b 1
