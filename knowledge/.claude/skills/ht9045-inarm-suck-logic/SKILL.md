---
name: ht9045-inarm-suck-logic
description: HT9045 InArm 吸取 / 真空判定 / HotPlate 放料邏輯知識庫。當使用者詢問 InArm 吸取、Loader pick、真空吸取、Vacuum sensor、HAS_NULL_IC、HAS_TRY_SUCK_IC、HotPlate pick/place、HotPlate Data Swap、掉料、Drop Error、WAR0132、JAM0109、MES0101、DoArmPickFromLoadStage_9045_2x4_16、DoInArmPickFromHotPlate_9045_2x4_16、TMySucker::Suck、16-site 2x4_16 吸嘴映射、SearchPlateToPlace、DoPlaceToHotPlate、DoPlaceToHPSwapData、bZFlgToHP、bE74_InspectArmPosition、InspectInArmPosition、InspectOutArmPosition、座標偏差檢測、XDivision、Row2CanPutHP、GetHotPlateColStep、GetPlaceToHotPlateSuckCol 等相關問題時，應先載入此技能。
  ⚠ **流程層不在這裡**：iArmTask 狀態機、Tray End、Clean Out、Auto Site Mapping、Pick Error 的 Retry/Skip/Home 分支 → 用 `ht9045-inarm-flow`。
---

# HT9045 InArm Suck Logic

## 適用範圍

- 預設分析版本：`d:\HT9045\HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422\`
- 預設機型配置：`e9045_2x4_16`（16-site）
- 問題類型：InArm 吸取失敗、真空異常、HAS_NULL_IC 流向、HotPlate pick/place 資料轉換、吸嘴對位與 Auto Site Mapping

## 快速入口

- 入口 guard：`ainarm2.cpp` 的 `DoInArm()`
- 主狀態機：`ainarm9045_2x4_16.cpp` 的 `DoInArm_9045_2x4_16()`
- Loader 取料：`ainarm9045_2x4_16.cpp` 的 `DoArmPickFromLoadStage_9045_2x4_16()`
- HotPlate 取料：`ainarm9045_2x4_16.cpp` 的 `DoInArmPickFromHotPlate_9045_2x4_16()`
- HotPlate 取料資料轉換：`ainarm_SearchPickPlate.cpp` 的 `HotplateDataConversion()`
- HotPlate 放料資料交換：`ainarm_SearchPlacePlate.cpp` 的 `DoPlaceToHPSwapData()`
- 真空底層：`mykitsuck.cpp` 的 `TMySucker::Suck()` / `TMySucker::Destroy()`
- 初始真空一致性：`ainarm2.cpp` 的 `CheckInArmSuckInitial()`、`csystem.cpp` 的 `CheckSuckInitialStatus()`
- HotPlate 放料狀態機：`ainarm9045_2x4_8.cpp` 的 `DoPlaceToHotPlate_9045_2x4_8()`
- HotPlate 空位搜尋：`ainarm_SearchPlacePlate.cpp` 的 `SearchPlateToPlace()`
- 座標偏差檢測：`ainarm9045.cpp` 的 `InspectInArmPosition()`（參數 `bE74_InspectArmPosition`）

## 回答流程

1. 先判斷異常位於哪一段：`進入 DoInArm 前`、`Loader 取料`、`HotPlate 取料`、`放 HotPlate`、`取完後掉料`。
2. 區分是 `動作尚未完成` 還是 `真空已失敗 / 資料已錯位`。
3. 追三組核心狀態：
   - `InArmSuck.Item[][]`
   - `bSuckEnd[][]`、`bPickLoaderDuplicateErr[][]`、`bPickHPDuplicateErr[][]`
   - `bZFlgToHPPick[][]`
4. 對照常見 alarm / message：
   - `WAR0132`：初始真空狀態與 `InArmSuck.Item` 不一致
   - `MES0101`：Loader pick error 收斂處理
   - `JAM0109`：HotPlate pick error
   - `HotPlate Data Swap error 1/2`：HotPlate 與 InArm 資料衝突
5. 需要深挖時讀 [references/v899-2x4-16-deep-dive.md](references/v899-2x4-16-deep-dive.md)

## 關鍵資料語意

- `NULL_IC`：該吸嘴目前沒有資料，也不預期有料
- `HAS_NULL_IC`：該吸嘴屬於本輪使用範圍，但最後沒有吸到實料
- `HAS_TRY_SUCK_IC`：HotPlate 驗證用暫態標記
- `HAS_IC`：一般實料
- `HAS_HOT_IC`：從 HotPlate 帶回的實料

## 高風險誤判

- `Suck()` 回傳 `false` 不一定是失敗，常見情況只是還在等真空建立或 on-delay。
- `InArmSuck.Item` 不是感測器；真空是否真的成立仍以 `TMySucker::GetStatus()` 為準。
- `HAS_NULL_IC` 是流程故意保留的邏輯占位，不等於硬體異常。
- `DoPlaceToHPSwapData()` 的 error 1/2 是資料交換衝突，不是單純 nozzle vacuum NG。
- Auto Site Mapping 會改寫 `Prod.fInArmSuckUse_*`、HP first team 與 `bZFlgToHPPick`，分析前要先確認 run mode。

## 參考文件

- 吸取邏輯深度解析：[references/v899-2x4-16-deep-dive.md](references/v899-2x4-16-deep-dive.md)
  - When to read: Loader pick、HotPlate pick、真空狀態機、HAS_NULL_IC 流次、WAR0132 問題
- HotPlate 放料深度解析：[references/v899-place-to-hotplate.md](references/v899-place-to-hotplate.md)
  - When to read: Place to HotPlate、SearchPlateToPlace、Destroy 吹氣、Data Swap Error、吸嘴映射複雜性、bE74_InspectArmPosition 座標驗證