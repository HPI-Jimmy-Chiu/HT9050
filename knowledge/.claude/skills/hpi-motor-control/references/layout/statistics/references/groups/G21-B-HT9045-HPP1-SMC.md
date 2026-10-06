> 保存來源：`.claude/skills/ht9045-motor-spatial-layout/references/groups/G21-B-HT9045-HPP1-SMC.md`，main `eb7f47e7f`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../../common.md)。

<!-- preserved-content:start -->
# G21 — B · `HT9045`

> **群組**：B (HT-9045 標準機 (6-site GPIB))  
> **HPP**：HPP=1 (宬 HP)  
> **GearRatio**：GearY~0.350(SMC)  
> **樣本數**：2 筆  
> [← 返回索引](../teach-position-statistics.md)

## G21 · `HT9045` · HPP=1 (宬 HP ~-82000) · GearY~0.350(SMC)

- **機台群組**：B （HT-9045 標準機 (6-site GPIB)）
- **SubModel 分布**：0×2
- **Picker (USE_PICKER_COUNT) 分布**：1×2
- **Y Pitch (USE_IN_OUT_ARM_Y_PITCH) 分布**：0×2
- **GPIB Model 分布**：`9045GPIB`×2
- **樣本數**：2 筆 state record
- **獨立機台數**（依 Serial No 計）：1 台

### 機台清單

| Serial No | Factory | 樣本數 |
|-----------|---------|--------|
| `GLD144` | KYEC | 2 |

### 來源 State Records（最多列 15 筆）

- `JCET\2025-06-26 16_57_28 Yield` &nbsp;*(S/N: GLD144)*
- `JCET\2025-11-03 16_01_36` &nbsp;*(S/N: GLD144)*

### Teach Position 統計

#### `[MInArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditInXPitch120` | 2 | 114 | 114 | 114 | 1 | 114×2 |
| `setEditInXPitch40` | 2 | -5232 | -5232 | -5232 | 1 | -5232×2 |

#### `[MInArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanX` | 2 | 11394 | 11394 | 11394 | 1 | 11394×2 |
| `setEditHP1X` | 2 | 5440 | 5440 | 5440 | 1 | 5440×2 |
| `setEditHP2X` | 2 | 5430 | 5430 | 5430 | 1 | 5430×2 |
| `setEditInSht1X` | 2 | 31971 | 31971 | 31971 | 1 | 31971×2 |
| `setEditInSht2X` | 2 | 32029 | 32029 | 32029 | 1 | 32029×2 |
| `setEditLoaderX` | 2 | 20929 | 20929 | 20929 | 1 | 20929×2 |
| `setInPickX` | 2 | 21386 | 21386 | 21386 | 1 | 21386×2 |

#### `[MInArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanY` | 2 | -81785 | -81785 | -81785 | 1 | -81785×2 |
| `setEditHP1Y` | 2 | -82365 | -82365 | -82365 | 1 | -82365×2 |
| `setEditHP2Y` | 2 | -43400 | -43400 | -43400 | 1 | -43400×2 |
| `setEditInSht1Y` | 2 | -37716 | -37716 | -37716 | 1 | -37716×2 |
| `setEditInSht2Y` | 2 | -5742 | -5742 | -5742 | 1 | -5742×2 |
| `setEditLoaderY` | 2 | -55949 | -55949 | -55949 | 1 | -55949×2 |
| `setInPickY` | 2 | -57890 | -57890 | -57890 | 1 | -57890×2 |

#### `[MInArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditHP` | 2 | -1500 | -1500 | -1500 | 1 | -1500×2 |
| `SetEditPickLoader` | 2 | -2070 | -2070 | -2070 | 1 | -2070×2 |
| `SetEditPlaceInShuttle` | 2 | -1830 | -1830 | -1830 | 1 | -1830×2 |

#### `[MInShutte1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht1OctSiteKit` | 2 | 10000 | 10000 | 10000 | 1 | 10000×2 |
| `edtSetEditIS1BarCode` | 2 | 17212 | 17212 | 17212 | 1 | 17212×2 |
| `edtSetEditOS1BarCode` | 2 | 25500 | 25500 | 25500 | 1 | 25500×2 |
| `setEditInSht1Left` | 2 | -12 | -12 | -12 | 1 | -12×2 |
| `setEditInSht1Right` | 2 | 38486 | 38486 | 38486 | 1 | 38486×2 |
| `setEditOutSht1KitPos` | 2 | 20533 | 20533 | 20533 | 1 | 20533×2 |
| `setEditOutSht1OneRowKit` | 2 | 20480 | 20480 | 20480 | 1 | 20480×2 |

#### `[MInShutte2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht2OctSiteKit` | 2 | 10000 | 10000 | 10000 | 1 | 10000×2 |
| `edtSetEditIS2BarCode` | 2 | 17379 | 17379 | 17379 | 1 | 17379×2 |
| `edtSetEditOS2BarCode` | 2 | 25502 | 25502 | 25502 | 1 | 25502×2 |
| `setEditInSht2Left` | 2 | 20 | 20 | 20 | 1 | 20×2 |
| `setEditInSht2Right` | 2 | 38517 | 38517 | 38517 | 1 | 38517×2 |
| `setEditOutSht2KitPos` | 2 | 20493 | 20493 | 20493 | 1 | 20493×2 |
| `setEditOutSht2OneRowKit` | 2 | 20432 | 20432 | 20432 | 1 | 20432×2 |

#### `[MOutArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditOutXPitch120` | 2 | 99 | 99 | 99 | 1 | 99×2 |
| `setEditOutXPitch40` | 2 | -5262 | -5262 | -5262 | 1 | -5262×2 |

#### `[MOutArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1X` | 2 | -53569 | -53569 | -53569 | 1 | -53569×2 |
| `setEditAuto2X` | 2 | -35070 | -35070 | -35070 | 1 | -35070×2 |
| `setEditAuto3X` | 2 | -16582 | -16582 | -16582 | 1 | -16582×2 |
| `setEditFix1X` | 2 | -35650 | -35650 | -35650 | 1 | -35650×2 |
| `setEditFix2X` | 2 | -21368 | -21368 | -21368 | 1 | -21368×2 |
| `setEditFix3X` | 2 | -16000 | -16000 | -16000 | 1 | -16000×2 |
| `setEditOutSht1X` | 2 | -48878 | -48878 | -48878 | 1 | -48878×2 |
| `setEditOutSht2X` | 2 | -48876 | -48876 | -48876 | 1 | -48876×2 |
| `setOutPickX` | 2 | -53582 | -53582 | -53582 | 1 | -53582×2 |

#### `[MOutArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1Y` | 2 | -55913 | -55913 | -55913 | 1 | -55913×2 |
| `setEditAuto2Y` | 2 | -55929 | -55929 | -55929 | 1 | -55929×2 |
| `setEditAuto3Y` | 2 | -55904 | -55904 | -55904 | 1 | -55904×2 |
| `setEditFix1Y` | 2 | -10375 | -10375 | -10375 | 1 | -10375×2 |
| `setEditFix2Y` | 2 | -10365 | -10365 | -10365 | 1 | -10365×2 |
| `setEditFix3Y` | 2 | -10372 | -10372 | -10372 | 1 | -10372×2 |
| `setEditOutSht1Y` | 2 | -37722 | -37722 | -37722 | 1 | -37722×2 |
| `setEditOutSht2Y` | 2 | -5726 | -5726 | -5726 | 1 | -5726×2 |
| `setOutPickY` | 2 | -57624 | -57624 | -57624 | 1 | -57624×2 |

#### `[MOutArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditPickOutSht` | 2 | -1820 | -1820 | -1820 | 1 | -1820×2 |
| `SetEditPlaceAuto` | 2 | -1980 | -1980 | -1980 | 1 | -1980×2 |
| `SetEditPlaceFix` | 2 | -1280 | -1280 | -1280 | 1 | -1280×2 |

#### `[MTestY1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Y` | 2 | -75 | -75 | -75 | 1 | -75×2 |
| `setEditIndex1ToSocketY` | 2 | 15958 | 15958 | 15958 | 1 | 15958×2 |

#### `[MTestY2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Y` | 2 | 96 | 96 | 96 | 1 | 96×2 |
| `setEditIndex2ToSocketY` | 2 | -15965 | -15965 | -15965 | 1 | -15965×2 |

#### `[MTestZ1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Z` | 2 | -4357 | -4357 | -4357 | 1 | -4357×2 |
| `setEditTestZSafePos` | 2 | 10 | 10 | 10 | 1 | 10×2 |
| `setEditWaitTestZDown` | 2 | -1000 | -1000 | -1000 | 1 | -1000×2 |

#### `[MTestZ2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Z` | 2 | -4306 | -4306 | -4306 | 1 | -4306×2 |
| `setEditTestZSafePos` | 2 | 10 | 10 | 10 | 1 | 10×2 |

#### `[MTrayX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditTrayAuto1X` | 2 | 78710 | 78710 | 78710 | 1 | 78710×2 |
| `setEditTrayAuto2X` | 2 | 97110 | 97110 | 97110 | 1 | 97110×2 |
| `setEditTrayAuto3X` | 2 | 115570 | 115570 | 115570 | 1 | 115570×2 |
| `setEditTrayColorX` | 2 | 49660 | 49660 | 49660 | 1 | 49660×2 |
| `setEditTrayEmptyX` | 2 | 25780 | 25780 | 25780 | 1 | 25780×2 |
| `setEditTrayLoaderX` | 2 | -4370 | -4370 | -4370 | 1 | -4370×2 |

#### `[InArm]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `AutoCleanPick` | 2 | -1500 | -1500 | -1500 | 1 | -1500×2 |

#### `[InArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 2 | -10 | -10 | -10 | 1 | -10×2 |
| `Picker Ab` | 2 | -20 | -20 | -20 | 1 | -20×2 |
| `Picker Ad` | 2 | -20 | -20 | -20 | 1 | -20×2 |
| `Picker Bb` | 2 | -10 | -10 | -10 | 1 | -10×2 |
| `Picker Bc` | 2 | -20 | -20 | -20 | 1 | -20×2 |

#### `[OutArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Ad` | 2 | -10 | -10 | -10 | 1 | -10×2 |
| `Picker Bb` | 2 | -10 | -10 | -10 | 1 | -10×2 |
| `Picker Bd` | 2 | -20 | -20 | -20 | 1 | -20×2 |

#### `[Teach INI]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Update2` | 2 | 1 | 1 | 1 | 1 | 1×2 |

<!-- preserved-content:end -->
