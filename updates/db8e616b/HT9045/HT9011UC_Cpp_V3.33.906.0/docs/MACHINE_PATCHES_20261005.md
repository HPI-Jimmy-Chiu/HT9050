# 機台端 patch 整合 1005（cpp 0202～0209、web 0103～0108）

> 接在 `docs/MACHINE_PATCHES_20261002.md`（第三輪，cpp 0047～0129、web 0045～0082）之後。中間兩輪沒有另寫這份檔：
> cpp 0130～0179 在第 53 批（1004 00:08 `5d6a27ae`，第 135 包）收進 main；cpp 0180～0201 在第 65 批（第 145 包）收進 main，逐顆帳本是 `docs/handoff/MACH0180_LEDGER_20261004.md`。
> 裁決照舊：RULINGS_20260930 第 11 條（衝突以機台為準）；機台的暫時繞道留在機台。
> ⚠ 筆電 1004 晚上到 1005 04:5x 只巡了 GitLab，**漏讀了機台 10/04 的兩次推送**（GitHub `machine/integ-ioweb` `d8abb7c` 20:58、`04048c0` 22:17）——
> 原因：這個 session 載入的 night-loop 技能是主 checkout 的舊版，沒有「§1.0 機台端優先：每輪先抓 GitHub」那一節（main 版有）。技能自己要求的「開工先比對主 checkout 與 main 的技能」那一步沒做。

## 0. 這一輪收到什麼

| 機台 patch | 是什麼 | 處置 |
|---|---|---|
| cpp 0202 `13eca55` | PKG-134：機台合筆電第 134 包 | 不收（是我們的包；機台的合併解法照它的 WORKLOG §3） |
| cpp 0203 `ecfa1a4` | PKG-135..140：機台合筆電第 135～140 包；**裡面夾帶機台自己的 TRAYSAFE-3**（照 St01 S-26：`mymotor` 的 `TrayArmMotorMove` 防呆改回原版，只有 HOME 的 `W906_HomeTrayArm` 在 HT9050 傳 `bCheckPos=false`，自動運轉照原版檢查） | 不收：包的部分是我們的；TRAYSAFE-3 收窄 golden 檢查（HOME 時不檢查 Tray 臂位置）＝機台繞道，留機台 |
| cpp 0204 `0540c57` | WORKLOG：135～140 包 ctest 失敗清單 | 沒收（見 §1 WORKLOG） |
| cpp 0205 `e820eea` | HT9050-TEACH-KB-LOADERY：Teach 的 Loader Y 小鍵盤在 HT9050 不夾上下限 | **留在機台**：改的是機台自己的 `FileRW/TeachKb.cpp`（機台 10/02 的 TEACH-KB，裁決 C），main 沒有這個檔；main 照 #51＝A 本來就不夾 |
| cpp 0206 `3800d02` | HT9050-TEACH-KB-ALL：Teach 全部欄位在 HT9050 不夾（開關 `W906_HT9050_TEACH_KB_CHECK` 預設 0） | **留在機台**（同上）；= NIGHT_REPORT §0 #99 機台自己解了 |
| cpp 0207 `fa29db7` | WORKLOG 10-04：§2 第 120～123 列、§4 空跑前必處理、HT9050 Out Shuttle 規則 9／10、Frank 待回 | 沒收（見 §1）；內容已轉 St01（W-62）、NB2-1（W-63） |
| cpp 0208 `33880da` | PKG-141..144：機台合筆電第 141～144 包；**`WebBridge/WebBridgeServer.cpp` 沒套**（!176 把 `act.observerSG.state` 加進免權杖，等 EastSun） | 不收（是我們的包）；main 維持 !176，等 EastSun |
| cpp 0209 `fef1f41` | WORKLOG 10-04：§2 第 124～125 列（第 125 列＝三個動作的唯讀查證＋約 14 項待 EastSun） | 沒收（見 §1）；內容已轉 St01（W-62）、NB2-1（W-63） |
| web 0103 `94b238e`、0104 `f6f923a`、0108 `af09ed6` | PKG：機台合筆電第 134、135～140、141～144 包的網頁那一半 | 不收（是我們的包） |
| web 0105 `9a0cc07` | HT9050-TEACH-KB-ALL 網頁那一半 | **留在機台**：改的是機台自己的 `page/ht9045_teach_kb_c.js`，main 沒有 |
| web 0106 `1554472` | TEACH-1PICKZ：單吸嘴機台 Teach 的 In／Out Arm Pick Up／Place Z 點重新顯示（EastSun 1004「我的ZA需要校正」） | **收**：第 66 批 `e55b4748`（git am，作者照機台） |
| web 0107 `1e95562` | TEACH-1PICKZ-2：HMI 外框裡也顯示（等整頁載完再判斷吸嘴數） | **收**：第 66 批 `84da524e` |

## 1. WORKLOG 鏡像（1005 05:4x 補上）

用 `resume_20261001/mach_chain.py` 把 C++ 歷史鏈從 `a64c46ff`（cpp 0129）接到 0209：0130～0187 → `9792ea5b`；**0188 跳過**（0187 重複匯出，MACH-0180 帳本已記）；0189～0209 → 鏈尾 **`2f49d2ca`**。每一顆都不帶模糊比對地套上。
鏈尾的 `docs/WORKLOG_MACHINE.md` 跟 main 比只多 20 行（main 沒有任何一行是機台版沒有的）⇒ 整份照鏈尾鏡像（第 66 批 `73971487`）＝等於收了 WORKLOG 0204／0207／0209 加上機台合包 commit（0202／0203／0208）寫的 §3 列。
（先前用 `git am -3` 單收 0204 會失敗，就是因為它的上下文是那些合包列。）

## 2. 下一次從哪裡開始

- 下一次從 **cpp 0210／web 0109／tools 0162** 開始。
- C++ 歷史鏈尾 **`2f49d2ca`**（cpp 0209）；web 鏈尾仍是 `78ee7510`（web 0084，0085～0108 還沒接——這一輪收的 web 0106／0107 是直接 `git am --directory=web` 套上的）。

## 3. 1005 上午到中午第二輪：cpp 0210～0217、web 0109～0111、tools 0162～0163（第 67 批 → main `61f514a8` → 第 147 包 GitHub `0947f76`）

| 機台 patch | GitHub 推送 | 收法 | main 上的 commit |
|---|---|---|---|
| cpp 0210 HT9050-DRYRUN（＋HT9050-TYPE、SHTSAFE、SHTCENTER、RULE10、FIXER）＋web 0109 HT9050-SITEPANEL | 08:11 `e12e19e` | 整合 agent cherry-pick（重建鏈 `bb824feb`）；不收：`SOFT_SIMULTE` 註解、launch.json、TeachKb 測試；帳本 `docs/handoff/MACH0210_LEDGER_20261005.md` | `5cf1fea6`、`59cbcc1b`、`e31f53e0` |
| cpp 0211 PKG-145、web 0110 PKG-145 | 09:48 `a87bc18` | 第 145 包本身（main 是來源）→ 跳過 | — |
| cpp 0212 OUTSHT2HOME-2 | 同上 | cherry-pick，乾淨 | `9574148e` |
| cpp 0213 HT9050-BRAKEPOWER | 同上 | `WebMotorAccessLive.cpp` 衝突：HT9050 分支照機台，非 HT9050 維持 main，機台 PKG-140 留下的 `goldenPower`（BRAKE-BOOT）不收 | `d7dacd17` |
| cpp 0214／0215 BRAKEPOWER-2（＋fix）、0216 WORKLOG | 同上 | cherry-pick，乾淨（WORKLOG 跟機台位元組相同） | `1342f459`、`68a32a87`、`74c8e0ae` |
| cpp 0217 PKG-146、web 0111 PKG-146 | 10:45 `b9a4337` | 機台 10:44 已套第 146 包（08:23 的暫緩作廢）；只收它套包時的決定：`C_OutArmSmallY`＝RULE10 的 152（303 那組註解掉）、HOME 1310 MLoaderZ 用 `W906_HomeMove`＋速度 100——**HOMEPOS0（`W906_HomePos`）照舊不收**（MACH0180 帳本 0192）；`test_flow9050_tray.cpp` 用機台版 | `09080fbb` |
| tools 0162／0163 HTDESIGNER | 09:10 `bf78d74`、11:42 `8d530c5` | 0162 套不上 main 的 0.163（樹已分岔）→ ES02 收成 0.164／0.165（`v906/es02-htdesigner`，main `8fd4e0a7`）；0163 交給 ES02 | — |

- 同一批另外收：Ifor !182（I-08）、NB2-1 !195（R230）、Ifor !198（W-61 b）。gate b67b：ship＝基準 4＋cJSON（F-Secure 11:31 誤判隔離，非回歸）、sim＝基準 19；4 支逾時單獨重跑都過；absence／pagewire 13／numcmp PASS。證據 `D:\HT9045\backup\night_tools_20260927\resume_20261001\_g_b67b_notes.txt`。
- 第 147 包＝GitLab `61f514a8` 對 `bfb30b76`（第 146 包）62 檔、基準 44 檔；README 第 147 包那條寫了 Jimmy「不要擋」（RULINGS_20261005 第 1 條）與套包前的 MLoaderZ 檢查。

## 4. 下一次從哪裡開始（1005 13:5x）

- 下一次從 **cpp 0218／web 0112／tools 0164** 開始（tools 0163 歸 ES02）。
- C++ 歷史鏈尾 **`ac3721c5`**（cpp 0217；`mach_chain.py` 產出的鏈是 LF 換行，比對時忽略 EOL）；web 鏈尾仍是 `78ee7510`（web 0084；0085～0111 都是直接 `git am --directory=web` 或屬於包本身，沒接進鏈）。
- GitLab 上 `v906/jimmy-machine-0210`（`bb824feb`）是給 St01／St02 審查用的機台 0210 樹，第 147 包推完後可刪。

## 5. 1005 下午：cpp 0218（第 68b 批 → main `44b2b9ce` → 第 148 包 GitHub `f38cae6`）

- **cpp 0218 HOME-AUDIT1005**（EastSun「你檢查一下全機回HOME 還有沒有BUG」）：cherry-pick 成 `74399ee8`（作者照 patch）。uhome.cpp 兩處衝突：①1310 逾時訊息照機台的兩行寫法（點名 flag19）；②1530 的「最多重跑 1300～1530 三次，然後停下並寫出 Tray Arm 位置」做成 `W906_Home1530GiveUp(waitPos)`，套在 golden 的 Color／Empty 兩個等待位置（main 沒有機台的 TRAYWAIT／HOMEPOS0 那層，`W906_TrayArmHomeWaitPos` 只在機台）。
- **tools 0163**（HTDESIGNER，版面設定跟著外掛走）：ES02 做成 0.166.0（`9cf03b94`），1005 14:1x 合進 main `cc1bb9c0`，隨第 148 包。
- 同批：St02 !174、Jerry !200／!202（11:54 那版）、NB2-1 !203、test_cJSON 版本資訊、WB_WsLink [12] 與 FastClk_Jobs 上限校正（兩顆都只動測試）。

## 6. 下一次從哪裡開始（1005 15:3x）

- **第 69 批**（機台優先）：cpp **0219** JOG-VLTIME-2（鏈 `f462761c`）、**0220** WORKLOG 第 123 項＝派工給 Jimmy（`fd413256`）、**0221** HOME-PHASETIMER（`b3f4694e`）；加上機台派工 `dispatch/20261005_dryrun14_indexz` 的 **Index Z 兩個高度**（`indexz_outsht_cpp.patch`／`indexz_outsht_web.patch`，機台寫好、在 `7d58cf2`／web `8bd78b0` 上驗證可套、沒 commit、沒上機）。
- C++ 歷史鏈尾 **`b3f4694e`**（cpp 0221）；web 鏈尾仍是 `78ee7510`；下一次從 **cpp 0222／web 0112／tools 0164** 開始。
- 機台套包進度：機台最後整合的是第 146 包（PKG-146）；第 147 包 13:27 推出、第 148 包 15:4x 推出，都還沒看到 PKG（`pkg_uptake.py` 超過 3 小時會提醒）。
