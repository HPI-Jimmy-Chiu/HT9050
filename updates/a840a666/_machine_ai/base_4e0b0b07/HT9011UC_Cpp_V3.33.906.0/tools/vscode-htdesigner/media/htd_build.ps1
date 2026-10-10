# AI(W906-HTDESIGNER) 20261001 (0.149): the green run button's build -- EastSun: "綠色箭頭 啟動軟體並編譯",
# "勾選開關 ... 是否使用模擬模式 (SOFT_SIMULTE 有被定義的時候) ... 是否是 debug 模式".
# One build dir per choice (the dirs the tree's own F5 entries use):
#   simulation + release = build_nonoracle        simulation + debug = build_dbg_nonoracle
#   shipping   + release = build_integ_ship_x86   shipping   + debug = build_integ_dbg_x86
# Simulation = SOFT_SIMULTE defined = CMake W906_NO_SOFT_SIMULTE=OFF (MachineType.h: #ifndef W906_NO_SOFT_SIMULTE);
# no source file is changed. A missing dir is configured with build_nonoracle.bat's flags (WinLibs i686 g++, C++14,
# -static); an existing dir configured the OTHER way is refused, never re-configured (it may be someone's baseline).
# build\ (the oracle lane) is never touched. ASCII only (Windows PowerShell 5.1 reads this file in the ANSI code page).
param(
  [Parameter(Mandatory = $true)][string]$Tree,
  [Parameter(Mandatory = $true)][string]$Dir,
  [int]$Sim = 1,
  [int]$Dbg = 1,
  [string]$Target = 'wb_serve',
  [int]$Jobs = 6
)
$ErrorActionPreference = 'Stop'
if ($Dir -match '^(build|build_dbg)$') { Write-Host "FATAL: $Dir is the oracle lane -- never built from here"; exit 2 }
$bin = Join-Path $env:LOCALAPPDATA 'Programs\ht9045-nonoracle-toolchain\mingw32\bin'
if (-not (Test-Path (Join-Path $bin 'g++.exe'))) { Write-Host "FATAL: the WinLibs toolchain is not there: $bin"; exit 2 }
$env:PATH = "$bin;$env:PATH"
$cmake = $null
foreach ($c in @((Join-Path $env:LOCALAPPDATA 'Programs\cmake-4.4.2-windows-x86_64\bin\cmake.exe'), 'C:\Program Files\CMake\bin\cmake.exe')) {
  if (Test-Path $c) { $cmake = $c; break }
}
if (-not $cmake) { $g = Get-Command cmake.exe -ErrorAction SilentlyContinue; if ($g) { $cmake = $g.Source } }
if (-not $cmake) { Write-Host 'FATAL: cmake.exe not found'; exit 2 }
$b = Join-Path $Tree $Dir
# (1009 review (windows #2): one build at a time in this folder -- the same lock as htd_f5_mode.ps1)
if ($true) {
  $__key=[IO.Path]::GetFullPath($b).TrimEnd('\','/').ToLowerInvariant()
  $__h=[BitConverter]::ToString([Security.Cryptography.SHA1]::Create().ComputeHash([Text.Encoding]::UTF8.GetBytes($__key))).Replace('-','').Substring(0,16)
  $__mx=New-Object System.Threading.Mutex($false, "Local\htd_build_$__h")
  $__own=$false
  try { $__own=$__mx.WaitOne(0) } catch [System.Threading.AbandonedMutexException] { $__own=$true }
  if(-not $__own){ Write-Host "FATAL: another build is running in $b (another VS Code window?) -- nothing built"; exit 5 }
}
$noSim = if ($Sim) { 'OFF' } else { 'ON' }
$cache = Join-Path $b 'CMakeCache.txt'
$what = $(if ($Sim) { 'SIMULATION (SOFT_SIMULTE defined)' } else { 'SHIPPING (SOFT_SIMULTE not defined)' }) + ' / ' + $(if ($Dbg) { 'DEBUG (-g)' } else { 'release' })
Write-Host "== $what -> $b"
if (Test-Path $cache) {
  $t = [IO.File]::ReadAllText($cache)
  $m = [regex]::Match($t, 'W906_NO_SOFT_SIMULTE:BOOL=(\w+)')
  $have = if ($m.Success) { $m.Groups[1].Value.ToUpper() } else { 'OFF' }
  # (CMake's true values: ON / TRUE / 1 / YES / Y)
  $haveOn = @('ON', 'TRUE', '1', 'YES', 'Y') -contains $have
  if ($haveOn -ne ($noSim -eq 'ON')) {
    Write-Host "FATAL: $Dir is configured with W906_NO_SOFT_SIMULTE=$have, this run wants $noSim -- refused (not re-configured)."
    exit 3
  }
  $bt = [regex]::Match($t, 'CMAKE_BUILD_TYPE:STRING=(\w*)')
  $isDbg = $bt.Success -and $bt.Groups[1].Value -ieq 'Debug'
  if ([bool]$Dbg -ne $isDbg) {
    Write-Host "FATAL: $Dir has CMAKE_BUILD_TYPE='$($bt.Groups[1].Value)', this run wants $(if ($Dbg) { 'Debug (-g)' } else { 'no -g' }) -- refused (not re-configured)."
    exit 3
  }
} else {
  $f = $bin -replace '\\', '/'
  $a = @('-S', $Tree, '-B', $b, '-G', 'MinGW Makefiles', "-DW906_NO_SOFT_SIMULTE=$noSim", '-DHT9045_CXX_STANDARD=14', '-DCMAKE_EXE_LINKER_FLAGS=-static',
    "-DCMAKE_CXX_COMPILER=$f/g++.exe", "-DCMAKE_C_COMPILER=$f/gcc.exe", "-DCMAKE_MAKE_PROGRAM=$f/mingw32-make.exe")
  if ($Dbg) { $a += '-DCMAKE_BUILD_TYPE=Debug' }
  Write-Host "== configure (first time): cmake $($a -join ' ')"
  & $cmake @a
  if ($LASTEXITCODE -ne 0) { Write-Host "FATAL: configure failed ($LASTEXITCODE)"; exit $LASTEXITCODE }
}
Write-Host "== build $Target"
& $cmake --build $b --target $Target -j $Jobs
$rc = $LASTEXITCODE
if ($rc -ne 0) { Write-Host "BUILD FAILED ($rc)"; exit $rc }
$exe = Join-Path $b ($Target + '.exe')
if (-not (Test-Path $exe)) { Write-Host "FATAL: $exe was not made"; exit 4 }
Write-Host "== OK $exe"
exit 0
