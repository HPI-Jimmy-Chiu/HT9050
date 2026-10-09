@echo off
rem ============================================================================
rem  start_wb_serve_ioweb.bat -- start wb_serve on the HT9050 machine WITHOUT VS Code / F5.
rem  AI(W906-W197) 20261009 (Ifor01): W-197 (machine cpp 0349, EastSun "install / update without AI").
rem  Same program, arguments and environment as .vscode/launch.json
rem  "IOWEB(...) wb_serve ship config (no debugger)": build_integ_ship_x86\wb_serve.exe --root ..\web --port 8055,
rem  machine files from D:\HT9045\system (Gerneral.ini / IO_Table.csv / Mot_Table.csv), everything else in ..\runcfg.
rem  Steps: refuse while an old wb_serve still runs -> open the wait page (switches to the HMI when wb_serve answers)
rem         -> start wb_serve in THIS window.
rem  Close: the HMI's Exit button (normal close), or run tools\stop_wb_serve.ps1. Do NOT select text in this window
rem         (Windows QuickEdit freezes wb_serve's output and with it the control loop).
rem  Manual: tools\laptop_ops\pkg_template\NO_AI_INSTALL.md (in each update package: _machine_ai\NO_AI_INSTALL.md).
rem  ASCII only on purpose (cmd.exe reads .bat files in the console code page).
rem ============================================================================
setlocal
set "TREE=%~dp0.."
for %%I in ("%TREE%") do set "TREE=%%~fI"
set "EXE=%TREE%\build_integ_ship_x86\wb_serve.exe"
set "WEB=%TREE%\..\web"
set "RUNCFG=%TREE%\..\runcfg"
if not exist "%EXE%" (
  echo [start] wb_serve.exe not found: %EXE%
  echo [start] build it first: ^<update package^>\_machine_ai\build_and_verify.ps1 or install_machine.ps1 -Build,
  echo [start] or copy the prebuilt wb_serve.exe to the folder above. See NO_AI_INSTALL.md section 4.
  pause
  exit /b 1
)
powershell -NoProfile -ExecutionPolicy Bypass -File "%TREE%\tools\check_wb_serve_not_running.ps1"
if errorlevel 1 (
  pause
  exit /b 1
)
powershell -NoProfile -ExecutionPolicy Bypass -File "%TREE%\tools\open_boot_wait.ps1"

set "W906_GENERAL_INI_PATH=D:\HT9045\system\Gerneral.ini"
set "W906_SETUPINF_PATH=%RUNCFG%\SetUp.inf"
set "W906_IOTABLE_PATH=D:\HT9045\system\IO_Table.csv"
set "W906_MOTTABLE_PATH=D:\HT9045\system\Mot_Table.csv"
set "W906_AUTH_PATH=%RUNCFG%\config\"
set "W906_E84DATA_ROOT=%RUNCFG%\logs\E84DataTxt"
set "W906_TCPDATA_ROOT=%RUNCFG%\logs\TCP_Data"
set "W906_SUMMARYLOT_ROOT=%RUNCFG%\logs\Summary_Lot"
set "W906_EVENTLOG_ROOT=%RUNCFG%\logs\EventLogTxt"
set "W906_BINCOUNT_PATH=%RUNCFG%\logs\BinCount.ini"
set "W906_PWBOOK_PATH=%RUNCFG%\logs\pwbook.ini"
set "W906_TEACH_INI_PATH=%RUNCFG%\system\teach.ini"
set "W906_IOTIMING_LOG=%RUNCFG%\logs\io_click_timing.csv"
set "W906_OPLOG_DIR=%RUNCFG%\logs"
set "W906_HMI_URL=http://127.0.0.1:8055/background.html?mode=debug&machine=HT9050"
set "W906_F5_OPERATOR_CLOSE=1"

cd /d "%TREE%"
echo [start] %EXE% --root %WEB% --port 8055
echo [start] opening the card takes 30-90 s with no screen change -- that is normal.
"%EXE%" --root "%WEB%" --port 8055
echo [start] wb_serve ended (exit code %errorlevel%).
pause
endlocal
