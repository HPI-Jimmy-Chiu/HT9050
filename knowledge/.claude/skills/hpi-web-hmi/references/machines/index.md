# HT9050與其他Handler共同項／差異

同題共用技術契約與讀寫／視窗／權限規則，各機台按以下條件分流。

| 條件 | 分流方式 |
|---|---|
| 機型與身分 | Machine-profile／Machine-type的Web metadata、C++ Type與snapshot的Model各有caller，不由runtimeSupported或頁面可見性推定硬體流程已實作 |
| HT9050 MotionView | 手寫Main.MotionView9050與layout、播放／編輯歷史在原畫面樹；目前共同資料與差異見 [MotionView](../../../hpi-motionview/SKILL.md)，不覆寫成DFM通用頁 |
| IO／Motor／溫控 | 相同Web頁／tag名可能有不同軸、卡、Alias、通道及單位；查 [IO](../../../hpi-io-control/SKILL.md)、[Motor](../../../hpi-motor-control/SKILL.md)、[溫控](../../../hpi-temperature/SKILL.md) |
| 配方／Config／Lot | owner與表單機制共用，欄位固定／可編輯／客戶功能、路徑與Lot流程分開；轉讀 [Config](../../../hpi-config/SKILL.md)與[LotInfo／Recipe](../../../hpi-lotinfo-recipe/SKILL.md) |
| 登入與紀錄 | CUSTOMER_CODE與CosFunction等客戶條件另查，不由HT9050推定791／957／登入模式／CSV欄位；[登入](../auth/index.md)、[紀錄](../logs/index.md)保留差異 |
| 版本 | BCB V912、906 0618 golden與V906 Cpp區隔，V899與其他舊樹唯讀；原V910與舊Code根的generator範例按歷史解讀 |

[HT9050快照來源](../../../../../machines/HT9050/snapshot/SNAPSHOT_SOURCE.md)與目前AGENTS／CLAUDE是機台事實與同步流程依據。machine→GitHub→GitLab為單向鏡像；本次不改snapshot、runtime或搬檔。只有後續被授權測試才跑同步／備份／還原。
