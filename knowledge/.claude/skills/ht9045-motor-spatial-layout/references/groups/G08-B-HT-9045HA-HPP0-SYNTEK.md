# G08 — B · `HT-9045HA`

> **群組**：B (HT-9045 標準機 (6-site GPIB))  
> **HPP**：HPP=0 (窄 HP)  
> **GearRatio**：GearY~0.194(SYNTEK)  
> **樣本數**：7 筆  
> [← 返回索引](../teach-position-statistics.md)

## G08 · `HT-9045HA` · HPP=0 (窄 HP ~-46000) · GearY~0.194(SYNTEK)

- **機台群組**：B （HT-9045 標準機 (6-site GPIB)）
- **SubModel 分布**：0×7
- **Picker (USE_PICKER_COUNT) 分布**：1×7
- **Y Pitch (USE_IN_OUT_ARM_Y_PITCH) 分布**：0×7
- **GPIB Model 分布**：`9045GPIB`×7
- **樣本數**：7 筆 state record
- **獨立機台數**（依 Serial No 計）：2 台

### 機台清單

| Serial No | Factory | 樣本數 |
|-----------|---------|--------|
| `PLLS979` | KYEC | 2 |
| `PLLS983` | KYEC | 5 |

### 來源 State Records（最多列 15 筆）

- `ATK\Folder\2025-06-05 16_53_04_v3.21.871.2_QORVO check\2025-06-05 16_53_04` &nbsp;*(S/N: PLLS983)*
- `ATK\Folder\2025-09-18 11_13_30_Qorvo ART 883 Hangup` &nbsp;*(S/N: PLLS979)*
- `ATK\Folder\2025-09-24 14_53_30_V3.21.883.1_Teaching data bug\V883.1\2025-09-24 14_53_30` &nbsp;*(S/N: PLLS979)*
- `ATK\Old\2024-12-10 11_44_53` &nbsp;*(S/N: PLLS983)*
- `ATK\Old\2024-12-26 17_29_59` &nbsp;*(S/N: PLLS983)*
- `ATK\Old\2025-02-14 15_40_20_QORVO_ART GPIB_Hangup` &nbsp;*(S/N: PLLS983)*
- `ATK\Old\2025-02-28 15_45_59_QORVO_ART GPIB_Hangup` &nbsp;*(S/N: PLLS983)*

### Teach Position 統計

#### `[MInArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditInXPitch120` | 7 | 0 | 30 | 30 | 2 | 30×5, 0×2 |
| `setEditInXPitch40` | 7 | -5310 | -5280 | -5280 | 2 | -5280×5, -5310×2 |

#### `[MInArmPitchX2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditInX240` | 7 | -5320 | -5320 | -5320 | 1 | -5320×7 |

#### `[MInArmPitchY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditInY15` | 7 | -4862 | -4862 | -4862 | 1 | -4862×7 |
| `setEditInY60` | 7 | -1432 | -1432 | -1432 | 1 | -1432×7 |

#### `[MInArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanX` | 7 | 11505 | 11505 | 11548 | 2 | 11505×5, 11548×2 |
| `setEditHP1X` | 7 | 1402 | 1402 | 1402 | 1 | 1402×7 |
| `setEditHP2X` | 7 | 1403 | 1403 | 1403 | 1 | 1403×7 |
| `setEditInSht1X` | 7 | 32160 | 32160 | 32204 | 2 | 32160×5, 32204×2 |
| `setEditInSht2X` | 7 | 32135 | 32135 | 32186 | 2 | 32135×5, 32186×2 |
| `setEditLoaderX` | 7 | 21058 | 21058 | 21101 | 2 | 21058×5, 21101×2 |
| `setEditRotateX` | 7 | 33926 | 33926 | 33926 | 1 | 33926×7 |
| `setInPickX` | 7 | 21386 | 21386 | 21386 | 1 | 21386×7 |

#### `[MInArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanY` | 7 | -81838 | -81824 | -81824 | 2 | -81824×5, -81838×2 |
| `setEditHP1Y` | 7 | -42714 | -42714 | -42714 | 1 | -42714×7 |
| `setEditHP2Y` | 7 | 1158 | 1158 | 1158 | 1 | 1158×7 |
| `setEditInSht1Y` | 7 | -37851 | -37775 | -37775 | 2 | -37775×5, -37851×2 |
| `setEditInSht2Y` | 7 | -5855 | -5826 | -5826 | 2 | -5826×5, -5855×2 |
| `setEditLoaderY` | 7 | -56034 | -56013 | -56013 | 2 | -56013×5, -56034×2 |
| `setEditRotateY` | 7 | -26968 | -26968 | -26968 | 1 | -26968×7 |
| `setInPickY` | 7 | -57890 | -57890 | -57890 | 1 | -57890×7 |

#### `[MInArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditAutoClean` | 7 | -1400 | -1400 | 0 | 2 | -1400×6, 0×1 |
| `SetEditHP` | 7 | -1400 | -1400 | -1400 | 1 | -1400×7 |
| `SetEditPickInRotate` | 7 | -1000 | -1000 | -1000 | 1 | -1000×7 |
| `SetEditPickLoader` | 7 | -2620 | -2620 | -2500 | 2 | -2620×5, -2500×2 |
| `SetEditPlaceInRotate` | 7 | -800 | -800 | -800 | 1 | -800×7 |
| `SetEditPlaceInShuttle` | 7 | -2280 | -2280 | -2120 | 2 | -2280×5, -2120×2 |

#### `[MInRotate]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditRotateA` | 7 | 770 | 770 | 770 | 1 | 770×7 |

#### `[MInShutte1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht1OctSiteKit` | 7 | 0 | 11000 | 11000 | 2 | 11000×5, 0×2 |
| `edtSetEditIS1BarCode` | 7 | 0 | 16191 | 16191 | 2 | 16191×5, 0×2 |
| `edtSetEditOS1BarCode` | 7 | 0 | 20460 | 20460 | 2 | 20460×5, 0×2 |
| `setEditInSht1Left` | 7 | 0 | 125 | 125 | 2 | 125×5, 0×2 |
| `setEditInSht1Right` | 7 | 0 | 38653 | 38653 | 2 | 38653×5, 0×2 |
| `setEditOutSht1KitPos` | 7 | 0 | 20666 | 20666 | 2 | 20666×5, 0×2 |
| `setEditOutSht1OneRowKit` | 7 | 0 | 20655 | 20655 | 2 | 20655×5, 0×2 |

#### `[MInShutte2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht2OctSiteKit` | 7 | 0 | 11000 | 11000 | 2 | 11000×5, 0×2 |
| `edtSetEditIS2BarCode` | 7 | 0 | 16255 | 16255 | 2 | 16255×5, 0×2 |
| `edtSetEditOS2BarCode` | 7 | 0 | 20473 | 20473 | 2 | 20473×5, 0×2 |
| `setEditInSht2Left` | 7 | 0 | 92 | 92 | 2 | 92×5, 0×2 |
| `setEditInSht2Right` | 7 | 0 | 38650 | 38650 | 3 | 38650×4, 0×2, 38636×1 |
| `setEditOutSht2KitPos` | 7 | 0 | 20687 | 20687 | 2 | 20687×5, 0×2 |
| `setEditOutSht2OneRowKit` | 7 | 0 | 20593 | 20593 | 2 | 20593×5, 0×2 |

#### `[MInShuttle1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht1OctSiteKit` | 3 | 11000 | 11000 | 11000 | 1 | 11000×3 |
| `edtSetEditIS1BarCode` | 3 | 16191 | 16239 | 16239 | 2 | 16239×2, 16191×1 |
| `edtSetEditOS1BarCode` | 3 | 20460 | 20460 | 20460 | 1 | 20460×3 |
| `setEditInSht1Left` | 3 | 78 | 78 | 125 | 2 | 78×2, 125×1 |
| `setEditInSht1Right` | 3 | 38599 | 38599 | 38653 | 2 | 38599×2, 38653×1 |
| `setEditOutSht1KitPos` | 3 | 20407 | 20407 | 20666 | 2 | 20407×2, 20666×1 |
| `setEditOutSht1OneRowKit` | 3 | 20494 | 20494 | 20655 | 2 | 20494×2, 20655×1 |

#### `[MInShuttle2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht2OctSiteKit` | 3 | 11000 | 11000 | 11000 | 1 | 11000×3 |
| `edtSetEditIS2BarCode` | 3 | 16255 | 16419 | 16419 | 2 | 16419×2, 16255×1 |
| `edtSetEditOS2BarCode` | 3 | 20473 | 20473 | 20473 | 1 | 20473×3 |
| `setEditInSht2Left` | 3 | 92 | 117 | 117 | 2 | 117×2, 92×1 |
| `setEditInSht2Right` | 3 | 38636 | 38657 | 38657 | 2 | 38657×2, 38636×1 |
| `setEditOutSht2KitPos` | 3 | 20447 | 20447 | 20687 | 2 | 20447×2, 20687×1 |
| `setEditOutSht2OneRowKit` | 3 | 20585 | 20585 | 20593 | 2 | 20585×2, 20593×1 |

#### `[MOutArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditOutXPitch120` | 7 | 0 | 60 | 60 | 2 | 60×5, 0×2 |
| `setEditOutXPitch40` | 7 | -5280 | -5250 | -5250 | 2 | -5250×5, -5280×2 |

#### `[MOutArmPitchX2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditOutX2120` | 7 | -40 | -40 | -40 | 1 | -40×7 |
| `setEditOutX240` | 7 | -5370 | -5370 | -5370 | 1 | -5370×7 |

#### `[MOutArmPitchY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditOutY15` | 7 | -4987 | -4987 | -4987 | 1 | -4987×7 |
| `setEditOutY60` | 7 | -1564 | -1564 | -1564 | 1 | -1564×7 |

#### `[MOutArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1X` | 7 | -53760 | -53732 | -53732 | 2 | -53732×5, -53760×2 |
| `setEditAuto2X` | 7 | -35287 | -35287 | -35269 | 2 | -35287×5, -35269×2 |
| `setEditAuto3X` | 7 | -16785 | -16785 | -16762 | 2 | -16785×5, -16762×2 |
| `setEditFix1X` | 7 | -35831 | -35800 | -35800 | 2 | -35800×5, -35831×2 |
| `setEditFix2X` | 7 | -21721 | -21710 | -21710 | 2 | -21710×5, -21721×2 |
| `setEditFix3X` | 7 | -7651 | -7629 | -7629 | 2 | -7629×5, -7651×2 |
| `setEditOutSht1X` | 7 | -49073 | -49038 | -49038 | 2 | -49038×5, -49073×2 |
| `setEditOutSht2X` | 7 | -49066 | -49062 | -49062 | 2 | -49062×5, -49066×2 |
| `setEditRotateOutX` | 7 | -44575 | -44575 | -44575 | 1 | -44575×7 |
| `setOutPickX` | 7 | -53582 | -53582 | -53582 | 1 | -53582×7 |

#### `[MOutArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1Y` | 7 | -56279 | -56279 | -55957 | 2 | -56279×5, -55957×2 |
| `setEditAuto2Y` | 7 | -56213 | -56213 | -55944 | 2 | -56213×5, -55944×2 |
| `setEditAuto3Y` | 7 | -56156 | -56156 | -55953 | 2 | -56156×5, -55953×2 |
| `setEditFix1Y` | 7 | -10459 | -10459 | -10374 | 2 | -10459×5, -10374×2 |
| `setEditFix2Y` | 7 | -10439 | -10439 | -10381 | 2 | -10439×5, -10381×2 |
| `setEditFix3Y` | 7 | -10431 | -10431 | -10389 | 2 | -10431×5, -10389×2 |
| `setEditOutSht1Y` | 7 | -37832 | -37832 | -37771 | 2 | -37832×5, -37771×2 |
| `setEditOutSht2Y` | 7 | -5848 | -5848 | -5791 | 2 | -5848×5, -5791×2 |
| `setEditRotateOutY` | 7 | -26913 | -26913 | -26913 | 1 | -26913×7 |
| `setOutPickY` | 7 | -57624 | -57624 | -57624 | 1 | -57624×7 |

#### `[MOutArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditPickOutRotate` | 7 | -1000 | -1000 | -1000 | 1 | -1000×7 |
| `SetEditPickOutSht` | 7 | -2200 | -2200 | -2200 | 1 | -2200×7 |
| `SetEditPlaceAuto` | 7 | -2450 | -2450 | -2450 | 1 | -2450×7 |
| `SetEditPlaceFix` | 7 | -1700 | -1700 | -1700 | 1 | -1700×7 |
| `SetEditPlaceFix2` | 7 | -490 | -490 | -490 | 1 | -490×7 |
| `SetEditPlaceOutRotate` | 7 | -800 | -800 | -800 | 1 | -800×7 |

#### `[MOutRotate]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditRotateOutA` | 7 | 722 | 722 | 722 | 1 | 722×7 |

#### `[MTestY1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Y` | 7 | -134 | -134 | -129 | 2 | -134×5, -129×2 |
| `setEditIndex1ToSocketY` | 7 | 15794 | 15794 | 15851 | 2 | 15794×5, 15851×2 |

#### `[MTestY2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Y` | 7 | 110 | 110 | 111 | 2 | 110×5, 111×2 |
| `setEditIndex2ToSocketY` | 7 | -15911 | -15911 | -15896 | 2 | -15911×5, -15896×2 |

#### `[MTestZ1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Z` | 7 | -4857 | -4857 | -4857 | 1 | -4857×7 |
| `setEditTestZSafePos` | 7 | 10 | 10 | 10 | 1 | 10×7 |
| `setEditWaitTestZDown` | 7 | -1000 | -1000 | -1000 | 1 | -1000×7 |

#### `[MTestZ2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Z` | 7 | -4806 | -4806 | -4806 | 1 | -4806×7 |
| `setEditTestZSafePos` | 7 | 10 | 10 | 10 | 1 | 10×7 |

#### `[MTrayX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditTrayAuto1X` | 7 | 79465 | 79465 | 80115 | 3 | 79465×4, 80115×2, 79715×1 |
| `setEditTrayAuto2X` | 7 | 98050 | 98050 | 98600 | 3 | 98050×4, 98600×2, 98300×1 |
| `setEditTrayAuto3X` | 7 | 116595 | 116595 | 117145 | 3 | 116595×4, 117145×2, 116845×1 |
| `setEditTrayColorX` | 7 | 50160 | 50160 | 50710 | 3 | 50160×4, 50710×2, 50410×1 |
| `setEditTrayEmptyX` | 7 | 28230 | 28230 | 28880 | 3 | 28230×4, 28880×2, 28480×1 |
| `setEditTrayLoaderX` | 7 | -4512 | -4512 | -3762 | 3 | -4512×4, -3762×2, -4262×1 |

#### `[ArmAlignment]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `InArmAlignmentPitch_Y` | 6 | 1500 | 1500 | 1500 | 1 | 1500×6 |
| `InArmZAlignmentAa` | 6 | -500 | -500 | -500 | 1 | -500×6 |
| `InArmZAlignmentAb` | 6 | -500 | -500 | -500 | 1 | -500×6 |
| `InArmZAlignmentAc` | 6 | -500 | -500 | -500 | 1 | -500×6 |
| `InArmZAlignmentAd` | 6 | -500 | -500 | -500 | 1 | -500×6 |
| `InArmZAlignmentBa` | 6 | -500 | -500 | -500 | 1 | -500×6 |
| `InArmZAlignmentBb` | 6 | -500 | -500 | -500 | 1 | -500×6 |
| `InArmZAlignmentBc` | 6 | -500 | -500 | -500 | 1 | -500×6 |
| `InArmZAlignmentBd` | 6 | -500 | -500 | -500 | 1 | -500×6 |
| `OutArmAlignmentPitch_Y` | 6 | 1500 | 1500 | 1500 | 1 | 1500×6 |
| `OutArmZAlignmentAa` | 6 | -500 | -500 | -500 | 1 | -500×6 |
| `OutArmZAlignmentAb` | 6 | -500 | -500 | -500 | 1 | -500×6 |
| `OutArmZAlignmentAc` | 6 | -500 | -500 | -500 | 1 | -500×6 |
| `OutArmZAlignmentAd` | 6 | -500 | -500 | -500 | 1 | -500×6 |
| `OutArmZAlignmentBa` | 6 | -500 | -500 | -500 | 1 | -500×6 |
| `OutArmZAlignmentBb` | 6 | -500 | -500 | -500 | 1 | -500×6 |
| `OutArmZAlignmentBc` | 6 | -500 | -500 | -500 | 1 | -500×6 |
| `OutArmZAlignmentBd` | 6 | -500 | -500 | -500 | 1 | -500×6 |

#### `[InArm]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `AutoCleanPick` | 7 | -1400 | -1400 | -1400 | 1 | -1400×7 |

#### `[InArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 7 | 11 | 11 | 55 | 2 | 11×5, 55×2 |
| `Picker Ab` | 7 | -6 | -6 | 5 | 2 | -6×5, 5×2 |
| `Picker Ad` | 7 | 33 | 91 | 91 | 2 | 91×5, 33×2 |
| `Picker Ba` | 7 | -33 | -30 | -30 | 2 | -30×5, -33×2 |
| `Picker Bb` | 7 | 0 | 5 | 5 | 2 | 5×5, 0×2 |
| `Picker Bc` | 7 | -31 | -17 | -17 | 2 | -17×5, -31×2 |
| `Picker Bd` | 7 | -25 | 14 | 14 | 2 | 14×5, -25×2 |

#### `[OutArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 7 | 54 | 126 | 126 | 2 | 126×5, 54×2 |
| `Picker Ab` | 7 | 7 | 134 | 134 | 2 | 134×5, 7×2 |
| `Picker Ad` | 7 | 42 | 117 | 117 | 2 | 117×5, 42×2 |
| `Picker Ba` | 7 | -158 | -158 | -4 | 2 | -158×5, -4×2 |
| `Picker Bb` | 7 | -22 | 66 | 66 | 2 | 66×5, -22×2 |
| `Picker Bc` | 7 | -13 | 80 | 80 | 2 | 80×5, -13×2 |
| `Picker Bd` | 7 | 24 | 104 | 104 | 2 | 104×5, 24×2 |

#### `[Teach INI]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Update2` | 7 | 1 | 1 | 1 | 1 | 1×7 |
