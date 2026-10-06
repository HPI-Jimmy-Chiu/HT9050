# Bin Display共同項與差異

main `372b91908`，20261006只讀核對。外接面板與網頁是不同資料／通訊階段。

| 項目 | 共同項 | 分流／界線 |
|---|---|---|
| Bin資料 | 盤要收哪些Bin、目標與顯示狀態分層 | 顯示器不能證明IC已成功放到盤 |
| type 0／1／2 | 無面板／DIO脈衝路線 | 不能去診斷序列COM或new顯示控制器 |
| type 3 | 三色七段legacy／Modbus-ASCII | NUMBER_PANEL2與部分Fix路線依AUTO_EMPTY_COLOR，不是所有顯示器必用兩埠 |
| type 4 | TFT二進位封包、字型／數量／輪播 | ack、iColorNow／iBinNow鏡像與實體面板讀回不同 |
| Magazine | MAGAZINE_BIN_DISP_TYPE獨立決定 | HT-A18／BT008／TFT不同封包；不能只看NUMBER_PANEL_TYPE |
| V906／912 | C14原基準906、UB保護按原規則 | 1003第20條後允許D／E／A的912 TFT修正，現在main已含S22；不是全部912都搬 |
| HT9050 | 一份Skill內放機型配置差異 | 1003裁決TFT／COM14；目前Git快照仍type3／COM11，與實際設備分開查 |
| 網頁 | binsel.disp由C++資料發布 | null／灰X／ctrl=false分別判讀，無tag不憑空填0 |

[目前main](runtime/current-main.md)／[原資料](source/original-entry.md)／[機型](machines/index.md)。
