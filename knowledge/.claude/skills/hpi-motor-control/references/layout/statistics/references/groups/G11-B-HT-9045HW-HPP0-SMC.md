> 保存來源：`.claude/skills/ht9045-motor-spatial-layout/references/groups/G11-B-HT-9045HW-HPP0-SMC.md`，main `eb7f47e7f`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../../common.md)。

<!-- preserved-content:start -->
# G11 — B · `HT-9045HW`

> **群組**：B (HT-9045 標準機 (6-site GPIB))  
> **HPP**：HPP=0 (窄 HP)  
> **GearRatio**：GearY~0.350(SMC)  
> **樣本數**：6 筆  
> [← 返回索引](../teach-position-statistics.md)

## G11 · `HT-9045HW` · HPP=0 (窄 HP ~-46000) · GearY~0.350(SMC)

- **機台群組**：B （HT-9045 標準機 (6-site GPIB)）
- **SubModel 分布**：0×6
- **Picker (USE_PICKER_COUNT) 分布**：1×6
- **Y Pitch (USE_IN_OUT_ARM_Y_PITCH) 分布**：0×5, 1×1
- **GPIB Model 分布**：`(無)`×3, `9045GPIB`×3
- **樣本數**：6 筆 state record
- **獨立機台數**（依 Serial No 計）：3 台

### 機台清單

| Serial No | Factory | 樣本數 |
|-----------|---------|--------|
| `1378` | KYEC | 4 |
| `DLD255` | KYEC | 1 |
| `DLD256` | KYEC | 1 |

### 來源 State Records（最多列 15 筆）

- `JSCC\2025-01-22 18_25_50` &nbsp;*(S/N: DLD256)*
- `JSCC\rt site map` &nbsp;*(S/N: DLD255)*
- `確安\2024-08-30 14_03_19 piggy back异常-` &nbsp;*(S/N: 1378)*
- `確安\2024-12-27 15_35_57 site mapping异常` &nbsp;*(S/N: 1378)*
- `確安\2025-09-02 09_18_15` &nbsp;*(S/N: 1378)*
- `確安\2025-09-19 16_21_05` &nbsp;*(S/N: 1378)*

### Teach Position 統計

#### `[MInArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditInXPitch120` | 3 | 42 | 42 | 42 | 1 | 42×3 |
| `setEditInXPitch40` | 3 | -5272 | -5272 | -5272 | 1 | -5272×3 |

#### `[MInArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanX` | 3 | 11789 | 11789 | 11789 | 1 | 11789×3 |
| `setEditHP1X` | 3 | 1842 | 1842 | 1842 | 1 | 1842×3 |
| `setEditHP2X` | 3 | 1857 | 1857 | 1857 | 1 | 1857×3 |
| `setEditInSht1X` | 3 | 32471 | 32471 | 32471 | 1 | 32471×3 |
| `setEditInSht2X` | 3 | 32463 | 32463 | 32463 | 1 | 32463×3 |
| `setEditLoaderX` | 3 | 21367 | 21367 | 21367 | 1 | 21367×3 |
| `setEditRotateX` | 3 | 31880 | 31880 | 31880 | 1 | 31880×3 |
| `setInPickX` | 3 | 21386 | 21386 | 21386 | 1 | 21386×3 |

#### `[MInArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanY` | 3 | -81765 | -81765 | -81765 | 1 | -81765×3 |
| `setEditHP1Y` | 3 | -46466 | -46466 | -46466 | 1 | -46466×3 |
| `setEditHP2Y` | 3 | -7505 | -7505 | -7505 | 1 | -7505×3 |
| `setEditInSht1Y` | 3 | -37730 | -37730 | -37730 | 1 | -37730×3 |
| `setEditInSht2Y` | 3 | -5755 | -5755 | -5755 | 1 | -5755×3 |
| `setEditLoaderY` | 3 | -55962 | -55962 | -55962 | 1 | -55962×3 |
| `setEditRotateY` | 3 | -21796 | -21796 | -21796 | 1 | -21796×3 |
| `setInPickY` | 3 | -57890 | -57890 | -57890 | 1 | -57890×3 |

#### `[MInArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditAutoClean` | 3 | -1550 | -1550 | -1550 | 1 | -1550×3 |
| `SetEditHP` | 3 | -1550 | -1550 | -1550 | 1 | -1550×3 |
| `SetEditPickLoader` | 3 | -2330 | -2330 | -2330 | 1 | -2330×3 |
| `SetEditPlaceInRotate` | 3 | -1800 | -1800 | -1800 | 1 | -1800×3 |
| `SetEditPlaceInShuttle` | 3 | -2000 | -2000 | -2000 | 1 | -2000×3 |

#### `[MInRotate]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditRotateA` | 3 | 1428 | 1428 | 1428 | 1 | 1428×3 |

#### `[MInShutte1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht1OctSiteKit` | 3 | 11000 | 11000 | 11000 | 1 | 11000×3 |
| `edtSetEditIS1BarCode` | 3 | 17212 | 17212 | 17212 | 1 | 17212×3 |
| `edtSetEditOS1BarCode` | 3 | 25500 | 25500 | 25500 | 1 | 25500×3 |
| `setEditInSht1Left` | 3 | -14 | -14 | -14 | 1 | -14×3 |
| `setEditInSht1Right` | 3 | 38453 | 38453 | 38453 | 1 | 38453×3 |
| `setEditOutSht1KitPos` | 3 | 20427 | 20427 | 20427 | 1 | 20427×3 |
| `setEditOutSht1OneRowKit` | 3 | 20433 | 20433 | 20433 | 1 | 20433×3 |

#### `[MInShutte2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht2OctSiteKit` | 3 | 11000 | 11000 | 11000 | 1 | 11000×3 |
| `edtSetEditIS2BarCode` | 3 | 17379 | 17379 | 17379 | 1 | 17379×3 |
| `edtSetEditOS2BarCode` | 3 | 25502 | 25502 | 25502 | 1 | 25502×3 |
| `setEditInSht2Left` | 3 | -22 | -22 | -22 | 1 | -22×3 |
| `setEditInSht2Right` | 3 | 38466 | 38466 | 38466 | 1 | 38466×3 |
| `setEditOutSht2KitPos` | 3 | 20393 | 20393 | 20393 | 1 | 20393×3 |
| `setEditOutSht2OneRowKit` | 3 | 20439 | 20439 | 20439 | 1 | 20439×3 |

#### `[MOutArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditOutXPitch120` | 3 | -22 | -22 | -22 | 1 | -22×3 |
| `setEditOutXPitch40` | 3 | -5334 | -5334 | -5334 | 1 | -5334×3 |

#### `[MOutArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1X` | 3 | -53643 | -53643 | -53643 | 1 | -53643×3 |
| `setEditAuto2X` | 3 | -35170 | -35170 | -35170 | 1 | -35170×3 |
| `setEditAuto3X` | 3 | -16643 | -16643 | -16643 | 1 | -16643×3 |
| `setEditFix1X` | 3 | -35819 | -35819 | -35819 | 1 | -35819×3 |
| `setEditFix2X` | 3 | -21703 | -21703 | -21703 | 1 | -21703×3 |
| `setEditFix3X` | 3 | -16620 | -16620 | -16620 | 1 | -16620×3 |
| `setEditOutSht1X` | 3 | -48913 | -48913 | -48913 | 1 | -48913×3 |
| `setEditOutSht2X` | 3 | -48912 | -48912 | -48912 | 1 | -48912×3 |
| `setEditRotateOutX` | 3 | -49654 | -49654 | -49654 | 1 | -49654×3 |
| `setOutPickX` | 3 | -53582 | -53582 | -53582 | 1 | -53582×3 |

#### `[MOutArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1Y` | 3 | -55885 | -55885 | -55885 | 1 | -55885×3 |
| `setEditAuto2Y` | 3 | -55817 | -55817 | -55817 | 1 | -55817×3 |
| `setEditAuto3Y` | 3 | -55783 | -55783 | -55783 | 1 | -55783×3 |
| `setEditFix1Y` | 3 | -10340 | -10340 | -10340 | 1 | -10340×3 |
| `setEditFix2Y` | 3 | -10342 | -10342 | -10342 | 1 | -10342×3 |
| `setEditFix3Y` | 3 | -10305 | -10305 | -10305 | 1 | -10305×3 |
| `setEditOutSht1Y` | 3 | -37725 | -37725 | -37725 | 1 | -37725×3 |
| `setEditOutSht2Y` | 3 | -5760 | -5760 | -5760 | 1 | -5760×3 |
| `setEditRotateOutY` | 3 | -21742 | -21742 | -21742 | 1 | -21742×3 |
| `setOutPickY` | 3 | -57624 | -57624 | -57624 | 1 | -57624×3 |

#### `[MOutArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditPickOutSht` | 3 | -2060 | -2060 | -2060 | 1 | -2060×3 |
| `SetEditPlaceAuto` | 3 | -1990 | -1990 | -1990 | 1 | -1990×3 |
| `SetEditPlaceFix` | 3 | -1500 | -1500 | -1500 | 1 | -1500×3 |
| `SetEditPlaceOutRotate` | 3 | -1800 | -1800 | -1800 | 1 | -1800×3 |

#### `[MOutRotate]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditRotateOutA` | 3 | 1484 | 1484 | 1484 | 1 | 1484×3 |

#### `[MTestY1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Y` | 3 | -20 | -20 | -20 | 1 | -20×3 |
| `setEditIndex1ToSocketY` | 3 | 15921 | 15921 | 15921 | 1 | 15921×3 |

#### `[MTestY2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Y` | 3 | 37 | 37 | 37 | 1 | 37×3 |
| `setEditIndex2ToSocketY` | 3 | -15943 | -15943 | -15943 | 1 | -15943×3 |

#### `[MTestZ1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Z` | 3 | -5357 | -5357 | -5357 | 1 | -5357×3 |
| `setEditTestZSafePos` | 3 | 10 | 10 | 10 | 1 | 10×3 |
| `setEditWaitTestZDown` | 3 | -1000 | -1000 | -1000 | 1 | -1000×3 |

#### `[MTestZ2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Z` | 3 | -6000 | -6000 | -6000 | 1 | -6000×3 |
| `setEditTestZSafePos` | 3 | 10 | 10 | 10 | 1 | 10×3 |

#### `[MTrayX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditTrayAuto1X` | 3 | 78950 | 78950 | 78950 | 1 | 78950×3 |
| `setEditTrayAuto2X` | 3 | 97440 | 97440 | 97440 | 1 | 97440×3 |
| `setEditTrayAuto3X` | 3 | 115890 | 115890 | 115890 | 1 | 115890×3 |
| `setEditTrayColorX` | 3 | 49800 | 49800 | 49800 | 1 | 49800×3 |
| `setEditTrayEmptyX` | 3 | 28140 | 28140 | 28140 | 1 | 28140×3 |
| `setEditTrayLoaderX` | 3 | -4330 | -4330 | -4330 | 1 | -4330×3 |

#### `[ArmAlignment]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `InArmAlignmentPitch_Y` | 4 | 0 | 750 | 1500 | 2 | 0×2, 1500×2 |
| `InArmZAlignmentAa` | 2 | -500 | -500 | -500 | 1 | -500×2 |
| `InArmZAlignmentAb` | 2 | -500 | -500 | -500 | 1 | -500×2 |
| `InArmZAlignmentAc` | 2 | -500 | -500 | -500 | 1 | -500×2 |
| `InArmZAlignmentAd` | 2 | -500 | -500 | -500 | 1 | -500×2 |
| `InArmZAlignmentBa` | 2 | -500 | -500 | -500 | 1 | -500×2 |
| `InArmZAlignmentBb` | 2 | -500 | -500 | -500 | 1 | -500×2 |
| `InArmZAlignmentBc` | 2 | -500 | -500 | -500 | 1 | -500×2 |
| `InArmZAlignmentBd` | 2 | -500 | -500 | -500 | 1 | -500×2 |
| `OutArmAlignmentPitch_Y` | 4 | 0 | 750 | 1500 | 2 | 0×2, 1500×2 |
| `OutArmZAlignmentAa` | 2 | -500 | -500 | -500 | 1 | -500×2 |
| `OutArmZAlignmentAb` | 2 | -500 | -500 | -500 | 1 | -500×2 |
| `OutArmZAlignmentAc` | 2 | -500 | -500 | -500 | 1 | -500×2 |
| `OutArmZAlignmentAd` | 2 | -500 | -500 | -500 | 1 | -500×2 |
| `OutArmZAlignmentBa` | 2 | -500 | -500 | -500 | 1 | -500×2 |
| `OutArmZAlignmentBb` | 2 | -500 | -500 | -500 | 1 | -500×2 |
| `OutArmZAlignmentBc` | 2 | -500 | -500 | -500 | 1 | -500×2 |
| `OutArmZAlignmentBd` | 2 | -500 | -500 | -500 | 1 | -500×2 |

#### `[InArm]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `AutoCleanPick` | 6 | -1550 | -1550 | -1380 | 3 | -1550×4, -1380×1, -1430×1 |

#### `[InArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 3 | 10 | 10 | 10 | 1 | 10×3 |
| `Picker Ab` | 3 | -10 | -10 | -10 | 1 | -10×3 |
| `Picker Ad` | 3 | 20 | 20 | 20 | 1 | 20×3 |
| `Picker Ba` | 3 | -20 | -20 | -20 | 1 | -20×3 |
| `Picker Bb` | 3 | -10 | -10 | -10 | 1 | -10×3 |
| `Picker Bc` | 3 | 10 | 10 | 10 | 1 | 10×3 |
| `Picker Bd` | 3 | 20 | 20 | 20 | 1 | 20×3 |

#### `[OutArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 3 | -20 | -20 | -20 | 1 | -20×3 |
| `Picker Ad` | 3 | -30 | -30 | -30 | 1 | -30×3 |
| `Picker Ba` | 3 | -20 | -20 | -20 | 1 | -20×3 |
| `Picker Bb` | 3 | -30 | -30 | -30 | 1 | -30×3 |
| `Picker Bc` | 3 | 10 | 10 | 10 | 1 | 10×3 |
| `Picker Bd` | 3 | -10 | -10 | -10 | 1 | -10×3 |

#### `[Teach INI]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Update2` | 3 | 1 | 1 | 1 | 1 | 1×3 |

<!-- preserved-content:end -->
