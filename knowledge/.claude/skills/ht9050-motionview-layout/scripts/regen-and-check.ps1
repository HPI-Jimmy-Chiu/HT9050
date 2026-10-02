# HT9050 Motion View：改完 JSON／JS 後的必跑檢查
#   1. 重跑 JSON → JS 墊片（file:// 下才讀得到新值）
#   2. node --check 所有本頁相關 .js
#   3. 列出 Machine-profile / layout 的 runtimeSupported 旗標（必須為 false）
# 用法：powershell -ExecutionPolicy Bypass -File regen-and-check.ps1

$ErrorActionPreference = 'Stop'
$root = 'D:\HT9045'
$py   = Join-Path $root '.venv\Scripts\python.exe'
$gen  = 'D:\AI_TempFile\_gen_json_shim.py'

Write-Host "== 1. regen JSON shim ==" -ForegroundColor Cyan
& $py $gen
if ($LASTEXITCODE -ne 0) { throw "_gen_json_shim.py failed ($LASTEXITCODE)" }

Write-Host "`n== 2. node --check ==" -ForegroundColor Cyan
$js = @(
  "$root\page\settings.js",
  "$root\page\theme.js",
  "$root\page\motionview-sim.js",
  "$root\JSON\js\Machine-profile.js",
  "$root\JSON\js\MotionView9050-layout.js"
) | Where-Object { Test-Path $_ }
foreach ($f in $js) {
  & node --check $f
  if ($LASTEXITCODE -ne 0) { throw "node --check failed: $f" }
  Write-Host "  ok  $f"
}

Write-Host "`n== 3. runtimeSupported 旗標 ==" -ForegroundColor Cyan
# 只有 HT9050 相關的旗標必須是 false；Machine-profile.json 內的 profiles.HT9045.runtimeSupported
# 本來就是 true（HT9045/9046 有 BCB6 分支），不可整份檔案掃 "true" 就報錯
$mp = Join-Path $root 'JSON\Machine-profile.json'
if (Test-Path $mp) {
  $o = Get-Content $mp -Raw -Encoding UTF8 | ConvertFrom-Json
  if ($o.source.runtimeSupported -ne $false) { throw "$mp source.runtimeSupported 必須為 false" }
  if ($o.profiles.HT9050.runtimeSupported -ne $false) { throw "$mp profiles.HT9050.runtimeSupported 必須為 false —— BCB6 無 Type_HT9050" }
  Write-Host "  ok  $mp  (source=false, HT9050=false, HT9045=$($o.profiles.HT9045.runtimeSupported))"
}
$lay = Join-Path $root 'JSON\MotionView9050-layout.json'
if (Test-Path $lay) {
  $o = Get-Content $lay -Raw -Encoding UTF8 | ConvertFrom-Json
  if ($o.source.runtimeSupported -ne $false) { throw "$lay source.runtimeSupported 必須為 false" }
  Write-Host "  ok  $lay  (source=false, axes.bindings=$($o.axes.bindings.Count), axes.absent=$($o.axes.absent.Count))"
}

Write-Host "`n== 4. 編碎檢查（UTF-8 無 BOM） ==" -ForegroundColor Cyan
foreach ($f in @("$root\page\IDE.MotionView9050-Concept.html", "$root\page\Main.MotionView9050.html", "$root\JSON\MotionView9050-layout.json")) {
  if (-not (Test-Path $f)) { continue }
  $b = [System.IO.File]::ReadAllBytes($f)
  if ($b.Length -ge 3 -and $b[0] -eq 0xEF -and $b[1] -eq 0xBB -and $b[2] -eq 0xBF) { throw "$f 含 UTF-8 BOM" }
  Write-Host "  ok  $f"
}

Write-Host "`n全部通過。接著開 file:///D:/HT9045/page/IDE.MotionView9050-Concept.html 並在 console 貼上 scripts/verify-concept.js" -ForegroundColor Green
