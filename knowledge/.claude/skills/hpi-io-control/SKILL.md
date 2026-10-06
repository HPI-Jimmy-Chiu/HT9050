---
name: hpi-io-control
description: "Handler IO共用入口，整合HT9050與其他機型的TMyCylinder／TMySensor／TMySwitch／TMySucker／TMyKitSuck／TLaneIO、IO_Table與Enable、ISABase、PCI1203／MotionNet／MN200／ISA／安全PLC定址、四嘴合吸與吸嘴矩陣、氣缸到位、IO頁Alias／modal例外、Exit停機與Q44。分析IO、吸嘴底層、Enable或急停／安全門資料來源時使用。"
---

# Handler IO Control

通用分層評估：[標頭瘦身與 Motor／IO 隔離](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/ee4ed4df7a506badb6cb0ce72769eaeb8b2aebb5/.claude/skills/cpp_build/references/header-slimming-and-motor-io-isolation-20261006.md)（ST01-M，MR !262；評估提案，尚非本批實作）。

推送基準與後續裁決：[最新main整合](references/main-integration-20261006.md)。

同一份Skill管理HT9050與其他機型的IO層；依物件、定址、表格、拓樸或畫面問題選reference。

## 先確認

- 先讀 [共同與差異](references/common.md)，確認版本、機型、IO_CARD_TYPE、ISABase及實際接線／快照。
- 以 [目前main](references/runtime/current-main.md) 核對函式與gate；舊SDK範例或同事架構說明不等於目前caller。
- Enable=0、氣缸到位回true、開關快取、真正輸出與吸到IC分開判讀；不得把資料狀態當硬體證據。
- HT9050的1203 Bit不参與路由定址；四嘴合吸不是四顆IC。安全門／急停／IO頁各有不同條件。
- IO表、PLC、安全門及write gate依原裁決處理；此整理保留原文，不變更機台參數或輸出。

## 按問題選路

| 問題 | Reference |
|---|---|
| IO共同項與機型差異 | [共同對照](references/common.md)／[機型分流](references/machines/index.md) |
| Cylinder／Sensor／Switch與Enable | [物件層](references/classes/index.md)／[表格載入](references/tables/index.md) |
| MotionNet／MN200／1203／PLC定址 | [IO卡層](references/cards/index.md)／[目前路由](references/runtime/current-main.md) |
| TMySucker／TMyKitSuck、四嘴與矩陣 | [真空／拓樸](references/vacuum/index.md) |
| IO頁Alias、退不出、Exit停機 | [畫面／關站](references/ui/index.md) |
| 客戶條件與跨主題 | [客戶索引](references/customers.md)／[相鄰入口](references/related.md) |

## 查證與交付

使用版本／檔名＋function／特定變數定位，狀態機加Task／case；原行號只保留為歷史。每批推前fetch／整合main、驗原文及連結、補Change Log，追加同一整批MR。
