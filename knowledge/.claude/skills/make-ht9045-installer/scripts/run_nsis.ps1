# run_nsis.ps1 — 建置 HT9045 NSIS Installer
# 用法：.\scripts\run_nsis.ps1 [-NsiScript <path>]

param(
    [string]$NsiScript = "D:\HT9045_Updater_NSIS\NSIS_Script\HT9045_MUI.nsi"
)

$makensis = "C:\Program Files (x86)\NSIS\makensis.exe"

if (-not (Test-Path $makensis)) {
    Write-Error "找不到 NSIS：$makensis`n請先安裝 NSIS 3.x"
    exit 1
}

if (-not (Test-Path $NsiScript)) {
    Write-Error "找不到 NSI 腳本：$NsiScript"
    exit 1
}

Write-Host ">>> 開始建置 NSIS Installer" -ForegroundColor Cyan
Write-Host "    NSI 腳本：$NsiScript"
Write-Host ""

& $makensis $NsiScript
$exitCode = $LASTEXITCODE

Write-Host ""
if ($exitCode -eq 0) {
    Write-Host ">>> 建置成功！" -ForegroundColor Green
    Write-Host "    輸出目錄：D:\HT9045_Updater_NSIS\"
    Write-Host ""

    # 取得最新產生的 Installer
    $latest = Get-ChildItem "D:\HT9045_Updater_NSIS\*Installer.exe" |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1

    if ($latest) {
        Write-Host "    最新輸出：$($latest.Name)" -ForegroundColor Yellow

        # 壓縮為 7z
        $sevenZip = "C:\Program Files\7-Zip\7z.exe"
        if (Test-Path $sevenZip) {
            Write-Host ""
            Write-Host ">>> 壓縮為 7z..." -ForegroundColor Cyan
            $archivePath = "$($latest.FullName).7z"
            & $sevenZip a $archivePath $latest.FullName
            if ($LASTEXITCODE -eq 0) {
                $sz = [math]::Round((Get-Item $archivePath).Length / 1MB, 1)
                Write-Host ">>> 壓縮完成：$([System.IO.Path]::GetFileName($archivePath))  ($sz MB)" -ForegroundColor Green
            } else {
                Write-Warning "7z 壓縮失敗（exit code: $LASTEXITCODE）"
            }
        } else {
            Write-Warning "找不到 7-Zip：$sevenZip，跳過壓縮"
        }
    }
} else {
    Write-Host ">>> 建置失敗！(exit code: $exitCode)" -ForegroundColor Red
    exit $exitCode
}
