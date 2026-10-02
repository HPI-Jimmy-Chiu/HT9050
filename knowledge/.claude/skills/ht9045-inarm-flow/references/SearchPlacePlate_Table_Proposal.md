# SearchPlacePlate 查表法改寫方案

> **版本**: v1.0 Draft  
> **日期**: 2026-04-07  
> **配套文件**: [SearchPlacePlate_Rewrite_Proposal.md](SearchPlacePlate_Rewrite_Proposal.md) / [SearchPlacePlate_Route_Proposal.md](SearchPlacePlate_Route_Proposal.md)  
> **約束**: BCB6 C++98、Big5 編碼、不可用 C++11 語法  
> **核心思路**: 用一張靜態查表取代 16 個 XItem 函式，放置完成後路徑存入 `PickFromHPList`（FIFO），吸取時直接依序讀取。

---

## 目錄

- [§0 現狀對照](#0-現狀對照)
- [§1 設計原則](#1-設計原則)
- [§2 資料結構](#2-資料結構)
- [§3 完整查表 — PLACE_TABLE](#3-完整查表--place_table)
- [§4 統一放置引擎](#4-統一放置引擎)
- [§5 StepHint 統一計算](#5-stephint-統一計算)
- [§6 PickFromHPList 整合](#6-pickfromhplist-整合)
- [§7 Edge Case 統一處理](#7-edge-case-統一處理)
- [§8 查表索引函式](#8-查表索引函式)
- [§9 檔案結構與行數預估](#9-檔案結構與行數預估)
- [§10 相容性與風險](#10-相容性與風險)
- [§11 驗證方案](#11-驗證方案)

---

## §0 現狀對照

### 0.1 舊版: 16 個硬編碼函式

```
SearchPlateToPlace()                        // 4700+ 行
├── _1Suck()             → 逐格
├── _1x2Suck()           → AxEx/AxxG 2吸嘴 (XDiv=2/4/6/8)
├── _2x2Suck()           → AxEx/AxxG 4吸嘴 2Row (XDiv=2/4/6/8)
├── XItem3_2Suck_3Site() → TriSite 特殊掃描
├── XItem3_1x2Suck()     → 3-Col HP 雙吸嘴
├── XItem3_2x2Suck()     → 3-Col HP 四吸嘴
├── XItem4_8Suck()       → 4-Col ACEG
├── XItem6_2x2_8Suck()   → 6-Col 標準 AxEx/ACEG
├── XItem6_3Suck_ACEx()  → 6-Col WideHP 3吸嘴
├── XItem6_8Suck()       → 6-Col WideHP ACEG
├── XItem6_8Suck_NotStandY() → 6-Col 非標準Y
├── XItem8_8Suck()       → 8-Col ACEG ★主力
├── XItem10_8Suck()      → 10-Col 滑動窗口
├── XItem12_2x6()        → 12-Col Kit交錯
├── XItem12_8Suck()      → 12-Col ACEG 兩段
└── XItem16_8Suck()      → 16-Col 32Site
```

### 0.2 新版: 1 張表 + 1 個引擎

```
SearchPlateToPlace_New()
│
├── GetHotPlateYHalfPos()        // 不變
├── LookupPlaceEntry()           // ★ 查表: (XDiv, iInArmType, flags) → TPlaceEntry*
├── ExecutePlaceEntry()          // ★ 統一引擎: 讀表 → 掃描 → 設全域變數
├── ComputeStepHint()            // ★ 統一偏移: 取代 iForPlaceHPXnStep
└── PickFromHPList->Update/Add() // 記錄路徑供 Pick 使用
```

---

## §1 設計原則

| # | 原則 | 說明 |
|---|------|------|
| 1 | **一行一組合** | PLACE_TABLE 中每個 (XDiv, armCategory, condition) 佔一行，無抽象層 |
| 2 | **全走 CheckHasSpace** | 包括 XItem16（舊版手寫陣列）、NotStandY（舊版 flag 檢查）全統一 |
| 3 | **Zero new globals** | 沿用 iPlacePlateX/Y/Plate, iForPlaceHPXnStep，不新增全域 |
| 4 | **PickFromHPList 記錄路徑** | Place 完成後存 FIFO，Pick 時直接讀取，不需實作 Pick 搜尋 |
| 5 | **允許掃描順序微調** | 填滿率不變，但 ix 掃描順序可能與舊版不同 |
| 6 | **BuildScanPlan 不需要** | 表已是最終產物；edge case 在表中用不同行區分 |

---

## §2 資料結構

### 2.1 TPlaceSegment — 掃描段

```cpp
struct TPlaceSegment
{
    int ixStart;      // 起始列 (inclusive)
    int ixEnd;        // 結束列 (exclusive)
    int ixStep;       // 列步進 (一般=1, WideHP 3吸嘴=3)
    int spacX;        // CheckHasSpace 的 X spacing
    int nSuckCol;     // 每次放置佔幾欄 (1/2/3/4)
};
```

### 2.2 TPlaceEntry — 完整放置規則（表的一行）

```cpp
struct TPlaceEntry
{
    // === 查表鍵 ===
    int  xDiv;            // HotPlateForm.XDivision
    int  armCategory;     // eArmCategory enum (見 §8.1)
    int  condition;       // eCondition enum (見 §8.2)

    // === 掃描段 ===
    int  nSegments;       // 段數 (1~3)
    TPlaceSegment seg[3]; // 最多 3 段

    // === Row 參數 ===
    int  nSuckRow;        // 每次放置佔幾列 (1 or 2)
    int  spacYMode;       // 0=固定1, 1=iYHalf(動態), 2=固定5(b6x20HP)

    // === 控制旗標 ===
    bool bRowFallback;    // true: Row2 失敗時降級 Row1
    bool bPlateReverse;   // true: iPlate 1→0 (XDiv16 反向)
    bool bColFirst;       // true: col-first 掃描 (XDiv3 系列)
    bool bKitInterleave;  // true: 使用 ixSequence 自定義順序

    // === Kit 交錯 ===
    int  ixSequence[8];   // Kit 模式的 ix 序列, -1 結尾

    // === AutoClean ===
    int  autoCleanSkipRows; // HP0 跳過的起始列數 (0 or 4)

    // === StepHint 規則 ===
    int  stepHintRule;    // 0=無, 1=X3規則, 2=X6規則, 3=X10規則
};
```

### 2.3 TStepHint — 放置偏移

```cpp
struct TStepHint
{
    int iXOffset;      // X 方向偏移吸嘴數
    int iActiveCols;   // 實際可用的吸嘴欄數
};
```

---

## §3 完整查表 — PLACE_TABLE

### 3.1 armCategory 分類

| 值 | 名稱 | 對應 iInArmType | Picker 數 |
|:--:|------|----------------|:---------:|
| 0 | `AC_1SUCK` | ep1Picker / SingleSite / 1x4_1_Ac / Ab關站降級 | 1 |
| 1 | `AC_AX1R` | AxEx/AxxG 1Row (1x2_2_13/14) | 2 |
| 2 | `AC_AX2R` | AxEx/AxxG 2Row (2x2_4_13/14) | 4 |
| 3 | `AC_TRI` | TriSite1X3 (iModeX≠3) | 2 |
| 4 | `AC_ACEG_1R` | ACEG 1Row (1x4_4) | 4 |
| 5 | `AC_ACEG_2R` | ACEG 2Row (2x4_8 / 2x5_8 / 2x8_8 等) | 8 |
| 6 | `AC_ACEG_32` | ACEG 2x8_32 / 2x4_16 | 16/32 |
| 7 | `AC_NN14` | NN 1x4_4_13 (iShtRow=1) | 4 |
| 8 | `AC_H28` | 2x2_8_Hot | 8 |
| 9 | `AC_H14` | 1x4_8_Hot | 4 |
| 10 | `AC_W13` | WideHP 1x3_4 | 3 |
| 11 | `AC_W23` | WideHP 2x3_6 | 6 |
| 12 | `AC_W26` | WideHP 2x6_8 | 6 |
| 13 | `AC_KIT26` | 12×16 Kit 2x6 (b12x16HP_2x6) | 6 |

### 3.2 condition 分類

| 值 | 名稱 | 觸發條件 |
|:--:|------|---------|
| 0 | `COND_NORMAL` | 預設 |
| 1 | `COND_PITCH_OVER` | bPitchOver12000 == true |
| 2 | `COND_NOT_STAND_Y` | !HotPlateYPitchCanPutAll() && !WideHP |
| 3 | `COND_WIDE_HP` | i8PickerHPMode == iHPWideHP |
| 4 | `COND_4x11HP` | b4x11HP == true |
| 5 | `COND_4x10HP` | b4x10HP_2x2 == true |
| 6 | `COND_8x16_2x2` | b8x16HP_2x2 == true |

### 3.3 完整查表 — 50 條規則

下表涵蓋 **§19 全部 HP 配置 × 全部 iInArmType** 的所有有效組合。

**欄位說明**:
- `ID`: 表索引
- `XD`: XDivision
- `Cat`: armCategory
- `Cond`: condition
- `Seg`: 段數
- `S0 / S1`: 掃描段參數 `(ixStart→ixEnd, step, spacX, Col)`
- `Row`: nSuckRow
- `spY`: spacYMode (0=1, 1=iYHalf, 2=5)
- `FB`: bRowFallback
- `Rev`: bPlateReverse
- `CF`: bColFirst
- `Kit`: bKitInterleave
- `ACskip`: autoCleanSkipRows
- `SH`: stepHintRule

---

#### XDiv=2 (4 條)

| ID | XD | Cat | Cond | Seg | S0 | S1 | Row | spY | FB | Rev | CF | Kit | ACskip | SH |
|:--:|:--:|:---:|:----:|:---:|:---|:---|:---:|:---:|:--:|:---:|:--:|:---:|:------:|:--:|
| 1 | 2 | AC_1SUCK | NORMAL | 1 | 0→2, +1, spX=1, Col=1 | — | 1 | 0 | N | N | N | N | 4 | 0 |
| 2 | 2 | AC_AX1R | NORMAL | 1 | 0→1, +1, spX=1, Col=2 | — | 1 | 0 | N | N | N | N | 4 | 0 |
| 3 | 2 | AC_AX2R | NORMAL | 1 | 0→1, +1, spX=1, Col=2 | — | 2 | 1 | Y | N | N | N | 4 | 0 |
| 4 | 2 | AC_ACEG_1R | NORMAL | 1 | 0→1, +1, spX=1, Col=2 | — | 1 | 0 | N | N | N | N | 4 | 0 |

> ACEG 在 XDiv=2 降級為 2 吸嘴（只用 Aa+Ac），等同 AX1R 路徑。

---

#### XDiv=3 (5 條)

| ID | XD | Cat | Cond | Seg | S0 | S1 | Row | spY | FB | Rev | CF | Kit | ACskip | SH |
|:--:|:--:|:---:|:----:|:---:|:---|:---|:---:|:---:|:--:|:---:|:--:|:---:|:------:|:--:|
| 5 | 3 | AC_1SUCK | NORMAL | 1 | 0→3, +1, spX=1, Col=1 | — | 1 | 0 | N | N | N | N | 4 | 0 |
| 6 | 3 | AC_TRI | NORMAL | 1 | 0→3, +1, spX=1, Col=1 | — | 1 | 0 | N | N | ※ | N | 0 | 1 |
| 7 | 3 | AC_AX1R | NORMAL | 2 | 0→2, +1, spX=1, Col=2 | 2→3, +1, spX=1, Col=1 | 1 | 0 | N | N | Y | N | 4 | 1 |
| 8 | 3 | AC_AX2R | NORMAL | 2 | 0→2, +1, spX=1, Col=2 | 2→3, +1, spX=1, Col=1 | 2 | 1 | Y | N | Y | N | 4 | 1 |
| 9 | 3 | AC_ACEG_1R | NORMAL | 2 | 0→2, +1, spX=1, Col=2 | 2→3, +1, spX=1, Col=1 | 1 | 0 | N | N | Y | N | 4 | 1 |

> **ID6 (Tri)**: ※ CF 由 XPitch 動態決定 — 窄Pitch 走 Row-first，寬Pitch 走 Col-first。XItem3_2Suck_3Site 的 Pitch 切換邏輯需保留為特殊處理。  
> **ID7-9**: S0 放 Col0+Col1（2吸嘴），S1 放 Col2（單吸嘴，iForPlaceHPX3Step=1）。

---

#### XDiv=4 (9 條)

| ID | XD | Cat | Cond | Seg | S0 | S1 | Row | spY | FB | Rev | CF | Kit | ACskip | SH |
|:--:|:--:|:---:|:----:|:---:|:---|:---|:---:|:---:|:--:|:---:|:--:|:---:|:------:|:--:|
| 10 | 4 | AC_1SUCK | NORMAL | 1 | 0→4, +1, spX=1, Col=1 | — | 1 | 0 | N | N | N | N | 4 | 0 |
| 11 | 4 | AC_AX1R | NORMAL | 1 | 0→2, +1, spX=2, Col=2 | — | 1 | 0 | N | N | N | N | 4 | 0 |
| 12 | 4 | AC_AX1R | PITCH_OVER | 1 | 0→4, +1, spX=1, Col=1 | — | 1 | 0 | N | N | N | N | 4 | 0 |
| 13 | 4 | AC_AX2R | NORMAL | 1 | 0→2, +1, spX=2, Col=2 | — | 2 | 1 | Y | N | N | N | 4 | 0 |
| 14 | 4 | AC_AX2R | 4x11HP | 1 | 0→2, +2, spX=2, Col=2 | — | 1 | 0 | N | N | N | N | 4 | 0 |
| 15 | 4 | AC_AX2R | 4x10HP | 1 | 0→2, +1, spX=2, Col=2 | — | 2 | 0 | Y | N | N | N | 4 | 0 |
| 16 | 4 | AC_ACEG_1R | NORMAL | 1 | 0→1, +1, spX=1, Col=4 | — | 1 | 1 | N | N | N | N | 4 | 0 |
| 17 | 4 | AC_ACEG_2R | NORMAL | 1 | 0→1, +1, spX=1, Col=4 | — | 2 | 1 | Y | N | N | N | 4 | 0 |
| 18 | 4 | AC_NN14 | NORMAL | 1 | 0→1, +1, spX=1, Col=4 | — | 1 | 0 | N | N | N | N | 4 | 0 |

> **ID14 (b4x11HP)**: 奇數 YDiv + 2Row → 強制 Row=1, ix step=2。  
> **ID15 (b4x10HP_2x2)**: YPitch 非整除 → spacY=1(固定), iYHalf=1。  
> **ID16-17**: ACEG 4 吸嘴在 XDiv=4 直接填滿整行（ix 只有 0）。  
> **ID18 (NN14)**: spacYMode=0 表示固定 spacY=1（非 iYHalf）。

---

#### XDiv=6 標準 HP (8 條)

| ID | XD | Cat | Cond | Seg | S0 | S1 | Row | spY | FB | Rev | CF | Kit | ACskip | SH |
|:--:|:--:|:---:|:----:|:---:|:---|:---|:---:|:---:|:--:|:---:|:--:|:---:|:------:|:--:|
| 19 | 6 | AC_1SUCK | NORMAL | 1 | 0→6, +1, spX=1, Col=1 | — | 1 | 0 | N | N | N | N | 4 | 0 |
| 20 | 6 | AC_AX1R | NORMAL | 1 | 0→3, +1, spX=3, Col=2 | — | 1 | 0 | N | N | N | N | 4 | 2 |
| 21 | 6 | AC_AX1R | PITCH_OVER | 1 | 0→6, +1, spX=1, Col=1 | — | 1 | 0 | N | N | N | N | 4 | 0 |
| 22 | 6 | AC_AX2R | NORMAL | 1 | 0→3, +1, spX=3, Col=2 | — | 2 | 1 | Y | N | N | N | 4 | 2 |
| 23 | 6 | AC_AX2R | NOT_STAND_Y | 1 | 0→3, +1, spX=3, Col=2 | — | 1 | 0 | N | N | N | N | 4 | 2 |
| 24 | 6 | AC_AX2R | PITCH_OVER | 1 | 0→6, +1, spX=1, Col=1 | — | 1 | 0 | N | N | N | N | 4 | 0 |
| 25 | 6 | AC_ACEG_1R | NORMAL | 1 | 0→3, +1, spX=3, Col=2 | — | 1 | 0 | N | N | N | N | 4 | 2 |
| 26 | 6 | AC_ACEG_2R | NORMAL | 1 | 0→3, +1, spX=3, Col=2 | — | 2 | 1 | Y | N | N | N | 4 | 2 |

> **ID23 (NotStandY)**: 舊版使用 flag[2][2] 直接檢查 Tray.Data，新版改用 CheckHasSpace + Row=1。  
> **ID25-26 (ACEG 降級)**: 在標準 6-Col HP，ACEG spacX=3 只容 2 吸嘴（Aa+Ac），其餘 Ae/Ag 不放。

---

#### XDiv=6 WideHP (4 條)

| ID | XD | Cat | Cond | Seg | S0 | S1 | Row | spY | FB | Rev | CF | Kit | ACskip | SH |
|:--:|:--:|:---:|:----:|:---:|:---|:---|:---:|:---:|:--:|:---:|:--:|:---:|:------:|:--:|
| 27 | 6 | AC_W13 | WIDE_HP | 1 | 0→6, +3, spX=1, Col=3 | — | 1 | 1 | N | N | N | N | 4 | 0 |
| 28 | 6 | AC_W23 | WIDE_HP | 1 | 0→6, +3, spX=1, Col=3 | — | 2 | 1 | Y | N | N | N | 4 | 0 |
| 29 | 6 | AC_W26 | WIDE_HP | 1 | 0→6, +3, spX=1, Col=3 | — | 2 | 1 | Y | N | N | N | 4 | 0 |
| 30 | 6 | AC_ACEG_2R | WIDE_HP | 2 | 0→1, +1, spX=1, Col=4 | 4→6, +1, spX=1, Col=2 | 2 | 1 | Y | N | N | N | 4 | 2 |

> **ID27-29**: WideHP 3 吸嘴，ix 步進=3（左半 C0-2 → 右半 C3-5）。  
> **ID30**: WideHP ACEG 兩段式 — S0(4吸嘴 C0-3) + S1(2吸嘴 C4-5)。

---

#### XDiv=8 (8 條)

| ID | XD | Cat | Cond | Seg | S0 | S1 | Row | spY | FB | Rev | CF | Kit | ACskip | SH |
|:--:|:--:|:---:|:----:|:---:|:---|:---|:---:|:---:|:--:|:---:|:--:|:---:|:------:|:--:|
| 31 | 8 | AC_1SUCK | NORMAL | 1 | 0→8, +1, spX=1, Col=1 | — | 1 | 0 | N | N | N | N | 4 | 0 |
| 32 | 8 | AC_AX1R | NORMAL | 1 | 0→4, +1, spX=4, Col=2 | — | 1 | 0 | N | N | N | N | 4 | 0 |
| 33 | 8 | AC_AX1R | PITCH_OVER | 2 | 0→2, +1, spX=2, Col=2 | 4→6, +1, spX=2, Col=2 | 1 | 0 | N | N | N | N | 4 | 0 |
| 34 | 8 | AC_AX2R | NORMAL | 1 | 0→4, +1, spX=4, Col=2 | — | 2 | 1 | Y | N | N | N | 4 | 0 |
| 35 | 8 | AC_AX2R | 8x16_2x2 | 1 | 0→4, +1, spX=4, Col=2 | — | 2 | 0 | Y | N | N | N | 4 | 0 |
| 36 | 8 | AC_ACEG_1R | NORMAL | 1 | 0→2, +1, spX=2, Col=4 | — | 1 | 1 | N | N | N | N | 4 | 0 |
| 37 | 8 | AC_ACEG_2R | NORMAL | 1 | 0→2, +1, spX=2, Col=4 | — | 2 | 1 | Y | N | N | N | 4 | 0 |
| 38 | 8 | AC_ACEG_2R | PITCH_OVER | 2 | 0→1, +1, spX=1, Col=4 | 4→5, +1, spX=1, Col=4 | 2 | 1 | Y | N | N | N | 4 | 0 |

> **ID37**: ★主力路徑 — spacX=2, Col=4, ix 0→1 兩步填滿 8-Col。  
> **ID33 (AX1R PitchOver)**: XDiv=8 AxEx PitchOver 舊版 ix 跳 0→1→4→5，新版改兩段式。  
> **ID35 (b8x16HP_2x2)**: QualSite2X2 特殊 — spacY=1(固定), iYHalf=1。  
> **ID36-37 覆蓋所有 ACEG 子類**: 1x4, 2x4/2x5/2x6/2x8/32S, H28, H14, 2416 在 XDiv=8 HP 上掃描路徑完全相同。

---

#### XDiv=10 (2 條)

| ID | XD | Cat | Cond | Seg | S0 | S1 | Row | spY | FB | Rev | CF | Kit | ACskip | SH |
|:--:|:--:|:---:|:----:|:---:|:---|:---|:---:|:---:|:--:|:---:|:--:|:---:|:------:|:--:|
| 39 | 10 | AC_ACEG_1R | NORMAL | 2 | 0→5, +1, spX=1, Col=4 | 8→10, +1, spX=1, Col=2 | 1 | 1 | N | N | N | N | 4 | 3 |
| 40 | 10 | AC_ACEG_2R | NORMAL | 2 | 0→5, +1, spX=1, Col=4 | 8→10, +1, spX=1, Col=2 | 2 | 1 | Y | N | N | N | 4 | 3 |

> XDiv=10 為 ATK 專用，只支援 ACEG。S0 滑動窗口 Col 0-7，S1 餘數 Col 8-9。

---

#### XDiv=12 (4 條)

| ID | XD | Cat | Cond | Seg | S0 | S1 | Row | spY | FB | Rev | CF | Kit | ACskip | SH |
|:--:|:--:|:---:|:----:|:---:|:---|:---|:---:|:---:|:--:|:---:|:--:|:---:|:------:|:--:|
| 41 | 12 | AC_1SUCK | NORMAL | 1 | 0→12, +1, spX=1, Col=1 | — | 1 | 0 | N | N | N | N | 4 | 0 |
| 42 | 12 | AC_KIT26 | NORMAL | 1 | Kit, spX=2, Col=3 | — | 2 | 1 | Y | N | N | Y | 4 | 0 |
| 43 | 12 | AC_ACEG_1R | NORMAL | 2 | 0→2, +1, spX=2, Col=4 | 8→9, +1, spX=1, Col=4 | 1 | 1 | N | N | N | N | 4 | 0 |
| 44 | 12 | AC_ACEG_2R | NORMAL | 2 | 0→2, +1, spX=2, Col=4 | 8→9, +1, spX=1, Col=4 | 2 | 1 | Y | N | N | N | 4 | 0 |

> **ID42 (Kit2x6)**: ixSequence = {0, 6, 1, 7, -1, ...}。Kit0 Col0-5, Kit1 Col6-11 交替。  
> **ID43-44**: S0(spacX=2) 覆蓋 Col 0-7, S1(spacX=1) 覆蓋 Col 8-11。

---

#### XDiv=16 (2 條)

| ID | XD | Cat | Cond | Seg | S0 | S1 | Row | spY | FB | Rev | CF | Kit | ACskip | SH |
|:--:|:--:|:---:|:----:|:---:|:---|:---|:---:|:---:|:--:|:---:|:--:|:---:|:------:|:--:|
| 45 | 16 | AC_ACEG_32 | NORMAL | 1 | 0→4, +1, spX=4, Col=4 | — | 2 | 1 | Y | Y | N | N | 4 | 0 |
| 46 | 16 | AC_ACEG_2R | NORMAL | 1 | 0→4, +1, spX=4, Col=4 | — | 2 | 1 | Y | Y | N | N | 4 | 0 |

> **bPlateReverse=Y**: 舊版 XItem16 iPlate 從 1→0（先放 HP1 再 HP0），保持一致。  
> spacX=4: 吸嘴佔 Col[ix], Col[ix+4], Col[ix+8], Col[ix+12]。

---

### 3.4 表格總計

| XDiv | 條數 | 說明 |
|:----:|:----:|------|
| 2 | 4 | |
| 3 | 5 | 含 Tri 特殊 |
| 4 | 9 | 含 b4x11HP / b4x10HP 變體 |
| 6 Std | 8 | 含 NotStandY / PitchOver |
| 6 Wide | 4 | |
| 8 | 8 | 含 b8x16HP_2x2 / PitchOver |
| 10 | 2 | ATK |
| 12 | 4 | 含 Kit2x6 |
| 16 | 2 | 32-Site |
| **合計** | **46** | |

---

## §4 統一放置引擎

### 4.1 入口函式

```cpp
void SearchPlateToPlace_New()
{
    // === Phase 0: 前置條件 (不變) ===
    if (AutoSiteMap 跳過條件) return;

    // === Phase 1: 計算 iYHalf ===
    GetHotPlateYHalfPos();   // 設定 iYHalf, b6x20HP, b4x11HP, ...

    // === Phase 2: 查表 ===
    const TPlaceEntry* pEntry = LookupPlaceEntry(
        HotPlateForm.XDivision,
        iInArmType,
        InArmSuck.iPickRow,
        InArmSuck.iPickCol,
        (i8PickerHPMode == iHPWideHP),
        bPitchOver12000,
        iYHalf
    );

    if (pEntry == NULL)
    {
        ShowErrorMessage("MES0156", 0, MInArmX, 0, "SearchPlateToPlace");
        return;
    }

    // === Phase 3: 執行掃描 ===
    bool bFound = ExecutePlaceEntry(pEntry);

    if (bFound)
    {
        // === Phase 4: 計算 StepHint (寫回全域) ===
        ComputeStepHint(pEntry);

        // === Phase 5: 記錄路徑到 PickFromHPList ===
        PickFromHPList->UpdateHPSuckGroup(
            iPlacePlate[0], iPlacePlateY[0], iPlacePlateX[0],
            InArmSuck.iWhichSht, InArmSuck.iWhichKit);
        PickFromHPList->AddHPSuckGroup();
    }
    else
    {
        ShowErrorMessage("MES0156", 0, MInArmX, 0, "SearchPlateToPlace");
    }

    #ifndef SOFT_SIMULTE
    CheckSafeDoorIsClosed();
    #endif
}
```

### 4.2 ExecutePlaceEntry — 統一掃描引擎

```cpp
bool ExecutePlaceEntry(const TPlaceEntry* e)
{
    // --- Plate 方向 ---
    int startPlate = 0, endPlate = 1, plateStep = 1;
    if (e->bPlateReverse)
    {
        startPlate = 1; endPlate = 0; plateStep = -1;
    }
    // 單盤模式
    if (HotPlateForm.iPlateSelect == 0x01) { startPlate = 1; endPlate = 1; }
    if (HotPlateForm.iPlateSelect == 0x02) { startPlate = 0; endPlate = 0; }

    // --- 計算真正的 spacY ---
    int spacY;
    if (e->spacYMode == 0) spacY = 1;
    else if (e->spacYMode == 2) spacY = 5;     // b6x20HP
    else spacY = iYHalf;                        // spacYMode == 1

    // --- 逐段掃描 ---
    for (int s = 0; s < e->nSegments; s++)
    {
        const TPlaceSegment& seg = e->seg[s];

        // --- 逐盤 ---
        for (int iPlate = startPlate; ; iPlate += plateStep)
        {
            // --- 逐列 (Row) ---
            for (int iy = 0; iy < HotPlateForm.YDivision; iy++)
            {
                // AutoClean 跳過
                if (e->autoCleanSkipRows > 0 && iPlate == 0 &&
                    bRunAutoClean && TestIF_File.iAutoClean_Function &&
                    TestIF_File.iAutoClean_Tray == eCKPos_HP2 &&
                    iy < e->autoCleanSkipRows)
                {
                    continue;
                }

                // --- Row 決策 ---
                int nRow = e->nSuckRow;
                if (nRow == 2 && iy + spacY >= HotPlateForm.YDivision)
                {
                    if (e->bRowFallback) nRow = 1;
                    else continue;   // 不能放 2Row, 且不允許降級 → 跳過
                }

                // --- 逐欄 (ix) ---
                if (e->bKitInterleave)
                {
                    // Kit 交錯: 使用 ixSequence
                    for (int q = 0; q < 8; q++)
                    {
                        int ix = e->ixSequence[q];
                        if (ix == -1) break;

                        bool bOK = CheckHotPlateHasSpace_9045_8_New_V(
                            iPlate, iy, ix, spacY, seg.spacX,
                            nRow, seg.nSuckCol, 0);

                        if (bOK)
                        {
                            iPlacePlateX[0] = ix;
                            iPlacePlateY[0] = iy;
                            iPlacePlate[0]  = iPlate;
                            return true;
                        }
                    }
                }
                else if (e->bColFirst)
                {
                    // Col-first: 先掃完一 Col 全部 Row, 再換 Col
                    // (XDiv=3 系列使用)
                    // 注意: 這裡 iy 是外層迴圈, 需要重組
                    // → 見 §4.3 ColFirst 特殊處理
                }
                else
                {
                    // 標準 Row-first
                    for (int ix = seg.ixStart; ix < seg.ixEnd; ix += seg.ixStep)
                    {
                        bool bOK = CheckHotPlateHasSpace_9045_8_New_V(
                            iPlate, iy, ix, spacY, seg.spacX,
                            nRow, seg.nSuckCol, 0);

                        // Row2 失敗 → 嘗試 Row1
                        if (!bOK && e->bRowFallback && nRow == 2)
                        {
                            bOK = CheckHotPlateHasSpace_9045_8_New_V(
                                iPlate, iy, ix, spacY, seg.spacX,
                                1, seg.nSuckCol, 0);
                            if (bOK) nRow = 1;
                        }

                        if (bOK)
                        {
                            iPlacePlateX[0] = ix;
                            iPlacePlateY[0] = iy;
                            iPlacePlate[0]  = iPlate;
                            return true;
                        }
                    }
                }
            }

            // 換盤
            if (HotPlateForm.iPlateSelect != 0x03) break;
            if (iPlate == endPlate) break;
        }
    }

    return false;   // 全段全盤無空間
}
```

### 4.3 Col-first 掃描（XDiv=3 專用）

XDiv=3 的 XItem3_1x2Suck / XItem3_2x2Suck 使用 **col-first** 掃描，即先掃完整個 Col 的所有 Row，再換下一 Col。標準引擎的 Row-first 不適用。

處理方案有兩種：

**方案 A — 引擎內加 bColFirst 分支**:

```cpp
// bColFirst == true 時: 外層=Col, 內層=Row
if (e->bColFirst)
{
    for (int s = 0; s < e->nSegments; s++)
    {
        const TPlaceSegment& seg = e->seg[s];
        for (int iPlate = startPlate; ; iPlate += plateStep)
        {
            for (int ix = seg.ixStart; ix < seg.ixEnd; ix += seg.ixStep)
            {
                for (int iy = 0; iy < HotPlateForm.YDivision; iy++)
                {
                    // 奇數 YDiv 保護: 最後段的末格跳過
                    if (s == e->nSegments - 1 &&
                        ix == seg.ixEnd - 1 &&
                        iy == HotPlateForm.YDivision - 1 &&
                        HotPlateForm.YDivision % 2 != 0)
                    {
                        continue;
                    }

                    int nRow = e->nSuckRow;
                    if (nRow == 2 && iy + spacY >= HotPlateForm.YDivision)
                    {
                        if (e->bRowFallback) nRow = 1;
                        else continue;
                    }

                    bool bOK = CheckHotPlateHasSpace_9045_8_New_V(
                        iPlate, iy, ix, spacY, seg.spacX,
                        nRow, seg.nSuckCol, 0);

                    if (bOK)
                    {
                        iPlacePlateX[0] = ix;
                        iPlacePlateY[0] = iy;
                        iPlacePlate[0]  = iPlate;
                        return true;
                    }
                }
            }
            if (HotPlateForm.iPlateSelect != 0x03) break;
            if (iPlate == endPlate) break;
        }
    }
    return false;
}
```

**方案 B — XDiv=3 Tri 特殊入口**: TriSite 的 Pitch 方向切換邏輯過於特殊，保留為獨立函式 `SearchPlateToPlace_Tri()`，其他 XDiv=3 模式統一走 bColFirst。

> **建議**: 採方案 A + B 混合 — Tri(ID6) 保留獨立函式，其餘 XDiv=3 (ID7-9) 走 bColFirst 引擎。

---

## §5 StepHint 統一計算

取代舊版零散的 `iForPlaceHPX3Step` / `iForPlaceHPX6Step` / `iForPlaceHPX10Step`。

### 5.1 stepHintRule 定義

| 值 | 名稱 | 適用 | 計算邏輯 |
|:--:|------|------|---------|
| 0 | 無 | 大部分 | 不需計算偏移 |
| 1 | X3 規則 | XDiv=3 (ID 7-9) | Col2 單放 → iForPlaceHPX3Step=1 |
| 2 | X6 規則 | XDiv=6 (ID 20,22-26,30) | 左半空→偏移右放 → iForPlaceHPX6Step=1 |
| 3 | X10 規則 | XDiv=10 (ID 39-40) | 左半無IC→偏移 → iForPlaceHPX10Step=2 |

### 5.2 擬碼

```cpp
void ComputeStepHint(const TPlaceEntry* e)
{
    // 先歸零
    iForPlaceHPX3Step  = 0;
    iForPlaceHPX6Step  = 0;
    iForPlaceHPX10Step = 0;

    int ix = iPlacePlateX[0];
    int iy = iPlacePlateY[0];
    int ip = iPlacePlate[0];

    switch (e->stepHintRule)
    {
    case 0:  // 無需計算
        break;

    case 1:  // X3 規則
    {
        // 在 Seg1 (Col2 單放) 時
        if (e->nSegments >= 2 && ix >= e->seg[1].ixStart)
            iForPlaceHPX3Step = 1;
        // 奇數 y 且在 Col2 也觸發
        if (ix == HotPlateForm.XDivision - 1 && iy % 2 == 1)
            iForPlaceHPX3Step = 1;
        break;
    }

    case 2:  // X6 規則
    {
        // 檢查找到位置時，左側吸嘴位是否全空
        const TPlaceSegment& seg = (ix < e->seg[0].ixEnd) ? e->seg[0] : e->seg[1];
        bool bLeftEmpty = true;
        int half = seg.nSuckCol / 2;
        if (half < 1) half = 1;
        for (int c = 0; c < half; c++)
        {
            int col = ix + c * seg.spacX;
            if (col < HotPlateForm.XDivision &&
                MOT[MMPlate1 + ip].Tray.Data[col][iy] != NULL_IC)
            {
                bLeftEmpty = false;
                break;
            }
        }
        if (bLeftEmpty && seg.nSuckCol >= 2)
            iForPlaceHPX6Step = 1;
        break;
    }

    case 3:  // X10 規則
    {
        // 左半 2 吸嘴皆空 + 右半有 IC → 偏移
        bool bLeftEmpty = true;
        bool bRightHasIC = false;
        int half = 2;  // 左半 2 Col

        for (int c = 0; c < half; c++)
        {
            int col = ix + c;
            if (col < HotPlateForm.XDivision &&
                MOT[MMPlate1 + ip].Tray.Data[col][iy] != NULL_IC)
                bLeftEmpty = false;
        }
        for (int c = half; c < 4; c++)
        {
            int col = ix + c;
            if (col < HotPlateForm.XDivision &&
                MOT[MMPlate1 + ip].Tray.Data[col][iy] != NULL_IC)
                bRightHasIC = true;
        }
        if (bLeftEmpty && bRightHasIC)
            iForPlaceHPX10Step = 2;
        break;
    }

    default:
        break;
    }
}
```

---

## §6 PickFromHPList 整合

### 6.1 設計要點

放置完成後路徑存入 `PickFromHPList` 結構（FIFO），吸取時直接依序讀取，不需重新搜尋。

### 6.2 呼叫時機

```
SearchPlateToPlace_New()
  └─ ExecutePlaceEntry() 找到位置
       ├─ 設定 iPlacePlateX[0]/Y[0]/Plate[0]
       ├─ ComputeStepHint()
       └─ PickFromHPList->UpdateHPSuckGroup(
              iPlacePlate[0], iPlacePlateY[0], iPlacePlateX[0],
              InArmSuck.iWhichSht, InArmSuck.iWhichKit)
          PickFromHPList->AddHPSuckGroup()
```

### 6.3 Pick 端讀取

```cpp
// InArm Pick from HP 流程 (既有代碼, 不需修改):
uHPSuckTeam* HTTeam = PickFromHPList->ExtractLastTeamHasData();
// HTTeam->iSht, HTTeam->iKit → 取料位置
// PickFromHPList->GetHPFirstTeamPlate(iP, iR, iC, iSht, iKit)
```

> 由於 Pick 端只讀 PickFromHPList 結構，與掃描引擎完全解耦。新版不需實作 state=1 (Pick) 或 state=3 (AutoClean) 搜尋。

---

## §7 Edge Case 統一處理

### 7.1 以表項區分的 Edge Case

所有 Edge Case 在查表階段就已分流到專屬表項，引擎不需特判。

| Edge Case | 對應表項 ID | 關鍵差異 |
|-----------|:-----------:|---------|
| b4x11HP (奇數 YDiv) | 14 | Row=1, ix step=2 |
| b4x10HP_2x2 (YPitch 非整除) | 15 | spacYMode=0（spacY=1 固定）|
| b8x16HP_2x2 (QualSite2x2) | 35 | spacYMode=0（spacY=1 固定）|
| b6x20HP (6×20, YPitch=12.7mm) | — | spacYMode=2（spacY=5 固定），適用 ID 22/26/28/29 等 |
| bPitchOver12000 | 12,21,24,33,38 | 降級或兩段式 |
| NotStandY (!CanPutAll) | 23 | Row=1 強制 |
| WideHP | 27-30 | 獨立表項 |
| Kit 交錯 (b12x16HP_2x6) | 42 | bKitInterleave=Y |

### 7.2 b6x20HP 的處理

b6x20HP 屬於動態條件（由 `GetHotPlateYHalfPos()` 計算），不適合在靜態表中硬編碼。

**處理方案**: `LookupPlaceEntry()` 查表成功後，覆蓋 spacYMode：

```cpp
const TPlaceEntry* LookupPlaceEntry(...)
{
    // 查表...
    TPlaceEntry* pEntry = FindInTable(xDiv, armCategory, condition);

    // b6x20HP 動態覆蓋
    if (b6x20HP && pEntry != NULL && pEntry->nSuckRow == 2)
    {
        // 需要返回一份修改後的副本
        static TPlaceEntry modified;
        modified = *pEntry;
        modified.spacYMode = 2;  // 固定 spacY=5
        return &modified;
    }

    return pEntry;
}
```

### 7.3 AutoClean 統一

所有表項的 `autoCleanSkipRows` 預設為 4。引擎在掃描時統一檢查：

```
HP0 + bRunAutoClean + iAutoClean_Function + (eCKPos_HP2) + iy < 4 → skip
```

### 7.4 奇數 YDiv 末格保護

XDiv=3 系列（ID 7-9）的末格 `[XDiv-1][YDiv-1]` 在奇數 YDiv 時需跳過。  
在 bColFirst 分支中已處理（見 §4.3）。

### 7.5 PlateSelect 統一

| iPlateSelect | 行為 | 引擎處理 |
|:---:|------|---------|
| 0x01 | 只用 HP1 (iPlate=1) | startPlate=1, endPlate=1 |
| 0x02 | 只用 HP0 (iPlate=0) | startPlate=0, endPlate=0 |
| 0x03 | 雙盤交替 | startPlate=0, endPlate=1 (或反向) |

---

## §8 查表索引函式

### 8.1 armCategory 分類函式

```cpp
int GetArmCategory(int iType, int iPickRow, int iPickCol, bool bWideHP, bool bKit2x6)
{
    // 1Suck
    if (USE_PICKER_COUNT == ep1Picker)       return AC_1SUCK;
    if (iType == e9045_1x1_1)               return AC_1SUCK;
    if (iType == e9045_1x4_1_Ac)            return AC_1SUCK;
    // DualSite Ab關站降級
    if (bDualSiteCloseAbCanFullHotplate)
    {
        if (iType == e9045_1x2_2_13 && iSiteMap[0][1] == 0) return AC_1SUCK;
        if (iType == e9045_2x1_2_13 && iSiteMap[1][0] == 0) return AC_1SUCK;
    }

    // Tri
    if (iType == e9045_1x3_2_14 && iModeX != 3)  return AC_TRI;

    // WideHP 路徑
    if (bWideHP)
    {
        if (iType == e9045_1x3_4)   return AC_W13;
        if (iType == e9045_2x3_6)   return AC_W23;
        if (iType == e9045_2x6_8)   return AC_W26;
        // WideHP ACEG → 走 AC_ACEG_2R + COND_WIDE_HP
    }

    // Kit 2x6
    if (bKit2x6 && iType == e9045_2x6_8)  return AC_KIT26;

    // AxEx / AxxG
    if (bUseAxExPicker() || bUseAxxGPicker())
    {
        if (iPickRow == 1) return AC_AX1R;
        if (iPickRow == 2) return AC_AX2R;
    }

    // NN14
    if (iType == e9045_1x4_4_13NN && InArmSuck.iShtRow == 1) return AC_NN14;

    // Hot 模式
    if (iType == e9045_2x2_8_Hot) return AC_H28;
    if (iType == e9045_1x4_8_Hot) return AC_H14;

    // ACEG 分 1Row / 2Row
    if (iPickRow == 1) return AC_ACEG_1R;

    // 32-Site / 2x4_16 特殊
    if (iType == e9045_2x8_32 || iType == e9045_2x4_16) return AC_ACEG_32;

    return AC_ACEG_2R;  // 預設 ACEG 2Row
}
```

### 8.2 condition 分類函式

```cpp
int GetCondition(int xDiv, int armCat, bool bPitchOver, bool bWideHP)
{
    if (bWideHP && (armCat == AC_ACEG_1R || armCat == AC_ACEG_2R))
        return COND_WIDE_HP;

    if (bPitchOver && (armCat == AC_AX1R || armCat == AC_AX2R ||
                       armCat == AC_ACEG_1R || armCat == AC_ACEG_2R))
        return COND_PITCH_OVER;

    if (b4x11HP && armCat == AC_AX2R) return COND_4x11HP;
    if (b4x10HP_2x2 && armCat == AC_AX2R) return COND_4x10HP;
    if (b8x16HP_2x2 && armCat == AC_AX2R) return COND_8x16_2x2;

    if (xDiv == 6 && !bWideHP && armCat == AC_AX2R &&
        !HotPlateYPitchCanPutAll())
        return COND_NOT_STAND_Y;

    return COND_NORMAL;
}
```

### 8.3 LookupPlaceEntry

```cpp
const TPlaceEntry* LookupPlaceEntry(int xDiv, int iType, int iPickRow,
    int iPickCol, bool bWideHP, bool bPitchOver, int iYHalf_)
{
    int armCat = GetArmCategory(iType, iPickRow, iPickCol, bWideHP, b12x16HP_2x6);
    int cond   = GetCondition(xDiv, armCat, bPitchOver, bWideHP);

    // 線性搜尋（46 條，效能不是問題）
    for (int i = 0; i < PLACE_TABLE_COUNT; i++)
    {
        if (PLACE_TABLE[i].xDiv == xDiv &&
            PLACE_TABLE[i].armCategory == armCat &&
            PLACE_TABLE[i].condition == cond)
        {
            // b6x20HP 動態覆蓋
            if (b6x20HP && PLACE_TABLE[i].nSuckRow == 2)
            {
                static TPlaceEntry modified;
                modified = PLACE_TABLE[i];
                modified.spacYMode = 2;
                return &modified;
            }
            return &PLACE_TABLE[i];
        }
    }

    return NULL;  // 查無 → 呼叫端報 MES0156
}
```

---

## §9 檔案結構與行數預估

### 9.1 新增檔案

```
ainarm_SearchPlacePlate_table.h     // TPlaceEntry, TPlaceSegment, TStepHint,
                                    // enum eArmCategory, enum eCondition
ainarm_SearchPlacePlate_table.cpp   // PLACE_TABLE[], LookupPlaceEntry(),
                                    // ExecutePlaceEntry(), ComputeStepHint(),
                                    // SearchPlateToPlace_New()
```

### 9.2 行數預估

| 區塊 | 行數 |
|------|:----:|
| `.h` — 結構定義 + enum | ~70 行 |
| `.cpp` — PLACE_TABLE[46] 靜態陣列 | ~200 行 |
| `.cpp` — GetArmCategory() | ~50 行 |
| `.cpp` — GetCondition() | ~25 行 |
| `.cpp` — LookupPlaceEntry() | ~30 行 |
| `.cpp` — ExecutePlaceEntry() (含 ColFirst) | ~120 行 |
| `.cpp` — ComputeStepHint() | ~60 行 |
| `.cpp` — SearchPlateToPlace_New() 入口 | ~35 行 |
| **合計** | **~590 行** |

> 對比舊版 4700+ 行 → ★ 程式碼量減少 87%。

### 9.3 保留不動的檔案

```
ainarm_SearchPlacePlate.cpp   // 舊版保留（#ifdef USE_OLD_SEARCH_PLATE 切換）
GetHotPlateYHalfPos()         // 引用既有
HotPlateYPitchCanPutAll()     // 引用既有
CheckHotPlateHasSpace_9045_8_New_V()  // 引用既有
MoveInArmXYToHotPlatePlace()  // 引用既有（讀 iPlacePlateX/Y/Plate）
```

---

## §10 相容性與風險

### 10.1 全域變數相容

引擎寫入完全相同的全域變數，下游不需修改：

| 全域變數 | 寫入者 | 讀取者 |
|---------|--------|--------|
| `iPlacePlateX[0]` | ExecutePlaceEntry | MoveInArmXYToHotPlatePlace |
| `iPlacePlateY[0]` | ExecutePlaceEntry | MoveInArmXYToHotPlatePlace |
| `iPlacePlate[0]` | ExecutePlaceEntry | MoveInArmXYToHotPlatePlace |
| `iForPlaceHPX3Step` | ComputeStepHint | ainarm9045_InArmXxx.cpp |
| `iForPlaceHPX6Step` | ComputeStepHint | ainarm9045_InArmXxx.cpp |
| `iForPlaceHPX10Step` | ComputeStepHint | ainarm9045_InArmXxx.cpp |

### 10.2 PickFromHPList 相容

既有的 `UpdateHPSuckGroup` / `AddHPSuckGroup` 呼叫簽名不變。Pick 端讀取 `ExtractLastTeamHasData()` 不受影響。

### 10.3 潛在行為差異

| 項目 | 舊版行為 | 新版行為 | 影響 |
|------|---------|---------|------|
| XItem16 Plate 方向 | iPlate 1→0 | bPlateReverse=Y | 相同 |
| XItem16 手寫陣列掃描 | 不用 CheckHasSpace | 統一 CheckHasSpace | 邏輯等價 |
| NotStandY flag 直檢 | flag[2][2] | CheckHasSpace(Row=1) | 邏輯等價 |
| AX1R PitchOver XDiv=8 | ix 跳 0→1→4→5 | 兩段式 S0(0→2)+S1(4→6) | 掃描順序不同,填滿率相同 |
| XDiv=3 Col2 掃描順序 | col-first 手寫 | bColFirst 分支 | 相同 |
| 12_2x6 Kit 交錯 | 0→6→1→7 | ixSequence{0,6,1,7,-1} | 相同 |
| Row2 失敗後降級 | 各函式各異 | 統一 bRowFallback | 可能更積極填充 |

### 10.4 風險評估

| 風險 | 等級 | 緩解措施 |
|------|:----:|---------|
| 掃描順序差異導致填充結果不同 | 中 | Python 模擬驗證（見 §11） |
| PLACE_TABLE 遺漏某組合 | 中 | 查表失敗返回 NULL → MES0156 + 紀錄日誌 |
| b6x20HP 動態覆蓋用 static 變數 | 低 | InArm 為單一 Task，不會並行 |
| Tri 特殊邏輯分離 | 低 | 保留獨立函式 SearchPlateToPlace_Tri() |
| PickFromHPList 寫入時機不同 | 低 | 統一在 bFound 後立即寫入 |

---

## §11 驗證方案

### 11.1 Python 模擬驗證

```python
# scripts/verify_place_table.py
# 模擬舊/新版對所有 46 條表項的填充行為

class HPSimulator:
    def __init__(self, xdiv, ydiv, num_plates=2):
        self.xdiv = xdiv
        self.ydiv = ydiv
        self.data = [[[0]*ydiv for _ in range(xdiv)] for _ in range(num_plates)]

    def check_has_space(self, iP, iy, ix, spacY, spacX, nRow, nCol):
        for r in range(nRow):
            for c in range(nCol):
                cy = iy + spacY * r
                cx = ix + spacX * c
                if cy >= self.ydiv or cx >= self.xdiv:
                    return False
                if self.data[iP][cx][cy] != 0:
                    return False
        return True

    def place(self, iP, iy, ix, spacY, spacX, nRow, nCol):
        for r in range(nRow):
            for c in range(nCol):
                self.data[iP][ix + spacX * c][iy + spacY * r] = 1

# 對每個 PLACE_TABLE entry:
# 1. 建立空 HP
# 2. 反覆呼叫 ExecutePlaceEntry 模擬直到放滿
# 3. 計算填滿率
# 4. 與舊版 XItem 函式的已知填充結果比對
```

### 11.2 驗證覆蓋率

```
PLACE_TABLE 46 條
× 2 種 YDiv (最小/最大)
× 2 種 PlateSelect (單盤/雙盤)
= 184 組測試案例

預計 Python 跑完 < 2 秒
```

### 11.3 差異報告格式

```
ID=37 XDiv=8 ACEG_2R NORMAL
  Old: 步驟1→(0,0,0) 步驟2→(1,0,0) 步驟3→(0,4,0) ...
  New: 步驟1→(0,0,0) 步驟2→(1,0,0) 步驟3→(0,4,0) ...
  填滿率: Old=100% New=100% ✅
  順序差異: 無
```

---

## 附錄 A: PLACE_TABLE C++ 靜態陣列（完整版）

```cpp
// ainarm_SearchPlacePlate_table.cpp

static const TPlaceEntry PLACE_TABLE[] =
{
// ============================================================================
// XDiv=2
// ============================================================================
// ID=1: 1Suck
{ 2, AC_1SUCK, COND_NORMAL,
  1, // nSegments
  { {0, 2, 1, 1, 1}, {0,0,0,0,0}, {0,0,0,0,0} }, // seg[3]
  1, 0,  // nSuckRow=1, spacYMode=0 (固定1)
  false, false, false, false, // FB, Rev, CF, Kit
  {-1,0,0,0,0,0,0,0}, // ixSequence (unused)
  4, 0  // autoCleanSkipRows, stepHintRule
},

// ID=2: AX1R
{ 2, AC_AX1R, COND_NORMAL,
  1,
  { {0, 1, 1, 1, 2}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=3: AX2R
{ 2, AC_AX2R, COND_NORMAL,
  1,
  { {0, 1, 1, 1, 2}, {0,0,0,0,0}, {0,0,0,0,0} },
  2, 1,  // nSuckRow=2, spacYMode=1 (iYHalf)
  true, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=4: ACEG降級
{ 2, AC_ACEG_1R, COND_NORMAL,
  1,
  { {0, 1, 1, 1, 2}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ============================================================================
// XDiv=3
// ============================================================================
// ID=5: 1Suck
{ 3, AC_1SUCK, COND_NORMAL,
  1,
  { {0, 3, 1, 1, 1}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=6: Tri (特殊, Pitch 切換)
{ 3, AC_TRI, COND_NORMAL,
  1,
  { {0, 3, 1, 1, 1}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,   // CF 由 Pitch 動態決定
  {-1,0,0,0,0,0,0,0},
  0, 1
},

// ID=7: AX1R (col-first, 2段)
{ 3, AC_AX1R, COND_NORMAL,
  2,
  { {0, 2, 1, 1, 2}, {2, 3, 1, 1, 1}, {0,0,0,0,0} },
  1, 0,
  false, false, true, false,  // bColFirst=true
  {-1,0,0,0,0,0,0,0},
  4, 1
},

// ID=8: AX2R (col-first, 2段)
{ 3, AC_AX2R, COND_NORMAL,
  2,
  { {0, 2, 1, 1, 2}, {2, 3, 1, 1, 1}, {0,0,0,0,0} },
  2, 1,
  true, false, true, false,
  {-1,0,0,0,0,0,0,0},
  4, 1
},

// ID=9: ACEG 降級 (col-first, 2段, 同AX1R)
{ 3, AC_ACEG_1R, COND_NORMAL,
  2,
  { {0, 2, 1, 1, 2}, {2, 3, 1, 1, 1}, {0,0,0,0,0} },
  1, 0,
  false, false, true, false,
  {-1,0,0,0,0,0,0,0},
  4, 1
},

// ============================================================================
// XDiv=4
// ============================================================================
// ID=10: 1Suck
{ 4, AC_1SUCK, COND_NORMAL,
  1,
  { {0, 4, 1, 1, 1}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=11: AX1R 標準
{ 4, AC_AX1R, COND_NORMAL,
  1,
  { {0, 2, 1, 2, 2}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=12: AX1R PitchOver (降級逐格)
{ 4, AC_AX1R, COND_PITCH_OVER,
  1,
  { {0, 4, 1, 1, 1}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=13: AX2R 標準
{ 4, AC_AX2R, COND_NORMAL,
  1,
  { {0, 2, 1, 2, 2}, {0,0,0,0,0}, {0,0,0,0,0} },
  2, 1,
  true, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=14: AX2R b4x11HP
{ 4, AC_AX2R, COND_4x11HP,
  1,
  { {0, 2, 2, 2, 2}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=15: AX2R b4x10HP
{ 4, AC_AX2R, COND_4x10HP,
  1,
  { {0, 2, 1, 2, 2}, {0,0,0,0,0}, {0,0,0,0,0} },
  2, 0,  // spacYMode=0 → 固定 spacY=1
  true, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=16: ACEG 1Row
{ 4, AC_ACEG_1R, COND_NORMAL,
  1,
  { {0, 1, 1, 1, 4}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 1,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=17: ACEG 2Row
{ 4, AC_ACEG_2R, COND_NORMAL,
  1,
  { {0, 1, 1, 1, 4}, {0,0,0,0,0}, {0,0,0,0,0} },
  2, 1,
  true, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=18: NN14
{ 4, AC_NN14, COND_NORMAL,
  1,
  { {0, 1, 1, 1, 4}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,  // spacYMode=0 → spacY=1
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ============================================================================
// XDiv=6 Standard
// ============================================================================
// ID=19: 1Suck
{ 6, AC_1SUCK, COND_NORMAL,
  1,
  { {0, 6, 1, 1, 1}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=20: AX1R 標準
{ 6, AC_AX1R, COND_NORMAL,
  1,
  { {0, 3, 1, 3, 2}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 2
},

// ID=21: AX1R PitchOver
{ 6, AC_AX1R, COND_PITCH_OVER,
  1,
  { {0, 6, 1, 1, 1}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=22: AX2R CanPutAll
{ 6, AC_AX2R, COND_NORMAL,
  1,
  { {0, 3, 1, 3, 2}, {0,0,0,0,0}, {0,0,0,0,0} },
  2, 1,
  true, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 2
},

// ID=23: AX2R NotStandY
{ 6, AC_AX2R, COND_NOT_STAND_Y,
  1,
  { {0, 3, 1, 3, 2}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 2
},

// ID=24: AX2R PitchOver
{ 6, AC_AX2R, COND_PITCH_OVER,
  1,
  { {0, 6, 1, 1, 1}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=25: ACEG 1Row (降級)
{ 6, AC_ACEG_1R, COND_NORMAL,
  1,
  { {0, 3, 1, 3, 2}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 2
},

// ID=26: ACEG 2Row (降級)
{ 6, AC_ACEG_2R, COND_NORMAL,
  1,
  { {0, 3, 1, 3, 2}, {0,0,0,0,0}, {0,0,0,0,0} },
  2, 1,
  true, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 2
},

// ============================================================================
// XDiv=6 WideHP
// ============================================================================
// ID=27: W13
{ 6, AC_W13, COND_WIDE_HP,
  1,
  { {0, 6, 3, 1, 3}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 1,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=28: W23
{ 6, AC_W23, COND_WIDE_HP,
  1,
  { {0, 6, 3, 1, 3}, {0,0,0,0,0}, {0,0,0,0,0} },
  2, 1,
  true, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=29: W26
{ 6, AC_W26, COND_WIDE_HP,
  1,
  { {0, 6, 3, 1, 3}, {0,0,0,0,0}, {0,0,0,0,0} },
  2, 1,
  true, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=30: ACEG 2Row WideHP (兩段)
{ 6, AC_ACEG_2R, COND_WIDE_HP,
  2,
  { {0, 1, 1, 1, 4}, {4, 6, 1, 1, 2}, {0,0,0,0,0} },
  2, 1,
  true, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 2
},

// ============================================================================
// XDiv=8
// ============================================================================
// ID=31: 1Suck
{ 8, AC_1SUCK, COND_NORMAL,
  1,
  { {0, 8, 1, 1, 1}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=32: AX1R 標準
{ 8, AC_AX1R, COND_NORMAL,
  1,
  { {0, 4, 1, 4, 2}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=33: AX1R PitchOver (兩段)
{ 8, AC_AX1R, COND_PITCH_OVER,
  2,
  { {0, 2, 1, 2, 2}, {4, 6, 1, 2, 2}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=34: AX2R 標準
{ 8, AC_AX2R, COND_NORMAL,
  1,
  { {0, 4, 1, 4, 2}, {0,0,0,0,0}, {0,0,0,0,0} },
  2, 1,
  true, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=35: AX2R b8x16_2x2
{ 8, AC_AX2R, COND_8x16_2x2,
  1,
  { {0, 4, 1, 4, 2}, {0,0,0,0,0}, {0,0,0,0,0} },
  2, 0,  // spacYMode=0 → 固定 spacY=1
  true, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=36: ACEG 1Row ★
{ 8, AC_ACEG_1R, COND_NORMAL,
  1,
  { {0, 2, 1, 2, 4}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 1,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=37: ACEG 2Row ★ 主力路徑
{ 8, AC_ACEG_2R, COND_NORMAL,
  1,
  { {0, 2, 1, 2, 4}, {0,0,0,0,0}, {0,0,0,0,0} },
  2, 1,
  true, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=38: ACEG PitchOver (兩段)
{ 8, AC_ACEG_2R, COND_PITCH_OVER,
  2,
  { {0, 1, 1, 1, 4}, {4, 5, 1, 1, 4}, {0,0,0,0,0} },
  2, 1,
  true, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ============================================================================
// XDiv=10
// ============================================================================
// ID=39: ACEG 1Row
{ 10, AC_ACEG_1R, COND_NORMAL,
  2,
  { {0, 5, 1, 1, 4}, {8, 10, 1, 1, 2}, {0,0,0,0,0} },
  1, 1,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 3
},

// ID=40: ACEG 2Row
{ 10, AC_ACEG_2R, COND_NORMAL,
  2,
  { {0, 5, 1, 1, 4}, {8, 10, 1, 1, 2}, {0,0,0,0,0} },
  2, 1,
  true, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 3
},

// ============================================================================
// XDiv=12
// ============================================================================
// ID=41: 1Suck
{ 12, AC_1SUCK, COND_NORMAL,
  1,
  { {0, 12, 1, 1, 1}, {0,0,0,0,0}, {0,0,0,0,0} },
  1, 0,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=42: Kit 2x6
{ 12, AC_KIT26, COND_NORMAL,
  1,
  { {0, 12, 1, 2, 3}, {0,0,0,0,0}, {0,0,0,0,0} },
  2, 1,
  true, false, false, true,   // bKitInterleave=true
  {0, 6, 1, 7, -1, 0, 0, 0}, // ixSequence
  4, 0
},

// ID=43: ACEG 1Row (兩段)
{ 12, AC_ACEG_1R, COND_NORMAL,
  2,
  { {0, 2, 1, 2, 4}, {8, 9, 1, 1, 4}, {0,0,0,0,0} },
  1, 1,
  false, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=44: ACEG 2Row (兩段)
{ 12, AC_ACEG_2R, COND_NORMAL,
  2,
  { {0, 2, 1, 2, 4}, {8, 9, 1, 1, 4}, {0,0,0,0,0} },
  2, 1,
  true, false, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ============================================================================
// XDiv=16
// ============================================================================
// ID=45: ACEG 32Site
{ 16, AC_ACEG_32, COND_NORMAL,
  1,
  { {0, 4, 1, 4, 4}, {0,0,0,0,0}, {0,0,0,0,0} },
  2, 1,
  true, true, false, false,   // bPlateReverse=true
  {-1,0,0,0,0,0,0,0},
  4, 0
},

// ID=46: ACEG 2Row (2x4_16 等)
{ 16, AC_ACEG_2R, COND_NORMAL,
  1,
  { {0, 4, 1, 4, 4}, {0,0,0,0,0}, {0,0,0,0,0} },
  2, 1,
  true, true, false, false,
  {-1,0,0,0,0,0,0,0},
  4, 0
},

};

static const int PLACE_TABLE_COUNT = sizeof(PLACE_TABLE) / sizeof(PLACE_TABLE[0]);
```

---

## 附錄 B: 不支援的組合

以下 (XDiv × iInArmType) 組合在查表中無對應項，`LookupPlaceEntry()` 返回 `NULL` → 報 `MES0156`：

| XDiv | 不支援的 iInArmType | 原因 |
|:----:|----------------------|------|
| 2 | ACEG 2x4+, H28, H14, NN14 | 2-Col 容納不了 4 吸嘴 |
| 3 | ACEG 2x4+, W13/W23/W26 | 需新增 PlateForm |
| 10 | 1Suck, AX1R, AX2R | ATK 僅 ACEG |
| 16 | 1Suck, AX1R, AX2R, W13/W23/W26 | 32-Site 僅 ACEG |

完整不可達矩陣請參考 [HP_SearchPlacePlate_AllFunctions.md §17.5](HP_SearchPlacePlate_AllFunctions.md)。
