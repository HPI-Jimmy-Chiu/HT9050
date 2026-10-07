> 保存來源：`.claude/skills/ht9045-uph-model/references/uph-formula-worked-example.md`，main `76dd45f37`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# UPH 公式 Worked Example

## Case 1：QFP 7×7、10×25 Tray、2D Disabled、Test Time 10s

### 輸入條件

| 項目 | 值 |
|------|---|
| Package | QFP 7×7 |
| Tray X×Y | 10 × 25 = 250 pcs |
| Sites/Index | 4 |
| HotPlate | 啟用，容量 16 pcs |
| 2D Enable | No |
| Soak Time | 8 sec |
| Start Delay | 0.5 sec |
| Test Time | 10 sec |
| Yield | 95% |
| InArm Speed | 80% |
| Index Speed | 90% |
| OutArm Speed | 80% |

### 各段時間（假設由 MotorProfiler CSV 查表得）

| 段 | 時間 (sec) | 主要動作 |
|----|-----------|---------|
| $T_{IA}$ | 3.2 | 4 顆 Sht↔Loader cycle |
| $T_{HP}/N_{HP}$ | 8 / 16 = 0.5 | Soak 攤提到 16 顆 |
| $T_{IDX}$ | 1.8 | Sht pick + Drop + release |
| $T_{SD}$ | 0.5 | Initial Delay |
| $T_{TT}$ | 10 | Tester EOT |
| $T_{OA}$ | 3.5 | 4 顆 Sht↔Tray cycle |
| $T_{LT}$ | 30 | Tray Arm 補 Loader |
| $T_{ULT}$ | 30 | Tray Arm 退 Unloader |

### 套公式

**規則 1**：$T_{IA\_eff} = \max(3.2, 0.5) = 3.2$ sec（Soak 完全吸收）

**規則 2**：$T_{cycle} = \max(3.2, 1.8 + 0.5 + 10, 3.5) = \max(3.2, 12.3, 3.5) = 12.3$ sec（Index+TT 主導）

**規則 3**：$T_{LT}/N_{Tray} = 30/250 = 0.12$ sec、$T_{ULT}/N_{Tray} = 0.12$ sec → 完全藏起來

$T_{cycle}' = 12.3$ sec

**UPH** = 3600 / 12.3 × 4 × 0.95 ≈ **1112 pcs/hr**

---

## Case 2：QFN 3×3、14×35 Tray、2D Enabled、TT=5s

### 輸入

| 項目 | 值 |
|------|---|
| Tray | 14 × 35 = 490 pcs |
| Sites | 8 |
| 2D | Yes（影響 InArm 多一段 Bottom 2D） |
| Soak | 0 |
| TT | 5 sec |
| Yield | 98% |

### 各段（查表）

| 段 | 時間 |
|----|------|
| $T_{IA}$（含 Bottom 2D） | 4.5 |
| $T_{IDX}$ | 1.6 |
| $T_{SD}$ | 0.3 |
| $T_{TT}$ | 5.0 |
| $T_{OA}$ | 3.8 |
| $T_{LT}$ | 30 |

### 計算

- $T_{IA\_eff}$ = 4.5（無 HotPlate）
- $T_{cycle}$ = max(4.5, 1.6+0.3+5.0, 3.8) = max(4.5, 6.9, 3.8) = **6.9**（Index+TT 主導）
- $T_{LT}/490$ ≈ 0.06 → 不影響

**UPH** = 3600 / 6.9 × 8 × 0.98 ≈ **4090 pcs/hr**

---

## Case 3：極短 TT 案例（驗證 Tray Arm 何時主導）

### 輸入

| 項目 | 值 |
|------|---|
| Tray | QFN 3×3、5×5 = 25 pcs（小盤） |
| Sites | 8 |
| TT | 0.5 sec |
| Yield | 100% |
| $T_{IA}$ | 1.0 |
| $T_{IDX}$ | 1.6 |
| $T_{OA}$ | 1.0 |
| $T_{LT}$ | 30 |

### 計算

- $T_{cycle}$ = max(1.0, 1.6+0+0.5, 1.0) = **2.1**
- $T_{LT}/25$ = **1.2** → 不影響（< 2.1）

**UPH** = 3600 / 2.1 × 8 × 1.0 ≈ **13714 pcs/hr**

若 tray 改成 25×1 = 5 pcs（極小）：
- $T_{LT}/5$ = 6.0 → **Tray Arm 主導**
- UPH = 3600 / 6.0 × 8 = 4800 pcs/hr（掉 65%）

---

## 客戶要求格子產生方式

以 Case 1 的條件框架，掃描 (Test Time, 2D, Soak) 三維：

| Package | TrayXY | 2D | Soak | TT=5s | TT=10s | TT=15s | TT=20s | TT=40s |
|---------|--------|----|----|-----|-----|-----|-----|------|
| QFP 7×7 | 10×25 | OFF | 16s | UPH₁ | UPH₂ | ... | ... | ... |
| QFP 7×7 | 10×25 | OFF | 32s | ... | ... | ... | ... | ... |
| QFP 7×7 | 10×25 | ON  | 16s | ... | ... | ... | ... | ... |
| ... | ... | ... | ... | ... | ... | ... | ... | ... |

對應你貼的那張 5×4×8 = 160 格 Excel，每格代入公式一次即可，
Excel 端用 `UPH_Calc` sheet 的單格公式拉滿即得整張表。

<!-- preserved-content:end -->
