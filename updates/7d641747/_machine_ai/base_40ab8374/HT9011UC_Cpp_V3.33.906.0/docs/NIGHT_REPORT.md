# 夜間報告 2026-09-29（二）晚起 —— **進行中版本**（每推一批整份更新；最後更新 0930 20:5x，新帳號）

> 使用者 0929 17:5x 下班：「完成手頭任務且無任務情況下，自動切換到Loop周末下班任務」；18:0x：「不用限制，我如果需要停止Loop會主動說明白」
> ⇒ **沒有收尾時間**，一直做到使用者說停；早上使用者在也照跑。迴圈：舊帳號的 cron `4ca5c42b` 已隨帳號停掉；新帳號 0930 第六批推完後重掛 `/loop 20m /night-loop`（cron 編號見 §5 最後一列）。
> ⚠ session-scoped：視窗關掉、或筆電被公司 AutoTools 強制關機（`night-loop` 技能 §5.1，0928 17:08 發生過）就會中斷；狀態全在 git 與本檔。
> 0924～0929 週末那一輪的舊報告在 git 歷史（`f455a765` 之前的版本）。
> **0930 09:3x 使用者不小心下了 `/clear`**：只清掉對話記憶；commit、各批 gate 紀錄（上一輪暫存區 `ef8cf4db…\scratchpad\iz3`／`aml3`／`flow4`）、NIGHT_REPORT／INBOX、cron 心跳 `4ca5c42b` 都還在，已從磁碟接回，**沒有遺失任何工作**。

## ★ 接手（0930 20:5x 更新：新帳號接手後，第六批＝機台 patch 全收已上 main）

| 項目 | 在哪 | 狀態／下一步 |
|---|---|---|
| ✅ **第六批：機台端 patch 全部收進 main** | GitLab main `1742fc1e`＝GitHub 第 89 包 `472b8e1` | cpp 0022～0044、web 0012～0033 全收（RULINGS_20260930 第 11 條）＋INBOX 125。做法、4 處衝突怎麼解、全面核對、筆電編譯器才報的 `_stricmp`、IO 表測試常數 134→136：`docs/MACHINE_PATCHES_20260930.md` §0。gate：模擬 267 支＝19 項基準；出貨在最終 commit 上全新重跑（§3）；執行期資料夾 0／0／0。**下一次機台 patch 從 cpp 0045／web 0034 開始** |
| 🔜 **INBOX 129 全樹「空砲彈」普查**（你 0930 19:5x） | `docs/INBOX_QUEUE.md` 第 129 列 | 第六批推完接著做：先用工具普查五類（空殼／恆回常數的函式、理由過期的閘、網頁命令沒人接、從沒被寫入的旗標、golden 有而移植樹沒有的 IO／馬達物件），分級後照 golden 補、附 ctest，每批 gate 推 main＋GitHub 包 |
| 🔜 **INBOX 131 網頁更新太頻繁：筆電全部接手**（你 0930 20:4x＝RULINGS_20260930 第 12 條） | `docs/INBOX_QUEUE.md` 第 131 列（含現況量測與 Duet3D 對照） | 原則「有開的網頁才更新資料」。先量（`[STREAM]` 計數）後改：1203 那一大塊與 Motion View 托盤沒開就不整理、hub 只送給開著的頁面、IO 頁／教導頁照 golden 視窗關著不拿。已通知 St01 先不要動（TO_STEVEN §1／§4），做完再請 St01／St02 照做 |
| 🔜 INBOX 130 機台新推的 tools 0001～0080（HTDESIGNER，VS Code 設計工具） | GitHub `machine/integ-ioweb` `24100e6`..`3617712` | 只碰 `tools/vscode-htdesigner`，不影響 wb_serve；照「照機台」精神下一批收（先跑它自己的測試） |
| ✅ 第五批 | GitLab main `f5ad8ade`＝GitHub 第 88 包 `9e38ee3` | 0930 18:39 推上：INBOX 115 B＋Y pitch 防呆、St02 MR !6 到 `236adb8e`、St01 到 `ed4716f2`。gate：出貨 265 支 4 項＝基準；模擬 19 項＝基準 |
| **INBOX 128 入料臂卡 1100（WIP，不要合）** | `v906/jimmy-hpdisp` `bd8eab19`（worktree `l2q`） | 派送器照 golden 解開（0 差異已量），但新 canary 測試在出貨組態 **SegFault**；下一步 gdb 取 backtrace（`W6_2_InArmCanary`），修測試設定或真的 NULL，再做反證。研究報告在舊 session scratchpad `B-inarm1100_report.md`（內容摘要在 INBOX 128 列） |
| ✅ INBOX 125（auto9045 的 fMain 替身 → 真的 fMain） | 第六批 | 主機下的換溫度模式／切 tester 連線／查主畫面狀態／查目前工單照 golden 真的執行（換溫度模式會動到加熱）；換工單本身還是空的（要另外把 W906_RC_ChangeSetUpFile 接出來） |
| **INBOX 127**（`SystemInitialOK` 從沒設 true ⇒ 面板鍵在任何畫面都沒作用） | worktree `l2r`（分支 `v906/jimmy-sysinit`，**還沒 commit**，改到 `FileRW/MainBoot.cpp`、`FileRW/MainRecord.cpp`、`tools/wb_serve.cpp`、`tests/CMakeLists.txt`，新檔 `tests/test_sysinit_boot.cpp`） | 舊帳號的 agent 隨帳號結束；0930 20:1x 看過還是沒 commit。夜間迴圈線 B 先收成 WIP commit 保住，再照 golden `main.cpp:9659` 做完（HT9050 IO 表 31 個面板鍵點全 Enable=0，修完真機也要等 IO 表） |
| **St01 串流提案** | RULINGS 第 10 條；TO_STEVEN §4 17:0x | ✅ 已回 St01（§10 全同意、優先）。筆電這邊待做：把 `D:\HT9045\web\JSON\Production-update.json`（8.6 MB 舊檔）搬到隔離區；第 1 階段量測可在筆電 SIM 跑 |
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

> 已經照安全預設做、不用你回的：St01 的 review6 原本先不合（St02 20:24 審出 2 個機台安全的 major）；**St01 01:19 兩個都修好了**（頁面消失只放開還按著的 jog、絕不送 STOP；Exit 關機過程讓 Light Scale 的停止通過），02:4x 已合進 main（取 St01 gate 過的 `1e5316eb`，後面新加的 B5 重新登入等 St01 自己 gate 完再說）。

---

## §1 做完了（0929 下午 → 0930）

| 時間 | 事項 | 證據 |
|---|---|---|
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
| — | — | **夜間迴圈（新帳號）**：cron `f34a7ab2`（`7,27,47 * * * *`＝每 20 分鐘，session 閒置才觸發、7 天自動到期），0930 21:0x 掛上。⚠ session-scoped：視窗關掉或筆電被強制關機就沒了 |

## §6 我自己犯的錯與更正

- **03:0x 用 `sed -i` 改了 INBOX 一個字，整份工作副本從 CRLF 變成 LF**（INDEXZ agent 剛提醒過同一個坑）：git 存的本來就是 LF，所以 commit 沒被影響；工作副本已改回 CRLF。之後這類檔一律用 Python 以二進位讀寫。
- **02:5x 第一次 111b gate 編不過**：`DoOutArmSuckPreOn` 用到 `Sen[]`，但本檔 `mysensor.h` 要到 :2401 才 include；補在上面現成的空行（:80，行號不動）後重跑通過。
- **03:3x 第 83 包的密碼掃描一度跟複製同時跑**（平行下指令），結果可能看到半份資料夾；複製完重掃一次，12 個檔 0 處。
- **02:2x 執行期資料夾檢查一度回報「IniData 少了 1807 個檔」**：檔案其實都在。備份快照是連 IniData 一起拍的，我檢查時沒帶 `SYSGUARD_DIRS`，工具只走了 system／config，於是把快照裡的 IniData 全部當成消失。已改工具（`sysguard.py`：沒帶環境變數時照快照裡記錄的資料夾檢查；帶了但不一致會先印警告），重跑是 0／0／0。
- **22:2x INBOX 114 那行開機訊息，第一版根本沒編進去**：我把 `printf` 接在 `InitAllProcessTask();` 那一行**原本的行尾註解後面**，整段變成註解（記憶裡早就記過「同一行附加要插在第一個 `//` 之前」）。是用短的模擬 wb_serve 抓那行字抓不到才發現的；已移到註解前面，前處理輸出確認有那個字串，重跑兩組態 gate。今晚其他同一行附加逐一看過：程式碼都在註解前面（托盤的 `#if 1`），其餘本來就是註解行。
- **21:0x 我一度把「Motor View 會更新」當成證據**：那一輪的探針每次都重新開頁，數字會變是因為重讀，不是頁面自己在輪詢；而無頭瀏覽器的 `document.hidden` 是 true，頁面的輪詢根本沒跑。同一輪「Motion View 畫面變了」也不能歸功給轉接檔。改用「告訴頁面它看得到＋不重新開頁、只改 API 回應」重量一次，才拿到上面 §1 那組數據。
- **WSLINK-B 第一版的前提錯了**（14:2x）：以為關 Motor Test 分頁會斷線；已照各自視窗的 golden FormClose 重做，TO_STEVEN §4 已更正。
- 端到端驗證前兩次判「失敗」都是我的腳本錯（沒帶數字 id、讀了還在緩衝的 log）；我一度說「指令被擋」，其實 `allowCmd` 從 0918 起就恆為 true，已當場更正。
- GitHub 第 72 包 README 我先寫了一句「套完機台的權杖會恢復擋人」，推之前發現錯了才改掉。
