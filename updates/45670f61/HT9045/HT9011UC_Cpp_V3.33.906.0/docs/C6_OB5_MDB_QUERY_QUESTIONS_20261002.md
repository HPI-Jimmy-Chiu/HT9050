# ST02-C6（E-019 OB-5）Observer 的「MDB」查詢子分頁：不是翻譯題，是設計題 —— 只列待決，不實作

- 讀者：Steven、St02-M、ST01-M、筆電（Jimmy）。第一節看完就知道結論；第二節是證據（檔:行），第三節是要決定的事。
- 盤點人：St02-E 的 helper（AI(W906-ST02-OB7) 20261002）。只讀、沒有改任何程式、沒有建置、沒有執行。
- 對照版本：移植樹與網頁＝origin/main 0cf8598a（在 D:\AI_TempFile\st02-s20 讀；下文路徑以 D:\HT9045\ 為根）。
  golden＝906 樹 D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven（cp950），下文「golden」一律指這棵樹（RULINGS_20261002 第 20 條：只照 906 翻）。
- 起因：E-019 盤點表 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\E019_DATA_STATUS_EVENTS_20261001.md 第 63 行、第 120 行把 OB-5 交給 Steven02，寫「cMyDB 已改 CSV，這一頁要不要做、怎麼做由 St02 定」；
  St02 現況板 D:\HT9045\.claude\skills\ht9045-st02-workflow\references\current-state.md 第 58 行寫「OB-5 是新設計（MDB 改 CSV 查詢），先不做」。

---

## 一、結論（先看這一節就好）

1. **照 golden、照既有裁決，OB-5 現在沒有東西要做。** 這個子分頁在 golden 只在 CosFunction.bUseMDB 為 true 時才看得到；
   移植樹依使用者 20260923／20260926 的裁決把 bUseMDB 鎖成 false（不用 SQLite、只做 CSV），所以 golden 自己就把這一頁藏起來。
   網頁 Data.Observer.html 的 System Message 本來就沒有這一頁（只有 Text／Time Data／SG_JamCount 三個頁籤），**跟 golden 一致**。
2. golden 這一頁的四支處理器（Query、Save、日期檢查、Total Loader 右鍵）**只有這一頁的元件會呼叫**；
   Query 另外被 bAutoSaveEventLog 呼叫四次，但 bAutoSaveEventLog 在 906 全樹**沒有活的呼叫者**（唯一一處在 btAutoSaveClick 裡被註解掉）。
   ⇒ bUseMDB＝false 時，操作員碰不到這四支，機台也不會自己跑到。
3. 這一頁裡唯一還「活著」的功能 —— Production Summary Report（生產摘要報表）—— 另有一條不經過這一頁的路：
   cMyDB.cpp 的 O19 每日／每週／每月自動報表會直接呼叫 DoProduction_Summary_Report，St02 已在 ST02-C4 翻好（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\O19SummaryReport.cpp）。
4. 「把這一頁改成查 CSV」是**新功能**（golden 沒有這種寫法），要 Steven 決定要不要做；決定要做之前，St02 不動。

---

## 二、證據

### 2.1 golden 的畫面與處理器

| 項目 | golden 出處 | 內容 |
|---|---|---|
| 子分頁 | D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cObserver.dfm:1337-1338 | tsMDB（Caption 'MDB'），在 System Message（tsMDBQuery :1323）的 pgcMessage 裡 |
| 可見 | ...\cObserver.cpp:379 | `tsMDB->TabVisible=(CosFunction.bUseMDB);` |
| 顯示什麼 | ...\cObserver.dfm:1405-1435 | cbDisplayData 19 項：Event Log、Process／Message／Motion／Production Record、Alarm History（全部／JAM／MES／WAR）、Alarm Statistics（兩種）、Jam Chart（Summary、In Arm、Out Arm、Index、In Shuttle、Out Shuttle）、Alarm Code List、Production Summary Report |
| Query 鈕 | dfm :1494-1502 → cObserver.cpp:2421-2775 BtnQueryClick | 依 cbDisplayData 組 SQL，交給 cMyDB 的 MyDBVProcess（:1338）、MyDBVProcessFilter（:1487）、MyDBVEventFreq（:1254）、MyDBVUnitEventCount（:1190）、MyDBVAxleEventCount（:1209）填 strngrdMDBQuery 或 Chart2；Alarm Code List 另外用 fSecurity->GetJamLevel 填「Level」欄（:2644-2685，這是 Jam 等級欄的顏色，不是權限檢查）；Production Summary Report 呼叫 DoProduction_Summary_Report（:2686-2688） |
| Save 鈕 | dfm :1503-1511 → cObserver.cpp:2393-2419 BtnSaveClick | SystemStart 時不做；SaveDialog 存 D:\EventLog.xls（表格）、D:\EventLog.csv（摘要報表）或 D:\EventLog.BMP（Jam 圖表） |
| 日期 | dfm :1414（cbDisplayData OnChange）、:1449、:1478（OnCloseUp）→ cObserver.cpp:2777-2784 DateTimePicker1CloseUp | 起始日期不得晚於結束日期；原本會順便 Query，golden 已註解掉（:2781「Mark MDB 避免卡卡」） |
| Total Loader 右鍵 | dfm :1388-1403 → cObserver.cpp:3177-3184 lbltTotalLoaderMouseDown | 右鍵切換 bFilterTheAgainData（Event Log 查詢要不要濾掉 Duplicate） |

### 2.2 bUseMDB 在 golden 與移植樹

- golden 預設 false：D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\CosFunction.cpp:4295（InitialCosFunction）。
  只有六個客戶碼函式設 true：FUNC_CC_PTI :1486、FUNC_CC_TERAPOWER :1809、FUNC_CC_TeraProbe :1857、SPILFunction :3098、MaximFunction :3167、FUNC_CC_AnalogDevice_Phil :3526（都是客戶專屬 ⇒ S25 取 golden 的 else＝false）。
- golden bUseMDB＝false 時 cMyDB 不開資料庫：MyDBOpenDB :116-130（:119 守衛）；MyDBV* 查詢函式本身沒有守衛（:1345 直接 sqlite3_get_table），golden 是靠「這一頁藏起來」讓它們跑不到。
- 移植樹強制 false：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CosFunction.cpp:4060-4092（AI(W906-SJSON-NOMDB) 20260923、AI(W906-CSVONLY) 20260926；使用者原話「目前機台就是 CosFunction.bUseMDB=false, 沒有其他選項」「只需移植 CosFunction.bUseMDB==false 的部分」）；
  SQLite 路徑由 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cMyDB.cpp:113-119 的 `W906_CMYDB_SQLITE`（預設 0）關掉。
- 移植樹照 golden 藏頁：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp:3832（同 golden :379）；observer.get 回 tsMDB 可見＝false（同檔 :8310）。

### 2.3 誰呼叫 BtnQueryClick（golden 全樹）

| 呼叫處 | golden 出處 | 活的嗎 |
|---|---|---|
| tsMDB 的 Query 鈕 | cObserver.dfm:1501 | 只在 tsMDB 看得到時 |
| pgcObservChange | cObserver.cpp:2349 | 註解掉（「Mark MDB 避免卡卡」） |
| DateTimePicker1CloseUp | cObserver.cpp:2781 | 註解掉 |
| bAutoSaveEventLog（:3001-3170）裡四次 | cObserver.cpp:3084、:3107、:3130、:3153 | bAutoSaveEventLog 唯一的呼叫在 btAutoSaveClick :3172-3175，**被註解掉**（:3174）；906 全樹 *.cpp／*.h 沒有別的呼叫者 |

移植樹相同：BtnQueryClick D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp:1626、BtnSaveClick :6235、DateTimePicker1CloseUp :6309、lbltTotalLoaderMouseDown :6697 都有本體；bAutoSaveEventLog :6444 在移植樹也沒有呼叫者（:6581、:6607、:6633 是它自己裡面的 BtnQueryClick）。

### 2.4 網頁

- D:\HT9045\web\page\Data.Observer.html:130：pgcMessage 的頁籤只有 data-t 1 Text、2 Time Data、3 SG_JamCount，**沒有 tsMDB**（golden bUseMDB＝false 時看到的就是這三頁）。
- St01 E-021 的 act.observer.msgTab 接受 arg 0～3（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp 檔尾 W906_ObserverAct，arg 0＝tsMDB）；頁面不會送 0，不影響。

---

## 三、要決定的事（給 Steven；St02-M 轉 ST01-M 統整）

**Q1　Observer → System Message 要不要有「MDB」這一頁？**

- 這是什麼功能：golden 這一頁讓操作員選日期區間，查事件記錄、各種記錄、警報歷史與統計、Jam 圖表、警報代碼表、生產摘要報表，並另存成 xls／csv／bmp。
- 問題：golden 只在用資料庫（bUseMDB）的客戶看得到；我們已裁決不用資料庫（bUseMDB 永遠 false），所以照 golden 這一頁不出現。
- 選項：
  - A（照 golden，建議）：不做。網頁維持三個頁籤；OB-5 在 E-019 標「golden 藏頁（bUseMDB＝false），不需接」。例：操作員打開 Observer → System Message，只看到 Text、Time Data、SG_JamCount，跟一般客戶的 BCB 版一樣。
  - B（新功能）：做一個「查 CSV」版的 MDB 頁。例：選 2026/10/01～10/02、Alarm History (JAM Only) → 從 D:\HT9045_Log\EventLogTxt\YYYY\MM\EventLogTxt_YYYYMMDD.csv 撈 JAM 開頭的列顯示。要另外回答 Q2～Q4。
- 建議：A。理由：golden 與兩次裁決都指向「不出現」；B 等於重寫 cMyDB 的 SQL 查詢，golden 沒有可照翻的本體。
- 目前狀態：移植樹與網頁已經是 A 的樣子，不需要改任何一行。

**Q2（只有 Q1＝B 才要答）　19 種「Display Data」哪幾種要做、各自讀哪個檔？**

- golden 每一種都是一條 SQL（cObserver.cpp:2437-2622、:2644-2734）。CSV 版要逐一指定來源，例：Event Log／Alarm History* → EventLogTxt CSV；Process／Message／Motion Record → cMyDB P4 的 CSV 記錄（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\LogObjects.cpp）；Production Record／Alarm Statistics／Jam Chart（依手臂軸號 100～590 分組，:2725-2734）沒有現成的 CSV 欄位可對；Production Summary Report 已有（O19SummaryReport.cpp）。

**Q3（只有 Q1＝B 才要答）　Save 存到哪？**

- golden 用 Windows 的另存對話框（預設 D:\EventLog.xls／.csv／.BMP）。網頁打不開機台的檔案對話框（同 OB-10、Q41 CC-E12 的情況）。選項：瀏覽器下載，或存到機台固定路徑。

**Q4（只有 Q1＝B 才要答）　Total Loader 右鍵（濾掉重複訊息）網頁要不要保留右鍵的操作方式？**

- golden 右鍵切換、畫面上沒有任何提示（cObserver.cpp:3180-3183）。

給筆電／ST01-M：無（Q1＝A 時不動任何人的檔）。
