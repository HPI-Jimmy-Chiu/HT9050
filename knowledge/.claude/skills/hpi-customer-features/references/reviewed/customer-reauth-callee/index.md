# 重新驗證 callee：答案消耗與結果分流

本層接續[解析／清除／Ack](../customer-reauth/index.md)，只補 V906 八個指定函式 body；來源、blob、guard 及 overload 見[source manifest](source-manifest.json)。同題共用路由供 HT9050 與其他 Handler 查閱，但客戶碼、機型、runtime 權限與啟用值不從這些共用函式推定。

| 路由 | 已讀內容 | 尚未閉合 |
| --- | --- | --- |
| [答案消耗與 wrapper](answers.md) | TakeAnswer、HasAnswer／HasAnswerFor、Setup／M01／I37 wrapper、Keep(Result) | 全部 caller、stash／結果寫者、還原 callback 與點位所有權 |
| [權限判斷與結果](authentication.md) | W906_Reauth 的指定 body、early return、REAL_TIME_CCD 與 config 分支 | BookCompare／stOperatorClick／DoPasswordMBox／StateJson 等 nested callee、V912 golden 對照、所有執行路徑 |

此層不是 HSys 客戶碼儲存必經的登入證明；HSys point 邊界仍依[既有解析](../customer-reauth/parse.md)。未送登入／存檔請求、讀取現場憑證、啟動程式或改 runtime；原候選 ID 與人工客戶列不增加。

- [接續I37 MBox／PTI、密碼本與StateJson（局部）](../customer-login/index.md)
