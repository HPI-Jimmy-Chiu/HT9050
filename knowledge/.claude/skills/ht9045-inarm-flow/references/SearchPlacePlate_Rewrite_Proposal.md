# ainarm_SearchPlacePlate_new.cpp 重寫方案

> **版本**: v1.0 Draft  
> **日期**: 2026-04-07  
> **目標**: 將 16 個硬編碼 `SearchPlacePlateXItem*()` 函式統一為 1 個參數化函式 + 1 張查表  
> **約束**: BCB6 C++98、Big5 編碼、不可用 C++11 語法

---

## 0. 現狀問題分析

### 0.1 舊版架構

```
SearchPlateToPlace()                        // 4700+ 行
├── if(ep1Picker)         → _1Suck()        // XDiv=any, 1 格逐放
├── if(1Row + AxEx/AxxG)  → _1x2Suck()      // XDiv=2/4/6/8
├── if(2Row + AxEx/AxxG)  → _2x2Suck()      // XDiv=2/4/6/8
├── if(XDiv==3)           → 3_1x2/3_2x2/3_2Suck_3Site()
├── if(XDiv==4)           → 4_8Suck()
├── if(XDiv==6 + WideHP)  → 6_3Suck_ACEx() / 6_8Suck()
├── if(XDiv==6 + StdHP)   → 6_2x2_8Suck() / 6_NotStandY()
├── if(XDiv==8)           → 8_8Suck()
├── if(XDiv==10)          → 10_8Suck()
├── if(XDiv==12)          → 12_2x6() / 12_8Suck()
├── if(XDiv==16)          → 16_8Suck()
└── else                  → MES0156
```

### 0.2 主要痛點

| 問題 | 影響 |
|------|------|
| 16 個函式各自實作掃描邏輯，大量重複 | 維護成本高，修 bug 需改 N 處 |
| XItem16 跳過 CheckHotPlateHasSpace，獨立手寫陣列掃描 | 行為不一致 |
| 新增 XDiv / iInArmType 需寫全新函式 | 擴充性差 |
| edge case（b6x20HP, b4x11HP 等）散布各函式 | 容易遺漏 |
| iForPlaceHPXnStep 在各函式中以不同方式計算 | 語意不清 |

---

## 1. 新版架構總覽

```
SearchPlateToPlace_New()
│
├── 1) GetHotPlateYHalfPos()           // 不變，計算 iYHalf 及各 HP flag
│
├── 2) BuildScanPlan()                 // ★ 核心：查表 → 產生 TScanPlan
│       ├── ResolvePickerLayout()      // 判斷 (iInArmType, XDiv) → picker 排列參數
│       └── ResolveScanSegments()      // 切分掃描段 (主段 + 餘數段)
│
├── 3) ExecuteScanPlan()               // ★ 核心：統一掃描引擎
│       ├── for each segment
│       │     for each plate
│       │       for each iy (row)
│       │         for each ix (col shift)
│       │           CheckHotPlateHasSpace_9045_8_New_V()
│       │           → 成功：設定 iPlacePlateX/Y/Plate, 計算 StepHint
│       │           → 失敗：下一個 (ix, iy, plate)
│       └── 全失敗 → MES0156
│
└── 4) ComputeStepHint()               // 取代 iForPlaceHPX3/6/10Step 的計算
```

### 1.1 設計原則

1. **查表取代 if-else 樹**: 每個 (XDiv, picker_mode) 組合對應一行描述資料
2. **掃描段（Segment）**: 將多段式掃描（如 XDiv=10 的 ix=0~4 + ix=8）抽象為陣列
3. **統一 CheckHotPlateHasSpace**: 所有 XDiv 包括 16 都走同一套檢查
4. **StepHint 集中計算**: 搜尋完位置後，根據實際 Tray.Data 狀態統一決定偏移
5. **Zero new global state**: 不新增全域變數，沿用既有的 iPlacePlateX/Y/Plate

---

## 2. 資料結構定義

### 2.1 TScanSegment — 掃描段

```cpp
// ainarm_SearchPlacePlate_new.h

struct TScanSegment
{
    int ixStart;      // 起始列 (inclusive)
    int ixEnd;        // 結束列 (exclusive)，-1 表示由 XDiv 計算
    int ixStep;       // 列步進 (通常 = 1)
    int spacX;        // CheckHasSpace 的 X spacing
    int nSuckCol;     // 每次放置佔幾欄
};
```

### 2.2 TScanPlan — 完整掃描計畫

```cpp
struct TScanPlan
{
    // --- 掃描段 ---
    int nSegments;              // 段數 (1~3)
    TScanSegment seg[3];        // 最多 3 段

    // --- 共用參數 ---
    int nSuckRow;               // 每次放置佔幾列 (1 or 2)
    int spacY;                  // CheckHasSpace 的 Y spacing (= iYHalf)

    // --- 掃描控制 ---
    bool bRowFallback;          // true: Row2 失敗時自動降級 Row1 重試
    bool bPlateForward;         // true: iPlate 0→1; false: iPlate 1→0 (XItem16 反向)
    int  iAutoCleanSkipRows;    // AutoClean 跳過的起始列數 (0=不跳, 4=跳 row 0~3)

    // --- Kit 交錯 (12_2x6 專用) ---
    bool bKitInterleave;        // true: 使用 ixSequence 而非線性遞增
    int  ixSequence[8];         // 自定義 ix 順序, 以 -1 結尾 (如 {0,6,1,7,-1,...})
};
```

### 2.3 TStepHint — 放置偏移提示（取代 iForPlaceHPXnStep）

```cpp
struct TStepHint
{
    int iXOffset;     // X 方向偏移吸嘴數 (0=正常, 1=偏右, 2=偏移模式)
    int iYOffset;     // Y 方向偏移 (0=正常)
    int iActiveCols;  // 實際可用的吸嘴欄數 (≤ nSuckCol)
};
```

---

## 3. 掃描計畫查表 — 完整 §19 × iInArmType 映射

### 3.1 查表函式

```cpp
TScanPlan BuildScanPlan(
    int XDiv,              // HotPlateForm.XDivision
    int YDiv,              // HotPlateForm.YDivision
    int iType,             // iInArmType enum
    int iPickRow,          // InArmSuck.iPickRow (1 or 2)
    int iPickCol,          // InArmSuck.iPickCol
    bool bWideHP,          // i8PickerHPMode == iHPWideHP
    bool bPitchOver,       // bPitchOver12000
    int iYHalf,            // GetHotPlateYHalfPos() 的結果
    bool bCanPutAll,       // HotPlateYPitchCanPutAll()
    int iPlateSelect       // HotPlateForm.iPlateSelect
);
```

### 3.2 掃描參數表

下表涵蓋 **§19 全部 HP 配置 × §17.2 全部 iInArmType** 的有效組合。

**圖例**:
- `Seg` = 掃描段數
- `spacX` / `Col` = 每段的 X spacing / 吸嘴欄數
- `Row` = nSuckRow (1 or 2)
- `ixRange` = ix 起止與步進
- `Kit` = 是否使用 Kit 交錯

#### 3.2.1 XDiv=2 (2×6, 2×7 HP)

| iInArmType | Seg | Seg0: spacX, Col, ixRange | Row | 備註 |
|---|:---:|---|:---:|---|
| ep1Picker / SingleSite | 1 | 1, 1, [0→2) | 1 | 逐格 |
| AxEx 1Row (1x2_13/14) | 1 | 1, 2, [0→1) | 1 | 2 吸嘴佔 2 欄 |
| AxxG 1Row | 1 | 1, 2, [0→1) | 1 | 同上 |
| AxEx 2Row (2x2_13/14) | 1 | 1, 2, [0→1) | 2 | |
| AxxG 2Row | 1 | 1, 2, [0→1) | 2 | |
| ACEG 1x4 | 1 | 1, 2, [0→1) | 1 | 降級為 2-吸嘴 |

#### 3.2.2 XDiv=3 (3×5, 3×7 HP — 目前無 PlateForm，需新增)

| iInArmType | Seg | Seg0: spacX, Col, ixRange | Row | 備註 |
|---|:---:|---|:---:|---|
| ep1Picker / SingleSite | 1 | 1, 1, [0→3) | 1 | 逐格 |
| TriSite (iModeX≠3) | 1 | 1, 1, [0→3) + Pitch 切換 | 1 | 3_2Suck_3Site 邏輯 |
| TriSite (iModeX=3) | 1 | 1, 2, col-first | 1 | 3_1x2 |
| AxEx 1Row | 1 | 1, 2, col-first | 1 | 3_1x2 |
| AxEx 2Row | 1 | 1, 2, col-first | 2 | 3_2x2, 奇數 YDiv 封鎖末格 |
| AxxG 1Row / 2Row | 同 AxEx | | | |

#### 3.2.3 XDiv=4 (4×8 HP, Pitch 40×40)

| iInArmType | Seg | Seg0: spacX, Col, ixRange | Row | 備註 |
|---|:---:|---|:---:|---|
| ep1Picker / SingleSite | 1 | 1, 1, [0→4) | 1 | |
| AxEx 1Row | 1 | 2, 2, [0→2) | 1 | |
| AxEx 1Row + PitchOver | 1 | 1, 1, [0→4) | 1 | 降級逐格 |
| AxEx 2Row | 1 | 2, 2, [0→2) | 2 | b4x11HP/b4x10HP_2x2 邊界 |
| ACEG 1x4 / 2x4 | 1 | 1, 4, [0→1) | 1/2 | 4 吸嘴填滿 4 欄 |
| NN 1x4 (iShtRow=1) | 1 | 1, 4, [0→1) | 1 | spacY=1 |
| 2x2_8_Hot | 1 | 1, 4, [0→1) | 2 | |
| 2x8_8 / 2x8_32 | 1 | 1, 4, [0→1) | 2 | |

#### 3.2.4 XDiv=6 標準 HP (6×11, Pitch 26.67×30)

| iInArmType | Seg | 各段 spacX, Col, ixRange | Row | 條件 |
|---|:---:|---|:---:|---|
| ep1Picker / SingleSite | 1 | 1, 1, [0→6) | 1 | |
| AxEx 1Row | 1 | 3, 2, [0→3) | 1 | |
| AxEx 1Row + PitchOver | 1 | 1, 1, [0→6) | 1 | 降級 |
| AxEx 2Row (CanPutAll) | 1 | 3, 2, [0→3) | 2 | 6_2x2 三段式 |
| AxEx 2Row (!CanPutAll) | 1 | 3, 2, [0→3) | 1~2 | 6_NotStandY |

#### 3.2.5 XDiv=6 WideHP (6×11/6×16, Pitch 26.67×30/20)

| iInArmType | Seg | 各段 spacX, Col, ixRange | Row | 備註 |
|---|:---:|---|:---:|---|
| 1x3_4 (WideHP) | 1 | 1, 3, [0→6) step=3 | 1 | 6_3Suck_ACEx |
| 2x3_6 (WideHP) | 1 | 1, 3, [0→6) step=3 | 2 | 6_3Suck_ACEx |
| 2x6_8 (WideHP) | 1 | 1, 3, [0→6) step=3 | 2 | 6_3Suck_ACEx |
| ACEG 1x4 (WideHP) | 2 | S0: 1, 4, [0→1); S1: 1, 2, [4→6) | 1/2 | 6_8Suck 兩段 |
| ACEG 2x4 (WideHP) | 2 | S0: 1, 4, [0→1); S1: 1, 2, [4→6) | 2 | 6_8Suck 兩段 |

#### 3.2.6 XDiv=8 (8×12/8×16/8×18/8×24 HP)

| iInArmType | Seg | 各段 spacX, Col, ixRange | Row | 備註 |
|---|:---:|---|:---:|---|
| ep1Picker / SingleSite | 1 | 1, 1, [0→8) | 1 | |
| AxEx 1Row | 1 | 4, 2, [0→4) | 1 | |
| AxEx 1Row + PitchOver | 1 | 1, 1, [0→8) | 1 | 降級 |
| AxEx 2Row | 1 | 4, 2, [0→4) | 2 | b8x16HP_2x2 邊界 |
| ACEG 1x4 / 2x4 | 1 | 2, 4, [0→2) | 1/2 | **8_8 主力路徑** |
| ACEG 2x5 | 1 | 2, 4, [0→2) | 2 | |
| ACEG 2x6_8 | 1 | 2, 4, [0→2) | 2 | |
| ACEG 2x8 / 2x8_32 | 1 | 2, 4, [0→2) | 2 | |
| 1x4_8_Hot | 1 | 2, 4, [0→2) | 1 | |
| 2x2_8_Hot | 1 | 2, 4, [0→2) | 2 | |
| 2x4_16 | 1 | 2, 4, [0→2) | 2 | |
| PitchOver (any ACEG) | 2 | S0: 1, 4, [0→1); S1: 1, 4, [4→5) | 1 | 二段降級 |

#### 3.2.7 XDiv=10 (10×16 HP, ATK 專用)

| iInArmType | Seg | 各段 spacX, Col, ixRange | Row | 備註 |
|---|:---:|---|:---:|---|
| ACEG 2x4 | 2 | S0: 1, 4, [0→5); S1: 1, 2, [8→10) | 2 | 10_8 主段+餘數 |
| ACEG 2x5 | 2 | S0: 1, 4, [0→5); S1: 1, 2, [8→10) | 2 | |
| ACEG 2x8 / 2x8_32 | 2 | S0: 1, 4, [0→5); S1: 1, 2, [8→10) | 2 | |

#### 3.2.8 XDiv=12 (12×18/12×24 HP, Type B 大板)

| iInArmType | Seg | 各段 spacX, Col, ixRange | Row | 備註 |
|---|:---:|---|:---:|---|
| ep1Picker | 1 | 1, 1, [0→12) | 1 | |
| 2x6_8 (b12x16HP_2x6) | 1 | 2, 3, Kit{0,6,1,7} | 2 | **Kit 交錯模式** |
| ACEG 2x4 | 2 | S0: 2, 4, [0→2); S1: 1, 4, [8→9) | 2 | 12_8 主段+餘數 |
| ACEG 2x8 / 2x8_32 | 2 | S0: 2, 4, [0→2); S1: 1, 4, [8→9) | 2 | |

#### 3.2.9 XDiv=16 (16×24 HP, 32-Site 專用)

| iInArmType | Seg | Seg0: spacX, Col, ixRange | Row | 備註 |
|---|:---:|---|:---:|---|
| ACEG 2x8_32 | 1 | 4, 4, [0→4) | 2 | **統一走 CheckHasSpace** |
| 2x4_16 | 1 | 4, 4, [0→4) | 2 | |

---

## 4. 統一掃描引擎 — 核心演算法

### 4.1 擬碼 (Pseudocode)

```cpp
// SearchPlacePlate_New() — 取代所有 16 個 XItem 函式
void SearchPlacePlate_New()
{
    // === Phase 0: 前置條件 (不變) ===
    if (AutoSiteMap 相關條件) return;
    if (AutoSiteMapping + HotPlateSave) { /* 既有邏輯 */ return; }

    // === Phase 1: 計算 Y 半距 ===
    GetHotPlateYHalfPos();  // 設定 iYHalf, b6x20HP, b4x11HP, ...

    // === Phase 2: 建構掃描計畫 ===
    TScanPlan plan = BuildScanPlan(
        HotPlateForm.XDivision,
        HotPlateForm.YDivision,
        iInArmType,
        InArmSuck.iPickRow,
        InArmSuck.iPickCol,
        (i8PickerHPMode == iHPWideHP),
        bPitchOver12000,
        iYHalf,
        HotPlateYPitchCanPutAll(),
        HotPlateForm.iPlateSelect
    );

    // === Phase 3: 執行掃描 ===
    bool bFound = ExecuteScanPlan(plan);

    if (!bFound)
    {
        ShowErrorMessage("MES0156", 0, MInArmX, 0, "SearchPlateToPlace");
    }

    #ifndef SOFT_SIMULTE
    CheckSafeDoorIsClosed();
    #endif
}
```

### 4.2 ExecuteScanPlan — 統一掃描引擎

```cpp
bool ExecuteScanPlan(const TScanPlan& plan)
{
    int startPlate = 0;
    int endPlate   = 1;
    int plateStep  = 1;

    // Plate 方向
    if (!plan.bPlateForward)  // XItem16 反向
    {
        startPlate = 1;
        endPlate   = 0;
        plateStep  = -1;
    }

    // 單盤模式
    if (HotPlateForm.iPlateSelect == 0x01) { startPlate = 1; endPlate = 1; }
    if (HotPlateForm.iPlateSelect == 0x02) { startPlate = 0; endPlate = 0; }

    // === 逐段掃描 ===
    for (int s = 0; s < plan.nSegments; s++)
    {
        const TScanSegment& seg = plan.seg[s];

        // === 逐盤 ===
        for (int iPlate = startPlate; /* see below */; iPlate += plateStep)
        {
            // === 逐列 (Row) ===
            for (int iy = 0; iy < HotPlateForm.YDivision; /* iy 由內部控制 */)
            {
                // AutoClean 跳過
                if (plan.iAutoCleanSkipRows > 0 && iPlate == 0 &&
                    bRunAutoClean && TestIF_File.iAutoClean_Function &&
                    TestIF_File.iAutoClean_Tray == eCKPos_HP2 &&
                    iy < plan.iAutoCleanSkipRows)
                {
                    iy = plan.iAutoCleanSkipRows;
                }

                // === Row 決策 ===
                int nRow = plan.nSuckRow;
                int curSpacY = plan.spacY;

                if (nRow == 2)
                {
                    // 邊界檢查: Row2 是否超出 YDiv
                    if (iy + curSpacY >= HotPlateForm.YDivision)
                        nRow = 1;  // 自動降級 Row1
                }

                // === 逐欄 (Col shift) ===
                int ixCount = 0;
                int ixMax = (plan.bKitInterleave) ? 8 : (seg.ixEnd - seg.ixStart);

                for (int ixIdx = 0; ixIdx < ixMax; ixIdx++)
                {
                    int ix;
                    if (plan.bKitInterleave)
                    {
                        ix = plan.ixSequence[ixIdx];
                        if (ix == -1) break;  // 結尾
                    }
                    else
                    {
                        ix = seg.ixStart + ixIdx * seg.ixStep;
                        if (ix >= seg.ixEnd) break;
                    }

                    // === 核心: 空間檢查 ===
                    bool bOK = CheckHotPlateHasSpace_9045_8_New_V(
                        iPlate, iy, ix,
                        curSpacY, seg.spacX,
                        nRow, seg.nSuckCol, 0);

                    // Row2 失敗 → 嘗試 Row1
                    if (!bOK && plan.bRowFallback && nRow == 2)
                    {
                        bOK = CheckHotPlateHasSpace_9045_8_New_V(
                            iPlate, iy, ix,
                            curSpacY, seg.spacX,
                            1, seg.nSuckCol, 0);
                        if (bOK) nRow = 1;  // 降級成功
                    }

                    if (bOK)
                    {
                        // ★ 找到位置
                        iPlacePlateX[0] = ix;
                        iPlacePlateY[0] = iy;
                        iPlacePlate[0]  = iPlate;

                        // 計算偏移提示
                        ComputeStepHint(plan, seg, ix, iy, iPlate, nRow);

                        return true;
                    }
                }

                // 該 iy 全部 ix 都失敗，前進到下一列
                iy++;
            }

            // 該盤全滿，換盤
            if (HotPlateForm.iPlateSelect != 0x03) break;  // 單盤則不換
            if (iPlate == endPlate) break;
        }
    }

    return false;  // 全部段、全部盤都無空間
}
```

### 4.3 ComputeStepHint — 統一偏移計算

取代舊版零散的 `iForPlaceHPX3Step` / `iForPlaceHPX6Step` / `iForPlaceHPX10Step`。

```cpp
void ComputeStepHint(
    const TScanPlan& plan,
    const TScanSegment& seg,
    int ix, int iy, int iPlate, int nRow)
{
    // 讀取找到位置的實際格位狀態
    // 判斷有效吸嘴數（哪些吸嘴有 IC 可放）
    // 舊版:
    //   iForPlaceHPX6Step = (Item[0][0]==NULL && Item[0][2]==NULL) ? 1 : 0
    //   iForPlaceHPX10Step = 2 if left-half empty
    //   iForPlaceHPX3Step = 1 if Col2 only-single

    // 新版: 統一用 TStepHint
    TStepHint hint;
    hint.iXOffset = 0;
    hint.iYOffset = 0;
    hint.iActiveCols = seg.nSuckCol;

    // 檢查找到的區域中，哪些格位可用
    int nAvailable = 0;
    for (int c = 0; c < seg.nSuckCol; c++)
    {
        int col = ix + c * seg.spacX;
        if (col < HotPlateForm.XDivision &&
            MOT[MMPlate1 + iPlate].Tray.Data[col][iy] == NULL_IC)
        {
            nAvailable++;
        }
    }
    hint.iActiveCols = nAvailable;

    // 若左半無 IC、右半有 → 偏移
    // (這裡可以用通用的 left/right 半區邏輯)
    if (seg.nSuckCol >= 4 && nAvailable < seg.nSuckCol)
    {
        // 檢查是左半空還是右半空
        bool bLeftEmpty = true;
        bool bRightEmpty = true;
        int half = seg.nSuckCol / 2;

        for (int c = 0; c < half; c++)
        {
            int col = ix + c * seg.spacX;
            if (col < HotPlateForm.XDivision &&
                MOT[MMPlate1 + iPlate].Tray.Data[col][iy] != NULL_IC)
                bLeftEmpty = false;
        }
        for (int c = half; c < seg.nSuckCol; c++)
        {
            int col = ix + c * seg.spacX;
            if (col < HotPlateForm.XDivision &&
                MOT[MMPlate1 + iPlate].Tray.Data[col][iy] != NULL_IC)
                bRightEmpty = false;
        }

        if (bLeftEmpty && !bRightEmpty)
            hint.iXOffset = half;  // 偏移到右半
    }

    // 寫回全域（相容舊版移動函式）
    iForPlaceHPX6Step  = hint.iXOffset;
    iForPlaceHPX10Step = hint.iXOffset;
    iForPlaceHPX3Step  = (hint.iActiveCols < seg.nSuckCol) ? 1 : 0;
}
```

---

## 5. BuildScanPlan 查表實作

### 5.1 查表結構

```cpp
// 靜態查表 — 編譯期資料
struct TScanEntry
{
    int  xdiv;           // XDivision
    int  pickerMode;     // 0=1Suck, 1=AxEx1Row, 2=AxEx2Row, 3=ACEG, 4=WideHP_3, 5=WideHP_ACEG, 6=Kit2x6
    bool bPitchOver;     // bPitchOver12000 條件

    // Segment 0 (主段)
    int  seg0_spacX;
    int  seg0_col;
    int  seg0_ixStart;
    int  seg0_ixEnd;     // -1 = auto (由 XDiv 推算)
    int  seg0_ixStep;

    // Segment 1 (餘數段, nSeg>=2 時啟用)
    int  seg1_spacX;     // 0 = 無此段
    int  seg1_col;
    int  seg1_ixStart;
    int  seg1_ixEnd;
};

static const TScanEntry SCAN_TABLE[] =
{
    // --- XDiv=2 ---
    // xdiv, picker,  pitchOv, s0_spX, s0_col, s0_ixS, s0_ixE, s0_step, s1_spX, s1_col, s1_ixS, s1_ixE
    {  2,    0,       false,    1,      1,      0,      2,      1,       0,      0,      0,      0  },  // 1Suck
    {  2,    1,       false,    1,      2,      0,      1,      1,       0,      0,      0,      0  },  // AxEx 1Row
    {  2,    2,       false,    1,      2,      0,      1,      1,       0,      0,      0,      0  },  // AxEx 2Row
    {  2,    3,       false,    1,      2,      0,      1,      1,       0,      0,      0,      0  },  // ACEG (降級)

    // --- XDiv=3 ---
    {  3,    0,       false,    1,      1,      0,      3,      1,       0,      0,      0,      0  },  // 1Suck
    {  3,    1,       false,    1,      2,      0,      2,      1,       0,      0,      0,      0  },  // AxEx 1Row
    {  3,    2,       false,    1,      2,      0,      2,      1,       0,      0,      0,      0  },  // AxEx 2Row

    // --- XDiv=4 ---
    {  4,    0,       false,    1,      1,      0,      4,      1,       0,      0,      0,      0  },  // 1Suck
    {  4,    1,       false,    2,      2,      0,      2,      1,       0,      0,      0,      0  },  // AxEx 1Row
    {  4,    1,       true,     1,      1,      0,      4,      1,       0,      0,      0,      0  },  // AxEx 1Row PitchOver
    {  4,    2,       false,    2,      2,      0,      2,      1,       0,      0,      0,      0  },  // AxEx 2Row
    {  4,    3,       false,    1,      4,      0,      1,      1,       0,      0,      0,      0  },  // ACEG

    // --- XDiv=6 standard ---
    {  6,    0,       false,    1,      1,      0,      6,      1,       0,      0,      0,      0  },  // 1Suck
    {  6,    1,       false,    3,      2,      0,      3,      1,       0,      0,      0,      0  },  // AxEx 1Row
    {  6,    1,       true,     1,      1,      0,      6,      1,       0,      0,      0,      0  },  // AxEx 1Row PitchOver
    {  6,    2,       false,    3,      2,      0,      3,      1,       0,      0,      0,      0  },  // AxEx 2Row (CanPutAll)

    // --- XDiv=6 WideHP ---
    {  6,    4,       false,    1,      3,      0,      6,      3,       0,      0,      0,      0  },  // WideHP 3-Suck
    {  6,    5,       false,    1,      4,      0,      1,      1,       1,      2,      4,      6  },  // WideHP ACEG (2段)

    // --- XDiv=8 ---
    {  8,    0,       false,    1,      1,      0,      8,      1,       0,      0,      0,      0  },  // 1Suck
    {  8,    1,       false,    4,      2,      0,      4,      1,       0,      0,      0,      0  },  // AxEx 1Row
    {  8,    1,       true,     1,      1,      0,      8,      1,       0,      0,      0,      0  },  // AxEx PitchOver
    {  8,    2,       false,    4,      2,      0,      4,      1,       0,      0,      0,      0  },  // AxEx 2Row
    {  8,    3,       false,    2,      4,      0,      2,      1,       0,      0,      0,      0  },  // ACEG ★ 主力路徑
    {  8,    3,       true,     1,      4,      0,      1,      1,       1,      4,      4,      5  },  // ACEG PitchOver (2段)

    // --- XDiv=10 ---
    { 10,    3,       false,    1,      4,      0,      5,      1,       1,      2,      8,     10  },  // ACEG (2段)

    // --- XDiv=12 standard ---
    { 12,    3,       false,    2,      4,      0,      2,      1,       1,      4,      8,      9  },  // ACEG (2段)
    { 12,    6,       false,    2,      3,      0,      2,      1,       0,      0,      0,      0  },  // 2x6 Kit (ixSeq)

    // --- XDiv=16 ---
    { 16,    3,       false,    4,      4,      0,      4,      1,       0,      0,      0,      0  },  // ACEG 32-site
};
```

### 5.2 Picker Mode 分類函式

```cpp
int GetPickerMode(int iType, int iPickRow, bool bWideHP)
{
    if (USE_PICKER_COUNT == ep1Picker)     return 0;  // 1Suck
    if (iType == e9045_1x1_1)             return 0;
    if (iType == e9045_1x4_1_Ac)          return 0;

    // AxEx / AxxG 1~2 Row
    if (bUseAxExPicker() || bUseAxxGPicker())
    {
        if (iPickRow == 1) return 1;  // AxEx 1Row
        if (iPickRow == 2) return 2;  // AxEx 2Row
    }

    // WideHP
    if (bWideHP)
    {
        if (iType == e9045_1x3_4 ||
            iType == e9045_2x3_6 ||
            iType == e9045_2x6_8)
            return 4;  // WideHP 3-Suck
        else
            return 5;  // WideHP ACEG
    }

    // 12x16 Kit 模式
    if (b12x16HP_2x6) return 6;

    return 3;  // ACEG 通用
}
```

---

## 6. Edge Case 統一處理

### 6.1 統一到 BuildScanPlan 的修正

所有 edge case 在 `BuildScanPlan()` 中作為 **plan 修正** 處理，而非散布在引擎中。

| Edge Case Flag | 修正位置 | 修正內容 |
|---|---|---|
| `b6x20HP` | BuildScanPlan | `spacY = 5`（非標準 iYHalf） |
| `b4x11HP` | BuildScanPlan | Row 強制 = 1（奇數 YDiv 無法 2Row）|
| `b4x10HP_2x2` | BuildScanPlan | `spacY = 1`，Row 退化 |
| `b8x16HP_2x2` | BuildScanPlan | 特殊 iy 步進邏輯 |
| `b12x16HP_2x6` | BuildScanPlan | `pickerMode = 6`，啟用 Kit 交錯 |
| `bPitchOver12000` | SCAN_TABLE 查表鍵 | 直接匹配不同的表項 |

### 6.2 AutoClean 統一

```cpp
// 在 BuildScanPlan 中:
if (bRunAutoClean && TestIF_File.iAutoClean_Function &&
    TestIF_File.iAutoClean_Tray == eCKPos_HP2)
{
    plan.iAutoCleanSkipRows = 4;  // 統一: HP2 前 4 列保留給 CleanPad
}
```

### 6.3 PlateSelect 統一

```cpp
// 在 ExecuteScanPlan 中:
// iPlateSelect = 0x01 → 只用 HP1 (iPlate=1)
// iPlateSelect = 0x02 → 只用 HP2 (iPlate=0)
// iPlateSelect = 0x03 → 雙盤交替
// 已在 §4.2 的 startPlate/endPlate 邏輯中統一處理
```

---

## 7. 檔案結構規劃

```
ainarm_SearchPlacePlate_new.h      // 新增: TScanPlan, TScanSegment, TStepHint, SCAN_TABLE
ainarm_SearchPlacePlate_new.cpp    // 新增: BuildScanPlan, ExecuteScanPlan, ComputeStepHint
                                   //       SearchPlateToPlace_New()
ainarm_SearchPlacePlate.cpp        // 保留不動 (舊版)
ainarm_SearchPlacePlate.h          // 保留不動
```

### 7.1 新檔案行數預估

| 函式 / 區塊 | 預估行數 |
|---|---|
| 結構定義 (.h) | ~60 行 |
| SCAN_TABLE 靜態表 | ~80 行 |
| BuildScanPlan() | ~120 行（查表 + 修正） |
| GetPickerMode() | ~30 行 |
| ExecuteScanPlan() | ~100 行（統一掃描引擎） |
| ComputeStepHint() | ~60 行 |
| SearchPlateToPlace_New() | ~40 行（入口） |
| GetHotPlateYHalfPos() | 引用既有（不重寫） |
| HotPlateYPitchCanPutAll() | 引用既有（不重寫） |
| CheckHotPlateHasSpace_9045_8_New_V() | 引用既有（不重寫） |
| **合計** | **~490 行**（vs 舊版 4700+）|

---

## 8. 相容性與風險

### 8.1 iForPlaceHPXnStep 相容

舊版的 `iForPlaceHPX3Step`, `iForPlaceHPX6Step`, `iForPlaceHPX10Step` 被下游的
`MoveInArmXYToHotPlatePlace()` 和各 `ainarm9045_*.cpp` 讀取。

**方案**: `ComputeStepHint()` 仍然寫入這三個全域變數，確保下游不需修改。

### 8.2 MoveInArmXYToHotPlatePlace 不需修改

此函式只讀取 `iPlacePlateX[0]`, `iPlacePlateY[0]`, `iPlacePlate[0]`，
新版寫入相同的全域變數，因此完全相容。

### 8.3 潛在行為差異

| 項目 | 舊版行為 | 新版行為 | 影響 |
|------|---------|---------|------|
| XItem16 Plate 方向 | `iPlate--` (1→0) | 由 `bPlateForward=false` 控制 | 相同 |
| XItem16 手動掃描 | 不用 CheckHasSpace | 統一用 CheckHasSpace | 邏輯等價但實作不同 |
| 6_NotStandY 掃描順序 | x=0→1→2 穿插 | 統一改為線性 | 填充結果可能不同但填滿率相同 |
| 12_2x6 Kit 交錯 | 0→6→1→7 序列 | 由 ixSequence 驅動 | 相同 |
| Row2 失敗後行為 | 各函式各異 | 統一 bRowFallback | 可能更積極填充 |

### 8.4 風險評估

| 風險 | 等級 | 緩解措施 |
|------|------|----------|
| 掃描順序不同導致填充結果不同 | 中 | 設計驗證用 Python 腳本比對舊/新版填充結果 |
| Edge case 遺漏 | 中 | 對照 §6 清單逐一驗證 |
| SCAN_TABLE 表項遺漏 | 低 | 查表失敗時 fallback 到 MES0156 並紀錄 |
| 全域變數競爭 (多執行緒) | 低 | 同舊版，InArm 為單一執行緒 task |

---

## 9. 驗證方案

### 9.1 Python 模擬驗證

建立 Python 腳本模擬舊/新版的掃描邏輯，對所有 §19 × iInArmType 組合：

1. **初始化空白 HP 盤** (XDiv × YDiv 全 NULL_IC)
2. **反覆呼叫** SearchPlateToPlace（舊版） / SearchPlateToPlace_New
3. **記錄** 每次找到的 (ix, iy, iPlate)
4. **比對** 填滿率是否 100%（或達到物理上限）
5. **輸出差異報告**: 哪些組合新版掃描順序不同

### 9.2 比對矩陣 (162 組合)

```
§19 HP 配置數: 18 (含 A/B 型各尺寸)
iInArmType 數:  ~18 (扣除不適用的)
有效組合:       ~90 (依 §17.2)

每組合模擬 1 次完整填滿 = 90 次迭代
預計 Python 跑完 < 1 秒
```

---

## 10. 實作步驟

| 步驟 | 內容 | 產出 |
|------|------|------|
| **Step 1** | 建立 .h 檔：定義 TScanPlan, TScanSegment, TStepHint, SCAN_TABLE | `ainarm_SearchPlacePlate_new.h` |
| **Step 2** | 實作 BuildScanPlan + GetPickerMode | `.cpp` 查表函式 |
| **Step 3** | 實作 ExecuteScanPlan | `.cpp` 統一引擎 |
| **Step 4** | 實作 ComputeStepHint | `.cpp` 偏移計算 |
| **Step 5** | 實作 SearchPlateToPlace_New 入口 | `.cpp` |
| **Step 6** | Python 模擬腳本 — 對照舊版 | `scripts/verify_scan_plan.py` |
| **Step 7** | 逐一驗證 §19 × iInArmType 全組合 | 驗證報告 |
| **Step 8** | BCB6 編譯測試 | `#ifdef USE_NEW_SEARCH_PLATE` |

---

## 11. 開放問題（需與你確認）

| # | 問題 | 預設假設 |
|---|------|---------|
| 1 | `SearchPlacePlateXItem3_2Suck_3Site()` 的 Pitch 方向切換邏輯特殊（非單純的 CheckHasSpace），是否也要統一？ | 統一，但可能需額外參數 |
| 2 | `SearchPlacePlateXItem6_8Suck_NotStandY()` 使用 flag[2][2] 直接讀 Tray.Data 而非 CheckHasSpace，新版是否改用 CheckHasSpace？ | 改用，驗證等價性 |
| 3 | `PlaceSpecialPos()` 是否保留？它處理邊界列的特殊回退。 | 保留但移入 ExecuteScanPlan 作為 hook 呼叫 |
| 4 | `SetTraySingleData()` 呼叫（僅 3_2x2 和 12_2x6 有）是否移到引擎外？ | 是，由 caller (InArm) 統一呼叫 |
| 5 | ~~新版是否支援 Pick 模式（state=1）和 AutoClean 模式（state=3）？~~ | **已確認：不需要**。放置完成後路徑存入 `PickFromHPList` 結構（FIFO），吸取時直接依序讀取，不需重新搜尋。新版只做 Place（state=0）即可。 |
