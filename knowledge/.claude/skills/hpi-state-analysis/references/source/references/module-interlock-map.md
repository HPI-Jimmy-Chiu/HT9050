> 保存來源：`.claude/skills/ht9045-state-record-analysis/references/module-interlock-map.md`，main `811d95068`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# 模組間安全互鎖 / 依賴關係

## 符號說明

- `A → B`：A 需要 B 的條件滿足才能動作
- `A ⊗ B`：A 與 B 互斥（不可同時動作）
- `A ← flag`：A 被某旗標控制

> 此文件為累積式文件。每次發現新互鎖關係時補入。

---

## 1. Shuttle ↔ Index（Z 軸安全互鎖）

### SHT2 state 210 → TestZ2 安全高度

- **函式**：`IsTestZ2NotSafeShuttle2CanNotMove()`
- **位置**：`acarry.cpp` line ~4929
- **條件**：
  ```
  TestZ2 低於安全高度（Gali_ReadEncoderBelowCheckHeight(Prod.TestZ2_Safe)）
  AND
  TestY2 在 Rear 位置（Gali_ReadEncoderInRandge(Prod.TestY2_Rear)）
  ```
- **影響**：SHT2 state 210 無法向右移動
- **死鎖風險**：若 TestZ2 在下壓後未歸位（如 JAM0302 SKIP 後），此函式永遠返回 true

### SHT1 state 210 → TestZ1 安全高度

- **函式**：`IsTestZ1NotSafeShuttle1CanNotMove()`（對稱結構）
- **位置**：`acarry.cpp`（SHT1 對稱版）
- **條件**：同上（TestZ1 + TestY1）
- **影響**：SHT1 state 210 無法向右移動

---

## 2. InArm → Shuttle 位置

### InArm state 2000 → SHT2 在左側

- **條件**：`InSHT2InLF()`（Shuttle2 在左側位置）
- **位置**：`ainarm9045S_2x4_4_13.cpp` state 2000
- **影響**：InArm 等待 SHT2 到左側且 IC 被取走
- **死鎖風險**：若 SHT2 被 TestZ2 互鎖擋住無法移回左側

### InArm state 2000 → SHT1 在左側（對稱）

- **條件**：`InSHT1InLF()`
- **影響**：同上

---

## 3. OutArm → Shuttle 位置 + IC 資料

### OutArm state 1100 → SHT1 IC 就緒

- **條件**（多重 AND）：
  ```
  OutSHT1InRT()                   — SHT1 在右側
  bCheckShuttle1Flag == true       — SHT1 資料準備完成
  FRCarryKit.UseSiteHasIC()        — SHT1 上有 IC
  IsOutArmCleanOutFinish()         — Clean Out 完成
  WhichAutoNeedTray() 無阻擋       — 有可用 Tray
  ```
- **位置**：`aoutarm9045_2x4_4.cpp` state 1100
- **影響**：任一條件不滿足 → OutArm 停在 1100 等待
- **死鎖風險**：若 SHT1 無法向右移動（被 InArm 或 TestZ1 擋住），OutArm 永遠等不到 IC

### OutArm pick 子任務完成 ≠ OutArm 已持料

- **案例觀察**：`OutArmTask` 可在 `50 -> 100 -> 1140 -> 1200 -> 50` 空轉，
  同時 `iPickFromShuttle1Task` 已完成 `1 -> 10 -> 200 -> 1000 -> 1`。
- **核心依賴**：OutArm 主流程要前進到放料段（3000+），實際依賴 `OutArmSuck` 持料結果。
- **影響**：若 `FRCarryKit`/`BRCarryKit` 仍有料但 `OutArmSuck` 維持全 0，
  主流程會重回 50 重試，Shuttle 也會因此停在右側等待清空。

**分析規則**：
- 看到 Shuttle 停右側時，先檢查 `OutArmSuck` 是否有料，再判定是否真為 Shuttle 無法移動問題。
- 若子任務已回 idle 但 `OutArmSuck` 無料，優先查 pick side / nozzle / vacuum 條件。

---

## 4. DoTestHeadMotor → 系統旗標（門控）

### DoTestHeadMotor 進入條件

- **位置**：`csystem.cpp` line ~9765
- **門控**：
  ```
  if(SoftStop==true || SystemStart==false || fAllMotorHome==false) return;
  ```
- **影響**：DoTestHeadMotor 控制所有測試頭動作，包含呼叫 `GetTesterResult()`
- **死鎖風險**：
  - PAUSE 設 `SoftStop=true` → DoTestHeadMotor 停止 → iTestTask 不被驅動
  - ShowErrorMessage 設 `SystemStart=false` → 同效果

### SoftStop / SoftStart 生命週期

| 旗標 | 設為 true | 設為 false |
|------|----------|-----------|
| `SoftStop` | `Pause()` (main.cpp ~6136) | `ckernel.cpp ScanSystemSensor` 當 `SoftStart==true` 時 (line ~378) |
| `SoftStart` | `Start()` (main.cpp ~4287) | `ckernel.cpp` 消費後清除 |
| `SystemStart` | `Start()` 路徑 | `ShowErrorMessage()` (note.cpp ~789) |

### ShowErrorMessage 重置行為

- **位置**：`note.cpp` line ~789
- **效果**：`SoftStop=false; SoftStart=false; SystemStart=false;`
- **加上**：呼叫 `StopAllMotor()`
- **重要**：`StopAllMotor()` 停止馬達在**當前位置**，不會送回安全位

---

## 5. bEcho 生命週期

| 事件 | 設定值 | 位置 |
|------|--------|------|
| `RunTestProgram()` | `bEcho = false` | `main.cpp` ~17738 |
| 收到 BINON + ECHOOK | `bEcho = true` | `main.cpp OnMyCopyMsg()` ~16517 |
| `SendMessageToGpibProg()` | `bEcho = false` | `main.cpp` ~21263（僅設定變更時） |

- **消費點**：`atester.cpp` case 60 — 讀取 `bEcho` 後處理測試結果
- **依賴**：需要 `DoTestHeadMotor()` → `GetTesterResult()` 被呼叫

---

## 6. bShuttle2Pause（D42 專用）

| 事件 | 設定值 | 位置 |
|------|--------|------|
| JAM0302 SKIP + `bD42IndexPickICShuttlePause=1` | `true` | `aTester_Rear.cpp` ~1844 |
| SHT2 到左側後觸發 Pause | `false` + 呼叫 `fMain->Pause()` | `acarry.cpp Do_Auto_SHT2()` |

- **影響**：`bShuttle2Pause=true` 時，SHT2 到左側後會觸發整機暫停
- **死鎖風險**：暫停後若 TestZ2 未歸位，START 恢復後 SHT2 仍被 IsTestZ2NotSafe 擋住

---

## 7. 依賴圖（文字版）

```
                 ┌─────────────┐
                 │  GPIB 回傳   │
                 │  bEcho=true  │
                 └──────┬──────┘
                        │
                        ▼
              ┌──────────────────┐
              │  DoTestHeadMotor │ ← SoftStop / SystemStart 門控
              │  → GetTesterResult│
              │  → iTestTask=60  │
              └──────────────────┘
                        │
                 TestZ 歸位？
                   │        │
                  YES       NO（下壓位）
                   │        │
                   ▼        ▼
         SHT 可右移    IsTestZNotSafe = true
              │              │
              ▼              ▼
      InArm/OutArm      SHT 被擋 → InArm 等 SHT
        正常循環              → OutArm 等 IC
                              → 死鎖
```

---

## 待補充互鎖

以下互鎖在後續案例中遇到時再補充：

- InArm ⊗ Shuttle 安全保護（`DoInOutARM_SHT_MoveSafe`）
- OutArm ⊗ Shuttle 安全保護
- CatchTray → Loader 位置
- Fix3 氣缸保護
- EtherCAT 馬達安全互鎖（HT9046AU）

<!-- preserved-content:end -->
