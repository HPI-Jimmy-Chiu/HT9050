param(
    [Parameter(Mandatory=$true)]
    [string]$TargetPath,

    [string]$BprFile = 'HT9045.bpr',

    [string]$LogPath = ''
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = 'Stop'

$BCB = 'D:\ProgramFiles\Borland\CBuilder6'
$selfPid = $PID
$psBefore = @(Get-Process powershell -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Id)

if (-not $LogPath) {
    $LogPath = Join-Path $TargetPath 'build.log'
}

if (-not (Test-Path $TargetPath)) {
    Write-Error "找不到目標路徑: $TargetPath"
    exit 1
}

if (-not (Test-Path (Join-Path $TargetPath $BprFile))) {
    Write-Error "找不到專案檔: $(Join-Path $TargetPath $BprFile)"
    exit 1
}

$MakFile = [System.IO.Path]::ChangeExtension($BprFile, '.mak')

$exitCode = 1

try {
    $env:BCB = $BCB
    $env:PATH = "$BCB\Bin;" + $env:PATH

    Push-Location $TargetPath

    & bpr2mak $BprFile
    if ($LASTEXITCODE -ne 0) {
        throw "bpr2mak failed: $LASTEXITCODE"
    }

    cmd /c "make -f $MakFile" 2>&1 | Tee-Object -FilePath $LogPath
    $exitCode = $LASTEXITCODE
}
finally {
    Pop-Location -ErrorAction SilentlyContinue

    $psAfter = @(Get-Process powershell -ErrorAction SilentlyContinue)
    $newPs = $psAfter | Where-Object {
        ($psBefore -notcontains $_.Id) -and
        ($_.Id -ne $selfPid) -and
        ($_.MainWindowHandle -ne 0)
    }

    if ($newPs) {
        $newPs | Stop-Process -Force -ErrorAction SilentlyContinue
        Write-Host "[ht9045-code-merge] 已清理 PowerShell 視窗: $($newPs.Count) 個"
    }
}

Write-Host "[ht9045-code-merge] build exit code: $exitCode"
exit $exitCode
