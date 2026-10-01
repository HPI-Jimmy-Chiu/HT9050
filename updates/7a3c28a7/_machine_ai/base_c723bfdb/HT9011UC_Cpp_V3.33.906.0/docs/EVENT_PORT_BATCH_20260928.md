# BCB 畫面事件沒接到 C++：一次歸類與移植批次（20260928）

- 讀者：Steven（主管）。每一列先講操作員做什麼、畫面／機台會怎樣，程式出處放在後面幾欄。
- 依據：Steven 20260928 08:3x（ST01-M 轉述）原話：「我發現很多問題都跟bcb的事件沒有移植到c++有關」「你把這些一次歸類並移植做完, 然後再看有沒有新的問題」「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上, 如果沒有移植的, 我們直接實作」。「我們」＝St01（Steven01，資料讀寫）＋St02（Steven02，測試通訊），分工照舊：St02 留自己的列（Tester IF 5 列＋Q41 盤點裡 33 列「只在網頁上算」的），St01 拿自己的 33 列＋原本歸 Jimmy 的 12 列＋11 列「待確認」；主畫面的事件歸 St01。
- 盤點來源：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md（94 個事件列＋14 個等級列＋跨頁 C-3／C-4／CC-L1）、D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md、D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md（HEAD 6f0162cf 的內容，含 dd5f97c9 記下的 Steven 20260928 裁決）、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\D005_TFMAIN_TESTMODE_WRITERS_20260927.md、D:\HT9045\.claude\skills\ht9045-html-json\references\route-c-golden-bridge.md §3.0g～§3.0k。
- 對照版本：golden＝V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy（BCB6 原始碼，cp950）；移植樹＝D:\HT9045\HT9011UC_Cpp_V3.33.906.0（分支 v906/steven-cbridge-review6，HEAD 6f0162cf）；網頁＝D:\HT9045\web\page；St02 的頁面補件看 St02 分支 origin/v906/steven-gpib-widget（本機最後拿到的是 5e16f04f 之後那幾顆，還沒合進 main）。
- 盤點人：St01 工程線（ST01-E 派工），只讀不改；用的腳本在 C:\Users\steven\AppData\Local\Temp\claude\d---github\99781e6b-2c4e-4591-81e8-fc879566566c\scratchpad\eval6\（psearch.py 查移植樹、body.py／sig.py 讀 golden、mainscan.py 掃主畫面 main.dfm 的事件綁定、rows.py＋gen.py 產本檔）。
- 日期：2026-09-28。標「未驗證」的列是這次沒有逐支讀本體或逐檔查過的，做之前要再查一次。

---

## 一、摘要（先看這一節就好）

**這張表一共 94 列**（一列＝一個 golden 處理器，或幾個元件共用同一支、做同一件事的一組；已經做完的不算，另列在第五節）。

| 誰做 | 列數 | 接上（已經翻好，只差接線） | 直接實作（沒翻） | 接上＋補實作（翻了一部分） | 已做、待合 main |
|---|---|---|---|---|---|
| St01 | 87 | 24 | 57 | 6 | 0 |
| St02 | 7 | 0 | 5 | 0 | 2 |
| 合計 | 94 | 24 | 62 | 6 | 2 |

St02 那一欄只有 7 列，是因為 Q41 盤點給 St02 的 38 列，大部分 St02 已經在自己的分支做完（還沒合 main），這裡併成一列「Q41-St02」加一列 CC-E2；St02 還沒做的是 TS-9、OS-4、CL-5 三列，另加今天新增的 R120-F（FTP 下載成功設回 true）與 W44-1b（N10 指定時間的存檔清單）。St01 的 87 列＝Q41 盤點原本 St01 的 33 列還剩 23 列（另 10 列已做，見第五節）＋原本歸 Jimmy 的 12 列拆成 15 列（Contact 關頁那一段另列 CT-3b、Offset「自動確認」另列 OS-1b、等級 176 的 CT-L2）＋原「待確認」11 列＋等級與密碼 7 列＋主畫面 20 列（M-1～M-20）＋跨頁基礎 5 列（X-1～X-5）＋其他 6 列（C-3、T20、R107、W44-1、Q40-TF、Q40-HP）。

大小：S（半天以內）53 列、M（半天到兩天）26 列、L（兩天以上，或要上機驗）15 列。

**最常見的兩種「沒接上」**：
1. **C++ 已經照 golden 翻好、網頁沒送**：昨晚做好的「一點就跑程式」（form.event：網頁上的元件一動，就請 C++ 當下跑 golden 那一支處理程式、把連動結果回給畫面）在 Tray Assignment、Tray Form、Hot Plate、Temp_Set 這幾頁，網頁一次都沒送過——原本排給 Jimmy 在引擎加送出點，照今天的通則改由我們在頁面補件裡送（B2）。現在這幾頁要嘛存檔時才由伺服器補點（畫面不會當場變），要嘛整頁拒存（Temp_Set 換加熱模式）。
2. **golden 的處理程式根本沒翻**：大多是一顆按鈕做一件小事（歸零、帶入、重設），產生器照 golden 轉、註冊事件表、頁面送出就好（B3、B4）；少數會讓機台動（B8），要上機驗。

**建議先做**：第 1 輪同時開 B1（共用層）、B2（Setup 頁送出點）、B6（主畫面：Run Mode ▣、溫度 🌡️、Tester 🔗 三個圖示先做）；第 2 輪 B3、B4、B7；第 3 輪 B5、B8；B10 等頁面狀態陣列（X-1）；B9、B11 等 Steven 回新題 N-1、N-3。

**各批列數**：

| 批次 | 內容 | 列數 | 什麼時候 |
|---|---|---|---|
| B1 | 共用層：「一點就跑程式」（form.event）能帶滑桿／捲軸位置 | 1 | 第 1 輪 |
| B2 | Setup 頁的「送出點」：C++ 已經翻好、網頁沒送的事件 | 7 | 第 1 輪 |
| B3 | Setup 頁的 C++ 事件：產生器轉 golden、註冊事件表、頁面送出 | 16 | 第 2 輪 |
| B4 | Configuration 頁：C++ 事件補齊＋頁面送出 | 7 | 第 2 輪 |
| B5 | 重新登入（Q45 甲：#4、#6、#7）＋Supervisor 密碼 | 4 | 第 3 輪 |
| B6 | 主畫面：圖示、按鈕、子頁的事件 | 15 | 第 1～2 輪 |
| B7 | Exit 最小停機（Q44） | 1 | 第 2 輪 |
| B8 | 會讓機台動的事件（原本歸 Jimmy 的 12 列＋主畫面溫度設定、FT／RT） | 14 | 第 3 輪 |
| B9 | 主畫面跟 Jimmy 正在做的重疊：Site 格、6 顆控制鈕 | 2 | 等新題 N-1 |
| B10 | 要先有「頁面狀態陣列」的事件（開窗、關窗、切分頁的那一下） | 12 | 最後，X-1 做完之後 |
| B11 | 需要新頁面或新模組的事件 | 7 | 等新題 N-3 |
| S1 | St02 的列（照 S85 與 Q41 的分法） | 7 | St02 自己排 |
| 另案 | 頁面狀態陣列（X-1） | 1 | ST01-E 另案設計 |

---

## 二、名詞（白話）

- **一點就跑程式（form.event）**：網頁上的下拉、勾選、按鈕一動，就送一條指令請 C++ 當下跑 golden 那一支處理程式，C++ 把「哪些元件變了」回給網頁局部更新。C++ 入口在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4826 那一行、本體 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.cpp；每一頁要有「事件表」（例 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.gen.inc:2670）並註冊，網頁要有「送出點」（頁面補件 JS 送 form.event）。兩邊都有才算接上。
- **存檔補點（BeforeApply）**：網頁沒送事件就直接存檔時，C++ 存檔前照最後的值替頁面補點一次（例 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp:333）。存進檔案的值是對的，但畫面不會當場連動，而且看不出操作員點的先後。
- **關窗尾段**：golden 關掉設定視窗之後主畫面還會跑的幾行（單位換算、重載參數…）；網頁照 Steven S107-1 在「存檔成功後」跑（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp 檔尾）。
- **頁面狀態陣列（X-1）**：Steven 20260928 裁 Q51——C++ 用一個陣列記住所有頁面、每一頁現在是開還是關，跟網頁雙向同步；golden 判斷畫面開沒開的 fShow／Visible 全部改讀它；網頁與 C++ 一起改；第一優先。ST01-E 另案設計，本表只標誰依賴它。
- **產生器**：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py 讀 golden 原始碼轉出 C 路的 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\*.gen.inc；要多轉一支處理程式，是在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\<結構>.py 的 events／methods 加一行再重產，不手改產生檔（D:\HT9045\.claude\skills\ht9045-html-json\references\route-c-golden-bridge.md §3.0g-4）。
- **舊表單翻譯**：移植樹根目錄與 forms\ 底下還有一份較早（FW 戰役）照 V906 翻的表單類別（例 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cConfiguration.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uTemp_Set.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fOffSet.cpp）。網頁走的 C 路不用它們的元件，所以「那邊翻過」只算參考，接上時還是走產生器。

---

## 三、批次（依檔案分，平行的人不改同一個檔）

### B1　共用層：「一點就跑程式」（form.event）能帶滑桿／捲軸位置（第 1 輪）

- 列：X-2
- 會改的檔：
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.h
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.cpp
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp（RunPageEvent）
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\（新 ctest，CMakeLists 檔尾附加）
- 說明：做完 TA-5、CC-E8 才接得上；St02 的 Speed 滑桿（SP-1／SP-5）要不要改走伺服器由 St02 自己決定。改 _EditPage.cpp 之前跟 X-1（頁面狀態陣列）的設計對一下。
- 平行：可跟 B2、B6 同時做（不同檔）

### B2　Setup 頁的「送出點」：C++ 已經翻好、網頁沒送的事件（第 1 輪）

- 列：TA-1、TA-2、TA-3、TA-4、Q40-TF、Q40-HP、TS-1
- 會改的檔：
  - D:\HT9045\web\page\ht9045_trayassign_ev.js（新）
  - D:\HT9045\web\page\ht9045_trayform_ev.js（新）
  - D:\HT9045\web\page\ht9045_hotplate_ev.js（新）
  - D:\HT9045\web\page\ht9045_temp_set_ts1.js（新）
  - D:\HT9045\web\page\Setup.TrayAssignment.html、D:\HT9045\web\page\Setup.TrayForm.html、D:\HT9045\web\page\Setup.HotPlate.html、D:\HT9045\web\page\Setup.Temp_Set.html 各加一行 script（Setup.Temp_Set.html:348 是 St02 的那一行，不要用同一行；先在 FROM_STEVEN 寫明區段）
- 說明：只寫頁面，C++ 不動。寫法照 St02 已經在用的樣子（St02 分支 D:\HT9045\web\page\ht9045_config_q41.js 第 117-163 行：R.rawCmd('form.event')、一次一個、等回覆再送下一個、照回覆的 changed 局部套值、operable＝false 的不送、回 reload page 就重開頁）。Jimmy 的引擎 D:\HT9045\web\page\ht9045_wire_engine.js 不改。
- 平行：可跟 B1、B6 同時做；同一頁的頁面 JS 之後 B3 也要加，同一頁交給同一個人

### B3　Setup 頁的 C++ 事件：產生器轉 golden、註冊事件表、頁面送出（第 2 輪）

- 列：TA-5、TF-4、YM-2、TS-2、TS-L1、T20、CL-1、CL-2、CL-3、CL-7、CT-1、CT-L1、BC-1、BC-3、OS-1、OS-2
- 會改的檔：
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\<結構>.py → 重產 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\<結構>.gen.inc → D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\<結構>.cpp 註冊 PageEventsRegistrar（D:\HT9045\.claude\skills\ht9045-html-json\references\route-c-golden-bridge.md §3.0g-4 的標準流程）；結構：UserDefForm_File（TF-4）、TestIF_File_YieldMonitoring（YM-2）、Temperature（TS-2、TS-L1、T20）、TestIF_File_Cleaning（CL-1、CL-2、CL-3、CL-7）、DeviceForm_File（CT-1、CT-L1）、TestIF_File_BarCode（BC-1、BC-3）、Offset_File（OS-1、OS-2）、TrayForm（TA-5）
  - 頁面：TrayForm／TrayAssignment／Temp_Set 加在 B2 的新檔；其他新檔 D:\HT9045\web\page\ht9045_yield_ev.js、D:\HT9045\web\page\ht9045_cleaning_ev.js、D:\HT9045\web\page\ht9045_contact_ev.js、D:\HT9045\web\page\ht9045_barcode_ev.js、D:\HT9045\web\page\ht9045_offset_ev.js
- 說明：一個結構一條線，不同結構可以分給不同工程師同時做。TA-5 要等 B1。每支 golden 裡「伺服器做不到」或「要改寫」的行照 §4.1 用 replace／blocks，不手改 .gen.inc。
- 平行：結構之間可平行；同一結構的 .py／.gen.inc／.cpp 只能一個人

### B4　Configuration 頁：C++ 事件補齊＋頁面送出（第 2 輪）

- 列：CC-E1、CC-E3、CC-E4、CC-E6、CC-E8、W44-1、CC-L2
- 會改的檔：
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\IniConfig.py
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc（重產）
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp
  - D:\HT9045\web\page\ht9045_config_st01_ev.js（新）
  - D:\HT9045\web\page\Config.Configuration.html 加一行 script
- 說明：St02 的 D:\HT9045\web\page\ht9045_config_q41.js（CC-E2／E7／E9／E14）不動；W44-1 的存檔清單那一半是 St02 的。CC-E8 要等 B1。
- 平行：不能跟 B5 同時做（都改 IniConfig.cpp）；可跟 B3、B6 同時

### B5　重新登入（Q45 甲：#4、#6、#7）＋Supervisor 密碼（第 3 輪）

- 列：C-3、SU-L1、CC-L4、CC-E5
- 會改的檔：
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp（檔尾）
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp（M01 那一段）
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_SetUp.py → D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.cpp
  - D:\HT9045\web\page\ht9045_setup_c_wire.js
  - D:\HT9045\web\page\ht9045_iniconfig_auth_c.js（新）
- 說明：設計照 D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\q45-web-password.md §3.2（St01 約 5 人天）。CC-E5 等新題 N-2。主畫面登入顯示那一格（M-12）放 B6。
- 平行：排在 B4 之後（同一檔）

### B6　主畫面：圖示、按鈕、子頁的事件（第 1～2 輪）

- 列：M-2、M-3、M-4、M-8、M-9、M-10、M-12、M-13、M-15、M-16、M-17、M-18、M-19、M-20、R107
- 會改的檔：
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp（檔尾附加）
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp（St01 那一行同行插入：act.main.runMode、act.main.tempOnOff、燈／風扇等）
  - D:\HT9045\web\page\main.html
  - D:\HT9045\web\page\ht9045_main_st01_ev.js（新）
  - D:\HT9045\web\page\Main.CommView.html、D:\HT9045\web\page\Main.HeaterView.html、D:\HT9045\web\page\Main.MotorView.html、D:\HT9045\web\page\Main.TaskList.html、D:\HT9045\web\page\Main.Record.html、D:\HT9045\web\page\Main.MotionView.html（各加一行 script）
  - D:\HT9045\web\page\ht9045_alarm_motionview.js（M-20，不是 St01 的檔：先登記區段）
- 說明：Q47／Q48 Steven 已裁「我們做」。M-4 的 C++（兩道限制、I27）是 St02 在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp 做。
- 平行：不能跟 B7 同時改 wb_serve.cpp；其他可平行

### B7　Exit 最小停機（Q44）（第 2 輪）

- 列：M-1
- 會改的檔：
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp（W906_ServeQuitDue 那一行與停機那一行，St01 自己的行尾）
  - D:\HT9045\web\page\ht9045_main_close.js
- 說明：停馬達（StopAllMotor＋這次開機開過的 1203 軸）＋關 SwHeaterRelay（SIM 接真卡時用 1203 DO 寫入），讀回沒停就不關，給［重試停機］［強制關閉］（等級照 Exit 的 LevelSet.AccessLevel[6]）；其他 Q44 項目只列待辦。真卡讀回要在 HT9050 機台驗。⚠ 工作樹已經有另一位工程師未 commit 的 Q44 改動（AI(W906-FRW-S167)），排工前先問 ST01-E，可能已經在做。
- 平行：跟 B6 錯開 wb_serve.cpp（或同一個人做）

### B8　會讓機台動的事件（原本歸 Jimmy 的 12 列＋主畫面溫度設定、FT／RT）（第 3 輪）

- 列：SU-7、SU-8、TS-11、TS-12、CL-4、CT-3、CT-L2、OS-1b、OS-5、BC-4、AG-1、CC-E11、M-5、M-11
- 會改的檔：
  - 各結構的 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\*.py／D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\*.gen.inc／D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\*.cpp（排在 B3／B4 同結構之後）
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp（M-5、M-11，排在 B6 之後）
  - AGV 新結構（AG-1）
- 說明：每一列動手前先寫風險與影響範圍（CLAUDE.md 執行準則 7）；照 golden 的互鎖在 C++ 端重查，不信任網頁；都要上機驗。
- ⛔ 20260930 風險與影響範圍已寫：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md（14 列：低 2、中 5、高 7；建議順序 CT-L2 → OS-5 → SU-7 → M-11 → AG-1 → OS-1b → CL-4 → SU-8 → CT-3 → M-5 → CC-E11 → TS-11 → TS-12 → BC-4）。它也更正了本表幾格（TS-11／TS-12 舊表單翻了但閘著、OS-1 已做、OS-5 是 S 不是 L、M-5 的出處行號）——以那一份為準。
- 平行：跟 B3／B4／B6 同結構的要錯開

### B9　主畫面跟 Jimmy 正在做的重疊：Site 格、6 顆控制鈕（等新題 N-1）

- 列：M-6、M-7
- 會改的檔：
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp
  - D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp（Jimmy 的檔）
  - D:\HT9045\web\page\main.html
  - D:\HT9045\web\page\Main.gbControlBtn.html
  - D:\HT9045\web\page\ht9045_opbuttons.js
- 說明：Steven 回 N-1 之後才排。
- 平行：—

### B10　要先有「頁面狀態陣列」的事件（開窗、關窗、切分頁的那一下）（最後，X-1 做完之後）

- 列：YM-3、TS-10、SU-9、OS-6、OS-7、CT-3b、CC-E10、SA-2、X-3、X-4、X-5、CC-L1；BC-6（20260930 補列，B10c 後續，已做——不算進第一節的 94 列）
- 會改的檔：
  - X-1 的新檔（ST01-E 設計中）
  - 各頁的 .py／.gen.inc／.cpp
  - D:\HT9045\web\page\各頁 JS
- 說明：X-1 是 Steven 20260928 定的第一優先，但另案設計；這一批全部掛在它上面。CC-L1（Q46）排在最後。
- 平行：看 X-1 的設計再拆

### B11　需要新頁面或新模組的事件（等新題 N-3）

- 列：BS-1、OS-3、CC-E12、CC-E13、SA-1、SA-L1、M-14
- 會改的檔：
  - 視 N-3 的答案
- 說明：Q41 裁決 S158 是「新頁面先不做」；R120 的 FTP 下載頁是唯一有急迫性的（不做的話確安機台結批後要重開程式才能 START）。
- 平行：—

### S1　St02 的列（照 S85 與 Q41 的分法）（St02 自己排）

- 列：R120-F、W44-1b、TS-9、OS-4、CL-5、CC-E2、Q41-St02
- 會改的檔：
  - St02 自己的檔（St02 分支 origin/v906/steven-gpib-widget）
- 說明：大部分頁面補件 St02 已經做完、在 St02 分支上（還沒合 main）；剩 TS-9、OS-4、CL-5、R120-F、W44-1 的存檔清單。
- 平行：—

### 另案　頁面狀態陣列（X-1）（ST01-E 另案設計）

- 列：X-1
- 會改的檔：
  - —
- 說明：本表不排；B10 全部依賴它。
- 平行：—

**建議的人手配置**（同一時間最多三條線）：甲＝B1 → B4 → B5；乙＝B2 → B3 的 TrayForm／TrayAssignment／Temperature 三個結構；丙＝B6 → B7；B3 其他結構（Yield、Cleaning、Contact、BarCode、Offset）等甲或乙空出來再分。B8 每一列都要上機，等 B3／B4／B6 同結構做完再排。

---

## 四、總表（依批次排）

欄位：「移植樹現況」的「已翻未接」＝C++ 有 golden 的翻譯、但沒有被網頁叫到；「部分」＝只在存檔時補跑，或只翻了一部分；「沒翻」＝移植樹找不到。「要做什麼」照 Steven 的話分「接上」與「直接實作」。

### B1　共用層：「一點就跑程式」（form.event）能帶滑桿／捲軸位置

| 編號 | 畫面 | 白話：操作員做什麼、會發生什麼 | golden 出處 | 移植樹現況 | 誰做 | 要做什麼 | 依賴 | 大小 | 待決題號 |
|---|---|---|---|---|---|---|---|---|---|
| X-2 | （跨頁）C 路共用層 | 拖滑桿、拖捲軸時，BCB 當下就跑那個元件的事件（例：Tray Assignment 圖像模式捲軸換一組用途、Configuration 的 EP 滑桿更新標籤並記 log）。網頁送的「一點就跑程式」（form.event）沒有「位置」這個欄位，C++ 收不到新位置，只能等存檔。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1639（sbNormalTestChange）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6271-6684（tbD25_*／tbD60_* OnChange） | 部分：form.event 格式沒有 position（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.h；D:\HT9045\.claude\skills\ht9045-html-json\references\route-c-golden-bridge.md §3.0g-8 最後一條「共用層缺口」）；RunPageEvent 第 5 步不套 TTrackBar／TScrollBar（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp） | St01 | 直接實作 | 無（先做，TA-5、CC-E8 要用）；改 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp 前先跟 X-1 的設計對一下（同一檔） | M | — |

### B2　Setup 頁的「送出點」：C++ 已經翻好、網頁沒送的事件

| 編號 | 畫面 | 白話：操作員做什麼、會發生什麼 | golden 出處 | 移植樹現況 | 誰做 | 要做什麼 | 依賴 | 大小 | 待決題號 |
|---|---|---|---|---|---|---|---|---|---|
| TA-1 | Setup.TrayAssignment（D:\HT9045\web\page\Setup.TrayAssignment.html） | 點盤位旁的方向圖，每點一下換下一個擺放方向（8 種），存檔寫進配方。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:971-991（imgLoaderClick，13 個圖共用） | 已翻未接：C++ 事件表 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.gen.inc:2670 kTA_Events、註冊 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp:429（4e74e8b4）；存檔補點 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp:333（R94）；網頁 D:\HT9045\web\page\ht9045_wire_trayassignment.js 沒送 form.event | St01 | 接上 | 無 | S | R94 |
| TA-2 | Setup.TrayAssignment | 選 Loader 是 Empty 還是 Color：哪個下拉能選、自動帶值、圖像模式捲軸跳位。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1153、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1233、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1275、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1317 | 已翻未接：同 TA-1 的事件表（rgLoaderType／RGLoader／rgLoad_RT／cbLoader）；網頁沒送；沒送時存檔不重播（R96） | St01 | 接上 | 無 | S | R96 |
| TA-3 | Setup.TrayAssignment | Fix 盤上還有料時，不准切「Fix Tray Mode」（改回原值）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1166-1195 | 已翻未接：事件表有 rgFixTrayMode；存檔時也重查（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp:342-353，R95）；網頁沒送，畫面不會當場改回 | St01 | 接上 | 無 | S | R95 |
| TA-4 | Setup.TrayAssignment | Auto2 設成 bin 盤時，Auto3 那一格不能再選（當場變灰）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1197、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1772 | 已翻未接：事件表有 cbEmpty／cbColor／RGAuto1～6／rgAuto1～6_RT；網頁沒送 | St01 | 接上 | 無 | S | R96 |
| Q40-TF | Setup.TrayForm（D:\HT9045\web\page\Setup.TrayForm.html） | 「從資料庫選」下拉選一筆，自動把盤的尺寸填進 9 個欄位。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayForm.cpp:570-602（cbTrayType1Change） | 已翻未接：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\UserDefForm_File.gen.inc:891 kTF_Events、註冊 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\UserDefForm_File.cpp:40（76058840）；網頁沒送（原本排給 Jimmy 的引擎送出點） | St01 | 接上 | 無 | S | Q40（送出點改由我們做）、R78 |
| Q40-HP | Setup.HotPlate（D:\HT9045\web\page\Setup.HotPlate.html） | 「從資料庫選」下拉選一筆，自動填 7 個欄位。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cHotPlate.cpp:412-438（cbSelectHPFromDBChange） | 已翻未接：A 形狀事件表 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HotPlateForm_File.cpp:466（76058840）；網頁沒送 | St01 | 接上 | 無 | S | Q40、R78、R79 |
| TS-1 | Setup.Temp_Set（D:\HT9045\web\page\Setup.Temp_Set.html） | 換「Index Heating Mode」或勾「Temperature calibration by recipe」，當場換讀另一份溫度補償表、畫面數字跟著變。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:4232-4236、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:6333-6341 | 已翻未接：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.gen.inc:6413 kTS_Events 有這兩個（4e74e8b4）；St02 的 D:\HT9045\web\page\ht9045_temp_set_c.js（St02 分支）只送 TS-7 那 6 個、刻意不送這兩個；沒送就存檔 → 整頁拒存（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.cpp:178-210，R97） | St01 | 接上 | 無（頁面 JS 用 St01 自己的新檔，不改 St02 的檔） | S | R97、R98、R99 |

### B3　Setup 頁的 C++ 事件：產生器轉 golden、註冊事件表、頁面送出

| 編號 | 畫面 | 白話：操作員做什麼、會發生什麼 | golden 出處 | 移植樹現況 | 誰做 | 要做什麼 | 依賴 | 大小 | 待決題號 |
|---|---|---|---|---|---|---|---|---|---|
| TA-5 | Setup.TrayAssignment | 圖像模式（Configuration 開了 bTrayAssignUseGraphic）時拖捲軸，換一組 Tray 用途組合。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1639、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1674 | 部分：C++ 已翻 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.gen.inc:1985、OnChange 已掛 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp:443；但網頁送回的位置存檔時直接套、不觸發 OnChange（盤點 TA-5 列 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md:101 記的 _EditList.cpp:344；c9d3c932 之後行號已移動，未重查） | St01 | 接上 | X-2（form.event 要能帶捲軸位置） | S | — |
| TF-4 | Setup.TrayForm | 按「Bin Box Reset」：已裝數歸零，並清掉 Fix3 盤的格子狀態。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayForm.cpp:729-734（btnBinBoxResetClick：LastSet.iBinBoxCount=0、MOT[MManualTray3].ClearTray） | 沒翻：移植樹全樹 0 筆（20260928 用 scratchpad\eval6\psearch.py 查） | St01 | 直接實作 | 無 | S | — |
| YM-2 | Setup.YieldMonitoring（D:\HT9045\web\page\Setup.YieldMonitoring.html） | 按「Reset Interval」：清掉自適應良率的基準值，並刷新主畫面良率監控。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uYieldMonitoring.cpp:6058-6062 | 沒翻（C 路產生檔沒有；fLotInfo->RefreshYieldMonitor 在移植樹有沒有本體未驗證） | St01 | 直接實作 | 無 | S | — |
| TS-2 | Setup.Temp_Set | ATC Active 與 ATC 7.0 二選一；取消「參考溫度感測器」時，旁邊的 TC2 offset 勾選要藏起來。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:5407-5451、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:7073-7084 | 部分：只在存檔時重播（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.cpp:213-223），沒進 kTS_Events，畫面不即時 | St01 | 接上 | 無 | S | — |
| TS-L1 | Setup.Temp_Set | 等級 17 不夠時，點 Arm offset 欄位不開小鍵盤。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:5503（edArm1OffsetMouseDown） | 部分：開頁已依等級 17 停用多數欄位；這支沒轉（待驗證是否已被開頁停用完全蓋掉） | St01 | 接上 | 無 | S | — |
| T20 | Setup.Temp_Set（主畫面 Temp. 鈕的尾段） | 溫度頁存完後，加熱模式是 Hot 時讓溫控任務重跑一次，把新設定值送到加熱器。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28381-28383（fHeaterOK=false; bHeatOKBellowError=false; iThermoTask=1;） | 已翻未接：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.cpp:103-107 在 #if 0（GATE W906-FRW-S88-THERMO），三個符號都在（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\D005_TFMAIN_TESTMODE_WRITERS_20260927.md §2.1 T20） | St01 | 接上 | golden 加熱執行緒要先接上（RULINGS_20260927 第 16 條，筆電做）；上機要有人在機台旁 | S | todo D-005（D5-Q5） |
| CL-1 | Setup.Cleaning（D:\HT9045\web\page\Setup.Cleaning.html） | 按「Include」：用 Loader Tray 的起點／間距／數量填清潔 Tray 欄位。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp:2269-2283 | 沒翻：列在未移植清單 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_Cleaning.py:70；網頁停用（D:\HT9045\web\page\ht9045_cleaning_c.js:22） | St01 | 直接實作 | 無 | S | — |
| CL-2 | Setup.Cleaning | 按「Reset Clean Count」：清潔次數歸零並存檔、解除清潔計數警報、送 SECS 事件。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp:2116-2138 | 沒翻（C 路）；舊的表單翻譯 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fCleaning.cpp:168 有一份，但不是 C 路的替身，不能直接叫 | St01 | 直接實作 | 無 | M | — |
| CL-3 | Setup.Cleaning | 按 AI Clean 的「Reset Interval」：間隔回初始並直接寫配方。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp:2842-2847 | 沒翻（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_Cleaning.py:70 未移植清單） | St01 | 直接實作 | 無 | S | — |
| CL-7 | Setup.Cleaning | 改清潔數量或力量時，旁邊的數字當場夾值、重算 N／Gf。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp:2140（XCT1Change 等） | 部分：已翻（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_Cleaning.gen.inc:48 CL_XCT1Change），只在存檔時重播（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_Cleaning.cpp:33-40） | St01 | 接上 | 無 | S | — |
| CT-1 | Setup.Contact（D:\HT9045\web\page\Setup.Contact.html） | 換 Kit 直徑、Contact 模式等選項後，力量當場重算、該出現的欄位出現。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:15509（rgKitDiameterClick）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:14084（cbContactModeChange）等 | 部分：已翻（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.gen.inc:3664、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.gen.inc:3815），只在存檔時重算（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp:120-135） | St01 | 接上 | 無 | M | — |
| CT-L1 | Setup.Contact | 按 Contact 頁的「Sensor Adj.」面板開 Shuttle Sensor Utility，要等級 109。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:14171-14173（pnlSensorAdjClick → Insufficient(109)） | 沒有：St02 的 D:\HT9045\web\page\ht9045_contact_q41.js（St02 分支）刻意留著等這一列；HW.MyCCLinkSensor 頁沒有 C++ 入口可以重查 | St01 | 直接實作 | 無 | S | — |
| BC-1 | Setup.BarCode（D:\HT9045\web\page\Setup.BarCode.html） | 勾「多 2DID」才顯示那個分頁；換類型時 4 個 site 下拉重建。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp:8825、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp:8861 | 部分：已翻，只在存檔時重播（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_BarCode.cpp:59-76）；網頁不即時（D:\HT9045\web\page\ht9045_barcode_c.js:17） | St01 | 接上 | 無 | S | — |
| BC-3 | Setup.BarCode | 按「Reset BarCode Count」：讀碼計數（4×8 格）歸零。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp:2424-2433 | 沒翻（TfBarCode:: 在移植樹 0 筆） | St01 | 直接實作 | 無 | S | —（原「待確認」） |
| OS-1 | Setup.OffSet（D:\HT9045\web\page\Setup.OffSet.html） | 按微調 offset 的上／下／左／右鈕：每按一次調 0.1 mm 並立刻存檔。（勾「自動確認」時讓機台跑一次，見 OS-1b） | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp:2892-2903（sb_AutoOffsetUpClick 等 4 支） | 沒翻：舊表單翻譯只有宣告 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fOffSet.h:386（GATE O-5） | St01 | 直接實作 | 無 | S | — |
| OS-2 | Setup.OffSet | 改 Index socket offset 時記一個「改過」旗標（iIndexChange=2）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp:3263-3266 | 舊表單翻譯有 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fOffSet.cpp:311，C 路沒有；golden 全樹只有寫、沒有人讀這個旗標（20260928 grep） | St01 | 接上 | 無 | S | —（原「待確認」） |

### B4　Configuration 頁：C++ 事件補齊＋頁面送出

| 編號 | 畫面 | 白話：操作員做什麼、會發生什麼 | golden 出處 | 移植樹現況 | 誰做 | 要做什麼 | 依賴 | 大小 | 待決題號 |
|---|---|---|---|---|---|---|---|---|---|
| CC-E1 | Config.Configuration（D:\HT9045\web\page\Config.Configuration.html） | 按 [D47]「Reset」：Socket 已測次數歸零。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6062 | 舊表單翻譯有 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cConfiguration.cpp:6624（btD47Click），C 路事件表 kIC_Events（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc:9606）沒有 | St01 | 接上 | 無 | S | — |
| CC-E3 | Config.Configuration | 按「Clear」：依時間統計的 Jam 率計數歸零。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6691 | 舊表單翻譯有 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cConfiguration.cpp:6614，C 路沒有 | St01 | 接上 | 無 | S | — |
| CC-E4 | Config.Configuration | 按「Resume」：手動解除伺服器鎖機，跳「手動恢復機台動作」。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6257-6261（bLockByServer=false） | 沒翻（C 路） | St01 | 直接實作 | 無 | S | —（原「待確認」） |
| CC-E6 | Config.Configuration | 常溫模式下機台內還有 IC 時，不准切 [A09]，跳「請先 One Cycle」（加熱模式跳「請先 Clean out」）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6453-6480 | 沒翻：舊表單只有宣告註解（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fConfiguration.h:2001）；要讀機台有沒有料，只能在 C++ 判斷，存檔時也要重查（同 R95 的做法） | St01 | 直接實作 | 無 | S | — |
| CC-E8 | Config.Configuration | 拖 [D25]／[D60] 的 EP 滑桿（8 條）：標籤當場更新，並記一筆 EP 修改 log。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6271-6684 | 部分：C 路已翻（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc 9371 行起），拖動不觸發 | St01 | 接上 | X-2 | S | — |
| W44-1 | Config.Configuration | [N10-3-1] 選「指定時間」上傳生產資料時，畫面挑幾點就每天幾點上傳（例：挑 07:30 → 每天 07:30）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm:15549（dtpN10_3_1_SpecifiedTime）；舊版只宣告不存（golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Config.h:1041） | 沒有：網頁有這個時間框（D:\HT9045\web\page\Config.Configuration.html，id dtpN10_3_1_SpecifiedTime，值是 DFM 預設），C 路 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc 0 筆 ⇒ 不送、不存 | St01 | 直接實作 | St02 同時把這一格登記進存檔清單（W44-1 現況：St02 認領 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cConfiguration.cpp 那幾行） | S | W44-1＝A（Steven 20260928） |
| CC-L2 | Config.Configuration | 勾 [C12]（PE 模式）要輸入寫死的廠商密碼；[A27]、[N07-5] 同樣（CC-L3）、[A32_1] 雙擊圖片輸 SG_PW.ini 密碼（CC-L5）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6775、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6824、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6860、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7274 | 已擋：改了就整次拒存（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:139-233，a8eca460）；Q45 裁 C＝維持改不了。只剩頁面把這幾格標成「要廠商密碼，網頁版不提供」 | St01 | 直接實作 | 無 | S | Q45（#1～#3、#5＝C）、R73 |

### B5　重新登入（Q45 甲：#4、#6、#7）＋Supervisor 密碼

| 編號 | 畫面 | 白話：操作員做什麼、會發生什麼 | golden 出處 | 移植樹現況 | 誰做 | 要做什麼 | 依賴 | 大小 | 待決題號 |
|---|---|---|---|---|---|---|---|---|---|
| C-3 | （跨頁）SetUp、Configuration | 要改某些設定時，先重新登入到夠高的等級（BCB 跳登入框）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6482-6529、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:4325-4362（兩支 DoPassword） | 已做（20260930，B5；C++ 699dc06d，ST01-E commit）：重新登入核心 W906_Reauth 在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp 檔尾（St02 的 1～1419 行沒動），宣告 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebReauth.h；答案跟著 editlist.save 的 reauth（跟 widgets 並列）送，D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp 一解析就拿出來清掉；ctest WebLogin_Reauth 117 項過。dialog.auth（告警框）仍回「not wired」，不在 B5 | St01 | 直接實作 | 無 | M | Q45（#4、#6、#7＝A；子題 1A 2A 3A 4A 5A 6A 7B 8A 9A 10A） |
| SU-L1 | Setup.SetUp（D:\HT9045\web\page\Setup.SetUp.html） | 裝了 RTC 的機台，要關掉 Real Time CCD 或 OCR，必須重新登入到權限表第 37 格的等級。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:3573-3606 → golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:4325 | 已做（20260930，B5）：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_SetUp.py 的 SU_DoPassword 改呼叫 W906_ReauthSetupDoPassword（重產 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc 只動 1 行）；沒裝 RTC 照舊直接過，有裝＋有帶帳密照 golden 比對，沒帶照舊視同密碼錯；開頁 extra.auth（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.cpp）；頁面 D:\HT9045\web\page\ht9045_setup_c_wire.js 按 Save 時跳登入小鍵盤（還沒 commit，等 ST01-E） | St01 | 接上 | C-3 | M | Q45 #6、#7＝A；R75 |
| CC-L4 | Config.Configuration | 改 [M01] 監控功能 16 格，要重新登入到權限表第 92 格的等級，問完照 BCB 登出成 Operator。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6531-6544 → golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6482 | 已做（20260930，B5）：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp 有帶帳密就不拒存，套值之後、golden FormClose 之前跑 golden DoPassword（檔尾 IC_ReauthM01），錯了改回改過的 M01 格子，有密碼本時一律登出成 Operator（Q45-3＝A：開了 [A01_2] 的機台照 golden 不存）；沒帶照舊整次拒存；#1～#3 仍拒存，說明改成「這一項要廠商密碼，網頁版不提供，請到 BCB 版機台改」；頁面 D:\HT9045\web\page\ht9045_iniconfig_auth_c.js（新，還沒 commit） | St01 | 接上 | C-3；B4 做完再動 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp（同一檔） | M | Q45 #4＝A、Q45-2、Q45-3 |
| CC-E5 | Config.Configuration | 按「設定 Supervisor 密碼」：跳登入框輸入新密碼，寫進 lastdata.dat 的 Supervisor 欄。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6052-6060（BitBtn1Click） | 查過，照 golden 不做寫入（20260930，B5）：golden BitBtn1（Caption「Set Vender Password」）在 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm:817 是 Visible = False（V899 同），V912 全樹沒有任何程式把它設成看得見 ⇒ 操作員點不到 BitBtn1Click。網頁原本畫成看得見、點了沒反應 → D:\HT9045\web\page\ht9045_iniconfig_auth_c.js 照 DFM 藏起來；C++ 不加寫入路徑 | St01 | 直接實作 | 新題 N-2 要先回；C-3 | S | 新題 N-2 |

### B6　主畫面：圖示、按鈕、子頁的事件

| 編號 | 畫面 | 白話：操作員做什麼、會發生什麼 | golden 出處 | 移植樹現況 | 誰做 | 要做什麼 | 依賴 | 大小 | 待決題號 |
|---|---|---|---|---|---|---|---|---|---|
| M-2 | 主畫面（D:\HT9045\web\page\main.html） | 點 Run Mode 的 ▣ 圖示：Real → Dummy → Tray Only 循環切換並存檔（機台內有 IC 不給切，MES1646）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29796-29871（imgRunModeClick → RunICModeChange） | 沒翻：全樹只有 JCET 那一處客戶專屬註記（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebRecipeChange.cpp:541）；網頁 D:\HT9045\web\page\main.html:190 的 ▣ 只是字 | St01 | 直接實作 | 無（Jimmy 的 ShowTestHeadComp 還是空殼，照 golden 呼叫即可） | M | Q47＝B（Steven 20260928）；D-005 T15 |
| M-3 | 主畫面 | 點溫度 🌡️ 圖示：Hot ↔ Ambient 切換溫控模式並存檔（等級 7）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:22392-22449（Panel42Click → ChangeTempMode(10)） | 沒翻入口：本體 ChangeTempMode 在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\MainTempMode.cpp（Jimmy 的檔，不動）；網頁 D:\HT9045\web\page\main.html:176 的 🌡️ 只是字 | St01 | 直接實作 | 無 | M | Q48＝B；D-005 T13 |
| M-4 | 主畫面 | 點 Tester 🔗 圖示：在連線／離線之間切換。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29732-29794（imgTesterClick → ChangeTesterConnect(10)） | 已翻未接：C++ 有 act.main.testerConnect（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\actions\MainTesterConnect.cpp:41，St02）；網頁 D:\HT9045\web\page\main.html:185 的 🔗 沒有 id、沒有 click（舊的 D:\HT9045\web\page\main-control.js:139-141 沒有任何頁載入） | St01 | 接上 | C++ 的兩道限制（W53＝A）與 I27 手動分類（W55）由 St02 在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp 做 | S | B6（交接）、W53、W55、todo H-016 |
| M-8 | 主畫面 | 按 Light 鈕：開／關 CCD 燈（RTC 機台運轉中只准關）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:26780-26811（spbLightClick → LightOn） | 沒翻：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp:237 LightOn 是空殼；網頁 D:\HT9045\web\page\main.html:217 只讀 tag light.off | St01 | 直接實作 | 無 | S | — |
| M-9 | 主畫面 | 按 FAN 鈕：大風扇開／關切換，字變 FAN ON／FAN OFF。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:26763-26770（spbFanClick：LastSet.bBigFan 反相） | 沒翻：欄位其實在（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\LastSet.h:199），D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp:304-305 註解說「不存在」已過期；網頁 D:\HT9045\web\page\main.html:218 顯示 --- | St01 | 直接實作 | 無 | S | — |
| M-10 | 主畫面 | 按 ⬅️：收合／展開右邊的功能狀態清單。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:8689-8701（btnViewClick） | 沒有：D:\HT9045\web\page\main.html:240 沒綁（純畫面） | St01 | 直接實作 | 無 | S | — |
| M-12 | 主畫面 | 別的頁（SetUp、Configuration 重新登入）改了登入狀態時，主畫面的使用者名字／Login 鈕要跟著變。 | —（網頁才有的問題；golden 同一個程式只有一個登入狀態） | 沒有：設計在 D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\q45-web-password.md §3.2.4（原列 Jimmy 的 J1） | St01 | 直接實作 | C-3 | S | Q45 |
| M-13 | 主畫面／Motion View（D:\HT9045\web\page\Main.MotionView.html） | Motion View 依機型畫出每個托盤、每一格有沒有料。 | —（St02 的 producer 照 golden 送 34 個托盤） | 沒有：St02 的 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\ChanMvTrays.h（St02 分支）已推、還沒呼叫端；網頁讀 tag 這一半歸 St01 | St01 | 直接實作 | St02 分支合 main＋D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp 兩行（St02 等筆電同意） | S | W54＝A |
| M-15 | 主畫面／Comm View（D:\HT9045\web\page\Main.CommView.html） | 按 Read Z1／Z2、Set Z1／Z2：讀／寫 Index 扭力設定（寫完要求重新回原點）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:22635-22658（btnSetZ1Click／btnReadZ1Click 等） | 沒翻：COM2 的 ReadIndexTorqueSetting 在移植樹是空殼（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\atester_32Site.cpp:373）；其餘 5 顆（btnRecordSHT 本體是空的、btnSaveIndexPosLog 只有 SPIL／QC）照 golden 標客戶專屬 | St01 | 直接實作 | 無 | M | — |
| M-16 | 主畫面／Heater View（D:\HT9045\web\page\Main.HeaterView.html） | 點 HP2 面板（後門）：關掉「開批自動上線」IniConfig.bStartProductOnLine。rgHotplateShowMessage 的本體在 golden 整段註解掉，不用做。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:31875-31878（palHP2ViewClick）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:35150（本體全註解） | 沒翻 | St01 | 直接實作 | 無 | S | — |
| M-17 | 主畫面／Motor View（D:\HT9045\web\page\Main.MotorView.html） | 勾「Check Encoder Every Time」：有伺服警報輸入的馬達每次都檢查編碼器。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29548-29560 | 沒翻 | St01 | 直接實作 | 無 | S | — |
| M-18 | 主畫面／Task List（D:\HT9045\web\page\Main.TaskList.html） | 雙擊 Task List 表格一列：golden StringGrid2DblClick（清／切換那一列的顯示）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:25121-25128 | 已翻未接（待驗證）：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp 有 StringGrid2DblClick；D:\HT9045\web\page\Main.TaskList.html 沒有任何 JS 綁它 | St01 | 接上 | 無 | S | — |
| M-19 | 主畫面／Record（D:\HT9045\web\page\Main.Record.html） | 按「Save Log」：把記錄存檔。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:31310（btSavelogClick） | 已翻未接（待驗證）：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp 有 btSavelog 那一段（R30：不算生產資料）；頁面沒綁 | St01 | 接上 | 無 | S | — |
| M-20 | 警報視窗（D:\HT9045\web\page\Alert.Note.html） | 警報視窗的長說明語言要跟主畫面的語言選單走（例：主畫面選「繁體中文」，警報說明就是中文）。 | —（網頁版的語言設定；golden 由 IniConfig.iUserLanguage 決定） | 沒有：D:\HT9045\web\page\ht9045_alarm_motionview.js:261-266 langOf() 讀自己的 localStorage 鍵 ht9045-alarm-lang；主畫面選單在 D:\HT9045\web\page\main.html:47、D:\HT9045\web\page\main.html:538-568 | St01 | 直接實作 | 這支檔不是 St01 的：改之前在 FROM_STEVEN 登記區段（W42 其他部分是 St02 的 i18n.js） | S | W42-c（Steven 20260928：全部跟 main 畫面統一選） |
| R107 | Setup.HotPlate（關窗尾段） | HotPlate 存完重讀 Auto Clean 資料時，遇到不支援的組合 BCB 會跳警告（例「The site Y-pitch can not use arm 2 for auto clean!!」）；網頁現在看不到。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28424 → golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp:457 | 部分：尾段有跑（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp:787），訊息被吞掉 | St01 | 直接實作 | 無 | S | R107（照通則改做 B） |

### B7　Exit 最小停機（Q44）

| 編號 | 畫面 | 白話：操作員做什麼、會發生什麼 | golden 出處 | 移植樹現況 | 誰做 | 要做什麼 | 依賴 | 大小 | 待決題號 |
|---|---|---|---|---|---|---|---|---|---|
| M-1 | 主畫面 Exit（D:\HT9045\web\page\ht9045_main_close.js） | 按 Exit 關程式：先停馬達、關加熱器繼電器，讀回確認真的停了才關；停不了給「重試停機」和「強制關閉」（等級照 Exit）。其他 BCB 沒移植的停機項目列待辦。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:11852-12478（FormClose 停機順序）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29051-29131（sbCloseProgramClick） | 部分：停機在離開主迴圈之後才做、結果只印主控台、不管有沒有停都結束（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp W906_ServeQuitDue 那一行、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:1082-1096） | St01 | 直接實作 | 無（1203 卡的讀回要在 HT9050 機台驗）。⚠ 20260928 10:0x 看到共用工作樹 D:\HT9045 已經有另一位工程師未 commit 的 Q44 改動（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp 多 709 行、標 AI(W906-FRW-S167)，另有 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp、D:\HT9045\web\page\ht9045_main_close.js）⇒ 這一列可能已在做，排工前先問 ST01-E | M | Q44（Steven 20260928：最小停機＝停馬達＋關 SwHeaterRelay） |

### B8　會讓機台動的事件（原本歸 Jimmy 的 12 列＋主畫面溫度設定、FT／RT）

| 編號 | 畫面 | 白話：操作員做什麼、會發生什麼 | golden 出處 | 移植樹現況 | 誰做 | 要做什麼 | 依賴 | 大小 | 待決題號 |
|---|---|---|---|---|---|---|---|---|---|
| SU-7 | Setup.SetUp | 勾／取消 RTC（即時 CCD）6 個勾選框：關 RTC 要 Index 兩軸在安全位並送 vision 結束指令；開 RTC 要先 Clean-Out。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:4364（cbEnableRealTimeCCDClick） | 已做判斷＋改回＋存檔重查（20260930，B8 第 2 輪，待 ST01-E commit）：golden cbEnableRealTimeCCDClick（:4364-4399，6 格共用、只看 cbEnableRealTimeCCD）照產生器轉成 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc 的 SU_cbEnableRealTimeCCDClick（設定 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_SetUp.py 的 _SU7_*）：[D55] 擋關、RTC 在跑時 Index Z 不在安全位就勾回去、RTC 關著時機台有料（真的 fMain->CheckCanChangeRealDummy，D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cMainStatus.cpp:323，不是 Automation\auto9045.cpp 的「先回 true」替身）就取消。改回勾選照 VCL 再進一次處理器（P-8）；golden 在 [D55] 開＋RTC 關＋有料時會無限遞迴，移植樹停在上限、勾選留在機台目前的 RTC 狀態。6 格送 form.event（頁面 D:\HT9045\web\page\ht9045_setup_c_wire.js (8)）；存檔前重查（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.cpp 檔尾 FileRW_Setup_B8Su7BeforeApply）⇒ 網頁存檔不再能繞過。**沒做**：送 vision rtInspEnd（RTC 視覺序列埠沒移植，P-4；ack.todo 說明）。ctest B8_Su7_RtcClick 30／30 | St01 | 直接實作 | 無；牽涉 COM2 vision，要上機驗 | L | —（原歸 Jimmy） |
| SU-8 | Setup.SetUp | 按「Auto Shuttle Pitch」：主畫面的 Timer9 讓機台去量 Shuttle pitch。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:4765（btAutoShuttlePitchClick） | 沒翻 | St01 | 直接實作 | 無；機台會動，要上機驗 | M | —（原歸 Jimmy） |
| TS-11 | Setup.Temp_Set | 按「Safe Test ATC」：送 ATC 自我測試。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:5623（sbSafeTestATCClick） | 沒翻（舊表單翻譯 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uTemp_Set.cpp 也沒有這支） | St01 | 直接實作 | 無；ATC 通訊，要上機驗 | M | —（原歸 Jimmy） |
| TS-12 | Setup.Temp_Set | 手動除霜開始／結束（含「全部使用」兩顆）與除霜計時器。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:6844、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:6885、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:7051、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:7066、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:6931 | 沒翻（未逐支驗證） | St01 | 直接實作 | 無；三溫機（Tri_Temp）才有，要上機驗 | M | —（原歸 Jimmy） |
| CL-4 | Setup.Cleaning | 按「Start Auto Clean」：讓機台去做一次清潔。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp:2901（btnStartAutoCleanClick） | 沒翻 | St01 | 直接實作 | 無；機台會動 | M | —（原歸 Jimmy） |
| CT-3 | Setup.Contact | Contact 模式的機台操作（約 17 支）：啟動／暫停、T.Start／Step、One Cycle、切模式、Index Jog 上／下、OTD 氣缸、EP 讀值計時器、Auto Z Teach、編輯 Tray、RTC Auto Tuning。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:14072、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:14079、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:2280、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:2285、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:17152、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:15555、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:17241、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:17293、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:15395、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:15420、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:15445、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:17177、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:18721、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:17144、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:20658 | 沒翻（TfContact:: 這幾支在移植樹 0 筆） | St01 | 直接實作 | 無；機台會動、有互鎖（政策表第 4 列 Contact 互斥），要上機驗；關頁那一段（FormClose :1842）歸 B10 | L | —（原歸 Jimmy） |
| CT-L2 | Setup.Contact | RTC Auto Tuning 選項要等級 176 才顯示。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:17177（TimerEPTimer 內） | 已做（20260930，B8 第 1 輪，待 ST01-E commit）：golden TimerEPTimer（:17177-17232）照產生器轉成 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.gen.inc 的 DF_TimerEPTimer（設定 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\DeviceForm_File.py 的 _EP_REPLACE／_EP_BLOCKS）；顯示條件照 golden 四項（!bCCDDummyRum、D74、等級 176、bRTCVerSupportAutoTurnning），最後一項 golden 只有 rs232.cpp:119 設 false、沒人設 true ⇒ 計時器有跑的機台一拍之後一律隱藏（照翻）；那個成員放在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp 檔尾（TCOM2Shim 沒有，沒改 atester_shims.h）。開頁（golden FormShow 之後）與每個 form.event 之後補一拍（FileRW_Contact_TimerEPTick，TimerEP->Enabled 才跑）。EP 讀值與 KYEC ATC 倒數字不在這一列（留在 #if 0，CT-3）。頁面不用改（隱藏由引擎套 proxies）。ctest B8_CtL2_TimerEP 31／31 | St01 | 直接實作 | 跟 CT-3 的計時器一起 | S | —（原歸 Jimmy） |
| OS-1b | Setup.OffSet | 微調 offset 時勾了「自動確認」：調完讓機台跑一次（fMain->Start）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp:2899-2902 | 沒翻 | St01 | 直接實作 | OS-1；START 入口是 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp（Jimmy 的戰役），呼叫前要跟 Jimmy 對 | S | —（原歸 Jimmy） |
| OS-5 | Setup.OffSet | A30 Setup Teach：12 顆排序鈕與教導流程計時器。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp:3211（btnSortAuto1Click 等 12 顆）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp:3104（TimerSetupTeachTimer） | 已做（20260930，B8 第 1 輪，待 ST01-E commit）：golden btnSortAuto1Click（:3211-3221，12 顆共用）照產生器轉成 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Offset_File.gen.inc 的 OS_btnSortAuto1Click（設定 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\Offset_File.py 的 _OS5_EVENTS，DFM Tag 0～11 一起帶）：A30＋離線＋需要 Setup Teach ⇒ iSortUnloadT6＝鈕的 Tag，按下當下機台不動（運轉中 Out Arm 放料才用）。form.event 走 Offset 的別名頁 Setup.OffSet，不經微調鈕的「先選部位」（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Offset_File.cpp 檔尾 OS_EvSort）；golden 方法沒提到的 5 顆補 DFM 父層 grpOutArm456（點不點得到照 golden 畫面）。計時器 TimerSetupTeachTimer 在 V912 是空的（:3104-3209 除開頭一行全註解），不轉。頁面 D:\HT9045\web\page\ht9045_offset_ev.js 接 12 顆、伺服器說看得見時打開 grpSetupTeach。⚠ 運轉中網頁送不進來（既有守衛），golden 運轉中按得到——交 ST01-E。ctest B8_Os5_SortButtons 22／22 | St01 | 接上＋補實作 | 無；機台會動 | L | —（原歸 Jimmy） |
| BC-4 | Setup.BarCode | 讀碼器／CCD 通訊（約 35 支）：手動連線、觸發、收資料、重開 COM、存影像。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp 1782 行起（Barcode_1～4 收資料、Reader On、Shuttle CCD 連線／觸發等） | 沒翻（未逐支驗證） | St01 | 直接實作 | 無；設備通訊，要上機驗 | L | —（原歸 Jimmy） |
| AG-1 | Setup.AGV（D:\HT9045\web\page\Setup.AGV.html） | E84 AGV 設定頁：存檔、Initial Load／Unload、計時器、開頁讀檔。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\AGV.cpp:1184、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\AGV.cpp:1316、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\AGV.cpp:1324、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\AGV.cpp:1632、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\AGV.cpp:1304 | 已做讀寫（20260930，B8 第 2 輪 patch A，待 ST01-E commit；**Steven 請看**）：golden TfAGV 的 FormShow／ReadFile／DoIniDataToForm／spbSaveClick（:1304／:1227／:1264／:1184）照產生器轉成 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_AGV.gen.inc（設定 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_AGV.py，新 C 路結構 TestIF_File_AGV；入口 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_AGV.cpp，開機建替身在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4064 同一行）；D:\HT9045\config\AGV.ini 的路徑走測試縫 W906_AGVINI_PATH（沒設＝golden 字面值；ctest 全部導到沙盒）。開窗閘照 golden：工具選單＋spbAGV 看得見＝USE_E84_Sensor（main.cpp:24333；AGVModal=0 的機台開不了），開頁記 golden :35779 MES2198 "Enter AGV Form"（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp kOpenGates／kOpenEnters）。頁面 D:\HT9045\web\page\ht9045_agv_c.js（存檔鈕、小鍵盤），D:\HT9045\web\page\Setup.AGV.html:105 同一行載入。⚠ 開頁／存檔就會把 AGV.ini 的 "E84 Enable" 讀進 TestIF_File.bEnableE84 ⇒ AGVModal=1 的機台 E84 交握從那一刻起每一拍都跑（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:30394，golden 開過 AGV 畫面也一樣）。⚠ 引擎 D:\HT9045\web\page\ht9045_wire_engine.js 的 GOLDEN_BRIDGE 要加 'Setup.AGV.html': 'TestIF_File_AGV' 一行（筆電的檔，待認領），沒加之前這一頁不讀也不存。**patch B（20260930，待 ST01-E commit；Steven 請看：動真機 E84 交握輸出）**：開機／換配方讀檔 golden main.cpp:9378 fAGV->ReadFile() → FileRW_AGV_ReadFile（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:3454 W906_DoReadLastData 的 FrmAOI 那一行同一行）⇒ AGVModal=1 且 "E84 Enable"=1 的機台開機就跑 E84 交握；Initial Load／Unload 兩顆鈕接 form.event（golden :1316-1322／:1324-1330：狀態機回第 1 步、E84_1／E84_2 各 6 顆交握輸出 Off、bE84Loader／UnloaderActionflag[0..2]=false），頁面 D:\HT9045\web\page\ht9045_agv_c.js 送。golden 兩顆鈕不查；C++ 用 form.event 既有的運轉中拒收＋開過頁＋點得到（⚠ 比 golden 嚴：golden 非模態、運轉中按得到）。ctest B8_Ag1_Initial 28／28（假 IO）。**不在 AG-1**：Timer2（P-1）、36 顆燈、E84 log、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Automation\AGV_E84.cpp:926-937 的 YES／NO（筆電的檔）。ctest B8_Ag1_AgvIni 28／28 | St01 | 直接實作 | 無 | L | —（原歸 Jimmy） |
| CC-E11 | Config.Configuration | 手動測加熱器：全選／清除、送溫度、讀溫度、切加熱繼電器。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:5572、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:5581、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:5761、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:5800 | 部分：舊表單翻譯只有 btHeaterSelectAllClick（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cConfiguration.cpp:6592）；C 路沒有 | St01 | 接上＋補實作 | B4 做完再動 IniConfig（同一檔）；IO，要上機驗 | M | —（原歸 Jimmy） |
| M-5 | 主畫面 | 在主畫面改工作溫度、Soak 時間，按 🔄 Set 送出（檢查補償上下限、25 度控制、機台內有 IC 不准改等）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28131-28294（spbSetClick）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:26571（edWorkTemperBaseMouseDown）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:26480（edSoakTimeMouseDown） | 沒翻：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain_OperateMode.cpp:146 在 #if 0；網頁 D:\HT9045\web\page\main.html:179 的 🔄 Set 沒有 id、沒有 click | St01 | 直接實作 | 無；會把新溫度送到加熱器／ATC，要上機驗 | L | — |
| M-11 | 主畫面 | 點 FT／RT 小方塊：切 FT／RT 起動模式（Configuration 開了「FT & RT 鈕可以按」才有效）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:30700-30708（palFTClick／palRTClick → DoFTRTClick）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:31420-31457（按下／放開的外觀） | 已做主畫面那一條（20260930，B8 第 2 輪，待 ST01-E commit）：golden DoFTRTClick（:35752-35774）→ FTClick（:30710-30832）／RTClick（:30834-31001）逐行照翻在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp 檔尾（W906_Main_DoFTRTClick、WS act.main.ftrt 的 W906_Main_FtRtOp；D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp St01 那一行 act.main.* 條件同一行附加）。點得到嗎照 golden：palFT／palRT 的 Visible，Enabled＝golden ChangeLevelAttr（:12937-13038，每一拍）算的起動模式鎖 —— 移植樹 ChangeLevelAttr 是空殼、cbRunStartMode->Enabled 沒人維護，所以按下當下照同一套規則重算（機台有料、Loader 有料、等級不夠、權限檔、Auto Clean 任務都鎖；運轉中 golden 不改小方塊的 Enabled，按了 FTClick／RTClick 第一行就回 1），FTClick／RTClick 讀的也是它。起動模式照 golden 改後叫 Jimmy 的 W906_CbRunStartModeChange（只呼叫）。頁面 D:\HT9045\web\page\ht9045_main_st01_ev.js 接 palFT／palRT 的 click（main.html 不改）。**沒做**：遠端 FT／RT（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp:12162-12165、:12177-12180、:17008-17010、:17016-17018 仍閘著，Command.cpp 不是 St01 的檔；W906_Main_DoFTRTClick 可以直接接）；D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp:513-514 的 FTClick／RTClick 替身（Jimmy 的檔）沒改；按下去的外觀（palFTMouseDown／Up）。ctest B8_M11_FtRt 40／40 | St01 | 接上＋補實作 | 無；會改起動模式 | M | — |

### B9　主畫面跟 Jimmy 正在做的重疊：Site 格、6 顆控制鈕

| 編號 | 畫面 | 白話：操作員做什麼、會發生什麼 | golden 出處 | 移植樹現況 | 誰做 | 要做什麼 | 依賴 | 大小 | 待決題號 |
|---|---|---|---|---|---|---|---|---|---|
| M-6 | 主畫面 | 點 Test Site 格子：開／關某個 site（等級 10），並照 golden 寫 TestMode.Data、lastdata.dat、送 ATC 的 site 使用指令、重算 site map。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29932-30616（mtDutOnOffMouseUp，685 行）＋golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:23253-24232（ShowTestHeadComp1，980 行） | 部分：St01 只出了 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp:248 那一段；ShowTestHeadComp 是空殼（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp:247，37 個呼叫點都打到它）；網頁 D:\HT9045\web\page\main.html:169 點格被拿掉 | St01 | 直接實作 | 新題 N-1（Jimmy 已排進自己的待辦，要先確認） | L | todo D-005 T14（D5-Q1）、新題 N-1 |
| M-7 | 主畫面控制鈕（D:\HT9045\web\page\Main.gbControlBtn.html） | 按 HOME、RESET、ONE CYCLE、CLEAN OUT、TRAY FEED、ALARM RESET：網頁現在按下去只顯示「尚未接線」。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:7545（BtnHomeClick）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:7655（BtnResetClick）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:4469（BtnOneCycleClick）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:4395（BtnCleanOutClick）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:14467（BtnTrayEndClick）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:22867（BtnAlarmResetClick） | 部分：CLEAN OUT 已翻（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cCleanOut.cpp:64）；RESET、ONE CYCLE 是空殼（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp:404-405）；TRAY FEED 只計次（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp:506）；HOME、ALARM RESET 全樹 0 筆；網頁 D:\HT9045\web\page\ht9045_opbuttons.js:60-61 列為 UNWIRED | St01 | 接上＋補實作 | 新題 N-1（跟 Jimmy 的回原點／START 戰役重疊） | L | 新題 N-1 |

### B10　要先有「頁面狀態陣列」的事件（開窗、關窗、切分頁的那一下）

| 編號 | 畫面 | 白話：操作員做什麼、會發生什麼 | golden 出處 | 移植樹現況 | 誰做 | 要做什麼 | 依賴 | 大小 | 待決題號 |
|---|---|---|---|---|---|---|---|---|---|
| YM-3 | Setup.YieldMonitoring | 按「OK」：重讀檔、更新主畫面狀態列的 Yield 指示，然後關窗。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uYieldMonitoring.cpp:3190-3195 | 沒翻 | St01 | 直接實作 | X-1（關窗） | S | — |
| TS-10 | Setup.Temp_Set | 按 Exit 離開溫度頁：依「ATC Active」勾選讓 ATC 上線或下線。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:5175（sbtExitClick） | 舊表單翻譯有 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uTemp_Set.cpp:5412，沒接到網頁關窗 | St01 | 接上 | X-1（關窗）；ATC，要上機驗 | M | —（原歸 Jimmy） |
| SU-9 | Setup.SetUp | 按 Exit 離開 SetUp：Auto Shuttle Sensor 需要重設時不准離開；離開時把 site map 送給 GPIB；關 ASM 時清 HotPlate site map。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:3476（sbtExitClick）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:3363（FormClose） | 部分：FormClose 已翻（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc:2863）；存檔後尾段已跑（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp:335）；「不准離開」與 GPIB 那段沒接 | St01 | 接上＋補實作 | X-1（關窗，而且要能擋關） | M | —（原「待確認」） |
| BC-6 | Setup.BarCode（D:\HT9045\web\page\Setup.BarCode.html） | 按 Exit 離開 Bar Code 頁：關窗，並把兩個「Shuttle 2DID 檢查」工作重設、兩顆「Cheack 2DID SH1／SH2」鈕重新可按。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp:2408-2417（sbtExitClick）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp:531（FormClose） | 已做（20260930，B10c 後續；St01 工程師做、ST01-E 核對後 commit）：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_BarCode.py 加 sbtExitClick（逐行釘 golden、Close() 轉「記 closed＋BC_FormClose」）、events 加 sbtExit，--only 重產 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_BarCode.gen.inc；替身在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_BarCode.cpp 開機建；兩個 golden 全域（BarCode.cpp:46-47）補在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\BarCode\BarCode.cpp 檔尾；頁面 D:\HT9045\web\page\ht9045_barcode_ev.js 攔 Exit 送 form.event、回 closed 才關。運轉中 form.event 回 running ⇒ 頁面直接關、只跑 FormClose（那四行沒跑；今天沒有讀者：State Record 那兩列閘著 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cStateRecord.cpp:1944-1949，2DID 檢查沒翻）。ctest FormEvent_Position [18]、EvB10A_Edges [7] | St01 | 接上＋補實作 | X-1（關窗；B10c 的 BarCode 關窗邊緣 6d0dfbb1） | S | — |
| OS-6 | Setup.OffSet | 關 Offset 頁時，Z 教導沒做完就要求重新回原點。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp:835（FormClose） | 沒翻（未驗證） | St01 | 直接實作 | X-1（關窗） | S | —（原歸 Jimmy） |
| OS-7 | Setup.OffSet | 開 Offset 頁時送 SECS「Enter Offset」事件（同 Speed 已做的 EnterSpeed）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28716-28722 | 沒有：Speed 那顆已做（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp:646）；Offset 還沒 | St01 | 直接實作 | X-1（開窗） | S | —（原「待確認」） |
| CT-3b | Setup.Contact | 關 Contact 頁時停下 Contact 模式、收尾。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:1842（FormClose） | 沒翻 | St01 | 直接實作 | X-1（關窗）；CT-3 | M | —（原歸 Jimmy） |
| CC-E10 | Config.Configuration | 按 Exit 離開 Configuration：更新主畫面速度顯示、PE 鈕要不要出現；[C12] 關掉而 PE 模式還開著時自動關 PE。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6383-6390 | 沒翻；PE 關閉可用現成 act.main.peModel（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4826 那一行） | St01 | 直接實作 | X-1（關窗） | S | — |
| SA-2 | Setup.SCK_ART（D:\HT9045\web\page\Setup.SCK_ART.html） | Auto Retest（ART）流程計時器、TSV 連線收資料、Exit。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\SCK_ART.cpp:1404、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\SCK_ART.cpp:4064、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\SCK_ART.cpp:4075、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\SCK_ART.cpp:809 | 沒翻（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fSCKART.cpp 沒有這些處理器） | St01 | 直接實作 | X-1（Exit）；這其實是流程，不只是頁面 | L | —（原「待確認」） |
| X-3 | Setup.TrayAssignment、Config.Configuration | 開頁時 BCB 設單選群組／勾選框會「順便觸發」它的點擊程式（例：Tray Assignment 一開頁 RGAuto3 就是灰的；Configuration 開頁 D36 勾著會把 D33 取消、D35 勾上）；網頁開頁沒有這一步。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:548、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:570-626；golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6551、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Public\HTEditList.cpp:1390 | 沒有：開頁設值不觸發（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp:47 TA_SetRadioIndex）；Configuration 開頁照檔案顯示 | St01 | 直接實作 | X-1（要在操作員真的開窗那一下跑，bin 訊息才會跳在對的時候） | M | R100、R118 |
| X-4 | 主畫面選單＋所有設定頁 | 按開窗那一下：查權限（不夠就開不了、說明原因）、記一筆「Enter …」、重跑開頁程式——跟 BCB 一樣是「開窗當下」，不是開站時。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29030-29037（sbSettingClick）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29009-29016（sbConfigClick） | 部分：C++ 開頁／存檔已重查閘（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp kOpenGates，cb306f89）；但開頁發生在開站時（Q49）；主畫面選單 D:\HT9045\web\page\main.html:294、D:\HT9045\web\page\main.html:297 點了直接開 | St01 | 直接實作 | X-1 | M | Q49（被 Q51 裁決吸收）、R108、R110、C-1／C-2 頁面那一半 |
| X-5 | Setup.Temp_Set、Config.Configuration 等 | 點分頁標籤時 BCB 會自動跑一段程式（例：Temp_Set 切到 ATC 頁，「基準溫度」那幾格藏起來、改不到）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:5224-5262（pgcTempOffsetChange）等 5 項（D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-control-tabs.md §2.3） | 沒有：切頁事件都沒翻（route-c-golden-bridge.md §3.0g-6 已知缺口 2）；Temp_Set 舊表單翻譯有 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uTemp_Set.cpp:5476；St02 的頁面已做畫面那一半（TS-5） | St01 | 接上＋補實作 | X-1（Q51 的分頁細節跟陣列一起設計） | M | Q51-1～Q51-5、R105、R106 |
| CC-L1 | Config.Configuration | 切 Configuration 的分頁時，照權限檔 D:\HT9045\config\Security_new.def 把旗標 0 的那頁整頁鎖住。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6101（PageControl1Change）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6116（pcConfigChange） | 沒有：開頁有讀權限檔（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc:8448），切頁事件沒翻 | St01 | 直接實作 | X-1、X-5 | M | Q46（要做、排後面） |

### B11　需要新頁面或新模組的事件

| 編號 | 畫面 | 白話：操作員做什麼、會發生什麼 | golden 出處 | 移植樹現況 | 誰做 | 要做什麼 | 依賴 | 大小 | 待決題號 |
|---|---|---|---|---|---|---|---|---|---|
| BS-1 | Setup.BinSel（D:\HT9045\web\page\Setup.BinSel.html） | 按「AOI Bin」：開 AOI Bin 子視窗設定、存檔、刷新主畫面 Bin 顯示（V912 20260714 新功能）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cBinSel.cpp:6679-6697 | 沒有：沒有 AOI Bin 子視窗的網頁 | St01 | 直接實作 | 新題 N-3（要不要做新子頁） | L | —（原「待確認」）；新題 N-3 |
| OS-3 | Setup.OffSet | 改「伺服器 Contact Force」：透過 ProductionInfo 設定伺服器的 contact force，顯示最終力量。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp:3356-3365 | 沒有：移植樹沒有 fProductionInfo（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cConfiguration.cpp btnSetIPSCQtyClick 的 GATE W7-IPSC 註記） | St01 | 直接實作 | 新題 N-3（要先移植 ProductionInfo 模組） | L | —（原「待確認」）；新題 N-3 |
| CC-E12 | Config.Configuration | 按 Tester List／Tester Map／[N15]／[A25] 的「選檔」鈕：開本機選檔視窗選路徑。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6263、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6445、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6793、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6811 | 沒有：網頁不能開本機選檔視窗 | St01 | 直接實作 | 新題 N-3 | M | —（原「待確認」）；新題 N-3 |
| CC-E13 | Config.Configuration | 其他子視窗與網路動作（約 16 支）：ESD 表、MES、RMS、Config 上傳／下載、FTP、批次複製配方、N25 手動、N31／N32／N35、A71、IPSC 數量設定／清除。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6806、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7745、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7933、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7833、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7841、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7739、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7750、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7880、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7894、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7860、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7875、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7254、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7260、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7899 | 沒翻（IPSC 兩顆在舊表單翻譯裡閘著：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cConfiguration.cpp btnSetIPSCQtyClick 的 GATE W7-IPSC） | St01 | 直接實作 | 新題 N-3（多數要新子頁或網路模組）；做之前逐支拆列 | L | —（原「待確認」）；新題 N-3 |
| SA-1 | Setup.SCK_ART | ART 設定套用：Apply Setting、Lot Info、Qty、Count、Input Qty、刪 Lot Info、FTCT 重設、點計數面板。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\SCK_ART.cpp:1533、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\SCK_ART.cpp:1597、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\SCK_ART.cpp:1522、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\SCK_ART.cpp:724、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\SCK_ART.cpp:4311、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\SCK_ART.cpp:1392、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\SCK_ART.cpp:4350、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\SCK_ART.cpp:4327 | 沒翻（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fSCKART.cpp 沒有；這頁走 B 路欄位，不是 C 路） | St01 | 直接實作 | 新題 N-3（這頁要先改走 C 路或另做入口） | L | —（原「待確認」）；新題 N-3 |
| SA-L1 | Setup.SCK_ART | 等級 129 才能開這頁；等級 130 才看得到設定面板、可改。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:33810（spbAutoRetestClick）、golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\SCK_ART.cpp:675-690（SettingPanelOnOff） | 沒有：這頁不是 C 路，開窗閘表 kOpenGates 管不到 | St01 | 直接實作 | 跟 SA-1 一起 | S | —（原「待確認」） |
| M-14 | 主畫面 Tools／Config 選單 | 選單裡 13 項還沒有網頁的畫面：ATC、CCD、OCR、Dyna. Temp、Socket、Rotate、Laser、Tray Function、SECS/GEM、Auto Temp.、Air Con.、PM Alarm、Manual。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29598（sbATCClick）起各 sbXxxClick（逐項行號見 scratchpad\eval6\mainscan.txt） | 沒有：D:\HT9045\web\page\main.html 選單項標成 todo／shot（沒有 DFM_MAP 對應） | St01 | 直接實作 | Q41 裁決 S158：新頁面先不做 | L | Q41（新頁面先不做） |

### S1　St02 的列（照 S85 與 Q41 的分法）

| 編號 | 畫面 | 白話：操作員做什麼、會發生什麼 | golden 出處 | 移植樹現況 | 誰做 | 要做什麼 | 依賴 | 大小 | 待決題號 |
|---|---|---|---|---|---|---|---|---|---|
| R120-F | FTP 下載工作檔（確安 CC_CYUEAN，網頁沒有這個畫面） | 確安機台結批後，操作員到 FTP 畫面重新下載工作檔、下載成功就解開 START（不用重開程式）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\KYECFTP\FTPClient.cpp:1011-1015（在 TfFTPClient::plSLoadClick golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\KYECFTP\FTPClient.cpp:862 裡：成功設 bFTPDownloadSetupFile=true） | 沒有：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\KYECFTP 底下沒有 plSLoadClick、也找不到 bFTPDownloadSetupFile；網頁沒有 FTP 下載頁 | St02 | 直接實作 | 新題 N-3（網頁要有地方讓操作員按下載） | M | R120＝A（Steven 20260928 09:4x）；St02-M 21043077 說可由 St02 做 |
| W44-1b | Config.Configuration（Handler 端） | 把 [N10-3-1] 指定時間登記進 Configuration 存檔清單，並做「錯過時間，開機後補傳」。 | 舊版只宣告不存：golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Config.h:1041 | St02 認領中（W44-1 現況） | St02 | 直接實作 | W44-1 St01 頁面那一半（B4） | S | W44-1＝A、W44-2＝要補發 |
| TS-9 | Setup.Temp_Set | 輸入氣流溫度偏移，加上主畫面工作溫度後夾在允許範圍。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:7018 | 沒找到：St02 分支的頁面檔裡沒有這支（20260928 查） | St02 | 直接實作 | 要主畫面工作溫度的 tag | S | — |
| OS-4 | Setup.OffSet | 勾 CheckBox1：切換吸嘴逐顆 offset 的顯示。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp:2791 | 沒找到：St02 分支的 D:\HT9045\web\page\ht9045_offset_wire.js 沒有 | St02 | 直接實作 | 無 | S | —　⛔ 20260928 不用做：golden 那個勾選框是隱藏的（St02 查證，FROM_STEVEN §4），沒有操作員能點的事件 |
| CL-5 | Setup.Cleaning | 按 sbTrayAssign：golden 其實是切主畫面的 AutoCleanStringGrid（不是開 Tray Assignment 頁，盤點原寫錯）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp:2311 | 沒做：St02 分支 D:\HT9045\web\page\ht9045_cleaning_c.js 註明「切主畫面 AutoCleanStringGrid——不在設定頁接」 | St02 | 直接實作 | 主畫面那一格是 St01 的（M 列），要兩邊對一下 | S | — |
| CC-E2 | Config.Configuration | [D46] 上下箭頭改等待時間；St02 頁面已排隊一次送一下（R117 的 C）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6094 | C++ 已做（64ade3b7）；頁面 St02 已做（D:\HT9045\web\page\ht9045_config_q41.js，St02 分支，未合 main） | St02 | 已做（待合 main） | — | S | R115、R117 |
| Q41-St02 | Setup.*（St02 的其餘頁面事件） | LU-1、TF-2、TF-3、HP-2、SU-1～SU-6、YM-4、TS-3～TS-8、CL-6、CT-2、BC-2、BC-5、CC-E7、CC-E9、CC-E14、SP-1～SP-5、TI-1～TI-5：頁面上的連動、開子頁鈕、小計算。 | 見盤點 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md 第三節各列 | St02 分支（origin/v906/steven-gpib-widget，HEAD 5e16f04f 之後）已有頁面補件：D:\HT9045\web\page\ht9045_lduld_q41.js、D:\HT9045\web\page\ht9045_trayform_q41.js、D:\HT9045\web\page\ht9045_hotplate_wire.js、D:\HT9045\web\page\ht9045_setup_sitemap.js、D:\HT9045\web\page\ht9045_yieldmon_q41.js、D:\HT9045\web\page\Setup.Temp_Set.html、D:\HT9045\web\page\ht9045_temp_set_c.js、D:\HT9045\web\page\ht9045_cleaning_c.js、D:\HT9045\web\page\ht9045_contact_q41.js、D:\HT9045\web\page\ht9045_barcode_c.js、D:\HT9045\web\page\ht9045_config_q41.js、D:\HT9045\web\page\ht9045_speed_c.js、D:\HT9045\web\page\ht9045_testerif_c_wire.js（逐列未驗證；未合 main） | St02 | 已做（待合 main） | Q52（合 main） | M | R119（回預設值鈕要改走不改鎖住欄位的做法，St01 另告知） |

### 另案　頁面狀態陣列（X-1）

| 編號 | 畫面 | 白話：操作員做什麼、會發生什麼 | golden 出處 | 移植樹現況 | 誰做 | 要做什麼 | 依賴 | 大小 | 待決題號 |
|---|---|---|---|---|---|---|---|---|---|
| X-1 | （跨頁）所有設定頁 | 操作員按開窗鈕、關窗鈕的那一刻，C++ 要知道「哪一頁現在開著」。BCB 用各表單的 fShow／Visible 旗標；網頁版沒有，所以「開窗當下查權限、記 Enter、關窗時做收尾」都對不準（見 Q49）。 | golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28694-28699（sbSpeedClick：開窗記 MES2187、ShowModal、關窗後換算）等各 sbXxxClick；fShow 旗標例 golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28145（spbSetClick 讀 fTemp_Set->fShow） | 沒有：只有 Teach／Motor Test 與 St02 的 Tester IF 用到的「表單開／關掛勾」D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp（5b73d905，W906_WindowEdgeRegister）；網頁開站就把設定頁在背景載好（D:\HT9045\web\background.html:906-907） | St01 | 直接實作 | —（這一列本身就是前提；ST01-E 另案設計中：D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md（20260928，工作樹還沒 commit；§8 有它自己的 Q-P 題）；網頁要一起改 D:\HT9045\web\background.html、D:\HT9045\web\page\ht9045_wire_engine.js，要先跟 Jimmy 登記區段） | L | Q51（Steven 20260928：C++ 陣列記所有頁的開／關，fShow 改讀它，第一優先）；Q49、R108、R110 |

---

## 五、已經做完、不列入上表的（給 ST01-M 標 superseded 用）

| 項目 | 在哪一顆 commit／哪個檔 |
|---|---|
| LU-2、SP-6（含開頁 SECS EnterSpeed 模擬計數）、BS-2、CT-4（R84：開頁時跑） | c8028b21（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp 檔尾、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Ld_UldDelayTime.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ArmSpeed_File.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\BinSelect.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp:213） |
| TF-5、HP-3、TA-6、YM-5、CC-E15（關窗尾段第二批） | 9268162b（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp:760-954） |
| SP-1～SP-5（Speed 滑桿、勾軸、全選、±10、回預設值）、TS-7（Temp_Set 基準點數） | C++ 70aa17e8（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ArmSpeed_File.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.cpp 檔尾）；頁面 St02（D:\HT9045\web\page\ht9045_speed_c.js、D:\HT9045\web\page\ht9045_temp_set_c.js，St02 分支）。R119（Steven 20260928 裁 B：滑桿與回預設值不能改到鎖住的欄位）另一位工程師在修 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ArmSpeed_File.cpp，不在本表 |
| CC-E2（D46 上下鍵）、CC-E7（勾選連動顯示） | C++ 64ade3b7（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp 檔尾、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc:9606）；頁面 St02（D:\HT9045\web\page\ht9045_config_q41.js，St02 分支，已照 R117 的 C 一次送一下） |
| C-4（即時事件入口本身）、Q40 三頁的 C++ 分派 | 76058840（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\FormBridge.cpp） |
| TA-1～TA-4 的 C++ 那一半、TS-1 的 C++ 那一半 | 4e74e8b4（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.cpp）——頁面那一半還沒有，列在 B2 |
| C-1、C-2、SP-L1、YM-L1、BC-L1、DI-L1 的 C++ 重查 | cb306f89（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp kOpenGates）——網頁選單點下去先問 C++ 那一半併進 X-4 |
| YM-1（Yield 的 Apply 鈕） | a8eca460（D:\HT9045\web\page\ht9045_yieldmonitoring_c.js）；還沒用瀏覽器實測，排在今晚 Q50 的沙盒實跑 |
| R76（單選鈕沒分組） | Jimmy 00f9a882（D:\HT9045\web\page\ht9045_wire_engine.js:2166 groupBareRadios），已在 St01 分支 |
| TI-4 的關窗掛勾 | 5b73d905（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp W906_WindowEdgeRegister）；St02 的頁面那一半在 St02 分支 |

盤點更正（照查到的寫，不改盤點原檔）：CL-5 golden sbTrayAssignClick 不是開 Tray Assignment 頁，是切主畫面的 AutoCleanStringGrid（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp:2311；St02 分支 D:\HT9045\web\page\ht9045_cleaning_c.js 也這樣註明）；SP-6 的 SECS EnterSpeed 已在尾段送（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp:646）；OS-2 的旗標 iIndexChange 在 golden 全樹只有寫、沒有讀（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp:734、:2889、:3240、:3265）。

---

## 六、待決題：做完這幾批就結案的，和還要 Steven 回的

### 6.1 做完對應批次就可以標 superseded（不用再問 Steven）

| 題號 | 為什麼可以結 | 在哪一批 |
|---|---|---|
| Q40（引擎送出點） | Q40 已裁 A；「等 Jimmy 在引擎加送出點」那一句改成 B2 由 St01 在頁面補件送 | B2 |
| Q44 | Steven 20260928 已裁（最小停機＋其他列待辦） | B7 |
| Q45（含 Q45-1～10） | Steven 20260928「按照你的建議執行」：#4、#6、#7＝A（B5），#1～#3、#5＝C（B4 的 CC-L2 只補頁面說明） | B4、B5 |
| Q46 | Steven 20260928「要做、排後面」 | B10（CC-L1） |
| Q47、Q48 | Steven 20260928「我們做」 | B6（M-2、M-3） |
| Q49 | 被 Q51 的裁決吸收（開窗那一下查權限、記 Enter、跑開頁程式都掛在頁面狀態陣列上，我們做、不等 Jimmy） | X-1＋B10（X-4） |
| R73、R75 | Q45 裁了：#1～#3 維持拒存（R73 的做法留著）；#6／#7 改成重新登入（R75 剩下那一半） | B4、B5 |
| R96 | 頁面開始送事件之後，「沒送事件就不重播」只剩備援，維持 A 即可 | B2 |
| R100、R118 | 照「沒有移植的直接實作」改做 B：開頁時照 VCL 觸發點擊程式；放在頁面狀態陣列之後，bin 訊息才會跳在操作員真的開窗的時候 | B10（X-3） |
| R107 | 照通則改做 B：HotPlate 尾段的 golden 警告放進存檔回覆 | B6 |
| R108、R110 | 開窗記 Enter 改成「真的開窗那一下」，存完重讀多一筆的問題跟著消失 | B10（X-4） |
| R117 | St02 頁面已經一次送一下（選項 C），合 main 後結案 | S1 |
| R120 | Steven 20260928 已裁 A；後續「FTP 下載成功設回 true」列成 R120-F（St02） | S1（等 N-3） |
| todo D-005 | T13（M-3）、T15（M-2）、T20（B3）；T14（Site 格）等 N-1；DoTrayFeedProcess 三處仍歸 Jimmy（S79、D5-Q4），不在本表 | B3、B6、B9 |
| 交接 B6（主畫面 Tester 圖示沒接） | — | B6（M-4） |
| W44-1、W42-c、W54 | Steven 20260928 已裁；頁面那一半是 St01 的 | B4、B6、S1 |

**不受這幾批影響、照原樣留著的**：頁面狀態陣列設計檔（D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md §8）自己的 Q-P 題；Q4-2（等 Steven 回「選 A」）、Q51-1～Q51-5 與 R105／R106（分頁細節要跟頁面狀態陣列的設計一起對）、Q52、R97～R99（Temp_Set 沒送事件就拒存、讀檔補寫、記憶體換表——頁面送事件之後仍是備援）、R109、R111～R116、R119（另一位工程師在修）。

### 6.2 還要 Steven 回的新題（3 題）

#### N-1. 主畫面的 Site 格和 6 顆機台控制鈕，照「主畫面是 St01 做」要不要現在就接手？
**背景**：
- 操作員看到的：主畫面點 Test Site 格子開關 site，網頁現在點了沒反應（D:\HT9045\web\page\main.html:169）；右邊控制面板的 HOME、RESET、ONE CYCLE、CLEAN OUT、TRAY FEED、ALARM RESET 六顆，按下去只顯示「尚未接線」（D:\HT9045\web\page\ht9045_opbuttons.js:60-61）。只有 START、PAUSE 接好了。
- 這兩件都是機台流程：Site 格要翻 golden 兩大段（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29932-30616 mtDutOnOffMouseUp 685 行＋D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:23253-24232 ShowTestHeadComp1 980 行，會寫配方的 TestMode.Data、D:\HT9045\system\lastdata.dat、送 ATC 的 site 指令），Jimmy 的筆電 20260927 已經把它排進自己的待辦（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\D005_TFMAIN_TESTMODE_WRITERS_20260927.md 第 1 節第 1 點：INBOX 第 69、77 列、NIGHT_REPORT:135）；HOME／START 是 Jimmy 的「golden 邏輯」戰役（D:\HT9045\.claude\skills\gl-wave-loop）。移植樹現況：CLEAN OUT 已翻（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cCleanOut.cpp:64）、RESET 和 ONE CYCLE 是空殼（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp:404-405）、TRAY FEED 只計次（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp:506）、HOME 和 ALARM RESET 沒翻。
**選項**：
- **A** St01 接手：先在交接檔問 Jimmy 做到哪裡，沒開工的由 St01 照 golden 做完（C++＋網頁）。例：操作員在網頁按 HOME → C++ 照 golden BtnHomeClick（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:7545）整機回原點；Jimmy 那邊不再做同一件。
- **B** C++ 留給 Jimmy 的戰役，St01 只接網頁（按鈕送指令、顯示結果）。例：Jimmy 把 HOME 的 C++ 做好推上來之後，St01 當天在網頁接上；在那之前按 HOME 照舊顯示「尚未接線」。
- **C** 已經翻好的 CLEAN OUT 先由 St01 接上，其餘照 B。例：今天起網頁按 CLEAN OUT 會照 golden 清料；HOME 等 Jimmy。
**St01 建議**：C（兩邊不會同時改同一條機台流程；已經翻好的先接上）。

#### N-2. Configuration 的「設定 Supervisor 密碼」鈕（盤點 CC-E5），網頁版要不要做？
**背景**：golden 按這顆會跳登入框，把輸入的新密碼寫進 D:\HT9045\system\lastdata.dat 的 Supervisor 欄（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6052-6060）。移植樹沒有；這個欄位在移植樹誰會讀，這次沒查（未驗證）。Steven 已裁的相關規則：S55「密碼需要加密」、Q45-1＝A「重新登入的密碼跟主畫面登入一樣只走本機 127.0.0.1 送原字、不印不記」。
**選項**：
- **A** 做，照 Q45-1 同一個做法送（只走本機、不印、不記）。例：工程師在 Configuration 按這顆 → 網頁跳輸入框 → 輸入新密碼 → C++ 寫進 lastdata.dat；主控台只印「Supervisor password changed」，不印密碼。
- **B** 不做：網頁上這顆停用，說明「請到權限頁（Status.Security）或 BCB 版機台改」。例：按了只跳說明，什麼都不改。
**St01 建議**：A（跟 Q45 甲同一條路，工作量小）。

#### N-3. 需要新頁面或新模組的 7 件，這次要不要一起做？（Q41 裁決 S158 是「新頁面先不做」）
**背景**：下面這些事件本身不難，但網頁上沒有它要開的那一頁，或移植樹沒有它要叫的模組：
- R120-F：確安（客戶碼 868）機台結批後要到 FTP 畫面重新下載工作檔才能 START；golden 在 FTP 畫面下載成功時設回 true（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\KYECFTP\FTPClient.cpp:1011-1015），網頁沒有 FTP 下載頁，移植樹 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\KYECFTP 也沒有這支 ⇒ 現在要重開程式。
- BS-1：Bin Select 的「AOI Bin」子視窗（V912 20260714 新功能，D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cBinSel.cpp:6679-6697）。
- OS-3：Offset 頁的「伺服器 Contact Force」（要先移植 ProductionInfo 模組，D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp:3356-3365）。
- CC-E12：Configuration 的 4 個「選檔」鈕（網頁不能開本機的選檔視窗）。
- CC-E13：Configuration 其他約 16 個子視窗與網路動作（ESD、MES、RMS、Config 上傳下載、FTP、IPSC…）。
- SA-1／SA-L1：Auto Retest（SCK_ART）頁的設定套用與等級（這頁走舊的 B 路，要先改走 C 路）。
- M-14：Tools／Config 選單裡還沒有網頁的 13 項。
**選項**：
- **A** 這次一起做（每件先出一份小方案給 Steven 看再動手）。例：做一個最小的 FTP 下載頁，確安機台結批後在網頁按「下載」，成功就解開 START；AOI Bin 子視窗也照 golden 做一頁。
- **B** 維持 S158，先不做，排到「新頁面」那一批；**只有 R120-F 例外先做**（做一個最小的 FTP 下載入口）。例：確安機台結批後在網頁就能重新下載；AOI Bin 鈕在網頁上停用並說明「這一頁還沒做」。
- **C** 全部先不做（R120-F 也不做）。例：確安機台每次結批後都要重開程式才能 START。
**St01 建議**：B。

---

## 七、限制與沒做的

1. **未驗證的列**：YM-2（fLotInfo->RefreshYieldMonitor 在移植樹有沒有本體）、TS-12、BC-4（沒逐支查）、OS-6（沒查 FormClose 在不在）、M-18、M-19（C++ 那一段有、頁面沒綁，本體沒讀）、TS-L1（可能已被開頁停用蓋掉）、N-2 的 Supervisor 欄位用途。St02 那 38 列只看 St02 分支的檔頭與註解有沒有提到，沒有逐列驗證、也沒跑。
2. **主畫面只掃了網頁上有對應元件的處理器**（scratchpad\eval6\mainscan.py：main.dfm 250 個綁定裡，元件出現在 D:\HT9045\web\page\main.html 或 Main.*.html 的 135 個）；網頁上根本沒有那個元件的 golden 處理器（例如只有某些客戶才顯示的面板）沒有列。客戶專屬的照 Steven 20260925 的決定跳過：BtnSTEP／BtnT_Start／BtnZUpDown（TSMC，D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:32687-32703）、cbRunStartModeClick（ASE 高雄，D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:33505）；labTesterClick、labRunModeClick 本體整段註解掉，不用做；cbSetupFileNameCloseUp 只改寬度，純畫面。
3. **「沒翻」的宣稱**只代表 20260928 用 scratchpad\eval6\psearch.py（移植樹 .cpp／.h／.inc／.py 全文，排除 build*、tests、_retired、vendor）在 HEAD 6f0162cf 查的結果；平行的同事隨時可能加進來，動手前再查一次。
4. **行號**：golden 是 V912 主 repo 那份；移植樹是 HEAD 6f0162cf。TA-5 引用的 _EditList.cpp:344 是盤點當時的行號，c9d3c932 之後已移動、沒重查。
5. **大小是粗估**：B8 的每一列都要上機驗（機台會動、加熱、通訊），實際時間看機邊排程。

