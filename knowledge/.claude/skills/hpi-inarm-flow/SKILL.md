---
name: hpi-inarm-flow
description: "Handler InArm流程與吸取共用入口，整合HT9050和其他機型的DoInArm、iArmTask、Loader取料、HotPlate放取／帳本、Shuttle放料、Single Picker／iInArmType、TMySucker、HAS_NULL_IC／HAS_TRY_SUCK_IC、真空、Destroy、Pitch／基準軸、ASM、Clean Out、Retry／Skip／Home、WAR0132／0150／0152／0157、JAM0109及Data Swap知識。分析入料手臂、吸料與熱盤幾何或掛機時使用。"
---

# Handler InArm Flow

推送基準與後續裁決：[最新main整合](references/main-integration-20261006.md)。

HT9050與其他機型共用一個InArm Skill；依流程、真空／資料、熱盤、幾何與機型選reference。

## 先確認

- 先讀 [共同與差異](references/common.md)，確認版本、`USE_PICKER_COUNT`、`iInArmType`與run mode；不由site數推吸嘴配置。
- `InArmSuck.Item`是資料，真空另查感測與`TMySucker`；`Suck()==false`可能仍在等待，`HAS_NULL_IC`不是單純吸料故障。
- 先查 [當前入口](references/runtime/current-main.md) 的guard與dispatcher，再追Task；歷史案例與提案不能直接當目前main已修／未修。
- HT9050單Picker的四嘴合吸一顆、Loader教導與飛梭配置查 [機型差異](references/machines/ht9050.md)，不可套多吸嘴映射。
- HotPlate空位、吸嘴資料、`PickFromHPList`帳本與ASM借還分開核對；保留舊裁決與回退紀錄。

## 按問題選路

| 問題 | Reference |
|---|---|
| 共同介面與機型分界 | [共同與差異](references/common.md)／[HT9050](references/machines/ht9050.md)／[其他配置](references/machines/other-models.md) |
| 入口guard、dispatcher與狀態機 | [流程樹](references/flow/index.md)／[目前main](references/runtime/current-main.md) |
| 真空、占位、掉料、Data Swap與16-site | [吸取樹](references/vacuum/index.md) |
| 熱盤搜尋、Pitch、ASM帳本、幽靈帳與落點 | [HotPlate樹](references/hotplate/index.md) |
| 吸嘴排列、基準軸、Pitch與Teach | [幾何樹](references/geometry/index.md) |
| 客戶條件與跨模組 | [客戶索引](references/customers.md)／[相鄰主題](references/related.md) |

## 查證與交付

以版本／檔名＋function名稱＋關鍵變數定位，流程再加Task／case。舊程式行號僅是歷史定位；V899／golden只讀，修正交付依V912／V906授權。每批fetch／整合main、驗證原文與連結並補Change Log；此整理不運行機台或改執行期設定。
