---
name: hpi-autostart-autoclean
description: "HT9050 與其他 Handler 的 AutoStart／AutoClean 共用入口。用於 OLP／GTK Agent 握手、HTSET 333／334、Lot／Setup／Safety Interlock、OneCycle、Auto Clean 觸發與 Clean Kit／Pad／Air、Shuttle／Index task、Cleaning／Status／GPIB／SECS 入口差異及 HT9050 乾跑分流。"
---

# Handler AutoStart／AutoClean

按機型、客戶、版本與入口分層查共通流程，同一主題涵蓋 HT9050 及其他機台。

## 先確認

- 先讀 [共同項與差異](references/common.md)，區分啟動指令、清潔觸發、Task初始化與真正運動。
- HTSET 333、OLP START_REQUEST與Web START是不同入口；reply／OK不等於機台已啟動，讀 [目前啟動路徑](references/runtime/start.md)。
- Cleaning頁與Status頁的Auto Clean運轉中規則不同；OneCycle已在跑則目前本體直接返回，讀 [目前清潔入口](references/runtime/cleaning.md)。
- HT9050乾跑會接管tick；SIM與非SIM又不同，先讀 [機型／乾跑](references/machines/index.md)，不能把0210舊review當現在main缺口。
- [客戶分流](references/customers.md)與Recipe／機構參數分開核對，不因功能旗標或一個畫面可點就推論全部呼叫者接通。
- 活文件用版本／檔名＋function／特定變數／Task；原文行號僅留歷史，不把靜態查證稱為實機驗證。

## 按問題選路

| 問題 | Reference |
|---|---|
| OLP、GTK／Agent、Lot與Setup五階段 | [AutoStart原文與協定](references/autostart/index.md)／[目前main](references/runtime/start.md) |
| 清潔觸發、OneCycle、模式與計數 | [AutoClean](references/autoclean/index.md)／[目前入口](references/runtime/cleaning.md) |
| Pick／Place、Shuttle／Index Task | [狀態機](references/autoclean/tasks/index.md) |
| HT9050與其他機型、0210乾跑差異 | [機型與版本](references/machines/index.md)／[歷史review](references/machines/mach0210.md) |
| 客戶、權限、Yield／Alarm與Clean Pad | [客戶](references/customers.md)／[Alarm](../hpi-alarm/SKILL.md) |
| 舊完整主體與metadata | [AutoStart](references/autostart/original-entry.md)／[AutoClean](references/autoclean/original-entry.md) |

20261006推前更新：[HT9050／其他機型與caller](../hpi-customer-features/references/main-integration-20261006-2022.md)，舊原文與查證日期保持。
