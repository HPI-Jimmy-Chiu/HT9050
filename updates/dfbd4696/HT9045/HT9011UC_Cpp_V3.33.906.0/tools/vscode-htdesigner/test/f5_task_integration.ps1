# Isolated VS Code task API verification. No Handler or hardware is started.
param([string]$CodePath='', [switch]$CppTools)
$ErrorActionPreference='Stop'
$caseDir=Join-Path $env:TEMP ('htd_f5_task_'+[guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Force "$caseDir/user/User","$caseDir/ext","$caseDir/workspace","$caseDir/harness" | Out-Null
$utf8=New-Object System.Text.UTF8Encoding($false)
[IO.File]::WriteAllText("$caseDir/harness/package.json",'{"name":"f5-task-verification","publisher":"local","version":"1.0.0","engines":{"vscode":"^1.85.0"},"main":"index.js","activationEvents":["*"],"contributes":{"debuggers":[{"type":"cppdbg","label":"Isolated F5 verification","configurationAttributes":{"launch":{"properties":{}}}}]}}',$utf8)
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'f5_task_adapter.js') -Destination "$caseDir/harness/index.js"
[IO.File]::WriteAllText("$caseDir/user/User/settings.json",'{"security.workspace.trust.enabled":false,"workbench.startupEditor":"none","telemetry.telemetryLevel":"off","update.mode":"none"}',$utf8)
if($CppTools){
 [IO.File]::WriteAllText("$caseDir/harness/package.json",'{"name":"f5-task-verification","publisher":"local","version":"1.0.0","engines":{"vscode":"^1.85.0"},"main":"index.js","activationEvents":["*"]}',$utf8)
 [IO.File]::WriteAllText("$caseDir/harness/index.js",'exports.activate=()=>{};',$utf8)
 $cppExt=Get-ChildItem "$env:USERPROFILE/.vscode/extensions" -Directory -Filter 'ms-vscode.cpptools-*-win32-x64'|Sort-Object Name|Select-Object -Last 1
 New-Item -ItemType Junction -Path "$caseDir/ext/$($cppExt.Name)" -Target $cppExt.FullName | Out-Null
}
$testFile=if($CppTools){'f5_cpptools_integration.js'}else{'f5_task_integration.js'}
$oldReport=$env:HTD_IT_REPORT;$oldElectron=$env:ELECTRON_RUN_AS_NODE
try{
 $env:HTD_IT_REPORT="$caseDir/report.json";$env:ELECTRON_RUN_AS_NODE=$null
 $code=if($CodePath){$CodePath}else{Join-Path $env:LOCALAPPDATA 'Programs/Microsoft VS Code/Code.exe'}
 $argsList=@("--user-data-dir=`"$caseDir/user`"","--extensions-dir=`"$caseDir/ext`"","--extensionDevelopmentPath=`"$caseDir/harness`"","--extensionTestsPath=`"$PSScriptRoot/$testFile`"",'--disable-workspace-trust','--skip-welcome','--new-window','--disable-gpu','--verbose',"`"$caseDir/workspace`"")
 $proc=Start-Process $code -WindowStyle Hidden -ArgumentList $argsList -PassThru -RedirectStandardOutput "$caseDir/stdout.txt" -RedirectStandardError "$caseDir/stderr.txt"
 $end=(Get-Date).AddSeconds(240)
 while(!(Test-Path "$caseDir/report.json") -and (Get-Date) -lt $end){Start-Sleep -Milliseconds 300}
 if(!(Test-Path "$caseDir/report.json")){throw "No report: $caseDir"}
 $report=[IO.File]::ReadAllText("$caseDir/report.json");Write-Output $report
 if(!(($report|ConvertFrom-Json).pass)){exit 1}
}finally{$env:HTD_IT_REPORT=$oldReport;$env:ELECTRON_RUN_AS_NODE=$oldElectron}
