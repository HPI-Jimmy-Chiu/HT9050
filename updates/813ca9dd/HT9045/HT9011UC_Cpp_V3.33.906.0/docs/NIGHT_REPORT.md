# 夜間報告 2026-09-29（二）晚起 —— **進行中版本**（每推一批整份更新；最後更新 1001 07:3x，新帳號）

> 使用者 0929 17:5x 下班：「完成手頭任務且無任務情況下，自動切換到Loop周末下班任務」；18:0x：「不用限制，我如果需要停止Loop會主動說明白」
> ⇒ **沒有收尾時間**，一直做到使用者說停；早上使用者在也照跑。迴圈：舊帳號的 cron `4ca5c42b` 已隨帳號停掉；新帳號 0930 第六批推完後重掛 `/loop 20m /night-loop`（cron 編號見 §5 最後一列）。
> ⚠ session-scoped：視窗關掉、或筆電被公司 AutoTools 強制關機（`night-loop` 技能 §5.1，0928 17:08 發生過）就會中斷；狀態全在 git 與本檔。
> 0924～0929 週末那一輪的舊報告在 git 歷史（`f455a765` 之前的版本）。
> **0930 09:3x 使用者不小心下了 `/clear`**：只清掉對話記憶；commit、各批 gate 紀錄（上一輪暫存區 `ef8cf4db…\scratchpad\iz3`／`aml3`／`flow4`）、NIGHT_REPORT／INBOX、cron 心跳 `4ca5c42b` 都還在，已從磁碟接回，**沒有遺失任何工作**。

## ★ 接手（1001 07:0x 更新：🔴🔴 **先看 §0 第 20 項——公開的 GitHub repo 裡有三組機台密碼**；第七～十二批都上 main＝GitHub 第 92～97 包；其餘待你決定的是 §0 第 16～19 項，第 19 項：St02 的 W58 會讓模擬組態關掉 SECS GEM／主機啟動）

| 項目 | 在哪 | 狀態／下一步 |
|---|---|---|
| ✅ **第六批：機台端 patch 全部收進 main** | GitLab main `1742fc1e`＝GitHub 第 89 包 `472b8e1` | cpp 0022～0044、web 0012～0033 全收（RULINGS_20260930 第 11 條）＋INBOX 125。做法、4 處衝突怎麼解、全面核對、筆電編譯器才報的 `_stricmp`、IO 表測試常數 134→136：`docs/MACHINE_PATCHES_20260930.md` §0。gate：模擬 267 支＝19 項基準；出貨在最終 commit 上全新重跑（§3）；執行期資料夾 0／0／0。**下一次機台 patch 從 cpp 0045／web 0034 開始** |
| 🔧 **INBOX 129 全樹「空砲彈」普查**（你 0930 19:5x） | `docs/INBOX_QUEUE.md` 第 129 列；清單 §0b | **(a)(b) 回來了**（空殼函式與過期的閘）：944 列、手動逐條讀 114 列、**確認 56 個真的空砲彈**（31 個只影響某個客戶選項、12 個卡在運動／加熱／IO 的決定、13 個沒有選項條件）。幾乎都會讓機台多做動作 ⇒ 照夜間規則不自己動，整理成 §0 第 17 項＋§0b 讓你一次決定。(c)(d)（網頁命令沒人接、從沒寫入的旗標）與 (e)（缺的物件與計時器）還在跑，一次一個 |
| ✅ **INBOX 131 網頁更新太頻繁：筆電全部接手**（你 0930 20:4x＝RULINGS_20260930 第 12 條「有開的網頁才能更新資料」） | GitLab main `40ab8374`＝GitHub 第 90 包 `7c87b10`；第 F 段＋Motion View＝第 91 包（§5） | **筆電模擬、一個瀏覽器分頁實測（改前→改後）**：C++ 每次整理的 tag 8,120 → **1,787**（1203 設定頁沒開就不整理 `pci1203.*` 6,298 個、Motion View 沒開不整理 `motionView.trays.*`）；每次整理 27～32 ms → **4～5 ms**；伺服器差異比對 6.8 → 1.4 ms；連上時整份快照 338 KB → 147 KB；**視窗全關的 40 秒**：`io/runtime` 200 次／29.2 MB → **0**、`motor/runtime` 120 次／5.5 MB → **0**（IO 頁、教導頁照 golden `fShow==false` 就不拿；Motion View 的馬達輪詢原本漏掉外框的開關通知一直在拿，已修）；IO 視窗開著的 30 秒 300 次 → 151 次（原本開著時會重複拿兩份）。**hub（第 F 段）**：關著的視窗的 iframe 不再收 snapshot／patch，連上時只給一小份開機 tag（登入等級、機種、site 開關），打開那一下補完整快照——稽核 71 個視窗後才定的做法。還沒動的：外框的對話框信箱每秒 30 次、Production-update 每秒 4 次（這兩個是「不能停的」，第 3 階段再跟你討論改成推送）。St01／St02 的新規則已寫在 TO_STEVEN §4 23:5x |
| ✅ **第七批** | GitLab main `beccfda4`＝GitHub 第 92 包 `05c15d1` | INBOX 127（開機照 golden 設 `SystemInitialOK`：面板鍵、面板燈、訊息框停馬達）、St02 MR !11／!12、`[STREAM]` 族群少讀；兩組態全新 gate＝基準（出貨 273 支 4 項、模擬 273 支 19 項），system／config 0／0／0 |
| ✅ **第八批：機台 patch 第二輪** | GitLab main `3b476827`＝GitHub 第 93 包 `c5248ed` | cpp 0045～0046（VC8 真空）、web 0034～0044（版面不重疊）、tools 0001～0109（HTDESIGNER）全收；13 顆裡 12 顆逐行等於機台 patch，手動合 4 個檔（`docs/MACHINE_PATCHES_20261001.md`）＋為 `FShow_Audit` 改 `VacuumUnitLive.inc` 2 行；兩組態全新 gate＝基準（出貨 274 支 4 項、模擬 274 支 19 項），system／config 0／0／0 |
| ✅ **第九批** | GitLab main `f528311a`＝GitHub 第 94 包 `9f5de0c` | St01 review6 到 `ea17dd3a`、St02 MR !14／!15／!16、串流 F2（外框早報開關；Contact CT／Observer 縮小照拍；Smart Diagnostic 照 golden 不改）、NB2 Q15；兩組態全新 gate＝基準（出貨 279 支 4 項、模擬 279 支 19 項），system／config 0／0／0 |
| ✅ **第十批** | GitLab main `b7572fdd`（MR !18）＝GitHub 第 95 包 `af0802d` | **D-024**（St01 回報）：vclcompat 的 `TRadioGroup` 清空選項（`Items->Clear()`）後照 VCL 把選中格歸成 -1——golden Contact 頁讀配方時靠它把找不到的 Kit 直徑預設成第一格（golden cContact.cpp:907／:925），移植樹原本留著舊格號（可能超出清單）；全樹 30 個清空點逐一查過（清單在 commit `3cb6f811`）。另 St01 review6 到 `f2df9e4f`（D-019 Setup.Contact 每次開窗照 golden 重算、D-020 Lot Info 藏頁不送命令）。gate：出貨 280 支 4 項＝基準、模擬 280 支 19 項＝基準（逐項同）；`system\`／`config\` 指紋 0 變動 |
| ✅ **第十一批：8.6 MB 輪詢拖慢「停止」命令**（NB2 1001 05:0x 量到；INBOX 131 的一部分） | GitLab main `6f0d4302`＝GitHub 第 96 包 `a7c2455` | 外框每 250 ms 抓一次 `JSON/Production-update.json`（8.6 MB），每次讓 wb_serve 的 socket 執行緒（「停止」命令也走這條）忙 115～132 ms。改法：伺服器端檔案沒變就不重讀、不重掃（輸出位元組完全相同）＋開機時在背景先讀好；瀏覽器端先問 ETag、沒變就不下載不解析。另 D-027（St01）：`fTeachPara.cpp` 註解更正（`CheckAndReadIniData` 缺鍵會補寫預設值，照 golden）。gate：兩組態全新 gate＝基準（出貨 282 支 4 項、模擬 282 支 19 項，逐項同）；gate 前後 system／config 579 檔 0 變動。實測：新舊兩版模擬 wb_serve 對同一支 8.6 MB 檔：回應位元組 MD5 相同（清過密碼的 General-config.json 也相同），伺服器首位元組 GET 98→19 ms、HEAD 86→6 ms；開機背景預讀 10 個大檔 21.7 MB 用 265 ms；兩次單獨跑 wb_serve 改到的 6 個 system／config 檔已從快照還原（MD5 核對）、新增的 lastdata_backup2.dat 搬到隔離區。⚠ 這支檔**不在 git 裡**（`.gitignore:258`，移植樹沒有 C++ 會寫它），是 0922 佈署留在你主工作目錄的模擬器時代樣本；要不要移走見 §2 |
| ✅ **第十二批：換日時昨天的 JAM 次數不存檔就被清掉（普查 E-FT2-011）——`cprod.cpp` 那一半** | GitLab main＝GitHub 第 97 包（hash 見 §5） | golden 在新的一天第一次 JAM 時先把昨天的逐日 JAM 檔存起來再清；移植樹那支本體 `#if 0`，所以只清不存。這批在 `cprod.cpp` 加一個「本體」掛勾（同一行改；沒裝＝跟現在一樣），新 ctest `JamDayHook`。**裝上它要改 St01 的 `FileRW/MainClose.cpp`**（照 golden 寫好的那份在他檔裡、外面叫不到），已在 TO_STEVEN §4 06:4x 提案、我沒動他的檔；裝上之後只寫本機 `D:\HT9045_Log\JamRate_Daily`，FTP／網路上傳照樣閘著。gate：兩組態全新 gate＝基準（出貨 283 支 4 項、模擬 283 支 19 項，逐項同）；gate 前後 system／config 0 變動 |
| ⛔ **INBOX 132 IO 畫面吸嘴沒作用——原因更正**（你 0930 22:0x 轉述 EastSun） | `docs/INBOX_QUEUE.md` 第 132 列、`docs/MACHINE_PATCHES_20261001.md` §3 | 機台自己的 cpp 0045（VACUNIT-1203，第八批收）查清楚了：吸嘴照 golden 交給 **VC8 真空模組**，而**這台 ring 1 上目前沒有 VC8**，所以按了沒作用（0045 讓畫面寫出原因）。筆電 0930 23:5x 給的 A（改 IO 表）／B（網頁不擋）**兩條都作廢**——`BTestSuck` 的輸出點跟上料／Auto1 氣缸同一條通道，B 會打到氣缸線圈。第 91 包 README 已更正（§0 第 16 項） |
| 🔜 INBOX 130 機台新推的 tools 0001～0080（HTDESIGNER，VS Code 設計工具） | GitHub `machine/integ-ioweb` `24100e6`..`3617712` | 只碰 `tools/vscode-htdesigner`，不影響 wb_serve；照「照機台」精神下一批收（先跑它自己的測試） |
| ✅ 第五批 | GitLab main `f5ad8ade`＝GitHub 第 88 包 `9e38ee3` | 0930 18:39 推上：INBOX 115 B＋Y pitch 防呆、St02 MR !6 到 `236adb8e`、St01 到 `ed4716f2`。gate：出貨 265 支 4 項＝基準；模擬 19 項＝基準 |
| **INBOX 128 入料臂卡 1100（WIP，不要合）** | `v906/jimmy-hpdisp` `bd8eab19`（worktree `l2q`） | 派送器照 golden 解開（0 差異已量），但新 canary 測試在出貨組態 **SegFault**；下一步 gdb 取 backtrace（`W6_2_InArmCanary`），修測試設定或真的 NULL，再做反證。研究報告在舊 session scratchpad `B-inarm1100_report.md`（內容摘要在 INBOX 128 列） |
| ✅ INBOX 125（auto9045 的 fMain 替身 → 真的 fMain） | 第六批 | 主機下的換溫度模式／切 tester 連線／查主畫面狀態／查目前工單照 golden 真的執行（換溫度模式會動到加熱）；換工單本身還是空的（要另外把 W906_RC_ChangeSetUpFile 接出來） |
| **INBOX 127**（`SystemInitialOK` 從沒設 true ⇒ 面板鍵在任何畫面都沒作用） | worktree `l2r`（分支 `v906/jimmy-sysinit`，**還沒 commit**，改到 `FileRW/MainBoot.cpp`、`FileRW/MainRecord.cpp`、`tools/wb_serve.cpp`、`tests/CMakeLists.txt`，新檔 `tests/test_sysinit_boot.cpp`） | 舊帳號的 agent 隨帳號結束；0930 20:1x 看過還是沒 commit。夜間迴圈線 B 先收成 WIP commit 保住，再照 golden `main.cpp:9659` 做完（HT9050 IO 表 31 個面板鍵點全 Enable=0，修完真機也要等 IO 表） |
| **St01 串流提案** | RULINGS 第 10 條；TO_STEVEN §4 17:0x | ✅ 已回 St01（§10 全同意、優先）。~~筆電這邊待做：把 `D:\HT9045\web\JSON\Production-update.json`（8.6 MB 舊檔）搬到隔離區~~（1001 改列 §2：它在你的主工作目錄，改由你決定；第十一批之後它已不拖慢停止命令）；第 1 階段量測可在筆電 SIM 跑 |
| MR !6 | GitLab | 標題已改成「滾動 MR」說明；第五批推 main 後會自動標成已合併；St02 以後一批一張 MR（TO_STEVEN §2） |
| St02 下一批／AG-1／Kevin K-01 | FROM_STEVEN、FROM_KEVIN | St02 6 列已在第五批；AG-1 等 St01 的 §2 可合列（Steven Q60 已同意）；Kevin 資料已放共用區 |
| St01 20:01 D-015（A01 閒置自動登出）＋三個問筆電的問題 | FROM_STEVEN §1／§3（`e4fa6c4d`） | ✅ 已答（TO_STEVEN §4 20:4x，跟第六批一起推）：三題都同意 St01 自己做；提醒他 `wb_serve.cpp:5953` 同一行第六批也插了一段（機台 0034），合 main 時兩段都留。St02 的 MR !11（技能文件）St01 正在代跑 gate |

### 怎麼讀

| 節 | 內容 |
|---|---|
| **§0** | **要你決定／處理的**（只放真的需要你的） |
| §1 | 做完了什麼（附證據與 commit） |
| §2 | 佇列（INBOX 111→117 已做完；現在是 115 B、127、128 與後續） |
| §3 | 紅燈／哨兵 |
| §4 | 清理與還原 |
| §5 | commit／push 清單 |
| §6 | 我自己犯的錯與更正 |

---

## §0 要你決定／處理的

| # | 事項 | 為什麼需要你 | 預設（你沒回之前照這個做） | 在哪 |
|---|---|---|---|---|
| 16 | **IO 畫面吸嘴：原因已更正，只剩「VC8 有沒有接」** | 機台自己的 0045 查到：吸嘴要走 VC8 真空模組，這台 ring 1 沒有 VC8 ⇒ 按了沒作用是對的（畫面現在會寫原因）。筆電先前寫給你的 A／B 兩條路作廢（B 會讓吸嘴鈕打到上料／Auto1 氣缸）。**只要確認一件事**：VC8 真空模組實際有沒有裝、有沒有接上 EtherCAT ring 1——有接但抓不到，是接線／卡片設定的問題；沒裝，就是等硬體 | 照現狀（網頁照擋、IO 表不動）；第 91 包 README 已請 EastSun 回 | `docs/MACHINE_PATCHES_20261001.md` §3 |
| 17 | **INBOX 129 普查 (a)(b)：20 個確定的空砲彈要不要照 golden 補** | 白話：這些地方 golden 會做事，移植樹現在是空的或回假答案。大多補了之後**機台會多做動作**（例：一輪結束時料盤氣缸真的會放開、急停時真的通知 ATC 溫控器、三溫機的安全門 6 真的會檢查），所以要你決定。建議先補「安全缺口」兩項（補了比較安全，但如果那些感測器／線沒接，可能會擋住啟動）、再補「純資料」三項；運動類等你在機台旁再一項一項開。清單分三組在 §0b | 都先不動（照夜間規則：運動／IO／互鎖不自己做） | §0b（本檔）、`scratchpad\census129\census129_ab.md` §4 |
| 18 | **🔴 溫度：`TempFuseLimitType` 在移植樹永遠是 0（普查 (c)(d) 抓到的活 bug）** | 白話：golden 開機時（`TfMain::FormShow` main.cpp:9539～9551，照客戶碼與 `iTempLimitation`，例如 `tTemp200`）會設「加熱保險絲上限」，移植樹沒翻，所以是 0。結果：① Temp_Set 頁**每次存檔都被 WAR15194 擋**（forms/fHS.cpp:857）；② 插座溫度範圍被夾到約 -36（cprod.cpp:3033-3035）；③ **出貨組態下 `CheckHeater` 41 秒後就把加熱繼電器關掉**（uHeaterThread.cpp:1294；模擬組態在 :505-511 先 return，所以筆電看不到）。golden 規則（906 main.cpp:9535-9551）：超豐 QC／力旺 或 `tTemp200` → 250；SCC／HT9046_LS 或 `tTemp150`／`155`／`175` → 200；其他 → **170**（三個常數 `cmydef.h:5429-5431` 都在）。補法：照翻這 16 行，放在開機讀完設定之後、加熱執行緒開始之前（`FileRW/MainBoot.cpp`，INBOX 127 旁邊），附 ctest。**牽涉加熱，照夜間規則我沒有自己做** | 不動（機台上溫度存檔會一直被擋、加熱會被關）；你說「做」我就照 golden 補、跑 gate、出包 | `census129_cd.md`（scratchpad）§1／§5 |
| 19 | **St02 W58 第一階段：模擬組態要不要每次開機把網路類選項關掉**（Steven 0929 對 W58 全部回「要」；St02 向筆電認領 `cprod.cpp` 4 行、`wb_serve.cpp:4052`、`CMakeLists.txt:3321`） | 白話：只影響**模擬組態**（`build.bat` 預設就是模擬組態，也就是你筆電 F5 跑的那個），出貨組態不變。模擬組態每次讀 LastSet（開機、開 Configuration 頁、每次存檔後）會把最多 60 個網路類鍵當場關掉——包含 **[N07-1] Enable SECS GEM、[N07-2] 主機啟動（host control start）**、RMS、FTP、網路磁碟；存檔時這些鍵把 config.ini 原來的字寫回去（有時整檔重寫、不是原子寫入）。例：你在筆電模擬 SECS 主機下 START，W58 之後會因為 SECS GEM 被關掉而收不到。檔案層面沒有衝突（6 行逐字相同），審查全文在 TO_STEVEN §4 06:0x | 先不合（保持現行）。回 St02：檔案認領可以，等你決定模擬組態的行為；另請他改成原子寫入、補一支兩組態都跑的 ctest。你若說「可以」我就照原案收；若要保留 SECS／主機啟動，就請 St02 把 N07 兩個鍵拿出遮罩清單 | TO_STEVEN §4 06:0x；St02 `ST02_W58_CLAIMS_20260930.md`（steven-handoff） |
| 20 | **🔴🔴 安全：公開的 GitHub 機台更新包 repo 裡有三組機台密碼**（NB2 0930 21:5x 發現、1001 02:1x 補充，標「今天要決定」；筆電 1001 06:1x 才讀到） | 白話：`github.com/HPI-Jimmy-Chiu/HT9050`（你第 18 條裁決公開）現在有 **157 個檔**含 golden 寫死的三組機台密碼——**全產品線的預設值**，不是 HT9050 自己的設定；大多是更新包帶上去的原始碼，另有一處網頁註解。任何人都看得到。機台自己推到同一個 repo 的分支也有。**哪些檔、哪幾行我刻意不寫在這裡**：這份報告本身會隨更新包公開。細節看 NB2 在私人 GitLab 分支 `v906/nb2-assist` 的 `docs/nb2_assist/notes_webref/SEC_public_repo_password_exposure.md`（不含值）。NB2 的選項：**A** 改私人（機台要給唯讀權限才能 pull）；**B** 清歷史（舊 clone 還在，要配 C）；**C** 換密碼（＝改全產品線的產品碼）；**D** 推之前掃這三組值 | **D 筆電已經先做**：新掃描器 `D:\HT9045\backup\night_tools_20260927\machine_pw_scan.py`（不進 git；值在執行時從本機讀、只放記憶體、不印不存；正對照＝公開 repo 157 檔，逐檔來源跟 NB2 清單相同；負對照 0）。之後每一包推之前都掃，**帶到這三組值的包就先不推**、列在這裡等你（第 96 包掃過：0）。**A／B／C 是對外或不可逆的，等你決定**（NB2 建議 A＋D 今天做）。那一處網頁註解可以拿掉值（行為不變），但一改，下一包的 base 副本會把舊版再公開一次 ⇒ 等你選完再改 | NB2 的兩份筆記（`v906/nb2-assist`，私人 GitLab，不含值） |
| 1 | **Motion View 要不要照 golden 加「Simulte Enable」勾選框** | golden 的 Motion View 只有勾了主畫面 Motion View 分頁上的「Simulte Enable」（CheckBox2，golden `main.cpp:8676-8686`）才會讓手臂圖跟著馬達動；沒勾就不動。網頁版（Steven 的設計）只要頁面是 LIVE 就一直跟著動。<br>例：開機後直接打開 Motion View —— **A**：跟 golden 一樣，手臂圖不動，要先勾「Simulte Enable」；**B**：一打開就跟著動（現在的樣子）。 | **B（保持現在的樣子）**：可逆，而且你 19:1x 問的正是「會不會跟著動」。你選 A 我再加勾選框 | `web/page/ht9045_mv_motor.js` 檔頭 |
| 1b | **HT9050 馬達表的軟體極限全是 ±999999（占位值）——武裝引擎馬達走 1203 之前要填真的值** | 這台 `system\Mot_Table.csv` 跟 repo 的 `machines/HT9050/Mot_Table.csv` 一樣：**19 個啟用軸的 SoftLimitN／P 全是 -999999／999999**。golden 拿它做好幾件事：MotorMove 超限就拒絕、Motor Test 的「移到極限」、路由寫到 1203 卡上的軟體極限，還有出料臂讓位的位置——`aoutarm.cpp:902` 讓位 X＝SoftLimitN＋iOutArmXBase×2000＋100，SIM 量到出料臂真的跑到 **-995899**（St02 21:43 問的就是這個）。引擎路由現在是關的（馬達命令到不了卡），所以今天不會動；**一旦武裝，出料臂讓位會往 -995899 跑、而且沒有軟體極限擋**。<br>例：Fix 盤滿要讓位 → golden 算出 -995899 → 真機一路撞到硬體極限。 | **先不動表**（那是機台設定，要 EastSun 量實際行程）；我在 GitHub README 最上面的機台通知加一行，請 EastSun 上機武裝前填好。你要換別的做法（例如先填保守值）再跟我說 | `machines/HT9050/Mot_Table.csv`；`aoutarm.cpp:902` |
| ~~1c~~ | ~~**引擎馬達走 1203：停止指令送不出去時，要不要照 golden 跳 WAR16122 警報**…~~ | ✅ **已決定**：你 0930 13:5x：A（只記錄＋畫面看得到，不另跳警報；第 6 條） | — | `RULINGS_20260930.md` |
| ~~1d~~ | ~~**INBOX 115：HIGH 那 92 個 `#if 0` 裡，有 10 個相依已經都在、但碰到安全面，要不要照 g…~~ | ✅ **已決定**：你 0930 13:5x：**全部開**（第 4 條）——agent 照分級文件逐項照 golden 解中 | — | `RULINGS_20260930.md` |
| ~~2~~ | ~~GitLab 上的 MR !7 要在網頁上按「Close」~~ | ✅ **不用了**：!7 在 0930 14:34 已被 GitLab 自動標成已合併；另外 0930 14:5x 照你的指示把 GitLab 預設分支改成 main、保護 main、!6 改指 main（RULINGS_20260930 第 9 條） | — | — |
| ~~7~~ | ~~St01 提案：只送「有開著的頁面」的資料~~ | ✅ **已決定**：你 0930 16:5x「同意優先進行」（RULINGS 第 10 條）；筆電已逐節審完回 St01 | — | TO_STEVEN §4 |
| ~~8~~ | ~~機台 patch：Z 軸煞車（cpp 0028／0030／0039）~~ | ✅ **已決定**：你 0930 18:25「一切按照機台建議」（RULINGS 第 11 條） | — | — |
| ~~9~~ | ~~機台 patch：IO 頁一律不卡控（cpp 0022 IO-NOGUARD）~~ | ✅ **已決定**：你 0930 18:25「一切按照機台建議」（RULINGS 第 11 條） | — | — |
| ~~10~~ | ~~機台 patch：關程式不用先歸零（cpp 0042 EXIT-NOHOME）~~ | ✅ **已決定**：你 0930 18:25「一切按照機台建議」（RULINGS 第 11 條） | — | — |
| ~~11~~ | ~~機台 patch：程式外殼改用自己的視窗（cpp 0042 HMI-SHELL，WebView2）~~ | ✅ **已決定**：你 0930 18:25「一切按照機台建議」（RULINGS 第 11 條） | — | — |
| ~~12~~ | ~~機台 patch：入料飛梭改名（cpp 0036 SHTSPELL：MInShutte1 → MInShuttle1）~~ | ✅ **已決定**：你 0930 18:25「一切按照機台建議」（RULINGS 第 11 條） | — | — |
| ~~13~~ | ~~機台 patch：Motor Test 回寫 Mot_Table（cpp 0043）~~ | ✅ **已決定**：你 0930 18:25「一切按照機台建議」（RULINGS 第 11 條） | — | — |
| ~~14~~ | ~~機台 patch：歸零速度（cpp 0026 HOME-VENDOR）~~ | ✅ **已決定**：你 0930 18:25「一切按照機台建議」（RULINGS 第 11 條） | — | — |
| ~~15~~ | ~~機台 patch：開機加速改寫 ini 寫檔方式（cpp 0032）~~ | ✅ **已決定**：你 0930 18:25「一切按照機台建議」（RULINGS 第 11 條） | — | — |
| 4 | **機台端 session 不在線** —— 「引擎馬達走 1203 改由筆電做、機台不要改那 5 個檔」只寫在 GitHub README 最上面 | 如果機台上今晚有人在開發，最好口頭說一聲 | — | GitHub README 最上面 |
| ~~5~~ | ~~**主畫面 RESET 鈕與 Site 格點擊要不要接上 golden 動作**（FLOW-4 做完另外三顆，這兩個刻意…~~ | ✅ **已決定**：你 0930 13:5x：A（維持不接；第 5 條） | — | `RULINGS_20260930.md` |
| ~~6~~ | ~~HT9050 的 Loader／Empty／Auto1～3 升降 Z 軸：馬達還是氣缸~~ | ✅ **已決定**（你 0930 13:0x：「Z軸是氣缸，維持A」＝RULINGS_20260930 第 2 條）：不改設定，模擬裡那 5 個軸不動是對的。馬達表那 5 列 `Enable=1` 跟「是氣缸」不一致，寫進 GitHub README 請 EastSun 看 | — | `RULINGS_20260930.md` 第 2 條 |

### §0b INBOX 129 普查 (a)(b) 待決定清單（1001 01:2x；每項都附 golden 位置，細節在 census129_ab.md §4）

| 組 | # | 空在哪 | golden 做什麼 | 補了會怎樣 |
|---|---|---|---|---|
| **安全缺口** | 13 | `csystem.cpp:519` `IsSafeLockCheck` 少了 `IsTriSafeDoor6LockCheck()` | 三溫機（`Tri_Temp_Machine`，Gerneral.ini 讀進來）會檢查安全門 6 鎖 | 三溫機上門 6 沒鎖就不能動；原本註解說「Tri_Temp_Machine 從沒被設」是錯的（database.cpp:1639 會讀） |
| 安全缺口 | 17 | `csystem.cpp:19860` 急停路徑沒送 `SendCommToATC7(ATC_EMG_DOWN)`（golden :1442） | 急停時通知 ATC 溫控器 | ATC 在急停時會收到停止 |
| **IO 動作** | 5 | `csystem.cpp:4188` `AutoTrayCylinderFree` 是空的（golden csystem.cpp:12705） | 一輪結束時放開軌道夾與料盤壓缸（`bEnableUnloadTrayFree` 開時） | 那兩個氣缸會真的動 |
| IO 動作 | 6 | `DoSwCoolingFan` 的呼叫點全在 `#if 0`（csystem.cpp:19309 等、uHeaterThread.cpp:541），本體其實在 :24787 | 開冷卻風扇／冷卻閥 | 風扇與冷卻閥會真的開關 |
| IO 動作 | 7 | `csystem.cpp:9879` `PrePushLoaderCylinder` 恆回 true（本體 uhome.cpp:388） | 回原點前先推上料氣缸（選項 P05） | 那個氣缸會動 |
| **運動／流程** | 1 | 入料臂到除靜電示教位 `MoveInArm2XYToDecayTeach` 三處被閘，ESD 讓位假裝「到了」（ainarm9045.cpp:1387 等；本體在 ainarm2.cpp） | 手臂真的移過去 | 手臂會多一段移動；WAR2026 每 30 秒重跳的問題會消失 |
| 運動／流程 | 2 | Die Clean 被 `if(false)` 取代（ainarm2.cpp:3034，golden :3813） | 清潔流程往下走 | Die Clean 會真的做 |
| 運動／流程 | 3、4 | 上料／出料的軟體齒輪比 `TransferLoaderRatio`／`TransferAutoRatio` 被略過 | XY 位置照比例換算 | 位置會變（先前已問過你 Q-RATIO） |
| 運動／流程 | 8 | `aoutarm.cpp:575` `ClearFixTray` 是空的（標著「DO NOT SHIP」卻出貨了） | 滿的 Fix 盤會被清掉換盤 | Fix 盤流程會照 golden 走 |
| 運動／流程 | 9 | `SetShuttleToHasNullIC_9045` 空的，26 個呼叫點 | 標記飛梭上的空位 | IC 帳會照 golden |
| 運動／流程 | 10、12、18、19 | RestoreLoadeIC（E62）、自動重測游標、空 socket 檢查（I41）、主機 SetMapping（ChangeSite）、幾個選配馬達動作 | 各自的選項功能 | 只影響開了那些選項的機台 |
| **純資料／通訊** | 11 | `ReadWriteBinCountMode` 從不寫 `system\BinCount.txt`（本體 csystem.cpp:28971） | 一輪結束寫 Bin 計數檔 | 會多寫一個檔 |
| 純資料／通訊 | 14 | 超豐 OLP 主機回報與查詢（含 MTBF／MUBF）都回 0 或空白（auto9045.cpp 的 W5FA 替身，真的資料其實都在） | 回真的數字 | 主機收到的數字會變成真的（客戶碼功能） |
| 純資料／通訊 | 15、16 | `RecordSafeDoorStates` 從沒被呼叫（門的 DB／SECS 事件，還會設 Err Bin 旗標）；`SetLotState` 是空的（tester 收不到 LOTSTART／LOTEND） | 記錄門狀態、通知 tester 批次開始／結束 | 15 會影響分 Bin；16 會多送 tester 訊息 |


**(c)(d) 普查（網頁命令沒人接、從沒被寫入的旗標；975 列，信心 ≥70 的 34 列）裡最要緊的**（全部都先不動，理由同上）：

| # | 在哪 | 空在哪 | 補了會怎樣 |
|---|---|---|---|
| D1-006 | 見 §0 第 18 項 | `TempFuseLimitType` 沒人寫 | 溫度存檔、加熱繼電器照 golden |
| C3-002 | `main.html:205` 主畫面 Set 鈕 | `TfMain::SetTemp` 是空的（forms/fMain.cpp:511） | 主畫面設溫度會真的送出 |
| C3-005 | `HW.home.html:56` Abort Home | 沒有接任何動作（golden uhome.cpp:5010） | 回原點中途可以中止 |
| C3-008 | 主畫面 Light／FAN 鈕 | 出貨版 `theme.js` 會拿掉 `title`，用 `[title]` 找鈕的程式綁不到（已知坑：要一起讀 `data-htitle`；普查只讀了程式、**還沒在瀏覽器驗證**） | 燈、風扇鈕在出貨版會動 |
| C1-003 | `MainClick.cpp:1408` `act.main.fan` | `SW[SwBigFan]` 從沒被驅動 | 大風扇會開 |
| D1-008～013 | `ckernel.cpp:1692` SECS 的塔燈／蜂鳴器指令 | 閘的理由過期（全域 0924 就有了），主機下指令卻回 HCACK=1 | 主機能控塔燈／蜂鳴器 |
| C3-006 | Shuttle Move 頁 19 顆運動鈕 | 反灰、理由過期；`ShuttleMoveClick` 沒翻 | 飛梭手動移動 |
| C3-119 | HotPlate 頁 5 個欄位 | 改了不會存 | 加熱盤設定能存 |

**(e) 普查（缺的 SW／Sen／MOT 物件、計時器、開機步驟；1001 06:2x 回來，360 列，413 個引用逐一對過 main）**：STATE 194、IO 輸出 92、通訊 34、寫檔 15、畫面 13、運動 12（不算 IO 表那 202 列：STATE 55、通訊 34、IO 輸出 29）。14 個主畫面計時器與 105 個其他表單計時器都盤到；其中 65 個的本體還沒逐行讀（信心 ≤8，動手前要先讀）。對 HT9050 這台最相關的前 15 項（全部先不動——大多會讓機台多做動作或多送東西出去）：

| # | 在哪（golden） | 空在哪 | 補了會怎樣 |
|---|---|---|---|
| E-T2-001 | 大風扇 `SwBigFan`（main.cpp:20968） | 只有旗標在翻（FileRW/MainClick.cpp:1407），輸出從沒被驅動 | 風扇會開（IO 表 Enable=1） |
| E-T1-012 | 安全門鎖 `SwSafeDoorLock` 跟 SystemStart（main.cpp:3051-3064；INBOX 65） | 只有開機錯誤路徑寫它 | 運轉中門鎖會上鎖（Enable=1） |
| E-T2-008＋E-TH-001＋E-RT-007 | 加熱鏈（main.cpp:20743／:20266／:18613；uHeaterThread.cpp:363） | `bUT150Install` 從沒填、執行緒沒建 | 出貨組態才會讀溫度通道、出溫度警報 |
| E-T3-001 | 8 個良率警報檢查（uYieldMonitoring.cpp:349…2031） | 沒有呼叫者 | 良率警報會響 |
| E-TM-002 | `tESDError` 佇列（main.cpp:30861-30945） | 11 個生產者、沒有消費者 | tester 警報會送到操作員 |
| E-FT2-001（新） | OLP 引擎（力成／MTI／超豐；automation.cpp:435-644） | 翻好了但沒建立，`fAutomation` 是空殼 | 會連主機、送報表、收主機命令 |
| **E-FT2-011（新，活 bug）** | `SaveJamRateByDay`（cprod.cpp:995-1110） | 本體 `#if 0`（:1072；缺 golden `FileInfo` 類別與 `FormHS`） | **半夜換日時昨天的逐日 JAM 次數不存檔就被清掉**；第十二批做了 `cprod.cpp` 的掛勾，裝上要 St01 改他的 `MainClose.cpp`（TO_STEVEN §4 06:4x 提案） |
| E-TM-004／005／008／014 | 定期寫檔（main.cpp Timer8、ESD 計時器） | 沒有呼叫者 | 良率、生產、溫度、JAM 率檔案會產生 |
| E-BOOT-005 | `RunInfo.Factory`（main.cpp:10615） | 開機沒做 | SECS SV 1005、Observer 標籤有值 |
| E-T3-003（＋E-RT-001、E-SMC-004） | Bin 顯示面板（main.cpp:25237） | 沒有呼叫者 | 面板會更新（NUMBER_PANEL_TYPE=3） |
| E-T2-003＋E-BOOT-003 | EP／ADAM-6024（adam6024.cpp:1798-1915） | 空的替身（缺驅動，信心 40） | Die force 壓力會寫出去 |
| E-T1-015 | Index 吸嘴的 PAUSE 處理（main.cpp:3098-3114） | 沒有 | PAUSE 時吸／破真空不會做一半 |
| E-FT1-001（新） | 警報框／訊息框開著時的工作（note.cpp:3143-3525；mymessbox.cpp:538-757） | `W906_ModalWaitTick` 只涵蓋約一半 | 框開著時照樣掃盤、記門、查加熱、查 PLC 安全、E84 暫停 |
| E-BOOT-002 | 開機 `ServoOnAllMOT`（main.cpp:10165） | 空替身（mymotor.cpp:2564） | 開機會 Servo On |
| E-TM-011 | Timer6 啟動檢查（main.cpp:31299-31548） | 沒有（WebStart.cpp:3684） | SECS／ChipMOS／GM-Test 的啟動觸發會作用 |
| （更正帶出來的，要你決定） | Motor Test 頁「非 1203 軸的單軸回原點」（`WebMotorAccess.cpp:1390`、Light Scale 回原點 `:2675`） | 以前拒絕，理由是「`ProcessSingleMotorHome` 在移植樹是替身」；普查 (e) 查到那支現在是真的 ⇒ 拒絕的理由過期了 | 解開＝網頁可以讓那幾軸自己回原點（**會動**），所以我沒改；你說可以我就照 golden `uhome.cpp:415` 接上、附測試 |

另兩個活 bug 在這台不會發生：E-T1-030（ChipMOS 選項才有的卡死；這台 `SnEmptyTrayIsLock1` Enable=0）、E-T3-002（soak 位元的潛在卡死，出貨組態被遮住）。順帶更正三個舊說法：INBOX 132 的前提在 main 上已過期（吸嘴綁定是活的）；`ProcessSingleMotorHome` 是真的，但 `WebMotorAccess.cpp:1390`／`:2675` 兩個字串與 (c)(d) 表 C2-142 還寫它是替身（我排進第十二批改字）；TfTesterTCP 的計時器是活的。完整表在 scratchpad `census129\census129_e.md`／`.tsv`。
> 大模組沒翻（AutoAlignment、Magazine、AutoTeach、TrayMapping CCD、LoadCCD）不是「補一行」，是新的翻譯波，另外排。

> 已經照安全預設做、不用你回的：St01 的 review6 原本先不合（St02 20:24 審出 2 個機台安全的 major）；**St01 01:19 兩個都修好了**（頁面消失只放開還按著的 jog、絕不送 STOP；Exit 關機過程讓 Light Scale 的停止通過），02:4x 已合進 main（取 St01 gate 過的 `1e5316eb`，後面新加的 B5 重新登入等 St01 自己 gate 完再說）。

---

## §1 做完了（0929 下午 → 0930）

| 時間 | 事項 | 證據 |
|---|---|---|
| 1001 05:4x～07:0x | **第十一批上 main＝GitHub 第 96 包**：`WebJsonScrubCache.{h,cpp}`（新檔）、`tools/wb_serve.cpp` 三處同一行改（行數不變）、`web/page/settings.js` refreshProduction（HEAD＋ETag）、D-027 註解 | 新 ctest `WebJsonScrubCache`（38 項，真實檔）、`ScrubCache_SettingsEtag`（node 18 項；改之前的 settings.js 紅 12 項＝活的對照組）；兩組態全新 gate＝基準（出貨 282 支 4 項、模擬 282 支 19 項，逐項同）；gate 前後 system／config 579 檔 0 變動；新舊兩版模擬 wb_serve 對同一支 8.6 MB 檔：回應位元組 MD5 相同（清過密碼的 General-config.json 也相同），伺服器首位元組 GET 98→19 ms、HEAD 86→6 ms；開機背景預讀 10 個大檔 21.7 MB 用 265 ms；兩次單獨跑 wb_serve 改到的 6 個 system／config 檔已從快照還原（MD5 核對）、新增的 lastdata_backup2.dat 搬到隔離區 |
| 1001 05:0x～06:0x | **第十批上 main（`14b278a4`）＝GitHub 第 95 包**：D-024（`3cb6f811`：`TRadioGroup::Items` 改成覆寫 `Clear()` 的子類別，清空時把 `ItemIndex` 夾到 -1，同 VCL `ItemsChange`）＋St01 review6 `ea17dd3a..f2df9e4f`（`14b278a4`） | 單獨探針 145/145；同一支測試對舊 `Controls.h` 正好紅 4 個新檢查（活的對照組）；兩組態全新 gate＝基準；gate 前後 `system\`／`config\` 579 檔 0 變動 |
| 1001 01:0x | **第七批上 main（`beccfda4`）＝GitHub 第 92 包 `05c15d1`**：INBOX 127、St02 MR !11／!12、`[STREAM]` 族群少讀、`IoBtnPanelClick.cpp` 註解更正；兩組態全新 gate＝基準 | `_g_b7*`（scratchpad） |
| 1001 00:3x | **更正 GitHub 第 91 包 README**（`46fb729`）：吸嘴 A／B 兩條路作廢（見 ★ 表 INBOX 132） | GitHub `HPI-Jimmy-Chiu/HT9050` README |
| 1001 00:3x～01:1x | **第八批：機台 patch 第二輪**（cpp 0045～0046、web 0034～0044、tools 0001～0109）用第六批的重建鏈方法收進 `v906/jimmy-mach1001`：13 顆裡 12 顆逐行等於機台 patch，0045 手動合 4 個檔（做法與理由在 `docs/MACHINE_PATCHES_20261001.md`）；HTDESIGNER 第 1 層 162／0、第 3 層 188／0 | 同左 |
| 1001 01:2x | **第九批**（`v906/jimmy-b9`）：St01 review6 到 `ea17dd3a`、St02 MR !14（Observer 版本兩格）／!15（Q3 fConfiguration）、串流 F2（外框早報開關；Contact CT／Observer 縮小照拍；Smart Diagnostic 照 golden 不改）＋新 ctest `Stream2E_St01Polls` | TO_STEVEN §4 01:2x |
| 1001 00:2x | **網頁串流第 F 段＋Motion View 上 main＝第 91 包**：hub（`web/page/ht9045_link.js`）不再把 snapshot／patch 給關著的視窗（`e977284a`，WB_WsLink 多 20 項、108 項連跑 4 次全過）；Motion View 的馬達輪詢在視窗關著時不再拿（`ec427cc5`，新 ctest `Stream2E_MvMotor` 11 項、拿修改前的檔當對照組 4 項紅）。瀏覽器實測數字見 ★ 表 INBOX 131 | `scratchpad\measure\before_f1`／`after_f1`／`after_f2`（summary.json、probe.json、wb_serve_stdout.txt 的 [STREAM] 行） |
| 0930 23:4x | **網頁串流 C++ 那一半上 main（`40ab8374`）＝GitHub 第 90 包 `7c87b10`**：第 1 階段 `[STREAM]` 計數、2A／2B／2C（頁面表問「這個視窗有沒有開」，`pci1203.*`／`motionView.trays.*` 沒開就不整理）、2E（St01 的 IO／教導頁，作者保留）。兩組態全新 gate＝基準（出貨 4 項＋`WB_F5Contract` 暫存目錄 EPERM 一次、單跑重過；模擬 19 項＝基準，271 支），system／config 整目錄 0 改 0 刪 0 新增；GitLab 自動開了 MR !13 | `_g_st7b*`／`_g_st7s*`（scratchpad） |
| 0930 23:5x | **INBOX 132 IO 吸嘴查完**（見 ★ 表與 §0 第 16 項）；**回 St02**：H-022 T3 的 `MessageDef.cpp` 檔尾 10 行＋`.h:390` 1 行筆電同意（Observer 頁 GPIB／TTL 版本兩格照 golden 顯示），附「寫的與讀的若不在同一條執行緒要加鎖」 | TO_STEVEN §4 23:5x、CHAT_JIMMY |
| 0930 20:5x | **第六批上 GitLab main＝GitHub 第 89 包：機台端 patch 全部收進來**（你 18:25「一切按照機台建議」＝RULINGS_20260930 第 11 條）。前一個帳號試套了 35 顆；剩下的 6 顆馬達／煞車 patch（cpp 0022 IO 頁不卡控＋Motor Test 只列啟用軸＋State Record 不再卡死＋操作紀錄記卡片錯誤、0026 DS402 歸零先寫卡片的歸零速度、0028／0030 每軸激磁 0.5 秒才放煞車／關激磁前先鎖煞車、0037 群組放煞車被擋時已就緒的軸各自放、0039 M36／M40 煞車輸出補上）改用前一個帳號留的**機台歷史重建鏈**逐顆 cherry-pick（真正的三方合併；先驗證重建鏈跟機台 patch 位元組相同）。衝突只有 4 處，都是兩邊各加了東西、兩邊都留；其中 State Record 背景執行緒的兩行 log 保留 St02 的執行緒安全寫法（機台的底稿比那個修正早）。全面核對：機台 0022～0044 加／刪的每一行都在（只差刻意保留的那一行），web 0012～0033 零差異。另收 **INBOX 125**。建置時抓到 2 件機台沒遇到的：① 0042 用的 `_stricmp` 筆電主 oracle（MinGW.org 6.3）不認 ⇒ 換成樹裡的 `::lstrcmpiA`，行為相同；② 0039 讓 HT9050 IO 表多綁 2 個輸出，3 支測試的常數 134→136 | `MACHINE_PATCHES_20260930.md` §0；gate 見 §3；commit 見 §5 |
| 0930 16:10 | **第四批上 GitLab main（`58cfe155`）＝GitHub 第 87 包**：① **INBOX 121**：料盤馬達照 golden `TTrayMotor::SetHTrayPanel` 綁定（50 個），常溫模擬「每個入料盤都是有盤、0 顆」的根因；重跑模擬後入料臂真的從 Loader 取料（下一個停點 ⇒ INBOX 128）。② **St01 review6 到 `7b15a0c3` 的 10 項**（Steven 14:36 的信＋交接檔 14:39；Q59：St01 兩組態綠了就請筆電合，加熱／馬達／IO 安全除外，這批沒有）：B5 重新登入、一個瀏覽器一條連線的兩個小修正、BarCode Exit＋Program Close、Windows 登出／關機送停機、C++ 標題文字套標籤、FLOW-4 提示文字、B8 風險文件、Contact RTC Auto Tuning、Offset 12 顆排序鈕、探針修正。合併只衝突 `tests\CMakeLists.txt`（兩邊檔尾各加測試段，兩段都留、沒有重名）；`wb_serve.cpp` 自動合併，跟 St01 自己合的 `0388ca16` 一致（只差還在測試中的 M-11 那一行）。 | 全新 build dir 兩組態 gate（見 §3）；執行期資料夾 0／0／0；GitHub 第 87 包 `e314949`（58 檔，掃描 0 處；README 最上面加「第 87 包起入料臂會真的去料盤取料，第一次跑 START 要有人在旁」） |
| 0930 15:5x | **Kevin K-01 的兩題答覆**：golden 用 `906.0_20260618`（要在他那份自己開 `SOFT_SIMULTE`，`MachineType.h:43`）；原始碼（886 檔、不含 .svn）與這台 HT9050 模擬用的 system／config／IniData／setup.inf（2387 檔）做成加密 7z 放共用區 `U:\共用區\HT-9050\K01_golden0618_HT9050snapshot_20260930\`（密碼同歷次交付包、不寫在任何檔） | TO_KEVIN §4（main `1b70caba`）；共用區 SHA256 與本機一致 |
| 0930 15:3x | **回 St02**：14:37 更正照收（GetBundleInfo 5 列＋asendic 替身留閘）；15:13 那 6 列：1525／1530／1538／1551 可解（1551 請連 aTester_Front／Rear 同前提的兩個閘一起列）、1539 留閘、N8 請拿掉（筆電 115 B 已改同一行）；MR !6 `d5e348a7` 跟第五批 gate。**回 St01**：AG-1 那兩行他自己在 review6 改，合併等 Steven 看過 | TO_STEVEN §4（main `6bba156c`、`1b70caba`） |
| 0930 16:1x | **你要的防呆：料盤 Y pitch ≤0 不再除以零**（`ainarm2.cpp:7293` `LoadTrayCanUse8Suck`，`AI(W906-YPITCH0)`）：pitch ≤0 時回答「不能一次吸 8 顆」走一般取料路徑；**沒有照「最小值 1」做**，因為 `手臂 pitch % 1` 一定是 0，會回答「可以一次吸 8 顆」，把壞掉的料盤定義送進 8 吸嘴路線。同一行改、行數不變；測試補 0／0.4／-4000 三種＋正常值對照 | 在第五批分支 `v906/jimmy-i115b`，跟 115 B 一起 gate |
| 0930 14:3x | **第三批上 GitLab main（`05353af8`）＝GitHub 第 86 包**：① **INBOX 119（Jerry J-5 的 C++ 那一半）**：新 WS 指令 `dialog.notifyAck`，通知型警報框（含撞機警報）可以確認關掉；確認時照 golden 按 PAUSE 關框的那幾行補上 Jam 次數、停機秒數、Recovery（機台還停在通知的停機狀態才套用暫停／ESD 停止）；新 ctest `NoticeAck`（130 項）。網頁「確認」鈕是 St01 的 `ht9045_dialog_host.js`，契約已給他（TO_STEVEN §4）——**推上來之前框仍關不掉**。② **馬達表 5 軸 Enable→0**（你 13:1x；RULINGS 第 3 條）：repo 與筆電 `system\` 兩份同改、只差 5 個位元組，改前備份在 gate 通過後刪掉；出貨組態啟用軸 19→14。③ St02：near-miss 那 11 個只差 include 的閘照 golden 解（`INDEXCYCLETIME?` 回真的 index cycle time）、Contact 的 Edit Tray 鈕照 golden 開 Tray Edit。④ **MR !7**（原生表單六個唯讀視窗 v1）：程式早已在 main，合併時保留 main 的程式、只收 README 第 11 節，GitLab 標成已合併。 | 全新 build dir 兩組態 gate：模擬 19＝基準；出貨 4＋3 個有重跑證據（`GaliRouteLive`、`HSys_HeaterMix` 單獨過；`TesterComm_TcpCmdServer` 負載下寫檔慢，放寬逾時 116.5 秒全過，第 10 段一個指令 58.8 秒 ⇒ INBOX 126）。執行期資料夾 0／0／0。GitHub 第 86 包 `8b43ae4`（25 檔，掃描 0 處；README 請機台端照 5 列改自己的馬達表） |
| 0930 13:4x | **你的 6 個回答都記成裁決**（`RULINGS_20260930.md` 第 2～8 條）：Z 軸是氣缸維持 A、馬達表 5 軸 Enable→0、INBOX 115 那 10 個全部開、RESET／Site 格 A、停止失敗 A、撞機紀錄停機秒數 A、GitLab MR 筆電審完就合。 | `f98d4939` 等 |
| 0930 13:4x | **GitLab MR 自動發現（機制 A）**：`backup\night_tools_20260927\mr_scan.py` 用 git 讀 `refs/merge-requests/*/head`（不需要 API 權杖），列出還沒進 main 的 MR、新開／有更新的標 NEW／UPDATED；已寫進夜間迴圈技能的每輪步驟。0930 當下 !1～!10 中只有 !6（St02 gpib-widget）與 !7 沒進 main，兩張都已處理。限制：看不到標題與開／關狀態（機制 B＝給一個 read_api 權杖）。 | 技能 5b；`mr_seen.json` |
| 0930 14:4x | 回同事：St02 重掃「筆電放開的檔」可解 8 列同意（GetBundleInfo 5 處讓 SECS EC 38214／38218 不再是空字串）、5 列取放流程判斷請列 OLD／NEW 逐行看、1532 維持閘住；St01 的 B8 六題答完（其中 HT9050 沒有 OTD／E84／讀碼器／RTC／ATC——他的評估用的是 Steven01 開發機設定）；**Jerry J-6**（`CheckSocketSensor` 替身恆回 true＝golden 的「有錯誤」，模擬一啟動就判 Socket 殘料、最後死結）請他照先例解閘並附 ctest。 | TO_STEVEN／TO_JERRY §4 |
| 0930 12:4x | **第二批上 GitLab main（`b21ca17e`）＝GitHub 第 85 包**：① **INBOX 118 馬達撞到（JAM）時照 golden 停機**（golden `note.cpp:1052-1169` `ShowMotorErrorMessage`，`forms/fNote_ShowError.cpp` 檔尾）：停所有馬達、Galil ST、Index 煞車關，再跳警報（走既有的 kcode==0 通知路徑，不擋主迴圈），**框送出去就立刻暫停**（你 11:4x 裁決「撞機後立刻暫停，照你的預設做」＝RULINGS_20260930 第 1 條）；閘住 3 行缺相依的（ShowErrorUnit、FTP 存 Jam 檔、瑞薩 FT-CT）；新 ctest `NoteMotorError`。⚠ 警報框目前還關不掉（Jerry J-5 → INBOX 119 進行中）。② Jerry J-1／J-2／J-4（INBOX 120 ✅）。③ 回 St01 B8 的 6 題（其中 (6) 發現他的評估用的是 Steven01 開發機的設定，HT9050 這台沒有 OTD／E84／讀碼器／RTC／ATC）。④ CLAUDE.md 的 START 普查數字更正成 34／31／3。 | 全新 build dir 兩組態 gate：出貨 4 項＝基準、模擬 19 項＝基準（各 256 個測試，沒有多也沒有少）；執行期資料夾 0／0／0。GitHub 第 85 包 `8b4a7a9`（17 檔，掃描 0 處；README 機台通知改成「118 已在第 85 包、119 進行中」） |
| 0930 12:1x | **常溫模擬卡住的根因找到了（INBOX 111 的驗收）**：11:00～11:15 用模擬跑 S1（Initial Start→HOME→Lot Start→START），跟昨晚一樣，START 後 2 分鐘除了 Tray 臂全部停住。原因：golden 開機時把約 50 個料盤馬達「綁到畫面元件」（`SetHTrayPanel`），這個綁定的副作用是打開「盤面資料跟著補盤更新」的開關；移植樹把整段當成「只是畫面」用 `#if 0 // GATE n5-G3` 關掉 ⇒ **Loader 每補一盤，盤子是有了、格子卻全是空的**：入料臂以為沒 IC 可取，一直等；Tray 臂以為這盤取完了，搬走再換一盤，無限循環。直接量到：11:10:25 的快照 `MMTrayY.fHasTray=true` 同時 `Tray.HasIC()=false`；18 次補盤、18 次都被當成空盤搬走。golden 真機紀錄（2025-12-11）補盤後是 `HasIC()=true`。**模擬與真機組態都有這個問題**（golden 是無條件綁定）。 | workflow 5 個 agent（兩路追蹤、一路綜合、兩路反向驗證都駁不倒）；報告 `docs/AMBIENT_STALL_HTRAY_20260930.md`（跟修正一起 commit）；修正＝INBOX 121 進行中 |
| 0930 11:0x | **四批上 GitLab main（`e2dac07e`）＝GitHub 第 84 包**（上一輪 0930 03:5x～07:0x 在三棵 worktree 做完、各自 gate 過，`/clear` 之後我接手合成一批再 gate 一次）：<br>① **FLOW-4（INBOX 109）**：主畫面 **ONE CYCLE／TRAY FEED／ALARM RESET 三顆鈕照 golden 真的會做事**（golden 906 `main.cpp:4332-4380`／`:13944-13947`／`:22159-22166`，本體在 `cCleanOut.cpp`；wb_serve 主迴圈每一圈取 St01 的按鈕事件、按一下跑一次）。原本呼叫空殼的引擎路徑（Auto Clean 前的 one cycle、低良率、GPIB／SECS ONE_CYCLE、SECS TRAY_FEED 約 20 處）也跟著照 golden 走。RESET 與 Site 格沒接 ⇒ §0 第 5 題。<br>② **AMB-L3（INBOX 111 第二層收尾）**：入料臂最後 8 個 `{}` 空殼換成 golden 本體（`ainarm9045.cpp` 檔尾，golden `:2231-2525`、`:4175-4191`）。最明顯的差別：**Auto Skip 次數到了會照 golden 放棄 Loader 盤**（之前計數器永遠不動）。<br>③ **INDEXZ-1203 重做＋第二輪審查修正（INBOX 113），開關 `WB_ENGINE_INDEXZ_1203` 維持關**：暫停後的恢復移動只剩 golden 那一發（DoSystem 門與電源檢查過之後的 VS/SP，G22 照 golden 解）、DP 對位照 golden、安裝器移到 1203 監控器建立之後、回原點被停住算失敗、操作員停止與警報掃描的停止分開；31 個突變全被測試抓到。開關關著時唯一的行為差異是 G22：每次 START 後送一發 `VS…;SP…` 給 Galil 層（沒有 Galil 卡＝不做事）、運轉中每輪清手動吸嘴旗標（今天沒有讀者）。**武裝前還差 INBOX 118**。<br>④ **St02 S-10 Tray Edit 頁**（golden `uTrayEditForm` 逐行＋`act.trayEdit`＋`HW.TrayEdit`；12 處直接讀 `TrayEditForm->fShow` 附理由列入基準，St01 三個條件都滿足）＋S-09 第一批剩下兩個（bthermo 常數副本、`btnClearCountClick(NULL)`）。<br>⑤ 你早上的「上傳週報」commit `4b7300ec`（只有 `.claude/commands`、CLAUDE.md、代理檔；不進 GitHub 包）。 | 合併樹 `e2dac07e`，**全新 build dir 兩組態 gate**：出貨 ＝4 項基準（config_db、ini_helpers、config_loaders、GA1_ReadGeneralIni；255 個測試）；模擬 ＝19 項基準（`PTW1_MyStringList` 啟動偶發 Not Run 0 秒，單獨重跑 0.6 秒通過）。三批合併前各自的 gate 也都＝基準（`iz3\gate_*2.log`、`aml3\after_*.log`＋單獨重跑、`flow4\gate_*.log`）。執行期資料夾（system、config、IniData，2386 檔）gate 前後比對＋還原後 0 差異。衝突只有 3 處（`CMakeLists.txt` 來源清單一行、`tests/CMakeLists.txt` 兩邊各自附加的測試、INBOX 第 107～110 列），逐列保留兩邊。GitHub 第 84 包 `c6280e6`（48 檔；README 最上面加了 INDEXZ／INBOX 118 的機台通知） |
| 0930 11:0x | 回 Steven（TO_STEVEN §4）：St02 near-miss 認領 **Q1 的 11 列同意**（Command／uHGem／cObserver／fLotInfo，相依都在、逐行對 golden；1121 的 `INDEXCYCLETIME?` 會開始回真的 index cycle time＝golden 行為）；**Q3 fConfiguration 選 A**（不建全域，寫進頁面真正用的 FileRW 編輯表；FileRW 兩支是 St01 的，要 ST01-E 同意）；提醒 St01 主畫面三顆鈕已經接上、他網頁的「實際動作還沒接」提示字要改 | 同一顆 commit |
| 0930 11:0x | **回 Jerry（新開 `docs/handoff/TO_JERRY.md`）**：他 0929 18:24 推了 5 項回報（`v906/jerry-handoff`；他能推公司 GitLab、測的是 main `aafa3953`／`3a93a28f` ⇒ 原本 §0 第 3 題已經不用問），**夜間迴圈當時沒讀他的分支，漏了 16 小時**，已補進 `night-loop` 技能的每輪讀取清單。J-1 輪詢閘門同意合（下一批 gate）、J-2／J-4 筆電修（INBOX 120）、J-3 在筆電這台會過（他那台環境）、**J-5「kcode==0 的通知框關不掉、馬達已停，只能重開程式」排成 INBOX 119，緊接在 118 後面**（118 之後每次撞機都會碰到） | 同一顆 commit |
| 03:3x | **INBOX 111b 兩個後續上 main（照 golden）**：① 出料臂 `DoOutArmSuckPreOn`（golden `aoutarm9045.cpp:678-706`）——實機＋「下降時吸」打開＋沒按手動單步時，**出料臂下降途中離目標 50 脈波內就先開真空**（之前要等 Z 到位才開，比 golden 晚）；② 入料臂三個過期閘（golden `ainarm9045.cpp:5606`／`:5768`／`:6841`）——每次取料前照 golden 重算 X 方向 pitch（之前沿用上次的值）。④ 2DID 順序檢查**沒解**：寫那個步驟計數器的是 golden `cContact.cpp:20401-20787`（整段檢查流程還沒翻），只解這一段是半套；它是 F33 選配功能，排後面。 | `30736a76`；兩組態 gate＝基準（出貨 4、模擬 19，第一次 gate 我漏了一個 include 編不過，補上重跑）；執行期資料夾 0／0／0；GitHub 第 83 包 `3d9ab3b` |
| 03:2x | 回 St02 兩題（S-09 第一批剩下的）：bthermo 那份常數副本改用 `forms/fTemp_Set.h`（定義 `uTemp_Set.cpp:228-244` 的 0～16 跟副本逐一相同）、`btnClearCountClick(NULL)`（本體只拿 Sender 比對那顆按鈕 ⇒ 跟 golden 一樣走自動清除）都同意 | `da4ec373` |
| 02:4x | **St01 review6（到 `1e5316eb`）＋St02 S-09（`53868b63`）上 main**（188 檔）。review6：網頁視窗關掉／縮小時放開還按著的 jog（**不會送 STOP**）、Exit 關機時讓 Light Scale 的停止通過、14 個設定視窗關閉照 golden FormClose、D-012 關機步驟、無畫面的停止警報送 MES16441。S-09：`cBinSel.cpp` 50 個寫檔 gate 照 golden 解開（GPIB `SETOSBIN_`、遠端命令、開機自動設 ContFail 這些路徑會寫 Bin 設定檔；網頁 BinSelect 存檔本來就會寫）、32 個過期 gate 照 golden（遠端命令回覆、SECS SV／EC 登錄、紀錄檔寫入）、`SaveASECLTestLogInfo`／`SetOEEState` 照 golden。合進來後出貨組態多紅 2 個、模擬多紅 1 個，**都是測試跟不上解閘，不是行為錯**：`Command.cpp:9889` 兩個裸讀改包 `W906_FormShowing`（跟 `ckernel.cpp:1974` 同寫法）；SecsCatalogue 的 741→769、219→229 逐筆對過 golden 四個區塊（+28 筆 SV，其中 10 筆 TObject*）。St02 02:37 說要自己修同兩處，我 02:3x 先推一列交接請他不要重做。 | 修正 `527ce723`；兩組態 gate：出貨＝4 項基準（HSys_HeaterMix 負載逾時、單獨重跑過）、模擬＝19 項基準，兩個修過的測試兩組態單獨重跑都過；執行期資料夾（system、config、IniData）對備份 0／0／0；St01 代理 gate 結果相同。GitLab `910892e4`（review6 開 MR !10、gpib-widget 是 MR !6）；GitHub 第 82 包 `9bcfea5`（147 檔） |
| 01:1x | INBOX 115 A 類 4 個上 main | GitLab `7a4aea10`；GitHub 第 81 包 `e4f0f29` |
| 00:5x | **INBOX 112「引擎馬達走 1203」進 main，開關 `WB_ENGINE_MOTOR_1203` 預設關**（你 18:5x 的決定）。關著時跟之前行為一模一樣（複審逐項確認：沒裝路由時新程式一律直接回去、不改任何狀態）；打開後引擎的馬達命令（移動、停止、回原點、速度、軟體極限）會經過新的路由送到 1203 卡。做了三路對抗式審查＋一次修正＋一次複審：Motor Test／警報路徑直接送出的停止現在路由看得到、不會把中途被停的移動記成到位；停止失敗每軸鎖存；23 個突變全被測試抓到；新測試兩組態都過（EcatMotorRoute 143／143、Pci1203MotorRoute 218／218）。**武裝前還要處理的 4 件**寫在設計書 §9.4（最重要的：停止失敗目前畫面看不到，見 §0 第 1c 題）。 | `5bffea92`、`4cc243e9`、`d84d6e74`（l2e 的 f5341505／d13494b9／541fa7e7）；兩組態 gate＝基準（245 個測試）；`pci1203_control_gate.ps1` 只剩原本的 2 個 FAIL、確認開關沒開；GitLab `e8dda454`；GitHub 第 80 包 `19ec3bc`（README 機台通知已改成「已進 main、預設關」） |
| 00:2x | St02 Motion View 自審修正（`a539e0f6`：飛梭沒讀到位置時不飛出畫面、手臂讀數顯示馬達脈波）上 main；回 St02 S-09 在筆電檔裡的認領（大部分同意，會改變操作行為的 4 類先不解） | GitLab `8f9edf73`；GitHub 第 79 包 `d7a58d2`；交接 `0f6ded24` |
| 23:5x | **St02 INBOX 117 第二步上 main**：Motion View 的手臂位置改照 golden `SetScreenScale` 的兩個教點換算（C++ 發 `motionView.screenScale`；`mymotor.h:156` 一個唯讀 getter，筆電 21:5x 同意）。筆電的 `ht9045_mv_motor.js` 繼續餵位置，換算交給 St02 的 `liveMech()` | merge `06505b34`；兩組態 gate 剛好＝基準；GitLab `3a26d332`；GitHub 第 78 包 `d6368dd` |
| 23:0x | **INBOX 114 PumpInit 寫死的機台設定全部拿掉**（12 個：`USE_OUT_SORT_ARM`、`AUTO_EMPTY_COLOR`、`AUTO3_IS_MAGAZINE`、`TRAY_VIBRATION`、`SUPPORT_2_EMPTY_EMPTY`、`bUseAuto2Empty`、`TrayForm.bEnableAMR`，和 5 個 golden 從不載入的執行期旗標）。golden 沒有這種寫死；拿掉後改用 Gerneral.ini／配方讀到的值。**量過行為不變**：這台的 `Gerneral.ini` 就是 HT9050 機台那份（MD5 相同；「HT-9045W」只是那份檔的型號字串，所以 INBOX 原本說「筆電設定會變」是過期的前提），模擬開機印出的實際值 `USE_OUT_SORT_ARM=0 AUTO_EMPTY_COLOR=0 … bEnableAMR=0` 跟原本寫死的一模一樣。`bEnableAMR` 不再被強制關掉 ⇒ golden 的 AMR 對接互鎖恢復。 | `b8d6c956`、`33052919`（開機訊息第一版掉進註解，見 §6）；兩組態 gate 剛好＝基準；GitLab `283c4567`；GitHub 第 77 包 `44b793b`（README 機台通知加了軟體極限那一行） |
| 21:4x | **St02 MR !6 第三批上 main**：S72 尾巴（golden `SetLotStart` 的 `slEventLog->SetLotData` 照跑）、S-12 模擬組態的加熱執行緒（`HeaterSimTick.cpp`，只在 SOFT_SIMULTE 下；你 16:1x 選 A）、S-09 `cprod.cpp:3009` 閘退役 | merge `927740a5`；兩組態 gate 剛好＝基準（出貨 4、模擬 19，243 個測試）；GitLab `d40fa5a0`；GitHub 第 76 包 `e72da27` |
| 21:1x | **INBOX 117 Motor View／Motion View 畫面驗證**（你 19:1x：「確認 motor view 畫面是否能正常顯示並且更新數據」「確認 motion view 畫面是否能因為 motor 位置不同而更新內容」「可以透過今天跑模擬的方式驗證」）。**兩個都原本不行，都改好、都用模擬驗過。**<br>① **Motor View 原本數字不會動**：頁面只在開頁時讀一次 `JSON/Motor-runtime.json`，C++ 根本沒在寫這個檔（`D:\HT9045\web\JSON` 那份是 09-02 的快照，provider「TBD」）。改成讀 C++ 的即時值（`/api/struct/motor/runtime`），每 500 ms 更新一次（跟 Motor Test 頁一樣）。<br>② **Motor View 的 Current 欄有幾軸一直是 0**：原本優先顯示 encoder，模擬時 `MOutArmX` 的命令位置是 -995899、encoder 是 0，畫面就一直是 0（`MOutArmY`、`MTestZ1` 也一樣）。改成照 golden `UpdateMotorScreen`（`main.cpp:8363-8384`）：顯示命令位置，只有 4 個光學尺軸跟 Galil 的 Index 軸才用 encoder。<br>③ **Motion View 原本手臂完全不動**：頁面其實有畫手臂位置的程式（`liveMech()`），但沒有人把馬達位置餵給它（St02 的 `ht9045_mv_trays.js` 只送托盤格子）。新檔 `web/page/ht9045_mv_motor.js` 每 500 ms 把命令位置餵進去。<br>**實測數據**：模擬 S1 流程（HOME→Lot Start→START，20:53～21:01）API 的位置一直在變（`MInArmX` 0→30732→4203→35399→5326；`MTrayX` 0→39662→61662→96912→4562）；改之前 Motion View 整段 SVG 從頭到尾一模一樣。頁面層級的直接證據（不重新開頁，只改 API 回應）：Motor View 表格 0/0 → 11111/22222 → 33333/44444；Motion View `MInArmX`=0 → 30000，SVG 有 28 行不同（例：入料臂橫樑 x 69.1→219.1），改回 0 又一模一樣。<br>⚠ **這台筆電的 `Gerneral.ini` 機種是 `HT-9045W`**，所以 HMI 開的是 HT9045 版的 Motion View；HT9050 版（`Main.MotionView9050.html`）目前沒有任何即時資料來源（頁面自己寫「wb_serve 無 producer」），它的手臂圖也還不會動——要做就是另一件事（交給 St02，見 §2）。 | `9c0c3209`、`4f1a080d`、`4de9813e`；量測資料 `%TEMP%\claude\…\scratchpad\mv117\`（api.jsonl 233 筆、pages.jsonl、page_probe2.json） |
| 21:1x | **INBOX 111 常溫接線第二層（四組）上 main**：<br>A 出料臂 `PCIL112_OutArmXYMove` 照 golden、`MotorMoveShuttleShake` 生效；B `GetOutShuttleStatus_9045` 照 golden；C 入料臂 4 個過期閘退役、`InArmSuckReset`、Clean Out 收尾、`DoInArm_SuckerMap`／`DoSiteMappingResult` 真本體；**D 整機退料 `DoTrayFeedProcess` 第一次接上**＋`TfMain::InitialTrayFeedTask`。<br>審查抓到的都處理了：DoTrayFeed 裡 AMR 等待（G21）與 P53 的 MESxx24（G22/G23）三個閘理由過期 → 照 golden 打開；`SYN_TEK_MOTION_MODULE` **刻意維持 0**（golden 預設 0xA7 會讓 1203 機台走 M204 latch 模式，但 latch 感測器在移植樹還是離線替身，出料飛梭殘料判斷會讀到空資料）；`GetInArm2DIDMapping` 暫停呼叫（本體 85% 還在閘裡）。沒做的列在 INBOX 111b。 | `1b726051`、`78a4e58f`、`6c0c27e9`、`290ecfa8`、`d707083d`、`40386775`；gate：出貨＝4 項基準、模擬＝19 項基準（3 項負載逾時／exe 被拿走，單獨重跑都過） |
| 20:4x | 回 St02：`SetOEEState` 放 St02 自己的 `LogObjects.cpp` 可以；review6 暫不合（見 §0 註） | `533b7c8d` |
| 19:5x | 回 St02：S-10 Tray Edit 筆電那 3 處（CMakeLists／wb_serve 分派／替身 Close）同意 | `5d27de65` |
| 14:24～19:14 | S-08 伺服器端、St02 MR !6（兩批）、WSLINK-B、原生畫面（OFF）、機台 OPLOG／SR-WIRE、St01 MR !8、常溫接線靜態盤點（79 列） | 見 §5 |
| 18:5x | **`WB_ENGINE_MOTOR_1203` 維持預設關、上機再開**（你的決定，RULINGS_20260929 第 14 條） | — |
| 18:3x | **0018 TOKEN-OFF 不收**（你的決定，RULINGS_20260929 第 13 條） | INBOX 116 結案 |

---

## §2 佇列（INBOX 111→117 已做完；0930 16:10 整份重寫）

| 順序 | 項目 | 狀態 |
|---|---|---|
| 1001 06:0x | **筆電 `D:\HT9045\web\JSON\Production-update.json`＋`JSON\js\Production-update.js`（各 8.6 MB，0922 佈署留下、不在 git）要不要搬到隔離區** | 0930 St01 串流提案 §10 I 就列過（本表上面 ★ 那列「筆電這邊待做」）。第十一批上 main 後它不再拖慢停止命令，所以不急；但它是模擬器時代的假生產資料，Motion View 等頁看得到的生產數字就是它。它在你**主工作目錄**的網頁根目錄，所以我沒動。建議：搬到 `backup\night_quarantine\`（附 manifest，隨時可搬回） |
| ✅ | INBOX 111～114、116～118、120、121 | 都在 main（最後一個是 121，第四批 `58cfe155`） |
| 1 | **INBOX 115 B：那 10 個照 golden 開**（你 13:4x「全部開」） | 🔧 l2n agent 逐項做（省電伺服關、VC8 真空、SCKART 破真空、8 吸嘴取料、Galil Index 幾何、湖口腔體燈、熱風槍流量警報、汽缸自我測試、DUT 冷卻風扇 ×2），每項照 golden＋ctest；做完當第五批 |
| 2 | **St01 下一批 `ed4716f2`**（`0388ca16`＋INBOX 119 網頁「確認」鈕；含 SU-7、M-11） | 等 St01 在交接檔 §2 寫「可合」；綠了跟第五批一起合。**「確認」鈕只在程式模擬測過，你機台開著時請在瀏覽器點一次**。AG-1（AGV.ini／E84）照 Steven 規則先給 Steven 看 |
| 3 | St02 MR !6 `d5e348a7`（出料臂 2 處 `CalTrayICCount` 照 golden 解，4 行） | 跟第五批一起 gate |
| 4 | **INBOX 128 常溫模擬第二個停點**（入料臂取完料 `InArmTask` 停在 1100、入料飛梭不動） | 🔧 唯讀研究 workflow 在跑，出來照 golden 補＋ctest，再重跑流程 |
| 5 | **INBOX 127 跳出 Message／Note 時面板實體鍵誰接**（你 14:4x） | 🔧 唯讀研究 workflow 在跑；出來跟 122（按 START 關通知框）、123（阻塞型 PAUSE 少送事件報告／ESD 停止）、124（面板 PAUSE 鍵）、86 一起照 golden 補 |
| 6 | INBOX 125 `CheckCanChangeRealDummy` 恆回 true 的替身、126 `TcpCmdServer` 第 10 段寫檔太多 | 🔜 |
| 7 | 121 帶出來的兩個舊替身（`InArmLeftSideNoIC`、`IsPick*／IsSht*Finish` 恆回 true） | 🔜 看 128 的研究結果一起排 |
| — | St02：GetBundleInfo 5 列＋asendic 替身 | **不做**（St02 14:37 更正：真本體 `cpublic.cpp:2336-2505` 還是 `#if 0`，GA1-B3 翻好再一起做）；另 6 列（1525／1530／1538／1539／N8／1551）等 St02 列 OLD／NEW 我逐行看；1532 維持閘住 |
| — | INBOX 111b 剩下的 ④⑥ | ④ 要連 golden 2DID 檢查流程一起翻（F33 選配，排後面） |
| — | HT9050 版 Motion View 接即時資料 | 已請 St02 看 |
| — | Kevin K-01／K-02、Jerry J-6（`CheckSocketSensor` 替身） | 等他們推 |
| — | **上機清單**（要人在機台旁） | G14 TCP_IP_MODE＋ON_LINE 的 SOT；引擎路由第一次上機（EastSun）；一個瀏覽器一條連線；**第 87 包起入料臂會取料**；INBOX 119「確認」鈕 |

---

## §3 紅燈／哨兵

- **1001 02:4x～04:30 額度第二次用完**：普查 (e)（缺的物件與計時器）被中斷，沒有產出；第九批的模擬組態 gate 是背景程序，02:40 照樣跑完（279 支 19 項＝基準）。04:3x 額度重置後接著推 main、做第 94 包；(e) 之後再接回。
- **1001 02:1x 第九批的模擬組態又碰到同一個 cmake 錯誤（第三次：0930 串流批、第八批、第九批）**：出貨組態一跑完、模擬組態一開始 configure，`CMakeDetermineCompilerABI_CXX.bin cannot be read`（剛產生的 ABI 檢測程式被鎖住，最可能是防毒）。照「同一個坑第三次就改機制」：新的 gate 腳本 `D:\HT9045\backup\night_tools_20260927\gate_wt.sh <worktree> <tag> [ship|sim|both]` 只在 log 同時出現 `CMakeDetermineCompilerABI` 與 `cannot be read` 時，等 15 秒、清掉那個 build 目錄、自動重跑一次；其他失敗照樣直接紅。
- **1001 01:2x 第八批第一次 gate：出貨多紅一項 `FShow_Audit`**——機台 0045 的新檔 `VacuumUnit/VacuumUnitLive.inc` 直接讀 `fShow` 兩處（專案規則要走 `W906_FormShowing`，機台那邊沒跑這支稽核）；同一行改好（`3b476827`），稽核 60 ≤ 基準 61。模擬組態那次是 **cmake 設定階段讀不到 `CMakeDetermineCompilerABI_CXX.bin`**（剛產生就被鎖住，0930 串流批也遇過一次），整批重跑。
- **HTDESIGNER 在這台的測試**：第 2 層（無頭 Edge）1 項紅（對齊線吸附）、第 3b 層 2 項沒結果（msedge 回「invalid option -d」）；機台端四層全過。這是開發工具、不進 wb_serve，照 EastSun「編譯沒問題就能收」收了，差異寫進第 93 包 README。
- **1001 00:0x 量測工具留下一個檔**：NB2 的量測工具（筆電副本 `scratchpad\nb2tools\measure_laptop.py`）跑完會把 system／config／IniData 被改的檔還原，但**新增的檔只列出來、不動**。wb_serve 開機時 `SetMD5ByFolder` 在工單 `IniData\Data\FT005054\` 寫了新的 `218f48….MD5`，還原又放回舊的 `13fcc8f3….MD5` ⇒ 資料夾有兩個 MD5，golden `CompareMD5ByFolder` 看到超過一個會全部刪掉（這張工單就沒有檢查碼了）。兩次都已移到 `D:\HT9045\backup\night_quarantine\20260930\`（有 MANIFEST，可搬回），資料夾回到量測前只剩 12/11 那一個；筆電副本改成把新增的檔搬到量測輸出夾。**NB2 的原版（`v906/nb2-assist` 的 `tools/nb2_assist/webhmi_runtime_measure.py`）有同一個缺口**，待 NB2 改。
- **0930 23:3x～23:30 額度用完**：當時在跑的 5 個 agent（普查三個＋(e) 的一個子 agent＋IO 吸嘴調查）全部中斷，模擬 gate 是背景程序照跑完（19 項＝基準）；額度重置後依序接回。
- **0930 20:5x 第六批 gate（全新 build dir）**：模擬 b6b（20:15～20:36）267 支＝**19 項基準，逐項相同**；出貨 b6b 267 支＝4 項基準＋3 支 HT9050 IO 表測試（常數 134 還沒改成 136 時跑的，改完直接跑 exe 三種卡別 PASS）⇒ 在要推的 commit `d5c51c86` 上**全新重跑出貨組態** b6c（20:38～20:49）267 支＝**4 項基準，逐項相同**。執行期資料夾（system、config、IniData，2,386 檔）b6b 前後、b6c 前後兩次整目錄比對都是 **0／0／0**，快照已刪。另外量到：出貨組態第一次建置（b6）只有 1 個錯（cpp 0042 的 `_stricmp`，MinGW.org 6.3 不宣告），用 `-k 0` 探測建置確認全部 1019 個目標只有這一個錯後才修。
- ⚠ 主 checkout 根目錄（`D:\HT9045`）有一個**未追蹤、檔名就是舊的 7z 密碼**的檔（`git status` 的 `??` 看得到）：不是筆電這一輪產生的、沒有進任何 commit；它不在 GitHub 包的範圍（包只帶 `HT9011UC_Cpp_V3.33.906.0/`、`web/`），推之前的掃描也會比對密碼字串。主 checkout 有別的 session 的在製工作，這一輪不去動它，只記在這裡（清掉要你同意）。

- ⚠ **0930 17:30:22～27 模擬用工單 FT005054 的 `Binasgn.Data`／`BinasgnOff.Data`／`BinasgnOff-Line.Data` 被改寫成全部 NotUse**（原本 Auto1～3／Fix1～3 的分 Bin 對應；另多出 `AOIBinTraySetting=`、`BinTrayLinked(10998)` 等空鍵）。第五批 gate 的 sysguard `g0930e` 抓到，已從快照逐檔還原（cmp 相同，再比對 0／0／0）。**不是第五批 gate**：它的出貨 ctest 17:32 才開始；重跑第五批新測試（B8_M11_FtRt、B8_Su7_RtcClick）與 INBOX 125 的 3 支 committed 測試都 0 差異。最可能是 INBOX 125 agent 17:3x 前後的反證（mutation）執行：它故意讓 `SetStartMode` 走進 `SetRunStartMode → TfBinSel` 那條路（不在任何 commit 裡）。**教訓**：ctest 的真實檔圍堵要蓋到 agent 的反證執行；sysguard 已知清單沒有 Binasgn，這類寫入只有整目錄比對抓得到。
- **0930 16:10 第四批 gate**：兩組態全新 build dir＝基準（出貨 261 支 4 項失敗＝基準；模擬 19 項＝基準，另 `W7_S0_MotorConvergence` 顯示 Not Run——「Process not started／operation not permitted」，exe 剛連結完被防毒鎖住，單獨重跑 1 秒通過）；執行期資料夾 0／0／0；GitHub 包掃描 0 處。期間 INBOX 115 B agent 在自己的工作樹跑了 43 支目標測試（15:27 起），sysguard 仍是 0 差異。
- **0930 11:0x 哨兵**：配方數 66 個目錄／65 份有 Contact.Data（＝基準）；控制字元：這一批改到的 54 個檔 0 處；接線 `check_deployed.py --selftest` 通過。
- **0930 12:4x 哨兵**：兩組態 gate＝基準；執行期資料夾 0／0／0；GitHub 包掃描 0 處。
- **03:1x 配方數哨兵：66 個目錄／65 份有 Contact.Data**（基準 65／64）——多的那個是 `FT005054`（0928 10:13 建立，就是 HT9050 模擬用的工單），預期中；0929 pagewire 在它加進來之後量過，沒有新的接線回歸。新基準 66／65。
- **03:1x 控制字元哨兵**：我們今晚寫的檔 0 處；命中的是產生的手冊（NUL）、技能裡從 PDF 抽出來的參考檔、以及三棵樹都有的 `atester_ProcessCount.cpp`（golden 帶來的），都是舊的。
- **接線哨兵（pagewire `check_deployed.py`）23:5x：自我測試通過；實跑 14 項紅，跟 INBOX 102 記的一樣**（`sync_web.py` 的 OURS 清單 0922 之後沒更新，St01／St02 0925～0928 加的接線 js 都不在裡面；client repo 已凍結，沒人在跑同步）⇒ 今晚沒有新增的接線回歸，筆電新加的 `ht9045_mv_motor.js` 沒被標。
- **23:1x～23:50 額度用完**：112 複審與 115 分級兩個 agent 被中斷（沒有產出），23:5x 重派；gate 是背景程序，照跑完。
- **機器負載造成的 ctest 逾時**（不是回歸，都有重跑證據）：21:0x 那次 `TesterComm_TcpCmdServer`（146 秒逾時，單獨 16.5 秒過）、`HSys_HeaterMix`（119 秒逾時，單獨 14.9 秒過）。
- **`IniFiles_Win32Diff` 在模擬組態「沒啟動」**：`build_sim` 裡那支 exe 剛連結完就不見了（Process not started／operation not permitted），像是被防毒拿走；重新連結後單獨跑 18 秒通過。出貨組態那支一直都在。再發生就去看 Defender 的隔離紀錄。
- **無頭瀏覽器的 `document.hidden` 是 true**：Motor View、Motor Test、Motion View 的輪詢在「看不到的視窗」會停（這是對的），所以用無頭瀏覽器驗證頁面輪詢時，要先告訴頁面它是看得到的，不然會量到假的「沒更新」。

## §4 清理與還原

- 21:0x 跑了 3 次模擬 wb_serve（S1 流程一次、頁面短測兩次，第一次短測因為 tag 撞名沒啟動）。每次開關機都會補寫 `Gerneral.ini`、`config.ini`、`lastdata.dat`、`machinerecord.dat`、工單 FT005054 的 6 個檔、換 MD5 檔 ⇒ `flow_run.py` 照 sysguard 快照 `ht9050s1b_0929` 逐檔還原、log 與 StateRecord 新檔移走；**最後 sysguard 0／0／0**。

## §5 commit／push 清單

| GitLab main | GitHub 包 | 內容 |
|---|---|---|
| `4a4040ce` | 69 `ba3b57b` | S-08 伺服器端、ST02 工作卡 S-09～S-11 |
| `a84d25cc` | 70 `60afc75` | St02 MR !6 |
| `3a93a28f` | 71 `9227de3` | WSLINK-B、原生畫面（OFF）、測試修正 |
| `180657c7` | 72 `7a708de` | 機台 OPLOG／SR-WIRE、INBOX 111～116、TO_KEVIN、機台通知 |
| `052194b2` | 74 `7153246` | St02 MR !6 新一批＋St01 MR !8 |
| `5d27de65`、`533b7c8d` | （只有交接檔，併進 75） | 回 St02 的兩列 |
| `64d8554e` | 75 `320a0c5` | INBOX 111 四組＋INBOX 117 三顆（16 檔；引擎走 1203 不在這包） |
| `d40fa5a0` | 76 `e72da27` | St02 MR !6 第三批（11 檔） |
| `283c4567` | 77 `44b793b` | INBOX 114、111b 註解、交接檔（7 檔） |
| `3a26d332` | 78 `d6368dd` | St02 INBOX 117 Motion View 換算（5 檔） |
| `8f9edf73` | 79 `d7a58d2` | St02 Motion View 自審修正（4 檔） |
| `e8dda454` | 80 `19ec3bc` | INBOX 112 引擎馬達走 1203（開關關著，18 檔） |
| `7a4aea10` | 81 `e4f0f29` | INBOX 115 A 類 4 個 |
| `910892e4` | 82 `9bcfea5` | St01 review6 到 `1e5316eb`＋St02 S-09 `53868b63`＋兩個測試修正（147 檔） |
| `16aafc23`、`da4ec373` | （只有文件，併進 83） | INBOX 狀態更新＋FLOW-4 認領、回 St02 兩題 |
| `30736a76` | 83 `3d9ab3b` | INBOX 111b ①②（2 檔） |
| `e2dac07e` | 84 `c6280e6` | FLOW-4、AMB-L3、INDEXZ-1203（關）、St02 S-10＋S-09 剩下兩個、交接檔（48 檔）；另含你的「上傳週報」commit（不在包裡） |
| `b21ca17e` | 85 `8b4a7a9` | INBOX 118、Jerry J-1／J-2／J-4、裁決 0930 第 1 條、INBOX／報告（17 檔；交接檔與 CLAUDE.md 不在包裡，包只帶移植樹與 web） |
| `05353af8` | 86 `8b43ae4` | INBOX 119、馬達表 5 軸、St02 near-miss＋Contact Edit Tray、MR !7 README、交接檔（25 檔；交接檔與 `machines/` 不在包裡） |
| `58cfe155` | 87 `e314949` | INBOX 121、St01 review6 到 `7b15a0c3`（10 項）、INBOX 127／128、交接檔（58 檔；交接檔不在包裡） |
| `f5ad8ade` | 88 `9e38ee3` | INBOX 115 B＋Y pitch 防呆、St02 S-09 放開檔、St01 SU-7／M-11／INBOX 119 網頁確認鈕（33 檔） |
| `1742fc1e` | 89 `472b8e1` | 第六批：機台 patch cpp 0022～0044／web 0012～0033 全收＋INBOX 125＋`_stricmp`／IO 表測試常數；文件：RULINGS 第 12 條、INBOX 129～131、TO_STEVEN（72 檔；掃描 0 處；README 最上面加第 89 包與 Cassette 煞車的機台通知） |
| `40ab8374` | 90 `7c87b10` | 網頁串流 C++ 那一半：第 1 階段＋2A／2B／2C＋2E（21 檔；兩組態 gate＝基準；MR !13） |
| `7d641747`（`e977284a`、`ec427cc5`＋文件） | 91 `a18116c`（README 更正 `46fb729`） | 串流第 F 段（hub）＋Motion View 馬達輪詢＋INBOX 132 查證＋回 St02 MessageDef |
| `beccfda4` | 92 `05c15d1` | 第七批：INBOX 127、St02 MR !11／!12、`[STREAM]` 族群少讀、註解更正（12 檔） |
| `3b476827` | 93 `c5248ed` | 第八批：機台 patch 第二輪（cpp 0045～0046、web 0034～0044、tools 0001～0109；114 檔，59 個是 HTDESIGNER）＋`FShow_Audit` 兩行 |
| `f528311a` | 94 `9f5de0c` | 第九批：St01 review6 到 `ea17dd3a`、St02 MR !14／!15／!16、串流 F2、NB2 Q15、INBOX 115／127／129～132 狀態 |
| `b7572fdd`（MR !18） | 95 `af0802d` | 第十批：D-024（vclcompat `TRadioGroup`）、St01 review6 `ea17dd3a..f2df9e4f`、St02 四個認領的回覆 |
| `b25f6816`、`6f0d4302` | 96 `a7c2455` | 第十一批：`/JSON` 路由檔案快取＋開機預讀、settings.js HEAD／ETag、D-027 註解；§0 第 20 項（公開 repo 的機台密碼）＋推送前密碼掃描 |
| 本批（第十二批，含本份報告） | 97 | JAMDAY（`cprod.cpp` 的 SaveJamRateByDay 本體掛勾＋ctest `JamDayHook`）、本報告 |
| — | — | **夜間迴圈（新帳號）**：cron `f34a7ab2`（`7,27,47 * * * *`＝每 20 分鐘，session 閒置才觸發、7 天自動到期），0930 21:0x 掛上。⚠ session-scoped：視窗關掉或筆電被強制關機就沒了 |

## §6 我自己犯的錯與更正

- **1001 06:5x 第十二批第一次 gate：我自己寫的測試錯了**：`test_jamday_hook.cpp` 自己宣告 `extern int SystemDate;`，真正的是 `Word`（2 位元組，`cmydef.h:227`），讀了 4 個位元組、3 個 iToday 檢查失敗；編譯器與連結器都不比對變數型別，所以編得過也連得過。改成 include `cmydef.h`，停掉那一輪 gate 重跑（產品碼沒改）。教訓：測試要用到全域就 include 它的標頭，不要自己寫 extern。
- **1001 06:3x 差點把「密碼在哪些檔、哪一行」寫進會公開的報告**：§0 第 20 項第一版列了含值的檔名與那一行網頁註解的位置；這份報告本身會隨每一包更新包公開。推 GitHub 前發現，改成只指向 NB2 在私人 GitLab 分支的筆記（`6f0d4302`），第 96 包是改過之後才打的包（GitLab 私人 main 上有那一版約 3 分鐘）。
- **1001 06:1x 才發現 NB2 0930 21:5x 就標了「今天要決定」的安全問題（公開 repo 有機台密碼，§0 第 20 項）**：整夜只讀了 NB2 條目裡跟串流、效能有關的段落，沒讀它最上面「給主電腦／Jimmy」那段，之後又推了第 92～95 包——其中第 94 包多公開了 4 份含值的檔。更正：推送前改掃這三組值（`machine_pw_scan.py`，第 96 包起），列 §0 第 20 項等你決定 A／B／C；報告裡也不寫它們在哪些檔（報告會隨包公開）。教訓：讀 NB2 的新條目要先讀「給主電腦／Jimmy」那一段，它標了「安全」「今天要決定」的，當輪就列 §0。
- **0930 23:5x 我給了 INBOX 132 一個會打到氣缸的建議（B：網頁不擋 Enable=0）**。當時只讀了 0925 的 IO 表跟網頁／C++ 的程式，**沒有先去讀機台 GitHub 上最新的 patch**——機台 0930 21:09 推的 cpp 0045 已經量到 `BTestSuck` 跟上料／Auto1 氣缸同通道、並把吸嘴交給 VC8。好在我照夜間規則沒有動手、只寫成決策題；1001 00:3x 讀到 0045 就更正了 README 與 INBOX。以後碰機台端的 IO／運動問題，**先看機台分支最新的 patch 與 README 再下結論**。
- 第八批的 gate 腳本是從第七批的複製來的，`sed` 換路徑時漏了 Windows 路徑那一行（反斜線），差點去建錯的樹；開跑前發現、改用 Python（`chr(92)`）換掉，建置前確認 0 處殘留。
- **0930 22:2x agent 總數超過上限**：普查 (e) 自己開了 5 個子 agent，加上其他 4 個，同時在跑的到 9 個（上限 5，CLAUDE.md）。我派 (e) 時沒寫「不准再開子 agent」；當場停掉 4 個、叫它一次只開一個，接回時每個 agent 的訊息都重寫了這條。
- **1001 00:0x 兩個測試斷言寫錯（都在提交前抓到）**：第 F 段測試一開始數「目前開著的連線數」，會被前面段落閒置關掉的連線影響，改成數「新建立的連線數」；Motion View 修正一開始斷言檔案是純 ASCII，其實檔頭註解本來就有中文，改成檢查沒有 U+FFFD。
- **03:0x 用 `sed -i` 改了 INBOX 一個字，整份工作副本從 CRLF 變成 LF**（INDEXZ agent 剛提醒過同一個坑）：git 存的本來就是 LF，所以 commit 沒被影響；工作副本已改回 CRLF。之後這類檔一律用 Python 以二進位讀寫。
- **02:5x 第一次 111b gate 編不過**：`DoOutArmSuckPreOn` 用到 `Sen[]`，但本檔 `mysensor.h` 要到 :2401 才 include；補在上面現成的空行（:80，行號不動）後重跑通過。
- **03:3x 第 83 包的密碼掃描一度跟複製同時跑**（平行下指令），結果可能看到半份資料夾；複製完重掃一次，12 個檔 0 處。
- **02:2x 執行期資料夾檢查一度回報「IniData 少了 1807 個檔」**：檔案其實都在。備份快照是連 IniData 一起拍的，我檢查時沒帶 `SYSGUARD_DIRS`，工具只走了 system／config，於是把快照裡的 IniData 全部當成消失。已改工具（`sysguard.py`：沒帶環境變數時照快照裡記錄的資料夾檢查；帶了但不一致會先印警告），重跑是 0／0／0。
- **22:2x INBOX 114 那行開機訊息，第一版根本沒編進去**：我把 `printf` 接在 `InitAllProcessTask();` 那一行**原本的行尾註解後面**，整段變成註解（記憶裡早就記過「同一行附加要插在第一個 `//` 之前」）。是用短的模擬 wb_serve 抓那行字抓不到才發現的；已移到註解前面，前處理輸出確認有那個字串，重跑兩組態 gate。今晚其他同一行附加逐一看過：程式碼都在註解前面（托盤的 `#if 1`），其餘本來就是註解行。
- **21:0x 我一度把「Motor View 會更新」當成證據**：那一輪的探針每次都重新開頁，數字會變是因為重讀，不是頁面自己在輪詢；而無頭瀏覽器的 `document.hidden` 是 true，頁面的輪詢根本沒跑。同一輪「Motion View 畫面變了」也不能歸功給轉接檔。改用「告訴頁面它看得到＋不重新開頁、只改 API 回應」重量一次，才拿到上面 §1 那組數據。
- **WSLINK-B 第一版的前提錯了**（14:2x）：以為關 Motor Test 分頁會斷線；已照各自視窗的 golden FormClose 重做，TO_STEVEN §4 已更正。
- 端到端驗證前兩次判「失敗」都是我的腳本錯（沒帶數字 id、讀了還在緩衝的 log）；我一度說「指令被擋」，其實 `allowCmd` 從 0918 起就恆為 true，已當場更正。
- GitHub 第 72 包 README 我先寫了一句「套完機台的權杖會恢復擋人」，推之前發現錯了才改掉。
