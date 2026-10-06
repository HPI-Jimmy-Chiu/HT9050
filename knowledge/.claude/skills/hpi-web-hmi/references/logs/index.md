# cMyDB與CSV紀錄

- [完整原入口](original-entry.md)：MyDBIEvent／MyDBIProcess／ProductionLog、AlarmCode目錄、CSV落點與原移植狀態。
- [CSV移植計畫](references/cmydb-csv-port-plan.md)：P0～P6、裁決、借住替身與containment沿革。
- [MDB Updater原工具規範](references/mdb-updater.md)：BCB工具、Unit、AlarmCode格式與衝突案例，保留原字句。
- [Alarm](../../../hpi-alarm/SKILL.md)、[JSON通道](../bridge/index.md)、[EventLogAnalyzer原Skill](../../../ht9045-eventlog-analyzer/SKILL.md)。

名字中的DB與外掛SQLiteUpdater不代表V906現在依賴SQLite；原20260923／26裁決只移植bUseMDB=false CSV路。舊「未建slEventLog／沒寫CSV／P4本機未推」有明確日期，現在先查LogObjects.cpp、cMyDB.cpp與wb_serve實際caller，再查CMYDB_PORT_LEDGER.md；本批未重跑所有logging路徑或統計呼叫數。

讀取、開機建立與MyDBUpdateDB本身可能寫檔，不能拿啟動／呼叫當只讀文件檢查。AlarmCode唯一性與32 Unit的原規則保持，若後續改碼走原權威資料與對應版本生成程序，本批不重產目錄／CSV、不啟動外掛。
