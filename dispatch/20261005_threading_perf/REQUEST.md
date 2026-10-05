# 派工 2026-10-05：機台執行效率過慢 —— 評估 BCB 的執行緒用法，改架構（不要全部在一條執行緒跑）

> 給：Jimmy（筆電開發樹）　來自：EastSun（機台端，1203 層作者）　整理：機台端 Claude，2026-10-05 下午
> 機台樹：`D:\HT9045\_integ_ioweb\HT9011UC_Cpp_V3.33.906.0`，分支 `integ/ioweb-8484bdb4`，HEAD = `868be4e`；web `8bd78b0`。
> golden = `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618`（Big5，唯讀）。行號會漂，用之前 grep 名稱。

## 0. EastSun 原話

「請你派工給jimmy Eastsun 需要支援 執行效率過慢 請評估BCB 處理時執行續使用方式
機台不應該全部都在一個執行續跑 請更改架構」

---

## 1. 結論先講（機台端查證過的）

1. **golden 的機台邏輯其實也幾乎都在 UI 執行緒**（MainProc、Heater、PLC IO、Pad、Omron 都是 `Synchronize()` 回 UI 執行緒）——但它**快**，因為：
   - `TRunControl`（`uruncontrol.cpp:42-70`）：`timeBeginPeriod(1)`（:45），`Synchronize(MainProc)`（:49），1 ms 睡一下，RUN 時每 3 圈有 1 圈不睡；`tpTimeCritical`（`main.cpp:22476`）→ **MainProc 每 1～3 ms 一次**。
   - 每次 `Sen[].IsOn()`／讀位置都直接打卡（`MyLaneIo.cpp:399` `Acm_DaqDiGetBitEx`、`myEthercatmotor.cpp:1577` `Acm_AxGetActualPosition`），讀到的是當下值。
   - golden **真正平行的工作執行緒只有一條**：`HThreadCtrlShuttle`（`acarry.cpp:6941-7229`，`CreateThread` :6998，**沒有 Synchronize、沒有鎖**，1 ms 取樣 Out Shuttle 殘料感測器＋編碼器）。`TShuttleThread`（uShuttleThread.cpp）從沒被 new 過，是死碼。
2. **移植版（wb_serve）慢的原因**：
   - `MainProc` 在 `PumpTick` 裡，**每 500 ms 才跑一次**（`tools/wb_serve.cpp:2932` `kServeTickMs = 500`，B13 裁決，`docs/RULINGS_20260917.md`）＝比 golden 少 250～500 倍。
   - 引擎讀 IO／軸是讀 **200 ms 一次的 1203 監看快照**（`:2941` `kIoTickMs = 200`；DI 走 `EtherCAT/Pci1203IoRoute.cpp:51 FillDi`，軸走 `Motor/EcatMotorRoute.h:59-64`），下指令後還有 `pending` 要等下一次 Poll（`myEthercatmotor.cpp:1938`）→ **每個「動一下、等到位」的步驟至少吃一個 500 ms 節拍**。
   - `Pci1203Monitor()->Poll()` 是**同步、無逾時**（`Pci1203Monitor.h:1365`「Poll() BLOCKS」），每次約 90～130 ms、一秒 5 次，佔掉主執行緒大部分時間（DI 批次＋約 40 個站狀態＋每軸 5 個讀值＋條件式 SDO／屬性／扭力）。
   - **所有 `Acm_*` 都在主迴圈執行緒**（Poll、控制層、引擎 IO／馬達路線、軸 INI、link watch、模組檢查、VC8 SDO）。
   - 跳警報／對話框等待時，`PumpTick` 和 fast clock **都不跑**（`FastClockWbServe.cpp:20-26`）；`MySleep` 是真的 `::Sleep`（`common.cpp:2201-2204`），引擎裡任何 MySleep 都卡住全部。
   - golden 的 Heater 執行緒（20 ms）現在是主迴圈 fast clock 的一格（`FastClockWbServe.cpp:84`、`FastClockJobs.cpp:57-71`）→ 主迴圈一忙就漏跑。
   - ⚠ `docs/EVAL_THREADING_20261002.md` 有 4 處跟現況不符（golden 真正的平行執行緒是 HThreadCtrlShuttle 不是 TShuttleThread；golden 還有 HThreadCtrl socket 執行緒；移植版不只兩條執行緒——WebBridgeServer 有自己的 select 執行緒；1203 Poll 就是主要負載，不只是上限）。

## 2. 機台實測（附檔，全部逐筆統計，不是抽樣）

`oplog_20261005_FASTCLK_all.txt`＝今天 `runcfg\logs\oplog_20261005.txt` 全部 3,455 個 `[FASTCLK]` 10 秒區間（約 9.75 小時）：

| 指標 | p50 | p90 | p99 | 最大 |
|---|---|---|---|---|
| 主迴圈每圈平均忙碌（ms） | 29.1 | 40.0 | 253.3 | 20,512 |
| 每 10 秒內最慢一圈（ms） | 126.9 | 390.9 | 8,023 | **85,247** |
| 每秒圈數 | 21.2 | 45.6 | 47.9 | （最小 0） |

- **Heater 20 ms 那格：應跑約 175.6 萬次，實際 73.1 萬次（42%），漏 93.7 萬次。**
- 最慢一圈 ≥1 秒的區間 208 個、≥0.5 秒 293 個。最嚴重的停頓都對得上「主迴圈在等」：警報框等回答（03:24 WAR1602、14:24 MCCDY 回原點逾時框）、回原點等馬達電繼電器（14:25～14:26 第 1→2 步 25 秒）、關程式（13:58）、開 Teach 頁（13:12）、開機（09:00）。
- 安靜時（16:58 一個區間的細拆）：每秒約 22 圈、平均 28 ms、最慢 133 ms；≥100 ms 的圈每 10 秒 50 次＝剛好是 1203 Poll 的 5 次／秒；apiCache 6 次／秒×19.6 ms；tag publish 6 次／秒×3.85 ms（1808 個 tag）；主執行緒約 61% 忙碌。
- `oplog_20261005_1422-1428_MCCDY_home_START.txt`：14:22～14:28 原文（回原點、MCCDY 誤判逾時、按 START 後自動回原點再進空跑），可看到一輪 HOME 的節奏。

## 3. golden 執行緒／計時器全表（評估用）

| 項目 | 做什麼／週期 | 碰什麼 | 跟主執行緒同步 | 開關（HT9050） |
|---|---|---|---|---|
| TRunControl | MainProc，~1 ms | 全部 | Synchronize | 一定開 |
| THeaterThread（uHeaterThread.cpp:56-76） | CheckATC6System／DoThermo／HeaterDoorIsOpen／CheckHeater／DoHeaterOn，20 ms | 溫控 COM、SW[SwHeaterRelay]、門感測器、fHeaterOK、WAR15xx | Synchronize | 一定開 |
| **HThreadCtrlShuttle**（acarry.cpp:7026-7229） | Out Shuttle 殘料偵測，1 ms busy loop | Sen[] 直接讀、MOT[] 讀編碼器；寫 bShuttleHasIC／bEnter；從工作執行緒呼叫 fMain->AddShuttleMessage | **無**（真平行、無鎖） | main.cpp:10139 啟動；body 看 `SThreadPara.bExeShuttleThread`（cinitial.cpp:14479-14570） |
| ScanBtn（ScanBtnThread.cpp） | RTC CCD 停止 | Galil | 無 | REAL_TIME_CCD＝0（關） |
| TPLCIOThread | Modbus PLC IO 1 ms | PLC socket | Synchronize | 關 |
| TPadRS232Thread（uPadInterface.cpp） | 通訊式面板 1 ms | COM | Synchronize | ControlPanelMode＝0（關；**而且未移植**，另見下） |
| Omron EJ1N 兩條 | 5／50 ms | COM | Synchronize | 關 |
| HThreadCtrl socket（ATC／MonitorTCPIP） | TCP | socket | 無（TCPData CS） | ATC 關 |

main.dfm 14 個 TTimer 都在 UI 執行緒：Timer1 30 ms（大掃描：ProcessSensorScan／ProcessKeyFlush／UpdateMotorHomeLed…）、**TimerScanKey 30 ms**、Timer9／TimerDLL／Timer10 100 ms，其餘 1000 ms（詳見機台端 Claude 的盤點，機台樹 `docs/` 可補）。

移植版的對應：TRunControl 只有類別沒有執行緒（MainProc 在 PumpTick、500 ms）；Heater＝fast clock 20 ms；HThreadCtrlShuttle 沒有執行緒（`acarry.cpp:7169-7229` OpenThread 無作用）——⚠ `cinitial.cpp:12793-12884` 仍可能把 `bExeShuttleThread` 設 true，那時 `acarry.cpp:4168/4181` 用一個沒人更新的 `bShuttleHasIC` 判 JAM0560，請確認 HT9050 的 Tech 值；TimerScanKey＝每圈（≤50 ms）；Timer1 拆成 bin panel（fast clock 30 ms）＋PumpTick 內＋節拍；Timer3／8／ESD／溫度紀錄＝St02 dispatcher 1000 ms（在 PumpTick 裡，所以 500 ms 粒度）。

移植版自己開的執行緒：WebBridgeServer（HTTP／WebSocket select；⚠ `/api/editlist`、`/api/form` 會在這條執行緒上以 FormLock 跑 golden 表單碼，`FileRW/_ProxyTry.cpp:11-15`）、兩個看門狗、關機視窗、開機取樣器（每秒暫停主執行緒一次）、State Record 壓縮、COM 收送（vclcompat Comm；多數只排隊，⚠ `OmronLaser/LaserSensor.cpp:268-271` 直接在收執行緒跑 golden handler）、socket 收／accept、TesterComm、ELA、Jam 匯出、JSON 快取預熱。**都不呼叫 1203 API。**

## 4. 改架構的限制

- **Advantech SDK 的執行緒安全沒有文件**（`EtherCAT/vendor/*.h` 沒寫）。「只准一條執行緒呼叫 Acm_*」是本樹的約定（`Pci1203Monitor.h:1360-1363`、`Pci1203Control.h:802-803`）。最安全：**一條執行緒擁有全部 Acm_***。
- golden 本身的共用全域變數（cmydef／cprod／LastSet／Sen／SW／MOT）**完全沒有同步**；golden 的 Heater 靠「在主執行緒上、MainProc 兩圈之間跑」來避開競爭，計數也依賴 20 ms 節奏（WAR1637＝1000 次＝20 秒，`MachineType.h:1850`）。
- `PumpTick` 不拿鎖；tag publish 讀 MainProc 正在寫的同一批全域變數。
- 模態警報框會卡住呼叫它的那條執行緒。
- **500 ms 節拍是 B13 裁決**（`wb_serve.cpp:2924-2931`、`docs/RULINGS_20260917.md`）。EastSun 這次的要求等於要重新決定它——**改之前請跟 EastSun 確認 B13 的取代方式**。
- ctest 直接 pump `PumpTick`（`FastClockJobs.cpp:30-33`），改架構要一起顧。

## 5. 建議的切法（照 golden 有的地方照 golden，請 Jimmy 評估後定案）

1. **IO 執行緒（擁有 1203 卡）**：快迴圈 1～5 ms（DI 批次、排隊的 DO、軸狀態／位置），慢項目輪流做（站狀態、SDO、屬性、扭力、link／模組檢查）；雙緩衝快照交給邏輯執行緒；命令用佇列（控制層 Execute、VC8 SDO、Motor Test、回原點、Index Z 路線）。好處：把每次約 90 ms 的 Poll 從邏輯執行緒拿掉。風險：golden 的「寫完立刻讀」語意要保留（pending 機制）；每種 Acm 呼叫的實際耗時要量（目前推估約 0.8 ms／次）；所有直接呼叫點要找出來改走它。
2. **邏輯執行緒**：MainProc＋fast clock＋St02 計時器＋timer table＋ScanKey＋命令處理，照 golden `timeBeginPeriod(1)`、1～2 ms 一圈。**Heater 留在這條**（golden 就是 Synchronize 回主執行緒；獨立出去會跟 SW／Sen／警報搶），圈短了自然到 50 Hz。需要 B13 的新裁決。
3. **Web／publish 執行緒**：apiCache 只讀監看樣本，可以移到 IO 快照旁；tag publish 要一份一致的快照（邏輯每圈結尾產生），或先降頻（EVAL 選項 D，風險最低）。
4. **紀錄執行緒**：OpLine／StreamEmit／RecordProcess 改非同步單一寫入者佇列（目前 `OpLine` 在主迴圈 `fprintf`＋`fflush`，`wb_serve.cpp:8179-8199`，一天約 5 MB；今天 HDD 被塞住時就會拖慢主迴圈）。風險低，可以先做。
5. **Shuttle 殘料偵測**（golden 唯一的真平行執行緒）：只有 HT9050 的 `bExeShuttleThread` 可能為 true 才需要；放到 IO 執行緒 1 ms，不然 JAM0560 會沒偵測。
6. 模態警報等待時，邏輯執行緒以外的東西（IO、溫控、publish）不應停擺。

**驗收建議**：同樣一輪全機回原點＋空跑，`[FASTCLK]` 的每圈最慢、Heater 漏跑率、每個「動一下等到位」步驟的時間，前後對照；ctest 全過；1203 只有一條執行緒呼叫（`tools/pci1203_readonly_gate.ps1` 類的機械檢查）。

## 6. 附檔

| 檔名 | 內容 |
|---|---|
| `oplog_20261005_FASTCLK_all.txt` | 今天全部 3,455 行 `[FASTCLK]` 統計 |
| `oplog_20261005_1422-1428_MCCDY_home_START.txt` | 14:22～14:28 原文 |
| `EVAL_THREADING_20261002.md` | 機台樹 10-02 的舊評估（有 4 處要更正，見 §1） |

另：實體面板 RS-232（`uPadInterface`，`[System] ControlPanelMode=1`、`[TrayY] COM PORT`、115200 8N1、`t05…` 封包）**完全沒有移植**，mysensor.cpp／myswitch.cpp／cinitial.cpp:17139 都還 `#if 0`。電控說面板已照舊接上 RS-232。若改架構要新增 COM 執行緒，這條請一起考慮（golden 是 TPadRS232Thread 1 ms ＋ Synchronize）。
