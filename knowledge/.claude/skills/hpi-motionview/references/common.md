# MotionView共同項與機型差異

基準main `84233b648`，20261006。原模板與20260908／09 JSON部署筆記完整保留，但正式資料通道依Steven 20260915／18及目前web頁面查證。

| 項目 | 共用要求 | HT9045／9046原模板 | HT9050原layout／今日界線 |
|---|---|---|---|
| 正式機台資料 | 頁面讀目前HTTP/API來源；未知真值不由動畫猜 | teach、Gerneral、recipe目前以wb_serve /api即時讀 | HT9045Live與liveSettings9050讀即時model／recipe；layout能力集仍可用靜態JSON |
| SIM／LIVE | 概念／離線模型與真實投影分開 | 多吸嘴排程、變距、StateRecord原示例保存 | 概念頁可自己跑；正式整機runtime沒producer，不拿舊快照頂替；軸表API另查 |
| 機構／軸 | 按實際機構映射，不靠相似名字內插 | 原模板有雙Index、雙Kit／多吸嘴、Auto／Fix群組 | 單吸嘴、Z1 Index、獨立Out Shuttle；Out Y氣缸二值，與伺服Y不同 |
| 幾何 | 來源、單位、pulse／mm與有效性分開 | 原teach推導為模板算法，不能當已校正實機值 | layout PDF估值／stations／strokes不是實機teach；SLK／AOI等選配另分 |
| 料件顯示 | 格點、吸嘴持料、Task與庫存不混算；concept需守恆 | HotPlate／Tray格位、Site與Pitch依原算法 | In P&P補料、singleKIT、OutKit與Auto1～3依9050概念模型 |
| 生成來源 | 先看目前source與生成器能否保留接線 | 原模板／本機範例只是來源資源 | 今日Main頁的B路與軸讀數不在舊build完整覆寫保證內，不能盲重生 |

[原9045模板](template/original-entry.md)／[原9050 layout](layout/original-entry.md)／[目前接線](runtime/current-main.md)。
