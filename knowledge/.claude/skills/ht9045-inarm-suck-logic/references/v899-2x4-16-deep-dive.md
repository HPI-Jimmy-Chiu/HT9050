# HT9045 V899 16-site InArm 吸取邏輯深度解析

## 1. 範圍與關鍵檔案

本文件以 V899 版本的 16-site `e9045_2x4_16` 為主，對 InArm 吸取路徑做三層解析：

1. `ainarm2.cpp`：InArm 入口 guard 與初始真空保護
2. `ainarm9045_2x4_16.cpp`：16-site 主狀態機、Loader pick、HotPlate pick
3. `mykitsuck.cpp`：單顆吸嘴真空狀態機

常用定位點：

| 檔案 | 函式 | 約略行號 | 用途 |
|---|---|---:|---|
| `ainarm2.cpp` | `DoInArm()` | 1585 | 入口 guard 與前置攔截 |
| `ainarm2.cpp` | `SetInArmUseSuckToHasNullIC()` | 194 | 將本輪使用但未吸到的 nozzle 標成 `HAS_NULL_IC` |
| `ainarm2.cpp` | `CheckInArmSuckInitial()` | 1198 | 初始真空狀態一致性檢查 |
| `ainarm9045_2x4_16.cpp` | `DoArmPickFromLoadStage_9045_2x4_16()` | 1011 | Loader 取料主流程 |
| `ainarm9045_2x4_16.cpp` | `DoInArmPickFromHotPlate_9045_2x4_16()` | 1614 | HotPlate 取料主流程 |
| `ainarm9045_2x4_16.cpp` | `DoInArm_9045_2x4_16_SuckerMap()` | 2879 | 16-site 吸嘴映射 |
| `ainarm9045_2x4_16.cpp` | `DoInArm_9045_2x4_16()` | 2927 | 主狀態機 |
| `ainarm_SearchPickPlate.cpp` | `HotplateDataConversion()` | 157 | HP 到 InArm 的資料搬移與吸取判定 |
| `ainarm_SearchPickPlate.cpp` | `SearchPlateToPick()` | 1222 | 決定 HotPlate first team |
| `ainarm_SearchPlacePlate.cpp` | `DoPlaceToHPSwapData()` | 4615 | InArm 放 HotPlate 後的資料交換 |
| `mykitsuck.cpp` | `TMySucker::GetStatus()` | 1909 | 真空感測器讀值 |
| `mykitsuck.cpp` | `TMySucker::Suck()` | 2152 | 單顆吸嘴吸取狀態機 |
| `mykitsuck.cpp` | `TMySucker::Destroy()` | 2349 | 單顆吸嘴吹氣 / 釋放狀態機 |
| `csystem.cpp` | `CheckInArmSuckFromLoaderICFallDown()` | 1715 | Loader 吸完後掉料檢查 |
| `csystem.cpp` | `CheckSuckInitialStatus()` | 2342 | 初始真空異常型別判斷 |

## 2. 三層架構總覽

### 第 1 層：入口保護

`DoInArm()` 先攔掉不該進流程的條件：

- `bInitialStartIndexCheckDone == false`：初始化尚未完成
- `iHPHangUpCount != 0`：HotPlate Hang Up
- `bF16CheckShuttleSensorBroken && bDoingF16`：Shuttle sensor broken 保護
- QA mode 可能直接把空位補成 `HAS_NULL_IC`
- Auto Alignment 執行中、Index jam、reset / pause 等條件

這一層不做吸取，只決定 `DoInArm_9045()` 是否能進入。

### 第 2 層：16-site 主狀態機

`DoInArm_9045_2x4_16()` 主要路由：

```text
case 1    -> Move wait
case 10   -> 依 InArm 現有資料判斷要去 Loader / HP / Shuttle
case 50   -> 準備 Loader pick 或 HP wait
case 75   -> 是否先做 HP try-suck 驗證
case 100  -> Loader pick
case 200  -> Close-site 檢查
case 400  -> Additional function
case 500  -> 等 HotPlate ready
case 1000 -> 判斷是否可直接送 Shuttle
case 1100 -> Place to HotPlate
case 1500/1501 -> Pick from HotPlate
case 1600 -> HP pick 後 close-site 異常處理
case 2000 -> Place to Shuttle
case 15000 -> TryPick HP verify
```

### 第 3 層：單顆吸嘴真空狀態機

`TMySucker::Suck()` 不是一次函式呼叫就完成，而是多次被主循環重入：

1. 關閉 destroy、開啟 vacuum
2. 等待 sensor 成立或 timeout
3. 真空成立後再等 `OnDelayTime`
4. 成功才回傳 `true`
5. 若 timeout，先進入補償 / grace 狀態，最後才把 `Error=true`

結論：

- `Suck() == false` 只代表「尚未成功完成」，不等於立即失敗
- 真正失敗要看 `InArmSuck.Suck[i][j].Error`

## 3. 16-site 吸嘴映射怎麼建立

`DoInArm_9045_2x4_16_SuckerMap()` 會把 16-site 的 socket 使用狀態轉成 InArm 可用吸嘴表：

- `Prod.fInArmSuckUse_TwoArm32Site[][]`：兩 arm / 32-site 視角的使用表
- `Prod.fInArmSuckUse[0/1][][]`：切回目前 InArm 的使用視角
- Auto Site Mapping 模式下，映射可能由 `DoInArmAutoSiteMapping()` 直接改寫

因此「哪顆吸嘴本輪應該吸」不是看 `InArmSuck.Item`，而是看：

- `InArmSuckUse[][]`
- `Prod.fInArmSuckUse_*`
- HotPlate team 時再加 `bZFlgToHPPick[][]`

## 4. Loader 取料邏輯

### 4.1 流程骨架

`DoArmPickFromLoadStage_9045_2x4_16()` 的主線：

```text
1    -> Z safe / 前置條件
10   -> Loader 是否有料、有沒有 tray sensor 問題
12   -> XY 到 Loader 取料點
200  -> Z 下降到 pick 高度，做 offset check
1000 -> 逐顆 Suck()
1010 -> Retry：先升 Z，再重試
1020 -> 在同位置重試 pick
1050 -> 超過 retry 上限，轉錯誤處理
1100 -> MES0101 / tray end / clean out / skip / home / retry
2000 -> Z safe 後做掉料檢查
2100 -> 收尾，必要時把 `NULL_IC` 轉成 `HAS_NULL_IC`
```

### 4.2 真正吸取發生在 case 1000

關鍵條件：

- `InArmSuckUse[i][j] == true`
- `InArmSuck.Item[i][j] == NULL_IC`
- `bSuckEnd[i][j] == false`

每顆 nozzle 都會呼叫：

```cpp
if(bSuckEnd[i][j]==false && InArmSuck.Suck[i][j].Suck())
```

呼叫後有三種結果：

1. `Suck() == true`
   - 呼叫 `AddLoadingCount()`
   - `bSuckEnd[i][j] = true`
   - `InArmSuckUse[i][j] = false`
   - `CopyFromTray(... HAS_IC ...)`
   - 該格資料正式從 Loader tray 搬到 InArm

2. `Suck() == false && Error == false`
   - 代表仍在等待真空 / delay
   - `flag1 = false`
   - 此輪流程停在 case 1000，下一圈再進來

3. `Error == true`
   - `bSuckEnd[i][j] = true`
   - 記錄錯誤 tray 座標
   - 稍後進 1010 或 1050

### 4.3 `AddLoadingCount()` 做了什麼

這是 Loader pick 成功後的資料搬移核心：

- 設 `bSuckEnd = true`
- 關閉本輪該吸嘴的 `InArmSuckUse`
- 清 `bPickLoaderDuplicateErr`
- 計數 `AddInArmPickerCount()`
- `InArmSuck.CopyFromTray(... HAS_IC ...)`
- `InspectInArmPosition()` 記錄吸嘴與 tray 格位對應

也就是說，`AddLoadingCount()` 不只是統計，還是資料從 Loader 轉進 InArm 的實際提交點。

### 4.4 Retry / Alarm / 清料路徑

- `1010`：先抬 Z 到安全位
- `1020`：如果 `bE56LoaderRetryAtSamePosition` 啟用，原點位重吸
- `1050 -> 1100`：超過 retry 上限，進 `ProcessMES0101InArmPickLoaderError()`

`1100` 可能得到：

- `K_TRAY_END`
- `K_CLEAN_OUT`
- `K_SKIP`
- `K_HOME`
- `K_RETRY`

對應後續動作不是固定 alarm，而是會進一步呼叫 tray end / clean out / skip / home 邏輯。

### 4.5 為什麼成功後還會再檢查掉料

`case 2000` 在 Z 抬回安全位之後，會跑 `CheckInArmSuckFromLoaderICFallDown()`。

判斷條件：

- 該 nozzle 啟用感測器
- `InArmSuck.Item[i][j]` 不是 `NULL_IC` / `HAS_NULL_IC`
- 但 `GetStatus() == false`

這代表：

- 先前吸取曾成功
- 但離開 tray 後真空掉了，料掉了

掉料後可依設定走 `retry` 或 `auto skip`。

### 4.6 `HAS_NULL_IC` 在 Loader 路徑的意義

收尾時會呼叫 `SetInArmUseSuckToHasNullIC(iSht, iKit)`。

它的意思是：

- 本輪配置裡該吸嘴本來要參與取料
- 但最後資料仍是 `NULL_IC`
- 流程把它轉成 `HAS_NULL_IC`

這是邏輯佔位，用來告訴後段流程「這格 site 本輪已處理，但沒有實料」。

## 5. 初始真空一致性檢查

`DoInArm_9045_2x4_16()` 在 `case 10` 一開始就會先呼叫：

- `CheckInArmSuckInitial()`

它逐顆用 `CheckSuckInitialStatus()` 比對：

- `flag1`：sensor 當前是否有真空
- `flag2`：`InArmSuck.Item` 是否代表應該有實料

`flag2` 判定規則：

- 不是 `NULL_IC`
- 不是 `HAS_NULL_IC`
- 不是 `HAS_TRY_SUCK_IC`

才算應該有真空。

不一致時：

- sensor 有真空，但資料不該有料 -> `Vaccum_Initial_On`
- sensor 沒真空，但資料顯示有實料 -> `Vaccum_Initial_Off`

對外會收斂成 `WAR0132`。

若使用者按 `SKIP`：

- `HAS_HOT_IC` 會被改成 `NULL_IC`
- 其他不一致資料會改成 `HAS_NULL_IC`

這說明 `WAR0132` 本質上是「資料面與感測面不同步」，不只是一顆吸嘴 NG。

## 6. HotPlate 取料邏輯

### 6.1 先決定要吸哪一 team

`DoInArmPickFromHotPlate_9045_2x4_16()` 一開始：

1. `DoInArm_9045_SuckerMap()`
2. `SearchPlateToPick()`
3. `PickFromHPList->GetHPFirstTeamSuckUse(InArmSuckUse)`

`SearchPlateToPick()` 本質上是在決定：

- 哪個 plate
- 哪個 first team
- 對應哪一組 shuttle / kit

### 6.2 `bZFlgToHPPick[][]` 才是本輪 HP pick 的核心旗標

當 `PickFromHPList->GetHPFirstTeam(...)` 執行後：

- `bZFlgToHPPick[i][j] = true`：這顆 nozzle 本輪應該對應到某個 HotPlate 格位
- `false`：這顆不參與本輪 HP pick

因此 HP pick 是否應該吸，不是只看 `InArmSuckUse`，而要看：

- `bSuck[i][j]`
- `bZFlgToHPPick[i][j]`

### 6.3 真正的 HP 吸取發生在 `HotplateDataConversion()`

`case 300` 會針對本輪 `bSuck == true` 的 nozzle 呼叫 `HotplateDataConversion()`。

此函式同時做三件事：

1. 若該 plate 格位資料存在，嘗試建立真空
2. 真空成功後，把 HotPlate 格位資料搬到 InArm
3. 把 HotPlate 對應 metadata 清掉或更新 SiteMap

### 6.4 `HotplateDataConversion()` 的關鍵分支

#### 分支 A：HotPlate 該格已是 `HAS_NULL_IC`

如果：

- `InArmSuck.Item == NULL_IC`
- `HotPlate Tray.Data == HAS_NULL_IC`

流程會直接：

- `CopyFromTray(... HAS_NULL_IC ...)`
- 清該格 `iHotWhichShuttle / iHotWhichKit / iHotPlateCount`

意義：不是硬體吸到料，而是把「此 team 這格本來就沒有實料」的邏輯狀態搬回 InArm。

#### 分支 B：本輪應該吸 (`bZFlgToHPPick == true`) 且 `Suck()` 成功

會做：

- `AddInArmPickerCount()`
- 清 `bZFlgToHPPick` / `bPickHPDuplicateErr`
- 記錄 `HotCount`、`WhichKitBackup`、`WhichShuttleBackup`
- `CopyFromTray()` 把 HP 格位資料搬到 InArm
- `InspectInArmPosition()`
- 清 HotPlate metadata 或更新 Auto Site Map log

#### 分支 C：本輪不該吸或 team 裡沒有資料

若 `bZFlgToHPPick == false` 且 `InArmSuck.Item == NULL_IC`，流程會把該格改成 `HAS_NULL_IC`。

這是 HP 路徑非常容易被誤解的地方：

- 它不代表這顆 nozzle 吸失敗
- 只是表達「此輪 team 對應的這顆位置邏輯上沒有料」

### 6.5 HP pick error 如何收斂成 `JAM0109`

`case 320 -> 330` 會統計 `InArmSuck.Suck[i][j].Error`。

超過 retry 次數後：

- 顯示 `JAM0109`
- 使用者可選 `RETRY` / `SKIP` / `HOME`（依設定）

若選 `SKIP`：

- 會呼叫 `PorcessJAM0109HotPlatePickUpErrorSkip()`
- 將該 HotPlate 位置資料轉為 `HAS_NULL_IC` 搬回 InArm
- 同步清 / 更新 HP metadata

因此 HP pick 的 skip 不是直接丟掉資料，而是把此顆轉成邏輯空料繼續往後走。

### 6.6 取完一 team 後怎麼往下走

`case 350` 會呼叫：

- `DoInArm_CheckSuckerMap()`
- `PickFromHPList->DataForwardAndNextTeam()`

意思是：

- 檢查這一 team 的吸嘴映射結果
- 若還有下一 team，繼續循環
- 若沒有，回到 `case 400` 結束 HP pick

## 7. HotPlate 放料與 Data Swap error

放料成功後真正的資料交換發生在 `DoPlaceToHPSwapData()`。

它會先寫：

- `iHotWhichShuttle`
- `iHotWhichKit`
- `iHotPlateCount`
- `PickFromHPList->SetArrPlateXY(...)`

然後檢查衝突。

### Error 1

條件：

- `HotPlate Tray.Data == HAS_HOT_IC || HAS_IC`
- `InArmSuck.Item == HAS_IC`

含義：

- HotPlate 該格本來就有實料
- InArm 又想再放一顆實料進去

這是重複放料或資料未清乾淨。

### Error 2

條件：

- `HotPlate Tray.Data == HAS_HOT_IC || HAS_IC`
- `InArmSuck.Item == HAS_NULL_IC`

含義：

- HotPlate 該格已有資料
- InArm 這格卻是邏輯空料

這通常代表前段資料同步已經錯位，不是單純真空硬體問題。

### 延伸判讀

因此遇到 `HotPlate Data Swap error 1/2` 時，應追：

1. `HotplateDataConversion()` 是否正確清除 HP metadata
2. `DoPlaceToHPSwapData()` 前 `InArmSuck.Item` 是否已被錯誤改成 `HAS_NULL_IC`
3. `PickFromHPList` 與 `bZFlgToHPPick` 是否仍指向舊 team

## 8. `TMySucker::Suck()` 真空狀態機解讀

### 8.1 重要欄位

- `OnTask`：吸取內部 state
- `RetryCT` / `rct`：內部 retry 控制
- `OnAlarmTime`：真空建立 timeout
- `OnDelayTime`：真空建立後再等待的穩定時間
- `Error`：本顆吸嘴已確定失敗
- `bSuckOK`：此顆吸取動作流程已完成

### 8.2 實機模式流程

1. `DoOffIO(false)`：先關 destroy
2. `DoOnIO(true)`：開 vacuum
3. 若 sensor 已成立，進 `OnTask=100`
4. 否則進 `OnTask=50` 等 `OnAlarmTime`
5. 成立後再等 `OnDelayTime`
6. delay 結束才回傳 `true`

### 8.3 何時 `Error=true`

不是 sensor 一次沒到就立刻報錯，而是：

1. timeout 後可能先回到 `OnTask=200`
2. 經過補償 timer 後才把 `Error=true`

所以主流程一定要用：

- `Suck() == true` 判成功
- `Error == true` 判確定失敗

中間的 `false && Error == false` 只是 아직未完成。

### 8.4 `GetStatus()` 與 `Sensor()`

`GetStatus()` 直接讀真空 sensor，並依 `TYPE_A / TYPE_B` 反相；`Sensor()` 只是對 `Enable == false` 做包裝。

這表示：

- `InArmSuck.Item` 是邏輯資料層
- `GetStatus()` 才是實體真空層

兩者一旦不同步，就會在 `WAR0132`、掉料檢查或 swap error 中爆出來。

## 9. `HAS_NULL_IC` 的真正角色

整條 InArm 吸取邏輯中，`HAS_NULL_IC` 不是異常碼，而是流程占位資料。

常見來源：

1. Loader / HP 本輪本來就應該有這個 site，但沒有實料
2. Pick error 選 `SKIP`
3. 初始真空不一致時使用者選 `SKIP`
4. Auto Site Mapping 在 HotPlate / team 清理時補上的空料

因此看見大量 `HAS_NULL_IC`，先不要直接判定 vacuum NG，要先分清：

- 它是正常的邏輯占位
- 還是因為前段錯誤被動補上

## 10. Debug Checklist

遇到 InArm 吸取問題時，建議照這個順序查：

1. 先看 `DoInArm()` 有沒有在入口 guard 就被擋住。
2. 若是 Loader 問題，直接進 `DoArmPickFromLoadStage_9045_2x4_16()` 的 `case 1000/1010/1050/1100/2000/2100`。
3. 若是 HotPlate 問題，直接看 `DoInArmPickFromHotPlate_9045_2x4_16()` 的 `case 300/320/330/350`。
4. 查 `InArmSuck.Item[][]` 是否已被改成 `HAS_NULL_IC`。
5. 查同位置的 `GetStatus()` 是否仍有真空，排除資料 / 感測不同步。
6. 查 `bZFlgToHPPick[][]` 是否正確反映本輪 HP first team。
7. 若是 swap error，回頭核對 `DoPlaceToHPSwapData()` 前後的 HotPlate tray data 與 `PickFromHPList`。

## 11. 一句話總結

V899 16-site 的 InArm 吸取邏輯不是單純的 `吸到 / 沒吸到`，而是三層同時運作：

- 主狀態機決定何時去 Loader / HotPlate / Shuttle
- `InArmSuck.Item` 維護邏輯資料狀態
- `TMySucker::Suck()` 維護實際真空建立與失敗判定

大部分 hang up、空料誤判、HotPlate swap error，都是這三層不同步造成的。