param(
    [Parameter(Mandatory=$true)]
    [string]$ProjectDir,

    [Parameter(Mandatory=$true)]
    [string]$BprFile,

    [ValidateSet('','clean','rebuild')]
    [string]$Mode = ''
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = 'Stop'

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$batPath = Join-Path $scriptDir 'build_bcb.bat'

if (-not (Test-Path $batPath)) {
    Write-Error "找不到 build 腳本: $batPath"
    exit 1
}

$selfPid = $PID
$psBefore = @(Get-Process powershell -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Id)
$exitCode = 1

try {
    $argList = @("`"$ProjectDir`"", "`"$BprFile`"")
    if ($Mode) {
        $argList += $Mode
    }

    # 用 cmd /c 呼叫 bat，避免再開新 PowerShell 視窗
    $proc = Start-Process -FilePath 'cmd.exe' -ArgumentList @('/c', "`"$batPath`" $($argList -join ' ')") -Wait -PassThru -NoNewWindow
    $exitCode = $proc.ExitCode
}
catch {
    Write-Error $_
    $exitCode = 1
}
finally {
    # 清理本次流程期間新開的 PowerShell 視窗（成功/失敗都執行）
    $psAfter = @(Get-Process powershell -ErrorAction SilentlyContinue)
    $newPs = $psAfter | Where-Object {
        ($psBefore -notcontains $_.Id) -and
        ($_.Id -ne $selfPid) -and
        ($_.MainWindowHandle -ne 0)
    }

    if ($newPs) {
        $newPs | Stop-Process -Force -ErrorAction SilentlyContinue
        Write-Host "[BCB Build] 已清理 PowerShell 視窗: $($newPs.Count) 個"
    }
}

exit $exitCode
