---
name: hpi-alarm
description: "HT9050 與其他 Handler 的告警共同入口：Yield／Low Yield／AutoSiteOff、Note／Message／NonStop、kCode／ReturnCode、實體面板鍵與畫面解除、密碼／SpecialPanel、V906 hook／通知 ack、Dialog 信箱／FIFO／過期 tag、告警位置／Motion View／Code 說明及產生器輸入表。"
---

# Handler Alarm

同一份 Skill 管理各機型的解除、停機、權限與位置顯示，按機型及原生／V906／Web 路徑分流。

## 先確認

- 讀 [共同與差異](references/common.md)，先區分 Note、Message、NonStop，以及 kCode、position、Code。
- 原生 Note 的兩段式與 Message 的一段式保留；[目前 V906](references/runtime/current-main.md) 的畫面 START 客戶限制、hook 與通知 ack 需另外核對。
- kCode==0 不等於不停機；畫面關閉也不等於馬達停止或重啟，先追 C++ 呼叫與回傳。
- HT9050 與其他機型的 Motion View／門環／unit 映射分開；Code 文本與 position 紅框分別核對。
- 兩份原對照表是產生器輸入，保持原路徑及內容；[地圖與 caller](references/maps/index.md) 說明何時讀它們。
- 原文與行號保存為歷史；活文件使用版本／檔名加 function／特定變數，靜態查證不稱實機驗證。

## 按問題選路

| 問題 | Reference |
|---|---|
| 實體鍵／畫面解除、密碼與停機 | [解除與權限](references/dismissal/index.md)／[目前main](references/runtime/current-main.md) |
| Yield／Low Yield、關 site、清計數 | [良率告警](references/yield/index.md) |
| 通知、FIFO、框關不掉／過期 tag | [信箱與佇列](references/dialog/index.md) |
| 位置、Code／UnitName、紅框與說明 | [顯示與資料來源](references/display/index.md)／[輸入表](references/maps/index.md) |
| HT9050 與其他機台配置差異 | [機型](references/machines/index.md)／[客戶條件](references/customers.md) |
| 舊原文／版本對照 | [保存原文](references/source/original-entry.md)／[告警原入口](../ht9045-alarm-dismissal/SKILL.md)／[Yield原入口](../ht9045-yield-flow/SKILL.md) |
