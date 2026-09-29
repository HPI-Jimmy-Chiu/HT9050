# 夜間報告 2026-09-29（二）晚起 —— **進行中版本**（每推一批整份更新；最後更新 18:3x）

> 使用者 0929 17:5x 下班：「完成手頭任務且無任務情況下，自動切換到Loop周末下班任務」；18:0x：「不用限制，我如果需要停止Loop會主動說明白」
> ⇒ **沒有收尾時間**，一直做到使用者說停；早上使用者在也照跑。迴圈：cron `4ca5c42b`（每小時 :07／:27／:47，session 閒置才觸發）。
> ⚠ session-scoped：視窗關掉、或筆電被公司 AutoTools 強制關機（`night-loop` 技能 §5.1，0928 17:08 發生過）就會中斷；狀態全在 git 與本檔。
> 0924～0929 週末那一輪的舊報告在 git 歷史（`f455a765` 之前的版本）。

### 怎麼讀

| 節 | 內容 |
|---|---|
| **§0** | **要你決定／處理的**（只放真的需要你的） |
| §1 | 做完了什麼（附證據與 commit） |
| §2 | 佇列（照 INBOX 111→116 的順序） |
| §3 | 紅燈／哨兵 |
| §4 | 清理與還原 |
| §5 | commit／push 清單 |
| §6 | 我自己犯的錯與更正 |

---

## §0 要你決定／處理的

| # | 事項 | 為什麼需要你 | 預設（你沒回之前照這個做） | 在哪 |
|---|---|---|---|---|
| 1 | **GitLab 上的 MR !7 要在網頁上按「Close」** | St01 和筆電都沒有 API 權杖，只能用網頁關；它已經被筆電的直接合併取代 | 放著不影響任何東西 | `gitlab.honprec.com/.../ht9045/-/merge_requests/7` |
| 2 | **Jerry**：他能不能推公司 GitLab、測的是哪一版 | 決定用 git 交接分支還是寄 patch | 等你問 | 18:1x 那則回覆 |
| 3 | **機台端 session 不在線** —— 「引擎馬達走 1203 改由筆電做、機台不要改那 5 個檔」只寫在 GitHub 第 72 包 README 最上面 | 如果機台上今晚有人在開發，最好口頭說一聲 | — | GitHub README 最上面 |

---

## §1 做完了（0929 下午到晚上）

| 時間 | 事項 | 證據 |
|---|---|---|
| 14:24 | **S-08 伺服器端**（機台 0016 WSFANOUT＋TAKEOVER：tag 補丁每個基準只算一次、連線上限 64、`control.takeover`）＋web 0008＋審查修正 TK-1／TK-2 上 main | GitLab `4a4040ce`；GitHub 第 69 包；通知 ST02 的信 14:26 |
| 15:03 | **St02 MR !6**（OEE W61、HANDLER LOG、Motion View 料盤頁、R117）上 main；`W906_Trace` 的偶發失敗查到是**測試自己的競態**（送出時 `SendLocalDataFrom` 又取一次 GemClock） | `a84d25cc`；第 70 包 |
| 17:14 | **頁面關閉的馬達保護 WSLINK-B**（一個瀏覽器一條連線之後，關 Motor Test／Teach 不會斷線）：照各自視窗的 golden FormClose —— 關 Motor Test＝LoopMove 結束、HOME 照走、停它自己的 jog；關 Teach＝golden 的 STOP；運轉中關 Teach 只停 Teach 的 jog。兩輪對抗式審查（第一版的前提錯了、Light Scale 回原點會被記成成功，都修了）；ctest 549／0 兩組態、SIM wb_serve 端到端通過、突變測試證明新測試會抓到 | `a4581654`、`6d16e728` |
| 17:14 | **C++ 原生畫面兩條分支合進 main（預設 OFF）**（你 15:4x 選 A）；原生保活對齊主迴圈（StateRecordDrainLog、TK-1）；FShow_Audit 那一行保留直接讀；W906_Trace 測試改成「前後兩次取樣夾住」 | `0f547f9a`、`a0c0df88`、`2c663ab9`、`4d44d43c`；`3a93a28f`；第 71 包 |
| 17:0x | **常溫接線靜態盤點**（4 個 agent，79 列）：只有 12 列真的接上。三層原因：①引擎的馬達命令到不了 1203 卡（生產函式庫沒武裝）；②移植樹的替身與過期閘（出料臂 XY 移動是回「到位」的假函式等）；③9050 自己的表（吸嘴／Tray 臂氣缸 Enable=0，你確認是硬體還沒接） | RULINGS_20260929 第 11 條；`D:\HT9045\backup\night_tools_20260928\ambient_audit_20260929.json` |
| 18:09 | **機台 OPLOG＋OPLOG-2＋SR-WIRE（主畫面 State Record 鈕）合進 main**（0018 TOKEN-OFF 沒收）；INBOX 111～116；Kevin 的交接（TO_KEVIN.md，K-02 Index 流程交給 Kevin） | `5f4a7a42`、`8817e3cc`、`180657c7`；第 72 包（README 最上面加機台通知） |
| 18:3x | **0018 TOKEN-OFF 不收**（你的決定，RULINGS_20260929 第 13 條） | INBOX 116 結案 |
| 16:56 | 回 Steven 原生六頁那封信（你說可以寄） | Outlook 寄件備份 16:56:17 |

---

## §2 佇列（照 INBOX 111→116）

| 順序 | 項目 | 狀態 |
|---|---|---|
| 1 | **INBOX 111 常溫接線第二層**：出料臂 `PCIL112_OutArmXYMove`、`GetOutShuttleStatus_9045`、入料臂 4 個過期閘＋`InArmSuckReset`＋Clean Out 收尾、`MotorMoveShuttleShake`、`DoTrayFeed` 閘＋`InitialTrayFeedTask`、`SYN_TEK` 初值 | 🔧 18:2x 起：workflow `wf_423e59ae-70e`（4 組各自翻譯＋對抗式審查，在獨立 worktree `l2a`～`l2d`） |
| — | St02 MR !6 新一批（S-09 第三／四批、W59-1、Q45、E-016 認領）＋St01 MR !8（Q53、Teach 測試修正） | 🔧 本機已合（`b2b4ee63`、`a753ac89`），兩組態＋原生 ON gate 在跑 |
| 2 | **INBOX 112 引擎馬達走 1203**（設計書 `docs/ENGINE_1203_MOTOR_ROUTE_DESIGN.md`，Q1～Q11 用建議預設值） | 等 111 |
| 3 | INBOX 113 INDEXZ-1203 重做（Index Z1） | 等 112（共用 1203 控制層） |
| 4 | INBOX 114 PumpInit 寫死的機台設定一個一個拿掉 | 等 |
| 5 | INBOX 115 HIGH 那 92 個 `#if 0`（避開 ST02 已認領的） | 等 |
| — | Kevin K-01（golden 模擬錄 State Record，常溫、FT005054_9050）／K-02（9050 Index BCB6 流程） | Kevin 18:04 認領 K-01 |
| — | St02 S-12（加熱，只接模擬組態）、S72 尾巴 | 筆電 18:2x 同意他們的認領，由 St02 做 |
| — | **上機清單**（要人在機台旁）：G14 TCP_IP_MODE＋ON_LINE 的 SOT 有沒有真的送出（St02 16:11）；引擎路由第一次上機（EastSun 在旁）；一個瀏覽器一條連線（23→1 條、Motor Test 還會動） | 等上機 |

---

## §3 紅燈／哨兵

- **機器負載造成的 ctest 逾時**（不是回歸，都有重跑證據）：`EditList_PageIndex`／`PlcGates`／`WebMotorAccess`（16:44 那次，4 個盤點 agent＋VS Code 的 cpptools 同時在吃 CPU，單獨重跑都過）、`HSys_HeaterMix`（17:53 那次 118 秒逾時，空機重跑 1.97 秒）、`LastDataSandbox`（「Not Run」，這台已知的暫時狀況，單獨 0.86 秒過）。
- VS Code 的 cpptools（C++ IntelliSense）在合併大量新檔後重建索引，用了 6,000 多秒 CPU；gate 期間會拖慢測試。

## §4 清理與還原

- 16:3x 端到端驗證跑了三次 SIM wb_serve，開機／關機會照 golden 補寫 `Gerneral.ini`、`config.ini`、工單 FT005054 的 6 個檔、刪換 MD5 檔 ⇒ **每次都照 sysguard 快照 `ht9050s1b_0929` 逐檔還原，最後 0／0／0**；被換掉的內容存在 `D:\HT9045\backup\night_quarantine\20260929\e2e_wslink_b\after_run\`。

## §5 commit／push 清單

| GitLab main | GitHub 包 | 內容 |
|---|---|---|
| `4a4040ce` | 69 `ba3b57b` | S-08 伺服器端、ST02 工作卡 S-09～S-11 |
| `a84d25cc` | 70 `60afc75` | St02 MR !6 |
| `3a93a28f` | 71 `9227de3` | WSLINK-B、原生畫面（OFF）、測試修正 |
| `180657c7` | 72 `7a708de` | 機台 OPLOG／SR-WIRE、INBOX 111～116、TO_KEVIN、機台通知 |

## §6 我自己犯的錯與更正

- **WSLINK-B 第一版的前提錯了**：以為「今天關 Motor Test 分頁會斷線」，所以照「連線消失」取消全部工作；審查量到外框只是把視窗藏起來、從來不斷線。改成照各自視窗的 golden FormClose。**我在 14:2x 回 St01 的那一句也跟著錯了**，已在 TO_STEVEN §4 更正。
- 端到端驗證前兩次判「失敗」都是**我的腳本錯**：一次沒帶數字 id、一次在執行中讀還在緩衝的 log；另外我一度說「指令被擋」，其實 `allowCmd` 從 0918 起就恆為 true，已當場更正。
- GitHub 第 72 包 README 我先寫了一句「套完機台的權杖會恢復擋人」，推之前發現錯了（三方合併會保留機台的本地修改），改掉才推。
