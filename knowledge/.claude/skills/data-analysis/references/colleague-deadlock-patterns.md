# 已知死鎖模式（案例庫）

## 快速比對指南

收到 StateRecord 後，按以下順序比對：

1. **先看 Task 最終狀態組合**（查下方各 Pattern 的「識別特徵」）
2. **再看 EventLog** 是否有匹配的 ALARM 序列
3. **最後確認 Config 旗標**

命中任一 Pattern → 直接引用結論 + 修正建議，標記「已知模式 Pattern #N」。

> 此文件為累積式文件。每次新案例確認後，以遞增編號新增 Pattern。

---

## ⚠️ 分析誤區與反模式（必讀！）

### 核心原則

**❌ 錯誤思維**：Task 停在哪個 state → 就改那個 Task 的程式碼  
**✅ 正確思維**：Task 停在哪個 state → 找出它等待的條件 → 找上游模組為何沒滿足條件

### Shuttle case 10 誤判案例（20260417 教訓）

**錯誤分析鏈**：
1. 看到 `AutoSHT1Task = 10`, `AutoSHT2Task = 10` 長時間停滯
2. 查 `acarry.cpp` case 10 程式碼 → "prepare left move"
3. 發現有多個安全互鎖條件（Index Z、OutArm 位置、Dummy 模式等）
4. **錯誤結論**：Shuttle 互鎖有問題 → 建議修改 Shuttle case 10 邏輯

**為什麼錯了**：
- **只看 Task 表象，沒看實際狀態**：Shuttle 當時在右邊，FRCarryKit/BRCarryKit 有 IC，正確地等待 OutArm 取料
- **忽略上下游關係**：Shuttle case 10 是**等待者**（被動），不是**執行者**（主動）
- **應該優先查**：為何 OutArm 沒把 IC 取走？（OutArmTask = 1200 循環，OutArmSuck = 0）

**正確分析鏈**：
1. Shuttle case 10 停滯 → 查程式碼 → "等 OutArm 清空"
2. 查 Shuttle 實際位置 → 在右邊 ✓
3. 查 FRCarryKit/BRCarryKit → 有 IC（1002）✓
4. **推論**：Shuttle 在正確位置，正確地等待 OutArm
5. **轉向 OutArm**：OutArmTask = 1200 循環 → 這才是根因！
6. 查 OutArmSuck → 全 0（沒拿到料）→ 確認 OutArm pick 失敗（見 Pattern #3）

### 死鎖分析檢查清單（避免誤判）

**禁止直接修改停滯 Task 的程式碼**，必須先：

1. **確認 Task 職責**：
   - □ 主動執行動作（馬達、氣缸、通訊）
   - □ 等待條件成立（等上游模組、等感測器、等旗標）

2. **若是等待者，先查條件**：
   - 它在等什麼條件？（查程式碼）
   - 條件是否該成立？（查 StateRecord 實際狀態：馬達位置、IC 快照、旗標）
   - → 若條件應該成立但未成立 → **轉向上游模組**
   - → 若條件不該成立 → 才懷疑該 Task 邏輯

3. **找真正的執行者**：
   - 誰負責滿足該條件？
   - 該模組的 Task 在幹嘛？
   - 為何沒成功執行？

**快速參考**：

| 表象 Task | ❌ 誤判方向 | ✅ 正確方向 | 根因模組 |
|-----------|------------|------------|----------|
| AutoSHT1/2Task=10 | 改 Shuttle 互鎖 | 查 OutArm 為何沒取料 | OutArm pick 失敗 |
| InArmTask=2000 | 改 InArm 放料流程 | 查 Shuttle 為何沒回左 | Shuttle 等 OutArm |
| OutArmTask=50 循環 | 改 OutArm retry 次數 | 查 pick 子任務為何無料 | pick side 選擇錯誤 |

---

## Pattern #1：PAUSE-during-test + JAM0302 + D42 四方死鎖

### 識別特徵

| 條件 | 值 |
|------|-----|
| `AutoSHT2Task` | **210**（停滯 > 1 分鐘） |
| `InArmTask` | **2000** |
| `OutArmTask` | **1100**（或 50） |
| `TestTask` | **60**（或已脫離但 TestZ2 未歸位） |
| EventLog | 有 **JAM0302** + 之前有 **PAUSE → START** 序列 |
| Config | `bD42IndexPickICShuttlePause = 1` |

### 時間線模式

```
測試中 PAUSE → GPIB 回傳但 TestTask 未消費 → START 恢復
→ TestTask 處理完結果 → Index 動作 → JAM0302 (pick-up error)
→ 操作員按 SKIP → bShuttle2Pause=true（因 D42 啟用）
→ SHT2 到左側後觸發 Pause → 操作員按 START
→ TestZ2 仍在下壓/中間位（SKIP 未觸發 Z2 歸位）
→ IsTestZ2NotSafeShuttle2CanNotMove() = true → SHT2 state 210 永久阻擋
→ InArm 等 SHT2 → OutArm 等 IC → 四方死鎖
```

### 根因

JAM0302 SKIP 路徑中，`bD42IndexPickICShuttlePause=true` 暫停了 SHT2，
但 **TestZ2 未被送回安全高度**。操作員按 START 恢復後：
- SHT2 嘗試在 state 210 向右移動
- `IsTestZ2NotSafeShuttle2CanNotMove()` 因 TestZ2 仍在不安全位置 → 阻擋
- InArm 等 SHT2 清空 → OutArm 等 IC → 全系統死鎖

### 偶發條件

需同時滿足（缺一不可）：
1. 測試期間操作員按 PAUSE
2. 恢復後發生 JAM0302（Index pick-up error）
3. `bD42IndexPickICShuttlePause=1`（啟用）
4. 操作員選擇 SKIP（非 RETRY）
5. TestZ2 在 SKIP 後未歸位

### 修正方向

| 優先序 | 建議 |
|--------|------|
| P0 | JAM0302 SKIP + D42 暫停後，START 恢復時強制檢查 TestZ2 歸位 |
| P1 | `IsTestZ2NotSafe...()` 加超時保護（卡 >30s 報 WAR） |
| P2 | D42 暫停 → START 恢復路徑需驗證 bShuttle2Pause 已正確處理 |
| P3 | 檢修反覆出錯的 Site 吸嘴（硬體）|

### 案例

| 日期 | 客戶 | 機台 | 版本 | 測試模式 |
|------|------|------|------|---------|
| 2026-04-15 | JCET (KYEC) CC_959 | JLD675 | V3.33.893.14 | 8-Site (2x4) |

### 相關程式碼

| 檔案 | 行號 | 內容 |
|------|------|------|
| `aTester_Rear.cpp` | ~1793 | JAM0302 ShowErrorMessage |
| `aTester_Rear.cpp` | ~1844 | SKIP 路徑 bShuttle2Pause=true |
| `acarry.cpp` | ~4929 | IsTestZ2NotSafeShuttle2CanNotMove() |
| `acarry.cpp` | ~6084 | Do_Auto_SHT2() state 210 |
| `csystem.cpp` | ~9765 | DoTestHeadMotor() 門控 |
| `atester.cpp` | ~1443 | iTestTask case 60 |
| `main.cpp` | ~16517 | bEcho=true (BINON) |
| `note.cpp` | ~789 | ShowErrorMessage 重置 |

---

## Pattern #2：OutArm 取料子任務完成但資料未轉移（Shuttle 右側空轉）

### 識別特徵

| 條件 | 值 |
|------|-----|
| `OutArmTask` | 常見循環 **50 -> 100 -> 1140 -> 1200 -> 50** |
| `iPickFromShuttle1Task` | 循環 **1 -> 10 -> 200 -> 1000 -> 1**（看似完成） |
| `OutArmSuck` | 全 0（無料） |
| `FRCarryKit` / `BRCarryKit` | 仍有 `HAS_IC` 或 `100x` 測後資料 |
| Shuttle 位置 | 右側（Current/Target 接近 `iRight`） |

### 時間線模式

```
OutArm 進入 pick 路徑
-> PickFromShuttle 子任務完成並回 1
-> OutArm 主流程未進入放料段（3000+）
-> 回到 50 再次檢查
-> 再進 100/1140/1200 重複取料
-> Shuttle 持續停在右側等待被清空
```

### 根因

主流程判斷依賴「OutArm 實際持料結果」而非「子任務是否回 idle」。
因此即使 `iPickFromShuttle1Task` 回到 1，若 `OutArmSuck` 沒有成功拿到料，
OutArm 仍不會進入放料流程，最終形成取料空轉迴圈。

### 偶發條件

常見於以下任一條件存在時：
1. pick side 選擇與 CarryKit 實際有料 side 不一致
2. 吸嘴幾何配置或 site mask 不匹配
3. 真空吸取條件未成立（程式判定未成功持料）

### 修正方向

| 優先序 | 建議 |
|--------|------|
| P0 | 在 OutArm pick 成功條件後加「OutArmSuck 至少一點有料」的診斷 EventLog |
| P1 | 當 pick 子任務完成但 `OutArmSuck` 全 0 且 CarryKit 有料時，拋出明確 WAR（避免無限空轉） |
| P2 | 檢查 pick side / nozzle 對應表與 FR/BR CarryKit site 映射 |
| P3 | 檢查真空與破真空時序參數，避免程式判定吸取失敗 |

### 案例

| 日期 | 客戶 | 機台 | 版本 | 測試模式 |
|------|------|------|------|---------|
| 2026-03-27 | (待補) | (待補) | V3.33.878.0 | 2x8_32 |

---
## Pattern #3：GetNowShuttleMode 判斷錯誤導致 OutArm 取料位置錯誤（2x8 模式）

### 識別特徵

| 條件 | 值 |
|------|-----|
| `iCloseSiteModeFor2x8` | **e2x8CloseEven (5)** 或其他 2x8 模式 |
| `AutoSHT1Task` / `AutoSHT2Task` | **10**（準備向左移動，停滯 > 30 秒） |
| `InArmTask` | **2000**（等待 Shuttle 清空） |
| `OutArmTask` | 循環 **50 → 100 → 1200 → 50**（持續取料但失敗） |
| `PickFromShuttle1Task` | 循環 **1 → 10 → 200 → 1000 → 1**（子任務看似完成） |
| `OutArmSuck` | **全 0**（完全沒拿到料） |
| `FRCarryKit` / `BRCarryKit` | 只有**單一 Row**（RowA 或 RowB）有料，位置明確 |

### 時間線模式

```
機台正常運行中
→ Shuttle 上只有單一 Row 的 Column 4 或 6 有料
   （例如：Item[0][6] = 1002，其他位置全空）
→ OutArm 準備從 Shuttle 取料
→ 呼叫 GetNowShuttleMode_2x8_8() 判斷取料位置
→ ❌ 函數判斷錯誤（重複檢查 Item[0][4]，遺漏 Item[0][6]）
   ├─ 應回傳：51000（RowA 有料）
   └─ 實際回傳：51001（RowB 有料）
→ OutArm 移動到錯誤的 Row（RowB）
→ RowB 沒有 IC → 吸取失敗
→ OutArmSuck 保持全 0
→ PickFromShuttle1Task 完成（回 state 1）
→ OutArm 主流程檢查發現沒拿到料
→ 回到 state 50 再次嘗試 → 無限循環
→ Shuttle 無法被清空 → AutoSHT1Task 卡在 state 10
→ InArm 等待 Shuttle 左移 → 三方循環依賴死鎖
```

### 根因

**GetNowShuttleMode_2x8_8() 函數在 e2x8CloseEven 模式下的判斷邏輯錯誤**：

1. **重複檢查同一位置**：
   ```cpp
   // aoutarm9045_2x8_8.cpp 第 194 行（錯誤版本）
   else if(ptrOutSHT->Item[0][4]>=HAS_IC || ptrOutSHT->Item[0][4]>=HAS_IC)
       return 51000;  // 重複檢查 Item[0][4]，應該檢查 Item[0][6]
   ```

2. **遺漏關鍵位置**：
   - 實際 IC 在 `Item[0][6]`（RowA, Column 6）
   - 程式只檢查 `Item[0][4]`（重複兩次）
   - 未檢查 `Item[0][6]` → 判定失敗 → 進入 else → 錯誤回傳 51001

3. **後續連鎖反應**：
   - OutArmZNeedDown_2x8_8() 根據錯誤的 mode 設定 SetOutArmNeedDestory()
   - OutArm 移動到 RowB 位置（但 IC 在 RowA）
   - 吸取失敗但無錯誤處理 → 無限空轉

### 偶發條件

需同時滿足：

1. `iCloseSiteModeFor2x8 == e2x8CloseEven` 或相關 2x8 模式
2. Shuttle 上**只有單一 Row 有料**（RowA 或 RowB，不是兩行都有）
3. IC 位置在 **Column 4 或 Column 6**（右側兩列）

**注意**：函數 bug 本身就會觸發，不需要特殊操作。

### 修正方向

| 優先序 | 建議 |
|--------|------|
| **P0** | **修正 GetNowShuttleMode_2x8_8() 判斷邏輯**：<br>- 修正 `Item[0][4]` 重複檢查 → 改為檢查 `Item[0][6]`<br>- 補齊 RowB 的對應檢查：`Item[1][4]` 和 `Item[1][6]`<br>- 加入錯誤處理：若兩行都沒料則報 WAR |
| **P1** | **OutArm 取料失敗診斷**：<br>- PickFromShuttle 完成後檢查 OutArmSuck 是否至少一點有料<br>- 若子任務完成但 OutArmSuck 全 0 且 CarryKit 有料 → 拋出 WAR<br>- EventLog 記錄：GetNowShuttleMode 回傳值 vs 實際資料位置 |
| **P2** | **OutArm 無限空轉保護**：<br>- 加入取料重試計數器（連續 3 次失敗報 WAR）<br>- Shuttle state 10 超時保護（停留 > 30 秒報 WAR） |
| **P3** | **定期驗證機制**：<br>- 定期檢查 CarryKit 資料與 GetNowShuttleMode 回傳值是否一致<br>- 發現不一致時觸發診斷 EventLog 或 WAR |

### 案例

| 日期 | 客戶 | 機台 | 版本 | 測試模式 |
|------|------|------|------|---------||
| 2026-04-17 | CUSTOMER_CODE 910 | (測試機) | V3.33.902.1 | 16-Site 2x8 (e2x8CloseEven) |

### 相關程式碼

| 檔案 | 行號 | 內容 |
|------|------|------|
| `aoutarm9045_2x8_8.cpp` | ~68 | GetNowShuttleMode_2x8_8() 函數定義 |
| `aoutarm9045_2x8_8.cpp` | ~194 | ❌ **錯誤**：重複檢查 `Item[0][4]`，應為 `Item[0][6]` |
| `aoutarm9045_2x8_8.cpp` | ~234-235 | ⚠️ e2x8CloseEven1By1 模式也有類似問題 |
| `aoutarm9045_2x8_8.cpp` | ~458 | OutArmZNeedDown_2x8_8() 使用 GetNowShuttleMode 結果 |
| `aoutarm9045_2x8_8.cpp` | ~695 | MoveOutArmToShuttle_2x8_8() 使用 mode 計算位置 |
| `aoutarm9045_2x8_8.cpp` | ~1326 | DoPickFromShuttle_2x8_8() 使用 mode 執行取料 |

### 關鍵學習：如何從 StateRecord 推論此類問題

**分析步驟**：

1. **持料快照異常發現**：
   - OutArmSuck 全 0（沒拿到料）
   - 但 FRCarryKit 有料且位置明確（例如 Item[0][6] = 1002）
   - PickFromShuttle 子任務完成（回到 state 1）
   - → **取料動作執行了，但吸取位置錯誤**

2. **定位到位置判斷邏輯**：
   - OutArm 必須透過某函數決定移動到哪個 Row
   - 在 2x8 模式下，關鍵函數是 GetNowShuttleMode_2x8_8()
   - 回傳值格式：5XYZW（X=模式，Y=Column組，Z=Row編號）

3. **比對預期 vs 實際**：
   - **實際 IC 位置**：Item[0][6]（RowA, Column 6）
   - **應該回傳**：51000（表示 RowA）
   - **實際回傳**：51001（表示 RowB）
   - → **判斷邏輯錯誤**

4. **程式碼 Bug 模式**：
   - 重複檢查同一位置（Copy-Paste Bug）
   - 遺漏關鍵位置（Column 6 未檢查）
   - if-else 最後的 else 無條件假設某個結果

**推論技巧總結**：
- 當 OutArm 持續空轉但 Shuttle 有料時 → 優先查位置判斷邏輯
- 比對持料快照的實際位置 vs 函數回傳的預期位置
- 檢查 if-else 鏈中的重複條件或遺漏分支
- 注意 Copy-Paste Bug（相同變數名出現兩次）

---
<!-- 新增 Pattern 時，複製以下模板：

## Pattern #N：{簡短描述}

### 識別特徵

| 條件 | 值 |
|------|-----|
| TaskA | 值 |
| TaskB | 值 |
| EventLog | 事件 |
| Config | 旗標 |

### 時間線模式
### 根因
### 偶發條件
### 修正方向
### 案例
### 相關程式碼

-->
