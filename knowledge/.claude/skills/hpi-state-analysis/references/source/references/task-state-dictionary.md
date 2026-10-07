> 保存來源：`.claude/skills/ht9045-state-record-analysis/references/task-state-dictionary.md`，main `811d95068`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Task 狀態值字典

## 格式說明

每個模組列出已知的狀態值及其語義。
- `[confirmed]` — 已從原始碼驗證
- `[inferred]` — 從行為/案例推斷
- `[partial]` — 部分驗證，可能有子分支

> 此文件為累積式文件。每次新案例分析後，將新發現的狀態值補入。

---

## AutoSHT1Task / AutoSHT2Task

**原始碼**：`acarry.cpp` — `Do_Auto_SHT1()` / `Do_Auto_SHT2()`
**正常循環**：1 → 10 → 100 → 120 → 1（左移）；1 → 200 → 202 → 210 → 1（右移）

| 值 | 語義 | 備註 | 確認 |
|----|------|------|------|
| 1 | idle（空閒等待） | 等待條件滿足後開始移動 | [confirmed] |
| 10 | 準備向左移動 | 檢查前置條件 | [confirmed] |
| 100 | 向左移動中 | 馬達移動執行中 | [confirmed] |
| 120 | 向左移動完成 | 到達左側位置 | [confirmed] |
| 200 | 準備向右移動 | 檢查前置條件 | [confirmed] |
| 202 | 向右移動等待 | 等待安全條件（如 TestZ 安全高度） | [confirmed] |
| 210 | 等待向右移動 | **常見阻塞點**：被 `IsTestZ{1,2}NotSafe...CanNotMove()` 擋住 | [confirmed] |

**死鎖常見狀態**：**210**（被 TestZ 安全互鎖永久阻擋）

---

## InArmTask

**原始碼**：`ainarm9045S_2x4_4_13.cpp`（及其他 ainarm*.cpp 變體）
**正常循環**：50 → 100 → 400 → 2000 → 50

| 值 | 語義 | 備註 | 確認 |
|----|------|------|------|
| 1 | idle / 完全閒置 | 無動作 | [confirmed] |
| 50 | 入口準備 | 檢查是否有 IC 需要取料 | [confirmed] |
| 100 | 取料中 | 從 Loader / HotPlate 取 IC | [confirmed] |
| 400 | 放料中 | 放 IC 到 Shuttle | [confirmed] |
| 2000 | 等待 Shuttle 清空 | **常見阻塞點**：等待 Shuttle 到左側且 IC 被 Index 取走 | [confirmed] |

**死鎖常見狀態**：**2000**（Shuttle 被擋 → InArm 永遠等不到清空）

---

## InArmPlaceToShuttleTask

**原始碼**：`ainarm9045S_2x4_4_13.cpp`（InArm 放料到 Shuttle 的子任務）
**正常循環（SHT1）**：1 → 900 → 1100 → 1200 → 1300 → 1400 → 1500 → 1
**正常循環（SHT2）**：1 → 1900 → 2100 → 2200 → 2300 → 2400 → 2500 → 1

| 值 | 語義 | 備註 | 確認 |
|----|------|------|------|
| 1 | idle | 等待 InArmTask 觸發 | [confirmed] |
| 900 | SHT1 放料準備 | — | [confirmed] |
| 930 | SHT1 放料等待 | 等待 Shuttle1 到位 | [confirmed] |
| 950 | SHT1 放料完成前 | — | [confirmed] |
| 1000 | SHT1 等待中間狀態 | 可能在等待 Shuttle 移動 | [confirmed] |
| 1050 | SHT1 快速循環等待 | 與 1000 交替出現（等待條件） | [confirmed] |
| 1100 | SHT1 移動到放料位 | — | [confirmed] |
| 1200 | SHT1 Z 軸下降 | — | [confirmed] |
| 1300 | SHT1 放料 | 真空釋放 | [confirmed] |
| 1400 | SHT1 Z 軸上升 | — | [confirmed] |
| 1500 | SHT1 放料完成 | 回到 idle | [confirmed] |
| 1900 | SHT2 放料準備 | — | [confirmed] |
| 1910 | SHT2 放料等待（前） | — | [confirmed] |
| 1930 | SHT2 放料等待（後） | — | [confirmed] |
| 1950 | SHT2 放料完成前 | — | [confirmed] |
| 2000 | SHT2 等待中間狀態 | — | [confirmed] |
| 2100 | SHT2 移動到放料位 | — | [confirmed] |
| 2200 | SHT2 Z 軸下降 | — | [confirmed] |
| 2300 | SHT2 放料 | — | [confirmed] |
| 2400 | SHT2 Z 軸上升 | — | [confirmed] |
| 2500 | SHT2 放料完成 | — | [confirmed] |

---

## OutArmTask

**原始碼**：`aoutarm9045_2x4_4.cpp`（及其他 aoutarm*.cpp 變體）

| 值 | 語義 | 備註 | 確認 |
|----|------|------|------|
| 1 | 初始化 | 設定 bSortingAllBinTrayFinish=false | [confirmed] |
| 5 | 移動到 auto safe | — | [confirmed] |
| 10 | 檢查初始狀態 | — | [confirmed] |
| 50 | idle 等待 | 等待 IC 準備或返回安全位 | [confirmed] |
| 100 | 判斷取料來源 | Shuttle 1 或 2 | [confirmed] |
| 200-300 | 移動到 shuttle safe | — | [confirmed] |
| 1000 | SHT1 取料路徑初始化 | — | [confirmed] |
| 1100 | **SHT1 idle/等待** | **常見阻塞點**：等待 SHT1 到右側 + IC 就緒 | [confirmed] |
| 1150 | 移動到 SHT1（含 Z） | — | [confirmed] |
| 1200 | 從 SHT1 取 IC | — | [confirmed] |
| 2000 | SHT2 取料路徑初始化 | — | [confirmed] |
| 2100 | SHT2 idle/等待 | — | [confirmed] |
| 2200 | 從 SHT2 取 IC | — | [confirmed] |
| 3000 | 取料後移動到 auto safe | — | [confirmed] |
| 3010 | 放 IC 到 auto tray | — | [confirmed] |
| 3100-3301 | 驗證 tray 狀態 + 執行放料 | — | [confirmed] |
| 3500 | 放料後處理 | — | [confirmed] |
| 4000-4100 | Fix tray 滿盤處理 | — | [confirmed] |
| 7000 | OutArm 附加功能 | AOI、Rotator 等 | [confirmed] |

**死鎖常見狀態**：**1100**（SHT1 沒有 IC 送達，所有前置條件失敗）

### OutArm 空轉迴圈（案例補充）

在 2026-03-27 個案中，觀察到 OutArm 主流程常見循環：

`50 -> 100 -> 1140 -> 1200 -> 50`

同時 `iPickFromShuttle1Task` 會完成一輪循環（`1 -> 10 -> 200 -> 1000 -> 1`），
但 `OutArmSuck` 仍維持全 0，表示「子任務完成」不等於「資料轉移成功」。

實務判讀：
- 若 `FRCarryKit`/`BRCarryKit` 有料、`OutArmSuck` 無料，且 OutArm 主流程反覆回到 50，
	應優先查 OutArm pick side/吸嘴配置/真空條件，而不是先懷疑 Shuttle 馬達。

---

## iTestTask (TestTask)

**原始碼**：`atester.cpp` — `GetTesterResult()`
**正常循環**：50 → 55 → 60 → (處理結果) → 50

| 值 | 語義 | 備註 | 確認 |
|----|------|------|------|
| 20 | 準備 | 檢查 `SystemStart==true`，否則 break | [confirmed] |
| 50 | 準備送測 | 等待 Index 下壓完成等條件 | [confirmed] |
| 55 | 送出測試指令 | `RunTestProgram()` 設 `bEcho=false`，若 `SoftStop` 則 break | [confirmed] |
| 60 | **等待 bEcho** | **常見阻塞點**：等待 GPIB 回傳 BINON；需要 `DoTestHeadMotor()` 被呼叫才會被驅動 | [confirmed] |

**死鎖常見狀態**：**60**（bEcho 已設 true 但 DoTestHeadMotor 被 SoftStop 門控阻擋）

### bEcho 生命週期
- `false` ← `RunTestProgram()`（main.cpp ~17738）— 送測時清除
- `true` ← `OnMyCopyMsg()` 收到 BINON（main.cpp ~16517）— GPIB 回傳時設定
- 消費 ← `atester.cpp` case 60 — 讀取後處理結果

---

## OneCycleTask

**原始碼**：`csystem.cpp`

| 值 | 語義 | 備註 | 確認 |
|----|------|------|------|
| 0 | 無 One Cycle | 正常狀態 | [confirmed] |
| 3 | One Cycle 執行中 | 操作員按了 ONE CYCLE | [confirmed] |

---

## iLifterTask[x][y]

**原始碼**：Lifter 模組

| 值 | 語義 | 備註 | 確認 |
|----|------|------|------|
| 100 | Lifter 動作中 | — | [inferred] |
| 200 | Lifter 完成 | — | [inferred] |

---

## 待補充模組

以下模組在後續案例中遇到時再補充：

- `iCatchFromLoaderTask` — CatchTray 補盤任務
- `iCatchNewTrayFromBufferTask` — 從 Buffer 取空盤
- `iPlaceTrayToAutoTask` — 放盤到 Auto
- `DoTestYFront` / `DoTestYRear` — Index 下壓（Front/Rear）
- `iSortArmTask` — SortArm（HT9046AU）

<!-- preserved-content:end -->
