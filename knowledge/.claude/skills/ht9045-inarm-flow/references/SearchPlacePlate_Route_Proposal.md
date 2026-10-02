# SearchPlacePlate 新版路徑圖總表

> **版本**: v1.0 Draft  
> **日期**: 2026-04-07  
> **配套文件**: [SearchPlacePlate_Rewrite_Proposal.md](SearchPlacePlate_Rewrite_Proposal.md)  
> **引用**: [HP_SearchPlacePlate_AllFunctions.md](HP_SearchPlacePlate_AllFunctions.md) §17.2 / §19

---

## 目錄

- [§0 閱讀指南](#0-閱讀指南)
- [§1 XDiv=2 (2×6, 2×7 HP)](#1-xdiv2-26-27-hp)
- [§2 XDiv=3 (3×5, 3×7 HP)](#2-xdiv3-35-37-hp)
- [§3 XDiv=4 (4×8 HP)](#3-xdiv4-48-hp)
- [§4 XDiv=6 標準 (6×11 HP)](#4-xdiv6-標準-611-hp)
- [§5 XDiv=6 WideHP (6×11, 6×16 HP)](#5-xdiv6-widehp-611-616-hp)
- [§6 XDiv=8 (8×12, 8×16, 8×18, 8×24 HP)](#6-xdiv8-812-816-818-824-hp)
- [§7 XDiv=10 (10×16 HP)](#7-xdiv10-1016-hp)
- [§8 XDiv=12 (12×18, 12×24 HP)](#8-xdiv12-1218-1224-hp)
- [§9 XDiv=16 (16×24 HP)](#9-xdiv16-1624-hp)
- [§10 全組合掃描參數總覽表](#10-全組合掃描參數總覽表)

---

## §0 閱讀指南

### 0.1 路徑圖符號

| 符號 | 意義 |
|------|------|
| `[Aa1]` | A-Row 吸嘴 a 在步驟 1 放置的位置 |
| `[Ac1]` | A-Row 吸嘴 c 在步驟 1 放置的位置 |
| `[Ba1]` | B-Row 吸嘴 a 在步驟 1（Row2）放置的位置 |
| `[ · ]` | 該步驟未使用的格位 |
| `[跳]` | 因關站而跳過的格位 |
| `R0, R1...` | Row 索引（Y 方向） |
| `C0, C1...` | Column 索引（X 方向） |
| `┐ ┘` | 一次放置動作的上下界 |
| `HP0→HP1` | 盤滿後換盤 |

### 0.2 掃描參數定義

| 參數 | §19 配置表名稱 | 意義 |
|------|---------------|------|
| `spacX` | — | CheckHasSpace 的 X 方向 Column 間距 |
| `nSuckCol` (Col) | — | 每次放置佔幾欄 |
| `nSuckRow` (Row) | — | 每次放置佔幾列 (1=A-Row only, 2=A+B Row) |
| `spacY` | — | A-Row 與 B-Row 之間的 Row 間距 (= iYHalf) |
| `ixRange` | — | ix 掃描範圍 [start → end), step |
| `Seg` | — | 掃描段數 (1=單段, 2=主段+餘數段) |

### 0.3 iInArmType 縮寫對照

| 縮寫 | 全名 | Picker 數 |
|------|------|:---------:|
| `1Suck` | ep1Picker / SingleSite / 1x4_1_Ac | 1 |
| `Ax1R` | AxEx 1Row (1x2_2_13) | 2 |
| `Ag1R` | AxxG 1Row (1x2_2_14) | 2 |
| `Tri` | TriSite1X3 (1x3_2_14, iModeX≠3) | 2 |
| `TriX3` | TriSite1X3 (1x3_2_14, iModeX=3) | 2 |
| `Ax2R` | AxEx 2Row (2x2_4_13) | 4 |
| `Ag2R` | AxxG 2Row (2x2_4_14) | 4 |
| `NN14` | NN 1x4_4_13 (iShtRow=1) | 4 |
| `1x4` | ACEG 1x4_4 | 4 |
| `W13` | WideHP 1x3_4 | 3 |
| `W23` | WideHP 2x3_6 | 6 |
| `W26` | WideHP 2x6_8 | 6 |
| `2x4` | ACEG 2x4_8 | 8 |
| `2x5` | ACEG 2x5_8 | 8 |
| `2x8` | ACEG 2x8_8 | 8 |
| `32S` | ACEG 2x8_32 | 32 |
| `H28` | 2x2_8_Hot | 8 |
| `H14` | 1x4_8_Hot | 8 |
| `2416` | 2x4_16 | 16 |

---

## §1 XDiv=2 (2×6, 2×7 HP)

> §19 對應: (A) 36x36\~45x45 封裝, Pitch 50×45\~50mm

### 1.1 HP 規格

| 名稱 | Pocket | Pitch X×Y | HP×2 容量 |
|------|:------:|:---------:|:---------:|
| (A) 36×36\~40×40 | 2×7 | 50×45 | 28 |
| (A) 41×41\~45×45 | 2×6 | 50×50 | 24 |

### 1.2 支援的 iInArmType 與路徑圖

#### 1Suck (ep1Picker / SingleSite)

`spacX=1, Col=1, Row=1, spacY=1`

```
  C0  C1
  ──  ──        (2×7 HP)
  [ 1] [ 2]  R0
  [ 3] [ 4]  R1
  [ 5] [ 6]  R2
  [ 7] [ 8]  R3
  [ 9] [10]  R4
  [11] [12]  R5
  [13] [14]  R6
  ─── HP0 → HP1 ───
  [15] [16]  R0
  ...
```

#### Ax1R / Ag1R (AxEx/AxxG 1Row)

`spacX=1, Col=2, Row=1, spacY=1`  
ix 固定 = 0（2 Col 剛好填滿），每步放 2 顆。

```
  C0    C1
  ──    ──        (2×7 HP)
  [Aa1] [Ac1]  R0  ← 步驟1
  [Aa2] [Ac2]  R1  ← 步驟2
  [Aa3] [Ac3]  R2
  [Aa4] [Ac4]  R3
  [Aa5] [Ac5]  R4
  [Aa6] [Ac6]  R5
  [Aa7] [Ac7]  R6
  ─── HP0 → HP1 ───
  [Aa8] [Ac8]  R0
  ...
```

#### Ax2R / Ag2R (AxEx/AxxG 2Row)

`spacX=1, Col=2, Row=2, spacY=iYHalf`  
2×7: iYHalf = ARM_HP_Y_PITCH / 4500 ≈ 1\~2

```
  C0    C1
  ──    ──        (2×7 HP, iYHalf=2)
  [Aa1] [Ac1]  R0  ┐步驟1
  [Ba1] [Bc1]  R2  ┘
  [Aa2] [Ac2]  R4  ┐步驟2
  [Ba2] [Bc2]  R6  ┘
  ─── HP0 → HP1 ───
  (R1,R3,R5 由下盤或後續回填)
```

> ⚠️ 2×7 HP 容量小（14 格/盤），2Row 模式僅能放 ~3 步/盤。

---

## §2 XDiv=3 (3×5, 3×7 HP)

> §19 中無現成 3-Col HP 配置；需新增 PlateForm。  
> 預計規格：3×5 (Pitch ~53×68mm) 或 3×7 (Pitch ~53×48mm)

### 2.1 HP 規格（預計新增）

| 名稱 | Pocket | Pitch X×Y | HP×2 容量 |
|------|:------:|:---------:|:---------:|
| (新) TriSite HP | 3×7 | 53×48 | 42 |

### 2.2 支援的 iInArmType 與路徑圖

#### 1Suck (ep1Picker / SingleSite)

`spacX=1, Col=1, Row=1`

```
  C0  C1  C2
  ──  ──  ──        (3×7 HP)
  [ 1] [ 2] [ 3]  R0
  [ 4] [ 5] [ 6]  R1
  [ 7] [ 8] [ 9]  R2
  [10] [11] [12]  R3
  [13] [14] [15]  R4
  [16] [17] [18]  R5
  [19] [20] [21]  R6
```

#### Tri (TriSite1X3, iModeX≠3) — 3_2Suck_3Site

**窄 Pitch** (`XPitch ≤ iXpitchMaxX3/2`): Row-first  
**寬 Pitch** (`XPitch > iXpitchMaxX3/2`): Col-first

```
窄Pitch (Row-first):          寬Pitch (Col-first):
  C0  C1  C2                    C0  C1  C2
  [ 1] [ 2] [ 3]  R0           [ 1] [ 8] [15]  R0
  [ 4] [ 5] [ 6]  R1           [ 2] [ 9] [16]  R1
  [ 7] [ 8] [ 9]  R2           [ 3] [10] [17]  R2
  [10] [11] [12]  R3           [ 4] [11] [18]  R3
  [13] [14] [15]  R4           [ 5] [12] [19]  R4
  [16] [17] [18]  R5           [ 6] [13] [20]  R5
  [19] [20] [21]  R6           [ 7] [14] [21]  R6
```

#### TriX3 / Ax1R / Ag1R (1Row 2吸嘴) — 3_1x2Suck

`spacX=1, Col=2, col-first 掃描`  
Col0+Col1 同步放；Col2 單放 (iForPlaceHPX3Step=1)

```
  C0    C1    C2
  ──    ──    ──        (3×7 HP)
  [Aa1] [Ac1] [ · ]  R0  ← x=0
  [Aa2] [Ac2] [ · ]  R1
  [Aa3] [Ac3] [ · ]  R2
  [Aa4] [Ac4] [ · ]  R3
  [Aa5] [Ac5] [ · ]  R4
  [Aa6] [Ac6] [ · ]  R5
  [Aa7] [Ac7] [ · ]  R6
  [ · ] [ · ] [Aa① ]  R0  ← x=2 (Col2 單放)
  [ · ] [ · ] [Aa② ]  R1
  ...
  [ · ] [ · ] [Aa⑦ ]  R6
```

#### Ax2R / Ag2R (2Row 4吸嘴) — 3_2x2Suck

`spacX=1, Col=2, Row=2, col-first`  
奇數 YDiv: 末格 [2][YDiv-1] 強制 HAS_NULL_IC

```
  C0    C1    C2
  ──    ──    ──        (3×6 HP, iYHalf=2)
  [Aa1] [Ac1] [ · ]  R0  ┐步驟1: x=0, y=0
  [Ba1] [Bc1] [ · ]  R2  ┘
  [Aa2] [Ac2] [ · ]  R4  ┐步驟2: x=0, y=4
  [Ba2] [Bc2] [ · ]  R6  ┘(HP 盤滿 → 切 HP1)
  (接 x=2 Col2 單放，同 1Row 模式)
```

---

## §3 XDiv=4 (4×8 HP)

> §19 對應: (A) 23x23\~35x35 封裝, Pitch 40×40mm

### 3.1 HP 規格

| 名稱 | Pocket | Pitch X×Y | HP×2 容量 |
|------|:------:|:---------:|:---------:|
| (A) 23×23\~35×35 | 4×8 | 40×40 | 64 |

### 3.2 支援的 iInArmType 與路徑圖

#### 1Suck

`spacX=1, Col=1, Row=1`

```
  C0  C1  C2  C3
  [ 1] [ 2] [ 3] [ 4]  R0
  [ 5] [ 6] [ 7] [ 8]  R1
  ...
  [29] [30] [31] [32]  R7
```

#### Ax1R / Ag1R (AxEx/AxxG 1Row, 標準 Pitch)

`spacX=2, Col=2, Row=1`

```
  C0    C1    C2    C3
  ──    ──    ──    ──
  [Aa1] [ · ] [Ac1] [ · ]  R0  ← ix=0
  [ · ] [Aa2] [ · ] [Ac2]  R0  ← ix=1
  [Aa3] [ · ] [Ac3] [ · ]  R1  ← ix=0, iy=1
  [ · ] [Aa4] [ · ] [Ac4]  R1  ← ix=1
  ...
  [Aa15][ · ] [Ac15][ · ]  R7  ← ix=0
  [ · ] [Aa16][ · ] [Ac16] R7  ← ix=1
```

#### Ax1R + PitchOver (bPitchOver12000)

`spacX=1, Col=1, Row=1` — 降級為逐格

```
  C0  C1  C2  C3
  [ 1] [ 2] [ 3] [ 4]  R0
  [ 5] [ 6] [ 7] [ 8]  R1
  ...（同 1Suck）
```

#### Ax2R / Ag2R (AxEx/AxxG 2Row, 標準 Pitch)

`spacX=2, Col=2, Row=2, spacY=iYHalf`

```
  C0    C1    C2    C3
  ──    ──    ──    ──        (4×8, iYHalf=2)
  [Aa1] [ · ] [Ac1] [ · ]  R0  ┐步驟1
  [Ba1] [ · ] [Bc1] [ · ]  R2  ┘
  [ · ] [Aa2] [ · ] [Ac2]  R0  ┐步驟2
  [ · ] [Ba2] [ · ] [Bc2]  R2  ┘
  [Aa3] [ · ] [Ac3] [ · ]  R4  ┐步驟3
  [Ba3] [ · ] [Bc3] [ · ]  R6  ┘
  [ · ] [Aa4] [ · ] [Ac4]  R4  ┐步驟4
  [ · ] [Ba4] [ · ] [Bc4]  R6  ┘
```

> ⚠️ `b4x11HP`: 若 YDiv 為奇數，Row 強制 = 1，iYHalf = 1。  
> ⚠️ `b4x10HP_2x2`: 若 YPitch 無法整除 ARM_HP_Y_PITCH，spacY = 1。

#### ACEG (1x4, 2x4, 2x8, 2x8_32) — XItem4_8Suck

`spacX=1, Col=4, Row=1/2, spacY=iYHalf`  
4 吸嘴 (Aa/Ac/Ae/Ag) 填滿 4 Col，無 ix 位移。

```
  C0    C1    C2    C3
  ──    ──    ──    ──        (4×8, 2Row, iYHalf=2)
  [Aa1] [Ac1] [Ae1] [Ag1]  R0  ┐步驟1: ix=0
  [Ba1] [Bc1] [Be1] [Bg1]  R2  ┘
  [Aa2] [Ac2] [Ae2] [Ag2]  R4  ┐步驟2: ix=0, iy=4
  [Ba2] [Bc2] [Be2] [Bg2]  R6  ┘
  ─── HP0 → HP1 ───
```

#### NN14 (NN 1x4_4_13, iShtRow=1)

`spacX=1, Col=4, Row=1, spacY=1`（特殊：spacY=1 非 iYHalf）

```
  C0    C1    C2    C3
  [Aa1] [Ac1] [Ae1] [Ag1]  R0  ← 步驟1
  [Aa2] [Ac2] [Ae2] [Ag2]  R1  ← 步驟2
  ...
  [Aa8] [Ac8] [Ae8] [Ag8]  R7  ← 步驟8
```

#### H28 (2x2_8_Hot)

`spacX=1, Col=4, Row=2, spacY=iYHalf`（同 ACEG 2Row）

```
  （同 ACEG 2Row 路徑）
```

---

## §4 XDiv=6 標準 (6×11 HP)

> §19 對應: (A) 18x18\~22x22 封裝, Pitch 26.67×30mm  
> 條件: `i8PickerHPMode ≠ iHPWideHP` 且 `HotPlateYPitchCanPutAll() == true`

### 4.1 HP 規格

| 名稱 | Pocket | Pitch X×Y | HP×2 容量 |
|------|:------:|:---------:|:---------:|
| (A) 18×18\~22×22 | 6×11 | 26.67×30 | 132 |
| (A) 17.7×14.6 | 6×16 | 26.67×20 | 192 |

### 4.2 支援的 iInArmType 與路徑圖

#### 1Suck

`spacX=1, Col=1, Row=1`

```
  C0  C1  C2  C3  C4  C5
  [ 1] [ 2] [ 3] [ 4] [ 5] [ 6]  R0
  [ 7] [ 8] [ 9] [10] [11] [12]  R1
  ...
  [61] [62] [63] [64] [65] [66]  R10
```

#### Ax1R / Ag1R (1Row, 標準 Pitch)

`spacX=3, Col=2, Row=1`  
Aa@Col[ix], Ac@Col[ix+3]

```
  C0    C1    C2    C3    C4    C5
  ──    ──    ──    ──    ──    ──        (6×11)
  [Aa1] [ · ] [ · ] [Ac1] [ · ] [ · ]  R0  ← ix=0
  [ · ] [Aa2] [ · ] [ · ] [Ac2] [ · ]  R0  ← ix=1
  [ · ] [ · ] [Aa3] [ · ] [ · ] [Ac3]  R0  ← ix=2
  [Aa4] [ · ] [ · ] [Ac4] [ · ] [ · ]  R1  ← ix=0, iy=1
  [ · ] [Aa5] [ · ] [ · ] [Ac5] [ · ]  R1
  [ · ] [ · ] [Aa6] [ · ] [ · ] [Ac6]  R1
  ...
```

#### Ax1R + PitchOver (bPitchOver12000)

`spacX=1, Col=1, Row=1` — 降級為逐格（同 1Suck）

#### Ax2R / Ag2R (2Row, CanPutAll) — 6_2x2_8Suck

`spacX=3, Col=2, Row=2, spacY=iYHalf`  
三段式掃描: ix = 0 → 1 → 2

```
  C0    C1    C2    C3    C4    C5
  ──    ──    ──    ──    ──    ──        (6×11, iYHalf=2)
  [Aa1] [ · ] [ · ] [Ac1] [ · ] [ · ]  R0  ┐步驟1: ix=0
  [Ba1] [ · ] [ · ] [Bc1] [ · ] [ · ]  R2  ┘
  [ · ] [Aa2] [ · ] [ · ] [Ac2] [ · ]  R0  ┐步驟2: ix=1
  [ · ] [Ba2] [ · ] [ · ] [Bc2] [ · ]  R2  ┘
  [ · ] [ · ] [Aa3] [ · ] [ · ] [Ac3]  R0  ┐步驟3: ix=2
  [ · ] [ · ] [Ba3] [ · ] [ · ] [Bc3]  R2  ┘
  [Aa4] [ · ] [ · ] [Ac4] [ · ] [ · ]  R4  ┐步驟4: ix=0, iy=4
  [Ba4] [ · ] [ · ] [Bc4] [ · ] [ · ]  R6  ┘
  ...
```

#### Ax2R (!CanPutAll) — 6_NotStandY

`spacX=3, Col=2, Row=1~2（動態）`  
x = 0 → 1 → 2 穿插模式，flag 直檢

```
  C0    C1    C2    C3    C4    C5
  ──    ──    ──    ──    ──    ──
  [Aa1] [ · ] [ · ] [Ac1] [ · ] [ · ]  R0  ← x=0 (step=0)
  [ · ] [Aa2] [ · ] [ · ] [Ac2] [ · ]  R0  ← x=1 (step=1)
  [ · ] [ · ] [Aa3] [ · ] [ · ] [Ac3]  R1  ← x=2 (step=0, y切換)
  [Aa4] [ · ] [ · ] [Ac4] [ · ] [ · ]  R1  ← x=0
  [ · ] [Aa5] [ · ] [ · ] [Ac5] [ · ]  R1  ← x=1
  ...
```

> 新版將 NotStandY 統一為 Row=1 的標準掃描，掃描順序可能略異但填滿率相同。

#### ACEG (1x4, 2x4) — 降級走 _1x2Suck 路徑

在標準 6-Col HP 中，ACEG 模式走 `_1x2Suck` 分支（因 XDiv=6 的 spacX=3 只容 2 吸嘴）。

```
  （同 Ax1R 路徑，僅用 Aa+Ac 兩吸嘴參與放料）
```

---

## §5 XDiv=6 WideHP (6×11, 6×16 HP)

> §19 同 §4 HP 規格，差異在 `i8PickerHPMode == iHPWideHP`  
> WideHP 的 XPitch (26.67mm) 允許 3 吸嘴等距排列

### 5.1 條件

- `i8PickerHPMode == iHPWideHP`
- 機台必須 `IN_OUT_ARM_X_PITCH_MIN ≤ 2667` 才可使用（目前機台 4000 → **硬體限制**）

### 5.2 支援的 iInArmType 與路徑圖

#### W13 (1x3_4, WideHP 3吸嘴) — 6_3Suck_ACEx

`spacX=1, Col=3, Row=1, ix step=3`  
左半 Col0-2 → 右半 Col3-5

```
  C0    C1    C2    C3    C4    C5
  ──    ──    ──    ──    ──    ──        (6×11, WideHP)
  [Aa1] [Ac1] [Ae1] [ · ] [ · ] [ · ]  R0  ← ix=0 (左半)
  [ · ] [ · ] [ · ] [Aa2] [Ac2] [Ae2]  R0  ← ix=3 (右半)
  [Aa3] [Ac3] [Ae3] [ · ] [ · ] [ · ]  R1
  [ · ] [ · ] [ · ] [Aa4] [Ac4] [Ae4]  R1
  ...
  [Aa21][Ac21][Ae21][ · ] [ · ] [ · ]  R10
  [ · ] [ · ] [ · ] [Aa22][Ac22][Ae22] R10
```

#### W23 (2x3_6, WideHP 6吸嘴) — 6_3Suck_ACEx

`spacX=1, Col=3, Row=2, spacY=iYHalf, ix step=3`

```
  C0    C1    C2    C3    C4    C5
  ──    ──    ──    ──    ──    ──        (6×11, iYHalf=2)
  [Aa1] [Ac1] [Ae1] [ · ] [ · ] [ · ]  R0  ┐步驟1: ix=0
  [Ba1] [Bc1] [Be1] [ · ] [ · ] [ · ]  R2  ┘
  [ · ] [ · ] [ · ] [Aa2] [Ac2] [Ae2]  R0  ┐步驟2: ix=3
  [ · ] [ · ] [ · ] [Ba2] [Bc2] [Be2]  R2  ┘
  [Aa3] [Ac3] [Ae3] [ · ] [ · ] [ · ]  R4  ┐步驟3: ix=0, iy=4
  [Ba3] [Bc3] [Be3] [ · ] [ · ] [ · ]  R6  ┘
  ...
```

#### W26 (2x6_8, WideHP) — 6_3Suck_ACEx

同 W23 路徑（對 HP 而言，放料行為相同，差別在 Shuttle 側取料數）。

#### ACEG WideHP (1x4, 2x4 等) — 6_8Suck

**兩段式**: Seg0 = Col0-3 (4吸嘴), Seg1 = Col4-5 (2吸嘴)

```
  C0    C1    C2    C3    C4    C5
  ──    ──    ──    ──    ──    ──
  ═══ Seg0: ix=0, spacX=1, Col=4 ═══
  [Aa1] [Ac1] [Ae1] [Ag1] [ · ] [ · ]  R0  ┐步驟1
  [Ba1] [Bc1] [Be1] [Bg1] [ · ] [ · ]  R2  ┘
  [Aa2] [Ac2] [Ae2] [Ag2] [ · ] [ · ]  R4  ┐步驟2
  [Ba2] [Bc2] [Be2] [Bg2] [ · ] [ · ]  R6  ┘
  ...（Seg0 全 Row 掃完）

  ═══ Seg1: ix=4, spacX=1, Col=2 ═══
  [ · ] [ · ] [ · ] [ · ] [Ae①] [Ag①]  R0  ┐步驟N
  [ · ] [ · ] [ · ] [ · ] [Be①] [Bg①]  R2  ┘
  [ · ] [ · ] [ · ] [ · ] [Ae②] [Ag②]  R4  ┐步驟N+1
  [ · ] [ · ] [ · ] [ · ] [Be②] [Bg②]  R6  ┘
  ...
```

---

## §6 XDiv=8 (8×12, 8×16, 8×18, 8×24 HP)

> §19 對應: 最常見配置 — 3x3\~17x17 封裝 ((A)型 160×340 及 (B)型 220×380)

### 6.1 HP 規格

| 名稱 | Size | Pocket | Pitch X×Y | HP×2 容量 |
|------|------|:------:|:---------:|:---------:|
| (A) 3×3\~9×9 | 160×340 | 8×24 | 20×12 | 384 |
| (A) 3×3\~9×9 | 160×340 | 8×16 | 20×20 | 256 |
| (A) 10×10\~12×12 | 160×340 | 8×18 | 20×15 | 288 |
| (A) 13×13\~17×17 | 160×340 | 8×16 | 20×20 | 256 |
| (B) 3×3\~12×12 | 220×380 | 8×12 | 25×30 | 192 |
| (B) 3×3\~12×12 | 220×380 | 8×18 | 25×20 | 288 |
| (B) 3×3\~12×12 | 220×380 | 8×24 | 25×15 | 384 |
| (B) 13×13\~17×17 | 220×380 | 8×18 | 25×20 | 288 |

### 6.2 支援的 iInArmType 與路徑圖

#### 1Suck

`spacX=1, Col=1, Row=1`

```
  C0  C1  C2  C3  C4  C5  C6  C7
  [ 1] [ 2] [ 3] [ 4] [ 5] [ 6] [ 7] [ 8]  R0
  [ 9] [10] [11] [12] [13] [14] [15] [16]  R1
  ...（8×16 → 128 格/盤）
```

#### Ax1R / Ag1R (1Row, 標準 Pitch)

`spacX=4, Col=2, Row=1`

```
  C0    C1    C2    C3    C4    C5    C6    C7
  ──    ──    ──    ──    ──    ──    ──    ──
  [Aa1] [ · ] [ · ] [ · ] [Ac1] [ · ] [ · ] [ · ]  R0  ← ix=0
  [ · ] [Aa2] [ · ] [ · ] [ · ] [Ac2] [ · ] [ · ]  R0  ← ix=1
  [ · ] [ · ] [Aa3] [ · ] [ · ] [ · ] [Ac3] [ · ]  R0  ← ix=2
  [ · ] [ · ] [ · ] [Aa4] [ · ] [ · ] [ · ] [Ac4]  R0  ← ix=3
  [Aa5] [ · ] [ · ] [ · ] [Ac5] [ · ] [ · ] [ · ]  R1
  ...
```

#### Ax1R + PitchOver (bPitchOver12000)

`spacX=2, Col=2, Row=1`  
XDiv=8 AxEx: ix 步進 0→1→4→5

```
  C0    C1    C2    C3    C4    C5    C6    C7
  [Aa1] [ · ] [Ac1] [ · ] [ · ] [ · ] [ · ] [ · ]  R0  ← ix=0
  [ · ] [Aa2] [ · ] [Ac2] [ · ] [ · ] [ · ] [ · ]  R0  ← ix=1
  [ · ] [ · ] [ · ] [ · ] [Aa3] [ · ] [Ac3] [ · ]  R0  ← ix=4
  [ · ] [ · ] [ · ] [ · ] [ · ] [Aa4] [ · ] [Ac4]  R0  ← ix=5
  ...
```

#### Ax2R / Ag2R (2Row)

`spacX=4, Col=2, Row=2, spacY=iYHalf`

```
  C0    C1    C2    C3    C4    C5    C6    C7
  ──    ──    ──    ──    ──    ──    ──    ──      (8×16, iYHalf=4)
  [Aa1] [ · ] [ · ] [ · ] [Ac1] [ · ] [ · ] [ · ]  R0  ┐步驟1
  [Ba1] [ · ] [ · ] [ · ] [Bc1] [ · ] [ · ] [ · ]  R4  ┘
  [ · ] [Aa2] [ · ] [ · ] [ · ] [Ac2] [ · ] [ · ]  R0  ┐步驟2
  [ · ] [Ba2] [ · ] [ · ] [ · ] [Bc2] [ · ] [ · ]  R4  ┘
  ...
```

> ⚠️ `b8x16HP_2x2`: QualSite2X2 + 8×16 + YPitch=20mm → iYHalf=1, 特殊步進

#### ★ ACEG 2x4_8 — XItem8_8Suck（主力路徑）

`spacX=2, Col=4, Row=2, spacY=iYHalf`  
ix = 0 → 1（兩步填滿 8-Col）

```
  C0    C1    C2    C3    C4    C5    C6    C7
  ──    ──    ──    ──    ──    ──    ──    ──      (8×16, iYHalf=2)
  [Aa1] [ · ] [Ac1] [ · ] [Ae1] [ · ] [Ag1] [ · ]  R0  ┐步驟1: ix=0
  [Ba1] [ · ] [Bc1] [ · ] [Be1] [ · ] [Bg1] [ · ]  R2  ┘
  [ · ] [Aa2] [ · ] [Ac2] [ · ] [Ae2] [ · ] [Ag2]  R0  ┐步驟2: ix=1
  [ · ] [Ba2] [ · ] [Bc2] [ · ] [Be2] [ · ] [Bg2]  R2  ┘
  [Aa3] [ · ] [Ac3] [ · ] [Ae3] [ · ] [Ag3] [ · ]  R4  ┐步驟3: ix=0, iy=4
  [Ba3] [ · ] [Bc3] [ · ] [Be3] [ · ] [Bg3] [ · ]  R6  ┘
  [ · ] [Aa4] [ · ] [Ac4] [ · ] [Ae4] [ · ] [Ag4]  R4  ┐步驟4: ix=1, iy=4
  [ · ] [Ba4] [ · ] [Bc4] [ · ] [Be4] [ · ] [Bg4]  R6  ┘
  ...（每 2 iy 步 × 2 ix 步 = 放 32 顆/4步）
```

> spacX=2: Aa@Col[ix], Ac@Col[ix+2], Ae@Col[ix+4], Ag@Col[ix+6]  
> ix=0: 偶數 Col (0,2,4,6)。ix=1: 奇數 Col (1,3,5,7)

#### ACEG 1x4, 2x5, 2x6, 2x8, 32S, H28, H14, 2416

所有 ACEG 模式在 XDiv=8 HP 上走**相同掃描路徑**（同 2x4_8 上圖），差別只在：
- `Row`: 1Row (1x4, H14) 或 2Row (其他)
- 實際用到的吸嘴數（決定單步放幾顆 IC）

| iInArmType | Row | 每步放料數 | 備註 |
|---|:---:|:---:|---|
| 1x4 | 1 | 4 | A-Row only |
| 2x4 | 2 | 8 | |
| 2x5 | 2 | 8 | 第 5 對吸嘴在 Shuttle 側另外處理 |
| W26 (2x6_8) | 2 | 8 | 同 ACEG 路徑 |
| 2x8 | 2 | 8 | |
| 32S (2x8_32) | 2 | 8 | HP 側同 2x8 |
| H28 (2x2_8_Hot) | 2 | 8 | |
| H14 (1x4_8_Hot) | 1 | 4 | |
| 2416 (2x4_16) | 2 | 8 | HP 側同 2x4 |

#### ACEG + PitchOver (bPitchOver12000)

**兩段式**: Seg0 = ix=0 (Col 0-3), Seg1 = ix=4 (Col 4-7)  
`spacX=1, Col=4`

```
  C0    C1    C2    C3    C4    C5    C6    C7
  ──    ──    ──    ──    ──    ──    ──    ──
  ═══ Seg0: ix=0, spacX=1, Col=4 ═══
  [Aa1] [Ac1] [Ae1] [Ag1] [ · ] [ · ] [ · ] [ · ]  R0
  [Ba1] [Bc1] [Be1] [Bg1] [ · ] [ · ] [ · ] [ · ]  R2
  ...

  ═══ Seg1: ix=4, spacX=1, Col=4 ═══
  [ · ] [ · ] [ · ] [ · ] [Aa①] [Ac①] [Ae①] [Ag①]  R0
  [ · ] [ · ] [ · ] [ · ] [Ba①] [Bc①] [Be①] [Bg①]  R2
  ...
```

---

## §7 XDiv=10 (10×16 HP)

> §19 中無直接對應（ATK 專用），PlateForm 需新增或已有 8×12 Pitch 20×30 近似

### 7.1 HP 規格（ATK 專用）

| 名稱 | Pocket | Pitch X×Y | HP×2 容量 |
|------|:------:|:---------:|:---------:|
| ATK 10×16 | 10×16 | 16×20 | 320 |

### 7.2 支援的 iInArmType 與路徑圖

#### ACEG 2x4, 2x5, 2x8, 32S — XItem10_8Suck

**兩段式**: Seg0 = ix=0\~4 (Col 0-7 滑動窗口), Seg1 = ix=8 (Col 8-9)  
`Seg0: spacX=1, Col=4, ix=0→5 step=1`  
`Seg1: spacX=1, Col=2, ix=8→10`

```
  C0  C1  C2  C3  C4  C5  C6  C7  C8  C9
  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──      (10×16, iYHalf=2)

  ═══ Seg0: 滑動窗口 Col=4 ═══
  [A ] [A ] [A ] [A ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ]  R0  ┐ix=0
  [B ] [B ] [B ] [B ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ]  R2  ┘
  [ · ] [A ] [A ] [A ] [A ] [ · ] [ · ] [ · ] [ · ] [ · ]  R0  ┐ix=1
  [ · ] [B ] [B ] [B ] [B ] [ · ] [ · ] [ · ] [ · ] [ · ]  R2  ┘
  [ · ] [ · ] [A ] [A ] [A ] [A ] [ · ] [ · ] [ · ] [ · ]  R0  ┐ix=2
  [ · ] [ · ] [B ] [B ] [B ] [B ] [ · ] [ · ] [ · ] [ · ]  R2  ┘
  [ · ] [ · ] [ · ] [A ] [A ] [A ] [A ] [ · ] [ · ] [ · ]  R0  ┐ix=3
  [ · ] [ · ] [ · ] [B ] [B ] [B ] [B ] [ · ] [ · ] [ · ]  R2  ┘
  [ · ] [ · ] [ · ] [ · ] [A ] [A ] [A ] [A ] [ · ] [ · ]  R0  ┐ix=4
  [ · ] [ · ] [ · ] [ · ] [B ] [B ] [B ] [B ] [ · ] [ · ]  R2  ┘
  (ix=0→4 掃完 R0/R2 → iy+=iYHalf*2 → R4/R6...)
  (全部 Row 完成後)

  ═══ Seg1: 餘數 Col=2 ═══
  [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [A ] [A ]  R0  ┐ix=8
  [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [B ] [B ]  R2  ┘
  ...
```

> `[A]` = Aa/Ac/Ae/Ag 吸嘴群在該步佔位，`[B]` = Ba/Bc/Be/Bg。  
> ix=0\~4 每步 4 Col 向右平移 1 Col，覆蓋 Col 0\~7。ix=8 補 Col 8-9。

---

## §8 XDiv=12 (12×18, 12×24 HP)

> §19 對應: (B)型大板 220×380, 3x3\~12x12 封裝

### 8.1 HP 規格

| 名稱 | Pocket | Pitch X×Y | HP×2 容量 |
|------|:------:|:---------:|:---------:|
| (B) 3×3\~12×12 | 12×24 | 15×15 | 576 |
| (B) 3×3\~12×12 | 12×18 | 15×20 | 432 |

### 8.2 支援的 iInArmType 與路徑圖

#### 1Suck

`spacX=1, Col=1, Row=1`

```
  C0  C1  C2  C3  C4  C5  C6  C7  C8  C9  CA  CB
  [ 1] [ 2] [ 3] [ 4] [ 5] [ 6] [ 7] [ 8] [ 9] [10] [11] [12]  R0
  ...（12×24 → 288 格/盤）
```

#### W26 (2x6_8, b12x16HP_2x6=true) — XItem12_2x6 Kit 交錯

`spacX=2, Col=3, Row=2, spacY=iYHalf`  
`ixSequence = {0, 6, 1, 7, -1, ...}` — Kit0 左 → Kit1 左 → Kit0 右 → Kit1 右

```
  C0  C1  C2  C3  C4  C5  C6  C7  C8  C9  CA  CB
  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──    (12×18, iYHalf=1)

  [A ] [ · ] [A ] [ · ] [A ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ]  R0  ┐步驟1: ix=0 (Kit0左)
  [B ] [ · ] [B ] [ · ] [B ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ]  R1  ┘
  [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [A ] [ · ] [A ] [ · ] [A ] [ · ]  R0  ┐步驟2: ix=6 (Kit1左)
  [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [B ] [ · ] [B ] [ · ] [B ] [ · ]  R1  ┘
  [ · ] [A ] [ · ] [A ] [ · ] [A ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ]  R0  ┐步驟3: ix=1 (Kit0右)
  [ · ] [B ] [ · ] [B ] [ · ] [B ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ]  R1  ┘
  [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [A ] [ · ] [A ] [ · ] [A ]  R0  ┐步驟4: ix=7 (Kit1右)
  [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [B ] [ · ] [B ] [ · ] [B ]  R1  ┘
  (回到 ix=0, iy += iYHalf*2 → R2/R3...)
```

> spacX=2: 吸嘴間隔 2 Col (C0,C2,C4 或 C1,C3,C5)。Kit0 = Col 0-5, Kit1 = Col 6-11。

#### ACEG 2x4, 2x8, 32S — XItem12_8Suck

**兩段式**: Seg0 = ix=0,1 (Col 0-7, spacX=2), Seg1 = ix=8 (Col 8-11, spacX=1)

```
  C0  C1  C2  C3  C4  C5  C6  C7  C8  C9  CA  CB
  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──    (12×24, iYHalf=2)

  ═══ Seg0: spacX=2, Col=4 ═══
  [A ] [ · ] [A ] [ · ] [A ] [ · ] [A ] [ · ] [ · ] [ · ] [ · ] [ · ]  R0  ┐步驟1: ix=0
  [B ] [ · ] [B ] [ · ] [B ] [ · ] [B ] [ · ] [ · ] [ · ] [ · ] [ · ]  R2  ┘
  [ · ] [A ] [ · ] [A ] [ · ] [A ] [ · ] [A ] [ · ] [ · ] [ · ] [ · ]  R0  ┐步驟2: ix=1
  [ · ] [B ] [ · ] [B ] [ · ] [B ] [ · ] [B ] [ · ] [ · ] [ · ] [ · ]  R2  ┘
  (ix=0, iy+=4 → R4/R6...)
  ... (Seg0 掃完全部 Row)

  ═══ Seg1: spacX=1, Col=4 ═══
  [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [A ] [A ] [A ] [A ]  R0  ┐ix=8
  [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [ · ] [B ] [B ] [B ] [B ]  R2  ┘
  ...
```

> Seg0: 偶數 Col (ix=0) 和奇數 Col (ix=1) 交替，覆蓋 Col 0-7。  
> Seg1: 連續 Col 8-11 (spacX=1)。

---

## §9 XDiv=16 (16×24 HP)

> §19 中無直接對應（32-Site 專用），需 16×24 Pitch ~10×14 的 HP

### 9.1 HP 規格（32-Site 專用）

| 名稱 | Pocket | Pitch X×Y | HP×2 容量 |
|------|:------:|:---------:|:---------:|
| 32Site HP | 16×24 | 10×14 | 768 |

### 9.2 支援的 iInArmType 與路徑圖

#### 32S (2x8_32) / 2416 (2x4_16) — XItem16_8Suck

`spacX=4, Col=4, Row=2, spacY=iYHalf`  
ix = 0 → 1 → 2 → 3（4 步填滿全 16 Col）  
**Plate 方向: 反向** (`bPlateForward=false`, iPlate 1→0)

```
  C0    C4    C8    C12         (簡化：只顯示吸嘴落點 Col)
  ──    ──    ──    ──          (16×24, iYHalf=2)

  ═══ x=0: Col 0,4,8,12 ═══
  [A@0] [A@4] [A@8] [A@12]  R0  ┐步驟1: x=0
  [B@0] [B@4] [B@8] [B@12]  R+Y ┘
  (iy += iRunRute → 下一 Row 群)

  ═══ x=1: Col 1,5,9,13 ═══
  [A@1] [A@5] [A@9] [A@13]  R0  ┐
  [B@1] [B@5] [B@9] [B@13]  R+Y ┘

  ═══ x=2: Col 2,6,10,14 ═══
  [A@2] [A@6] [A@10][A@14]  R0  ┐
  [B@2] [B@6] [B@10][B@14]  R+Y ┘

  ═══ x=3: Col 3,7,11,15 ═══
  [A@3] [A@7] [A@11][A@15]  R0  ┐
  [B@3] [B@7] [B@11][B@15]  R+Y ┘
```

**展開路徑（16×24 完整序列）**:

```
  C0  C1  C2  C3  C4  C5  C6  C7  C8  C9  CA  CB  CC  CD  CE  CF
  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──

  步驟1 (x=0, y=0):
  [A ] [ · ] [ · ] [ · ] [A ] [ · ] [ · ] [ · ] [A ] [ · ] [ · ] [ · ] [A ] [ · ] [ · ] [ · ]  R0
  [B ] [ · ] [ · ] [ · ] [B ] [ · ] [ · ] [ · ] [B ] [ · ] [ · ] [ · ] [B ] [ · ] [ · ] [ · ]  R+Y

  步驟2 (x=1, y=0):
  [ · ] [A ] [ · ] [ · ] [ · ] [A ] [ · ] [ · ] [ · ] [A ] [ · ] [ · ] [ · ] [A ] [ · ] [ · ]  R0
  [ · ] [B ] [ · ] [ · ] [ · ] [B ] [ · ] [ · ] [ · ] [B ] [ · ] [ · ] [ · ] [B ] [ · ] [ · ]  R+Y

  步驟3 (x=2, y=0):
  [ · ] [ · ] [A ] [ · ] [ · ] [ · ] [A ] [ · ] [ · ] [ · ] [A ] [ · ] [ · ] [ · ] [A ] [ · ]  R0
  [ · ] [ · ] [B ] [ · ] [ · ] [ · ] [B ] [ · ] [ · ] [ · ] [B ] [ · ] [ · ] [ · ] [B ] [ · ]  R+Y

  步驟4 (x=3, y=0):
  [ · ] [ · ] [ · ] [A ] [ · ] [ · ] [ · ] [A ] [ · ] [ · ] [ · ] [A ] [ · ] [ · ] [ · ] [A ]  R0
  [ · ] [ · ] [ · ] [B ] [ · ] [ · ] [ · ] [B ] [ · ] [ · ] [ · ] [B ] [ · ] [ · ] [ · ] [B ]  R+Y

  (x=0, y += iRunRute → 下一 Row 群...)
```

---

## §10 全組合掃描參數總覽表

### 10.1 Place 路徑參數矩陣

下表列出 **所有 §19 HP 配置 × iInArmType 有效組合** 的掃描參數。

**欄位**: `Seg` = 段數, `spacX` = X spacing, `Col` = 佔用欄數, `Row` = 佔用列數, `ix` = ix 範圍 [start→end) step, `方向` = Plate 方向

#### XDiv=2

| iInArmType | Seg | spacX | Col | Row | ix 範圍 | spacY | 備註 |
|:---|:---:|:---:|:---:|:---:|:---|:---:|:---|
| 1Suck | 1 | 1 | 1 | 1 | [0→2) +1 | 1 | |
| Ax1R / Ag1R | 1 | 1 | 2 | 1 | [0→1) | 1 | 2Col 剛好填滿 |
| Ax2R / Ag2R | 1 | 1 | 2 | 2 | [0→1) | iYHalf | |
| ACEG (降級) | 1 | 1 | 2 | 1 | [0→1) | 1 | 只用 Aa+Ac |

#### XDiv=3

| iInArmType | Seg | spacX | Col | Row | ix 範圍 | spacY | 備註 |
|:---|:---:|:---:|:---:|:---:|:---|:---:|:---|
| 1Suck | 1 | 1 | 1 | 1 | [0→3) +1 | 1 | |
| Tri (窄Pitch) | 1 | 1 | 1 | 1 | Row-first | 1 | 3_2Suck_3Site 特殊 |
| Tri (寬Pitch) | 1 | 1 | 1 | 1 | Col-first | 1 | 3_2Suck_3Site 特殊 |
| TriX3 / Ax1R / Ag1R | 1 | 1 | 2 | 1 | col-first | 1 | 3_1x2, Col2 單放 |
| Ax2R / Ag2R | 1 | 1 | 2 | 2 | col-first | iYHalf | 3_2x2, 奇數末格封鎖 |

#### XDiv=4

| iInArmType | Seg | spacX | Col | Row | ix 範圍 | spacY | 備註 |
|:---|:---:|:---:|:---:|:---:|:---|:---:|:---|
| 1Suck | 1 | 1 | 1 | 1 | [0→4) +1 | 1 | |
| Ax1R / Ag1R | 1 | 2 | 2 | 1 | [0→2) +1 | 1 | |
| Ax1R + PitchOv | 1 | 1 | 1 | 1 | [0→4) +1 | 1 | 降級逐格 |
| Ax2R / Ag2R | 1 | 2 | 2 | 2 | [0→2) +1 | iYHalf | b4x11/b4x10 邊界 |
| ACEG (1x4/2x4/2x8/32S) | 1 | 1 | 4 | 1/2 | [0→1) | iYHalf | 4 吸嘴填滿 |
| NN14 | 1 | 1 | 4 | 1 | [0→1) | 1 | spacY=1 |
| H28 | 1 | 1 | 4 | 2 | [0→1) | iYHalf | |

#### XDiv=6 標準

| iInArmType | Seg | spacX | Col | Row | ix 範圍 | spacY | 備註 |
|:---|:---:|:---:|:---:|:---:|:---|:---:|:---|
| 1Suck | 1 | 1 | 1 | 1 | [0→6) +1 | 1 | |
| Ax1R / Ag1R | 1 | 3 | 2 | 1 | [0→3) +1 | 1 | |
| Ax1R + PitchOv | 1 | 1 | 1 | 1 | [0→6) +1 | 1 | 降級逐格 |
| Ax2R / Ag2R (CanPutAll) | 1 | 3 | 2 | 2 | [0→3) +1 | iYHalf | 6_2x2 |
| Ax2R (!CanPutAll) | 1 | 3 | 2 | 1~2 | [0→3) +1 | iYHalf | 6_NotStandY |
| ACEG (降級) | 1 | 3 | 2 | 1/2 | [0→3) +1 | iYHalf | 走 1x2/2x2 路徑 |

#### XDiv=6 WideHP

| iInArmType | Seg | spacX | Col | Row | ix 範圍 | spacY | 備註 |
|:---|:---:|:---:|:---:|:---:|:---|:---:|:---|
| W13 | 1 | 1 | 3 | 1 | [0→6) +3 | 1 | 6_3Suck_ACEx |
| W23 / W26 | 1 | 1 | 3 | 2 | [0→6) +3 | iYHalf | 6_3Suck_ACEx |
| ACEG (WideHP) | 2 | S0: 1 | 4 | 2 | S0:[0→1) | iYHalf | S0=Col0-3 |
| | | S1: 1 | 2 | 2 | S1:[4→6) | | S1=Col4-5 |

#### XDiv=8

| iInArmType | Seg | spacX | Col | Row | ix 範圍 | spacY | 備註 |
|:---|:---:|:---:|:---:|:---:|:---|:---:|:---|
| 1Suck | 1 | 1 | 1 | 1 | [0→8) +1 | 1 | |
| Ax1R / Ag1R | 1 | 4 | 2 | 1 | [0→4) +1 | 1 | |
| Ax1R + PitchOv | 1 | 2 | 2 | 1 | 0→1→4→5 | 1 | 特殊步進 |
| Ax2R / Ag2R | 1 | 4 | 2 | 2 | [0→4) +1 | iYHalf | |
| ★ ACEG 全模式 | 1 | 2 | 4 | 1/2 | [0→2) +1 | iYHalf | **主力路徑** |
| ACEG + PitchOv | 2 | S0: 1 | 4 | 1 | S0:[0→1) | 1 | S0=Col0-3 |
| | | S1: 1 | 4 | 1 | S1:[4→5) | | S1=Col4-7 |

#### XDiv=10

| iInArmType | Seg | spacX | Col | Row | ix 範圍 | spacY | 備註 |
|:---|:---:|:---:|:---:|:---:|:---|:---:|:---|
| ACEG (2x4/2x5/2x8/32S) | 2 | S0: 1 | 4 | 2 | S0:[0→5) +1 | iYHalf | 滑動窗口 |
| | | S1: 1 | 2 | 2 | S1:[8→10) | | 餘數 Col8-9 |

#### XDiv=12

| iInArmType | Seg | spacX | Col | Row | ix 範圍 | spacY | 備註 |
|:---|:---:|:---:|:---:|:---:|:---|:---:|:---|
| 1Suck | 1 | 1 | 1 | 1 | [0→12) +1 | 1 | |
| W26 (2x6, Kit) | 1 | 2 | 3 | 2 | Kit{0,6,1,7} | iYHalf | Kit 交錯 |
| ACEG (2x4/2x8/32S) | 2 | S0: 2 | 4 | 2 | S0:[0→2) +1 | iYHalf | Col0-7 |
| | | S1: 1 | 4 | 2 | S1:[8→9) | | Col8-11 |

#### XDiv=16

| iInArmType | Seg | spacX | Col | Row | ix 範圍 | spacY | 備註 |
|:---|:---:|:---:|:---:|:---:|:---|:---:|:---|
| 32S / 2416 | 1 | 4 | 4 | 2 | [0→4) +1 | iYHalf | Plate反向 |

### 10.2 不支援組合（MES0156 或不會進入）

| iInArmType | 不支援的 XDiv | 原因 |
|:---|:---|:---|
| TriSite (1x3) | 2, 4, 6, 8, 10, 12, 16 | 僅 XDiv=3 有 3_2Suck_3Site 分支 |
| NN14 (1x4_4_13 iShtRow=1) | 2, 3, 6, 8, 10, 12, 16 | 僅 XDiv=4 有 NN 分支 |
| W13 / W23 / W26 (WideHP) | 2, 3, 4, 8, 10, 16 | 僅 XDiv=6 WideHP |
| W26 (Kit 2x6) | 2, 3, 4, 6, 8, 10, 16 | 僅 XDiv=12 有 Kit 分支 |
| 2x5 | 2, 3, 4, 6, 12, 16 | 僅 XDiv=8, 10 |
| H14 (1x4_8_Hot) | 2, 3, 4, 6, 10, 12, 16 | 僅 XDiv=8 |
| 2416 (2x4_16) | 2, 3, 4, 6, 10, 12 | 僅 XDiv=8, 16 |
| AxEx/AxxG 全模式 | 10, 12, 16 | Dispatch 不進入 AxEx 分支 |

### 10.3 填滿效率概算

| XDiv | HP × 2 容量 | ACEG 每步放料 | 預估步數/盤 |
|:---:|:---:|:---:|:---:|
| 2 | 24~28 | 2 | 12~14 |
| 3 | 42 | 2 | 21 |
| 4 | 64 | 8 | 8 |
| 6 Std | 132~192 | 4~8 | 17~48 |
| 6 Wide | 132~192 | 6~8 | 17~32 |
| 8 | 192~384 | 8 | 24~48 |
| 10 | 320 | 8+2 | ~40 (含餘數段) |
| 12 | 432~576 | 8+4 | ~48~60 |
| 16 | 768 | 8 | ~96 |
