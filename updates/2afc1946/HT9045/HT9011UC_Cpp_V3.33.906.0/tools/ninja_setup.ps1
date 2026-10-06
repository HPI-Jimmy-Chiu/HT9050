# =============================================================================
#  tools/ninja_setup.ps1 -- put Ninja on this PC and on its CMake build dirs.
#
#  AI(W906-NINJA-ROLLOUT) 20261006 (NB2-2b A16; user in the NB2 chat 1006 13:4x: "deploy Ninja, then roll it out to
#  the colleagues; if feasible just do it" and "the VS Code F5 I use must use it too, the machine as well").
#
#  WHY: build.bat already picks Ninja when ninja.exe is found (AI(W906-FASTBUILD) 20260921), and NB2-1 measured it
#  (.claude/skills/cpp_build/references/speedup-ideas.md, 20261004): one changed .cpp 27-28 s -> 8-10 s, a fresh
#  build 389-391 s -> 301-315 s. But CMake fixes the generator at a build dir's FIRST configure, so a dir created
#  with "MinGW Makefiles" stays on make forever -- and the machine's two F5 dirs (build_integ_ship_x86 and _o2, made
#  by hand, dispatch 20261003_build_speed) are exactly that. Installing ninja.exe alone changes nothing for them.
#  The generator only decides WHO runs the compiler in WHICH order: the compiler, its flags and the objects are the
#  same, so the x87 / oracle rules are not touched.
#
#  WHAT IT DOES (nothing is deleted unless you say -DeleteOld):
#    (no switch)        report: ninja.exe found or not, and every CMake build dir under -Root with its generator
#    -Install           if ninja.exe is missing: winget install --id Ninja-build.Ninja --scope user (no admin;
#                       winget may ask you to accept its source agreement -- that question is yours to answer)
#    -Switch <dir,...>  re-create a "MinGW Makefiles" build dir as Ninja with the SAME settings: reads its
#                       CMakeCache.txt, renames the dir to <dir>.makefiles-<stamp>, configures a new one with the
#                       same cmake.exe, compilers, build type, flags and every W906_* / HT9045_* option, then compares
#                       the new cache against the old one value by value. Refuses while a build / ctest / wb_serve
#                       of that dir is running. The first build in the new dir is a full one.
#    -Build <target>    after -Switch, build that target (e.g. wb_serve) with -Jobs (default 6: the machine has 7.7 GB)
#    -DeleteOld         after a clean -Switch (and -Build, if given), delete the renamed old dir
#
#  EXAMPLES
#    powershell -NoProfile -ExecutionPolicy Bypass -File tools\ninja_setup.ps1
#    powershell -NoProfile -ExecutionPolicy Bypass -File tools\ninja_setup.ps1 -Install
#    powershell -NoProfile -ExecutionPolicy Bypass -File tools\ninja_setup.ps1 -Switch build_integ_ship_x86 -Build wb_serve
#
#  ROLLBACK of a -Switch: delete the new dir, rename <dir>.makefiles-<stamp> back to <dir>. Printed after every switch.
#  Exit codes: 0 ok; 2 usage / ninja missing; 3 a switch was refused or failed (details printed); 4 a build failed.
# =============================================================================
param(
    [switch]$Install,
    [string[]]$Switch,
    [string]$Build = '',
    [int]$Jobs = 6,
    [switch]$DeleteOld,
    [string[]]$Root,
    [int]$Days = 30,          # the report lists build dirs whose CMakeCache.txt changed within this many days ...
    [switch]$All              # ... or every one (old campaign dirs are many and unused)
)

$ErrorActionPreference = 'Stop'
#  powershell -File passes "a,b" as ONE string (no array parsing) -- accept comma / semicolon lists either way.
$Root   = @($Root   | ForEach-Object { $_ -split '[,;]' } | Where-Object { $_ -and $_.Trim() } | ForEach-Object { $_.Trim() })
$Switch = @($Switch | ForEach-Object { $_ -split '[,;]' } | Where-Object { $_ -and $_.Trim() } | ForEach-Object { $_.Trim() })
$tree = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path            # HT9011UC_Cpp_V3.33.906.0
if (-not $Root -or $Root.Count -eq 0) {
    $Root = @($tree)                                                   # in-tree dirs (the machine's build_integ_*)
    $objRoot = Join-Path (Split-Path $tree -Parent) 'Obj\V906'         # build.bat's dirs (AI(W906-OBJROOT))
    if ($env:V906_OBJ_ROOT) { $objRoot = $env:V906_OBJ_ROOT }
    if (Test-Path $objRoot) { $Root += (Resolve-Path $objRoot).Path }
}

function Find-Ninja([string]$besideCompiler = '') {
    #  1. the ninja that ships with the dir's own toolchain (WinLibs 16.2.0: mingw32\bin\ninja.exe) -- the machine has it
    if ($besideCompiler -and (Test-Path $besideCompiler)) {
        $p = Join-Path (Split-Path $besideCompiler -Parent) 'ninja.exe'
        if (Test-Path $p) { return $p }
    }
    $c = Get-Command ninja.exe -ErrorAction SilentlyContinue
    if ($c) { return $c.Source }
    $cands = @()
    $cands += Join-Path $env:LOCALAPPDATA 'Programs\ht9045-nonoracle-toolchain\mingw32\bin\ninja.exe'   # build_nonoracle.bat's default toolchain
    $cands += Get-ChildItem -Path (Join-Path $env:LOCALAPPDATA 'Microsoft\WinGet\Packages') -Directory -Filter 'Ninja-build.Ninja_*' -ErrorAction SilentlyContinue |
              ForEach-Object { Join-Path $_.FullName 'ninja.exe' }
    $cands += Join-Path $env:LOCALAPPDATA 'Microsoft\WinGet\Links\ninja.exe'
    foreach ($p in $cands) { if (Test-Path $p) { return $p } }
    return $null
}

function Read-Cache([string]$dir) {
    $h = @{}
    foreach ($line in Get-Content -LiteralPath (Join-Path $dir 'CMakeCache.txt')) {
        if ($line -match '^([A-Za-z0-9_\-\.]+):([A-Z]+)=(.*)$') {
            $h[$Matches[1]] = [pscustomobject]@{ Type = $Matches[2]; Value = $Matches[3] }
        }
    }
    return $h
}

function Get-BuildDirs {
    $out = @()
    foreach ($r in $Root) {
        if (-not (Test-Path $r)) { continue }
        foreach ($d in Get-ChildItem -LiteralPath $r -Directory -ErrorAction SilentlyContinue) {
            if ($d.Name -like '*.makefiles-*') { continue }
            $cc = Join-Path $d.FullName 'CMakeCache.txt'
            if (Test-Path $cc) { $out += $d.FullName }
        }
    }
    return $out
}

function Resolve-BuildDir([string]$name) {
    if (Test-Path (Join-Path $name 'CMakeCache.txt')) { return (Resolve-Path $name).Path }
    foreach ($r in $Root) {
        $p = Join-Path $r $name
        if (Test-Path (Join-Path $p 'CMakeCache.txt')) { return (Resolve-Path $p).Path }
    }
    return $null
}

#  The settings that define a build dir. Everything else in the cache is derived by the configure itself.
$Fixed = @('CMAKE_BUILD_TYPE','CMAKE_C_COMPILER','CMAKE_CXX_COMPILER','CMAKE_C_FLAGS','CMAKE_CXX_FLAGS',
           'CMAKE_C_FLAGS_DEBUG','CMAKE_CXX_FLAGS_DEBUG','CMAKE_C_FLAGS_RELEASE','CMAKE_CXX_FLAGS_RELEASE',
           'CMAKE_EXE_LINKER_FLAGS','CMAKE_SHARED_LINKER_FLAGS','CMAKE_STATIC_LINKER_FLAGS')
function Get-Settings($cache) {
    $s = [ordered]@{}
    foreach ($k in ($cache.Keys | Sort-Object)) {
        $e = $cache[$k]
        $own = ($k -match '^(W906|HT9045)_') -and ($e.Type -in @('BOOL','STRING','PATH','FILEPATH'))
        if (($Fixed -contains $k) -or $own) { $s[$k] = $e }
    }
    return $s
}

function Get-Busy([string]$dir) {
    $names = @('cmake.exe','ninja.exe','mingw32-make.exe','make.exe','cc1plus.exe','cc1.exe','ld.exe','collect2.exe','ctest.exe','wb_serve.exe')
    $hits = @()
    foreach ($p in Get-CimInstance Win32_Process) {
        if ($names -notcontains $p.Name.ToLower()) { continue }
        $cl = "$($p.CommandLine) $($p.ExecutablePath)"
        if ($cl.ToLower().Contains($dir.ToLower()) -or $cl.ToLower().Contains($dir.Replace('\','/').ToLower())) { $hits += "$($p.Name) pid $($p.ProcessId)" }
    }
    return $hits
}

# ---------------------------------------------------------------------------------------------------------------------
$ninja = Find-Ninja
if (-not $ninja -and $Install) {
    Write-Host '[ninja] ninja.exe not found -- installing with winget (user scope, no admin) ...'
    $ErrorActionPreference = 'Continue'   # PS 5.1: a native exe's stderr line would otherwise stop the script
    & winget install --id Ninja-build.Ninja -e --scope user --source winget
    $ErrorActionPreference = 'Stop'
    $ninja = Find-Ninja
}
if ($ninja) {
    $ErrorActionPreference = 'Continue'
    $ver = (& $ninja --version) 2>$null
    $ErrorActionPreference = 'Stop'
    Write-Host "[ninja] ninja.exe $ver : $ninja"
} else {
    Write-Host '[ninja] ninja.exe NOT FOUND (PATH, winget package dir, WinGet\Links). Run with -Install, or: winget install --id Ninja-build.Ninja --scope user'
}

Write-Host ''
Write-Host '[ninja] CMake build dirs (generator | make program):'
$dirs = Get-BuildDirs
if ($dirs.Count -eq 0) { Write-Host '    (none under: ' ($Root -join '; ') ')' }
$nNinja = 0; $nMake = 0; $nHidden = 0
foreach ($d in $dirs) {
    $c = Read-Cache $d
    $g0 = if ($c['CMAKE_GENERATOR']) { $c['CMAKE_GENERATOR'].Value } else { '?' }
    if ($g0 -eq 'Ninja') { $nNinja++ } else { $nMake++ }
    if (-not $All -and (Get-Item -LiteralPath (Join-Path $d 'CMakeCache.txt')).LastWriteTime -lt (Get-Date).AddDays(-$Days)) { $nHidden++; continue }
    $g = if ($c['CMAKE_GENERATOR']) { $c['CMAKE_GENERATOR'].Value } else { '?' }
    $m = if ($c['CMAKE_MAKE_PROGRAM']) { $c['CMAKE_MAKE_PROGRAM'].Value } else { '?' }
    $mark = if ($g -eq 'Ninja') { 'OK   ' } else { 'MAKE ' }
    Write-Host ("    {0} {1,-14} {2}" -f $mark, $g, $d)
    if ($g -eq 'Ninja' -and $m -ne '?' -and -not (Test-Path $m)) { Write-Host "          ! its ninja.exe is gone: $m" }
}
Write-Host ("    total {0}: {1} Ninja, {2} make{3}" -f $dirs.Count, $nNinja, $nMake, $(if ($nHidden -gt 0) { "; $nHidden not touched for $Days+ days not shown (-All lists them)" } else { '' }))

if (-not $Switch -or $Switch.Count -eq 0) { exit 0 }
#  (-Switch picks the ninja per dir: the one beside that dir's compiler first, see Find-Ninja.)

$rc = 0
foreach ($name in $Switch) {
    Write-Host ''
    $dir = Resolve-BuildDir $name
    if (-not $dir) { Write-Host "[switch] $name : no CMakeCache.txt found (looked in: $($Root -join '; '))"; $rc = 3; continue }
    $old = Read-Cache $dir
    $gen = $old['CMAKE_GENERATOR'].Value
    if ($gen -eq 'Ninja') { Write-Host "[switch] $dir : already Ninja, nothing to do"; continue }
    $busy = Get-Busy $dir
    if ($busy.Count -gt 0) { Write-Host "[switch] $dir : REFUSED, in use by: $($busy -join ', ') -- stop the build / F5 / wb_serve first"; $rc = 3; continue }
    $src = $old['CMAKE_HOME_DIRECTORY'].Value
    $cmake = $old['CMAKE_COMMAND'].Value
    if (-not (Test-Path $cmake)) { $cmake = 'cmake.exe' }
    $set = Get-Settings $old
    $dirNinja = Find-Ninja $(if ($set['CMAKE_CXX_COMPILER']) { $set['CMAKE_CXX_COMPILER'].Value } else { '' })
    if (-not $dirNinja) { Write-Host "[switch] $dir : REFUSED, no ninja.exe (not beside its compiler, not on PATH / winget) -- run with -Install first"; $rc = 3; continue }
    $defs = @()
    foreach ($k in $set.Keys) { $defs += ('-D{0}:{1}={2}' -f $k, $set[$k].Type, $set[$k].Value) }
    $stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    $bak = "$dir.makefiles-$stamp"
    Write-Host "[switch] $dir"
    Write-Host "         generator : $gen -> Ninja"
    Write-Host "         source    : $src"
    Write-Host "         cmake     : $cmake"
    Write-Host "         ninja     : $dirNinja"
    Write-Host "         settings  : $($set.Count) (build type '$($set['CMAKE_BUILD_TYPE'].Value)', compiler $($set['CMAKE_CXX_COMPILER'].Value))"
    try { Rename-Item -LiteralPath $dir -NewName (Split-Path $bak -Leaf) } catch { Write-Host "[switch] cannot rename the old dir (a file is held open?): $($_.Exception.Message)"; $rc = 3; continue }
    Write-Host "         old dir   : $bak"
    $cxx = $set['CMAKE_CXX_COMPILER'].Value
    $savedPath = $env:PATH
    if ($cxx -and (Test-Path $cxx)) { $env:PATH = (Split-Path $cxx -Parent) + ';' + $env:PATH }   # cc1plus finds its DLLs
    $log = "$dir.configure-ninja.log"
    $ErrorActionPreference = 'Continue'   # cmake warnings go to stderr
    & $cmake -S $src -B $dir -G Ninja ("-DCMAKE_MAKE_PROGRAM=" + $dirNinja.Replace('\','/')) @defs *> $log
    $cfgOk = ($LASTEXITCODE -eq 0)
    $ErrorActionPreference = 'Stop'
    $env:PATH = $savedPath
    if (-not $cfgOk) {
        Write-Host "[switch] configure FAILED -- see $log"
        Write-Host "         rollback: Remove-Item -Recurse -Force '$dir'; Rename-Item '$bak' '$(Split-Path $dir -Leaf)'"
        $rc = 3; continue
    }
    $new = Read-Cache $dir
    $diff = @()
    foreach ($k in $set.Keys) {
        $nv = if ($new[$k]) { $new[$k].Value } else { '<missing>' }
        if ($nv -ne $set[$k].Value) { $diff += "$k : '$($set[$k].Value)' -> '$nv'" }
    }
    $newSet = Get-Settings $new
    foreach ($k in $newSet.Keys) { if (-not $set.Contains($k)) { $diff += "$k : <not in old> -> '$($newSet[$k].Value)'" } }
    if ($new['CMAKE_GENERATOR'].Value -ne 'Ninja') { $diff += "CMAKE_GENERATOR is '$($new['CMAKE_GENERATOR'].Value)'" }
    if ($diff.Count -gt 0) {
        Write-Host "[switch] the new cache DIFFERS from the old one in $($diff.Count) setting(s):"
        $diff | ForEach-Object { Write-Host "           $_" }
        Write-Host "         rollback: Remove-Item -Recurse -Force '$dir'; Rename-Item '$bak' '$(Split-Path $dir -Leaf)'"
        $rc = 3; continue
    }
    Write-Host "[switch] OK: Ninja, all $($set.Count) settings identical to the old dir (log $log)"
    Write-Host "         rollback: Remove-Item -Recurse -Force '$dir'; Rename-Item '$bak' '$(Split-Path $dir -Leaf)'"
    if ($Build) {
        if ($cxx -and (Test-Path $cxx)) { $env:PATH = (Split-Path $cxx -Parent) + ';' + $env:PATH }
        Write-Host "[build] cmake --build $dir --target $Build -j $Jobs  (first build in a new dir = full build)"
        $ErrorActionPreference = 'Continue'
        & $cmake --build $dir --target $Build -j $Jobs
        $bOk = ($LASTEXITCODE -eq 0)
        $ErrorActionPreference = 'Stop'
        $env:PATH = $savedPath
        if (-not $bOk) { Write-Host "[build] FAILED (old dir kept: $bak)"; $rc = 4; continue }
        Write-Host '[build] OK'
    }
    if ($DeleteOld) { Remove-Item -LiteralPath $bak -Recurse -Force; Write-Host "[switch] old dir deleted: $bak" }
    else { Write-Host "         keep or delete the old dir yourself once the new one works: Remove-Item -Recurse -Force '$bak'" }
}
exit $rc
