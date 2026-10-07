> 保存來源：`.claude/skills/ht9045-state-record-analysis/references/config-recovery-impact.md`，main `811d95068`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Config 旗標對錯誤恢復路徑的影響

## 概述

HT9045 的 Config 旗標（`config.ini` 中的 IniConfig 欄位）會影響 Alarm 發生後的恢復路徑。
分析 StateRecord 時，需根據 Config 旗標判斷操作員可選的動作（RETRY/SKIP/HOME）
以及每個動作會觸發哪些副作用（旗標設定、馬達移動、暫停等）。

> 此文件為累積式文件。每次分析新案例發現 Config 影響時補入。

---

## D 系列（Index 相關）

### D42 — IndexPickICShuttlePause

| 項目 | 內容 |
|------|------|
| 變數名 | `IniConfig.bD42IndexPickICShuttlePause` |
| 預設值 | 0（關閉） |
| 功能 | 啟用時：JAM0302 (Index pick-up error) SKIP 後，強制暫停 Shuttle，要求操作員清空 |

**影響路徑**：

```
JAM0302 SKIP + D42=1
  → bShuttle2Pause=true
  → bIndexArm2PickupErrStop=true
  → bShowShuttle2Device=true
  → bInArmNeedToSafePos=true
  → SHT2 到左側後觸發 fMain->Pause("Do_Auto_SHT2")
  → 操作員需清空 Shuttle 後按 START
```

**死鎖風險**：若 TestZ2 未歸位 + 操作員直接按 START → SHT2 被 IsTestZ2NotSafe 擋住
（見 deadlock-patterns.md Pattern #1）

### D43 — IndexDropErrorCanRetryandSkip

| 項目 | 內容 |
|------|------|
| 變數名 | `IniConfig.bD43IndexDropErrorCanRetryandSkip` |
| 預設值 | 0（關閉） |
| 功能 | 影響掉料重試邏輯 |

**影響路徑**：

```
啟用時：
  → bIndexPickErrShtStayRight2 旗標被管理
  → JAM0302 SKIP 時額外清除 bIndexPickErrShtStayRight2=false
  （防止 Shuttle 停在右側）
```

### D44 — CheckIndexICDestroy

| 項目 | 內容 |
|------|------|
| 變數名 | `IniConfig.bD44CheckIndexICDestroy` |
| 預設值 | 0（關閉） |
| 功能 | Index 下壓後檢查 IC 是否損壞 |

### D50 — IndexPickErrSkipNeedCheckVac

| 項目 | 內容 |
|------|------|
| 變數名 | `IniConfig.bD50IndexPickErrSkipNeedCheckVac` |
| 預設值 | 0（關閉） |
| 功能 | JAM0302 SKIP 後慢速下降再吸一次 |

**影響路徑**：

```
啟用時（+ CosFunction.bIndexPickErrSkipNeedCheckVac=true）：
  → bSkipNeedCheckVac[1][i][j]=true
  → bArm2PressSkipNeedDownCheckVac=true
  → 下次 Index 下壓時會執行額外的真空檢查
```

### D64 — IndexPickErrOnlySKIP

| 項目 | 內容 |
|------|------|
| 變數名 | `IniConfig.bD64IndexPickErrOnlySKIP` |
| 預設值 | 0（關閉） |
| 功能 | JAM0302 只允許 SKIP（移除 RETRY 選項） |

**影響路徑**：

```
啟用時：
  → ShowErrorMessage("JAM0302", K_SKIP, ...)  ← 只有 SKIP
  （正常時：ShowErrorMessage("JAM0302", K_SKIP|K_RETRY, ...)）
```

### D72 — NNModeMoveShtAfterContact

| 項目 | 內容 |
|------|------|
| 變數名 | `IniConfig.bD72NNModeMoveShtAfterContact` |
| 預設值 | 0（關閉） |
| 功能 | NN 模式下，Contact 後才移動 Shuttle |

---

## F 系列（Shuttle 相關）

### F07 — OutShuttleSensorMode

| 項目 | 內容 |
|------|------|
| 變數名 | `IniConfig.iF07OutShuttleSensorMode` |
| 預設值 | 0 |
| 功能 | 出料 Shuttle Sensor 模式選擇 |

---

## 交互影響分析

### D42 + JAM0302 SKIP → 死鎖風險

```
條件：D42=1 + JAM0302 發生
路徑：SKIP → bShuttle2Pause=true → SHT2 暫停
風險：若 TestZ2 同時未歸位
      → IsTestZ2NotSafe = true
      → SHT2 無法右移
      → 需要 TestZ2 歸位 + 操作員清空 Shuttle 兩個條件都完成
      → 若操作員直接按 START 跳過清空 → 死鎖
```

### D43 + D42 + JAM0302 SKIP

```
條件：D43=1 + D42=1 + JAM0302 發生
路徑：SKIP 時 D43 額外清除 bIndexPickErrShtStayRight2=false
      → 避免 Shuttle 停在右側（D43 是 D42 的輔助）
      → 但不解決 TestZ2 未歸位問題
```

### bIndexPickErrOnlySKIP (D64) + D42

```
條件：D64=1 + D42=1
影響：操作員只能 SKIP（無法 RETRY）
      → SKIP 必定觸發 D42 的 bShuttle2Pause
      → 死鎖風險更高（無法透過 RETRY 避開 D42 路徑）
```

---

## 待補充

以下 Config 在後續案例中遇到時再補充影響分析：

- A 系列（通用功能）
- B 系列（InArm 相關）
- C 系列（OutArm 相關）
- E 系列（SECS/GEM 相關）
- G 系列（GPIB 相關）
- I 系列（IO 相關）
- L 系列（Loader 相關）
- M 系列（Magazine 相關）
- N 系列（網路/下載相關）
- O 系列（其他）
- P 系列（Piggyback 相關）

<!-- preserved-content:end -->
