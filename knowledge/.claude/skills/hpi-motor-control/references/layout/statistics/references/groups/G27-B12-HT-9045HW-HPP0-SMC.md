> 保存來源：`.claude/skills/ht9045-motor-spatial-layout/references/groups/G27-B12-HT-9045HW-HPP0-SMC.md`，main `eb7f47e7f`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../../common.md)。

<!-- preserved-content:start -->
# G27 — B12 · `HT-9045HW`

> **群組**：B12 (HT-9045 12-site GPIB 變體 (9045GPIB_12Site))  
> **HPP**：HPP=0 (窄 HP)  
> **GearRatio**：GearY~0.350(SMC)  
> **樣本數**：3 筆  
> [← 返回索引](../teach-position-statistics.md)

## G27 · `HT-9045HW` · HPP=0 (窄 HP ~-46000) · GearY~0.350(SMC)

- **機台群組**：B12 （HT-9045 12-site GPIB 變體 (9045GPIB_12Site)）
- **SubModel 分布**：0×3
- **Picker (USE_PICKER_COUNT) 分布**：1×3
- **Y Pitch (USE_IN_OUT_ARM_Y_PITCH) 分布**：0×3
- **GPIB Model 分布**：`9045GPIB_12Site`×3
- **樣本數**：3 筆 state record
- **獨立機台數**（依 Serial No 計）：1 台

### 機台清單

| Serial No | Factory | 樣本數 |
|-----------|---------|--------|
| `ILD084` | KYEC | 3 |

### 來源 State Records（最多列 15 筆）

- `JCET\2025-08-22 01_40_50` &nbsp;*(S/N: ILD084)*
- `JCET\2025-09-16 14_06_07` &nbsp;*(S/N: ILD084)*
- `JCET\2025-10-08 08_52_07` &nbsp;*(S/N: ILD084)*

### Teach Position 統計

#### `[MInArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditInXPitch120` | 3 | 42 | 42 | 42 | 1 | 42×3 |
| `setEditInXPitch40` | 3 | -5265 | -5265 | -5265 | 1 | -5265×3 |

#### `[MInArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanX` | 3 | 11385 | 11385 | 11385 | 1 | 11385×3 |
| `setEditHP1X` | 3 | 1540 | 1540 | 1540 | 1 | 1540×3 |
| `setEditHP2X` | 3 | 1550 | 1550 | 1550 | 1 | 1550×3 |
| `setEditInSht1X` | 3 | 32045 | 32045 | 32045 | 1 | 32045×3 |
| `setEditInSht2X` | 3 | 32033 | 32033 | 32033 | 1 | 32033×3 |
| `setEditLoaderX` | 3 | 20944 | 20944 | 20944 | 1 | 20944×3 |
| `setInPickX` | 3 | 21386 | 21386 | 21386 | 1 | 21386×3 |

#### `[MInArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanY` | 3 | -81727 | -81727 | -81727 | 1 | -81727×3 |
| `setEditHP1Y` | 3 | -46399 | -46399 | -46399 | 1 | -46399×3 |
| `setEditHP2Y` | 3 | -7408 | -7408 | -7408 | 1 | -7408×3 |
| `setEditInSht1Y` | 3 | -37746 | -37746 | -37746 | 1 | -37746×3 |
| `setEditInSht2Y` | 3 | -5748 | -5748 | -5748 | 1 | -5748×3 |
| `setEditLoaderY` | 3 | -56032 | -56032 | -56032 | 1 | -56032×3 |
| `setInPickY` | 3 | -57890 | -57890 | -57890 | 1 | -57890×3 |

#### `[MInArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditAutoClean` | 3 | -1480 | -1480 | -1480 | 1 | -1480×3 |
| `SetEditHP` | 3 | -1480 | -1480 | -1480 | 1 | -1480×3 |
| `SetEditPickLoader` | 3 | -2290 | -2290 | -2290 | 1 | -2290×3 |
| `SetEditPlaceInShuttle` | 3 | -1780 | -1780 | -1780 | 1 | -1780×3 |

#### `[MInShutte1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht1OctSiteKit` | 3 | 11000 | 11000 | 11000 | 1 | 11000×3 |
| `edtSetEditIS1BarCode` | 3 | 17212 | 17212 | 17212 | 1 | 17212×3 |
| `edtSetEditOS1BarCode` | 3 | 25500 | 25500 | 25500 | 1 | 25500×3 |
| `setEditInSht1Left` | 3 | 1 | 1 | 1 | 1 | 1×3 |
| `setEditInSht1Right` | 3 | 38500 | 38500 | 38500 | 1 | 38500×3 |
| `setEditOutSht1KitPos` | 3 | 20494 | 20494 | 20494 | 1 | 20494×3 |
| `setEditOutSht1OneRowKit` | 3 | 20587 | 20587 | 20587 | 1 | 20587×3 |

#### `[MInShutte2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht2OctSiteKit` | 3 | 11000 | 11000 | 11000 | 1 | 11000×3 |
| `edtSetEditIS2BarCode` | 3 | 17379 | 17379 | 17379 | 1 | 17379×3 |
| `edtSetEditOS2BarCode` | 3 | 25502 | 25502 | 25502 | 1 | 25502×3 |
| `setEditInSht2Left` | 3 | 56 | 56 | 56 | 1 | 56×3 |
| `setEditInSht2Right` | 3 | 38561 | 38561 | 38561 | 1 | 38561×3 |
| `setEditOutSht2KitPos` | 3 | 20581 | 20581 | 20581 | 1 | 20581×3 |
| `setEditOutSht2OneRowKit` | 3 | 20565 | 20565 | 20565 | 1 | 20565×3 |

#### `[MOutArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditOutXPitch120` | 3 | 64 | 64 | 64 | 1 | 64×3 |
| `setEditOutXPitch40` | 3 | -5247 | -5247 | -5247 | 1 | -5247×3 |

#### `[MOutArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1X` | 3 | -53585 | -53585 | -53585 | 1 | -53585×3 |
| `setEditAuto2X` | 3 | -35088 | -35088 | -35088 | 1 | -35088×3 |
| `setEditAuto3X` | 3 | -16589 | -16589 | -16589 | 1 | -16589×3 |
| `setEditFix1X` | 3 | -35599 | -35599 | -35599 | 1 | -35599×3 |
| `setEditFix2X` | 3 | -21480 | -21480 | -21480 | 1 | -21480×3 |
| `setEditFix3X` | 3 | -16583 | -16583 | -16583 | 1 | -16583×3 |
| `setEditOutSht1X` | 3 | -48871 | -48871 | -48871 | 1 | -48871×3 |
| `setEditOutSht2X` | 3 | -48890 | -48890 | -48890 | 1 | -48890×3 |
| `setOutPickX` | 3 | -53582 | -53582 | -53582 | 1 | -53582×3 |

#### `[MOutArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1Y` | 3 | -55766 | -55766 | -55766 | 1 | -55766×3 |
| `setEditAuto2Y` | 3 | -55758 | -55758 | -55758 | 1 | -55758×3 |
| `setEditAuto3Y` | 3 | -55753 | -55753 | -55753 | 1 | -55753×3 |
| `setEditFix1Y` | 3 | -10229 | -10229 | -10229 | 1 | -10229×3 |
| `setEditFix2Y` | 3 | -10227 | -10227 | -10227 | 1 | -10227×3 |
| `setEditFix3Y` | 3 | -10243 | -10243 | -10243 | 1 | -10243×3 |
| `setEditOutSht1Y` | 3 | -37633 | -37633 | -37633 | 1 | -37633×3 |
| `setEditOutSht2Y` | 3 | -5620 | -5620 | -5620 | 1 | -5620×3 |
| `setOutPickY` | 3 | -57624 | -57624 | -57624 | 1 | -57624×3 |

#### `[MOutArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditPickOutSht` | 3 | -1760 | -1760 | -1760 | 1 | -1760×3 |
| `SetEditPlaceAuto` | 3 | -1910 | -1910 | -1910 | 1 | -1910×3 |
| `SetEditPlaceFix` | 3 | -1200 | -1200 | -1200 | 1 | -1200×3 |

#### `[MTestY1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Y` | 3 | -92 | -92 | -92 | 1 | -92×3 |
| `setEditIndex1ToSocketY` | 3 | 15855 | 15855 | 15855 | 1 | 15855×3 |

#### `[MTestY2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Y` | 3 | 70 | 70 | 70 | 1 | 70×3 |
| `setEditIndex2ToSocketY` | 3 | -15925 | -15925 | -15925 | 1 | -15925×3 |

#### `[MTestZ1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Z` | 3 | -4357 | -4357 | -4357 | 1 | -4357×3 |
| `setEditTestZSafePos` | 3 | 10 | 10 | 10 | 1 | 10×3 |
| `setEditWaitTestZDown` | 3 | -1000 | -1000 | -1000 | 1 | -1000×3 |

#### `[MTestZ2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Z` | 3 | -4306 | -4306 | -4306 | 1 | -4306×3 |
| `setEditTestZSafePos` | 3 | 10 | 10 | 10 | 1 | 10×3 |

#### `[MTrayX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditTrayAuto1X` | 3 | 78640 | 78640 | 78640 | 1 | 78640×3 |
| `setEditTrayAuto2X` | 3 | 97110 | 97110 | 97110 | 1 | 97110×3 |
| `setEditTrayAuto3X` | 3 | 115550 | 115550 | 115550 | 1 | 115550×3 |
| `setEditTrayColorX` | 3 | 49560 | 49560 | 49560 | 1 | 49560×3 |
| `setEditTrayEmptyX` | 3 | 27960 | 27960 | 27960 | 1 | 27960×3 |
| `setEditTrayLoaderX` | 3 | -4540 | -4540 | -4540 | 1 | -4540×3 |

#### `[ArmAlignment]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `InArmAlignmentPitch_Y` | 3 | 1500 | 1500 | 1500 | 1 | 1500×3 |
| `InArmZAlignmentAa` | 3 | -500 | -500 | -500 | 1 | -500×3 |
| `InArmZAlignmentAb` | 3 | -500 | -500 | -500 | 1 | -500×3 |
| `InArmZAlignmentAc` | 3 | -500 | -500 | -500 | 1 | -500×3 |
| `InArmZAlignmentAd` | 3 | -500 | -500 | -500 | 1 | -500×3 |
| `InArmZAlignmentBa` | 3 | -500 | -500 | -500 | 1 | -500×3 |
| `InArmZAlignmentBb` | 3 | -500 | -500 | -500 | 1 | -500×3 |
| `InArmZAlignmentBc` | 3 | -500 | -500 | -500 | 1 | -500×3 |
| `InArmZAlignmentBd` | 3 | -500 | -500 | -500 | 1 | -500×3 |
| `OutArmAlignmentPitch_Y` | 3 | 1500 | 1500 | 1500 | 1 | 1500×3 |
| `OutArmZAlignmentAa` | 3 | -500 | -500 | -500 | 1 | -500×3 |
| `OutArmZAlignmentAb` | 3 | -500 | -500 | -500 | 1 | -500×3 |
| `OutArmZAlignmentAc` | 3 | -500 | -500 | -500 | 1 | -500×3 |
| `OutArmZAlignmentAd` | 3 | -500 | -500 | -500 | 1 | -500×3 |
| `OutArmZAlignmentBa` | 3 | -500 | -500 | -500 | 1 | -500×3 |
| `OutArmZAlignmentBb` | 3 | -500 | -500 | -500 | 1 | -500×3 |
| `OutArmZAlignmentBc` | 3 | -500 | -500 | -500 | 1 | -500×3 |
| `OutArmZAlignmentBd` | 3 | -500 | -500 | -500 | 1 | -500×3 |

#### `[InArm]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `AutoCleanPick` | 3 | -1480 | -1480 | -1480 | 1 | -1480×3 |

#### `[InArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 3 | 20 | 20 | 20 | 1 | 20×3 |
| `Picker Ab` | 3 | -11 | -11 | -11 | 1 | -11×3 |
| `Picker Ad` | 3 | 20 | 20 | 20 | 1 | 20×3 |
| `Picker Ba` | 3 | -120 | -120 | -120 | 1 | -120×3 |
| `Picker Bb` | 3 | 30 | 30 | 30 | 1 | 30×3 |
| `Picker Bc` | 3 | 10 | 10 | 10 | 1 | 10×3 |
| `Picker Bd` | 3 | 40 | 40 | 40 | 1 | 40×3 |

#### `[OutArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 3 | -131 | -131 | -131 | 1 | -131×3 |
| `Picker Ab` | 3 | -161 | -161 | -161 | 1 | -161×3 |
| `Picker Ad` | 3 | -37 | -37 | -37 | 1 | -37×3 |
| `Picker Ba` | 3 | 33 | 33 | 33 | 1 | 33×3 |
| `Picker Bb` | 3 | -132 | -132 | -132 | 1 | -132×3 |
| `Picker Bc` | 3 | 11 | 11 | 11 | 1 | 11×3 |
| `Picker Bd` | 3 | -184 | -184 | -184 | 1 | -184×3 |

#### `[Teach INI]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Update2` | 3 | 1 | 1 | 1 | 1 | 1×3 |

<!-- preserved-content:end -->
