# 夜間報告 2026-09-29（二）晚起 —— **進行中版本**（每推一批整份更新；最後更新 22:1x）

> 使用者 0929 17:5x 下班：「完成手頭任務且無任務情況下，自動切換到Loop周末下班任務」；18:0x：「不用限制，我如果需要停止Loop會主動說明白」
> ⇒ **沒有收尾時間**，一直做到使用者說停；早上使用者在也照跑。迴圈：cron `4ca5c42b`（每小時 :07／:27／:47，session 閒置才觸發）。
> ⚠ session-scoped：視窗關掉、或筆電被公司 AutoTools 強制關機（`night-loop` 技能 §5.1，0928 17:08 發生過）就會中斷；狀態全在 git 與本檔。
> 0924～0929 週末那一輪的舊報告在 git 歷史（`f455a765` 之前的版本）。

### 怎麼讀

| 節 | 內容 |
|---|---|
| **§0** | **要你決定／處理的**（只放真的需要你的） |
| §1 | 做完了什麼（附證據與 commit） |
| §2 | 佇列（照 INBOX 111→117 的順序） |
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
| 1c | **引擎馬達走 1203：停止指令送不出去時，要不要照 golden 跳 WAR16122 警報** | golden 停止失敗（StopDec／ExtDrive 失敗）會跳 WAR16122，機台停下來等操作員。路由不能在自己裡面跳（會卡住主迴圈），所以目前做法是：失敗的停止**每一軸都記下來、wb_serve 開著就不清掉**，在 `/api/struct/motor/...` 的 `diag.why` 看得到，console 一定印。<br>例：某軸 StopDec 被卡拒絕 → **A**：只記錄＋畫面看得到（現在）；**B**：另外在主迴圈外面補跳一次 WAR16122（會開一個要按的警報框）。 | **A（現在的做法）**：路由預設關著、上機才開，B 要跟 EastSun 一起決定警報框的時機 | `docs/ENGINE_1203_MOTOR_ROUTE_DESIGN.md` §9.2 第 3 項 |
| 2 | **GitLab 上的 MR !7 要在網頁上按「Close」** | St01 和筆電都沒有 API 權杖，只能用網頁關；它已經被筆電的直接合併取代 | 放著不影響任何東西 | `gitlab.honprec.com/.../ht9045/-/merge_requests/7` |
| 3 | **Jerry**：他能不能推公司 GitLab、測的是哪一版 | 決定用 git 交接分支還是寄 patch | 等你問 | 18:1x 那則回覆 |
| 4 | **機台端 session 不在線** —— 「引擎馬達走 1203 改由筆電做、機台不要改那 5 個檔」只寫在 GitHub README 最上面 | 如果機台上今晚有人在開發，最好口頭說一聲 | — | GitHub README 最上面 |

> 已經照安全預設做、不用你回的：**St01 的 review6 先不合進 main**（St02 20:24 審出 2 個機台安全的 major：頁面重新整理就送整台 StopAllMotor、生產中也會；Exit 期間 Light Scale 停不下來）。等 St01 修好再合（TO_STEVEN §4 20:4x）。

---

## §1 做完了（0929 下午到晚上）

| 時間 | 事項 | 證據 |
|---|---|---|
| 21:4x | **St02 MR !6 第三批上 main**：S72 尾巴（golden `SetLotStart` 的 `slEventLog->SetLotData` 照跑）、S-12 模擬組態的加熱執行緒（`HeaterSimTick.cpp`，只在 SOFT_SIMULTE 下；你 16:1x 選 A）、S-09 `cprod.cpp:3009` 閘退役 | merge `927740a5`；兩組態 gate 剛好＝基準（出貨 4、模擬 19，243 個測試）；GitLab `d40fa5a0`；GitHub 第 76 包 `e72da27` |
| 21:1x | **INBOX 117 Motor View／Motion View 畫面驗證**（你 19:1x：「確認 motor view 畫面是否能正常顯示並且更新數據」「確認 motion view 畫面是否能因為 motor 位置不同而更新內容」「可以透過今天跑模擬的方式驗證」）。**兩個都原本不行，都改好、都用模擬驗過。**<br>① **Motor View 原本數字不會動**：頁面只在開頁時讀一次 `JSON/Motor-runtime.json`，C++ 根本沒在寫這個檔（`D:\HT9045\web\JSON` 那份是 09-02 的快照，provider「TBD」）。改成讀 C++ 的即時值（`/api/struct/motor/runtime`），每 500 ms 更新一次（跟 Motor Test 頁一樣）。<br>② **Motor View 的 Current 欄有幾軸一直是 0**：原本優先顯示 encoder，模擬時 `MOutArmX` 的命令位置是 -995899、encoder 是 0，畫面就一直是 0（`MOutArmY`、`MTestZ1` 也一樣）。改成照 golden `UpdateMotorScreen`（`main.cpp:8363-8384`）：顯示命令位置，只有 4 個光學尺軸跟 Galil 的 Index 軸才用 encoder。<br>③ **Motion View 原本手臂完全不動**：頁面其實有畫手臂位置的程式（`liveMech()`），但沒有人把馬達位置餵給它（St02 的 `ht9045_mv_trays.js` 只送托盤格子）。新檔 `web/page/ht9045_mv_motor.js` 每 500 ms 把命令位置餵進去。<br>**實測數據**：模擬 S1 流程（HOME→Lot Start→START，20:53～21:01）API 的位置一直在變（`MInArmX` 0→30732→4203→35399→5326；`MTrayX` 0→39662→61662→96912→4562）；改之前 Motion View 整段 SVG 從頭到尾一模一樣。頁面層級的直接證據（不重新開頁，只改 API 回應）：Motor View 表格 0/0 → 11111/22222 → 33333/44444；Motion View `MInArmX`=0 → 30000，SVG 有 28 行不同（例：入料臂橫樑 x 69.1→219.1），改回 0 又一模一樣。<br>⚠ **這台筆電的 `Gerneral.ini` 機種是 `HT-9045W`**，所以 HMI 開的是 HT9045 版的 Motion View；HT9050 版（`Main.MotionView9050.html`）目前沒有任何即時資料來源（頁面自己寫「wb_serve 無 producer」），它的手臂圖也還不會動——要做就是另一件事（交給 St02，見 §2）。 | `9c0c3209`、`4f1a080d`、`4de9813e`；量測資料 `%TEMP%\claude\…\scratchpad\mv117\`（api.jsonl 233 筆、pages.jsonl、page_probe2.json） |
| 21:1x | **INBOX 111 常溫接線第二層（四組）上 main**：<br>A 出料臂 `PCIL112_OutArmXYMove` 照 golden、`MotorMoveShuttleShake` 生效；B `GetOutShuttleStatus_9045` 照 golden；C 入料臂 4 個過期閘退役、`InArmSuckReset`、Clean Out 收尾、`DoInArm_SuckerMap`／`DoSiteMappingResult` 真本體；**D 整機退料 `DoTrayFeedProcess` 第一次接上**＋`TfMain::InitialTrayFeedTask`。<br>審查抓到的都處理了：DoTrayFeed 裡 AMR 等待（G21）與 P53 的 MESxx24（G22/G23）三個閘理由過期 → 照 golden 打開；`SYN_TEK_MOTION_MODULE` **刻意維持 0**（golden 預設 0xA7 會讓 1203 機台走 M204 latch 模式，但 latch 感測器在移植樹還是離線替身，出料飛梭殘料判斷會讀到空資料）；`GetInArm2DIDMapping` 暫停呼叫（本體 85% 還在閘裡）。沒做的列在 INBOX 111b。 | `1b726051`、`78a4e58f`、`6c0c27e9`、`290ecfa8`、`d707083d`、`40386775`；gate：出貨＝4 項基準、模擬＝19 項基準（3 項負載逾時／exe 被拿走，單獨重跑都過） |
| 20:4x | 回 St02：`SetOEEState` 放 St02 自己的 `LogObjects.cpp` 可以；review6 暫不合（見 §0 註） | `533b7c8d` |
| 19:5x | 回 St02：S-10 Tray Edit 筆電那 3 處（CMakeLists／wb_serve 分派／替身 Close）同意 | `5d27de65` |
| 14:24～19:14 | S-08 伺服器端、St02 MR !6（兩批）、WSLINK-B、原生畫面（OFF）、機台 OPLOG／SR-WIRE、St01 MR !8、常溫接線靜態盤點（79 列） | 見 §5 |
| 18:5x | **`WB_ENGINE_MOTOR_1203` 維持預設關、上機再開**（你的決定，RULINGS_20260929 第 14 條） | — |
| 18:3x | **0018 TOKEN-OFF 不收**（你的決定，RULINGS_20260929 第 13 條） | INBOX 116 結案 |

---

## §2 佇列（照 INBOX 111→117）

| 順序 | 項目 | 狀態 |
|---|---|---|
| 1 | **INBOX 112 引擎馬達走 1203** | 🔧 22:4x 修正完成（l2e `d13494b9`：HIGH／MEDIUM 修好並有測試，14 個存活突變都抓到了，新碼 9 個突變也抓到；開關照舊關），複審 agent 在跑，過了就合進來跑 gate。原本：實作完成（l2e `f5341505`，開關預設關）。三路對抗式審查：**HIGH** —— Motor Test／警報路徑直接送出的停止，路由的帳本看不到 ⇒ 武裝後可能把「中途被停」的移動記成到位、把中途被停的回原點記成完成；**MEDIUM** —— 停止失敗沒有任何警報（golden 會跳 WAR16122）；突變測試 32 個活下來 18 個（連「確認路由保持關閉」那一條都沒被測到）。**21:1x 修正 agent 在 l2e 做**，修完兩組態 gate 綠才上 main |
| 2 | INBOX 113 INDEXZ-1203 重做（Index Z1） | 等 112 進 main（兩邊都動 `wb_serve.cpp:4298`、CMakeLists、MachineType.h） |
| 3 | INBOX 114 PumpInit 寫死的機台設定 | 🔧 22:0x 做完（`b8d6c956`，12 個全退役；量過這台與 HT9050 今天的值都跟寫死值相同 ⇒ 行為不變；AMR 對接互鎖因此恢復），gate 在跑 |
| 4 | INBOX 115 HIGH 那 92 個 `#if 0`（避開 ST02 已認領的） | 等 |
| — | INBOX 111b（第二層審查留下的 8 項後續：`DoOutArmSuckPreOn`、k4-G2／G9、P57、2DID、16 吸嘴越界、`fMain->Pause`／`SetLotState` 空殼、`WriteIniData` 空巨集、SYN_TEK） | 排在 115 後面 |
| — | **HT9050 版 Motion View 接即時資料**（`Main.MotionView9050.html`：沒有 producer、軸讀數表的值固定「–」） | 已在 TO_STEVEN §4 21:0x 告知 St02；St02 也被請看 `liveMech()` 的 `/100` 換算（模擬時 `MOutArmX`=-995899 會畫到畫面外） |
| — | Kevin K-01（golden 模擬錄 State Record，常溫、FT005054_9050）／K-02（9050 Index BCB6 流程） | Kevin 18:04 認領 K-01，之後沒有新推送（21:3x 量：3.5 小時；K-01 沒擋到筆電的工作，所以不收回） |
| — | St02 S-10 Tray Edit、(A) SetOEEState／SaveASECLTestLogInfo、S-12（加熱，只接模擬） | St02 在做；S-10 等 review6 |
| — | **上機清單**（要人在機台旁）：G14 TCP_IP_MODE＋ON_LINE 的 SOT 有沒有真的送出；引擎路由第一次上機（EastSun 在旁）；一個瀏覽器一條連線；**這一包的出料臂 XY、整機退料、Out Shuttle 殘料判斷** | 等上機 |

---

## §3 紅燈／哨兵

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

## §6 我自己犯的錯與更正

- **22:2x INBOX 114 那行開機訊息，第一版根本沒編進去**：我把 `printf` 接在 `InitAllProcessTask();` 那一行**原本的行尾註解後面**，整段變成註解（記憶裡早就記過「同一行附加要插在第一個 `//` 之前」）。是用短的模擬 wb_serve 抓那行字抓不到才發現的；已移到註解前面，前處理輸出確認有那個字串，重跑兩組態 gate。今晚其他同一行附加逐一看過：程式碼都在註解前面（托盤的 `#if 1`），其餘本來就是註解行。
- **21:0x 我一度把「Motor View 會更新」當成證據**：那一輪的探針每次都重新開頁，數字會變是因為重讀，不是頁面自己在輪詢；而無頭瀏覽器的 `document.hidden` 是 true，頁面的輪詢根本沒跑。同一輪「Motion View 畫面變了」也不能歸功給轉接檔。改用「告訴頁面它看得到＋不重新開頁、只改 API 回應」重量一次，才拿到上面 §1 那組數據。
- **WSLINK-B 第一版的前提錯了**（14:2x）：以為關 Motor Test 分頁會斷線；已照各自視窗的 golden FormClose 重做，TO_STEVEN §4 已更正。
- 端到端驗證前兩次判「失敗」都是我的腳本錯（沒帶數字 id、讀了還在緩衝的 log）；我一度說「指令被擋」，其實 `allowCmd` 從 0918 起就恆為 true，已當場更正。
- GitHub 第 72 包 README 我先寫了一句「套完機台的權杖會恢復擋人」，推之前發現錯了才改掉。
