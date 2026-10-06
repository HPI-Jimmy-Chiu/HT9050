# 溫度共同項與機型差異

基準main `84233b648`，20261006；原文的golden V912、V906移植樹、V910 HT9050及各日期分開保留。

## 共同查證方法

設定→使用通道→迴圈／控制器→PV／SV→畫面與告警是一條查證鏈，不以頁面有下拉就推定底層輪詢已接好。
71個`eTempControll`是軟體通道集合；控制器序號、站號、CH、機構與機型顯示另外對照。
`999`、`---`、null、No Heater、-9999各有不同來源，先查版本及裝配／回讀狀態。

| 項目 | 共同項 | HT9045／其他配置 | HT9050差異／界線 |
|---|---|---|---|
| 溫控節拍 | 查實際caller與SIM／SHIP編譯路徑 | V912 VCL執行緒與V906 fast clock架構不同 | 不由機型名稱推定節拍；目前V906接入見執行期對照 |
| 廠牌選擇 | 全機廠牌、逐通道選項、Index控制器三層分開 | golden及V906頁面／底層不可混稱；Q34／Q71～76原文保存 | 不把頁面5／6代碼寫入HEATER_CTRL_TYPE或當成BCB可用代碼 |
| Index通道 | enum相同不代表接線相同 | EJ1N／DTME08的原Index映射及4／16／32組互斥 | DTM原硬體表為3站24通道，SLK映射含推導；不能套9045 Index原順序 |
| 顯示 | 通道→名稱→格子→實體位址分層 | 4／16／32組及CCD／DUT等選配影響版面 | HT9050歷史畫面沿用9045，不證明3站配線已接對 |
| 控制器協定 | 按系列查格式、單位、站／CH與回覆 | KT4H與E5DC不同協定；EJ1N與E5DC同品牌仍分系列 | DTM Ethernet群組、主機IP及感測器限制與串列匯流排不同 |

目前main的fast clock、COM2接線及SIM／SHIP差異見 [執行期](runtime/current-main.md)。
完整歷史原文：[溫度總地圖](core/original-entry.md)／[廠牌與裁決](controllers/configuration/original-entry.md)。
