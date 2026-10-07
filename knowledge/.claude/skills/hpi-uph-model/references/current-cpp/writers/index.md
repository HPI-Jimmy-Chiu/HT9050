# UPH CSV writer 的局部證據

來源 pin `06fb64e54`，完整版本與八個 source blob、十一個 body、六個宣告見 [manifest](source-manifest.json)。這是指定函式的靜態查證，沒有寫檔或執行機台。

| 問題 | Reference | 範圍 |
| --- | --- | --- |
| 預設模式、overload、換行與結果 | [共用寫檔](common-io.md) | V906／V912 WriteDataToFile 與 common.h |
| KYEC、FOREHOPE、VTEST 格式與版本差異 | [格式與分流](customer-formats.md) | 五個指定 writer body；CalculateUPH caller 另保留 |
| 每日檔名、目錄建立與路徑 | [路徑 helper](paths.md) | V912 FileInfo 兩個 body／宣告；V906 FOREHOPE 是空本體 |
| 還沒閉合的 caller、輸出與機型 | [查證界線](limits.md) | 全 caller／consumer、現場旗標、容量／site／校正仍待查 |

共同與差異都在 [UPH 主題](../../../SKILL.md)：本層 V906／V912 Handler 原碼不自動代表 HT9050 的作用中分派；HT9050 Hot／Ambient 模型與 runtime 分流仍讀 [機型樹](../../machines/index.md)。
