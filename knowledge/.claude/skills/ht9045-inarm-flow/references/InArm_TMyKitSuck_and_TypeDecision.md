# InArm 流程 — TMyKitSuck 吸嘴結構與 iInArmType 決策

> **最後更新**：2026-04-07  
> **對應版本**：HT9011UC_Code_V3.33.900.0_20260331  
> **關鍵原始檔**：`mykitsuck.h`, `mykitsuck.cpp`, `ainarm9045.cpp`

---

## 1. 機台模式起點：TestIF_File.iTestMode

每次 InArm 流程開始前，**`TestIF_File.iTestMode`** 決定測試的拓樸結構：使用多少 Site、排列方式（1×N 或 M×N）。

```cpp
// TestMode.Data 中讀取，存入全域結構
// enum eTestMode (定義於 MachineType.h)
TestIF_File.iTestMode = SingleSite;   // 例：單站
TestIF_File.iTestMode = _16Site2X8;  // 例：2×8 站點
```

`TestIF_File.iTestMode` 是 `DoInArm_9045_Type()` 的主要判斷依據。

---

## 2. TMyKitSuck 吸嘴結構說明

`TMyKitSuck` 是描述「一組吸嘴（Kit Sucker）」的核心類別，同時描述：
- **吸嘴物理佈局**（Pick 時的矩陣）
- **Shuttle 放料佈局**（Sht = Shuttle 端）

### 2.1 關鍵成員變數

| 成員 | 意義 | 說明 |
|------|------|------|
| `iPickRow` | 取料行數（Row） | 從 Loader/HP 吸取時的行數 |
| `iPickCol` | 取料列數（Col） | 從 Loader/HP 吸取時的列數 |
| `iShtRow` | 放料行數（Row） | 放到 Shuttle 時的有效行數 |
| `iShtCol` | 放料列數（Col） | 放到 Shuttle 時的有效列數 |
| `iShtCnt` | 放料總數 | `iShtRow * iShtCol` |
| `iPickStep` | X 方向吸嘴間距倍數 | 13吸嘴：2（使用 Aa/Ac），14吸嘴：3（使用 Aa/Ad）；對應硬體 X 軸間距 4000~12000 (詳見 § 2.1.1) |
| `iPickKitStep` | Kit 間距倍數 | 吸嘴 Kit 間的間距 |
| `iShtKitStep` | Shuttle Kit 間距倍數 | Shuttle Kit 間的間距 |
| `iXStep` | X 方向需移動幾次才能放完 | 當 Shuttle X-Pitch > 吸嘴 X-Pitch 時，吸嘴上的 IC 需分多次 X 移動才能全部放到 Shuttle；由 `CheckPickerMode()` 根據 `dSiteXPitch` 動態設定（1/2/3/4）|
| `iYStep` | Y 方向放料次數 | 1=一次全放，2=分兩次放；由 `CheckInArmYStep()` 根據 `dSiteYPitch` 設定 |
| `iModeX` | X 非標準陣列模式 | 對應 XPHSuckToSht 陣列索引 |
| `Item[R][C]` | 各吸嘴狀態資料 | `NULL_IC` / `HAS_IC` / `HAS_HOT_IC` / 測試結果代碼 |
| `iWhichSht` | 目標 Shuttle 編號 | 決定本次放料要放到 Shuttle 1 或 Shuttle 2 |
| `iWhichKit` | 目標 Kit（左/右） | 決定放到 Shuttle 的左側 Kit 或右側 Kit |
| `iWhichShtPickFor32` | 32-Site 取料目標 Shuttle | 從 HotPlate 取料時（HP Pick）對應的 Shuttle 編號 |
| `iWhichKitPickFor32` | 32-Site 取料目標 Kit | 從 HotPlate 取料時對應的 Kit 左/右側 |

### 2.1.1 X 軸吸嘴間距硬體設定

根據機台設定檔 `Gerneral.ini` 的硬體配置：

| 設定項 | 設定值 | 對應吸嘴 | 物理距離 | 備註 |
|--------|--------|---------|---------|------|
| `USE_IN_OUT_ARM_X_PITCH` | 0（iXPitch60） | Aa → Ad（Column direction） | 4000 ~ 12000（0.1um 單位） | **固定值**，由硬體決定 |
| `IN_OUT_ARM_X_PITCH_MIN` | 4000 | — | 400mm（最小夾爪寬度） | — |
| `IN_OUT_ARM_X_PITCH_MAX` | 12000 | — | 1200mm（最大夾爪寬度） | — |

**說明**：
- 當 `USE_IN_OUT_ARM_X_PITCH=0` 時，InArm 吸嘴的 X 方向間距採用硬體預設值 **4000 ~ 12000**（0.1um 單位）
- 此值定義了吸嘴組 (Aa/Ac/Ab/Ad) 在夾爪上的最小與最大展開範圍
- 每組吸嘴間距相差一個 Sucker Pitch（40.0mm）根據iInArmXBase的位置決定與基準軸的位置, 假設iInArmXBase=2，則 **Aa(位置-80) → Ab(位置-40) → Ac(位置0) → Ad(位置40)**
- `iPickStep` 決定每次 Pick 時跳過的吸嘴數量（若 `iPickStep=2`，則只使用 Aa、Ac；若 `iPickStep=1`，則連續使用 Aa、Ab、Ac、Ad）

### 2.2 SetPickerCount 完整版

```cpp
// ainarm9045.cpp 中各子類型呼叫此函式設定吸嘴矩陣
void TMyKitSuck::SetPickerCount(
    int _iPickRow, int _iPickCol,   // 取料矩陣
    int _iShtRow,  int _iShtCol,    // 放到 Shuttle 的矩陣
    int _iPickStep,                 // X 吸嘴間距倍數（13: step=2, 14: step=3）
    int _iKitStep,                  // Kit 間距倍數
    int _iShtStep                   // Shuttle Kit 步距倍數
)

// 精簡版（用於 FLCarryKit, BLCarryKit, TestSuck 等）
void TMyKitSuck::SetPickerCount(int _iPickRow, int _iPickCol)
{
    iPickRow = iShtRow = _iPickRow;
    iPickCol = iShtCol = _iPickCol;
    iShtCnt  = iShtRow * iShtCol;
}
```

### 2.3 全域 TMyKitSuck 實例

| 實例名 | 用途 |
|--------|------|
| `InArmSuck` | InArm 吸嘴（進料臂） |
| `OutArmSuck` | OutArm 吸嘴（出料臂） |
| `FLCarryKit` | 前左 Shuttle（FLCarry）的 Kit |
| `BLCarryKit` | 後左 Shuttle（BLCarry）的 Kit |
| `FRCarryKit` | 前右 Shuttle（FRCarry）的 Kit |
| `BRCarryKit` | 後右 Shuttle（BRCarry）的 Kit |
| `FTestSuck` | 前方 Test Socket 吸嘴 |
| `BTestSuck` | 後方 Test Socket 吸嘴 |
| `TestSocket` | 測試頭 Socket（含 NN 模式補償） |

---

## 3. iPickRow / iPickCol 與 iShtRow / iShtCol 的三種關係

`iInArmType` 最終決定 InArm 每次吸取與放料的幾何比例。有以下三種可能：

### 情況 A：Pick 與 Shuttle 放料尺寸相同（1:1）

```cpp
// 典型：e9045_2x4_8  （SetInOutArmParameter_2x4_8()）
InArmSuck.SetPickerCount(2, 4, 2, 4, 1, 0, 0);
//                       PickRow=2, PickCol=4
//                       ShtRow=2,  ShtCol=4   ← Pick 幾何 == Shuttle 放料幾何
//                       iPickStep=1（吸嘴不跳格）
```

此模式一次從 Loader 吸 **2×4 個**，一次就能放滿 Shuttle 對應的 **2×4 格位**，Pick/Sht 完全對稱。
`iShtCol` 會依 TestMode 動態調整（`_10Site2X5` → 5，`_12Site2X6` → 6，`_16Site2X8` → 8），
對應 `_8Site2X4` 時預設為 **4**。

**規律**：`iPickRow == iShtRow`，`iPickCol == iShtCol`，`iXStep=1` 一次放完

---

### 情況 B：Pick 行數是 Shuttle 的 2 倍（Hot 模式雙行吸嘴）

Hot 模式下 InArm 同時吸取「兩行」IC，但每次只放「一行」到加熱盤 / Shuttle，
因此 `iPickRow = iShtRow * 2`。共有三種子類型：

#### B-1：`e9045_1x2_4_Hot`（DualSite 2-Site Hot 模式）

```cpp
// SetInOutArmParameter_1x2_4_Hot()
InArmSuck.SetPickerCount(2, 2, 1, 2, 2, 0, 0);
//                       PickRow=2, PickCol=2  ← 一次吸 2行×2列（共4顆）
//                       ShtRow=1,  ShtCol=2   ← 每次放 1行×2列 到 Shuttle/HP
//                       iPickStep=2（Aa/Ac 跳格排列）
```

#### B-2：`e9045_1x4_8_Hot`（QualSite1X4 Hot 模式）

```cpp
// SetInOutArmParameter_1x4_8_Hot()
InArmSuck.SetPickerCount(2, 4, 1, 4, 1, 0, 0);
//                       PickRow=2, PickCol=4  ← 一次吸 2行×4列（共8顆）
//                       ShtRow=1,  ShtCol=4   ← 每次放 1行×4列 到 Shuttle/HP
//                       iPickStep=1（吸嘴不跳格）
//                       PickRow 2行 分別對應 SHT1（Row1）/ SHT2（Row0）
```

> `GetNowSiteKitMode_1x4_8_Hot()` 依 `iSht==1` 決定使用 Row0 或 Row1 放料。

#### B-3：`e9045_2x2_8_Hot`（QualSite2X2 Hot 模式）

```cpp
// SetInOutArmParameter_2x2_8_Hot()
InArmSuck.SetPickerCount(2, 4, 2, 2, 1, 0, 0);
//                       PickRow=2, PickCol=4  ← 一次吸 2行×4列（8顆）
//                       ShtRow=2,  ShtCol=2   ← 每次放 2行×2列 到 HP（4顆）
//                       需兩次 Place 才填完 Pick 到的 8 顆
```

**規律**：`iPickCol = iShtCol * 2`（B-1/B-2 是 Row 之比，B-3 是 Col 之比）

---

### 情況 C：吸嘴 Pick 幾何＜Shuttle 格位（需多次 X 移動放完）

```cpp
// 典型：e9045_2x8_8  （SetInOutArmParameter_2x8_8()）
InArmSuck.SetPickerCount(2, 4, 2, 8, 1, 4, 4);
//                       PickRow=2, PickCol=4  ← 每次從 Loader 吸 2×4 個（8 顆）
//                       ShtRow=2,  ShtCol=8   ← Shuttle 有 2×8 個格位
//                       iPickStep=1（吸嘴不跳格）
//                       iKitStep=4, iShtStep=4 ← 第二次放料的 X 偏移量
```

InArm 一次從 Loader 吸 **2×4 = 8 顆**；Shuttle 格位是 **2×8 = 16 個**。
由於 **Shuttle X-Pitch 大於吸嘴 X-Pitch**，8 顆 IC 無法一次對齊 8 個格位，
因此 `CheckPickerMode()` 會依實際 Pitch 差設定 `iXStep`：

- `iXStep=4`：Shuttle X-Pitch 遠大於吸嘴 X-Pitch → 每次移動只放 2 顆，共 4 次才放完（2×8 站常見）
- `iXStep=2`：Shuttle X-Pitch 約為吸嘴 X-Pitch 的 2 倍 → 分 2 次放完
- `iXStep=1`：Pitch 相符 → 一次全放（罕見於 2x8 模式）

**`iXStep` 是由 `CheckPickerMode()` 在 recipe 載入後依 `dSiteXPitch` 動態設定，  
與 `SetPickerCount` 的參數無關。**

---

### 特殊情況

#### `e9045_1x4_1_Ac`（1×4 機台僅開啟 Ac Site）

```cpp
// 觸發條件（ainarm9045.cpp line ~1785）：
// QualSite1X4 + CosFunction.b1x4OnlyAaUse1x1Mode == true
// + iSiteMap[0][0/1/3] <= 0（只有 Ac（Site index=2）啟用）
iInArmType = e9045_1x4_1_Ac;
// InArm 使用 e9045_1x4_1 的 SetPickerCount，但 Sub-function 走 DoInArm_9045_1x1_1()
// CheckShuttleSensor_9045_1x1() 對此型別特殊處理，內部呼叫 CheckShuttleSensor_9045_1x4()
```

此為 4 站機台的降級模式：只有 Ac（第 3 個）Site 啟用，InArm 實質上以單站模式執行，
但仍使用 1x4 的位置參數對齊 Shuttle。

#### `USE_PICKER_COUNT == ep1Picker`（1 Picker 模式）

```cpp
// SetInOutArmParameter_All_1Pick()
InArmSuck.SetPickerCount(1, 1, 2, 2, 1, 0, 0);
//                       PickRow=1, PickCol=1  ← 每次只對一個吸嘴操作
//                       ShtRow=2,  ShtCol=2   ← Shuttle 期望的最終佈局
//                       iPickStep=1
```

`ep1Picker`（`rgPickerCount ItemIndex=4`）在 `DoInArm_9045()` Dispatch 前即被截獲，
強制路由至 `DoInArm_9045_All_1Pick()`，不進入一般的 iInArmType sub-function。
每次只對單一吸嘴執行 Pick/Place，依序巡迴填滿 2×2 Shuttle 格位。

---

## 4. DoInArm_9045_Type() — iInArmType 決策流程

> **Source**：`ainarm9045.cpp` line 1614

此函式在以下時機被呼叫：
- `cinitial.cpp` 初始化時
- `ainarm2.cpp` 入口（每次 recipe 切換後）
- AutoClean 開始前
- 機台設定變更後

### 決策邏輯（精簡版）

```
TestIF_File.iTestMode
├── SingleSite          → iInArmType = e9045_1x1_1
├── DualSite / 2X2NN    → 根據 iUseSuckMode / dSiteXPitch 決定
│   ├── iUseSuckMode==4 → e9045_1x2_4_Hot
│   └── else            → e9045_1x2_2_13 或 e9045_1x2_2_14（依 XPitch）
├── TriSite1X3 / 2X3NN  → e9045_1x3_4 或 e9045_1x3_2_14（依 CheckPickerMode）
├── QualSite1X4 / 2X4NN → 根據 iUseSuckMode / dSiteXPitch 決定
│   ├── iUseSuckMode==8 → e9045_1x4_8_Hot
│   ├── iModeX==13      → e9045_1x4_4_13
│   ├── iModeX==14      → e9045_1x4_2_14
│   └── else            → e9045_1x4_4 或 e9045_1x4_4_Back
├── DualSite2x1         → e9045_2x1_2_13
├── QualSite2X2         → e9045_2x2_4_xx 或 e9045_2x2_8_Hot（依 Pitch）
│   ├── SiteXPitch > 80mm                              → _14 (AxxG)
│   ├── HP XPitch>26.66mm 且 HP XItem==6               → _14 (AxxG, 20250827)
│   ├── HP XItem==12 且 HP XPitch×6>80mm               → _14 (AxxG, 20260512)
│   └── else                                           → _13 (AxEx)
├── _6Site2X3           → e9045_2x3_6 或 e9045_2x3_6_14
├── _8Site2X4           → e9045_2x4_8（預設），可降為 e9045_2x4_4_14
├── _10Site2X5          → e9045_2x5_8
├── _12Site2X6          → e9045_2x6_8
├── _16Site2X8          → e9045_2x8_8
└── _32Site4X8N         → e9045_2x8_32
```

### iInArmType 決策輔助函式

| 函式 | 說明 |
|------|------|
| `CheckPickerMode(4, true, false)` | 判斷是 13 排列（step=2）還是 14 排列（step=3）|
| `ArmXCanSuck4IC_9045()` | 當前 X-Pitch 允許 4 吸嘴嗎？ |
| `ArmXCanSuck2IC_9045S()` | 當前 X-Pitch（HT-9045S）允許 2 吸嘴嗎？ |
| `ArmYCanSuck2IC()` | 當前 Y-Pitch 允許雙行吸取嗎？ |
| `CheckInArmYStep()` | 決定 `InArmSuck.iYStep`=1 還是 =2 |

### iXStep — Shuttle X-Pitch 補償次數

`iXStep` 由 `CheckPickerMode(iPickCount, bSupport14Mode, bSupport13Mode)` 在 recipe 載入後設定，
**不是 `SetPickerCount` 的參數**。決定邏輯如下：

```
if(dSiteXPitch 在 iXpitchMinX1_MM ~ iXpitchMaxX1_MM 之間)  → iXStep=1（Pitch相符，一次放完）
if(dSiteXPitch 在 iXpitchMinX2_MM ~ iXpitchMaxX2_MM 之間)  → iXStep=2（Shuttle Pitch 2 倍）
if(dSiteXPitch > iXpitchMaxX2_MM)                          → iXStep=4（Shuttle Pitch 差距大）
特殊：e9045_2x8_8 固定 iXStep=4（JerryYang 20231003）
```

| iXStep | 物理意義 | 典型場景 |
|--------|----------|---------|
| 1 | Shuttle X-Pitch ≈ 吸嘴 X-Pitch，一次放完所有吸嘴上的 IC | 小 Pitch 模式 |
| 2 | Shuttle X-Pitch ≈ 吸嘴 X-Pitch × 2，半數吸嘴各移動一次 | 中 Pitch，如 13/14 排列 |
| 3 | 特殊 1x3 模式（Pitch ≥ 120mm，只開 Col 0/2） | `e9045_1x3_2_14` 特殊場景 |
| 4 | Shuttle X-Pitch >> 吸嘴 X-Pitch，逐顆或逐對放置 | **2x8/2x6 標準**，或 `iInArmToShtReleaseMode==1` |

### Shuttle 路由：iWhichSht / iWhichKit

`InArmSuck.iWhichSht`、`InArmSuck.iWhichKit` 是**每次放料動作前**由狀態機設定的路由變數：

| 變數 | 說明 |
|------|------|
| `InArmSuck.iWhichSht` | 本次吸嘴上的 IC 要放到哪個 Shuttle（0=SHT1，1=SHT2） |
| `InArmSuck.iWhichKit` | 放到 Shuttle 的哪一個 Kit（左側=0 或右側=1）|
| `InArmSuck.iWhichShtPickFor32` | 從 HotPlate 取料時，IC 預計放到的 Shuttle（HP Pick 用）|
| `InArmSuck.iWhichKitPickFor32` | 從 HotPlate 取料時，IC 預計放到的 Kit 左/右側（HP Pick 用）|

`DoInArmPlaceToShuttle_9045()` 讀取 `iWhichSht` 決定目標 Shuttle，  
讀取 `iWhichKit` 決定放入左 Kit 還是右 Kit（影響 `GetNowSiteKitMode_XXX()` 回傳值）。  

> `iWhichShtPickFor32` / `iWhichKitPickFor32` 在 Hot 模式中由 `SearchPlateToPick()` 設定，  
> 記錄「放到 HotPlate 這格 IC 的目標 Shuttle/Kit」，供 OutArm 取料後正確路由 Bin。

---

## 5. 關鍵：Shuttle 端如何使用 InArmSuck 參數

```cpp
// SetArmRowCount() 在 SetInOutArmParameter() 後呼叫
FLCarryKit.SetPickerCount(InArmSuck.iShtRow, InArmSuck.iShtCol);
BLCarryKit.SetPickerCount(InArmSuck.iShtRow, InArmSuck.iShtCol);
FTestSuck .SetPickerCount(InArmSuck.iShtRow, InArmSuck.iShtCol);
BTestSuck .SetPickerCount(InArmSuck.iShtRow, InArmSuck.iShtCol);

// TestSocket 的 Row/Col 在 NN 模式下要 ×2
if(IsNNMode()==None_NN)
    TestSocket.SetPickerCount(InArmSuck.iShtRow, InArmSuck.iShtCol);
else
    TestSocket.SetPickerCount(InArmSuck.iShtRow * 2, InArmSuck.iShtCol);
```

**重要**：Shuttle（FLCarryKit / BLCarryKit）的 Row/Col 是以 `InArmSuck.iShtRow/iShtCol` 為基準，不是 `iPickRow/iPickCol`。

---

## 6. InArm 整體流程概覽

### 6.1 常溫模式（`LastSet.iTemperature != Tempture_Hot`）

```
Loader (MMTrayY)
    ↓ DoInArmPickFromLoadStage_9045()
InArmSuck（HAS_IC）
    ↓ DoInArmAdditionalFunction()（可選：Precisor / Rotator / 2DID / DieClean）
    ↓ DoInArmPlaceToShuttle_9045()
FLCarryKit / BLCarryKit（Shuttle = FLCarryKit 或 BLCarryKit，HAS_IC）
```

### 6.2 加熱模式（`LastSet.iTemperature == Tempture_Hot`）

```
Loader (MMTrayY)
    ↓ DoInArmPickFromLoadStage_9045()
InArmSuck（HAS_IC）
    ↓ DoInArmAdditionalFunction()（可選）
    ↓ 必須先把吸嘴上的 IC 全部放掉（2.1）
    ↓ DoPlaceToHotPlate_9045()
HotPlate (MMPlate1 / MMPlate2)
    ├── [有空位] 繼續由 Loader 取料（2.2）
    └── [預熱完成 Temperature.fSoakTime] 根據 PickFromHPList（2.3）
            ↓ DoInArmPickFromHotPlate_9045()
            ↓ HAS_HOT_IC → InArmSuck
            ↓ DoInArmPlaceToShuttle_9045()
        FLCarryKit / BLCarryKit（HAS_HOT_IC → 轉為 HAS_IC 後送 IndexArm 測試）
```

### 6.3 main loop 關係

```
DoInArm()
  │
  ├─ HotPlate HangUp 檢查（iHPHangUpCount != 0 → 返回）
  │
  └─ DoInArm_9045()
       │
       └─ DoInArm_9045_XXX()  ← 由 iInArmType 選擇
            │
            case 100:  DoInArmPickFromLoadStage_9045()
            case 400:  DoInArmAdditionalFunction()
            case 1100: DoPlaceToHotPlate_9045()       ← Hot 模式
            case 1500: DoInArmPickFromHotPlate_9045()  ← Hot 模式
            case 2000: DoInArmPlaceToShuttle_9045()
```

---

## 7. 加熱模式詳細流程

### 7.1 放到 HotPlate（case 1100）

- 進入條件：`InArmSuck` 上有 `HAS_IC`，`DoInArm_9045_XXX()` 路由到 case 1100
- 目的：先把所有吸嘴上的 IC 全放到加熱盤
- 呼叫：`DoPlaceToHotPlate_9045()`（詳見 ht9045-hotplate SKILL）
- 放料後，呼叫 `PickFromHPList->UpdateHPSuckGroup()` 記錄本次放料位置與對應 Kit 資訊
- 放完後：`InArmSuck` 全部 `NULL_IC`，回到 case 50 繼續從 Loader 進料

### 7.2 PickFromHPList 資料結構

`PickFromHPList` 是記錄「已放到 HotPlate 的分組清單」的物件。每次放料後，一個新的 Group 加入。

| 操作 | 函式 | 呼叫時機 |
|------|------|---------|
| 更新已放位置 | `PickFromHPList->UpdateHPSuckGroup(iPlate, Y, X, iSht, iKit)` | DoPlaceToHotPlate 放料後 |
| 新增一個 Group | `PickFromHPList->AddHPSuckGroup()` | 一次放料動作完成後 |
| 取得可取料的 Group | `PickFromHPList->GetHPFirstTeamPlate(...)` | DoInArmPickFromHotPlate 前 |
| 取得最後一個有資料的 Team | `PickFromHPList->ExtractLastTeamHasData()` | ainarm2.cpp 判斷 HP 狀態用 |

### 7.3 從 HotPlate 取料（case 1500）

- 進入條件：HotPlate 上有 `HAS_HOT_IC`（預熱完成），且 `InArmSuck` 全部 `NULL_IC`
- 呼叫：`DoInArmPickFromHotPlate_9045()`
- 根據 `PickFromHPList` 決定本次取的位置（哪個 Plate、哪個 X/Y、哪個 Shuttle Kit）
- 取到後：`InArmSuck.Item[][]` = `HAS_HOT_IC`
- 隨後送往 Shuttle（case 2000）

### 7.4 加熱模式下 Shuttle 放料

- `DoInArmPlaceToShuttle_9045()` 中，`HAS_HOT_IC` 的 IC 放到 `FLCarryKit` 或 `BLCarryKit`
- 放完後 Shuttle 的 `Item[][]` 記錄為 `HAS_IC`（或保持 `HAS_HOT_IC`，等 Index Arm 消耗）

### 7.5 Hot Mode 的 HP 空位判斷

```
if(加熱盤有空位)
    → case 50（從 Loader 繼續進料）
else
    → case 500（等待預熱完成後取料）
```

---

## 8. iInArmType 與 SetPickerCount 對照表（常見型號）

| iInArmType | SetPickerCount(Pick- Row/Col, Sht- Row/Col, PickStep, KitStep, ShtStep) | 說明 |
|------------|------------------------------------------------------------------------|------|
| `e9045_1x1_1` | (1,1, 1,1, 1,0,0) | 單站，1 吸嘴 |
| `e9045_1x2_2_13` | (1,2, 1,2, 3,0,0) | 2 站，13 排，PickStep=3 |
| `e9045_1x2_2_14` | (1,2, 1,2, 2,0,0) | 2 站，14 排，PickStep=2 |
| `e9045_1x2_4_Hot` | (2,2, 1,2, 2,0,0) | 2 站熱模，Pick 2 行×2列，Sht 1 行×2列 |
| `e9045_1x4_4` | (1,4, 1,4, 1,2,2) | 4 站，標準，1 吸嘴對 1 site |
| `e9045_1x4_4_13` | (1,4, 1,4, 2,2,2) | 4 站，13 排，PickStep=2 |
| `e9045_1x4_8_Hot` | (2,4, 1,4, 1,0,0) | 4 站熱模，Pick 2 行，Sht 1 行 |
| `e9045_2x2_4_13` | (2,2, 2,2, 2,2,2) | 2×2 站，13 排 |
| `e9045_2x2_4_14` | (2,2, 2,2, 3,2,2) | 2×2 站，14 排（AxxG, jStep=3）|
| `e9045_2x2_8_Hot` | (2,4, 2,2, 1,0,0) | 2×2 熱模，Pick 4 列，Sht 2 列 |
| `e9045_2x4_8` | (2,4, 2,4, 1,2,2) | 2×4 站，8 吸嘴 |
| `e9045_2x8_8` | (2,4, 2,8, 1,4,4) | 2×8 站，Pick 4 列，Sht 8 列，分兩次放 |
| `e9045_2x8_32` | 同 e9045_2x8_8 | 32 Site NN 模式 |

---

## 9. Item 狀態碼快查

| 代碼 | 意義 |
|------|------|
| `NULL_IC` | 無 IC（空位） |
| `HAS_NULL_IC` | 感應到有 IC 但資料未知（HAS＋NULL混合） |
| `HAS_IC` | 有 IC，尚未預熱 |
| `HAS_HOT_IC` | 有 IC，預熱已完成（Hot 模式） |
| `HAS_TESTING_IC` | IC 正在測試中（已放到 Socket） |
| `START_TEST` 以上 | IC 已完成測試，數值代表測試結果 |
| `CLEAN_FINISH_IC` | AutoClean 用的 dummy IC 狀態 |

---

## 10. 常見問題索引

| 問題 | 查看 |
|------|------|
| iPickRow 與 iShtRow 不一樣?原因? | 第 3 節（三種關係）|
| 為什麼有些模式要跑兩次才放滿 Shuttle？ | 第 3 節情況 A，`iXStep=2` |
| 決定 iInArmType 的邏輯在哪裡？ | 第 4 節 DoInArm_9045_Type |
| 加熱盤的 IC 怎麼知道預熱完了？ | 第 7.3 節；詳見 ht9045-hotplate SKILL |
| PickFromHPList 是什麼？ | 第 7.2 節 |
| InArmSuck 的 Item 什麼時候變成 NULL_IC？ | 第 7.1 節（放料後），第 7.3 節（載入後變 HAS_HOT_IC）|
| Shuttle Kit 的 Row/Col 怎麼決定？ | 第 5 節 SetArmRowCount |
| `iPickStep` 13 和 14 代表什麼？ | 第 2.2 節（PickStep）；13=吸嘴第1/3位，14=吸嘴第1/4位 |

---

## 相關知識庫連結

| Skill | 關聯點 |
|-------|-------|
| [HotPlate 放料路徑知識庫（整合於 inarm-flow SKILL）](../SKILL.md#hotplate-放料路徑知識庫) | HotPlate 放料路徑、PickFromHPList 詳解、WAR0150/0151、JAM0109 |
| [ht9045-inarm-flow SKILL](file:///d:\HT9045\.github\skills\ht9045-inarm-flow\SKILL.md) | InArm 完整狀態機 case 說明 |
| [ht9045-shuttle-flow SKILL](file:///d:\HT9045\.github\skills\ht9045-shuttle-flow\SKILL.md) | Shuttle 接收 IC 後的流程 |
| [ht9045-mode-naming-convention](file:///d:\HT9045\.github\skills\ht9045-mode-naming-convention.md) | eTestMode 對應 TestSiteFileName 標準名稱 |
| `d:\HT9045\.github\instructions\ht9045-mode-naming.instructions.md` | eTestMode 對應 TestSiteFileName 標準名稱 |
