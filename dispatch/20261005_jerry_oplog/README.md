# 交接給 Jerry：機台 10-05 的 op log（W-15 kServeTickMs 量測參考）

> 來自：EastSun（機台端）　整理：機台端 Claude，2026-10-05 晚。EastSun：「幫忙把這個檔案用GIT交接給Jerry，看看是不是他要的」

| 檔 | 大小 | MD5 | 範圍 |
|---|---|---|---|
| `oplog_20261005.txt` | 5,360,558 B | `EFF74B83EA26FDFF488D907228901193` | 01:10:34 ～ 17:38:43（機台 runcfg\logs\，取自工作區搬 SSD 前的 HDD 備份 D:\HT9045\_integ_ioweb_HDD_backup） |

## 請 Jerry 注意：這份是「對照組」，不是 measure_tick.ps1 要的乾淨量測

1. **節拍是 500 ms**：整份都在機台套第 148 包（`kServeTickMs` 100 ms，機台 18:2x 套、cpp ced388a）**之前**。
2. **op log 開著**：你在 `tools/measure_tick.ps1` 寫的「op log 會讓 apiCache 灌水約 3 倍（W906_OpLogMotorRuntime 在計時區間內）」——這份就是開著錄的。
3. 內容：3,741 行 `[FASTCLK]`＋`[STREAM]` 10 秒統計；機台當天做過的事：多輪全機 HOME（含 MCCDY 被誤判逾時 14:23、按 START 先回原點再進空跑 14:25～14:28）、空跑、Motor Test／Teach JOG、警報框等待（03:24、14:24 主迴圈停 37～54 秒）、關程式。
4. 機台端對這份的統計（3,455 個區間，逐筆）：每圈平均忙碌 p50 29.1／p90 40.0／p99 253 ms；每 10 秒最慢一圈 p50 127／p90 391 ms、最大 85,247 ms；Heater 20 ms 那格只跑到應有次數的 42%（詳見 `dispatch/20261005_threading_perf`）。
5. 帳號／密碼類命令紀錄器本來就只寫「內容不記」（26 行）；推送前掃描 0 筆。

## 100 ms 的乾淨量測
機台已在 18:28 起用 100 ms（包 148／149）。`measure_tick.ps1` 預設 exe 路徑 `D:\HT9045\server\wb_serve.exe` 在機台上不存在，機台要用 `-Exe D:\HT9045\_integ_ioweb\HT9011UC_Cpp_V3.33.906.0\build_integ_ship_x86\wb_serve.exe`；它會自己啟動 wb_serve（機台端規矩：要 EastSun 同意／在場才跑），跑完的 .log 會再另外交給你。