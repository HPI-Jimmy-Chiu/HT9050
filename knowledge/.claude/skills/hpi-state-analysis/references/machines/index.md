# HT9050與其他Handler的分流

| 範圍 | 共用方法／差異 |
|---|---|
| HT9050／V906移植樹 | 分清記錄包的版本／Model／CUSTOMER_CODE、PCI1203軸與present／enable／讀取狀態，先按實際CSVheader讀；V906的W906_*附加診斷與原格式分開 |
| HT9045／其他BCB6機台 | 原記錄格式與案例依其BCB6版本；V912修正目標與V899客戶實際碼分開，不借用9050軸名／站點數或V906診斷欄 |
| 其他V906機型 | 共同source不代表每台都有相同裝配、caller／gate或記錄欄；W906_*是移植診斷命名，不單憑檔名認定HT9050 |

MainProc與Task／sensor／motor觀測方法共用；站點、軸、等待條件與CSV欄位依機型／版本確認。缺欄、沒有fresh sample或not-present不是設備已故障，現場身分與快照時間另查。

目前V906來源／附加診斷導讀見[source核對](../runtime/current-source.md)，歷史資料格式見[格式樹](../formats/index.md)。若要測試或重現才依專案機台同步／備份／還原流程，本次不執行。
