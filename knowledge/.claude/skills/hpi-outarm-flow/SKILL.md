---
name: hpi-outarm-flow
description: "Handler OutArm流程共用入口，整合HT9050與其他機型的DoOutArm／OutArmTask、Shuttle取料、Single Picker／iInArmType、SmallY、Unloader／Auto／Fix／Magazine／BinBox放料、SearchTrayToPlace、Destroy、Retry／Skip／Home、Clean Out、Rotator／AOI、Pitch／Offset、JAM0203／0217／1940及E90退讓知識。分析出料手臂、分類盤、吸料或相機定位時使用。"
---

# Handler OutArm Flow

推送基準與後續裁決：[最新main整合](references/main-integration-20261006.md)。

HT9050與其他機型共用一個OutArm Skill；依取料、附加功能、搜盤、放料、機型及客戶選reference。

## 先確認

- 先讀 [共同與差異](references/common.md)；確認`USE_PICKER_COUNT`、`iInArmType`、`OutArmTask`與run mode。
- 先從 [當前guard／dispatcher](references/runtime/current-main.md) 追callee；函式9045名稱也涵蓋通用單Picker，不能排除HT9050。
- `OutArmSuck.Item`、真空、Bin、Tray.Data與實際取放成功分開；Retry／Skip／Home先看原配置Task。
- HT9050有OutArm Y伺服與SmallY氣缸，沒有Fix選項；不可套舊概念頁「Y是氣缸」或其他機型Fix退讓路線。
- AOI／Pitch／Offset與Teach分層核對；保留原例子的單位、機型、安全條件與推論界線。

## 按問題選路

| 問題 | Reference |
|---|---|
| 共同流程與機型差異 | [共用對照](references/common.md)／[機型層](references/machines/index.md) |
| 入口、guard、Task與取料 | [流程樹](references/flow/index.md)／[目前main](references/runtime/current-main.md) |
| 搜盤、Auto／Fix／Magazine、Destroy與計數 | [放料／Bin](references/place/index.md) |
| Rotator、AOI、XPitch與YPitch／Offset | [幾何與附加功能](references/geometry/index.md) |
| Fix3／JAM1940／1941案例 | [歷史警報案例](references/flow/references/fix3-cylinder-jam1940-false-alarm.md) |
| 客戶條件與跨主題 | [客戶索引](references/customers.md)／[相鄰入口](references/related.md) |

## 查證與交付

以版本／檔名＋function／關鍵變數定位，流程再加Task／case；舊行號僅是歷史定位。每批先fetch／整合main、驗證原文與連結並補Change Log，沿同一整批MR追加；本整理不操作機台或修改執行期設定。
