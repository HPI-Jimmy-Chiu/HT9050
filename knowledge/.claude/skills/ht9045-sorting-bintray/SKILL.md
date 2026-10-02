---
name: ht9045-sorting-bintray
description: "HT9045 Unloader 整盤功能 (SortingBinTray / P27) 知識庫。涵蓋 DoSortingBinTray 狀態機、DoFix3FullTray 流程、觸發條件、CosFunction/IniConfig 旗標、互鎖邏輯。Use when: 分析 Clean Out 整盤、Tray Feed 整盤、P27 設定、Fix3 FullTray、OutArm 整盤流程。關鍵字：SortingBinTray, DoSortingBinTray, bP27AutoSortingBinTrayByOutArmwhenCleanOut, bSortingBinTraywhenCleanOut, bSortingBinTrayWhenTrayFeed, DoFix3FullTray, bSortingAllBinTrayFinish, bSortingSuckMode, Clean Out, Tray Feed, Unloader 整盤, P27"
---

# HT9045 SortingBinTray (Unloader 整盤) 知識庫

## 功能概述

SortingBinTray 是 HT9045 的 **Unloader 整盤功能**，在 Clean Out 或 Tray Feed 時，將 Auto/Fix Tray 上散落的 IC 整理排列整齊（消除中間的空位），讓出料盤面排列緊密。

**核心邏輯**：OutArm 用基準吸嘴逐顆從 Tray 上取出散落的 IC，再放到 Tray 上的空位，依 Tray Direction 方向從對角開始填補。

## 旗標體系

### 啟用條件（三層控制）

| 層級 | 旗標 | 定義位置 | 說明 |
|------|------|----------|------|
| INI 設定 (P27) | `IniConfig.bP27AutoSortingBinTrayByOutArmwhenCleanOut` | `Config.h:1378` | 使用者可在 Configuration P27 開關 |
| 客戶功能 (模式A) | `CosFunction.bSortingBinTraywhenCleanOut` | `CosFunction.h:155` | Clean Out 時整盤 |
| 客戶功能 (模式B) | `CosFunction.bSortingBinTrayWhenTrayFeed` | `CosFunction.h:156` | Tray Feed 前整盤 |

**必須 P27=true + (模式A 或 模式B) 才會啟用整盤。**

### Configuration P27 顯示邏輯

```
cConfiguration.cpp L4079-4087:
  if(AUTO3_IS_MAGAZINE==1)
      → P27 隱藏且強制關閉 (Magazine 與整盤衝突)
  else if(bSortingBinTraywhenCleanOut || bSortingBinTrayWhenTrayFeed)
      → P27 顯示、可編輯、從 INI 讀值
  else
      → P27 隱藏且強制關閉
```

### CosFunction 初始化

`bSortingBinTraywhenCleanOut` 預設 `false`（CosFunction.cpp L4072），由客戶函式開啟：
- `FUNC_CC_SIGURD_ChungXing()` → `true`（矽格中興）
- `FUNC_CC_SIGURD_PeiXing()` → `true`（矽格北興）
- `FUNC_CC_UTAC_TW()` → `true`（聯測台灣）

`bSortingBinTrayWhenTrayFeed` 預設 `false`（CosFunction.cpp L4150），由客戶函式開啟：
- `FUNC_CC_SPIL_HsinChu()` → `true`（矽品新竹，L3034）— 注意此客戶 Clean Out 模式被 mark 掉

### 強制關閉條件

```
cprod.cpp L3661-3662:
  if(AUTO3_IS_MAGAZINE==1)
      CosFunction.bSortingBinTraywhenCleanOut = false;
      CosFunction.bSortingBinTrayWhenTrayFeed = false;
```

### 執行時輔助旗標

| 旗標 | 說明 |
|------|------|
| `bSortingAllBinTrayFinish` | 所有 Tray 整盤完成旗標，OutArm 主迴圈 case 1 重置為 false |
| `bSortingSuckMode` | 整盤進行中旗標 (Sam 20250415)，防止 ChangeSite 干擾 |

## 觸發路徑

### 路徑 A：Clean Out 時整盤

```
aoutarm.cpp: CheckOutArmCleanOut()
  條件: iCleanOut==1
      && IniConfig.bP27AutoSortingBinTrayByOutArmwhenCleanOut==true
      && CosFunction.bSortingBinTraywhenCleanOut==true
  進入條件: InArm/OutArm/Shuttle/Index 全無 IC
            && IsInArmCleanOutFinish()==true
  → return 5000

aoutarm9045.cpp: DoOutArmAfterPlaceToAuto() case 5000
  條件: P27==true && bSortingBinTraywhenCleanOut==true
      && bSortingAllBinTrayFinish==false
  → DoSortingBinTray(0) 初始化
  → Task=5100

case 5100:
  → DoSortingBinTray() 持續呼叫
  → 完成後回到 case 5000
```

### 路徑 B：Tray Feed 前整盤

```
csystem.cpp: TrayFeedTask case 2
  條件: CosFunction.bSortingBinTrayWhenTrayFeed==true
      && IniConfig.bP27AutoSortingBinTrayByOutArmwhenCleanOut==true
  → DoSortingBinTray(0) 初始化
  → Task=10000

case 10000:
  → DoSortingBinTray() 持續呼叫
  → 完成後 Task=3 (繼續 Tray Feed)
```

### OneCycleFinish 保護

```
csystem.cpp L13832-13836:
  if(iCleanOut==1)
      if(P27==true && bSortingAllBinTrayFinish==false
         && bSortingBinTraywhenCleanOut)
          → 強制留在 iCleanOutCycleTask=1，不跳出 OneCycleFinish
```

## DoSortingBinTray 狀態機

**檔案**：`SortingBinTray/SortingBinTray.cpp`
**Task 變數**：`iTask_DoSortingBinTray`
**注冊**：`QueueTaskList[228]`

### 初始化 (iFlag==0)

```cpp
bSortingSuckMode = true;
SortingBinTray_Data.iMAXSUCK_OutArmSuckX = 4;
SortingBinTray_SetMotorPosData();   // 設定 OutArm XYZ 馬達點位
SortingBinTray_SetTraySpecData();   // 遍歷 Auto/Fix Tray，判斷哪些需要整盤
iTask_DoSortingBinTray = 1;
```

### 主流程

```
case 1:   MoveOutArmToAutoSafe() → Task=100
case 100: 設定初值 → Task=200
case 200: TrayArm 移到安全點 (Empty)
          Guard: MTrayXCanSafeMove()==false → 等待
          Guard: MOT[MTrayX].fHasTray → 等待
          → Task=300
case 300: 遍歷 eTrayCount，找 bTrayAction[i]==true
          → 設定 iNowActionTray=i → Task=1000
          若全部完成 → Task=400
case 400: → Task=10000

--- 取 IC 流程 ---
case 1000: → Task=1100
case 1100: 檢查吸嘴是否有 IC
           有 → Task=3000 (放 IC)
           無 → Task=1200
case 1200: 檢查是否還需整盤 (SortingBinTray_IsBinTrayNeedToSorting)
           是 → SortingBinTray_ToTrayPickIC(0) 初始化 → Task=1400
           否 → bTrayAction[i]=false → Task=300 (下一盤)
case 1400: SortingBinTray_ToTrayPickIC() 執行取 IC
           完成 → Task=3000

--- 放 IC 流程 ---
case 3000: → Task=3100
case 3100: 吸嘴有 IC → SortingBinTray_ToTrayPlaceIC(0) → Task=3200
           無 IC → Task=1000
case 3200: SortingBinTray_ToTrayPlaceIC() 執行放 IC
           完成 → Task=3400
case 3400: InitialFix3CanFullTask() → Task=1000

--- 結束 ---
case 10000: Fix3 氣缸歸位 → Task=10100
case 10100: bSortingSuckMode=false
            bSortingAllBinTrayFinish=true
            return true
```

## SortingBinTray_ToTrayPickIC 狀態機

**Task 變數**：`iTask_ToTrayPickIC`

```
case 1/100: MoveOutArmToAutoSafe → Task=400
case 400: 檢查 Tray 有 IC → Task=500 / Task=9000(無IC)
          Fix3 氣缸控制
case 500: TrayDataToSortingBin 轉換 Tray 資料 → Task=600
case 600: SortingBinTray_CalPos 計算位置 → Task=650
case 650: SetMotorPosData → Task=700
case 700: OutArmContinuousMove_9045 移動到取IC位置 → Task=1100
case 1100: Suck() 吸取 → Task=1200
case 1200-1300: Z 軸上升
case 1400: 檢查吸取結果
           Error → OutArmPickTrayAlarm (JAM0211-JAM0216)
           RETRY/HOME → Task=650
           SKIP → TRAYSORTING_ERR
case 1600: DataChange 更新資料 → Task=1700
case 1700: SortingBinToTrayData 回寫 → Task=10000 (完成)
```

## SortingBinTray_ToTrayPlaceIC 狀態機

**Task 變數**：`iTask_ToTrayPlaceIC`

```
case 1/200: MoveOutArmToAutoSafe → Task=500
case 500: 確認 Tray 有空位 → Task=600 / Task=9000(滿)
case 600: TrayDataToSortingBin → Task=700
case 700: CalPos → Task=800
case 800: OutArmContinuousMove_9045 移動 → Task=1200
case 1200: Destroy() 放 IC → Task=1300-1400
case 1400: Z 上升 → Task=1500
case 1500: 確認放掉 → 若 Error: JAM0217 → RETRY → Task=700
case 1700: DataChange → Task=1800
case 1800: SortingBinToTrayData → Task=10000 (完成)
```

## DoFix3FullTray 流程

**功能**：Fix3 滿盤整理（Ifor 20161122），與 P27 整盤獨立。

**前提**：`CosFunction.bUseFix3FullTray && IniConfig.bE55UseFix3FullTray`

使用第 4 支吸嘴（HT9045）或第 3 支（HT9045S），在 Fix3 區找 `HAS_IC`/`HAS_NULL_IC` 進行搬移。

## SortingBinTray_IsBinTrayNeedToSorting 判斷邏輯

依 Tray Direction (0-3) 決定遍歷方向，從放料起始角開始掃描：
1. 找到第一個有 IC 的格子後 `bFindIC=true`
2. 之後遇到 `NULL_IC` → 需要整盤 → `return true`
3. 全部掃完沒空隙 → `return false`

## 吸嘴使用

| 模式 | 使用吸嘴 | 基準軸 |
|------|----------|--------|
| DoSortingBinTray (P27 整盤) | `OutArmSuck[iOutArmYBase][iOutArmXBase]`（基準軸第 3 支） | `iOutArmXBase` |
| DoFix3FullTray | `OutArmSuck[iOutArmYBase][SLECT_SUCK3]`（第 4 支, HT9045）或 `SLECT_SUCK2`（第 3 支, HT9045S） | USE_PICKER_COUNT 判斷 |

## 互鎖與安全

| 互鎖點 | 位置 | 說明 |
|--------|------|------|
| TrayArm 安全 | DoSortingBinTray case 200 | MTrayXCanSafeMove + fHasTray |
| OutArm Z 安全 | bOutArmAxisXMove / bOutArmAxisYMove | CheckOutArmZNeedHome 確認 Z 軸不遮 |
| TrayArm 防撞 | bOutArmAxisXMove / bOutArmAxisYMove | IsTrayArmMoveAvoidOutArmCrash |
| ChangeSite 防護 | cinitial.cpp L11497 | P27 && bSortingSuckMode → 跳過 ChangeSite |
| OneCycle 防護 | csystem.cpp L13832 | 整盤未完成不跳出 CleanOut |
| Fix3 氣缸 | case 3400 / case 10000 | InitialFix3CanFullTask / UseFix3Cylinder(0) |

## Offset 設定

`cOffSet.cpp`:
- P27 開啟時，InShuttle Offset 顯示 PickUp 欄位
- P27 開啟時，Auto/Fix Tray Offset 顯示 PickUp 欄位（整盤需要 Pick 高度）
- P27 關閉時隱藏 PickUp

## 相關 Alarm

| Alarm Code | 觸發位置 | 說明 |
|------------|----------|------|
| JAM0211 | OutArmPickTrayAlarm (Auto1) | 整盤 OutArm 吸取異常 |
| JAM0212 | OutArmPickTrayAlarm (Auto2) | 同上 |
| JAM0213 | OutArmPickTrayAlarm (Auto3) | 同上 |
| JAM0214 | OutArmPickTrayAlarm (Fix1) | 同上 |
| JAM0215 | OutArmPickTrayAlarm (Fix2) | 同上 |
| JAM0216 | OutArmPickTrayAlarm (Fix3) | 同上 |
| JAM0217 | ToTrayPlaceIC/DoPlaceTrayIC | 放 IC 真空異常 |

## 客戶啟用清單 (V899)

| 客戶函式 | bSortingBinTraywhenCleanOut | bSortingBinTrayWhenTrayFeed | 備註 |
|----------|:--:|:--:|------|
| FUNC_CC_SIGURD_ChungXing | ? | — | 矽格中興 |
| FUNC_CC_SIGURD_PeiXing | ? | — | 矽格北興 |
| FUNC_CC_UTAC_TW | ? | — | 聯測台灣 |
| FUNC_CC_SPIL_HsinChu | — | ? | 矽品新竹 (Clean Out 被 mark) |
| 預設 | false | false | 大部分客戶未啟用 |

## 潛在風險點

1. **iMAXSUCK_OutArmSuckX=4**：DoSortingBinTray 初始化硬設 4，若改用其他吸嘴配置可能出問題
2. **bSortingSuckMode 與 ChangeSite**：Sam 20250415 加入的防護，整盤期間跳過 ChangeSite，防止 site 映射被改
3. **Magazine 互斥**：AUTO3_IS_MAGAZINE==1 時在 cprod.cpp 和 cConfiguration.cpp 都強制關閉
4. **Fix3 氣缸互搶**：case 200 加了 MTrayXCanSafeMove 檢查（Sam 20210616），防止 P27 與 P24 同時開啟時 CatchTray 互搶控制
5. **Fix Tray Up/Down 模式**：TrayForm.iFixTrayMode==1 時 Fix 區不參與整盤
6. **只用基準吸嘴（單支）**：整盤一次只搬一顆 IC，效率較低但安全
7. **Clean Out 完成判定**：csystem.cpp 中 OneCycleFinish 有整盤未完成的保護，但若 bSortingBinTraywhenCleanOut 為 false 而 bSortingBinTrayWhenTrayFeed 為 true 時，該保護不會生效

## 關鍵原始檔清單

| 檔案 | 角色 |
|------|------|
| `SortingBinTray/SortingBinTray.cpp` | 主邏輯：DoSortingBinTray, ToTrayPickIC, ToTrayPlaceIC, DoFix3FullTray |
| `SortingBinTray/SortingBinTray.h` | 函式宣告、extern Task 變數 |
| `aoutarm.cpp` | CheckOutArmCleanOut() 觸發入口 |
| `aoutarm9045.cpp` | DoOutArmAfterPlaceToAuto() case 5000/5100 執行 |
| `csystem.cpp` | TrayFeed case 2/10000、OneCycleFinish 保護 |
| `cConfiguration.cpp` | P27 Configuration UI 顯隱控制 |
| `CosFunction.cpp` / `CosFunction.h` | 客戶旗標定義與初始化 |
| `Config.h` | IniConfig.bP27 定義 |
| `cprod.cpp` | Magazine 互斥強制關閉 |
| `cinitial.cpp` | ChangeSite 中 bSortingSuckMode 防護 |
| `cOffSet.cpp` | Offset UI PickUp 顯隱 |
| `cmydef.h` / `cmydef.cpp` | bSortingSuckMode 全域宣告 |
