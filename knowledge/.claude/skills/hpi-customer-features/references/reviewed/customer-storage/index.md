# 客戶碼存檔：proxy與INI寫入的局部查證

S8續查，來源釘住main `de7c2bf77`，以版本／檔名＋function與變數定位。本樹補V906的proxy、存檔標記與INI包裝；V912只核對共用包裝的呼叫介面，不將V906相容層行為套給BCB。HT9050與其他機台仍在同一份Skill，機型、客戶與實際runtime須另核對。

| 問題 | Reference |
|---|---|
| 欄位可改性、文字型別與真正的父層 | [Proxy判斷](proxy.md) |
| 存檔標記、CUSTOMER_CODE賦值與後續reader | [存檔與再讀](save-read.md) |
| V906 TIniFile寫入路徑、V912查證界線 | [INI包裝](ini.md) |
| 7份Git blob／查證範圍 | [來源manifest](source-manifest.json) |
| Web接收／PageSave／HSys前段 | [先前Web樹](../customer-web-save/index.md) |

這次未新增客戶差異列，原12列人工查證、6382個候選ID、846個未決與舊來源／日期保持。僅靜態讀碼，未執行Web命令、登入、存檔、建置或機台；沒有宣稱磁碟寫入成功、所有proxy寫者已核對或完整存檔交易已驗證。
