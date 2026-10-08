# F5 and toolbar use the same explicit build mode. Build only; no Handler start.
param([Parameter(Mandatory=$true)][string]$Tree,
      [Parameter(Mandatory=$true)][string]$Dir,
      [Parameter(Mandatory=$true)][string]$Baseline,
      [int]$Sim=1, [int]$Dbg=1, [string]$Query='', [switch]$PlanOnly, [string]$ResultFile='', [string]$RunId='')
$ErrorActionPreference='Stop'
$resultCode=1
# (1008 audit D6: last run's log goes first -- a configure that failed left it, and its old errors were listed again)
if(-not $PlanOnly){ Remove-Item -LiteralPath (Join-Path $Dir 'htd_build.log') -Force -ErrorAction SilentlyContinue }
try {
$bt=if($Dbg){'Debug'}else{'Release'}
$noSim=if($Sim){'OFF'}else{'ON'}
if([IO.Path]::GetFullPath($Dir).TrimEnd('\','/') -eq [IO.Path]::GetFullPath($Baseline).TrimEnd('\','/')){throw 'Mode build must not overwrite the selected baseline'}
$cache=Join-Path $Baseline 'CMakeCache.txt'
$seed=@{}
if(Test-Path -LiteralPath $cache){
  foreach($line in [IO.File]::ReadAllLines($cache)){
    if($line -match '^([^#/][^:]*):(STRING|BOOL|FILEPATH|PATH|UNINITIALIZED|INTERNAL)=(.*)$'){$seed[$matches[1]]=$matches[3]}
  }
}
$a=@('-S',$Tree,'-B',$Dir)
# (1007 audit D7: no configured baseline = no known toolchain -- it fell back to Ninja + C:\MinGW, the oracle compiler)
if(-not(Test-Path -LiteralPath (Join-Path $Dir 'CMakeCache.txt')) -and -not(Test-Path -LiteralPath $cache)){
  # (1008, EastSun "我怎沒辦法編譯?": a fresh PC has no configured folder at all -- the WinLibs i686 g++ 16 lane every
  #  machine F5 task names (C++14 for std::byte vs w32api byte, -static for libwinpthread), when it is installed)
  $wl=Join-Path $env:LOCALAPPDATA 'Programs\ht9045-nonoracle-toolchain\mingw32\bin'
  if(-not(Test-Path -LiteralPath (Join-Path $wl 'g++.exe'))){
    Write-Output "[F5 mode] $Baseline has no CMakeCache.txt and $wl\g++.exe is not installed: build that configuration once with its own task first (its toolchain is taken from it)"
    exit 2
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
    if($k -match '^(CMAKE_(CXX_COMPILER|C_COMPILER|MAKE_PROGRAM|EXE_LINKER_FLAGS|CXX_FLAGS|C_FLAGS)|HT9045_\w+|W906_\w+|WB_\w+|ADVMOT_\w+)$' -and $k -ne 'W906_NO_SOFT_SIMULTE'){$a+="-D$k=$($seed[$k])"}
  }
  if(-not $seed['CMAKE_CXX_COMPILER']){$a+=@('-DCMAKE_CXX_COMPILER=C:/MinGW/bin/g++.exe','-DCMAKE_C_COMPILER=C:/MinGW/bin/gcc.exe')}
}
$a+=@("-DCMAKE_BUILD_TYPE=$bt","-DW906_NO_SOFT_SIMULTE=$noSim",'-DBUILD_TESTING=OFF')
# (1007 audit D8: at most 6 at once, like the tree's own build tasks -- AI(W906-BOOTSPEED-2): more paged the whole PC)
$jobs=[Math]::Max(1,[Math]::Min(6,[Environment]::ProcessorCount-2))
Write-Output "[F5 mode] $bt / $(if($Sim){'SIMULATION'}else{'SHIPPING'}) -> $Dir"
if($PlanOnly){
  @{configure=$a;build=@('--build',$Dir,'--target','wb_serve','-j',"$jobs");buildType=$bt;noSoftSimulate=$noSim;baseline=$Baseline;dir=$Dir}|ConvertTo-Json -Depth 4 -Compress
  $resultCode=0
  exit 0
}
& cmake @a
if($LASTEXITCODE -ne 0){$resultCode=$LASTEXITCODE;exit $LASTEXITCODE}
# (AI(W906-HTDESIGNER) 20261007, EastSun "編譯出錯的code 下面條列 可以點選跳過去": every line also goes to
#  <Dir>\htd_build.log -- the extension reads the compiler's errors from it into the 問題 panel, each one a link)
$buildLog=Join-Path $Dir 'htd_build.log'
$sw=New-Object IO.StreamWriter($buildLog,$false,(New-Object System.Text.UTF8Encoding($false)))
try {
  & powershell.exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $Tree 'tools/build_with_status.ps1') -BuildDir $Dir -Target wb_serve -Jobs $jobs 2>&1 | ForEach-Object { $l="$_"; Write-Output $l; $sw.WriteLine($l) }
} finally { $sw.Close() }
if($LASTEXITCODE -ne 0){$resultCode=$LASTEXITCODE;exit $LASTEXITCODE}
$text=[IO.File]::ReadAllText((Join-Path $Dir 'CMakeCache.txt'))
if($text -notmatch "(?m)^CMAKE_BUILD_TYPE:STRING=$bt\r?$" -or $text -notmatch "(?m)^W906_NO_SOFT_SIMULTE:BOOL=$noSim\r?$" -or $text -notmatch '(?m)^BUILD_TESTING:BOOL=OFF\r?$'){throw 'Built cache does not match the toolbar selection'}
$resultCode=0
Write-Output "[F5 mode] verified $bt / W906_NO_SOFT_SIMULTE=$noSim / BUILD_TESTING=OFF"
if($Query){
  & powershell.exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $Tree 'tools/open_boot_wait.ps1') -Query $Query
  $resultCode=$LASTEXITCODE
  exit $LASTEXITCODE
}

} finally {
  if($ResultFile -and $RunId){
    $result=@{runId=$RunId;code=$resultCode;buildType=$bt;noSoftSimulate=$noSim;dir=$Dir;finished=(Get-Date).ToString('o')}|ConvertTo-Json -Compress
    [IO.File]::WriteAllText($ResultFile,$result,(New-Object System.Text.UTF8Encoding($false)))
  }
}
