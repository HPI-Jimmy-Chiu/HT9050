> 保存來源：`.claude/skills/ht9045-motor-spatial-layout/references/groups/G24-B-HT9045WA-HPP0-SMC.md`，main `eb7f47e7f`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../../common.md)。

<!-- preserved-content:start -->
# G24 — B · `HT9045WA`

> **群組**：B (HT-9045 標準機 (6-site GPIB))  
> **HPP**：HPP=0 (窄 HP)  
> **GearRatio**：GearY~0.350(SMC)  
> **樣本數**：6 筆  
> [← 返回索引](../teach-position-statistics.md)

## G24 · `HT9045WA` · HPP=0 (窄 HP ~-46000) · GearY~0.350(SMC)

- **機台群組**：B （HT-9045 標準機 (6-site GPIB)）
- **SubModel 分布**：0×6
- **Picker (USE_PICKER_COUNT) 分布**：1×6
- **Y Pitch (USE_IN_OUT_ARM_Y_PITCH) 分布**：0×6
- **GPIB Model 分布**：`9045GPIB`×6
- **樣本數**：6 筆 state record
- **獨立機台數**（依 Serial No 計）：2 台

### 機台清單

| Serial No | Factory | 樣本數 |
|-----------|---------|--------|
| `POLD1250` | KYEC | 1 |
| `POLD1251` | KYEC | 5 |

### 來源 State Records（最多列 15 筆）

- `JCET\2025-09-05 18_22_51` &nbsp;*(S/N: POLD1251)*
- `SJSM\2025-02-21 09_43_26` &nbsp;*(S/N: POLD1251)*
- `SJSM\2025-02-23 10_41_30` &nbsp;*(S/N: POLD1251)*
- `SJSM\2025-02-24 09_44_55` &nbsp;*(S/N: POLD1251)*
- `SJSM\2025-02-25 16_27_30` &nbsp;*(S/N: POLD1251)*
- `SJSM\2025-04-17 09_50_46 In Arm撞機` &nbsp;*(S/N: POLD1250)*

### Teach Position 統計

#### `[MInArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditInXPitch120` | 6 | 32 | 128 | 128 | 2 | 128×5, 32×1 |
| `setEditInXPitch40` | 6 | -5277 | -5198 | -5198 | 2 | -5198×5, -5277×1 |

#### `[MInArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanX` | 6 | 11382 | 11546 | 11546 | 2 | 11546×5, 11382×1 |
| `setEditHP1X` | 6 | 1536 | 1578 | 1578 | 2 | 1578×5, 1536×1 |
| `setEditHP2X` | 6 | 1540 | 1586 | 1586 | 2 | 1586×5, 1540×1 |
| `setEditInSht1X` | 6 | 31993 | 32009 | 32009 | 2 | 32009×5, 31993×1 |
| `setEditInSht2X` | 6 | 31995 | 31995 | 32036 | 2 | 31995×5, 32036×1 |
| `setEditLoaderX` | 6 | 20895 | 21051 | 21051 | 2 | 21051×5, 20895×1 |
| `setEditRotateX` | 6 | 35364 | 35364 | 35364 | 1 | 35364×6 |
| `setInPickX` | 6 | 21386 | 21386 | 21386 | 1 | 21386×6 |

#### `[MInArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanY` | 6 | -81830 | -81830 | -81727 | 2 | -81830×5, -81727×1 |
| `setEditHP1Y` | 6 | -46597 | -46597 | -46406 | 2 | -46597×5, -46406×1 |
| `setEditHP2Y` | 6 | -7536 | -7536 | -7424 | 2 | -7536×5, -7424×1 |
| `setEditInSht1Y` | 6 | -37740 | -37705 | -37705 | 2 | -37705×5, -37740×1 |
| `setEditInSht2Y` | 6 | -5770 | -5718 | -5718 | 2 | -5718×5, -5770×1 |
| `setEditLoaderY` | 6 | -56071 | -56071 | -56026 | 2 | -56071×5, -56026×1 |
| `setEditRotateY` | 6 | -30049 | -30049 | -30049 | 1 | -30049×6 |
| `setInPickY` | 6 | -57890 | -57890 | -57890 | 1 | -57890×6 |

#### `[MInArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditAutoClean` | 6 | -2000 | -2000 | -2000 | 1 | -2000×6 |
| `SetEditHP` | 6 | -2000 | -2000 | -2000 | 1 | -2000×6 |
| `SetEditPickLoader` | 6 | -1800 | -1800 | -1800 | 1 | -1800×6 |
| `SetEditPlaceInShuttle` | 6 | -1940 | -1940 | -1940 | 1 | -1940×6 |

#### `[MInRotate]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditRotateA` | 6 | 605 | 605 | 605 | 1 | 605×6 |

#### `[MInShutte1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht1OctSiteKit` | 6 | 11000 | 11000 | 11000 | 1 | 11000×6 |
| `edtSetEditIS1BarCode` | 6 | 13347 | 17212 | 17212 | 2 | 17212×5, 13347×1 |
| `edtSetEditOS1BarCode` | 6 | 25500 | 25500 | 25500 | 1 | 25500×6 |
| `setEditInSht1Left` | 6 | 11 | 59 | 59 | 2 | 59×5, 11×1 |
| `setEditInSht1Right` | 6 | 38461 | 38512 | 38512 | 2 | 38512×5, 38461×1 |
| `setEditOutSht1KitPos` | 6 | 20360 | 20360 | 20545 | 2 | 20360×5, 20545×1 |
| `setEditOutSht1OneRowKit` | 6 | 20224 | 20224 | 20313 | 2 | 20224×5, 20313×1 |

#### `[MInShutte2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht2OctSiteKit` | 6 | 11000 | 11000 | 11000 | 1 | 11000×6 |
| `edtSetEditIS2BarCode` | 6 | 13625 | 17379 | 17379 | 2 | 17379×5, 13625×1 |
| `edtSetEditOS2BarCode` | 6 | 25502 | 25502 | 25502 | 1 | 25502×6 |
| `setEditInSht2Left` | 6 | 35 | 35 | 56 | 2 | 35×5, 56×1 |
| `setEditInSht2Right` | 6 | 38490 | 38490 | 38497 | 2 | 38490×5, 38497×1 |
| `setEditOutSht2KitPos` | 6 | 20415 | 20415 | 20551 | 2 | 20415×5, 20551×1 |
| `setEditOutSht2OneRowKit` | 6 | 20243 | 20243 | 20394 | 2 | 20243×5, 20394×1 |

#### `[MOutArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditOutXPitch120` | 6 | 50 | 128 | 128 | 2 | 128×5, 50×1 |
| `setEditOutXPitch40` | 6 | -5227 | -5227 | -5217 | 2 | -5227×5, -5217×1 |

#### `[MOutArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1X` | 6 | -53597 | -53597 | -53552 | 2 | -53597×5, -53552×1 |
| `setEditAuto2X` | 6 | -35166 | -35166 | -34919 | 2 | -35166×5, -34919×1 |
| `setEditAuto3X` | 6 | -16670 | -16670 | -16555 | 2 | -16670×5, -16555×1 |
| `setEditFix1X` | 6 | -35771 | -35768 | -35768 | 2 | -35768×5, -35771×1 |
| `setEditFix2X` | 6 | -21785 | -21669 | -21669 | 2 | -21669×5, -21785×1 |
| `setEditFix3X` | 6 | -17539 | -17539 | -16692 | 2 | -17539×5, -16692×1 |
| `setEditOutSht1X` | 6 | -48903 | -48872 | -48872 | 2 | -48872×5, -48903×1 |
| `setEditOutSht2X` | 6 | -48900 | -48900 | -48890 | 2 | -48900×5, -48890×1 |
| `setEditRotateOutX` | 6 | -43213 | -43213 | -43213 | 1 | -43213×6 |
| `setOutPickX` | 6 | -53582 | -53582 | -53582 | 1 | -53582×6 |

#### `[MOutArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1Y` | 6 | -55980 | -55980 | -55955 | 2 | -55980×5, -55955×1 |
| `setEditAuto2Y` | 6 | -56024 | -56024 | -55955 | 2 | -56024×5, -55955×1 |
| `setEditAuto3Y` | 6 | -55992 | -55950 | -55950 | 2 | -55950×5, -55992×1 |
| `setEditFix1Y` | 6 | -10421 | -10421 | -10220 | 2 | -10421×5, -10220×1 |
| `setEditFix2Y` | 6 | -10421 | -10421 | -10174 | 2 | -10421×5, -10174×1 |
| `setEditFix3Y` | 6 | -10421 | -10421 | -10158 | 2 | -10421×5, -10158×1 |
| `setEditOutSht1Y` | 6 | -37701 | -37701 | -37591 | 2 | -37701×5, -37591×1 |
| `setEditOutSht2Y` | 6 | -5706 | -5706 | -5630 | 2 | -5706×5, -5630×1 |
| `setEditRotateOutY` | 6 | -29992 | -29992 | -29992 | 1 | -29992×6 |
| `setOutPickY` | 6 | -57624 | -57624 | -57624 | 1 | -57624×6 |

#### `[MOutArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditPickOutSht` | 6 | -2010 | -2010 | -2010 | 1 | -2010×6 |
| `SetEditPlaceAuto` | 6 | -1660 | -1660 | -1660 | 1 | -1660×6 |
| `SetEditPlaceFix` | 6 | -1650 | -1650 | -1650 | 1 | -1650×6 |

#### `[MOutRotate]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditRotateOutA` | 6 | 378 | 378 | 378 | 1 | 378×6 |

#### `[MPreciser]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditPreciserPitchClose` | 6 | -650 | -650 | -650 | 1 | -650×6 |
| `setEditPreciserPitchOpen` | 6 | -150 | -150 | -150 | 1 | -150×6 |

#### `[MTestY1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Y` | 6 | -126 | -104 | -88 | 3 | -104×4, -88×1, -126×1 |
| `setEditIndex1ToSocketY` | 6 | 15849 | 15886 | 15886 | 2 | 15886×5, 15849×1 |

#### `[MTestY2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Y` | 6 | 51 | 51 | 109 | 2 | 51×5, 109×1 |
| `setEditIndex2ToSocketY` | 6 | -15885 | -15885 | -15834 | 2 | -15885×5, -15834×1 |

#### `[MTestZ1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Z` | 6 | -4857 | -4857 | -4857 | 1 | -4857×6 |
| `setEditTestZSafePos` | 6 | 10 | 10 | 10 | 1 | 10×6 |
| `setEditWaitTestZDown` | 6 | -1000 | -1000 | -1000 | 1 | -1000×6 |

#### `[MTestZ2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Z` | 6 | -4806 | -4806 | -4806 | 1 | -4806×6 |
| `setEditTestZSafePos` | 6 | 10 | 10 | 10 | 1 | 10×6 |

#### `[MTrayX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditTrayAuto1X` | 6 | 78755 | 78900 | 78900 | 2 | 78900×5, 78755×1 |
| `setEditTrayAuto2X` | 6 | 97205 | 97360 | 97360 | 2 | 97360×5, 97205×1 |
| `setEditTrayAuto3X` | 6 | 115650 | 115810 | 115810 | 2 | 115810×5, 115650×1 |
| `setEditTrayColorX` | 6 | 49655 | 49740 | 49740 | 2 | 49740×5, 49655×1 |
| `setEditTrayEmptyX` | 6 | 28090 | 28090 | 28110 | 2 | 28090×5, 28110×1 |
| `setEditTrayLoaderX` | 6 | -4320 | -4320 | -4300 | 2 | -4320×5, -4300×1 |

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
| `AutoCleanPick` | 6 | -2000 | -2000 | -2000 | 1 | -2000×6 |

#### `[InArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 6 | 3 | 3 | 60 | 2 | 3×5, 60×1 |
| `Picker Ab` | 6 | 4 | 4 | 20 | 2 | 4×5, 20×1 |
| `Picker Ad` | 6 | -6 | -6 | 40 | 2 | -6×5, 40×1 |
| `Picker Ba` | 6 | -10 | -10 | 30 | 2 | -10×5, 30×1 |
| `Picker Bb` | 6 | -20 | -20 | 40 | 2 | -20×5, 40×1 |
| `Picker Bc` | 6 | 1 | 1 | 20 | 2 | 1×5, 20×1 |
| `Picker Bd` | 6 | 2 | 2 | 20 | 2 | 2×5, 20×1 |

#### `[OutArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 6 | 60 | 106 | 106 | 2 | 106×5, 60×1 |
| `Picker Ab` | 6 | -30 | -30 | 10 | 2 | -30×5, 10×1 |
| `Picker Ad` | 6 | -10 | -10 | 30 | 2 | -10×5, 30×1 |
| `Picker Ba` | 6 | 30 | 31 | 31 | 2 | 31×5, 30×1 |
| `Picker Bb` | 6 | -3 | -3 | 30 | 2 | -3×5, 30×1 |
| `Picker Bc` | 6 | -21 | -21 | 40 | 2 | -21×5, 40×1 |
| `Picker Bd` | 6 | -35 | -35 | 40 | 2 | -35×5, 40×1 |

#### `[Teach INI]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Update2` | 6 | 1 | 1 | 1 | 1 | 1×6 |

<!-- preserved-content:end -->
