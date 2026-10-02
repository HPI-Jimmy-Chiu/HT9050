# HotPlate Variable Pitch 完整參考表

> 歸納自 `ainarm_SearchPlacePlate.cpp`（V3.33.904.2）  
> 用於快速判斷特定 XDiv × XPitch 組合下的實際行為：  
> VariablePitch（馬達跨距）、bPitchOver12000、Search spacX、Place ix 公式  
> **修改 HP 搜尋/放料前務必查表確認**

---

## 1. GetVariableInHotPlateData — 三函式 Pitch 公式對照

### 閾值常數

| 常數 | 預設值 | 說明 |
|------|--------|------|
| `iXpitchMax` | 4000 (40mm) | bPitchOver12000 判定閾值（AxEx 用） |
| `iXpitchMaxX3` | 12000 (120mm) | VariablePitch 上限（= iXpitchMax × 3） |
| `iXpitchMaxX3+300` | 12300 | AxxG / ACEG 的 bPitchOver12000 判定閾值 |

> 注意：`iXpitchMaxX3` 可被 `database.cpp` 依機型設定為 15000 或 6600 等值。

---

### 1.1 GetVariableInHotPlateData_AxEx（bUseAxExPicker = true）

**公式邏輯**：A-E 間距 = XDiv/2 個 Pitch → VariablePitch = XPitch × (XDiv/2) × 1.5

| XDiv | 公式 | 範例 (XP=1700) | bPitchOver12000? | 備註 |
|------|------|----------------|-------------------|------|
| 2,3 | XP × 1.5 | 2550 | false | |
| 4 | XP × 3.0 | 5100 | if > 12000: XP×1.5, true | XP > 4000 觸發 |
| 6 | XP × 4.5 | 7650 | if > 12000: 見下方 | 按 col 分段 |
| 6 (Over) | col≠4: XP×3.0, col==4: XP×1.5 | — | true | |
| 8 | XP × 6.0 | 10200 | if > 12000: XP×3.0, true | |
| 10 | — | — | — | ⚠ 未實作 |
| **12** | **XP × 9.0** | **15300** | **if > 12000: col8/9: XP×3.0, else XP×6.0, true** | **已修正：V3.33.904.2 起 HP XItem=12 且 XP×6>iXpitchMaxX2 自動切 _14(AxxG)，不再走此路徑** |
| 16 | — | — | — | ⚠ 未實作 |

> **⚠ XDiv=12 + XP=1700 陷阱**：15300 > 12000 → bPitchOver12000=true，搜尋路徑退化為只掃 4 個位置。

---

### 1.2 GetVariableInHotPlateData_AxxG（bUseAxxGPicker = true）

**公式邏輯**：A-G 間距 = XDiv 個 Pitch × (2/3) → VariablePitch = XPitch × XDiv × (2/3)

| XDiv | 公式 | 範例 (XP=1700) | bPitchOver12000? | 備註 |
|------|------|----------------|-------------------|------|
| 2,3 | XP × 1.0 | 1700 | false | 1x3 3x7HP 特殊邏輯 |
| 4 | XP × 2.0 | 3400 | if > 12000: XP×1.0, true | |
| 6 | XP × 3.0 | 5100 | false | 不判 over |
| 8 | XP × 4.0 | 6800 | if > 12300: XP×2.0, true | |
| 10 | XP × 5.0 | 8500 | if > 12300: XP×2.0, true | |
| **12** | **XP × 6.0** | **10200** | **if > 12300: XP×3.0, true** | **10200 < 12300 → false ✓（V3.33.904.2 修正：HP 12×N 自動走此路徑）** |
| 16 | XP × 4.0 | 6800 | if > 12300: XP×2.0, true | |

> **XDiv=12 + XP=1700 在 AxxG 下安全**：10200 < 12300 → bPitchOver12000=false。
> V3.33.904.2 起，HP XItem=12 且 XPitch×6>iXpitchMaxX2(8000) 時 iInArmType 自動切為 _14(AxxG)，
> 避免走 AxEx 路徑造成 VP=15300 → bPitchOver12000=true → WAR0150。

---

### 1.3 GetVariableInHotPlateData_ACEG（bUseACEGPicker = true）

**公式邏輯**：A-C-E-G 各間距 = XDiv/4 × Pitch → VariablePitch = XPitch × (XDiv/4) × 3

| XDiv | 公式 | 範例 (XP=1700) | bPitchOver12000? | 備註 |
|------|------|----------------|-------------------|------|
| 2,3 | XP × 1.0 | 1700 | false | ⚠ 未完成 |
| 4 | XP × 3.0 | 5100 | if > 12300: XP×1.5, true | |
| 6 (Wide) | XP × 3.0 | 5100 | false | 1x3/2x3/2x6 特殊 |
| 6 (Normal) | XP × 4.5 | 7650 | false | |
| 8 | XP × 6.0 | 10200 | if > 12300: XP×3.0, true | |
| 10 | XP × 3.0 | 5100 | false | |
| 12 | XP × 6.0 | 10200 | if > 12300: XP×3.0, true | col==8: 強制 true+XP×3 |
| 12 (2x6) | XP × 6.0 | 10200 | false | b12x16HP_2x6 特殊 |
| 16 | XP × 12.0 | 20400 | if > 12300: XP×3.0, true | |

---

## 2. GetPlaceToHotPlateSuckCol — 吸嘴→陣列索引對應

| 吸嘴模式 | 函式路徑 | j=0 | j=1 | 說明 |
|----------|---------|-----|-----|------|
| 1x1 | 直接 | 0 (或 1) | — | bSingleUseOtherSuck |
| AxEx (1-3) | `bUseAxExPicker()` | 0 (=A) | 2 (=E) | `j2 = j*2` |
| AxxG (1-4) | `bUseAxxGPicker()` | 0 (=A) | 3 (=G) | `j2 = j*3` |
| ACEG (全4) | else | j (=A,C,E,G) | — | `j2 = j` (XDiv=6/10 有特殊) |

---

## 3. GetPlaceToHotPlateCol — HP 格位→col 索引對應

### 3.1 AxEx 模式（bUseAxExPicker = true）

| XDiv | bPitchOver12000 | ix 公式 | 範例 (base=0, j=0/1) |
|------|-----------------|---------|---------------------|
| 3 (特殊) | — | 特殊邏輯 | 見 1x2_13 + 3x6HP |
| 4 | true | base + j | (0, 1) |
| 6 | true, col≠4 | base + j*2 | (0, 2) |
| 6 | true, col==4 | base + j | (4, 5) |
| 8 | true | base + j*(XDiv/4) = j*2 | (0, 2) |
| **通用** | **false** | **base + j*(XDiv/2)** | **(0, 6) for XDiv=12** |

> ⚠ XDiv=12 無 bPitchOver12000 分支 → 走通用 `base + j*6` → ix = (0, 6)

### 3.2 AxxG 模式（bUseAxxGPicker = true）

| XDiv | bPitchOver12000 | ix 公式 | 範例 (base=0, j=0/1) |
|------|-----------------|---------|---------------------|
| 3 (特殊) | — | 特殊邏輯 | 見 1x2_14 / 1x3_14 |
| 4 | true | base + j | (0, 1) |
| 6 | true, col≠4 | base + j*2 | (0, 2) |
| 6 | true, col==4 | base + j | (4, 5) |
| 8 | true | base + j*(XDiv/4) = j*2 | (0, 2) |
| 3 (1x3) | — | 特殊 iPlacePlateX 邏輯 | — |
| **通用** | **false** | **base + j*(XDiv/2)** | **(0, 6) for XDiv=12** |

### 3.3 ACEG 模式（else, 4-picker）

| XDiv | bPitchOver12000 | ix 公式 | 範例 (base=0, j=0~3) |
|------|-----------------|---------|---------------------|
| 4 (32Site Over) | true | base | 不移動 |
| 4 | false | base + j | (0,1,2,3) |
| 6 (Wide) | — | base + j | (0,1,2,3) |
| 6 (Normal) | — | base + j*3 | (0,3) — 只 j=0,1 |
| 8 | true | base + j | (0,1,2,3) |
| 8 | false | base + j*2 | (0,2,4,6) |
| 10 (col≠8) | — | base + j*2 | (0,2,4,6) |
| 10 (col==8) | — | base + j | (8,9) |
| 12 (2x6) | — | base + j*2 (特殊) | 見程式碼 |
| 12 | true | base + j | (0,1,2,3) |
| 12 | false | base + j*2 | (0,2,4,6) |
| 16 | — | base + j*4 | (0,4,8,12) |

---

## 4. SearchPlacePlateXItem_2x2Suck — spacX / spacY 計算

### 4.1 正常路徑（HotPlateYPitchCanPutAll = true, iPickRow=2）

| 條件 | spacX | spacY | Col (吸嘴列數) | Row | 備註 |
|------|-------|-------|------|-----|------|
| bPitchOver12000 + XDiv=4 + AxEx | 4 | iYHalf | 1 | 2 | |
| bPitchOver12000 + XDiv=8 + AxEx | — | iYHalf | 2 | 2 | |
| bPitchOver12000 + 其他 | 2+特殊 | iYHalf | 2 | 2 | ix==2 時 spacX=5 |
| **非 PitchOver（通用）** | **XDiv/2** | **iYHalf** | **2** | **2** | **最常見路徑** |

> **XDiv=12 通用路徑**：spacX=6, Col=2 → 搜尋 (ix, ix+6)，共掃 6 對 = 12 cols ✓

### 4.2 bSpecialPlace 路徑（HotPlateYPitchCanPutAll = false 或 iPickRow=1）

spacX 計算**完全相同**（照搬正常路徑），差異只在 spacY 和 Row：

| 條件 | spacY | Row | 備註 |
|------|-------|-----|------|
| b6x20HP / b4x11HP | iYHalf | 2 | 特殊 HP 保持 spacY |
| 其他 | **1** | 2 | ⚠ spacY 退化為 1 |

### 4.3 迭代邏輯（搜尋 next position）

```
ix++
if(ix >= spacX)     // 正常：ix 掃完 spacX 個位置
    ix = 0
    iy++ (或 iy+=2 for 4x10)
    if(spacY>1 && iy%spacY==0) iy += spacY   // 跳過已搜尋的 Y 區域
    if(iy >= YDiv) → 切換 plate / 重置
```

> 注意：bPitchOver12000=true 時迭代邏輯走**獨立分支**（非通用 ix++），有 ix=0→1→4→5 的硬編碼路徑。

---

## 5. CheckHotPlateHasSpace_9045_8_New_V — 空格檢查

```
for numR = 0 to iSuckRow-1:
    for numC = 0 to iSuckCol-1:
        check Tray.Data[iCol + spacX*numC][iRow + spacY*numR]
```

- **iSuckRow** = `Row`（1 或 2）
- **iSuckCol** = `Col`（1 或 2）
- 放置（state=0）：所有格位必須 == NULL_IC
- 吸取（state=1）：任一格位 != NULL_IC → 有料

> spacX 和 spacY **同時**影響搜尋和此檢查，兩者必須一致。

---

## 6. 常見 XDiv × XPitch 組合速查表

以下列出實際機台常用的 HP 規格，標注各吸嘴模式下的行為：

| XDiv | 典型 XPitch (mm) | AxEx Pitch | Over? | AxxG Pitch | Over? | ACEG Pitch | Over? |
|------|-------------------|------------|-------|------------|-------|------------|-------|
| 4 | 45.0 (4500) | 13500 | ✓ | 9000 | ✗ | 13500 | ✓ |
| 6 | 17.0 (1700) | 7650 | ✗ | 5100 | ✗ | 7650 | ✗ |
| 6 | 36.6 (3660) | 16470 | ✓ | 10980 | ✗ | 16470 | ✓ |
| 8 | 17.0 (1700) | 10200 | ✗ | 6800 | ✗ | 10200 | ✗ |
| 8 | 25.0 (2500) | 15000 | ✓ | 10000 | ✗ | 15000 | ✓ |
| 10 | 17.0 (1700) | ⚠ N/A | — | 8500 | ✗ | 5100 | ✗ |
| **12** | **17.0 (1700)** | **15300** | **✓** | **10200** | **✗** | **10200** | **✗** |
| 12 | 26.66 (2666) | 23994 | ✓ | 15996 | ✓ | 15996 | ✓ |
| 16 | 8.0 (800) | ⚠ N/A | — | 3200 | ✗ | 9600 | ✗ |

> **關鍵發現**：XDiv=12 + XP=17mm 在 AxEx 模式下 bPitchOver12000=true（退化搜尋），在 AxxG 模式下 false（正常搜尋）。

---

## 7. 交叉驗證清單（修改 HP 邏輯時必查）

修改任何 HP 搜尋/放料函式前，逐項確認：

1. [ ] 目標機台的 **iInArmType** 是什麼？（不要假設）
2. [ ] 該 iInArmType 走哪個吸嘴函式？（AxEx / AxxG / ACEG）
3. [ ] 該 XDiv + XPitch 下 **bPitchOver12000** 是否為 true？
4. [ ] 搜尋 spacX 值是多少？迭代能掃到所有 col 嗎？
5. [ ] 放料 `GetPlaceToHotPlateCol(j)` 的 ix 值與搜尋的 spacX 一致嗎？
6. [ ] 該 iInArmType 的 `DoPlaceToHotPlate` 是哪個函式？（不同 type 在不同 .cpp）
7. [ ] `GetVariableInHotPlateData` 回傳的 pitch 值 ≤ iXpitchMaxX3？（馬達實際能到？）
8. [ ] `HotPlateYPitchCanPutAll()` = true 或 false？對 spacY 的影響？
