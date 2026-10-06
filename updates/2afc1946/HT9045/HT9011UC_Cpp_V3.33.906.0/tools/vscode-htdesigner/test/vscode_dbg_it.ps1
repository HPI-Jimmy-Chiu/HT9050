# AI(W906-HTDESIGNER) 20261006 (ES02): Debug in a real VS Code (test\integration_dbg\index.js): a small 32-bit program with
# ten threads is built with the tree's WinLibs i686 g++, then debugged through the C++ extension (cpptools, COPIED in once
# from the user's extensions, as test\vscode_run_it.ps1 does) with a launch that names gdb -- on a PC where gdb cannot
# start a program the extension hands it to LLDB's lldb-mi. A breakpoint must stop, a variable read, Continue end it.
# Nothing of the machine is touched (the program only counts). ASCII only (Windows PowerShell 5.1).
param([int]$TimeoutSec = 300)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$ext = Resolve-Path (Join-Path $here '..')
$code = Join-Path $env:LOCALAPPDATA 'Programs\Microsoft VS Code\Code.exe'
$tc = Join-Path $env:LOCALAPPDATA 'Programs\ht9045-nonoracle-toolchain\mingw32\bin'
if (-not (Test-Path (Join-Path $tc 'gcc.exe'))) { Write-Host "FATAL: the i686 toolchain is not at $tc"; exit 2 }
$cpp = Get-ChildItem (Join-Path $env:USERPROFILE '.vscode\extensions') -Directory -Filter 'ms-vscode.cpptools-*-win32-x64' | Sort-Object Name | Select-Object -Last 1
if (-not $cpp) { Write-Host 'FATAL: ms-vscode.cpptools is not installed'; exit 2 }
$cache = Join-Path $env:TEMP 'htd_it_cpptools'
$cacheExt = Join-Path $cache $cpp.Name
if (-not (Test-Path (Join-Path $cacheExt 'package.json'))) {
  New-Item -ItemType Directory -Force $cache | Out-Null
  robocopy $cpp.FullName $cacheExt /E /NFL /NDL /NJH /NJS /NP | Out-Null
}
$tmp = Join-Path $env:TEMP ('htd_vscode_dbgit_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Force (Join-Path $tmp 'user\User'), (Join-Path $tmp 'ext'), (Join-Path $tmp 'prog') | Out-Null
cmd /c mklink /J "$(Join-Path $tmp ('ext\' + $cpp.Name))" "$cacheExt" | Out-Null
# the program: built here, with debug information
$src = Join-Path $tmp 'prog\thr.c'
Copy-Item (Join-Path $here 'integration_dbg\thr.c') $src
$exe = Join-Path $tmp 'prog\thr.exe'
$oldPath = $env:PATH
$env:PATH = "$tc;$env:PATH"
& (Join-Path $tc 'gcc.exe') -g -O0 -static -o $exe $src
$env:PATH = $oldPath
if (-not (Test-Path $exe)) { Write-Host 'FATAL: the test program did not build'; exit 2 }
$settings = [ordered]@{
  'security.workspace.trust.enabled'  = $false
  'workbench.startupEditor'           = 'none'
  'telemetry.telemetryLevel'          = 'off'
  'update.mode'                       = 'none'
  'extensions.autoCheckUpdates'       = $false
  'extensions.autoUpdate'             = $false
  'task.allowAutomaticTasks'          = 'off'
  'debug.openDebug'                   = 'neverOpen'
}
[IO.File]::WriteAllText((Join-Path $tmp 'user\User\settings.json'), ($settings | ConvertTo-Json), (New-Object System.Text.UTF8Encoding($false)))
$report = Join-Path $tmp 'report.txt'
$env:ELECTRON_RUN_AS_NODE = $null
$env:HTD_IT_REPORT = $report
$env:HTD_DBG_EXE = $exe
$env:HTD_DBG_SRC = $src
$env:HTD_DBG_GDB = Join-Path $tc 'gdb.exe'
$a = @("--user-data-dir=`"$(Join-Path $tmp 'user')`"", "--extensions-dir=`"$(Join-Path $tmp 'ext')`"",
  "--extensionDevelopmentPath=`"$ext`"", "--extensionTestsPath=`"$(Join-Path $here 'integration_dbg')`"",
  '--new-window', '--skip-welcome', '--skip-release-notes', '--disable-workspace-trust')
$p = Start-Process -FilePath $code -ArgumentList $a -PassThru -NoNewWindow -RedirectStandardOutput (Join-Path $tmp 'stdout.txt') -RedirectStandardError (Join-Path $tmp 'stderr.txt')
if (-not $p.WaitForExit($TimeoutSec * 1000)) { Write-Host 'timeout: killing the test instance'; $p.Kill() }
foreach ($v in 'HTD_IT_REPORT', 'HTD_DBG_EXE', 'HTD_DBG_SRC', 'HTD_DBG_GDB') { Remove-Item "Env:$v" -ErrorAction SilentlyContinue }
if (-not (Test-Path $report)) { Write-Host "no report written (logs: $tmp)"; exit 2 }
$text = Get-Content $report -Raw -Encoding utf8
$text
Write-Host "(scratch: $tmp)"
$m = [regex]::Match($text, 'RESULT: (\d+) pass, (\d+) fail')
if (-not $m.Success -or [int]$m.Groups[2].Value -gt 0) { exit 1 }
exit 0
