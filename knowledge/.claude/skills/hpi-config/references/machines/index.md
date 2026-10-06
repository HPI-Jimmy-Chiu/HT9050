# HT9050與其他Handler的設定差異

同題共用Config／General／Customer知識，以下條件需逐台核對：

| 層 | 共同點與差異 |
|---|---|
| 機型／硬體 | Model、CUSTOMER_CODE、IO／Motor配置分開；相同INI／CSV檔名不代表相同值或軸／卡 |
| IO／Mot表 | HT9050的早期Mot_Table_9050是原歷史資料，現在作用中表及快照另查；不可當新機通用種子，見 [IO](../../../hpi-io-control/SKILL.md)、[Motor](../../../hpi-motor-control/SKILL.md) |
| Config與Recipe | 依客戶決定elConfig或elConfig_byRecipe、讀檔或固定值；P04供收盤、O12計數等機構／客戶耦合不能全機套用 |
| 957與Factory | 957是目前CC_PTI數值，HT9050的實際客戶值需snapshot／裁決；名字查表與UI列表可來自不同版本 |
| 教點／Alias | 機構與版本的section／Alias不同，MInShutte／MInShuttle原案例按版本定位，不寫runtime遷移 |

先看 [HT9050 snapshot來源](../../../../../machines/HT9050/snapshot/SNAPSHOT_SOURCE.md)及AGENTS；機台→GitHub→GitLab單向鏡像不可在本批反向改。機台專屬可分享檔放machines，system／config是runtime。若有後續被授權測試，再依既有machine_sync程序更新／同步／還原；本次沒執行測試或apply。
