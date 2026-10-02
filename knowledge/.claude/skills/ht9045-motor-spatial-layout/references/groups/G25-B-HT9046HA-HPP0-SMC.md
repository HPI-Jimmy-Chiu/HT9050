# G25 — B · `HT9046HA`

> **群組**：B (HT-9045 標準機 (6-site GPIB))  
> **HPP**：HPP=0 (窄 HP)  
> **GearRatio**：GearY~0.350(SMC)  
> **樣本數**：5 筆  
> [← 返回索引](../teach-position-statistics.md)

## G25 · `HT9046HA` · HPP=0 (窄 HP ~-46000) · GearY~0.350(SMC)

- **機台群組**：B （HT-9045 標準機 (6-site GPIB)）
- **SubModel 分布**：0×5
- **Picker (USE_PICKER_COUNT) 分布**：1×5
- **Y Pitch (USE_IN_OUT_ARM_Y_PITCH) 分布**：0×5
- **GPIB Model 分布**：`(無)`×3, `9046GPIB`×2
- **樣本數**：5 筆 state record
- **獨立機台數**（依 Serial No 計）：3 台

### 機台清單

| Serial No | Factory | 樣本數 |
|-----------|---------|--------|
| `1526` | KYEC | 1 |
| `1527` | KYEC | 3 |
| `1528` | KYEC | 1 |

### 來源 State Records（最多列 15 筆）

- `TF-AMD\2024-02-01 09_56_37` &nbsp;*(S/N: 1526)*
- `通富微\2023-08-29 17_16_26_Yield` &nbsp;*(S/N: 1527)*
- `通富微\2024-03-22 17_56_31` &nbsp;*(S/N: 1527)*
- `通富微\2024-04-03 15_58_40` &nbsp;*(S/N: 1527)*
- `通富微\2025-10-29 15_18_16` &nbsp;*(S/N: 1528)*

### Teach Position 統計

#### `[MInArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditInXPitch40` | 1 | -5314 | -5314 | -5314 | 1 | -5314×1 |

#### `[MInArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanX` | 1 | 11326 | 11326 | 11326 | 1 | 11326×1 |
| `setEditHP1X` | 1 | 1491 | 1491 | 1491 | 1 | 1491×1 |
| `setEditHP2X` | 1 | 2022 | 2022 | 2022 | 1 | 2022×1 |
| `setEditInSht1X` | 1 | 32040 | 32040 | 32040 | 1 | 32040×1 |
| `setEditInSht2X` | 1 | 32034 | 32034 | 32034 | 1 | 32034×1 |
| `setEditLoaderX` | 1 | 20924 | 20924 | 20924 | 1 | 20924×1 |
| `setInPickX` | 1 | 21386 | 21386 | 21386 | 1 | 21386×1 |

#### `[MInArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanY` | 1 | -81649 | -81649 | -81649 | 1 | -81649×1 |
| `setEditHP1Y` | 1 | -46424 | -46424 | -46424 | 1 | -46424×1 |
| `setEditHP2Y` | 1 | -7437 | -7437 | -7437 | 1 | -7437×1 |
| `setEditInSht1Y` | 1 | -37748 | -37748 | -37748 | 1 | -37748×1 |
| `setEditInSht2Y` | 1 | -5738 | -5738 | -5738 | 1 | -5738×1 |
| `setEditLoaderY` | 1 | -56310 | -56310 | -56310 | 1 | -56310×1 |
| `setInPickY` | 1 | -57890 | -57890 | -57890 | 1 | -57890×1 |

#### `[MInArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditHP` | 1 | -1540 | -1540 | -1540 | 1 | -1540×1 |
| `SetEditPickLoader` | 1 | -2150 | -2150 | -2150 | 1 | -2150×1 |
| `SetEditPlaceInShuttle` | 1 | -1850 | -1850 | -1850 | 1 | -1850×1 |

#### `[MInShutte1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht1OctSiteKit` | 1 | 11000 | 11000 | 11000 | 1 | 11000×1 |
| `edtSetEditIS1BarCode` | 1 | 17212 | 17212 | 17212 | 1 | 17212×1 |
| `edtSetEditOS1BarCode` | 1 | 25500 | 25500 | 25500 | 1 | 25500×1 |
| `setEditInSht1Left` | 1 | -50 | -50 | -50 | 1 | -50×1 |
| `setEditInSht1Right` | 1 | 38408 | 38408 | 38408 | 1 | 38408×1 |
| `setEditOutSht1KitPos` | 1 | 20383 | 20383 | 20383 | 1 | 20383×1 |
| `setEditOutSht1OneRowKit` | 1 | 20060 | 20060 | 20060 | 1 | 20060×1 |

#### `[MInShutte2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht2OctSiteKit` | 1 | 11000 | 11000 | 11000 | 1 | 11000×1 |
| `edtSetEditIS2BarCode` | 1 | 17379 | 17379 | 17379 | 1 | 17379×1 |
| `edtSetEditOS2BarCode` | 1 | 25502 | 25502 | 25502 | 1 | 25502×1 |
| `setEditInSht2Left` | 1 | -130 | -130 | -130 | 1 | -130×1 |
| `setEditInSht2Right` | 1 | 38319 | 38319 | 38319 | 1 | 38319×1 |
| `setEditOutSht2KitPos` | 1 | 20333 | 20333 | 20333 | 1 | 20333×1 |
| `setEditOutSht2OneRowKit` | 1 | 20050 | 20050 | 20050 | 1 | 20050×1 |

#### `[MOutArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditOutXPitch120` | 1 | 54 | 54 | 54 | 1 | 54×1 |
| `setEditOutXPitch40` | 1 | -5260 | -5260 | -5260 | 1 | -5260×1 |

#### `[MOutArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1X` | 1 | -53634 | -53634 | -53634 | 1 | -53634×1 |
| `setEditAuto2X` | 1 | -35164 | -35164 | -35164 | 1 | -35164×1 |
| `setEditAuto3X` | 1 | -16660 | -16660 | -16660 | 1 | -16660×1 |
| `setEditFix1X` | 1 | -35715 | -35715 | -35715 | 1 | -35715×1 |
| `setEditFix2X` | 1 | -21649 | -21649 | -21649 | 1 | -21649×1 |
| `setEditFix3X` | 1 | -7482 | -7482 | -7482 | 1 | -7482×1 |
| `setEditOutSht1X` | 1 | -48897 | -48897 | -48897 | 1 | -48897×1 |
| `setEditOutSht2X` | 1 | -48897 | -48897 | -48897 | 1 | -48897×1 |
| `setOutPickX` | 1 | -53582 | -53582 | -53582 | 1 | -53582×1 |

#### `[MOutArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1Y` | 1 | -56183 | -56183 | -56183 | 1 | -56183×1 |
| `setEditAuto2Y` | 1 | -56124 | -56124 | -56124 | 1 | -56124×1 |
| `setEditAuto3Y` | 1 | -56130 | -56130 | -56130 | 1 | -56130×1 |
| `setEditFix1Y` | 1 | -10364 | -10364 | -10364 | 1 | -10364×1 |
| `setEditFix2Y` | 1 | -10364 | -10364 | -10364 | 1 | -10364×1 |
| `setEditFix3Y` | 1 | -10364 | -10364 | -10364 | 1 | -10364×1 |
| `setEditOutSht1Y` | 1 | -37721 | -37721 | -37721 | 1 | -37721×1 |
| `setEditOutSht2Y` | 1 | -5720 | -5720 | -5720 | 1 | -5720×1 |
| `setOutPickY` | 1 | -57624 | -57624 | -57624 | 1 | -57624×1 |

#### `[MOutArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditPickOutSht` | 1 | -1900 | -1900 | -1900 | 1 | -1900×1 |
| `SetEditPlaceAuto` | 1 | -2100 | -2100 | -2100 | 1 | -2100×1 |
| `SetEditPlaceFix` | 1 | -1400 | -1400 | -1400 | 1 | -1400×1 |

#### `[MTestY1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Y` | 1 | -102 | -102 | -102 | 1 | -102×1 |
| `setEditIndex1ToSocketY` | 1 | 15885 | 15885 | 15885 | 1 | 15885×1 |

#### `[MTestY2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Y` | 1 | 98 | 98 | 98 | 1 | 98×1 |
| `setEditIndex2ToSocketY` | 1 | -15915 | -15915 | -15915 | 1 | -15915×1 |

#### `[MTestZ1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Z` | 1 | -4357 | -4357 | -4357 | 1 | -4357×1 |
| `setEditTestZSafePos` | 1 | 10 | 10 | 10 | 1 | 10×1 |
| `setEditWaitTestZDown` | 1 | -1000 | -1000 | -1000 | 1 | -1000×1 |

#### `[MTestZ2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Z` | 1 | -4306 | -4306 | -4306 | 1 | -4306×1 |
| `setEditTestZSafePos` | 1 | 10 | 10 | 10 | 1 | 10×1 |

#### `[MTrayX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditTrayAuto1X` | 1 | 78520 | 78520 | 78520 | 1 | 78520×1 |
| `setEditTrayAuto2X` | 1 | 96932 | 96932 | 96932 | 1 | 96932×1 |
| `setEditTrayAuto3X` | 1 | 115350 | 115350 | 115350 | 1 | 115350×1 |
| `setEditTrayColorX` | 1 | 49501 | 49501 | 49501 | 1 | 49501×1 |
| `setEditTrayEmptyX` | 1 | 27733 | 27733 | 27733 | 1 | 27733×1 |
| `setEditTrayLoaderX` | 1 | -4480 | -4480 | -4480 | 1 | -4480×1 |

#### `[InArm]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `AutoCleanPick` | 5 | -1740 | -1740 | -1540 | 3 | -1740×3, -1600×1, -1540×1 |

#### `[InArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 1 | -8 | -8 | -8 | 1 | -8×1 |
| `Picker Ab` | 1 | 30 | 30 | 30 | 1 | 30×1 |
| `Picker Ad` | 1 | -11 | -11 | -11 | 1 | -11×1 |
| `Picker Ba` | 1 | -2 | -2 | -2 | 1 | -2×1 |
| `Picker Bb` | 1 | 32 | 32 | 32 | 1 | 32×1 |
| `Picker Bc` | 1 | -5 | -5 | -5 | 1 | -5×1 |
| `Picker Bd` | 1 | 8 | 8 | 8 | 1 | 8×1 |

#### `[OutArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 1 | 23 | 23 | 23 | 1 | 23×1 |
| `Picker Ab` | 1 | 22 | 22 | 22 | 1 | 22×1 |
| `Picker Ad` | 1 | -8 | -8 | -8 | 1 | -8×1 |
| `Picker Ba` | 1 | 84 | 84 | 84 | 1 | 84×1 |
| `Picker Bb` | 1 | 45 | 45 | 45 | 1 | 45×1 |
| `Picker Bc` | 1 | 19 | 19 | 19 | 1 | 19×1 |
| `Picker Bd` | 1 | 44 | 44 | 44 | 1 | 44×1 |

#### `[Teach INI]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Update2` | 1 | 1 | 1 | 1 | 1 | 1×1 |
