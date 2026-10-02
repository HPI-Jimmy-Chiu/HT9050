# StateRecord 資料夾結構與檔案格式

## 1. 資料夾結構

```
<StateRecord 時間戳>/                        ← 命名格式：YYYY-MM-DD HH_MM_SS
├── Ver.txt                                  ← 軟體版本、GPIB 版本、Setup、Config 摘要
├── Task.xls                                 ← Task 快照（XLS 格式，人工讀取用）
├── Task_List.xls                            ← Task 清單（XLS 格式）
├── Task_ListWithTime.csv                    ← ★核心：各 Task 帶時間戳的狀態變化歷史
├── Task_ListWithTime2.csv                   ← 補充 Task 歷史（較低頻率的 Task）
├── EventLogTxt_YYYYMMDD.csv                 ← 事件日誌（ALARM/START/PAUSE/OneCycle）
├── Motor.xls                                ← 馬達位置快照
├── AutoClean.xls                            ← Auto Clean 計數
├── MainForm.bmp / MainForm.png              ← 主畫面截圖
├── MotionView.bmp / MotionView.png          ← 馬達座標監視截圖
├── MotorView.bmp / MotorView.png            ← 馬達狀態（Servo/Alarm）截圖
├── GPIB.bmp / GPIB.png                      ← GPIB 通訊畫面截圖
├── HT9045/
│   ├── config/                              ← 機台 Config 設定
│   │   ├── config.ini                       ← IniConfig 功能開關
│   │   ├── LastSet.ini                      ← 最後設定（Bin 分配等）
│   │   ├── ATC.ini                          ← ATC 溫控設定
│   │   ├── ATCWinWay.ini                    ← ATC WinWay 設定
│   │   ├── CriticalParaControl.ini          ← 關鍵參數管控
│   │   ├── Description.ini                  ← 設備描述
│   │   ├── ESDconfig.ini                    ← ESD 設定
│   │   ├── FormPos.def                      ← 表單位置
│   │   ├── ReleaseNote.txt                  ← 當前版本 Release Note
│   │   └── Security_new.def                 ← 安全設定
│   ├── system/                              ← 機台系統設定
│   │   ├── Gerneral.ini                     ← 主設定（CUSTOMER_CODE、機號等）
│   │   ├── teach.ini                        ← 教導資料
│   │   ├── RunMode.txt                      ← 運行模式
│   │   ├── motor.DB / Sensor.DB / ...       ← 馬達/感測器/氣缸定義
│   │   ├── SocketCount.ini                  ← Socket 計數
│   │   ├── ContactInfo.ini                  ← 接觸資訊
│   │   └── ...（其他 .dat / .db / .ini）
│   └── IniData/
│       └── Data/                            ← Recipe 工作檔
├── GPIB9045/
│   └── system/                              ← GPIB 端設定
└── GPIBLOG/                                 ← GPIB 通訊 Log
    ├── YYYY-MM-DD HH MM SS.txt             ← 每次 GPIB 連線一個檔案
    ├── ...（多個 log 檔，按連線時間命名）
    └── YYYY_MM.7z                           ← 壓縮歷史 log
```

## 2. Ver.txt 格式

Ver.txt 由程式自動產生，內容為純文字，包含機台完整狀態快照。

### 關鍵欄位提取

| 欄位 | 說明 | 範例 |
|------|------|------|
| Handler 版本 | 軟體版本號 | `V3.33.893.14` |
| GPIB 版本 | GPIB 通訊程式版本 | `V12.13.883.0` |
| 機台序號 | Serial Number | `JLD675` |
| CUSTOMER_CODE | 客戶代碼 | `959` |
| Setup | 測試 Setup 名稱 | `KQX-2.99X3.54-NFC...` |
| TestMode | 測試模式 | `8-Site (2x4)` |
| BinMode | Bin 分類模式 | `FT Bin` |
| Temperature | 溫度設定 | `Ambient` / `Hot` / `Cold` |
| Tester Type | 測試機型號 | `Advan type1` |
| GPIB Mode | GPIB 模式 | `Normal GPIB` |
| SECS GEM | SECS/GEM 狀態 | `Enabled (Mode 4)` |
| Simulte | Simulte 模式 | `Enable` / `Disable` |
| DUT On/Off | 各 Site 啟用狀態 | `Aa=1,Ab=0,...` |

### 解讀要點
- CUSTOMER_CODE 對應 `MachineType.h` 的 `#define CC_xxx nnn`
- DUT On/Off 表示哪些 Site 被停用（0=關閉，可能與 Yield 相關）
- Simulte Enable 表示模擬模式（影響某些安全檢查行為）

## 3. Task_ListWithTime.csv 格式

### 基本格式

```
TaskName, timestamp1, value1, timestamp2, value2, timestamp3, value3, ...
```

- **每行一個 Task**，後接時間戳-值配對
- **最新狀態在最前面**（時間遞減排列）
- **時間格式**：`HH:MM:SS.mmm`（毫秒精度）
- **值為整數**：對應 `switch(Task)` 的 case 編號
- **行可能非常長**（數百個配對，記錄數分鐘~數小時的歷史）

### 範例

```
AutoSHT2Task, 15:38:00.703, 210, 15:38:00.671, 200, 15:38:00.671, 202, 15:37:14.765, 1, ...
```

解讀：
- 最新：15:38:00.703 進入 state **210**（卡住）
- 之前：15:38:00.671 在 state 200（準備向右移動）
- 更早：15:38:00.671 在 state 202
- 更早：15:37:14.765 在 state 1（idle）
- ...

### 分析技巧

1. **找最終狀態**：每行第一個 value 就是 StateRecord 時的狀態
2. **找停滯開始**：第一個 timestamp 就是進入最終狀態的時間
3. **計算停滯時長**：StateRecord 時間 - 第一個 timestamp
4. **找狀態變化頻率**：正常運行時，Task 應高頻切換（~15ms）；停滯時只有一個值不變
5. **跨日處理**：若時間戳跨越午夜（如 23:59 → 00:01），需注意日期換算

### 末尾擴充區塊（SaveTaskList 寫入）

`Task_ListWithTime.csv` 不只有 Task 時序，檔案末尾還包含 `SaveTaskList()` 追加的即時快照：

- 流程旗標（例如 `bPickFromLoader`, `bPlaceToHotplate`, `bPickFromHotplate`）
- 測試需求旗標（例如 `fFrontNeedTest`, `fRearNeedTest`, `f32SiteNeedSuck`）
- Auto Clean 旗標（例如 `bLockPlaceToShuttleByAutoClean`）
- InArm 選擇資訊（`InArmSuck.iWhichSht`, `InArmSuck.iWhichKit`）
- 六組持料矩陣（`InArmSuck`, `OutArmSuck`, `FRCarryKit`, `BRCarryKit`, `FTestSuck`, `BTestSuck`）
- CleanKitTime 矩陣

因此分析時請先看「Task 最終狀態」，再看末尾旗標與矩陣，
可快速區分：
- 邏輯未進入（旗標未成立）
- 邏輯有進入但資料未轉移（例如 CarryKit 有料、OutArmSuck 無料）
- 邏輯被 Auto Clean 等機制鎖定

### 重點 Task 清單

| Task 名稱 | 對應模組 | 正常行為 |
|-----------|---------|---------|
| `AutoSHT1Task` | Shuttle 1 | 高頻循環 1→10→100→120→1 |
| `AutoSHT2Task` | Shuttle 2 | 高頻循環 1→10→100→120→1 |
| `InArmTask` | InArm | 循環 50→100→400→2000→50 |
| `InArmPlaceToShuttleTask` | InArm 放料子任務 | 循環 1→900→1100→1500→1 |
| `OutArmTask` | OutArm | 循環 1100→... |
| `TestTask` | 測試結果等待 | 循環 50→55→60→(處理完)→50 |
| `OneCycleTask` | 單循環 | 0 或 3 |
| `iLifterTask[x][y]` | Lifter | 100→200→... |
| `iCatchFromLoaderTask` | CatchTray | — |

## 4. Task_ListWithTime2.csv 格式

格式與 `Task_ListWithTime.csv` 相同。

區別：
- 記錄 **較低頻率變化** 的 Task（如 `OneCycleTask`、`iLifterTask`）
- 第一行可能帶有標題：`Time,Task,Task_Name`
- 後續行格式：`HH:MM:SS.mmm, value, TaskName`

## 5. EventLogTxt_YYYYMMDD.csv 格式

事件日誌，記錄當天所有操作與系統事件。

### 關鍵事件類型

| 事件 | 說明 | 分析價值 |
|------|------|---------|
| START | 操作員按 START | 標記恢復時間點 |
| PAUSE | 操作員按 PAUSE | 標記暫停時間點 |
| ONE CYCLE | 操作員按 ONE CYCLE | 嘗試單步恢復 |
| ALARM RESET | 操作員清除警報 | 確認操作員反應 |
| JAMxxxx | JAM 警報 | 嚴重錯誤（需停機處理） |
| WARxxxx | WAR 警告 | 輕微警告（可繼續運行） |
| LOT START | 開始 Lot | Lot 邊界 |
| LOT END | 結束 Lot | Lot 邊界 |

### 分析技巧

- 重點看 **PAUSE → START** 之間的時間差（操作員等待時間）
- 重點看 **JAM** 事件後的操作（RETRY / SKIP / HOME）
- 連續多次 START/PAUSE 表示操作員在嘗試恢復（可能已經死鎖）

## 6. GPIBLOG/*.txt 格式

每次 GPIB 連線產生一個 log 檔。

### 檔名格式

`YYYY-MM-DD HH MM SS.txt`（連線建立時間）

### 內容格式

每行一條訊息，包含方向（Send/Receive）、內容、時間戳。

### 關鍵 Pattern

| Pattern | 說明 |
|---------|------|
| `BINON:xxxxxxxx` | 測試結果回傳（含 Bin 編碼） |
| `ECHOOK` | Echo 確認（bEcho=true 設定點） |
| `Disable Bar Code Command` | 心跳/閒置訊息 |
| `TESTON` | 開始測試 |
| `TESTOFF` | 測試結束 |

### 分析技巧

1. 找最後一條 `BINON` → 確認 GPIB 最後正常回傳時間
2. 找 `BINON` 後是否有 `ECHOOK` → 確認握手是否完成
3. `BINON` 後只剩心跳 → GPIB 正常，問題在 Handler 端
4. 選擇最接近 StateRecord 時間的 log 檔

## 7. 截圖說明

| 截圖 | 看什麼 |
|------|--------|
| `MainForm.bmp/png` | 主畫面 — 運行狀態、Alarm 顯示、Tray 狀態 |
| `MotionView.bmp/png` | 馬達座標 — 各軸實際位置（判斷是否在安全位） |
| `MotorView.bmp/png` | 馬達狀態 — Servo On/Off、Alarm 旗標 |
| `GPIB.bmp/png` | GPIB 畫面 — 通訊狀態、最後收到的訊息 |

截圖用 `view_image` 工具檢視。

## 8. Config 子目錄

### HT9045/config/config.ini

IniConfig 功能開關。格式為 INI 格式，以 `[Section]` 分組。

重點 Section：
- `[D]` — Index 相關（D42/D43/D44/D50/D64 等）
- `[F]` — Shuttle 相關（F07 等）
- `[N]` — 網路/下載相關

### HT9045/system/Gerneral.ini

主設定檔。包含 CUSTOMER_CODE、機號、Factory 等。

### HT9045/config/LastSet.ini

最後設定。包含 Bin 分配、Site 設定等。
