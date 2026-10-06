> 保存來源：`.claude/skills/ht9045-motor-spatial-layout/references/groups/G32-C-HT-9046A-HPP1-SMC.md`，main `eb7f47e7f`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../../common.md)。

<!-- preserved-content:start -->
# G32 — C · `HT-9046A`

> **群組**：C (HT-9046 系列 (16-site 真空))  
> **HPP**：HPP=1 (宬 HP)  
> **GearRatio**：GearY~0.350(SMC)  
> **樣本數**：1 筆  
> [← 返回索引](../teach-position-statistics.md)

## G32 · `HT-9046A` · HPP=1 (宬 HP ~-82000) · GearY~0.350(SMC)

- **機台群組**：C （HT-9046 系列 (16-site 真空)）
- **SubModel 分布**：0×1
- **Picker (USE_PICKER_COUNT) 分布**：1×1
- **Y Pitch (USE_IN_OUT_ARM_Y_PITCH) 分布**：0×1
- **GPIB Model 分布**：`9046GPIB`×1
- **樣本數**：1 筆 state record
- **獨立機台數**（依 Serial No 計）：1 台

### 機台清單

| Serial No | Factory | 樣本數 |
|-----------|---------|--------|
| `ILD088` | KYEC | 1 |

### 來源 State Records（最多列 15 筆）

- `JCET\2025-09-19 10_37_42` &nbsp;*(S/N: ILD088)*

### Teach Position 統計

#### `[MInArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditInXPitch40` | 1 | -5302 | -5302 | -5302 | 1 | -5302×1 |

#### `[MInArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanX` | 1 | 11363 | 11363 | 11363 | 1 | 11363×1 |
| `setEditHP1X` | 1 | 5436 | 5436 | 5436 | 1 | 5436×1 |
| `setEditHP2X` | 1 | 5439 | 5439 | 5439 | 1 | 5439×1 |
| `setEditInSht1X` | 1 | 32008 | 32008 | 32008 | 1 | 32008×1 |
| `setEditInSht2X` | 1 | 32021 | 32021 | 32021 | 1 | 32021×1 |
| `setEditLoaderX` | 1 | 20970 | 20970 | 20970 | 1 | 20970×1 |
| `setInPickX` | 1 | 21386 | 21386 | 21386 | 1 | 21386×1 |

#### `[MInArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAutoCleanY` | 1 | -81651 | -81651 | -81651 | 1 | -81651×1 |
| `setEditHP1Y` | 1 | -82392 | -82392 | -82392 | 1 | -82392×1 |
| `setEditHP2Y` | 1 | -43361 | -43361 | -43361 | 1 | -43361×1 |
| `setEditInSht1Y` | 1 | -37670 | -37670 | -37670 | 1 | -37670×1 |
| `setEditInSht2Y` | 1 | -5710 | -5710 | -5710 | 1 | -5710×1 |
| `setEditLoaderY` | 1 | -55912 | -55912 | -55912 | 1 | -55912×1 |
| `setInPickY` | 1 | -57890 | -57890 | -57890 | 1 | -57890×1 |

#### `[MInArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditHP` | 1 | -1440 | -1440 | -1440 | 1 | -1440×1 |
| `SetEditPickLoader` | 1 | -2090 | -2090 | -2090 | 1 | -2090×1 |
| `SetEditPlaceInShuttle` | 1 | -1800 | -1800 | -1800 | 1 | -1800×1 |

#### `[MInShutte1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht1OctSiteKit` | 1 | 10000 | 10000 | 10000 | 1 | 10000×1 |
| `edtSetEditIS1BarCode` | 1 | 17212 | 17212 | 17212 | 1 | 17212×1 |
| `edtSetEditOS1BarCode` | 1 | 25500 | 25500 | 25500 | 1 | 25500×1 |
| `setEditInSht1Left` | 1 | -21 | -21 | -21 | 1 | -21×1 |
| `setEditInSht1Right` | 1 | 38490 | 38490 | 38490 | 1 | 38490×1 |
| `setEditOutSht1KitPos` | 1 | 20490 | 20490 | 20490 | 1 | 20490×1 |
| `setEditOutSht1OneRowKit` | 1 | 20354 | 20354 | 20354 | 1 | 20354×1 |

#### `[MInShutte2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `edtEditInSht2OctSiteKit` | 1 | 10000 | 10000 | 10000 | 1 | 10000×1 |
| `edtSetEditIS2BarCode` | 1 | 17379 | 17379 | 17379 | 1 | 17379×1 |
| `edtSetEditOS2BarCode` | 1 | 25502 | 25502 | 25502 | 1 | 25502×1 |
| `setEditInSht2Left` | 1 | 33 | 33 | 33 | 1 | 33×1 |
| `setEditInSht2Right` | 1 | 38534 | 38534 | 38534 | 1 | 38534×1 |
| `setEditOutSht2KitPos` | 1 | 20537 | 20537 | 20537 | 1 | 20537×1 |
| `setEditOutSht2OneRowKit` | 1 | 20459 | 20459 | 20459 | 1 | 20459×1 |

#### `[MOutArmPitch]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditOutXPitch120` | 1 | 24 | 24 | 24 | 1 | 24×1 |
| `setEditOutXPitch40` | 1 | -5272 | -5272 | -5272 | 1 | -5272×1 |

#### `[MOutArmX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1X` | 1 | -53557 | -53557 | -53557 | 1 | -53557×1 |
| `setEditAuto2X` | 1 | -35089 | -35089 | -35089 | 1 | -35089×1 |
| `setEditAuto3X` | 1 | -16570 | -16570 | -16570 | 1 | -16570×1 |
| `setEditFix1X` | 1 | -35672 | -35672 | -35672 | 1 | -35672×1 |
| `setEditFix2X` | 1 | -21594 | -21594 | -21594 | 1 | -21594×1 |
| `setEditFix3X` | 1 | -16571 | -16571 | -16571 | 1 | -16571×1 |
| `setEditOutSht1X` | 1 | -48888 | -48888 | -48888 | 1 | -48888×1 |
| `setEditOutSht2X` | 1 | -48880 | -48880 | -48880 | 1 | -48880×1 |
| `setOutPickX` | 1 | -53582 | -53582 | -53582 | 1 | -53582×1 |

#### `[MOutArmY]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditAuto1Y` | 1 | -55896 | -55896 | -55896 | 1 | -55896×1 |
| `setEditAuto2Y` | 1 | -55898 | -55898 | -55898 | 1 | -55898×1 |
| `setEditAuto3Y` | 1 | -55877 | -55877 | -55877 | 1 | -55877×1 |
| `setEditFix1Y` | 1 | -10343 | -10343 | -10343 | 1 | -10343×1 |
| `setEditFix2Y` | 1 | -10333 | -10333 | -10333 | 1 | -10333×1 |
| `setEditFix3Y` | 1 | -10329 | -10329 | -10329 | 1 | -10329×1 |
| `setEditOutSht1Y` | 1 | -37667 | -37667 | -37667 | 1 | -37667×1 |
| `setEditOutSht2Y` | 1 | -5695 | -5695 | -5695 | 1 | -5695×1 |
| `setOutPickY` | 1 | -57624 | -57624 | -57624 | 1 | -57624×1 |

#### `[MOutArmZE]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `SetEditPickOutSht` | 1 | -1910 | -1910 | -1910 | 1 | -1910×1 |
| `SetEditPlaceAuto` | 1 | -2070 | -2070 | -2070 | 1 | -2070×1 |
| `SetEditPlaceFix` | 1 | -1180 | -1180 | -1180 | 1 | -1180×1 |

#### `[MTestY1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Y` | 1 | -87 | -87 | -87 | 1 | -87×1 |
| `setEditIndex1ToSocketY` | 1 | 15902 | 15902 | 15902 | 1 | 15902×1 |

#### `[MTestY2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Y` | 1 | 120 | 120 | 120 | 1 | 120×1 |
| `setEditIndex2ToSocketY` | 1 | -15885 | -15885 | -15885 | 1 | -15885×1 |

#### `[MTestZ1]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex1ToSht1Z` | 1 | -9357 | -9357 | -9357 | 1 | -9357×1 |
| `setEditTestZSafePos` | 1 | 10 | 10 | 10 | 1 | 10×1 |
| `setEditWaitTestZDown` | 1 | -1000 | -1000 | -1000 | 1 | -1000×1 |

#### `[MTestZ2]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditIndex2ToSht2Z` | 1 | -9306 | -9306 | -9306 | 1 | -9306×1 |
| `setEditTestZSafePos` | 1 | 10 | 10 | 10 | 1 | 10×1 |

#### `[MTrayX]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `setEditTrayAuto1X` | 1 | 79120 | 79120 | 79120 | 1 | 79120×1 |
| `setEditTrayAuto2X` | 1 | 97490 | 97490 | 97490 | 1 | 97490×1 |
| `setEditTrayAuto3X` | 1 | 115950 | 115950 | 115950 | 1 | 115950×1 |
| `setEditTrayColorX` | 1 | 49880 | 49880 | 49880 | 1 | 49880×1 |
| `setEditTrayEmptyX` | 1 | 28280 | 28280 | 28280 | 1 | 28280×1 |
| `setEditTrayLoaderX` | 1 | -4250 | -4250 | -4250 | 1 | -4250×1 |

#### `[InArm]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `AutoCleanPick` | 1 | -1440 | -1440 | -1440 | 1 | -1440×1 |

#### `[InArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 1 | -12 | -12 | -12 | 1 | -12×1 |
| `Picker Ab` | 1 | -18 | -18 | -18 | 1 | -18×1 |
| `Picker Ad` | 1 | -72 | -72 | -72 | 1 | -72×1 |
| `Picker Ba` | 1 | -110 | -110 | -110 | 1 | -110×1 |
| `Picker Bb` | 1 | -79 | -79 | -79 | 1 | -79×1 |
| `Picker Bc` | 1 | -81 | -81 | -81 | 1 | -81×1 |
| `Picker Bd` | 1 | -90 | -90 | -90 | 1 | -90×1 |

#### `[OutArmZSub]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Picker Aa` | 1 | 20 | 20 | 20 | 1 | 20×1 |
| `Picker Ab` | 1 | 2 | 2 | 2 | 1 | 2×1 |
| `Picker Ad` | 1 | -15 | -15 | -15 | 1 | -15×1 |
| `Picker Ba` | 1 | -19 | -19 | -19 | 1 | -19×1 |
| `Picker Bb` | 1 | -24 | -24 | -24 | 1 | -24×1 |
| `Picker Bc` | 1 | -41 | -41 | -41 | 1 | -41×1 |
| `Picker Bd` | 1 | -8 | -8 | -8 | 1 | -8×1 |

#### `[Teach INI]`

| Key | n | min | median | max | unique | 主流値 (top3) |
|-----|---|-----|--------|-----|--------|----------------|
| `Update2` | 1 | 1 | 1 | 1 | 1 | 1×1 |

<!-- preserved-content:end -->
