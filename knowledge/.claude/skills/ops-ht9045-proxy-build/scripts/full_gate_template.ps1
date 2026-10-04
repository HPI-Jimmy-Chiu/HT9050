$C='__COMMIT__'   # commit to gate (St01 branch or St02 head). Keep this file ASCII-only: Windows PowerShell 5.1 reads BOM-less UTF-8 as ANSI and a CJK comment can swallow the line break
$WT='D:\AI_TempFile\st02-gb-p1'; $BD='D:\AI_TempFile\st02-gb-p1-build'; $BS='D:\AI_TempFile\st02-gb-p1-build-ship'; $LOG="D:\AI_TempFile\st01-$C-gate.log"
function W($m) { $m | Out-File $LOG -Append -Encoding ascii }
$T0 = Get-Date
"=== start $($T0.ToString('HH:mm:ss'))" | Out-File $LOG -Encoding ascii
# Vanishing-exe hunt: log every *.exe delete in the build dirs while the gate runs (see the exe_watch.ps1 header)
$EW = 'D:\HT9045\.claude\skills\ops-ht9045-proxy-build\scripts\exe_watch.ps1'
if (Test-Path $EW) { Start-Process powershell -WindowStyle Hidden -ArgumentList @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $EW, '-GateLog', $LOG, '-Out', "$LOG.exewatch.txt") | Out-Null; W "=== exe watch -> $LOG.exewatch.txt" }
git -C $WT fetch -q origin 2>&1 | Out-File $LOG -Append -Encoding ascii
git -C $WT checkout -f --detach $C 2>&1 | Out-File $LOG -Append -Encoding ascii
W "=== HEAD $(git -C $WT rev-parse --short HEAD)"
$env:PATH='C:\MinGW\bin;C:\CMake\bin;C:\Program Files\Git\cmd;C:\Program Files\nodejs;C:\Windows\System32;C:\Windows;C:\Windows\System32\WindowsPowerShell\v1.0'   # WindowsPowerShell: PE_TruncationCheckTimeout (laptop a40b4873) runs plain 'powershell' -> 'Not Run / Unable to find executable' without it (ST01-M 20261004, gate 11f99613). nodejs: the node-based ctests (WB_WsLink, WB_F5Contract, D015_A01MenuPage ...) are only registered when CMake finds node (20260930: missing before, those tests silently did not run)
$env:HT9045_GOLDEN_ROOT='D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618'   # golden 0618 (RULINGS_20261003 #2), read by dfm2rc_fidelity / dfm2rc_idempotent / TeachButtonsGen (ST01-E E-032 20261003)
Set-Location "$WT\HT9011UC_Cpp_V3.33.906.0"
# Real-file check lists D:\HT9045\system and D:\HT9045\IniData too (20260930: cBinSel writes Bin Func keys, WebLogin_Reauth stamps login.dat / levelset.dat)
# 20261002: also D:\PrecautionRecord and D:\MajorMaintenanceRecord, folders included (TfObserver FormShow / LoadPrecautionMenu mkdirs a real D:\PrecautionRecord\system in ctests that open the Observer; E-021 writes maintenance records there)
$HF = @('D:\HT9045\config\config.ini','D:\HT9045\system\Gerneral.ini','D:\HT9045\system\ContactInfo.ini','D:\HT9045\system\levelset.dat','D:\HT9045\system\lastdata.dat','D:\HT9045\Error\English\JAM0000.dat')
function HS { ($HF | ForEach-Object { if (Test-Path $_) { "$_|$((Get-FileHash $_ -Algorithm SHA256).Hash)|$((Get-Item $_).LastWriteTimeUtc.Ticks)" } else { "$_|missing" } }) + (Get-ChildItem 'D:\HT9045_Log' -Recurse -File | ForEach-Object { "$($_.FullName)|$($_.Length)|$($_.LastWriteTimeUtc.Ticks)" }) + (Get-ChildItem 'D:\HT9045\system' -Recurse -File | ForEach-Object { "$($_.FullName)|$($_.Length)|$($_.LastWriteTimeUtc.Ticks)" }) + (Get-ChildItem 'D:\HT9045\IniData' -Recurse -File | ForEach-Object { "$($_.FullName)|$($_.Length)|$($_.LastWriteTimeUtc.Ticks)" }) + (@('D:\PrecautionRecord','D:\MajorMaintenanceRecord') | ForEach-Object { if (Test-Path $_) { "$_|dir"; Get-ChildItem $_ -Recurse -Force | ForEach-Object { "$($_.FullName)|$($_.Length)|$($_.LastWriteTimeUtc.Ticks)" } } else { "$_|missing" } }) }
function RealCheck($tag, $before) {
  $after = HS
  $hd = Compare-Object $before $after
  W "=== real-file check after $tag (config.ini / Gerneral.ini / ContactInfo.ini / levelset.dat / lastdata.dat / JAM0000.dat SHA256 + HT9045_Log + system + IniData + PrecautionRecord + MajorMaintenanceRecord listing) diff: $(@($hd).Count)"
  $hd | ForEach-Object { W "$($_.SideIndicator) $($_.InputObject)" }
  return @($hd).Count
}
function StaleExe($dir) {
  $old = Get-ChildItem $dir -Recurse -Filter *.exe | Where-Object { $_.FullName -notmatch 'CMakeFiles' -and $_.LastWriteTime -lt $T0 }
  W "=== stale exe in $dir (older than start): $(@($old).Count)"
  $old | ForEach-Object { W "STALE $($_.FullName) $($_.LastWriteTime.ToString('MM-dd HH:mm:ss'))" }
}
foreach ($cfg in @(@{n='SIM'; d=$BD; x=@()}, @{n='SHIP'; d=$BS; x=@('-DW906_NO_SOFT_SIMULTE=ON')})) {
  $n=$cfg.n; $d=$cfg.d
  W "=== $n configure $(Get-Date -Format 'HH:mm:ss')"
  $cargs = @('-S','.','-B',$d,'-G','MinGW Makefiles','-DCMAKE_CXX_COMPILER=C:/MinGW/bin/g++.exe','-DCMAKE_C_COMPILER=C:/MinGW/bin/gcc.exe') + $cfg.x
  & cmake @cargs 2>&1 | Select-String 'W906 config|CMake Error' | ForEach-Object { W $_.Line }
  W "=== $n configure exit $LASTEXITCODE; full build $(Get-Date -Format 'HH:mm:ss')"
  cmake --build $d -j 8 -- -k 2>&1 | Out-File "$LOG.$n.build.txt" -Encoding ascii
  $brc = $LASTEXITCODE
  $errs = @(Select-String -Path "$LOG.$n.build.txt" -Pattern ': error:|undefined reference|\*\*\* \[' ).Count
  W "=== $n build exit $brc; error-ish lines $errs; $(Get-Date -Format 'HH:mm:ss')"
  Select-String -Path "$LOG.$n.build.txt" -Pattern ': error:|undefined reference|\*\*\* \[' | Select-Object -First 60 | ForEach-Object { W "ERR $($_.Line)" }
  StaleExe $d
  $H0 = HS
  W "=== $n full ctest $(Get-Date -Format 'HH:mm:ss')"
  ctest --test-dir $d --output-on-failure --timeout 600 2>&1 | Out-File "$LOG.$n.ctest.txt" -Encoding ascii
  $crc = $LASTEXITCODE
  Select-String -Path "$LOG.$n.ctest.txt" -Pattern 'tests passed|tests failed|\*\*\*Failed|\*\*\*Timeout|\*\*\*Exception|Not Run|\(Failed\)|\(Timeout\)|\(SEGFAULT\)|\(Subprocess' | ForEach-Object { W "CT $($_.Line)" }
  W "=== $n ctest exit $crc; $(Get-Date -Format 'HH:mm:ss')"
  # A test exe can vanish between build and ctest (seen 20260929 twice on WB_WsProto, 20260930 on WB_Crypto SHIP; cause unknown; only old unrelinked exes).
  # Rebuild the target of every BAD_COMMAND / Not Run test and rerun that test alone, so a missing exe is not counted as a failure.
  $nr = Select-String -Path "$LOG.$n.ctest.txt" -Pattern '^\s+\d+ - (\S+) \((BAD_COMMAND|Not Run)\)' | ForEach-Object { $_.Matches[0].Groups[1].Value } | Sort-Object -Unique
  foreach ($tn in $nr) {
    $cmdl = (ctest --test-dir $d -N -V -R "^$tn$" 2>&1 | Select-String -Pattern 'Test command: (.+)$' | Select-Object -First 1)
    $exe = if ($cmdl) { ($cmdl.Matches[0].Groups[1].Value -split '\s+')[0] } else { '' }
    # ctest -N -V prints no command when the exe is missing (20260930 WB_Crypto SHIP), so read the path from CTestTestfile.cmake
    if (-not $exe) { $m = Get-ChildItem $d -Recurse -Filter CTestTestfile.cmake | Select-String -Pattern ('add_test\(' + [regex]::Escape($tn) + ' "([^"]+)"') | Select-Object -First 1; if ($m) { $exe = $m.Matches[0].Groups[1].Value } }
    $tg = [IO.Path]::GetFileNameWithoutExtension($exe)
    W "=== $n RERUN $tn (exe $exe, target $tg, exists $(Test-Path $exe), mtime before rebuild $(if ($exe -and (Test-Path $exe)) { (Get-Item $exe).LastWriteTime.ToString('MM-dd HH:mm:ss') } else { '-' }))"
    if ($tg) { cmake --build $d --target $tg -j 4 2>&1 | Out-File "$LOG.$n.rerun.$tg.txt" -Encoding ascii }
    ctest --test-dir $d -R "^$tn$" --output-on-failure --timeout 600 2>&1 | Out-File "$LOG.$n.rerun.$tn.ctest.txt" -Encoding ascii
    Select-String -Path "$LOG.$n.rerun.$tn.ctest.txt" -Pattern 'tests passed|Test +#' | ForEach-Object { W "CT RERUN $($_.Line)" }
  }
  $rd = RealCheck $n $H0
  if ($rd -gt 0) { W "=== STOP: real files changed during $n ctest"; break }
}
W "=== gate done $(Get-Date -Format 'HH:mm:ss')"
