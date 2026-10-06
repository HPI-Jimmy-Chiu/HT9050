---
name: hpi-web-hmi
description: >
  Handler Web HMI共用入口，整合HT9050與其他機型的HTML畫面、JSON／C++橋接、
  視窗狀態、登入權限、CSV記錄與原生六頁。查Web接線、A／B／C路、editlist、
  fShow、登入／reauth、cMyDB或產生器時使用，按版本與機型逐層讀references。
---

# Handler Web HMI

先讀 [共同層與名詞](references/common.md)與[目前main查證](references/runtime/index.md)，再選分支；[完整閱讀地圖](references/read-map.md)提供逐題入口。

| 主題 | 參考樹 |
|---|---|
| JSON資料來源、A／B／C路與API契約 | [JSON](references/json/index.md) |
| DFM轉HTML、畫面／元件／主題／輸入 | [畫面](references/pages/index.md) |
| golden表單橋、HTEditList、產生器與保存 | [C++橋接](references/bridge/index.md) |
| fShow、頁面表、WebSocket hub與停止分工 | [視窗狀態](references/windows/index.md) |
| 密碼本、登入、reauth、告警框權限 | [登入](references/auth/index.md) |
| cMyDB CSV、AlarmCode與外掛歷史 | [紀錄](references/logs/index.md) |
| 原生六頁、方案與裁決沿革 | [原生表單](references/native/index.md) |
| HT9050／其他機台、客戶與版本分流 | [機台差異](references/machines/index.md) |

入口只導讀；七個舊Skill的原文、裁決、metadata與舊引用入口保留，[資源／相容路徑](references/resources/index.md)另列。舊程式行號與量測屬原日期；活查證用版本／檔名加function、變數、Task／case。

修改程式或接新頁前讀對應agent、最新CLAUDE／裁決及write-boundary-policy；確認golden來源版本、owner、互鎖、客戶條件與呼叫端。頁面／tag／產生檔存在不等於動作接線、可存檔或實機驗證完成；保留原生方案的提案與實作狀態差別。

本次整理僅Skill／文件，不執行產生器、HTTP／WS寫入、啟機或runtime測試。後續被授權測試時才依機台快照同步與備份還原流程；禁止由舊命令範例推定本次硬體授權。

20261006推前更新：[HT9050／其他機型與caller](../hpi-customer-features/references/main-integration-20261006-2022.md)，舊原文與查證日期保持。
