<!-- AI(skill-merge) 20260501 (RogerYang): 採用 Steven 2026-04-28 版（含 r901→r902→r904 版本演進、cpp 範例），保留 Local Jerry Yang 2026-04-13 「+5mm 案例」整合至「診斷方式」章節 -->

# Flag 邊界（Single Site Other Suck）— OutArm 端

> 來源：Jerry Yang 2026-04-15；Ifor 2022-08-23；Steven 2026-04-28 更新設計規則

## 兩個旗標的作用範圍

| 旗標 | 影響模組 | 吸嘴 | 說明 |
|------|---------|------|------|
| `Prod.bSingleUseOtherSuck` | InArm + OutArm | E 吸嘴 | 單 Site 使用 E（Other）吸嘴的旗標 |
| `Prod.bSingleInArmUseOtherSuck` | InArm 主控、OutArm 作為否決條件 | G 吸嘴 | InArm 使用 G 吸嘴；OutArm 需用 G 對位補償（**否決 bSingleUseOtherSuck**） |

## 優先級規則（aoutarm9045_1x1_1.cpp）

當兩個旗標**同時為 true** 時，**`bSingleInArmUseOtherSuck` 優先**（InArm 用 G → OutArm 必須用 G 對位）。

```cpp
// 正確寫法（Ifor 20220823，Steven 20260428 整合進 r904）
if(Prod.bSingleUseOtherSuck && Prod.bSingleInArmUseOtherSuck==false)
    *iX = *iX - dMovePitchX;      // E 吸嘴位置
else
    *iX = *iX - dMovePitchX*2;    // G 吸嘴位置（預設）
```

## 版本演進

| 版本 | 條件 | 問題 |
|------|------|------|
| base r901（wei 20220823）| `if(bSingleUseOtherSuck \|\| bSingleInArmUseOtherSuck)` → E | **Bug**：bSingleInArmUseOtherSuck=true 也給 E，應給 G |
| r902 JerryYang 20260415 | `if(bSingleUseOtherSuck)` → E；`else if(bSingleInArmUseOtherSuck)` → G | **殘留 Bug**：兩個同時 true 時仍走第一個 if → E（應走 G） |
| r904 Ifor 20220823 整合 | `if(bSingleUseOtherSuck && bSingleInArmUseOtherSuck==false)` → E；`else` → G | ✅ 正確 |

## 設計規則

1. `bSingleInArmUseOtherSuck=true` 時，OutArm **一律**使用 G 對位（不論 `bSingleUseOtherSuck` 值）。
2. 只有在 `bSingleUseOtherSuck=true` **且** `bSingleInArmUseOtherSuck=false` 時，OutArm 才使用 E 對位。
3. InArm 端的 `iWhichShtPickFor32` 設定：FLCarryKit=0（Shuttle1），BLCarryKit=1（Shuttle2）— 固定值，不使用 `iWhichSht`。

## Regression Warning（1x1 模式）

- 兩個旗標同時為 true 若走 E 補償，會造成 OutArm 對位偏移（約 +dMovePitchX）。
- 若在 1x1 模式發現 IC 放置偏移，檢查 `if(bSingleUseOtherSuck)` 條件是否缺少 `&& bSingleInArmUseOtherSuck==false`。

### 診斷方式（實際案例參照）

若在 1x1 模式 + InArm C/G 吸嘴設定下，發現 OutArm 對位偏移約 **+5mm**（或 ±dMovePitchX 量級），直接檢查 `aoutarm9045_1x1_1.cpp` 內 Shuttle X/Y 補償判斷式：

- ❌ 錯誤寫法：`if(Prod.bSingleUseOtherSuck)` → 兩旗標同時 true 時走 E 補償，造成 G 吸嘴需要的位置偏掉一個 pitch
- ✅ 正確寫法：`if(Prod.bSingleUseOtherSuck && Prod.bSingleInArmUseOtherSuck==false)`

對照本檔「版本演進」表，可快速判斷該機台 OutArm 程式碼是 r901 / r902 / r904 哪一版。

> Local 案例（Jerry Yang 2026-04-13 觀察）：1x1 + InArm C 吸嘴情境下需手補 X +5mm，已對應到 r902 殘留 Bug，r904 修正後不需手補。
