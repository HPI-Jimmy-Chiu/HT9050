# 夜間報告 2026-09-24 → 2026-09-29（週末連續運作；**進行中版本**，9/29 08:00 起收尾時整份重寫）

> 使用者 20260924：「周末任務也要執行下班任務，而且隔天不受上班時間影響，可以連續運作下去，目前預計9/29才要上班」。
> ⚠ **20260925 更新**：原本跑迴圈的 session 在 09-25 09:12 被關掉，10:0x 由新 session 接手（原因與經過見 `docs/INBOX_QUEUE.md` 最上面一節）。
> 使用者當面裁決 **T＝9/29（二）09:00** ⇒ 07:30 起不開新 gate、08:00 收尾、08:30 靜默。今天所有裁決的單一出處：`docs/RULINGS_20260925.md`。
> 迴圈：每小時 :07 一次（cron `cc97a0fb`；20260925 17:5x 為了省用量由每 20 分鐘改成每小時）＋9/29 07:57 單發一次開始收尾（`6eafadd2`）（session-scoped —— 視窗關掉或機器重開就停，磁碟上的 commit 與本檔就是全部狀態）。
> 順序（20260924 晚更新）：W0 馬達 → W1 通道普查 → W3 硬體物件層＋cinitial 初始化 → **W4 MotorTest 完善化 → W5 Teach 完善化** → W2 解除 `#if 0` → C 類。計畫在 `docs/WEEKEND_PLAN_20260925.md` §0.6 開頭的表。

### 怎麼讀

| 節 | 內容 |
|---|---|
| **§0** | **要你決定／處理的**（只放真的需要你的） |
| §1 | 做完了什麼（附證據與 commit） |
| §2 | 刻意沒做的（附原因） |
| §3 | 紅燈／哨兵 |
| §4 | 清理了什麼 |
| §5 | commit／push 清單 |
| §6 | 我自己犯的錯與更正 |

---

## ☀ 20260926（六）下午 12:4x～ 進度 —— 先看這節

### 要你決定的（三題）—— ✅ 0926 14:0x 已答：1＝A、2＝B、3＝A（RULINGS_20260926 第 27 條）

| # | 事情 | 白話＋例子 | 選項 | 建議 |
|---|---|---|---|---|
| 1 | 🔴 **R66-GALI**：第 2 條「甲」（引擎的 Z1 改走 1203）要不要連 `Gali_*` 那一層一起接 | golden 的 Index 流程（下壓測試、AutoClean、歸零、停止）**不是**呼叫馬達物件，而是直接呼叫 `MOT[MTestZ1].Gali_MotMove(...)` 這類 Galil 專用方法，全樹 105 個活的呼叫點，前面都沒有「是不是 Galil 卡」的判斷。例：就像把車換成電動車、也接好充電線，但油門踏板其實接在一台不存在的汽油引擎上 —— 只做第 2 條原案，START 之後 Z1 的生產移動還是到不了 1203。 | **A**：在 `Gali_*` 這一層依軸的 CardType（PCI1203）分流到 `Motor->`，改一個檔涵蓋 105＋處；B：105 處逐一改；C：只做原案 | **A**（機台端和第 2 條一起做） |
| 2 | 🟡 **R66-D13**：歸零時「Index 有沒有離開原點」檢查會讀到停用軸寫死亮著的 Home 燈 | HT9050 只有一支 Z（Z2 停用）。開了 D13（`bCheckIndexHomeSensor=1`）的話，停用那支永遠「沒離開原點」⇒ 歸零一直重來。預設是 0，所以今天不會發生。 | **A**：跟 R63 A 同一個做法，停用的軸不檢查；B：照 golden（HT9050 不能開 D13） | **A** |
| 3 | 🟡 第 9 條：告警框期間蜂鳴器要「一律叫」還是「照每個警報碼的靜音設定」 | 你 Q3 答「告警框期間蜂鳴器一律叫」。golden 其實還有一層：每個警報碼可以在安全設定頁勾「靜音」（`GetJemSilent`，例如客戶把某個常見的 WAR 碼設成不叫）。移植樹目前沒有這一層需要的欄位（fNote 的 sJamArea／sJamCode），我照你的字面做成**一律叫**。 | **A**：一律叫（現在的做法）；B：照每個碼的靜音設定（要先補那兩個欄位，約半天） | **A**，等有客戶真的用到「單碼靜音」再做 B |

另外 NB2 R66 提醒機台端兩件事（不用你決定，已記在 INBOX 第 51 列）：第 6 條必須在 `HSys.LoadMotData()`（cinitial.cpp:3874）**之前**生效，否則 Index 四軸會被建成停用的 SMC 軸；R63 A 不夠 —— Z1 變成 1203 軸後伺服燈也要從 1203 監看器取。

### 新的要你決定的（0926 14:3x～16:0x，四題）

| # | 事情 | 白話＋例子 | 選項 | 建議 |
|---|---|---|---|---|
| 4 | 🔴 第 8 條：VacuumUnit 頁的吸／破真空鈕在 HT9050 上要不要能用 | golden 只有 `Gerneral.ini [System] VacuUnitType` 不是 0 才顯示這一頁（HT9050 是 0）。golden 這兩顆鈕寫的是頁面裡**寫死的位址**，研究量到它在 HT9050 的 IO 表上剛好對到 **Loader／Auto1 的托盤氣缸**（`C_Load_Up`、`C_Auto1_Up` 等，IP 80/81、port 16-19）。照寫死的位址做，網頁按「吸」可能讓托盤氣缸動 | **A**：照 golden，`VacuUnitType=0` 時 hw.access 直接拒絕（HT9050 網頁上這兩顆鈕不能用）；B：HT9050 也要能用，改走 IO 表綁定的吸嘴點（偏離 golden，要先逐點確認位址） | **A** |
| 5 | 🟡 第 8 條：塔燈頁「試聽」期間要不要暫停警報音樂 | golden 塔燈設定頁開著時會暫停所有警報音樂（`ckernel.cpp` ShowRunLed 的 `fTowerLight->fShow` 閘），好讓試聽不被蓋掉。網頁照做的話，按試聽後的 30 秒內（Q4 的自動停）機台真的出警報也不會叫 | **A**：照 golden，試聽期間暫停警報音樂（最多 30 秒；斷線或權杖還掉時提早停）；B：試聽中一出警報就停止試聽、恢復警報音樂（偏離 golden，較保險） | **B**（機台是實彈，寧可不漏警報） |
| 6 | 🔴 **V912 量產碼**：`OCR.dfm` 少了 `rgOCRTriggerMode`（St01 14:32 報，筆電已查證） | 這個「OCR 觸發模式」選項是**我們 0522 在 V899 加的**（`AI(ht9045-v899) 20260522`），公司整併進 golden 906（0618）與 V912（0908）時只帶了 `.h`／`.cpp`，**沒帶 `.dfm` 那 12 行**（V899 `OCR.dfm:479-490`）。後果（BCB6 的規則推論，沒有在機台試）：V912 有裝 OCR（`INSTALL_OCR≠0`）的機台，一開 OCR 頁就跳 Access violation；按存檔在 `OCR.cpp:2181` 當掉，後面幾個鍵沒存到 | **A**：筆電把 V899 那 12 行補進 V912 的 `OCR.dfm`（照 dfm 格式、同一個父元件），**這台沒有 BCB6，要在有 BCB6 的電腦私有建置驗證**，再請公司確認有沒有已出貨的 OCR 機台；B：只通知公司（電話）由他們修；C：先不動 | **A＋電話通知**（改 `.dfm` 要你明說，所以先問）。⚠ Steven 14:3x 的判斷是「不是很重要，記為待辦」（skill 待辦 G-006） |
| 7 | 🔴 網頁存檔在機台運轉中要不要擋（St01 F-003） | golden 的設定畫面（Setup／Config 那排按鈕）只有停機時按得到（`main.cpp:3842-3847` 運轉中把那兩排藏起來），所以運轉中改不到工單參數。網頁的 `editlist.save`（各設定頁的存檔）**沒有檢查運轉狀態**；S88 之後溫度頁存檔會照 golden 跑 `SetWorkParameter` —— 運轉中從網頁存檔，後果跟運轉中換配方一樣 | **A**：C++ 的 editlist.save 一律擋 `SystemStart||SoftStart`（同 IO 頁的規則），簡單、最安全；B：逐頁比照 golden 那一頁在運轉中能不能開（要先列清單，少數 golden 允許運轉中改的頁會被 A 誤擋）；C：先不擋 | **A**，之後真的有「運轉中要改」的頁再逐頁開 |

### 做完了

| # | 做了什麼 | 證據 |
|---|---|---|
| 1 | **機台端（EastSun）的修改合進 main**（`20494aea`，TEMP-DOORS 不含）。合併時自己量到並處理四件事：① 機台版有 3 個檔是舊內容（`build.bat`、`pe_truncation_check.ps1`、`f5_contract_probe.cjs`，更新包刻意沒送 OBJROOT 那一版），直接合會把 main 的 OBJROOT 退掉 ⇒ 取 main 版；② G03 閘機台的註解說「關著」、程式卻是開的 ⇒ 保留照 golden 的開，註解就地改正；③ 5 個 HT9050 IO 測試被改成對照機台的**現場 IO 表**，但那張表沒跟過來 ⇒ 舊表時設 Disabled，請機台送表（INBOX 第 50 列）；④ MotorTest 的 Light Scale 進行中也不還權杖 | 合併前 gate 兩組態都多 6 個紅＝上面 ①③；處理後見下面 |
| 2 | **警報框一次只跳一個**（`0c0dd4ba`，NB2 R66 §5）：golden 關警報框時會把還在排隊的警報一起清掉（`note.cpp:2531`），移植樹沒有 ⇒ 兩個氣缸同時逾時就跳兩個框、停兩次機、按 START 被吃掉一次 | test_halarm [I]：真的跑 ProcessAlarm，修後 1 個框、對照組 2 個框 |
| 3 | **Steven 的 GPIB 引擎（P1）代跑**：編譯錯只有一個根因 —— 我們的 `WebBridge/Sync.h` 一行（已修 `04a2c66f`）；連結錯 2 個（同一個全域變數定義兩次）⇒ 這次沒進 main，改法寫給他了（TO_STEVEN.md §4） | 11 個 TU 逐一語法檢查、整包連結量出真正撞名的只有 2 個 |
| 4 | 兩組態 gate（`04a2c66f`）：出貨失敗集合＝基準 3 項、模擬＝基準 18 項，5 個 HT9050 IO 測試 Disabled；基準內失敗測試的子檢查逐行相同；system\ config\ 586 檔 0 變動 | 兩件環境雜訊見 §3 |
| 5 | **推 GitLab main `b5fb53be`**（40 顆，含 Steven 交接檔合併）；**GitHub 更新包 5 `updates/b5fb53be/`**（`49784c2`，94 檔，權杖／私鑰／7z 密碼掃描 0 處；裡面請機台送現場 IO 表） | 機台要不要套由你決定 |
| 6 | Steven 那邊：他現在分兩台（St01 資料讀寫轉檔、St02 測試介面），交接檔只在 `v906/steven-handoff`；開了三方聊天檔，我們這邊是 main 的 `docs/handoff/CHAT_JIMMY.md`，夜間迴圈每輪讀另外兩個。St01 問的 `hw.access` 位置已答（`wb_serve.cpp:5625` 後面，他可以先插自己的） | `f1b234de`、CLAUDE.md、night-loop 5b |
| 7 | **第 9 條**（`66c2480e`，審查 11 條全收）＋文件 `0dbb7e9f`＋Steven 交接檔 ⇒ **推 GitLab main `1e15c4f9`**；**GitHub 更新包 6 `updates/1e15c4f9/`**（`cb98e2f`，11 檔，掃描 0 處；說明裡知會機台「等待中也會 Poll 1203 監看器」與第 27 條） | 兩組態 gate＝基準；子檢查 0 差異 |
| 8 | 線 D 哨兵：接線自測 OK、配方 65 dirs／64 with Contact.Data（新筆電基準）、absence-claim 全部仍成立、system\ config\ 586 檔 0 變動 | `verify_wiring_live.py` 要開著的 wb_serve，這輪沒跑 |
| 9 | **安全 PLC 閘照 golden 打開**（`4abcdf0b`，第 20／22 條；SafePlcIO 維持 0，行為不變）＋**網頁權杖兩件**（`31d643e3`，NB2 R68／R69：motor.access 被拒不自動重送、按住 jog 時不還權杖）⇒ 推 main `bc5add9e`；**GitHub 更新包 7**（`cf15bbe`，10 檔） | 兩組態 gate＝基準；新 ctest `PlcGates` 兩組態通過；權杖 selftest 33／33（對照組紅 4 條） |
| 10 | **St02 測試機通訊（GB P1～P5＋P7＋P2c）第一次接上 wb_serve**（`79060249`）：合併時修掉 2 個匿名 namespace 的連結錯（`wb_serve.cpp:2867`／`:6759`）；**開機約 1 秒自動啟動 GPIB／RS232Standard 引擎**（`HT9045_TESTERCOMM=0` 可關）⇒ **GitHub 更新包 8**（`7185e04`，59 檔） | 兩組態 gate＝基準；`TesterComm_*` 4 個新 ctest 兩組態通過 |
| 11 | **教導頁 HOME 的權杖**（`378fbb77`，NB2 R69 B4）：HOME 進行中不還權杖、做完照 golden（`uTeach.cpp:1392`／`:1403`）彈起按鈕、進行中每 60 秒續一次 ⇒ **GitHub 更新包 9**（`be7bbf5`，1 檔） | 權杖 selftest |
| 12 | **St02 的 P2b(b)／P2e／P2d 合進 main**（`7f332938`）：atester.cpp 的 `GetTesterResult`／`ProcessTestResult`／`ProcessTesterTimeOut`／`DoIndexSocketCheck` 換成活的翻譯；主畫面 Tester 鈕（`act.main.testerConnect`，運轉中照 golden 不動作）；On-Line／Off-Line 切換本體（Off-Line 時不論配方選哪種介面都走 GPIB 的模擬，這是使用者裁決的偏離 golden，RS232／TTL 才有差）；**SECS/GEM 遠端切換（`uHGemHT9045.cpp:3431`）以前走空殼回 0，現在也真的會切**。筆電補了 1 個 include（St02 那台不能建）⇒ 推 main `4c067b5f`；**GitHub 更新包 10 `updates/4c067b5f/`**（`5575b48`，16 檔，要全量重編；權杖／私鑰／7z 密碼掃描 0 處）。⚠ 知會：St02 的 D2 照他們的裁決閘著 —— golden「機台裡還有 IC 就不准切換連線模式」（MES1646）**在 V906 沒有**；網頁目前還沒有按鈕呼叫它 | 兩組態 gate＝基準、子檢查 0 差異、system\ config\ 0 變動 |
| 13 | pagewire 分母照新筆電重量：`fields` 119 筆三元組在 64 份配方全部都在 ⇒ 分類不用改，只有註解的 `63/63` 過期（night-loop skill 待辦結案） | scratchpad 量測，唯讀 |
| 14 | **golden 的 IO 輸出快取越界修掉**（`6367d599`，0925 第 18／29 條「要修」，INBOX 第 16 列）：`OutPortData[4][64][4]` 裝不下 1203 的輸出位址，HT9050 表上 14 組不同的輸出點位共用同一個快取位元 ⇒ 開 A 會讓 B 的「是否已開」也讀成開（`SW[].Status()`／氣缸 `GetOutBit()`）。只放大快取、MotionNet 規則不動；⚠ 原本想在範圍檢查一律擋 1203，量表時發現輸入端真空列的 Port 是 128～135 ⇒ 改成只擋輸出（不然機台上每次讀真空都會跳框） | `LaneIORoute` [5]：修正前紅 16 條（對照組）、修正後 96／96；兩組態 gate＝基準 |

### 接下來

* ✅ **第 9 條做完**（`66c2480e`）：阻塞框在等的時候，塔燈／蜂鳴器／面板鍵燈照 golden 框自己的 Timer1Timer 動；30 秒沒有網頁就自動開 Edge 正式版畫面（之後每 2 分鐘最多一次、一個框最多 3 次）；是／否框可用面板 Alarm Reset 消音。3 個審查 agent 找到 11 條（2 條 medium：鍵燈沒閃、**HT9050 上等待期間 1203 的 DI 凍住 ⇒ 沒辦法消音**），全修；兩組態 gate＝基準。⚠ 在機台上要看的：框開著時 Retry／Skip 鍵燈閃、按 Alarm Reset 會停叫、按 Start 會啟動（這三件移植樹的 ctest 測不到）。
* 安全 PLC 五個閘 ✅（`4abcdf0b`）。下一個：第 8 條（網頁硬體鈕 `hw.access`）—— 卡在上面「新的要你決定的」第 4、5 題；St01 的分支（`3a7a252f`，S57／S86／S88～S90）Steven 說「暫時先不要 build」，等他說再代跑。

### §3 紅燈／雜訊（這一段）

* `IniFiles_Win32Diff`：兩組態 ctest 都「Process not started [operation not permitted]」，之後 exe 消失 —— 看起來是防毒／EDR 把它隔離了；重新連結後兩組態都通過（9.9 s／10.1 s）。它大量呼叫 `WritePrivateProfileStringA`，可能被當成可疑行為。之後每輪 gate 再看一次。
* `WebMotorAccess`：模擬組態 `-j 8` 下逾時一次（159 s），單獨重跑 0.82 s 通過；上一輪同樣負載是通過的。下一輪 gate 若再出現就認真查（可能是測試裡有等待牆鐘的迴圈）。

---

## 🌙 20260926（六）凌晨 03:3x～06:1x 進度

| # | 做了什麼 | commit（都在 main） |
|---|---|---|
| 1 | W3 稽核收尾：⑸ database.cpp 38 個錯誤視窗標題改回 golden 中文；⑵ else 閘註解統一＋5 支檔的過期註解；⑴⑹ mykitsuck／myTimer／MyTempPanel／myio 逐函式表＋產生工具 `tools/w3_function_tables.py`（W3_PROGRESS §15）；⑷ 早在 `88eeb91a` 做完 | `f4cddea6`、`4462beb5`、`84279edf`、`6e61686a` |
| 2 | **W2 開工**：A4-6 合一後前提失效的閘／替身（IC 料況搬移、生產紀錄、陷阱 5 類）共 4 批，另翻 SetTestRunMode＋ModifyTester 換成 golden 本體 ⇒ 表在 `docs/W2_PROGRESS.md` | `a5e0db2a`、`afc9e3c7`、`28b7d86b`、`033358a2`、`0dcc9c2c` |
| 3 | 機台端（經同步 session）：回覆週末進度與分工；記下 USB 包 `TO_LAPTOP_USB_20260926`、機台 Gerneral.ini 實際值、請筆電先別動的 3 個檔（INBOX 第 31～32 列）；**出第三份機台更新包** `D:\HT9045\backup\HT9050_update_afc9e3c7`（INBOX 第 33 列） | `bd66a068`、`ad790291` |

每一顆都過兩組態 gate（出貨 3／模擬 18＝基準）＋逐條子項比對 0 差異 —— ⚠ 也就是現有 ctest 沒走到這些路徑，W2 的行為要等清單最後的 START／PAUSE 探針與機台實跑。
**這段沒有新的待決項目。**

---

## ☀ 20260925 下午（14:5x～16:5x）進度 —— 先看這節

> 今天下午你當面給的裁決都已記在 `docs/RULINGS_20260925.md` 第 38～43 條。
>
> **✅ 已裁決 A（0926，docs/RULINGS_20260926.md 第 1 條）：HT9050 機台的出貨 exe 改用 oracle（MinGW 6.3）建，機台端照步驟評估中。** 原題如下：
>
> | | |
> |---|---|
> | 白話 | 移植樹規定唯一的「標準編譯器」是 MinGW.org GCC 6.3（32 位元、C++17）——它是唯一能重現 BCB6 x87 浮點算術的。機台現在跑的 exe 是 **WinLibs g++ 16.2（32 位元、C++14）** 建的，是一路沿用下來的處境（0825 導入時機台沒有 C:\MinGW → 只能走非 oracle 線；x64 的 wb_publish 開機前就 heap 損毀），**沒有人裁決過**。 |
> | 例子 | 同一段浮點比較，兩個編譯器結果不同：`GA1_LastSet` 的 `3.14 == 3.14` 在機台（g++ 16.2）是 false、在筆電（6.3）是 true —— 已改測試繞開，但量產碼裡的同類比較（位置、溫度、良率門檻）也可能在兩邊走不同分支。 |
> | 選項 A（建議） | 機台出貨 exe 改用 oracle（C:\MinGW 6.3，機台 0831 已複製過去）建；先確認 1203 SDK 能用 6.3 連結 |
> | 選項 B | 維持 WinLibs 16.2，接受「與 BCB6 算術可能不同」，並把 16.2 加進 gate（兩邊都要綠） |
> | 預設 | 沒回覆前不動（機台照舊用 WinLibs） |

| # | 做了什麼 | 證據／commit（都在 main） |
|---|---|---|
| 1 | **HT9050 機台更新包兩份**：第一包 `D:\backup\HT9050`（main 66cb14e0 整份＋覆蓋前檢查腳本＋bundle），機台端已合進整合樹 `95c398b`（NEW 158／OLD 72／LOCAL 30 全合完）；第二包 `D:\HT9045\backup\HT9050_update_ff4b1d8b`（只含 58 檔差異，base＝66cb14e0），等你用 USB 帶過去 | INBOX 第 20、23、26 列 |
| 2 | HT9050＝HT9046 家族：R18 的 11 處（`f45f6235`）＋只列 HT9046 的 12 處（`17a54d68`，第 39 條；cConfiguration 兩處加括號避免運算優先序錯） | 普查重量：只剩 2 處在 `#if 0` |
| 3 | InitDIOStstus（Steven 交辦）：存檔那處照 golden；開機那處先閘住（開機沒讀 DIO 檔，照翻會以相反極性打 START 線） | `0ca03ee6` |
| 4 | YES/NO 對話框照 golden 跳網頁框等回答（第 10 條）＋兩個等待迴圈補跑 Index 防掉落（審查 high） | `ee5de164` |
| 5 | Index Z 扭力照 BCB6 走 RS232（第 38 條）＋vclcompat TComm 改 SPComm 語意（獨立寫入執行緒、DCB 套 dfm 流控、讀取不空轉） | `c55e2954` |
| 6 | W5-b 教導頁運動鈕（兩輪審查 14＋17 條修正）＋怪按鈕照 2C | `a3db68f2`、`ff4b1d8b` |
| 7 | 1203 軸不看 Direction（第 13 條 6B） | `b1e4271c` |
| 8 | **建置輸出搬出原始碼樹**：`<repo>\Obj\V906\`；你那棵已預先建好 `D:\HT9045\Obj\V906\build_dbg`（F5 用）；刪樹內 build_dbg 12.8 GB／build_night 1.9 GB ⇒ **原始碼資料夾 14,828 MB → 169 MB** | `99af9f82`、`43a41b19` |
| 9 | 開機 `[BOOT]` 摘要行（IO_CARD_TYPE 不在 {2,3,4} 時印 WARNING） | `0d5cc768` |
| 10 | 你的 `D:\HT9045` 從已鎖的 feat 切到 main，每次推 main 後都快轉；本機修改 config.ini／Alarm-dialog-request.json 原樣保留 | 第 40 條 |
| 11 | 兩組態 gate：出貨 3 項、模擬 18 項失敗＝基準（新測試 InitDIOStstus／MachineSuckers_HT9050／W906_YesNoDialog／Rs232Torque／WebMotorAccess／TeachButtonsGen 全過）；system／config 587 檔跑前跑後 0 變動 | — |

| 12 | 1203 軸 Direction 照 6B 不看；TfMain 建構子其餘機台鍵（**ZSafePos 從一直是 20 改成照 ini 讀，筆電＝50**；INDEX_SUCKER_TYPE 因泵未翻先閘住）；M108 的 m_Axishand 開到 2560 | `b1e4271c`、`212c8e1d`、`afbcb6b8` |
| 13 | **20:55 兩組態完整 gate（main `212c8e1d`，建置輸出在新位置、全新 configure）**：出貨 179 項失敗 3 項、模擬 179 項失敗 18 項＝基準；ctest 寫了 machinerecord.dat 已還原 | — |
| 14 | 已查明、刻意沒做：氣缸逾時警報要照 golden 接 HAlarm，但 HAlarm 原始碼（elec\Component）不在這台筆電 ⇒ 請 NB2 提供（需求單 Q12） | INBOX 第 30 列 |

**新發現（已查明，不用你決定）**：wb_serve 開機會在 `teach.ini`、`config.ini` 的各區段之間多插空行 —— **是照 golden**（BCB6 TMemIniFile 存檔時每個區段後補空行）。
你那棵 `config/config.ini` 一直顯示「已修改」就是它（版控那份不是 TMemIniFile 寫的）；存過一次後格式就穩定。這次試跑造成的已照備份還原。

**接下來**：等舊電腦（NB2）的預勘表（需求單 Q6～Q11）→ M108 馬達陣列、TfMain 建構子機台鍵（D44 泵在前）、EP 電控比例閥、W3 剩餘機械清單；
等機台 USB 帶回 `machine_P17_P25c.patch` → P17／P25 收進 main＋golden 陣列缺陷；**最後**：評估 HT9050 的 Index Z 扭力改走 1203（第 25 列／Q11）。

---

## §0 要你決定／處理的

### 第 1 件 ✅ **已裁決並做完（20260925 11:2x：A）** —— 密碼已換，版控檔全部改讀本機環境變數 `HT9045_7Z_PW`（`f9278b0f`）。**你要做的一件事：口頭把新密碼告訴 Steven**（他打包要設同名環境變數；新密碼在本機記憶檔 `shared-drive-delivery-7z-password.md`）：共用區 7z 密碼的字串，其實散在版本庫好幾處，要怎麼處理？

> ⚠ 本段刻意**不寫出**密碼與那兩個檔名。前一版（已推上 GitLab）把它們寫出來了，違反你「密碼只記在本機」的交代 —— 20260924 夜已改掉，
> 但 git 歷史裡那一版還查得到，所以下面的建議不變。

**白話：** 放版本到共用區時，發現版本庫最上層有兩個 0 位元組的空檔，**其中一個的檔名就是共用區 7z 密碼**（另一個是同樣格式的字串，
是不是密碼我不知道）。前一版報告以為只有這兩個空檔，**再查一次發現不只**：

| 位置 | 誰加的、何時 | 性質 |
|---|---|---|
| 最上層兩個空檔 | 04-16 第一個 commit（`4402603f`） | 檔名就是那個字串 |
| `.gitignore:107-108` | 同上 | 當成目錄名忽略 |
| V912 原始碼 5 處（`Command.cpp:5333`、`HS_Function.cpp:29`、`ProductionInfo.cpp:4993/5017/5019`） | 04-20／04-28 的 AI 修改註解，09-09 隨 V912 進來 | 被當成 `//AI(…)` 的標籤名 ⇒ **公司出貨版原始碼裡就有**，V906 `Command.cpp:14906` 也照翻了一份 |
| `.claude/skills/ht9045-html-mirror-backup/SKILL.md:208` | Steven 09-22 | 7z 打包指令的 `-p` 參數（功能性使用，我沒動） |
| `docs/INBOX_QUEUE.md:590` | 我 09-23 | 明文寫「7z 密碼 = …」 —— **已改掉**（本顆） |

**舉例：** 就像保險箱密碼剛好跟某個案件編號一樣，而那個編號已經印在出貨的說明書上 —— 把桌上那張便條收掉也沒用。

| 選項 | 做什麼 | 效果 |
|---|---|---|
| A | **換掉共用區 7z 密碼**（新密碼只記本機）；那兩個空檔從版本庫刪掉 | 真的保密。原始碼裡的註解標籤不用動（它變成單純的名字） |
| B | 只刪那兩個空檔 | 字串仍在 V912 原始碼與 git 歷史裡 ⇒ 等於沒保密 |
| C | 不處理 | 維持現狀 |

這次放到共用區的內容**已經拿掉**那兩個空檔。**你要回：** A／B／C（建議 A；換了之後 Steven 那支 skill 的打包指令也要跟著改，要不要我寫信跟他說由你決定）

---

### 第 2 件 ✅ **已裁決（20260925 10:3x）：B 照 golden** —— 真機組態開機是 Operator（`RULINGS_20260925.md` §4）：「開發期權限一律最高」在**真機組態**上要怎麼落實？

**白話：** Steven 今晚的 05f2695b 讓 `config.ini` 開機真的讀進來，golden 的權限檢查（A02）跟著生效：權限等級 0（Operator）時，
存設定會被擋下（[A01_2]）。他也照 golden 讓**模擬組態**開機預設最高等級（HonPrec，golden `main.cpp:11062`）。
但 **EastSun 機台用的是真機組態，開機是 0**。所以在機台上，EastSun 要先在主畫面登入才能存設定 —— 跟你說的「開發期一律最高」不一樣。

**舉例：**

| 情境 | 模擬組態（你的筆電） | 真機組態（EastSun 機台） |
|---|---|---|
| 開機後直接改 Configuration 頁、按存檔 | 可以（開機就是最高） | **被擋**，要先登入 |
| 登入後再存 | 可以 | 可以 |

| 選項 | 做法 | 影響 |
|---|---|---|
| A | 加一個開發期開關（`MachineType.h`，例如 `W906_DEV_STAGE_MAX_LEVEL`），開著時**真機組態開機也是最高**；正式上線前關掉 | 跟你的裁決一致；一行就能收回；開關在 git 裡，每台機器一致 |
| B | 照 golden：真機開機 Operator，要存時先登入 | 不改程式；EastSun 每次開機要登入 |

**你要回：** A／B（建議 A：你已裁決開發期一律最高，而且 EastSun 正在機台上測；上線前關掉開關，並照你交代在寄全體的信裡說明）

---

### 第 3 件 ✅ **已裁決（20260925 11:2x：A 延後）**，先確認 HT9050 的 ControlPanelMode：RS232 實體操作面板（`uPadInterface`，W3 子項 4b）要不要現在翻？

**白話：** 有些機台的 Start／Pause／Reset… 實體按鍵不是接 IO，而是一塊用 RS232 串列埠通訊的面板（`ControlPanelMode=1`）。
這塊在移植樹**完全沒翻**。NB2 預勘後發現它不只是翻譯量的問題：
* golden 大約**每 1 毫秒**讀一次串列埠（面板和托盤步進馬達共用同一個埠），而 wb_serve 的輪詢是**每 500 毫秒一拍、單執行緒**；
* 就算翻完，今天的移植樹裡「掃描面板按鍵」的函式（`ScanPannelKey`）**沒有正式呼叫者**，所以實體 Start／Pause 鍵也不會真的觸發動作。

**舉例：** 像是門鈴線路接好了，但屋裡還沒裝會響的鈴 —— 接線（翻譯）做完，按了照樣沒反應。

| 選項 | 做法 | 影響 |
|---|---|---|
| A | **延後**：先確認 HT9050 用的是不是這種面板（看機台 `Gerneral.ini` 的 `ControlPanelMode`）；不是就排到 W2 之後 | 不佔現在的時間；HT9050 若是 IO 面板就完全不受影響 |
| B | 現在翻，**另開一條執行緒**每 1 ms 讀串列埠（貼近 golden） | 最忠實；但多一條執行緒要跟 500 ms 輪詢同步，是新的整合風險 |
| C | 現在翻，**掛在 500 ms 輪詢上** | 簡單；面板反應會慢很多（golden 是 1 ms） |

**你要回：** A／B／C（建議 **A**：筆電的 `ControlPanelMode=1` 是從別台機器複製來的，HT9050 實際用哪種面板要先確認；而且按鍵掃描本身還沒接上）。
NB2 的完整預勘在 `docs/nb2_assist/RD5軟體_NB2回覆Q2_uPadInterface預勘_20260924_222022.md`（它另外列了 7 件細節待你決定，選 B/C 時我再整理成表）。

---

### 第 4 件 ✅ **已裁決（20260925 09:4x，NB2 R28 轉達）：A** —— 「他的確是HT9046家族，透過machine type來分類」（`RULINGS_20260925.md` §5；寄 EastSun 那封信併入對話中的決策第 1 題）：HT9050 在程式裡要不要當成「HT9046 家族」（2×8 機台）？

**白話：** golden 沒有 HT9050 這個機種。9/7 照你的裁決加了 `Type_HT9050`（Model=`9050GPIB` 時生效，Index 8 欄）。
但 golden 裡有 **37 處**判斷是「HT9046／HT9046_LS／HT1032 這一組 2×8 機台要怎麼做」，`Type_HT9050` 一處都不在裡面，
所以 HT9050 在那 37 處走的是「HT9045 預設」的路。
NB2 今晚抓到其中最危險的一處（我今晚打開吸嘴格數與換站還原造成的）：HT9050 的吸嘴格線被設成 2×4，但它的 IO 表是 2×8，
換站時會把**空的接線蓋到有效站**（測試重現：1x2 模式下 4 顆 Index 吸嘴被清空）。**這一處我已經修了**（格數照你裁決的 8 欄；
新測試有修／沒修各跑一次，確認能抓到），其餘 36 處（LED、AutoClean、真空單元、EtherCAT、Setup 站數選項、單站換位…）要你決定。

**舉例：** 就像新車型登記成「新車種」，但保養手冊的 37 條「這一系列的車要怎樣」都沒寫到它 —— 它就一直照「預設車型」保養。

| 選項 | 做法 | 影響 |
|---|---|---|
| A | 把 `Type_HT9050` 加進那 36 處（我來做，每處註明「golden 沒有 HT9050，照 HT9046 家族」） | HT9050 走 2×8 機台的全部 golden 行為；前提是 HT9050 硬體跟 HT9046 同系列 |
| B | 機台端 Model 改回 `9046_32GPIB`（程式直接當 HT9046），不動程式 | 最快；但你 9/7 要的「獨立的 HT9050 機種」就不用了 |
| C | 維持現狀（只修了吸嘴格數） | 其他 36 處 HT9050 照 HT9045 預設 |

**你要回：** A／B／C（建議 **A**：保留你要的獨立機種，而且 HT9050 的 Index／飛梭都是 8 格，跟 HT9046 同一類）。

⚠ **另一件要你決定的：要不要寫信給 EastSun（副本 Steven）？** 我 9/24 18:50 那封信請他把 Model 改成 `9050GPIB`，
而共用區 d52a7ead 那一版正好帶著上面的格數錯誤。今天 HT9050 的吸嘴 Enable 全是 0，真空不會真的動，所以沒有實際危險；
修好的版本推上去之後我會更新共用區。建議信的內容：「共用區已更新；吸嘴 Enable 設 1 之前請先換到新版」。**要寄嗎？**（寄前我會把全文給你看）

---

### 第 5 件 ✅ **已裁決（20260925 11:2x：B 照 EastSun，不看 Direction，HT9050 表改 0）** —— 待做：排在 W5-b 合進 main 之後（同一支 WebMotorAccess.cpp），且 machines/HT9050/Mot_Table.csv 要等機台端傳來修正後的表一起改：1203 馬達的「方向」（Mot_Table 的 Direction 欄）要怎麼算？

**白話：** 馬達表每一軸有一欄 Direction（0／1）。golden 給 1203 卡寫的馬達程式（`Motor/myEthercatmotor.cpp`）對它的處理**前後不一致**：
讀位置、軟體極限、寸動（jog）、相對移動都會依 Direction **反號**，但「移到絕對位置」（自動流程和馬達測試頁的 GO 都走這條）**不反號**。
Direction=0 的軸兩邊一致、沒有問題；Direction=1 的軸會「命令它去 +1000，讀回來顯示 −1000」。
HT9050 的 19 個 1203 軸有 **11 個是 Direction=1**（InArm X／Y、OutArm X、Index Z1、OutShuttle2、Loader／Empty／Auto1～3 的 Z、CCD Y）。
EastSun 的做法完全不看這一欄 —— 方向交給伺服驅動器自己的參數（Pn000）。
NB2 R20 獨立量過屬實，並補一個佐證：golden 的 `RealG00` 其實**有**反號，但它唯一的入口 `G00` 全樹 0 個呼叫者；`Motor/mymotor.cpp:632-633` 還留著被註解掉的上層反號 ⇒ 這個不一致是歷史改動留下來的，不是設計。

**舉例：** 就像一把尺，看刻度的時候倒過來看，畫線的時候卻正著畫 —— 只要尺是倒放的（Direction=1），畫出來的位置就跟讀到的相反。

| 選項 | 做法 | 影響 |
|---|---|---|
| A | 1203 軸**一律照 Direction 反號**（讀、寫、寸動、極限全部一致；補上 golden 漏掉的那一處） | 保留馬達表這一欄的意義；教導點照舊。偏離 golden 一處（補反號） |
| B | 1203 軸**不看 Direction**（照 EastSun：方向交給驅動器 Pn000），並把 HT9050 馬達表的 Direction 欄全改 0 | 跟 EastSun 實機驗證過的一致、golden 各路徑也自然一致；要動機台的馬達表（`machines/HT9050/Mot_Table.csv`，EastSun 那邊也要同步改） |
| C | 完全照 golden（包括那個不一致） | Direction=1 的軸用絕對移動會跑錯方向，不建議 |

**你要回：** A／B／C（建議 **B**：你定的通則是「方向／極限極性跟 EastSun」，而 EastSun 在機台上量過的就是「方向歸驅動器」）。
**在你回覆之前我的做法**：W4-b 的運動命令只開放 **Direction=0 的 8 軸**；Direction=1 的軸按了會回「方向慣例待決定（夜間報告 §0 第 5 件）」，不會動。

---

### 第 6 件 ✅ **已裁決（20260925 11:2x：A 不寫，維持現行）**：教導頁存檔時，二進位的 `system\tech.dat` 要不要照寫？

> ⚠ 前一版這裡說「差在兩個編譯器的對齊」是**錯的**。NB2 R24 用新工具（`tools/nb2_assist/struct_layout_across_trees.py`）量了四棵樹，我重跑過一樣：
> 差別來自**版本**，不是對齊。

**白話：** golden 教導頁按「存檔」會做兩件事：把每個教導點寫進 `teach.ini`（文字檔，**這是現在真正在用的**），
再把整個教導結構原封不動寫進 `tech.dat`（二進位檔，舊格式）。量產 exe 只有在 `teach.ini` 缺了 `[Teach INI] Update2` 這個鍵時才會回頭讀 `tech.dat`
（例如 `teach.ini` 被刪、或舊機升級第一次開機）。問題是這個結構**每一版長得不一樣**：

| 版本 | 大小 | 跟移植樹（= golden 906）的關係 |
|---|---|---|
| V899（這台的 `tech.dat` 就是它寫的） | 3792 bytes | 移植樹 = V899 ＋ 結尾多 20 個欄位 ⇒ V899 讀移植樹寫的檔**讀得對** |
| golden 906／移植樹 | 3872 bytes | — |
| V912（量產維護版） | 3872 bytes，**但把 2 個欄位（旋轉背隙）從中間搬到結尾** | 大小一樣、第 161 個欄位起全部錯開 ⇒ V912 讀移植樹寫的檔會**錯位** |

**舉例：** 就像同一份表格改版時在中間拿掉一欄、加到最後 —— 總欄數沒變，但照舊表格的位置抄，從那一欄之後全部抄錯。

| 選項 | 做法 | 影響 |
|---|---|---|
| A | **不寫 `tech.dat`**（現在的做法） | `teach.ini` 照常同步，三個版本都以它為準；`tech.dat` 維持原狀。偏離 golden 一處（少寫一個舊格式檔） |
| B | 照 golden 906 寫 | 跟 golden 一致；V899 讀得對，**V912 走遷移路徑時會錯位**（而 V912 是量產版） |
| C | 依機台上要共用的量產 exe 版本寫對應版面 | 誰讀都對；但要多寫版面轉換，而且要知道每台機器配的是哪一版 exe |

**你要回：** A／B／C（建議 **A**：`teach.ini` 是三個版本共同的真實來源，`tech.dat` 只在遷移時才被讀，而那時寫哪一種版面都有一邊會錯）。
**在你回覆之前的做法**：A —— ⚠ 前一版的程式是「檔案大小相同才寫」，NB2 R24 指出它**兩邊都錯**（這台 V899 的 3792 其實安全卻不寫；V912 的 3872 不安全卻會寫），
已改成一律不寫。存檔結果會附一行說明讓操作員看得到。
另外 NB2 R24 附帶的一件（待你、跟 V912 量產有關）：**V899 → V912 升級**時若走遷移路徑，V912 會把 V899 寫的 `tech.dat` 錯位讀進來、再永久寫進 `teach.ini` —— 那是 V912 自己的問題，跟 V906 無關，列在這裡讓你知道。

---

### 第 7 件（新，20260925 13:0x）：golden 有兩個氣缸常數撞號，要不要修？（V912 量產版也有）

**白話：** golden `cmydef.cpp:442` 的 `C_StackedTrayLockOff`（wei 2018 MR）與 `:444` 的 `C_LoadRobotX`（Sam 2019 LM）**都是 75**。
`InitialCylinderName` 先把 75 號叫做 StackedTrayLockOff、下一行又改叫 LoadRobotX ⇒ 75 號最後綁的是 LoadRobotX 的 IO；
**程式裡所有用 `Cylinder[C_StackedTrayLockOff]` 的地方，實際推的是 LoadRobotX 那顆氣缸**。906、V912、移植樹三份都一樣（移植樹照翻）。
今天補氣缸測試時量出來的：259 個名稱只綁上 258 個槽（`tests/test_machine_cylinders.cpp` 已釘 258，並寫明原因）。

**舉例：** 就像兩個房間門牌都寫 75 號，郵差只會把信送到後掛牌的那間；前一間的人永遠收不到，還會誤收別人的信。

| 選項 | 做法 | 影響 |
|---|---|---|
| A | 維持照翻（不修） | 只有「同時裝了 MR 疊盤鎖與 LM 取料機器人」的機台會出事；沒同時裝就沒差 |
| B | 移植樹給其中一個新槽號（例如接在最後一個之後），註明「golden 缺陷，使用者裁決修正」 | 移植樹正確；V912 量產版維持原樣 |
| C | 移植樹與 V912 都修 | 量產版也正確；V912 要另外走出貨流程 |

**你要回：** A／B／C（建議 **先 A，並請機台端／客服查「哪些機台同時有這兩種機構」**：沒有就不急；有的話選 C，因為量產機台才是真的會出事的地方）。

---

前一版列的兩件，你 20260924 晚已回覆（紀錄）：

| # | 事項 | 你的裁決（原話） | 我做了什麼 |
|---|---|---|---|
| 1 | 要不要問 EastSun 機台設定值 | 「不用寄信，馬達要動，要開卡，我這台是模擬環境，你之前寄信有告知這部分設定對嗎?沒有的話要補寄信告知哪些參數要設定才能正常運作，CC steven」 | 查了 17:12 那封：馬達段只寫「先不要測」，**沒講參數** ⇒ **18:50:07 已補寄**（收件 EastSun、副本 Steven）。內容見 §1-7 |
| 2 | 要不要讓網頁檢查權限等級 | 「現階段是開發階段，人員權限一律都是最高，未來要寄給每個人時候，要特別說明我的這個想法」 | 不發 `security.accessLevel`（網頁維持放行）；已記進週末計畫通則與記憶，之後寫給全體的信會說明 |

---

## §1 做完了

1. **W0 馬達（分析完）** —— ⚠ 20260924 晚更正結論：不是「卡在機台設定」。`INSTALL_ETHETCAT()` 是 Sam 2023 為 HT9045 的
   EtherCAT Shuttle 感測／真空模組寫的條件，不涵蓋「馬達全走 1203」的 HT9050；叫機台改 `SHUTTLE_SENSOR_TYPE` 會改掉 Shuttle 感測邏輯。
   ⇒ 開卡是**程式缺口**，歸 W4 照 EastSun 的做法補。下面是原本的分析：
   * 非模擬建置 MainProc 被 sim canary 擋下：保護的是 Contec 入料 Shuttle latch 互鎖，而它依賴的 `TfLtcSensor` 仍是離線樁（讀恆 0）、
     golden `LtcSensor.cpp` 只有 SMC／SYNTEK 分支沒有 1203 ⇒ canary **目前有正當理由**；HT9050 較合理是機台 `MOTION_CARD_TYPE` 設非 Contec（golden 自己跳過）。
   * 1203 開卡路徑**照翻而且活著**（`InitHontechHardware` → `OpenPCI132Card` → `INSTALL_ETHETCAT()` → `OpenEtherCatMastCard`），
     但 `INSTALL_ETHETCAT()` 只在 `SHUTTLE_SENSOR_TYPE==6/7` 或 `VacuUnitType==1` 為真；筆電與 repo 種子都不是 ⇒ 不開卡 ⇒ 每一軸靜默不開。
   * 兩個 shim 疑慮（`DoInShZHome`、`SetShuttlefCanMoveL`）查證為**誤警報**（都已 `#if 0 RETIRED`）。
   * commit：eaea873、5723535（寫進週末計畫 W0）
2. **W1 C++↔Web 通道普查**：新工具 `tools/webprobe/channel_census.py`（附 selftest），報告 `docs/CHANNEL_CENSUS_20260924.md`。
   重點：C++ 發 7,082 個 tag，頁面在讀但值為 null 的 41 個（扣掉沒卡的 `pci1203.*`）；`Motor-runtime.json`／`Production-runtime.json`／
   `Setup-current.json` 頁面在讀、C++ 沒寫（快照）；`prod.*` 16 個沒有任何頁面讀。
3. **塔燈接上（W1 的第一個修正）**：`tower.red/amber/green` 原本恆 null，理由「kernel tick 在 wb_serve 不跑」已過期。
   接上後實測：START 後綠、PAUSE 後黃、歸零時閃爍，與 gdb 讀 `fMain->ledX->Value` 一致。
   出貨組態閘門 165 項 4 失敗，與基準逐項相同。commit fea72460。
4. **W3 第 1 步**：`tools/w3_caller_census.py`（nm 盤點）→ `docs/W3_CALLER_CENSUS_20260924.md`。
   `mykitsuck.cpp` 刻意沒編（ODR），`myio` 16 個函式 0 個被引用，`TMySensor::IsOff/IsOn` 分別被 55／46 個檔引用。
5. 回 Steven 的信已寄（副本 EastSun，17:48:57）；5 張截圖進版控（16f122a）。
6. **馬達測試頁改讀 C++（W1 第二個修正）**：原本讀 09-02 的舊快照 `Motor-runtime.json`（86 軸、全 unknown）。
   | 項目 | 內容 |
   |---|---|
   | 新 API | `/api/struct/motor/config`（C++ 實際載入的馬達表）、`/api/struct/motor/runtime`（每軸的 `MOT[]` 位置／歸零旗標） |
   | 只讀快取欄位 | 位置、編碼器、目標、歸零旗標；**不打驅動卡**（網頁每秒輪詢，不能讓 HTTP 回應去碰卡）。速度、警報、servo 沒有快取 ⇒ 送 null |
   | 沒有驅動物件的軸 | 送 null＋`nosource`，不送 0（0 是合法座標） |
   | 頁面 | 有伺服器 → 先讀 C++；讀不到或直接開檔案 → 退回原本的 JSON 檔（兩條都實測過） |
   | 測試 | 新 ctest `MotorPoints_HT9050` 37/37（讀版控的 `machines/HT9050/Mot_Table.csv`） |
   | 實測 | 筆電 wb_serve：45 軸全部讀到、模擬建置下 45 軸都有驅動物件；無頭 Edge 開頁面確認來源是 C++ |
   | 閘門／commit | 出貨組態 166 項 4 失敗（與基準逐項相同）；05a4d574，已推 |
7. **補寄給 EastSun（副本 Steven）的馬達參數信**（18:50:07）：
   | 段落 | 內容 |
   |---|---|
   | 結論 | 他的 `/pci1203.html` 照舊可用；HW.MotorTest 與 C++ 流程這一版還不能驅動 1203 馬達 —— 頁面的 `motor-access` 命令 C++ 沒有人接（`git grep` .cpp／.h 0 處），開卡條件也還沒照 HT9050 改。原因在程式，不在參數 |
   | 要設對的 | 兩張表與兩個 `Pci1203*.ini` 用 `machines/HT9050`；`IO_CARD_TYPE=2`；`MOTION_CARD_TYPE=0`（設 1 會觸發 latch 互鎖）；GPIB `Model=9050GPIB` |
   | 不要改的 | `SHUTTLE_SENSOR_TYPE`、`VacuUnitType`（改了只為開卡會動到 Shuttle 感測／真空模組） |
   | 給 Steven | HW.MotorTest 已讀 C++（05a4d574）；這一頁在他的來源裡也有，`sync_web.py --apply` 會蓋掉，請他帶進原始檔 |
8. **09-09 的 1203 參數文件勘誤**（`docs/HT9050_1203_BRINGUP_PARAMETERS.md`）：它寫 `IO_CARD_TYPE=4` 是錯的 —— 綁 IO 表只認 2／3，設 4 一個點都綁不上。
   週末計畫插入 W4（MotorTest）、W5（Teach）與你今晚的五條通則。e335c78f，已推。

9. **版本放到共用區給 EastSun**（使用者：「Eastsun沒辦法上git抓資料到機台端，只能透過內網分享」）：
   `\\192.168.190.53\rd5\Jimmychiu\HT9050\HT9045`，內容 = `git archive` 匯出的 **8484bdb4**（跟 clone 拿到的一樣）。
   | 項目 | 內容 |
   |---|---|
   | 排除 | `config\`、`CFG\`、`SECS\`、`CurrentSetupData.txt`（機台專屬，照蓋會洗掉他那台的設定）；以及上面 §0 第 1 件說的兩個空檔 |
   | 核對 | 5,768 個檔（含說明檔 `README_先讀我.txt`）；抽查 4 個檔的 MD5 與來源相同；IO 表 MD5 `264b93ec` 與信裡一致 |
   | 說明檔 | 先備份 → `robocopy 共用區 D:\HT9045 /E`（不會動 config／CFG／system）→ 換 HT9050 設定檔 → build.bat serve → 測 IO |
   | 更新紀錄 | 0de428ac（20:2x）→ **c6528dfe（22:00，A4-6＋吸嘴初始化）**：robocopy /E 5,809 檔 0 失敗（兩版之間沒有刪除檔）；抽查 4 檔 MD5 相同；說明檔加第九節（⚠ HT9050 吸嘴 Enable 全 0，要測真空得在機台表設 1）。之後照同樣方式放 |
10. **W3 進度**（`docs/W3_PROGRESS.md`，每支檔一張表）：
    | 項 | 內容 | commit |
    |---|---|---|
    | 1 MyLaneIo | **A4-7 依點位分派**三路後端（1203／MN200／舊卡），`InitHontechHardware` 開頭換成真後端；新 ctest `LaneIORoute` 35 項 | 8484bdb4 |
    | 2～4 myio／mysensor／myswitch | 舊式路徑的檔內樁改接 myio 真函式；面板閘理由更正為「RS232 實體面板未翻」 | 4d94ae38 |
    | 5 IO 表初始化 | `InitialSwitch`／`InitialSensor` 逐列逐欄比對：Switch 132 列、Sensor 289 列 **0 不符**；else（BDE）閘改寫為你的裁決；`SetIOTableByNUEC1` 量過在非 NU-EC1 機台是 no-op | 3b4f08a1 |
    | 6 mycylin | 18 個被引用函式都已開；計數閘等 vclcompat 修正（6b）、警報等 HAlarm（W2） | 3b4f08a1 |
    | 7 氣缸初始化 | `InitCylinder` 逐列逐欄：258 個氣缸，輸出 72／On 感測 54／Off 感測 54 組 **0 不符**；新 ctest `MachineCylinders_HT9050` 521 項 | 本顆 |
    | 8 A4-6 | **兩套吸嘴類別合一**：186 支檔原本用缺 IO 接線的精簡版 `TMySucker`（15 vs golden 71 欄位、方法不碰 IO）；改成全部用 golden 佈局的 `mykitsuck.h/.cpp`。連帶消掉 NB2 量到的活 ODR（`cOffSet.cpp:63` 讀超出物件尾端約 19.8 KB）。重複定義 5 個退役（nm 掃過只剩這些）。出貨組態閘門同基準；模擬組態對照 W3 之前的基準 **0 個新失敗** | 8ff6c754 |
    | 9 吸嘴初始化 | `InitSucker` 的 if 半邊（IO_Table.csv）＋`InitialSuckerName` 7 個閘＋備份端解閘；else（BDE）照你的裁決閘住。新 ctest `MachineSuckers_HT9050`：49 顆吸嘴 × 三組 7 欄 **0 不符**＋手算值，出貨 193/193、模擬 191/191。⚠ HT9050 IO 表 49 列吸嘴 `Enable` 全是 0 ⇒ 機台上吸嘴不會動，要在機台表設 1 | 2726306d |
    | 9b／9c 吸嘴格數＋還原端 | NB2 抓到我漏的：所有吸嘴格線都停在 1×1（49 個 `SetItemAmount` 全在一個閘裡）。照 golden 打開格數（n4-1），再開 ChangeSite 的 47 個還原閘與臂的 COPYBACKUP ⇒ 選了哪幾站，真空接線才會照 golden 換位。兩組態 0 個新失敗；吸嘴測試加驗整張格線（201/201） | 8445ed2f、c22dcb12 |
    | 10 馬達初始化 | 本來就活著，補第三級測試 `MachineMotors_HT9050`：48 軸驅動類型／16 欄位／位址 **0 不符**，出貨 86/86、模擬 83/83。⚠ 量到 HT9050 要 `INDEX_MOTION_CARD=1`（筆電是 0；0 會把 Index 四軸改成 Galil、跳過表），W4 交付信一起講 | 39f3a04d |
    | RJ-03 開機順序 | NB2 抓到：開機與存配方後重讀，QAMode 在 TrayAssignment 之前（golden 相反）⇒ 缺鍵時把錯的預設值寫進真實 `Tester.Data`。照 golden 對調。筆電 64 份配方都已有該鍵，沒有被寫壞的檔 | 93042c72 |
    | 11 mytray／myTimer／MyTempPanel | `mytray` 整支照 golden 重翻：原本 11 個樁（`HowManyIC` 等）＋6 處偏離，其中 `ClearData` 會把開機配置好的生產紀錄指標清成 NULL、`HasIC` 不認清潔片／OCR／卡匣狀態。新測試 `MyTray_Golden` 32/32（兩組態）。`myTimer` 對 golden 0 差異；`MyTempPanel` 剩下的閘全是畫面版面／滑鼠事件，歸網頁 | 2c12c408 |
    | 6b StringGrid 語意 | vclcompat 的 `TStringGrid` 改成 BCB6 語意（越界讀回 ""、越界寫存起來、縮小不刪資料），NB2 逐行讀過 grids.pas。mycylin 的氣缸計數 4 個閘因此解開；7 處寫錯「BCB6 會丟 ERangeError」的註解更正；`cObserver` 回到 golden 的 10 欄 | db8cb324、feacf1a8 |
    | 12 cinitial 其餘 | 21 個被別的檔引用的函式逐一列；A4-6 後理由失效的 8 個閘解開（吸嘴重試次數、破壞延時與重吹、CheckKitSuck、SetGaliRate）；其餘閘都有真理由（UI 歸網頁／BDE 裁決／真的缺相依） | bda2d6fc、65bc0e58 |
    | 新增 4b | `uPadInterface`：RS232 實體操作面板（Start／Pause／Reset… 17 鍵）**沒翻**；筆電 `ControlPanelMode=1`，這種機台上移植樹的實體面板鍵兩邊都讀不到 | 待做 |
    | 新增 6b | vclcompat `TStringGrid` 越界讀丟例外，BCB6 是回空字串；SECS 有程式依賴目前行為 | 待做 |

11. **Steven 05f2695b（C 路表單橋＋主畫面登入）的穿插處理**（使用者：「看完後，判斷如何在任務中穿插並執行之」）：
    | 他的內容 | 跟我們的交集 | 處理 |
    |---|---|---|
    | C 路：頁面存讀走 golden FormShow／FormClose，存完重新 Load | W5 Teach「存讀＋跟 C++ 變數同步」 | **W5 定案採用 C 路**，已寫進週末計畫 W5 |
    | 檔案擁有者閘（config.ini／LastSet.ini／configByRecipe.ini／HotPlate.Data 歸 C 路，B 路寫回 409） | W4／W5 寫檔的路徑 | 寫進 W4（馬達表不在清單，照舊）、W5（Teach 的檔接 C 路時要加進清單） |
    | 開機補 `ReadTasterInfo`、`FileRW/TestIF_File.cpp` | W1 的 `tester.name` 空值 | 重驗：**前提仍成立**（他自己的 `TestIF_File.cpp:525` 寫明 `ReadTestIFFile` 仍是 GATE F-5），維持 null |
    | config.ini 真的讀進來、golden A02 生效；模擬組態開機 HonPrec、真機組態開機 0 | 你「開發期一律最高」的裁決 | §0 第 2 件請你選 |
    | 補上漏掉的 `InitialSuperVisorPassword`（原本空密碼會被判成 HonPrec） | 安全 | 他已修好 |
    | ⚠ **他這顆讓全量建置壞了兩處**（GitLab 上 05f2695b 本身就編不過測試）：`cprod.cpp` 新引用 `elConfig`／`cbLastSet`／`HTEditList_*`／`FileRW_*`，但 `test_ga1_cprod` 的精簡連結組合沒補；而且 `FileRW/*.cpp` 只編進 wb_serve 執行檔，其他連 `ht9045_globals` 的 6 支測試缺 `FileRW_ProxyChecked`／`FileRW_IniConfig_ChangeCBListProperty` | 我的合併要推，樹得是綠的 | 修：`test_ga1_cprod` 補替身（三個清單指標 NULL，cprod 每處都有 `!=NULL` 保護）；新增 `FileRW/_fallback.cpp` 放進 `ht9045_globals`，只定義那兩個函式（= FileRW 沒啟動時的行為）。**nm 確認 wb_serve 連的仍是 Steven 的真函式**（92／25 位元組 vs 後備 10 位元組／空函式）。長久正解是把 FileRW 搬進 archive，要跟 Steven 對齊 |

12. **NB2（舊筆電）協助的採用紀錄**（你說「有幫助就用，沒幫助就跑自己的」；每一條都先在樹上重驗）：
    | NB2 輪次 | 發現 | 我怎麼處理 |
    |---|---|---|
    | R1 §A | `cOffSet.cpp:63` 活 ODR（讀超出物件尾端約 19.8 KB） | A4-6 合一後消失，不用改 JerryYang 那行；R3a 用三層獨立驗證確認 |
    | R1 §5a | 第 5 個重複定義 `InArmPlaceSuck` | 退役（nm 確認同一 archive、目前只是運氣沒撞） |
    | R3／Q5 | 吸嘴格線全停在 1×1；還原端要先開 n4-1 | 照做（9b→9c），兩組態 0 個新失敗 |
    | R3 RJ-03 | 開機讀檔順序錯，寫錯值進 `Tester.Data` | 照 golden 對調（93042c72） |
    | R3 RB-3 | COPYBACKUP 閘理由失效、W3-9 漏開 | 併入 9c |
    | R2 Q4 | M108 的 MotorID=1080 超過 `m_Axishand[999]`（golden 相同，有卡時寫出陣列尾端） | **排進 W4 開卡第一步**（要先查所有用到它的迴圈才知道怎麼修對） |
    | R2 Q3 | S7F4 那段打開會即時刪除其他配方（`ClearAllSetupFile`），不是延後生效 | 維持閘住，W2 時改寫理由 |
    | R2 Q1 | StringGrid 改成 BCB6 語意，只有 `uHGemEquipment` 測試會紅 | W3-6b 照這份做 |
    | R4～R7 | 閘理由重驗器、試開閘編譯工具、主流程 52 閘開閘清單、`MoveSuckData` 在三條測試流程被 no-op 替身吃掉 | W2 的起點（照你的順序排在 W4／W5 之後）；前後臂 destroy 那兩處要人在機台旁驗 |
    | R1.5 | NB2 那台建出來的 wb_serve 是武裝的（裝了 1203 SDK） | 記錄；NB2 不跑 wb_serve，沒有機台接在上面 |

13. **W4-a 馬達測試頁的按鈕真的送進 C++**（3878bcc9；表與落差在 `docs/W4_PROGRESS.md`）：
    原本線上模式**根本沒送**（只寫瀏覽器的 localStorage），STOP 等非運動按鈕立刻顯示假成功，寸動放開什麼都不送。
    現在：按鈕 → WS `motor.access` → C++ 依 golden 按鈕邏輯分派 → 1203 軸用 EastSun 的控制層、其他軸用 golden 馬達物件。
    這一步接上 **STOP（停全部＋每個 1203 軸各送一次）、寸動放開（只停那一軸）、伺服開關**；其餘 23 個按鈕按了會說「還沒接上、哪一波接」，不再假成功。
    STOP 另開一條 `motor.stop`，**不需要操作權**（別的分頁拿著權限時也停得了）。測試 57 條＋三個故意改壞的版本都抓得到；兩組態 0 新失敗。
14. **W3-12 的一句話說錯了，已更正（W3-12c）**：NB2 覆核指出，我 W3-12 說「開機時吸嘴的破壞延時、重吹次數／間隔現在會照參數設定」，
    但那段程式所在的函式**根本沒人呼叫**（呼叫它的那一行還被另一道「全樹找不到」的過期閘擋著）。我用 nm＋反組譯重量屬實。
    已把 6 道過期閘打開（`UpdateMyKitSuckDelayTimeToProd`／`SetHangupMaxTime`／`InitialMachine`／`SaveMachineRecord`×3），AutoClean 裡遮住真本體的空殼拆掉，
    並加測試鎖住那些值。⚠ 開機會照 golden 寫 `system\machinerecord.dat`，真機驗證前要先備份。**共用區 35881445 那一版的說明第十一節第 4 點是錯的**，下次更新共用區時一起更正。

15. **W4-b1 馬達測試頁的運動按鈕接上**（寸動、相對移動、GO、到軟體極限、6 顆參數鈕）：
    每顆都照 golden 按鈕的檢查順序（位置溢位、安全門、急停、未歸零、馬達被鎖、軟體極限、軸還在動）先擋，過了才送 EastSun 的 1203 命令；
    速度照 golden 的「百分比 → 卡片速度」公式換算；寸動照 EastSun 用「連續移動＋放開停止」（原本 golden 的寸動指令在這張卡不會動）。
    ⚠ **1203 軸只開放方向欄＝0 的 8 軸**，另外 11 軸等你回 §0 第 5 件。golden 的「到正向軟體極限」鈕在 golden 裡本來就一定被拒（它用 >= 比），照翻。
    測試 99 條，三個故意改壞的版本都抓得到。

16. **W4-b2 歸零、來回測試、MNet 重置接上**：
    歸零照 EastSun 在機台上驗過的一鍵歸零（DS402 124／128，依馬達表的歸零方向欄選），**要看到「歸零中 → 完成」才把歸零旗標設成完成**；
    沒進歸零狀態 5 秒、出錯、逾時都設成「歸零失敗」，不會假裝歸零好了。來回測試（LoopMove）照 golden 兩段＋等待＋計數，再按一次或 STOP 停。
    **馬達電源鈕、重新載入馬達資料鈕不做**，按了會說原因：前者 golden 開電要的三個剎車釋放函式移植樹沒有；後者會把全部座標歸零，EastSun 刻意不提供。

17. **NB2 覆核 W4-b1 抓到的六條，全部補上**（在有人到 HT9050 旁邊試運動之前）：馬達表 Enable=0 的軸在真機上一律按不動（golden 本來就選不到）；
    入料飛梭移動前照 golden 先開閘門、兩顆感測器到位才動；卡片回錯誤碼不再當成功；**寸動時操作員的網頁連線斷了就自動停**；
    上一個命令之後要等監看器更新一次才接受下一個運動命令；Index 軸的速度鈕照 golden 寫目前速度。另外端對端探針抓到「成功回應頁面收不到」的 bug，已修（§6）。

18. **共用區更新到 02f22d05**（給 EastSun；06:28 robocopy 5857 檔、說明檔 `README_先讀我.txt` 已改）：第六節安全說明改成「馬達測試頁可以驅動 1203，機台旁一定要有人」，
    新增第十二節（W4 全部、只開放 Direction=0 的 8 軸、兩顆不做的原因、第一次試的順序），並**更正第十一節第 4 點**（重吹參數那一版其實沒生效）。沒有寄信。

19. **W5-a 教導頁可以存讀參數，而且跟 C++ 變數同步**（照你定的「Steven 的 C 路」，跟 Configuration 頁同一條路）：
    開頁 = golden 開頁讀檔；存檔 = golden「存檔」鈕的整段流程（確認框 → 畫面值寫回變數 → 寫 `teach.ini` → **重讀回變數** → 重新初始化飛梭參數）。
    頁面重開看到的值是從 C++ 變數來的，所以「存了沒進變數」這種情況不會發生。頁面原本直接讀檔的 387 個欄位全部涵蓋，另外多補 ~140 個檔案裡還沒有的欄位。
    端對端實測（模擬建置的 wb_serve＋真的 WebSocket＋真的 `teach.ini`，先備份、驗完還原並逐檔比對、刪備份）**19 項全過**：
    沒開頁就存會被拒；189 個教導點＋139 個 elTeach 值跟檔案一致；改一個值存檔 → 檔案變、重開頁看到新值；答「否」→ 檔案不變、畫面回原值；
    舊的直接寫檔方式對 `teach.ini` 回 409。⚠ 兩件照 golden 的副作用（跟量產 exe 現在的行為相同，不是新的）：
    `teach.ini` 每節後面會多一個空行（BCB6 記憶體 ini 的存檔格式，值一個都沒變）；非 Latch 機台按存檔會把 `Gerneral.ini [Shuttle] iInShtZRange` 寫成 0（golden 的潛在 bug，見 §2）。
    `tech.dat` 先不寫，等你回 §0 第 6 件。詳見 `docs/W5_PROGRESS.md`。

20. **W4-d：NB2 覆核抓到的「機台旁試 HOME／LoopMove 之前要補的」全部補上**（R22 一條＋R23 六條高＋四條中低，逐條先對 golden 才動）：
    * Mot_Table 裡 Enable=0 的 1203 軸在**任何建置**都按不動（原本模擬建置還按得動，而機台用的正是模擬＋實彈的預設建置）；
      順帶修掉我自己 W4-b1 的錯：模擬建置裡**每一個** 1203 的 GO／相對移動／來回原本都被誤擋成「Enable=0」（§6）；
    * 歸零照 golden：**門開著不歸零**、**先設歸零速度**（原本會拿剛才寸動的速度去找原點）、一按就把歸零旗標清成 0；
    * Z 軸歸零後去安全高度那一步照 golden 的互鎖（門開、被鎖都不會動）；
    * 來回測試／歸零在「操作員網頁斷線、安全鎖、跳告警」時停止推進；跳告警時對所有 1203 軸補送停止（golden 的全停走不到 EastSun 開的軸）；
    * 頁面按鈕改成「明講要開還是要停」，伺服端照做並回報狀態 —— 修掉「想停卻變成重新開始」；
    * 歸零／來回中，其他寸動、移動鈕照 golden 擋住；換軸、按伺服鈕會取消；
    * 跳告警框時網頁的 STOP 仍然有效；即時位置的資料改成執行緒安全（原本 HTTP 執行緒直接讀監看器，可能讀到寫一半的資料）。
    測試 183 條，七個故意改壞的版本各被抓到。詳見 `docs/W4_PROGRESS.md` §10。另外 NB2 查到的：馬達電源鈕原本寫的拒絕理由是錯的（見 §2）。

21. **整合 Steven 今早的 `v906/steven-cbridge-review6`（39e41cf，C 路第二批）並合到 main**（你 09-25 08:0x 指示）：
    Steven 信（07:34，收件研五軟體）說 feat/v912-port 09:00 鎖定、之後走 main。合併本身沒有衝突，但抓到一個**合併造成的建置破口**：
    他把 wb_serve 的 FileRW 原始檔改成產生的清單，我 W5-a 手寫加的 `FileRW/Teach.cpp` 因此掉出建置（`wb_serve.cpp` 仍呼叫它 ⇒ 會連結失敗）；
    已在他的產生器加「手寫入口」支援＋`_integrated.txt` 加 Teach 補回（ca6094c9；他的產生器在這台跑不起來，清單照結果手動補一行）。
    驗證：兩種組態全量建置＋ctest（出貨 4 失敗＝基準、模擬 19 失敗＝前一次集合）；端對端：教導頁 19/19、馬達頁 10/10；
    Steven 的頁面探針（無頭 Edge，唯讀模式）：他的 Ld_ULd 頁全過，**教導頁在真瀏覽器裡 536 筆清單值與畫面一致**。
    ⚠ 過程中發現 wb_serve 預設送的是 `D:\HT9045\web`（主工作區的舊頁面）—— 第一次瀏覽器探針因此失敗，改帶 `--root <工作樹>\web` 才是測這一版的頁面。
    軟體群的信已於 **09-25 08:58:43 寄出**（你看過全文後說「寄出」；收件研五軟體）。

## §2 刻意沒做的

* 翻 golden `LtcSensor`：V906 golden 沒有 1203 分支，那是新開發不是翻譯 —— 等 EastSun 回覆再決定。
* 改 `MOTION_CARD_TYPE`／`SHUTTLE_SENSOR_TYPE`：那是機台執行期設定，而且要 HT9050 的實際值。
* **馬達測試頁「命令位置」欄位把 null 顯示成 0**（`web/page/HW.MotorTest.html` `updateLiveFields`）：這個欄位會被相對移動、SetPos
  命令當成「現在位置」讀（同檔 `numOf('edtCommandPos',0)` 共 5 處）。改成顯示「---」會讓那些命令拿到預設 0，不改又會把「不知道」顯示成 0。
  兩邊都碰到移動命令 ⇒ 安全關鍵，佇列。模擬建置下每軸都有驅動物件所以不會出現 null；真機上沒開卡的軸才會。
* 馬達頁「資料庫」分頁的存檔：**不用動**。同事的接線引擎（`page/ht9045_wire_hwmotortest.js` 表格模式）在捕獲階段就攔下存檔鈕
  （`ht9045_wire_engine.js:1611-1619`），存檔只把改過的格子經 `/api/system/motTable` 寫回真的 `Mot_Table.csv`；頁面舊的
  `dbSave`（寫 `Motor-config.json`）已經跑不到。見 §6 第 4 條。
* ⚠ `HW.MotorTest.html`（跟 `HW.IoSetView.html` 一樣）在網頁同事的鏡像來源裡也有，`sync_web.py --apply` 會整檔蓋掉這次的改動。
  要長久保留，得請網頁同事把這段帶進他的原始檔。

* **樹外寫檔要補圍堵接縫**：✅ 已補（92cc8002，`W906_UNLOADERINFO_ROOT`，cinitial 三處＋tests/CMakeLists 四組圍堵字串）。量過目前沒有 ctest 走到這條路 ⇒ 預防性；`D:\UnloaderInfo` 那個 09-24 的檔是 wb_serve 開機照 golden 寫的。
* **M108 的 MotorID=1080 超過 `m_Axishand[999]`**（NB2 R2 Q4，golden 相同）：有卡的機台開軸時會寫出陣列尾端。W4 開卡第一步處理（要先查所有用到它的迴圈）。
* **教導頁存檔把 `iInShtZRange` 寫成 0**（W5-a 端對端量到，golden `uteach.cpp:2280` 相同）：非 Latch 機台上那格是空的，golden 照樣 `atoi("")` 寫回 0；
  量產 exe 在同一台按存檔也一樣。日後改成 Latch 機台時範圍會是 0 不是預設 250。改法（只在 Latch 時才寫／空字串時保留原值）都偏離 golden，
  而且目前沒有觀察到實害 ⇒ 照翻、只記錄。要改的話跟我說一聲。
* **馬達電源鈕（motorPowerToggle）仍不做，但原本的理由寫錯了**（NB2 R23 抓到，已查證）：開電要的三個剎車釋放函式現在都有了
  （csystem.cpp:18964／:29240／:29274），擋住的是 `csystem.cpp:14389` 那道閘 G9 —— 它的理由「全樹沒有定義」已經過期。
  解 G9 等於「開馬達電源時會釋放 Index／Magazine／Cassette 的剎車」，關電那半（`GaliMotorServoOff`）也還沒翻，所以另案處理，先把拒絕理由改對。
* 歸零完成要不要加 golden 的「原點感測器亮」檢查：DS402 歸零由驅動器做，卡片的原點位元歸零後亮不亮沒量過，照翻可能讓每次歸零都判失敗 —— 等機台上量一次再決定。
* ⚠ `web/page/ht9045_wire_engine.js` 是 Steven 的檔，W5-a 在裡面加了兩處（`GOLDEN_BRIDGE` 多一行 `HW.teach.html`、各結構的存檔確認題 `GB_SAVE_Q`）。
  他若從自己的來源整檔同步，這兩處會被蓋掉 ⇒ 下次跟 Steven 對版時請他帶進去（跟上面 `HW.MotorTest.html` 同一類）。

## §3 紅燈／哨兵

* 配方數量哨兵：65 個目錄／64 個有 `Contact.Data`（技能文件基準 64／63，09-16）。所有工單目錄建立時間都是 09-22 16:0x（新筆電佈署複製）
  ⇒ 不是今晚新增；新基準記為 65／64，pagewire 分母 63 要重算（併 C25）。
* **「不存在」宣稱哨兵（20:1x 重跑，`tools/absence_sentinel.py --build build_nosimg`）：1 條過期** ——
  `SECSGEM/uHGemHT9045.cpp:823` 的 A3「TfOffSet has no GetOffsetPath」，以及 `ainarm2.cpp:1323-1358` 同樣的宣稱。
  JerryYang `8bfbab2f`（09-24 10:26，開機讀檔補齊八支 ReadFile）已新增 `TfOffSet::GetOffsetPath`。
  ⇒ 依賴它的閘列入 W2 候選：uHGemHT9045 的 S7 那段（SECS 遠端配方）與 ainarm2 的位置補償檔路徑（碰定位，安全相關，要逐條重問為什麼閘）。
  配方數哨兵：65 dirs／64 with Contact.Data，與今晚新基準相同。pagewire／absence 的 selftest 都通過。
* （前一輪）「不存在」宣稱哨兵：全部仍成立。`system\` 指紋：起點 540 檔已記錄，收尾時比對。
* 備份比對抓到：wb_serve 開機時在作用中工單 `T6-SIQ-PT43-BGA25X25-4-25-FT1T0\Contact.Data` 的 `[Mode]` 補寫 `bUseDieForce=0`。
  來源 `forms/fContact.cpp:1683` 的 `CheckAndReadIniData`（讀不到就寫預設值），golden `cContact.cpp:551` 同樣寫法 ⇒ **照翻的正常行為**，
  不是回歸（這份工單本來就少這個鍵，任何版本開機都會補）。每次都已還原並逐檔比對。

## §4 清理了什麼

* 刪：worktree 裡 9 個 `build_*.out`（我的 build.bat 記錄檔）與 `rc.txt`（11:30 一次指令輸出）。
* 還原：兩份被 dfm2rc 測試改寫的 `tools/dfm2rc/reports/*.json`（`git show HEAD:` 以 CRLF 寫回，未用 checkout）。

## §5 commit／push

見 `git log --oneline origin/feat/v912-port`（本週末全部推 `feat/v912-port`；分支切換要使用者先改 GitLab，放假期間不做）。

| commit | 內容 | 驗證 |
|---|---|---|
| e8ef2d4 | W1 普查工具＋W3 引用盤點工具與結果 | 只動工具／文件；普查 selftest 通過 |
| fea72460 | 塔燈接上 | 出貨組態 165 項 4 失敗＝基準 |
| 05a4d574 | 馬達測試頁改讀 C++（API＋ctest＋頁面） | 出貨組態 166 項 4 失敗＝基準；MotorPoints_HT9050 37/37 |
| e335c78f | 週末計畫 W4／W5＋通則；1203 參數文件勘誤 | 只動文件 |
| ad7561d4 | W5-a 教導頁存讀參數（C 路 `FileRW/Teach.cpp`） | 出貨 173 項 4 失敗＝基準；模擬 19 失敗＝R21 集合；端對端 19/19，真實檔已還原 |
| 9dd66076 | W4-d NB2 R22／R23 補完（HOME／LoopMove 機台旁試前）＋修正 W4-b1 模擬建置誤擋 1203 移動 | 出貨 173 項 4 失敗＝基準；模擬 19 失敗＝R21 集合；單元 185/185、七個突變各被抓到；端對端 10/10（HT9050 馬達表），真實檔已還原 |
| 4ea858da | W5-a 更正：tech.dat 一律不寫（NB2 R24） | -fsyntax-only；wb_serve 兩種組態建置 |
| 5db82ae4＋ca6094c9 | 合併 Steven `v906/steven-cbridge-review6`（39e41cf）＋補回合併掉出建置的 `FileRW/Teach.cpp` | 兩種組態全量＋ctest＝基準；端對端教導 19/19、馬達 10/10；Steven 頁面探針（唯讀）Ld_ULd／教導頁全過 |

## §6 我自己犯的錯與更正

* W0 第 2 項一度說「port 的 `myMN200motor.cpp` 沒有呼叫 `OpenEtherCatMastCard`」—— grep 輸出被 `head -8` 截斷，前 8 行都是註解；
  直接看碼 `:1175`／`:1470` 都有呼叫。已在 commit 5723535 更正。
* 塔燈接線第一版把 3 個 tag 加進覆蓋率分母 —— 違反「process tag 不進分母」的設計（test_wb_simpump O1），已改回。
* 普查工具第一版的前綴規則寫成「必須以 `.` 結尾」，selftest 當場抓到（實際是 `'pci1203.di' + i`）。
* 馬達頁改讀 C++ 時，我以為「資料庫」分頁的存檔會把 C++ 的表寫進 `Motor-config.json`，加了一道擋，還在本報告 §0 列成要你決定的第 3 件。
  之後讀同事的接線引擎才發現它早就在捕獲階段攔下那顆鈕、改寫真的 `Mot_Table.csv` —— 我擋的那條路根本跑不到。已把擋拿掉、§0 第 3 件收回（未 commit 前就發現）。
  教訓：改一頁之前，先看那一頁有沒有接線檔（`page/ht9045_wire_<slug>.js`）接管了哪些按鈕。
* W0 的結論一度寫成「卡在機台設定、要問 EastSun 三個值」。讀了 `INSTALL_ETHETCAT()` 的來歷（Sam 2023，Shuttle 感測／真空模組）才知道
  改那兩個參數會動到別的功能；正確結論是程式缺口（見 §1-1 更正）。
* 補寄信的第一次執行被我自己的檢查擋下（副本檢查讀了解析前的名字欄位），**信沒有寄出**；改用 SMTP 位址檢查後第二次寄出。收件人兩次都正確。
* **第二輪閘門第一次「看起來通過」其實建置失敗**：`test_ga1_cprod` 直接編 mysensor.cpp 但不連 myio ⇒ 連結失敗，ctest 根本沒跑；
  判定器讀到上一輪留下的 ctest.log，失敗集合「剛好等於基準」。是 `G_EXIT=2` 讓我發現。補替身後重跑才是真的通過。
  教訓：看閘門結果先看 `G_EXIT` 與 ctest.log 的時間戳，不能只看失敗集合。
* **本報告前一版把共用區 7z 密碼與另一個同格式字串直接寫出來，而且已推上 GitLab** —— 違反你「只記在本機」的交代（我自己的規則也寫了不可以進報告）。
  20260924 夜改掉，同時發現 `docs/INBOX_QUEUE.md:590` 是我 09-23 寫的明文，一起改掉。git 歷史改不掉（不做 force push），所以 §0 第 1 件建議換密碼。
  另外前一版說「只有那兩個空檔」也是查得不夠：同一字串在 V912 原始碼、`.gitignore`、Steven 的 skill 都有（§0 第 1 件的表）。
* **W3-12 說「重吹參數現在會照設定」是錯的**（NB2 R17 抓到）：我開的是函式**裡面**的閘，沒檢查函式**有沒有人叫** —— 唯一的呼叫點被另一道過期閘擋著。也把 W3_PROGRESS §12「DoSetupSystemToProd 0 個閘」照抄工具輸出，實際 9 個。已在 W3-12c 開閘＋更正文件。教訓：解開函式內的閘之後，要用 nm／反組譯確認那個函式真的有活的呼叫者（「符號存在」三級裡的第三級）。
* **W4-a 推出去的版本，頁面收不到任何「成功」的馬達回應**：我在 ack 裡放了 `"id":"cmd-N"`，而伺服器把成功 ack 的內容攤平併進 WS 的 ack，跟傳輸層的 id 撞名、後者蓋前者 ⇒ 頁面對不到號，成功的命令都會等 15 秒逾時顯示錯誤（拒絕類正常）。單元測試只看 state／seq 所以沒抓到；起真的 wb_serve 跑新探針 `tools/webprobe/w4_motor_probe.py` 才量到。已改名 `reqId`，並加測試鎖住 13 種成功回應都不帶保留字。教訓：WS 命令一定要跑一次端對端探針，不能只信假後端。
* 新測試原名 `test_lane_io_dispatch`，檔名含 "patch" 被 Windows 當安裝程式要求提權（Permission denied／Not Run），改名 `test_lane_io_route`。
* **W4-b1 的 1203 移動在預設建置裡其實一直按不動**（W4-d 端對端量到）：`Move1203` 用 golden 的 `Motor->Enable` 判斷「有沒有裝這一軸」，
  但 golden 在模擬建置（`SOFT_SIMULTE`，也就是預設建置）把**每一軸**的 `Motor->Enable` 都設成 false —— 所以 GO／相對移動／來回在模擬＋1203 實彈的建置裡
  全部回「Enable=0」。筆電沒卡、拒絕發生在更前面，假後端又把 Enable 設成 true，所以兩邊都沒量到；W4-d 照 NB2 R22 補 Enable 檢查時，
  端對端探針換上 HT9050 的馬達表才看出來。已改成看 Mot_Table 的 Enable 欄（真機建置 golden 的 `Motor->Enable` 就是這一欄），並加測試鎖住
  「模擬建置 `Motor->Enable=false`＋表上 Enable=1 時不被擋」。⚠ 共用區 02f22d05 那一版說明寫「可以驅動 1203」，實際上只有寸動／歸零／伺服會動、移動類會被擋 ——
  下次更新共用區時要一起說明。教訓：判斷「這一軸存在嗎」的旗標，要先查它在**每一種建置**裡是誰設的。
* **§0 第 6 件第一版把 `tech.dat` 的大小差說成「編譯器對齊」**（NB2 R24 抓到）：我比的是 golden 906 對移植樹 —— 兩者本來就一樣；
  3792 那個檔其實是 V899 寫的（V899 的版面短 80 bytes），而 V912 另外把兩個欄位搬到結尾。連帶「大小相同才寫」的安全預設兩邊都判反，已改成一律不寫、§0 第 6 件重寫。
  教訓：要解釋「檔案跟我的結構不一樣大」，先查**是哪一版程式寫的檔**，再比那一版的結構。
