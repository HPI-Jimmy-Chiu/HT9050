# InArm 變體完整對照表 (V3.33.899.0)

## iInArmType → 函式 → 配置

| # | iInArmType 列舉值 | 函式名稱 | 配置 (Row × Col × Sucker) | 原始檔 |
|---|---|---|---|---|
| 1 | `e9045_1x1_1` | `DoInArm_9045_All_1Pick()` | 1×1, 1 pick | ainarm9045_All_1Pick.cpp |
| 2 | `e9045_1x2_2_13` | `DoInArm_9045_1x2_2()` | 1×2, 2 suck | ainarm9045_1x2_2.cpp |
| 3 | `e9045_1x2_2_14` | `DoInArm_9045_1x2_2_14()` | 1×2, 2 suck (site 1-4) | ainarm9045_1x2_2_14.cpp |
| 4 | `e9045_1x2_4_Hot` | `DoInArm_9045_1x2_4_Hot()` | 1×2, 4 suck (HP) | ainarm9045_1x2_4_Hot.cpp |
| 5 | `e9045_1x3_2_14` | `DoInArm_9045_1x3_2_14()` | 1×3, 2 suck (site 1-4) | ainarm9045_1x3_2_14.cpp |
| 6 | `e9045_1x3_4` | `DoInArm_9045_1x3_4()` | 1×3, 4 suck | ainarm9045_1x3_4.cpp |
| 7 | `e9045_1x4_1_Ac` | `DoInArm_9045_1x4_1_Ac()` | 1×4, 1 pick (AutoClean) | ainarm9045_1x4_4.cpp |
| 8 | `e9045_1x4_4_13` | `DoInArm_9045S_1x4_4()` | 1×4, 4 suck (S 系列) | ainarm9045S_1x4_4.cpp |
| 9 | `e9045_1x4_4` | `DoInArm_9045_1x4_4()` | 1×4, 4 suck | ainarm9045_1x4_4.cpp |
| 10 | `e9045_1x4_4_Back` | `DoInArm_9045_1x4_4_Back()` | 1×4, 4 suck (Back) | ainarm9045_1x4_4_Back.cpp |
| 11 | `e9045_1x4_2_14` | `DoInArm_9045_1x4_2()` | 1×4, 2 suck | ainarm9045_1x4_2.cpp |
| 12 | `e9045_1x4_8_Hot` | `DoInArm_9045_1x4_8_Hot()` | 1×4, 8 suck (HP) | ainarm9045_1x4_8_Hot.cpp |
| 13 | `e9045_2x1_2_13` | `DoInArm_9045_2x1_2()` | 2×1, 2 suck | ainarm9045_2x1_2.cpp |
| 14 | `e9045_2x2_4_12` | `DoInArm_9045_2x2_4_12()` | 2×2, 4 suck (site 1-2) | ainarm9045_2x2_4_12.cpp |
| 15 | `e9045_2x2_4_13` | `DoInArm_9045_2x2_4()` | 2×2, 4 suck | ainarm9045_2x2_4.cpp |
| 16 | `e9045_2x2_4_14` | `DoInArm_9045_2x2_4_14()` | 2×2, 4 suck (site 1-4) | ainarm9045_2x2_4_14.cpp |
| 17 | `e9045_2x2_8_Hot` | `DoInArm_9045_2x2_8_Hot()` | 2×2, 8 suck (HP) | ainarm9045_2x2_8_Hot.cpp |
| 18 | `e9045_2x3_6_14` | `DoInArm_9045_2x3_6_14()` | 2×3, 6 suck | ainarm9045_2x3_6_14.cpp |
| 19 | `e9045_2x3_6` | `DoInArm_9045_2x3_6()` | 2×3, 6 suck | ainarm9045_2x3_6.cpp |
| 20 | `e9045_2x4_4_13` | `DoInArm_9045_2x4_4_13()` | 2×4, 4 suck | ainarm9045S_2x4_4_13.cpp |
| 21 | `e9045_2x4_4_14` | `DoInArm_9045_2x4_4()` | 2×4, 4 suck | ainarm9045_2x4_4.cpp |
| 22 | `e9045_2x4_8` | `DoInArm_9045_2x4_8()` | 2×4, 8 suck | ainarm9045_2x4_8.cpp |
| 23 | **`e9045_2x4_16`** | **`DoInArm_9045_2x4_16()`** | **2×4, 16 suck** | **ainarm9045_2x4_16.cpp** |
| 24 | `e9045_2x5_8` | `DoInArm_9045_2x5_8()` | 2×5, 8 suck | ainarm9045_2x5_8.cpp |
| 25 | `e9045_2x6_8` | `DoInArm_9045_2x6_8()` | 2×6, 8 suck | ainarm9045_2x6_8.cpp |
| 26 | `e9045_2x8_8` | `DoInArm_9045_2x8_8()` | 2×8, 8 suck | ainarm9045_2x8_8.cpp |
| 27 | `e9045_2x8_32` | `DoInArm_9045_2x8_8()` | 2×8, 32 suck (shared) | ainarm9045_2x8_32.cpp |

## InArm 2x4-16 Sucker 配置

```cpp
InArmSuck.SetPickerCount(2, 8, 2, 4, 1, 0, 0);
// Row=2, Col=8 物理; 使用 2×4=16 邏輯 site
// 內部用 Prod.fInArmSuckUse_TwoArm32Site[][] 映射
```

## OutArm 變體（相同配置概念）

OutArm 對應相同的 site 配置命名，函式前綴為 `DoOutArm_9045_*`。
例如 `aoutarm9045_2x4_16.cpp` 對應 16-site OutArm 流程。
