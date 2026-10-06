---
name: ht9045-mydb
description: >
  HT9045 cMyDB 記錄層知識庫（cMyDB.cpp／cMyDB.h）：告警事件（MyDBIEvent）、流程／按鍵／生產／時間統計
  （MyDBIProcess／MyDBIProcessNew／NewRecordProcess／RecordProcess／RecordChangeLogProcess／MyDBITotalLoader／
  MyDBITimeData／MyDBIUPH／MyDBIProductionData／RecordTimeData）、AlarmCode 目錄（MyDBUpdateDB／DoInsertAlarmCode／
  AlarmCodeList.txt／AlarmCodeMap／UnitNameMap／AlarmUnit[32]）、CSV 落點（SaveEventLogInfo 的 HANDLER LOG csv、
  SaveEventTracker、slEventLog／TMyStringList 的 EventLogTxt）。使用者裁決（20260923、20260926）：SQLite／Handler.db3／
  sqlite3 退役，C++ 只做 CSV 版，只移植 CosFunction.bUseMDB==false 的路徑。含 MDB Updater（SQLiteUpdater.exe、
  CreatDatabase.cpp 32 個 Unit 的靜態 AlarmCode）與 EventlogAnalyzer.exe 的關係、AlarmCode 格式與唯一性規則、
  V906 現況（P0～P4＋W7 已落地；P4／W7 是 20260927 St02 本機 commit、只編譯未上機：五個借住替身歸位、2 參數 MyDBIProcess 改成
  轉接器、golden log 物件全建 24 名 58 個、ProductionLog（D2）、SaveMessageHistroy（D4）、ctest MyDB_P4_Containment）與 CSV 版移植計畫。
  Use when：告警沒寫進 log、Unknown Alarm Code、AlarmCodeList.txt 缺碼或重複、新增 JAM/WAR/MES 碼、EventLogTxt／
  HANDLER LOG csv 欄位、SPIL 格式 event log、bUseMDB、cMyDB 移植、MDB Updater 升版、Event Log Analyzer。
  關鍵字：cMyDB, MyDBIEvent, MyDBIProcess, MyDBIProcessNew, NewRecordProcess, RecordProcess, RecordChangeLogProcess,
  MyDBUpdateDB, DoInsertAlarmCode, AlarmCodeList.txt, AlarmCodeMap, UnitNameMap, AlarmUnit, GetMyDBIMessage,
  GetAlarmCodeList, GetJameCodeOfAxis, SaveEventLogInfo, SaveEventTracker, HANDLER LOG, EventTracker, slEventLog,
  TMyStringList, EventLogTxt, SaveEventLog, bUseMDB, Handler.db3, sqlite3 退役, MDB Updater, SQLiteUpdater,
  CreatDatabase.cpp, CreateTableAlarmList, EventlogAnalyzer, EventLogSaver, JAM, WAR, MES, AlarmID 9 碼,
  Unknown Alarm Code, bSPILFunction, ht9045_db, homecoming, AlarmCodeCatalog, CSV 版, P4, W7, LogObjects,
  W906_CreateLogObjects, ProductionLog, MemoProductionLog, SaveMessageHistroy, W906_HeaterLogHook, W906_RMS_ROOT,
  W906_PRODINFO_ROOT, MyDB_P4_Containment, 轉接器。
  CSV 版移植計畫全文 → references/cmydb-csv-port-plan.md
---

# ht9045-mydb 相容入口

同主題已整合到 [hpi-web-hmi](../hpi-web-hmi/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-web-hmi/references/logs/original-entry.md)

## 1. 這個模組是什麼

[讀取此節](../hpi-web-hmi/references/logs/original-entry.md#1-這個模組是什麼)

## 2. 檔案與 CSV 落點

[讀取此節](../hpi-web-hmi/references/logs/original-entry.md#2-檔案與-csv-落點)

## 3. AlarmCode 規則（與 MDB Updater agent 一致）

[讀取此節](../hpi-web-hmi/references/logs/original-entry.md#3-alarmcode-規則與-mdb-updater-agent-一致)

## 4. 相關外掛工具（BCB 線，C++ 線不依賴）

[讀取此節](../hpi-web-hmi/references/logs/original-entry.md#4-相關外掛工具bcb-線c-線不依賴)

## 5. V906 現況（20260926 盤點；**落地狀態見本節末「20260926 晚」**）

[讀取此節](../hpi-web-hmi/references/logs/original-entry.md#5-v906-現況20260926-盤點落地狀態見本節末20260926-晚)

## 6. CSV 版移植計畫（摘要，全文 `references/cmydb-csv-port-plan.md`）

[讀取此節](../hpi-web-hmi/references/logs/original-entry.md#6-csv-版移植計畫摘要全文-referencescmydb-csv-port-planmd)

## 7. 跨技能連動

[讀取此節](../hpi-web-hmi/references/logs/original-entry.md#7-跨技能連動)
