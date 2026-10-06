# EP壓力共同項與機型差異

基準main `84233b648`，20261006。共同的是查證鏈與換算介面；實體I/O與安裝型態依配置分派，不把所有EP都稱ADAM。

| 項目 | 共同項／HT9045等配置 | HT9050差異與界線 |
|---|---|---|
| 量測鏈 | 力量kg→TransformFuntion→碼→寫出；AI回授V→kPa→kg；數值與成功讀／寫分開 | 原機型裁決為EP_Install=3，只有Z1下壓；模擬參數不是機台正本 |
| EP安裝 | 0未裝；1Analog；2Digital DO；3Digital and Encoding；4ET7226；5Two EP | 不由機型名稱取代EP_Install與INSTALL_DOUBLE_EP實際值 |
| 主／Dual EP | golden主AO1 reg12／AI5；Die Force AO0 reg11／AI2，Two EP另分派 | 回授通道與現在配置核對；不要把第二臂／Dual Force視為同一機構 |
| APAX | INSTALL_DOUBLE_EP=2／3依原文分獨立／Multi EP；不同於單6024雙AO | 不能由HT9050模擬組的INSTALL_DOUBLE_EP=1推所有9050皆相同 |
| DLL | 32位元ADAMTCP、stdcall、runtime loader與錯誤路徑查原文 | 預設live OFF不代表已載DLL或量到真壓力；test override也不是上機證據 |
| Index告警 | D24／D26、回授電壓檢查與Index壓力記錄是不同路徑 | helper存在不能補足仍為return false的CheckAndRecodrEP；見當前對照 |

[原裝置／程式全稿](adam/original-entry.md)／[目前main](runtime/current-main.md)／[Index與力量流程](related.md)。
