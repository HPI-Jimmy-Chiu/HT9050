# 給 Jimmy：機台 IO 執行緒與節拍（回 RULINGS_20261006 #127／#128）——20261006 機台端

EastSun 1006 看過 #127＝B、#128＝A 之後的裁決：
- **#128（IO 執行緒先關）→ 機台不關，機台端當天修完審查 Part 1 的 #1～#6**（「我現在修 #1～#6，修好繼續開」）。
- **#127（機台節拍改 500）→ 維持 100，把今天的數據回報 Jimmy**（「維持 100，把今天數據回報 Jimmy」）。

## 1. 為什麼節拍 100 可以（機台 10-06 實測）

量法：照審查 Q4 的建議——同一顆 release exe、不開 HMI 網址、什麼都不按（待機）、每組 45 秒、sys.ping 60 次；數字取 oplog 的 [FASTCLK]／[STREAM] 逐 10 秒統計，每組去掉開機前 20 秒取中位數。

| 組態 | 主迴圈每圈平均 | 最慢一圈 | ≥20 ms 的圈／10 s | 溫控 20 ms 漏跑／10 s | 溫控最晚 | 快取重建 | ping p90 | 程式 CPU |
|---|---|---|---|---|---|---|---|---|
| 單執行緒（IO 執行緒 OFF） | 33 ms | 141 ms | 100 | 272 | 140 ms | 18.3 ms×10/s | 77 ms（最慢 1,231） | 77% |
| IO 執行緒 10 ms 間隔 | 5.7 ms | 31.5 ms | 100 | 53 | 39 ms | 18.3 ms×10/s | 12.6 ms | 126% |
| IO 執行緒 30 ms（cpp 0230 IOTUNE） | 5.7 ms | 33～43 ms | 100 | 52～56 | 38～43 ms | 18.4 ms×10/s | 6.6 ms | 110% |
| **IO 執行緒 30 ms＋IOTUNE-D（cpp 0232，現行）** | **2.6 ms** | **24.7 ms** | **1** | **7** | **27.7 ms** | **5.5 ms** | 9.8 ms | 96% |
| 單執行緒＋IOTUNE-D | 19.1 ms | 125 ms | 50 | 230 | 119 ms | 5.3 ms | 84 ms（最慢 1,207） | 67% |

- 專案自己的規則（measure_tick.ps1：tick ≥ 3 × 最壞單趟）：**現行最壞單趟約 25 ms ⇒ ≥ 75 ms，100 ms 成立**；單執行緒最壞約 125～141 ms ⇒ ≥ 390 ms，才需要 500。
- 審查 Q4 指出的根本原因（每一拍都重建 148 KB io-runtime JSON）已在 IOTUNE-D 處理：io-runtime 只在最近 3 秒有網頁讀過才重建（10-06 待機時 http io=0 讀取）；馬達那份照舊（oplog MOT 行要用）。
- 還沒量：**負載下**（HOME、空跑、Motor Test、IO／Motion 頁開著）。EastSun F5 跑這些時，機台端會從 oplog 算同一張表補上。
- 原始資料：機台 oplog_20261006.txt（08:30～09:10 的八組，IOTHR 行標出每組設定）、D:\HT9045\_iothread_1006\measure_run1～4.txt；WORKLOG 第 129 列。

## 2. 審查 Part 1 #1～#6 的修正（cpp IOFIX，計畫 D:\HT9045\_iothread_1006\PLAN_IOFIX.md）

| # | 修法 |
|---|---|
| 1 | Poll() 的「卡沒開／停用」提前 return 在 IO 執行緒上也發布；Rescan() 結尾立刻發布 ⇒ 重掃失敗時主執行緒看到 open=false，LinkWatch 第 3 種情況照常 10 秒跳 WAR16152 |
| 2 | RunSync 500 ms 分段等＋心跳：還在佇列、心跳 5 秒沒前進 ⇒ 撤回、命令失敗（printf）；已被接手 ⇒ 繼續等並每 5 秒記一行。CreateEvent 失敗直接拒絕（消掉 use-after-return 競態） |
| 3 | 發布帶時間；主執行緒 Adopt 時超過 3 秒沒新發布 ⇒ 已發布那份標 card().open=false＋lastErrorText 寫原因 ⇒ WAR16152；恢復發布整份覆蓋。nextIo 漂移已在 IOTUNE 修掉（只有 IO 時鐘到期才前進） |
| 4 | IO 執行緒結束前自己清 PubReady／running／tid（RunSync 的「執行緒已結束」路徑不再遞迴）；Stop 逾時 ⇒ 棄置：代理一律拒絕、Pci1203MonitorDisable／ControlDisable 在執行緒還活著時不 Close／delete |
| 5 | kCmdCardRescan 的代理回來後主執行緒立刻 Adopt（Rescan 已在 IO 執行緒發布新對照表）⇒ 下一個 DO 用新對照表 |
| 6 | IndexZTorqueCore armPoll ← Pci1203IoFreshMark |
| 低 | Q44 pollsSinceIssue 不繞回；GaliRouteCore 外來停止視窗在 IO 執行緒時不設下限；RouteSiblingHoming 不繞回 |

- 驗證：o2 全建＋完整 ctest（單執行緒路徑行為不變）。**IO 執行緒路徑沒有卡的 ctest 起不來**（Start 要監看器開著），上機驗由 EastSun：1203 頁拔一顆驅動器電源後按 Refresh 看 WAR16152、正常 Refresh 後按 DO、EXIT。
- W-90 的審查結論仍是 Jerry 的（#126＝A），這份給他參考。

## 3. 同一天機台的其他相關改動（給 main 參考）
- **INDEX_MOTION_CARD 0→1（EastSun 1006，Q130）**：HT9050 Index Z1 改走 1203 類別，所有 TMyMotor::Gali_* 依類別分流（cpp IDX1203，WORKLOG 第 130 列、§1）。推翻 0929 D1。
