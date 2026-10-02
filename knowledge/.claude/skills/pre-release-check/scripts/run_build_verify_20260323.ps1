$proj = 'D:\HT9045\HT9011UC_Code_V3.33.899.0_20260323'
$bcb = 'D:\ProgramFiles\Borland\CBuilder6'
$log = 'D:\HT9045\Merge Report\2026\BuildVerify_HT9045_20260323.log'

$env:PATH = "$bcb\Bin;" + $env:PATH

Push-Location $proj
bpr2mak HT9045.bpr | Out-Null
if ($LASTEXITCODE -ne 0)
{
    Write-Output "BUILD_EXIT_CODE=$LASTEXITCODE"
    Pop-Location
    exit $LASTEXITCODE
}

# ⚠ 注意：SKILL.md 建議使用 cmd /c 重導向以確保 ilink32 能建立 MAKE0000.@@@ 暫存檔。
# 以下 *> 為 PowerShell 重導向，在多數環境下可行，但若出現 MAKE0000.@@@ 錯誤請改用：
# cmd /c "make -f HT9045.mak -B > `"$log`" 2>&1"
make -f HT9045.mak -B *> $log
$code = $LASTEXITCODE
Pop-Location

Write-Output "BUILD_EXIT_CODE=$code"
exit $code
