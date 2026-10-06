---
name: ht9045-inarm-suck-logic
description: HT9045 InArm 吸取 / 真空判定 / HotPlate 放料邏輯知識庫。當使用者詢問 InArm 吸取、Loader pick、真空吸取、Vacuum sensor、HAS_NULL_IC、HAS_TRY_SUCK_IC、HotPlate pick/place、HotPlate Data Swap、掉料、Drop Error、WAR0132、JAM0109、MES0101、DoArmPickFromLoadStage_9045_2x4_16、DoInArmPickFromHotPlate_9045_2x4_16、TMySucker::Suck、16-site 2x4_16 吸嘴映射、SearchPlateToPlace、DoPlaceToHotPlate、DoPlaceToHPSwapData、bZFlgToHP、bE74_InspectArmPosition、InspectInArmPosition、InspectOutArmPosition、座標偏差檢測、XDivision、Row2CanPutHP、GetHotPlateColStep、GetPlaceToHotPlateSuckCol 等相關問題時，應先載入此技能。
  ⚠ **流程層不在這裡**：iArmTask 狀態機、Tray End、Clean Out、Auto Site Mapping、Pick Error 的 Retry/Skip/Home 分支 → 用 `ht9045-inarm-flow`。
---

# ht9045-inarm-suck-logic 相容入口

同主題已整合到 [hpi-inarm-flow](../hpi-inarm-flow/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-inarm-flow/references/vacuum/original-entry.md)

## 適用範圍

[讀取此節](../hpi-inarm-flow/references/vacuum/original-entry.md#適用範圍)

## 快速入口

[讀取此節](../hpi-inarm-flow/references/vacuum/original-entry.md#快速入口)

## 回答流程

[讀取此節](../hpi-inarm-flow/references/vacuum/original-entry.md#回答流程)

## 關鍵資料語意

[讀取此節](../hpi-inarm-flow/references/vacuum/original-entry.md#關鍵資料語意)

## 高風險誤判

[讀取此節](../hpi-inarm-flow/references/vacuum/original-entry.md#高風險誤判)

## 參考文件

[讀取此節](../hpi-inarm-flow/references/vacuum/original-entry.md#參考文件)
