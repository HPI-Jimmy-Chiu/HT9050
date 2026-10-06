---
name: hpi-ep-pressure
description: "Handler EP類比壓力／ADAM6024共用裝置與機型差異，整合HT9050與其他機台的ADAMTCP.dll、AI／AO、EP_Install、APAX／ET7226、kPa／kg／V／輸出碼、TransformFuntion、ADAM_WriteVoltage／ReadPA、ReturnValueCheck、WAR1605／16322／16323及live gate知識。分析EP未寫出、壓力回授、DLL位元數、Index D24／D26或Double EP時使用；流程與Contact Force依相鄰reference導向。"
---

# Handler EP Pressure

推送基準與後續裁決：[最新main整合](references/main-integration-20261006.md)。

HT9050與其他機型共用一個EP壓力Skill，依裝置／DLL、換算、安裝型態、接入狀態與流程選reference。

## 先確認

- 先讀 [共同與差異](references/common.md)；ADAM6024為EP類比I/O，不能當成溫控器。
- 分清力量kg、壓力kPa、回授V與輸出碼；`TransformFuntion`只換算，不代表寫到硬體。
- `EP_Install`與`INSTALL_DOUBLE_EP`分開查；AO0／AO1、AI2／AI5與APAX路徑不可互換。
- [目前接入狀態](references/runtime/current-main.md) 區分預設OFF、測試override、已編譯helper與Index未接入流程。
- 不因helper存在就開live gate、改IP／校正或載DLL；此整理不接手其他session的實機派工。

## 按問題選路

| 問題 | Reference |
|---|---|
| 安裝型態與HT9050／其他機台差異 | [共同與差異](references/common.md) |
| ADAM6024／AI／AO／ADAMTCP DLL | [裝置與協定](references/device/index.md) |
| kg／kPa／V／輸出碼與golden函式 | [換算與程式地圖](references/pressure/index.md) |
| 今日gate、接入與歷史缺口 | [執行期對照](references/runtime/current-main.md) |
| 連線、壓力、Dual EP與警報排錯 | [排錯原文](references/adam/references/troubleshooting.md) |
| Index D24／D26／Contact Force | [相鄰流程](references/related.md) |
| 客戶差異 | [客戶索引](references/customers.md) |

## 查證與交付

依原版本追輸出／回授／校正／告警，分開程式證據與機台實測。保存Steven／Jimmy裁決及歷史原文，每批核對main、附Change Log；不修改執行期設定或操作機台。
