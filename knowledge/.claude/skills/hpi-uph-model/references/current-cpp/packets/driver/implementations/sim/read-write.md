# Sim 讀寫與計數

以 `SimGpibDriver::ibrd`／`ibwrt`、in_／out_／talk_／cnt_／err_定位，完整 body 在 [manifest](../source-manifest.json)。

| 路徑 | 選讀來源行為 |
| --- | --- |
| ibrd 開頭 | cnt_=0 |
| in_非空且cnt>0 | 取front字串並pop整筆；複製min(字串size轉long,cnt)個byte到buf，cnt_=n |
| ibrd 結尾 | Recompute後加END再返回sta_，即使沒有讀到資料也走此結尾；body沒清err_ |
| ibwrt，talk_為false | err_=6、sta_=ERR\|TIMO、cnt_=0，返回sta_；沒有追加out_ |
| ibwrt，talk_為true | 用buf及cnt轉size_t建立字串，追加out_；cnt_=cnt、err_=0，Recompute後返回sta_ |

ibrd 的pop發生於複製前，當一筆字串比cnt長，這個body沒有把尾段留回in_。ibwrt 的成功分支只是將資料保存於本地out_，不能當作外部Tester收件／ACK。

這兩個body沒有建立buf有效性契約；ibwrt也沒有先檢查cnt非負。有效輸入、上游容量／封包ABI與呼叫序列仍待查。回 [Sim 索引](index.md)、[狀態](state.md)、[界線](../limits.md)。
