---
name: hpi-bin-display
description: "HT9050與其他Handler機型的Bin Display共用入口：NUMBER_PANEL_TYPE 0～4、DIO脈衝／三色七段／TFT、Magazine協定、COM／NUMBER_PANEL2、TMyBinDispCtrl／TMyBinDispHT9046、Timer1Timer、Bin／Count／Color／ack、C14 bring-up、binsel.disp與Bin Display Status、S22 TFT修正、Error／X／0／灰格及通訊排錯。"
---

# Handler Bin Display

推送基準與後續裁決：[最新main整合](references/main-integration-20261006.md)。

一份Skill比較HT9050與其他機型的外接Bin顯示器、內部控制與狀態頁；依協定／資料流／機型分層。

## 先確認

- 先讀 [共同與差異](references/common.md)，確認NUMBER_PANEL_TYPE、MAGAZINE_BIN_DISP_TYPE、配置與實際COM接線。
- [目前main](references/runtime/current-main.md) 已有C14／S22；原20261003「尚未進main」與「912修正都不移植」記錄有後續裁決，不能當現況。
- 要收的Bin、發送目標、ack、GetBinNow鏡像、網頁字色與真實面板分開核對；狀態鏡像不是實體讀回證明。
- HT9050的TFT／COM裁決與目前Git快照值有差異，先核對最新機台快照；不把模擬組或舊設定當機台現值。
- 原UB防護、客戶gate、SIM／ctest不開COM的條件保留；本批不開埠、改設定或變更C14程式／HTML。

## 按問題選路

| 問題 | Reference |
|---|---|
| 共同資料流與機型差異 | [共同對照](references/common.md)／[機型](references/machines/index.md) |
| NUMBER_PANEL／Magazine、封包／COM | [硬體／協定](references/protocol/index.md) |
| Timer／WriteTargetBin／Count／顏色 | [控制流程](references/control/index.md)／[目前main](references/runtime/current-main.md) |
| Bin Display Status、灰X／jumpSeq | [網頁與鏡像](references/ui/index.md) |
| 通訊／錯色／0／COM錯接 | [排錯](references/source/references/troubleshooting.md) |
| 客戶選項與跨主題 | [客戶條件](references/customers.md)／[相鄰入口](references/related.md) |

## 查證與交付

以版本／檔名＋function／特定變數定位，Timer Task／case輔助；舊行號標為歷史。每批先fetch／整合main、驗原文與連結，補Change Log並追加同一整批MR。
