$WT='D:\AI_TempFile\st02-gb-p1'; $BD='D:\AI_TempFile\st02-gb-p1-build'; $BS='D:\AI_TempFile\st02-gb-p1-build-ship'; $LOG='D:\AI_TempFile\st02-__COMMIT__-build.log'
function W($m) { $m | Out-File $LOG -Append -Encoding ascii }
"=== start $(Get-Date -Format 'HH:mm:ss')" | Out-File $LOG -Encoding ascii
git -C $WT fetch -q origin 2>&1 | Out-File $LOG -Append -Encoding ascii
git -C $WT checkout -f --detach __COMMIT__ 2>&1 | Out-File $LOG -Append -Encoding ascii
W "=== HEAD $(git -C $WT rev-parse --short HEAD)"
$env:PATH='C:\MinGW\bin;C:\CMake\bin;C:\Program Files\Git\cmd;C:\Program Files\nodejs;C:\Windows\System32;C:\Windows;C:\Windows\System32\WindowsPowerShell\v1.0'   # nodejs + WindowsPowerShell as in full_gate_template.ps1 (node / powershell based ctests are skipped or Not Run without them)
Set-Location "$WT\HT9011UC_Cpp_V3.33.906.0"
$T = @('wb_serve','test_testercomm_gpib','test_testercomm_handler','test_testercomm_ipc','test_p6_gpib_aux','test_testercomm_rs232','test_set_test_timeout_timer')
W "=== SIM configure $(Get-Date -Format 'HH:mm:ss')"
cmake -S . -B $BD -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=C:/MinGW/bin/g++.exe -DCMAKE_C_COMPILER=C:/MinGW/bin/gcc.exe 2>&1 | Select-String 'W906 config|CMake Error' | ForEach-Object { W $_.Line }
W "=== SIM configure exit $LASTEXITCODE; build $(Get-Date -Format 'HH:mm:ss')"
cmake --build $BD --target $T -j 8 -- -k 2>&1 | Out-File $LOG -Append -Encoding ascii
W "=== SIM build exit $LASTEXITCODE; $(Get-Date -Format 'HH:mm:ss')"
$HF = @('D:\HT9045\config\config.ini','D:\HT9045\system\Gerneral.ini','D:\HT9045\Error\English\JAM0000.dat')
function HS { ($HF | ForEach-Object { if (Test-Path $_) { "$_|$((Get-FileHash $_ -Algorithm SHA256).Hash)|$((Get-Item $_).LastWriteTimeUtc.Ticks)" } else { "$_|missing" } }) + (Get-ChildItem 'D:\HT9045_Log' -Recurse -File | ForEach-Object { "$($_.FullName)|$($_.Length)|$($_.LastWriteTimeUtc.Ticks)" }) }
$H0 = HS
W "=== SIM ctest $(Get-Date -Format 'HH:mm:ss')"
ctest --test-dir $BD -R '^(TesterComm_.*)$' --output-on-failure 2>&1 | Out-File $LOG -Append -Encoding ascii
$rc1 = $LASTEXITCODE
W "=== SHIP configure $(Get-Date -Format 'HH:mm:ss')"
cmake -S . -B $BS -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=C:/MinGW/bin/g++.exe -DCMAKE_C_COMPILER=C:/MinGW/bin/gcc.exe -DW906_NO_SOFT_SIMULTE=ON 2>&1 | Select-String 'W906 config|CMake Error' | ForEach-Object { W $_.Line }
W "=== SHIP configure exit $LASTEXITCODE; build $(Get-Date -Format 'HH:mm:ss')"
cmake --build $BS --target $T -j 8 -- -k 2>&1 | Out-File $LOG -Append -Encoding ascii
W "=== SHIP build exit $LASTEXITCODE; $(Get-Date -Format 'HH:mm:ss')"
W "=== SHIP ctest $(Get-Date -Format 'HH:mm:ss')"
ctest --test-dir $BS -R '^(TesterComm_.*)$' --output-on-failure 2>&1 | Out-File $LOG -Append -Encoding ascii
$rc2 = $LASTEXITCODE
$H1 = HS
$hd = Compare-Object $H0 $H1
W "=== real-file check (config.ini / Gerneral.ini / JAM0000.dat SHA256 + HT9045_Log listing) diff: $(@($hd).Count)"
$hd | ForEach-Object { W "$($_.SideIndicator) $($_.InputObject)" }
W "=== exe times"
foreach ($b in @($BD,$BS)) { Get-ChildItem $b -Recurse -Include wb_serve.exe,test_testercomm_gpib.exe,test_testercomm_handler.exe,test_testercomm_ipc.exe,test_p6_gpib_aux.exe,test_testercomm_rs232.exe,test_set_test_timeout_timer.exe | ForEach-Object { W "$($_.FullName) $($_.LastWriteTime.ToString('MM-dd HH:mm:ss'))" } }
W "=== ctest exit SIM=$rc1 SHIP=$rc2; done $(Get-Date -Format 'HH:mm:ss')"
