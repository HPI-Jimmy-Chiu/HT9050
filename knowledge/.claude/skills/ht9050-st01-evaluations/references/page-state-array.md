# 設計：C++ 的「頁面表」取代 BCB 的 fShow（Q51 裁決＋第一優先）

> 讀者：Steven。撰寫：ST01-E（Steven01 工程線）派的工程師，20260928。**只讀研究＋設計，沒有改任何程式、沒有 build、沒有跑 wb_serve。**
> 基準：`D:\HT9045` 分支 `v906/steven-cbridge-review6`，commit `bbe5e1fc`。移植樹的檔一律用 `git -C D:\HT9045 show bbe5e1fc:<路徑>` 讀（別人可能正在改工作樹）；`D:\HT9045\web\background.html` 另外跟工作樹比過（忽略換行後相同）。
> 三棵樹的寫法：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` 開頭＝**移植樹**（C++，UTF-8）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` 開頭＝**golden V912**（BCB6，Big5／cp950）；`D:\HT9045\web\` 開頭＝**網頁**。行號都是 `bbe5e1fc` 當下。
> Steven 20260928 的原話（ST01-M 轉）：
> - Q51：「在c++端弄一個陣列, 紀錄目前總共有多少頁面, 每個頁面的狀態是開還是關, 然後跟html端互通」
> - 第一優先：「另外, bcb的 fShow的flag, 使用剛剛說的那個網頁顯示的陣列來替代, html跟c++要同時修改, 這個看起來比較重要, 要先做好」（「頁面」＝整個畫面／表單，不是分頁標籤）
> - 「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上, 如果沒有移植的, 我們直接實作」
>
> 名詞（先講白話，括號裡是程式裡的名字）：
> - **畫面開著沒**：BCB 每個表單自己帶一個「我現在顯示著」的旗標（`fShow`，有些表單叫 `bShow`），表單打開時自己設成真、關掉時設成假；別的程式讀它來決定要不要做某件事。
> - **視窗總表**：網頁外框每次視窗開／關就把「哪些視窗開著」整份送給 C++（指令 `ui.windows.put`），C++ 收在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp`（Jimmy 寫、EastSun 補過）。
> - **頁面表**：本設計要新做的那個「陣列」（程式名建議 `W906PageTable`，檔 `WebPageTable.cpp/.h`）。
> - **C 路頁**：由 C++ 跑 golden 開頁／存檔程式的設定頁（24 頁）。

---

## 0. 一句話結論（操作員／機台先看到什麼）

做一張 C++ 的**頁面表**：一列一個畫面（網頁外框的 68 個視窗＋golden 會讀、但網頁沒有視窗的 22 個表單），每列記「開／縮小／關／這台沒建立／沒有網頁」、最後是誰改的（操作員從網頁、或程式自己）；網頁照舊用視窗總表回報操作員的動作，C++ 程式自己開關畫面（例：回原點時 golden 會自己叫出 Home Monitor）也寫進同一張表；整張表再用一個 tag（`ui.pages`）送回網頁，網頁照著開／關視窗。golden 所有「畫面開著沒」的判斷，一律改問這張表的**一個函式**（`W906_FormShowing("fTeach", …)`）。

做完之後操作員／機台會看到的（依選項，細節在 §8）：

1. **瀏覽器全部關掉，機台不會再無聲停住**（Jimmy 已確認的問題：今天 15 秒沒收到網頁回報，C++ 把 Teach／Motor Test 當成開著，主流程暫停）。建議規則：沒有任何網頁連著＝所有網頁畫面都關了。
2. **F5 重新整理後立刻以新畫面為準**（今天舊畫面的回報要 15 秒才過期）。
3. **這台沒裝的功能頁（例 Bar Code、Ground Man）不會被當成「開著」**（今天沒建立的視窗不進總表 ⇒ C++ 當成「不知道＝開著」）。
4. **golden 裡今天「永遠當成沒開」的 224 個判斷開始照 BCB 生效**。例：Contact 頁開著時不報低良率、不進省電模式、遠端指令回「設定中」——這是照 golden，但**行為會變**（§8 Q-P3 問要不要先只記錄不生效）。
5. **程式自己開的畫面會在網頁跳出來**（例：按 HOME ALL，golden 會自己叫出 Home Monitor 看進度；§8 Q-P2）。
6. **設定頁在操作員按開窗那一下才讀資料、才記「Enter …」**（Q49 的 B＋D；ST01-M 註記「可視為已由 Q51 決定」）。

先做的第一刀（約 2～2.5 人天）就能解掉 1、2、3，並把今天已經接上總表的 25 個判斷＋主流程暫停＋GPIB 設定中位元＋Teach 開關邊緣一次轉到新表（只改 4 行呼叫端）。

---

## 1. 背景（白話）

### 1.1 BCB 版

每張表單打開時（FormShow）把自己的 `fShow` 設成真、關掉時（FormClose）設成假，例：Home Monitor `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uhome.cpp:4847`（開）、`:4870`（關）。別的程式直接讀：
- 回原點狀態機每一步都問「Home Monitor 還開著嗎，沒開就停」（golden 對應移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uhome.cpp:2153` 起 15 處）。
- 主流程在 Teach／Motor Test 開著時暫停（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:30493-30494`，EastSun R8）。
- START 前一連串「Contact 頁開著就跳過某檢查」（例 golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:6227-6231`）。

只有一台螢幕、一個程式，旗標永遠是對的。

### 1.2 網頁版今天

畫面在瀏覽器裡，C++ 沒有表單物件的 FormShow／FormClose 事件。今天有**三套各自為政**的答案：

| 誰在回答 | 在哪 | 回答什麼 |
|---|---|---|
| 表單物件的成員（facade 的 `fShow`） | 例 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fContact.cpp:139`（建構設 false）、`:492`（Init 設 false），全樹**沒有任何地方把 `fContact->fShow` 設成真** | 網頁視窗類的表單：**永遠 false**。C++ 自己開的對話框（告警 fNote、是否框 MyMessageBox）：對的——wb_serve 等回答時自己設 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:7597`、`:7600`、`:7608`、`:7614`；Home Monitor：回原點程式自己設 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uhome.cpp:646`、`:652` |
| 視窗總表的政策版答案（`WebWindowRegistryFShowPolicy`） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:365-377` | 網頁報的；**15 秒沒收到＝當成開著**（`:226-237`、門檻 `:16`） |
| C 路頁自己的「這頁開過了」 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:26-30`、`:57-60`、`:211` | 開站時頁面在背景要資料就算「開了」（Q49 查到的問題） |

「成員 或 總表」的聯集有兩個入口，外加一個 GPIB 狀態位元的專用入口：
- `W906_FShow(表單名, 成員)`：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp:210-213`（START 那 19 處）。
- `W906_FormFShow(表單名, 成員)`：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:30046-30049`，透過函式指標 `W906_FormFShowHook`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.h:421`、`:440`），由 EastSun 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebMotorAccessLive.cpp:1140-1143` 接上、`:1196` 安裝。
- GPIB「設定中」位元（`Bit4_HandlerDiagnostics`）：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp:15061` 呼叫 `W906_DiagnosticsWindowOpen_Hook`，wb_serve 在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:885-891` 轉給 `WebWindowRegistryDiagnosticsOpen`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:380-435`）；`:4389` 安裝。（ⓘ 該函式 `TfMain::MachineStatus()` 今天沒有呼叫點，見 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp:15056-15060` 的註解，所以這一位今天不會被算。）
- 另外 St01 的 Teach／Motor Test 開關邊緣（S122）與通用開關掛勾：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp:82-88`（取樣）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.h:165`（`W906_WindowEdgeRegister`）。

---

## 2. 現況盤點

### 2.1 golden V912 讀「畫面開著沒」的地方：672 處

量法：cp950 解碼、去掉註解、`#if 0` 堆疊；抓 `X->fShow`／`X->bShow`（X 是任何物件），以及 `X->Visible`／`X->Showing`（X 限 golden 標頭 `extern PACKAGE T… *x;` 宣告的 135 個表單物件），外加表單自己函式裡讀自己的 `fShow`。**不算**表單裡叫 `bShow` 的區域變數（例 Configuration 頁有 1,284 個同名區域變數，那不是畫面狀態）。腳本在暫存目錄（不留檔）。

| 表單（golden 名） | golden 讀幾處 | 說明 |
|---|---:|---|
| fContact（Contact 頁） | 121 | START、製程、良率、省電、AutoClean… |
| fNote（告警框） | 79 | 對話框，C++ 自己開 |
| MyMessageBox（是否框） | 61＋`Visible` 39＋`Showing` 2 | 對話框，C++ 自己開 |
| Zteach（In/Out Arm Z 自動教導） | 38 | **網頁沒有這個視窗** |
| fiosetview（IO 頁） | 37 | |
| fHome（Home Monitor） | 25 | 程式開＋操作員開（Q9 保留選單入口） |
| fShuttleMove | 15 | |
| fOffSet／FormBarcodeReader | 11／11 | |
| fSetup／fTeach／fQwertyKey | 10／10／10 | |
| fFTPClient／fTemp_Set | 9／9 | |
| fConfiguration／fInput | 8／8 | |
| 其餘 70 個表單名（含表單讀自己） | 各 1～7，合計 164 | 例 fMotorTest 5、FTestIF 3、fTrayMapping 1 |
| **合計** | **672** | 讀別的表單 628＋讀自己 44 |

讀得最多的 golden 檔：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp` 127（其中 `TfMain::Start` 30、`Timer3Timer` 22、`Timer1Timer` 14、`ScanKey` 6）、`Command.cpp` 82、`csystem.cpp` 46、`mymessbox.cpp` 38、`note.cpp` 26、`atester.cpp` 22、`ckernel.cpp` 21、`cContact.cpp` 18、`uhome.cpp` 17。

### 2.2 移植樹今天怎麼回答：400 處

同一個腳本掃移植樹（`bbe5e1fc`，排除 `tests\`、`docs\`、`tools\dfm2rc\`）：

| 類別 | 處數 | 今天的值 |
|---|---:|---|
| 已接總表（`W906_FShow`／`W906_FormFShow`） | **25** | 成員 或 總表政策版 |
| 直接讀成員（bare） | **224** | 見下表 |
| 在 `#if 0` 閘裡 | **151** | 不跑（其中約 33 處是 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp:15074` 起的 golden 原文副本，該判斷已由總表按表單名代算） |
| golden 有、移植樹找不到對應 | 約 272 | 所在的 golden 程式還沒翻、或已被別的機制取代（例 note.cpp／mymessbox.cpp 的對話框改走網頁對話框通道）。**概數**：golden 與移植樹檔名不一一對應（例 golden main.cpp 的 Start 在移植樹是 `WebStart.cpp`），672−400 只是差額 |

224 個直接讀成員的，依「誰開這個畫面」分五類：

| 類 | 處數 | 表單 | 今天讀到什麼 |
|---|---:|---|---|
| A 操作員從網頁開的視窗 | **69** | fContact 47、fiosetview 7、fSetup 5、fTeach 4、fShuttleMove 2、fMotorTest 1、fObserver 1、fTowerLight 1、fTestCategory 1 | 幾乎全部**恆為 false**（成員沒人設真）；例外 fSetup 在 C 路存讀時被暫時換成那一頁的值（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc:70`） |
| B C++ 自己開的對話框 | **72** | fNote 32、MyMessageBox 32（fShow 17＋Visible 15）、fPassword 3、fQwertyKey／fQwertyKey2 3、fDefrostNote 1、fShowBinSet 1 | 對的（C++ 自己設） |
| C Home Monitor | **21** | fHome | 對的（回原點程式自己設 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uhome.cpp:646`、`:652`）；但操作員從網頁開／關它，C++ 不知道 |
| D 網頁沒有視窗的表單 | **35** | Zteach 34、fLaserSensor 1 | 恆為 false（網頁打不開 ⇒ 照 golden 也是「沒開」） |
| E 表單讀自己（C 路頁的「這頁開著」、表單計時器） | **27** | C 路 gen.inc 13、LaserSensor／VacuumUnit／SetUp／Temp_Set 各 2、TrayMapping 2、Configuration／StartCondition／Precaution／TowerLight 各 1 | C 路頁：開站就真（Q49 問題）；其餘多半恆 false |

逐檔清單在 §5.3。

### 2.3 C++ 現有的一半：視窗總表

`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp`／`.h`（Jimmy `f21860e6`、`22b485bc`；EastSun `7414cc6f`）：
- 每條網頁連線記最後一份總表，**只增不減**（`:50-57`、`:157-161`）；join key 是 golden 表單名，form 是 null 的視窗略過（`:137-141`）。
- 狀態：`open`／`minimized`（算開）／`closed`／`never`，其他字串一律當「不知道」（`:64-72`）。
- 查一個表單：有新鮮回報就只看新鮮的、新鮮的之間取聯集（任何一個分頁說開就是開）；**全部過期（15 秒）才保守當成開**（`:171-223`、`:226-237`）。
- 政策版（`:365-377`）再加兩條使用者裁決：**從沒收過任何總表＝全部當關**（Q8-B，黏的，`:145-147`、`:354`）；**瀏覽器根本不會回報的 5 個表單＝當關**（Q20-甲，`:337-339`：FrmRotate、HandlerSystem、Zteach、fTrayMapping、TrayEditForm）。
- GPIB 設定中位元的三層表單清單（`:290-316`）。
- 收指令在 wb_serve 四個地方：主分派 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5820-5842`，以及三個「等操作員回答」的迴圈 `:631`、`:847`、`:6791`（等告警框回答時照收，不會因等太久而過期）；不需要單一操作員權杖（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.cpp:1410-1427`）。

**已知問題（Jimmy 已確認）**：瀏覽器全關 15 秒後，所有報過的表單都「過期＝當成開著」⇒ `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:30493-30494` 讓主流程暫停，沒有警報；打開網頁才繼續。筆電在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\NIGHT_REPORT.md:84`（第 36 題）列了 A～D 四個改法，A＝只改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebMotorAccessLive.cpp:1140-1143`。

**新查到的：「過期＝當成開著」對 START 也不一定比較安全。** 兩個例子（都已接總表）：
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp:1945-1950`：「Contact 與 Temp Set **都沒開**」才跳 MES1052 並**拒絕 START**。當成開著 ⇒ 這個拒絕被**跳過**。
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp:2120-2122`：「Contact 沒開」才檢查 Lot ID 長度。當成開著 ⇒ 檢查被跳過。
- 反例（當成開著比較擋得住）：主流程暫停 `csystem.cpp:30493`、設定中位元。
⇒ 不存在一個對所有判斷都安全的「不知道」預設值；**最安全的是讓「不知道」盡量不發生**（§3.4）。

### 2.4 網頁現有的一半

- 視窗定義表：`D:\HT9045\web\background.html:418-520`，**68 列**；47 列有 golden 表單名（含主畫面 fMain），21 列 form:null（主畫面頁籤、網頁自己的工具頁）。另有截圖視窗 `SHOT_WINDOWS` 接在後面（`:522`，form 不是識別字就送 null）。
- 四態：`D:\HT9045\web\background.html:576`、`:665-670`（狀態一變就 60 毫秒後送總表 `:845-848`）；開 `:746-770`、縮小 `:771-779`、關 `:780-803`；開站就顯示的是 open、預設隱藏的是 never（`:913`）。
- **沒建立的視窗不進總表**：`:829`。沒建立的原因：debug 專用（`:888`，例 HandlerSystem）、設定檔判定未安裝（`:890`，例 Bar Code、Ground Man 的 `onFalse:"hide"`）。⇒ C++ 那邊「沒人報過這個表單＝不知道＝開著」。
- 送出：開站送一次（`:1061`）、連上送一次（`:1066-1070`）、重連送一次（`:1076-1081`）、連著時每 5 秒心跳（`:1088-1092`）。通道 `HT9045Windows.put`（`D:\HT9045\web\page\ht9045_recipe_client.js:832-891`）。
- 通知頁面自己「你的視窗開了／關了」（`HT_WIN` 訊息）：`D:\HT9045\web\background.html:671-683`；iframe 載好補送一次（`:1196-1201`）。目前只有 Motor Test 頁（`D:\HT9045\web\page\HW.MotorTest.html:995-1019`）與 Cleaning（`D:\HT9045\web\page\ht9045_cleaning_c.js:197-205`）在聽；引擎 `D:\HT9045\web\page\ht9045_wire_engine.js` 沒聽。
- 引擎一接上就向 C++ 要資料：`D:\HT9045\web\page\ht9045_wire_engine.js:2046`（attach）→`:2108` `load();` →`:1298-1299`→C 路讀取 `:1188-1223`（Q49）。
- 多個 HMI 分頁：每個分頁各有一個外框、各送各的總表；C++ 以連線為單位存（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:157-161`）。
- **C++ → 網頁「請開／關這個視窗」今天沒有**：回原點時 `fHome->Show()`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uhome.cpp:637-646`）只改 C++ 的旗標，網頁的 Home Monitor 不會跳出來。golden 由程式自己開／關畫面的地方（V912 grep）：`uhome.cpp` fHome Show／Close、`csystem.cpp` fHome->Close()、fSpeed->Close()、fShowBinSet->ShowModal()、`Command.cpp` fSCKART->Show()、`Automation\SCK_ART.cpp` fSCKART Show／Close、`main.cpp` 閒置自動登出關 Offset（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:26043-26044`）等。

---

## 3. 設計

### 3.1 頁面表長什麼樣

一列一個畫面。**列表是寫死在 C++ 的**（編譯時就知道「總共有多少頁面」），由一個 ctest 對 `D:\HT9045\web\background.html` 的 WINDOWS 表核對（§6 T2），不一致就紅。

| 欄位（白話） | 程式名 | 例 |
|---|---|---|
| golden 表單名（fShow 的查詢鍵） | `form` | `fTeach`；網頁自己的頁是空字串 |
| 網頁視窗 id | `webId` | `teach`；C++ 對話框是 `cpp:fNote` 這種 |
| 誰會開這個畫面 | `opener` | 操作員（網頁）／程式（C++）／兩者都會／沒有網頁 |
| 現在的狀態 | `state` | 開、縮小（算開）、關、從沒開過（算關）、這台網頁沒建立（算關）、沒有網頁（永遠關） |
| 最後是誰改的、什麼時候 | `by`、`since` | 「HMI 連線 7，10:02:31」或「程式 uhome.cpp:646，10:05:00」 |
| 程式要求網頁開／關（還沒照做） | `want` | 回原點時 fHome＝請開 |
| 幾個 HMI 說它開著 | `openIn` | 2 |

列數（`bbe5e1fc`）：網頁視窗 68 列（47 有 golden 表單名＋21 沒有）＋golden 會讀、網頁沒有視窗的 22 個表單（fNote、MyMessageBox、fPassword、fPassword2、fInput、fQwertyKey、fQwertyKey2、fShowBinSet、fDefrostNote、MemoryAlarmForm、Zteach、FrmRotate、fTrayMapping、TrayEditForm、fFTPClient、FormBarcodeReader、fRFID、FormHS、fLiftCount、frmDTME08、fARMSLog、fLaserSensor）＝**90 列**。前 10 個是 C++ 自己開的對話框（opener＝程式），後 12 個是「沒有網頁」（永遠關，等於把 Q20-甲那 5 個的名單補齊、改成由這張表決定）。

### 3.2 誰寫

1. **網頁（操作員的動作）**：照舊用視窗總表 `ui.windows.put`，不加新指令。外框加三件小事（§4.3）：
   - 沒建立的視窗也列出來，狀態 `absent`（這台沒有這頁）。C++ 今天就把不認得的狀態當「不知道」而**不算開**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:64-72`、`:205-211`、`:236`）⇒ 視窗總表**不用改**就收得下。
   - 頁面要離開時（`pagehide`：F5、關分頁、關瀏覽器）送最後一份「全部關」。
   - 聽 C++ 回傳的 `ui.pages`，照 `want` 開／關視窗。
2. **C++（程式自己開關畫面）**：新函式「程式打開／關掉這個畫面」（`W906_PageProgramSet(表單名, 開或關, 出處)`），放在 golden `fXxx->Show()`／`Close()` 的移植位置旁（同一行附加）：回原點 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uhome.cpp:646`、`:652`；告警框／是否框的等待 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:7597`、`:7600`、`:7608`、`:7614`；其餘（fShowBinSet、fSCKART、自動登出關 Offset）等那段 golden 翻進來時照這個慣例做。
   - 在 `ht9045_sm` 函式庫裡的程式（例 uhome.cpp）拿不到 wb_serve 的東西，比照現有 `W906_FormFShowHook` 用函式指標（新 `W906_FormProgramShowHook`，宣告在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.h:421` 同一行、定義在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:30040` 同一行）；沒安裝（ctest）＝什麼都不做，行為與今天相同。

### 3.3 怎麼回傳給網頁、兩邊怎麼對得上

- C++ 把整張表送成一個字串 tag `ui.pages`（JSON：`{seq, count, rows:[{form, id, state, by, want, openIn}]}`），在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp` 用現成的 `stageStr`（`:179`）送；表有變才變，tag 通道本來就只送變化。`WebBridgeTags.cpp` 只編在 wb_serve（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CMakeLists.txt:3316`），頁面表也在 wb_serve，直接呼叫。
- 外框收到後做三件事，**只做這三件**（安全判斷仍在 C++）：
  1. 某列 `want＝開` 而本分頁那個視窗沒開 ⇒ `openWin(id)`（走原本的開窗政策 `D:\HT9045\web\background.html:746-770`）。例：按 HOME ALL → 每個連著的 HMI 都跳出 Home Monitor。
  2. `want＝關` 而本分頁開著 ⇒ `closeWin(id)`。例：golden 自動登出關 Offset。
  3. 本分頁 WINDOWS 裡有、C++ 表裡沒有（或反過來）⇒ console 印一次「頁面表不一致」並在總表訊框帶 `mismatch` 欄（C++ 也印一行）。這就是「總共有多少頁面」兩邊互相核對。
- 網頁照做之後會送出新的總表，C++ 看到該列真的變成開／關，就把 `want` 清掉（不會來回打架）。
- 多個 HMI 分頁：`want` 每個分頁都照做（golden 只有一個螢幕，多一個分頁也跳出 Home Monitor 無害）；「開著」取聯集（任何一個分頁開著就算開著，沿用今天的規則）。

### 3.4 fShow 改讀頁面表：一個函式、一套規則

**單一函式**：`W906_FormShowing(golden 表單名, 成員值)` ⇒ 回答「這個畫面現在開著嗎」。
- 放在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:30046` 旁（同一行；今天的 `W906_FormFShow` 改成呼叫它，現有 7 個呼叫點不用動）；另開一支只有兩行宣告的小標頭 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\W906FormShowing.h`，讓 atester、ainarm、AutoClean 這些不 include `csystem.h` 的檔在**同一行**附加 include 就能用。
- wb_serve 之下，答案來自頁面表（透過既有的 `W906_FormFShowHook`：把 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebMotorAccessLive.cpp:1142` 那一行改成問頁面表）；ctest 之下沒有 hook ⇒ 回傳成員值，**與今天完全相同**。
- `WebStart.cpp` 的 `W906_FShow`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp:212`）改成呼叫 `W906_FormShowing`；GPIB 設定中位元（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:888-890`）改成頁面表版的三層判斷（清單照舊從 `WebWindowRegistryTierForms` 拿，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:438-454`）；St01 的開關邊緣取樣（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp:82-88`）改問頁面表。⇒ **4 處呼叫端，今天已接總表的 25 個判斷＋主流程暫停＋設定中位元＋S122 一起轉過來。**

**回答規則**（依序，第一條成立就回答）。★＝與今天不同：

| # | 情況 | 回答 | 例 |
|---|---|---|---|
| 1 | 這列是「程式開」的（對話框、fHome 的程式那一半） | 成員值 **或** 表上程式狀態 | 告警框等回答中 ⇒ 開 |
| 2 | 這列是「沒有網頁」 | 關 | Zteach（網頁打不開 ⇒ golden 的「沒開」） |
| 3 | 從來沒收過任何網頁的總表（開機、沒開網頁；Q8-B） | 關 | 開機後直接用面板 START |
| 4 | 有 HMI 新鮮地（15 秒內）報了這個表單 | 任何一個說開／縮小 ⇒ 開；都說關／沒建立 ⇒ 關 | 甲分頁 Teach 開、乙分頁沒開 ⇒ 開 |
| 5 ★ | 連著的 HMI 都沒提到這個表單 | 關（今天：不知道＝開） | 這台沒裝 Bar Code、網頁沒建立那個視窗 |
| 6 ★ | 只剩過期的回報，但**還有網頁連著**（WebSocket 數 > 0） | 照最後一次回報（今天：一律開） | 分頁切到背景、瀏覽器把心跳節流成一分鐘一次：Teach 開著就還算開著 |
| 7 ★ | 只剩過期的回報，而且**沒有任何網頁連著** | 關（今天：一律開） | 夜班把瀏覽器全關 ⇒ 主流程不再暫停 |

- 「有沒有網頁連著」用 wb_serve 現成的 `LiveWebSocketCount()`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.h:247`，Jimmy 0926 為了「沒網頁就自動開瀏覽器」加的，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:7512` 已在用）。正常關掉的連線立刻減掉；網路斷掉沒收到關閉的，最多等 45 秒（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.h:94-95`，ping 15 秒、無回應 45 秒斷）。頁面表不去改 WebBridgeServer，由 wb_serve 開機時把這個函式交給頁面表（同一行）。
- F5／關分頁：外框在 `pagehide` 送「全部關」⇒ 規則 4 立刻生效，不用等 15 秒。沒送到（瀏覽器當掉）⇒ 退到規則 6／7。
- ★ 5、6、7 **改掉視窗總表契約 §6「不知道／過期＝當成開著」**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.h:27-36`），也就是筆電第 36 題的 B 案的延伸。理由：§2.3 那兩個 START 例子證明「當成開著」並不總是比較擋得住；而新規則下「不知道」只剩「連著但暫時沒送」這一種，這時照最後一次回報就是最接近事實的答案。**這要 Steven 決定，也要知會 Jimmy／EastSun（§8 Q-P1）**。視窗總表本身（`WebWindowRegistryFShowPolicy`）不改，照舊給它自己的 ctest 用。

**安全方向（寫給審查的人）**：
- 互鎖、START、運動的判斷仍全部在 C++；網頁只回報事實、只照 `want` 開關視窗，不算「能不能 START」（沿用契約 §9，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.h:20-25`）。
- 新規則下「網頁全關 ⇒ Teach 算關」：Teach 的操作指令本來就只能從那條已經不在的連線來，沒有人能在 Teach 裡動機台；Teach 關掉時 S122 照 golden 標成「要重新回原點」（停機中才清，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.h` 開頭），下一次 START 先回原點。golden 也不能在運轉中打開 Teach（NIGHT_REPORT 第 36 題引 906 `main.cpp:27827`，本次未重驗）。
- 人員安全靠安全門／急停互鎖，不靠畫面旗標；本設計不碰那些。

### 3.5 FormShow／FormClose 對到頁面表的開／關那一下

頁面表每一拍（wb_serve 主迴圈 500 毫秒，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5953` St01 那一行已經在跑 S122）比對上一拍，得到「某頁從關變開」「從開變關」兩種邊緣。**在主迴圈跑、不在收指令時跑**：`ui.windows.put` 在「等告警框回答」的迴圈裡也會收（`:631`、`:847`、`:6791`），在那裡直接跑 golden 關頁程式會重入。

| 邊緣 | 做什麼 | 對到 golden |
|---|---|---|
| Teach／Motor Test 開或關 | S122 照舊（清「要重新回原點」） | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uteach.cpp:1610`、`main.cpp:28846-28847` |
| 任何登記的表單開或關 | `W906_WindowEdgeRegister` 照舊（取樣改問頁面表；第一個用的是 St02 的 Tester I/F 關頁 FormClose） | 例 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTesterIF.cpp:1215-1235` |
| C 路頁「關」 | 把那頁「開過了」清掉（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:26-30` 的 `shown=false`），有登記 golden 關頁程式的順便跑 | golden 每次關都跑 FormClose |
| C 路頁「開」 | **網頁那一頁自己**在收到「你的視窗開了」時才向 C++ 要資料（Q49 D，§4.3 H4）⇒ C++ 在那一刻跑 golden 開頁、查權限、記「Enter …」 | golden 每按一次開窗鈕：查權限、記 Enter（例 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28694`）、FormShow |
| fHome 被操作員關掉 | 跑移植的 `TfHome::Close()`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uhome.cpp:650-652`）⇒ 回原點照 golden 停下（`if(fHome->fShow==false) SoftStop`） | golden 關 Home Monitor 就是 FormClose |
| Tray Edit 被操作員在 HMI 直接關掉（20260930 `e6e537c2`，St02 S-10 認領 3.7／3.8） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp:463` 同一行：TrayEditForm 叫 `W906_TrayEditWindowClosedHook`（St02 的 `TrayEditForm.cpp` WindowClosed＝golden Cancel：Close → FormClose，什麼都不寫）；指標在 `:630`，只有 wb_serve 連 TrayEditForm.cpp 時才設，測試裡是 0 ⇒ 走原本的 printf。其他 kPgBoth 表單照舊 printf（還沒接 FormClose） | golden 的 Tray Edit 是 ShowModal，不會自己不見；視窗沒了唯一的出口就是 Cancel（SpeedButton2Click） |

⇒ R108（Enter 記在開站）、R110（Configuration 存完多記一筆）、Teach 開 HMI 就清回原點旗標、Contact 開頁換算（R84）都自然對到「按開窗那一下」。**開窗時查權限、記 Enter 由 C 路讀取那一條負責，不在邊緣裡再做一次**（避免記兩筆）。

### 3.6 放在哪：新 St01 模組 `WebPageTable`（建議），不擴充視窗總表

| | 新模組（建議） | 擴充 `WebWindowRegistry.cpp` |
|---|---|---|
| 做法 | 新檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp`／`.h`：90 列表、程式寫入、回答規則、邊緣、`ui.pages` JSON；網頁的回報照舊由視窗總表解析，頁面表用它現成的查詢函式讀（`WebWindowRegistryQuery`、`EverAnyFrame`、`TierForms`） | 在總表檔裡加程式寫入、`absent`、連線數、回傳 JSON，改 `FShowPolicy` 的過期規則 |
| 改別人的檔 | 視窗總表 **0 行**；別人的檔只有 4 行呼叫端＋同一行附加 | Jimmy／EastSun 的檔大改；他們的 ctest `test_winregistry` 很多斷言要改 |
| 權威分工 | 總表＝「網頁說了什麼」（契約層，跟 Jimmy 對）；頁面表＝「C++ 怎麼用它」（政策層，跟 Steven 對）——總表檔頭自己就是這樣分的（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.h:116-124`） | 兩種權威混在一個檔 |
| 風險 | 多一層；要確保沒有人再直接呼叫 `WebWindowRegistryFShowPolicy`（稽核工具會抓，§6 T3） | 改動面大、跟 Jimmy 撞檔 |

---

## 4. 要改的檔與行（主人、認領）

改之前在 `FROM_STEVEN.md` §1 登記**確切的行**（共用檔區段規則：不同段就直接做，碰到別人登記的段才等）。一律「同一行附加」或「檔尾附加」，不移動別人的行號。

### 4.1 C++ 新檔（St01）

| 檔 | 內容 | 大小（估） |
|---|---|---|
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.h`／`.cpp` | 90 列表、回答規則（§3.4）、程式寫入、邊緣、`ui.pages` JSON、連線數函式的安裝座；只依賴 WebWindowRegistry 與標準庫（不 include cmydef.h，ctest 秒級） | 約 350 行 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\W906FormShowing.h` | 兩行宣告：`W906_FormShowing`、`W906_FormProgramShowHook` | 約 20 行 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_pagetable.cpp` | §6 T1 | 約 300 行 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\fshow_audit.py` | 把 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\p6b_fshow_audit.py`（只看 WebStart.cpp）擴成全樹＋棘輪（§6 T3） | 約 200 行 |

### 4.2 C++ 既有檔（呼叫端與安裝，全部同一行）

| 檔:行 | 主人 | 改什麼 |
|---|---|---|
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CMakeLists.txt:3368` | St01 的行 | 同一行加 `WebPageTable.cpp` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt` 檔尾 | St01 | 加 `test_pagetable`、`FShow_Audit` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:30040`、`:30046-30049` | EastSun（0926 已放行，`D:\HT9045_handoff\TO_STEVEN.md` §1 表格 EastSun 那一列（檔案第 26 行）已劃掉） | 同一行加 `W906_FormProgramShowHook` 定義；`W906_FormFShow` 轉呼叫 `W906_FormShowing` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.h:421`、`:440` | 同上 | 同一行加宣告 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebMotorAccessLive.cpp:1142` | EastSun | 改問頁面表（**就是筆電第 36 題 A 案那一行**） |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp:212` | Jimmy | 改呼叫 `W906_FormShowing` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:888-890` | Jimmy（P6-b） | 設定中位元改頁面表版 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4389` | 共用行（安裝 hook） | 同一行附加：安裝頁面表的 hook、交連線數函式 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5953` | St01 的行 | 同一行附加：頁面表每拍（邊緣） |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:7597`、`:7600`、`:7608`、`:7614` | Jimmy（等回答的迴圈，0926 已放行） | 同一行附加 `W906_PageProgramSet`（告警框、是否框） |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uhome.cpp:646`、`:652` | EastSun（已放行） | 同一行附加程式寫入（Home Monitor） |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp:82-88` | St01 | 取樣改問頁面表 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp` 檔尾＋`:211` 附近 | St01 | C 路頁關邊緣清 `shown` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp:1263` | 共用（W906-GUARD-TAGS 那一段） | 同一行附加 `stageStr(snap,"ui.pages",…)` |

### 4.3 網頁（最小改動）

| 代號 | 檔:行 | 主人 | 改什麼 | 大小 |
|---|---|---|---|---|
| H1 | `D:\HT9045\web\background.html:829` | 共用（總表段是 Steven 20260918 寫的；MODAL_POLICY 那段是 Jimmy 的，不碰） | 沒建立的視窗改送 `{form, state:'absent'}`，不再略過 | 約 3 行 |
| H2 | `D:\HT9045\web\background.html:1092` 之後 | 同上 | `pagehide` 送最後一份「全部關」（`bye:true`） | 約 8 行 |
| H3 | `D:\HT9045\web\background.html:1071-1072` 附近（已有 `HT9045Tags.on` 的地方） | 同上 | 聽 `ui.pages`：照 `want` 開／關、核對列數 | 約 25 行 |
| H4 | `D:\HT9045\web\page\ht9045_wire_engine.js:2108` | **Jimmy 登記中**（`D:\HT9045_handoff\TO_STEVEN.md` §1 第 13 條那一列，筆電還在改） | 嵌在外框裡就先不 `load()`，等 `HT_WIN` 說「開了」才要；3 秒沒收到（舊外框、單獨開頁）照舊直接要 | 約 25 行 |

H4 是 Q49 的 D；Steven 說 Jimmy 的檔現在可以由我們改，但這一列筆電正在改，**要先在 FROM_STEVEN 認領 `:2046-2108` 這一段、等筆電回一聲**再動。H1～H3 不用等人。

### 4.4 fShow 讀取點分批（呼叫端清單）

每一處都是同一行把 `X->fShow` 換成 `W906_FormShowing("X", X->fShow)`（成員值照傳，C++ 自己開的畫面不會因此失真）。

| 批 | 內容 | 處數 | 值會不會變 |
|---|---|---:|---|
| 0 | 已接總表的 25 處＋主流程暫停＋設定中位元＋S122：只改 §4.2 那 4 個呼叫端 | 25（呼叫端 4） | 會：規則 5～7 |
| 1 | 操作員開的視窗、在主要機台迴圈裡（清單 §5.3 批 1） | 26 | **會**：今天恆 false，接上後照 golden |
| 2 | 操作員開的視窗、在製程／功能模組裡（清單 §5.3 批 2） | 43 | **會** |
| 3 | Home Monitor | 21 | 只有「操作員從網頁開／關」那一半會變 |
| 4 | C++ 自己開的對話框 | 72 | 不會（成員已經對）；只為了統一，可最後做或不做 |
| 5 | 網頁沒有視窗的表單（Zteach 34、fLaserSensor 1） | 35 | 不會（照舊關）；機械式替換 |
| 6 | 表單讀自己（C 路頁 13＋表單計時器 14） | 27 | 跟 H4 一起做：C 路頁的「這頁開著」改成真的只在視窗開著時為真 |
| 7 | `#if 0` 閘裡的 | 151 | 解閘的人照這個函式寫；稽核工具（§6 T3）保證新解的沒漏 |

---

## 5. 附表

### 5.1 golden 會自己開／關畫面的地方（C++ → 網頁 `want` 的來源，V912 grep 計數）

`uhome.cpp` fHome->Show() 1、fHome->Close() 1；`csystem.cpp` fHome->Close() 1、fSpeed->Close() 1、fShowBinSet->ShowModal() 1；`Command.cpp` fSCKART->Show() 1；`Automation\SCK_ART.cpp` fSCKART Show 1／Close 1；`RPDefault.cpp` fSpeed Show／Close 各 3、fCleaning Show／Close 各 3；`note.cpp` fCCLink->Show() 1；`main.cpp` 閒置自動登出 fOffSet->Close()（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:26043-26044`）。其餘 Show／ShowModal 都是按鈕處理常式（操作員動作，走網頁）。移植樹今天只有 fHome（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uhome.cpp:637-652`）翻了；其他在翻到時照 §3.2 做。

### 5.2 已接總表的 25 處（批 0）

- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp`：1626（TrayEditForm）、1945（fContact、fTemp_Set）、2108、2120、2150、2788（fContact）、3034-3037（fContact、fTemp_Set、Zteach、fShuttleMove）、3314、3626-3627、3645、3681、3705、3707、3780。共 19。
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:30098-30099`（VerifyMotorAction）、`:30493-30494`（主流程暫停）。共 4。
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Handler\HandlerBridgeCtl.cpp:1226`（FTestIF）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain_ATCSiteUse.cpp:117`（fTemp_Set）。共 2。

### 5.3 直接讀成員的 224 處（依批、依檔；行號都在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` 底下）

**批 1（26）**：
`csystem.cpp`：9034（fShuttleMove、fContact）、14066（fContact）、31186、31267（fContact）、31406（fTeach）、31454（fShuttleMove）、31580（fSetup）、32861（fContact）；
`ckernel.cpp`：237、306、1106、1176（fContact）、877（fSetup）、1690（fTowerLight）；
`Command.cpp:10261`（fSetup、fContact；同一行的 fHome 算批 3）；
`cinitial.cpp:13604`（fContact）；`uruncontrol.cpp:184`（fContact）；
`TesterComm\Handler\HandlerGpibMsg.cpp`：706（fiosetview）、707（fMotorTest）、708（fTeach）、1256、1285（fContact）；
`handlerlog.cpp:343`（fSetup）；`FileRW\TestIF_File_SetUp.gen.inc:70`（fSetup，C 路暫存交換，跟批 6 一起看）。

**批 2（43）**：
`uYieldMonitoring.cpp`：205、375、608、846、1105、1568、1859、1967、2048（fContact）；
`atester.cpp`：1521、2970；`atester_ProcessCount.cpp:582`；`aTester_Front.cpp`：6685、6730；`aTester_Rear.cpp`：6800、6844；
`ainarm9045.cpp`：6312、10177、10546；`asendic_Loader.cpp`：1137、1320；
`AutoClean\AutoClean.cpp`：1386、1428、4745、8255；`BarCode\BarCode_Shuttle2_CCDScan.cpp`：2100、2218；
`PowerSavingMode.cpp:611`；`ATC\ATCInterface.cpp:2236`；`uTemp_Set.cpp:2681`；`FileRW\Temperature.gen.inc:3511`；`cShowBinSelect.cpp:511`；`FileRW\MainRecord.cpp:167`（以上 fContact）；
`TempCtrl\TriTemp.cpp`：577、790（fTeach 的替身 `W7TT_FTeach`）、3058、3183、3192、3206、3219、3719（fiosetview 的替身 `W7TT_Fiosetview`）——⚠ 這幾個讀的是 TriTemp.cpp 自己的替身物件（`:479-480`），不是真的 facade，要換成函式才讀得到網頁；
`FileRW\MainRecord.cpp:324`（fObserver）；`cTestCategory.cpp:648`（fTestCategory）。

**批 3（21）**：`uhome.cpp`：2153、2162、2187、2480、2670、2777、2881、3258、3267、3280、3308、3412、3444、3454、4166；`ckernel.cpp`：1644、2078；`csystem.cpp`：30963、32715；`cShowBinSelect.cpp:2351`；`Command.cpp:10261`（fHome）。

**批 4（72，對話框）**：`Command.cpp` 3012、9891、9947、14992、15131、16336、16337、17708、17922、17935；`ckernel.cpp` 1608、1629（×2，含 fShowBinSet）、1974、1976、2849、3252、3283、3475、3506；`csystem.cpp` 13842、30652、30653、32443、32609、32741（×2）、32863；`SECSGEM\uHGemHT9045.cpp` 4841、5681、6455、6464、6532（×2）、7684、7938；`Motor\mymotor.cpp` 4296、4315、4332、5198、5217、5235；`acatchtray.cpp` 938、5511、5563、5650、5721、7207、7214；`asendic_Color.cpp:1511`；`asendic_Empty.cpp:1174`；`aTester_Front.cpp` 3258、4271；`aTester_Rear.cpp` 3142、4123；`FileRW\MainRecord.cpp` 193、207；`TesterComm\Handler\HandlerGpibMsg.cpp` 704、705、861；`tools\wb_serve.cpp` 6859、6864、6949、7577、7578、7630（×2）；`uhome.cpp:5080`；`forms\fPassword.cpp:75`；`forms\fQwertyKey.cpp` 168、170；`TempCtrl\TriTemp.cpp:3465`（fDefrostNote）。

**批 5（35）**：Zteach——`ainarm9045S_1x4_4.cpp:88`、`ainarm9045S_2x4_4_13.cpp:146`、`ainarm9045_1x2_2.cpp` 152、202、`ainarm9045_1x2_2_14.cpp` 90、140、`ainarm9045_1x3_2_14.cpp:104`、`ainarm9045_1x3_4.cpp:128`、`ainarm9045_1x4_2.cpp:162`、`ainarm9045_1x4_4.cpp:249`、`ainarm9045_1x4_4_Back.cpp:73`、`ainarm9045_1x4_8_Hot.cpp:125`、`ainarm9045_2x1_2.cpp:117`、`ainarm9045_2x2_4.cpp:192`、`ainarm9045_2x2_4_12.cpp:85`、`ainarm9045_2x2_4_14.cpp:103`、`ainarm9045_2x3_6.cpp:135`、`ainarm9045_2x3_6_14.cpp:137`、`ainarm9045_2x4_4.cpp:192`、`ainarm9045_2x4_8.cpp:156`、`ainarm9045_2x5_8.cpp:99`、`ainarm9045_2x6_8.cpp:141`、`ainarm9045_2x8_8.cpp` 203、652、656、667、671、`cinitial.cpp:13636`、`ckernel.cpp:1153`、`csystem.cpp` 9034、31365、31406、`uHeaterThread.cpp` 1643、2003；fLaserSensor——`csystem.cpp:31481`。

**批 6（27）**：`FileRW\DeviceForm_File.gen.inc` 2684、3620、3630、3647、3926；`FileRW\IniConfig.gen.inc:9567`；`FileRW\Temperature.gen.inc` 6368、6377；`FileRW\TestIF_File_Cleaning.gen.inc` 2551、2687；`FileRW\TestIF_File_SetUp.gen.inc:70`；`FileRW\TestIF_File_TesterIF.gen.inc:1743`；`FileRW\TestIF_File_VacuumUnit.gen.inc:383`；`OmronLaser\LaserSensor.cpp` 1312、1849；`VacuumUnit\VacuumUnit.cpp` 392、427；`cSetUp.cpp` 1075、1120；`uTemp_Set.cpp` 4481、6610；`cConfiguration.cpp:6396`；`cStartCondition.cpp:746`；`forms\fPrecaution.cpp:34`；`forms\fTowerLight.cpp:205`；`forms\fTrayMapping.cpp` 507、520。

**批 7（151，`#if 0` 裡）**：`Command.cpp` 73、`atester.cpp` 20、`csystem.cpp` 17、`WebStart.cpp` 10、`SECSGEM\uHGemHT9045.cpp` 4、`bthermo.cpp` 4、`BarcodeReader.cpp` 3，其餘 15 個檔各 1～2。

---

## 6. 測試（這台機器不開 wb_serve 就驗得到的，與要上機的）

| 代號 | 內容 | 在哪跑 |
|---|---|---|
| T1 | `test_pagetable`（只連 `WebPageTable.cpp`＋`WebWindowRegistry.cpp`＋cJSON，秒級）：①列數＝90、每列欄位；②§3.4 規則 1～7 各一格（連線數用假函式、過期用現成的 `WebWindowRegistryAgeConnForTest`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.h:221`）；③`absent`＝關；④兩個 HMI 聯集；⑤F5 順序（全部關的離開訊框→新頁總表）；⑥程式開→`want`→網頁回報開→`want` 清掉；⑦程式開的列＝成員 或 表；⑧邊緣：開、關、過期但還連著不算邊緣、網頁全走算一次關；⑨設定中位元：新鮮時與 `WebWindowRegistryDiagnosticsOpen` 結果相同；⑩`ui.pages` JSON 形狀 | 本機 ctest |
| T2 | 列表對表：解析 `D:\HT9045\web\background.html` 的 WINDOWS，47 個 golden 表單名＋21 個 null 必須跟 C++ 表的網頁列一模一樣；Q20-甲那 5 個必須在「沒有網頁」或「debug 專用」列 | 本機 ctest（Python） |
| T3 | 稽核棘輪 `FShow_Audit`：全樹抓 `->fShow`／`->bShow`／表單的 `->Visible`／`->Showing`（沿用 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\p6b_fshow_audit.py` 的 `#if` 堆疊與巨集規則），每檔「直接讀成員」數只能減不能增；另抓「除了頁面表以外有人直接呼叫 `WebWindowRegistryFShowPolicy`」 | 本機 ctest |
| T4 | 既有 ctest 比對失敗清單：`test_winregistry`（不動）、`test_teachleave`（取樣改問頁面表後，[17]「全部過期一律當開」那格要改寫成新規則 6／7，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_teachleave.cpp:316-327`）、`test_mt_e3b_engine`（hook 形狀不變） | 本機 build gate |
| T5 | 網頁：H1 產生的總表、H3 的 `want` 處理用 node 跑外框的純函式（`D:\HT9045\web\tests\` 已有測試資料夾，框架**未查**） | 本機（若有 node） |
| T6 | 上機：開／縮小／關、F5、兩個分頁、關掉整個瀏覽器、回原點時 Home Monitor 自動跳出、告警框等待中照收、C 路頁開窗才讀資料與 Enter；wb_serve console 會印每個邊緣與 `ui.pages` 變化 | St02（測試介面機）或沙盒（`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\wbserve-sandbox-run.md`） |

---

## 7. 工作量與順序

| 順序 | 內容 | 人天（估） | 誰 |
|---|---|---:|---|
| 1 | 頁面表核心＋T1／T2＋批 0 四個呼叫端＋H1／H2（解掉全關停機、F5、沒建立當開） | 2～2.5 | St01 |
| 2 | 程式寫入（fHome、告警框、是否框）＋`ui.pages` tag＋H3（程式開的畫面在網頁跳出） | 1 | St01 |
| 3 | 稽核棘輪 T3＋批 1 | 1 | St01 |
| 4 | 批 2、批 3、批 5 | 1 | St01 |
| 5 | H4（引擎開窗才讀）＋批 6＋C 路頁關邊緣（Q49 D／R108／R110） | 1～1.5 | St01（引擎那段先跟筆電對） |
| 6 | 批 4（統一，可不做） | 0.5 | St01 |
| 7 | 批 7 隨各閘解開 | — | 解閘的人 |
| | **合計** | **約 6.5～7.5** | |

第 1 步做完就可以上機驗；每一步收尾都跑全新 build 目錄的 gate 並比對失敗清單。

---

## 8. 要 Steven 決定的題目

### Q-P1. 網頁全部關掉、或暫時沒回報的時候，C++ 要把畫面當成「開著」還是「關著」

**背景**：
- 今天的規則是「15 秒沒收到網頁回報＝當成畫面還開著」（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:226-237`，Jimmy 的契約 §6）。結果：瀏覽器全關 15 秒，主流程以為 Teach／Motor Test 開著而暫停，沒有警報（Jimmy 已確認；筆電列在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\NIGHT_REPORT.md:84` 第 36 題）。
- 而且「當成開著」不一定比較安全：START 有兩個檢查是「Contact 頁沒開才檢查」，當成開著反而**跳過**檢查（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp:1945-1950` MES1052 拒絕 START、`:2120-2122` Lot ID 長度）。
- 新的頁面表可以知道「有沒有任何網頁連著」（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.h:247`），網頁離開時也會先送「全部關」。
- 設計全文：`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md` §3.4。

**選項**：
- **A** 照看得到的事實：沒有任何網頁連著＝所有網頁畫面都關；還連著但暫時沒送＝照最後一次；網頁說這台沒有那一頁＝關。
- **B** 維持現在：15 秒沒收到＝全部當成開著，直到有網頁連上。
- **C** 分兩種答案：START 用的照 B、其他（主流程暫停、Contact 頁的製程判斷…）照 A。

**St01建議**：**A**。它同時解掉第 36 題與 START 那兩個被跳過的檢查；「不知道」只剩「連著但沒送」，照最後一次就是最接近事實的答案。B 會一直有第 36 題；C 讓同一個畫面在同一時刻有兩個答案，跟「一個陣列取代 fShow」相反。選 A 要知會 Jimmy／EastSun（改的是他們契約的那一格，但不改他們的檔）。

**例子**：21:00 機台運轉中、Teach 和 Motor Test 都關著，夜班把所有瀏覽器關掉去吃飯，22:00 回來。
- A：機台一直照跑。
- B：21:00:15 主流程暫停、沒有警報；22:00 打開網頁後自己繼續。
- C：機台照跑；但 22:00 前若有人用面板或 GPIB 按 START，START 把 Contact 等頁當成開著 ⇒ 例如 ASE 高雄的「Operator 等級不能離線 START」（MES1052）不檢查就放行。

**目前狀態**：B（現況）；等 Steven 決定。

### Q-P2. 程式自己打開／關掉的畫面，網頁要不要跟著自動開／關

**背景**：golden 有幾個地方是程式自己叫出或關掉畫面，不是操作員按的：按 HOME ALL 時回原點程式叫出 Home Monitor 讓人看進度（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uhome.cpp:637-646`，golden `uhome.cpp:4844-4849`）、閒置自動登出時關掉 Offset（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:26043-26044`）、跳 Site Map 確認框等。網頁今天完全不知道這些事——Home Monitor 不會自己跳出來。

**選項**：
- **A** 每個連著的 HMI 都自動開／關那個視窗（照 golden）。
- **B** 不自動開，只在工作列那顆按鈕閃爍，操作員自己點開；自動關照做。
- **C** 都不做（維持現在）。

**St01建議**：**A**（照 golden；兩個分頁都跳出 Home Monitor 無害）。

**例子**：操作員按 HOME ALL。
- A：兩秒內每個 HMI 都跳出 Home Monitor，看得到各軸回原點進度，跟 BCB 版一樣；中途把它關掉 ⇒ 照 golden 停止回原點。
- B：工作列「Home Monitor」閃爍，點了才看得到；沒點也照回原點。
- C：什麼都不出現，操作員要自己去選單開。

**目前狀態**：C（現況）；等 Steven 決定。

### Q-P3. 接上之後，今天「永遠當成沒開」的 golden 判斷會開始生效——要直接生效，還是先只記錄

**背景**：移植樹有 69 個 golden 判斷讀的是「操作員從網頁開的畫面」，但那個旗標從來沒被設成真，所以今天一律當成「沒開」（§2.2 A 類）。接到頁面表之後它們會照 BCB 生效。例：Contact 頁開著時不報低良率（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uYieldMonitoring.cpp:205` 等 9 處）、不進省電模式（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\PowerSavingMode.cpp:611`）、遠端控制回「設定中」而不執行（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp:10261`、`:16336`）、Shuttle Maintain／Contact 開著不做 auto decay（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:9034`）。都是 BCB 本來的行為，但對用慣網頁版的人來說是「變了」。

**選項**：
- **A** 直接生效（照 golden）。
- **B** 先「只記錄不生效」一天：C++ 照舊用今天的值，但每次新答案跟舊答案不同就印一行（哪個檔哪一行、哪個畫面），看過紀錄再切成 A。
- **C** 只接主流程那幾個（批 1），製程模組（批 2）等各模組的人自己接。

**St01建議**：**B 再 A**：批 1、批 2 先跑一天只記錄，紀錄給 Steven 看過就切 A。多約半天工作量。

**例子**：工程師開著 Contact 頁調高度，這時某個 site 良率掉到門檻以下。
- A：不跳低良率警報（跟 BCB 一樣）。
- B：照舊跳警報，但 console 多一行「uYieldMonitoring.cpp:1105 新答案＝Contact 開著（會不跳警報）」。
- C：跳不跳看那個模組接了沒。

**目前狀態**：尚未接（等於 C 的「都還沒接」）；等 Steven 決定。

### 已決定、不另問

- **Q49（設定頁開窗的時機）**：ST01-M 註記 Steven 裁 Q51 時已等於 B＋D、由我們做（`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md:187`）。本設計照這個做（§3.5、§4.3 H4）。ST01-E 確認後即可登記為已決定。
- **Q51-1～5（分頁標籤細節）**：Steven 說「頁面」是整個畫面不是分頁；分頁細節等本設計落地後再套進同一張表（`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md:227`）。

---

## 9. 風險與沒做的

1. **行為會變（批 1、2）**：見 Q-P3。`iContactMode` 在這支程式今天恆為 0（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:406-409` 的量測註解），所以「Contact 頁開著且非一般模式」那一類今天仍不會成立；會變的是「Contact 頁沒開才做」那一類。
2. **改了 Jimmy 契約的一格（規則 5～7）**：見 Q-P1；檔不動、ctest 不動，但要在 FROM_STEVEN 知會。
3. **`pagehide` 不保證送得到**（瀏覽器當掉）：退到規則 6／7；網路斷而沒收到關閉的，最多 45 秒才算沒有網頁。
4. **靜態函式庫的連結陷阱**：`W906_FormShowing` 放在 `csystem.cpp`（`ht9045_sm`），別的函式庫的檔呼叫它；今天 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Handler\HandlerBridgeCtl.cpp:75` 已經這樣呼叫 `W906_FormFShow` 並連得過，但每一批做完仍要用 `nm` 驗 hook 真的是那一支（KNOWLEDGE「build 綠證明不了接上了」）。
5. **替身物件**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TempCtrl\TriTemp.cpp:479-480`（fTeach／fiosetview 的替身）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\atester_shims.cpp:297`、`:379`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\acatchtray_shims.cpp:48`、`:103`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\aHotPlateSubstrate.h:1170` 各有自己的 `fShow`；換成函式時傳表單名就好，不要去改替身本身。
6. **多人同檔**：csystem.cpp、Command.cpp、ckernel.cpp、wb_serve.cpp 都有人在改；全部同一行替換、不增減行，改前在 FROM_STEVEN §1 登記行號。
7. **執行緒**：總表的寫（`ui.windows.put`）與讀（主流程）都在 wb_serve 那一條主迴圈執行緒（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\canary_support.h:157-165` 的說明）；頁面表沿用，另加一把鎖保護 `ui.pages` 的組字串（便宜、防將來）。
8. **V912 與移植樹 golden 基準不同**：例 V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:6231` 多了 `fTeach->fShow==false` 一條，移植樹對應的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp:3034-3037` 沒有（照的是 906 golden）。本設計只換「怎麼回答」，不補 V912 的差異。
9. **沒做**：批 7（`#if 0` 裡的 151 處）不在本設計範圍，由棘輪保證解閘時用新函式；網頁「主畫面頁籤」（form:null 的 21 列）只記開關、不參與 fShow。

---

## 10. 沒查證／推論的地方

- 672／400／224／151／25 是腳本數的，規則在 §2.1；「golden 有、移植樹沒有」約 272 是差額，不是逐一比對。
- 「今天恆為 false」是依寫入點盤點（全樹找 `fContact->fShow =` 等的寫入）推的，沒有執行驗證。
- `LiveWebSocketCount()` 會把 PCIE-1203 頁、Event Log 頁等別的 WebSocket 也算進去；外框關掉但那些頁還連著的情況（它們都在外框的 iframe 裡，理論上一起關）沒實測。
- 瀏覽器在背景分頁把 5 秒心跳節流到多慢，沒在機台的 Edge 上量。
- `ui.pages` 字串 tag 的大小（90 列約 4 KB）對 tag 通道的影響沒量；tag 快照是否在主迴圈執行緒組，沒逐行驗。
- `D:\HT9045\web\tests\` 的測試框架沒查。
- golden 閒置自動登出、Site Map 確認框在移植樹的翻譯狀態沒逐一查。

## 11. 指令紀錄（20260928，全部唯讀）

- `git -C D:\HT9045 log -3`（HEAD `bbe5e1fc`，分支 `v906/steven-cbridge-review6`）；`git show bbe5e1fc:<檔>` 讀 WebWindowRegistry.h／.cpp、WebTeachLeave.h／.cpp、WebStart.cpp、WebMotorAccessLive.cpp、csystem.cpp／.h、Command.cpp、canary_support.h、uhome.cpp、FileRW\_EditPage.cpp、tools\wb_serve.cpp、WebBridge\WebBridgeServer.h、CMakeLists.txt、docs\NIGHT_REPORT.md、web\background.html、web\page\ht9045_wire_engine.js、web\page\ht9045_recipe_client.js。
- `git grep` 找 `WebWindowRegistry`、`W906_FShow`、`W906_FormFShow`、`W906_FormFShowHook`、`ui.windows.put`、`LiveWebSocketCount`、`HT_WIN`。
- 暫存目錄的 Python 腳本（cp950 解碼 golden V912 全樹、`git cat-file --batch` 讀移植樹 HEAD）：去註解、`#if 0` 堆疊、分類讀／寫、依表單與檔分組；另 grep golden 的 `fXxx->Show()`／`ShowModal()`／`Close()`。
- 讀 `D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\window-open-edge.md`（Q49）、`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`（Q49、Q51）、`D:\HT9045_handoff\TO_STEVEN.md` §1（誰在改哪些檔）。

## 12. 實作紀錄：第 1＋2 步（20260928，commit `6273f82f`）
- 新檔：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.h`／`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp`（90 列、回答規則、程式寫入、START 前的畫面檢查、500 ms 拍子、ui.pages）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\W906FormShowing.h`（單一函式 W906_FormShowing）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_pagetable.cpp`（ctest WebPageTable，98 項）。
- Steven Q-P1：① 沒有畫面（WebSocket 0 條，或主畫面 fMain 的網頁答案是關）⇒ `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp:1148` StartFromWeb 開頭拒絕 START（網頁 START 與告警框 START 都經過這裡）；SECS S2F42 直接寫 SoftStart 的 4 處由 ② 在一拍內停下（入口擋在第 3～5 步做）。② 畫面不見滿 10 秒而且（SystemStart 或 HOME ALL 或網頁馬達工作）⇒ golden TfMain::Pause（PauseFromWeb）／fHome->Close()／motor.stop，停下後出告警 MES1690（Jimmy 20260928 E#36＝C；寬限 10 秒是 R122）。
- 回答規則：新鮮 ⇒ 聯集；過期但還有 WebSocket ⇒ 最後一次；過期而且沒有 WebSocket ⇒ 關；沒回報／absent ⇒ 關；沒有網頁的 12 列 ⇒ 關；C++ 對話框 10 列 ⇒ 程式狀態。成員一律 OR。
- 網頁（`D:\HT9045\web\background.html`）：H1 沒建立的視窗送 `state:'absent'`；H2 pagehide 送 bye 全部關；H3 聽 ui.pages，want 每個 wseq 只照做一次（openWin(id,true) 不跳 alert、不問操作員守衛）。與設計不同：want 不在網頁回報後清掉，改成「跟著程式狀態＋wseq」，讓 F5 後的新頁也補開 Home Monitor。
- ⚠ 從這顆起，沒透過 background.html 直接送 `start.run` 的探針會被拒；要先送一份 `ui.windows.put` `{"main":{"form":"fMain","state":"open"}}` 並保持 WebSocket 連著。
- 待辦：MES1690 說明檔（機台資料，請 Jimmy）；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:30425` 同一行擋 SoftStart；第 3～5 步（讀取批 1～6、引擎只在開窗時要資料、C 路關窗清 shown）20260928 下午起在做。

### 第 3～5 步（20260928 下午，ST01-E 派的工程師；基準 `730e98bd`，工作樹同時有別人在改）

**一句話**：golden 裡「這個畫面開著嗎」的讀取，操作員從網頁開的畫面（批 1、批 2）與網頁沒有的畫面（批 5）改問頁面表的單一函式；遠端（SECS／HOME）START 沒有畫面就擋；設定頁（C 路）只在操作員真的打開視窗時才讀資料、記 "Enter"，關窗之後再開＝重新開頁。

**讀取點分批結果**（行號是 `730e98bd`；全部同一行改，行數不變）：

| 批 | 設計處數 | 接上 | 留下 | 留下的理由 |
|---|---:|---:|---:|---|
| 1 主迴圈（操作員開的畫面） | 26 | 20 | 6 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Handler\HandlerGpibMsg.cpp:706`、`:707`、`:708`、`:1256`、`:1285`（St02 的檔，不動，交給 St02 同一行改）；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc:70`（產生檔，而且那是「先存成員、換成這一頁的值、再還原」，不是問畫面開著沒） |
| 2 製程模組（fContact 為主） | 43 | 41 | 2 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.gen.inc:3511`（tools/gen_editlist.py 產生檔，手改會被下次重產蓋掉；要改得加 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\Temperature.py` 的 REPLACE）；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cTestCategory.cpp:648`（開機時「C++ 門面自己的 FormShow 跑過沒」的守衛，改問網頁會在 HMI 已開著那一頁時跳過 C++ 的初始化） |
| 3 Home Monitor（fHome） | 21 | 0 | 21 | 見下面 ⚠ |
| 5 網頁沒有的畫面（Zteach 34、fLaserSensor 1） | 35 | 35 | 0 | 答案照舊是關（頁面表 noweb 列） |
| 4 C++ 自己開的對話框 | 72 | 0 | 72 | 設計說可不做；成員本來就對 |

⚠ **批 3（fHome）刻意不接**：golden 全樹只有回原點程式會開 Home Monitor（V912 `uhome.cpp:2497`），所以 golden 的 `fHome->fShow` 意思是「正在回原點而且畫面開著」；網頁版操作員可以自己從選單開它看進度（Q9 保留）。如果改問頁面表（程式開 或 網頁開），`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:32715` 停機分支的 `if(fHome->fShow) fHome->sbAbortHomeClick(fHome);` 會在 **HOME ALL 剛做完**（程式已關、網頁還沒關掉視窗的那半秒）就中止回原點、伺服 OFF、`fAllMotorHome=false` —— 剛回好的原點馬上失效。成員本身已經是對的答案：程式開關有寫（`uhome.cpp:646`、`:652`），操作員從網頁關掉也已經由頁面表的拍子轉成 golden `fHome->Close()`（第 1 步）。所以留成讀成員，列 R 題。

**棘輪**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\fshow_audit.py`＋基準 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\fshow_audit_baseline.json`，ctest `FShow_Audit`／`FShow_Audit_SelfTest`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt` 檔尾）。全樹 208 → 112 個直接讀（27 個檔），已接 25 → 121；任何一個檔變多就紅，`#if 0` 裡的不算（解閘的人一解開就被抓）。例：下一個波次在 `ckernel.cpp` 翻進一行 `if(fContact->fShow)` ⇒ `FShow_Audit` 紅，訊息叫他寫成 `W906_FormShowing("fContact", fContact->fShow)`。

**SECS／HOME 的 START 沒有畫面就擋**（Steven Q-P1）：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:30425`（MainProc，`ScanSystemSensor()` 之前同一行）：沒有畫面 ⇒ `SoftStart=false`；是 HOME 的話照 golden TfMain::Home 取消那兩句清 `iHome`、`bLampHome`。安裝座 `W906_PageStartAllowedHook`（`csystem.cpp:30040` 同一行定義，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4389` 裝）。運轉中不看。例：沒開瀏覽器，主機送 S2F41 START ⇒ 機台不動，wb_serve 主控台印 `[PAGETAB] START refused: no HMI screen connected ... [MainProc SoftStart]` 與 `W906 PAGETAB: START/HOME refused -- no HMI screen connected ...`；主機那邊已經收到 HCACK=0（入口沒改），只是等不到開始的事件。

**設定頁只在真的開窗時讀（H4）＋關窗邊緣**：
- 網頁：`D:\HT9045\web\page\ht9045_wire_engine.js:2046-2066`（模組層的 `var H4` 與 HT_WIN 監聽）、`:2129-2135`（attach 尾的閘）。C 路頁嵌在外框裡 ⇒ 等 `HT_WIN` 說 open 才 `load()`；每一次 關→開 再讀；3 秒沒收到 HT_WIN（舊外框、單獨開頁）照舊直接讀。
- C++：頁面表每拍把網頁列的邊緣交給 hook（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp` `PageTableSetEdgeHook`）；wb_serve 接到 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp` 檔尾 `W906_EditPageWindowClosed`：那個表單的每一個 C 路 tag 清「開過了」（下次存檔／form.event 前一定要重新開頁），並清掉「這一次開窗已經記過 Enter」。裝了之後 Enter 改成「每一次開窗記一次」（`OpenEnterRecord` 開頭 `EnterWindowGate`）；沒裝（ctest）照 S165 舊規則。
- 例（Configuration）：開窗 ⇒ 記一筆 "Enter Configuration"、跑 golden FormShow；按存檔（golden 存檔＝FormClose）⇒ 引擎自動重讀，**不再多記一筆**（R110 沒了）；關窗再開 ⇒ 再記一筆。開站那一刻不再每頁記一筆（R108 沒了）。
- 限制：開→關→開 在同一拍（500 ms）裡做完，頁面表看不到那一次關 ⇒ 不算重新開頁。

**測試**：`WebPageTable`（新增 [15] 邊緣 hook，105 項）、`OpenEnterLog`（新增 [9] 跟著視窗走，72 項）、`WebTeachLeave`、`WebWindowRegistry`、`P6b_FShowWired`、`FShow_Audit`（＋SelfTest）通過；god-stack 測試結果見交件。網頁 H4 只做了 `node --check`，沒在瀏覽器跑。

**MES1690 撞號**：`D:\HT9045\Error\AlarmCodeList.txt:949` 已經有 `WAR1690=No SYN-TEK Master Card!!!`（`D:\HT9045\Error\English\WAR1690.dat`、`D:\HT9045\Error\Chinese\WAR1690.dat` 也在）。第 1 步查的是「MES1690 這個字串」，沒有查數字；golden 事件資料庫用數字當 AlarmID（V912 `cMyDB.cpp:630`），兩個會混在一起。建議改用沒人用的 16441（AlarmCodeList、Error 說明檔、golden／移植樹程式、`D:\HT9045\web\JSON` 都查過沒有）。列 R 題。
