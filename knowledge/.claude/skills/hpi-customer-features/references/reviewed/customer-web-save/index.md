# 客戶碼存檔：Web入口與HSys守衛

這次補S8的局部靜態caller證據，來源釘住main `de7c2bf77`。同一份Skill包含HT9050與其他Handler；V906共用Web存檔路徑與V912表單路徑分開核對，不宣稱機型或客戶功能完全相同。

| 問題 | Reference |
|---|---|
| 控制權、主迴圈分派、PageSave與回覆意義 | [接收與存檔層](transport.md) |
| HSys開窗客戶條件、V912對照與CUSTOMER_CODE存檔caller | [HSys守衛](hsys.md) |
| 本次10份Git blob及查證界線 | [來源manifest](source-manifest.json) |
| 先前存檔函式與boot reader | [存檔caller](../customer-persistence.md)／[開機reader](../customer-boot/index.md) |

新增一列HSys開窗條件；原ART／名稱8列與boot3列原來源／日期不改，合計12列仍是局部查證。原6382個候選ID、846個未決、原scanner及來源manifest保持。未執行Web／HTTP／WS命令、存檔、登入、建置或機台；磁碟效果、全部reauth／proxy／callback、副作用與運行中最後consumer仍待查。

20261007續補[reauth頁面分流／清除與ack局部查證](../customer-reauth/index.md)：HSys不在PointOf對照表，沒帶答案仍需原存檔閘。原12列／候選／舊來源保持；完整登入、owner時序、所有結果寫者與磁碟效果仍未完成。

20261007續補[owner／queue局部時序](../customer-owner/index.md)：核對接收owner、控制權變更、connId／ticket、queue／共用guard與所選存檔分支。完整HTTP／carry／callee／owner競態仍未完成；不由接收通過或ack推定持續控制權與磁碟成功。
