# G07 — B · `HT-9045`

> **群組**：B (HT-9045 標準機 (6-site GPIB))  
> **HPP**：HPP=1 (宬 HP)  
> **GearRatio**：GearY~0.350(SMC)  
> **樣本數**：2 筆  
> [← 返回索引](../teach-position-statistics.md)

## G07 · `HT-9045` · HPP=1 (宬 HP ~-82000) · GearY~0.350(SMC)

- **機台群組**：B （HT-9045 標準機 (6-site GPIB)）
- **SubModel 分布**：0×2
- **Picker (USE_PICKER_COUNT) 分布**：1×2
- **Y Pitch (USE_IN_OUT_ARM_Y_PITCH) 分布**：0×2
- **GPIB Model 分布**：`(無)`×1, `9045GPIB`×1
- **樣本數**：2 筆 state record
- **獨立機台數**（依 Serial No 計）：1 台

### 機台清單

| Serial No | Factory | 樣本數 |
|-----------|---------|--------|
| `PMLR298` | KYEC | 2 |

### 來源 State Records（最多列 15 筆）

- `TF-AMD\2024-06-27 18_19_18` &nbsp;*(S/N: PMLR298)*
- `確安\2024-08-14 14_21_36.zip` &nbsp;*(S/N: PMLR298)*

### Teach Position 統計

#### `[MInArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditInXPitch120` | 1 | 12 | 12 | 12 | 1 | 12×1 |
| `setEditInXPitch40` | 1 | -5322 | -5322 | -5322 | 1 | -5322×1 |

#### `[MInArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanX` | 1 | 11321 | 11321 | 11321 | 1 | 11321×1 |
| `setEditHP1X` | 1 | 5415 | 5415 | 5415 | 1 | 5415×1 |
| `setEditHP2X` | 1 | 5383 | 5383 | 5383 | 1 | 5383×1 |
| `setEditInSht1X` | 1 | 31965 | 31965 | 31965 | 1 | 31965×1 |
| `setEditInSht2X` | 1 | 31950 | 31950 | 31950 | 1 | 31950×1 |
| `setEditLoaderX` | 1 | 20848 | 20848 | 20848 | 1 | 20848×1 |
| `setEditRotateX` | 1 | -10848 | -10848 | -10848 | 1 | -10848×1 |
| `setInPickX` | 1 | 21386 | 21386 | 21386 | 1 | 21386×1 |

#### `[MInArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanY` | 1 | -81623 | -81623 | -81623 | 1 | -81623×1 |
| `setEditHP1Y` | 1 | -82389 | -82389 | -82389 | 1 | -82389×1 |
| `setEditHP2Y` | 1 | -43395 | -43395 | -43395 | 1 | -43395×1 |
| `setEditInSht1Y` | 1 | -37656 | -37656 | -37656 | 1 | -37656×1 |
| `setEditInSht2Y` | 1 | -5695 | -5695 | -5695 | 1 | -5695×1 |
| `setEditLoaderY` | 1 | -55854 | -55854 | -55854 | 1 | -55854×1 |
| `setEditRotateY` | 1 | -29913 | -29913 | -29913 | 1 | -29913×1 |
| `setInPickY` | 1 | -57890 | -57890 | -57890 | 1 | -57890×1 |

#### `[MInArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditHP` | 1 | -2110 | -2110 | -2110 | 1 | -2110×1 |
| `SetEditPickLoader` | 1 | -1810 | -1810 | -1810 | 1 | -1810×1 |
| `SetEditPlaceInShuttle` | 1 | -1940 | -1940 | -1940 | 1 | -1940×1 |

#### `[MInShutte1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht1OctSiteKit` | 1 | 11000 | 11000 | 11000 | 1 | 11000×1 |
| `edtSetEditIS1BarCode` | 1 | 17212 | 17212 | 17212 | 1 | 17212×1 |
| `edtSetEditOS1BarCode` | 1 | 25500 | 25500 | 25500 | 1 | 25500×1 |
| `setEditInSht1Left` | 1 | 21 | 21 | 21 | 1 | 21×1 |
| `setEditInSht1Right` | 1 | 38494 | 38494 | 38494 | 1 | 38494×1 |
| `setEditOutSht1KitPos` | 1 | 20474 | 20474 | 20474 | 1 | 20474×1 |
| `setEditOutSht1OneRowKit` | 1 | 19422 | 19422 | 19422 | 1 | 19422×1 |

#### `[MInShutte2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht2OctSiteKit` | 1 | 11000 | 11000 | 11000 | 1 | 11000×1 |
| `edtSetEditIS2BarCode` | 1 | 17379 | 17379 | 17379 | 1 | 17379×1 |
| `edtSetEditOS2BarCode` | 1 | 25502 | 25502 | 25502 | 1 | 25502×1 |
| `setEditInSht2Left` | 1 | 19 | 19 | 19 | 1 | 19×1 |
| `setEditInSht2Right` | 1 | 38475 | 38475 | 38475 | 1 | 38475×1 |
| `setEditOutSht2KitPos` | 1 | 20536 | 20536 | 20536 | 1 | 20536×1 |
| `setEditOutSht2OneRowKit` | 1 | 19436 | 19436 | 19436 | 1 | 19436×1 |

#### `[MOutArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditOutXPitch40` | 1 | -5322 | -5322 | -5322 | 1 | -5322×1 |

#### `[MOutArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtBinBoxX` | 1 | -10848 | -10848 | -10848 | 1 | -10848×1 |
| `setEditAuto1X` | 1 | -53553 | -53553 | -53553 | 1 | -53553×1 |
| `setEditAuto2X` | 1 | -35051 | -35051 | -35051 | 1 | -35051×1 |
| `setEditAuto3X` | 1 | -16600 | -16600 | -16600 | 1 | -16600×1 |
| `setEditFix1X` | 1 | -35658 | -35658 | -35658 | 1 | -35658×1 |
| `setEditFix2X` | 1 | -21562 | -21562 | -21562 | 1 | -21562×1 |
| `setEditFix3X` | 1 | -16649 | -16649 | -16649 | 1 | -16649×1 |
| `setEditOutSht1X` | 1 | -48816 | -48816 | -48816 | 1 | -48816×1 |
| `setEditOutSht2X` | 1 | -48851 | -48851 | -48851 | 1 | -48851×1 |
| `setOutPickX` | 1 | -53582 | -53582 | -53582 | 1 | -53582×1 |

#### `[MOutArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtBinBoxY` | 1 | -20245 | -20245 | -20245 | 1 | -20245×1 |
| `setEditAuto1Y` | 1 | -55665 | -55665 | -55665 | 1 | -55665×1 |
| `setEditAuto2Y` | 1 | -55649 | -55649 | -55649 | 1 | -55649×1 |
| `setEditAuto3Y` | 1 | -55636 | -55636 | -55636 | 1 | -55636×1 |
| `setEditFix1Y` | 1 | -10346 | -10346 | -10346 | 1 | -10346×1 |
| `setEditFix2Y` | 1 | -10367 | -10367 | -10367 | 1 | -10367×1 |
| `setEditFix3Y` | 1 | -10354 | -10354 | -10354 | 1 | -10354×1 |
| `setEditOutSht1Y` | 1 | -37708 | -37708 | -37708 | 1 | -37708×1 |
| `setEditOutSht2Y` | 1 | -5727 | -5727 | -5727 | 1 | -5727×1 |
| `setOutPickY` | 1 | -57624 | -57624 | -57624 | 1 | -57624×1 |

#### `[MOutArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditPickOutSht` | 1 | -2130 | -2130 | -2130 | 1 | -2130×1 |
| `SetEditPlaceAuto` | 1 | -1820 | -1820 | -1820 | 1 | -1820×1 |
| `SetEditPlaceFix` | 1 | -1820 | -1820 | -1820 | 1 | -1820×1 |

#### `[MTestY1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Y` | 1 | -108 | -108 | -108 | 1 | -108×1 |
| `setEditIndex1ToSocketY` | 1 | 15870 | 15870 | 15870 | 1 | 15870×1 |

#### `[MTestY2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Y` | 1 | 84 | 84 | 84 | 1 | 84×1 |
| `setEditIndex2ToSocketY` | 1 | -15903 | -15903 | -15903 | 1 | -15903×1 |

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
| `setEditTrayAuto1X` | 1 | 78560 | 78560 | 78560 | 1 | 78560×1 |
| `setEditTrayAuto2X` | 1 | 96935 | 96935 | 96935 | 1 | 96935×1 |
| `setEditTrayAuto3X` | 1 | 115410 | 115410 | 115410 | 1 | 115410×1 |
| `setEditTrayColorX` | 1 | 49420 | 49420 | 49420 | 1 | 49420×1 |
| `setEditTrayEmptyX` | 1 | 27910 | 27910 | 27910 | 1 | 27910×1 |
| `setEditTrayLoaderX` | 1 | -4540 | -4540 | -4540 | 1 | -4540×1 |

#### `[InArm]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `AutoCleanPick` | 2 | -2110 | -1765 | -1420 | 2 | -1420×1, -2110×1 |

#### `[InArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 1 | 40 | 40 | 40 | 1 | 40×1 |
| `Picker Ab` | 1 | -30 | -30 | -30 | 1 | -30×1 |
| `Picker Ad` | 1 | 10 | 10 | 10 | 1 | 10×1 |
| `Picker Ba` | 1 | 20 | 20 | 20 | 1 | 20×1 |
| `Picker Bc` | 1 | -10 | -10 | -10 | 1 | -10×1 |
| `Picker Bd` | 1 | -10 | -10 | -10 | 1 | -10×1 |

#### `[OutArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 1 | -30 | -30 | -30 | 1 | -30×1 |
| `Picker Ab` | 1 | 10 | 10 | 10 | 1 | 10×1 |
| `Picker Ad` | 1 | 10 | 10 | 10 | 1 | 10×1 |
| `Picker Ba` | 1 | 60 | 60 | 60 | 1 | 60×1 |
| `Picker Bb` | 1 | 70 | 70 | 70 | 1 | 70×1 |
| `Picker Bc` | 1 | 60 | 60 | 60 | 1 | 60×1 |
| `Picker Bd` | 1 | 80 | 80 | 80 | 1 | 80×1 |

#### `[Teach INI]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Update2` | 1 | 1 | 1 | 1 | 1 | 1×1 |
