# St01 唯讀盤點報告：生產資料的顯示與存檔（golden → 網頁）

> 本檔由 St01（github-02）從 St01 本機 scratch 的 `audit_prod\report.md` 複製進交接分支（原檔不在 git，你們那邊看不到）；只刪掉開頭一段誤帶進來的非報告外框並去掉整段縮排，其餘原樣。文中的 scratch 路徑只在 St01 那台有效。


- 範圍：golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy`。先把 cp950 轉成 UTF-8 放到 scratch 的 `audit_prod\golden_utf8\`，這次連 .dfm 也一起轉，共 859 檔，只換 CRLF，行號不變。
- 移植樹：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0`；網頁：`D:\HT9045\web\page\`。
- ⚠ HEAD 在盤點途中變了：開工時是 62f064bf，另一個 session 在 19:20 合進 `058af036`（Merge origin/main）。文中的關鍵行號都已在 058af036 重新 grep 核對過。
- 沒有改任何檔、沒有 build、沒有 commit、沒有開子代理。
- 新 S 編號照主 session 的更正，從 **S111** 開始暫編。

---

## 1. 總表（依頁面／區塊，共 40 列：✅ 10、⚠ 14、⏳ 12、— 4）

分類說明：✅＝有，而且是機台在用的那一份；⚠＝有，但來源或更新有問題（含寫死、看起來像真值的假數字）；⏳＝網頁沒有；—＝golden 也只在特定客戶或選配時才顯示（S25，只列不做）。

### 主畫面 main.html

| # | 區塊 | golden 的來源與更新時機 | 網頁現況 | 分類 | 歸屬 | 備註 |
|---|---|---|---|---|---|---|
| 1 | 機台狀態 palMainStatus | ShowRunLabel 等 | tag `machine.state`，每 500 ms | ✅ | 已完成 | main.html:134 |
| 2 | 狀態列 [0]：Index 節拍 | csystem.cpp:20795，每次 Index 完成就寫一次 | **寫死 "0.662 Sec"**（main.html:241）。tag `status.indexTime` 列在 kUnloadedTags 裡，永遠是 null（WebBridgeTags.cpp:401） | ⚠ 假值 | St01 | 移植樹的 RunInfo.IndexTime 其實有值（csystem.cpp:24169），只有畫到狀態列那一行被 GATE H1-03 擋住（:24194） |
| 3 | 狀態列 [2]：UPH | ainarm9045.cpp:5762，寫 "UPH = "+UPH_StringGrid[3][1]（這一格就是 RunInfo.iUPH） | **寫死 "UPH = 533"**。`prod.uph` 有發布（ChanProduction.cpp:77），但沒有任何頁面在用 | ⚠ 假值 | St01 | RunInfo.iUPH 在移植樹是活的（ainarm9045.cpp:8780） |
| 4 | 狀態列 [7]：Curr UPH／Net UPH | CaculateUPH（cShowBinSelect.cpp:2428），由 TimerAutoCleanCountTimer（:2317）驅動，條件是 G10 開且 SystemStart | 沒有 | ⏳ | St01 | 本體已翻（移植樹 cShowBinSelect.cpp:908），但沒有計時器去呼叫它 |
| 5 | 狀態列 [5]：測試秒數 | TfMain::Timer2Timer（main.cpp:21855-21903） | 寫死 "0" | ⏳ | 發布歸 St01；產生端歸 Jimmy | St02 的 P2f 也在翻 Timer2Timer |
| 6 | 狀態列 [3]：版本 | main.cpp:10690-10754（asHandlerVersion 加 _SLT／_L8／_CCD／_Debug 尾碼） | 寫死 "V3.21" | ⚠ 假值 | St01 | 開機時就已經有 asHandlerVersion（wb_serve.cpp:3886） |
| 7 | 狀態列 [6]：時鐘 | ProcessTimeUpdate（main.cpp:8233-8242），用機台的時間 | 用瀏覽器自己的時鐘（main.html:480-484）。tag `clock.text`（WebBridgeTags.cpp:1100）沒有人用 | ⚠ 來源不同 | St01 | 瀏覽器不在機台 PC 上時兩邊時間會不一樣 |
| 8 | Fail alarm count | main.cpp:4199-4222，G04 開時顯示 Prod.iContsFail*AlarmCT（配方值） | **寫死 "All=10 Site=10"**（main.html:128） | ⚠ 假值 | St01 | |
| 9 | 功能清單 palFunc（Cleaned Count、Auto Skip、ATC／RTC／FTP 線上…） | AutoClean.cpp:1359、ainarm9045.cpp:2475；清單建立在 main.cpp:1507-1529 | 全部寫死（main.html:224-233）。其中 "1: 44.65 %" 其實是 lbArm1Torque 的扭力百分比，不是良率。11 個 `status.*` tag 永遠是 null | ⏳ | St01 | 線上要送什麼形狀還沒裁（WebBridgeTags.cpp:358-399） |
| 10 | Tray 圖 PanelMain6（mtLoader…mtFix3） | TTMyTray 綁在 MOT[].Tray 上 | 假格子，顏色寫死（main.html:461-479）。全樹沒有任何 tray 狀態的 tag | ⚠ 假值 | St01 | |
| 11 | SitePanel（DUT on/off） | LastSet.bUseTestSocket | tag `site.arm*.s*` | ✅ | 已完成 | 這是設定值，不是產量 |
| 12 | Main.Record（sgDebugRecord、兩個 memo、CLEAR、Save Log） | main.cpp:8923-8945、:31109 | 靜態空表，按鈕沒有動作 | ⏳ | St01 | 價值低：golden 只在 bShowMainDebugRecord 開時才有 |
| 13 | Main.Logs 的 Production Log | ProductionLog（cpublic.cpp:595，O06，矽品蘇州用） | 靜態空白 | — | Jimmy（本體在 `#if 0`） | O06 選配 |
| 14 | Main.UnloaderInfo | main.cpp:8245（ATK 用） | 靜態 | — | S25 | |

### Data.SortCT

| # | 區塊 | golden 的來源與更新時機 | 網頁現況 | 分類 | 歸屬 | 備註 |
|---|---|---|---|---|---|---|
| 15 | Loading、33 站數量與良率、Total、ART、IC Count | cSortCT.cpp:210-442（ShowLoadingIC／ShowSortIC），機台流程每放一顆就呼叫 | tag `sort.*`，每 500 ms（WebBridgeTags.cpp:939-973、:2652-2708），讀的是 LastSet 本尊 | ✅ | 已完成 | 上游沒有在產生數字，見 Jimmy 的 J1 |
| 16 | Clear Count、點兩下清單站、Lot ID | :585、:1933、:869 | `act.sortCT.clearCount`（wb_serve.cpp:4816） | ✅ | 已完成（S65／S97） | spbClearAllCount 只有 CC_ASE_M 才有，屬 S25 |
| 17 | ShowSortIC 的副作用：RunInfo.iUnloadCount、SECS Pass／Fail、ShowCategoryBin 的低良率告警 | cSortCT.cpp:352-441 | 9045 的 OutArm 路徑是 stub，不會呼叫；csystem 的 ART 路徑是空巨集（csystem.cpp:4105-4106） | ⚠ | Jimmy | `prod.unload` 讀的是 RunInfo.iUnloadCount，會跟 `sort.total` 對不上 |

### Status.ShowBinSelect

| # | 區塊 | golden 的來源與更新時機 | 網頁現況 | 分類 | 歸屬 | 備註 |
|---|---|---|---|---|---|---|
| 18 | Test Bin 分頁 | ShowBinSel（cShowBinSelect.cpp:420），33 站 | 只接了 6 站：`bin.auto1`～`bin.fix3`（WebBridgeTags.cpp:986） | ⚠ 只有 6/33 | St01 | AUTO_EMPTY_COLOR>=3 的機型會少 Auto4-6 和 Fix4-12 |
| 19 | Category Info（各 Bin 數量與百分比） | ShowCategoryBin（:1832），讀 LastSet.iBinData32[0][i] | 寫死的示意列（Status.ShowBinSelect.html:31-38） | ⏳ | St01 | 不可以在 tick 裡呼叫 ShowCategoryBin：它會發 DoLowYieldAlarm、也會 ClearCount |
| 20 | UPH Information | ainarm9045 的 CalculateUPH | 寫死 533／521／0.662 | ⏳ | Jimmy 先處理 J3，再由 St01 發布 | |
| 21 | Bin Display Status | ChangeBinDispStatus（:208），NUMBER_PANEL 硬體選配 | 寫死「收料中／待機」 | ⏳ | St01（低） | |
| 22 | Index 分頁 | LastSet.iIndexInputOutPut[0..3]（cSortCT.cpp:425-428）、btnClearCountClick（cShowBinSelect.cpp:2411） | 寫死 "DUT1..4" | ⏳ | St01 | facade 沒有這四個元件（GATE G4） |

### Data.ContactCT、Data.TestCategory

| # | 區塊 | golden 的來源與更新時機 | 網頁現況 | 分類 | 歸屬 | 備註 |
|---|---|---|---|---|---|---|
| 23 | Data.ContactCT 各 Site 次數與良率 | 每筆測試結果後 atester_ProcessCount.cpp:1999 呼叫 sgYield->Refresh | `contactct.get`，只在開頁或點選項時才抓（ht9045_contactct_wire.js:95） | ⚠ 不會即時更新 | St01 | 除了 auth.*，每個 WS 指令都要權杖（WebBridgeServer.cpp:1383-1390） |
| 24 | Data.TestCategory | cTestCategory.cpp:145-359 | tag `tcat.*`（WebBridgeTags.cpp:2798-2827） | ✅ | 已完成 | |

### Data.LotInfo

| # | 區塊 | golden 的來源與更新時機 | 網頁現況 | 分類 | 歸屬 | 備註 |
|---|---|---|---|---|---|---|
| 25 | Lot 分頁 17 格 | 開頁時 FormShow 呼叫 SetLotStart(…, true) 讀 config.ini | tag `lot.*`，但開機到第一次 lot.start 之前都是 "---" | ⚠ | Jimmy | 那一行在 GATE WC-19，標為 SAFETY 紅線（forms/fLotInfo.cpp:4037-4046） |
| 26 | Loading | LastSet.SendCT[0] | `sort.loading` | ✅ | | |
| 27 | BarCode 計數、Tester Log、Selection | uLotInfo.cpp:10168 等 | `lotinfo.op`（WebLotInfo.cpp） | ✅ | 已完成 | 清除前存 .xls 目前是 no-op（S44） |
| 28 | Lot End | sbSECSLotEndClick（:1387）→ SetLotEnd（:2014-2380） | 網頁沒有按鈕；移植樹沒有 SetLotEnd（fLotInfo.h 沒有宣告；Command.cpp:17888-17890 在 `#if 0`） | ⏳ | St01 | golden 只在 SECS/GEM 開或 bLotStartLockCriticalPara 時顯示這顆鈕（uLotInfo.cpp:407-412） |
| 29 | ATR 計數、LowYield IC Count、AMR 分類 | | | — | S25 | |

### Data.Observer

| # | 區塊 | golden 的來源與更新時機 | 網頁現況 | 分類 | 歸屬 | 備註 |
|---|---|---|---|---|---|---|
| 30 | Operating Information 8 格 | Timer1 每秒：GetMachineData（:755）、ProcessRunInfo（:2255） | `observer.get`，每秒一次，通道本身是好的 | ⚠ 值不會動 | 時間累計歸 St01；Jam 次數歸 Jimmy | 移植樹沒有任何地方在累加 SystemAccSecond（golden 在 TfMain::UpdateRecordScreen，main.cpp:8584）；iJamCount 也沒有人在加（golden note.cpp:376） |
| 31 | 版本、Factory、四個 *Version | | `observer.get`；沒有來源的格子顯示 "---" 並寫明原因 | ✅ | | |
| 32 | Tester Category | WriteCategoryData | `observer.get` tab=1 | ✅ | | |
| 33 | System Message（EventLogTxt） | GetEventLogText | `observer.get` | ⚠ 只看得到舊檔 | St02（S72） | 移植樹的 slEventLog 是 NULL，不會寫新事件 |
| 34 | Counter 分頁 StringGrid2／3 | WriteContactKind | 靜態 | ⏳ | St01 | 本體已翻（cObserver.cpp:840），但 observer.get 只收 tab 1 和 3（:7292） |
| 35 | Yield 分頁：mtRowA-D、ChartYield | UpdateBin、UpdateYieldChart | 靜態 | ⏳ | St01 | HistroyBin 在移植樹是活的（atester_ProcessCount.cpp:2497）；iYieldChart 沒有人寫 |
| 36 | Test Information（TimeInfoGrid 等） | RecordIndexTime（:2815）等 | 靜態 | ⏳ | 顯示歸 St01；上游歸 Jimmy（H1-02） | |
| 37 | Record、OEE、Lot information、SG_JamCount 分頁 | | | — | S25（Record 分頁待確認） | |

### Data.CounterClear、Status.CounterSel、prod.* 通道

| # | 區塊 | golden 的來源與更新時機 | 網頁現況 | 分類 | 歸屬 | 備註 |
|---|---|---|---|---|---|---|
| 38 | Data.CounterClear 九類清除 | spbExeClick → ClearCount → 設 bRefreshCount；TfMain::Timer10Timer（main.cpp:35331-35355，每 100 ms）看到後做 WriteCTInfo、ShowSortIC、ClearYieldCount | `counterclear.*`（wb_serve.cpp:5557）：清記憶體並寫 lastdata.dat | ⚠ 清完的收尾沒做 | St01 | bRefreshCount 沒有人讀（cCounterClear.cpp:412）；WriteCTInfo 沒有任何呼叫者；刪 BinCount.txt 那段在 GATE CC1（:152-166） |
| 39 | Status.CounterSel | TfCounterSel | C 路 | ✅ | | |
| 40 | `prod.*` 16 個 tag | | 沒有任何頁面在用 | ⚠ | St01 | 併進 S112 處理 |

### 存檔點

| 存檔 | golden | 移植樹 | 狀態與編號 |
|---|---|---|---|
| lastdata.dat，事件觸發 | 23 個呼叫點，**沒有一個在 Timer 事件裡**（20260926 19:3x，逐一 awk 所在函式名） | S65 已逐一對照 | ✅ |
| lastdata.dat，關程式 | FormClose（main.cpp:11861）WriteLastDataFile(true) | 只寫 `Program Close=1`（wb_serve.cpp:5965、:6157） | ⏳ S95。因為 golden 沒有定時存檔，這一步不做的話，上一次事件存檔之後累計的產量，關程式時全部丟掉 |
| Arm*.dat（各 Site 計數） | WriteCTInfo 三個呼叫點：one cycle 結束（csystem.cpp:14455）、清除後（main.cpp:35335）、關程式（:12195） | 沒有任何呼叫者；one cycle 那一處是空巨集（csystem.cpp:4096）；開機有讀（wb_serve.cpp:4163） | ⏳ 分成新 S111、S95、Jimmy J4 三段 |
| BinCount.txt | 開機讀；csystem 3 處寫；Exit 鈕 sbCloseProgramClick 寫（:29129）；清除時刪（cCounterClear.cpp:118-152） | 讀已接；寫是 S76；刪檔在 CC1 閘；網頁的 Exit 鈕（main.html:31）沒有接任何動作 | ⚠ S76、S111、S95 |
| DailyJamRate | 跨日時寫（cprod.cpp:970）；FormClose 寫（:11919） | 讀已接（S91）；沒有 Jam 計數，所以沒東西可寫 | ⏳ S95 與 Jimmy J2 |
| EventLogTxt | slEventLog | NULL | ⏳ St02（S72） |
| sqlite 生產資料 | MyDBIProductionData、O19 每日報表（DoProduction_Summary_Report） | DB 沒有開、報表函式是空殼 | ⏳ St02 |
| Lot Info log、End Time | SetLotEnd | 沒有 | ⏳ 新 S117 |
| 每顆 IC 的生產紀錄 PordRec.SaveRecord | aoutarm9045.cpp:3145-3221 | 9045 OutArm 那一支是 stub | ⏳ Jimmy J1（旗標部分屬 S25） |
| RunMode.txt、LotSummary.csv、RMS、MachineRecord、TimerRecordLoaderDate、btSavelog | FormClose | 沒有 | ⏳ S95（btSavelog 原本沒列在 S95，要補進去） |

---

## 2. St01 建議的新 S 編號（從 S111 起，依「價值高、風險低」排序）

| # | 內容 | 工作量 | 風險 | 相依 |
|---|---|---|---|---|
| S111 | **Counter Clear 清完之後的收尾**。(a) 翻 golden TfMain::Timer10Timer 裡 bRefreshCount 那一段（main.cpp:35331-35355），接在 PumpTick 上，做法比照 P1 的 W906_FlushFlagTick（WebBridgeTags.cpp:2446）：WriteCTInfo、ShowLoadingIC／ShowSortIC（含 ART 版）、bShowSiteYield[] 清成 false、ClearYieldCount。(b) 解開 GATE CC1，也就是清除時刪 BinCount.txt 那一段。CC1 的理由是舊波次任務說明裡的寫入禁區清單，已經過期 | 小 | 會寫 system\Arm*.dat、會刪 BinCount.txt，這兩個都不在版控，做之前要先備份 | Timer10 是 TfMain 的計時器段；P1 當時是使用者裁決，建議先問 Steven |
| S112 | **主畫面狀態列與功能清單改成真值**。第一步先把寫死的 7 個假值改成 "---"，零風險。第二步：`status.indexTime` 接 RunInfo.IndexTime、`status.uph` 接 RunInfo.iUPH、版本字串照 golden 的組法、改用 `clock.text`、Fail alarm count、Cleaned Count、Auto Skip，這些名字移出 kUnloadedTags。`prod.*` 一併整理 | 小～中 | 低，只影響顯示 | 線上要送 caption 還是送值加可見度，要 Steven 裁決。如果要引擎多一種格式字串的渲染型別，ht9045_wire_engine.js 是 Jimmy 登記的檔，要嘛寫在頁面自己的 JS，要嘛等 Jimmy |
| S113 | **Observer 的累計時間與 Run Info**。翻 golden UpdateRecordScreen（main.cpp:8584-8697，每拍）和 UpdateRunInfo（:22740-22805，每分鐘）：累加 SystemAccSecond 各項、iYieldChart、MTBA／MUBA。客戶分支照 S25 只註記 | 中 | 會改到 LastSet 裡的時間計數（之後會跟著存進 lastdata.dat）。需要的 HSys.SysTimer.LatchCycleTime 移植樹已經有（database.h:244、myTimer.h:25） | 要 Steven 裁決；UpdateRunInfo 在 Timer2Timer 裡，St02 的 P2f 也在翻同一支，要先對齊 |
| S114 | **Status.ShowBinSelect 其餘四個分頁，並把 Test Bin 補到 33 站**。Category Info 直接發布 iBinData32；Index 分頁發布 iIndexInputOutPut[0..3]，Clear Counter 鈕另開一個 act（照 golden：Insufficient(108)、兩段確認、ClearCount(ctIndexCount)）；UPH 分頁先顯示 "---" | 中 | 低 | UPH 分頁要等 Jimmy 的 J3 |
| S115 | **Data.ContactCT 即時更新**。頁面開著時每秒送一次 `contactct.get`，別人拿著權杖就跳過這一拍（比照 ht9045_observer_wire.js:293）；或改成加一個「版本」tag，變了才重抓 | 小 | 低 | 唯讀的 *.get 要不要免權杖，請 Steven／Jimmy 決定（WebBridge 是共用元件） |
| S116 | **Data.Observer 其餘分頁**：W906_ObserverJson 放行 tab 0（Counter）、4（Yield）、6（Test Information） | 中 | 低，只讀 | ChartYield 要等 S113；Index 平均要等 Jimmy 的 J6 |
| S117 | **Lot End**：把 SetLotEnd（366 行）和 sbSECSLotEndClick 翻進 forms/fLotInfo.cpp，網頁照 golden 的顯示條件加一顆 Lot End 鈕 | 中～大 | 中：會動到 RunInfo.bLotStart 和主畫面按鈕的 Enabled，跟 START 的前置條件相連 | Jimmy（WC-19／WC-20 的 lot 閘）、St02（Command.cpp:13707、:17889）。建議開工前先問 Steven 要不要做 |
| S118 | **主畫面 Tray 圖**：發布 MOT[].Tray 的格子狀態（比照 io.di 位元打包，或每盤 HowManyIC 加版本號）；在那之前先把假格子改成「不可知」 | 中 | 低 | 如果 MotionView 也要用同一份資料，先跟那邊對好格式 |
| S119 | **Main.Record 分頁**：三個顯示區、CLEAR（spbClearRecordClick）、Save Log | 小 | 低（價值也低，golden 要 bShowMainDebugRecord 開才有） | CLEAR 裡的 UpdateRecordScreen 要等 S113 |

**S95 拆分建議（請 Steven 定，不另外開號）**：FormClose 裡屬於生產資料的那幾項，就是 S106 說的「生產資料…存檔」：WriteLastDataFile(true)、SaveJamRateByDay、WriteCTInfo、btSavelog、Exit 鈕的 BinCount 寫入。建議把這幾項往前提。另外網頁的 Exit 鈕目前沒有接任何動作。

---

## 3. 給 Jimmy／Steven02 的清單

**Jimmy**

- **J1**：aoutarm9045.cpp:220 的 `static bool DoOutArmPlaceToAuto(int){return true;}` 是 stub（呼叫點在 :1541），擋住了 golden aoutarm9045.cpp:2812-3360 的全部計數：BinCT、iBinData32、BinCT_ART、PordRec 生產紀錄、QA 計數…。標準 9045 機型的 SortCT、ShowBinSelect、Observer 數字因此只會停在 lastdata.dat 讀進來的值。**優先度最高。**
- **J2**：Jam 次數。golden 在關告警框時（TfNote::FormClose，note.cpp:2551）呼叫 CheckRecordJamType（:344-386）累加 iJamCount；移植樹 0 個命中。這條關框路徑要跟告警通道（dialog.response）一起看。
- **J3**：CalculateUPH 寫到 ainarm9045.cpp:8649-8688 的私有 seam 表格，真正的 fShowBinSelect->UPH_StringGrid 一直是空的。受影響的還有 Command.cpp:3455 的遠端 UPH 查詢。
- **J4**：csystem.cpp:4096 的 W7C2_FCOUNTER_WRITECTINFO 是空巨集，one cycle 結束時不會存 Arm*.dat。
- **J5**：csystem.cpp:4105-4106 的 SortCT 兩個空巨集。本體現在已經有了，可以退役。
- **J6**：GATE H1-02（csystem.cpp:24179）。理由寫的是「shim 沒有 RecordIndexTime」，但 forms/fObserver.h:826 現在已經有這支，理由可能過期。
- **J7**：GATE WC-19（Lot 資訊開機讀回）。
- **J8**：S76；**J9**：低良率告警的產生端；**J10**：S95 裡屬於 Jimmy 的項目；**J11**：ProductionLog；**J12**：Timer2Timer 的測試秒數。

**Steven02**

- **T1**：S72 slEventLog。
- **T2**：cMyDB 的生產資料、O19 每日報表、APHeadLabel18。
- **T3**：Observer 的 GPIB／TTL 版本那兩格，Tester 引擎上線後可能已經有來源。
- **T4**：Command.cpp:13707、:17889 的遠端 Lot End 目前回 OK，但實際沒有執行（S117 做完後要一起解）。
- **T5**：Timer2Timer 跟 S113 的分工要對齊。

---

## 4. skill 裡過期的敘述

- **ht9045-json-bridge/references/porting-gaps.md:414-441（第十一節）**：寫「cSortCT.cpp 整支沒翻、ShowSortIC 是 no-op」。現況：本體已翻並安裝（cSortCT.cpp、forms/fSortCT.cpp:355-356），只剩 csystem 那兩個空巨集。
  - 同一句過期的話還寫在程式裡：JsonBridge/ChanAction.cpp:201，`act.main.clarnData` 的回應還會把這一項列進 sideEffectsSkipped，這個回應現在是錯的。
- **porting-gaps.md:618-640（第十八節）**：漏了清除那一側，也就是 CC1 把刪 BinCount.txt 擋住了。
- **ht9045-html-json/references/route-b-wbserve.md:419**：寫 CounterSel、CounterClear、ContactCT 三頁「尚未接」。現況：三頁都已經接了。
- **ht9045-html-version/SKILL.md:382-386**：寫 ShowBinSelect 有 6 個 tag、SortCT 有 8 個，是「完成態」。現況：SortCT 的 tag 遠多於 8 個；ShowBinSelect 有 4 個分頁是假值，不是完成態。
- **ht9045-html-version/SKILL.md:108**：提到 bin-mode.js 和 Production context。web 底下找不到 bin-mode.js（find 0 筆），Production context 也已經隨 JSON Simulator 退場。
- **ht9045-html-version/references/production-runtime.md（整份，例如 :3、:139）**、**observer-tables.md:75**、**bin-select-form.md:49**：寫的都是 JSON Simulator／HTSettings 那條路，那條路 0916／0918 已經退場。
- **ht9045-eventlog-analyzer/SKILL.md:67** 和 **references/ela-web-conversion-plan.md:69**：寫 Observer「有版面沒資料」。現況：System Message 分頁已經接上。
- **ht9045-json-bridge/references/api-shape.md:154**：寫「Clarn_Data、ClearCount 有沒有翻，動手前先量」。現況：兩支都已經翻了。
- **ht9045-yield-flow/SKILL.md:8-9、:29**：資料通道寫的是 B 路，預設原始碼根目錄是 897。現況：Setup.YieldMonitoring 走 C 路（route-c §6 第 5 列），golden 是 V912。
- **ht9045-lotinfo-flow/SKILL.md:26**：用的是 V899 路徑，St01 現在的 golden 是 V912（S37）。
- 程式註解（不是 skill）：
  - WebBridgeTags.cpp:390-399 說 indexTime 和 uph「需要還沒翻的 main.cpp 才能組」。實際上值已經有了（RunInfo.IndexTime、RunInfo.iUPH）。
  - ht9045_wire_dataobserver.js 對那 14 格寫的「wb_serve 沒有 producer」，已經被 observer_wire.js 取代。

**「全樹沒有」這類結論的量法**（20260926 19:1x～19:4x，對 058af036 用 `git grep … ':!*/tests/*' ':!*/docs/*'`）：SystemAccSecond 的 `+=`、`iJamCount[i]++`、CheckRecordJamType、WriteCTInfo 的呼叫者、SetLotEnd 的定義、tray 狀態 tag、web/page 裡 `prod.*`／`status.*` 的使用者。每一項都是 0 筆。平行作業中這些數字可能會變，開工前要重跑。

**相關檔案：**
- D:\HT9045\HT9011UC_Cpp_V3.33.906.0\aoutarm9045.cpp
- D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cCounterClear.cpp
- D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp
- D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp
- D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ainarm9045.cpp
- D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp
- D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp
- D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp
- D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\ChanAction.cpp
- D:\HT9045\web\page\main.html
- D:\HT9045\web\page\Status.ShowBinSelect.html
- D:\HT9045\web\page\ht9045_contactct_wire.js
- C:\Users\steven\AppData\Local\Temp\claude\d---github\99781e6b-2c4e-4591-81e8-fc879566566c\scratchpad\audit_prod\conv.py
- C:\Users\steven\AppData\Local\Temp\claude\d---github\99781e6b-2c4e-4591-81e8-fc879566566c\scratchpad\audit_prod\golden_utf8\