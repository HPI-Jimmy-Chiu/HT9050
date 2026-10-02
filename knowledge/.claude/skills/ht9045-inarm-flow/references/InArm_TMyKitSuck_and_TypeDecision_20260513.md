# InArm / OutArm — TMyKitSuck 吸嘴結構、類型決策、基準軸與 Pitch 計算

> **最後更新**：2026-05-13（v3，整合 OutArm 類型路由 + SetPickerCount 完整對照）
> **對應版本**：HT9011UC_Code_V3.33.904.2_20260511_RogerYang
> **關鍵原始檔**：`mykitsuck.h`, `mykitsuck.cpp`, `ainarm9045.cpp`, `aoutarm9045.cpp`, `database.cpp`, `cinitial.cpp`, `cmydef.cpp`, `LastSet.h`
> **歷史版本**：v1（2026-04-07，基於 V3.33.900.0_20260331）→ v2（2026-05-13，補基準軸 / Pitch）→ **v3**（2026-05-13，整合 OutArm）

---

## v2→v3 重大補充摘要

| 章節 | 內容 | 來源 |
|------|------|------|
| §7 | Y-Pitch Modular 5 種模式定義（`iXPitch60` / `iXPitchManual635` / `iXYPitchVariable` / `iXPitchManual360` / `iXYPitchRowA` 等） | `cmydef.cpp` L3094-3102 |
| §8 | 基準軸 `iInArmXBase` / `iInArmYBase` 設定邏輯，依 `USE_IN_OUT_ARM_Y_PITCH` 切換 | `database.cpp` L786-940 |
| §8.3 | **InArm ↔ OutArm 基準軸對稱關係**（固定→兩臂同 col2；可變XY→InArm 在 col2、OutArm 在 col1，鏡像對稱） | `database.cpp` L786-940 |
| §9 | Teach 點位結構（`Tech.iInArmLoadStageX/Y/Z2`, `iInArmZHeightSub[2][4]`）與基準軸的關係 | `LastSet.h`, `cinitial.cpp` L8637 |
| §10 | Loader 取料絕對位置計算公式（`Prod.XInArm_Tray_Pick[base]` + col × Tray.iXPitch + iInArmXBase × dInArmXPitch_1Step） | `ainarm9045.cpp` L4660-4670 |
| §11 | HP 取放料縮 Pitch 數學模型（`iBaseXToHP` / `iHPBasePos` / `iPickStep1Pos`），對應 PPT Slide 23-26 | `ainarm9045.cpp`, `cinitial.cpp` |
| §12 | `dInArmXPitch_1Step` / `dInArmXPitch_MovePitch` 與 `InArmClose_PitchX` 關係（吸嘴間距 = Tray IC 間距 ÷ 步數） | `ainarm9045.cpp` L4476-4610 |
| §13 | Eastsun 20251231 修正：`Prod.XInArm_Tray_Pick[i][j]` 一律先設為基準位置，個別吸嘴 X/Y Offset 移到 `GetInArmToLoaderPosition` 才加（`GetArmX(i,j)`） | `cinitial.cpp` L8678 |
| **§19** | **OutArm 類型路由** — OutArm 無獨立 `iOutArmType`，由 `iInArmType` 直接路由 `DoOutArm_9045_Xxx()` | `aoutarm9045.cpp` L540-630 |
| **§20** | **OutArm SetPickerCount 完整對照表** — 26 種 iInArmType 對應的 InArm/OutArm 共用參數 | 各 `ainarm9045_*.cpp` |

---

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
| 基準軸是哪一顆吸嘴？InArm/OutArm 對稱嗎？ | 第 11 節（Y-Pitch 模式）、第 12 節（InArm↔OutArm 對稱）|
| Teach 位置存哪裡？怎麼推算到所有吸嘴？ | 第 13 節（Teach 點位結構）、第 14 節（Loader 公式） |
| HP 縮 Pitch 怎麼算？ | 第 15 節（HP 縮 Pitch 數學模型） |
| `dInArmXPitch_1Step` 是什麼？ | 第 16 節（X-Pitch 馬達與單步距離） |

---

## 11. Y-Pitch Modular 模式定義（`USE_IN_OUT_ARM_Y_PITCH`）

| 值 | 常數名 | UI 名稱（cSetUp 頁面） | Y 機構 / Pitch 馬達數 |
|---|---|---|---|
| 0 | `iXPitch60` | Fixed Y Pitch 60mm | 固定機構，**僅 XP1** 一軸 X-Pitch |
| 1 | `iXPitchManual635` | Manual Y Pitch 60 & 63.5mm | 固定機構（手動換間距片），**僅 XP1** |
| 2 | `iXYPitchVariable` | Auto Y Pitch In Bc / Out Bb | **XP1 + XP2 + YP** 三軸（可變 X+Y） |
| 3 | `iXPitchManual360` | Manual Y Pitch 60 & 36mm | 固定機構（手動換間距片），**僅 XP1** |
| 4 | `iXYPitchRowA` | Auto Y Pitch in Row A | 可變 Y（Row A 基準）|
| 5 | `iXYPitch16Picker` | Auto Y Pitch 16 Picker @ Bd | HT-1032 16 picker |
| 6 | `iXYPitchBb` | Auto Y Pitch @ Bb (HT7080) | HT-7080 |
| 7 | `iXYPitch16Bd_Be` | (HT-1032 變種) | HT-1032 |
| 8 | `iXYPitchIn_Bb_Out_Bc` | (HT-1132 變種) | HT-1132 |

> **檔案位置**：`cmydef.cpp` L3094-3102（const 定義）、`cmydef.h` L2848-2856（extern 宣告）
> **HT-9xxx 常用範圍**：值 0~4。值 5/6/7/8 屬其他機型（不在本技能範圍）。
> **JerryYang 20251218 新增**：`USE_OUT_ARM_Y_PITCH` 獨立於 `USE_IN_OUT_ARM_Y_PITCH`，允許 IN/OUT ARM 使用不同 Pitch 模組（`database.cpp` L933-940 覆蓋 OutArm 基準軸）。

---

## 12. 基準軸 — 「全部 Teach 點位的參考原點」

### 12.1 基準軸概念

```
吸嘴矩陣（HT-9xxx 8 吸嘴實體 = 2 Row × 4 Col）：

  j=0   j=1   j=2   j=3
┌─────┬─────┬─────┬─────┐
│  A  │  C  │  E  │  G  │  i=0 (Row 0)
│[0,0]│[0,1]│[0,2]│[0,3]│
├─────┼─────┼─────┼─────┤
│  B  │  D  │  F  │  H  │  i=1 (Row 1)
│[1,0]│[1,1]│[1,2]│[1,3]│
└─────┴─────┴─────┴─────┘
```

**基準軸 = `InArmSuck.Suck[iInArmYBase][iInArmXBase]`**

所有 Teach 點位（`Tech.iInArmLoadStageX/Y/Z`, `iInArmPlate1X/Y`...）儲存的都是「基準軸吸嘴」對應的馬達座標。其他 7 顆吸嘴的座標由基準軸 + 個別 Z 高差 + Offset 推算而得。

### 12.2 InArm 基準軸（依 `USE_IN_OUT_ARM_Y_PITCH`）

| Y-Pitch 模式 | `iInArmYBase` | `iInArmXBase` | InArm 基準吸嘴 |
|---|---|---|---|
| `iXPitch60` (固定 60) | 0 | 2 | **E** [0,2] |
| `iXPitchManual635` (60/63.5) | 0 | 2 | **E** [0,2] |
| `iXYPitchVariable` (可變 X+Y) | **1** | 2 | **F** [1,2] |
| `iXPitchManual360` (60/36) | 0 | 2 | **E** [0,2] |
| `iXYPitchRowA` (RowA 可變 Y) | 0 | 2 | **E** [0,2] |
| `iXYPitch16Picker` (HT-1032) | 1 | 3 | (16 吸嘴拓樸，另立技能) |
| `iXYPitchBb` (HT-7080) | 1 | 1 | (HT-7080，另立技能) |

> **可變 X+Y 模式為何基準改到 Row 1（F）**：因為 YP 馬達需要一個固定端參考，Row 1 是 YP 馬達的固定參考端，Row 0 隨 YP 變動。基準必須選在「YP 不會動」的那一側，才能讓 Teach 位置不受 Y-Pitch 變化影響。
> **Source**：`database.cpp` L786-940（依 `USE_IN_OUT_ARM_Y_PITCH` 分支設定）

### 12.3 OutArm 基準軸 — 與 InArm 鏡像對稱

| Y-Pitch 模式 | `iOutArmYBase` | `iOutArmXBase` | OutArm 基準吸嘴 | 對稱關係 |
|---|---|---|---|---|
| `iXPitch60` (固定 60) | 0 | 2 | **E** [0,2] | 兩臂同位置（col 2） |
| `iXPitchManual635` | 0 | 2 | **E** [0,2] | 兩臂同位置 |
| `iXYPitchVariable` (可變 X+Y) | **1** | **1** | **D** [1,1] | **InArm=col 2、OutArm=col 1（鏡像）** |
| `iXYPitchRowA` | 0 | **1** | **C** [0,1] | InArm=col 2、OutArm=col 1（鏡像）|
| `iXYPitchBb` (HT-7080) | 1 | **2** | (HT-7080) | InArm=col 1、OutArm=col 2（鏡像）|

> **物理意義**：InArm 與 OutArm **面朝相反方向**安裝在機台上（InArm 朝 Loader、OutArm 朝 Unloader），可變 X+Y Pitch 機構也是鏡像安裝。從機台「全域座標」看 InArm 的基準軸在 col 2，鏡像到 OutArm 就會落在「col 1」（4 顆吸嘴鏡射：col 0↔3, col 1↔2，由於是奇數對應，可變模式選擇 col 2 → col 1）。
> **Source**：`database.cpp` L786-940

### 12.4 各模式基準軸視覺化（HT-9xxx，2x4 = 8 吸嘴）

```
固定 Y-Pitch（iXPitch60 / iXPitchManual635 / iXPitchManual360 / iXYPitchRowA）：

InArm：                              OutArm（鏡像）：
  A   C   ★E★  G   ← Row 0          A   C   ★E★  G   ← Row 0
  B   D    F   H   ← Row 1          B   D    F   H   ← Row 1
  ↑                                  ↑
  XP1                                XP1
                                     （兩臂基準軸都在 E[0,2]）

可變 X+Y Pitch（iXYPitchVariable）：

InArm：                              OutArm（鏡像）：
  A   C   E   G   ← Row 0           A   C   E   G   ← Row 0
  B   D  ★F★  H   ← Row 1           B  ★D★  F   H   ← Row 1
  ↑       ↑                          ↑   ↑
  XP1     YP                         XP1 YP
                                     （InArm 基準在 F[1,2]，OutArm 基準在 D[1,1]，鏡像對稱）
```

★ 表示基準軸吸嘴。

---

## 13. Teach 點位結構（`LastSet.h` Tech 結構 → `Prod` 全域）

### 13.1 Setup 頁面只 Teach 基準軸

工程師在 Setup 頁面實際 Teach 的只有「基準軸吸嘴」對應的馬達位置：

| Tech 結構欄位 | 意義 | 對應馬達 |
|---|---|---|
| `Tech.iInArmLoadStageX` | InArm 基準軸在 Loader Stage 的 X 馬達位置 | `MInArmX` |
| `Tech.iInArmLoadStageY` | InArm 基準軸在 Loader Stage 的 Y 馬達位置 | `MInArmY` |
| `Tech.iInArmLoadStagePickZ1/Z2` | InArm 基準軸 Z 馬達吸取高度（Z1/Z2 兩段） | `MInArmZE`（或可變模式下的 `MInArmZF`）|
| `Tech.iInArmPlate1X/Y/PickZ1` | InArm 基準軸在 HotPlate1 的位置 | 同上 |
| `Tech.iInArmShuttle1X/Y` | InArm 基準軸在 InShuttle1 的位置 | 同上 |
| `Tech.iInArmZHeightSub[2][4]` | **其他 7 顆吸嘴與基準軸的 Z 高差**（0.1um） | `MInArmZA~ZH`（除基準軸外）|
| `Tech.TechInArmPitchX/Y` | XP1/YP 馬達 Teach 位置 | `MInArmPitch`, `MInArmPitchY` |
| `Tech.iInArmX40Pitch/X120Pitch` | XP1 馬達在 40mm / 120mm pitch 時的位置 | `MInArmPitch` |
| `Tech.iInArmX40Pitch2/X120Pitch2` | XP2 馬達（可變模式）在 40/120mm pitch 的位置 | `MInArmPitchX2` |
| `Tech.iInArmRearOffsetX/Y` | 後排（Row 1）相對 Row 0 的 XY 偏移 | — |

### 13.2 個別吸嘴的 Z 高度差（`iInArmZHeightSub[2][4]`）

```
                     j=0    j=1    j=2     j=3
              ┌──────┬──────┬──────┬──────┐
  i=0 (Row0)  │ ZA-Zbase│ ZC-Zbase│  0    │ ZG-Zbase│  ← Z 基準(E)=[0,2]，自己差值=0
              ├──────┼──────┼──────┼──────┤
  i=1 (Row1)  │ ZB-Zbase│ ZD-Zbase│ ZF-Zbase│ ZH-Zbase│
              └──────┴──────┴──────┴──────┘
```

工程師「Teach Z 高度差」的操作：
1. 先讓基準軸（E 或 F）吸到 Tray IC，記錄 `iInArmLoadStagePickZ2`
2. 再讓 A/B/C/D/F/G/H 各自吸到對應 IC，記錄相對於基準軸的差值到 `iInArmZHeightSub[i][j]`

> **16 吸嘴擴充**：`Tech.iInArmZHeightSub_16[2][4]` 儲存 col 4-7 的 Z 高差（`LastSet.h` L1003）。

### 13.3 初始化時推算所有吸嘴座標（`cinitial.cpp` L8637+）

```cpp
//------------------------------------------------------------
// 步驟 1：先算「基準軸」的絕對 X/Y/Z 座標
//------------------------------------------------------------
Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase] =
    Tech.iInArmLoadStageX
    + LoadForm->XStart + LoadForm->BlockXStart - KitPitchX
    + InArmOffSet[InOfsLoader]->GetX()
    + Prod.iTrayKitStartX;

Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase] =
    Tech.iInArmLoadStageY - LoadForm->YStart - LoadForm->BlockYStart
    + KitPitchY + InArmOffSet[InOfsLoader]->GetY() - Prod.iTrayKitStartY;

Prod.ZInArm_Tray_Pick[iInArmYBase][iInArmXBase] =
    Tech.iInArmLoadStagePickZ2 + InArmOffSet[InOfsLoader]->GetPickUp();

//------------------------------------------------------------
// 步驟 2：其他吸嘴 Z 高度 = 基準軸 Z + 個別差值 + 個別 Offset
//------------------------------------------------------------
for(int i=0; i<InArmSuck.iMaxRow; i++)
  for(int j=0; j<InArmSuck.iMaxCol; j++) {
    if(i==iInArmYBase && j==iInArmXBase) continue;

    iTemp = (j<4) ? Tech.iInArmZHeightSub[i][j]
                  : Tech.iInArmZHeightSub_16[i][j-4];

    Prod.ZInArm_Tray_Pick[i][j]  = Prod.ZInArm_Tray_Pick[base] + iTemp
                                  + InArmOffSet[InOfsLoader]->GetPickUp(i, j);
    Prod.ZInArm_Tray_Place[i][j] = Prod.ZInArm_Tray_Place[base] + iTemp
                                  + InArmOffSet[InOfsLoader]->GetPlace(i, j);

    //------------------------------------------------------------
    // ★ Eastsun 20251231 修正：
    //   非基準軸的 X/Y 一律先設為「基準軸位置」，
    //   個別吸嘴的 X/Y Offset 移到 GetInArmToLoaderPosition() 才用 GetArmX(i,j) 加上去
    //   理由：iPickRow=1 的 Fix 模式無法判斷使用哪顆吸嘴，
    //         若初始化時就加 Offset，會把錯誤吸嘴的 Offset 加到當下使用的吸嘴上
    //------------------------------------------------------------
    Prod.XInArm_Tray_Pick[i][j] = Prod.XInArm_Tray_Pick[base];
    Prod.YInArm_Tray_Pick[i][j] = Prod.YInArm_Tray_Pick[base];
  }
```

---

## 14. Loader / HP / Shuttle 取放料 X/Y 公式

### 14.1 Loader 取料（多吸嘴版，`ainarm9045.cpp` L4660）

```cpp
// GetInArmToLoaderPosition(iSelRow, iXPos, iYPos, iRow, iCol)
// iRow / iCol 是 Tray 上 IC 的索引
//
// X 公式：
iXPos = Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase]   // 基準軸 Teach 位置
      + iCol * Prod.LoadForm.iXPitch                       // 跨 Tray IC × 幾列
      + iInArmXBase * dInArmXPitch_1Step;                  // 基準軸到「第 0 顆吸嘴」的 Pitch 偏移

// Y 公式：
iYPos = Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase]
      - iRow * Prod.LoadForm.iYPitch;
```

> **`+iInArmXBase × dInArmXPitch_1Step` 的含意**：MInArmX 馬達指的是「基準軸正下方」位置。要讓 A 吸嘴對齊 Tray[*][0]、E 吸嘴對齊 Tray[*][2]，就要把 MInArmX 往右偏 `iInArmXBase × 1step` 距離。

### 14.2 Loader 取料（單吸嘴 Fix 模式，`ainarm9045.cpp` L4704）

```cpp
// GetInArmToLoaderPosition_Single() — 1×1 / Fix 模式使用
// 用「實際使用的吸嘴」反推 X 位置（吸嘴位置 ≠ 基準軸位置）：
iXPos = Prod.XInArm_Tray_Pick[iSelRow][iCurrSuck]
      + iCol * Prod.LoadForm.iXPitch
      + (iInArmXBase - iRealUseSuck) * dInArmXPitch_1Step;
//      ↑↑↑ 把基準軸 → 實際使用吸嘴 的偏移補回去
```

### 14.3 個別吸嘴 X/Y Offset（Eastsun 20251231 修正後）

```cpp
// 在 GetInArmToLoaderPosition() 最後加上：
iXPos += InArmOffSet[InOfsLoader]->GetArmX(i, j);
iYPos += InArmOffSet[InOfsLoader]->GetArmY(i, j);

// 其中：
double ARM_OFFSET::GetArmX(int iX, int iY) {
  if(bOneByOne)
      return SingleOffSet->dPosOffSetX[iX][iY];  // 每吸嘴獨立 Offset（cprod.h L103）
  else
      return -dArmX;                              // 整組共用 Offset
}
```

---

## 15. HotPlate 縮 Pitch 數學模型（PPT Slide 23-26）

當 HotPlate 物理 XPitch ≠ Tray XPitch 時，需要縮 XP1 馬達使吸嘴間距對齊 HP：

```cpp
iHPBasePos    = HP.XStart - iBaseXToHP;            // 基準軸到 HP 邊緣
iPickStep1Pos = iHPBasePos + HP.XPitch;            // 1x2 / 2x2 mode vs 3-Col HP
              = iHPBasePos + HP.XPitch * 2;        // 1x4 / 2x4 mode vs 6-Col HP
```

| 符號 | 來源 | 意義 |
|---|---|---|
| `BASE_X_TO_HP` | `database.cpp` L944-950 | 基準軸到 HP 邊緣的距離（HT-9xxx 預設 ≥ 6800，HT1028 ≤ 4000）|
| `iHPBasePos` | 計算結果 | 假設吸嘴 step=1 時的 HP 右側起始位置 |
| `iPickStep1Pos` | 計算結果 | 縮 Pitch 後第一步要對應的 X 位置 |
| `HP.XStart` | Recipe HotPlate.Data | HP X 起始座標 |
| `HP.XPitch` | Recipe HotPlate.Data | HP 上 IC 之間 X 間距 |

> **`BASE_X_TO_HP` 的邊界**：`database.cpp` 強制下限與上限，避免機構碰撞。

---

## 16. X-Pitch 馬達與「單步距離」（`ainarm9045.cpp` L4476-4610）

### 16.1 兩個關鍵全域變數

| 變數 | 意義 |
|---|---|
| `dInArmXPitch_1Step` | **相鄰兩顆吸嘴之間的 X 距離**（XP1 馬達在「閉合」狀態時，A-C-E-G 之間的單一 Pitch）|
| `dInArmXPitch_MovePitch` | XP1 馬達展到最開時 A→G 的「總距離」（= 1Step × (N-1)，N=4 → 3×1Step） |

### 16.2 計算公式

```cpp
// AutoCalculateInArmXClosePitch() — recipe 載入後呼叫
// iInArmXStep = Tray XDivision / 2 或對應的 N 值
InArmClose_PitchX = UserDefForm[Ld].XPitch * iInArmXStep;
//                  Tray IC X-Pitch × 跨幾顆 IC

dInArmXPitch_1Step    = InArmClose_PitchX / 3.0;  // 4 吸嘴模式，A→G 跨 3 步
dInArmXPitch_MovePitch = dInArmXPitch_1Step * 3.0; // = InArmClose_PitchX
```

對於 16 吸嘴模式（`USE_16PICKER_TYPE==1`），除數變 6 / 7：

```cpp
dInArmXPitch_1Step    = InArmClose_PitchX / 6.0;   // 8 吸嘴橫向，跨 6 步
dInArmXPitch_MovePitch = dInArmXPitch_1Step * 7.0;
```

### 16.3 X-Pitch 機構模式（`USE_IN_OUT_ARM_X_PITCH`）

| 值 | 常數名 | XP1 行程範圍 |
|---|---|---|
| 0 | `iXPitch40mm` | 4000 ~ 12000（A→G 40-120mm）|
| 1 | `iXPitch50mm` | 4000 ~ 15000（A→G 40-150mm）|
| 2 | `iXPitchAuto` | 由 `IN_OUT_ARM_X_PITCH_MIN/MAX` 動態決定 |
| 3 | `iXPitch16Pick` | 7700 ~ 15400（HT-1032）|

> **Source**：`cmydef.cpp` L3103-3106、`database.cpp` L962-1018

---

## 17. OutArm 對稱版（鏡像） — 取放料公式參考

OutArm 公式與 InArm 完全平行，只是基準軸不同：

| InArm 變數 | OutArm 對應 |
|---|---|
| `iInArmXBase` / `iInArmYBase` | `iOutArmXBase` / `iOutArmYBase` |
| `dInArmXPitch_1Step` | `dOutArmXPitch_1Step` |
| `Prod.XInArm_Tray_Pick[i][j]` | `Prod.XOutArm_Tray_Place[i][j]` 等 |
| `InArmOffSet[]` | `OutArmOffSet[]` |
| `MInArmX/Y/Pitch/ZA~ZH` | `MOutArmX/Y/Pitch/ZA~ZH` |
| `MInArmPitchY` (YP) | `MOutArmPitchY` |
| `MInArmPitchX2` (XP2) | `MOutArmPitchX2` |

**取放料公式**（出料）：

```cpp
// OutArm 從 Shuttle 取料的公式（aoutarm9045.cpp 系列）
iXPos = Prod.XOutArm_Shuttle_Pick[iOutArmYBase][iOutArmXBase]
      + iCol * SHT.XPitch
      + iOutArmXBase * dOutArmXPitch_1Step;
```

> **詳見** [ht9045-outarm-flow](file:///d:\HT9045\.github\skills\ht9045-outarm-flow\SKILL.md) 的對稱章節。

---

## 18. v1 → v2 修正記錄（缺漏與錯誤補充，v2 版保留）

| # | v1 狀態 | v2 修正 | 證據 |
|---|---|---|---|
| 1 | 未提及 Y-Pitch Modular 5 種模式 | 補上 §11 完整表（`iXPitch60` ~ `iXYPitchIn_Bb_Out_Bc`） | `cmydef.cpp` L3094-3102 |
| 2 | 未提及 `iInArmXBase` / `iInArmYBase` | 補上 §12 基準軸概念與依模式切換的對照表 | `database.cpp` L786-940、`cmydef.cpp` L3878 預設值 (XBase=2, YBase=0) |
| 3 | InArm 與 OutArm 基準軸關係未說明 | 補上 §12.3 **鏡像對稱關係**：固定模式同位置，可變模式 InArm=F[1,2]、OutArm=D[1,1] | `database.cpp` L808-810, L825-828 |
| 4 | Teach 結構（`iInArmZHeightSub`）未說明 | 補上 §13.2 Z 高度差陣列與 Teach 流程 | `LastSet.h` L589, L1003 |
| 5 | Loader 取料公式（含基準軸 Offset）未列出 | 補上 §14.1/§14.2 多吸嘴 / 單吸嘴公式 | `ainarm9045.cpp` L4660, L4704 |
| 6 | HP 縮 Pitch 數學模型未列出 | 補上 §15 公式 + `BASE_X_TO_HP` 邊界 | `database.cpp` L944-950 |
| 7 | `dInArmXPitch_1Step` 沒有與 `InArmClose_PitchX` 關聯說明 | 補上 §16 計算公式 + 4 吸嘴 vs 16 吸嘴除數差異 | `ainarm9045.cpp` L4476-4610 |
| 8 | Eastsun 20251231 修正（個別 Offset 移到計算階段）未記錄 | 補上 §13.3 與 §14.3 修正前後對比 | `cinitial.cpp` L8678 |
| 9 | OutArm 對稱公式未提及 | 補上 §17 OutArm 對應變數對照表 | `aoutarm9045.cpp`, `aoutarm.h` |
| 10 | `USE_OUT_ARM_Y_PITCH` 獨立於 `USE_IN_OUT_ARM_Y_PITCH` 未說明（JerryYang 20251218） | 補上 §11 註腳：IN/OUT ARM 允許不同 Pitch 模組 | `database.cpp` L933-940 |

---

## 19. OutArm 類型路由 — `DoOutArm_9045()` Dispatch（`aoutarm9045.cpp` L540）

### 19.1 關鍵結論：OutArm 沒有獨立的 `iOutArmType`

**OutArm 直接根據 `iInArmType` 決定呼叫哪個 `DoOutArm_9045_Xxx()` 函式**，不像 InArm 有 `DoInArm_9045_Type()` 這種獨立決策函式。

InArm 的類型決策涉及 TestMode、Pitch、HotPlate、USE_PICKER_COUNT 等多重因素；OutArm 只需對應 InArm 已決定的結果。

### 19.2 `DoOutArm_9045()` 路由表

```cpp
// aoutarm9045.cpp L540-630
void DoOutArm_9045()
{
    // ... Guard Checks（Servo Off / Destroy 確認 / QA Mode / Pause 等）...

    if(USE_PICKER_COUNT==ep1Picker)
        DoOutArm_9045_All_1Picker();
    else if(iInArmType==e9045_1x1_1 || iInArmType==e9045_1x4_1_Ac)
        DoOutArm_9045_1x1_1();
    else if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_1x2_2_13)
        DoOutArm_9045_1x2_2();
    else if(iInArmType==e9045_1x2_4_Hot)
        DoOutArm_9045_1x2_4();
    else if(iInArmType==e9045_1x3_2_14)
        DoOutArm_9045_1x3_2_14();
    else if(iInArmType==e9045_1x3_4)
        DoOutArm_9045_1x3_4();
    else if(iInArmType==e9045_1x4_2_14)
        DoOutArm_9045_1x4_2();
    else if(iInArmType==e9045_1x4_4_13)
        DoOutArm_9045_1x4_4S();
    else if(iInArmType==e9045_1x4_4_Back)
        { /* 空實作 */ }
    else if(iInArmType==e9045_1x4_4)
        DoOutArm_9045_1x4_4();
    else if(iInArmType==e9045_1x4_8_Hot)
        DoOutArm_9045_1x4_8();
    else if(iInArmType==e9045_2x1_2_13)
        DoOutArm_9045_2x1_2();
    else if(iInArmType==e9045_2x2_4_12 || ...4_13 || ...4_14)
        DoOutArm_9045_2x2_4();
    else if(iInArmType==e9045_2x2_8_Hot)
        DoOutArm_9045_2x2_8();
    else if(iInArmType==e9045_2x3_6_14)
        DoOutArm_9045_2x3_6_14();
    else if(iInArmType==e9045_2x3_6)
        DoOutArm_9045_2x3_6();
    else if(iInArmType==e9045_2x4_4_13 || ...4_14)
        DoOutArm_9045_2x4_4();
    else if(iInArmType==e9045_2x4_8)
        DoOutArm_9045_2x4_8();
    else if(iInArmType==e9045_2x5_8)
        DoOutArm_9045_2x5_8();
    else if(iInArmType==e9045_2x6_8)
        DoOutArm_9045_2x6_8();
    else if(iInArmType==e9045_2x8_8 || iInArmType==e9045_2x8_32)
        DoOutArm_9045_2x8_8();
}
```

### 19.3 OutArm Guard Checks（InArm 沒有的）

| Guard Check | 說明 |
|---|---|
| `bTestingStopAllMotor` | 測試中停止所有馬達（InArm 也有） |
| `bMyServoOffOutArm` | 發生 JAM 後 Servo Off，避免重啟後位置偏移（JerryYang 20161227）|
| `CheckOutArmDestroyActive()` | 確認 Destroy 吹氣完成（InArm 對應 `CheckInArmDestroyActive`）|
| `Check_QA_ModeUnloadCount()` | Maxim QA Mode 出料數量限制（JerryYang 20221004）|
| `CheckOutArmAutoAlignmentTrayModeBeUse()` | Auto Alignment CCD 模式（KenHsieh 20210813）|

### 19.4 InArm vs OutArm 類型決策差異

| 面向 | InArm | OutArm |
|------|-------|--------|
| **決策函式** | `DoInArm_9045_Type()`（復雜多重判斷） | **無**（直接讀 `iInArmType`） |
| **決策依據** | TestMode + Pitch + HotPlate + USE_PICKER_COUNT + HP XItem/XPitch | 僅 `iInArmType` 一個變數 |
| **SetPickerCount** | 在各 `SetInOutArmParameter_Xxx()` 設定 | 與 InArm **共用同一函式** |
| **額外輔助函式** | `CheckPickerMode()`, `ArmXCanSuck4IC_9045()`, `ArmYCanSuck2IC()` | 無 |
| **合併路由** | 某些型號走專屬函式（如 `DoInArm_9045_1x4_4_Back`） | `e9045_1x4_4_Back` 是空實作 |

> **設計思維**：InArm 決定「這次跑料用哪種吸嘴配置」，OutArm 只需「用相同配置把 IC 從 Shuttle 取出放到 Tray」。所有 Pitch/HotPlate/TestMode 的複雜判斷集中在 InArm 端，OutArm 是「純粹的 follower」。

---

## 20. InArm / OutArm SetPickerCount 完整對照表

`SetPickerCount(PickRow, PickCol, ShtRow, ShtCol, PickStep, KitStep, ShtStep)` — InArm 與 OutArm **共用同一組參數**（在 `SetInOutArmParameter_Xxx()` 中同時設定）。

| # | iInArmType | PickRow | PickCol | ShtRow | ShtCol | PickStep | KitStep | ShtStep | 說明 |
|---|---|---|---|---|---|---|---|---|---|
| 0 | `e9045_1x1_1` | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 單站 |
| 1 | `e9045_1x4_1_Ac` | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 1x4 降級單站（Ac only） |
| 2 | `e9045_1x2_2_13` | 1 | 2 | 1 | 2 | 2 | 0 | 0 | 2 站 13 排（Aa/Ac） |
| 3 | `e9045_1x2_2_14` | 1 | 2 | 1 | 2 | 3 | 0 | 0 | 2 站 14 排（Aa/Ad） |
| 4 | `e9045_1x2_4_Hot` | 2 | 2 | 1 | 2 | 2 | 0 | 0 | 2 站 Hot（Pick 2行、Sht 1行） |
| 5 | `e9045_1x3_2_14` | 1 | 2 | 1 | 3 | 3 | 2 | 2 | 3 站 14 排 |
| 6 | `e9045_1x3_4` | 1 | 3 | 1 | 3 | 1 | 0 | 0 | 3 站標準 |
| 7 | `e9045_1x4_2_14` | 1 | 2 | 1 | 4 | 3 | 2 | 2 | 4 站 14 排 |
| 8 | `e9045_1x4_4` | 1 | 4 | 1 | 4 | 1 | 0 | 0 | 4 站標準 |
| 9 | `e9045_1x4_4_13` | 2 | 2 | 1 | 4 | 2 | 0 | 0 | 4 站 HT-9045S |
| 10 | `e9045_1x4_4_Back` | 1 | 4 | 1 | 4 | 1 | 0 | 0 | 4 站背列（OutArm 空實作）|
| 11 | `e9045_1x4_8_Hot` | 2 | 4 | 1 | 4 | 1 | 0 | 0 | 4 站 Hot（Pick 2行、Sht 1行）|
| 12 | `e9045_2x1_2_13` | 1 | 2 | 2 | 1 | 1 | 0 | 0 | 2x1 站 |
| 14 | `e9045_2x2_4_12` | 2 | 2 | 2 | 2 | 1 | 0 | 0 | 2x2 站 12 排 |
| 15 | `e9045_2x2_4_13` | 2 | 2 | 2 | 2 | 2 | 0 | 0 | 2x2 站 13 排（Aa/Ac）|
| 16 | `e9045_2x2_4_14` | 2 | 2 | 2 | 2 | 3 | 0 | 0 | 2x2 站 14 排（Aa/Ad）|
| 17 | `e9045_2x2_8_Hot` | 2 | 4 | 2 | 2 | 1 | 0 | 0 | 2x2 Hot（Pick 4列、Sht 2列）|
| 18 | `e9045_2x3_6` | 2 | 4 | 2 | 3 | 1 | 0 | 0 | 2x3 站 |
| 19 | `e9045_2x3_6_14` | 2 | 2 | 2 | 3 | 3 | 2 | 2 | 2x3 站 14 排 |
| 20 | `e9045_2x4_4_13` | 2 | 2 | 2 | *iShtCol* | 2 | 2 | 2 | 2x4 站 HT-9045S 13 排 |
| 21 | `e9045_2x4_4_14` | 2 | 2 | 2 | *iShtCol* | 2 | 2 | 2 | 2x4 站 14 排 |
| 22 | `e9045_2x4_8` | 2 | 4 | 2 | *iShtCol* | 1 | 0 | 0 | 2x4 站 8 吸嘴 |
| 23 | `e9045_2x5_8` | 2 | 4 | 2 | 5 | 1 | 4 | 3 | 2x5 站（10 Site）|
| 24 | `e9045_2x6_8` | 2 | 4 | 2 | 6 | 1 | 4 | 3 | 2x6 站（12 Site）|
| 25 | `e9045_2x8_8` | 2 | 4 | 2 | 8 | 1 | 4 | 4 | 2x8 站（16 Site）|
| 26 | `e9045_2x8_32` | 同上 | — | — | — | — | — | — | 32 Site NN 模式 |

> **`*iShtCol*`**：動態值，依 TestMode 中的 Site 數量決定（`_8Site2X4` → 4、`_10Site2X5` → 5 等）。

### 20.1 InArm → OutArm 函式名對應

| iInArmType | InArm 函式 | OutArm 函式 |
|---|---|---|
| `e9045_1x1_1` / `e9045_1x4_1_Ac` | `DoInArm_9045_1x1_1()` | `DoOutArm_9045_1x1_1()` |
| `e9045_1x2_2_13` / `_14` | `DoInArm_9045_1x2_2()` | `DoOutArm_9045_1x2_2()` |
| `e9045_1x2_4_Hot` | `DoInArm_9045_1x2_4_Hot()` | `DoOutArm_9045_1x2_4()` |
| `e9045_1x3_2_14` | `DoInArm_9045_1x3_2_14()` | `DoOutArm_9045_1x3_2_14()` |
| `e9045_1x3_4` | `DoInArm_9045_1x3_4()` | `DoOutArm_9045_1x3_4()` |
| `e9045_1x4_2_14` | `DoInArm_9045_1x4_2()` | `DoOutArm_9045_1x4_2()` |
| `e9045_1x4_4_13` | `DoInArm_9045S_1x4_4()` | `DoOutArm_9045_1x4_4S()` |
| `e9045_1x4_4` | `DoInArm_9045_1x4_4()` | `DoOutArm_9045_1x4_4()` |
| `e9045_1x4_8_Hot` | `DoInArm_9045_1x4_8_Hot()` | `DoOutArm_9045_1x4_8()` |
| `e9045_2x1_2_13` | `DoInArm_9045_2x1_2()` | `DoOutArm_9045_2x1_2()` |
| `e9045_2x2_4_12/13/14` | `DoInArm_9045_2x2_4()` | `DoOutArm_9045_2x2_4()` |
| `e9045_2x2_8_Hot` | `DoInArm_9045_2x2_8_Hot()` | `DoOutArm_9045_2x2_8()` |
| `e9045_2x3_6` | `DoInArm_9045_2x3_6()` | `DoOutArm_9045_2x3_6()` |
| `e9045_2x3_6_14` | `DoInArm_9045_2x3_6_14()` | `DoOutArm_9045_2x3_6_14()` |
| `e9045_2x4_4_13/14` | `DoInArm_9045_2x4_4()` | `DoOutArm_9045_2x4_4()` |
| `e9045_2x4_8` | `DoInArm_9045_2x4_8()` | `DoOutArm_9045_2x4_8()` |
| `e9045_2x5_8` | `DoInArm_9045_2x5_8()` | `DoOutArm_9045_2x5_8()` |
| `e9045_2x6_8` | `DoInArm_9045_2x6_8()` | `DoOutArm_9045_2x6_8()` |
| `e9045_2x8_8/32` | `DoInArm_9045_2x8_8()` | `DoOutArm_9045_2x8_8()` |

> **命名差異**：InArm 的 Hot 後綴（`_Hot`）在 OutArm 省略（如 `_1x2_4_Hot` → `_1x2_4`）。OutArm 不需區分 Hot/非 Hot，因為 IC 到了 Shuttle 已完成預熱。

### 20.2 OutArm 主狀態機概覽（與 InArm 對稱）

| InArm 步驟 | OutArm 對應 |
|---|---|
| `DoInArmPickFromLoadStage_9045()` — 從 Loader Tray 取料 | `DoPickFromShuttle_9045()` — 從 Shuttle 取料 |
| `DoInArmAdditionalFunction()` — Precisor / Rotator / 2DID | `DoOutArmAdditionalFunction()` — AOI / Rotator / Fix AI CCD |
| `DoPlaceToHotPlate_9045()` — 放到 HotPlate | ×（OutArm 沒有 HotPlate 動作） |
| `DoInArmPickFromHotPlate_9045()` — 從 HotPlate 取料 | ×（OutArm 沒有 HotPlate 動作） |
| `DoInArmPlaceToShuttle_9045()` — 放到 Shuttle | `DoOutArmPlaceToAuto_9045()` — 放到 Auto Tray |
| × | `DoOutArmPlaceToFix_9045()` — 放到 Fix Tray（Bin 分類） |
| × | `DoOutArmPlaceToMagazine_9045()` — 放到 Magazine |

> OutArm 特有的 **Bin 分類邏輯**（Auto Tray、Fix Tray、Magazine、BinBox 擇一放料）是 InArm 沒有的核心差異，但屬於 [ht9045-outarm-flow SKILL](file:///d:\HT9045\.github\skills\ht9045-outarm-flow\SKILL.md) 的範圍，本報告不再展開。

---

## 21. 本次技能更新摘要（2026-05-13）

本報告 v2 完成後，同步將「基準軸 / Teach 結構 / 取放料公式 / InArm↔OutArm 鏡像對稱」等知識回寫至以下 3 個 SKILL 檔，使 AI 輔助開發時能自動取得正確的取放料計算上下文。

### 19.1 `ht9045-sucker-architecture` SKILL 更新

**檔案**：`d:\HT9045\.github\skills\ht9045-sucker-architecture\SKILL.md`

| 新增章節 | 內容摘要 |
|----------|----------|
| §4.3 基準軸 | 補上 Y-Pitch 9 種模式完整表（`iXPitch60` ~ `iXYPitchIn_Bb_Out_Bc`）、InArm 基準軸依模式對照（固定→E、可變→F）、OutArm 鏡像對照（固定→E、可變→D）、ASCII 視覺化（固定 vs 可變）、「為什麼可變模式基準選 Row 1」說明 |
| §4.4 Teach 點位結構 | Tech 結構關鍵欄位表（含馬達對應）、`iInArmZHeightSub[2][4]` Z 高度差陣列視覺化 |
| §4.5 取放料公式 | `cinitial.cpp` 初始化推算程式碼（含 Eastsun 20251231 修正）、多吸嘴 Loader 取料公式、單吸嘴 Fix 模式公式、X-Pitch 單步公式（4 吸嘴 vs 16 吸嘴除數差異）、OutArm 對稱變數對應表 |
| Frontmatter | description 新增觸發關鍵字：`iInArmXBase`, `iOutArmXBase`, `USE_IN_OUT_ARM_Y_PITCH`, `dInArmXPitch_1Step`, `GetArmX`, `Eastsun 20251231`, `基準軸`, `鏡像對稱` 等 |

### 19.2 `ht9045-inarm-flow` SKILL 更新

**檔案**：`d:\HT9045\.github\skills\ht9045-inarm-flow\SKILL.md`

| 新增章節 | 內容摘要 |
|----------|----------|
| InArm 基準軸（取放料計算原點） | 基準軸概念簡述、Y-Pitch 模式 5 種對照表（`iInArmYBase` / `iInArmXBase` / 基準吸嘴）、可變模式選 F 的原因、與 OutArm 鏡像的交叉參照 |
| Loader 取料公式 | `ainarm9045.cpp` L4660 多吸嘴版公式（含 `+iInArmXBase × dInArmXPitch_1Step` 意義說明）、`GetArmX(i,j)` 個別吸嘴 Offset |
| X-Pitch 單步距離 | `InArmClose_PitchX` / `dInArmXPitch_1Step` / `dInArmXPitch_MovePitch` 三者關係公式 |

### 19.3 `ht9045-outarm-flow` SKILL 更新

**檔案**：`d:\HT9045\.github\skills\ht9045-outarm-flow\SKILL.md`

| 新增章節 | 內容摘要 |
|----------|----------|
| OutArm 基準軸（與 InArm 鏡像對稱） | 4 種 Y-Pitch 模式 OutArm 基準表（`iOutArmYBase` / `iOutArmXBase` / 對稱關係）、`USE_OUT_ARM_Y_PITCH` 獨立覆蓋（JerryYang 20251218）、物理意義（InArm 朝 Loader / OutArm 朝 Unloader 面朝相反）、可變 X+Y ASCII 視覺化 |
| OutArm 取放料公式 | Shuttle 取料公式（`Prod.XOutArm_Shuttle_Pick[base]` + col × Pitch + base × 1Step）、X-Pitch 單步距離（`dOutArmXPitch_1Step = OutArmClose_PitchX / 3.0`）|
| InArm↔OutArm 馬達對應表 | `MInArmX/Y/Pitch/ZA~ZH` ↔ `MOutArmX/Y/Pitch/ZA~ZH`、`MInArmPitchY` ↔ `MOutArmPitchY`、`MInArmPitchX2` ↔ `MOutArmPitchX2` |

### 19.4 更新前後差異總結

| 知識面向 | 更新前（v1 技能） | 更新後（v2 技能） |
|----------|-------------------|-------------------|
| 基準軸定義 | 未記錄 | 3 份 SKILL 均有基準軸表 + 視覺化 |
| Y-Pitch 模式 | 僅 sucker-architecture 提及模式名 | 完整 9 種模式對照 + 基準軸對應 |
| InArm↔OutArm 對稱 | 未記錄 | 鏡像對稱關係、物理意義、ASCII 圖 |
| Teach 結構 | 未記錄 | Tech 欄位表 + Z 高差陣列 + 基準軸推算 |
| 取放料公式 | 未記錄 | Loader / HP / Shuttle 三段公式 + Eastsun 修正 |
| X-Pitch 計算 | 未記錄 | `1Step` / `MovePitch` / `InArmClose_PitchX` 完整鏈路 |
| OutArm 公式 | 未記錄 | OutArm 對稱版公式 + 變數對應表 |

---

## 相關知識庫連結

| Skill | 關聯點 |
|-------|-------|
| [HotPlate 放料路徑知識庫（整合於 inarm-flow SKILL）](../SKILL.md#hotplate-放料路徑知識庫) | HotPlate 放料路徑、PickFromHPList 詳解、WAR0150/0151、JAM0109 |
| [ht9045-inarm-flow SKILL](file:///d:\HT9045\.github\skills\ht9045-inarm-flow\SKILL.md) | InArm 完整狀態機 case 說明 |
| [ht9045-outarm-flow SKILL](file:///d:\HT9045\.github\skills\ht9045-outarm-flow\SKILL.md) | OutArm 基準軸鏡像對稱 + 取放料公式 |
| [ht9045-sucker-architecture SKILL](file:///d:\HT9045\.github\skills\ht9045-sucker-architecture\SKILL.md) | §4.3-4.5 基準軸 / Teach 結構 / 取放料公式完整推導 |
| [ht9045-shuttle-flow SKILL](file:///d:\HT9045\.github\skills\ht9045-shuttle-flow\SKILL.md) | Shuttle 接收 IC 後的流程 |
| [ht9045-mode-naming-convention](file:///d:\HT9045\.github\skills\ht9045-mode-naming-convention.md) | eTestMode 對應 TestSiteFileName 標準名稱 |
