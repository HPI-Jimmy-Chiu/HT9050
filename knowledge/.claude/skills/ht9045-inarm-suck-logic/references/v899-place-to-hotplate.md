# HT9045 V899 InArm 放料到 HotPlate 深度解析

## TOC

1. [範圍與關鍵檔案](#1-範圍與關鍵檔案)
2. [全流程總覽](#2-全流程總覽)
3. [主狀態機觸發路徑](#3-主狀態機觸發路徑-case-10001100)
4. [SearchPlateToPlace 搜尋空位](#4-searchplatetoplace-搜尋空位)
5. [DoPlaceToHotPlate_9045_2x4_8 放料狀態機](#5-doplacetohotplate_9045_2x4_8-放料狀態機)
6. [Destroy 吹氣與 DoPlaceToHPSwapData 資料交換](#6-destroy-吹氣與-doplacetoHpswapdata-資料交換)
7. [bZFlgToHP 與 Z 軸下降控制](#7-bzflgtohp-與-z-軸下降控制)
8. [Shuttle / Kit 對 HotPlate 放料的影響](#8-shuttle--kit-對-hotplate-放料的影響)
9. [吸嘴到 HotPlate 的映射複雜性](#9-吸嘴到-hotplate-的映射複雜性)
10. [HotPlate XDivision 對放料路徑的影響](#10-hotplate-xdivision-對放料路徑的影響)
11. [Row2CanPutHP 與 iPlaceHPOrder](#11-row2canputhp-與-iplacehporder)
12. [放料後多輪迴路 (case 400)](#12-放料後多輪迴路-case-400)
13. [InspectInArmPosition 座標驗證工具](#13-inspectinarmposition-座標驗證工具)
14. [Data Swap Error 1/2 根因分析](#14-data-swap-error-12-根因分析)
15. [Debug Checklist](#15-debug-checklist)

---

## 1. 範圍與關鍵檔案

| 檔案 | 函式 | 約略行號 | 用途 |
|---|---|---:|---|
| `ainarm9045_2x4_16.cpp` | `DoInArm_9045_2x4_16()` case 1000→1100 | 3530-3581 | 主狀態機觸發放 HP |
| `ainarm9045_2x4_8.cpp` | `DoPlaceToHotPlate_9045_2x4_8()` | 821 | 16-site 放料實作（共用 2x4_8 變體） |
| `ainarm_SearchPlacePlate.cpp` | `SearchPlateToPlace()` | 4446 | 搜尋 HotPlate 空位 |
| `ainarm_SearchPlacePlate.cpp` | `SearchPlacePlateXItem4_8Suck()` | 3179 | XDiv=4 放料搜尋 |
| `ainarm_SearchPlacePlate.cpp` | `SearchPlacePlateXItem6_8Suck()` | 3502 | XDiv=6 放料搜尋 |
| `ainarm_SearchPlacePlate.cpp` | `CheckHotPlateHasSpace_9045_8_New_V()` | 2000 | HP 空位判定核心 |
| `ainarm_SearchPlacePlate.cpp` | `GetHotPlateColStep()` | 745 | HP 每輪放幾列 |
| `ainarm_SearchPlacePlate.cpp` | `GetPlaceToHotPlateSuckCol()` | 860 | 吸嘴→HP col 映射 |
| `ainarm_SearchPlacePlate.cpp` | `GetPlaceToHotPlateCol()` | 928 | HP col→實際格位 |
| `ainarm_SearchPlacePlate.cpp` | `Row2CanPutHP()` | 310 | Row2 是否可放 |
| `ainarm_SearchPlacePlate.cpp` | `GetVariableXInHotPlateData()` | 1614 | XPitch 判定 |
| `ainarm_SearchPlacePlate.cpp` | `GetInArmZToHPPos()` | 1655 | Z 軸放料高度 |
| `ainarm_SearchPlacePlate.cpp` | `DoPlaceToHPSwapData()` | 4615 | 放完後資料交換 |
| `ainarm9045.cpp` | `InspectInArmPosition()` | 7971 | 座標偏差檢測 |
| `ainarm9045.cpp` | `GetInArmCellPos()` | 7913 | 取得吸嘴實際位置 |
| `Motor/mymotor.cpp` | `CheckArmPosArrival()` | 5090 | 位置到達判定 |
| `Config.h` | `bE74_InspectArmPosition` | 628 | 啟用座標檢測開關 |

## 2. 全流程總覽

```
InArm 從 Loader 吸完 IC
  │
  ▼
DoInArm_9045_2x4_16() case 10 判斷：是否 Hot mode
  │ 是 → Temperature == Tempture_Hot && InArmSuck.HasIC()
  │ 先把 NULL_IC → HAS_NULL_IC
  ▼
case 1000: 判斷是否直接送 Shuttle (CheckPlaceToShuttle)
  │ 否
  ▼
case 1100: InitInArmPlaceToHotPlateTask() → 呼叫 DoInArmPlaceToHotPlate_9045()
  │         ↓（dispatcher 路由到 DoPlaceToHotPlate_9045_2x4_8()）
  ▼
DoPlaceToHotPlate_9045_2x4_8()
  case 1:   GetVariableXInHotPlateData() + SearchPlateToPlace()
  case 100: MoveInArmXYToHotPlatePlace() — 移 XY 到目標
  case 200: UpdateHPSuckGroup + 決定是否需 delay
  case 350: 逐顆 Destroy() 吹氣放料 + DoPlaceToHPSwapData() 資料交換
  case 400: Z 抬至安全位 → 還有 IC？case 1 回頭再放
  case 500: CheckInArmDestroyICFail() 確認全部放完
  case 501: AdjustShuttleWhichKitOrder() + HPPlaceLog + AddHPSuckGroup
  │
  ▼
回到 DoInArm_9045_2x4_16() case 1100 後
  Task=50 → 等 HP ready / Shuttle 判斷
```

## 3. 主狀態機觸發路徑 (case 1000→1100)

`ainarm9045_2x4_16.cpp` L3530-3581:

```
case 1000:
  InArmSuck.SetType1ToType2ByPickCol(NULL_IC, HAS_NULL_IC)  // 關鍵：先補 HAS_NULL_IC
  if Tempture_Hot:
    if CheckPlaceToShuttle():  // Soak time 已滿可直接送 Shuttle
      Task=2000  return
    if QA mode 超過設定數:
      Task=2000  return
  Task=1100
  InitInArmPlaceToHotPlateTask()  // 初始化子狀態機 Task=1

case 1100:
  bPlaceToHotplate = true
  bPickFromHotplate = false
  if DoInArmPlaceToHotPlate_9045():
    HPPlaceLog.SetPosition()
    bPlaceToHotplate = false
    Task=50  // 回到等待/判斷
```

**注意**：`DoInArmPlaceToHotPlate_9045()` 在 2x4_16 中是一個函式名稱，實際由 linker 對應到 `DoPlaceToHotPlate_9045()` dispatcher。dispatcher 根據 `iInArmType` 路由，其中 `e9045_2x4_8` / `e9045_2x4_16` 使用 `DoPlaceToHotPlate_9045_2x4_8()`。

## 4. SearchPlateToPlace 搜尋空位

`SearchPlateToPlace()` 在 `ainarm_SearchPlacePlate.cpp` L4446，是一個大型分派器：

### 路由邏輯

```
if 1Picker / 1x1       → SearchPlacePlateXItem_1Suck()
if DualSite close one  → SearchPlacePlateXItem_1Suck()
if 1 row + AxEx/AxxG   → SearchPlacePlateXItem_1x2Suck() / XItem3 variants
if 2 row + AxEx/AxxG   → SearchPlacePlateXItem_2x2Suck() / XItem3 variants
if XDivision==2         → SearchPlacePlateXItem_1x2Suck()
if XDivision==3         → SearchPlacePlateXItem3_1x2Suck()
if XDivision==4         → SearchPlacePlateXItem4_8Suck()     ← 常見 16-site 路徑
if XDivision==6         → SearchPlacePlateXItem6_8Suck()
if XDivision==8         → SearchPlacePlateXItem8_8Suck()
if XDivision==10/12/16  → 對應變體
```

### SearchPlacePlateXItem4_8Suck 核心邏輯

在 do-while 迴圈中：

1. 計算 `spacX`, `spacY`, `Row`, `Col`（目前 InArm 配置能一次佔多少 HP 格）
2. 呼叫 `CheckHotPlateHasSpace_9045_8_New_V(iPlate, iy, ix, spacY, spacX, Row, Col, state=0)`
3. 若全空（state=0 檢查 `NULL_IC`）→ 成功，輸出 `iPlacePlateX[0]`, `iPlacePlateY[0]`, `iPlacePlate[0]`
4. 若不成功 → `iy++`，到底了就 `ix++`，翻頁就切另一片 plate
5. 帶 Hang Up 保護計時器：過長未找到空位會觸發 `DoHotPlateHangUp()`

### CheckHotPlateHasSpace_9045_8_New_V 判定邏輯

- state=0（放料找空位）：所有 `SuckRow * SuckCol` 格必須 `== NULL_IC`
- state=1（取料找有料位）：至少一格 `!= NULL_IC`
- state=3（AutoClean 找可清位）

遍歷後回傳 `!bfail`。

**關鍵限制**：InArm 的 2x4 吸嘴排列必須映射到 HP 的 grid，不是任意位置都能對齊。

## 5. DoPlaceToHotPlate_9045_2x4_8 放料狀態機

`ainarm9045_2x4_8.cpp` L821:

| case | 動作 | 說明 |
|---:|---|---|
| 1 | `GetVariableXInHotPlateData` + `SearchPlateToPlace` | 決定放哪裡 |
| 100 | `MoveInArmXYToHotPlatePlace()` | XY 移動到 HP 座標 |
| 110 | IC 掉料處理 | 移動途中檢查 |
| 200 | `UpdateHPSuckGroup` + offset check + release delay | 到位後準備 |
| 340 | 等 release delay 結束 | for TSMC config |
| 350 | **逐顆 Destroy + DoPlaceToHPSwapData** | 核心放料區 |
| 400 | Z 安全位 → 是否還有 IC | 多輪迴路判斷 |
| 500 | `CheckInArmDestroyICFail()` | 確認吹完無殘留 |
| 501 | `AdjustShuttleWhichKitOrder()` + `AddHPSuckGroup()` | 資料收尾 |

## 6. Destroy 吹氣與 DoPlaceToHPSwapData 資料交換

### case 350 核心邏輯

```cpp
for(i=0; i<2; i++)           // Row: 0=Row1, 1=Row2
  for(j=0; j<iStepHP; j++)   // Col: 步幅由 GetHotPlateColStep 決定
    j2 = GetPlaceToHotPlateSuckCol(j)   // 吸嘴 index
    ix = GetPlaceToHotPlateCol(j)       // HP col index

    if Item[i][j2] == HAS_IC || HAS_NULL_IC:
      if HAS_NULL_IC || Destroy():      // HAS_NULL_IC 不用吹氣，直接做資料搬移
        DoPlaceToHPSwapData(i, j2, ip, iy, ix)
      else if !Error:
        flag = false                    // 還在 Destroy 中，下一輪再進來
```

**HAS_NULL_IC 不需要實際 Destroy**：這是因為邏輯上該位置本來就沒有實料，只要做資料搬移。

### DoPlaceToHPSwapData 做了什麼

1. 邊界檢查（iSuckRow/Col/iP/iPlateR/iPlateC）
2. 寫入 `iHotWhichShuttle[P][C][R]`, `iHotWhichKit[P][C][R]`, `iHotPlateCount[P][C][R]`
3. `PickFromHPList->SetArrPlateXY()` — 記錄哪個吸嘴放到哪個 HP 格，供後續 pick 回用
4. 設定 HotTime（Soak timer 起算用）
5. 設定 `bInArmCheckDestroyACT[][]`（非 NULL_IC 時驗證吹氣是否完成）
6. Laser 距離檢查（如啟用）
7. `iRowOnHotPlate[P][C][R]` — 記錄 Arm row/col 資訊
8. `InArmSuck.PordRec[].AddHPRecord()` — 生產追蹤
9. **衝突檢查（Error 1/2）**
10. Rotate Data Swap error 檢查
11. `InspectInArmPosition()` — 座標偏差檢測
12. `InArmSuck.CopyToTray()` — 正式把 InArm 資料搬到 HP tray

## 7. bZFlgToHP 與 Z 軸下降控制

`bZFlgToHP[i][j]` 是放料用的 Z 軸下降旗標：

- `true` → 該吸嘴對應的 Z 軸應下降到放料高度
- `false` → 該吸嘴 Z 維持安全位

由 `GetInArmZToHPPos()` 使用，計算每顆吸嘴的放料 Z 座標：

- `bZFlgToHP == true` → `Prod.ZInArm_Plate1/2_Place[i][j]`
- `bZFlgToHP == false` → `ZSafePos`（不下降）

**與 bZFlgToHPPick 的區別**：
- `bZFlgToHP`：放料用，by ainarm_SearchPlacePlate.cpp
- `bZFlgToHPPick`：取料用，by ainarm_SearchPickPlate.cpp

## 8. Shuttle / Kit 對 HotPlate 放料的影響

### 為什麼放料與 Shuttle 有關

1. `InArmSuck.iWhichSht`：記錄這批 IC 預定送往哪個 Shuttle
2. `InArmSuck.iWhichKit`：記錄送往 Shuttle 的哪個 Kit（左半/右半）
3. `DoPlaceToHPSwapData()` 將這兩個值寫入 `iHotWhichShuttle / iHotWhichKit`
4. **取料時**，`PickFromHPList` 會根據這些值決定 IC 取回後應該送往哪個 Shuttle/Kit

因此 **HotPlate 的放料格位選擇不僅要看 HP 空位，還要考慮後續取料的 Shuttle 對應關係**。

### Close-site 的影響

- 當某些 site 被 close（`TestIF.iSiteMap[r][c]==0`），InArm 配置會改變
- `InArmSuck.iPickRow` / `iPickCol` 可能從 2x4 降為 1x4 或 2x2
- `SearchPlateToPlace` 的 `Row/Col` 參數跟著改變
- HP 上的搜尋範圍與步幅也改變
- 嚴重情況：某些吸嘴完全無法到達 HP 的邊緣格位

### iWhichShtPickFor32

32-site 模式下，放 HP 時記錄的 shuttle 資訊會使用 `iWhichShtPickFor32`/`iWhichKitPickFor32`，因為前後 arm 分別負責不同 shuttle。

## 9. 吸嘴到 HotPlate 的映射複雜性

### GetPlaceToHotPlateSuckCol — 吸嘴 index 映射

根據 `iInArmType` 和 `HotPlateForm.XDivision` 決定吸嘴 [i][j] 中的 `j` 如何映射到 HP 列：

| iInArmType | XDiv | 映射邏輯 | 說明 |
|---|---|---|---|
| 1x1 | any | j2=0 or 1 | 單吸嘴 |
| AxEx | any | j2=j*2 | 交錯吸嘴 |
| AxxG | any | j2=j*3 | 間隔更大 |
| 2x4 | 6 (WideHP) | j2=j+iForPlaceHPX6Step*2 | 分兩步放 |
| 2x4 | 6 (normal) | j2=j*2+iForPlaceHPX6Step | 交錯 + 步幅 |
| 2x4 | 10 | j2=j or j+2 | 靠右時偏移 |
| 2x4 | 4 | j2=j | 直接對應 |

### GetPlaceToHotPlateCol — HP 列映射

吸嘴 loop index `j` → HP 格位 `ix`：

- 單吸嘴：`ix=j`
- AxEx/AxxG 配合 XDiv 有多種 pitch 計算
- 6x11 HP 的 WideHP 模式：`iPlateX + j*2`
- 4xN HP：`iPlateX + j`（直接步幅 1）

### GetHotPlateColStep — 每輪放幾列

`iAction==iPlaceHP` 時，依 XDiv 決定：

| XDiv | 正常步幅 | 邊界步幅 | 說明 |
|---|---|---|---|
| 4 | 4 | — | 一次放 4 列 |
| 6 (WideHP) | 4 或 2 | `iPlateC==4` → 2 | 最右邊只剩 2 列可放 |
| 6 (normal) | 2 | — | 交錯放 |
| 10 | 4 或 2 | `iPlateC==8` → 2 | 最右邊只剩 2 列 |
| 其他 | 4 | — | 預設 |

**這就是使用者提到的複雜性來源**：吸嘴物理間距 vs HP 格位間距的不匹配，加上邊界效應，導致某些吸嘴無法在某些 HP 位置放下料。

## 10. HotPlate XDivision 對放料路徑的影響

`SearchPlateToPlace()` 根據 XDivision 選擇完全不同的搜尋函式：

- XDiv=4 → `SearchPlacePlateXItem4_8Suck()`：Row + Col 配置靈活
- XDiv=6 → 分 WideHP / Normal / NotStandY 三種
- XDiv=8 → `SearchPlacePlateXItem8_8Suck()`
- XDiv=10/12/16 → 各自特化

每個搜尋函式內部的 `spacX/spacY/Row/Col` 配置都不同，導致：

1. **不同 HP 格式下，相同 InArm 配置的放料行為完全不同**
2. 某些 close-site 組合在特定 HP 格式下會出現「無法找到空位」
3. Hang Up 通常發生在搜尋迴圈跑滿 `iHangUpCount` 次後

## 11. Row2CanPutHP 與 iPlaceHPOrder

`Row2CanPutHP(PlaceMode)` 決定 Row2（下方列）的吸嘴是否可以放到 HP：

判定條件：
1. AutoClean 佔用了 HP2 → return false
2. 2x3_6_14 型態 → return false（固定限制）
3. `HotPlateYPitchCanPutAll()` → Y 間距是否允許放兩列
4. `iy + iYHalf >= HotPlateForm.YDivision` → Row2 放上去會超出邊界

`iPlaceHPOrder`：
- `0`：先放 Row1，若 Row2CanPutHP 才放 Row2
- `1`：僅放 Row2（特殊翻轉情況）

**在 case 350 中**，`Row2CanPutHP == false` 時 `i==1` 會被 `continue`，跳過所有 Row2 的吸嘴。

## 12. 放料後多輪迴路 (case 400)

`case 400` 在 Z 抬安全位後檢查 `InArmSuck.HasIC()`：

- 若仍有未放完的 IC → `Task=1`，重新執行 `SearchPlateToPlace` 再來
- 典型場景：6x11 HP，前 4 列已放，剩 2 列要第二輪放
- `iForPlaceHPX6Step` 控制是第 1 步還是第 2 步
- `iHotCount++` 每輪遞增

## 13. InspectInArmPosition 座標驗證工具

`ainarm9045.cpp` L7971，由 `bE74_InspectArmPosition`（Config.h L628）控制啟用。

### 原理

```
InspectInArmPosition(iTarget, iSuckRow, iSuckCol, iTargetRow, iTargetCol, bPlace)
```

1. 若 `bE74==false` → 直接 return
2. 跳過 `HAS_NULL_IC` / `HAS_NULL_CLEAN_IC`（無實料不用檢查）
3. 讀取實際 encoder 位置：`GetInArmCellPos(row, col, YEncoder, XEncoder)`
   - `XEncoder = MInArmX.ReadPos() + (PitchX/3)*(col - base)`
   - `YEncoder = MInArmY.ReadPos() - (PitchY)*(row - base)`
4. 計算目標硬體位置（根據 iTarget 查 Prod 教導值 + Pitch * 格位 + Offset）
5. 比較：`CheckArmPosArrival(HardwarePos, Encoder, iLimit)`
   - `iLimit = InputLimit.iOffsetXYHigh * 100`
   - 若 `|NowPos - Destination| <= Tolerance` → pass
6. 不匹配 → `ShowMyMessage` 顯示偏差警告

### 支援的目標

| iTarget | 名稱 | 用於 |
|---|---|---|
| MMTrayY | Loader | Pick from Loader |
| MMPlate1 | Hot Plate 1 | Place / Pick HP1 |
| MMPlate2 | Hot Plate 2 | Place / Pick HP2 |
| MInShuttle1/2 | Shuttle 1/2 | Place to Shuttle |
| MMAutoCleanKit | Clean Kit | AutoClean |
| MInRotateKit | In Rotator | Rotate |
| MPreciser | Precisor | 精確定位 |

### 呼叫位置

- `ainarm9045.cpp` L489/L510: Place to Shuttle 1/2（`bPlace=true`）
- `ainarm9045.cpp` L2626: Pick from Loader（`bPlace=false`）
- `ainarm_SearchPlacePlate.cpp` DoPlaceToHPSwapData 結尾: Place to HP（`bPlace=true`）

### 已知限制與改進方向

1. **僅檢查 XY，不檢查 Z**：實際放料高度偏差不會被捕捉
2. **Tolerance 由 `InputLimit.iOffsetXYHigh * 100` 決定**：可能太寬鬆
3. **32-site 模式下停用**：`bUseTwoArm32Site==true` 時不 ShowMyMessage
4. **不阻擋流程**：只顯示警告，不中斷自動運行（非 JAM/MES 級別）
5. **Pitch 計算依賴 encoder 即時讀值**：如果 pitch 馬達還在移動中讀值，會有瞬態偏差
6. **Shuttle 放料的 CellPos 使用 GetInArmCellPos 而非 GetInArmToShtCellPos**：L7996 已註解掉 Shuttle 專用版（可能影響精度）
7. **OutArm 版（`InspectOutArmPosition`）的 Pick from Shuttle 被 return 跳過**：尚未啟用

## 14. Data Swap Error 1/2 根因分析

（與取料部分的 deep-dive 交叉對照）

### Error 1：HP有料 + InArm也有料

- `HP Tray.Data == HAS_HOT_IC || HAS_IC` && `InArmSuck.Item == HAS_IC`
- 根因：HP 格位未被正確清除（前次 pick 只搬了資料沒清 HP），或搜尋演算法多次指向同一格

### Error 2：HP有料 + InArm無料

- `HP Tray.Data == HAS_HOT_IC || HAS_IC` && `InArmSuck.Item == HAS_NULL_IC`
- 根因：InArm 這格在前段被降級為 HAS_NULL_IC（pick error skip / vacuum fail），但 SearchPlateToPlace 仍指向了一個已有料的 HP 格位

### 追蹤方向

1. 檢查 `SearchPlateToPlace()` 的 `CheckHotPlateHasSpace_9045_8_New_V` 判定是否正確找到空位
2. 檢查前一輪 `HotplateDataConversion()` 是否正確清除 HP metadata
3. 檢查 `PickFromHPList` 的 team/group 資訊是否與實際 HP 狀態同步

## 15. Debug Checklist

遇到 InArm 放料到 HotPlate 問題時：

1. **確認 HP 格式**：`HotPlateForm.XDivision` / `YDivision` → 影響搜尋函式選擇
2. **確認 iInArmType**：決定走 `DoPlaceToHotPlate_9045_2x4_8` 或其他變體
3. **確認 close-site 狀態**：`TestIF.iSiteMap[][]` → 影響 Row/Col 步幅
4. **查 SearchPlateToPlace 輸出**：`iPlacePlate[0]`, `iPlacePlateY[0]`, `iPlacePlateX[0]`
5. **查 Destroy 是否全部完成**：`InArmSuck.Suck[i][j2].Error` 或仍在等待
6. **查 DoPlaceToHPSwapData**：是否觸發 Error 1/2
7. **查 Row2CanPutHP**：Row2 吸嘴是否因 Y pitch 限制被跳過
8. **查 bZFlgToHP**：哪些吸嘴的 Z 軸有下降
9. **查 InspectInArmPosition 警告**：是否出現座標偏差
10. **查 iHotCount**：多輪放料計數是否正確遞增
