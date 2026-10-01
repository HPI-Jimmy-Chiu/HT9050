# E019 盤點：Data／Status 頁還沒接上的 golden 事件（工作卡 ST02-C2）

- 讀者：Steven（主管，不必看程式也能讀；程式出處放在表格最右邊幾欄）、St01（用這張表把 St01 頁面的列拆成 todo）。
- 依據：St01 → St02 工作卡 ST02-C2 E-019（D:\HT9045\docs\handoff\FROM_STEVEN.md §4，20261001 18:12，commit b1f866d4）原文：「Q41 §6 的 Data／Status 頁從來沒拆成列……請照 Q41 的做法列一張表：每一頁 × golden 每個事件處理器（檔:行）× 移植樹有沒有本體 × 網頁有沒有送 × 會不會動機台／寫檔 × 頁面歸誰；放 docs\ 新檔，不改程式」。St02-M 派給 St02-E。做法照 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md（摘要在前、一列的單位、不列的種類、客戶專屬另列）。
- 盤點人：St02-E 的唯讀 helper。只讀不改程式；本檔是這次唯一新增的檔。
- 對照版本：移植樹與網頁＝origin/main 4b890ffb（在 D:\AI_TempFile\st02-s14 讀；表中路徑寫成 D:\HT9045\... 的正式位置，行號以 4b890ffb 為準，主 checkout 不在這一版時可能差幾行）。golden＝906 D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven（Steven 20260927 裁決：本機 906 golden 先用這一份；cp950）。⚠ Q41 用的是 V912，行號不同；網頁 title 和 C++ 註解裡寫的行號多半是 V912。
- 日期：2026-10-01

---

## 一、摘要（先看這一節就好）

**範圍**：Q41 §6 只粗估、從來沒拆成列的 Data／Status 頁，一共 **17 頁**：Data.LotInfo、Data.Observer、Data.ContactCT、Data.SortCT、Data.SmartDiagnostic、Data.StartCondition、Data.Builder、Data.CounterClear、Data.TestCategory，Status.ShowBinSelect、Status.LtcSensor、Status.Security、Status.GroundMan、Status.TemperFrom、Status.TowerLight、Status.ShowMessage、Status.CounterSel。17 頁都在 D:\HT9045\web\background.html 的視窗表裡（:436-472），操作員都開得到。

**計數單位**：同 Q41——表格一列＝一個 golden 處理器，或幾個元件共用同一個處理器、或同一個分頁上做同一件事的一組。已經做好的、純顯示（重畫、換版面、改視窗大小）、純輸入輔助（按鍵過濾、點欄位跳小鍵盤）不列；但每頁表格下面都把它們逐支點名，**17 頁、332 支 golden 處理器每一支都有歸類**（已接好／本表的列／客戶專屬／不列）。客戶專屬另列第四節。

**數字（17 頁）**

| 頁（D:\HT9045\web\page\） | golden 處理器 | 已接好 | 缺（本表列數／涵蓋幾支） | 客戶專屬 | 不列 | 頁面歸誰 |
|---|---|---|---|---|---|---|
| Data.LotInfo.html | 108 | 6 | 13 列／16 支 | 57 | 29 | St01 |
| Data.Observer.html | 56 | 15 | 10 列／30 支 | 2 | 9 | St01 |
| Data.ContactCT.html | 9 | 3 | 3 列／4 支 | 0 | 2 | St01 |
| Data.SortCT.html | 19 | 8 | 1 列／1 支 | 1 | 9 | St01 |
| Data.SmartDiagnostic.html | 12 | 6 | 0 | 0 | 6 | St01 |
| Data.StartCondition.html | 28 | 14 | 2 列／3 支 | 2 | 9 | St01 |
| Data.Builder.html | 13 | 12 | 0 | 0 | 1 | St01 |
| Data.CounterClear.html | 6 | 6 | 1 列（開頁等級） | 0 | 0 | St01 |
| Data.TestCategory.html | 4 | 2 | 0 | 0 | 2 | St01 |
| Status.ShowBinSelect.html | 16 | 2 | 5 列／6 支 | 3 | 5 | St01 |
| Status.LtcSensor.html | 15 | 0 | 5 列／11 支（含開頁等級） | 0 | 4 | 待確認 |
| Status.Security.html | 13 | 12 | 0 | 0 | 1 | St01 |
| Status.GroundMan.html | 10 | 2 | 3 列／6 支 | 0 | 2 | St01（頁）；RS232 段待確認 |
| Status.TemperFrom.html | 8 | 0 | 2 列／4 支 | 2 | 2 | 待確認 |
| Status.TowerLight.html | 6 | 5 | 1 列／1 支 | 0 | 0 | St01（試聽鈕 Jimmy） |
| Status.ShowMessage.html | 6 | 0 | 0 | 0 | 6 | 待確認 |
| Status.CounterSel.html | 3 | 3 | 0 | 0 | 0 | St01 |
| **合計** | **332** | **96** | **46 列／82 支** | **67** | **87** | |

**依歸屬（46 列）**

| 項目 | 列數 | St01 | Steven02 | Jimmy | 待確認 |
|---|---|---|---|---|---|
| 缺的事件 | 44 | 28 | 2 | 1 | 13 |
| 缺的開頁等級 | 2 | 1（＋Jimmy 政策表） | 0 | 0 | 1 |
| 合計 | 46 | 29 | 2 | 1 | 14 |

「頁面歸誰」照 git log 與交接紀錄：這 17 頁的網頁和 C++ 入口（WebLotInfo.cpp、cObserver.cpp 的 observer.get、contactct.get、WebSortCT.cpp、WebSmartDiag.cpp、WebBuilder.cpp、counterclear.\*、WebShowBinSelect.cpp、WebLevelSet.cpp、FileRW\GroundMan.cpp、WebTowerLight.cpp、FileRW\IniConfig_CounterSel.cpp、FileRW\StartCondition.cpp）都是 St01 這兩週做的（S56、S64、S94、S97、S113～S117 等）。Status.LtcSensor、Status.TemperFrom、Status.ShowMessage 三頁只有 20260923 進版控時的畫面，沒有人接過，所以標「待確認」。**沒有一頁是 St02 的頁**；跟 St02 領域（Tester 通訊、cMyDB）有關的只有 2 列（LI-12、OB-5），另外客戶專屬裡有 2 項也碰到 Tester 通訊（第四節標「St02 相關」）。

**工作卡前提的更正**：卡上寫「光 Data.LotInfo 約 44 個 click／change 處理器、移植樹沒有 TfLotInfo 本體」。實際（906 golden）：uLotInfo 有 108 支處理器，名字是 Click／Change／DblClick／DropDown 的有 65 支（其中很多是客戶專屬）；移植樹**有** TfLotInfo 門面 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp（7351 行），108 支裡 59 支有本體，但網頁只接到其中 6 支——其餘有本體的多半沒人呼叫。

**最值得先做的 8 項**

1. **GM-2　接地監測沒有在跑**（待確認，要上機）。golden 的接地監測（Status.GroundMan 的計時器，每 30 ms）發現接地阻值異常時會**停所有馬達、跳 WAR1609、讓機台停下**；移植樹那一段閘住、也沒人呼叫，開機的 RS232 也沒開。裝了接地監測板（USE_GROUND_MAN=1）的機台，接地異常不會停機。
2. **TP-1　溫度監視列的計時器沒有在跑**（待確認，溫度＋IO）。golden 開機就打開這個視窗，它每 0.1 秒刷新所有溫度、把溫度字串給 SECS，而且在 RTC 或 ATC 6.0 以上的機台**依過溫開關 CCD 冷卻**（SW[SwCCDCooling]）。移植樹沒有這支，網頁 11 個溫度通道全部顯示 ---。
3. **LI-4／LI-5　新 ATC 系統的通訊迴圈在 Lot Info 的計時器裡**（待確認，請 Ifor 看）。ATC_SYSTEM＝新 ATC 時，golden 由 Lot Info 視窗的 NetATCTimeTimer（793 行）負責跟 ATC 連線、送溫度、Run／Stop；Lot 頁的「ATC On／Off Line」也在這裡。移植樹兩支都沒有；ATC\ATCInterface.cpp 有沒有另外做掉這件事，這次沒追到底。
4. **LI-1　Lot Start 的 golden 檢查沒跑**（St01；伺服器那一臂是筆電寫的）。主畫面 Lot Start 送的 lot.start 只設 Lot ID、Operator ID 就呼叫 SetLotStart；golden 按 Lot Start 時的 1035 行（字元檢查、必填、TCP 測試機要先開 SECS、讀碼器開時 Barcode Recipe 必填、SECS 開批事件、首盤檢查……）都沒跑。
5. **LI-6　換配方時沒有通知 RTC 視覺**（St01，要上機）。golden 換配方會經 Lot Info 的 Timer1 送 @FILE／@SITE 給 RTC；移植樹沒有，而 cSetUp.cpp、uhome.cpp 有三個呼叫點閘住在等它。
6. **X-1　背景計時器**（跨頁）。golden 的這些計時器不管視窗有沒有開都一直在跑（LotInfo、TemperFrom 開機就 Show；GroundMan、ShowBinSelect 的計時器開機就打開）；移植樹有些有本體但沒人每拍呼叫。上面 1、2、3、5 都屬於這一類，建議一起決定放在哪裡跑（第二節）。
7. **SB-1／SB-2、CK-1　計數與 Auto Clean 的按鈕**（St01）。BinSelect 的 Index 分頁可以手動啟動 Auto Clean、清清潔計數；Contact Counter Kinds 頁的 Clear Count（等級 107）按了只顯示「尚未接」。
8. **OB-1～OB-4　Observer 的保養紀錄分頁**（St01）。[B01]／[B02] 開著的機台，Precautions／Major Maintenance 分頁的鈕都在畫面上、按得下去，但**按了沒反應**；golden 關 Observer 時還會寫 PrecautionParameter.ini。

**給 Steven02 的 2 列**：LI-12（Lot Info 的 Tester TCP Show 鈕；閘住的理由已過期，可照 Q41 TI-3 的做法開 testercomm.html 的 TCP/IP 分頁）、OB-5（Observer 的 MDB 查詢分頁；cMyDB 已改 CSV，這一頁要不要做、怎麼做由 St02 定）。

---

## 二、跨頁的共通事項（先決定這幾件，後面很多列會一起解）

| 編號 | 事項 | 牽涉的列 | 白話 | 現況 | 建議 |
|---|---|---|---|---|---|
| X-1 | 背景計時器 | LI-2、LI-3、LI-4、LI-6、LI-8、SB-5、GM-2、TP-1（另：客戶專屬的 LotInfo Timer4／LotKeyInTime） | golden 視窗上的 TTimer 只要表單存在就會跑，不管看不看得見。主畫面開機時 Show 了 fTemperFrom、fLotInfo（golden main.cpp:8732、:8813），也打開了 fShowBinSelect->TimerAutoCleanCount（:10187）；fGroundMan 的 Timer1 在 dfm 就是開著的 | 移植樹網頁版只在「網頁送指令」時才跑（例：Observer 的 act=timer、ContactCT 的每秒 contactct.get），或完全沒人呼叫（LotInfo Timer2Timer D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp:4144、ShowBinSelect D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cShowBinSelect.cpp:2049、GroundMan D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fGroundMan.cpp:303 都有本體沒人呼叫） | 照既有的每拍掛勾做法（例 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp:601 的 W906_Timer2HeaterTick）在 wb_serve 主迴圈每拍呼叫。⚠ WebBridgeTags.cpp 的 PublishHandlerTags、wb_serve 主迴圈發布點目前在筆電 TO_STEVEN §1「先不要動」（網頁更新頻率那一列），加掛勾要先認領 |
| X-2 | 開別的視窗的鈕 | CK-3、SC-2、LI-12 | golden 一按就 Show／ShowModal 另一個表單 | 網頁三顆都停用或只顯示「尚未接」 | 照 St02 的頁面補件做法：window.parent.postMessage({open: background.html 視窗表的 id})，要開到某一分頁時先寫 localStorage 請求鍵（D:\HT9045\.claude\skills\ht9045-st02-workflow\references\q41-page-pattern.md） |
| X-3 | 開頁等級 | CR-L1、LT-L1（另：Observer 等級 5＝Q42 已排） | 主畫面的開頁鈕先查等級，不夠就不開 | 同一選單的 Start Mode（24）、C.Select（25）已在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:841-846 由 C++ 重查；C.Clear（26）、Sensor Latch（46）沒有 | 分工同 Q41 C-1／C-2：開窗權限表歸 Jimmy（D:\HT9045\.github\specs\page-access-policy.md、background.html MODAL_POLICY :582），C++ 重查歸頁面擁有者 |
| X-4 | 關窗時要跑的程式 | OB-1、GM-3 | golden 關視窗一定跑 FormClose | 設定頁已有關窗表（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\WindowEdgeTails.h，19 列），fObserver 不在表上；fGroundMan 是刻意不放（:53「RS232 重開」） | Observer 照同一張表加一列；GroundMan 等 GM-1／GM-2 一起決定 |

---

## 三、逐頁明細

欄位說明：「移植樹本體」的「有」＝照 golden 翻好、可以呼叫；「部分」＝翻了但有一段閘住、或翻了沒人呼叫、或只做了一部分；「沒有」＝找不到這支。「網頁有沒有送」寫頁面現在的樣子。每頁最後列出已經做好的、不列的，免得重複。

### 3.1 Data.LotInfo（golden TfLotInfo）

網頁 D:\HT9045\web\page\Data.LotInfo.html、ht9045_lotinfo_wire.js。golden D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\uLotInfo.cpp（綁定在 uLotInfo.dfm）。移植樹 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp（門面，108 支裡 59 支有本體）、WebLotInfo.cpp（WS lotinfo.op，W906_LotInfoOp :345）。檔案歸屬：forms\fLotInfo.cpp 是筆電 FW 波次的門面，St01 在檔尾加了自己的段落（S94、S117），St02 也認領過幾行（S72、S-09）——改之前照 S85「看段落不看檔名」先認領。

| 編號 | 事件（golden 處理器） | 做什麼（一句白話） | 動機台／寫檔 | 移植樹本體 | 網頁有沒有送 | 建議歸屬 | golden 出處 | 備註 |
|---|---|---|---|---|---|---|---|---|
| LI-1 | sbSECSLotStartClick（Lot Start 鈕） | 開批：檢查 Lot ID／Operator ID／Run Mode 的字元與必填、TCP 測試機要先開 SECS、讀碼器開時 Barcode Recipe 必填、送 SECS 開批事件、首盤檢查（P62），最後 SetLotStart | 寫檔（config.ini [Lot Info]、2D 清單）；不動機台，但沒開批不能 START | 部分：這支本體沒有翻（最後呼叫的 SetLotStart 有翻：forms\fLotInfo.cpp:5438）。網頁的 Lot Start 走 lot.start，伺服器 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5757-5789 只設兩個欄位就呼叫 SetLotStart | Data.LotInfo 頁沒有這顆鈕；主畫面控制鈕 D:\HT9045\web\page\Main.gbControlBtn.html（ht9045_lotstart.js）送 lot.start | St01（頁）；wb_serve.cpp 的 lot.start 那一臂是筆電寫的（c8e5e3aa），改之前先認領 | uLotInfo.cpp:7382-8416（dfm:339） | [O23] 只准刷條碼輸入 Lot ID／OP ID（edtSysLotIDMouseUp :11796、edtSysOperatorIDMouseUp :11755）網頁也沒有 |
| LI-2 | FormShow 的副作用（golden 開機就 Show 這個視窗，main.cpp:8813） | 開窗時把上次沒結批的 Lot 狀態接回來（SetLotStart(..., true)）、把 ATC 運轉旗標設成 false、依客戶排 Run Mode 選項 | 改記憶體（RunInfo.bLotStart、bStartATCRun）、讀 config.ini | 部分：本體 forms\fLotInfo.cpp:3024（1118 行）沒人呼叫；網頁只抽出分頁可見度與開窗頁（forms\fLotInfo.cpp:5975-6000 說明為什麼）；接回 Lot 那一行閘在 GATE WC-19（:4036-4047，SAFETY）；ResetLotInfo 已在開機跑（FileRW\MainBoot.cpp:260） | 開頁只問分頁可見度 | St01＋Jimmy（WC-19＝todo D-011 J7） | uLotInfo.cpp:316-1233（dfm:21）；:646、:1158 | 影響：重開程式後 golden 自動當作還在開批，移植樹要重新 Lot Start |
| LI-3 | Timer2Timer 的非顯示部分（每秒） | OCR 連線測試的結果訊息（OK／NG）；WinWay ATC 機種每秒對 4 站設溫、斷線處理 | 動溫控設備（WinWay 串列埠） | 部分：本體 forms\fLotInfo.cpp:4144（292 行）沒人每秒呼叫；分頁可見度那幾行已由 WebBridgeTags.cpp:2950 每拍算 | 背景，不需要網頁送 | 待確認（WinWay＝溫度，RULINGS_20261001 第 38 條請教 Ifor；OCR 那段歸 St01） | uLotInfo.cpp:6934-7191（dfm:14504）；:6960-6985、:7041-7085 | 只在 INSTALL_OCR 或 ATC_SYSTEM＝eWinWay 的機台；VTEST 三小時警報那段是客戶專屬 |
| LI-4 | NetATCTimeTimer（新 ATC 系統的通訊迴圈，200 ms） | 跟 ATC 連線／斷線、送設定溫度與 Run／Stop、Air machine 開關、收 ATC 警報 | 動溫控設備 | 沒有：forms\fLotInfo.h:1674 只有 Enabled 旗標；ATC\ATCInterface.cpp 有沒有另外做掉，這次沒追 | 背景 | 待確認（ATC，請 Ifor 確認） | uLotInfo.cpp:8558-9350（dfm:14522）；SetATCFormVisible :9938-9985 打開它 | 只在 ATC_SYSTEM＝eNewATCSystem 的機台 |
| LI-5 | pl_ATC_OnlineClick（ATC On／Off Line；palATC61Status 同一支） | 切 ATC 上線／下線（等級 114），改 bStartATCRun、讓 ATC 介面上線或下線 | 動溫控設備 | 沒有 | 頁面只顯示燈號（Data.LotInfo.html:171），不能按 | 待確認（同 LI-4） | uLotInfo.cpp:10239-10276（dfm:2257、:11420） | |
| LI-6 | Timer1Timer＋RTCChangeFile（RTC 換工作檔通知） | 換配方時通知 RTC 視覺：COM2 送 @FILE／@SITE，需要時送 DELETE，最後送 MODEL／InspEnd，設 bRTCChangeFileFinish | 動設備（RTC 視覺通訊） | 沒有：兩支都沒有；呼叫端 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSetUp.cpp:1083、:1093（GATE G-SU-RtcChangeFile）與 uhome.cpp:1546 都閘住在等它 | 背景 | St01（要上機；同 Q41 SU-7 的 RTC，EVENT_PORT_BATCH 的 B8 已改由 St01 做） | uLotInfo.cpp:5255-5342（dfm:14499）、:5344-5364 | 只在 REAL_TIME_CCD 機台 |
| LI-7 | btDownloadClick／btUploadClick／coLevelModeChange（Device Info 分頁） | 從 RMS 伺服器下載配方並換配方、上傳配方；Level Mode 輸入密碼切 Engineer | 寫檔（覆寫配方、寫 Selection）、換配方 | 沒有 | 頁面註明不接（Data.LotInfo.html:74） | St01（todo G-021 S105，Steven「晚點做」） | uLotInfo.cpp:2366-2877（dfm:205）、:2322-2330（dfm:214）、:9906-9936（dfm:278） | 分頁只在 [Server] Server Enable |
| LI-8 | TimerERMSTimer | ERMS：定時看資料夾、自動下載配方並換配方，寫 OK.dat／NG.dat | 寫檔、換配方 | 沒有 | 背景 | St01（同 G-021） | uLotInfo.cpp:10035-10211（dfm:14529） | 只在 IniConfig.bEnableErms |
| LI-9 | btnFtpServerClick（Server／HD／Tester／Data FTP Save 四顆）＋btSaveSetupFileClick（隱藏） | 開 FTP 視窗下載／上傳 | 寫檔（網路） | 沒有 | 頁面註明不接（Data.LotInfo.html:133） | 待確認（網路；同 Q41 CC-E13） | uLotInfo.cpp:5001-5126（dfm:1843、:1859、:1875、:1907）、:5128-5223（dfm:1891） | 分頁只在 [FTP] Enable FTP |
| LI-10 | BtnPauseClick（FTP 分頁上的 Pause） | 機台暫停（fMain->Pause） | 動機台（暫停） | 沒有（同一顆的 MouseDown／Up 有：forms\fLotInfo.cpp:1968、:1972）；fMain->Pause 已轉到 PauseFromWeb（筆電第 18 批） | 頁面沒畫這顆 | St01 | uLotInfo.cpp:14528-14531（dfm:1940） | 跟 LI-9 同一分頁 |
| LI-11 | btChangeFileClick（BarCode 分頁 Change File） | 讀碼器／CCD 重新連線、換 job 檔 | 動設備（讀碼器通訊） | 部分：forms\fLotInfo.cpp:2247 整段 #if 0（GATE WB-3：fBarCode 缺這 4 個成員） | 頁面停用（Data.LotInfo.html:302） | St01（要上機；同 Q41 BC-4 的讀碼器通訊） | uLotInfo.cpp:10213-10237（dfm:4524） | |
| LI-12 | btTesterTCPShowClick（Tester Log 分頁） | 開 Tester TCP 視窗 | 不動 | 部分：forms\fLotInfo.cpp:2004 #if 0；閘的理由「fTesterTCP 沒有門面」已過期（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fTesterTCP.h:408 有了） | 頁面停用（Data.LotInfo.html:417） | Steven02 | uLotInfo.cpp:13841-13844（dfm:11149） | 走 X-2：開 testercomm.html 的 TCP/IP 分頁（localStorage ht9045.testercomm.tab，同 Q41 TI-3）；分頁只在 TCP_IP_MODE |
| LI-13 | palSecsGemMouseDown（Lot 面板的隱藏點擊） | HonPrec 等級依序點「左左右右左左」，自動填入測試用的 Lot ID／Operator ID | 改記憶體 | 有：forms\fLotInfo.cpp:4794，沒人呼叫 | 沒有 | St01 | uLotInfo.cpp:10351-10425（dfm:323） | 低；KYEC_LEE 不准 |

已做好（6 支）：sbSECSLotEndClick（Lot End，S117／G-029，WebLotInfo.cpp:608）、pgLotinfoChange（selection.get，:391）、btnSaveClick（Selection 存檔，:418）、btClearBarcodeCountClick（:665）、btClearBarcodeListClick（:477）、spOCRCleanListClick（:511）。另外 Tester Log 內容（testerLog.get）與 27 個分頁的可見度（WebBridgeTags.cpp:2950）照 golden 顯示。

不列（29 支）：FormClose、FormDestroy；按鍵／滑鼠輸入輔助 25 支——edDeviceName（MouseDown／KeyDown／KeyPress／KeyUp）、edTemp（KeyUp／KeyPress／KeyDown）、edtSysLotID（KeyUp／KeyPress／KeyDown／MouseUp）、edtSysOperatorID（KeyUp／KeyPress／KeyDown／MouseUp）、cbRunMode（KeyPress／KeyDown／KeyUp／DropDown）、edPage（KeyPress／MouseDown）、edtLotVerify（MouseDown／KeyPress）、BtnPause（MouseDown／MouseUp）；cbbDeviceNameChange（元件隱藏）；btnAirStreamOnOffClick（golden 本體整段註解掉，什麼都不做）。這些輸入輔助大多是客戶專屬分支（Murata、AMD_M、SCC、TFME、JCET、KYEC）；標準的 [O23] 那一段見 LI-1 備註。

客戶專屬（57 支）：第四節。

### 3.2 Data.Observer（golden TfObserver）

網頁 D:\HT9045\web\page\Data.Observer.html、ht9045_observer_wire.js。golden D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cObserver.cpp（cObserver.dfm）。移植樹 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp（56 支全部有本體；St01 的檔，St02 認領過 :1351-1385 的 ELA 段）、WS observer.get（tools\wb_serve.cpp:5254 → W906_ObserverJson cObserver.cpp:7924）。

先講一個共通現象：下表 OB-2～OB-4、OB-6、OB-7、OB-9 的鈕**在網頁上看得到、按得下去，但沒有綁任何事件，按了沒反應**（Data.Observer.html 的 btnClearTime、sbPrecautionSave、sbMajorMaintenanceSave、btnSG_QueryNow、lstTimeData 等，ht9045_observer_wire.js 沒有它們的 listener）。

| 編號 | 事件（golden 處理器） | 做什麼（一句白話） | 動機台／寫檔 | 移植樹本體 | 網頁有沒有送 | 建議歸屬 | golden 出處 | 備註 |
|---|---|---|---|---|---|---|---|---|
| OB-1 | FormClose＋BtnExitClick（關 Observer） | 關窗時存 Precaution 參數、還原分頁可見；Major Maintenance 沒填完不准關 | 寫檔 D:\PrecautionRecord\system\PrecautionParameter.ini | 有：cObserver.cpp:4226、:4315，沒人呼叫 | 網頁關窗不跑（不在關窗表） | St01 | cObserver.cpp:654-674（dfm:16）、:697-706（dfm:6046） | 走 X-4 |
| OB-2 | Precautions Record 分頁 7 支：cobNoteContentsSet、sbHandlerPrecautionRecordSet／Clear、sbHandlerPrecautionFormShow、sbPrecautionSave、sbPRStartDate、sbPRFinishDate | 注意事項紀錄：加入／清除內容、記開始與完成日期、打開 Precaution 提醒視窗、存檔 | 寫檔 D:\PrecautionRecord\ | 有：cObserver.cpp:5689-5810，沒人呼叫 | 鈕在頁上但沒綁 | St01 | cObserver.cpp:4452-4533（dfm:3311-3657） | 分頁只在 [B01] |
| OB-3 | Handler Major Maintenance 分頁 9 支：保養日期／開始／結束時間、不良現象與對策的加入和清除、存檔、查詢 | 大保養紀錄 | 寫檔 D:\MajorMaintenanceRecord\ | 有：cObserver.cpp:5818-6012 | 同上 | St01 | cObserver.cpp:4535-4678（dfm:4024-4561） | 分頁只在 [B02]；Greatek 換配方時強制開這一頁（客戶專屬） |
| OB-4 | sbSearchPrecautionLogClick | 查詢注意事項的歷史紀錄 | 只讀檔 | 有：cObserver.cpp:6048 | 同上 | St01 | cObserver.cpp:4680-4753（dfm:4969） | |
| OB-5 | MDB 子分頁 4 支：BtnQueryClick、BtnSaveClick、DateTimePicker1CloseUp、lbltTotalLoaderMouseDown | 查 Event Log／Jam 圖表（有等級檢查），另存 xls／csv／bmp | 讀資料庫、另存檔 | 有：cObserver.cpp:1626、:6235、:6309、:6697 | 頁面沒有這個子分頁 | Steven02（cMyDB 已改 CSV；這一頁要不要做由 St02 定） | cObserver.cpp:2393-2784、:3177-3184（dfm:1403-1510） | 子分頁只在 CosFunction.bUseMDB（各客戶預設不同，例 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CosFunction.cpp:1630） |
| OB-6 | Time Data 子分頁：pgcMessageChange、lstTimeDataClick | 列出 D:\HT9045_Log\TimeData 的檔、點檔顯示內容 | 只讀檔 | 有：cObserver.cpp:7064、:7116 | 子分頁與清單在頁上，但切子分頁、點檔都不送 | St01 | cObserver.cpp:4766-4793（dfm:1336）、:4841-4844（dfm:1811） | TimeData 檔由 St02 的 W48 每小時寫 |
| OB-7 | SG_JamCount 子分頁：btnSG_QueryNow／Yesterday | 統計今天／昨天的 Jam 次數、寫統計檔；[N26] 開著時把昨天的上傳 FTP | 寫檔、網路 | 有：cObserver.cpp:3605、:3610 | 鈕在頁上但沒綁 | 待確認（Jam 統計＋FTP） | cObserver.cpp:5361-5369（dfm:1886、:1895）；StatisticalJamCount :5060 | |
| OB-8 | btnBackupLogYearClick | 把一整年的 EventLogTxt 複製到年資料夾 | 寫檔：先寫 D:\HT9045\system\2.bat 再執行 xcopy | 有：cObserver.cpp:7147 | 頁面按了只顯示「尚未接」（ht9045_observer_wire.js:702） | St01 | cObserver.cpp:5371-5391（dfm:1774） | ⚠ 會寫 system\ 共用資料夾 |
| OB-9 | btnClearTimeClick（Counter 分頁 Clear Time Data） | Jam 次數與累計時間歸零 | 改 LastSet（下次存 lastdata.dat 時寫入）、記 log | 有：cObserver.cpp:7196 | 鈕在頁上但沒綁 | St01 | cObserver.cpp:5393-5399（dfm:457） | |
| OB-10 | btOpenLoadLogClick（Test Information > Load Info） | 開檔選一個 load log、解析後顯示 | 只讀檔（要開檔案對話框） | 有：cObserver.cpp:6985 | 頁面標「沒接」（ht9045_observer_wire.js:681） | St01 | cObserver.cpp:3638-3681（dfm:2856） | 同 Q41 CC-E12：網頁不能開本機檔案對話框 |

已做好（15 支）：FormShow、Timer1Timer、pgcObservChange（第 0／1／3／4／6 頁）、rgRowNoClick（含 4 個 Display Form 選項）、cbbMonthChange、lstEventLogClick、btnQueryEventLogTxtClick、rgContactCountKinds／KindsForm／History／HistoryForm 四支、mtRowAMouseUp、edYieldMaxClick、edYieldMinClick、SpeedButton1Click（S113／S116）。

不列（9 支）：FormDestroy、StringGrid2／3／5DrawCell、StringGrid2／3MouseDown（清兩格顯示）、cbbTempChartChange（溫度圖，純顯示；Temperature 分頁整頁沒接）、sbScreenkeyboardClick（開 Windows 螢幕小鍵盤）、btAutoSaveClick（golden 本體註解掉）。

已知：開窗等級 5＝Q42（等 Jimmy 在政策表加第 7 列）；GPIB／TTL 版本兩格＝todo H-022 T3（St02 MR !14）。

### 3.3 Data.ContactCT（golden TfContactCT）

網頁 D:\HT9045\web\page\Data.ContactCT.html、ht9045_contactct_wire.js。golden D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cContactCT.cpp。移植樹 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cContactCT.cpp（9 支都有本體；WS contactct.get → W906_ContactCTJson）。

| 編號 | 事件（golden 處理器） | 做什麼（一句白話） | 動機台／寫檔 | 移植樹本體 | 網頁有沒有送 | 建議歸屬 | golden 出處 | 備註 |
|---|---|---|---|---|---|---|---|---|
| CK-1 | btClearCountClick（Clear Count） | 清 Contact 計數、良率監控計數、Index 計數等（等級 107） | 改計數（LastSet、ArmData），golden 之後存 lastdata.dat | 部分：cContactCT.cpp:1178（169 行），確認框、fProductionInfo、主畫面等級切換三段 #if 0 | 頁面按了只顯示「尚未接」（ht9045_contactct_wire.js:237-245） | St01 | cContactCT.cpp:944-1069（dfm:104） | 運轉中停用已照 golden 做（c35f375e）；KYEC_LEE、ASE_K 另有分支 |
| CK-2 | sgYieldDblClick（＋sgYieldMouseDown 記下點的位置） | 在格子上點兩下，清掉那一站的接觸種類計數 | 改計數 | 部分：cContactCT.cpp:946、:1169（內有 #if 0） | 沒有 | St01 | cContactCT.cpp:740-886（dfm:73）、:937-942（dfm:75） | HANA_MICRON 分支是客戶專屬 |
| CK-3 | btYieldChartClick（Yield Chart） | 開 Observer 的 Yield 圖 | 不動 | 部分：cContactCT.cpp:1351，fObserver->ShowModal 閘住 | 頁面按了只顯示「尚未接」 | St01 | cContactCT.cpp:1071-1075（dfm:120） | 走 X-2：開 observer 並切到 Yield 分頁 |

已做好（3 支）：FormShow、rgYieldTypeClick、sgYieldDrawCell（S115，每秒重取）。不列：FormClose、FormDestroy。

### 3.4 Data.SortCT（golden TfSortCT）

網頁 D:\HT9045\web\page\Data.SortCT.html、ht9045_sortct_wire.js。golden D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cSortCT.cpp。移植樹 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fSortCT.cpp、cSortCT.cpp、WebSortCT.cpp（St01）、TrayEditForm.cpp／WebTrayEdit.cpp（St02 S10）。

| 編號 | 事件（golden 處理器） | 做什麼（一句白話） | 動機台／寫檔 | 移植樹本體 | 網頁有沒有送 | 建議歸屬 | golden 出處 | 備註 |
|---|---|---|---|---|---|---|---|---|
| ST-1 | spbClearAllCountClick（ART Sort Count 分頁的 Clear All Count） | 確認後清 Loading／Sort／Tester Category／Index 計數與所有 Arm 計數，送 SECS Clear Count 事件，記 log 與 Production Data | 改計數、寫 log、SECS 事件 | 沒有 | 頁面的 ART 分頁沒有這顆鈕（Data.SortCT.html:32） | St01 | cSortCT.cpp:771-821（dfm:2124） | ART 分頁只在 ART 機台；Production Data 那一行經 cMyDB（St02） |

已做好（8 支）：FormShow、btnClearCountClick（act.sortCT.clearCount）、pnlAuto1DblClick（點兩下歸零，S65）、右鍵編盤四支 pnlLoaderMouseDown／pnlAuto1YieldMouseDown／pnlHP1MouseDown／pnlHP2MouseDown（St02 S10：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TrayEditForm.cpp:554、act.trayEdit）、Timer2Timer（WebBridgeTags.cpp:2657 每拍 UpForm）。

不列（9 支）：FormClose、lblTotalClick（重畫）、pnlAuto1CIDDblClick（小鍵盤）；pnlFix1～6 的六支 MouseDown——golden 建構子 cSortCT.cpp:127-135 把這些格子的 OnMouseDown 換成 pnlAuto1YieldMouseDown，dfm 綁的這六支執行期永遠不會跑（移植樹照這個做，TrayEditForm.cpp:458-463）。

客戶專屬：Timer1Timer（CC_ASE_M 的 Lot ID 欄；C++ 已做，S97）。

### 3.5 Data.SmartDiagnostic（golden TfSmartDiagnostic）——沒有新缺

12 支：6 支已接（smartdiag.op，D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSmartDiag.cpp:296-340）：FormButtonClick（Save／Exit／切頁）、FormShow、sb_SmarDiagnostic_CreateCyliderNameClick、sb_SmartDiagnostic_ResetRecordCountClick、SmartDiagnosticTimerTimer、sgCylinderSelectCell。6 支不列：FormCreate、FormDestroy、兩支 DrawCell、兩支小鍵盤。開頁等級 1、24 已查。

### 3.6 Data.StartCondition（golden TfStartCondition）

網頁 D:\HT9045\web\page\Data.StartCondition.html、ht9045_startcondition_c.js。golden D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cStartCondition.cpp。移植樹 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\StartCondition.cpp（C 路，處理器在 StartCondition.gen.inc，名字前綴 SC_）、cStartCondition.cpp（顯示段）。

| 編號 | 事件（golden 處理器） | 做什麼（一句白話） | 動機台／寫檔 | 移植樹本體 | 網頁有沒有送 | 建議歸屬 | golden 出處 | 備註 |
|---|---|---|---|---|---|---|---|---|
| SC-1 | Cylinder 頁：strngrdCylinderViewSelectCell＋彈出選單 7 項（共用 mniResetOnOffCountClick） | 氣缸壽命表：點格子決定選單有哪幾項；選單重設次數、設警報門檻後存檔 | 寫檔 D:\HT9045\system\MachineLife.ini | 部分：顯示段有（cStartCondition.cpp:737 UpdateCylinderScreen、:850 LoadCylinderLife、:923 DrawCell），這兩支事件沒有；開機讀檔閘在 cinitial.cpp（N1-G2e） | 頁面整頁停用並註明（ht9045_startcondition_c.js:258-262） | St01（todo G-012，BLOCKED：等 Jimmy 解 cinitial.cpp 的 #if 0） | cStartCondition.cpp:1321-1416（dfm:4394-4423）、:1452-1477（dfm:4051） | |
| SC-2 | sb_Maintenance_SmartDiagnosticFunctionClick | 開 Smart Diagnostic 視窗 | 不動 | 沒有 | 頁面停用，提示改用 Data.SmartDiagnostic 頁（ht9045_startcondition_c.js:265） | St01 | cStartCondition.cpp:1080-1084（dfm:2725） | 走 X-2，一行 postMessage |

已做好（14 支，C 路）：FormShow（SC_FormShow FileRW\StartCondition.gen.inc:692）、FormClose（關窗表 B10c）、sbSameAsHead1Click、sbClearCountClick、sbSaveClick、spbExitClick、sbHeadCondition1ClearClick、sbHeadCondition1SaveClick、pgLifeTimeChange、btnInArmA／btnOutArmA／btnArm1Aa／btnArm2Aa／btnClearAa 五組清除鈕；開頁等級 24（FileRW\_EditPage.cpp:841）。

不列（9 支）：sgContactCountSelectCell、sgHeadCondition1SelectCell（點格子跳小鍵盤，頁面自己改格子）、editContactCountAlarmMouseDown、PanelAaDblClick、edtInOutArmPickerAlmCntClick、edtInArmAClick、edtSetXYOffsetLimitClick、edtSetZOffsetLimitClick（小鍵盤）、strngrdCylinderViewDrawCell（重畫）。

客戶專屬：btnSetOffsetLimitClick、cbStartModeOnlyFTClick（第四節）。

### 3.7 Data.Builder（golden TfBuilder）——沒有新缺

13 支：12 支已接（builder.op，D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBuilder.cpp），edNewFileNameMouseDown（小鍵盤）不列。備註：建立／刪除配方後 golden 會呼叫 fMain->LookForFile() 重抓主畫面的配方清單，移植樹那支是計數替身（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp:509）——屬於主畫面，不算本頁的列。

### 3.8 Data.CounterClear（golden TfCounterClear）

網頁 D:\HT9045\web\page\Data.CounterClear.html、ht9045_counterclear_wire.js。移植樹 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cCounterClear.cpp、JsonBridge\ChanAction.cpp 檔尾（W906_CounterClearGet／Click／Exe）。

| 編號 | 事件或等級檢查 | 做什麼（一句白話） | 動機台／寫檔 | 移植樹本體 | 網頁有沒有送 | 建議歸屬 | golden 出處 | 備註 |
|---|---|---|---|---|---|---|---|---|
| CR-L1 | 開頁等級 26（主畫面 C.Clear 鈕 sbClearClick） | 等級 26 不夠就不開 Counter Clear | 不動 | 沒有：counterclear.get（JsonBridge\ChanAction.cpp:618 W906_CounterClearGet）直接跑 FormShow，不查 26；同一選單的 C.Select（25）、Start Mode（24）已在 FileRW\_EditPage.cpp:841-846 查 | 主畫面點了直接開 | St01（C++ 重查）＋Jimmy（開窗權限表） | main.cpp:27657-27665 | 走 X-3；每個清除框的權限（GetCountClrAuth）已照 golden 做 |

已做好（6 支）：cbSelectAllMouseUp、cbAlarmDataMouseUp、FormShow、spbExeClick、spbExitClick、FormClose（counterclear.get／click／exe）。

### 3.9 Data.TestCategory（golden TfTestCategory）——沒有新缺

FormShow、sgArm1DrawCell 由 tcat.\* tag 照 golden 顯示；FormClose、FormDestroy 不列。

### 3.10 Status.ShowBinSelect（golden TfShowBinSelect）

網頁 D:\HT9045\web\page\Status.ShowBinSelect.html、ht9045_showbinselect_wire.js。golden D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cShowBinSelect.cpp。移植樹 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cShowBinSelect.cpp（筆電 FW 門面＋St01 S114）、WebShowBinSelect.cpp。

| 編號 | 事件（golden 處理器） | 做什麼（一句白話） | 動機台／寫檔 | 移植樹本體 | 網頁有沒有送 | 建議歸屬 | golden 出處 | 備註 |
|---|---|---|---|---|---|---|---|---|
| SB-1 | btnAutoCleanClick（Index 分頁的 Auto Clean） | 手動啟動 Auto Clean：沒回原點、Tray arm 不在安全位、Index 不在待機位就不准；機台內有 IC 時先清料 | 動機台 | 沒有 | Index 分頁沒有這顆鈕 | St01（要上機） | cShowBinSelect.cpp:2101-2166（dfm:3024） | 跟 Q41 CL-4（Setup.Cleaning 的啟動 Auto Clean）是同一件事、不同入口，守衛不一樣；只在 [Enable Auto Clean] |
| SB-2 | btnCleanResetClick（等級 97）＋ed_AutoCleanCountClick（等級 43） | 清潔計數歸零（呼叫 fCleaning 的歸零） | 寫檔（同 Q41 CL-2） | 部分：cShowBinSelect.cpp:1094、:1009 只做到等級檢查與清畫面，真正歸零那一行閘住（fCleaning 沒有這支） | 頁面沒有 | St01（等 Q41 CL-2 的本體） | cShowBinSelect.cpp:2253-2260（dfm:3041）、:2216-2223（dfm:3070） | |
| SB-3 | UPH_StringGridDblClick | 點兩下刪一筆 UPH 紀錄、重算平均 UPH（HonPrec、停機時） | 改記憶體（RunInfo.iAvgUPH） | 部分：cShowBinSelect.cpp:850，刪除那段 #if 0（GATE B2：確認框） | UPH 分頁只顯示 | St01 | cShowBinSelect.cpp:2054-2099（dfm:1358） | |
| SB-4 | sbCopyRecipeClick（Copy Recipe） | 把目前配方資料夾整個 xcopy 到 D:\Run | 寫檔 D:\Run\ | 沒有（主畫面的 RunBatchCopyRecipe 也沒有） | 頁面沒有 | St01 | cShowBinSelect.cpp:2845-2850（dfm:3340）；main.cpp:34522-34529 | |
| SB-5 | TimerAutoCleanCountTimer | 每秒：運轉中算即時 UPH（[G10]）、Auto Clean 欄位可不可用、ART 分頁可見 | 改記憶體 | 有：cShowBinSelect.cpp:2049，只有 tests 呼叫 | 背景 | St01 | cShowBinSelect.cpp:2168-2214（dfm:6565）；main.cpp:10187 開機打開 | 走 X-1；低 |

已做好（2 支）：FormShow（tag 每拍算）、btnClearCountClick（act.showBinSelect.clearCount，WebShowBinSelect.cpp:136）。

不列（5 支）：FormDestroy、FormClose、PageControl1Change（切頁換版面）、labAuto1Click、btReturnClick（改視窗大小與位置）。另外 golden 的 ART、Fix AI、SECS Category、ContactCT Category 分頁網頁沒有畫（顯示問題，不算列）。

客戶專屬：btnICMisPlacementClick（SPIL）、btnSetInpputCntClick＋EdLoadCountClick（ASE 分頁）。

### 3.11 Status.LtcSensor（golden TfLtcSensor）

網頁 D:\HT9045\web\page\Status.LtcSensor.html。golden D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\LtcSensor.cpp。**整頁是靜態畫面**：頁面只載 theme.js、hwidgets.js，鈕看得到、按得下去，但什麼都不送；移植樹沒有 TfLtcSensor 本體，只有替身 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\acarry_shims.cpp:27-62（GetLtcSensor 一律回 0、SetLtcSensor 什麼都不做；這組替身也是生產流程在用）。

| 編號 | 事件（golden 處理器） | 做什麼（一句白話） | 動機台／寫檔 | 移植樹本體 | 網頁有沒有送 | 建議歸屬 | golden 出處 | 備註 |
|---|---|---|---|---|---|---|---|---|
| LT-1 | btSh1ServoClick／btSh2ServoClick／btSh3ServoClick | 切 In Shuttle 1／2、Out Sort Shuttle 的 Servo On／Off，並要求重新回原點（fAllMotorHome=false） | 動機台 | 沒有 | 沒有 | 待確認（馬達卡；像 Jimmy／EastSun） | LtcSensor.cpp:105-121（dfm:156、:165、:196） | |
| LT-2 | btSetLtcClick＋btGetSh1／2／3LtcClick（含 Button5、Button6） | 設定三個 shuttle 的 latch 來源；讀回 latch 資料與計數顯示 | 改馬達卡設定（設 latch）；讀 | 沒有（替身 acarry_shims.cpp:42-43） | 沒有 | 待確認 | LtcSensor.cpp:123-128（dfm:147）、:482-676（dfm:270-860） | 生產流程的疊料／飛料判斷用同一組 latch |
| LT-3 | TimerLtcSensorTimer（＋FormShow 打開它） | 開窗時每拍讀 shuttle 的 servo 狀態與 latch 輸入燈 | 只讀 | 沒有 | 沒有 | 待確認 | LtcSensor.cpp:130-171（dfm:869）、:77-103 | |
| LT-4 | BtnDellTestClick＋DellTestTimer | 測試：兩個 shuttle 來回跑，記 latch 計數存 CSV | 動機台（MotorMove）、寫 D:\DellShuttleLog\ | 沒有 | 沒有 | 待確認 | LtcSensor.cpp:683-687（dfm:175）、:826-835（dfm:875）；DellTestP :725 | 工程測試用 |
| LT-L1 | 開頁等級 46（主畫面 sbSensorLatchClick） | 等級 46 才能開 | — | 沒有 | 主畫面直接開 | 待確認（分工同 CR-L1） | main.cpp:28748-28755 | 走 X-3 |

不列（4 支）：FormClose、FormDestroy、btnCloseClick、ScrollBar1Change。

### 3.12 Status.Security（golden TfSecurity）——沒有新缺

13 支：12 支已接（S55／S64／W45：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLevelSet.cpp 權限表、WebLogin.cpp 四顆改密碼鈕、WebSecurityJam.cpp 的 Jam 區／碼／語言與匯入匯出），FormDestroy 不列。開頁等級 29 已查。

### 3.13 Status.GroundMan（golden TfGroundMan）

網頁 D:\HT9045\web\page\Status.GroundMan.html、ht9045_groundman_c.js。golden D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\GroundMan\GroundMan.cpp。移植樹 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\GroundMan.cpp（C 路讀寫段，St01）、forms\fGroundMan.cpp（筆電 FW 門面）。

| 編號 | 事件（golden 處理器） | 做什麼（一句白話） | 動機台／寫檔 | 移植樹本體 | 網頁有沒有送 | 建議歸屬 | golden 出處 | 備註 |
|---|---|---|---|---|---|---|---|---|
| GM-1 | spbStartComClick／spbStopComClick | 開／關接地監測板的 RS232 | 動設備（COM 埠） | 沒有（FileRW\GroundMan.cpp:40-44 列為 S48、不在本檔） | 頁面停用（ht9045_groundman_c.js:46-60） | 待確認（S48 硬體鈕） | GroundMan\GroundMan.cpp:247-257（dfm:175、:317） | |
| GM-2 | Timer1Timer＋comGMReceiveData（每 30 ms 監測） | 收接地板資料；阻值超限、在設定時間內連續發生達到次數，就停所有馬達、跳 WAR1609、SystemStart=false；寫接地 log | **動機台（停機）**、寫 D:\HT9045_Log\GroundManLog | 部分：Timer1Timer 在 forms\fGroundMan.cpp:303，但監測本體 DoGroundMasterMonitor 閘在 #if 0（GATE GM-2）而且沒人每拍呼叫；comGMReceiveData 沒有；開機開 RS232（golden main.cpp:11110 Init_GM_RS232）也沒接 | 背景；頁面上的阻值與燈號是設計期字樣 | 待確認（安全監測，要上機） | GroundMan\GroundMan.cpp:551-575（dfm:1878）、:303-455（dfm:1872）；停機 :1293-1297 | ⚠ USE_GROUND_MAN=1 的機台接地異常不會停機；走 X-1 |
| GM-3 | FormClose（Exit 鈕 sbtExitClick 也會觸發） | 關窗時重開 RS232（ReStart；SIGURD 湖口、北興兩廠例外） | 動設備 | 部分：forms\fGroundMan.cpp:259 有本體、ReStart 閘住；關窗表刻意不放（WindowEdgeTails.h:53） | 網頁只關視窗 | 待確認（同 GM-1） | GroundMan\GroundMan.cpp:228-245（dfm:14）、:1471-1476（dfm:1778） | 走 X-4 |

已做好（2 支）：FormShow、spbSaveClick（C 路，GroundMan.ini）。不列：edOccurrencesMouseDown、edContinuous_TimeMouseDown（小鍵盤）。btnMaintenanceMode 在 dfm 沒有 OnClick，不是處理器。

### 3.14 Status.TemperFrom（golden TfTemperFrom）

網頁 D:\HT9045\web\page\Status.TemperFrom.html。golden D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cTemperFrom.cpp。移植樹 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cTemperFrom.cpp（筆電 FW-3 的檔，只有顯示核心，8 支處理器只有 FormClose）。

| 編號 | 事件（golden 處理器） | 做什麼（一句白話） | 動機台／寫檔 | 移植樹本體 | 網頁有沒有送 | 建議歸屬 | golden 出處 | 備註 |
|---|---|---|---|---|---|---|---|---|
| TP-1 | Timer1Timer（100 ms） | 刷新所有溫度通道、把顯示字串給 SECS（RunInfo.ShowTempComp）；RTC 或 ATC 6.0 以上的機台依過溫開／關 CCD 冷卻；開了記錄時記溫度；刷新 Lot Info 的 ATC 溫度 | **動 IO**（SW[SwCCDCooling]）；記錄時寫 CSV | 沒有（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\StageThermo.cpp:108-123 寫明這支沒翻） | 背景；頁面 11 個通道全部 ---（Status.TemperFrom.html:90-93） | 待確認（溫度＋IO；溫度請教 Ifor） | cTemperFrom.cpp:1617-1682（dfm:3287）；冷卻 :1648-1670 | golden 開機就 Show 這個視窗（main.cpp:8732），所以一直在跑；走 X-1 |
| TP-2 | Panel73／Panel72／Panel71MouseDown（Handler System 隱藏入口） | 依序左鍵 73、左鍵 72、右鍵 71，HonPrec 以上且停機時，確認「Reset the hardware apparatus information?」再輸密碼，開 Handler System | 不動（開設定視窗） | 部分：C++ 開頁時照 golden 重查等級與運轉中（FileRW\_EditPage.cpp:899-914 GHandlerSys）；三下點擊順序與確認框沒做；密碼只在網頁 JS 比對（Status.TemperFrom.html:120-127） | 頁面點 Panel71 就問密碼 | St01（HSys 開頁閘是 St01 的） | cTemperFrom.cpp:1692-1769（dfm:43、:59、:75） | 密碼在網頁比對與 S55「密碼不在網頁明文」的關係，待 Steven 定（RULINGS_20261001 第 40 條：程式裡的密碼是測試用） |

客戶專屬：Button1Click、Button6Click（HONPREC_QC）。不列：FormShow（排通道與顯示；顯示問題見 TP-1 備註）、FormClose。

### 3.15 Status.TowerLight（golden TfTowerLight）

網頁 D:\HT9045\web\page\Status.TowerLight.html、ht9045_towerlight_wire.js。移植樹 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fTowerLight.cpp、WebTowerLight.cpp。

| 編號 | 事件（golden 處理器） | 做什麼（一句白話） | 動機台／寫檔 | 移植樹本體 | 網頁有沒有送 | 建議歸屬 | golden 出處 | 備註 |
|---|---|---|---|---|---|---|---|---|
| TL-1 | rgMusicTestClick（Music Test） | 試聽四段警報音樂（直接開關 SW[SwMusic1～4]） | 動 IO | 部分：forms\fTowerLight.cpp:260-271 #if 0（GATE T-3） | 頁面停用（WebTowerLight.cpp:180-182） | Jimmy（todo E-004，BLOCKED：等 Jimmy 回「試聽時要不要暫停警報音樂」） | cTowerLight.cpp:141-146（dfm:549） | |

已做好（5 支）：RGB00Click（點燈號輪替＋寫 lastdata.dat）、FormShow、FormClose（換音樂）、spbExitClick、Timer1Timer（燈號閃爍）。

### 3.16 Status.ShowMessage（golden TfShowMessage）——沒有事件可補

6 支都是顯示（sgdSpeedViewDrawCell）、開關窗，或把各視窗疊到最上層（FormMouseUp／FormClick → SetFormLayer）。注意：網頁 D:\HT9045\web\page\Status.ShowMessage.html 是靜態畫面，Index Cycle Time、Test Time 和速度表都是寫死的數字（:27-40），沒有接任何 tag——這是顯示問題，不在本表計數，但建議一起處理（移植樹 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fShowMessage.cpp 只有空殼）。

### 3.17 Status.CounterSel（golden TfCounterSel）——沒有新缺

3 支都已接（C 路，D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig_CounterSel.cpp；FormClose 由 Exit 與關窗表跑）；開頁等級 25 已查。

---

## 四、客戶專屬（Steven 20260925 已決定先跳過，只記下來）

條件欄的 CC_xxx 是客戶代碼；CosFunction.\* 是客戶功能開關。golden 行號是 906_0625。

| 頁 | golden 出處 | 內容 | 條件 |
|---|---|---|---|
| Data.LotInfo（Lot 分頁） | uLotInfo.cpp:10016、:8423、:12285、:16074、:12139、:14060、:10427、:14175、:14490、:11518／:11528、:14066／:14152 | SECS Lot Check、Lot ID 鍵盤、每 10 盤存 summary、依 Device 清單 FTP 下載、XML On Line、站號、執行外部程式、2DID Sort 檔、OEE 狀態、QA mode 數量存檔、RFID 讀 Loader | TSMC_TAINAN、KYEC_LEE、TSI（鈕隱藏）、JSCC_OS、Murata、SIGURD_ChungXing、OEE（CosFunction.bOEEFunction）、2D Sort、VTEST、SPIL、USE_RFID_READER（SJSEMI） |
| Data.LotInfo（PAT 分頁） | uLotInfo.cpp:14632、:15675、:15745、:15775、:15780、:15792-15814 | PAT 安裝、模式、上傳 Job、Lot Reset、Run Card、Real Time／Hourly／End Lot | PANTHER |
| Data.LotInfo（RTC／ATC 分頁） | uLotInfo.cpp:8418、:14697、:13846 | SCK 開始計數、RTC Auto STD、清溫度 | SCK、SPIL（鈕隱藏）、ASE_SG |
| Data.LotInfo（OCRBarCode 分頁） | uLotInfo.cpp:8430、:8458＋:8476（Timer3）、:8520 | 存讀碼 log、換 OCR 檔、OCR 重新登入（**St02 相關**：會經 fMain->SendMSG_CMD 送 EnableBarCode／Pin1 給 GPIB 程式） | CosFunction.bTrayOCR（TSMC Tray OCR；St01 頁面註明客戶專屬，但同分頁的 OCR Clean List 已做） |
| Data.LotInfo（ARMS、Event Log 分頁） | uLotInfo.cpp:10346；:10491、:10497、:16155、:10475 | ASE ARMS；ASE-CL Lot Start／End、Socket ID 鍵盤、Event Log 登入 | CosFunction.bUseARMSFunction；ASE_CL／HANA_MICRON／SJ_Semiconductor／CosFunction.bUseSocketContactCount |
| Data.LotInfo（Chamber Boost 分頁） | uLotInfo.cpp:11739、:11747、:11664 | 開始／停止 Chamber Boost 與它的計時器（溫控動作） | CosFunction.bUseChamberBoostMode |
| Data.LotInfo（RFMD 分頁） | uLotInfo.cpp:12297、:12303 | 取消 Tester Pause（**St02 相關**：Qorvo GPIB Pause 時 golden main.cpp:15803 打開這一頁）、手動 Empty Socket One Cycle（動機台） | [I41] Empty Socket Check、Qorvo |
| Data.LotInfo（FTP Automation 分頁） | uLotInfo.cpp:13719、:13754、:13777、:13784 | 密碼後顯示、FTP 上傳／下載配方、存設定 | [A32]（Sigurd） |
| Data.LotInfo（Yield Monitor、Tester Log 分頁） | uLotInfo.cpp:13790、:14054、:15883；:16080 | 手動檢查清單、標準件、離線清料模式（改 Contact 檔、Reset）；Unloader 選擇 | SIGURD_PeiXing、TERAPOWER／PTI；JSCC_OS |
| Data.LotInfo（KYEC AMR、Other Tool 分頁） | uLotInfo.cpp:16161-16232、:16186、:16487（Timer4）；:15994、:16029 | AMR 補料、上下料檢查與完成、清計數、設 SECS；Timer4 依 SECS 旗標驅動 AMR 上下料氣缸；PTI 首盤檢查 | KYEC AMR；PTI |
| Data.LotInfo（計時器） | uLotInfo.cpp:11543、:16599；Timer2Timer :7096-7113 | 刷條碼輸入時間、ATC 配方 FTP 上傳、VTEST 三小時送檢警報 WAR16123 | KYEC_LEE／AMD_M、CosFunction.bUseATCFileTransfer、VTEST |
| Data.Observer | cObserver.cpp:2786、:5401；pgcObservChange :2380-2388；main.cpp:24906-24912 | Jam 次數歸零（密碼）、SPIL Lot Info 五顆、OEE 分頁開 Production Info、Greatek 換配方時強制開大保養紀錄 | SCC／HONPREC_QC／IniConfig.bMaximFunction、SPIL、CosFunction.bOEEFunction、Greatek |
| Data.StartCondition | cStartCondition.cpp:1234、:1061、:987 | 設 Offset 上限（寫 Security_new.def）、Start Mode 只跑 FT、格子歸零確認 | CosFunction.bSetOffsetLimitToAll（ASE-CL）、SIGURD_HUKOU、AMD_M |
| Data.SortCT | cSortCT.cpp:823 | Lot ID 欄寫 config.ini（C++ 已做 S97） | ASE_M |
| Data.ContactCT | cContactCT.cpp:846、:950-984 | 清歷史資料、Clear Count 前刷條碼與權限 | HANA_MICRON、ASE_KaohSiung、KYEC_LEE |
| Status.ShowBinSelect | cShowBinSelect.cpp:2805、:2339、:2800 | 手動觸發 IC 錯置警報；Load Input Count | IniConfig.bSPILFunction；ASE 分頁（IniConfig.bASE_Report） |
| Status.TemperFrom | cTemperFrom.cpp:1950、:2016 | 溫度記錄 CSV、Alarm Simulator | HONPREC_QC（FormShow :134-137 才顯示） |

另外全樹很多輸入處理器第一段是 KYEC 的刷條碼（Barcode_Reader／InputBarcodeNumber），一律視為客戶專屬，不另列。

---

## 五、已知／已排（不算新缺）

- Data.Observer 開窗等級 5：Q42 已裁決，等 Jimmy 在 D:\HT9045\.github\specs\page-access-policy.md 加第 7 列。
- Data.Observer 的 GPIB／TTL 版本兩格：todo H-022 T3（St02，MR !14）。
- Status.TowerLight 試聽（TL-1）：todo E-004（Jimmy，BLOCKED）。
- Data.StartCondition Cylinder 頁（SC-1）：todo G-012（St01，BLOCKED）。
- Data.LotInfo RMS 下載上傳（LI-7、LI-8）：todo G-021 S105（St01，Steven「晚點做」）。
- Data.LotInfo 開機接回 Lot 狀態（LI-2 的 WC-19）：todo D-011 J7（Jimmy，SAFETY）。
- Status.GroundMan 的 RS232（GM-1～GM-3）：S48 硬體鈕，網頁 → 機台的指令通道還沒設計。
- Status.ShowBinSelect 的 SB-1／SB-2：跟 Q41 CL-4／CL-2（Setup.Cleaning）共用 fCleaning 的本體，先做 CL-2 再接 SB-2。
- Data.SortCT 右鍵編盤：St02 S10 已上 main（TrayEditForm.cpp、WebTrayEdit.cpp）。
- Data.LotInfo 藏掉目前分頁時不送指令：todo D-020（St01，已做 7cbb478d）。

---

## 六、方法、基準與限制

1. **盤點方法**：用程式掃 golden .dfm 的 On\* 綁定（加上 .cpp 裡執行期的 ->OnXxx= 指定，例 cSortCT.cpp:130-133），在 .cpp 找每支處理器的本體與行號；再到移植樹找 `TfXxx::處理器(` 的定義、判斷它在不在 `#if 0` 裡、有沒有被呼叫（C 路頁另查 FileRW 資料夾裡 .gen.inc 產生檔的 SC_／GM_ 等前綴）；網頁查頁面 HTML 與補件 JS 送什麼指令、wb_serve 怎麼分派。最後逐支讀 golden 本體寫「做什麼」，標動機台／寫檔。腳本在 C:\Users\steven\AppData\Local\Temp\claude\d---github\c8311755-ee3b-4c2b-9f5f-bc5682ac9613\scratchpad\e019\（scan_form.py、port_check.py、summarize.py、dfm_tree.py、show.py、elem.py、counts.py），可以重跑。
2. **「不列」的判準**同 Q41：純按鍵過濾、點欄位跳小鍵盤（本體只改自己）、純重畫、改視窗大小位置、golden 本體整段註解掉的。邊界案例可能有漏判。
3. **分頁是不是客戶專屬**：以 golden 的可見條件為準（CC_xxx、CosFunction.\*、VTEST／SPIL 等客戶旗標算客戶專屬；ATC_SYSTEM、REAL_TIME_CCD、INSTALL_OCR、TCP_IP_MODE、[FTP]／[Server] 這類機台設定算標準）。OCRBarCode、RFMD、FTP Automation、Yield Monitor 四個分頁照 St01 頁面上既有的分類算客戶專屬（Data.LotInfo.html 的開發說明）。
4. **沒有實測**：沒開瀏覽器、沒建置、沒跑 ctest；「按了沒反應」「沒人呼叫」是讀程式的結論。
5. **這次沒追到底的**：(a) ATC\ATCInterface.cpp 等有沒有另外驅動新 ATC 的連線（LI-4）；(b) StatisticalJamCount 的 FTP 上傳歸誰（OB-7）；(c) Status.LtcSensor、Status.TemperFrom、Status.ShowMessage 三頁歸誰（只有進版控時的畫面）；(d) HT9050 機台有沒有接地監測板、RTC、新 ATC——決定 GM-2、LI-6、LI-4 的急迫性；(e) 頁面擁有者是照 git log／todo／交接檔推的，作者欄 Steven01 與 Steven02 都是 steven@honprec.com，以 commit 訊息與 todo 為準。
6. **計數是「列」不是「處理器」**：一列可能包含好幾支（例 OB-3 九支、LT-2 四支）；要換成處理器數看摘要表「涵蓋幾支」那一欄。
7. **改檔前的認領**：forms\fLotInfo.cpp、cShowBinSelect.cpp、forms\fGroundMan.cpp、forms\fTowerLight.cpp、cTemperFrom.cpp 是筆電 FW 波次的門面，St01 與 St02 都在裡面加過段落；wb_serve.cpp 主迴圈與 WebBridgeTags.cpp 的發布點目前在筆電 TO_STEVEN §1 的「先不要動」裡（X-1）。照 S85「看區段不看檔名」先認領。
