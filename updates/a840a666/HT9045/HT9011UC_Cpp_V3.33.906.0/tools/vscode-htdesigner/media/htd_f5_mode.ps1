# F5 and toolbar use the same explicit build mode. Build only; no Handler start.
param([Parameter(Mandatory=$true)][string]$Tree,
      [Parameter(Mandatory=$true)][string]$Dir,
      [Parameter(Mandatory=$true)][string]$Baseline,
      [int]$Sim=1, [int]$Dbg=1, [string]$Query='', [switch]$PlanOnly, [string]$ResultFile='', [string]$RunId='')
$ErrorActionPreference='Stop'
# (1010 review of paths #3: g++ writes UTF-8; read back as code page 950 a Chinese path in htd_build.log was garbled and its
#  problems did not open. Not the port tree's build_with_status.ps1 -- its own output passes through this one)
try { [Console]::OutputEncoding = [Text.Encoding]::UTF8 } catch { }
$resultCode=1
try {
# (1009 review (windows #2): one build at a time in this folder -- another VS Code window's F5 / Make into it at the same
#  moment broke the folder (made-up undefined references, cut exes). Released when this script ends)
if(-not $PlanOnly){
  $__key=[IO.Path]::GetFullPath($Dir).TrimEnd('\','/').ToLowerInvariant()
  $__h=[BitConverter]::ToString([Security.Cryptography.SHA1]::Create().ComputeHash([Text.Encoding]::UTF8.GetBytes($__key))).Replace('-','').Substring(0,16)
  $__mx=New-Object System.Threading.Mutex($false, "Local\htd_build_$__h")
  $__own=$false
  try { $__own=$__mx.WaitOne(0) } catch [System.Threading.AbandonedMutexException] { $__own=$true }
  if(-not $__own){ Write-Output "[F5 mode] refused: another build is running in $Dir (another VS Code window?) -- nothing built"; $resultCode=5; exit 5 }
  # (1008 audit D6: last run's log goes first -- a configure that failed left it, and its old errors were listed again.
  #  1009 second review (build #6): only once the folder is ours -- another window's build had it open)
  Remove-Item -LiteralPath (Join-Path $Dir 'htd_build.log') -Force -ErrorAction SilentlyContinue
}
$bt=if($Dbg){'Debug'}else{'Release'}
$noSim=if($Sim){'OFF'}else{'ON'}
if([IO.Path]::GetFullPath($Dir).TrimEnd('\','/') -eq [IO.Path]::GetFullPath($Baseline).TrimEnd('\','/')){throw 'Mode build must not overwrite the selected baseline'}
$cache=Join-Path $Baseline 'CMakeCache.txt'
$seed=@{}
if(Test-Path -LiteralPath $cache){
  foreach($line in [IO.File]::ReadAllLines($cache)){
    if($line -match '^([^#/][^:]*):(STRING|BOOL|FILEPATH|PATH|UNINITIALIZED|INTERNAL)=(.*)$'){$seed[$matches[1]]=$matches[3]}
  }
};  if($seed['CMAKE_CXX_COMPILER']){ $tcBin=Split-Path ($seed['CMAKE_CXX_COMPILER'] -replace '/','\'); if(Test-Path -LiteralPath (Join-Path $tcBin 'g++.exe')){ $env:PATH="$tcBin;$env:PATH" } }   # (1008 machine, EastSun "F5 closes at once": the baseline's toolchain bin goes on PATH -- windres runs "gcc" to preprocess wb_serve.rc and stopped with "'gcc' is not recognized")
$a=@('-S',$Tree,'-B',$Dir)
# (1007 audit D7: no configured baseline = no known toolchain -- it fell back to Ninja + C:\MinGW, the oracle compiler)
if(-not(Test-Path -LiteralPath (Join-Path $Dir 'CMakeCache.txt')) -and -not(Test-Path -LiteralPath $cache)){
  # (1008, EastSun "我怎沒辦法編譯?": a fresh PC has no configured folder at all -- the WinLibs i686 g++ 16 lane every
  #  machine F5 task names (C++14 for std::byte vs w32api byte, -static for libwinpthread), when it is installed)
  $wl=Join-Path $env:LOCALAPPDATA 'Programs\ht9045-nonoracle-toolchain\mingw32\bin'
  if(-not(Test-Path -LiteralPath (Join-Path $wl 'g++.exe'))){
    Write-Output "[F5 mode] $Baseline has no CMakeCache.txt and $wl\g++.exe is not installed: build that configuration once with its own task first (its toolchain is taken from it)"
    $resultCode=2; exit 2   # (1009 review: the result file says 2 too -- it said 1)
  }
  $w=$wl -replace '\\','/'
  $seed=@{CMAKE_GENERATOR=$(if(Test-Path -LiteralPath (Join-Path $wl 'ninja.exe')){'Ninja'}else{'MinGW Makefiles'});
    CMAKE_CXX_COMPILER="$w/g++.exe";CMAKE_C_COMPILER="$w/gcc.exe";CMAKE_MAKE_PROGRAM=$(if(Test-Path -LiteralPath (Join-Path $wl 'ninja.exe')){"$w/ninja.exe"}else{"$w/mingw32-make.exe"});
    CMAKE_EXE_LINKER_FLAGS='-static';HT9045_CXX_STANDARD='14'}
  $env:PATH="$wl;$env:PATH"
  Write-Output "[F5 mode] $Baseline not configured yet: using the WinLibs g++ 16 toolchain (C++14, -static)"
}
if(-not(Test-Path -LiteralPath (Join-Path $Dir 'CMakeCache.txt'))){
  $gen=if($seed['CMAKE_GENERATOR']){$seed['CMAKE_GENERATOR']}else{'Ninja'}
  $a+=@('-G',$gen)
  foreach($k in $seed.Keys){
    if($k -match '^(CMAKE_(CXX_COMPILER|C_COMPILER|RC_COMPILER|MAKE_PROGRAM|EXE_LINKER_FLAGS|CXX_FLAGS|C_FLAGS)|HT9045_\w+|W906_\w+|WB_\w+|ADVMOT_\w+)$' -and $k -ne 'W906_NO_SOFT_SIMULTE'){$a+="-D$k=$($seed[$k])"}
  }
  if(-not $seed['CMAKE_CXX_COMPILER']){$a+=@('-DCMAKE_CXX_COMPILER=C:/MinGW/bin/g++.exe','-DCMAKE_C_COMPILER=C:/MinGW/bin/gcc.exe')}
}
$a+=@("-DCMAKE_BUILD_TYPE=$bt","-DW906_NO_SOFT_SIMULTE=$noSim",'-DBUILD_TESTING=OFF')
# (1007 audit D8: at most 6 at once, like the tree's own build tasks -- AI(W906-BOOTSPEED-2): more paged the whole PC)
$jobs=[Math]::Max(1,[Math]::Min(6,[Environment]::ProcessorCount-2))
# (1008 machine, EastSun: "detect this PC's memory and set the build up from it" -- the extension passes the -j it worked out from the free memory)
if($env:HTD_BUILD_JOBS -match '^\d+$' -and [int]$env:HTD_BUILD_JOBS -ge 1){ $jobs=[int]$env:HTD_BUILD_JOBS }
Write-Output "[F5 mode] $bt / $(if($Sim){'SIMULATION'}else{'SHIPPING'}) -> $Dir"
if($PlanOnly){
  @{configure=$a;build=@('--build',$Dir,'--target','wb_serve','-j',"$jobs");buildType=$bt;noSoftSimulate=$noSim;baseline=$Baseline;dir=$Dir;pathHead=(($env:PATH -split ';')[0])}|ConvertTo-Json -Depth 4 -Compress
  $resultCode=0
  exit 0
}
# (AI(W906-HTDESIGNER) 20261007, EastSun "編譯出錯的code 下面條列 可以點選跳過去": every line also goes to
#  <Dir>\htd_build.log -- the extension reads the compiler's errors from it into the 問題 panel, each one a link)
# (1008 full test, audit E2: the configure's lines too -- a CMake error left the log missing and 問題 empty)
# (1008 review: no cmake on this PATH -- "& cmake" threw, and this script still ended with exit code 0: the build counted as
#  done and the old exe was started. Said, and a failure)
if(-not (Get-Command cmake -ErrorAction SilentlyContinue)){ Write-Output '[F5 mode] cmake is not on PATH -- nothing built'; $resultCode=1; exit 1 }
if(-not (Test-Path -LiteralPath $Dir)){ New-Item -ItemType Directory -Force -Path $Dir | Out-Null }
$buildLog=Join-Path $Dir 'htd_build.log'
$sw=New-Object IO.StreamWriter($buildLog,$false,(New-Object System.Text.UTF8Encoding($false))); $sw.AutoFlush=$true   # (1009 review (build #6): the log read while a link runs long -- not 4 KB behind)
try {
  $eap=$ErrorActionPreference; $ErrorActionPreference='Continue'   # (CMake writes its errors on stderr: lines, not exceptions)
  & cmake @a 2>&1 | ForEach-Object { $l="$_"; if($l -eq 'System.Management.Automation.RemoteException'){ $l='' }; Write-Output $l; $sw.WriteLine($l) }   # (PS 5.1: a blank stderr line comes as that text)
  $cfgCode=$LASTEXITCODE
  $ErrorActionPreference=$eap
  if($cfgCode -ne 0){ $resultCode=$cfgCode; $sw.Flush(); exit $cfgCode }
  & powershell.exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $Tree 'tools/build_with_status.ps1') -BuildDir $Dir -Target wb_serve -Jobs $jobs 2>&1 | ForEach-Object { $l="$_"; Write-Output $l; $sw.WriteLine($l) }
} finally { $sw.Close() }
if($LASTEXITCODE -ne 0){$resultCode=$LASTEXITCODE;exit $LASTEXITCODE}
$text=[IO.File]::ReadAllText((Join-Path $Dir 'CMakeCache.txt'))
if($text -notmatch "(?m)^CMAKE_BUILD_TYPE:STRING=$bt\r?$" -or $text -notmatch "(?m)^W906_NO_SOFT_SIMULTE:BOOL=$noSim\r?$" -or $text -notmatch '(?m)^BUILD_TESTING:BOOL=OFF\r?$'){throw 'Built cache does not match the toolbar selection'}
$resultCode=0
Write-Output "[F5 mode] verified $bt / W906_NO_SOFT_SIMULTE=$noSim / BUILD_TESTING=OFF"
if($Query){
  # (1008 review: the waiting page is a courtesy -- a tree without web\boot_wait.html, or no browser found, made a good
  #  build read as failed and F5 started nothing without a word. Said as a warning; the build's result stays 0)
  & powershell.exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $Tree 'tools/open_boot_wait.ps1') -Query $Query
  if($LASTEXITCODE -ne 0){ Write-Output "[F5 mode] warning: the waiting page did not open (open_boot_wait exit $LASTEXITCODE); the build is fine" }
  exit 0
}

} finally {
  if($ResultFile -and $RunId){
    $result=@{runId=$RunId;code=$resultCode;buildType=$bt;noSoftSimulate=$noSim;dir=$Dir;finished=(Get-Date).ToString('o')}|ConvertTo-Json -Compress
    [IO.File]::WriteAllText($ResultFile,$result,(New-Object System.Text.UTF8Encoding($false)))
  }
}
