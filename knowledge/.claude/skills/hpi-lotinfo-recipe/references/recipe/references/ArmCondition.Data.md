> 保存來源：`.claude/skills/ht9045-recipe/references/ArmCondition.Data.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# ArmCondition.Data — 機械臂速度與動作參數

**路徑：** `D:\HT9045\IniData\Data\[工作檔名]\ArmCondition.Data`
**模組：** [cSpeed.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cSpeed.cpp)
**結構體：** `ARM_CONDITION ArmSpeed_File[SpeedPartTotal]`、`SHUTTLE_SPEED SHSpeed`
**讀：** Local function (cSpeed.cpp) ｜ **寫：** Local function (cSpeed.cpp)

> **注意：** 函數實作在 `cSpeed.cpp` 的 TfSpeed 表單類別中，非獨立的 cArmCondition.cpp。

---

## Section 總覽

| Section | 用途 |
|---------|-----|
| `[All]` | 全域預設速度/加速 |
| `[Index Arm]` | Index 測試頭 (Z 軸) |
| `[Input Arm]` | InArm 入料臂 |
| `[Output Arm]` | OutArm 出料臂 |
| `[Empty Tray Arm]` | 補盤臂 |
| `[Shuttle]` | Shuttle 移動 |
| `[Tray]` | 各托盤馬達速度 |
| `[Magazine]` | Magazine 機構 |

---

## `[All]` — 全域預設

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| Speed | int | 1 | 全域預設速度 (%, 0~100) |
| Accel | int | 1 | 全域預設加速度 (%, 0~100) |
| EPControl | int | 1 | EP（端點）控制值；安裝 CKD FCM CleanAir 時生效 |

---

## `[Index Arm]` — 測試頭

| Key | 型態 | 預設 | 說明 | 對應變數 |
|-----|------|------|------|---------|
| X Speed | int | 1 | X 軸移動速度 (%) | `ArmSpeed_File[IndexArm].iBodySP` |
| X Accel | int | 100 | X 軸加速度 (%) | `ArmSpeed_File[IndexArm].iACDCBodySP` |
| Retry Count | int | 0 | Socket 檢查失敗重試次數（目前程式強制=0） | `ArmSpeed_File[IndexArm].iRetryCT` |
| Retry Down | float | 0.0 | 重試時 Z 補償量 (mm) | `ArmSpeed_File[IndexArm].dRetryDown` |
| Vacuum Check Time | float | 1.0 | 真空 EP 到達確認時間 (秒) | `ArmSpeed_File[IndexArm].dVacuumTI` |
| Counter Air ON Time | float | 1.0 | 反吹氣持續時間 (秒) | `ArmSpeed_File[IndexArm].dCTAirOn` |
| Destroy Again Time | float | 0.0 | 掉料再次吹氣時間 (秒) | `ArmSpeed_File[IndexArm].dDestroyAgainTime` |
| Destroy Again Count | int | 1 | 掉料再次吹氣次數 | `ArmSpeed_File[IndexArm].iDestroyAgainCount` |
| Socket Check | int | 0 | 1=每次下壓前檢查 Socket 狀態 | `ArmSpeed_File[IndexArm].bIndexFloatCHK` |
| Vacuum Timing | int | 0 | 0=同步真空, 1=序列真空 | `ArmSpeed_File[IndexArm].bSuckOnDown` |
| Pick IC when out shuttle no IC | int | 0 | 1=OutShuttle 無料時仍執行吸取 | `TestIF_File.bIndexPickICWhenOutShtNoIC` |
| Enable Delay Time Zero | int | 0 | 1=允許延遲時間為 0 | `TestIF_File.bEnableDelayTimeZero` |
| EnableIndexCycleTimeMonitoring | bool | true | SCK/SPIL 功能：啟用 Index 週期時間監控 | `TestIF_File.bIndexCycleTimeMonitor` |
| Monitoring_IndexCycletime | float | 0.0 | Index 週期時間目標值 | `TestIF_File.dIndexCycletimeMonitor` |
| Monitoring_Outlier | float | 0.0 | 離群值容許量 | `TestIF_File.dMonitorOutlier` |
| Monitoring_Window | int | 0 | 統計窗口大小 | `TestIF_File.iMonitorWindow` |
| IndexCycleTimetolerance | int | 15 | 週期時間容許誤差 (%) | `TestIF_File.dICTTolerance` |
| IndexCycleTimeAction | int | 0 | 超限動作：0=停機 | `TestIF_File.iICTAction` |

---

## `[Input Arm]` — 入料臂

| Key | 型態 | 預設 | 說明 | 對應變數 |
|-----|------|------|------|---------|
| XY Speed | int | 1 | XY 移動速度 (%) | `ArmSpeed_File[InArm].iBodySP` |
| XY Accel | int | 1 | XY 加速度 (%) | `ArmSpeed_File[InArm].iACDCBodySP` |
| Z Up Speed | int | 1 | Z 軸上升速度 (%) | `ArmSpeed_File[InArm].iZupSP` |
| Z Up Accel | int | 1 | Z 軸上升加速 (%) | — |
| Auto Speed Down | int | 0 | 1=啟用自動降速模式 | — |
| AutoSpeedLow | int | 50 | 自動速度的下限 (%) | — |
| Retry Count | int | 1 | 吸取失敗重試次數 | `ArmSpeed_File[InArm].iRetryCT` |
| Retry Down | float | 1.0 | 重試時 Z 補償量 (mm) | `ArmSpeed_File[InArm].dRetryDown` |
| Vacuum Check Time | float | 1.0 | 真空確認時間 (秒) | `ArmSpeed_File[InArm].dVacuumTI` |
| HP Vacuum Check Time | float | 1.0 | HotPlate 真空確認時間 (秒) | — |
| Use HP Vacuum Check Time | bool | false | 1=使用 HP 真空確認時間 | — |
| Counter Air ON Time | float | 1.0 | 反吹持續時間 (秒) | `ArmSpeed_File[InArm].dCTAirOn` |
| Shuttle Wait Time | float | 1.0 | 等待 Shuttle 的時間 (秒) | — |
| Auto Skip | int | 0 | 1=異常時自動跳過 | — |
| Auto Skip CT | int | 20 | 自動跳過觸發計數 | — |
| One by one | int | 0 | 1=逐個吸取（Pick Error 保護） | — |
| iInArmToShtReleaseMode | int | 0 | InArm 放到 Shuttle 的釋放模式 | — |
| Two Speed On Off | int | 0 | 0=單速, 1=雙速模式（靠近時降速） | — |
| Two Speed Distance | float | 3.0 | 切換雙速的距離 (mm) | — |
| Two Speed Precent | int | 10 | 雙速時的低速比例 (%) | — |
| Two Speed ADC | int | 10 | 雙速 ADC 調整值 | — |
| Two Speed Only Loader | bool | false | 1=雙速僅對 Loader 生效 | — |
| Vacuum Timing | int | 0 | 0=同步真空, 1=序列真空 | — |
| Open/Close Speed | int | 1 | 開關夾具速度 (%) | — |
| Open/Close Accel | int | 1 | 開關夾具加速 (%) | — |
| Destroy Again Time | float | 0.0 | 掉料再次吹氣時間 (秒) | — |
| Destroy Again Count | int | 1 | 掉料再次吹氣次數 | — |
| Destroy Check Time | float | 0.1 | 掉料偵測確認時間 (秒) | — |
| Destroy Pause Check | int | 0 | 1=掉料後暫停確認 | — |
| Rotate Input Speed | float | 0.1 | Rotator 入料速度 | — |
| Rotate Input Accel | int | 0 | Rotator 入料加速 | — |
| Cylinder Delay | float | 0.1 | 吸嘴氣缸延遲 (秒) | — |
| PrecisorOpenSpeed | int | 1 | Precisor 開啟速度 (%) | — |
| PrecisorCloseSpeed | int | 1 | Precisor 關閉速度 (%) | — |
| Release Delay Time | float | 0.0 | 放料後延遲時間 (秒) | — |
| Enable Relase Delay | int | 1 | 1=啟用放料延遲 | — |
| Enable Die Clean | int | 0 | 1=啟用 Die Clean 功能 | — |
| Die Clean Delay | float | 0.0 | Die Clean 延遲 (秒) | — |
| Die Clean Height | float | 0.1 | Die Clean 高度 (mm) | — |
| Enable Height Check | bool | false | 1=啟用高度檢測 | — |
| Height Check | int | 1000 | 高度檢測閾值 | — |
| bYPitchNotUseSearchLastMode | bool | false | 1=Y 間距不使用 SearchLast 模式 | — |
| Test Time Set Speed | int | 0 | 測試時設定速度 | — |

---

## `[Output Arm]` — 出料臂

同 Input Arm 大部分欄位，另含：

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| XY Speed | int | 1 | XY 速度 (%) |
| XY Accel | int | 1 | XY 加速 (%) |
| Wait before Air On | float | 0.01 | 放置前真空等待 (秒) |
| Rotate Output Speed | float | 0.1 | Rotator 出料速度 |
| Rotate Output Accel | int | 0 | Rotator 出料加速 |
| Out Pick Err Action | bool | false | 出料拾取錯誤動作 |
| Out Pick Err Tray | int | 6 | 出料拾取失敗放置托盤 |
| iE50_OutArmPickUpErrorOption | int | 0 | E50 出料拾取錯誤選項 |

---

## `[Empty Tray Arm]` — 補盤臂

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| Speed | int | 1 | 速度 (%) |
| Accel | int | 1 | 加速 (%) |
| Retry Count | int | 1 | 重試次數 |
| Vacuum Check Time | float | 1.0 | 真空確認時間 (秒) |
| Counter Air ON Time | float | 1.0 | 反吹持續時間 (秒) |
| Hand Down Time | float | 1.0 | 手臂下壓停留時間 (秒) |

---

## `[Shuttle]` — Shuttle 移動

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| Shuttle 1 Speed | int | 1 | Shuttle 1 移動速度 (%) |
| Shuttle 1 Accel | int | 1 | Shuttle 1 加速 (%) |
| Shuttle 2 Speed | int | 1 | Shuttle 2 移動速度 (%) |
| Shuttle 2 Accel | int | 1 | Shuttle 2 加速 (%) |
| Step Shuttle | int | 0 | 1=步進模式 Shuttle |
| Shake Shuttle | int | 0 | 1=震動 Shuttle 功能 |
| Shake Cycles | int | 1 | 震動週期數 |
| Shake distance | int | 5 | 震動距離 (pulse) |
| Delay between shakes | float | 0.0 | 震動間隔 (秒) |
| Shake Accel | int | 100 | 震動加速度 (%) |

---

## `[Tray]` — 托盤驅動（存在獨立 szTrayDir，非主路徑）

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| Loader Tray Speed | int | default | Loader 托盤電機速度 (%, 1~100) |
| Auto 1 Tray Speed | int | default | Auto1 托盤速度 (%) |
| Auto 2 Tray Speed | int | default | Auto2 托盤速度 (%) |
| Auto 3 Tray Speed | int | default | Auto3 托盤速度 (%) |
| Auto 4 Tray Speed | int | default | Auto4 托盤速度 (%) |
| Auto 5 Tray Speed | int | default | Auto5 托盤速度 (%) |
| Auto 6 Tray Speed | int | default | Auto6 托盤速度 (%) |
| Empty Tray Speed | int | default | 空盤補盤速度 (%) |
| Color Tray Speed | int | default | Color Tray 速度 (%) |

---

## `[Magazine]` — 料管機構

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| Magazine CatchY Speed | int | 1 | Y 軸抓取速度 (%) |
| Magazine CatchY Accel | int | 1 | Y 軸抓取加速 (%) |
| MagazineZ Speed | int | 1 | Z 軸速度 (%) |
| MagazineZ Accel | int | 1 | Z 軸加速 (%) |
| Two Speed Distance | float | 3.0 | 雙速切換距離 (mm) |
| Two Speed Precent | int | 10 | 雙速低速比例 (%) |
| Two Speed ADC | int | 10 | 雙速 ADC 調整 |

---

## 範例

```ini
[All]
   Speed=30
   Accel=100
   EPControl=0
[Index Arm]
   X Speed=50
   X Accel=100
   Retry Count=0
   Retry Down=0.50
   Vacuum Check Time=2.50
   Counter Air ON Time=0.05
   Socket Check=1
   Vacuum Timing=0
   Pick IC when out shuttle no IC=0
[Input Arm]
   XY Speed=30
   XY Accel=100
   Z Up Speed=50
   Z Up Accel=100
   AutoSpeedLow=50
   Retry Count=2
   Retry Down=0.30
   Vacuum Check Time=0.10
   Counter Air ON Time=0.05
   Shuttle Wait Time=0.00
   Auto Skip=0
   One by one=0
   Two Speed On Off=0
   bYPitchNotUseSearchLastMode=0
[Output Arm]
   XY Speed=30
   XY Accel=100
   Wait before Air On=0.01
[Empty Tray Arm]
   Speed=30
   Accel=100
   Hand Down Time=0.50
[Shuttle]
   Shuttle 1 Speed=30
   Shuttle 1 Accel=30
   Shuttle 2 Speed=30
   Shuttle 2 Accel=30
   Step Shuttle=0
   Shake Shuttle=0
[Tray]
Loader Tray Speed=70
Auto 1 Tray Speed=70
Auto 2 Tray Speed=70
Loader Tray 2nd Speed=10
[Magazine]
Magazine CatchY Speed=30
MagazineZ Speed=30
```

---

## 注意事項

- **實際讀寫函數在 `cSpeed.cpp`**（TfSpeed 表單類別），非獨立 cArmCondition.cpp
- `[All]` Speed/Accel 為全域預設；各模組獨立值若更大則以各模組為準
- `[Tray]` 存放路徑使用 `szTrayDir`（專用路徑），與其他 section 不同
- `Two Speed` 雙速模式：臂接近目標達到 `Two Speed Distance` 時切換為低速 (`Two Speed Precent`)
- `bLimitMaxSpeed` (IniConfig) 啟用時會自動限制最高速度
- `CKD FCM CleanAir` 安裝時才顯示/使用 EPControl 欄位

---

## 關聯程式碼

- 讀寫實作：[cSpeed.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cSpeed.cpp) — 搜尋 `"ArmCondition.Data"` 定位函數
- 表單定義：[cSpeed.h](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cSpeed.h)
- 結構體定義：[MachineType.h](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\MachineType.h) — `ARM_CONDITION`
- InArm 流程：ht9045-inarm-flow Skill
- OutArm 流程：ht9045-outarm-flow Skill
- Index 流程：ht9045-index-flow Skill
- Shuttle 流程：ht9045-shuttle-flow Skill
<!-- preserved-content:end -->
