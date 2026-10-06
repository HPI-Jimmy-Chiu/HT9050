> 保存來源：`.claude/skills/ht9045-motor-spatial-layout/references/groups/G20-B-HT9045-HPP0-SMC.md`，main `eb7f47e7f`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../../common.md)。

<!-- preserved-content:start -->
# G20 — B · `HT9045`

> **群組**：B (HT-9045 標準機 (6-site GPIB))  
> **HPP**：HPP=0 (窄 HP)  
> **GearRatio**：GearY~0.350(SMC)  
> **樣本數**：3 筆  
> [← 返回索引](../teach-position-statistics.md)

## G20 · `HT9045` · HPP=0 (窄 HP ~-46000) · GearY~0.350(SMC)

- **機台群組**：B （HT-9045 標準機 (6-site GPIB)）
- **SubModel 分布**：0×3
- **Picker (USE_PICKER_COUNT) 分布**：1×3
- **Y Pitch (USE_IN_OUT_ARM_Y_PITCH) 分布**：1×2, 0×1
- **GPIB Model 分布**：`(無)`×2, `9045GPIB`×1
- **樣本數**：3 筆 state record
- **獨立機台數**（依 Serial No 計）：3 台

### 機台清單

| Serial No | Factory | 樣本數 |
|-----------|---------|--------|
| `PMLJ1039` |  | 1 |
| `1206` | KYEC | 1 |
| `FLD759` | KYEC | 1 |

### 來源 State Records（最多列 15 筆）

- `SPIL\2023-08-25 22_08_09` &nbsp;*(S/N: 1206)*
- `SPIL\2025-07-16 11_02_46` &nbsp;*(S/N: FLD759)*
- `復旦微\2026-04-08 15_22_48` &nbsp;*(S/N: PMLJ1039)*

### Teach Position 統計

#### `[MInArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditInXPitch120` | 1 | 42 | 42 | 42 | 1 | 42×1 |
| `setEditInXPitch40` | 1 | -5309 | -5309 | -5309 | 1 | -5309×1 |

#### `[MInArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanX` | 1 | 11349 | 11349 | 11349 | 1 | 11349×1 |
| `setEditHP1X` | 1 | 1454 | 1454 | 1454 | 1 | 1454×1 |
| `setEditHP2X` | 1 | 1454 | 1454 | 1454 | 1 | 1454×1 |
| `setEditInSht1X` | 1 | 31989 | 31989 | 31989 | 1 | 31989×1 |
| `setEditInSht2X` | 1 | 32017 | 32017 | 32017 | 1 | 32017×1 |
| `setEditLoaderX` | 1 | 20913 | 20913 | 20913 | 1 | 20913×1 |
| `setEditRotateX` | 1 | 31804 | 31804 | 31804 | 1 | 31804×1 |
| `setInPickX` | 1 | 21386 | 21386 | 21386 | 1 | 21386×1 |

#### `[MInArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanY` | 1 | -81761 | -81761 | -81761 | 1 | -81761×1 |
| `setEditHP1Y` | 1 | -46487 | -46487 | -46487 | 1 | -46487×1 |
| `setEditHP2Y` | 1 | -7433 | -7433 | -7433 | 1 | -7433×1 |
| `setEditInSht1Y` | 1 | -37781 | -37781 | -37781 | 1 | -37781×1 |
| `setEditInSht2Y` | 1 | -5789 | -5789 | -5789 | 1 | -5789×1 |
| `setEditLoaderY` | 1 | -55928 | -55928 | -55928 | 1 | -55928×1 |
| `setEditRotateY` | 1 | -21724 | -21724 | -21724 | 1 | -21724×1 |
| `setInPickY` | 1 | -57890 | -57890 | -57890 | 1 | -57890×1 |

#### `[MInArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditHP` | 1 | -1890 | -1890 | -1890 | 1 | -1890×1 |
| `SetEditPickInRotate` | 1 | -1400 | -1400 | -1400 | 1 | -1400×1 |
| `SetEditPickLoader` | 1 | -1750 | -1750 | -1750 | 1 | -1750×1 |
| `SetEditPlaceInRotate` | 1 | -1350 | -1350 | -1350 | 1 | -1350×1 |
| `SetEditPlaceInShuttle` | 1 | -1920 | -1920 | -1920 | 1 | -1920×1 |

#### `[MInRotate]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditRotateA` | 1 | 1323 | 1323 | 1323 | 1 | 1323×1 |

#### `[MInShuttle1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht1OctSiteKit` | 1 | 11000 | 11000 | 11000 | 1 | 11000×1 |
| `edtSetEditIS1BarCode` | 1 | 16720 | 16720 | 16720 | 1 | 16720×1 |
| `edtSetEditOS1BarCode` | 1 | 25500 | 25500 | 25500 | 1 | 25500×1 |
| `setEditInSht1Left` | 1 | 90 | 90 | 90 | 1 | 90×1 |
| `setEditInSht1Right` | 1 | 38551 | 38551 | 38551 | 1 | 38551×1 |
| `setEditOutSht1KitPos` | 1 | 20524 | 20524 | 20524 | 1 | 20524×1 |
| `setEditOutSht1OneRowKit` | 1 | 20531 | 20531 | 20531 | 1 | 20531×1 |

#### `[MInShuttle2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht2OctSiteKit` | 1 | 11000 | 11000 | 11000 | 1 | 11000×1 |
| `edtSetEditIS2BarCode` | 1 | 16320 | 16320 | 16320 | 1 | 16320×1 |
| `edtSetEditOS2BarCode` | 1 | 25502 | 25502 | 25502 | 1 | 25502×1 |
| `setEditInSht2Left` | 1 | 14 | 14 | 14 | 1 | 14×1 |
| `setEditInSht2Right` | 1 | 38482 | 38482 | 38482 | 1 | 38482×1 |
| `setEditOutSht2KitPos` | 1 | 20389 | 20389 | 20389 | 1 | 20389×1 |
| `setEditOutSht2OneRowKit` | 1 | 20431 | 20431 | 20431 | 1 | 20431×1 |

#### `[MOutArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditOutXPitch120` | 1 | 42 | 42 | 42 | 1 | 42×1 |
| `setEditOutXPitch40` | 1 | -5287 | -5287 | -5287 | 1 | -5287×1 |

#### `[MOutArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1X` | 1 | -53550 | -53550 | -53550 | 1 | -53550×1 |
| `setEditAuto2X` | 1 | -35076 | -35076 | -35076 | 1 | -35076×1 |
| `setEditAuto3X` | 1 | -16579 | -16579 | -16579 | 1 | -16579×1 |
| `setEditFix1X` | 1 | -35573 | -35573 | -35573 | 1 | -35573×1 |
| `setEditFix2X` | 1 | -21465 | -21465 | -21465 | 1 | -21465×1 |
| `setEditFix3X` | 1 | -7360 | -7360 | -7360 | 1 | -7360×1 |
| `setEditOutSht1X` | 1 | -48853 | -48853 | -48853 | 1 | -48853×1 |
| `setEditOutSht2X` | 1 | -48873 | -48873 | -48873 | 1 | -48873×1 |
| `setEditRotateOutX` | 1 | -49630 | -49630 | -49630 | 1 | -49630×1 |
| `setOutPickX` | 1 | -53582 | -53582 | -53582 | 1 | -53582×1 |

#### `[MOutArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1Y` | 1 | -55768 | -55768 | -55768 | 1 | -55768×1 |
| `setEditAuto2Y` | 1 | -55831 | -55831 | -55831 | 1 | -55831×1 |
| `setEditAuto3Y` | 1 | -55801 | -55801 | -55801 | 1 | -55801×1 |
| `setEditFix1Y` | 1 | -10382 | -10382 | -10382 | 1 | -10382×1 |
| `setEditFix2Y` | 1 | -10440 | -10440 | -10440 | 1 | -10440×1 |
| `setEditFix3Y` | 1 | -10426 | -10426 | -10426 | 1 | -10426×1 |
| `setEditOutSht1Y` | 1 | -37759 | -37759 | -37759 | 1 | -37759×1 |
| `setEditOutSht2Y` | 1 | -5753 | -5753 | -5753 | 1 | -5753×1 |
| `setEditRotateOutY` | 1 | -21733 | -21733 | -21733 | 1 | -21733×1 |
| `setOutPickY` | 1 | -57624 | -57624 | -57624 | 1 | -57624×1 |

#### `[MOutArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditPickOutRotate` | 1 | -1400 | -1400 | -1400 | 1 | -1400×1 |
| `SetEditPickOutSht` | 1 | -1970 | -1970 | -1970 | 1 | -1970×1 |
| `SetEditPlaceAuto` | 1 | -1600 | -1600 | -1600 | 1 | -1600×1 |
| `SetEditPlaceFix` | 1 | -1620 | -1620 | -1620 | 1 | -1620×1 |
| `SetEditPlaceOutRotate` | 1 | -1350 | -1350 | -1350 | 1 | -1350×1 |

#### `[MOutRotate]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditRotateOutA` | 1 | 1390 | 1390 | 1390 | 1 | 1390×1 |

#### `[MTestY1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Y` | 1 | -112 | -112 | -112 | 1 | -112×1 |
| `setEditIndex1ToSocketY` | 1 | 15893 | 15893 | 15893 | 1 | 15893×1 |

#### `[MTestY2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Y` | 1 | 143 | 143 | 143 | 1 | 143×1 |
| `setEditIndex2ToSocketY` | 1 | -15858 | -15858 | -15858 | 1 | -15858×1 |

#### `[MTestZ1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Z` | 1 | -4350 | -4350 | -4350 | 1 | -4350×1 |
| `setEditTestZSafePos` | 1 | 10 | 10 | 10 | 1 | 10×1 |
| `setEditWaitTestZDown` | 1 | -1000 | -1000 | -1000 | 1 | -1000×1 |

#### `[MTestZ2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Z` | 1 | -4320 | -4320 | -4320 | 1 | -4320×1 |
| `setEditTestZSafePos` | 1 | 10 | 10 | 10 | 1 | 10×1 |

#### `[MTrayX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditTrayAuto1X` | 1 | 78640 | 78640 | 78640 | 1 | 78640×1 |
| `setEditTrayAuto2X` | 1 | 97060 | 97060 | 97060 | 1 | 97060×1 |
| `setEditTrayAuto3X` | 1 | 115475 | 115475 | 115475 | 1 | 115475×1 |
| `setEditTrayColorX` | 1 | 49530 | 49530 | 49530 | 1 | 49530×1 |
| `setEditTrayEmptyX` | 1 | 27925 | 27925 | 27925 | 1 | 27925×1 |
| `setEditTrayLoaderX` | 1 | -4680 | -4680 | -4680 | 1 | -4680×1 |

#### `[InArm]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `AutoCleanPick` | 3 | -1900 | -1700 | -1480 | 3 | -1480×1, -1900×1, -1700×1 |

#### `[InArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 1 | -38 | -38 | -38 | 1 | -38×1 |
| `Picker Ab` | 1 | 9 | 9 | 9 | 1 | 9×1 |
| `Picker Ad` | 1 | 21 | 21 | 21 | 1 | 21×1 |
| `Picker Ba` | 1 | -19 | -19 | -19 | 1 | -19×1 |
| `Picker Bb` | 1 | 14 | 14 | 14 | 1 | 14×1 |
| `Picker Bc` | 1 | -15 | -15 | -15 | 1 | -15×1 |
| `Picker Bd` | 1 | -12 | -12 | -12 | 1 | -12×1 |

#### `[OutArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 1 | -10 | -10 | -10 | 1 | -10×1 |
| `Picker Ab` | 1 | -18 | -18 | -18 | 1 | -18×1 |
| `Picker Ad` | 1 | -18 | -18 | -18 | 1 | -18×1 |
| `Picker Ba` | 1 | -2 | -2 | -2 | 1 | -2×1 |
| `Picker Bb` | 1 | 10 | 10 | 10 | 1 | 10×1 |
| `Picker Bc` | 1 | 4 | 4 | 4 | 1 | 4×1 |
| `Picker Bd` | 1 | 37 | 37 | 37 | 1 | 37×1 |

#### `[Teach INI]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Update2` | 1 | 1 | 1 | 1 | 1 | 1×1 |

<!-- preserved-content:end -->
