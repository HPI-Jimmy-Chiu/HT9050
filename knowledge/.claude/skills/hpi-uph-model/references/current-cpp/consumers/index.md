# UPH consumer 局部樹

本層追計算結果如何被讀取、包裝與傳送；沒有把consumer存在當成writer、資料新鮮度或保存成功。

| 問題 | 路由 | 已查／待查 |
| --- | --- | --- |
| V906表格到TagSnapshot、空白／數值與customer | [WebBridge子樹](webbridge/index.md) | 三body文字、兩sentinel及一caller區段；完整transport／頁面待查 |
| V906／V912指令UPH字串／GetAll | [Command子樹](command/index.md) | 四完整getter／wrapper文字、兩GetAll call區段；transport／dispatcher未閉合 |
| 計數／計算來源 | [計算](../calculate.md)、[計數](../count-time.md) | 既有局部證據；全部caller／writer未閉合 |
| 磁碟保存 | [writer子樹](../writers/index.md) | reader與保存成功分開驗 |

DB／SECS／UI、全caller／thread、實際機型容量與site仍待補。回[目前C++](../index.md)及[同題機型樹](../../machines/index.md)；不啟動程式或runtime。

[Command傳送局部](command/transport/index.md)從取值續追callback與本地包裝；V906 hub與V912 WM_COPYDATA分開，接收端／送達未驗。
