<#
  measure_tick.ps1 -- AI(W906-TICK-MEASURE) 20261005 (Jerry)

  量 wb_serve 主迴圈的真實成本，給 kServeTickMs 的取值當依據（W-15／RULINGS_20261002 #19）。
  機台上要量的是「有 1203 卡時 Poll() 吃掉多少」—— 筆電量不到，因為筆電沒有卡。

  === 完全唯讀。不點任何按鈕、不碰輸出、不改任何設定檔。===

  用法（在機台的 PowerShell 裡）：
      cd D:\HT9045\HT9011UC_Cpp_V3.33.906.0
      .\tools\measure_tick.ps1                      # 預設量 180 秒
      .\tools\measure_tick.ps1 -Seconds 300 -Tag running   # 機台在跑的時候，標記一下

  做完會在 D:\HT9045_Log\tickmeasure\ 留一個 .log，把那個檔帶回來就好。

  ⚠ 為什麼不用 F5：
    1) [STREAM] 與 [FASTCLK] 只印到 stdout，VS Code 的 cppdbg 在這些機器上
       沒有把 debuggee 的 stdout 接到偵錯主控台 —— 看不到。
    2) .vscode/launch.json 有些設定自己寫死 W906_OPLOG_DIR，而 op log 開著會
       讓 apiCache 的數字灌水約 3 倍（wb_serve.cpp:6424 的 W906_OpLogMotorRuntime
       在 apiCache 的計時區間「內」，每秒 6 次 cJSON_Parse 整包 motor runtime）。
    本腳本自己起行程、自己轉向 stdout、並在這個行程裡清掉 W906_OPLOG_DIR。
#>
param(
  [int]    $Seconds = 180,
  [string] $Tag     = "",
  [string] $Exe     = "D:\HT9045\server\wb_serve.exe",
  [string] $OutDir  = "D:\HT9045_Log\tickmeasure"
)

$ErrorActionPreference = 'Stop'

# --- 0. 前置檢查 -----------------------------------------------------------
if (-not (Test-Path $Exe)) { throw "找不到 exe：$Exe   （機台的正式路徑；用別的請加 -Exe）" }

$running = Get-Process wb_serve -ErrorAction SilentlyContinue
if ($running) {
  Write-Host "[X] wb_serve 已經在跑（PID $($running.Id -join ',')）。" -ForegroundColor Red
  Write-Host "    請先把它關掉（正常關閉 HMI / 停掉 HT9045_Web.cmd），再跑這支。"
  Write-Host "    兩個一起跑會撞 8045 埠，量到的東西沒有意義。"
  exit 1
}

# --- 1. 這個行程裡關掉 op log（量測汙染的主因）-----------------------------
$env:W906_OPLOG_DIR = $null
Remove-Item Env:\W906_OPLOG_DIR -ErrorAction SilentlyContinue
if ($env:W906_STREAM_STATS -eq '0') {
  Remove-Item Env:\W906_STREAM_STATS -ErrorAction SilentlyContinue   # 0 會關掉 [STREAM]
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
$stamp = Get-Date -Format 'yyyyMMdd_HHmmss'
$name  = if ($Tag) { "tick_${stamp}_$Tag.log" } else { "tick_$stamp.log" }
$log   = Join-Path $OutDir $name

# --- 2. 記下量測當下的脈絡（之後看 log 才知道是什麼情況）-------------------
$head = @(
  "=== measure_tick.ps1  $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss') ===",
  "host      : $env:COMPUTERNAME",
  "exe       : $Exe",
  "exe built : $((Get-Item $Exe).LastWriteTime)",
  "exe size  : $((Get-Item $Exe).Length) bytes",
  "seconds   : $Seconds",
  "tag       : $(if ($Tag) { $Tag } else { '(none)' })",
  "W906_OPLOG_DIR : $(if ($env:W906_OPLOG_DIR) { $env:W906_OPLOG_DIR } else { '(unset -- good)' })",
  "=== 以下是 wb_serve 的 stdout ==="
)
$head | Set-Content -Path $log -Encoding UTF8

Write-Host ""
Write-Host "量測開始，$Seconds 秒。" -ForegroundColor Cyan
Write-Host "  log -> $log"
Write-Host "  期間請讓機台維持你要量的狀態（閒置就閒置、在跑就讓它跑），不要點 IO 按鈕。"
Write-Host ""

# --- 3. 跑 ------------------------------------------------------------------
$p = Start-Process -FilePath $Exe -WorkingDirectory (Split-Path $Exe -Parent) `
                   -RedirectStandardOutput "$log.raw" -RedirectStandardError "$log.err" `
                   -NoNewWindow -PassThru

for ($i = $Seconds; $i -gt 0; $i -= 10) {
  Start-Sleep -Seconds ([Math]::Min(10, $i))
  if ($p.HasExited) { Write-Host "[X] wb_serve 自己結束了（exit $($p.ExitCode)）—— 看 log 找原因。" -ForegroundColor Red; break }
  Write-Host ("  ... 剩 {0} 秒" -f [Math]::Max(0, $i - 10))
}

if (-not $p.HasExited) {
  Write-Host "時間到，關閉 wb_serve ..." -ForegroundColor Cyan
  try { $p.CloseMainWindow() | Out-Null } catch {}
  Start-Sleep -Seconds 3
  if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force; Start-Sleep -Seconds 2 }
}

Get-Content "$log.raw" -ErrorAction SilentlyContinue | Add-Content -Path $log -Encoding UTF8
if ((Test-Path "$log.err") -and (Get-Item "$log.err").Length -gt 0) {
  "=== stderr ===" | Add-Content -Path $log -Encoding UTF8
  Get-Content "$log.err" | Add-Content -Path $log -Encoding UTF8
}
Remove-Item "$log.raw","$log.err" -ErrorAction SilentlyContinue

# --- 4. 當場摘要（確認有量到東西，不用等帶回去才發現是空的）----------------
$txt      = Get-Content $log
$stream   = @($txt | Where-Object { $_ -like '`[STREAM`]*'  })
$fastclk  = @($txt | Where-Object { $_ -like '`[FASTCLK`]*' })
$oplogOff = @($txt | Where-Object { $_ -like 'oplog: off*'  }).Count -gt 0

Write-Host ""
Write-Host "=== 收穫 ===" -ForegroundColor Green
Write-Host ("  [STREAM]  行數 : {0}   （10 秒一行，{1} 秒應該有約 {2} 行）" -f $stream.Count, $Seconds, [int]($Seconds/10))
Write-Host ("  [FASTCLK] 行數 : {0}" -f $fastclk.Count)
Write-Host ("  op log 關閉確認: {0}" -f $(if ($oplogOff) { 'YES（數字乾淨）' } else { 'NO -- 看 log 裡 oplog 那一行，數字可能灌水' }))
if ($fastclk.Count) { Write-Host ""; Write-Host "  最後一行 [FASTCLK]："; Write-Host ("    " + $fastclk[-1]) -ForegroundColor Gray }
if ($stream.Count)  { Write-Host ""; Write-Host "  最後一行 [STREAM]：";  Write-Host ("    " + $stream[-1])  -ForegroundColor Gray }
Write-Host ""
Write-Host "把這個檔帶回去：$log" -ForegroundColor Yellow
Write-Host ""
Write-Host "要看的重點："
Write-Host "  [FASTCLK] 的 busy max 與 >=100ms  -> 含 1203 Poll() 的那幾圈有多長（筆電量不到）"
Write-Host "  [FASTCLK] 各 job 的 maxLate       -> Poll 擋住別人多久"
Write-Host "  [STREAM]  apiCache / publish      -> 餵網頁的成本（跟開著哪些頁面有關）"
