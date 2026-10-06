---
name: hpi-temperature
description: "Handler 溫度控制總入口，整合HT9050與HT9045／其他機型的溫控共同項與差異。分析DoThermo、HeaterThread、fast clock、溫度完成／WAR15、71通道、HEATER_CTRL_TYPE／HeaterInsOpt／USE_16_HEATER、KT4H／E5DC／EJ1N／DTK／DTB／DTM／RKC、Temp_Set／LotInfo與溫度畫面對實體位址時使用。包含控制器協定、設定、模擬與出貨版本界線；ATC及EP壓力由相鄰主題導向。"
---

# Handler Temperature

推送基準與後續裁決：[最新main整合](references/main-integration-20261006.md)。

HT9050與其他機台共用一個溫度Skill；依迴圈、控制器、通道／機型、設定與顯示選讀reference。

## 先確認

- 先讀 [共同與差異](references/common.md) 及 [目前main對照](references/runtime/current-main.md)。保存件的日期／版本不等於今日現況。
- 溫度通道、畫面格子、控制器站／CH與實體感測器分開查；同名或同enum不代表同接線。
- `HEATER_CTRL_TYPE`、逐通道`HeaterInsOpt_`與`USE_16_HEATER`作用不同；保留Q34／Q71～76的原裁決及BCB相容風險。
- SIM與SHIP的heater beat不同；不得用SIM顯示或通訊替身證明實體溫控正常。
- EP／ADAM6024是類比壓力I/O；不當成溫控器或溫度完成證據。

## 按問題選路

| 問題 | Reference |
|---|---|
| 迴圈／加熱判斷／目前接入狀態 | [執行期對照](references/runtime/current-main.md) |
| 控制器系列／協定／手冊疑點 | [控制器樹](references/controllers/index.md) |
| HT9050 DTM與其他機型的通道差異 | [機型樹](references/machines/index.md) |
| 71通道／廠牌／Index雙向設定 | [設定樹](references/setup/index.md) |
| 溫度畫面名稱／格子／實體位址 | [顯示樹](references/display/index.md) |
| Temp_Set／LotInfo／檔案與補償 | [設定與配方](references/setup/index.md) |
| ATC／FTP補償／其他溫度知識 | [相鄰主題](references/related.md) |
| 客戶差異與查證界線 | [客戶索引](references/customers.md) |

## 查證與交付

追入口→beat／Task→控制器傳收→通道回讀→顯示與告警；把裁決、已接入程式、歷史缺口及實機驗證分開。每批核對最新main並附Change Log；本Skill未授權上機或修改執行期設定。
