---
name: ht9045-sucker-architecture
description: HT-9xxx 系列（HT9045 / HT9046 / HT9011UC，InArm/OutArm 各 8 吸嘴）吸嘴模組軟體架構知識庫（Steven Chou 2023.07 軟體重構）。當使用者詢問 TMySucker / TMyKitSuck 結構成員、iPickRow / iPickCol / iShtRow / iShtCol / iPickStep / iPickKitStep / iShtKitStep / iXStep / iYStep / iModeX 意義、In/Out Suck 預先定義（A~H 對應 Item[0..1][0..3]）、實體位置 vs 迴圈位置、馬達數量與吸嘴數量為何分開指定、為什麼要全模式共用 Function、Loader 共用兩個 Function 設計（MoveArmXYToLoaderStage / DoMoveArmXYToLoaderStage / GetInArmToLoaderPosition / GetInArmToLoaderPosition_Single）、HotPlate 縮 Pitch 數學模型（iBaseXToHP / iHPBasePos / iPickStep1Pos / HP.XStart / HP.XPitch）、Shuttle 的 GetNowSiteKitMode Mode 編碼規則（Mode/100 = X、Mode%100 = Y）、CheckXYPitch Teaching 計算（X Teaching → E to Center → A to G Pitch → Site Aa/Ac 位置）、Auto Clean 在 Shuttle Mode 的特殊處理、IO_Table.csv / Mot_Table.csv 表格驅動的硬體選配、**基準軸 iInArmXBase/iInArmYBase 與 iOutArmXBase/iOutArmYBase 設定（依 USE_IN_OUT_ARM_Y_PITCH 切換）**、**InArm↔OutArm 基準軸鏡像對稱關係（固定模式同 col 2、可變模式 InArm=F[1,2]/OutArm=D[1,1]）**、**Y-Pitch Modular 5 種模式（iXPitch60 / iXPitchManual635 / iXYPitchVariable / iXPitchManual360 / iXYPitchRowA）**、**Tech 結構 Teach 點位（iInArmLoadStageX/Y/Z2、iInArmZHeightSub[2][4]）**、**Loader 取料絕對位置公式（基準位置 + iCol×Tray.iXPitch + iInArmXBase×dInArmXPitch_1Step）**、**Eastsun 20251231 修正（個別 Offset 移到 GetInArmToLoaderPosition 才加）** 等問題時，應先載入此技能。觸發關鍵字：TMySucker, TMyKitSuck, iPickRow, iPickCol, iShtRow, iShtCol, iPickStep, iPickKitStep, iShtKitStep, iXStep, iYStep, iModeX, GetNowSiteKitMode, CheckXYPitch, iBaseXToHP, iHPBasePos, iPickStep1Pos, MoveArmXYToLoaderStage, DoMoveArmXYToLoaderStage, GetInArmToLoaderPosition, GetInArmToLoaderPosition_Single, IO_Table.csv, Mot_Table.csv, 邏輯機台, Steven 軟體重構, 吸嘴拓樸, 吸嘴模組, A~H 吸嘴, 全模式共用, 縮 Pitch, Teaching 計算, iInArmXBase, iInArmYBase, iOutArmXBase, iOutArmYBase, USE_IN_OUT_ARM_Y_PITCH, USE_OUT_ARM_Y_PITCH, iXPitch60, iXYPitchVariable, iXYPitchRowA, iXPitchManual635, dInArmXPitch_1Step, dInArmXPitch_MovePitch, InArmClose_PitchX, iInArmZHeightSub, Tech.iInArmLoadStageX, XInArm_Tray_Pick, InArmOffSet, OutArmOffSet, GetArmX, GetArmY, Eastsun 20251231, 基準軸, 鏡像對稱, BASE_X_TO_HP。
---

<!-- AI(W906-BA-SKILL) 20260915：這支 skill 來自網頁同事 20260915 交付的
     D:\HT9050\HT9045_skills_20260915\skills\ht9045-sucker-architecture，內容未改。
     ⚠ 資料通道以使用者 20260915 裁決為準：wb_serve HTTP 直讀 .Data +
       recipe.doc.put 寫。本 skill 若描述 JSON-only 或 C# Simulator 通道，
       那是同事那邊的舊架構，不是本專案的正式路徑。 -->
<!-- AI(W906-BA-SKILL) 20260915：本檔提到 D:\AI_TempFile，那些路徑在 D:\HT9045 這台機器上**不存在**（或已凍結）。
     產生器請改用本 skill 自帶的 scripts\；引用那些路徑的段落只當史料讀。 -->

# HT-9xxx 吸嘴模組軟體架構（InArm / OutArm 各 8 吸嘴）

> **來源**：Steven Chou《邏輯機台_軟體架構.pptx》(2023.07)
> **基準版本**：`HT9011UC_Code_V3.33.904.2_20260511_RogerYang`（V3.33.8XX+ 後皆適用）
> **適用機型範圍**：**HT-9xxx 系列**（HT9045 / HT9046 / HT9011UC 等，InArm/OutArm 各 8 吸嘴 = 2 Row × 4 Col）
> **不在本技能範圍**：HT-1032AT（16 吸嘴）、HT-7080B、HT-9016 / HT-9115 等其他拓樸機型，若需要請另立技能

---

## 0. 與現有技能的範圍切分（避免內容重複）

本技能聚焦於 Steven Chou 2023.07 軟體重構 PPT 所定義的「**吸嘴類別結構**、**全模式共用 Function 設計**、**HP 縮 Pitch 數學模型**、**Teaching 推導**」。下表列出與既有技能的分工，便於確認彼此不重疊：

| 範疇 | 既有技能負責 | 本技能負責 |
|------|--------------|-----------|
| InArm 狀態機 case 流程 | ht9045-inarm-flow | 吸嘴類別結構、重構脈絡 |
| OutArm 狀態機 case 流程 | ht9045-outarm-flow | `TMyKitSuck` 全模式共用設計 |
| Shuttle `Do_Auto_SHT` 狀態機 | ht9045-shuttle-flow | `GetNowSiteKitMode` Mode 編碼、`CheckXYPitch` Teaching 推導 |
| `TMySucker` / `TMyKitSuck` 欄位 | ht9045-io-control（簡介） | 每個欄位完整語意、重構新增 4 個成員、`iMotRow` / `iMaxRow` / `iPickRow` / `iShtRow` 四層差異 |
| 馬達拓樸 / 空間配置 | ht9045-motor-spatial-layout | 吸嘴端拓樸（非馬達端）、Loader 共用 Function、HP 縮 Pitch 公式 |
| 多吸嘴 vs 單吸嘴 Loader 取料 | （無） | `GetInArmToLoaderPosition` vs `_Single` 對照、Eastsun 20251231 修正記錄 |
| HP 縮 Pitch 數學模型 | ht9045-inarm-flow §10（引用本技能） | 公式原始定義 |
| Teaching 推導範例 | ht9045-shuttle-flow §4（引用本技能） | 推導原始定義（X Teaching 35038 → Aa/Ac） |

**適用對象**：
1. **新進 RD 工程師**：理解 HT9045 為何不是「每個 TestMode 各寫一份程式」
2. **跨模式修改者**：新增 / 修改 1x2_4 / 1x4_8 / 2x2_8 / 2x4_16 吸嘴模式時的對照
3. **HP 縮 Pitch 除錯**：放料位置算錯時的公式對照
4. **Teaching 校驗**：客戶機台 X / Y Teaching 數值對不上時的根因分析

---

## 1. 適用場景

當以下情境發生時載入本技能：

1. **新人 onboarding**：想理解 HT9045 為何採用「全模式共用 Function」設計、而非每個 TestMode 各寫一份
2. **新增 / 修改吸嘴模式**：例如新加 2x4_16 / 1x4_8 / 2x2_8 / 1x2_4 模式時，需理解 `TMyKitSuck` 各欄位語意
3. **HP 縮 Pitch 計算除錯**：放料位置不對、`iBaseXToHP` 或 `iHPBasePos` 公式驗證
4. **Shuttle 吸嘴 Site 計算**：`GetNowSiteKitMode` Mode 編碼意義
5. **Teaching 數值核對**：A 到 G 的 X-Pitch、Site Aa/Ac 位置如何由 X Teaching 推導
6. **IO_Table.csv / Mot_Table.csv 表格驅動硬體選配** 的修改 / 新增 / 除錯

---

## 2. 表格驅動的硬體選配

HT9045 在重構後改採 **CSV 表格驅動** 取代寫死的 `#define`：

### IO_Table.csv
```cpp
enum eIOType
{
    eMotionNet = 0,                     //MotionNet 主流硬體
    eISABase   = 1,                     //ISA 基地址（舊式）
    ePCI1735U  = 2,                     //研華 PCI-1735U
    ePCI1203   = 3,                     //研華 PCI-1203 / EtherCAT
};
```

### Mot_Table.csv
```text
"MN200",   //泓格 ICP-DAS MotionNet
"SYNTEK",  //SYNTEK
"SMC",     //SMC 控制器
"MC88X1",  //其他馬達卡
```

> **不要直接改 enum 數字**：欄位是表格欄索引，新增硬體只能在 enum 末尾追加，避免破壞既有 CSV。

---

## 3. `TMySucker` — 單一吸嘴類別

完整定義位於 `mykitsuck.h`（V3.33.904.2 約 L14 起）。重構新增的 4 個成員：

| 成員 | 型別 | 用途 |
|------|------|------|
| `iMotNo` | `int` | 對應要使用的「馬達」或「汽缸」編號 |
| `iMyRow` | `int` | **實體** Row（不是迴圈索引） |
| `iMyCol` | `int` | **實體** Col |
| `sName`  | `AnsiString` | Alarm 顯示用名稱（如 `"A"`、`"H"`） |

設計重點：**實體位置與迴圈索引分離**，避免某模式跳格使用吸嘴（如 1-3 模式只用 A/E）時要寫多份程式碼。

---

## 4. `TMyKitSuck` — 一組吸嘴集合（核心）

`TMyKitSuck` 是 InArm / OutArm / Shuttle / TestSocket / CatchTray 等所有吸嘴集合的共同類別。
重構後新增的核心欄位（節錄，HT-9xxx 配置範例）：

| 欄位 | 含義 | 範例（2x4 標準 8 吸嘴模式） |
|------|------|----------------------------|
| `iMotRow` | **實體** 馬達數量（Row） | 2 |
| `iMotCol` | **實體** 馬達數量（Col） | 4 |
| `iMaxRow` | **實體** 吸嘴數量（Row） | 2 |
| `iMaxCol` | **實體** 吸嘴數量（Col） | 4 |
| `iPickRow` | **工作模式下**的吸嘴數量（Row） | 2 |
| `iPickCol` | **工作模式下**的吸嘴數量（Col） | 4 |
| `iShtRow`  | **工作模式下**在 Shuttle 上的數量（Row） | 2 |
| `iShtCol`  | **工作模式下**在 Shuttle 上的數量（Col） | 4 |
| `iPickStep` | Pitch 倍數（用於跳格模式：1-3 模式寫 2、1-4 模式寫 3） | 1 |
| `iPickKitStep` | For 吸嘴的 Kit Step | 1 |
| `iShtKitStep`  | For Shuttle 的 Kit Step | 1 |
| `iXStep` | X pitch 要跑幾次 | 1 |
| `iYStep` | Y pitch 要跑幾次 | 1 |
| `iModeX` | 針對 NON-Standard 的 X 陣列要用哪一組 | 0 |

> 注意：`_MAX_SUCK_ROW_ITEM` = 4 / `_MAX_SUCK_COL_ITEM` = 8 為陣列上限，但 HT-9xxx **實際** 只用 2 Row × 4 Col = 8 吸嘴。

### 4.1 為什麼要把「馬達數」和「吸嘴數」分開？

HT-9xxx 可變 Pitch 範例（例如 1x2 模式）：
- **實體**仍是 8 顆吸嘴（A/B/C/D/E/F/G/H，2×4 排列），但 1x2 模式只使用 A+E（跳格）。
- `iMaxRow=2, iMaxCol=4` 對應陣列邊界（8 吸嘴實體上限）。
- `iPickRow=1, iPickCol=2` 表示**該模式真正動作的吸嘴數量**。
- `iPickStep=3` 表示每兩顆吸嘴之間跳 3 個物理 Col（A→E 跨 4 個位置，即 j=0 → j=3，差距 = (`iPickCol`-1)×`iPickStep` 之類的語意）。
- 馬達數仍對應全部 X 方向動作能力，不會因模式而改變。

### 4.2 預先定義 In/Out Suck — HT-9xxx 拓樸

```
InArm 8 吸嘴實體：
    A   C   E   G          ← Row0: Item[0][0..3]
    B   D   F   H          ← Row1: Item[1][0..3]

OutArm 8 吸嘴實體：（同 InArm 拓樸）
    A   C   E   G
    B   D   F   H

迴圈/工作位置（1-3 模式，4 取 2 跳格）：
    A   -   E   -          ← 只用 A,E（j=0, j=2）
    B   -   F   -          ← 只用 B,F
```

設計原則：
- **沒用到的位置（C/D/G/H）也要交換**，避免異常（陣列 `Item[]` / `bPass[]` / `iBinData[]` 等保持一致）。
- **不管使用到哪個吸嘴，相同模式統一處理**——這是重構的核心動機。

---

## 4.3 基準軸（`iInArmXBase` / `iInArmYBase`）— 取放料計算原點

**基準軸 = `InArmSuck.Suck[iInArmYBase][iInArmXBase]`**，是所有 Teach 點位、Pitch 推算的參考原點。
工程師在 Setup 頁面只 Teach 基準軸的位置，其他 7 顆吸嘴透過 Pitch 公式 + Z 高差陣列推算。

### 4.3.1 Y-Pitch Modular 模式（`USE_IN_OUT_ARM_Y_PITCH`）

| 值 | 常數名 | UI（cSetUp） | Pitch 馬達 |
|---|---|---|---|
| 0 | `iXPitch60` | Fixed Y Pitch 60mm | **僅 XP1** |
| 1 | `iXPitchManual635` | Manual Y Pitch 60 & 63.5mm | **僅 XP1**（手動換間距片） |
| 2 | `iXYPitchVariable` | Auto Y Pitch In Bc / Out Bb | **XP1 + XP2 + YP** 三軸 |
| 3 | `iXPitchManual360` | Manual Y Pitch 60 & 36mm | **僅 XP1**（手動換間距片） |
| 4 | `iXYPitchRowA` | Auto Y Pitch in Row A | 可變 Y（Row A 基準）|

> **Source**：`cmydef.cpp` L3094-3102（const）、`cmydef.h` L2848-2860（extern）。
> **JerryYang 20251218 補充**：`USE_OUT_ARM_Y_PITCH` 獨立於 `USE_IN_OUT_ARM_Y_PITCH`，允許 IN/OUT ARM 不同模組（`database.cpp` L933-940 覆蓋 OutArm 基準）。

### 4.3.2 InArm 基準軸對照（依 Y-Pitch 模式）

| Y-Pitch 模式 | `iInArmYBase` | `iInArmXBase` | InArm 基準吸嘴 |
|---|---|---|---|
| `iXPitch60` / `iXPitchManual635` / `iXPitchManual360` / `iXYPitchRowA` | 0 | 2 | **E** [0,2] |
| `iXYPitchVariable` (可變 X+Y) | **1** | 2 | **F** [1,2] |

### 4.3.3 OutArm 基準軸 — 與 InArm 鏡像對稱

| Y-Pitch 模式 | `iOutArmYBase` | `iOutArmXBase` | OutArm 基準 | 對稱關係 |
|---|---|---|---|---|
| `iXPitch60` / `iXPitchManual635` / `iXPitchManual360` | 0 | 2 | **E** [0,2] | 兩臂同 col 2 |
| `iXYPitchVariable` (可變 X+Y) | **1** | **1** | **D** [1,1] | **InArm=col 2、OutArm=col 1（鏡像）** |
| `iXYPitchRowA` | 0 | **1** | **C** [0,1] | InArm=col 2、OutArm=col 1（鏡像）|

> **物理意義**：InArm 朝 Loader、OutArm 朝 Unloader，可變 X+Y Pitch 機構鏡像安裝。從機台「全域座標」看，InArm 在 col 2 的基準軸鏡射到 OutArm 就會落在 col 1。
> **Source**：`database.cpp` L786-940。

### 4.3.4 視覺對照（HT-9xxx，2x4 = 8 吸嘴）

```
固定 Y-Pitch（iXPitch60 / 60&63.5 / 60&36 / RowA）：

InArm：                              OutArm（同位置）：
  A   C   ★E★  G   ← Row 0          A   C   ★E★  G   ← Row 0
  B   D    F   H   ← Row 1          B   D    F   H   ← Row 1
  ↑                                  ↑
  XP1                                XP1

可變 X+Y Pitch（iXYPitchVariable）：

InArm：                              OutArm（鏡像）：
  A   C   E   G   ← Row 0           A   C   E   G   ← Row 0
  B   D  ★F★  H   ← Row 1           B  ★D★  F   H   ← Row 1
  ↑       ↑                          ↑   ↑
  XP1     YP                         XP1 YP

★ = 基準軸吸嘴
```

### 4.3.5 為什麼可變模式要把基準改到 Row 1

可變 Y-Pitch 機構中，YP 馬達需要一個「固定參考端」。
Row 1（B/D/F/H）是 YP 馬達的固定端，Row 0 隨 YP 變動。
基準必須設在「YP 不會帶動」的那一側，這樣 `Tech.iInArmLoadStageY` 才不會因 Y-Pitch 變化失準。

---

## 4.4 Teach 點位結構 — Setup 頁面只 Teach 基準軸

### 4.4.1 Tech 結構欄位（`LastSet.h`）

| 欄位 | 意義 | 對應馬達 |
|---|---|---|
| `Tech.iInArmLoadStageX/Y` | 基準軸在 Loader Stage 的 X/Y 馬達位置 | `MInArmX`, `MInArmY` |
| `Tech.iInArmLoadStagePickZ1/Z2` | 基準軸 Z 馬達吸取高度（Z1/Z2 兩段） | `MInArmZE` 或 `MInArmZF`（依模式） |
| `Tech.iInArmPlate1X/Y/PickZ1/Z2` | 基準軸在 HotPlate1 的位置 | 同上 |
| `Tech.iInArmShuttle1X/Y` / `iInArmShuttle2X/Y` | 基準軸在 InShuttle 的位置 | 同上 |
| `Tech.iInArmZHeightSub[2][4]` | **其他 7 顆吸嘴與基準軸的 Z 高差**（0.1um） | `MInArmZA~ZH`（除基準軸外）|
| `Tech.iInArmZHeightSub_16[2][4]` | 16 吸嘴擴充版 Z 高差（col 4-7）| HT-1032 用 |
| `Tech.TechInArmPitchX/Y` | XP1/YP 馬達 Teach 位置 | `MInArmPitch`, `MInArmPitchY` |
| `Tech.iInArmX40Pitch` / `iInArmX120Pitch` | XP1 馬達在 40/120mm pitch 的位置 | `MInArmPitch` |
| `Tech.iInArmX40Pitch2` / `iInArmX120Pitch2` | XP2 馬達（可變模式）在 40/120mm pitch 的位置 | `MInArmPitchX2` |
| `Tech.iInArmRearOffsetX/Y` | 後排（Row 1）相對 Row 0 的 XY 偏移 | — |

### 4.4.2 Z 高度差陣列示意

```
                     j=0       j=1       j=2       j=3
              ┌─────────┬─────────┬─────────┬─────────┐
  i=0 (Row0)  │ ZA-Zbase│ ZC-Zbase│  0      │ ZG-Zbase│  ← E[0,2] 自己差值=0
              ├─────────┼─────────┼─────────┼─────────┤
  i=1 (Row1)  │ ZB-Zbase│ ZD-Zbase│ ZF-Zbase│ ZH-Zbase│
              └─────────┴─────────┴─────────┴─────────┘
```

可變模式下基準改 F[1,2]，差值表結構不變，只是 `iInArmZHeightSub[1][2]=0`。

---

## 4.5 取放料公式 — Loader / HP / Shuttle

### 4.5.1 初始化：推算所有吸嘴的絕對座標（`cinitial.cpp` L8637+）

```cpp
// 步驟 1：先算「基準軸」的絕對 X/Y/Z 座標
Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase] =
    Tech.iInArmLoadStageX
    + LoadForm->XStart + LoadForm->BlockXStart - KitPitchX
    + InArmOffSet[InOfsLoader]->GetX()
    + Prod.iTrayKitStartX;

Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase] =
    Tech.iInArmLoadStageY - LoadForm->YStart - LoadForm->BlockYStart
    + KitPitchY + InArmOffSet[InOfsLoader]->GetY()
    - Prod.iTrayKitStartY;

Prod.ZInArm_Tray_Pick[iInArmYBase][iInArmXBase] =
    Tech.iInArmLoadStagePickZ2 + InArmOffSet[InOfsLoader]->GetPickUp();

// 步驟 2：其他吸嘴 Z 高度 = 基準軸 Z + 個別差值 + 個別 Offset
for(int i=0; i<InArmSuck.iMaxRow; i++)
  for(int j=0; j<InArmSuck.iMaxCol; j++) {
    if(i==iInArmYBase && j==iInArmXBase) continue;
    iTemp = (j<4) ? Tech.iInArmZHeightSub[i][j]
                  : Tech.iInArmZHeightSub_16[i][j-4];
    Prod.ZInArm_Tray_Pick[i][j] =
        Prod.ZInArm_Tray_Pick[base] + iTemp
        + InArmOffSet[InOfsLoader]->GetPickUp(i, j);

    // ★ Eastsun 20251231 修正：
    //   非基準軸 X/Y 一律先設為基準位置；個別 Offset 移到 GetInArmToLoaderPosition 才加
    //   理由：iPickRow=1 Fix 模式無法判斷使用哪顆吸嘴，
    //         初始化時加 Offset 會錯位
    Prod.XInArm_Tray_Pick[i][j] = Prod.XInArm_Tray_Pick[base];
    Prod.YInArm_Tray_Pick[i][j] = Prod.YInArm_Tray_Pick[base];
  }
```

### 4.5.2 Loader 取料 — 多吸嘴版（`ainarm9045.cpp` L4660）

```cpp
// GetInArmToLoaderPosition(iSelRow, iXPos, iYPos, iRow, iCol)
// iRow / iCol = Tray 上 IC 的索引

iXPos = Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase]   // 基準軸 Teach 位置
      + iCol * Prod.LoadForm.iXPitch                       // Tray IC 跨距
      + iInArmXBase * dInArmXPitch_1Step;                  // 基準軸→第 0 顆吸嘴的 Pitch 偏移

iYPos = Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase]
      - iRow * Prod.LoadForm.iYPitch;

// 最後加上個別吸嘴 Offset：
iXPos += InArmOffSet[InOfsLoader]->GetArmX(i, j);   //Eastsun 20251231
iYPos += InArmOffSet[InOfsLoader]->GetArmY(i, j);
```

**`+iInArmXBase × dInArmXPitch_1Step` 的含意**：MInArmX 馬達指的是「基準軸正下方」位置。要讓 A 對齊 Tray[*][0]、E 對齊 Tray[*][2]，必須把 MInArmX 往右偏 `iInArmXBase × 1step` 距離。

### 4.5.3 Loader 取料 — 單吸嘴 Fix 模式（`ainarm9045.cpp` L4704）

```cpp
// GetInArmToLoaderPosition_Single() — 1×1 / Fix 模式使用
// 用「實際使用的吸嘴」反推 X 位置（吸嘴位置 ≠ 基準軸位置）：
iXPos = Prod.XInArm_Tray_Pick[iSelRow][iCurrSuck]
      + iCol * Prod.LoadForm.iXPitch
      + (iInArmXBase - iRealUseSuck) * dInArmXPitch_1Step;
//      ↑↑↑ 基準軸 → 實際使用吸嘴 的偏移補回
```

### 4.5.4 X-Pitch 單步距離（`ainarm9045.cpp` L4476-4610）

```cpp
// AutoCalculateInArmXClosePitch() — recipe 載入後呼叫
InArmClose_PitchX = UserDefForm[Ld].XPitch * iInArmXStep;   //跨幾顆 IC

// 4 吸嘴模式（A→G 跨 3 步）：
dInArmXPitch_1Step    = InArmClose_PitchX / 3.0;
dInArmXPitch_MovePitch = dInArmXPitch_1Step * 3.0;          // = InArmClose_PitchX

// 16 吸嘴模式（USE_16PICKER_TYPE==1，A→H 跨 7 步）：
dInArmXPitch_1Step    = InArmClose_PitchX / 6.0;
dInArmXPitch_MovePitch = dInArmXPitch_1Step * 7.0;
```

### 4.5.5 OutArm 對稱版

OutArm 公式與 InArm 平行，只是變數命名 `iInArm*` → `iOutArm*`，基準軸見 §4.3.3：

| InArm 變數 | OutArm 對應 |
|---|---|
| `iInArmXBase` / `iInArmYBase` | `iOutArmXBase` / `iOutArmYBase` |
| `dInArmXPitch_1Step` | `dOutArmXPitch_1Step` |
| `Prod.XInArm_Tray_Pick[i][j]` | `Prod.XOutArm_Tray_Place[i][j]` |
| `InArmOffSet[InOfsLoader]` | `OutArmOffSet[OutOfsAuto1..3, Fix1..3]` |
| `MInArmX/Y/Pitch/ZA~ZH` | `MOutArmX/Y/Pitch/ZA~ZH` |
| `MInArmPitchY` (YP) | `MOutArmPitchY` |
| `MInArmPitchX2` (XP2) | `MOutArmPitchX2` |

---

## 5. 核心設計哲學 — 全模式共用 Function

> **Original**：每個不同模式要寫兩個 Function
> **New**：全部模式共用兩個 Function

以下是重構前後對照的關鍵函式：

### 5.1 Loader 操作（`ainarm9045.cpp`）

| 函式 | 角色 | 對應 PPT |
|------|------|----------|
| `MoveArmXYToLoaderStage()` | 對外介面 | Slide 19 |
| `DoMoveArmXYToLoaderStage()` | 根據不同模式計算 X-Pitch 與 Step 帶入下層 | Slide 20 |
| `GetInArmToLoaderPosition(iSelRow, iXPos, iYPos, iRow, iCol)` | 計算多吸嘴 Loader 取料絕對位置 | Slide 21（L4614） |
| `GetInArmToLoaderPosition_Single(iSelRow, iXPos, iYPos, iRow, iCol)` | 計算單吸嘴 Loader 取料絕對位置；**回傳值為 `iMovePitchX`**（修正 Fix 模式 X Pitch 錯誤） | Slide 22（L4673） |

**位置公式**（Slide 21，4 吸嘴模式）：
```
基準軸移動到要吸取的位置  =  iX * TrayPitch
吸嘴 A 移動到基準軸位置  =  3 個 Arm X-Pitch   //(N-1) 個 Arm X-Pitch，N=4
```

> **Eastsun 20251231 註**：`InArmOffSet[InOfsLoader]->GetArmX(i,j)` 須加在 `GetInArmToLoaderPosition` 而非 `_Single`，因 `_Single` 無法判斷吸嘴編號。

### 5.2 迴圈統一寫法

重構後迴圈統一以 `iMaxRow` / `iMaxCol` 為邊界，並依 `iPickStep` 跳格：

```cpp
//Original：每個模式各寫一份 for 迴圈
//New：以下 4 種迴圈全模式共用
for(int i=0; i<InArmSuck.iMaxRow; i++)                                          //吸嘴馬達
for(int i=0; i<InArmSuck.iPickRow; i++)                                         //吸嘴
for(int i=0; i<InArmSuck.iPickRow; i++) for(int j=0; j<InArmSuck.iPickCol; j++) //吸嘴 Alarm
for(int i=0; i<InArmSuck.iShtRow;  i++) for(int j=0; j<InArmSuck.iShtCol;  j++) //Shuttle
```

> **吸嘴與 Shuttle 的 Kit 不同**：當 InArm 吸嘴模式與 Shuttle 上 Site 配置不同時，必須分別用 `iPickKitStep` 與 `iShtKitStep`。

---

## 6. HotPlate（加熱盤）操作

### 6.1 縮 Pitch 數學模型（Slide 23–26）

當 HotPlate 物理 Col 數與 InArm 吸嘴模式不匹配時（例如 1x2/2x2 mode 對 3-Col HP，或 1x4/2x4 mode 對 6-Col HP），需要計算「能否縮 Pitch」以最大化加熱效率。

**核心公式**：
```
iHPBasePos      = HP.XStart - iBaseXToHP
iPickStep1Pos   = iHPBasePos + HP.XPitch        (1x2/2x2 mode vs 3-Col HP)
                = iHPBasePos + HP.XPitch * 2    (1x4/2x4 mode vs 6-Col HP)
```

| 符號 | 含義 |
|------|------|
| `iBaseXToHP` | 基準軸針對加熱盤邊緣的距離 |
| `iHPBasePos` | 假設吸嘴對比加熱盤 Step 為 1 時的右側槽穴位置 |
| `iPickStep1Pos` | 縮 Pitch 後第一步的位置 |
| `HP.XStart` | HotPlate Recipe 中 X 起始座標 |
| `HP.XPitch` | HotPlate Recipe 中 X 間距 |

### 6.2 放料 / 吸料流程（Slide 27–32）

重構後 HP 放/取料的核心函式（位於 `ainarm2.cpp` 與 `ainarm9045.cpp`）：

| 函式 | 行為 |
|------|------|
| `SearchPlateToPlace()` | 工作檔載入後產生加熱盤吸取陣列；對應位置上的加熱盤有料就往下一個陣列位置尋找 → 取出**預計要放料的位置** |
| `HasHotReadyIC()` | 直接檢查 `PickFromHPList` 陣列內有無已加熱完成的 IC |
| `SearchPlateToPick()` | 提取 `PickFromHPList` 陣列內的第一筆資料 → 決定要使用的吸嘴；資料交換 |
| `MoveInArmXYToPlate()` | **吸料的位置帶入與放料相同的 Function**（共用） |
| `PlaceToHPList` | 待放料陣列 |
| `PickFromHPList` | 待吸料陣列（已加熱完成） |

> **放料順序表（Slide 28，4 吸嘴範例）**：
> ```
> 次數  Row  Col
> 第0次 [0,0] [0,1]
>       [1,0] [1,1]
> ```

如果 IC 是分多次放入加熱盤，需要針對該 Flag++（典型用法：`iHotCount`）。

> **詳見** [ht9045-inarm-flow](../ht9045-inarm-flow/SKILL.md) §10「HP 縮 Pitch 公式與放/取料流程」。

---

## 7. Shuttle 操作

### 7.1 `GetNowSiteKitMode` — Mode 編碼規則（Slide 33–34）

Shuttle 在初始化時即判斷 InArm 在 Shuttle 上面要使用的 X / Y 軸移動次數，編碼為單一 `int`：

```
Mode = (X 倍數) * 100 + (Y 倍數)

Mode / 100  → X 軸要移動的標準 X 間距「倍數」（1 = 標準 X-Pitch）
Mode % 100  → Y 軸要移動的標準 Y 間距「倍數」（1 = 標準 Y-Pitch）
```

### 7.2 Auto Clean 模式下的特殊處理（Slide 34）

| 一般模式 | Auto Clean 模式 |
|---------|----------------|
| 以「預計要放入的 Shuttle Row」為基準 | 以「預計要吸取的 Shuttle Row」為基準 |

實作位置：例如 `GetNowSiteKitMode_2x4_16()`（`ainarm9045_2x4_16.h` L15）。

### 7.3 `CheckXYPitch` — Teaching 計算（Slide 35–36）

範例（HT-9046 2x4_16 標準模式，8 吸嘴）：

```
X Teaching                 = 35038
E to Center                = 35038 - 2000 = 33038
X-Pitch from A to G        = 8000                         //A→G 跨 3 個 Arm X-Pitch
A to E                     = 8000 / 3 * 2 = 5333.33       //A→E 跨 2 個 Arm X-Pitch
A move to Center           = 33038 + 5333.33 = 38371.33

A to site Aa (X pitch × -1.5) = 38371.33 - (8000 × 1.5) = 26371.33
A to site Ac (X pitch ×  0.5) = 38371.33 + (8000 / 2)   = 42371.33
```

公式抽象化（8 吸嘴 / 4-Site 模式）：
```
吸嘴 A 移動到基準軸位置          = 3 個 Arm X-Pitch       //(N-1) Arm X-Pitch，N=4
計算每隻吸嘴間的 Arm X-Pitch     = (XPitch_total) / (N - 1)
吸嘴 A 移動到第一個 Site 位置    = 1.5 個 Site Pitch      //(SiteCount/2 - 0.5) Site Pitch
```

> **詳見** [ht9045-shuttle-flow](../ht9045-shuttle-flow/SKILL.md) §4「GetNowSiteKitMode 與 CheckXYPitch」。

---

## 8. 關鍵 Source 位置（V3.33.904.2 基準）

| 主題 | 檔案 | 行號（近似） |
|------|------|--------------|
| `TMySucker` 定義 | `mykitsuck.h` | L14 |
| `TMyKitSuck` 定義 | `mykitsuck.h` | L145 |
| `_MAX_SUCK_ROW_ITEM` / `_MAX_SUCK_COL_ITEM` | `mykitsuck.h` | L11-12（值 4 / 8） |
| `InArmSuck` / `FTestSuck` / `BTestSuck` / `OutArmSuck` / `OutArm2Suck` | `mykitsuck.h` | L357-377 |
| `GetInArmToLoaderPosition` | `ainarm9045.cpp` | L4614 |
| `GetInArmToLoaderPosition_Single` | `ainarm9045.cpp` | L4673 |
| `GetNowSiteKitMode_2x4_16` | `ainarm9045_2x4_16.h` | L15 |
| `CheckXYPitch_2x4_16` | `ainarm9045_2x4_16.h` | L16 |
| `SearchPlateToPlace` | `ainarm2.cpp` / `ainarm2.h` L112 | — |
| `PickFromHPList` (extern) | `Public/HTEditList.h` | L244 |

---

## 9. 常見誤區

1. **直接以 `iMaxCol` 當迴圈邊界做運算 → 算錯 Pitch**
   應使用 `iPickCol` × `iPickStep` 才是「該模式真正吸取的物理跨距」。
2. **忘記交換沒用到的吸嘴位置**
   `Item[]`、`bPass[]`、`iBinData[]` 等所有 `[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]` 陣列都要保持一致，否則 Auto Clean / Rotator 會錯亂。
3. **HP 縮 Pitch 公式套錯機型**
   1x2/2x2 vs 3-Col 與 1x4/2x4 vs 6-Col **不是同一條公式**，差別在 `+HP.XPitch` 或 `+HP.XPitch*2`。
4. **`GetNowSiteKitMode` 直接拿來當數量用**
   它是「**倍數編碼**」（Mode/100 + Mode%100），不是「吸取數量」。
5. **改 `enum eIOType` 數字**
   會破壞 IO_Table.csv 欄位對應，新增只能在末尾追加。

---

## 10. 參考文件

- PPT 原檔：`D:\Work-rogeryang\~教育訓練\Handler\軟體架構\邏輯機台_軟體架構.pptx`
- PPT 文字萃取（48 張）：`D:\AI_TempFile\pptx_extract\all_text.txt`
- 相關技能：
  - [ht9045-inarm-flow](../ht9045-inarm-flow/SKILL.md)
  - [ht9045-shuttle-flow](../ht9045-shuttle-flow/SKILL.md)
  - [ht9045-motor-spatial-layout](../ht9045-motor-spatial-layout/SKILL.md)
  - [ht9045-mode-naming-convention](../ht9045-mode-naming-convention/SKILL.md)
  - [ht9045-recipe](../ht9045-recipe/SKILL.md)
  - [ht9045-config](../ht9045-config/SKILL.md)
  - [ht9045-general-ini](../ht9045-general-ini/SKILL.md)

---

**範圍說明**：
本技能聚焦 **HT-9xxx 系列**（InArm / OutArm 各 8 吸嘴 = 2 Row × 4 Col）。
若日後需涵蓋 HT-1032AT（16 吸嘴 Aa~Ah / Ba~Bh、氣缸版 / 馬達版 / 新馬達版）、HT-7080B、HT-9016 / HT-9115 等其他拓樸，建議**另立獨立技能**（如 `ht1032at-sucker-architecture`、`ht7080b-sucker-architecture`），避免單一技能涵蓋過多差異而難以維護。

**PPT 章節覆蓋狀態**：
本技能完整收錄 PPT Slide 2–36 的具體技術內容；Slide 37–48（資料存取、附加功能：Auto Clean / Auto Site Map / Rotater / Try Pick HP / AOA / Auto Teaching / 機台資料轉換）為 PPT **僅有標題、無實質內容**的章節，故暫不於本技能列入比較或敘述，待日後 PPT 補充或另行整理時再行補列。

---

**維護**：
- 建立日期：2026-05-12
- 維護者：RogerYang
- 基準版本：HT9011UC_Code_V3.33.904.2_20260511_RogerYang（V3.33.8XX+ 後皆適用）
- 來源：Steven Chou《邏輯機台_軟體架構.pptx》2023.07
