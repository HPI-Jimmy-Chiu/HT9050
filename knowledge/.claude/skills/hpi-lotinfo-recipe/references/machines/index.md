# HT9050與其他機台工單差異

Lot／Recipe／QA共同資料模型放同一Skill，機台值不固定寫成全體預設：

| 欄位／層 | 跨機型核對方式 |
|---|---|
| HandlerCondition的Shuttle Mode | 先對目前Read函式、TestIF.iShuttleMode／iShuttle_Sel及機構；HT9050單Shuttle與其他配置的限制看 [Shuttle](../../../hpi-shuttle-flow/SKILL.md) |
| Shuttle1／2 Cancel | 原Recipe鍵指使用／不使用設定，不是Lot取消按鈕；它與Shuttle Mode／選取／實際有無軸是不同問題 |
| TestMode的Temperature Mode | 使用ReadTestMode、TestMode.iTemperatureMode與當前enum核對；與Temperature.Data的Heater／ATC工作值、UI顯示與customer條件分開，見 [溫控](../../../hpi-temperature/SKILL.md) |
| QA的Count／SiteMap／Bin | 不由HT9050或9045名稱推counter步幅、閾值或Binindex；[目前QA](../runtime/qa.md)與原variant一併看 |
| 作用中Recipe與機台快照 | 先對snapshot來源、拍照時間與工單版本；開發機預設與現場不同時，不能先下機台錯誤結論 |

HT9050 snapshot是機台→GitHub→GitLab的單向鏡像，依 [機台來源規則](../../../../../machines/HT9050/snapshot/SNAPSHOT_SOURCE.md)與AGENTS同步／還原流程；本次不apply、不copy到system、不改鏡像。其他真機的設定修改另依使用者授權。

原HT9050硬體表SH-5的模擬Shuttle值及A66 open是歷史日期，回答現況需以現在source／snapshot確認。原Data欄位參照讀 [Recipe樹](../recipe/index.md)，不把JSON快照當即時記憶體。
