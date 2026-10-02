# 評估：網頁版怎麼做 golden 那 7 個「要密碼才能改」的設定（Q45 實作設計）

> 讀者：Steven。撰寫：ST01-E（Steven01 工程線）派的工程師，20260927 21:50。**只讀研究，沒有改任何程式、沒有 build、沒有跑 ctest。**
> 基準：`D:\HT9045` 分支 `v906/steven-cbridge-review6` 的 commit `c20bdec2`（那顆 commit 改的主要檔是 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`）。
> 三棵樹的寫法：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` 開頭＝**移植樹**（C++，UTF-8）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` 開頭＝**golden V912**（BCB6 原始碼，Big5／cp950）；`D:\HT9045\web\` 開頭＝**網頁**。行號是 20260927 當下的檔案。
> **本文件不含任何密碼值**：golden 寫死的密碼、`D:\HT9045\system\SG_PW.ini` 的內容、`D:\HT9045\system\login.dat` 的內容都沒有打開或印出；查證時用自己的腳本只輸出「長度／兩個字串相不相同」（Steven 裁決「密碼資料不可暴露給瀏覽器」RULINGS S41、「名單可以，密碼需要加密」RULINGS S55）。
> 題目原文：`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md` 的「### Q45.」。

---

## 0. 一句話結論

- 7 個點其實是**三種**檢查：
  - **甲、重新登入一次、等級要夠**（3 個點：Configuration 頁「監控功能」那組勾選框；SetUp 頁取消 Real Time CCD、取消 OCR）。
  - **乙、輸入程式寫死的廠商密碼**（3 個點：「使用 PE 模式」那組、「啟用光學尺」、「啟用員工 ID 檢查」；三個點是**同一組**密碼）。
  - **丙、輸入 `D:\HT9045\system\SG_PW.ini` 裡的密碼**（1 個點：這不是改值，是把一個藏起來的區塊「解鎖、變看得見」）。
- **建議（跟 Q45 裡 St01 的建議一致，這裡補成可以直接開工的設計）**：甲**現在做**，比對全部在 C++、沿用現有登入的比對程式，**在按「存檔」那一刻問**，密碼跟現在主畫面登入一樣只走本機 127.0.0.1（Steven Q10＝B1 的做法）；乙、丙**維持改不了**，網頁說明原因。約 5.5 人天（St01 約 5、Jimmy 約 0.5）。
- **全套「一次性亂數摘要」**（密碼完全不上線）做得出來，7 點全做約 13 人天；但保護增加有限（網頁伺服器本來就只聽本機），而且乙照 golden 做就要把 golden 寫死的廠商密碼放進移植樹，跟「repo 裡不可以放密碼」的規則衝突（見 5.2；Q45-7 有只放雜湊的替代做法）。
- **主畫面登入（auth.login）不建議現在改成摘要**：要推翻 Q10＝B1，約 3～4 人天；而且「改密碼」時的**新密碼**不可能用摘要傳（C++ 要把新密碼存進密碼本，一定要拿到原字），要做真的加密再 +2～3 人天。
- 順帶發現：移植樹一份舊的查證文件**已經寫著 golden 的廠商密碼**（兩處，見 5.2、Q45-10），建議遮掉。

---

## 1. 背景／現況

### 1.1 白話：golden 這 7 個點在畫面上是什麼

| # | 畫面上是什麼（白話） | 程式裡的名字 | 驗什麼 | golden 什麼時候問 |
|---|---|---|---|---|
| 1 | Configuration 頁「使用 PE 模式」勾選框；它勾著的時候，另外 4 個勾選框（「初始啟動時重啟 GroundMan」「存 bin 顯示的通訊紀錄」「使用 Loader 顏色感測器」「初始啟動檢查氣缸」）改了也要問 | [C12] `cbC12`；[C13] `cbC13`、[C14] `chkC14`、[C17] `cbC17`、[C24] `cbC24` | 乙：廠商密碼（程式寫死的一段字，後面接程式版本號；KYEC 要求「另一組廠商密碼」） | 點下去當下 |
| 2 | Configuration 頁「啟用光學尺」勾選框 | [A27] `cbA27` | 乙：同 #1 那組 | 點下去當下（勾、取消都問） |
| 3 | Configuration 頁「啟用員工 ID 檢查」勾選框 | [N07-5] `cbN07_EnableEmployeeCheak` | 乙：同 #1 那組 | 點下去當下（勾、取消都問） |
| 4 | Configuration 頁「監控功能」一整組 16 個勾選框（總開關＋15 個子項，例：「Contact 模式必須選不同速度」「Auto speed 必須開」「啟用 ATC」） | [M01] `cbM01`、`cbM01_01`～`cbM01_15` | 甲：重新登入，等級要到權限表第 92 項（Contact - Contact parameter）設的等級；只有裝了 Real Time CCD 的機台才問；第 92 項設 0 就不問 | 點下去當下（勾、取消都問） |
| 5 | Configuration 頁「Check List Enable」區塊（平常藏著）。在「[A32] Enable FTP Automation」那一格右邊有一塊看不見的小圖（41×21 點），連點兩下會問密碼 | 區塊 `grpA32_1`、小圖 `Image1` | 丙：`D:\HT9045\system\SG_PW.ini` 裡的密碼（檔案不在會先寫一份內建預設值） | 連點兩下時；對了之後區塊一直看得見，直到程式關掉 |
| 6 | SetUp 頁「Enable Real Time CCD」勾選框：開頁時勾著、現在取消 | `cbEnableRealTimeCCD` | 甲：重新登入，等級要到權限表第 37 項（Tools - CCD）設的等級；只有裝了 Real Time CCD 的機台才問 | 按「Save」時 |
| 7 | SetUp 頁「Enable OCR Function」勾選框：開頁時勾著、現在取消 | `cbOcrFunction` | 甲：同 #6（#6、#7 一起取消時只問一次） | 按「Save」時 |

權限表項目名稱出處：golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:65`（[37] Tools - CCD）、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:120`（[92] Contact - Contact parameter）。權限表存在 `D:\HT9045\system\levelset.dat`（256 個整數，沒有密碼；golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cprod.h:1164-1168`）。

**Steven01 這台的實況（只讀）**：`D:\HT9045\system\Gerneral.ini` 第 16 行 `REAL_TIME_CCD=1`（裝了）；`D:\HT9045\system\levelset.dat` 第 37 項＝3（HonPrec）、第 92 項＝0 ⇒ 這台上 #4 永遠不問（golden 的捷徑），#6、#7 會問、而且要重新登入成等級 3 的帳號。`D:\HT9045\system\SG_PW.ini` 存在（只看了檔案大小，沒打開）。

### 1.2 Steven 已經做過的相關裁決

- 「密碼資料不可暴露給瀏覽器」（RULINGS S41，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260925.md:177`）。
- Status.Security 改密碼：「名單可以，密碼需要加密」（RULINGS S55，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:172`）→ 實作成「C++ 回給網頁的東西一律不帶密碼」。
- 「改密碼的傳輸本身要不要另外加密」＝A 不另外加密（Q8／RULINGS S130，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:359`）。
  ⚠ 注意：`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md` 的「### Q8.」背景寫「密碼本身不會出現在傳輸內容裡」，這句只對 **C++→網頁** 這個方向成立；**網頁→C++** 的登入、改密碼訊息是帶著密碼原字的（下一條 Q10 講清楚了）。Q45 選項 A 寫的「符合 S130『密碼不進訊息內容』」也要照這個意思讀。
- 「golden 密碼存檔跟網頁傳輸」＝B1：存檔照 golden；**網頁傳輸維持現狀（密碼原字只走本機 127.0.0.1，C++ 不回送密碼），程式不改**（Q10／RULINGS S132 定案，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:417`）。
- 「既有頁面缺的 golden 事件與等級檢查先補」（Q41／RULINGS S158，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:392`）→ 這 7 點就是那份盤點裡的「C-3 密碼確認還沒接」（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md` 第 37、55 行）。

### 1.3 移植樹現況（C++）

| 項目 | 位置 | 現在的行為 |
|---|---|---|
| Configuration 存檔前的「要密碼的格子改了就拒存」檢查（IC_PasswordGuard） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:139-233`（說明＋本體），呼叫點 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:306-314`；commit `a8eca460`（改的主要檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp`） | #1～#4 改了就**整次拒存**並說明原因；#4 只在「裝了 RTC 且第 92 項不是 0」時拒 |
| #5 的區塊 | 同上說明 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:161-163` | 區塊一直藏著、頁面送來的值被丟掉 ⇒ 本來就改不到；「連點兩下」沒翻 |
| SetUp 的重新登入（SU_DoPassword） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc:69`（產生器設定 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_SetUp.py`）；呼叫點 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc:3038`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc:3065`；commit `97c70d62`（改的主要檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_SetUp.py`） | 沒裝 RTC 照 golden 直接過；裝了 → 視同密碼錯：把勾改回來、其他照存（「視同密碼錯」本體 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp:436-443`） |
| golden 成功後的紀錄 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc:3053`（RealTimeCCD Disabled）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc:3059`（OCR Disabled）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc:3075`（只關 RTC 那一條分支） | 已照翻；沒裝 RTC 的機台照常會記，裝了 RTC 的機台現在走不到（一律視同密碼錯） |
| 網頁送密碼給 C++ 驗證的指令（dialog.auth，Jimmy 的告警框用） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4889-4904` | 一律回「還沒接」 |
| 主畫面登入（auth.login，帳號＋密碼） | 分派 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5494-5524`；比對本體 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:383-516`（WebLogin_BookLogin；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:386` 已登入就要先登出） | 照 golden |
| 下拉選單模式登入（auth.select，只有密碼） | 分派 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5533-5555`；本體 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:266-376`；密碼比對 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:167-264` | 照 golden，但**少一臂**：golden「SetUp／告警框正在問密碼時」的特殊比對沒翻（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:238` 的註解） |
| 存檔時「開頁時的等級＝現在的等級」檢查（網頁版多的一道） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:239`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:89` | 等級不同就回「請重讀」 |
| golden「切到 Operator 後不能存設定」（[A01_2]） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc:7628`（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:5828-5832`） | 已照翻 |
| 運轉中不能存檔 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5303` | 有 |
| 登入等級即時送給網頁（tag auth.level） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp:789` | 有送；主畫面沒在聽（見 1.4） |
| 防連點（同一指令＋同一內容短時間內重送回 busy） | 分派迴圈頭 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4676`；規則 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebCmdGuard.cpp`（白名單 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebCmdGuard.cpp:83`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebCmdGuard.cpp:89`；被擋時的印出 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebCmdGuard.cpp:446`，只印指令名與 tag） | auth.login／editlist.save 都在保護範圍內 |
| 單一操作員權杖 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.cpp:1443-1445`（auth.* 開頭的指令一律免權杖） | editlist.save 要權杖 |
| 網頁來源限制 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.cpp:1089-1125` | 只接受 http(s)://127.0.0.1、localhost、[::1] 的網頁；file:// 開的頁（Origin: null）被拒；不帶 Origin 的連線（非瀏覽器程式，例如探針）允許 |
| 雜湊／亂數 | 全樹只有 WebSocket 握手用的 SHA-1（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\Sha1.cpp`） | **沒有 SHA-256、沒有密碼學等級的亂數**（查證指令見 9.1） |

### 1.4 網頁現況

| 項目 | 位置 | 歸屬 |
|---|---|---|
| 主畫面登入流程（先帳號小鍵盤、再密碼小鍵盤） | `D:\HT9045\web\page\main.html:761-800` | Jimmy |
| 送登入的函式（tag＝帳號、value＝密碼原字） | `D:\HT9045\web\page\ht9045_recipe_client.js:550-552`（authLogin）、`D:\HT9045\web\page\ht9045_recipe_client.js:559-563`（authSelect） | Jimmy（筆電，main 分支 `D:\HT9045\docs\handoff\TO_STEVEN.md:25` 登記） |
| 存檔函式（頁面補件可以包一層） | `D:\HT9045\web\page\ht9045_recipe_client.js:420-429`（editlistSave；第 4 個參數 extra 會併進送出的內容） | Jimmy |
| 共用網頁引擎的存檔 | `D:\HT9045\web\page\ht9045_wire_engine.js:1249-1297`（存檔前的附加資料掛勾 `D:\HT9045\web\page\ht9045_wire_engine.js:1273-1277`，只能同步回傳） | Jimmy（筆電） |
| SetUp 頁補件（已經包了 editlistSave） | `D:\HT9045\web\page\ht9045_setup_c_wire.js:332-341` | St01 |
| Configuration 頁補件（P26 那一支，做法可比照） | `D:\HT9045\web\page\ht9045_iniconfig_p26_c.js`（掛在 `D:\HT9045\web\page\Config.Configuration.html:128`） | St01 |
| 告警框的密碼層（畫面跟 golden 登入框一樣） | `D:\HT9045\web\page\dialog-bridge.js:489-520`、`D:\HT9045\web\page\login-page.js`、`D:\HT9045\web\page\Alert.Password.html`；契約 `D:\HT9045\web\JSON\js\Dialog-bridge-contract.js`（credentialsHandling：帳密原字只走本機） | Jimmy 的設計（`D:\HT9045\.claude\skills\ht9045-html-version\references\dialog-bridge.md`） |
| C++ 模式的網頁網址 | `http://127.0.0.1:8045/background.html`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\.vscode\launch.json:231`） | — |

另外，網頁在開站時就把所有設定頁在背景載好（`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\window-open-edge.md` 第 0、1.2 節），所以「開頁時的等級」其實是開站時或上次存檔後重讀時的等級。

---

## 2. 7 個點逐一

### 2.0 共用：golden 的「重新登入」（DoPassword）到底做了什麼（#4、#6、#7）

golden 有兩支同名函式：Configuration 版 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6482-6529`、SetUp 版 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:4325-4362`。

1. **要求等級**：Configuration 版用權限表第 92 項、SetUp 版用第 37 項。Configuration 版第 92 項＝0 就直接過（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6488-6490`）；**SetUp 版沒有這個捷徑**（第 37 項＝0 也照樣跳登入框，只是一定過）。
2. **沒裝 Real Time CCD 就不問**（`D:\HT9045\system\Gerneral.ini [System] REAL_TIME_CCD`，golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\database.cpp:634` 讀）。
3. **另一個輸入畫面正開著時不問**（Configuration 看 fInput、SetUp 看 fQwertyKey；移植樹沒有這兩個畫面，一律當「沒開」）。
4. **叫主畫面的登入**：
   - **有密碼本**（`FileExists(pwPath)`；Steven01 的客戶碼 791 是 `D:\HT9045\system\login.dat`）→ 呼叫主畫面登入的同一支 `TfMain::cbUserSelectChange`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:14918`），但**不經過** Login／Logout 鈕（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28080-28095`），所以已經登入的人也照樣問。跳帳號＋密碼框；對了＝**登入成那個人**（等級 0～3，記「USER login」）；錯了或按取消＝**變成 Operator**，記「Operator login」（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:15061-15105`）。帳號、密碼比對都不分大小寫。
   - **沒有密碼本（下拉選單模式）**→ `TfMain::stOperatorClick`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13193-13324`）：只跳密碼小鍵盤；先比「Configuration 頁設定的 Supervisor 特別密碼」（存在 `D:\HT9045\system\lastdata.dat`）與最高權限密碼（`D:\HT9045\system\Gerneral.ini [VENDER]`），對了都是最高權限（這兩個**分**大小寫）；接著：
     - SetUp 正在問時走「特殊比對」（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13283-13301`）：不管下拉選到哪一級，先比 `D:\HT9045\system\login.dat` 裡 Supervisor 等級那一格、再比 Engineer 等級那一格（不分大小寫）。
     - Configuration 走一般比對（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13304-13320`）：只比「下拉**目前選的那一級**」的密碼。
     - 錯了等級變 0，但下拉選單畫面不變（這條路 golden 沒有更新畫面，golden 怪處）。
5. **比等級**：登入後的等級 < 要求等級 ⇒ 失敗。
6. **只有 Configuration 版**：不管成功或失敗，**問完一律登出成 Operator**（有密碼本時，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6518-6525`）。SetUp 版不登出。

Configuration、SetUp 在 golden 都是用 ShowModal 開的（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28624`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28471`），表單開著時操作員回不到主畫面重新登入。

### 2.1 #1「使用 PE 模式」那一組（[C12] 與 [C13]／[C14]／[C17]／[C24]）

- **golden 出處**：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6775-6791`（cbC12Click）；五個勾選框都綁它（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 3161、3178、3238、3533、3903 行）。
- **驗什麼**：點完之後「使用 PE 模式」是勾著的才問；比對「寫死的一段字＋程式版本號」（分大小寫）。三個點（#1～#3）用的是同一段字（查證只比「相不相同」）。
- **錯了**：把「使用 PE 模式」取消勾——**就算點的是另外四格之一，也是 PE 模式被取消**，那一格保留新值（golden 怪處）。
- **副作用**：沒有。
- **移植樹**：改了就整次拒存（1.3）。
- **例**：「使用 PE 模式」原本勾著，操作員把「使用 Loader 顏色感測器」打勾 → golden 跳密碼 → 打錯 → 「使用 PE 模式」變成沒勾，「使用 Loader 顏色感測器」維持打勾。

### 2.2 #2「啟用光學尺」（[A27]）

- **golden 出處**：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6824-6858`（綁定 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 1323 行）。
- **驗什麼**：勾、取消都問；同 #1 的廠商密碼。
- **錯了**：改回原狀。
- **副作用**：不管對錯，最後都把**主畫面**一個小標記改成「L」（勾著）或「X」（沒勾）（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6849-6856`）。網頁主畫面沒有這個標記。
- **移植樹**：改了就整次拒存。

### 2.3 #3「啟用員工 ID 檢查」（[N07-5]）

- **golden 出處**：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6860-6885`（綁定 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 15080 行）。
- **驗什麼**：勾、取消都問；同 #1 的廠商密碼。錯了改回原狀；沒有其他副作用。
- **移植樹**：改了就整次拒存。

### 2.4 #4「監控功能」16 格（[M01]）

- **golden 出處**：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6531-6544`（cbM01Click，16 格都綁它，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 21162～21305 行）→ 2.0 的 Configuration 版重新登入。
- **驗什麼**：每點一格就問一次（重新登入＋等級到第 92 項）。
- **錯了**：那一格改回原狀。
- **副作用**：登入狀態變成「這次登入的人」或 Operator，**然後一律登出成 Operator**。
- **golden 怪處（跟「切到 Operator 後不能存設定」打架）**：客戶開了 [A01_2]（IniConfig.bA02DisableSaveParsWhenSwitchToOp）時，關 Configuration 頁存檔前會檢查「現在是 Operator 就不存」（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:5828-5832`）。而改 M01 一定被登出成 Operator，表單又是 ShowModal 回不到主畫面登入 ⇒ **在裝了 RTC、第 92 項不是 0、又開了 [A01_2] 的機台上，M01 的變更（連同這次其他變更）永遠存不進去**。這是由程式推得，沒有在 BCB 機台上試過。
- **移植樹**：改了就整次拒存（只在裝了 RTC、第 92 項不是 0 時）。
- **例**：第 92 項＝Supervisor 的 RTC 機台，Engineer 開 Configuration 勾「Auto speed 必須開」→ golden 跳登入框 → 輸入 Supervisor 帳號密碼 → 過，勾保留 → 主畫面變 Operator。

### 2.5 #5「Check List Enable」區塊（[A32_1]）

- **golden 出處**：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7274-7289`（Image1DblClick，綁定 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 1478 行）；區塊只有建構時被設成看不見（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:743`）。
- **驗什麼**：`D:\HT9045\system\SG_PW.ini` 裡的密碼（分大小寫）。
- **副作用**：**每次**連點兩下，檔案不在就先寫一份內建預設值（在問密碼之前，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7278-7281`）；對了區塊變看得見，直到程式關掉（沒有程式再把它藏起來）。
- **移植樹**：區塊一直藏著；網頁的小圖在（`D:\HT9045\web\page\Config.Configuration.html` 裡 id="Image1"），但連點兩下沒反應。

### 2.6 #6、#7 SetUp 取消 Real Time CCD／OCR

- **golden 出處**：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:3567-3615`（在存檔鈕 sbUpdateClick 裡，函式開頭 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:3492`）→ 2.0 的 SetUp 版重新登入。「開頁時是勾著的」記在開頁程式（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:3176`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:3191`，函式 DoIniDataToForm 開頭 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:3044`），存完又重新記一次（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:3617-3625`）。
- **什麼時候不問**：客戶開了「RTC 由檔案鎖住」且 RTC 功能關著（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:3567-3570`）；RTC／OCR 那一區是灰的；沒裝 RTC。
- **錯了**：把取消的勾改回來，**其他設定照存**。
- **對了**：事件紀錄記「RealTimeCCD Disabled」／「OCR Disabled」（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:3588`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:3594`）。
- **副作用**：登入狀態變成「這次登入的人」；錯了或取消＝Operator。不另外登出。
- **移植樹**：裝了 RTC 就視同密碼錯（勾改回、其他照存）。
- **例**：Steven01 這台（第 37 項＝3），Engineer 取消「Enable Real Time CCD」按 Save → golden 跳登入框 → 輸入等級 3 的帳號密碼 → 存檔、記一筆「RealTimeCCD Disabled」、主畫面變成那個帳號。

---

## 3. 設計

### 3.1 設計原則

1. **比對只在 C++**，網頁只收集操作員打的字（跟告警框的契約 `D:\HT9045\web\JSON\js\Dialog-bridge-contract.js` 的 verifier「C++ only」同一條）。
2. **沒帶就照舊擋**：頁面沒帶重新登入資料（舊頁面、沒更新的補件）時，C++ 維持現在的「整次拒存／視同密碼錯」，不會默默存進去。
3. **golden 的結果與副作用照做**（失敗改回、登入狀態改變、強制登出、事件紀錄）；時間點從「點下去當下」改成「按存檔那一刻」是唯一的刻意偏離（標 [W906]，Q45-2）。
4. **密碼不進 tag、不進日誌、不回給網頁**；C++ 取出之後就把它從 JSON 拿掉。
5. **重新登入的資料跟著存檔一起送**（放在同一個 editlist.save 裡），不另開「通行證」：這樣「開頁等級＝存檔等級」檢查在重新登入之前就做完、不用放寬；C++ 不必記住任何通行證；密碼在網頁端只活到那一次送出。

### 3.2 建議組合：甲（#4、#6、#7）現在做，乙丙（#1、#2、#3、#5）維持改不了

#### 3.2.1 操作員看到什麼（白話）

- **SetUp（#6、#7）**：取消「Enable Real Time CCD」或「Enable OCR Function」、按 Save → 跳出跟主畫面一樣的登入小鍵盤（有密碼本：先帳號、再密碼；下拉模式：只有密碼），上面寫「關閉 RTC／OCR 要重新登入，等級要到 HonPrec（權限表第 37 項）；按取消＝登出成 Operator（golden 同）」→ 輸入 → 存檔。
  - 例：對且等級夠 → 存檔成功、事件紀錄多一筆「RealTimeCCD Disabled」、主畫面變成剛登入的人。等級不夠（例如用 Engineer 登入）→ RTC 勾回去、其他設定照存、主畫面變 Engineer，頁面顯示「等級不夠，RTC 沒有關掉」。打錯 → RTC 勾回去、其他照存、主畫面變 Operator。
- **Configuration（#4）**：改了「監控功能」任何一格、按存檔 → 跳同樣的登入小鍵盤（寫「要到權限表第 92 項的等級；問完會照 golden 登出成 Operator」）→ 輸入 → 存檔。
  - 例：第 92 項＝Supervisor，改了「Auto speed 必須開」和「啟用 ATC」兩格 → 問一次 → 對且夠：兩格都存、其他照存、存完主畫面變 Operator；不夠：兩格改回、其他照存、主畫面變 Operator。
- **#1、#2、#3、#5**：維持現在（改了就整次拒存並說明；#5 區塊看不到）。說明文字改成「這一項要廠商密碼，網頁版不提供，請到 BCB 版機台改」。

#### 3.2.2 訊息格式（新增的欄位都沒有秘密，除了 3.2.2-② 的 password）

① **開頁（editlist.get）回應多一段**（C++→網頁），讓頁面知道「這台機台現在按存檔會不會要重新登入」：

```json
"extra": { "auth": {
  "mode": "book",
  "points": [
    { "id": "rtcOff", "controls": ["cbEnableRealTimeCCD"], "kind": "relogin",
      "armed": true, "armedAtOpen": true, "levelItem": 37, "level": 3, "logoutAfter": false },
    { "id": "ocrOff", "controls": ["cbOcrFunction"], "kind": "relogin",
      "armed": true, "armedAtOpen": false, "levelItem": 37, "level": 3, "logoutAfter": false }
  ] } }
```

- mode：book＝帳號＋密碼（有密碼本）、select＝只有密碼（下拉選單模式），照 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:159`（WebLogin_UsesBook）。
- armed＝裝了 RTC、沒被檔案鎖住、那一區可以改；armedAtOpen＝開頁時勾著（golden 的 bNeedPassword／bOCRNeedPassword，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc:45-46`）。
- Configuration 那一頁同樣回 `{ "id": "m01", "controls": ["cbM01","cbM01_01",…,"cbM01_15"], "kind": "relogin", "armed": <裝了 RTC 且第 92 項≠0>, "levelItem": 92, "level": …, "logoutAfter": <有密碼本> }`；#1～#3、#5 回 `"kind": "vendor"`／`"sgpw"`、`"armed": false`、`"why": "網頁版不提供"`，頁面可以一開始就把那幾格標成「要廠商密碼」。

② **存檔（editlist.save）的 value 多一個欄位**（網頁→C++，**跟 widgets 並列，不能放進 widgets**；tag 仍是結構名，不含秘密）：

```json
{ "widgets": { … }, "answers": { … },
  "reauth": { "point": "rtcOff", "userId": "eng1", "password": "（操作員打的字）" } }
```

- 下拉模式沒有 userId。
- 操作員按取消：`"reauth": { "point": "rtcOff", "cancelled": true }` ⇒ C++ 照 golden 的「取消＝空白帳密＝錯」處理（登出成 Operator，Q45-4）。
- 完全沒有 reauth 欄位 ⇒ 照現在（SetUp 視同密碼錯；Configuration 整次拒存）。

③ **存檔回應多一段**（C++→網頁，不含密碼）：

```json
"reauth": { "point": "rtcOff", "asked": true, "passed": false, "reason": "level 1 < 3 (levelset item 37)",
            "reverted": ["cbEnableRealTimeCCD"],
            "login": { "mode": "book", "level": 1, "userCaption": "Engineer", "btLogin": "Logout" } }
```

login 用現成的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:545`（WebLogin_StateJson）。asked＝false 表示 golden 這次根本不會問（例如沒裝 RTC），頁面帶了也不用。

#### 3.2.3 C++ 在哪裡驗、照翻哪一支 golden

| 步驟 | 照翻的 golden | 移植樹放哪、改什麼 |
|---|---|---|
| 重新登入核心（新） | 2.0 的兩支 DoPassword：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6482-6529`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:4325-4362` | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp` 檔尾新增 `W906_Reauth(point, userId, password, cancelled, &result)`：要求等級、[92]＝0 捷徑（只 Configuration）、沒裝 RTC 直接過、登入、比等級、Configuration 版強制登出 |
| 登入（有密碼本） | `TfMain::cbUserSelectChange` 密碼本分支 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:14918`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:15061-15105` | 把 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:383-516` 的比對本體抽成內部函式：auth.login 那一層保留 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:386` 的「已登入要先登出」（那是 golden Login 鈕的規則），重新登入直接呼叫本體（golden cbUserSelectChange 本身沒有這條） |
| 登入（下拉模式） | `TfMain::stOperatorClick` `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13193-13324`，含 SetUp 特殊比對 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13283-13301` | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:167-264` 補上那一臂（只補「SetUp 正在問」這一種；告警框那幾種，等告警框的密碼指令（dialog.auth）接上時再補） |
| SetUp 存檔接上 | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:3576`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:3600` 呼叫 DoPassword | 改產生器 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_SetUp.py` 的 SU_DoPassword 取代規則、重產 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc`：沒裝 RTC → 過；有帶重新登入資料 → 呼叫新的重新登入核心（W906_Reauth）；沒帶 → 照現在「視同密碼錯」（ELPasswordRefused）。後面的 golden（成功記紀錄、失敗勾回、其他照存）已照翻，不用動 |
| Configuration 存檔接上 | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6531-6544` | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:214-225`（M01 那段）改成：有改＋有 reauth → 不拒存；套值之後跑 W906_Reauth；失敗 → 把改過的 M01 格子改回開頁值；強制登出放在 golden 存檔流程之前或之後（Q45-3）。#1～#3 那幾段不動 |
| 開頁回 armed | — | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.cpp` 的 extraJson（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.cpp:196` ExtraJson 旁）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp` 的開頁回應 |
| 運轉中 | golden 運轉中不能登入（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28082-28083`） | 存檔本來就擋（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5303`），重新登入跟著存檔走，所以不會在運轉中發生 |

「開頁等級＝存檔等級」檢查（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:239`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:89`）在存檔一開始、重新登入之前就做完，不受影響；存完引擎照規則重讀（`D:\HT9045\web\page\ht9045_wire_engine.js:1290`），拿到的就是新的等級。其他背景載好的頁下次存檔會被「請重讀」擋一次——現在主畫面換人登入也是這樣。

#### 3.2.4 頁面／引擎要做的（檔案清單）

| 檔 | 誰 | 做什麼 |
|---|---|---|
| `D:\HT9045\web\page\ht9045_setup_c_wire.js` | St01 | 在既有的存檔包裝（`D:\HT9045\web\page\ht9045_setup_c_wire.js:332-341`）加一段：開頁回應的 rtcOff／ocrOff 是 armed＋armedAtOpen、而現在沒勾 → 先跳登入小鍵盤 → 把 reauth 放進 extra（save0 的第 4 個參數）→ 送；回應的 reauth.login 用來提示操作員 |
| `D:\HT9045\web\page\ht9045_iniconfig_auth_c.js`（新檔，掛在 `D:\HT9045\web\page\Config.Configuration.html`，做法同 `D:\HT9045\web\page\ht9045_iniconfig_p26_c.js`） | St01 | 包 HT9045Recipe.editlistSave：M01 16 格跟開頁值不同且 armed → 跳登入小鍵盤 → 附 reauth；#1～#3、#5 的格子標「要廠商密碼，網頁版不提供」 |
| `D:\HT9045\web\page\main.html` | Jimmy | 主畫面的「使用者名字／Login 鈕」要在**別的頁改了登入狀態**時跟著變：訂閱 C++ 已經在送的 tag auth.level（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp:789`）一變就重讀 auth.mode（`D:\HT9045\web\page\main.html:756` 的 refresh） |
| `D:\HT9045\web\page\ht9045_wire_engine.js`、`D:\HT9045\web\page\ht9045_recipe_client.js` | Jimmy | **不用改**（頁面補件包 editlistSave 就夠；要另外送指令用既有的 rawCmd） |
| 登入小鍵盤 | — | 沿用主畫面那一種（`D:\HT9045\web\page\main.html:790-800` 的 HTQwerty 兩段），不用動 Jimmy 的告警密碼層；以後要畫面跟 golden 登入框一模一樣，再改用 `D:\HT9045\web\page\Alert.Password.html` |

#### 3.2.5 日誌、畫面都不留密碼

- 密碼只放在 value 的 reauth.password，**不放 tag**：wb_serve 的存檔印出只印結構名與結果（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5334`），防連點的印出只印指令名與 tag（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebCmdGuard.cpp:446`）。
- C++ 一進存檔就把 reauth 從 JSON 拿出來、清掉，才交給套值與回應（若誤放進 widgets，會被當成「表單沒有的欄位」原樣列在回應的 unknown 裡——所以規定放在 widgets 旁邊，C++ 也要檢查 widgets 裡不准有 reauth）。
- 失敗紀錄只寫帳號與原因（golden 的 USER login／Operator login 事件本來就只記帳號）。
- 網頁：不 console.log、不存 localStorage／sessionStorage、送完清空輸入；錯誤訊息不帶輸入內容。
- 殘留（跟現在的 auth.login 一樣）：防連點表用 value 原位元組當鍵（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebCmdGuard.cpp` 檔頭說明），密碼會在 C++ 記憶體裡留到防連點視窗結束（約 10 秒內清表）；不寫檔、不印出。

#### 3.2.6 防連點與權杖

- editlist.save 要單一操作員權杖（既有）；同一個 value 連送兩次，第二次回 busy（既有，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4676`）。重新登入跟著存檔走，所以兩道都自動適用。
- 頁面第二道：登入小鍵盤開著、存檔在路上時，頁面補件自己鎖住 Save 鈕與那幾個勾選框（同全站「全部按鈕要防連點」的規則）。
- 一次存檔只問一次；錯了不自動重問（golden 也只問一次），要重試就再按一次存檔。

### 3.3 全面 A：一次性亂數摘要（密碼完全不上線），7 點全做

#### 3.3.1 白話流程

操作員打完密碼 → 網頁先跟 C++ 要一個「只能用一次的亂數」→ 網頁用「密碼＋亂數」算出一段摘要 → **只把摘要送給 C++** → C++ 用自己手上的比對目標（密碼本、`D:\HT9045\system\SG_PW.ini`、寫死的廠商密碼）同樣算一次，一樣就過。密碼原字不在任何訊息裡；攔到摘要也不能拿來再送一次。

例（#6）：取消 RTC 按 Save → 小鍵盤輸入 → 網頁送「要亂數（點＝rtcOff）」→ C++ 回亂數 → 網頁算摘要 → 存檔時帶「亂數編號＋帳號＋摘要」→ C++ 驗過 → 照 3.2 的 golden 流程繼續。

#### 3.3.2 訊息格式

① **要亂數**（新指令，名字建議 auth.challenge；auth.* 開頭本來就免權杖，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.cpp:1443-1445`）：tag＝點的代號（rtcOff／ocrOff／m01／vendor／sgpw），value＝`{"page":"TestIF_File_SetUp"}` → 回：

```json
{ "challengeId": "ch-3f2a…", "nonce": "<32 個十六進位字>", "kind": "relogin", "mode": "book",
  "digests": ["ci"], "expiresInMs": 60000 }
```

② **存檔帶摘要**（取代 3.2.2-② 的 password）：

```json
"reauth": { "point": "rtcOff", "challengeId": "ch-3f2a…", "userId": "eng1",
            "proof": { "ci": "<64 個十六進位字>", "cs": "<64 個十六進位字>" } }
```

③ **#5 解鎖**（點下去就要生效，不跟存檔走；新指令，名字建議 auth.unlock）：tag＝"sgpw"，value＝`{"challengeId":"…","proof":{"cs":"…"}}` → 回 `{"passed":true,"changed":{"grpA32_1":{"visible":true}}}`，頁面接著重讀（editlist.get），區塊裡的格子才變成可改。

#### 3.3.3 摘要怎麼算

- `key = SHA-256(密碼的 UTF-8 位元組)`
- `proof = HMAC-SHA256(key, "HT9045-AUTH-1|" + challengeId + "|" + point + "|" + nonce)`，十六進位小寫。
- **要送兩份**：cs＝照打的字算；ci＝先把 a～z 換成 A～Z 再算。原因：golden 密碼本的比對不分大小寫（帳號、密碼都 UpperCase，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:15068-15069`），但 Supervisor 特別密碼、最高權限密碼、廠商密碼、SG_PW 的比對分大小寫。C++ 對每個候選用對應的那一份。
- 只換 a～z：移植樹的 UpperCase 只轉英文字母（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\vclcompat\AnsiString.cpp:106-111`），網頁不能用 JavaScript 的 toUpperCase（它會轉其他語言的字母）。

#### 3.3.4 C++ 怎麼比（候選照 golden 的順序，第一個相同就停）

| 種類 | 候選 | 依據 |
|---|---|---|
| 重新登入，有密碼本 | 帳號（不分大小寫）相同的每一筆（客戶開了 JCET_FOR_EVAN 時不看帳號），比 ci；找到 → 等級＝那一筆。golden 登入還有 FTP 下載密碼本、ASE-CL、條碼登入幾臂，移植樹本來就只翻了本機會走到的兩臂（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:378-381` 的說明） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:464-502` 那段迴圈，比對改成比摘要 |
| 重新登入，下拉模式，SetUp | Supervisor 特別密碼（cs）、最高權限密碼（cs）→ HonPrec；接著 Supervisor 等級那一格（ci）→ 2、Engineer 等級那一格（ci）→ 1 | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13264-13301` |
| 重新登入，下拉模式，Configuration | Supervisor 特別密碼（cs）、最高權限密碼（cs）；接著「下拉目前選的那一級」那一格（ci） | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13264-13320` |
| 重新登入，下拉模式，矽品客戶（bSPILFunction） | 走 golden 另一臂：Supervisor 特別密碼或矽品專用的寫死值（cs）→ HonPrec；否則比「下拉目前選的那一級」（ci）。**SetUp 在這一臂也沒有特殊比對**（golden 的結構就是這樣） | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13234-13263`（移植樹已照翻，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:190-219`） |
| 廠商密碼（#1～#3） | 比對目標（cs） | 比對目標放哪見 Q45-7 |
| SG_PW（#5） | `D:\HT9045\system\SG_PW.ini` 的值（cs）；檔案不在先照 golden 寫預設（在發亂數時做，對應 golden「連點兩下時」） | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7278-7287` |

- 亂數：Windows 的 CryptGenRandom（advapi32）。**不要用 std::random_device**：舊版 MinGW 的 C++ 函式庫有「每次給同一串」的已知問題（在本機 MinGW 工具鏈上沒有實測，實作時要先驗）。
- 一次性：綁連線（WebCommand 的 connId）、60 秒過期、同一連線同一點同時只有一個有效、用過（對或錯）立刻作廢、斷線清掉；表格上限（例 32 筆）。
- 比較用固定時間的比較函式。
- SHA-256／HMAC 在 C++ 是新寫的基礎建設：放在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\Sha1.cpp` 旁邊，同樣寫法、開 -Wall -Wextra、附測試。

#### 3.3.5 瀏覽器的限制

- 瀏覽器內建的雜湊（crypto.subtle）只在「安全來源」可用：https、`http://127.0.0.1`、`http://localhost`。C++ 模式的網頁由 wb_serve 從 `http://127.0.0.1:8045` 提供（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\.vscode\launch.json:231`）⇒ 可用；但若將來用區網 IP 開（例 `http://192.168.1.20:8045`）就不能用。file:// 開的頁本來就連不上 wb_serve（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.cpp:1089-1125`）。
- 建議網頁**自帶一份** JavaScript 版 SHA-256／HMAC（約 150 行，不用 npm），不靠 crypto.subtle（Q45-9）。亂數由 C++ 發，網頁不需要產生亂數。

#### 3.3.6 能擋什麼、擋不了什麼

- **擋得住**：開發者工具（F12 → Network → WS）看不到密碼（依 `D:\HT9045\HT9045_Release.cmd` 開頭的說明，release 模式的 F12 由 `D:\HT9045\web\page\theme.js` 封鎖，除錯模式才看得到；C++ 版 HMI 用哪個模式開見 9.2 第 4 點）；攔到一次摘要不能重送；日誌不可能意外印出密碼。
- **擋不住**：
  - 能讀程式或檔案的人：`D:\HT9045\system\login.dat` 的轉換可以還原（金鑰寫在程式裡）、`D:\HT9045\system\SG_PW.ini` 是明文、golden 執行檔裡就有廠商密碼。
  - 攔到一次「亂數＋摘要」的人可以離線猜**短**密碼（全數字、位數少的密碼很快就猜到）。
  - 本機其他程式可以直接連 wb_serve 一直猜（不帶 Origin 的連線是允許的；golden 也沒有次數限制，Q45-6）。
  - 旁邊看人打字。
- 網頁伺服器只聽本機（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.h:39`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4305`），能看到明文訊息的人本來就能直接讀密碼檔 ⇒ 全面 A 的實際保護增加有限，主要價值是「符合 S55 字面『傳輸內容不能有明文密碼』」與「不可能從日誌外洩」。

### 3.4 主畫面登入（auth.login）要不要也改成摘要

- **現況**：帳號放 tag、密碼放 value（`D:\HT9045\web\page\ht9045_recipe_client.js:550-552`）；下拉模式 value＝密碼（`D:\HT9045\web\page\ht9045_recipe_client.js:559-563`）；改密碼的舊／新密碼放在 security.passwd 的內容裡（組內容 `D:\HT9045\web\page\ht9045_wire_statussecurity.js:570`、送出 `D:\HT9045\web\page\ht9045_wire_statussecurity.js:442`）。都是 Q10＝B1 決定維持的。
- **要改的話**：
  - C++：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp` 的登入、下拉選單比對改收摘要（沿用 3.3 的候選比較）。約 1 人天，St01。
  - 網頁：`D:\HT9045\web\page\main.html:761-800`、`D:\HT9045\web\page\ht9045_recipe_client.js` 的 authLogin／authSelect 先要亂數再送摘要。約 0.5～1 人天，Jimmy。
  - 10 支探針（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\` 裡用 auth.login 的，例 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\s12c_config_probe.py`）改成算摘要（Python 內建 hashlib／hmac）。約 0.5 人天，St01。
  - 告警框密碼層（dialog.auth）的契約 `D:\HT9045\web\JSON\js\Dialog-bridge-contract.js`（現在寫「帳密原字只走本機」）與 `D:\HT9045\web\page\dialog-bridge.js:489-520` 一起改。約 1 人天，Jimmy＋St01。
  - **改密碼的新密碼沒辦法用摘要**：C++ 要把新密碼寫進密碼本（`D:\HT9045\system\login.dat` 帳密都要轉換後存），一定要拿到原字。要「完全不上線」只能做真的加密（瀏覽器 ECDH＋AES-GCM，C++ 端也要自己寫這兩樣），+2～3 人天；舊密碼可以改成摘要。
  - 合計約 3～4 人天（不含新密碼加密）；+2～3 人天（含）。
- **代價以外的問題**：要 Steven 推翻 Q10＝B1；只改 auth.login、不改改密碼與告警框，全站還是有地方送明文，等於沒達成「不送明文」。
- **建議**：現在不改（Q45-8）。要做就「所有送密碼的地方一起改」排成一波，並另外決定新密碼怎麼傳。

---

## 4. 工作量與歸屬（人天；粗估）

### 4.1 建議組合（甲現在做、乙丙維持改不了）

| 部分 | 內容 | 誰 | 人天 |
|---|---|---|---|
| C1 | 重新登入核心（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp` 檔尾；抽出比對本體、補下拉模式 SetUp 那一臂、強制登出） | St01 | 1 |
| C2 | SetUp 存檔接上（改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_SetUp.py`、重產 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc`；取消＝失敗） | St01 | 0.5 |
| C3 | Configuration 存檔接上（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp` 的 M01 段；失敗改回；強制登出時機照 Q45-3） | St01 | 1 |
| C4 | 開頁回「哪些要問、要幾級、哪種模式」 | St01 | 0.25 |
| C5 | ctest＋探針（第 6 節） | St01 | 1 |
| W1 | 頁面補件兩支（`D:\HT9045\web\page\ht9045_setup_c_wire.js`、新檔 `D:\HT9045\web\page\ht9045_iniconfig_auth_c.js`） | St01 | 1 |
| J1 | 主畫面登入顯示跟著變（`D:\HT9045\web\page\main.html`） | Jimmy | 0.25～0.5 |
| V | 模擬＋上機（Steven01：要一個等級 3 的帳號） | St01＋Steven | 0.5 |
| | **小計** | St01 約 5、Jimmy 約 0.5 | **約 5.5** |

### 4.2 全面 A（7 點全做＋一次性亂數摘要），在 4.1 之上再加

| 部分 | 內容 | 誰 | 人天 |
|---|---|---|---|
| A1 | C++ SHA-256／HMAC／亂數＋測試向量 | St01 | 1 |
| A2 | 亂數表（auth.challenge）＋摘要比對（含重新登入改收摘要） | St01 | 1.5 |
| A3 | 廠商密碼三點（#1～#3）：照 golden 的失敗語意（2.1～2.3）＋比對目標（Q45-7） | St01 | 1 |
| A4 | SG_PW 解鎖（#5）：auth.unlock、檔不在寫預設、區塊到程式關掉都看得見、頁面重讀 | St01 | 1 |
| A5 | 網頁：自帶 JS 雜湊＋測試向量、#5 連點兩下、各點提示 | St01 | 1.5 |
| A6 | 主畫面「L／X」光學尺標記（#2 的副作用，網頁主畫面沒有） | Jimmy | 0.5 |
| A7 | 測試：重送、過期、跨連線、日誌掃描 | St01 | 1 |
| | **小計** | St01 約 7、Jimmy 約 0.5 | **約 7.5**（連 4.1 合計約 13） |

### 4.3 主畫面登入也改成摘要（3.4）

約 3～4 人天（St01 約 2、Jimmy 約 1.5）；新密碼要加密再 +2～3。

---

## 5. 風險

### 5.1 照翻 golden 之後，操作員會覺得奇怪的地方（全部是 golden 本來就這樣）

1. **M01＋[A01_2]**：開了「切到 Operator 後不能存設定」的機台，改 M01 會先被登出、再被擋存檔（2.4）⇒ Q45-3。
2. **取消登入框＝登出成 Operator**（golden 取消後比對空白帳密）⇒ Q45-4。
3. **下拉模式的 M01**：比的是「下拉目前選的那一級」的密碼；錯了等級變 0、畫面不變 ⇒ Q45-5。
4. **「使用 PE 模式」那組**：點的是別格，錯了卻是 PE 模式被取消（2.1）。
5. **SetUp 沒有「第 37 項＝0 就不問」的捷徑**：第 37 項設 0 的 RTC 機台照樣要登入一次（一定過）；而且那一次登入會改掉主畫面的登入者。
6. **#5 每次連點兩下**，檔案不在就寫一份預設密碼檔；解鎖後區塊一直看得見到程式關掉。

### 5.2 密碼外露

- **既有**：移植樹的查證文件 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RECON_cConfiguration_displayside.md` 第 132、268 行寫著 golden 的廠商密碼那段字（commit `80ebbb2e`，20260820 加入，主要檔就是這一份）。repo 曾經公開過（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:63-64`：「repo 裡不可以放權杖、密碼、客戶資料」）⇒ Q45-10。查證只比對「有沒有同一段字」，沒有印出。
- **做 #1～#3 就要讓 C++ 拿到比對目標**：照 golden 就是把那段字寫進移植樹原始碼，跟上面那條規則衝突 ⇒ Q45-7。
- 移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp` 已經照 golden 帶著寫死的比對值（下拉模式那幾臂，程式與註解裡都有）；本設計不再新增。

### 5.3 版本號：V906 上的廠商密碼跟 BCB 版不同

golden 的廠商密碼＝寫死的那段字＋程式版本號（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6779-6780`；版本號來自執行檔版本資源 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:1384`）。移植樹的版本號取法相同（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cpublic.cpp:2161-2169`，取版本的第 3、4 段），而 wb_serve.exe 的版本資源是 3.33.906.0（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.rc:5`，開機設定 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:3867`）⇒ V906 的廠商密碼跟 V912 BCB 版不同；客戶拿到的 BCB 版密碼在 V906 上不能用，要有人另外告訴客戶。

### 5.4 登入狀態被設定頁改掉，主畫面沒跟上

重新登入會改掉全機的登入者（golden 也是）。主畫面 `D:\HT9045\web\page\main.html` 現在只在自己按登入時才重讀，沒在聽 auth.level ⇒ 不做 J1 的話，主畫面會顯示舊的登入者（例：SetUp 打錯密碼後實際是 Operator，主畫面還寫 Engineer）。

### 5.5 時機偏離

golden #4 是「點下去當下」問、每點一格問一次；建議做法是「按存檔時」問一次。結果一樣（改了哪幾格、錯了改回、最後登出），但操作員的體驗不同 ⇒ Q45-2。

### 5.6 其他

- 本機其他程式可以連 wb_serve 一直猜密碼（golden 登入框也沒有次數限制）⇒ Q45-6。
- 中文等非英數字密碼：網頁送 UTF-8，密碼本是 Big5，本來就對不上（現在的 auth.login 也一樣）；golden 小鍵盤是「不收符號、不收空白」模式，實務上都是英數字。
- 舊頁面（補件沒更新）照舊擋，不會默默存進去（3.1 第 2 條）。
- 多個瀏覽器分頁：登入狀態全機一份；某一頁重新登入後，其他背景頁下次存檔會被「請重讀」擋一次（現在主畫面換人登入也是這樣）。

---

## 6. 測試計畫

測試一律用**測試程式自己產生的假帳號、假密碼**（每次執行隨機產生），絕不用 golden 的密碼、不讀 `D:\HT9045\system\login.dat`、不讀 `D:\HT9045\system\SG_PW.ini`。

### 6.1 ctest（MinGW，兩組態 gate）

1. **Auth_Reauth**（新，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_auth_reauth.cpp`）：
   - 文字密碼本：暫存檔，用測試縫 W906_PWBOOK_PATH 指過去（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5501-5511` 同一個縫）。
   - 二進位密碼本（login.dat 格式）：用 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\LoginDatBook.h` 寫一份暫存檔（handler 版轉換），W906_PWBOOK_PATH 指過去（二進位判斷 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:1197`）。
   - 下拉模式：W906_LOGINDAT_PATH（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:667`）。
   - 權限表第 37、92 項與 REAL_TIME_CCD 在記憶體裡設。
   - 案例：沒裝 RTC 不問；第 92 項＝0 不問（只 Configuration）；第 37 項＝0 照問且一定過；對且夠 → 過、登入者換人；對但不夠 → 失敗、登入者換人；打錯 → Operator；取消 → Operator；Configuration 版問完登出、SetUp 版不登出；下拉模式 SetUp 特殊比對（下拉選 Engineer、打 Supervisor 等級那一格的密碼也過）；下拉模式 Configuration 只比目前那一級；大小寫；帳號大小寫。
2. **SetUp 存檔整合**（比照既有的 C 路測試，例 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt:4647` EditList_PageIndex、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt:4673` HSys_HeaterMix 的寫法）：帶／不帶 reauth、取消、RTC 與 OCR 一起取消只問一次；檢查勾有沒有改回、其他欄位有沒有存、事件紀錄有沒有「RealTimeCCD Disabled」、回應的 login。
3. **Configuration 存檔整合**：M01 帶／不帶 reauth；[A01_2] 開／關（照 Q45-3 的結論）；#1～#3 仍整次拒存。
4. **日誌掃描**：測試期間把標準輸出接起來，斷言假密碼字串一次都沒出現；回應 JSON 裡也沒有。
5. **全面 A 另加**：Auth_Crypto（SHA-256 用 FIPS 180-4 的「abc」向量、HMAC 用 RFC 4231 第 1、2 組）；Auth_Challenge（用一次就作廢、60 秒過期、換連線不收、重送不收、比較函式是固定時間那一支）。網頁那份 JS 雜湊用同一組向量（放在 `D:\HT9045\web\tests\`）。
6. **收工比對失敗清單**：只有既知的 6 個（config_db、IniFiles、ini_helpers、config_loaders、dfm2rc_idempotent、GA1_ReadGeneralIni）可以失敗，其他任何失敗都是回歸。

### 6.2 探針（瀏覽器以外）

新 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\q45_reauth_probe.py`，啟動方式照既有探針（測試密碼本＋`--allow-cmd --root D:\HT9045\web --port 8046`，例 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\s12c_config_probe.py:16`）：送 editlist.get 看 armed；送帶 reauth 的 editlist.save 看結果；最後掃 wb_serve 的輸出檔，假密碼 0 次。

### 6.3 上機（Steven01）

- SetUp：用等級 3 的帳號關 RTC → 過、事件紀錄有一筆；用 Engineer → RTC 勾回、主畫面變 Engineer；打錯 → 主畫面變 Operator。
- M01：這台第 92 項＝0 永遠不問 ⇒ 要測 M01 的路，用測試縫 W906_LEVELSET_PATH 指一份暫存的權限表（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLevelSet.cpp:624` 有列這個縫），不要改真的 `D:\HT9045\system\levelset.dat`。

---

## 7. 建議

1. **Q45 選「甲現在做、乙丙維持改不了」**（Q45 裡 St01 的建議），細節照 3.2：在按存檔時問、跟著存檔一起送、密碼跟現在主畫面登入一樣只走本機（Q10＝B1）。先做 SetUp（#6、#7：Steven01 這台就會問），再做 Configuration（#4）。
2. 訊息欄位先留好擴充：reauth.password 以後可以換成 reauth.challengeId＋proof，頁面與 C++ 的其他部分不用動。
3. 乙丙（#1、#2、#3、#5）等客戶真的要在網頁上切再做，而且要先決定廠商密碼的比對目標放哪（Q45-7）。
4. 主畫面登入現在不改成摘要（Q45-8）。
5. 先把 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RECON_cConfiguration_displayside.md` 裡的廠商密碼遮掉（Q45-10）。

---

## 8. 要 Steven 決定的細節（Q45 的子題）

### Q45-1. 「重新登入」那一次的密碼怎麼送給 C++
**背景**：#4、#6、#7 要操作員重新輸入帳號密碼。現在主畫面登入是把密碼原字放在訊息裡、只走本機 127.0.0.1（Steven Q10＝B1「網頁傳輸維持現狀」）。S55 的字面是「密碼不可以明文出現在網頁或傳輸內容裡」，後來 Q10＝B1 把網頁→C++ 這個方向定成維持原字。
**選項**：A 跟主畫面登入一樣送原字（只走本機；約 5.5 人天就能三點全做）／B 做一次性亂數摘要（3.3；密碼不上線；再加約 4 人天：雜湊、亂數表、網頁雜湊、測試）。
**St01建議**：A；訊息欄位先留好，以後要換 B 只換那一個欄位。例：Engineer 在 SetUp 關 RTC、輸入帳號 eng1 和密碼 → A：存檔訊息裡有 password 欄位（F12 除錯模式看得到，release 模式看不到）；B：存檔訊息裡只有 64 個字的摘要，同一段摘要再送一次會被拒。
**目前狀態**：等 Steven 決定，程式沒動。

### Q45-2. Configuration「監控功能」16 格（#4）什麼時候問
**背景**：golden 每點一格就問一次，而且每次問完都登出成 Operator（下一格又要重新登入）。網頁版有一道「開頁時的等級要等於存檔時的等級」檢查（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:239`），點下去就登出會讓之後的存檔被「請重讀」擋掉。
**選項**：A 按「存檔」時問一次，涵蓋這次改過的全部格子；錯了全部改回（[W906] 時機偏離）／B 照 golden 每點一格就問一次：要一張「只能用一次的通行證」記住每一格已經問過，並放寬上面那道等級檢查（「golden 重新登入造成的等級變化不算」），約多 1～1.5 人天。
**St01建議**：A。例：改了「Auto speed 必須開」和「啟用 ATC」兩格 → golden 問兩次、中間被登出一次；A 問一次，兩格一起過或一起改回；B 問兩次，跟 golden 一樣。
**目前狀態**：等 Steven 決定，程式沒動。

### Q45-3. #4 問完「一律登出成 Operator」和「切到 Operator 後不能存設定」打架，要照翻嗎
**背景**：客戶開了 [A01_2]（切到 Operator 後不能存設定，golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:5828-5832`）的 RTC 機台，改 M01 一定先被登出，關頁存檔時又被這一條擋掉 ⇒ golden 上 M01 的變更（連同這次的其他變更）永遠存不進去（2.4；由程式推得，沒有在 BCB 機台上試）。
**選項**：A 照 golden：登出在存檔之前 ⇒ 開了 [A01_2] 的機台 M01 存不進去（照翻、程式加註解說明為什麼看起來錯）／B 登出放到存檔之後（[W906] 偏離）⇒ M01 存得進去，存完照樣登出成 Operator。
**St01建議**：A（忠實翻譯優先），並另外通報 Jimmy 這是 golden 的問題。例：某客戶開了 [A01_2]、第 92 項＝Supervisor，Supervisor 改 M01 某一格按存檔 → A：重新登入過了，但接著被登出、頁面顯示「[A01_2] 目前已切換到 Operator 權限，請重新登入再做設定」、什麼都沒存；B：存進去，然後主畫面變 Operator。
**目前狀態**：等 Steven 決定，程式沒動。

### Q45-4. 登入框按「取消」算什麼
**背景**：golden 的登入框按取消，後面照樣拿空白帳密去比 ⇒ 算錯 ⇒ 登入者變 Operator（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:15100-15105`）。
**選項**：A 照 golden：取消＝打錯，勾改回、登出成 Operator（網頁在小鍵盤上先寫明「取消會登出」）／B 取消只把勾改回、不動登入者（[W906] 偏離）。
**St01建議**：A。例：Engineer 取消 RTC 按 Save，小鍵盤跳出來又按取消 → A：RTC 勾回、其他照存、主畫面變 Operator；B：RTC 勾回、其他照存、主畫面還是 Engineer。
**目前狀態**：等 Steven 決定，程式沒動。

### Q45-5. 沒有密碼本的機台（下拉選單模式），golden 的兩個比對怪處要不要照翻
**背景**：下拉選單模式只打密碼、不打帳號。golden（2.0 第 4 點）：SetUp 問的時候不管下拉選到哪一級，先比 Supervisor 等級那一格、再比 Engineer 等級那一格；Configuration 問的時候只比「下拉目前選的那一級」那一格，錯了等級變 0、下拉選單畫面卻不變。移植樹目前少了 SetUp 那一臂（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:238`）。
**選項**：A 兩個都照翻（補 SetUp 那一臂；Configuration 照「目前那一級」比）／B 兩邊都用 SetUp 那一種比法（[W906] 偏離，Configuration 較寬）。
**St01建議**：A；畫面不變那一點，網頁主畫面照實顯示 C++ 的等級（比 golden 清楚，不算改行為）。例：下拉選在 Engineer 的機台，在 Configuration 改 M01、打 Supervisor 等級那一格的密碼 → A：不過（只比 Engineer 等級那一格），M01 改回、等級變 0；B：過。
**目前狀態**：等 Steven 決定，程式沒動。

### Q45-6. 密碼錯太多次要不要暫停（golden 沒有）
**背景**：golden 登入框沒有次數限制。wb_serve 只接受本機的網頁，但本機的其他程式（不是瀏覽器）可以直接連進來一直試（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.h` 的來源規則說明）。
**選項**：A 不做（照 golden）／B 同一個點 1 分鐘內錯 5 次就暫停 30 秒（[W906] 偏離），也套到主畫面登入。
**St01建議**：A（重新登入錯了本來就會被登出成 Operator，猜的代價已經不小）；若之後做了廠商密碼（#1～#3，同一組密碼可以一直試），再做 B。例：某程式在一分鐘內送上百次錯的密碼 → A：每一次都照常比、照常回錯；B：第 6 次起 30 秒內直接回「暫停中」。
**目前狀態**：等 Steven 決定，程式沒動。

### Q45-7. （只有要做 #1～#3 時才需要）廠商密碼的比對目標放哪
**背景**：golden 把一段字寫死在程式裡，後面接版本號（5.3）。C++ 要比對就得有這段字或它的替代品；repo 規則是不可以放密碼（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:63-64`）。
**選項**：A 照 golden 把那段字寫進移植樹程式／B 程式裡只放「那段字＋"906.0"」的 SHA-256（配合 3.3 的摘要做法；看得到這個值的人可以通過網頁這一關，但推不回 BCB 版的密碼；版本號一改就要有人重算）／C 放在機台本機一個不進 git 的檔（例 `D:\HT9045\system\` 底下；檔案不在就維持改不了）。
**St01建議**：B。例：出一版 V906.1 → A：什麼都不用做，但 repo 裡有廠商密碼；B：發版的人要重算一次、更新程式裡的值；C：每台機台要放一次檔案。
**目前狀態**：等 Steven 決定，程式沒動（#1～#3 目前維持改不了）。

### Q45-8. 主畫面登入（auth.login）、下拉選單登入要不要也改成摘要
**背景**：3.4。Q10＝B1 定了「網頁傳輸維持現狀」。改的話約 3～4 人天；改密碼的新密碼沒辦法用摘要傳，要真的加密再 +2～3 人天；只改登入、不改改密碼與告警框，全站還是有地方送原字。
**選項**：A 不改（Q10＝B1 維持）／B 排一波「所有送密碼的地方一起改」（登入、下拉、重新登入、告警框、改密碼的舊密碼），新密碼另外決定。
**St01建議**：A。例：操作員在主畫面登入 → A：訊息裡有密碼原字，只走本機；B：訊息裡只有摘要，F12 也看不到密碼。
**目前狀態**：等 Steven 決定，程式沒動。

### Q45-9. （只有選了摘要時才需要）網頁用什麼算摘要
**背景**：瀏覽器內建的雜湊（crypto.subtle）只在 https、http://127.0.0.1、http://localhost 可用（3.3.5）。
**選項**：A 網頁自帶一份 JavaScript 雜湊程式（約 150 行，不用 npm，用測試向量驗過）／B 用瀏覽器內建的。
**St01建議**：A。例：有人用區網 IP 開 HMI（`http://192.168.1.20:8045`）→ A：照常能登入；B：瀏覽器沒有這個功能，登入框送不出去。
**目前狀態**：等 Steven 決定，程式沒動。

### Q45-10. 移植樹文件裡已經有 golden 的廠商密碼，要不要遮掉
**背景**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RECON_cConfiguration_displayside.md` 第 132、268 行寫著 golden 寫死的那段字（commit `80ebbb2e`，Jimmy 20260820 寫的；主要檔就是這一份）。repo 曾經公開過。⚠ ST01-E 21:5x 補：同一個 repo 也收了 golden V912 原始碼（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp` 是 git 追蹤的檔），那段字本來就在 repo 裡；只遮文件擋不住翻原始碼的人，遮文件的用處是「讀文件的人不會順手看到」。文件是 Jimmy 的，選 A 要先知會 Jimmy。
**選項**：A 把現行檔那兩處換成「（廠商密碼，不寫出）」，git 歷史裡還在／B A 再加清掉 git 歷史（要所有人重新 clone，影響 Jimmy、Steven02）／C 不動。
**St01建議**：A；歷史要不要清由 Steven 判斷（那段字 golden 執行檔裡本來就有）。例：有人打開那份文件 → A：看不到；B：看不到，翻 git 歷史也看不到；C：看得到。
**目前狀態**：等 Steven 決定，文件沒動。

---

## 9. 查證紀錄與沒查證的地方

### 9.1 下過的指令（20260927 21:00～21:50，全部唯讀）

- golden 原始碼一律用自己的腳本以 cp950 解碼讀取，讀的時候把「疑似密碼」的字串與長數字遮掉，只看程式結構；三個廠商密碼字串只輸出「長度 23、三個相同」；移植樹與網頁全樹搜尋那段字，只輸出「哪個檔、第幾行」（得到 5.2 那一份文件）。
- 「移植樹沒有 SHA-256、沒有密碼學亂數」：在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0` 對 *.cpp／*.h 不分大小寫搜 `sha256|sha-256|CryptGenRandom|RtlGenRandom|BCryptGenRandom|SystemFunction036`，只有兩處註解提到 sha256（檔案校驗值），沒有實作（20260927 21:3x）。平行工作可能讓這個結果過時，開工前要重搜。
- 權限表第 37、92 項：讀 `D:\HT9045\system\levelset.dat` 的第 37、92 個整數（這個檔沒有密碼；結構照 golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cprod.h:1164-1168`）。
- `D:\HT9045\system\SG_PW.ini` 只看檔案大小；`D:\HT9045\system\login.dat` 沒有碰。

### 9.2 沒查證／推論的地方

1. 2.4 的「M01＋[A01_2] 永遠存不進去」是讀程式推得，沒有在 BCB 機台上試。
2. 移植樹的 [A01_2] 檢查（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc:7628`）在「存檔中途登出」之後會不會照樣擋，沒有追到底（要等 C3 實作時用測試確認）。
3. 舊版 MinGW 的 std::random_device 不隨機，是已知問題的記憶，沒有在本機工具鏈上實測；CryptGenRandom 在本機 MinGW 標頭裡有沒有，也沒有編譯確認。
4. crypto.subtle 在 `http://127.0.0.1:8045` 可用，是依瀏覽器規格；沒有在機台的 Edge 上打開開發者工具確認。release 啟動檔 `D:\HT9045\HT9045_Release.cmd:16` 開的還是舊的 file:// 外框（`D:\HT9045\background.html`，C# 模擬器時代），正式機台用哪個網址開 C++ 版 HMI 沒有確認。
5. golden「另一個輸入畫面正開著時不問」（fInput／fQwertyKey）在網頁上一律當沒開，是沿用移植樹既有的判斷，沒有逐一確認 golden 什麼情況會是開著。
6. 工作量是粗估，沒有拆到函式。
7. 沒有 build、沒有跑 ctest、沒有啟動 wb_serve。

---

## 10. 實作紀錄（20260930，B5）

> 撰寫：ST01-E 派的工程師（St01），20260930；ST01-E 審查。
> 依據：Steven Q45「按照你的建議執行」（子題 1A 2A 3A 4A 5A 6A 7B 8A 9A 10A）、Steven 20260929「請按照bcb的邏輯處理 … 你問題也太多了!」⇒ 照 golden 做，沒有另外提問。
> 派工列：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md` 的 B5（C-3、SU-L1、CC-L4、CC-E5）。
> 分支 `v906/steven-cbridge-review6`。C++ 部分（C1～C5）由 ST01-E 審查後 commit 成 `699dc06d`；頁面（W1）、ST01-E 審查意見第 1 條（帳密副本清成 0）與 ST01-E 審查加的一條（頁面沒載入小鍵盤時整次不送，不當成按了取消）一起 commit，見 git log。

### 10.1 做了什麼（白話）

- **SetUp（#6、#7）**：裝了 Real Time CCD 的機台，開頁時勾著「Enable Real Time CCD」或「Enable OCR Function」、按 Save 前取消了 ⇒ 網頁先跳跟主畫面一樣的登入小鍵盤（有密碼本：先帳號、再密碼；下拉選單模式：只有密碼），帳號密碼跟著這次存檔一起送。C++ 照 golden `TfSetup::DoPassword` 比對：等級到權限表第 37 項 ⇒ 關掉並記「RealTimeCCD Disabled」／「OCR Disabled」；不夠、打錯、按取消 ⇒ 勾改回、其他設定照存；登入者照 golden 變成這次登入的人（打錯或取消＝Operator）；SetUp 版不登出。RTC、OCR 一起關只問一次（golden 同）。
- **Configuration（#4）**：裝了 RTC、權限表第 92 項不是 0 的機台，改了「監控功能」[M01] 16 格中任何一格、按存檔 ⇒ 同一個登入小鍵盤。C++ 在套值之後、golden 存檔（FormClose）之前照 golden `TfConfiguration::DoPassword` 問一次：過了就存；不過就把改過的 M01 格子全部改回開頁值、其他照存；有密碼本時**不論過不過一律登出成 Operator**（golden 同）。
- **#1～#3（廠商密碼）、#5（SG_PW）**：照 Q45 維持改不了；拒存說明改成「這一項要廠商密碼，網頁版不提供，請到 BCB 版機台改」。
- **CC-E5（Configuration 的 Supervisor／Vender 密碼鈕）**：查過 golden，這顆鈕（BitBtn1，Caption「Set Vender Password」）在 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm:817` 是 `Visible = False`（V899 `D:\HT9045\HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422\cConfiguration.dfm` 同），V912 全樹沒有任何程式把它設成看得見（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp` 只有 :6052 處理器本身提到它）⇒ golden 操作員點不到。照 golden：C++ 不加寫入路徑（加了就是走不到的死碼）；網頁原本把它畫成看得見、點了沒反應，改成照 DFM 藏起來。
- 例（Steven01 這台，第 37 項＝3、第 92 項＝0）：Engineer 取消 RTC 按 Save → 跳小鍵盤 → 輸入等級 3 的帳號密碼 → 存檔、事件紀錄多一筆「RealTimeCCD Disabled」、主畫面變成那個帳號。改 M01：第 92 項＝0，golden 不問、也不登出，網頁同樣不跳小鍵盤。

### 10.2 檔案（全部在分支 `v906/steven-cbridge-review6`）

| 檔 | 改什麼 | 狀態 |
|---|---|---|
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp` | 只在檔尾附加（1420 行起；St02 的 1～1419 行逐位元組沒變）：`W906_Reauth`（兩支 DoPassword）、存檔暫存 `W906_ReauthTake`／`W906_ReauthAck`／`W906_ReauthClear`／`W906_ReauthHasAnswer`、SetUp 呼叫點 `W906_ReauthSetupDoPassword`、Configuration 呼叫點 `W906_ReauthConfigM01`、開頁 `W906_ReauthOpenJson`、控制項名單 `W906_ReauthControls` | `699dc06d`；1535、1602～1603 行（打的帳號密碼的副本在區塊結束時清成 0，ST01-E 審查意見第 1 條）在工作樹 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebReauth.h` | 新檔：上面那幾支的宣告與訊息格式說明（只用標準型別） | `699dc06d` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_SetUp.py` → `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc` | `SU_DoPassword` 改呼叫 `W906_ReauthSetupDoPassword`；沒帶帳密時照舊 `filerw::ELPasswordRefused`。只用 `--only TestIF_File_SetUp` 重產，產生檔只動第 70 行 | `699dc06d` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.cpp` | 第 31 行（原本的空行）include；第 213 行開頁回應多 `extra.auth` | `699dc06d` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp` | 第 37 行（原本的空行）include；拒存說明與 M01 檢查（有帶帳密就不在這裡拒，行數不變）；第 331 行 `IC_ReauthM01()`；第 458 行開頁 `extra.auth`；檔尾 `IC_ReauthM01`（golden cbM01Click） | `699dc06d` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` | editlist.save 那一臂三處同一行插入（5308 拿出 reauth＋結束時一律清掉、5309 拒存、5333 回應加 reauth）；4389 行沒動 | `699dc06d` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_weblogin_reauth.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt`（檔尾一段） | 新 ctest WebLogin_Reauth | `699dc06d` |
| `D:\HT9045\web\page\ht9045_setup_c_wire.js` | 存檔包裝加「先問再送」＋第 (7) 段（小鍵盤、鎖鈕、結果訊息、WAR1677 不停機告警） | 工作樹（`git add -f`） |
| `D:\HT9045\web\page\ht9045_iniconfig_auth_c.js` | 新檔：Configuration 的 M01 重新登入＋藏 BitBtn1 | 工作樹（`git add -f`） |
| `D:\HT9045\web\page\Config.Configuration.html` | 第 128 行：`ht9045_iniconfig_p26_c.js` 後面、註解之前加一個 script | 工作樹（`git add -f`） |

### 10.3 golden → 移植樹對照

| golden（V912） | 移植樹 |
|---|---|
| `TfSetup::DoPassword` `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:4325-4362`（第 37 項、沒有「＝0 就不問」捷徑、不登出） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp` `W906_Reauth("TestIF_File_SetUp", …)` |
| `TfConfiguration::DoPassword` `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6482-6529`（第 92 項＝0 直接過；有密碼本時問完登出 :6518-6525） | 同上 `W906_Reauth("IniConfig", …)` |
| `sbUpdateClick` 呼叫與改回 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:3573-3606` | 產生檔原本就照翻；`SU_DoPassword` → `W906_ReauthSetupDoPassword`（回應的 reverted 照 :3578-3581 算） |
| `cbM01Click` `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6531-6544`（錯了改回、`gbM01->Visible=cbM01->Checked`） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp` 檔尾 `IC_ReauthM01` → `W906_ReauthConfigM01` |
| `cbUserSelectChange` 密碼本分支 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:14918`、比對 :15264-15296、錯誤 :15333-15344 | St02 的 `WebLogin_BookCompare`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:383`）——不經過 btLogin 那一層（golden DoPassword 直接叫 cbUserSelectChange），所以已經登入的人也照樣問 |
| `stOperatorClick` `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13193-13324`，SetUp 那一臂 :13283-13301 | St02 的 `stOperatorClick`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:167`）＋ `g_W906SetupAsksPassword`（只在 SetUp 呼叫期間＝golden `fSetup->bNeedPassword` 時立起來）→ `W906_StOperatorClickAskArm`（:1403） |
| `btLoginClick` `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28080-28094` | 重新登入**不走**這一層（`WebLogin_BookLogin` 只給主畫面 auth.login） |

### 10.4 照翻的 golden 怪處（操作員會覺得奇怪，但 golden 就是這樣）

1. 取消登入框＝空白帳密＝錯 ⇒ 登入者變 Operator（Q45-4＝A）。密碼本模式還會照 golden 出 WAR1677（網頁發不停機告警，機台不停）。
2. Configuration 版問完一律登出成 Operator（有密碼本時），登出在 golden 存檔之前（Q45-3＝A）⇒ 開了 [A01_2]「切到 Operator 後不能存設定」的機台，這次什麼都不存（狀態列出現 C++ 的「[A01_2]目前已切換到Operator權限」）。這一條由程式推得，沒有在 BCB 機台上試過，要通報 Jimmy。
3. 下拉選單模式：Configuration 只比「下拉目前那一級」的密碼；錯了等級變 0 但下拉選單不動、也不登出（golden 只在有密碼本時登出）（Q45-5＝A）。SetUp 且開頁時 RTC 勾著：先比 Supervisor 那一格、再比 Engineer 那一格，不看下拉。
4. SetUp 第 37 項＝0 也照問（一定過），而且那一次會改掉主畫面的登入者。
5. 登入後等級不夠（例：Engineer 去關 RTC）：主畫面照樣變成 Engineer，只是勾改回。
6. 同一次存檔勾 cbM01 又勾子格：開頁時 cbM01 沒勾 ⇒ gbM01 看不見 ⇒ 子格這次照通用規則被丟（回應 ignored）；存完重讀才改得到子格（golden 也要先點 cbM01 才看得到子格）。

### 10.5 跟 golden 不同的地方（寫明）

1. **時機**（[W906]，Q45-2＝A）：golden 每點一格 M01 就問一次、每次都登出；網頁按存檔時問一次，涵蓋這次改過的全部格子，錯了全部改回。
2. 密碼本模式／下拉模式的判斷：`FileExists(pwPath)`（golden DoPassword 的 bTechComExist）；有設測試縫 `W906_PWBOOK_PATH` 時用它（同主畫面 auth.login 與 security.passwd），正式機台沒有設，就是 golden。
3. golden 登入框的 Label3／Label4（等級不夠的字）沒有畫；原因放在回應的 `reauth.reason`，頁面顯示在狀態列。
4. 設計 §2.0 寫「錯了或取消記 Operator login」：那是 golden 另一臂（Greatek 從 FTP 下載密碼本，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:15100-15105`）。本機走的那一臂錯誤時（:15333-15344）不記事件、只跳 WAR1677，移植樹照這一臂（St02 的 `WebLogin_BookCompare`）。
5. 產生器 `_auto()` 說明字串裡的「有裝視同密碼錯；R75」沒改（改了產生檔 98 行只動註解），以 `SU_DoPassword` 那一行的說明為準。
6. 殘留（同主畫面 auth.login，設計 3.2.5）：防連點表以 value 原位元組當鍵，密碼會在 C++ 記憶體裡留到防連點視窗結束；`pw::Acp` 轉碼時的暫存沒有清；頁面 `qwerty.js` 的模組變數到下一次開小鍵盤才被蓋掉。不寫檔、不印出。

### 10.6 訊息格式（實作後）

- 開頁 `editlist.get` 的 `extra.auth`：`{"mode":"book|select","realTimeCcd":b,"points":[…]}`。SetUp：`rtcOff`／`ocrOff`（kind relogin、armed、armedAtOpen、levelItem 37、level、levelZeroSkips:false、logoutAfter:false）。Configuration：`m01`（armed＝裝了 RTC 且第 92 項≠0、levelItem 92、levelZeroSkips:true、logoutAfter＝有密碼本）＋`c12`／`a27`／`n07_5`（kind vendor）、`sgpw`（kind sgpw），後四點一律 armed:false、why＝網頁版不提供。
- 存檔 `editlist.save` 的 value：`"reauth":{"point":"rtcOff|ocrOff|m01","userId":"…","password":"…"}` 或 `{"point":…,"cancelled":true}`，跟 widgets 並列。放進 widgets 裡、點不對、這一頁沒有重新登入點、型別不對 ⇒ 整次拒存（C++ 先把字清掉）。
- 存檔回應開頭多 `"reauth":{point, answered, asked, handled, passed, cancelled, mode, levelItem, required, levelBefore, level, loggedOut, alarm?, reason, reverted[], golden, login{WebLogin_StateJson}}`，不含密碼。帶了但 golden 這次沒問 ⇒ `asked:false`、登入沒變。

### 10.7 測試

- 新 ctest **WebLogin_Reauth**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_weblogin_reauth.cpp`）：117 項全過。帳號密碼每次執行隨機產生；密碼本、標準輸出擷取、三個登入測試縫都在 build 目錄的 `weblogin_reauth_scratch`；不在 ctest 環境就拒跑；最後比對 `D:\HT9045\system\login.dat`、`levelset.dat`、`lastdata.dat`、`Gerneral.ini` 沒變。涵蓋：存檔暫存（並列／放進 widgets 拒／點不對拒／沒有重新登入點的頁拒／型別錯拒／清掉）、SetUp 密碼本（過、錯、取消、等級不夠、第 37 項＝0、沒裝 RTC、沒帶答案、reverted）、SetUp 下拉（SetUp 那一臂、只關 OCR 時的一般比對、Supervisor 特別密碼、取消）、Configuration 密碼本（過／錯／不夠都登出、改回、第 92 項＝0 與沒裝 RTC 不問不登出、沒帶答案改回、主畫面已登入照樣問）、Configuration 下拉（只比目前那一級、不登出、下拉不動）、開頁 extra.auth、四個原始檔的呼叫點是程式不是註解、所有輸出（回應、登入狀態、ExString、標準輸出、這段時間寫的事件紀錄檔）裡假密碼 0 次。
- 重跑相關 19 個 ctest 全過：WebLogin_ForceOperator、WebLogin_Reauth、Security_LoginDatBook、SecurityCore、Security_JamMerge、WebCmdGuard、WB_Server、EditList_PageIndex、HSys_HeaterMix、OpenEnterLog、FormEvent_Position、WebPageTable、WebWindowRegistry、EvB10A_Edges、EvB10B_VclClicks、FShow_Audit、FShow_Audit_SelfTest、P6b_FShowWired、P6b_FShowWired_SelfTest。build 目錄 `D:\AI_TempFile\st01e-q44-build`（SIM 組態，-j4），wb_serve.exe 有 build。完整 gate 沒有跑（同一台機器當時在跑別的 gate）。
- 頁面：node `vm.Script` 語法檢查兩支補件與兩頁的 inline script 都過；node 假環境跑 Configuration 補件 11 項、SetUp 補件 7 項都過（沒有瀏覽器、沒有 wb_serve）。

### 10.8 沒做／交給別人

1. **沒有上機、沒有跑 wb_serve、沒有跑探針**（派工規定不跑 wb_serve；設計 6.2 的 `q45_reauth_probe.py` 沒寫）。SetUp／Configuration 存檔流程裡的接線只由 build＋原始碼檢查＋核心的單元測試證明，沒有端到端跑過。上機請 Steven 用一個等級 3 的帳號在 Steven01 試 6.3 那幾步。
2. Jimmy（`D:\HT9045\web\page\main.html`，J1）：M-12 已在 B6 由 St01 做了一半——`D:\HT9045\web\page\ht9045_main_st01_ev.js` 聽 tag `auth.level`／`user.level` 就叫 `main.html` 的 `HT9045MainLogin.refresh()`。要 Jimmy 確認：下拉選單模式重新登入只改 AccessLevel、不改下拉（golden 怪處），`refresh()` 之後 `sel.selectedIndex` 照 `itemIndex` 不動、`sel.title` 會顯示新的等級——這是照 golden，不是頁面的錯。
3. Jimmy：[A01_2]＋M01 永遠存不進去（10.4 第 2 條）是 golden 的問題，要不要在 BCB 修由 Jimmy 判斷。
4. 頁面產生器沒有照 DFM `Visible = False` 把 BitBtn1 藏起來（`D:\HT9045\web\page\Config.Configuration.html` 裡畫成看得見）：這次由補件藏，產生器那邊要不要修交給 Jimmy。
5. 告警框的密碼指令 dialog.auth 仍回「not wired」（不在 B5；stOperatorClick 另外幾臂 fNote／MyMessageBox／fYieldMonitoring／fBinSel 等 dialog.auth 接上時再補）。（20261001 補記：fNote 那一臂已由 D-026 接上，見 §11；MyMessageBox／fYieldMonitoring／fBinSel 仍沒有。）
---

## 11. 告警框（Alert.Note）的密碼層（20261001，todo D-026）

> 撰寫：ST01-E 派的工程師（St01），20261001。**安全層：合進 main 之前先給 Steven 看。**
> 分支 `v906/st01-d026`（從 `v906/st01-q59` `ec5941ca` 開出），工作樹 `D:\AI_TempFile\st01e-d026`。沒有 commit（ST01-E 核對後 commit）。
> 做法跟 §3（甲）一樣：**網頁不比密碼、只收集操作員打的字；比對只在 C++；密碼只走本機 127.0.0.1 的 WS；回應／log／printf 都不含密碼。** 沒有新的密碼傳輸——用的是本來就在的 `dialog.auth`（`D:\HT9045\web\page\ht9045_recipe_client.js:666`）與 dialog-bridge.js 本來就有的登入層（`D:\HT9045\web\page\Alert.Password.html`）。

### 11.1 golden 做什麼（白話）

- 告警框跳出來的時候（golden `TfNote::FormShow`，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\note.cpp:1496-1502`、`:1818-1941`）就決定「這一則要不要密碼」：
  - **要登入、等級要夠**：權限表 `D:\HT9045\Error\English\JAM0000.dat` 裡這個代碼設的等級不是 0（`fSecurity->GetJamLevel`；大部分客戶碼還把 JAM0301／JAM0302／JAM0508／WAR0310…這一串固定拉到權限表第 35 項）；或 SCC 那幾個代碼、同一個 JAM 連續 N 次（O16）、單位時間內 N 次（O17）、KYEC 工號檢查、條碼 WAR04217。
  - **要輸入 Alarm 解除密碼**：只有 CC_ASE_M（`CosFunction.bUseAlarmUnlockPassWord`），而且這個代碼在權限表勾了「UnlockPassWord」。密碼是 `C:\Windows\AlarmUnlock.ini` 第一行（golden 開機讀，`main.cpp:11381-11403`）。
- 操作員選好 RETRY／SKIP…按 Start 或 Pause（`note.cpp:3599`／`:3604`、`:3921`／`:3926`；只有「確認」的通知框 `:3724`／`:4092`）：先問解除密碼（`DoUnlockPassword` `:5431`），再跳登入框（`DoPassword` `:5277`）——**已經登入的人也照樣問**。
  - 對了而且等級夠 ⇒ 記一筆「==Login for unlock alarm.==」、關框。**有密碼本的機台問完一律登出成 Operator**（力成 CC_PTI 例外）。
  - 錯了、不夠、按取消 ⇒ 框留著（重按再問）。有密碼本時打錯＝變 Operator（golden 的 WAR1677 在框開著時不顯示）。
  - 例外：F15 開著時 JAM0508／JAM0509 只有選 SKIP 才問；VTEST 的 WAR16123 選 RETRY 不問；等 SECS 工號回覆時不問。

### 11.2 網頁怎麼做（白話）

1. C++ 貼告警時照 golden 算好上面那些旗標，告警信箱多寫一段 `auth`（`required:true`、要登入還是解除密碼、要不要帳號欄）——golden 不問的告警，信箱內容跟以前一模一樣。
2. 操作員按 Start／Pause ⇒ dialog-bridge.js 自己跳登入框（跟主畫面一樣的小鍵盤）⇒ 按 OK ⇒ `ht9045_dialog_host.js` 的 `verifyAuth` 把帳號密碼送 C++（`dialog.auth`）。
3. C++ 照 golden 那一次按鍵跑 `DoUnlockPassword`／`DoPassword`（登入、比等級、登出）。過了 ⇒ 回 accepted，並留一張**只給這一則、這個鍵、這個按鈕、用一次**的通行（**不綁連線、不綁操作權杖、不綁登入的人**：Steven Q64 (3) 20261001 09:4x「知道密碼的人就能解」；通知框的通行不比按鈕）；頁面接著送原本的回答（modal.answer／dialog.notifyAck），C++ 看到通行才關框。沒過 ⇒ 登入框留著、顯示 golden 的「Wrong ID or password or Insufficient privileges!!」，可以再輸入。
4. 沒有先過 dialog.auth 就直接回答（舊頁面、別的畫面）⇒ C++ 回 `auth-required`，框不關（golden：DoPassword 回 false 就 return）。
5. 登入框按取消 ⇒ 頁面補送一筆 `{"cancelled":true}`，C++ 照 golden 當空白帳密（有密碼本時變 Operator），框留著。
6. 實體面板的 START／PAUSE：golden 會在機台螢幕上跳登入框；網頁版 C++ 叫不出畫面上的框，所以**面板鍵這時不關框**，要在畫面上按（[W906]，寧可關不掉也不要跳過密碼）。

### 11.3 檔案（全部在 `D:\AI_TempFile\st01e-d026`，分支 `v906/st01-d026`）

| 檔 | 改什麼 |
|---|---|
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp` | 檔尾附加（`:1881` 起；St02 的 1～1419 行、B5 的 W906_Reauth 都沒動）：FormShow 密碼段、`DoPassword`、`DoUnlockPassword`、`W906_NoteAuthArm`／`RequestJson`／`Verify`／`AnswerGate`／`IoGate`／`NoticeGate` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebNoteAuth.h` | 新檔：宣告與訊息格式 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` | 同一行插入，行號不動：`:377`／`:381`（信箱 auth）、`:445`（開框）、`:585`（回答閘）、`:671`（等待迴圈收 dialog.auth）、`:681`／`:684`（面板鍵閘）、`:4889-4904`（通知閘＋主迴圈 dialog.auth，原本的「not wired」拿掉，行數相同） |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_dialog_mailbox.h` | `:576`／`:584-588`／`:611`／`:619`：`AlarmRequestJson`／`AlarmPost` 多一個選用參數（空＝原本的位元組） |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.cpp` | `:1448` 同一行：`dialog.auth` 免操作權杖（同 dialog.response） |
| `D:\HT9045\web\page\ht9045_dialog_host.js` | `:283-352`、`:354`：`verifyAuth`＋取消補送 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_note_auth.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\d026_note_auth_selftest.cjs`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt`（檔尾） | ctest `D026_NoteAuth`、`D026_NoteAuthPage` |

### 11.4 跟 golden 不同的地方（[W906]，寫明）

1. golden 在同一次按鍵裡問完就關框；網頁拆成 dialog.auth＋回答兩個指令，中間靠一次性通行（別的鍵、別的按鈕的網頁回答會讓它作廢；被擋的面板鍵不動它；不綁連線——Steven Q64 (3)，20261001 原本綁連線的那一條已拿掉）。
2. 解除密碼＋登入兩個框同一次按鍵（只有 CC_ASE_M）：一次 dialog.auth 只帶一組字，第一次當解除密碼，回 `stage:"login"`（登入框顯示「Unlock password OK -- now log in」），第二次才是登入。
3. 密碼本打錯：golden 的 WAR1677 在框開著時本來就不顯示，C++ 不呼叫 ShowErrorMessage（同 auth.login 的 Steven 20260924 裁決），回應帶 `alarm:"WAR1677"`，頁面不另發不停機告警。
4. `C:\Windows\AlarmUnlock.ini` 不在：golden 會寫一個內建密碼進去再用它；本樹不帶那個密碼（RULINGS S41／S55）⇒ 什麼都比不中，網頁版解不開，要在機台上建這個檔。
5. 面板鍵（見 11.2 第 6 點）。

### 11.5 沒做（交給別人或以後）

- HandlerResultServer 面板鎖（golden `BtnStartClick` `:3848`／`BtnPauseClick` `:3868` 的 `bNeedTCPAlarm` 直接不給關，要 IT 下指令解鎖）：解鎖指令沒翻，照做會讓框永遠關不掉；等級計算有照做。
- （20261002 更正：SpecialPanel 與 DoPassword_MBox 已在 D-034 做了，見 §12；SECS 工號檢查 St02 照 S25 結案）SpecialPanel 密碼（`bErrPan_err`／`Pwd`）、`MyMessageBox::DoPassword_MBox`、SECS 工號檢查（契約 `dialogAuth.kinds` 的另外三種）、Greatek FTP 密碼本（N15）。
- 開框當下畫面上的 PanSpecialNote、BtnHome 字樣等純顯示的 FormShow 行。
- 筆電的檔：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fNote.h` 的 GATE (N-14) 仍是「成員函式不定義」的連結互鎖（本次是自由函式翻譯，沒動它）；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fNote_ShowError.cpp` 的 GATE(W906-J5-ACK) B3 註解說 dialog.auth 沒接——現在接了，由 wb_serve 的通知閘擋，註解要筆電改。

### 11.6 測試

- `D026_NoteAuth`（C++）：開框旗標、密碼本（過／錯／不夠／取消／CC_PTI／登出）、下拉選單（fNote 那一臂）、F15／VTEST／SCC／KYEC_LEE／WAR04217／O16／統計／SECS 等待、解除密碼（錯／對／兩步／檔案不在）、通知、面板鍵、信箱 JSON、wb_serve 等三個檔的呼叫點；所有輸出與這段時間寫的 log 裡假密碼 0 次；六個真檔沒變。對照組 `W906_D026_SRC_ROOT` 指到改之前的三個檔 ⇒ 紅。
- `D026_NoteAuthPage`（node，離線）：`verifyAuth` 的送出形狀、密碼清空、回應對應、錯誤不帶密碼、不同步 throw、取消補送（沒有密碼欄）。對照組 `W906_DIALOG_HOST_JS` 指到改之前的檔 ⇒ 紅。
- 沒有上機、沒有跑 wb_serve、沒有跑探針。上機請 Steven 在一個權限表設了等級的代碼上試：按 Start 跳登入框 → 錯的不關 → 對的關、主畫面變 Operator（有密碼本時）。

## 12. 另外兩個密碼框（20261002，todo D-034，St01 那一半）

- **SpecialPanel**（golden V912 `note.cpp` FormShow `:1574-1616`、PanSpecialNoteClick `:5489-5507`）：比對目標＝`D:\HT9045\system\SpecialErrNote.ini` `[PASSWORD]`（明文檔，golden 設計）。網頁走 D-026 同一條 `dialog.auth`（信箱 `auth.kind:"special-note"`），鎖著時先比特殊密碼、不跑 DoPassword；對了而這一鍵還要登入 ⇒ `stage:"login"` 兩段式。不綁連線／權杖／登入的人（同 Q64 (3)）。
- **DoPassword_MBox**（`mymessbox.cpp:1228-1286`）：本設計「甲」的第 8 個點 —— Configuration `[I37_1]` FIFO 由關改開（`cConfiguration.cpp:7353-7369`），editlist.save 的 `reauth` point `"i37_1"`，`levelItem` 35、**沒有** REAL_TIME_CCD 條件、第 35 項＝0 照樣問（打什麼都過）、有密碼本一律登出成 Operator。同一次存檔 M01 也改 ⇒ `reauth` 送陣列 `[m01, i37_1]`（golden 問兩次），回應 `reauthAll`；單一物件位元組不變。CC_PTI 出貨組態是廠商密碼 ⇒ 照 #1～#3＝C 不提供。
- 檔案：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp`（B5 段的陣列／點、D-026 段的 SpecialPanel、檔尾 D-034 段）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebReauth.h`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebNoteAuth.h`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\IniConfig.py`＋`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp`（IC_PasswordGuard 看 m01 那一筆）、`D:\HT9045\web\page\ht9045_iniconfig_auth_c.js`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:7324`／`:7326`／`:4897`（認領）。
- ctest：`WebLogin_Reauth` 6b、`D026_NoteAuth` 8b、`D034_IniConfigFifoPage`。
