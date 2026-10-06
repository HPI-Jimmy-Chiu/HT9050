> 保存來源：`.claude/skills/ht9045-motor-spatial-layout/references/groups/G16-B-HT-9045WA-HPP1-GearY=.md`，main `eb7f47e7f`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../../common.md)。

<!-- preserved-content:start -->
# G16 — B · `HT-9045WA`

> **群組**：B (HT-9045 標準機 (6-site GPIB))  
> **HPP**：HPP=1 (宬 HP)  
> **GearRatio**：GearY=?  
> **樣本數**：1 筆  
> [← 返回索引](../teach-position-statistics.md)

## G16 · `HT-9045WA` · HPP=1 (宬 HP ~-82000) · GearY=?

- **機台群組**：B （HT-9045 標準機 (6-site GPIB)）
- **SubModel 分布**：0×1
- **Picker (USE_PICKER_COUNT) 分布**：1×1
- **Y Pitch (USE_IN_OUT_ARM_Y_PITCH) 分布**：0×1
- **GPIB Model 分布**：`9045GPIB`×1
- **樣本數**：1 筆 state record
- **獨立機台數**（依 Serial No 計）：1 台

### 機台清單

| Serial No | Factory | 樣本數 |
|-----------|---------|--------|
| `ILD589` | KYEC | 1 |

### 來源 State Records（最多列 15 筆）

- `JCET\2025-06-27 15_59_37` &nbsp;*(S/N: ILD589)*

### Teach Position 統計

#### `[MInArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditInXPitch120` | 1 | 32 | 32 | 32 | 1 | 32×1 |
| `setEditInXPitch40` | 1 | -5272 | -5272 | -5272 | 1 | -5272×1 |

#### `[MInArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanX` | 1 | 11387 | 11387 | 11387 | 1 | 11387×1 |
| `setEditHP1X` | 1 | 5522 | 5522 | 5522 | 1 | 5522×1 |
| `setEditHP2X` | 1 | 5529 | 5529 | 5529 | 1 | 5529×1 |
| `setEditInSht1X` | 1 | 32003 | 32003 | 32003 | 1 | 32003×1 |
| `setEditInSht2X` | 1 | 32068 | 32068 | 32068 | 1 | 32068×1 |
| `setEditLoaderX` | 1 | 20918 | 20918 | 20918 | 1 | 20918×1 |
| `setInPickX` | 1 | 21386 | 21386 | 21386 | 1 | 21386×1 |

#### `[MInArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanY` | 1 | -81718 | -81718 | -81718 | 1 | -81718×1 |
| `setEditHP1Y` | 1 | -82460 | -82460 | -82460 | 1 | -82460×1 |
| `setEditHP2Y` | 1 | -43477 | -43477 | -43477 | 1 | -43477×1 |
| `setEditInSht1Y` | 1 | -37757 | -37757 | -37757 | 1 | -37757×1 |
| `setEditInSht2Y` | 1 | -5739 | -5739 | -5739 | 1 | -5739×1 |
| `setEditLoaderY` | 1 | -55989 | -55989 | -55989 | 1 | -55989×1 |
| `setInPickY` | 1 | -57890 | -57890 | -57890 | 1 | -57890×1 |

#### `[MInArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditHP` | 1 | -1420 | -1420 | -1420 | 1 | -1420×1 |
| `SetEditPickLoader` | 1 | -1990 | -1990 | -1990 | 1 | -1990×1 |
| `SetEditPlaceInShuttle` | 1 | -1610 | -1610 | -1610 | 1 | -1610×1 |

#### `[MInShutte1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht1OctSiteKit` | 1 | 11000 | 11000 | 11000 | 1 | 11000×1 |
| `edtSetEditIS1BarCode` | 1 | 17212 | 17212 | 17212 | 1 | 17212×1 |
| `edtSetEditOS1BarCode` | 1 | 25500 | 25500 | 25500 | 1 | 25500×1 |
| `setEditInSht1Left` | 1 | -66 | -66 | -66 | 1 | -66×1 |
| `setEditInSht1Right` | 1 | 38452 | 38452 | 38452 | 1 | 38452×1 |
| `setEditOutSht1KitPos` | 1 | 20477 | 20477 | 20477 | 1 | 20477×1 |
| `setEditOutSht1OneRowKit` | 1 | 20348 | 20348 | 20348 | 1 | 20348×1 |

#### `[MInShutte2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht2OctSiteKit` | 1 | 11000 | 11000 | 11000 | 1 | 11000×1 |
| `edtSetEditIS2BarCode` | 1 | 17379 | 17379 | 17379 | 1 | 17379×1 |
| `edtSetEditOS2BarCode` | 1 | 25502 | 25502 | 25502 | 1 | 25502×1 |
| `setEditInSht2Left` | 1 | 53 | 53 | 53 | 1 | 53×1 |
| `setEditInSht2Right` | 1 | 38553 | 38553 | 38553 | 1 | 38553×1 |
| `setEditOutSht2KitPos` | 1 | 20562 | 20562 | 20562 | 1 | 20562×1 |
| `setEditOutSht2OneRowKit` | 1 | 20524 | 20524 | 20524 | 1 | 20524×1 |

#### `[MOutArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditOutXPitch120` | 1 | 42 | 42 | 42 | 1 | 42×1 |
| `setEditOutXPitch40` | 1 | -5267 | -5267 | -5267 | 1 | -5267×1 |

#### `[MOutArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1X` | 1 | -53538 | -53538 | -53538 | 1 | -53538×1 |
| `setEditAuto2X` | 1 | -35128 | -35128 | -35128 | 1 | -35128×1 |
| `setEditAuto3X` | 1 | -16570 | -16570 | -16570 | 1 | -16570×1 |
| `setEditFix1X` | 1 | -35472 | -35472 | -35472 | 1 | -35472×1 |
| `setEditFix2X` | 1 | -21373 | -21373 | -21373 | 1 | -21373×1 |
| `setEditFix3X` | 1 | -16032 | -16032 | -16032 | 1 | -16032×1 |
| `setEditOutSht1X` | 1 | -48894 | -48894 | -48894 | 1 | -48894×1 |
| `setEditOutSht2X` | 1 | -48853 | -48853 | -48853 | 1 | -48853×1 |
| `setOutPickX` | 1 | -53582 | -53582 | -53582 | 1 | -53582×1 |

#### `[MOutArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1Y` | 1 | -55904 | -55904 | -55904 | 1 | -55904×1 |
| `setEditAuto2Y` | 1 | -55899 | -55899 | -55899 | 1 | -55899×1 |
| `setEditAuto3Y` | 1 | -55889 | -55889 | -55889 | 1 | -55889×1 |
| `setEditFix1Y` | 1 | -10387 | -10387 | -10387 | 1 | -10387×1 |
| `setEditFix2Y` | 1 | -10361 | -10361 | -10361 | 1 | -10361×1 |
| `setEditFix3Y` | 1 | -10370 | -10370 | -10370 | 1 | -10370×1 |
| `setEditOutSht1Y` | 1 | -37747 | -37747 | -37747 | 1 | -37747×1 |
| `setEditOutSht2Y` | 1 | -5721 | -5721 | -5721 | 1 | -5721×1 |
| `setOutPickY` | 1 | -57624 | -57624 | -57624 | 1 | -57624×1 |

#### `[MOutArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditPickOutSht` | 1 | -1680 | -1680 | -1680 | 1 | -1680×1 |
| `SetEditPlaceAuto` | 1 | -1860 | -1860 | -1860 | 1 | -1860×1 |
| `SetEditPlaceFix` | 1 | -1260 | -1260 | -1260 | 1 | -1260×1 |

#### `[MTestY1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Y` | 1 | -180 | -180 | -180 | 1 | -180×1 |
| `setEditIndex1ToSocketY` | 1 | 15849 | 15849 | 15849 | 1 | 15849×1 |

#### `[MTestY2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Y` | 1 | 179 | 179 | 179 | 1 | 179×1 |
| `setEditIndex2ToSocketY` | 1 | -15849 | -15849 | -15849 | 1 | -15849×1 |

#### `[MTestZ1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Z` | 1 | -6357 | -6357 | -6357 | 1 | -6357×1 |
| `setEditTestZSafePos` | 1 | 10 | 10 | 10 | 1 | 10×1 |
| `setEditWaitTestZDown` | 1 | -1000 | -1000 | -1000 | 1 | -1000×1 |

#### `[MTestZ2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Z` | 1 | -6306 | -6306 | -6306 | 1 | -6306×1 |
| `setEditTestZSafePos` | 1 | 10 | 10 | 10 | 1 | 10×1 |

#### `[MTrayX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditTrayAuto1X` | 1 | 78070 | 78070 | 78070 | 1 | 78070×1 |
| `setEditTrayAuto2X` | 1 | 96470 | 96470 | 96470 | 1 | 96470×1 |
| `setEditTrayAuto3X` | 1 | 115020 | 115020 | 115020 | 1 | 115020×1 |
| `setEditTrayColorX` | 1 | 48880 | 48880 | 48880 | 1 | 48880×1 |
| `setEditTrayEmptyX` | 1 | 27400 | 27400 | 27400 | 1 | 27400×1 |
| `setEditTrayLoaderX` | 1 | -5060 | -5060 | -5060 | 1 | -5060×1 |

#### `[InArm]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `AutoCleanPick` | 1 | -1420 | -1420 | -1420 | 1 | -1420×1 |

#### `[InArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 1 | -21 | -21 | -21 | 1 | -21×1 |
| `Picker Ab` | 1 | -7 | -7 | -7 | 1 | -7×1 |
| `Picker Ad` | 1 | 7 | 7 | 7 | 1 | 7×1 |
| `Picker Ba` | 1 | 14 | 14 | 14 | 1 | 14×1 |
| `Picker Bb` | 1 | 42 | 42 | 42 | 1 | 42×1 |
| `Picker Bc` | 1 | 9 | 9 | 9 | 1 | 9×1 |
| `Picker Bd` | 1 | 22 | 22 | 22 | 1 | 22×1 |

#### `[OutArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 1 | 19 | 19 | 19 | 1 | 19×1 |
| `Picker Ab` | 1 | 5 | 5 | 5 | 1 | 5×1 |
| `Picker Ad` | 1 | 14 | 14 | 14 | 1 | 14×1 |
| `Picker Ba` | 1 | 42 | 42 | 42 | 1 | 42×1 |
| `Picker Bb` | 1 | 26 | 26 | 26 | 1 | 26×1 |
| `Picker Bc` | 1 | 10 | 10 | 10 | 1 | 10×1 |
| `Picker Bd` | 1 | 9 | 9 | 9 | 1 | 9×1 |

#### `[Teach INI]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Update2` | 1 | 1 | 1 | 1 | 1 | 1×1 |

<!-- preserved-content:end -->
