# SearchPlacePlateXItem 全函式放料路徑圖與適用表

> **Source**: `ainarm_SearchPlacePlate.cpp` (V3.33.900.0_20260331)  
> **最後更新**: 2026-04-07  
> **相關**: [HP_Knowledgebase.md](HP_Knowledgebase.md) — HotPlate 完整知識庫

---

## 目錄

- [§0 SearchPlateToPlace() Dispatch 總覽](#§0-searchplatetoplace-dispatch-總覽)
- [§1 _1Suck — 單吸嘴逐格掃描](#§1-_1suck)
- [§2 _1x2Suck — AxEx/AxxG 雙吸嘴（XDiv=2/4/6/8）](#§2-_1x2suck)
- [§3 _2x2Suck — AxEx/AxxG 四吸嘴 2Row（XDiv=2/4/6/8）](#§3-_2x2suck)
- [§4 XItem3_2Suck_3Site — TriSite 三吸嘴（XDiv=3）](#§4-xitem3_2suck_3site)
- [§5 XItem3_1x2Suck — 雙吸嘴（XDiv=3）](#§5-xitem3_1x2suck)
- [§6 XItem3_2x2Suck — 四吸嘴 2Row（XDiv=3）](#§6-xitem3_2x2suck)
- [§7 XItem4_8Suck — ACEG 四吸嘴（XDiv=4）](#§7-xitem4_8suck)
- [§8 XItem6_2x2_8Suck — 2x2 八吸嘴（XDiv=6, 標準Y）](#§8-xitem6_2x2_8suck)
- [§9 XItem6_3Suck_ACEx — 三吸嘴 WideHP（XDiv=6）](#§9-xitem6_3suck_acex)
- [§10 XItem6_8Suck — ACEG 八吸嘴 WideHP（XDiv=6）](#§10-xitem6_8suck)
- [§11 XItem6_8Suck_NotStandY — 八吸嘴非標準Y（XDiv=6）](#§11-xitem6_8suck_notstanyd)
- [§12 XItem8_8Suck — ACEG 八吸嘴（XDiv=8）](#§12-xitem8_8suck)
- [§13 XItem10_8Suck — ACEG 八吸嘴（XDiv=10）](#§13-xitem10_8suck)
- [§14 XItem12_2x6 — 2x6 十二吸嘴（XDiv=12）](#§14-xitem12_2x6)
- [§15 XItem12_8Suck — ACEG 八吸嘴（XDiv=12）](#§15-xitem12_8suck)
- [§16 XItem16_8Suck — 32Site（XDiv=16）](#§16-xitem16_8suck)
- [§17 全函式適用矩陣總表](#§17-全函式適用矩陣總表)
- [§18 關鍵變數速查](#§18-關鍵變數速查)
- [§19 HP 配置規格表](#§19-hp-配置規格表)

---

## §0 SearchPlateToPlace() Dispatch 總覽

```
SearchPlateToPlace()   ainarm_SearchPlacePlate.cpp line 4443
│
├─ ep1Picker                                        → _1Suck()
├─ DualSite + Ab 關站 + bDualSiteCloseAbCanFull     → _1Suck()
├─ DualSite2x1 + Ba 關站 + 同上                     → _1Suck()
├─ SingleSite / e9045_1x4_1_Ac                      → _1Suck()
│
├─ iPickRow==1 && (AxEx || AxxG)
│   ├─ TriSite1X3 + XDiv==3 + iModeX!=3             → XItem3_2Suck_3Site()
│   ├─ XDiv==3                                      → XItem3_1x2Suck()
│   └─ else                                         → _1x2Suck()
│
├─ iPickRow==2 && (AxEx || AxxG)
│   ├─ XDiv==3                                      → XItem3_2x2Suck()
│   └─ else                                         → _2x2Suck()
│
├─ XDiv==2                                           → _1x2Suck()
├─ XDiv==3                                           → XItem3_1x2Suck()
├─ XDiv==4                                           → XItem4_8Suck()
├─ XDiv==6
│   ├─ WideHP
│   │   ├─ 1x3_4 / 2x3_6 / 2x6_8                   → XItem6_3Suck_ACEx()
│   │   └─ else                                     → XItem6_8Suck()
│   ├─ YPitch 非標準                                 → XItem6_8Suck_NotStandY()
│   └─ else                                         → XItem6_2x2_8Suck()
├─ XDiv==8                                           → XItem8_8Suck()
├─ XDiv==10                                          → XItem10_8Suck()
├─ XDiv==12
│   ├─ b12x16HP_2x6==true                           → XItem12_2x6()
│   └─ else                                         → XItem12_8Suck()
├─ XDiv==16                                          → XItem16_8Suck()
└─ else                                              → MES0156 ❌
```

---

## §1 _1Suck

**全名**: `SearchPlacePlateXItem_1Suck()` (line 2196) | **每次放料**: 1 顆 | **適用 XDiv**: 全部

### 進入條件

| 優先序 | 條件 | 說明 |
|--------|------|------|
| 1 | `USE_PICKER_COUNT == ep1Picker` | 強制單吸嘴 |
| 2 | DualSite + Ab 關站 + `bDualSiteCloseAbCanFullHotplate` | 降為單吸嘴 |
| 3 | DualSite2x1 + Ba 關站 + 同旗標 | 降為單吸嘴 |
| 4 | SingleSite / `e9045_1x4_1_Ac` | 單站測試 |

> 上迴四種條件下，**不論 XDivision 為何**，均進入 `_1Suck()`。其他多吸嘴模式依 XDiv 分派。

### 掃描邏輯

```
Col++  →  Col≥XDiv: Col=0, Row++  →  Row≥YDiv: Col=0, Row=0, Plate++
```

行優先（先填滿同一 Row 的所有 Col，再換下一 Row）。

### AutoClean 保護

HP0 + `eCKPos_HP2` + Row < 4 → 跳至 Row=4（前 4 列保留給 AutoClean）。

### 馬達座標公式

```
Motor X = Prod.XInArm_Plate1_Pick[base] + HotPlateForm.iXPitch × Col
Motor Y = Prod.YInArm_Plate1_Pick[base] - HotPlateForm.iYPitch × Row
```

X 遞增 = Col 增加；Y 遞減 = Row 增加。限制：`Prod.XInArm_Plate1_Pick[0][0] >= BASE_X_TO_HP (6800)`

### 路徑範例

**4x8 HP**: `[1][2][3][4]R0 → [5]~[8]R1 → ... → [29]~[32]R7` (32格/盤)  
**1x3 HP (XDiv=1)**: `[1]R0 → [2]R1 → [3]R2` (3格/盤，雙盤 6格)

### ⚠️ XDiv=1 可達性限制

XDiv=1 在 Dispatch 鏈中無對應分支。**僅 ep1Picker / SingleSite / 關站降級** 可進入 `_1Suck()`。  
多吸嘴模式搭配 XDiv=1 → **MES0156 錯誤**。詳見 §17.5 可達性矩陣。

### ⚠️ 3xN 奇數 YDiv 差異

`_1Suck()` 無奇偶保護，掃描全部格位含 `[Col2][YDiv-1]`。`XItem3_1x2Suck()` 則會跳過該格（見 §5）。

### WAR0151

Col 或 Row < 0 時觸發。檢查 HP 教示值（`Tech.iInArmPlate1X/Y`）及 `BaseX_TO_HP`。

---

## §2 _1x2Suck

**全名**: `SearchPlacePlateXItem_1x2Suck()` (line 2287)  
**每次放料數**: 2 顆（Aa+Ab 或 Aa+Ac，依 AxEx/AxxG）  
**適用 XDiv**: 2, 4, 6, 8（由 dispatch 進入；XDiv=3 會轉 XItem3_1x2Suck）

### 核心機制
使用 `CheckHotPlateHasSpace_9045_8_New_V()` 驗證區塊空間。  
`bPitchOver12000` 為 true 時，改用**間隔放法**（spacX=2, Col=2），避免兩吸嘴超過行程。

### 掃描策略

**一般模式**（`bPitchOver12000==false`）:
```
spacX = XDiv / 2
spacY = iYHalf
Col = 2（一次佔 2 Col）
Row = 1

失敗路徑: ix++ → ix>=spacX: ix=0, iy++ → iy>=YDiv: 換盤
```

**bPitchOver12000 模式**:
```
spacX = 2
spacY = 1
Col = 2

XDiv=4 + AxEx:  Col=1, ix 步進 +=2（0→2），iy 繞回後換盤  ← V3.33.905.0 修正換盤時機
XDiv=8 + AxEx:  Col=2, 步進 0→1→4→5→0（跳過 Col2-3）
其他 AxxG:      Col=2, ix=2 時跳至 ix=4
```

> **pitch 門檻差異**：AxEx(`_13`) 的 pitchOver 分界 = `iXpitchMaxX2`(≈40mm)；
> AxxG(`_14`) = `iXpitchMaxX3`(≈60mm)。同一 45mm Pitch 在 AxEx 會進 pitchOver 缺陷分支、在 AxxG 仍走安全的一般模式。

> ⚠️ **B08 掛機修正（V3.33.905.0, 通富微 P260602）**：
> `XDiv=4 + AxEx + bPitchOver12000`（RogerYang 20250820）失敗遞增分支中，
> 換盤 `if(iPlateSelect==0x03)` 原本綁在 `iy` 繞回層、與 `ix+=2` 同層 →
> 每掃完一欄就換盤，`ix` 與盤號鎖死同相位（盤0 只到 Col0、盤1 只到 Col2），
> 每盤僅用一半欄位 → 兩欄填滿後搜尋恆失敗 → 跑滿 `iHangUpCount(10000)` → **WAR0150 掛機**。
> **修法**：換盤搬進 `if(ix>spacX){ix=0; ...swap}` 內，使 ix 掃完該盤 0、2 兩欄後才換盤。
> `_2x2Suck()` 的 `Steven 20251217`（XDiv=4 AxEx）分支同步修正。詳見
> [debug-knowledge-base B08](../../../../.github/skills/debug-knowledge-base/references/machine-flow-bugs.md#b08)。
>
> **修正後路徑（XDiv=4, bPitchOver12000, AxEx, 雙盤）**：
> ```
> 盤0: (ix0,iy0..7) → (ix2,iy0..7)  [填滿 Col0、Col2]
>      ix+=2→4 (4>2) → ix=0 → 換盤
> 盤1: (ix0,iy0..7) → (ix2,iy0..7)  [填滿 Col0、Col2]
>      ix=0 → 換回盤0 …
> ```


### CheckHotPlateHasSpace 公式

`CheckHotPlateHasSpace_9045_8_New_V(iP, iy, ix, spacY, spacX, Row, Col, state)`:  
迴圈 `Tray.Data[ix + spacX*numC][iy + spacY*numR]`（numC=0..Col-1, numR=0..Row-1）。  
spacX = 吸嘴間 Col 間距，spacY = A/B-Row 間 Row 間距。

### 路徑圖（XDiv=6, 標準 Pitch, AxEx, iPickRow=1）

spacX = XDiv/2 = 3, Col = 2, Row = 1

```
  C0    C1    C2    C3    C4    C5
  ──    ──    ──    ──    ──    ──
  [Aa1] [ · ] [ · ] [Ac1] [ · ] [ · ]  R0  ← 步驟1: ix=0（Aa@C0, Ac@C3）
  [ · ] [Aa2] [ · ] [ · ] [Ac2] [ · ]  R0  ← 步驟2: ix=1（Aa@C1, Ac@C4）
  [ · ] [ · ] [Aa3] [ · ] [ · ] [Ac3]  R0  ← 步驟3: ix=2（Aa@C2, Ac@C5）
  [Aa4] [ · ] [ · ] [Ac4] [ · ] [ · ]  R1  ← 步驟4: ix=0, iy=1
  [ · ] [Aa5] [ · ] [ · ] [Ac5] [ · ]  R1  ← 步驟5: ix=1
  [ · ] [ · ] [Aa6] [ · ] [ · ] [Ac6]  R1  ← 步驟6: ix=2
  ...
```

> **AxEx**：Aa 與 Ac 間隔 spacX=3 Col。每步放 2 顆（Aa+Ac 各 1 顆）。  
> **AxxG**：同理，Aa 與 Ad 間隔 spacX=3 Col。


### 路徑圖（XDiv=8, bPitchOver12000, AxEx）

bPitchOver12000: spacX=2, Col=2, Row=1  
XDiv=8 + AxEx: Col=2, 步進 0→1→4→5

```
  C0    C1    C2    C3    C4    C5    C6    C7
  ──    ──    ──    ──    ──    ──    ──    ──
  [Aa1] [ · ] [Ac1] [ · ] [ · ] [ · ] [ · ] [ · ]  R0  ← ix=0（Aa@C0, Ac@C2）
  [ · ] [Aa2] [ · ] [Ac2] [ · ] [ · ] [ · ] [ · ]  R0  ← ix=1（Aa@C1, Ac@C3）
  [ · ] [ · ] [ · ] [ · ] [Aa3] [ · ] [Ac3] [ · ]  R0  ← ix=4（Aa@C4, Ac@C6）
  [ · ] [ · ] [ · ] [ · ] [ · ] [Aa4] [ · ] [Ac4]  R0  ← ix=5（Aa@C5, Ac@C7）
  ...換 Row...
```

### 進入條件

| 條件 | 說明 |
|------|------|
| iPickRow==1 + AxEx/AxxG + XDiv≠3 | 標準 1Row 雙吸嘴 |
| XDiv==2（ACEG fallthrough） | 2-Col HP |

---

## §3 _2x2Suck

**全名**: `SearchPlacePlateXItem_2x2Suck()` (line 2455)  
**每次放料數**: 2~4 顆（2Row × 2Col，依 IC 狀況）  
**適用 XDiv**: 2, 4, 6, 8（XDiv=3 會轉 XItem3_2x2Suck）

### 核心機制
同 `_1x2Suck` 使用 `CheckHotPlateHasSpace_9045_8_New_V` 但 **Row=2**（兩排同時放）。  
當 `iy+iYHalf >= YDiv` 時降為 Row=1。  
`OnlyTop` fallback：第一次查詢失敗後嘗試 OnlyTop 版本。

### 特殊 HP 旗標處理

| 旗標 | 觸發條件 | 行為差異 |
|------|---------|---------|
| `b6x20HP` | XDiv=6, YDiv=20, YPitch=12.7mm | iYHalf=5, ArmUpSideNoIC 特殊路徑 |
| `b4x11HP` | XDiv=4, YDiv 奇數, iPickRow=2, iPickCol≥2 | iYHalf=1, ix 步進 +=2 |
| `b4x10HP_2x2` | XDiv=4, YPitch 非整除 | iYHalf=1, iy 步進 +=2 |
| `b8x16HP_2x2` | XDiv=8, YDiv=16, QualSite2X2 | iYHalf=1, 逐 Row 掃描 |
| `bPitchOver12000` | XPitch > 120mm(AxEx)/180mm(AxxG) | spacX=2, 間隔放法 |

> ⚠️ **B08 同源修正（V3.33.905.0）**：`_2x2Suck()` 的
> `bPitchOver12000 + AxEx + XDiv==4`（Steven 20251217）失敗遞增分支與 `_1x2Suck` 有
> **完全相同的換盤耦合錯誤**——換盤綁在 `iy` 繞回層，造成 ix 與盤號鎖死同相位、只放半盤掛機。
> 已比照 `_1x2Suck` 將換盤搬進 `if(ix>spacX)` 內同步修正。
> （另注意該分支 `spacX` 來源若為 4，需確認不致使 ix 越界空轉。）


### 路徑圖（XDiv=4, YDiv=8, iYHalf=2, AxEx, 標準 Pitch）

spacX = XDiv/2 = 2, Col = 2, Row = 2, spacY = iYHalf = 2

```
  C0    C1    C2    C3
  ──    ──    ──    ──
  [Aa1] [ · ] [Ac1] [ · ]  R0  ┐步驟1: ix=0, iy=0
  [Ba1] [ · ] [Bc1] [ · ]  R2  ┘(Aa@R0, Ba@R0+iYHalf=R2)
  [ · ] [Aa2] [ · ] [Ac2]  R0  ┐步驟2: ix=1, iy=0
  [ · ] [Ba2] [ · ] [Bc2]  R2  ┘
  [Aa3] [ · ] [Ac3] [ · ]  R4  ┐步驟3: ix=0, iy=4
  [Ba3] [ · ] [Bc3] [ · ]  R6  ┘
  [ · ] [Aa4] [ · ] [Ac4]  R4  ┐步驟4: ix=1, iy=4
  [ · ] [Ba4] [ · ] [Bc4]  R6  ┘
  (HP0 放滿，切 HP1)
```

> **注意**：B-Row 的實際 Row 位置 = `iy + iYHalf`，中間會跳過 Row（R1、R3、R5、R7 被跳過）。  
> 吸嘴 Aa 與 Ac 水平間距 = spacX = 2 Col。Ba 與 Bc 對應在下方 iYHalf Row。

### 進入條件

| 條件 | 說明 |
|------|------|
| iPickRow==2 + AxEx/AxxG + XDiv≠3 | 標準 2Row 雙排 |

---

## §4 XItem3_2Suck_3Site

**全名**: `SearchPlacePlateXItem3_2Suck_3Site()` (line 2917)  
**每次放料數**: 1~2 顆  
**適用 XDiv**: 3（僅 TriSite1X3 + iModeX≠3）

### 掃描邏輯

**XPitch ≤ iXpitchMaxX3/2 時**（窄 Pitch）:
```
Row-first 掃描: for j(Row) → for i(Col)
跳過 Col1 如果 Ab 關站（iSiteMap[0][1]==0）
```

**XPitch > iXpitchMaxX3/2 時**（寬 Pitch）:
```
Col-first 掃描: for i(Col) → for j(Row)
Col2 + Item[0][0]==NULL_IC → iForPlaceHPX3Step=1
```

### 路徑圖（3x5 HP, 窄Pitch, Ab開啟）

```
  C0  C1  C2
  ──  ──  ──
  [ 1] [ 2] [ 3]  R0
  [ 4] [ 5] [ 6]  R1
  [ 7] [ 8] [ 9]  R2
  [10] [11] [12]  R3
  [13] [14] [15]  R4
```

### 路徑圖（3x5 HP, 窄Pitch, Ab關站）

```
  C0  C1  C2
  ──  ──  ──
  [ 1] [跳] [ 2]  R0
  [ 3] [跳] [ 4]  R1
  [ 5] [跳] [ 6]  R2
  [ 7] [跳] [ 8]  R3
  [ 9] [跳] [10]  R4
```

### 路徑圖（3x7 HP, 寬Pitch）

```
  C0  C1  C2
  ──  ──  ──
  [ 1] [ 8] [15]  R0
  [ 2] [ 9] [16]  R1
  [ 3] [10] [17]  R2
  [ 4] [11] [18]  R3
  [ 5] [12] [19]  R4
  [ 6] [13] [20]  R5
  [ 7] [14] [21]  R6
```

### 進入條件

| 條件 | 說明 |
|------|------|
| iPickRow==1 + AxEx/AxxG + TriSite1X3 + XDiv==3 + iModeX≠3 | TriSite 專用三站位 |

---

## §5 XItem3_1x2Suck

**全名**: `SearchPlacePlateXItem3_1x2Suck()` (line 3023)  
**每次放料數**: 2 顆（Col0+Col1 同步放，Col2 單放時 iForPlaceHPX3Step=1）  
**適用 XDiv**: 3

### 掃描邏輯
```
Col-first: for i(Col) → for j(Row)
奇數 YDiv 保護: 跳過 [XDiv-1][YDiv-1]（即 [2][末尾]）
iForPlaceHPX3Step: x==2 && y%2==1 → step=1（Col2 單放模式）
```

### 路徑圖（3x6 HP, 偶數 YDiv）

XItem3_1x2Suck 逐格掃描（非 CheckHotPlateHasSpace），找到空格即返回。  
AxEx 模式 Aa + Ac 兩吸嘴，放料時 Aa→Col[x], Ac→Col[x+spacX]（spacX=1）。  
Col0+Col1 同步放；到 Col2 時只有 Aa 能放（Ac 位超出 XDiv）。

```
  C0    C1    C2
  ──    ──    ──
  [Aa1] [Ac1] [ · ]  R0  ← 步驟1: x=0 (Aa@Col0, Ac@Col1)
  [Aa2] [Ac2] [ · ]  R1  ← 步驟2
  [Aa3] [Ac3] [ · ]  R2  ← 步驟3
  [Aa4] [Ac4] [ · ]  R3  ← 步驟4
  [Aa5] [Ac5] [ · ]  R4  ← 步驟5
  [Aa6] [Ac6] [ · ]  R5  ← 步驟6
  (x 進到 Col1: 已被 Ac 放完→跳到 Col2)
  [ · ]  [ · ]  [Aa7]  R0  ← 步驟7: x=2, step=0（y偶數）
  [ · ]  [ · ]  [Aa8]  R1  ← 步驟8: x=2, step=1（y奇數→單放）
  [ · ]  [ · ]  [Aa9]  R2  ← 步驟9
  ...
```

> **注意**：Col0/Col1 是同步放（Aa@Col0 + Ac@Col1），Col2 因超出 spacX 範圍只有 Aa 放。  
> iForPlaceHPX3Step=1 時（x==2 且 y 為奇數），只放 Aa 的 IC 到 Col2。

### 路徑圖（3x5 HP, 奇數 YDiv）

```
  C0    C1    C2
  ──    ──    ──
  [Aa1] [Ac1] [ · ]  R0  ← 步驟1
  [Aa2] [Ac2] [ · ]  R1  ← 步驟2
  [Aa3] [Ac3] [ · ]  R2  ← 步驟3
  [Aa4] [Ac4] [ · ]  R3  ← 步驟4
  [Aa5] [Ac5] [ · ]  R4  ← 步驟5
  [ · ]  [ · ]  [Aa6]  R0  ← 步驟6: x=2
  [ · ]  [ · ]  [Aa7]  R1  ← 步驟7
  [ · ]  [ · ]  [Aa8]  R2  ← 步驟8
  [ · ]  [ · ]  [Aa9]  R3  ← 步驟9
  [ · ]  [ · ]  [跳]   R4  ← ⚠️ 奇數YDiv保護: [2][4] 被跳過！
  (切換 HP1)
```

### 進入條件

| 條件 | 說明 |
|------|------|
| iPickRow==1 + AxEx/AxxG + XDiv==3（非TriSite） | 1Row 雙吸嘴 + 3-Col HP |
| XDiv==3（ACEG fallthrough） | 4+吸嘴的 1Row 路徑 |

---

## §6 XItem3_2x2Suck

**全名**: `SearchPlacePlateXItem3_2x2Suck()` (line 3104)  
**每次放料數**: 2~4 顆  
**適用 XDiv**: 3

### 掃描邏輯
```
Col-first: for i(Col) → for j(Row)
奇數 YDiv: SetTraySingleData([XDiv-1][YDiv-1], HAS_NULL_IC)（強制封鎖末格）
iForPlaceHPX3Step: x==2 + Item[0][0]==NULL_IC → step=1
```

### 路徑圖（3x6 HP, AxEx 2Row）

掃描方式：Col-first (`for i(Col) → for j(Row)`)，每步定位 Col[x]，同時放 A-Row 和 B-Row。  
Col2 時，若 Aa 吸嘴無 IC 則 iForPlaceHPX3Step=1（只放 Ac 側）。

```
  C0    C1    C2
  ──    ──    ──
  [Aa1] [Ab1] [ · ]  R0  ┐步驟1: x=0 (2Row: Aa@Col0, Ab@Col1)
  [Ba1] [Bb1] [ · ]  R1  ┘
  [Aa2] [Ab2] [ · ]  R2  ┐步驟2
  [Ba2] [Bb2] [ · ]  R3  ┘
  [Aa3] [Ab3] [ · ]  R4  ┐步驟3
  [Ba3] [Bb3] [ · ]  R5  ┘
  [ · ] [ · ] [Aa4]  R0  ┐步驟4: x=2
  [ · ] [ · ] [Ba4]  R1  ┘(step 由 Item[0][0] 決定)
  [ · ] [ · ] [Aa5]  R2  ┐步驟5
  [ · ] [ · ] [Ba5]  R3  ┘
  [ · ] [ · ] [Aa6]  R4  ┐步驟6
  [ · ] [ · ] [Ba6]  R5  ┘
```

> Col0/Col1 由 Aa+Ab 同步放（AxEx: Aa@Col0, Ab=Ac@Col1），Col2 因寬度限制只有單吸嘴。

### 進入條件

| 條件 | 說明 |
|------|------|
| iPickRow==2 + AxEx/AxxG + XDiv==3 | 2Row 雙排 + 3-Col HP |

---

## §7 XItem4_8Suck

**全名**: `SearchPlacePlateXItem4_8Suck()` (line 3176)  
**每次放料數**: 4~8 顆（Col=4，2Row 時最多 8 顆）  
**適用 XDiv**: 4

### 核心機制
使用 `CheckHotPlateHasSpace_9045_8_New_V` 驗證：一次佔 **4 Col × 1~2 Row** 的區塊。

### 掃描策略

| 條件 | spacX | spacY | Row | Col |
|------|-------|-------|-----|-----|
| iShtRow==1（2x4 NN 模式）| 1 | 1 | 1 | 4 |
| bPitchOver12000 | 1 | 1 | 1 | 1（逐格）|
| YPitch 可放 + iy+iYHalf < YDiv | 1 | iYHalf | 2 | 4 |
| else（尾端/不可放）| 1 | iYHalf | 1 | 4 |

### 路徑圖（4x8 HP, ACEG, iYHalf=2）

spacX = 1, Col = 4, Row = 2, spacY = iYHalf = 2

```
  C0    C1    C2    C3
  ──    ──    ──    ──
  [Aa1] [Ac1] [Ae1] [Ag1]  R0  ┐步驟1: ix=0, iy=0
  [Ba1] [Bc1] [Be1] [Bg1]  R2  ┘(B-Row = iy+iYHalf = R2)
  [Aa2] [Ac2] [Ae2] [Ag2]  R4  ┐步驟2: ix=0, iy=4
  [Ba2] [Bc2] [Be2] [Bg2]  R6  ┘
  (HP0 放滿，切 HP1)
```

> spacX=1 表示 4 個吸嘴各佔相鄰 Col。Row=2 時 A-Row@iy, B-Row@iy+iYHalf。

### 進入條件

| 條件 | 說明 |
|------|------|
| XDiv==4（ACEG fallthrough） | 4-Col HP |

---

## §8 XItem6_2x2_8Suck

**全名**: `SearchPlacePlateXItem6_2x2_8Suck()` (line 3297)  
**每次放料數**: 4~8 顆  
**適用 XDiv**: 6（`HotPlateYPitchCanPutAll()==true` 且非 WideHP）

### 核心機制
**三段式掃描**：先放 Col0-1（ix=0），再放 Col2-3（ix=1），最後放 Col4-5（ix=2）。

### iForPlaceHPX6Step 判斷
```
Item[0][0]==NULL && Item[0][2]==NULL && Item[1][0]==NULL && Item[1][2]==NULL
  → iForPlaceHPX6Step=1（只放 Col4-5 的右側部分）
否則：iForPlaceHPX6Step=0（放 Col0-3 全域）
```

### 掃描策略

| ix | Col 範圍 | spacX | Col | Row | 說明 |
|----|---------|-------|-----|-----|------|
| 0/1 | Col0-3 | 3 | 2 | 1~2 | 左半區（4 格） |
| 2 | Col4-5 | 3 | 2 | 2 | 右半區（2 格） |

### 路徑圖（6x11 HP, AxEx, 2Row, iYHalf=2）

spacX = 3, Col = 2, Row = 2, spacY = iYHalf = 2

```
  C0    C1    C2    C3    C4    C5
  ──    ──    ──    ──    ──    ──
  [Aa1] [ · ] [ · ] [Ac1] [ · ] [ · ]  R0  ┐步驟1: ix=0
  [Ba1] [ · ] [ · ] [Bc1] [ · ] [ · ]  R2  ┘(B-Row@R2)
  [ · ] [Aa2] [ · ] [ · ] [Ac2] [ · ]  R0  ┐步驟2: ix=1
  [ · ] [Ba2] [ · ] [ · ] [Bc2] [ · ]  R2  ┘
  [ · ] [ · ] [Aa3] [ · ] [ · ] [Ac3]  R0  ┐步驟3: ix=2
  [ · ] [ · ] [Ba3] [ · ] [ · ] [Bc3]  R2  ┘
  [Aa4] [ · ] [ · ] [Ac4] [ · ] [ · ]  R4  ┐步驟4: ix=0, iy=4
  [Ba4] [ · ] [ · ] [Bc4] [ · ] [ · ]  R6  ┘
  ...
```

> A-Row 吸嘴 Aa 在 Col[ix]，Ac 在 Col[ix+3]。B-Row 同理。

### 進入條件

| 條件 | 說明 |
|------|------|
| XDiv==6 + 非WideHP + YPitch正常 | 標準 6-Col HP |

---

## §9 XItem6_3Suck_ACEx

**全名**: `SearchPlacePlateXItem6_3Suck_ACEx()` (line 3411)  
**每次放料數**: 3~6 顆（Col=3，1~2 Row）  
**適用 XDiv**: 6（WideHP + 1x3_4/2x3_6/2x6_8）

### 核心機制
每次佔 **3 Col × 1~2 Row**。ix 步進 **+=3**。

### 掃描策略

| ix | Col 範圍 | spacX | Col | 說明 |
|----|---------|-------|-----|------|
| 0 | Col0-2 | 1 | 3 | 左半 |
| 3 | Col3-5 | 1 | 3 | 右半 |

### 路徑圖（6x11 HP, WideHP, 2Row, iYHalf=2）

spacX = 1, Col = 3, Row = 2, spacY = iYHalf = 2

```
  C0    C1    C2    C3    C4    C5
  ──    ──    ──    ──    ──    ──
  [Aa1] [Ac1] [Ae1] [ · ] [ · ] [ · ]  R0  ┐步驟1: ix=0
  [Ba1] [Bc1] [Be1] [ · ] [ · ] [ · ]  R2  ┘(B-Row@R2)
  [ · ] [ · ] [ · ] [Aa2] [Ac2] [Ae2]  R0  ┐步驟2: ix=3
  [ · ] [ · ] [ · ] [Ba2] [Bc2] [Be2]  R2  ┘
  [Aa3] [Ac3] [Ae3] [ · ] [ · ] [ · ]  R4  ┐步驟3: ix=0, iy=4
  [Ba3] [Bc3] [Be3] [ · ] [ · ] [ · ]  R6  ┘
  ...
```

> spacX=1，3 吸嘴佔連續 3 Col。ix 步進 +=3（左半 Col0-2 → 右半 Col3-5）。

### 進入條件

| 條件 | 說明 |
|------|------|
| XDiv==6 + WideHP + (1x3_4 / 2x3_6 / 2x6_8) | 寬板 3-吸嘴間距放料 |

---

## §10 XItem6_8Suck

**全名**: `SearchPlacePlateXItem6_8Suck()` (line 3499)  
**每次放料數**: 4~8 顆  
**適用 XDiv**: 6（WideHP + ACEG，非 1x3/2x3/2x6）

### 核心機制
**兩段式掃描**：  
- ix=0: Col0-3（4 格，`Col=4`）  
- ix=4: Col4-5（2 格，`Col=2`，`iForPlaceHPX6Step` 判斷）

### iForPlaceHPX6Step 判斷（ix==4 時）
```
Item[0][0]==NULL && Item[0][1]==NULL && Item[1][0]==NULL && Item[1][1]==NULL
  → iForPlaceHPX6Step=1（只放右側 Col4-5）
否則：iForPlaceHPX6Step=0
```

### 路徑圖（6x11 HP, WideHP, ACEG, iYHalf=2）

**ix=0 段**：spacX = 1, Col = 4, Row = 2, spacY = iYHalf = 2
**ix=4 段**：spacX = 1, Col = 2, Row = 1~2

```
  C0    C1    C2    C3    C4    C5
  ──    ──    ──    ──    ──    ──
  [Aa1] [Ac1] [Ae1] [Ag1] [ · ] [ · ]  R0  ┐步驟1: ix=0, iy=0 (Col=4)
  [Ba1] [Bc1] [Be1] [Bg1] [ · ] [ · ]  R2  ┘(B-Row@R2)
  [Aa2] [Ac2] [Ae2] [Ag2] [ · ] [ · ]  R4  ┐步驟2: ix=0, iy=4
  [Ba2] [Bc2] [Be2] [Bg2] [ · ] [ · ]  R6  ┘
  ...（ix=0 掃完全部 Row 後）
  [ · ] [ · ] [ · ] [ · ] [Ae①] [Ag①]  R0  ┐步驟N: ix=4 (Col=2)
  [ · ] [ · ] [ · ] [ · ] [Be①] [Bg①]  R2  ┘
  [ · ] [ · ] [ · ] [ · ] [Ae②] [Ag②]  R4  ┐步驟N+1
  [ · ] [ · ] [ · ] [ · ] [Be②] [Bg②]  R6  ┘
  ...
```

> ix=0: 4 吸嘴佔 Col0-3。ix=4: 2 吸嘴佔 Col4-5。iForPlaceHPX6Step 決定 ix=4 時放哪些吸嘴的 IC。

### 進入條件

| 條件 | 說明 |
|------|------|
| XDiv==6 + WideHP + ACEG（非 1x3/2x3/2x6） | 寬板 4-吸嘴 ACEG |

---

## §11 XItem6_8Suck_NotStandY

**全名**: `SearchPlacePlateXItem6_8Suck_NotStandY()` (line 3636)  
**每次放料數**: 2~4 顆  
**適用 XDiv**: 6（非WideHP + `HotPlateYPitchCanPutAll()==false`）

### 核心機制
直接使用 `MOT[].Tray.Data` flag 陣列檢查（不經 CheckHotPlateHasSpace）。  
**掃描方式特殊**：`x` 值分為 0/1/2 三段，x=2 表示右半區 Col3-5。

### iForPlaceHPX6Step 規則
```
x==0 → step=0
x==2 → y%2==0 → step=0, y%2==1 → step=1
x==1 → step=1
```

### 路徑圖（6x11 HP, 非標準Y）

此函式直接使用 `flag[2][2]` 檢查 `Tray.Data[x+j*3][y]`（j=0,1），  
即 Col[x] 和 Col[x+3] 兩列。

```
  C0    C1    C2    C3    C4    C5
  ──    ──    ──    ──    ──    ──
  [Aa1] [ · ] [ · ] [Ac1] [ · ] [ · ]  R0  ← x=0 (step=0, Col0+Col3)
  [ · ] [Aa2] [ · ] [ · ] [Ac2] [ · ]  R0  ← x=1 (step=1, Col1+Col4)
  [ · ] [ · ] [Aa3] [ · ] [ · ] [Ac3]  R1  ← x=2 (step=0/1, Col2+Col5)
  [Aa4] [ · ] [ · ] [Ac4] [ · ] [ · ]  R1  ← x=0
  [ · ] [Aa5] [ · ] [ · ] [Ac5] [ · ]  R1  ← x=1
  ...
  (x 交替: 0→1→(x=2 穿插）→ 0→1 ...)
```

> ⚠️ 這個函式的路徑最為複雜，包含 `PlaceSpecialPos()` 特殊位置回退。  
> iPlaceHPOrder 決定 Row1 或 Row2 優先。iForPlaceHPX6Step 由(x,y%2) 組合決定。

### 進入條件

| 條件 | 說明 |
|------|------|
| XDiv==6 + 非WideHP + YPitch 不足 | Y 間距不足以同時放兩排 |

---

## §12 XItem8_8Suck

**全名**: `SearchPlacePlateXItem8_8Suck()` (line 3763)  
**每次放料數**: 4~8 顆  
**適用 XDiv**: 8

### 核心機制
使用 `CheckHotPlateHasSpace_9045_8_New_V`，一次佔 **4 Col × 1~2 Row**。  
`bPitchOver12000` 時 spacX=1（左右半各獨立），否則 spacX=2（跨半場掃描）。

### 掃描策略

| 條件 | spacX | Col | 說明 |
|------|-------|-----|------|
| bPitchOver12000 | 1 | 4 | ix 步進 XDiv/2（0→4）|
| 一般 | 2 | 4 | ix 步進 1（0→1→換Row）|

### ⚠️ 游標推進順序：`iy` 內圈、`ix` 外圈（**同一欄組先掃完所有列**）

```c
// SearchPlacePlateXItem8_8Suck()  失敗路徑（只有失敗才動游標）
if(bSuccess==false){
    iy++;                                              // ★ 列是內圈
    if(iy>=HotPlateForm.YDivision){
        iy=0;
        ++ix;                                          // ★ 列掃完才換欄組
        if(ix>=HotPlateForm.XDivision/4) ix=0;         // ★ 欄組上限 = XDiv/4
    }
}
```

兩個常被讀錯的點：

1. `iPlacePlateX[0]` / `iPlacePlateY[0]` 是**持續游標**（`int &ix=iPlacePlateX[0]`），
   **成功時不重置**。所以盤面呈滾動掃描，不是每次從 (0,0) 重找。
2. **`iy` 是內圈** → 連續幾趟放料會沿著**同一欄組**往下推列，
   把該欄組所有列用完才換到另一個欄組。（不是「兩步填滿一組 Row 再往下」。）

### 路徑圖（8x16 HP, ACEG, 一般, iYHalf=2）

spacX = 2, Col = 4, Row = 2, spacY = iYHalf = 2；`ix ∈ {0,1}`（= XDiv/4）

> 🔧 20261001 合併註記：repo 舊版此圖畫成「步驟1 ix=0 → 步驟2 ix=1，兩步填滿一組 Row」，與程式不符（`iy` 為內圈、`ix` 欄組上限 = XDiv/4），以本版為準。

```
  C0    C1    C2    C3    C4    C5    C6    C7
  ──    ──    ──    ──    ──    ──    ──    ──
  [Aa1] [ · ] [Ac1] [ · ] [Ae1] [ · ] [Ag1] [ · ]  R0   ┐趟1: ix=0, iy=0
  [ Aa2] [·]  [Ac2] [ · ] [Ae2] [ · ] [Ag2] [ · ]  R1   ┐趟2: ix=0, iy=1
  [Ba1] [ · ] [Bc1] [ · ] [Be1] [ · ] [Bg1] [ · ]  R2   ┘(趟1 的 B 排)
  [Ba2] [ · ] [Bc2] [ · ] [Be2] [ · ] [Bg2] [ · ]  R3   ┘(趟2 的 B 排)
  [Aa3] [ · ] [Ac3] [ · ] [Ae3] [ · ] [Ag3] [ · ]  R4   ┐趟3: ix=0, iy=4
  [Ba3] [ · ] [Bc3] [ · ] [Be3] [ · ] [Bg3] [ · ]  R6   ┘
  ...（ix=0 的所有列用完，才換 ix=1 → 奇數 Col）
```

> spacX=2: Aa@Col[ix], Ac@Col[ix+2], Ae@Col[ix+4], Ag@Col[ix+6]。
> ix=0: 偶數 Col；ix=1: 奇數 Col。

### ⚠️ `Row=1` 只該給「只剩一排有料」用；滿手走它會造成永遠取不滿

```c
if(HotPlateYPitchCanPutAll() && iy+iYHalf<HotPlateForm.YDivision){
    if(ArmRow1NotICForPlaceToPlate() && InArmSuck.HasIC()) { Row=1; }   // 上排已空
    else                                                   { Row=2; }
}else{ Row=1; }                                                         // ★ 無條件
```

第一個分支的 `Row=1` 有守門（`ArmRow1NotICForPlaceToPlate()`＝上排已空）；
但 `else`（游標到最後 `iYHalf` 列）是**無條件** `Row=1`。
此時若吸嘴滿手，上排 4 顆落在第 R 列、下排 4 顆落在第 R+1 列（游標只前進 1）——
兩列相距 1，而吸嘴兩排固定相距 `iYHalf` 列 → 這 8 顆**永遠湊不回一組**。

取料群組 == 放料群組（帳本 `PickFromHPList`，見 [SKILL.md §14](../SKILL.md)），
取料端沒有補救餘地 ⇒ 病徵：**「HotPlate 尾盤明明還有 8 顆料，InArm 只取 4 顆就去放 Shuttle」**。

### ⚠️ 配對數上限：8×16 盤的可用容量是 **112** 不是 128

把 `{0..YDiv-1}` 用 `(R, R+iYHalf)` 配對，以 `iYHalf` 取模的分支若節點數為奇數，
每個奇數分支會剩 1 列：

| YDiv | iYHalf | 取模分支大小 | 最多配對 | 配不到對的列 | 可用容量（8 欄）|
|---|---|---|---|---|---|
| 16 | 3 | 6 / 5 / 5 | **7 對＝14 列** | 2 列 | **112** / 128 |
| 16 | 2 | 8 / 8 | 8 對＝16 列 | 0 | 128 / 128 |
| 8 | 2 | 4 / 4 | 4 對＝8 列 | 0 | 64 / 64 |

⇒ `iYHalf` 是奇數且 `YDiv` 不是 `2×iYHalf` 的倍數時，一定會有列配不到對。
判斷「放不放得下整組」也**不可以只看剩幾格空**（最後 `iYHalf` 列的空格湊不成整組）。

模擬驗證與可直接套用的實作見
[ht9045-motionview-html-ui / placement-algorithms.md §2](../../ht9045-motionview-html-ui/references/placement-algorithms.md)。

### 進入條件

| 條件 | 說明 |
|------|------|
| XDiv==8（ACEG fallthrough） | 8-Col HP |

---

## §13 XItem10_8Suck

**全名**: `SearchPlacePlateXItem10_8Suck()` (line 3867)  
**每次放料數**: 4~8 顆  
**適用 XDiv**: 10

### 核心機制
**兩段式掃描**：  
- ix = 0~4: Col0-3（4 格），ix 步進 1，共 5 起始位	 （ix 遞增到 5 後重置並換 Row）  
- ix = 8: Col8-9（2 格）

> ⚠️ 程式碼中 ix 只用 0~4 和 8，**Col5-7 交給 ix=1~4 的 CheckHotPlateHasSpace 覆蓋**。

### iForPlaceHPX10Step
```
After search:
  若 Item[0][0]==NULL && Item[0][1]==NULL && Item[0][2] && Item[0][3]
  或 Item[1][0]==NULL && Item[1][1]==NULL && Item[1][2] && Item[1][3]
  → iForPlaceHPX10Step=2（左半無IC，右半有IC → 偏移放）
否則 → step=0
```

### 路徑圖（10x16 HP, ACEG, iYHalf=2）

**ix=0~4 段**：spacX = 1, Col = 4, Row = 2。吸嘴佔 Col[ix]~Col[ix+3]。  
**ix=8 段**：spacX = 1, Col = 2。吸嘴佔 Col8~Col9。

```
  C0  C1  C2  C3  C4  C5  C6  C7  C8  C9
  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──
  [A1][A1][A1][A1]  ·   ·   ·   ·   ·   ·   R0  ┐步驟1: ix=0 (Col0-3)
  [B1][B1][B1][B1]  ·   ·   ·   ·   ·   ·   R2  ┘
   ·  [A2][A2][A2][A2]  ·   ·   ·   ·   ·   R0  ┐步驟2: ix=1 (Col1-4)
   ·  [B2][B2][B2][B2]  ·   ·   ·   ·   ·   R2  ┘
   ·   ·  [A3][A3][A3][A3]  ·   ·   ·   ·   R0  ┐步驟3: ix=2 (Col2-5)
   ·   ·  [B3][B3][B3][B3]  ·   ·   ·   ·   R2  ┘
   ·   ·   ·  [A4][A4][A4][A4]  ·   ·   ·   R0  ┐步驟4: ix=3 (Col3-6)
   ·   ·   ·  [B4][B4][B4][B4]  ·   ·   ·   R2  ┘
   ·   ·   ·   ·  [A5][A5][A5][A5]  ·   ·   R0  ┐步驟5: ix=4 (Col4-7)
   ·   ·   ·   ·  [B5][B5][B5][B5]  ·   ·   R2  ┘
  (ix=0~4 完成 R0/R2 → iy+=2 → R4/R6...)
  (全部 Row 完成後)
   ·   ·   ·   ·   ·   ·   ·   ·  [AM][AM]  R0  ┐步驟M: ix=8 (Col8-9)
   ·   ·   ·   ·   ·   ·   ·   ·  [BM][BM]  R2  ┘
  ...
```

> ix=0~4 每步 4 Col 滑動窗口，覆蓋 Col0~7。ix=8 補 Col8-9。

### 進入條件

| 條件 | 說明 |
|------|------|
| XDiv==10（ACEG fallthrough） | 10-Col HP（ATK 10×16）|

---

## §14 XItem12_2x6

**全名**: `SearchPlacePlateXItem12_2x6()` (line 4008)  
**每次放料數**: 6 顆（Col=3, Row=2, spacX=2）  
**適用 XDiv**: 12（`b12x16HP_2x6==true`）

### 核心機制
每次佔 **3 Col × 2 Row × spacX=2 段**。  
Kit-based 掃描：ix 交替 0→6→1→7（Kit0→Kit1→Kit0偏→Kit1偏）。

### 掃描策略

**iXYPitchVariable 模式**:
```
ix步進: 0 → 6 → 1 → 7 → (0, iy++) → ...
```

**標準模式（含 Row13/14 特殊處理）**:
```
ix步進: 0 → 6 → 1 → 7 → (0, iy++) → ...
Row13/14: 每個 ix 位置先試 iy=13, 再試 iy=14
```

### 路徑圖（12x16 HP, 2x6, iXYPitchVariable）

spacX = 2, Col = 3, Row = 2。吸嘴每步佔 Col[ix], Col[ix+2], Col[ix+4]。

```
  C0  C1  C2  C3  C4  C5  C6  C7  C8  C9  C10 C11
  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──  ─── ───
  [A1] ·  [C1] ·  [E1] ·   ·   ·   ·   ·   ·   ·   R0 ┐步驟1: ix=0 (Kit0左)
  [B1] ·  [D1] ·  [F1] ·   ·   ·   ·   ·   ·   ·   R2 ┘
   ·   ·   ·   ·   ·   ·  [A2] ·  [C2] ·  [E2] ·   R0 ┐步驟2: ix=6 (Kit1左)
   ·   ·   ·   ·   ·   ·  [B2] ·  [D2] ·  [F2] ·   R2 ┘
   ·  [A3] ·  [C3] ·  [E3]  ·   ·   ·   ·   ·   ·   R0 ┐步驟3: ix=1 (Kit0右)
   ·  [B3] ·  [D3] ·  [F3]  ·   ·   ·   ·   ·   ·   R2 ┘
   ·   ·   ·   ·   ·   ·   ·  [A4] ·  [C4] ·  [E4]  R0 ┐步驟4: ix=7 (Kit1右)
   ·   ·   ·   ·   ·   ·   ·  [B4] ·  [D4] ·  [F4]  R2 ┘
  (全回 ix=0, iy+=iYHalf)
```

> spacX=2: 吸嘴間隔 2 Col（如 Col0,2,4 或 Col1,3,5）。  
> Kit0 放 Col0-5，Kit1 放 Col6-11。交替順序：0→6→1→7。

### 進入條件

| 條件 | 說明 |
|------|------|
| XDiv==12 + b12x16HP_2x6==true | 12-Col HP + 2x6 放法 |

---

## §15 XItem12_8Suck

**全名**: `SearchPlacePlateXItem12_8Suck()` (line 4205)  
**每次放料數**: 4~8 顆  
**適用 XDiv**: 12（`b12x16HP_2x6==false`）

### 核心機制
**兩段式掃描**：  
- ix = 0/1: Col0-7（4 格 × spacX=2），ix 步進 1  
- ix = 8: Col8-11（4 格 × spacX=1）

### 路徑圖（12x24 HP, ACEG, iYHalf=2）

**ix=0/1 段**：spacX = 2, Col = 4 → 吸嘴佔 Col[ix], Col[ix+2], Col[ix+4], Col[ix+6]  
**ix=8 段**：spacX = 1, Col = 4 → 吸嘴佔 Col8, Col9, Col10, Col11

```
  C0  C1  C2  C3  C4  C5  C6  C7  C8  C9  C10 C11
  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──  ─── ───
  [A1] ·  [C1] ·  [E1] ·  [G1] ·   ·   ·   ·   ·   R0 ┐步驟1: ix=0 (spacX=2)
  [B1] ·  [D1] ·  [F1] ·  [H1] ·   ·   ·   ·   ·   R2 ┘
   ·  [A2] ·  [C2] ·  [E2] ·  [G2]  ·   ·   ·   ·   R0 ┐步驟2: ix=1
   ·  [B2] ·  [D2] ·  [F2] ·  [H2]  ·   ·   ·   ·   R2 ┘
  (ix=0,1 完成 → iy+=iYHalf → 跳行)
  [A3] ·  [C3] ·  [E3] ·  [G3] ·   ·   ·   ·   ·   R4 ┐步驟3
  [B3] ·  [D3] ·  [F3] ·  [H3] ·   ·   ·   ·   ·   R6 ┘
  ... (掃完全部 Row 後)
   ·   ·   ·   ·   ·   ·   ·   ·  [AM][CM][EM][GM]  R0 ┐步驟M: ix=8 (spacX=1)
   ·   ·   ·   ·   ·   ·   ·   ·  [BM][DM][FM][HM]  R2 ┘
  ...
```

> ix=0: 偶數 Col。ix=1: 奇數 Col。ix=8: spacX=1，連續 Col 8-11。

### 進入條件

| 條件 | 說明 |
|------|------|
| XDiv==12 + b12x16HP_2x6==false | 12-Col HP 標準 ACEG |

---

## §16 XItem16_8Suck

**全名**: `SearchPlacePlateXItem16_8Suck()` (line 4348)  
**每次放料數**: 8~16 顆  
**適用 XDiv**: 16

### 核心機制
直接使用 `MOT[].Tray.Data` flag 陣列檢查 `flag[2][4]`。  
iStep=4, iLimit=4。每次佔 **4 Col × 2 Row**。  
iRunRute = ARM_HP_Y_PITCH / HP_YPitch（Row 跳行步伐）。

### 路徑圖（16x24 HP, 32Site）

iStep=4, iLimit=4。`flag[Row][Step]` = `Tray.Data[x + Step*4][y]`（Row0）/ `Tray.Data[x + Step*4][y+iYHalf]`（Row1）。  
每步佔 4 Col（Col[x], Col[x+4], Col[x+8], Col[x+12]），2 Row。

```
  C0  C1  C2  C3  C4  C5  C6  C7  C8  C9  C10 C11 C12 C13 C14 C15
  ──  ──  ──  ──  ──  ──  ──  ──  ──  ──  ─── ─── ─── ─── ─── ───
  [A1] ·   ·   ·  [A1] ·   ·   ·  [A1] ·   ·   ·  [A1] ·   ·   ·   R0  ┐步驟1: x=0 (Col0,4,8,12)
  [B1] ·   ·   ·  [B1] ·   ·   ·  [B1] ·   ·   ·  [B1] ·   ·   ·   R+Y ┘(A=Row0, B=Row0+iYHalf)
   ·  [A2] ·   ·   ·  [A2] ·   ·   ·  [A2] ·   ·   ·  [A2] ·   ·   R0  ┐步驟2: x=1 (Col1,5,9,13)
   ·  [B2] ·   ·   ·  [B2] ·   ·   ·  [B2] ·   ·   ·  [B2] ·   ·   R+Y ┘
   ·   ·  [A3] ·   ·   ·  [A3] ·   ·   ·  [A3] ·   ·   ·  [A3] ·   R0  ┐步驟3: x=2 (Col2,6,10,14)
   ·   ·  [B3] ·   ·   ·  [B3] ·   ·   ·  [B3] ·   ·   ·  [B3] ·   R+Y ┘
   ·   ·   ·  [A4] ·   ·   ·  [A4] ·   ·   ·  [A4] ·   ·   ·  [A4]  R0  ┐步驟4: x=3 (Col3,7,11,15)
   ·   ·   ·  [B4] ·   ·   ·  [B4] ·   ·   ·  [B4] ·   ·   ·  [B4]  R+Y ┘
  (x=0, y+=iRunRute → 下一個 Row 群)
  [A5]                        ...                                     R+2Y ┐步驟5
  [B5]                        ...                                     R+3Y ┘
  ... (全部 Row 完成 → y=0, iPlate--, 換盤)
```

> `[A1]` = A-Row 吸嘴群在步驟1放的位置，`[B1]` = B-Row 吸嘴群。  
> XItem16 是唯一使用 `iPlate--`（遞減）而非遞增的函式。  
> flag 索引: `flag[0][j]` = `Tray.Data[x + j*4][y]`, `flag[1][j]` = `Tray.Data[x + j*4][y+iYHalf]`。

### 進入條件

| 條件 | 說明 |
|------|------|
| XDiv==16 | 16-Col HP（32Site 專用）|

---

## §17 全函式適用矩陣總表

### 17.1 搜尋函式 × XDiv 對應

| 函式 | XDiv | 每次放料 | 掃描方式 | 特殊處理 |
|------|:----:|:-------:|---------|---------|
| `_1Suck` | **全部** | 1 | Col-first 逐格 | AutoClean Row 跳過 |
| `_1x2Suck` | 2,4,6,8 | 2 | spacX/Col=2 區塊 | bPitchOver12000 間隔放 |
| `_2x2Suck` | 2,4,6,8 | 2~4 | spacX/Col=2, Row=2 | b6x20/b4x11/b8x16 旗標 |
| `XItem3_2Suck_3Site` | **3** | 1~2 | Pitch 方向切換 | Ab 關站跳 Col1 |
| `XItem3_1x2Suck` | **3** | 2 | Col-first | 奇數YDiv保護 |
| `XItem3_2x2Suck` | **3** | 2~4 | Col-first, Row=2 | 奇數YDiv封鎖末格 |
| `XItem4_8Suck` | **4** | 4~8 | Col=4 區塊 | NN模式/bPitchOver |
| `XItem6_2x2_8Suck` | **6** | 4~8 | 三段: 0/1/2 | iForPlaceHPX6Step |
| `XItem6_3Suck_ACEx` | **6** | 3~6 | ix+=3, Col=3 | WideHP 3吸嘴 |
| `XItem6_8Suck` | **6** | 4~8 | 兩段: 0/4 | WideHP ACEG |
| `XItem6_NotStandY` | **6** | 2~4 | flag直檢, x=0/1/2 | PlaceSpecialPos |
| `XItem8_8Suck` | **8** | 4~8 | spacX=2, Col=4 | bPitchOver spacX=1 |
| `XItem10_8Suck` | **10** | 4~8 | 兩段: 0~4/8 | iForPlaceHPX10Step |
| `XItem12_2x6` | **12** | 6 | Kit交替 0/6/1/7 | Row13/14 特殊 |
| `XItem12_8Suck` | **12** | 4~8 | 兩段: 0~1/8 | iYHalf 跳行 |
| `XItem16_8Suck` | **16** | 8~16 | x=0~3, flag直檢 | iRunRute 跳行 |

### 17.2 生產模式 × XDiv × 搜尋函式

| 生產模式（iInArmType）| XDiv=2 | XDiv=3 | XDiv=4 | XDiv=6 | XDiv=8 | XDiv=10 | XDiv=12 | XDiv=16 |
|---|---|---|---|---|---|---|---|---|
| **ep1Picker / SingleSite** | _1S | _1S | _1S | _1S | _1S | _1S | _1S | _1S |
| **AxEx 1Row (1x2_13)** | 1x2 | 3_1x2 | 1x2 ᵖ | 1x2 ᵖ | 1x2 ᵖ | — | — | — |
| **AxxG 1Row (1x2_14)** | 1x2 | 3_1x2 | 1x2 ᵖ | 1x2 ᵖ | 1x2 ᵖ | — | — | — |
| **TriSite (1x3_14, iModeX≠3)** | — | 3_2S_3S | — | — | — | — | — | — |
| **TriSite (1x3_14, iModeX=3)** | — | 3_1x2 | — | — | — | — | — | — |
| **AxEx 2Row (2x2_13)** | 2x2 | 3_2x2 | 2x2 | 6_2x2 / 6_NS | 2x2 | — | — | — |
| **AxxG 2Row (2x2_14)** | 2x2 | 3_2x2 | 2x2 | 6_2x2 / 6_NS | 2x2 | — | — | — |
| **NN 1x4_4_13 (iShtRow=1)** | — | — | 4_8 | — | — | — | — | — |
| **ACEG 1x4_4** | 1x2 | 3_1x2 | 4_8 | 6_8 / 6_2x2 | 8_8 | — | — | — |
| **1x3_4 (WideHP)** | — | — | — | 6_3A | — | — | — | — |
| **2x3_6 (WideHP)** | — | — | — | 6_3A | — | — | — | — |
| **2x6_8 (WideHP)** | — | — | — | 6_3A | 8_8 | — | 12_2x6 | — |
| **ACEG 2x4_8** | 1x2 | 3_1x2 | 4_8 | 6_8 / 6_2x2 | 8_8 | 10_8 | 12_8 | — |
| **ACEG 2x5_8** | — | — | — | — | 8_8 | 10_8 | — | — |
| **ACEG 2x8_8** | — | — | 4_8 | — | 8_8 | 10_8 | 12_8 | 16_8 |
| **ACEG 2x8_32** | — | — | 4_8 | — | 8_8 | 10_8 | 12_8 | 16_8 |
| **2x2_8_Hot** | — | — | 4_8 | 6_8 / 6_2x2 | 8_8 | — | — | — |
| **1x4_8_Hot** | — | — | — | — | 8_8 | — | — | — |
| **2x4_16** | — | — | — | — | 8_8 | — | — | 16_8 |

**圖例說明**：
- `_1S` = `_1Suck()`
- `1x2` = `_1x2Suck()`，`ᵖ` = 可能觸發 `bPitchOver12000` 模式
- `2x2` = `_2x2Suck()`
- `3_1x2` = `XItem3_1x2Suck()`
- `3_2x2` = `XItem3_2x2Suck()`
- `3_2S_3S` = `XItem3_2Suck_3Site()`
- `4_8` = `XItem4_8Suck()`
- `6_2x2` = `XItem6_2x2_8Suck()`
- `6_3A` = `XItem6_3Suck_ACEx()`
- `6_8` = `XItem6_8Suck()`
- `6_NS` = `XItem6_8Suck_NotStandY()`
- `8_8` = `XItem8_8Suck()`
- `10_8` = `XItem10_8Suck()`
- `12_8` = `XItem12_8Suck()`
- `12_2x6` = `XItem12_2x6()`
- `16_8` = `XItem16_8Suck()`
- `—` = 不支援 / 不會進入該路徑

> **XDiv=1 HP 未列入**：僅 ep1Picker / SingleSite / 關站降級可進入 `_1Suck()`。  
> 其他模式搭配 XDiv=1 → **MES0156 錯誤**。

### 17.3 XDiv=6 子分派細節

```
XDiv==6
├─ WideHP (i8PickerHPMode==iHPWideHP)
│   ├─ 1x3_4 / 2x3_6 / 2x6_8       → XItem6_3Suck_ACEx
│   └─ else (1x4_4/2x4_8/2x8_8 等)  → XItem6_8Suck
├─ HotPlateYPitchCanPutAll()==false   → XItem6_8Suck_NotStandY
└─ else                               → XItem6_2x2_8Suck
```

### 17.4 XDiv=12 子分派細節

```
XDiv==12
├─ b12x16HP_2x6==true  → XItem12_2x6     (2x6 Kit 交替)
└─ else                 → XItem12_8Suck   (標準 ACEG)
```

### 17.5 XDiv=1 可達性矩陣

| 吸嘴模式 | XDiv=1 | 說明 |
|---------|:------:|------|
| ep1Picker / SingleSite / 1x4_1_Ac | ✅ _1Suck | 強制走 _1Suck() |
| DualSite 關站降級 | ✅ _1Suck | bDualSiteCloseAbCanFullHotplate |
| AxEx/AxxG 1Row/2Row | ❌ **MES0156** | XDiv=1 無對應分支 |
| 任何 ACEG 多吸嘴 | ❌ **MES0156** | XDiv=1 無對應分支 |

---

## §18 關鍵變數速查

| 變數 | 定義位置 | 說明 |
|------|---------|------|
| `iYHalf` | `GetHotPlateYHalfPos()` | 吸嘴 Y 方向跨行數（ARM_HP_Y_PITCH / HP_YPitch） |
| `bPitchOver12000` | `_1x2Suck/_2x2Suck` 內部 | XPitch > iXpitchMaxX2(AxEx) 或 iXpitchMaxX3(AxxG) → true |
| `iForPlaceHPX3Step` | 各 XItem3 函式 | 0=正常放, 1=Col2 單放模式 |
| `iForPlaceHPX6Step` | 各 XItem6 函式 | 0=左側 Col0-3, 1=右側 Col4-5（或反序） |
| `iForPlaceHPX10Step` | `XItem10_8Suck` | 0=正常, 2=偏移放（左半無IC右半有） |
| `b6x20HP` | `GetHotPlateYHalfPos()` | XDiv=6, YDiv=20, YPitch=12.7mm |
| `b4x11HP` | 同上 | XDiv=4, YDiv 奇數, iPickRow=2, iPickCol≥2 |
| `b4x10HP_2x2` | 同上 | XDiv=4, YPitch 非整除 ARM_HP_Y_PITCH |
| `b8x16HP_2x2` | 同上 | QualSite2X2 + XDiv=8, YDiv=16, YPitch=20mm |
| `b12x16HP_2x6` | 同上 | 2x6 模式 + XDiv=12, YDiv=16, YPitch 條件 |
| `iHPWideHP` | `i8PickerHPMode` | WideHP 寬版加熱盤標記 |
| `iCloseSiteState` | `CloseSiteState()` | 0=無關站, 1=關A排, 2=關B排 |
| `iPlaceHPOrder` | 全域 | 0=Row1→Row2順序, 1=反序 |
| `iXpitchMaxX2` | 全域常數 | AxEx XPitch 最大值（約 120mm×100） |
| `iXpitchMaxX3` | 全域常數 | AxxG XPitch 最大值（約 180mm×100） |

---

## §19 HP 配置規格表

> 原 `HP_PlateForm.md` 內容，已整合至本文件。

此表格涵蓋 HT9045 支援的加熱盤（Hot Plate）配置規格，
基於 `ainarm_SearchPlacePlate.cpp` 的 `SearchPlateToPlace()` 分派邏輯，
標示各配置實際支援的 Test Mode（iInArmType）。

### 19.1 欄位說明

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

### 19.2 配置表

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

### 19.3 支援 Picker 類型的 XDivision 範圍

根據 `bUseAxExPicker()`、`bUseAxxGPicker()`、`bUseACEGPicker()` 定義：

| iInArmType | Picker 分類 | 支援 XDivision | 搜尋路徑 |
|-----------|-----------|--------------|----------|
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

---

## 相關文件

| 文件 | 內容 |
|------|------|
| [HP_Knowledgebase.md](HP_Knowledgebase.md) | HotPlate 完整知識庫、SearchPlateToPlace dispatch 詳解 |
| [../SKILL.md](../SKILL.md) | InArm Flow SKILL 總目錄 |
