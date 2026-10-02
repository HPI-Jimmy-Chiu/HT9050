# HT9045 Hot Plate Configuration Reference

此表格涵蓋 HT9045 支援的加熱盤（Hot Plate）配置規格，
基於 `ainarm_SearchPlacePlate.cpp` 的 `SearchPlateToPlace()` 分派邏輯，
標示各配置實際支援的 Test Mode（iInArmType）。

## 欄位說明

| 欄位 | 說明 |
|------|------|
| **名稱** | 熱盤類型及尺寸 |
| **Package Size** | IC 封裝尺寸範圍 |
| **Start X** | 第一格 X 座標起點（mm） |
| **Pitch X** | X 方向間距（mm）|
| **Start Y** | 第一格 Y 座標起點（mm） |
| **Pitch Y** | Y 方向間距（mm）|
| **Pocket XxY** | 格位排列（XDivision × YDivision）|
| **HP×2 容量** | 雙盤時總容量（顆）|
| **搜尋函式** | `SearchPlateToPlace()` 呼叫的搜尋函式 |
| **支援 iInArmType** | 有效進入該搜尋路徑的 Test Mode 類型 |

## 配置表

| 名稱 | Package Size | Start X | Pitch X | Start Y | Pitch Y | Pocket XxY | HP×2 容量 | 搜尋函式 | 支援 iInArmType |
|------|------|:---:|:---:|:---:|:---:|:---:|:---:|------|------|
| (A)Hot Plate Pocket Matrix(160*340) | 3x3~9X9 | 10 | 20 | 32 | 12 | 8X24 | 384 | XItem8_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>2x4_8、2x5_8、2x6_8、2x8_8、2x8_32 → XItem8_8Suck<br>1x4_8_Hot → XItem8_8Suck<br>2x2_8_Hot → XItem8_8Suck<br>2x4_4_13（AxEx, 2Row）→ XItem8_8Suck |
| (A)Hot Plate Pocket Matrix(160*340) | 3x3~9X9 | 10 | 20 | 20 | 16 | 8X16 | 256 | XItem8_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>2x4_8、2x5_8、2x6_8、2x8_8、2x8_32 → XItem8_8Suck<br>1x4_8_Hot → XItem8_8Suck<br>2x2_8_Hot → XItem8_8Suck<br>2x4_4_13（AxEx, 2Row）→ XItem8_8Suck |
| (A)Hot Plate Pocket Matrix(160*340) | 10X10~12x12 | 10 | 20 | 42.5 | 15 | 8X18 | 288 | XItem8_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>2x4_8、2x5_8、2x6_8、2x8_8、2x8_32 → XItem8_8Suck<br>1x4_8_Hot → XItem8_8Suck<br>2x2_8_Hot → XItem8_8Suck<br>2x4_4_13（AxEx, 2Row）→ XItem8_8Suck |
| (A)Hot Plate Pocket Matrix(160*340) | 13x13~17x17 | 10 | 20 | 20 | 20 | 8x16 | 256 | XItem8_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>2x4_8、2x5_8、2x6_8、2x8_8、2x8_32 → XItem8_8Suck<br>1x4_8_Hot → XItem8_8Suck<br>2x2_8_Hot → XItem8_8Suck<br>2x4_4_13（AxEx, 2Row）→ XItem8_8Suck |
| (A)Hot Plate Pocket Matrix(160*340) | 18x18~22x22 | 13.33 | 26.67 | 20 | 30 | 6x11 | 132 | XItem6_3Suck_ACEx / XItem6_8Suck / XItem6_2x2_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck<br>2x2_4_13（AxEx, 2Row）→ _2x2Suck / XItem6_2x2_8Suck<br>2x2_4_14（AxxG, 2Row）→ _2x2Suck / XItem6_2x2_8Suck<br>1x3_4（WideHP）→ XItem6_3Suck_ACEx<br>2x3_6（WideHP）→ XItem6_3Suck_ACEx<br>2x6_8（WideHP）→ XItem6_3Suck_ACEx<br>1x4_4／2x4_8 等 ACEG Pickers → XItem6_8Suck（WideHP）/ XItem6_2x2_8Suck |
| (A)Hot Plate Pocket Matrix(160*340) | 17.7x14.6 | 13.33 | 26.67 | 20 | 20 | 6x16 | 96 | XItem6_3Suck_ACEx / XItem6_8Suck / XItem6_2x2_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck<br>2x2_4_13（AxEx, 2Row）→ _2x2Suck / XItem6_2x2_8Suck<br>2x2_4_14（AxxG, 2Row）→ _2x2Suck / XItem6_2x2_8Suck<br>1x3_4（WideHP）→ XItem6_3Suck_ACEx<br>2x3_6（WideHP）→ XItem6_3Suck_ACEx<br>2x6_8（WideHP）→ XItem6_3Suck_ACEx<br>1x4_4／2x4_8 等 ACEG Pickers → XItem6_8Suck（WideHP）/ XItem6_2x2_8Suck |
| (A)Hot Plate Pocket Matrix(160*340) | 23x23~35x35 | 20 | 40 | 30 | 40 | 4x8 | 64 | XItem4_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck<br>2x2_4_13（AxEx, 2Row）→ _2x2Suck / XItem4_8Suck<br>2x2_4_14（AxxG, 2Row）→ _2x2Suck / XItem4_8Suck<br>1x3_4、1x4_4、2x3_6、2x4_8、2x8_8、2x8_32 → XItem4_8Suck<br>1x4_4_Back → XItem4_8Suck<br>2x2_8_Hot → XItem4_8Suck |
| (A)Hot Plate Pocket Matrix(160*340) | 36x36~40x40 | 55 | 50 | 35 | 45 | 2x7 | 28 | XItem_1x2Suck（部分支援2x2） | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x1_Ac（SingleSite）<br>1x2_13（DualSite, AxEx, 1Row）<br>1x2_14（DualSite, AxxG, 1Row）<br>1x3_2_14（AxxG, 1Row, XDiv=2 fallthrough）<br>2x2_4_13（AxEx, 2Row）<br>2x2_4_14（AxxG, 2Row） |
| (A)Hot Plate Pocket Matrix(160*340) | 41x41~45x45 | 55 | 50 | 45 | 50 | 2x6 | 24 | XItem_1x2Suck（部分支援2x2） | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x1_Ac（SingleSite）<br>1x2_13（DualSite, AxEx, 1Row）<br>1x2_14（DualSite, AxxG, 1Row）<br>1x3_2_14（AxxG, 1Row, XDiv=2 fallthrough）<br>2x2_4_13（AxEx, 2Row）<br>2x2_4_14（AxxG, 2Row） |
| (B)Hot Plate Pocket Matrix(220*380) | 3x3~12x12 | 27.5 | 15 | 17.5 | 15 | 12x24 | 576 | XItem12_8Suck / XItem12_2x6 | ep1Picker（1吸嘴）<br>2x6_8（b12x16HP_2x6=true）→ XItem12_2x6<br>2x8_32 等 8吸嘴 → XItem12_8Suck |
| (B)Hot Plate Pocket Matrix(220*380) | 3x3~12x12 | 27.5 | 15 | 20 | 20 | 12x18 | 432 | XItem12_8Suck / XItem12_2x6 | ep1Picker（1吸嘴）<br>2x6_8（b12x16HP_2x6=true）→ XItem12_2x6<br>2x8_32 等 8吸嘴 → XItem12_8Suck |
| (B)Hot Plate Pocket Matrix(220*380) | 3x3~12x12 | 22.5 | 25 | 25 | 30 | 8x12 | 192 | XItem8_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>2x4_8、2x5_8、2x6_8、2x8_8、2x8_32 → XItem8_8Suck<br>1x4_8_Hot → XItem8_8Suck<br>2x2_8_Hot → XItem8_8Suck<br>2x4_4_13（AxEx, 2Row）→ XItem8_8Suck |
| (B)Hot Plate Pocket Matrix(220*380) | 3x3~12x12 | 22.5 | 25 | 20 | 20 | 8x18 | 288 | XItem8_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>2x4_8、2x5_8、2x6_8、2x8_8、2x8_32 → XItem8_8Suck<br>1x4_8_Hot → XItem8_8Suck<br>2x2_8_Hot → XItem8_8Suck<br>2x4_4_13（AxEx, 2Row）→ XItem8_8Suck |
| (B)Hot Plate Pocket Matrix(220*380) | 3x3~12x12 | 22.5 | 25 | 17.5 | 15 | 8x24 | 384 | XItem8_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>2x4_8、2x5_8、2x6_8、2x8_8、2x8_32 → XItem8_8Suck<br>1x4_8_Hot → XItem8_8Suck<br>2x2_8_Hot → XItem8_8Suck<br>2x4_4_13（AxEx, 2Row）→ XItem8_8Suck |
| (B)Hot Plate Pocket Matrix(220*380) | 13x13~17x17 | 22.5 | 25 | 20 | 20 | 8x18 | 288 | XItem8_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>2x4_8、2x5_8、2x6_8、2x8_8、2x8_32 → XItem8_8Suck<br>1x4_8_Hot → XItem8_8Suck<br>2x2_8_Hot → XItem8_8Suck<br>2x4_4_13（AxEx, 2Row）→ XItem8_8Suck |
| (B)Hot Plate Pocket Matrix(220*380) | 18x18~22x22 | 22.5 | 25 | 25 | 30 | 8x12 | 192 | XItem8_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck（XDiv=8分支）<br>2x4_8、2x5_8、2x6_8、2x8_8、2x8_32 → XItem8_8Suck<br>1x4_8_Hot → XItem8_8Suck<br>2x2_8_Hot → XItem8_8Suck<br>2x4_4_13（AxEx, 2Row）→ XItem8_8Suck |
| (B)Hot Plate Pocket Matrix(220*380) | 23x23~26x26 | 22.5 | 35 | 40 | 30 | 6x11 | 132 | XItem6_3Suck_ACEx / XItem6_8Suck / XItem6_2x2_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck<br>2x2_4_13（AxEx, 2Row）→ _2x2Suck / XItem6_2x2_8Suck<br>2x2_4_14（AxxG, 2Row）→ _2x2Suck / XItem6_2x2_8Suck<br>1x3_4（WideHP）→ XItem6_3Suck_ACEx<br>2x3_6（WideHP）→ XItem6_3Suck_ACEx<br>2x6_8（WideHP）→ XItem6_3Suck_ACEx<br>1x4_4／2x4_8 等 ACEG Pickers → XItem6_8Suck（WideHP）/ XItem6_2x2_8Suck |
| (B)Hot Plate Pocket Matrix(220*380) | 27x27~37x37 | 42.5 | 45 | 32.5 | 45 | 4x8 | 64 | XItem4_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck<br>2x2_4_13（AxEx, 2Row）→ _2x2Suck / XItem4_8Suck<br>2x2_4_14（AxxG, 2Row）→ _2x2Suck / XItem4_8Suck<br>1x3_4、1x4_4、2x3_6、2x4_8、2x8_8、2x8_32 → XItem4_8Suck<br>1x4_4_Back → XItem4_8Suck<br>2x2_8_Hot → XItem4_8Suck |
| (B)Hot Plate Pocket Matrix(220*380) | 27x27~37x37 | 50 | 40 | 32.5 | 45 | 4x8 | 64 | XItem4_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck<br>2x2_4_13（AxEx, 2Row）→ _2x2Suck / XItem4_8Suck<br>2x2_4_14（AxxG, 2Row）→ _2x2Suck / XItem4_8Suck<br>1x3_4、1x4_4、2x3_6、2x4_8、2x8_8、2x8_32 → XItem4_8Suck<br>1x4_4_Back → XItem4_8Suck<br>2x2_8_Hot → XItem4_8Suck |
| (B)Hot Plate Pocket Matrix(220*380) | 38x38~45x45 | 50 | 60 | 40 | 60 | 3x6 | 36 | XItem3_1x2Suck / XItem3_2x2Suck / XItem3_2Suck_3Site | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_13（AxEx, 1Row）→ XItem3_1x2Suck<br>1x2_14（AxxG, 1Row）→ XItem3_1x2Suck<br>1x3_2_14（AxxG, TriSite1X3 iModeX=3）→ XItem3_1x2Suck<br>1x3_2_14（AxxG, TriSite1X3 iModeX!=3）→ XItem3_2Suck_3Site<br>2x2_4_13（AxEx, 2Row）→ XItem3_2x2Suck<br>2x2_4_14（AxxG, 2Row）→ XItem3_2x2Suck |
| (B)Hot Plate Pocket Matrix(220*380) | 38x38~45x45 | 50 | 60 | 32.5 | 45 | 3x8 | 24 | XItem3_1x2Suck / XItem3_2x2Suck / XItem3_2Suck_3Site | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_13（AxEx, 1Row）→ XItem3_1x2Suck<br>1x2_14（AxxG, 1Row）→ XItem3_1x2Suck<br>1x3_2_14（AxxG, TriSite1X3 iModeX=3）→ XItem3_1x2Suck<br>1x3_2_14（AxxG, TriSite1X3 iModeX!=3）→ XItem3_2Suck_3Site<br>2x2_4_13（AxEx, 2Row）→ XItem3_2x2Suck<br>2x2_4_14（AxxG, 2Row）→ XItem3_2x2Suck |
| (B)Hot Plate Pocket Matrix(220*380) | 38x38~45x45 | 45 | 65 | 50 | 70 | 3x5 | 30 | XItem3_1x2Suck / XItem3_2x2Suck / XItem3_2Suck_3Site | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_13（AxEx, 1Row）→ XItem3_1x2Suck<br>1x2_14（AxxG, 1Row）→ XItem3_1x2Suck<br>1x3_2_14（AxxG, TriSite1X3 iModeX=3）→ XItem3_1x2Suck<br>1x3_2_14（AxxG, TriSite1X3 iModeX!=3）→ XItem3_2Suck_3Site<br>2x2_4_13（AxEx, 2Row）→ XItem3_2x2Suck<br>2x2_4_14（AxxG, 2Row）→ XItem3_2x2Suck |
| (B)Hot Plate Pocket Matrix(220*380) | 38x38~45x45 | 35 | 50 | 40 | 60 | 4x6 | 48 | XItem4_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck<br>2x2_4_13（AxEx, 2Row）→ _2x2Suck / XItem4_8Suck<br>2x2_4_14（AxxG, 2Row）→ _2x2Suck / XItem4_8Suck<br>1x3_4、1x4_4、2x3_6、2x4_8、2x8_8、2x8_32 → XItem4_8Suck<br>1x4_4_Back → XItem4_8Suck<br>2x2_8_Hot → XItem4_8Suck |
| (B)Hot Plate Pocket Matrix(220*380) | 46x46~55x55 | 65 | 90 | 40 | 60 | 2x6 | 24 | XItem_1x2Suck（部分支援2x2） | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x1_Ac（SingleSite）<br>1x2_13（DualSite, AxEx, 1Row）<br>1x2_14（DualSite, AxxG, 1Row）<br>1x3_2_14（AxxG, 1Row, XDiv=2 fallthrough）<br>2x2_4_13（AxEx, 2Row）<br>2x2_4_14（AxxG, 2Row） |
| (B)Hot Plate Pocket Matrix(220*380) | 56x56~60x60 | 45 | 65 | 55 | 90 | 3x4 | 12 | XItem3_1x2Suck / XItem3_2x2Suck / XItem3_2Suck_3Site | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_13（AxEx, 1Row）→ XItem3_1x2Suck<br>1x2_14（AxxG, 1Row）→ XItem3_1x2Suck<br>1x3_2_14（AxxG, TriSite1X3 iModeX=3）→ XItem3_1x2Suck<br>1x3_2_14（AxxG, TriSite1X3 iModeX!=3）→ XItem3_2Suck_3Site<br>2x2_4_13（AxEx, 2Row）→ XItem3_2x2Suck<br>2x2_4_14（AxxG, 2Row）→ XItem3_2x2Suck |
| (B)Hot Plate Pocket Matrix(220*380) | 56x56~60x60 | 65 | 90 | 50 | 70 | 2x5 | 20 | XItem_1x2Suck（部分支援2x2） | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x1_Ac（SingleSite）<br>1x2_13（DualSite, AxEx, 1Row）<br>1x2_14（DualSite, AxxG, 1Row）<br>1x3_2_14（AxxG, 1Row, XDiv=2 fallthrough）<br>2x2_4_13（AxEx, 2Row）<br>2x2_4_14（AxxG, 2Row） |
| (B)Hot Plate Pocket Matrix(220*380) | 61x61~65x65 | 65 | 90 | 50 | 70 | 2x5 | 20 | XItem_1x2Suck（部分支援2x2） | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x1_Ac（SingleSite）<br>1x2_13（DualSite, AxEx, 1Row）<br>1x2_14（DualSite, AxxG, 1Row）<br>1x3_2_14（AxxG, 1Row, XDiv=2 fallthrough）<br>2x2_4_13（AxEx, 2Row）<br>2x2_4_14（AxxG, 2Row） |
| (B)Hot Plate Pocket Matrix(220*380) | 66x66~85x85 | 65 | 90 | 55 | 90 | 2x4 | 16 | XItem_1x2Suck（部分支援2x2） | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x1_Ac（SingleSite）<br>1x2_13（DualSite, AxEx, 1Row）<br>1x2_14（DualSite, AxxG, 1Row）<br>1x3_2_14（AxxG, 1Row, XDiv=2 fallthrough）<br>2x2_4_13（AxEx, 2Row）<br>2x2_4_14（AxxG, 2Row） |
| (B)Hot Plate Pocket Matrix(220*380) | 27x27~37x37 | 42.5 | 45 | 40 | 30 | 4X11 | 88 | XItem4_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck<br>2x2_4_13（AxEx, 2Row）→ _2x2Suck / XItem4_8Suck<br>2x2_4_14（AxxG, 2Row）→ _2x2Suck / XItem4_8Suck<br>1x3_4、1x4_4、2x3_6、2x4_8、2x8_8、2x8_32 → XItem4_8Suck<br>1x4_4_Back → XItem4_8Suck<br>2x2_8_Hot → XItem4_8Suck |
| (A)Hot Plate Pocket Matrix(160*340) | 36x36~40x40 | 30 | 50 | 45 | 50 | 3X6 | 36 | XItem3_1x2Suck / XItem3_2x2Suck / XItem3_2Suck_3Site | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_13（AxEx, 1Row）→ XItem3_1x2Suck<br>1x2_14（AxxG, 1Row）→ XItem3_1x2Suck<br>1x3_2_14（AxxG, TriSite1X3 iModeX=3）→ XItem3_1x2Suck<br>1x3_2_14（AxxG, TriSite1X3 iModeX!=3）→ XItem3_2Suck_3Site<br>2x2_4_13（AxEx, 2Row）→ XItem3_2x2Suck<br>2x2_4_14（AxxG, 2Row）→ XItem3_2x2Suck |
| (B)Hot Plate Pocket Matrix(220*380) | 66x66~85x85 | 60 | 100 | 80 | 110 | 2x3 | 12 | XItem_1x2Suck（部分支援2x2） | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x1_Ac（SingleSite）<br>1x2_13（DualSite, AxEx, 1Row）<br>1x2_14（DualSite, AxxG, 1Row）<br>1x3_2_14（AxxG, 1Row, XDiv=2 fallthrough）<br>2x2_4_13（AxEx, 2Row）<br>2x2_4_14（AxxG, 2Row） |
| (B)Hot Plate Pocket Matrix(220*380) | 90x90~110x110 | 110 | - | 70 | 120 | 1x3 | 6 | XItem_1Suck | 所有 Test Mode（_1Suck 路徑） |
| (B)Hot Plate Pocket Matrix(220*380) | 110x110~120x120 | 110 | - | 100 | 180 | 1x2 | 4 | XItem_1Suck | 所有 Test Mode（_1Suck 路徑） |
| (B)Hot Plate Pocket Matrix(220*380) | 27x27~37x37 | 42.5 | 45 | 40 | 60 | 4x6 | 48 | XItem4_8Suck | ep1Picker（1吸嘴，放滿）<br>1x1（SingleSite）<br>1x2_2_13（AxEx, 1Row, bPitchOver12000）→ _1x2Suck<br>1x2_2_14（AxxG, 1Row, bPitchOver12000）→ _1x2Suck<br>2x2_4_13（AxEx, 2Row）→ _2x2Suck / XItem4_8Suck<br>2x2_4_14（AxxG, 2Row）→ _2x2Suck / XItem4_8Suck<br>1x3_4、1x4_4、2x3_6、2x4_8、2x8_8、2x8_32 → XItem4_8Suck<br>1x4_4_Back → XItem4_8Suck<br>2x2_8_Hot → XItem4_8Suck |
| (B)Hot Plate Pocket Matrix(220*380) | 120~130 | 110 | - | 105 | 170 | 1x2 | 4 | XItem_1Suck | 所有 Test Mode（_1Suck 路徑） |

---

## 附表：支援 Picker 類型的 XDivision 範圍（原始碼根據）

根據 `bUseAxExPicker()`、`bUseAxxGPicker()`、`bUseACEGPicker()` 定義：

| iInArmType | Picker 分類 | 支援 XDivision | 搜尋路徑 |
|-----------|-----------|--------------|---------|
| `ep1Picker` | 單吸嘴 | 全部（任意 XDiv）| `XItem_1Suck` |
| `1x1_1` / `1x4_1_Ac` | SingleSite | 全部 | `XItem_1Suck` |
| `1x2_2_13` | AxEx, iPickRow=1 | 2, 3, 4, 6, 8（bPitchOver12000）| `XItem_1x2Suck` / `XItem3_1x2Suck` |
| `1x2_2_14` | AxxG, iPickRow=1 | 2, 3, 4, 6, 8（bPitchOver12000）| `XItem_1x2Suck` / `XItem3_1x2Suck` |
| `1x3_2_14` | AxxG, TriSite1X3 | 3 | `XItem3_1x2Suck` / `XItem3_2Suck_3Site` |
| `1x4_4_13` | AxEx | 4, 8 | `XItem4_8Suck` / `XItem8_8Suck` |
| `1x4_2_14` | AxxG | 4, 6, 8 | `XItem4_8Suck` 等 |
| `2x1_2_13` | AxEx, iPickRow=1 | 2, 3, 4, 6, 8 | `XItem_1x2Suck` 等 |
| `2x2_4_12` | （一般）| 4 | `XItem4_8Suck` |
| `2x2_4_13` | AxEx, iPickRow=2 | 3, 4, 6 | `XItem3_2x2Suck` / `_2x2Suck` |
| `2x2_4_14` | AxxG, iPickRow=2 | 2, 3, 4, 6 | `XItem3_2x2Suck` / `_2x2Suck` |
| `2x2_8_Hot` | ACEG / 高溫 | 4, 6, 8 | `XItem4_8Suck` / `XItem8_8Suck` |
| `2x3_6` | ACEG, WideHP | 6 | `XItem6_3Suck_ACEx` |
| `2x3_6_14` | AxxG, iPickRow=2 | 2, 3, 4, 6 | `_2x2Suck` / `XItem6_2x2` |
| `1x3_4` / `2x4_4_13` | ACEG / AxEx | 6 | `XItem6_3Suck_ACEx` / `XItem6_8Suck` |
| `1x4_4` / `1x4_4_Back` | ACEG | 4, 6, 8 | `XItem4_8Suck` / `XItem6_8Suck` |
| `1x4_8_Hot` | ACEG | 8 | `XItem8_8Suck` |
| `2x4_8` | ACEG | 4, 6, 8 | `XItem4_8Suck` / `XItem8_8Suck` |
| `2x5_8` | ACEG | 8, 10 | `XItem8_8Suck` / `XItem10_8Suck` |
| `2x6_8` | ACEG, WideHP | 6, 8, 12 | `XItem6_3Suck_ACEx` / `XItem12_2x6` |
| `2x8_8` | ACEG | 8, 12, 16 | `XItem8_8Suck` / `XItem12_8Suck` / `XItem16_8Suck` |
| `2x8_32` | ACEG / 32Site | 4, 8, 10, 12, 16 | 依 XDiv |
| `2x4_16` | ACEG, 16Site | 8, 16 | `XItem8_8Suck` / `XItem16_8Suck` |

> **AxEx**：`bUseAxExPicker()` = Suck A+C 吸嘴（1&3），Pitch 13~80mm
> **AxxG**：`bUseAxxGPicker()` = Suck A+D 吸嘴（1&4），Pitch >40mm
> **ACEG**：`bUseACEGPicker()` = Suck A+C+E+G 四吸嘴
> **bPitchOver12000**：XPitch > 120mm 時觸發，AxEx/AxxG 改用間隔放法
