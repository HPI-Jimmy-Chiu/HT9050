> 保存來源：`.claude/skills/ht9045-motor-spatial-layout/references/groups/G31-B_-HT-9045F-HPP0-SYNTEK.md`，main `eb7f47e7f`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../../common.md)。

<!-- preserved-content:start -->
# G31 — B* · `HT-9045F`

> **群組**：B* (HT-9045F (ATK 客製獨立))  
> **HPP**：HPP=0 (窄 HP)  
> **GearRatio**：GearY~0.194(SYNTEK)  
> **樣本數**：1 筆  
> [← 返回索引](../teach-position-statistics.md)

## G31 · `HT-9045F` · HPP=0 (窄 HP ~-46000) · GearY~0.194(SYNTEK)

- **機台群組**：B* （HT-9045F (ATK 客製獨立)）
- **SubModel 分布**：0×1
- **Picker (USE_PICKER_COUNT) 分布**：1×1
- **Y Pitch (USE_IN_OUT_ARM_Y_PITCH) 分布**：0×1
- **GPIB Model 分布**：`9045GPIB`×1
- **樣本數**：1 筆 state record
- **獨立機台數**（依 Serial No 計）：1 台

### 機台清單

| Serial No | Factory | 樣本數 |
|-----------|---------|--------|
| `1391` | KYEC | 1 |

### 來源 State Records（最多列 15 筆）

- `ATK\Old\2024-11-08 11_06_40_2DID_Infinite Retry` &nbsp;*(S/N: 1391)*

### Teach Position 統計

#### `[MInArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditInXPitch40` | 1 | -5342 | -5342 | -5342 | 1 | -5342×1 |

#### `[MInArmPitchX2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditInX240` | 1 | -5320 | -5320 | -5320 | 1 | -5320×1 |

#### `[MInArmPitchY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditInY15` | 1 | -4862 | -4862 | -4862 | 1 | -4862×1 |
| `setEditInY60` | 1 | -1432 | -1432 | -1432 | 1 | -1432×1 |

#### `[MInArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanX` | 1 | 11433 | 11433 | 11433 | 1 | 11433×1 |
| `setEditHP1X` | 1 | 1402 | 1402 | 1402 | 1 | 1402×1 |
| `setEditHP2X` | 1 | 1403 | 1403 | 1403 | 1 | 1403×1 |
| `setEditInSht1X` | 1 | 32064 | 32064 | 32064 | 1 | 32064×1 |
| `setEditInSht2X` | 1 | 32046 | 32046 | 32046 | 1 | 32046×1 |
| `setEditLoaderX` | 1 | 20956 | 20956 | 20956 | 1 | 20956×1 |
| `setEditRotateX` | 1 | 33926 | 33926 | 33926 | 1 | 33926×1 |
| `setInPickX` | 1 | 21386 | 21386 | 21386 | 1 | 21386×1 |

#### `[MInArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanY` | 1 | -81985 | -81985 | -81985 | 1 | -81985×1 |
| `setEditHP1Y` | 1 | -42714 | -42714 | -42714 | 1 | -42714×1 |
| `setEditHP2Y` | 1 | 1158 | 1158 | 1158 | 1 | 1158×1 |
| `setEditInSht1Y` | 1 | -38000 | -38000 | -38000 | 1 | -38000×1 |
| `setEditInSht2Y` | 1 | -6042 | -6042 | -6042 | 1 | -6042×1 |
| `setEditLoaderY` | 1 | -55807 | -55807 | -55807 | 1 | -55807×1 |
| `setEditRotateY` | 1 | -26968 | -26968 | -26968 | 1 | -26968×1 |
| `setInPickY` | 1 | -57890 | -57890 | -57890 | 1 | -57890×1 |

#### `[MInArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditHP` | 1 | -1372 | -1372 | -1372 | 1 | -1372×1 |
| `SetEditPickInRotate` | 1 | -1000 | -1000 | -1000 | 1 | -1000×1 |
| `SetEditPickLoader` | 1 | -2450 | -2450 | -2450 | 1 | -2450×1 |
| `SetEditPlaceInRotate` | 1 | -800 | -800 | -800 | 1 | -800×1 |
| `SetEditPlaceInShuttle` | 1 | -2125 | -2125 | -2125 | 1 | -2125×1 |

#### `[MInRotate]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditRotateA` | 1 | 770 | 770 | 770 | 1 | 770×1 |

#### `[MInShutte1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht1OctSiteKit` | 1 | 11000 | 11000 | 11000 | 1 | 11000×1 |
| `edtSetEditIS1BarCode` | 1 | 16275 | 16275 | 16275 | 1 | 16275×1 |
| `edtSetEditOS1BarCode` | 1 | 25500 | 25500 | 25500 | 1 | 25500×1 |
| `setEditInSht1Left` | 1 | 80 | 80 | 80 | 1 | 80×1 |
| `setEditInSht1Right` | 1 | 38508 | 38508 | 38508 | 1 | 38508×1 |
| `setEditOutSht1KitPos` | 1 | 20347 | 20347 | 20347 | 1 | 20347×1 |
| `setEditOutSht1OneRowKit` | 1 | 20492 | 20492 | 20492 | 1 | 20492×1 |

#### `[MInShutte2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht2OctSiteKit` | 1 | 11000 | 11000 | 11000 | 1 | 11000×1 |
| `edtSetEditIS2BarCode` | 1 | 16117 | 16117 | 16117 | 1 | 16117×1 |
| `edtSetEditOS2BarCode` | 1 | 25500 | 25500 | 25500 | 1 | 25500×1 |
| `setEditInSht2Left` | 1 | 73 | 73 | 73 | 1 | 73×1 |
| `setEditInSht2Right` | 1 | 38580 | 38580 | 38580 | 1 | 38580×1 |
| `setEditOutSht2KitPos` | 1 | 20416 | 20416 | 20416 | 1 | 20416×1 |
| `setEditOutSht2OneRowKit` | 1 | 20581 | 20581 | 20581 | 1 | 20581×1 |

#### `[MOutArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditOutXPitch40` | 1 | -5345 | -5345 | -5345 | 1 | -5345×1 |

#### `[MOutArmPitchX2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditOutX2120` | 1 | -40 | -40 | -40 | 1 | -40×1 |
| `setEditOutX240` | 1 | -5370 | -5370 | -5370 | 1 | -5370×1 |

#### `[MOutArmPitchY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditOutY15` | 1 | -4987 | -4987 | -4987 | 1 | -4987×1 |
| `setEditOutY60` | 1 | -1564 | -1564 | -1564 | 1 | -1564×1 |

#### `[MOutArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1X` | 1 | -53723 | -53723 | -53723 | 1 | -53723×1 |
| `setEditAuto2X` | 1 | -35274 | -35274 | -35274 | 1 | -35274×1 |
| `setEditAuto3X` | 1 | -16833 | -16833 | -16833 | 1 | -16833×1 |
| `setEditFix1X` | 1 | -35789 | -35789 | -35789 | 1 | -35789×1 |
| `setEditFix2X` | 1 | -21713 | -21713 | -21713 | 1 | -21713×1 |
| `setEditFix3X` | 1 | -7641 | -7641 | -7641 | 1 | -7641×1 |
| `setEditOutSht1X` | 1 | -49007 | -49007 | -49007 | 1 | -49007×1 |
| `setEditOutSht2X` | 1 | -49011 | -49011 | -49011 | 1 | -49011×1 |
| `setEditRotateOutX` | 1 | -44575 | -44575 | -44575 | 1 | -44575×1 |
| `setOutPickX` | 1 | -53582 | -53582 | -53582 | 1 | -53582×1 |

#### `[MOutArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1Y` | 1 | -55884 | -55884 | -55884 | 1 | -55884×1 |
| `setEditAuto2Y` | 1 | -55876 | -55876 | -55876 | 1 | -55876×1 |
| `setEditAuto3Y` | 1 | -55912 | -55912 | -55912 | 1 | -55912×1 |
| `setEditFix1Y` | 1 | -10614 | -10614 | -10614 | 1 | -10614×1 |
| `setEditFix2Y` | 1 | -10596 | -10596 | -10596 | 1 | -10596×1 |
| `setEditFix3Y` | 1 | -10623 | -10623 | -10623 | 1 | -10623×1 |
| `setEditOutSht1Y` | 1 | -37926 | -37926 | -37926 | 1 | -37926×1 |
| `setEditOutSht2Y` | 1 | -5961 | -5961 | -5961 | 1 | -5961×1 |
| `setEditRotateOutY` | 1 | -26913 | -26913 | -26913 | 1 | -26913×1 |
| `setOutPickY` | 1 | -57624 | -57624 | -57624 | 1 | -57624×1 |

#### `[MOutArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditPickOutRotate` | 1 | -1000 | -1000 | -1000 | 1 | -1000×1 |
| `SetEditPickOutSht` | 1 | -2170 | -2170 | -2170 | 1 | -2170×1 |
| `SetEditPlaceAuto` | 1 | -2300 | -2300 | -2300 | 1 | -2300×1 |
| `SetEditPlaceFix` | 1 | -1500 | -1500 | -1500 | 1 | -1500×1 |
| `SetEditPlaceFix2` | 1 | -490 | -490 | -490 | 1 | -490×1 |
| `SetEditPlaceOutRotate` | 1 | -800 | -800 | -800 | 1 | -800×1 |

#### `[MOutRotate]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditRotateOutA` | 1 | 722 | 722 | 722 | 1 | 722×1 |

#### `[MTestY1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Y` | 1 | -75 | -75 | -75 | 1 | -75×1 |
| `setEditIndex1ToSocketY` | 1 | 15883 | 15883 | 15883 | 1 | 15883×1 |

#### `[MTestY2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Y` | 1 | 166 | 166 | 166 | 1 | 166×1 |
| `setEditIndex2ToSocketY` | 1 | -15823 | -15823 | -15823 | 1 | -15823×1 |

#### `[MTestZ1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Z` | 1 | -4857 | -4857 | -4857 | 1 | -4857×1 |
| `setEditTestZSafePos` | 1 | 10 | 10 | 10 | 1 | 10×1 |
| `setEditWaitTestZDown` | 1 | -1000 | -1000 | -1000 | 1 | -1000×1 |

#### `[MTestZ2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Z` | 1 | -4806 | -4806 | -4806 | 1 | -4806×1 |
| `setEditTestZSafePos` | 1 | 10 | 10 | 10 | 1 | 10×1 |

#### `[MTrayX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditTrayAuto1X` | 1 | 79740 | 79740 | 79740 | 1 | 79740×1 |
| `setEditTrayAuto2X` | 1 | 98320 | 98320 | 98320 | 1 | 98320×1 |
| `setEditTrayAuto3X` | 1 | 116860 | 116860 | 116860 | 1 | 116860×1 |
| `setEditTrayColorX` | 1 | 50420 | 50420 | 50420 | 1 | 50420×1 |
| `setEditTrayEmptyX` | 1 | 28640 | 28640 | 28640 | 1 | 28640×1 |
| `setEditTrayLoaderX` | 1 | -3834 | -3834 | -3834 | 1 | -3834×1 |

#### `[InArm]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `AutoCleanPick` | 1 | -1372 | -1372 | -1372 | 1 | -1372×1 |

#### `[InArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 1 | 14 | 14 | 14 | 1 | 14×1 |
| `Picker Ab` | 1 | -1 | -1 | -1 | 1 | -1×1 |
| `Picker Ad` | 1 | -42 | -42 | -42 | 1 | -42×1 |
| `Picker Ba` | 1 | -27 | -27 | -27 | 1 | -27×1 |
| `Picker Bb` | 1 | -21 | -21 | -21 | 1 | -21×1 |
| `Picker Bc` | 1 | -3 | -3 | -3 | 1 | -3×1 |
| `Picker Bd` | 1 | -52 | -52 | -52 | 1 | -52×1 |

#### `[OutArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 1 | -8 | -8 | -8 | 1 | -8×1 |
| `Picker Ab` | 1 | -15 | -15 | -15 | 1 | -15×1 |
| `Picker Ad` | 1 | -16 | -16 | -16 | 1 | -16×1 |
| `Picker Ba` | 1 | -35 | -35 | -35 | 1 | -35×1 |
| `Picker Bb` | 1 | -31 | -31 | -31 | 1 | -31×1 |
| `Picker Bc` | 1 | -11 | -11 | -11 | 1 | -11×1 |
| `Picker Bd` | 1 | -50 | -50 | -50 | 1 | -50×1 |

#### `[Teach INI]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Update2` | 1 | 1 | 1 | 1 | 1 | 1×1 |

<!-- preserved-content:end -->
