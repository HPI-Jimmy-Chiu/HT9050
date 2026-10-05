# ★ 已決斷（Steven 已裁決，或 St01 已照建議做）

> 從 `todo.md` 的 ★ 節拆出來（Steven 20260927 要求分檔）。還沒決定的在 [decisions-pending.md](decisions-pending.md)。
> **R 題**是 St01 照建議先做的，Steven 看了有意見仍可推翻（在對話裡說，或寫在該題「目前狀態」後面）。
> 裁決的正式紀錄：St01 的 `HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20260926.md`（S 編號）、筆電的 `RULINGS_20260927.md`（第 N 條）。
> **裁決進度表**（每條裁決的狀態、最後更新、原文連結）：[rulings-index.md](rulings-index.md)（20260927 從 `D:\docs\ops\registers\HT9045_裁決進度表.md` 搬進來）。
> **20261003 分檔**（ST01-M 整理 skill；Steven 1003「把這個skill的內容跟參照整理一下」）：2026-09-30 以前裁決的題目——St01 的 Q1～Q61、R1～R146，St02 的 W1～W62，以及早期「已裁決、不再問」清單——一字不改搬到 [archive/decisions-decided-202609.md](archive/decisions-decided-202609.md)（行號對照寫在該檔開頭）；本檔只留 2026-10-01 以後的。**找題號兩個檔都要搜**。新裁決照舊加在本檔**檔尾**；舊題要補 ⛔ 更正或 ⚠ 追問，也加在本檔檔尾並寫明題號（封存檔不再改）。

## St01（Steven01，資料讀寫）

> **路徑寫法**（Steven 20260927 要求改成絕對路徑）：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` 開頭＝golden V912（BCB6 原始碼，cp950／Big5 編碼，VS Code 要用「Reopen with Encoding」選 Big5 才不會亂碼）；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` 開頭＝移植樹（C++，UTF-8）；`D:\HT9045\HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422\` 開頭＝V899（BCB6，cp950）；`D:\HT9045\web\` 開頭＝網頁。只寫 `:行號` 的，是同一句（或表格同一格）前面那個檔。行號是 20260927 當下的檔案。commit 後面括號裡是那顆 commit 改的主要檔案。
> 裁決紀錄：RULINGS_20260926（S 編號）＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md`；RULINGS_20260927（第 N 條，筆電寫的）＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md`；FROM_STEVEN／TO_STEVEN／CHAT_ST02 的唯讀快照在 `D:\HT9045_handoff\`。

> **Q1～Q61、R1～R146（2026-09-30 以前裁決）已搬到 [archive/decisions-decided-202609.md](archive/decisions-decided-202609.md)**（20261003 一字不改搬移；舊引用「decisions-decided.md:13」～「:1875」直接看封存檔同一行）。

### 20261001 09:4x Steven 裁決：Q62、Q63（從 decisions-pending.md 搬來）

### Q62. Setup.Contact 存檔的兩個 bug 修好了（todo D-021 Head Device Mode 捲軸、D-023 Kit Diameter），要不要先上機看過再合進 main
> 一題兩件：都是 Setup.Contact 在網頁上存檔會存錯／存不進去，修好後存檔的內容會變（接觸氣壓相關），所以一起問。回一個答案就兩件都照辦；要分開回也可以（例：「D-021 A、D-023 B」）。

**（一）D-021 Head Device Mode 捲軸**
**建議：B，先上機看一次再合**（會改接觸氣壓；修好前的壞法是「改了不生效」，是比較安全的那一邊）。
**操作員會看到什麼**：今天在網頁 Setup.Contact 拉 Head Device Mode 捲軸、按存檔，**不會生效**：存下去的還是舊值，重開頁面捲軸也不會顯示機台上的值（bug，ST01-E 20261001 找到）。修好後，存檔會真的改 Head Device Mode，並照 golden 重算接觸氣壓（dPress）和扭力（Torque）存進配方；生產時 dPress 就是 EP 氣壓（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\atester.cpp`:9662 ADAM_WriteVoltage），所以**存檔後下一次壓測就用新的壓力**。
**為什麼（golden）**：golden 捲軸的 OnChange＝scrbSLKChange（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp`:1185-1186；cContact.dfm:16134），會重算顆數、每顆力、氣壓。
**移植樹現在**：C++ 存檔前的推導早就寫好（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp`:141 `DF_scrbSLKChange`），但網頁從來沒把捲軸位置送上去——`D:\HT9045\web\page\ht9045_contact_slk.js` 自己記位置，沒同步到引擎存檔時讀的欄位（`D:\HT9045\web\page\ht9045_wire_engine.js`:1250），所以這段 C++ 從來沒在網頁上跑過。
**改法**（ST01-E，review6 `8d04f578`，只改 `D:\HT9045\web\page\ht9045_contact_slk.js` §19）：捲軸位置跟引擎的欄位雙向同步——存檔讀到現在的位置、開頁顯示 C++ 的位置。沒改引擎、C++、串流。ctest D021_ContactScrollSync 32／32（修之前的版本 24 項失敗）。按下去馬上送 EP 電壓那一段移植版本來就沒做（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp`:319-320），壓力只在存檔後改。
**選項**：
- A：兩組態 gate 綠了就合（照 golden）。例：Jimmy 合進 main 之後，操作員改 Head Device Mode、存檔，機台下一次壓測就用新算的氣壓。
- B（建議）：先在機台上改一次、存檔，比對配方裡的 dPress／Torque（和 EP 輸出）跟 golden 同一份配方算的一樣，再合。例：Steven 或 Jimmy 在機台上換一個 Head Device Mode、存檔，同一份配方在 golden 上做同樣的操作，兩邊數字一樣才合；在那之前 main 維持「改了不生效」。
- C：先不合、也不上機，等 CT-3（Contact 頁的機台操作：啟動、One Cycle、Auto Z Teach）一起做。
**為什麼要問**：會改接觸氣壓（IO 輸出），照 Q59 屬於例外，要 Steven 先看。
**（二）D-023 Kit Diameter 選項**（review6 `8caaf1e4`，只改 `D:\HT9045\web\page\ht9045_contact_slk.js`）
**操作員會看到什麼**：今天網頁 Setup.Contact 的 Kit Diameter 選項會把**隱藏的尺寸也列出來**。這台 `D:\HT9045\system\ContactInfo.ini` [SLK Type] Visible=1,1,1,0,1（第 4 個 56 那一格是隱藏的），所以網頁列 5 格、C++ 只有 4 格；用 80mm 的配方時網頁選第 5 格（索引 4），C++ 對不上 ⇒ **網頁存檔會被拒，或（程式上可能的路）SaveSetupFile 把 Kit Diameter 寫成 0.0**（bug，ST01-E 20261001 找到）。修好後照 golden 只列看得到的尺寸，選哪一格跟 C++ 一致，存檔會寫對 Kit Diameter，最小力量／EP 相關數字也跟著照 golden 算。
**為什麼（golden）**：golden FormShow 只把 bShow 的尺寸加進選項（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp`:1109-1145），DoIniDataToForm 再照檔案設索引（:851-919；對不到時設 0，:915-917）。
**選項**：跟（一）一樣 A／B／C。**建議 B**：在機台上用 80mm 的配方按一次存檔，確認配方 [Mode] Kit Diameter 還是 8.0000、KitDiameterMode 還是 3，再合。
**另外（不用 Steven 決定）**：同一個 commit 裡的 D-022（Setup.Contact 視窗關著時開機不再向 C++ 讀資料，照 golden 只在打開時 FormShow，cContact.cpp:1094）只影響讀取時機，但因為跟 D-023 在同一個 commit，會跟著這題一起合。

**gate**：`8caaf1e4`（含 D-021、D-022、D-023 和兩次 merge main）排在 St02 MR !14／!15 之後跑；FROM_STEVEN §2 的 `7cbb478d` 那列只寫到 `f2df9e4f`，`8d04f578` 以後等 Steven 回了再另外寫一列。待人審 A16～A18／B21～B23。
**Steven 的裁決**（20261001 09:4x，在 ST01-M 對話裡）：原話「通知jimmy讓eastsun上機驗證」⇒ **B**：先上機驗證再合。由 Jimmy 安排 EastSun 在機台上驗（D-021：換一個 Head Device Mode 存檔，dPress／Torque 跟 golden 同一份配方一樣；D-023：80mm 配方存檔後 Kit Diameter＝8.0000、KitDiameterMode＝3）；驗過才在 §2 請 Jimmy 合。

### Q63. 主畫面 Config ▾ 按下去時照 golden 重設工作參數、更新操作模式（todo D-025），要不要先上機看過再合進 main
**建議：B，先上機按一次再合**（會動 IO 輸出和加熱繼電器）。
**操作員會看到什麼**：今天網頁主畫面按 **Config ▾**，只在瀏覽器裡打開選單，C++ 什麼都不做，運轉中也打得開。照 golden 接上之後，每按一次 Config ▾，C++ 會先做 golden 那兩件事、再打開選單：
- **SetWorkParameter**（重設工作參數）：寫 IO 輸出——吸嘴模式 Z1／Z2、Sw10Bit／Sw10Bit2（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cinitial.cpp`:19039-19057、:8766-8772），並重寫 `D:\HT9045\system\teach.ini`；
- **UpdateMainOperateMode**（更新操作模式）：切換加熱繼電器 SW[SwHeaterRelay]，可能送 ATC 7.0 溫度設定（ATC_SET_TEMP／ATC_RUN），並寫 `D:\HT9045\system\lastdata.dat`。
另外：運轉中按 Config ▾／Tools ▾ 會被拒（golden 一樣）；再按一次不會把選單關掉（golden 沒有「再按關閉」）；權限不夠時不跳 WAR1676 框（golden bAlarm=false）；SoftStart 中允許（golden 一樣）。**Tools ▾ 只多送一個 SECS 事件（EnterTool），不動 IO**；移植版的 SECS 事件目前只是計數，不會真的送到主機。
**為什麼（golden）**：golden sbConfigClick／sbSettingClick（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp`:29009-29049）按下去就跑這些（:29027／:29048 送 EnterConfig／EnterTool）。
**改法**（ST01-E，review6 `fc1e4d3a`）：St01 的 `D:\HT9045\web\page\ht9045_main_st01_ev.js` 攔下兩顆鈕，送 act.main.menuOpen；C++ 新檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Main_D025MenuOpen.cpp` 照 golden 查閘、跑那兩件事，回「可以開／不行」，可以才打開選單。main.html 沒改。呼叫的都是已經移植好的函式，照原樣呼叫，沒有拿掉任何閘。ctest D025_MenuOpen（C++）＋D025_MenuOpenPage（node，23 項）；修之前的版本會失敗；D015_A01MenuPage 照舊通過。
**選項**：
- A：兩組態 gate 綠了就合（照 golden）。例：Jimmy 合進 main 後，操作員每按一次 Config ▾，吸嘴模式 IO、加熱繼電器就照 golden 重設一次。
- B（建議）：先在機台上按一次 Config ▾，確認 IO（吸嘴模式、Sw10Bit）、加熱繼電器、ATC 溫度沒有異常，teach.ini 改寫的內容跟 golden 一樣，再合。例：Steven 或 Jimmy 在機台上停機時按 Config ▾，看 IO 監看頁和溫控器；在那之前 main 維持「按了什麼都不做」。
- C：只接 Tools ▾（只送 SECS 事件）和運轉中拒開，Config ▾ 的 SetWorkParameter／UpdateMainOperateMode 先不接。例：運轉中打不開選單，但停機時按 Config ▾ 還是不會重設 IO。
**為什麼要問**：會動 IO 輸出和加熱繼電器、可能送溫度設定，照 Q59 屬於例外，要 Steven 先看。
**同一串還有**：TA-5（review6 `d6f75103`，Setup.TrayAssignment 圖形模式的兩個捲軸，golden cTrayAssignment.cpp:1639／:1674）——圖形模式（bTrayAssignUseGraphic）下存檔會照 golden 依捲軸位置寫 Tray.Data 的 RGAuto1-3／RGLoader；不碰加熱器／馬達／IO，照 Q59 不用問，列在待人審 B24。但它跟 Q62、Q63 疊在同一條分支上，要等 Q62、Q63 回了才能一起合。
**Steven 的裁決**（20261001 09:4x，在 ST01-M 對話裡）：原話「同Q62」⇒ **B**：同 Q62，由 Jimmy 安排 EastSun 上機驗證（停機時按一次 Config ▾：吸嘴模式／Sw10Bit IO、加熱繼電器、ATC 溫度沒有異常，teach.ini 改寫的內容跟 golden 一樣），驗過才合。

### 20261001 09:4x Steven 裁決：Q64（從 decisions-pending.md 搬來）

### Q64. 警報框要輸入密碼才能清的，網頁照 golden 接上（todo D-026），有三點要你點頭
**建議：B，照 golden 做，Jimmy 審過程式、EastSun 上機試一次再合**（安全層，而且動到 Jimmy 的警報框核心）。
**操作員會看到什麼**：golden 有些警報要輸入等級密碼或解鎖密碼才能按 Start／Pause 清掉（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\note.cpp`:5277 DoPassword、:5431 DoUnlockPassword）。今天網頁遇到這種警報**清不掉**：伺服器回「dialog auth not wired」。接上之後，網頁警報框會跳密碼輸入，驗證過才能清，跟 golden 一樣。
**做法**（ST01-E，側分支 `v906/st01-d026`，還在寫）：重用 B5 的重新登入（W906_Reauth），照 Q45「網頁不送明碼」的設計；輸出裡不會有密碼。要動 Jimmy 警報框核心的 7 處（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp`、`wb_dialog_mailbox.h`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.cpp`，都在同一行改、行數不變），已在 FROM_STEVEN §1 認領、§3 請 Jimmy 審。
**要你點頭的三點**：
- **（一）動 Jimmy 的警報框核心**：規則上沒人認領的行可以直接改，但警報流程是 Jimmy 設計的，所以合之前要 Jimmy 看過程式（或 Jimmy 說伺服器那一半他自己做）。
- **（二）安全政策**：「輸入警報密碼」這個指令**不用操作員權杖**就能送（`WebBridgeServer.cpp`:1448），跟今天「回答警報框」（dialog.response）一樣。理由：golden 是站在機台前的人都能回答警報，真正的檢查是密碼本身。風險：任何打得開網頁的人都能試密碼。**背景（St02 1001 07:17 提醒）**：這份免權杖清單是照你 20260927 的 **S124＝B「唯讀指令免權杖」**做的（St02 維護）；不是唯讀的例外目前只有 modal.answer、dialog.response（讓機台前的人都能回警報框），dialog.auth 會是第三個同類例外。St02 不反對，以你的答案為準。
- **（三）機台面板上看得到的改變**：要密碼的警報開著時，**面板的 START／PAUSE 鈕會被拒**，要先在網頁輸入密碼（golden Start()／BtnPauseClick 就是這樣）；拒絕時不動任何軸。
**選項**：
- A：三點都照 golden，gate 綠＋Jimmy 審過就合。例：Jimmy 合進 main 後，要密碼的警報在網頁上輸入密碼就能清。
- B（建議）：同 A，另外先由 **EastSun 上機**試一次（照你 1001 09:4x「需要上機驗證的, 都是請Eastsun處理」，經 Jimmy 安排；開一個要密碼的警報：密碼錯拒絕、密碼對可清；面板 START／PAUSE 在輸入前被拒）再合。
- C：（二）不放行——輸入警報密碼也要操作員權杖（比 golden 嚴），其他照 B。例：沒登入網頁的人看得到警報，但不能在網頁上輸密碼清。
**為什麼要問**：密碼／解鎖是安全層（同 Q61），又動到權杖政策，照 Q59 屬於例外。
**Steven 的裁決**（20261001 09:4x，在 ST01-M 對話裡）：原話「（一）動 Jimmy 的警報框核心 : 通知jimmy後, 可以開工」「（二）安全政策 : 按照golden沒問題」「（三）機台面板上看得到的改變 :  這個密碼暫時不用太嚴謹沒關係, 任何知道密碼的人就是可以操作並排除密碼的人」⇒ **（一）**已在 FROM_STEVEN §3（1001 07:0x）通知 Jimmy，St01 可以開工（合併前仍請 Jimmy 看 diff）；**（二）**dialog.auth 照 golden 免操作員權杖（S124＝B 的第三個例外）；**（三）**照 golden：知道密碼的人就能操作、排除，面板 START／PAUSE 在輸入密碼前拒照 golden；**不另外加嚴**（不綁登入身分、不鎖次數、不加冷卻）。上機由 EastSun 試（Q62 規則）。

## St02（Steven02，測試通訊）

### Steven 已裁決的項目（W 系列）

以下每一條都用「這是什麼功能」＋「當時的問題」＋「選項」＋「St02建議」＋「Steven 的裁決」的順序整理，方便理解；原本的內部代號（如 P6-Q1、D-1、ELA-8 等）保留在標題括號內，只作對照，不作為說明本身。所有檔案路徑一律使用完整路徑；找不到明確的樹別時會標註（樹待確認）。

> **W1～W62（2026-09-30 以前裁決）已搬到 [archive/decisions-decided-202609.md](archive/decisions-decided-202609.md)**（20261003 一字不改搬移；舊檔第 1940～2948 行＝封存檔第 1882～2890 行，舊行號減 58）。

### 20261001 08:0x Steven 對 St02-E 的三句裁決（St02-E session 當面；St02-M 1001 08:15 請 ST01-M 代登記）
- **TTL**：原話「TTL 介面是跟RS232整合再一起的」⇒ TTL 屬於 RS232 引擎（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Rs232`），log 在 `D:\RS232Log\BinLog_TTL`；不另開 TTL 模組。
- **ISA 卡**：原話「ISA卡片版本先不移植」⇒ ISA／舊卡的程式維持關閉（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Gpib\GpibCore.cpp`:1431／:1452、PCI_L112C／L122C／CMnet DLL 那幾段；golden 905 也在 `/* */` 裡）。
- **pull**：原話「必要的時候要pull」⇒ 工作分支落後 origin/main 時，必要就同步（快轉或合 main）；St02-E 已把 St02 那台的 `D:\HT9045` 工作分支快轉到 origin/main（純快轉、沒有覆蓋）。St01 這邊照同一原則：review6／`v906/st01-q59` 由 ST01-E 在送合併前合 main（共用目錄有別人沒 commit 的檔，不手動 pull）。

### 20261001 13:4x Steven 裁決：W63（從 decisions-pending.md 搬來）

### W63. 模擬組態遮網路選項（W58 第一階段）：SECS GEM 那兩個鍵要不要也遮？你跟 Jimmy 的裁決不一樣（St02）
（St02-M 在 FROM_STEVEN §4 20261001 09:18 請 ST01-M 轉；**有預設**）
**背景**：W58＝模擬組態（SIM）開機時，把會連到外面的網路選項先當成沒勾，免得測試機去連真的主機／網路磁碟。你 20260929 08:1x 裁 W58 時，「確定的網路連線」表裡**有 [N07-1] Enable SECS GEM**（`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`:2779-2812）；Jimmy 20261001 08:4x 裁**保留 [N07-1] Enable SECS GEM 與 [N07-2] 主機啟動不遮、其他照遮**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20261001.md` 第 3 條），理由是筆電要在模擬組態測 SECS／主機啟動。**出貨組態兩種都不變。**
**選項**：
- **A（預設，照 Jimmy）**：模擬版一開，SECS GEM 照 config.ini 原值（勾著就連主機），FTP／網路磁碟等其他 58 個照遮。
- B（照你 0929 原裁決）：SECS GEM 也先當沒勾，要手動勾＋存才連。回法：「SECS 也遮」。
**目前狀態**：St02 本機先照 A 改好、等你回再推；你沒意見就照 A。
**Steven 的裁決**（20261001 13:4x，在 ST01-M 對話裡）：原話「W63. 模擬組態遮網路選項  A」⇒ **A，照 Jimmy**：模擬組態保留 [N07-1] Enable SECS GEM 與 [N07-2] 主機啟動不遮，其他 58 個照遮；出貨組態不變。St02 照 A 推。

### 20261001 13:0x～13:4x Steven 對 St02 testercomm 頁的裁決（St02-M session；St02-M 12:54／13:06／13:08 請 ST01-M 代登記）
- 原話「我以為site01~32的面板是共用的!」「然後通訊Log的memo也是共用的」「請協助調整畫面」。
- 原話（13:3x，附 32 格 Site 面板截圖）「截圖的元件只有一份, 在首頁」「然後主要通訊log也是只有一份, 在首頁」「其他的設定值可以根據不同的測試模式進行分頁」。
- 原話（13:4x）「C++端在 32site 與 通訊log部分, 也是整合一個JSON進行發送」。
- ⇒ `D:\HT9045\web\page\testercomm.html` 第一個分頁「首頁」放唯一一份 Site01～32 元件＋唯一一份主要通訊 Log（跟著目前的測試介面）；其他分頁依測試模式（GPIB／RS232·TTL／TCP/IP）放各自的設定值；C++ 端把 32 site 與主要通訊 Log 合成一個 JSON 送出。St02-E 做（St02 的檔），推 `v906/st02-tc-shared-panel`＋MR。

## 已裁決、不再問（早期整理；依據）

> 這一節（W2、W31～W34、舊 Q18／Q20／Q33，以及 S57／S62／S63／S65／S67／S70／S86／S88／S105／S107 等早期「已裁決、不再問」的清單）20261003 一字不改搬到 [archive/decisions-decided-202609.md](archive/decisions-decided-202609.md)（舊檔第 2971～3006 行＝封存檔第 2891～2926 行）。

## 20261002 起（依時間追加，St01／St02 混排；新裁決一律加在檔尾）

### 20261002 07:4x Steven 對 St02-M 的裁決：St02 這台可以跑測試（St02-M 1002 07:43 請 ST01-M 代登記）
- **原話**：「你如果能跑得起來的話, 可以做測試」
- **取代**：20260927 的規則「St02 這台只編譯、執行驗證一律請 St01 代跑」（也取代 St02-M 1002 06:29 那則「Steven 說可以之前照舊請 St01 代跑」）。
- **St02 的做法**（St02-M 07:43）：①node 類 ctest（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\*.cjs`）推之前在 St02 本機真跑（含對照組），§2 寫通過數；②新編出來的 exe（C++ ctest）可以試，被 F-Secure 擋或隔離就停、**不碰防毒設定**，那幾個照舊請 St01 代跑，§2 寫明哪些在 St02 跑過、哪些要 St01 跑；③推之前兩組態建置照舊。
- **St01 這邊的影響**：St02 的 MR 只有 §2 寫明「要 St01 跑」的才排進 St01 代跑佇列；筆電收進批次的照舊不跑。

### 20261002 08:0x Steven 裁決：Q65／Q66／Q67／W64／W65（從 decisions-pending.md 搬來）

### Q65. 機台運轉中，要不要讓操作員在 Contact 頁按 OTD 面板動對接氣缸？（B8 CT-3d，St01）
（ST01-E 1001 23:4x 提出；ST01-M 登記）
**機台上會發生什麼**：有 OTD 對接機構的機台（USE_OTD==1），Contact 頁有兩個 OTD 面板（240KG、360KG），按一下會讓 4 支對接氣缸伸出或縮回（測試頭跟機台對接／分開）。golden 不管機台有沒有在跑都照做——處理器裡沒有任何檢查，而且 Contact 視窗是非模態的，運轉中也開得著（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp`:15395-15443 palOTD_4Click／palOTD_6Click）。移植樹今天（review6 `d6cda69d`）運轉中按這兩個面板會被拒（網頁 form.event 的運轉中規則）。開發機與 HT9050 是否有 OTD 硬體還不確定（USE_OTD=2 看不到這兩個面板）。
**為什麼要問**：Steven Q61 說運轉中例外「依照golden」，照那條應該放行；但這是**運轉中動氣缸**，屬於 Q59 說的「IO 安全要 Steven 先看」，所以 ST01-M 不自己套 Q61。
**選項**：
- **A（ST01-M 建議）**：維持拒絕（比 golden 嚴）。例：機台在跑時操作員按 240KG 面板，畫面回「運轉中不能操作」，氣缸不動；要對接／分開先停機。之後 EastSun 在有 OTD 的機台上看過、確定運轉中真的需要，再改 B。
- B：照 golden 放行，運轉中例外清單（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.cpp`）加 palOTD_4、palOTD_6 兩列（規則例外，human-review C 區）。例：機台在跑時操作員按 240KG 面板，對接氣缸立刻動作，跟 BCB6 一樣。
**在哪看**：review6 `d6cda69d` commit 本文；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp`:872-988；待人審 B42。
**目前狀態**：等 Steven 回。回法：「Q65 A」或「Q65 B」。沒回之前照 A（今天的程式就是 A，不用改）。
**Steven 的裁決**：原話「B」（Steven 1002 08:0x）⇒ Contact 頁 OTD 面板運轉中照 golden 放行；運轉中例外清單加 palOTD_4／palOTD_6 兩列（規則例外 C），拿掉移植樹的拒絕；EastSun 在 USE_OTD==1 機台驗。ST01-E 在 review6 跟進。

### Q66. 保養紀錄的檔名要不要過濾操作員打的字？（E-021，St01）
（ST01-E 1002 04:0x 提出；ST01-M 登記）
**畫面上會發生什麼**：Data.Observer 的 Precautions Record 分頁（[B01] 開著的機台）與 Handler Major Maintenance 分頁（[B02]），按存檔時 golden 用**操作員自己打的文字**組檔名：`D:\PrecautionRecord\<機台 ID>_<時間>_<注意事項內容>.txt`、`D:\MajorMaintenanceRecord\<機台 ID>_<時間>_<備註第一行>.txt`，客戶設定的分享路徑（asB01／asB02）也照同一個名字再存一份。golden 沒有過濾（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cObserver.cpp`:4309-4383 SavePrecautionMemoInformation、:4542-4620 SaveMajorMaintenanceInformation）——注意事項如果打成 `..\..\HT9045\system\X`，就會在 `D:\HT9045\system\` 底下建立或**蓋掉**任何這個程式寫得到的 .txt（存檔會先清空舊檔）。BCB6 本來就有這個洞；移植版的文字是從瀏覽器送進來的（只聽 127.0.0.1、要有操作權杖）。
**選項**：
- A：照 golden，文字組出什麼路徑就寫什麼路徑。例：注意事項打「..\..\HT9045\system\X」，存檔後 `D:\HT9045\system\` 底下多一個（或被蓋掉）`X.txt`。
- **B（St01 建議）**：移植版自己加一道：文字裡有 `\ / : * ? " < > |` 或 `..` 時拒絕存檔並提示，一般註記（中文、英文、數字、空白、- _ 等）完全不受影響。例：打「換吸嘴 -- 0.5mm」照常存；打「..\x」畫面回「檔名不能含路徑字元」，不存。
**在哪看**：E-021（ST01-E 在 `v906/st01-q59` 做，commit 本文會附出處）；待人審 C12。
**目前狀態**：等 Steven 回。回法：「Q66 A」或「Q66 B」。沒回之前照 B（一行開關，選 A 就切回 golden）。
**Steven 的裁決**：原話「B」（Steven 1002 08:0x）⇒ 保養紀錄檔名有 `\ / : * ? " < > |` 或 `..` 就拒絕存檔，一般註記不受影響；E-021 的暫行做法即定案，註解改寫。

### Q67. 機台運轉中，要不要讓操作員在 Status.ShowBinSelect 的 Index 分頁按 Auto Clean？（E-023 SB-1，St01）
（ST01-E 1002 06:0x 提出；ST01-M 登記）
**機台上會發生什麼**：Status.ShowBinSelect 視窗是非模態的，運轉中也開得著；golden 允許運轉中按 Index 分頁的 Auto Clean（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp`:9190-9195）。按下去會看當時狀態走兩條路之一：
- 機台裡有料：預約一次 Auto Clean／One Cycle——只設旗標，不直接動馬達；但 golden 的 InitOneCycle（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\csystem.cpp`:391）會**在運轉中把 iOneCycle／iCleanOut／iHome／iReset／iTrayFeed 這些流程進度歸零**。
- 機台空的：golden 會先檢查四件事（要先 Home、Tray 手臂不在安全位置、Index Z 不在安全位置、Index 不在待命位置，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cShowBinSelect.cpp`:2275／:2281／:2287／:2295／:2303），不符合就跳拒絕框；golden 的拒絕框一跳出來就會 **SystemStart＝false、SoftStart＝false、StopAllMotor**（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\mymessbox.cpp`:303-308）——運轉中 Tray 手臂正在動時就可能碰到，等於**按一下就停掉所有馬達**。
移植樹（E-023，ST01-E 在 review6 做）目前運轉中按這顆會被拒（SystemStart 或 SoftStart 時），停機時按照 golden。
**為什麼要問**：照 Q61「依照golden」應該放行；但運轉中會直接停所有馬達、也會把流程進度歸零，屬於 Q59 說的「馬達／IO 安全要 Steven 先看」，跟 Q65（運轉中動 OTD 氣缸）同一類，所以 ST01-M 不自己套 Q61。
**選項**：
- **A（St01 建議）**：維持拒絕（比 golden 嚴）。例：機台在跑時按 Index 分頁 Auto Clean，畫面回「運轉中不能操作」，什麼都不會發生；要清料先停機再按。
- B：照 golden 放行。例：機台在跑時按下去，有料就預約 Auto Clean 並把其他流程進度歸零；空機而手臂不在安全位置就跳框並停掉所有馬達——跟 BCB6 一樣。
**在哪看**：E-023 commit 本文（推上 review6 後）；待人審會列 A＋B。另外同頁的 Copy Recipe 照 golden 運轉中也能按（golden 沒有閘，只列 B 告知，不問）。
**目前狀態**：等 Steven 回。回法：「Q67 A」或「Q67 B」。沒回之前照 A。
**Steven 的裁決**：原話「可以按, 按了之後機台會執行one cycle, 然後才是auto clean」（Steven 1002 08:0x）⇒ B：Status.ShowBinSelect Index 分頁的 Auto Clean 運轉中照 golden 放行，預期行為＝先跑 One Cycle 再 Auto Clean；golden 空機時的拒絕框（含 StopAllMotor）照 golden 保留。ST01-E 在 review6 跟進，EastSun 驗。

### W64. HANA RMS（客戶 HANA Micron 的配方伺服器連線）還要照 912 補進移植樹嗎？（H-013 第 3 項，St02）
（St02-M 在 FROM_STEVEN §4 20261001 19:56 請 ST01-M 轉）
**背景**：0926 你裁 11＝B「測試通訊這邊一起補」：照 912 補 S10F3 警報視窗、安全 PLC、HANA RMS。今天 14:2x Jimmy 的 RULINGS_20261001 第 26 條說「912是9046量產機版本，和我們進行的HT9050沒有關係，9050目前是用906 C++版本」。前兩項（7 行、值不變）St02 已照做、Jimmy 20:0x 同意；**第 3 項 HANA RMS 要不要補**變成問題。
**份量與影響**：912 `Automation/automation.cpp`:2514-3158 約 650 行翻成 St02 新檔＋筆電 3 行，拿掉 St02 HandlerBridgeCtl.cpp 的 G9；只有 `CUSTOMER_CODE==CC_HANA_MICRON` 且 A77 打勾的機台會不一樣；912.0 這版本身有現場缺陷（開機只連一次、伺服器 30 秒閒置就斷）。要補的話還會多一題新設計（HANA RMS 設定／紀錄的網頁畫面，網頁上沒有可以掛的頁）。
**選項**：
- A：照 912 補（650 行＋之後問網頁畫面）。例：HANA Micron 機台勾 A77 後，開批會跟 HANA 的 RMS 伺服器比對配方。
- **B（ST01-M 建議）**：照第 26 條，9050 用 906 golden——G9 改回 906 的文字（906 沒有 HANA 區塊），912 的 HANA RMS 留給之後真的要給 HANA 機台時再做。例：移植樹不連 HANA RMS，跟今天一樣。
**在哪看**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20261001.md`:161-166、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\TESTERCOMM_PORT_LEDGER.md`:182／:194-195、交接分支 `docs/handoff/ST02_H013_PLAN_20261001.md` §5。
**目前狀態**：等 Steven 回。回法：「W64 A」或「W64 B」。
**Steven 的裁決**：原話「A 912版本是比較新的, 另外Hana目前由RogerYang維護, 以RogerYang的註解為準」（Steven 1002 08:0x）⇒ 照 912 補 HANA RMS（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\automation.cpp`:2514-3158 → St02 自己的檔＋筆電 3 行，拿掉 St02 的 G9）；912 程式裡有 RogerYang 註解的地方以註解為準。是 RULINGS_20261001 第 26 條「912 跟 HT9050 無關」對 HANA 這一塊的例外（已告訴筆電）。St02 做（H-013 第二顆）。

### W65. TesterIF 的 RS232 多出來的選項（Bit Length 5／6、Stop Bit 1.5、Parity Mark／Space）要正式保留嗎？（TesterIF 5B，St02）
（St02-M 在 FROM_STEVEN §4 20261002 02:21 請 ST01-M 轉）
**畫面上會發生什麼**：Setup 的 TesterIF 頁，RS232 設定裡的 Bit Length、Stop Bit、Parity 三組選項，現在比 BCB6 多幾個：Bit Length 多 5 Bits、6 Bits；Stop Bit 多 1.5 Bits；Parity 多 Mark、Space。多出來的都接在 golden 原本選項的後面，所以配方裡存的索引（0、1、2…）原本代表什麼還是一樣。這是 0926 Tester 通訊計畫裁決 5＝B 加的，但程式註記還寫「暫照建議，待使用者確認 decision #5」。
**選項**：
- **A（St02 建議）**：確認保留（就是現狀）。例：客戶的測試機要 7E1.5 或 Mark 同位，操作員可以直接選；舊配方打開，選項不變。RS232Standard 那邊的對應已做好。
- B：照 golden 拿掉這幾個多出來的選項。例：選項跟 BCB6 一模一樣，只有 golden 原本的那幾個。
**在哪看**（main `19e8826c`）：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_TesterIF.cpp`:258-265（附加選項）、`D:\HT9045\web\page\Setup.TesterIF.html`:56（rgBitLength／rgStopBit／rgParity）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Rs232SetupCodes.h`（Setup.ini 對應）；盤點 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\TESTERIF_FIELD_AUDIT_20261002.md` §6 P7（St02 MR !92，合進 main 前在 `v906/st02-tif-audit`）。
**目前狀態**：等 Steven 回。回法：「W65 A」或「W65 B」。沒回之前照 A（現狀）。
**Steven 的裁決**：原話「A」（Steven 1002 08:0x）⇒ TesterIF 多出來的 RS232 選項（Bit Length 5／6、Stop Bit 1.5、Parity Mark／Space，接在 golden 選項後面）正式保留；程式裡「待使用者確認 decision #5」的註記改寫（St01 檔由 ST01-E、St02 檔由 St02）。

### 20261002 12:2x Steven 裁決：W66（從 decisions-pending.md 搬來）

### W66. HANA RMS 的設定與連線紀錄要不要做網頁畫面？（C10，St02）
（St02-M 在 CHAT_ST02 20261002 10:30 請 ST01-M 轉；W64＝A 之後的新設計題）
**畫面上會發生什麼**：golden 912 在 Configuration 頁 A77 旁邊有一顆鈕，打開 HANA RMS 分頁（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\automation.dfm`:468-723 tsHANARMSInterlock：IP、Port、HDNAME、連線紀錄）；移植樹的 `D:\HT9045\web\page\Config.Configuration.html`:70 有 cbA77 勾選框、沒有那顆鈕。
**選項**：
- A：照 912 版面做新頁面。例：操作員在網頁上改 HANA 的 IP 就生效、看得到連線紀錄。
- **B（St02 建議）**：先不做——只有 HANA Micron 且勾 A77 的機台會用到；設定直接改 `HANARMS.ini`、紀錄看 `D:\HT9045_Log\HANARMS\`，等真的有 HANA 機台要上線再做。
**目前狀態**：等 Steven 回。回法：「W66 A」或「W66 B」。沒回之前照 B。
**Steven 的裁決**：原話「W66. HANA RMS 的設定與連線紀錄要不要做網頁畫面？ B（St02 建議）：先不做」（1002 12:2x）⇒ HANA RMS 不做網頁設定／紀錄頁；設定改 `HANARMS.ini`、紀錄看 `D:\HT9045_Log\HANARMS\`，等真的有 HANA 機台要上線再做。St02 的 C10（照 912 補 HANA RMS 本體）照常進行。

### 20261002 12:2x Steven 裁決：Q68／Q69（從 decisions-pending.md 搬來）

### Q68. HW.HandlerSys 頁的開頁閘要不要綁 golden 的隱藏手勢＋密碼？（E-023 TP-2，St01）
（ST01-E 1002 07:5x 提出；ST01-M 登記）
**畫面上會發生什麼**：golden 只能經由 Status.TemperFrom 頁的隱藏手勢進 Handler System：連點三顆溫度燈號（Panel71～73MouseDown，`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cTemperFrom.cpp`:1692-1769）→ 輸入密碼 → 才開 TfHandlerSystem。E-023（review6 `f8da6505`）已照 golden 做好這個手勢＋密碼（密碼只在 C++ 比對，網頁、回覆、log、測試輸出都看不到）。但網頁版的 HW.HandlerSys 頁另外有自己的開頁閘（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`:899 GHandlerSys），**只查等級＋運轉中**，沒有綁這個手勢＋密碼——從別的地方開這頁（例如 background.html 裡 debugOnly 的 handlersys 視窗）就繞過了手勢與密碼。
**選項**：
- A：維持現狀（等級閘）。例：等級夠的人從選單直接開 HW.HandlerSys，不用手勢也不用密碼。
- B：要先用 TP-2 手勢＋密碼拿到一次性的解鎖權杖，HW.HandlerSys 才開得了（照 golden 唯一入口）。例：沒做手勢就開這頁，畫面回「請先從 TemperFrom 解鎖」。會動到 HSys 相關測試（如 OpenEnterLog）與筆電的 `D:\HT9045\web\page\background.html`。
**在哪看**：review6 `f8da6505` commit 本文；待人審 C15。
**目前狀態**：等 Steven 回。回法：「Q68 A」或「Q68 B」。沒回之前照 A（現狀）。
**Steven 的裁決**：原話「Q68. HW.HandlerSys 頁的開頁閘要不要綁 golden 的隱藏手勢＋密碼？ 先按照現況, 以後再說」⇒ A：HW.HandlerSys 維持等級閘，不綁 TP-2 手勢＋密碼；以後再議。

### Q69. HT9050 的主畫面溫度要不要另外設計？（ST01-E2 溫度對照，St01）
（ST01-E2 1002 09:4x 依 Steven「溫度怎麼顯示在主頁上」整理時發現；ST01-M 登記）
**畫面上會發生什麼**：golden 的溫度畫面（Status.TemperFrom＋主畫面溫度格）是照 HT9045 系列排的：Index 4／16／32、DUT 2／4、TriTemp、ATC 各有版面，**沒有 HT9050 自己的版面**；HT9050 的 DTM 溫控走 HT9045 的 Index 順序（iTempCode），跟 HT9050 三站的實際配線對不上——照 golden 移植，HT9050 上看到的溫度格會跟實際站位不對應。對照表在 `D:\HT9045\.claude\skills\ht9045-temperature\references\main-screen-display.md`（MR !103，合進 main 前在 `v906/steven-st01e2-temp-display`），71 個通道各對到哪台溫控器的哪個 CH。
**選項**：
- A：維持照 golden（HT9045 的版面與順序），等之後真的要給 HT9050 客戶再設計。例：HT9050 開機看到 HT9045 的溫度格，站位對應要對照表才看得懂。
- B：現在就設計 HT9050 的溫度版面（三站、照實際配線排），屬新設計，要定版面再做。例：HT9050 的畫面直接顯示站 1／2／3 的實際溫度。
**目前狀態**：等 Steven 回。回法：「Q69 A」或「Q69 B」。沒回之前照 A。
**Steven 的裁決**：原話「HT9045跟HT9050都要新的設計, 你先給我個提案吧, 可以用你能顯示的方式顯示給我看」⇒ 不是 A 也不是單純 B：**HT9045 與 HT9050 的主畫面溫度都要新設計**；ST01-M 先出提案（畫面示意），Steven 看過再定。登記為 todo E-027。

### 20261002 16:3x Steven 裁決：Q71／Q72（Steven 直接回 ST01-E 的提問，沒有經過 decisions-pending.md）

### Q71. Heater 分頁要加 EJ1N、DTM 兩種溫控器，代碼存在哪？（E-029，St01）
（ST01-E 1002 16:3x 用 AskUserQuestion 問；ST01-M 登記）
**畫面上會發生什麼**：HW.HandlerSys 的 Heater 分頁每個通道選一種溫控器廠牌，存進 `D:\HT9045\system\Gerneral.ini` [TempCtrl] HeaterInsOpt_<通道>；golden 只有 5 種廠牌，EJ1N、DTM 選不到。
**Steven 的裁決**：「同一個鍵加新代碼」⇒ **EJ1N＝5、DTM＝6，存同一個 [TempCtrl] HeaterInsOpt_<通道> 鍵**。Steven 接受風險：BCB V912 不認得 5／6，golden 的廠牌分派也沒有 else（舊程式讀到會落空）。

### Q72. Index 區選 EJ1N／DTM 時，要不要自動改 [System] USE_16_HEATER？（E-029，St01）
（ST01-E 1002 16:3x 用 AskUserQuestion 問；ST01-M 登記）
**畫面上會發生什麼**：Steven「使用 EJ1N 或是 DTM 應該是要設定兩個變數」——Index 區的溫控器廠牌與 `D:\HT9045\system\Gerneral.ini` [System] USE_16_HEATER（Index 加熱數與型式）是兩個鍵。
**Steven 的裁決**：「自動連動」⇒ **Index 區 EJ1N／DTM ↔ [System] USE_16_HEATER 雙向自動連動**（ST01-E 轉述：USE_16_HEATER 的值 2／3、5／6，保留 16／32 的數量；對應細節見 E-029 的 commit），一次存檔寫兩個變數。

### 20261002 17:1x Steven 裁決：Q73～Q76（Steven 在 ST01-E2 的對話裡回，每題都選建議；ST01-E2 轉 ST01-M 登記）

### Q73. Heater 分頁「全機相同」與「各溫控器不同」時，EJ1N／DTM 出現在哪？（E-029，St01）
**Steven 原話**：「當選擇全機相同, 那就是 index 跟其他部位的分成兩種溫控器進行選擇 EJ1N 跟 DTM 在index站都是可以選的」「當選擇全機不同單獨設定, 那就是每個位置要可以單獨設定, 包含使用 EJ1N跟DTM」「index區裡面, Head 1234 跟 Ax Bx這32組屬於互斥的, 也就是同時間只會顯示其中的一種」「socket跟 Dut 1~4也是互斥的」
**Steven 的裁決**：**全機相同**＝Index 與其他部位分兩個下拉：EJ1N／DTM 只出現在 Index 的下拉，其他部位維持 golden 5 種廠牌；**各溫控器不同**＝每個通道都可選 7 種廠牌（含 EJ1N、DTM）。取代 Q34 的 Index 鎖（W906_HeaterInsIndexLocked）。跟 golden 不同（golden 沒有 EJ1N／DTM）→ 待人審 B。

### Q74. Index 區的 Head1～4 與 Ax／Bx 32 組怎麼互斥？（E-029，St01）
**Steven 的裁決**：照 rgHeater（Index Heater Counts）：4 個 → Head1～4；16 個 → Aa1～Bd2；32 個 → Aa1～Bh2；同時只顯示一種。**刻意偏離 golden**：golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\MachineTypeUtility.cpp`:241-293 在 16／32 時仍顯示 Head1～4 → 待人審 B。

### Q75. Socket 與 DUT1～4 怎麼互斥？（E-029，St01）
**Steven 的裁決**：照 rgUse4DUT（Dut Heater Count）：1 EA → Socket；2 EA → DUT1～2；4 EA → DUT1～4；同時只顯示一種。等於打開 golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\MachineTypeUtility.cpp`:268 被註解掉的 Socket 那行。

### Q76. 頁面上改 rgHeater／rgUse4DUT 時，顯示的通道要不要即時跟著變？（E-029，St01）
**Steven 的裁決**：要，即時變。golden 只在開機時算一次，這是新行為 → 待人審 B。

### 20261002 20:4x Steven 裁決：Q70（從 decisions-pending.md 搬來）

### Q70. EastSun 的機台建不出 review6，「EastSun 驗過才進 main」做不到——review6 要不要先進 main，讓 EastSun 在下一個安裝包上驗？（ST01-M 1002 13:1x 登記）

- **現在的作法**：Steven 1001 09:4x 規定上機驗證一律請 EastSun。St01 把要上機看的一般修改放在 review6 分支（`v906/steven-cbridge-review6`），EastSun 驗過才合進 main。Steven 要先看的加熱／馬達／IO 安全項目另放側分支（例如 `v906/st01-d026`），不在 review6。
- **卡在哪**：已交 EastSun 六批（be065d1e、05e3f181、6dd4a772、91e136d4、d6cda69d、39293c0f），第七批 8d8b1345 正在跑兩組態測試，**結果都還沒回**。筆電 1002 12:2x 回報（main 分支 `docs/handoff/TO_STEVEN.md` 第 455 列）：EastSun 那台機台**沒有 GitLab 權限，只收從 main 做的 GitHub 安裝包**，所以拿不到 review6。照現在的規則，這七批永遠驗不了。
- **EastSun 要驗的清單**：`D:\HT9045\.claude\skills\ht9050-construction\references\human-review.md` A 區（A35～A48），例如 A46「運轉中按 Contact 頁 OTD 面板，對接氣缸照 golden 動作」（Steven Q65＝B）、A47「運轉中按 Auto Clean，先 One Cycle 再 Auto Clean」（Steven Q67＝B）。
- **A（筆電提議，ST01-M 建議）**：review6 先進 main。St01 在 §2 寫可以合到第七批 8d8b1345（兩組態測試綠了才寫），筆電照常兩組態測試後合進 main；機台下一個安裝包就有，EastSun 在那上面照 A 區清單驗，有問題 St01 直接在 main 上修。例：EastSun 拿到第 126 包，逐項試 A35～A48，A46 氣缸動作跟 BCB6 不同 → 回報 → St01 修 → 下一包。代價：還沒上機驗過的修改先進 main。這些都是一般項目，Steven 先看的安全項目在側分支、不受影響。
- **B**：review6 不進 main，請 Jimmy 另外從 review6 打一個安裝包給 EastSun。例：Jimmy 打一包「review6-8d8b1345」，EastSun 驗完再合 main。代價：Jimmy 那邊要多一條打包流程，而且 review6 會一直跟 main 分開，越拖衝突越多（今天第三輪機台整合已造成 5 支檔衝突，ST01-E 正在解）。
- **C**：維持現狀，等 EastSun 自己有辦法拿 review6。例：七批繼續放著。代價：驗不了，review6 離 main 越來越遠。
- **登記時的狀態**：等 Steven 選。筆電說 Steven／St01 同意之前，review6 不進 main。

**Steven 的裁決**：原話「Q70：review6 要不要先進 main。def051af 的測試已經跑完，跟基準一樣。  做完就可以push」⇒ **A：review6 進 main**（ST01-M 這樣理解，Steven 可以更正）。
**接下來**：ST01-E 的一次合併（E-029＋ST01-E2 版面 `f533d227`＋溫度 skill `caecc89f`＋main `48f97f51`，本機 `fb31e431`）兩組態測試綠了就推 review6；ST01-M 的排隊 gate `D:\AI_TempFile\st01-chain-115-review6.ps1`（q59 `4410648f` → MR !115 協助合併 `cd013917` → review6 新頭）跑完全量兩組態 gate，綠了在 FROM_STEVEN §2 寫「review6 可以合進 main 到 <hash>（Steven Q70＝A）」；EastSun 在 main 做的下一個安裝包上照 `D:\HT9045\.claude\skills\ht9050-construction\references\human-review.md` A 區驗，有問題 St01 在 main 上修。

### 20261002 22:4x Jimmy 裁決：Q77、W67～W69（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20261002.md` 第 23 條，main `b84064b4`；Jimmy 一次回 NIGHT_REPORT §0 的 15 題；ST01-M 23:5x 從 decisions-pending.md 搬來）

Jimmy 原話：「#1A #2C #3A #4A #5照建議 #6照建議 #7B #8B #9A #10A #11C #12B #13A #14A #15一切用+-99999，拉最大，測試中先不要卡，我自行降速驗證功能」。這幾題原本也列給 Steven；Jimmy 先答了，照 Jimmy 的答案做，Steven 要改再說。

### Q77. Auto Clean 鈕「One Cycle 跑的時候不准再觸發」的保護，912 才有，要留還是照 #20 拿掉？（ST01-M 1002 21:0x 登記；St01 912 稽核 Q-A）

- **畫面上會發生什麼**：主畫面 Auto Clean 鈕（golden `btnAutoCleanClick`）。912 在開頭多一行「One Cycle 在跑就直接返回」（912 main.cpp :2254-2255，RogerYang 20260810）；906 :2101-2166 沒有這行。照 906 做的話，One Cycle 跑的時候按 Auto Clean，會再呼叫一次 One Cycle。Steven Q67＝B 讓運轉中也按得到 Auto Clean，所以這個情況真的會發生。
- **A（ST01-E 建議）**：留著 912 的保護，記成 #20 的例外。例：One Cycle 跑到一半按 Auto Clean → 沒反應，等 One Cycle 做完再按。
- **B**：照 #20 拿掉，跟 906 一樣。例：One Cycle 跑到一半按 Auto Clean → One Cycle 又被叫一次。
- 同一份稽核的 Q-B（START 鈕的 Teradyne-US 保護，912 才有、客戶專用）照 #20 拿掉，不另外問；Q-C（Contact 頁第 11 個模式 Visual Detection）已經在 main 上，併進 W68。稽核全文：handoff 分支 `docs/handoff/ST01_912_AUDIT_20261002.md`。
- **登記時的狀態**：等 Steven 選。

**裁決**（#23 第 5 項＝§0 67「照建議」）：**A：留著**，記成第 20 條的例外（安全保護）。同一項：**Q-B 拿掉**（Teradyne START 權限，照 906）；**Q-C 先留**（Contact 第 11 個模式 Visual Detection Test，併入第 62 項清理）。St01 照做：todo E-030。

### W67. #20「沒有例外」有沒有蓋掉 Steven 之前逐項裁決過的 912 項目？（St02-M 1002 20:30 請 ST01-M 統整；ST01-M 21:0x 登記）

- **哪三項**：0927 #35 P65 ARM-QA 重測回 iLotStatus=1（912）；W15＝B 事件記錄 CSV 拆欄（912）；R68-MYDB 警報代碼表（912，本來就在等 Jimmy）。
- **A**：蓋掉，三項都照 906 重做。例：ARM-QA 重測照 906 的流程，事件記錄 CSV 回到 906 的欄位。
- **B**：之前的逐項裁決照舊，算 #20 的例外。例：這三項維持 912 的做法，稽核表標「Steven 例外」。
- 背景：handoff 分支 `docs/handoff/ST02_912_AUDIT_20261002.md`（St02-M 1002 20:30，`ac48ae83`）。
- **登記時的狀態**：等 Steven 選。

**裁決**（沒有直接回答，從 #23 第 6 項推得）：第 6 項的例外只列第 20a／20b／20c 條和 Q-A ⇒ **A：這三項也改回 906**。St02-M 1002 23:37 也照這樣讀（P65、W15＝B 事件記錄拆欄、AOI with912 改回 906；R68-MYDB 原本是 NB2 的待決題，St02 排最後、動之前先問）。

### W68. main 上已經有的 912 內容，什麼時候清、由誰清？（St02-M 1002 20:30；ST01-M 21:0x 登記，併入 St01 稽核的 main 層發現）

- **有哪些**：St02 自查 main 上「912 才有、要照 906 重做」16 項，依影響排：!114 ADAM-6024（見 W69）、GPIB 遠端 START／STOP（含 H-012 !33）、H-013 IsSafePLCIOInstall（安全 PLC）、H-013 fSecsAlarm、Tester Pause 蜂鳴延遲、P65 ARM-QA、AMD 執行期分支……；St01 稽核另列 Contact 頁 Visual Detection 模式（rbVisualDetectionTest）、DF_SetContactMode 的 ASE-CL 灰掉、D-026 WAR04217 密碼規則、editlist 產生器讀 V912（.gen.inc 內容是 V912 的行）。筆電 NIGHT_REPORT #62＝A：動已經在 main 的東西要等 Steven。
- **A（ST01-M 建議）**：各組清自己的，照影響順序一張一張 MR（安全相關的 GPIB 遠端 START／STOP、IsSafePLCIOInstall 先），每張照 906 重做、兩組態 gate。例：St02 先送「GPIB 遠端 START／STOP 照 906」一張 MR，筆電照常合。
- **B**：全部交給筆電在 #62 一次清。例：筆電排一批「#20 清理」，St01／St02 只提供清單。
- **C**：先不清，只是之後不再加 912 的東西。
- **登記時的狀態**：等 Steven 選。

**裁決**（#23 第 6 項＝§0 62「照建議」）：**A：從現在起一律 906；已經在 main 的逐件對 906**。(a) 內容跟 906 不同的改回 906（第 20a／20b／20c 條和 Q-A 例外不動）；(b) 內容相同、只是引用寫 912 的改註解，排後面。各同事照自己的自查清單做（handoff 分支 `docs/handoff/ST01_912_AUDIT_20261002.md`／`ST02_912_AUDIT_20261002.md`）。St01 的部分：todo E-030。

### W69. !114 ADAM-6024 已經在 main，要留著補一張 906 修正，還是 revert？（St02-M 1002 20:30；ST01-M 21:0x 登記）

- **現況**：!114（`9e46491f`，26 支檔、+7442 行，912 adam6024.cpp 翻的）在 1002 18:51 用網頁合進 main，在 #20 說「抽出 batch 45」之後 41 分鐘。St02-E 已經照 906 重翻好（Steven02 本機 `c800f2fc`，還沒推）。同一時段網頁也合了 MR !120 `v912/nb2-app`（`fe17dfda`，18:51），跟 #20「912 不做」對不上，一併請 Steven 知道。
- **A（ST01-M 建議）**：留著，St02 送一張「ADAM-6024 照 906」的修正 MR（`c800f2fc`）蓋過去。例：main 上的 ADAM 頁短暫是 912 版，修正 MR 合了就變 906 版；別人的分支不用跟著處理 revert。
- **B**：先 revert !114，再合 906 版。例：main 先拿掉 26 支檔，之後再加回 906 版；revert 這麼大的 MR，其他開著的分支合 main 時會多一輪衝突。
- **登記時的狀態**：等 Steven 選。

**裁決**（#23 第 2 項＝§0 56）：**C：保留 912 版，記成第 20 條的例外（第 20c 條）**。理由：912 的 `MultiTransferKG` 小數修正是真的 bug 修正（EastSun 0710：906 的 int 會把 0.5 kPa 截成 0）。St02-E 照 906 重翻的 `c800f2fc` 作廢；總開關 `W906_ADAM_EP_LIVE` 仍關，要開時另外通知、EastSun 在旁。同一條：**第 20b 條 HANA 照 912**（St02 的 C10 HANA RMS 可以推，MR !126）；**第 20a 條溫控照 V912 不變**（Ifor 21:1x 不同意見，Jimmy 選 Steven 的）。

### 20261003 05:3x Steven 裁決：Q78、Q79（從 decisions-pending.md 搬來；兩題原本也在 FROM_STEVEN §3 問 Jimmy＝夜間報告 §0 第 74～76 項）

Steven 原話：「Q78 Q79, 可以按照912，但是註解同時提供906的行號位置」。

### Q78. 19 個存檔鈕的「A02 開著、OP 等級時直接關頁、不存」保護，912 才有，要留還是照 #20 改回 906？（ST01-M 1003 01:1x 登記；ST01-E 的 E-030a 找到；同一題已在 FROM_STEVEN §3 問 Jimmy）

- **畫面上會發生什麼**：V912 在 19 個設定頁的存檔處理加了一段 A02 `Close(); return;`（RogerYang 的保護）：AOISetup、BinSelect、GroundMan、IniConfig_OCR、Ld_UldDelayTime、Rotate、StartCondition、Temperature、BarCode、Cleaning、QAMode、SetUp、TesterIF、VacuumUnit、YieldMonitoring、TrayForm、TTLCfg、UserDefForm_File、Contact（另有 TrayAssignment）。906 都沒有。**只有 Config [A02]（切到 OP 等級不准存參數）打開、而且目前是 OP 等級（AccessLevel＝0）時**，912 這 19 頁按存檔會直接關頁、不寫檔；906 在同樣情況會繼續存（一般對話框頁存操作員改的值；Contact 是非對話框，Close() 先跑 FormClose，改的值被丟掉、檔案用原值重寫）。**A02 沒開或工程師等級以上，兩版都照常存。**main／review6 自動產生的 .gen.inc 裡是 V912 的寫法。（⛔ ST01-M 1003 02:0x 更正：01:1x 的版本寫成「912 永遠不存」，漏了 A02＋OP 等級這個條件；ST01-E 指出。）
- **A（ST01-E 建議）**：留著 912 的保護，記成第 20 條的例外（跟 Q77 Auto Clean 一樣是保護）。例：A02 開著、OP 等級在 Cleaning 頁改了值按存檔 → 頁面關掉、不寫檔；工程師等級照常存。
- **B**：照 #20 改回 906。例：A02 開著、OP 等級在 TrayAssignment 改了值按存檔 → 寫進檔案；在 Contact 改了按存檔 → 改的值被丟掉、檔案用原值重寫。
- **登記時的狀態**：等 Jimmy／Steven。

**裁決**：**A：照 912**——19 個存檔處理的 A02 保護（Config [A02] 開著、OP 等級時直接關頁、不存）保留，記成第 20 條的例外；**程式註解同時寫 V912 與 906 的行號**（906 沒有這段的地方，寫明 906 對應的位置／「906 無」）。St01 照做：todo E-030 註記，human-review C24。
**⛔ 1003 14:2x 補**：E-031 找到第 20 個——HotPlateForm_File（V912 cHotPlate.cpp:447 也有 A02 `return;`，0618 沒有），同樣照 912 保留（Jimmy RULINGS_20261003 第 6 條、筆電 14:2x 同意），註解在全面切換時補上。

### Q79. ⚠ 送給測試機的接觸力字串（GPIB）現在照 912 的算法，要留還是改回 906？（ST01-M 1003 02:0x 登記；E-030a 報告 §8 (b)；同一題已在 FROM_STEVEN §3 問 Jimmy）

- **畫面上會發生什麼**：Contact 頁讀檔時算出每支手臂的接觸力字串 asArmForce1／2，這個字串會經 GPIB 送給測試機。V912 ReadFile :609-616 用 `CalcDeviceForce(...,true)`（RogerYang 20260624，註解「這裡決定送 Kgf 還是 N」）；906 :607-608 用 `iPinCT*ForcePerPinN/9.8`。畫面上的力量顯示（ShowArmAndDeviceForce）也一樣是 912 的算法。CalcDeviceForce 在 906 _0625 裡沒有。main／review6 現在送的是 912 算出來的值。
- **A**：留著 912 的算法，記成第 20 條的例外（像 #20c ADAM 那樣，當成真的 bug 修正）。例：測試機收到的力量照 912 的 Kgf／N 換算。
- **B**：改回 906 的算法。例：測試機收到 `腳數×每腳牛頓數÷9.8` 算出來的值，跟今天 906 量產機一樣。
- ST01-E 沒有建議，請 Jimmy 判斷（跟安全有關：力量單位錯會讓測試機設定錯的壓力）。詳見 `D:\AI_TempFile\st01e-e030a-report-20261003.md` §8。
- **登記時的狀態**：等 Jimmy／Steven。

**裁決**：**A：照 912**——Contact 讀檔算 asArmForce1／2 用 V912 的 CalcDeviceForce（ShowArmAndDeviceForce 同），送給測試機的 GPIB 力量字串維持 912 算法，記成第 20 條的例外；**程式註解同時寫 V912 與 906 的行號**（906 ReadFile :607-608 的 `iPinCT*ForcePerPinN/9.8`、ShowArmAndDeviceForce :1911-1912）。St01 照做：todo E-030 註記，human-review C25。

### 20261003 05:3x Steven 裁決：W70（從 decisions-pending.md 搬來）

### W70. ELA 事件記錄分析器的欄位拆法要不要也改回 906？（St02-M 1003 04:55 請 ST01-M 轉 Steven；ST01-M 05:0x 登記）

- **背景**：照 #62＝A，St02 已把 Data.Observer 的事件記錄檢視改回 906（golden cObserver.cpp GetEventLogText 用 CommaText）。但 ELA 分析器（`HT9011UC_Cpp_V3.33.906.0/EventLogAnalysis/ElaCore.cpp:672`、`ElaChipMos.cpp:654`）還是用 912 的拆欄（`EventLogCsv.h` 的 ela::SplitEventLogCsv）。ELA 不是 golden HT9045 程式裡的功能，Steven 之前 W15＝B 明確選了 912 的拆欄。
- **A（St02 現在的做法）**：ELA 維持 912 拆欄，當成 W15＝B 的延續（ELA 本來就不在 golden 裡）。例：分析器的停機次數、時間照現在的算法。
- **B**：ELA 也改回 906 的 CommaText。例：測試裡釘住的數字會變（ELA_Core 9a 停機 131 次／309 秒／MES 123 → 123／0／115；ELA_Core 10 6／105 秒 → 5／98；ELA_Reports 2 → 3），要實跑才量得到。
- 另外 St02 發現：移植樹的 CommaText（vclcompat）只用逗號切，BCB6 還會在沒加引號的空白切，所以「改回 CommaText」對有空白的列其實等於 912 的拆欄；要不要把 vclcompat 改成 BCB6 的切法，是筆電的檔，已在 CHAT_ST02 問筆電。
- **登記時的狀態**：等 Steven。

**ST01-M 先對過兩版原始碼**（Steven 問「W70 在906跟912沒差異 對吧？」）：ELA 分析器本身 906、912 都沒有；但事件記錄的拆法兩版不同——906 cObserver.cpp :3848 等用 `tsRow->CommaText`（BCB6 的 CommaText 連沒加引號的空白也切），912 cObserver.cpp :4023 另寫 `ParseEventLogLine`（只在逗號切，:3977 註解寫明就是為了避開 CommaText 切空白）。例：`07 Tester I/F` 這種含空白的欄位，906 拆成 11 欄，912 拆成 9 欄。
**裁決**：Steven 原話「看起來912的做法比較好啊」⇒ **A：ELA 維持 912 的拆法**，記成第 20 條的例外（跟 W15＝B 一致）；程式註解比照 Q78／Q79 同時寫 906 的行號。（Data.Observer 事件記錄檢視——St02 #62 第四批 c912-4 已改回 906 CommaText——要不要也照 912，ST01-M 05:31 另問 Steven；回答前請 St02 先不要推那一段。）

### 20261003 10:2x 依 Steven 既有裁決結案：W71（從 decisions-pending.md 搬來）

### W71. Qorvo 機台的 Tester Pause 蜂鳴：馬上響（906），還是等最長測試時間到才響（912）？

- **畫面上會發生什麼**：測試機叫停（Tester Pause）時的蜂鳴。906 馬上響；912（main.cpp:16372-16381、ckernel.cpp:744-748／:2144-2148／:2158-2162，RogerYang 20260626）等 MaxTestTime 到了才響，按 Alarm Reset 後重新計時。St02 MR !132 先改回 906。
- **A**：照 912（等最長測試時間才響）。例：Qorvo 機台測試機暫停 3 秒就恢復，蜂鳴不會響。
- **B**：維持 906（馬上響）。例：測試機一暫停蜂鳴就響。

**結論**：同一題 Steven 已在 0927 裁過 W8＝A（「已做，A」＝留 912）；加上 1003 常設規則 ⇒ **A：照 912**（等最長測試時間才響、Alarm Reset 後重新計時），註解寫 906 :行號與做法＋912 修正行號。St02 MR !132 改回 906 的那段要用修正 MR 改回 912（筆電：往 912 改的新變更先等 Jimmy §0 第 78 項）。Steven 若要改判再說。

### 20261003 10:0x～10:1x Jimmy 裁決：Q80（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20261003.md` 第 1、2 條；從 decisions-pending.md 搬來）

### Q80. 0618 拿到之前，St01／St02 還能用 906 的 0625 當翻譯基準嗎？（St02-M 1003 08:37 請 ST01-M 轉 Steven；St01 同樣情況；ST01-M 08:4x 登記）

- **背景**：Jimmy RULINGS_20261002 #20 與筆電說 golden 一律是 906 的 **0618**，0625 只供對照。St01、St02 兩台都讀不到 0618（St02 只有加密 7z；St01 只有 `D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260625_Steven`），照 Steven 0927 的裁決一直用 **0625**。N07 SECS/GEM 斷線警報（St02 MR !137／!138）就是 0625 才有、0618 沒有的例子，筆電因此先不收（夜間報告 §0 第 79 項）；St01、St02 已推的工作也可能有 0625 才有的行。
- **A（ST01-M 建議）**：繼續用 0625，請筆電給 0618 對 0625 的差異清單（至少 St01／St02 碰過的檔），0625 才有的項目先標出來暫停、交 Jimmy 定。例：N07 先不收，其他照常。
- **B**：0618 拿到之前停掉新的翻譯工作。例：St01／St02 只做 bug 修正與測試。
- **另外**：Steven 若能讓兩台讀到 0618（解開的樹放共用區，或口頭告訴壓縮檔密碼——不寫進任何檔），之後直接照 0618，這題就不存在。
- **St01 這邊拿到 0618 時要對的**（ST01-E 1003 08:50 列）：E-030 的 AMD 三個條件依賴 0625 cContact.cpp :1418／:1587／:1709——這是唯一要拿 0618 確認內容的；Q-A、Q78、Q79 註解寫的 906 行號、引用一次改（review6 `9c187d77`、q59 `c4b0937e`）的行號要對 0618 重核（只是註解）；Q-B、ASE-CL、D-026 WAR04217、條碼 CSV 比對是拿掉 912 才有的東西，0618 只有在「有」這些時才會不同（不太可能）。
- **登記時的狀態**：等 Steven。

**裁決**：Jimmy 第 2 條（§0 #80）＝**A：用統一的共用區 7z 密碼自己解開 golden 0618，之後一律照 0618，0625 只做對照**（密碼在 GitLab `docs/handoff/TO_STEVEN.md` §2 須知，不寫進任何檔）；已在 main 的程式用 NB2 R179 指紋工具（`v906/nb2-assist` `HT9011UC_Cpp_V3.33.906.0/docs/nb2_assist/w16/golden_fingerprint.py`）找出 0625 才有的行逐項查——是修正或比較好的照第 1 條保留並兩邊註明，其他改回 0618。第 1 條（§0 #78）＝**B：全面接受 Steven 1003 常設規則**（912 是修正或明顯比較好的就留 912，做的人自己判斷、不用等 Jimmy；兩邊行號都寫，帳本記一列理由）。St01 照做：todo E-032（St01 的工作對 0618 重核）。

### 20261003 14:3x Steven 裁決：W73（從 decisions-pending.md 搬來）

### W73. Murata 機台 2DID 比對 NG 的料，要不要寫一筆 NonTestToRBin 紀錄（912）？

- **畫面上會發生什麼**：客戶碼 CC_Murata、2DID 比對 NG 的料照樣放到 iTestBinCount；912 atester.cpp:1048 另外寫一筆 PordRec「NonTestToRBin」，批次統計的 E1 數會把它算進去；906 atester.cpp:1038-1058 沒有這行。St02 MR !136 先照 906（不寫，human-review B35）。
- **A**：照 912（寫這筆，E1 數包含這些料）。
- **B**：維持 906（不寫）。

**Steven 的裁決**：原話「W73. Murata 機台 2DID 比對 NG 的料，要不要寫一筆 NonTestToRBin 紀錄（912）？ A. 通知Jimmy, Steven認可的就是直接放行」 ⇒ **A：照 912**——CC_Murata 機台 2DID 比對 NG 的料照樣放 iTestBinCount，另寫一筆 PordRec「NonTestToRBin」（912 atester.cpp:1048），批次統計的 E1 數會算進去；註解寫 0618 行號與做法＋912 行號（常設規則）。St02 開修正 MR 補回（St02 MR !136 先照 906 拿掉的那行）。
**同時的常設指示**：「Steven認可的就是直接放行」——Steven 裁決過的項目直接進下一步，通知 Jimmy 即可，不必再等 Jimmy 點頭（例：W71 Qorvo 蜂鳴的 912 修正 MR 不用再等夜間報告 §0 第 78 項）。

### 20261003 14:3x Steven 裁決：W72（從 decisions-pending.md 搬來）

### W72. AMD 的特殊流程：用編譯旗標（906），還是執行時看 2DID 格式（912）？

- **畫面上會發生什麼**：906 main.cpp:18158-18168 的 AMD 版本包在沒有定義的 `AMD_Version` 編譯旗標裡（等於只跑 Delta Castle 那段）；912 main.cpp:18785／:18834／:19076 改成執行時看 `TestIF_File.i2DIDFormat==eAMD`（Ifor 20260717「不使用define方式處理」）。St02 MR !134 先改回 906。
- **A**：照 912（2DID 格式選 AMD 就走 AMD 流程）。例：AMD 客戶的機台不用另外編版本。
- **B**：維持 906（只有 Delta Castle；AMD 要另外編譯）。

**Steven 的裁決**：原話「W72. AMD 的特殊流程：用編譯旗標（906），還是執行時看 2DID 格式（912）？ A. Checked by Steven」 ⇒ **A：照 912**——AMD 特殊流程改成執行時看 `TestIF_File.i2DIDFormat==eAMD`（912 main.cpp:18785／:18834／:19076，Ifor 20260717「不使用define方式處理」），不再靠 906 沒定義的 `AMD_Version` 編譯旗標；註解寫 0618 行號與做法＋912 行號（常設規則）。St02 開修正 MR 補回（St02 MR !134 先改回 906 的那段）。照 Steven「Steven認可的就是直接放行」，通知 Jimmy、不必等他點頭。

### 20261003 14:3x Steven 裁決：Q81（從 decisions-pending.md 搬來）

### Q81. Contact 旁的 TrayForm（料盤設定）頁改讀 906 0618 時，6 個 912 才有的客戶專屬功能要不要留？（ST01-E E-031 試切換；ST01-M 1003 14:0x 登記）

- **背景**：Jimmy RULINGS_20261003 第 4 條讓 St01 把網頁表單產生器改讀 906 0618（todo E-031）。第 1 條寫「912 是修正或比較好的保留；**912 只是寫法不同、或 912 才有的客戶專屬功能，照 906**」；Steven 的常設規則說客戶專屬的要問 Steven。TrayForm 試切換（側分支 `v906/st01-e031-goldenroot` `725cde57`）會拿掉這 6 項：
  - B1 HT9046_LS 轉盤 kit：讀 Tray.Data 時不再強制關 Fix1／Fix4（V912 FixCanUse :114-123）。
  - B2 上下 AOI：讀檔時不再強制關 Fix2／3（5/6 或 8/9）（V912 :139-161）；但 TrayForm.cpp 開機預設還是會關，兩邊會不一致，要一起處理。
  - B3 CYUEAN：bDisableAutoTrayFeed 不再在讀檔時清掉 bAutoFeed（V912 :277-280）。
  - B4 CYUEAN：頁面上 chkAutoTrayFeed 不再被取消勾選並鎖住（V912 :859-865）。
  - B5 KYEC：AMR 分頁（fLotInfo->tsKYEC_AMR）不再跟著 bEnableAMR 顯示（V912 :538-545、:1399-1403）。
  - B6 KYEC：GroupBox2_KYEC 又看得到（906 的 dfm）。
- **A**：6 項都留 912（當常設規則的例外）。例：KYEC 機台的 AMR 分頁照舊跟著設定開關。
- **B（ST01-M 建議，照 Jimmy 第 1 條）**：照 906，6 項拿掉；B2 要同時把 TrayForm.cpp 開機預設對齊 906，避免不一致。HT9050 的客戶是台積電龍潭，這幾個客戶（HT9046、CYUEAN、KYEC）的功能用不到。
- **C**：逐項決定。
- 同一次試切換照第 1 條保留、不用問的：CASE-20260611-001 下半部 Fix 方向／料盤種類同步（V912 :453-481，客戶案件修正）、C27 Fix3、C24 Q78 的 return。後面其他結構大多有同類客戶功能，Steven 答了這題，ST01-E 才分批全面切換。
- **登記時的狀態**：等 Steven。

**Steven 的裁決**：原話「Q81. Contact 旁的 TrayForm（料盤設定）頁改讀 906 0618 時，6 個 912 才有的客戶專屬功能要不要留？ A. Checked by Steven」 ⇒ **A：6 項客戶功能都留 912**（B1 HT9046_LS 轉盤 kit、B2 上下 AOI、B3／B4 CYUEAN 自動進盤、B5 KYEC AMR 分頁、B6 KYEC GroupBox2）——E-031 TrayForm 改讀 0618 時這 6 項用 golden_root 的 per-method V912 override 保留，註解寫 0618 行號與做法＋912 行號（常設規則；Steven 選 A 而不是 ST01-M 建議的 B）。後面其他結構的同類客戶功能，比照本題預設 A（留 912）並在每批列出，Steven 要改再說。照「Steven認可的就是直接放行」通知 Jimmy。

### 20261003 14:5x Steven 裁決：Q82／Q83（ST01-M 14:4x 在對話裡問，Steven「有等 Jimmy 判定的可以讓我來裁決」）

#### Q82. 筆電移植的 7 處照 golden 0618 抄了 `==`（應為 `=`，那行等於沒做事），0625／912 已修

- 來源：St01 E-032 重核（FROM_STEVEN §3 1003 11:51；報告 `D:\AI_TempFile\st01e-e032-report-20261003.md`），原本「交 Jimmy 依 RULINGS_20261003 #1 決定」。
- 7 處（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`）：csystem OCR WAR0955 `ret==ShowErrorMessage`（csystem.cpp:32417；0618 :18646）按重試也不重試；uYieldMonitoring DoRTAutoSocketOff（0618 :5249／:5287-5288）關閉旗標設不上；acatchtray DoLoadCarRotArmReadRFID ×3；OCRInsp :408；cObserver RecordIndexTime :2897；cShowBinSelect ChangeBinDispStatus :272；cTemperFrom ShowOffYieldFun :1600（#20a，V912 是 `=`）。
- **Steven 的裁決**：原話「Q82.  A checked by steven」 ⇒ **A：照 0625／912 改成 `=`**，三段註解（0618 行號＋做法、912／0625 行號、"#20 exception (Steven 1003 standing rule)"）。St01 開側分支 MR（todo E-034），筆電 gate 後合；OCR 重試與 RT 關 socket 是行為改變 ⇒ human-review A 區給 EastSun。照「Steven認可的就是直接放行」通知 Jimmy。

#### Q83. SECS「Use Die Force」三個版本編號不同

- 0618（移植樹現況 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\SECSGEM\uHGemHT9045_SV.cpp:790`）：SV 2022，且 EC 2022＝「The no of pins on die」（`uHGemHT9045_EC.cpp:569`）同號；0625：EC 2025＝Use Die Force、EC 2022＝pin 數；912（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\SECSGEM\uHGemHT9045_EC.cpp:158`／`:161`）：EC 2022＝Use Die Force、EC 2025＝pin 數（跟 0625 對調）。
- **Steven 的裁決**：原話「Q83. SECS 的「Use Die Force」開關，三個版本的編號都不一樣 A. checked by steven」 ⇒ **A：照 912**（EC 2022＝Use Die Force、EC 2025＝pin 數；拿掉 SV 2022），三段註解。SECS 是 St02 領域 ⇒ 派 St02（FROM_STEVEN §4／CHAT_ST02）。照「Steven認可的就是直接放行」通知 Jimmy。

### 20261003 16:2x Steven 裁決：Q85（從 decisions-pending.md 搬來；筆電草稿 MR !70 TIMER_TABLE_PLAN §8 Q2）

### Q85. 告警框開著的時候，這些 Timer（TimerESD／Timer3／Timer8 等）要不要繼續跑？

- 白話：例如告警框開了 20 分鐘，A01 閒置自動登出要不要照數。9/30 Jimmy 答 St01「照 golden，框開著照數」（已做在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp`），St02 S-13／S-15 也這樣設計；但 1001 RULINGS_20261001 第 39 條第 3 題選了「框開著不跑」，兩者矛盾。第 132 包已做「框開著時快時鐘照跑」。
- **A（建議）照 golden，框開著也跑**：等待迴圈也呼叫排程表（跳過正在跑、跳出這個框的那一支）；跟 9/30 答覆、St02 設計、第 132 包一致。
- **B**：框開著全部不跑（A01 也要改回不數）。
- **C**：逐支決定（計數／記錄類照跑、會跳框的不跑；工作量大）。
- **登記時的狀態**：等 Steven。

**Steven 的裁決**：原話「Q85. 告警框開著的時候，這些 Timer（TimerESD／Timer3／Timer8 等）要不要繼續跑？ A」 ⇒ **A：照 golden，告警框開著時 Timer 也照跑**——排程表在告警框的等待迴圈也被呼叫（跳過正在跑、跳出這個框的那一支），A01 閒置計數維持 9/30 的做法（框開著照數），St02 S-13／S-15 的設計不用改；取代 RULINGS_20261001 第 39 條第 3 題的「框開著不跑」。照「Steven認可的就是直接放行」通知 Jimmy（!70 的 Q2 解除）。Q84（派發器要不要併）仍等 Steven。

### 20261003 16:3x Steven 裁決：Q84（從 decisions-pending.md 搬來；筆電草稿 MR !70 TIMER_TABLE_PLAN §8 Q1）

### Q84. St02 寫的 Timer8 小派發器（MR !48，已在 main），之後要不要併進排程表？

- 白話：St02 的派發器在 PumpTick 裡每 1000 ms 叫 Timer8；排程表做同一件事，另外有開關、統計、看門狗。兩套並存時，查「哪幾支 Timer 在跑」要看兩個地方。
- **A（建議）**：併進排程表；派發器與 `WebBridgeTags.cpp:605` 那一行退場（先在交接檔跟 St02 講好）。
- **B**：兩套並存。
- **登記時的狀態**：等 Steven；!70 仍是草稿、筆電還沒排。

**Steven 的裁決**：原話「Q84  A」（16:2x 先問「這是做什麼用的? 簡單說明它的用途」，ST01-M 白話說明後回） ⇒ **A：併進排程表**——St02 MR !48 的 Timer8／TemperatureStorageMinute 本體（`MainTimer8.cpp`）改由排程表當 OnTimer 呼叫，St02 的小派發器（`MainTimersSt02.cpp`）與 `WebBridgeTags.cpp:605` 那一行退場；機台行為不變（一樣每 1000 ms）。做的人：!70 的排程表由筆電做（LI-6／INBOX 145 ②），接上時先在交接檔跟 St02 講好退場的行（St02 的檔）。S-13／S-14 改寫進排程表的空殼。照「Steven認可的就是直接放行」通知 Jimmy 與 St02。!70 的 §8 兩題（Q84＝Q1、Q85＝Q2）至此都已裁決。

### 20261003 21:2x Steven 裁決：B69（分軌 Auto ART 加回 HandlerSys → Loader／Unloader → Track Matrix）

- 來源：ST01-E2 問 Steven（說明這個設定在 HandlerSys → Loader／Unloader → Track Matrix 畫面，建議加回）；Steven 21:2x 跑完 /usage 回「Go」，ST01-E2 依前後文當成同意 B69。
- **裁決**：加回分軌 Auto ART（rgAuto1-6ART），ST01-E2 做（側分支 off review6 `9db913c7`），同時更新 skill dfm-html-proxy-mapping.md 原本禁止投影 rgAuto1-6ART 的規則；照「Steven認可的就是直接放行」。

### 20261003 21:3x Steven 裁決：Q86（從 decisions-pending.md 搬來）

### Q86. B70／B71 拿掉的兩個 912 客戶功能，要不要照 Q81 的預設改回留 912？

- **B70 Teradyne-US 的 START 防呆**：912 在 Contact 存檔前，Teradyne-US 機型多一個 return（V912 DeviceForm `:6533-6534`）；現在照 906（0618 main.cpp:6261-6266，沒有這個檢查），移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp`:749-753。
- **B71 ASE-CL 灰掉 edContactHeight1／2**（Chrischen 20260316 的客戶需求，V912 :1306-1307、:1021-1028、:1478-1485、:15577-15633）：現在 7 處都以 `#if 0` 關掉（`DeviceForm_File.gen.inc` :1089、:1277、:2375、:2465、:2486、:2502、:2523），0618 沒有。
- **A（建議）**：照 Q81 的預設，兩項改回留 912，三段註解（0618 行號＋做法、912 行號、#20 exception (Steven Q81 default)）；HT9050 的客戶（台積電龍潭）用不到，所以機台行為不變。
- **B**：維持現狀（照 906，Jimmy #23-5／6），Q81 只管 TrayForm 那 6 項與之後的批次。
- **登記時的狀態**：等 Steven。

**Steven 的裁決**：原話「Q86 目前不急，慢慢做就好了，答案A」 ⇒ **A：B70（Teradyne-US START 防呆）、B71（ASE-CL 灰掉 edContactHeight1／2）都改回保留 912**，三段註解（0618 行號＋做法、912 行號、#20 exception (Steven Q81 default; Q86)）；**不急**——todo E-037 低優先，排在 E-031 第 1 批、E-036 之後。HT9050（台積電龍潭）機台行為不變。照「Steven認可的就是直接放行」通知 Jimmy。

### 20261003 22:1x Steven 裁決：Q87（筆電 NIGHT_REPORT §0 #86：HT9050 Index 自動測高 Auto Height）

- **題目**：解閘前要不要加 golden 沒有的防護（NB2-1 R197：port 只有宣告、呼叫在 #if 0，今天不會動 Z；照 golden 翻會卡在 case 555 等扭力；把 1203 的 6077h 直接接進 edTorue0 會因單位差 10 倍／正負號讓門檻永不觸發）。選項 A 照翻＋三道防護、B（建議）先關著等 EastSun 量、C 照翻不加防護。
- **過程**：Steven 21:4x「Q87, 你沒有馬達驅動器的手冊嗎？」「你這題丟回去給eastsun 他有手冊，等他確認」（St01 寫 TO_ES02 main `3b63f43c`）；「能同時間，到網路上爬文嗎？」（St01 查到安川 SIEP C710812 02H：6077h＝扭力命令、預設 0.1 %、正負跟參考座標，貼 TO_ES02 main `e3403015`）。
- **Steven 的裁決**：原話「所以我們可以開工Q87嗎？ 把安川讀回來的值，轉換成跟原本國際牌馬達一樣的比例」「正負號是要比較的」 ⇒ **A：開工**——把安川 6077h 讀值換算成 golden 國際牌（Panasonic）驅動器的同一比例（依 2704h 分子／分母，預設 0.1 % → %），**保留正負號比較**（只在往下壓的方向達到門檻才停，方向由設定／量測決定，不取絕對值），加 port 專用防護（扭力來源未確認前拒絕、case 555 逾時→ST＋警報），程式先寫好測過，**機台上等 EastSun 照手冊／上機確認 M14 的 2704h 與下壓正負號後才解開**。St01 做（todo E-038），照「Steven認可的就是直接放行」通知 Jimmy。

### 20261003 22:4x Steven 裁決：S-17 可以改 dialog-bridge.js（Steven 的對話框合約 v1.3.0）

- 來源：St01 接筆電卡 S-17（對話框「C++ 單槽、網頁排隊」全面檢查，另一個驗證用 session 第 1 步唯讀報告）；D 項（排隊中的框收到關閉要求被拒、同一個 100 ms 輪詢兩個關閉要求掉一個）要改 `web/page/dialog-bridge.js` :344-371／:712-740，那是 Steven 定的合約，先問 Steven。
- **Steven 的裁決**：原話「Steven 同意改 dialog-bridge.js 嗎？ 改」⇒ 可以改；合約版本升 v1.3.1 並在檔頭註明；另一個驗證用 session 跟 A／B／C 一起做在 `v906/st01-s17`。

### 20261003 22:5x Steven 裁決：Q88（筆電 NIGHT_REPORT §0 #87：機台自己寫的 HOME 修正 cpp 0160～0179，St01 V-6 的 3 個中度安全問題）

- 題目：A 照機台現況收進第 53 批、修正等 EastSun；B 先改 M-a 再收；C 先不收。St01 V-6：M-a 斷電又回來自動重置＋激磁＋繼續 HOME；M-b 真警報每秒重試 20 次；M-c 驅動器回原點只看 READY。
- **Steven 的裁決（機台端歸零規則，跟 golden 分開）**：原話「斷電後就必須歸零」「Alarm如果可以clear, 基本上也是需要歸零」「Alarm如果不能clear,就必須斷電重啟後歸零」「機台只要不斷電，基本上不需要歸零」「這個動作跟golden是分開的無誤」「一直沒斷電：已經歸零過的軸不用再歸零。但是要把encoder pulse回寫給command pulse」「比照golden的 servo on功能」。
  ⇒ M-a：HOME 途中斷電（含急停／開門）＝停止 HOME、全部軸要重新歸零，電回來不可自動繼續；M-b：警報清得掉就清（一次）、清掉的軸要重新歸零；清不掉就停、提示斷電重開再歸零；沒斷電、已歸零的軸不用重歸零，但伺服 ON 時要比照 golden ServoOnOff 把 encoder 回寫成 command（也解 NB2 !151 M2）。M-c 建議加原點到位旗標。St01 直接寫 TO_ES02（main `e2931f5a`／`10ab026f`）；收進 main 的時機照筆電（A＝照機台現況收、修正跟上）。

### 20261003 22:5x Steven 裁決：Q89（筆電 NIGHT_REPORT §0 #88：開機時 8 支 Z 軸煞車可能在馬達電源上來前就放開，NB2-1 R200）

- **Steven 的裁決**：原話「Q89 應該要先servo on後才能放煞車」⇒ **A 的方向**：放煞車條件＝馬達電源已上＋本輪伺服 ON 完成（＋延遲數完），開機不可因上一輪留下的 SVON 就放煞車。St01 寫進 TO_ES02（main `10ab026f`）；改的人照筆電安排（NB2-1 或機台端）。

### 20261004 07:2x Steven 裁決：Q90～Q97（1003 22:5x 留到上午的 8 題，一次回）

- 方式：St01（ST01-M）在對話裡用選擇題問，Steven 每題選一個；題目原文在 decisions-pending.md 20261004 07:2x 以前的版本（review6 `eea47df0`）。
- **Q90（E-038 要不要扣撐住自重的扭力基準）＝A 不扣，照 golden**：golden 國際牌路徑沒扣（golden 0618 rs232.cpp :1809-1812，k/20、負值歸 0）；ini 選項 HT9050_INDEXZ_TORQUE_BASELINE 預設關，等 EastSun 量完自重再看要不要開。E-038 Phase A（`v906/st01e-e038` `e6cec741`）本來就照 A。
- **Q91（E-038 Phase B：golden 自動測高本體約 8,200 行誰做）＝A St01 接**：St01 在筆電 CT-3c 卡上認領（Do_Z1／Z2_AutoGetHeight、DoTestContactFunction、Do_LoadCellAutoHigh、GetAutoHeightMaxKGTorque）；解開 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp`:31320-31322 的 #if 0 仍要等 EastSun 的 E-10 量測。
- **Q92（主迴圈卡死 ≥60 秒自動錄 State Record）＝A 自動錄**：照 golden hang-up 自動錄的先例（golden 0618 aTester_Front.cpp:7384、csystem.cpp:9685）；每次停擺只錄一次、只寫 `W906_*` 檔。St02 S-24 做。
- **Q93（對話框開著時 State Record 鈕還能錄）＝A 放行**：移植專用例外（同 motor.stop／act.home.abort 寫法），唯讀、不動機台。St02 S-24 做；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` :671／:856／:6801 這三行 St01 S-17 也改過，St02 動之前先跟 St01 對行。
- **Q94（🔴 §0 #92：HT9050 Index 下壓回「完成」但 Z1 沒動）＝A(a)**：**先擋**——HT9050 呼叫成對下壓時報警、不回完成（NB2-1 MR !170 已做，選 A 才合）；**正式版移植 V910 DoTestHead**（Frank 的 910 版本＝HT9050 真正的 Index 流程；移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp`:657 現在沒有），不走「馬達層成對指令→只動 Z1」。
- **Q95（§0 #93：開機照 Mot_Table 決定 Index 四軸存不存在）＝A**：只 HT9050 走，其他機種照 golden；改掉 Jimmy 0930 的 D1＝A。NB2-1 做。
- **Q96（§0 #91：開機驗表 St01 S-23 C5）＝B 做，而且 ERROR 擋 START**：12 條規則寫 op log＋開機摘要「驗表：ERROR n／WARN n」；**ERROR 級（例：HT9050 上 IO_CARD_TYPE 不是 4、1203 軸號重複）擋 START**，WARN 只記錄。golden 沒有＝新功能（not-golden）。設計在 `docs/handoff/S23_FINDINGS_A1A2C5_20261004.md` §5、§7；St01（ST01-E2 的設計）做。齒輪比讀成 0 不另外當 1（沿用建議）。
- **Q97（St02 S-24 改共用檔被 STEVEN-NB3 的權限檢查擋下）＝A**：Steven 本人到 STEVEN-NB3 的 St02-E session 允許這次修改；St02-E 先跑 `apply_hooks.py --dry` 再正式跑、兩組態建置、代跑後推。St01 不代改。
- 照「Steven 認可的直接放行」：St01 寫 FROM_STEVEN §3 通知筆電／Jimmy、CHAT_ST02 通知 St02，不等 Jimmy 點頭。
- **Q96 補充（Steven 1004 07:4x，ST01-E2 問 ERROR 要不要也擋 HOME）**：原話「一開始就開不了，怎麼會還可以home?」⇒ 開機驗到 ERROR＝這台機器不能運轉：**HOME 和 START 都擋**（不是只擋 START）；開機畫面照樣顯示驗表結果讓人知道要改哪一列。手動動馬達（Motor Test／JOG／Teach）要不要也擋，由 E-043 的設計列出入口後再定。
- **Q96 再補充（Steven 1004 07:4x）**：原話「明天有個任務是機台要可以動起來，先以安全能動為主，可以先不考慮擋」「但是明天18:00後要按照標準流程走」⇒ **20261005 18:00 以前**：驗表只寫紀錄＋開機摘要，不擋 HOME／START（先讓機台安全地動起來）；**20261005 18:00 以後**照標準流程：ERROR 擋 HOME 和 START。做法：E-043 分兩顆——第 1 顆只記錄（先合、先上機），第 2 顆加擋，18:00 以後才合／才打開；不用日期寫在程式裡判斷。
- **Q96 原則（Steven 1004 07:4x）**：原話「也就是：io 馬達的裝置有異常error的時候，機台就不可以動」⇒ 標準流程（20261005 18:00 以後）：IO／馬達裝置有 ERROR，**機台任何動作都不行**——HOME、START，以及手動動馬達（Motor Test／JOG／Teach）都擋。同一原則也適用執行中的裝置異常（例：S-26 R4 驅動器跑到一半報警卻沒有升成警報，要修成會停）。
- **Q94 補充（Steven 1004 07:5x）**：原話「Q94可以先不實作 測試流程，明天下午18：00後再做就好了」⇒ 正式版（移植 V910 DoTestHead 測試流程）**20261005 18:00 以後**才做；「先擋」（NB2-1 MR !170：HT9050 成對下壓報警、不回完成）照 A(a) 不變。

### 20261004 08:4x Steven 裁決：E-042（E-038 Phase B）的三題

- 方式：St01（ST01-M）用選擇題問；題目來自 ST01-E 的 E-042 計畫（`D:\AI_TempFile\st01e-e042-plan-20261004.md`）。
- **HT9050 的手動測高（Manual Height；golden 是伺服 OFF／Galil JG 推，1203 路由會拒絕）**：Steven 原話「手動測高有兩種方式」⇒ **還沒定案**：E-042 先把 golden 的兩種手動測高方式各自列出（入口、怎麼動 Z、在 1203 上能不能照做），再請 Steven 選；在那之前 HT9050 上手動測高維持拒絕（不動 Z）。
- **第一次解開（B6）要不要也開模式 3（Contact Test）**：Steven 選「2」＝**模式 1 和模式 3 一起開**（E-042 計畫 P8 的白名單改成模式 1＋3；仍先單臂、Load Cell 仍拒絕）。
- **B7（V910 DoTestHead）做好之前，HT9050 自動運轉停在 Index 扭力等待（atester case 12110）要不要逾時**：Steven 選「**加逾時：報警並停機**」⇒ not-golden；逾時秒數與警報碼要定（警報碼由 Jimmy 選，同 S-26 10-3）；待辦 E-044。

### 20261004 14:1x Steven 裁決：S-26 R4 派工、E-044 逾時秒數

- 方式：St01（ST01-M）用選擇題問（Steven 問「有沒有需要我決斷的項目？」）。
- **S-26 R4（驅動器移動到一半報警，自動運轉不會停、也不跳 JAM）**：Steven 選「派，先只寫計畫」⇒ ST01-E2 唯讀寫修法計畫（todo E-045），不改程式；計畫好了再決定誰做。
- **E-044 逾時秒數**：Steven 選「5 秒」（跟 E-038／E-042 的 P5 一致；比 E-038 的 10 秒提示先到，只跳一個警報）。

### 20261004 17:0x Steven 裁決：Q98（E-045／S-26 R4 的修正誰做、什麼時候做）

- 題目：ST01-E2 的唯讀計畫（`D:\AI_TempFile\st01e2-r4-plan-20261004.md`）提的副作用：HOME 後還停在 ERROR_STOP 的軸，會讓每次 START 前都要先 HOME。
- **Steven 的回答**：原話「Error stop的軸要嘗試 clear alarm, 如果不能clear, 就只能全機斷電重置」⇒ 修法加一條：ERROR_STOP 的軸先嘗試清警報（一次）；清得掉＝照 Q88 規則那一軸要重新歸零；清不掉＝報警、提示「全機斷電重置」（不自動重試、不繼續動）。跟 Q88（1003 22:5x）的歸零規則一致。
- 誰做／何時：Steven 沒有另選，照建議＝**ST01-E2 做**，跟 E-044 一起或更早進批次；進機台包前仍要 EastSun 回 Q1～Q4（TO_ES02 main `220edfa7`）。ST01-M 的理解，Steven 可推翻。

### 20261004 22:2x Steven 裁決：Q99（安川 A.A10／A.EA2）、Q100（A.9xx 警告）

- **Q99**：原話「可以reset就reset，不能reset的就斷電」⇒ 看實際結果：送一次 Reset 後驅動器真的回到沒警報＝算清掉（那一軸重新歸零）；清不掉＝提示全機斷電再 HOME。＝E-045（MR !185）現在的做法，不必改；不另外把 A.A10／A.EA2 固定歸到「一定斷電」。
- **Q100**：原話「電池低還是可以運作，所以不算」⇒ 安川 A.9xx 警告（例 A.930 電池低、A.910 過載預警）不算「裝置有 ERROR」，不擋動作；只記錄、提示（電池低要提醒換電池）。

### 20261004 23:1x Steven 回答機台題（1005 更正：原寫 23:3x，登記 commit f4baf1d2 是 23:15）（Jimmy RULINGS_20261004 #4：等 EastSun 的題目 St01 可答）

- **W-54（E-09 機台螢幕）**：原話「應該是full hd. 但是現場可能設定不一樣，這題留給eastsun」⇒ St02 先照 1920×1080（工作列露出時可用約 1032 高）修；EastSun 確認實際設定。
- **W-44（HT9050 Index 只有 Z1）**：原話「9050的飛梭分成 入料跟出料兩個。當兩個都在home位置的時候，index z1可以下壓到socket」⇒ Index Z1 下壓前的互鎖＝入料飛梭與出料飛梭都在 home 位置。
- **W-56（Out Arm 防掉氣缸 C_OutPnPDrop1-4、C_OutArmSmallY）**：原話「吸嘴會記錄有無ic的狀態，當執行suck的時候要由開到關。執行destory的時候要由關到開。有ic狀態下要夾起來（關, push） 無ic狀態下要開(pop)」⇒ 防掉氣缸跟著吸嘴的有無 IC 狀態：suck（吸料）時由開→關、destroy（放料）時由關→開；有 IC＝夾住（關、push），沒 IC＝打開（pop）。
- **W-45／W-46（急停輸入、SnServo、SnSystemPower、安全門）**：原話「不確定，請 EastSun 看」⇒ 維持現狀、繼續問 EastSun。

### 20261005 09:2x Steven 裁決：Q101（HT9050 自動測高把 IC 放回入料飛梭前，飛梭要不要移回右邊）

- **背景**：HT9050 的 Index 只有 Z1、沒有 Y 軸；W-44 要求測高（下壓 socket）前入料飛梭退到左邊（golden Do_Z1_AutoGetHeight case 150／151 的那段，E-042 B3b）；但 golden 把 IC 放回飛梭（TfContact::DoZPlaceToShuttle、DoArm1／2PlaceToShuttle）時從不移動飛梭（HT9045 靠 Index Y 移到 socket，飛梭一直在 Index 下方），所以 HT9050 照 golden 走會把 IC 放到沒有飛梭的位置（ST01-E 1005 08:2x 流程文件修訂時發現）。
- **Steven 1005 09:2x 選：加一步——放回前入料飛梭移回右邊**（AskUserQuestion，選項 1，St01 建議）。
- **做法（E-042 B4，not golden、只限 HT9050）**：最後一次測高之後、DoZPlaceToShuttle 之前：前提 Z1 在安全高度、出料飛梭 X 在 OutSHT[0].iRight ±100（不在交接點），讀不到或不符就報警＋ST、照 golden 出口、絕不移動；符合才把入料飛梭移到 InSHT[0].iRight 並確認到位（±100），再放 IC，然後照 Steven 7 步流程第 7 步退回 iLeft。跟 Steven 08:1x 的 HT9045 七步流程（第 6 步放回飛梭、第 7 步飛梭退回左側）一致。
- **上機**：human-review A66——EastSun 第一次用 1% 速度看這段移動。
