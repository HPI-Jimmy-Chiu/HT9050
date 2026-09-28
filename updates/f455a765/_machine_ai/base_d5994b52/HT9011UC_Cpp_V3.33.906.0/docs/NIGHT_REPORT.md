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

## 🔁 20260928（一）09:5x 機器關機後接手 —— **先看這一節**，再看下面 0927 的交接

> 0927（日）19:3x 左右機器被關機，週末迴圈（cron 每小時 :07＋9/29 07:57 收尾）跟著消失；最後一筆 session 紀錄 19:28。
> 0928 09:5x 使用者：「找回最後執行的任務並且繼續 Loop 周末下班任務」⇒ 新 session 重掛：cron `0bea332b`（每小時 :07，每輪結尾用小表回報進度＋未完成）、`8bc3a641`（9/29 07:57 收尾）。T＝9/29（二）09:00 不變。
> 關機時在跑的：FLOW-1 第二批的準備 workflow（3 個 agent）＋良率警報閘的準備 agent。舊 session 的暫存區**還在**（重開機沒清），產出直接拿來用：
> DoAllProcess 前段（做完）、task 初值＋網頁 HOME（做完）、比對工具 v2（做到一半）、良率警報閘（剛開始）。後兩件 0928 10:0x 重派。

| 項目 | 0928 10:3x 的值 |
|---|---|
| GitLab main | `8b5a91b5`（沒變） |
| 本機（`v906/jimmy-main0925`）領先 main | 7 顆：`54a9711c` 文件、`8495c8d9` N3-G5、`f513ffb8` RESUME 44、**FLOW-1 第二批** `b1b4fe36` B1（DoAllProcess 前段整段照 golden）、`d97c07ee` B6b（G15／G17）、`0a3d433e` B6（9 個 task 初值）、`816ce9b2` B5（WS `main.home`）；另有交接檔回覆（本顆） |
| 兩組態 gate（第二批） | ship：建置 OK，失敗＝基準 3 項（`config_db`／`config_loaders`／`GA1_ReadGeneralIni`）＋ 2 支 BAD_COMMAND（`SjsonAlarm`、`WB_WsProto`：連結完 exe 就不見了，Defender 近 3 天沒有紀錄 ⇒ 懷疑公司 EDR；重新連結後單獨跑 2/2 過）；sim：跑中 |
| 對抗式複驗 | workflow `wf_24aa0af3-780`（4 個 agent：兩個唯讀複驗第二批、比對工具 v2 補完、良率警報閘準備）跑中 |
| 執行期目錄 | gate 前 sysguard `opmode`：system／config／IniData 0 變動；`D:\HT9045_Log` 只有 QtyData；`D:\HT9045\Error` 不存在；沒有在換設定 |
| 哨兵 | 配方 65／64（＝新筆電基準）；pagewire `--selftest` OK；⚠ **pagewire 實跑 14 項紅**（見下） |

**⚠ pagewire 實跑 14 項紅（之前幾晚只跑了 `--selftest`，所以不知道是不是新的）**：兩類 ——
① `D:\HT9045\web\page` 的幾支接線 js 跟我們產生的 `tools\pagewire\wire\` 不一樣（例 `ht9045_wire_yieldmonitoring.js`）；
② `sync_web.py` 的 OURS 清單少了 St01／St02 這幾天加的 25～27 支接線 js（「下一次 sync 會把它們刪掉」）。
另外量到：**主 checkout `D:\HT9045` 停在 `0b506e82`（0926 12:25），落後 origin/main 588 顆**；F5 與這個哨兵讀的都是那一份。主 checkout 是你的 F5 環境、還有你本機改過的 `config/config.ini`，所以我沒動它。
⇒ 在誰跑 `sync_web.py` 之前都不會真的刪檔；OURS 清單排進待辦（我們的 `tools\websync\sync_web.py`；web repo 那份是 Steven 的，要在 TO_STEVEN 講）。

### 0928 11:3x 進度（接手之後）

| 項目 | 狀態 |
|---|---|
| FLOW-1 第二批 | ✅ **已推 main `77cac68a`**：兩組態 gate＝基準（ship 3 項＋2 支 EDR 隔離、重連結後過；sim 18 項，跟基準逐項相同）；對抗式複驗兩份都「可以套、沒有擋路的」（F1 HANA／F2 GPIB ART 兩個 major 只影響開了那兩個功能的機台，排下一顆修，見 INBOX 第 100 列） |
| GitHub 更新包 | ✅ **第 60 包** `updates/77cac68a/`（12 檔；權杖／私鑰 0、7z 密碼 0；README 註明 HANA／GPIB ART 的已知限制） |
| 第 43 題（合 St01 到 `0bca1318`） | ✅ 已在本機合（`a6f5022d`，52 顆、93 檔、0 衝突）；兩組態全量 gate 跑中，綠了推 main＋開 MR |
| 比對工具 flowcmp v2 | ✅ 本機 commit `5925d69d`（Python，不進建置）；還沒對真的 wb_serve 跑 |
| 第 40 題功能清單 | ⏳ workflow 3 個 agent：對照檔 210 個有值的 task、463 組「task＋步驟」逐一查 C++ 是 LIVE／GATED／STUB／MISSING／UNREACHABLE |
| 良率警報閘（GATE #5／F／G／I） | 暫存區準備好（兩組態 syncheck 0 錯）；要跟 `csystem.cpp` 的 G11／G12（Initial Start 清計數）一起解，排在功能清單之後 |
| R113 | ✅ golden 查完，答覆寫進 TO_STEVEN §4（建議同協定擋、跨協定不擋，請 Steven 用例子再確認） |

**⚠ 0928 10:13 機台設定被換掉 → 已修回（使用者 0928 11:0x：「這台設定被我改掉，你幫我修復成9050專用，我原本是要把該工單複製過去」「IniData 是生產參數，需要對齊。其他相關依據你之前設定為主，尤其是要設定成9050專用」）**
1. 發生：10:13:29～30 對照檔 `Staterecord\2025-12-11 17_47_57\HT9045\` 整份原地覆蓋 `system`（改 73 檔、多 1 檔）、`config`、`IniData`（多 FT005054 21 檔）、`setup.inf`。
   怎麼發現的：第二批 sim 的 ctest 剛好跑在那之後，`ini_helpers` 讀到 `HEATER_CTRL_TYPE=2`（原本 4）而紅；sysguard 對 10:05 快照比出 73 改＋22 新。⇒ 那一輪 sim 結果作廢、重跑。
2. 保存：換掉後的整份存在 `D:\HT9045\backup\gate_sysguard\swapped_0928_1013\`（2,394 檔＋setup.inf），什麼都沒丟。
3. 修回（10:4x，`backup\night_tools_20260928\restore_opmode_sysconfig.py`）：`system`／`config` 照 10:05 快照逐檔還原、MD5 全對（73 檔）；對照檔多帶的 `system\lastdata_backup2.dat` 搬到 `backup\night_quarantine\20260928\swap_extra\`（有 MANIFEST.tsv）；**`IniData` 保留對照檔的 FT005054**（生產參數要對齊）；`setup.inf` 暫時改回 `T6-SIQ-PT43-BGA25X25-4-25-FT1T0`（INBOX 第 968 行記的原值）讓第二批的 gate 跟基準比得起來 —— sim 重跑＝基準 18 項逐項相同。
4. **下一步：裝成 9050 專用**（St01 合併的 gate 跑完就做；腳本 `backup\night_tools_20260928\install_ht9050_config.py`，dry-run 已看過）：
   `machines\HT9050` 的 `Mot_Table.csv`（原本是 HT9045 的 SMC 表 33 軸 → 9050 的 1203 表 19 軸）、`Pci1203Axis.ini`／`Pci1203Io.ini`（原本沒有）、`IO_Table.csv`（本來就一樣）；
   `Gerneral.ini` 照真機改兩個卡別鍵 `IO_CARD_TYPE` 2→4（PCI1203_IO）、`MOTION_CARD_TYPE` 1→0；`setup.inf`＝`FT005054`。機種看 `D:\GPIB9045\system\general.ini` 的 `Model=9046_32GPIB`（跟 9050GPIB 一樣解成 HT9046_LS，第 25 條）不用改。
   裝完重量一次兩組態 gate 當新基準（ctest 會讀這份設定）。
   ⚠ `machines\HT9050` 正本比機台上的舊（INBOX 第 13 列：機台端 EastSun 改過 IO_Table 421 列 Lane、Mot_Table 7 列 BoardID/Port、Pci1203Io.ini 站號 176/177/178，全文還沒傳回來）。這台筆電沒有卡，不會真的動；要跟機台完全一樣要那 4 個檔。

### 0928 12:3x 進度

| 項目 | 狀態 |
|---|---|
| St01 合併（第 43 題） | ✅ 推 main `8b3a07af`＋MR !4＋GitHub 第 61 包（兩組態＝基準） |
| 9050 專用設定（第 1 題 A、第 2 題 A） | ✅ 機台 EastSun 的 5 個檔（`D:\HT9045\Staterecord\other`）裝進筆電；4 個進 repo 正本 `machines\HT9050`（`d7a375ff`），`Gerneral.ini` 有廠商密碼不進 git；新基準快照 `ht9050m_0928`；ctest 新基準 ship 4／sim 19（`backup\night_tools_20260928\*_base_9050.txt`） |
| ART 影子副本（INBOX 第 100 列 F2） | ✅ 推 main `fbfe9033`＋GitHub 第 62 包；HANA（F1）照 golden 不改、只補註解 |
| 功能清單（第 40 題） | ✅ `docs\FLOW_COVERAGE_FT005054.md`：LIVE 245／GATED 13／MISSING 17／UNREACHABLE 17；托盤全活；**AutoClean 跑不完**（FLOW-2 在補） |
| FLOW-2 | 托盤替身＋手臂守衛兩份 patch 已備好（暫存區 `flow2\standins_tray`、`gates_arms`）；AutoClean／InArm 空殼／Tester EP 三份還在準備；全部到齊後一次 gate |

**第一次 S1 動態對照（9050 設定＋FT005054，不換設定，12:05）**：`main.home` ✅ HOME 照 golden 跑完（WAR2207→WAR2208）；
起動模式「Initial Start」不在這台（`CUSTOMER_CODE` 957）的清單裡；**START 被 golden 的教導點位檢查擋下**（`cinitial.cpp:10477-10487`
「The teaching of fix tray is mistake」：機台設定的 `FIX3_FULL_PLACE=0`，筆電舊的 `teach.ini` 的 Fix2／Fix3 X 過不了）。還原後逐檔 CLEAN。
想在跑的期間借用對照檔那台的 `teach.ini`，被自動模式的安全分類器擋下（「不可逆的本機破壞」—— 會覆蓋真機教導資料，中途斷線就回不來）⇒ **停下，留給你決定**（§0 第 44 題）。

### 0928 15:3x 進度 —— FLOW-2（照功能清單補「動作流程功能」）

⚠ **重要發現**：移植樹的「入料臂放料到 Shuttle」原本就不完整 —— 6 個相關函式是空殼（`InArmZNeedDown_9045`、`GetInArmZShtDownPos_9045`、`SetShuttleStatus_9045`、
`CheckInArmDestroyActive`…），而且 `GetShuttleCol` 對每個吸嘴都回 0。所以不只 AutoClean 跑不完，正常生產放料到 Shuttle 也不會照 golden 走
（Z 不下降、資料不會交給 Shuttle）。FLOW-2 全部照 golden 補上，對抗式複驗 3 份（逐位元組比 golden、追 NULL 指標、查真實檔寫入）抓到的 1 個 blocking＋2 個 major 也都修了。

| 批 | commit | 內容 |
|---|---|---|
| FLOW-2 托盤 | `5d1694a4` | Loader／Auto 的過期替身退役（Tech 點位、續跑時 Auto 盤資料、出料紀錄、色感測器…） |
| FLOW-2 手臂 | `216f7bd8` | DoInArm／DoOutArm 5 個守衛閘；InArm 放料到 Shuttle 6 個空殼照 golden 翻；OutArm 4 個 golden 本體（本來就在 `#if 0` 裡） |
| FLOW-2 AutoClean | `44f36926` | 6 個 `if(false)` 閘（W7d-I1）、速度推送閘（W7a-I4）、`InitDoTestZHome`＋`TestZTask`、`ACSmartClearData`；測試配套 |
| FLOW-2 Tester | `df23bc65` | `EPSwitchOnOff`／`EpSwitch`（golden adam6024.cpp）、`DoTestHeadMotor` 前段 `CheckIndexConnect` |
| FLOW-2 修正 | `f78f5a2c`～`7a547214` | `GetShuttleCol` 完整階梯（blocking）、`IsCheckOutArmDestroyActiveFinish`、`ptrInSHT` 守門（第 45 題）、過期註解、SortingBinTray 路徑走 log 根目錄、測試第二層守門 |
| FLOW-3 良率 | `dd6c8a00` | 良率監控 5 個閘、csystem 3 個重複替身、G11／G12（Initial Start 清計數）、`DoInitialICCheck` 的 `ClearAutoSiteOffStatus`、新 Fix 盤的 `SaveUnloaderInfo` |

gate（到 `df23bc65`）兩組態＝9050 基準；含修正與 FLOW-3 的 gate 跑中，綠了一起推 main＋GitHub。

**新的決策題**（寫在下面 E 節第 43 題）：St01 分支要不要現在合進 main（Steven：「ok，但是 jimmy 還沒上班」）。

---

## 🔁 20260927（日）16:4x 交接 —— 換帳號接手的人**先看這一節**（原 session 算力用完）

> 使用者 16:4x：「算力快沒了，要確實做好紀錄，讓另一個帳號接手處理」。原 session 從 09-26 跑到 09-27 16:4x，下面是它停下時的**完整狀態**。

### A. 現在的狀態（量過的）

| 項目 | 值 |
|---|---|
| GitLab main | ~~`93284329`~~ ⇒ **0927 19:1x 接手 session 推到 `8b5a91b5`**（St01 合併 `53a55b35`＋MR !2、RULINGS 第 8 條、FLOW-1 第一批）；GitHub 到第 59 包。接手 session 的進度見下面 D 節開頭 |
| 工作樹 | `D:\HT9045\.claude\worktrees\m0925`，分支 `v906/jimmy-main0925`，推法 `git push origin HEAD:main`（先 `git fetch` 確認 fast-forward） |
| GitHub 機台更新包 | 到第 55 包（`D:\HT9045\backup\github_HT9050`，README 表格最後一列）；做法見 C |
| 樹上未 commit | 無（只有 `??` 的未追蹤檔，不是我們的） |
| 正在跑的 build／ctest／wb_serve | 無 |
| 換設定對照（flow swap） | 沒有在換（`D:\HT9045\backup\flowswap_ACTIVE.json` 不存在）；`system`／`config`／`IniData` 對 opmode 快照 0 變動 |
| 迴圈 | 原 session 的 cron（每小時 :07 `/night-loop`、9/29 07:57 收尾）是 **session 範圍的，換帳號就沒了**。要繼續跑請在新 session 重掛：`/loop 20m /night-loop`（或每小時），T＝9/29（二）09:00 不變 |
| 兩組態 gate 基準 | ship 失敗＝`config_db`／`config_loaders`／`GA1_ReadGeneralIni`；sim 失敗＝`D:\HT9045\backup\night_tools_20260927\sim_base.txt` 那 18 項；測試數 214 |

### B. 今天（09-27）做完、已推的（照順序）

RSMODE `5997abda`（起動模式下拉＋WS 指令 `main.runStartMode`）→ ZSAFE `a4408436`（Z 軸互鎖）→ SSMD `ff639260`（起動模式清單＋開機種回）→
ARM1 `4b2ec4f3`／ARM2 `e1dbad7c`／ARM3 `15df16ab`（出料臂吸→找盤→移→放）→ 合 St02 `ed7df426` → ARM1b `14d274ea` → IDXERR `ebc8e41c`（Index 位置錯誤停機）→
SMHOME `23c264b9`（單軸回原點）→ ARM4 `e593dede`（Fix3 盤滿）→ R76 `00f9a882`（網頁單選鈕分組）→ INADD `7a78e2ba`／OUTADD `1f425fa2`（入／出料臂附加功能）。
每一顆都跑過兩組態 gate＝基準、sysguard 0 變動；細節在 §1 第 54～65 列。GitHub 第 49～55 包。

### C. 接手要知道的工具（原 session 的 scratchpad 會消失，已複製到 `D:\HT9045\backup\night_tools_20260927\`）

| 工具 | 用法 |
|---|---|
| `gate_m0925.sh` | 兩組態 gate（ship→sim），輸出 `_m0925_gate.out`／`_m0925_ship.log`／`_m0925_sim.log`（**在腳本裡把 `S=` 改成新 session 的 scratchpad**）。用 `run_in_background` 跑；**停它要停整棵程序樹**（`taskkill /T /PID <build.bat 的 cmd>`），只 kill bash 會留孤兒 ctest（§6 0927 12:0x） |
| `syncheck.py <build_dir> ALL <src>` | 用 ninja 的編譯參數 `-fsyntax-only` 一個檔（連結缺口看不到，要 gate） |
| `syncheck_copy.py <build_dir> <src 片段> <副本>` | 同上但檢查 scratchpad 裡的**副本** —— gate 在跑時也能先量改動 |
| `flow/golden_span.py <golden 檔> <函式名>…` | 從 golden 量函式範圍（函式頭到結尾大括號）。**不要手抄行號**：今天手抄錯 3 次、NB2 R105 錯 7 處 |
| `flow/*_apply.py` | 今天每一顆的套用腳本（照 golden 逐行翻到檔尾＋原地退休樁＋gates.json），可當範本 |
| `sysguard.py check opmode`（`SYSGUARD_DIRS=system,config,IniData`） | 每次 gate／實跑後必查 0 變動；另查 `D:\HT9045_Log` 只有 `QtyData`、`D:\HT9045\Error` 不存在 |
| `flow/flow_swap.py <tag> 600 60,200,400,580 FLOWPROBE AIPROBE` | 換真機設定（`D:\HT9045\Staterecord\2025-12-11 17_47_57`）跑動作流程對照，跑完換回並驗 CLEAN；比對用 `flow/flow_cmp.py`。repo 版在 `tools/flowcmp/` |
| GitHub 包 | `mkpkg57.py <上一包 rev> <新 rev> D:\HT9045\backup\HT9050_update_<rev8>`（下一包請照 `mkpkg56→57` 的 sed 換 base 產生 mkpkg58）→ 複製到 `github_HT9050\updates\<rev8>` → README 補一列 → `gh_scan.py` ＋ PowerShell 數 7z 密碼（只印數量，必須 0）→ commit → push |

### D. 下一步（照優先序；第 32～35 題使用者 0927 16:5x／17:1x 都裁了，RULINGS_20260927 第 7 條）

> **⚠ 0927 17:2x 起接手 session 的更正與進度（RULINGS_20260927 第 8 條：動作流程對照排第一）**
> 1. **下面第 1 項的前提是錯的**：第 33 題不是「沒有料流」的根因。golden 模擬開機時把幾乎所有感測器 `Enable=false`（golden `cinitial.cpp:2531-2543`），關掉的感測器 `IsOn()`／`IsOff()` 都回 false、根本不讀 IO（golden `mysensor.cpp:87-91`）——輸入讀成亮或暗都一樣。golden 模擬的料是從 `DoLoad` 靠 `CheckBox1`（「Loader 有盤」）進來的，移植樹這條鏈是活的。第 33 題照裁決仍然要做（忠實度），**排在流程對照的基準量完之後**。完整盤點在 `docs/SIM_CAMPAIGN_PLAN.md` 檔尾 §11。
> 2. **對照組一盤都沒有跑完**（見 E 節第 40 題）。先比 S1 那一段（HOME → Initial Start → InitialICCheck → 一次 AutoClean → 生產開始 4 秒）。
> 3. 真正擋住流程對照的（研究 `staterecord-flow-gap`，逐項附 golden 行號）：**已做**——`chkHeaterOk` 沒照 dfm 預設勾選（入料臂一直回 1 的根因，`91c81d2b`）、升降／Auto 狀態機開機沒初始化＋task 紀錄的 [3][7] 位址（`a99f00ff`）、J1（`8b5a91b5`）；**進行中（FLOW-1 第二批，暫存區準備中）**——`DoAllProcess` 前段那一段整段是 `#if 0` 摘要（InitialICCheck／AutoClean 永遠不會跑，golden `csystem.cpp:9165` 起）、幾個 task 初值跟 golden 不同、網頁沒有「HOME」指令（真機是先按 HOME 再 START）、開機「要不要讀上次資料」被閘成一律讀（真機操作員答「否」）、比對工具 v2（視窗、前段比對、附 golden 行號的允許清單）。

1. **第 33 題＝照 golden：模擬時沒被寫過的輸入一律「亮」**。這是動作流程對照「沒有料流」的根因（NB2 R112：golden 讀卡失敗走 `#ifdef SOFT_SIMULTE return true`）。
   做法：改 `IOBackend.cpp:52` `TSimIOBackend::ReadBit`（與 ReadByte）—— 沒被寫過的輸入位址照 golden 回「亮」；被模擬指令明確寫過的照寫入值；
   先查清楚移植樹的輸出與輸入是不是共用同一張表（golden 模擬時輸出失敗是靜默的、不會影響輸入）。**單獨一顆**、兩組態 gate 逐項比；
   sim 組態的 ctest 基準會變，照 pt-wave-loop「測試期望值照鷹架校準」附 golden 行號重校；然後重跑 `flow_swap.py` 看料流有沒有進來。
2. **第 32 題＝照 golden：面板實體鍵全部接**（golden `TfMain::ScanKey` main.cpp:2379-2656、`TimerScanKeyTimer` :31994-32025、開機 :9617 開 timer）。
   START 一定走網頁 `start.run` 同一條 `StartFromWeb`（`WebStart.cpp`），**不可**改 base `TfMain::Start`；做完用 `tools/start_sites_census.py` 重量普查數字，
   更新 CLAUDE.md「StartFromWeb」那一列與 `WebStart.h` 檔頭。注意 tick 迴圈是單執行緒（記憶 v906-tick-loop-is-single-threaded）。
3. **模擬物流要補進計畫書** `docs/SIM_CAMPAIGN_PLAN.md`（新的一節）：golden（BCB）在 `SOFT_SIMULTE` 下 IC 怎麼從 Loader 流到出料盤、每一步靠什麼成立、移植樹差在哪。
   已寫好的研究 prompt 可原樣重跑：`D:\HT9045\backup\night_tools_20260927\workflows\ht9045-q32-q35-sim-research-*.js`（4 個唯讀 agent：
   Index 通道、golden 模擬物流、第 33 題設計、第 32 題面板鍵；**0927 17:1x 因算力不足被停掉，沒有產出**）。Index 通道那一個已經不需要（第 35 題已裁）。
4. **第 34 題＝照 golden（不跳過）、第 35 題＝A**：已在 TO_STEVEN §4 告知 St01，筆電這邊不用改程式。
5. `test_wb_crypto.exe` 在 `Obj\V906\build_ship\tests\` 不見了（OUTADD 那輪 ship 的 `WB_Crypto` BAD_COMMAND）：重建看會不會又被刪（懷疑防毒／EDR）。
6. St01 `v906/steven-cbridge-review6` 領先 main 202 顆：TO_STEVEN §4 16:2x 問了要不要合，等他們 §2 回。Q40 `form.event` 送出點與 #17（Exit／Ctrl-C）都要等那支進 main。
7. INBOX 第 88 列 RSMODE-2、第 89 列那幾項。
8. 每一輪照 night-loop：`git fetch`、讀 `origin/v906/steven-handoff` 的 `FROM_STEVEN.md` §3、看 `origin/v906/nb2-assist` 最新 R 號。

### E. 等使用者的決策題

第 32～35 題 0927 16:5x／17:1x 都裁了。**0927 19:0x 新增 4 題**（接手 session 整理 Steven 0927 下午的問題與 9/26 那 40 封信時找到的；使用者已下班，**裁決前一律維持現狀**，照常推進流程對照）：

| # | 題目 | 白話＋例子 | 選項（**建議**） | 裁決前 |
|---|---|---|---|---|
| 36 | 🔴 瀏覽器全關 15 秒，運轉中的機台會安靜停下來、沒有警報，要不要改？（St01 17:20） | C++ 用「視窗總表」判斷 Teach／Motor Test 有沒有開，規則是「15 秒沒收到瀏覽器心跳＝不知道＝當成開著」（`WebWindowRegistry.cpp:235`，筆電 0920 寫的）；EastSun 0925 把 golden「Teach／Motor Test 開著時主流程暫停」接到這個判斷上（`csystem.cpp:30493-30494`）。合起來：例如夜班把瀏覽器關掉去吃飯，機台跑到一半停住、沒有警報；打開網頁又自己繼續。golden 不會這樣，因為運轉中根本打不開 Teach（906 `main.cpp:27827`）。跟你 0927 第 18 題 B「主畫面斷線不暫停生產」相反 | **A** 只改主流程用的那支判斷（EastSun 的 `WebMotorAccessLive.cpp:1140-1143`）：全部過期時照最後一次回報，START 的「不知道就不准」不變（約 6 行＋1 個 ctest）／B 改總表規則（START 也一起放鬆）／C 不改、停住時跳警報／D 維持現狀（EastSun 原意：寧可停） | 不動；GitHub 第 57 包已提醒機台運轉中留一個 HMI 分頁 |
| 37 | 🟡 運轉中，網頁「工具／設定」選單底下的頁能不能打開？（St01 R89） | golden 運轉中按這兩個選單第一行就 return（906 `main.cpp:28031`／`:28010`），Bin 設定、Setup、Contact 都開不了；我們的政策表 §2 寫「運轉中不會鎖住整個 HMI（例：Bin 設定）」。網頁開頁不只是看：會重讀 config.ini／lastdata.dat（`tools/wb_serve.cpp:5200-5202`）、Contact 頁會補寫 Gerneral.ini | **A** 照 golden，運轉中不開（St01 `cb306f89` 照合、政策表改寫）／B 運轉中可開唯讀版（新工作）／C 只放行幾頁沒副作用的 | 現狀（St01 那顆還沒合） |
| 38 | 🟡 DoTrayFeedProcess 解閘時，要不要帶進 V912 的「QA 結束時同步並存 Tester 連線狀態」？（St01 D5-4） | V912（RogerYang 0629）在 QA 做完恢復原模式那兩段，把 `LastSet.iTester=…` 換成 `fMain->ModifyTester(…)` 並補 `SaveTestMode()`（V912 `csystem.cpp:11352`／`:11362`／`:11870`／`:11882`），註解寫「避免 ReadLastDataFile 讀回 Off-Line」；906 沒有。例：QA 前是 On-Line，QA 做完按 TrayEnd，906 只改記憶體，之後有人重讀檔就可能讀回 Off-Line，下一批不跟 Tester 要結果 | A 照 906（符合 0926 Q2「底層照 906」）／**B** 當成 Q2 的例外帶進那 4 行（先例：第 28 題） | A（現狀，那段還閘著） |
| 39 | 🔴 機台內有 IC 時，要不要照 golden 擋「On-Line／Off-Line 切換」（MES1646）？ | St02 翻 `ChangeTesterConnect` 時，照一個轉述的使用者裁決（`docs/TESTERCOMM_PORT_LEDGER.md:278` D2「先不做」）沒翻 golden 的「機台內有 IC 就拒絕」，但 RULINGS 裡找不到這條。SECS 的 S2F41 也會呼叫它（`uHGemHT9045.cpp:3431`）。例：機台裡還有測到一半的 IC，Host 遠端切成 Off-Line，golden 會拒絕、移植樹會照切（運轉中的 `SystemStart` 保護仍在） | **A** 照 golden 補上 MES1646／B 維持不做（請確認轉述的裁決是你的） | 維持不做 |

**動作流程對照的三題**（研究 `staterecord-flow-gap`，0927 19:0x；裁決前照「建議」那欄的預設做）：

| # | 題目 | 白話＋例子 | 選項（**建議**） | 裁決前 |
|---|---|---|---|---|
| 40 | 🔴 **對照組一盤都沒有跑完**，「跑一到兩盤後跟對照組一樣」要拿什麼當對照組？ | `2025-12-11 17_47_57`（FT005054）那台每次一開始生產就停在 WAR0170「Device superfloat at hot plate error!」（EventLog 17:35:39 起四次，hot plate 上的 IC 浮起來，實體問題），入料臂從沒從 Loader 取過料；之後是警報、IO 頁、改 offset、開門、POWER OFF。所以這份紀錄裡**沒有任何一盤**可以比。能比的只有 S1 那一段（17:30 HOME → 17:35 第一個 WAR0170：HOME、Initial Start、InitialICCheck、一次完整 AutoClean、生產開始 4 秒） | **A** 請機台那邊用同一個工單乾淨地跑一到兩盤，跑的過程中多存幾次 State Record（Task List 環每個 task 只留 500 筆，要分段存）／B 在暫存區複製一份 golden 906，用 BCB6 建模擬版錄 golden 自己的 Task List（要動 BCB 的副本，跟 RULINGS_20260927 第 1 條「只改 906 C++」有點擦邊，且這台要能建 BCB6）／C 只比 S1 那段就算驗收 | 先把 S1 那段做到一樣 |
| 41 | 🟡 對照時開機那一題「要不要讀上次的資料」怎麼處理？ | 真機操作員 17:28:38 答「否，放棄上次資料」；移植樹這個開機提問被閘住、一律當成「是」（`cinitial.cpp:9882-9950` GATE n2-18）⇒ 機台裡帶著 17:47 收尾時的盤與 IC，Initial Start 被 golden 自己擋掉（「Auto區有tray盤, 請先執行tray feed」） | A 照 golden 把開機提問接到網頁對話框（忠實，但開機時還沒有網頁連線，要設計）／**B** 對照工具換進真機設定後，只在那份副本裡把 `machinerecord.dat` 第一個欄位（`bInitialStart`）改成 0，走 golden「不讀上次資料」那條路（可逆，跑完整個換回）；A 排進佇列 | B |
| 42 | 🟡 跑整盤時測試機怎麼模擬？（第 40 題選 A 或 B 之後才需要） | 這個工單是 GPIB 模式（`Tester.Data` Tester Type=1），golden 要找得到 GPIB 橋接程式才會測（`atester.cpp:1424-1437`）；golden 唯一內建的模擬 bin 是 TTL 模式的 `Sim_TTL_Single`（移植樹閘著） | **A** 用行程內的 GPIB 模擬（St02 的 TesterComm，每站 bin 預設 1）／B 改成 TTL 模式並解閘 `Sim_TTL_Single`（要改配方）／C OFF_LINE | 等第 40 題 |

**0928 新增**：

| # | 題目 | 白話＋例子 | 選項（**建議**） | 裁決前 |
|---|---|---|---|---|
| 43 | 🟡 St01 分支 `v906/steven-cbridge-review6` 到 `0bca1318` 要不要現在合進 main？ | Steven 0928 08:2x：「Q52. ok，但是 jimmy 還沒上班」。St01 那支比 main 多一大批（開頁等級、close-tail、Configuration／Speed／Temp_Set 的畫面事件…），St01 自己的兩組態全量 gate 是綠的（0927 22:31）。例：合進去之後，運轉中打不開「工具／設定」底下的頁（第 37 題 A 的做法就在裡面） | **A** 筆電現在合：本機兩組態全量 gate 綠才推 main、開 MR 留紀錄／B 等你上班再合 | B（「還沒上班」可能是要等你，我不猜） |
| 44 | 🔴 **S1 動態對照要用哪一份教導點位（`system\teach.ini`）？** | 機台只傳了 5 個設定檔，沒有教導點位。用 9050 的機台設定之後，筆電舊的 `teach.ini` 過不了 golden 在 START 前的檢查（例：Fix2 的 X 要比 Fix1 大 10000 以上，Fix3 也要比 Fix2 大 10000 以上），START 被擋、流程跑不下去。 | **A（建議）** 請 EastSun 把機台的 `D:\HT9045\system\teach.ini`（和 `tech.dat`）也放到 `D:\HT9045\Staterecord\other`，我整份裝進筆電／B 只在模擬跑的期間借用對照檔那台（HT9046_LS，同一套機構）的 `teach.ini`，跑完照快照還原（自動模式會擋，要你在場或加權限）／C 兩者都不要，只做靜態功能清單 | 等你；先做 FLOW-2（不碰執行期設定） |
| 45 | 🟡 **（已照安全預設值先做，請事後確認）AutoClean 續跑時的 `ptrInSHT` 是 NULL** | 機台重開之後第一次 AutoClean 從中間續跑（case 100→540），程式要讀「現在在哪個 Shuttle 的夾具」，但這個指標還沒被設過。golden 一樣有這個洞，只是 BCB6 會把存取違規攔下來、跳錯誤框繼續跑；移植樹的 wb_serve 會整個當掉，重開又再當（清潔墊資料開機會讀回來）。 | **A（已做，`3084d2b6`）** 只在指標是 NULL 時，照 golden 其他地方的對應（Shuttle 1 → FLCarryKit、其他 → BLCarryKit）補上，其餘照 golden／B 照 golden 不補（會當機）／C 改成跳警報並停住 | A（已做，可逆：拿掉那一行就回到 golden） |

另外兩件**只要你確認**（不影響今晚）：① St02 帳本裡有幾條寫成「使用者裁決」的（例：0926 14:3x「906 為底、補上 912，目標是 912」），RULINGS 裡沒有，是不是你講的？是的話我補進 RULINGS。② ELA 計畫 §5-1「Observer 的 Event log 分頁要不要下架」標著「Jimmy 決定」（`docs/ELA_PORT_LEDGER.md:773`），還沒裁。

### F. 9/26 Steven 寄來的信 —— 讀過了嗎？（使用者 0927 17:0x 要求註明）

* **Outlook 實際量到**（Outlook COM 唯讀匯出，0927 17:0x）：9/26 Steven 寄來 **40 封**，全是「[HT9045 Gitlab 推送通知]」（00:01～19:58），**Outlook 裡 40 封都是已讀**（使用者讀過）；9/27 Steven 沒寄信（改用 git 的交接檔）。
  同期使用者只寄出 1 封（9/26 10:08 給研五軟體的 main 更新通知），**沒有用 email 回 Steven**；使用者對 Steven 各題的回覆都是當面裁決、記在 `RULINGS_20260926.md`／`RULINGS_20260927.md` ⇒ **兩邊衝突時以 RULINGS 為準**。
* **內容**：這 40 封是 Steven 推 `v906/steven-gpib-widget`／`v906/steven-handoff`／`v906/steven-cbridge-review6` 時的通知信，內容跟那幾支分支的 commit 訊息與 `FROM_STEVEN.md` 同一份。
  筆電這兩天是從 git 讀這些（每一輪 fetch＋讀 FROM_STEVEN），St02 那幾批（S-04 cMyDB P0～P2、測試通訊 GB P0～P7、P2a～P2f、ESD G5、ELA…）都已經合進 main（0926 `79060249`、0927 `ed7df426` 等）並在 TO_STEVEN §4 回過。
* **還沒逐封對帳**：原本開了一個唯讀 agent 把 40 封逐封對照 repo（有沒有漏回、有沒有沒合的 commit、有沒有值得擷取的資料），0927 17:1x 因算力不足被停掉、沒有產出。
  **接手的人請重跑**：信件內文已匯出到 `D:\HT9045\backup\night_tools_20260927\steven_mails_0926.txt`（UTF-8，40 封，已拿掉登入連結），
  對帳的 prompt 在 `D:\HT9045\backup\night_tools_20260927\workflows\ht9045-steven-mail-0926-audit-*.js`（把裡面的路徑改成新的 scratchpad 或直接指到 backup 那份）。
  對帳時的規則：信裡提議 X、使用者裁決 Y ⇒ 以 Y 為準；有用資料擷取到 INBOX／NIGHT_REPORT。
* ⚠ 收件匣裡另有兩封 Anthropic 的登入連結信（9/26 Maurice 轉寄、9/27 16:54），**不要**把連結抄進任何文件或 commit。

---

## ☀ 20260926（六）下午 12:4x～ 進度 —— （歷史，接手請先看上面的交接）

### 要你決定的（三題）—— ✅ 0926 14:0x 已答：1＝A、2＝B、3＝A（RULINGS_20260926 第 27 條）

| # | 事情 | 白話＋例子 | 選項 | 建議 |
|---|---|---|---|---|
| 1 | 🔴 **R66-GALI**：第 2 條「甲」（引擎的 Z1 改走 1203）要不要連 `Gali_*` 那一層一起接 | golden 的 Index 流程（下壓測試、AutoClean、歸零、停止）**不是**呼叫馬達物件，而是直接呼叫 `MOT[MTestZ1].Gali_MotMove(...)` 這類 Galil 專用方法，全樹 105 個活的呼叫點，前面都沒有「是不是 Galil 卡」的判斷。例：就像把車換成電動車、也接好充電線，但油門踏板其實接在一台不存在的汽油引擎上 —— 只做第 2 條原案，START 之後 Z1 的生產移動還是到不了 1203。 | **A**：在 `Gali_*` 這一層依軸的 CardType（PCI1203）分流到 `Motor->`，改一個檔涵蓋 105＋處；B：105 處逐一改；C：只做原案 | **A**（機台端和第 2 條一起做） |
| 2 | 🟡 **R66-D13**：歸零時「Index 有沒有離開原點」檢查會讀到停用軸寫死亮著的 Home 燈 | HT9050 只有一支 Z（Z2 停用）。開了 D13（`bCheckIndexHomeSensor=1`）的話，停用那支永遠「沒離開原點」⇒ 歸零一直重來。預設是 0，所以今天不會發生。 | **A**：跟 R63 A 同一個做法，停用的軸不檢查；B：照 golden（HT9050 不能開 D13） | **A** |
| 3 | 🟡 第 9 條：告警框期間蜂鳴器要「一律叫」還是「照每個警報碼的靜音設定」 | 你 Q3 答「告警框期間蜂鳴器一律叫」。golden 其實還有一層：每個警報碼可以在安全設定頁勾「靜音」（`GetJemSilent`，例如客戶把某個常見的 WAR 碼設成不叫）。移植樹目前沒有這一層需要的欄位（fNote 的 sJamArea／sJamCode），我照你的字面做成**一律叫**。 | **A**：一律叫（現在的做法）；B：照每個碼的靜音設定（要先補那兩個欄位，約半天） | **A**，等有客戶真的用到「單碼靜音」再做 B |

另外 NB2 R66 提醒機台端兩件事（不用你決定，已記在 INBOX 第 51 列）：第 6 條必須在 `HSys.LoadMotData()`（cinitial.cpp:3874）**之前**生效，否則 Index 四軸會被建成停用的 SMC 軸；R63 A 不夠 —— Z1 變成 1203 軸後伺服燈也要從 1203 監看器取。

### 新的要你決定的（0926 14:3x～0927 00:2x，十六題）

| # | 事情 | 白話＋例子 | 選項 | 建議 |
|---|---|---|---|---|
| 4 | 🔴 第 8 條：VacuumUnit 頁的吸／破真空鈕在 HT9050 上要不要能用 | golden 只有 `Gerneral.ini [System] VacuUnitType` 不是 0 才顯示這一頁（HT9050 是 0）。golden 這兩顆鈕寫的是頁面裡**寫死的位址**，研究量到它在 HT9050 的 IO 表上剛好對到 **Loader／Auto1 的托盤氣缸**（`C_Load_Up`、`C_Auto1_Up` 等，IP 80/81、port 16-19）。照寫死的位址做，網頁按「吸」可能讓托盤氣缸動 | **A**：照 golden，`VacuUnitType=0` 時 hw.access 直接拒絕（HT9050 網頁上這兩顆鈕不能用）；B：HT9050 也要能用，改走 IO 表綁定的吸嘴點（偏離 golden，要先逐點確認位址） | **A** |
| 5 | 🟡 第 8 條：塔燈頁「試聽」期間要不要暫停警報音樂 | golden 塔燈設定頁開著時會暫停所有警報音樂（`ckernel.cpp` ShowRunLed 的 `fTowerLight->fShow` 閘），好讓試聽不被蓋掉。網頁照做的話，按試聽後的 30 秒內（Q4 的自動停）機台真的出警報也不會叫 | **A**：照 golden，試聽期間暫停警報音樂（最多 30 秒；斷線或權杖還掉時提早停）；B：試聽中一出警報就停止試聽、恢復警報音樂（偏離 golden，較保險） | **B**（機台是實彈，寧可不漏警報） |
| 6 | 🔴 **V912 量產碼**：`OCR.dfm` 少了 `rgOCRTriggerMode`（St01 14:32 報，筆電已查證） | 這個「OCR 觸發模式」選項是**我們 0522 在 V899 加的**（`AI(ht9045-v899) 20260522`），公司整併進 golden 906（0618）與 V912（0908）時只帶了 `.h`／`.cpp`，**沒帶 `.dfm` 那 12 行**（V899 `OCR.dfm:479-490`）。後果（BCB6 的規則推論，沒有在機台試）：V912 有裝 OCR（`INSTALL_OCR≠0`）的機台，一開 OCR 頁就跳 Access violation；按存檔在 `OCR.cpp:2181` 當掉，後面幾個鍵沒存到 | **A**：筆電把 V899 那 12 行補進 V912 的 `OCR.dfm`（照 dfm 格式、同一個父元件），**這台沒有 BCB6，要在有 BCB6 的電腦私有建置驗證**，再請公司確認有沒有已出貨的 OCR 機台；B：只通知公司（電話）由他們修；C：先不動 | **A＋電話通知**（改 `.dfm` 要你明說，所以先問）。⚠ Steven 14:3x 的判斷是「不是很重要，記為待辦」（skill 待辦 G-006） |
| 7 | 🔴 網頁存檔在機台運轉中要不要擋（St01 F-003） | golden 的設定畫面（Setup／Config 那排按鈕）只有停機時按得到（`main.cpp:3842-3847` 運轉中把那兩排藏起來），所以運轉中改不到工單參數。網頁的 `editlist.save`（各設定頁的存檔）**沒有檢查運轉狀態**；S88 之後溫度頁存檔會照 golden 跑 `SetWorkParameter` —— 運轉中從網頁存檔，後果跟運轉中換配方一樣 | **A**：C++ 的 editlist.save 一律擋 `SystemStart||SoftStart`（同 IO 頁的規則），簡單、最安全；B：逐頁比照 golden 那一頁在運轉中能不能開（要先列清單，少數 golden 允許運轉中改的頁會被 A 誤擋）；C：先不擋 | **A**，之後真的有「運轉中要改」的頁再逐頁開 |
| 8 | 🟡 防連點（Steven 新規則，St01 在 wb_serve 做）：Motor Test 的動作鈕要不要依動作細分 | St01 的做法是「同一個指令 400 ms 內再來一次就回『忙』」，但 `motor.access` 整條列白名單（因為按住的 jog 本來就會連續送）。這樣「相對移動」「伺服開關」連點兩下還是會**真的動兩次**（例：相對移動 10 mm 連點 ⇒ 走 20 mm；伺服開關連點 ⇒ 開了又關）| **A**：依動作細分 —— jog／stop 放行，move／home／power 400 ms 內重複的擋；B：整條放行（現狀）| **A**（`WebMotorAccess.cpp:50-60` 已有動作清單（moveRelative／home／servoToggle…），擋錯的代價只是「再按一次」）|
| 9 | 🟡 防連點：IO 頁／面板的輸出鈕要不要也擋 | 面板鈕走「輸出優先」的快速路徑（`wb_serve.cpp:6172-6198`），St01 的 400 ms 擋不到。連點兩下＝切兩次＝回到原狀（例：想開夾爪，連點後又關回去）| **A**：同一顆鈕 400 ms 內只算一次；B：不擋（現狀）| **A** |
| 10 | 🟡 防連點：START／LOT START 的頁面要不要加第二道 | 伺服器已擋 400 ms 內的重複；頁面目前沒有「按下到收到回覆之前按鈕反灰」（`ht9045_opbuttons.js`、`ht9045_lotstart.js`）。START 回覆要等機台檢查完，常常超過 400 ms，操作員以為沒按到就再按 | **A**：頁面也鎖住按鈕直到回覆（或逾時）；B：只靠伺服器 | **A** |
| 11 | 🟡 配方存檔後的 MD5：`/api/recipe`（網頁存配方）要不要跟設定頁一樣更新 | St01 的 S92 接上 golden `BackupSetupFile` 後，15 個設定頁存檔會照 golden 刪掉舊的 `*.MD5`、寫新的；但網頁的 `/api/recipe` 存配方不會 ⇒ 同一份配方，看你從哪裡存，MD5 一個有更新一個沒有（主機端比對 MD5 的客戶會以為配方被動過）| **A**：`/api/recipe` 寫完也呼叫同一個本體；B：不動（兩條路不一致）| **A**（golden 機台上任何存檔都會更新 MD5）|
| 12 | 🟡 警報框／是否框開著時，網頁的「軸停止」（`pci1203.ax.stop`／`emgStop`）要不要能執行（NB2 R70-STOP） | 框開著時 golden 的其他畫面按不到，所以移植樹一律回「框開著，請先答框」；只有 Motor Test 的 `motor.stop` 例外放行。（NB2 R70 抓到第 9 條讓框裡的 1203 Poll 放行了網頁 IO／停止命令，筆電已改回一律拒絕）| **A**：維持拒絕（照 golden，現在的做法）；B：`ax.stop`／`emgStop` 也像 `motor.stop` 一樣放行 | **A**（停止鍵本來就有 `motor.stop` 那條路）|
| 13 | 🔴 **V912 量產碼**：`_8Site1X4` 機種存 Setup 頁時，Contact.Data 的 Test Arm1／2 **Drop 會被寫成 1，不是 2.00**（St01 報、筆電查證） | V912 `main.cpp:28494-28495` 寫 `WriteIniData(szDir, "Test Arm1", "Drop", "2.00")`，可是 `WriteIniData` 有 bool 版（`common.h:77`）—— C++ 會把字串常值當成 bool（true）選到 bool 版，所以寫進去的是 1。例：這種機台每按一次 Setup 存檔，Drop（下壓落差）就被寫成 1、接著 `fContact->ReadFile()` 讀進去的也是 1 | **A**：V912 改成 `AnsiString("2.00")`（改量產碼，要打包給客戶）；B：不動（現在的出貨行為，客戶機台一直是 1） | 先確認 `_8Site1X4` 有沒有客戶在用；有就 **A** |
| 14 | 🔴 **安全門鎖要不要照 golden 在運轉中鎖上**（NB2 R71 DOOR） | golden 設定 `SAFE_DOOR_LOCK=1` 的機台，運轉中會把門鎖（`SwSafeDoorLock`）鎖上、停機才開（main.cpp:3051-3064）；移植樹**從來沒有**在運轉中驅動它 ⇒ 運轉中門可以打開。例：HT9050 現在照第 24 條暫時關掉了門感測器，EastSun 在機台上測試時可能需要在運轉中開門 —— 一照 golden 補上，運轉中門就開不了了 | **A**：現在照 golden 翻（主迴圈與框等待都跑，同 golden）；B：先翻好但 HT9050 在第 24 條那段測試期間不驅動、上線前再開；C：先不動 | **B**（跟第 24 條同一個「上線前要恢復」清單）|
| 15 | 🟠 **開機時加熱器繼電器寫不到 1203 卡**（NB2 R72 OPM-1＋R75-BOOT） | 開機那一刻 1203 卡還沒開，程式送出「開加熱器電源繼電器」其實沒送到，卻記成「已開」；之後每次要開都先問「開了沒」，答案永遠是開了 ⇒ 整個工作階段繼電器都是斷的。只影響 **ATC 主動冷卻**的配方（例：用 ATC 配方開機，IO 頁 DO 是 0、程式以為是 1；golden 自己的註解 main.cpp:12966 就是這個症狀）。選項：**A（建議）** 卡開好之後先用卡上實際的輸出校正程式的記錄，再照 golden 重跑一次 `UpdateMainOperateMode`；**B** 記下開機沒送出的寫入、開卡後照順序補送；**C** 照 golden 先開卡再初始化（代價：開機時瀏覽器最多空白 90 秒，牴觸 Q34-1）。開卡那一段是機台端的區域，選好後由 EastSun 做 | NB2 R75 §0、R72 §2；INBOX 第 67 列 |
| 16 | 🟠 **golden 的加熱執行緒要不要接上**（NB2 R75-HEAT） | golden 每 20 ms 跑 `DoHeaterOn`／`CheckHeater`，會重新確認加熱器繼電器、熱風槍、風扇；移植樹從來沒有啟動它（全樹沒有 `new THeaterThread`）。例：配方要加熱，golden 會一直補開繼電器與風扇，移植樹只在切模式那一刻送一次。**A** 照 golden 接到主迴圈（會真的驅動 HT9050 的 `SwHeaterRelay`／`SwHeatGun`／`SwHeaterFan`，要機台端在場）；**B** 先不接。**建議 A**（忠於翻譯，也會順便治好第 15 題）| NB2 R75 §0 |
| 17 | 🔴 **按 Exit 要不要讓 C++ 完全停工**（Steven 裁決 S121，屬你的機台流程） | Steven：「要通知 c++ 完全停工，馬達跟加熱都要關掉，安全第一」。golden `TfMain::FormClose`（main.cpp:11852-12444）關站時會停所有馬達、關加熱執行緒、Index 馬達煞車、加熱器繼電器與風扇、ADAM EP 歸零、ATC OffLine、ESD 停止、`InitialOK=false`，再存 machinerecord；移植樹關站只寫 `Program Close=1`，**Ctrl-C／關主控台視窗那條也一樣，輸出可能停在最後狀態**。St01 會從 `FileRW/MainClose.cpp` 呼叫已有真本體的停機函式、把替身清單貼出來；**停機本體多在我們的檔**。選項：**A** 照 golden 順序整套翻（Exit 與 Ctrl-C 都走）；**B** 只 Exit 走、Ctrl-C 維持現狀；**C** 先不做。**建議 A**，但要你在機台旁驗一次（會真的關輸出）⚠ St01 23:55 已先把 Exit 的停機順序接上（只呼叫有真本體的，替身標「未停」，而且 `--seconds` 到期也會跑一遍）；**還沒有真本體、要我們補的**：EP 歸零（`ADAM_WriteVoltage` 等是空殼）、`W906_CheckKitSuckNormal`、`EndMainThread` 的 NULL 檢查、Galil "RS"／MN200 `mn_stop_line`、ATC 停止與離線、Ctrl-C 那條（三個等待迴圈要看退出旗標）| FROM_STEVEN §3 22:20／23:55；INBOX 第 70 列 |
| 18 | 🔴 **關分頁／重新整理時要不要停馬達**（Steven 裁決 S122，比 golden 嚴） | ① 關掉 **Teach／Motor Test** 分頁 ⇒ 標成「必須重新 find home」（golden 關這兩頁只按停止、存位置，不要求重新 home）；② **主畫面**斷線或重新整理 ⇒ 生產中先暫停馬達，網頁重新連上後再繼續（要自動接續還是操作員按 Start，Steven 還沒回）。偵測可以用視窗總表（`ui.windows.put`，connId 已修好）。選項：**A** 兩條都做；**B** 只做 ①；**C** 都不做、維持 golden。這是**刻意偏離 golden** 的新規則，而且會讓機台停，所以要你定 | FROM_STEVEN §3 22:30；INBOX 第 71 列 |
| 19 | 🟡 **「Socket 接觸次數超過」警報碼要照 V912 改成 `WAR0357` 嗎**（NB2 R77，R68-MYDB） | V912 把這個警報從 `WAR0354` 改成 `WAR0357`（`WAR0354` 已經被「RTC 半視野」用掉＝撞號）。移植樹只搬了一半：發警報照 906 用 `WAR0354`（`atester.cpp:4338`／`:4454`），開機建碼表（St02 的 P3）照 912 插 `WAR0357`。**今天一般機台看不出差別**（機台的 `AlarmCodeList.txt` 兩個碼都有）；只有機台缺這個檔時，接觸次數警報會被顯示成「RTC Alarm Arm 1 NG」。選項：**A（建議）** `atester.cpp` 兩處也改 `WAR0357`（三方一致、解掉撞號；⚠ SECS／MES 看到的碼會變，但跟已出貨的 V912 一樣）；**B** 碼表改回 906；**C** 維持現狀 | NB2 R77 §0 |
| 20 | 🟠 **wb_serve 要不要拒絕「把機台資料轉到別處」的環境變數**（NB2 R79） | 程式裡有 19 個 `W906_*` 環境變數，原本是讓 ctest 把讀寫導進沙盒用的；有設的話，正式的 wb_serve 會改讀／改寫那個路徑，**完全不提示**。其中 `W906_IOTABLE_PATH`／`W906_MOTTABLE_PATH`／`W906_GENERAL_INI_PATH` 是 IO 表、馬達表、機台設定本身。wb_serve 目前只拒絕 `W906_INIDATA_ROOT`（配方）一個。**例**：機台上為了跑 `ioweb_probe` 在 cmd 視窗設了 `W906_IOTABLE_PATH`，接著在同一個視窗開 `HT9045_Web.cmd`（會繼承環境）⇒ wb_serve 改用那份 IO 表。⚠ **NB2 說「git 裡沒有腳本會設」是錯的**：機台端 F5「IOWEB(這台)」（`.vscode/launch.json:109-121`，另一組 :149-161 也一樣，NB2 R81 補）**刻意**設了 12 個，把 SetUp.inf、config、teach、log 導到 `../runcfg` 副本 ⇒ 照 R79 改成一律拒絕，機台的 F5 會起不來。選項：**A（建議）** 正式啟動器（repo 根的 `HT9045_Web.cmd`／`HT9045_Web_Debug.cmd`、`server\run_wb_serve.cmd`）在啟動 wb_serve 前把這 19 個清空（`set W906_IOTABLE_PATH=` …），F5 不受影響；**B** wb_serve 一律拒絕（R79 原案），機台 F5 要改成不用轉向（SetUp.inf／config／teach 會開始讀寫正本）；**C** 維持現狀。今晚先做了不改行為的一半：開機時把「目前有設的轉向變數」逐一印出來（見「做完了」第 35 列） | NB2 R79；筆電 0927 01:0x 量到 launch.json |
| 21 | 🟡 **網頁的觀察頁（Observer）要不要照 golden 檢查權限等級 5**（St01 01:30 ⑤，Steven todo Q42） | golden 從主畫面工具列開 Observer 之前有 `if(fSecurity->Insufficient(5)==false) return;`（906 main.cpp:27933-27936；V912 :28935-28938）；Greatek 專屬的保養紀錄路徑（906 :24833）不檢查。網頁的 `web/background.html:565` `MODAL_POLICY` 只有 6 列，而 :484 寫明「那張表就是 page-access-policy.md §3 的六列，**不自行擴大**」⇒ 加一列就是改政策表。**例**（依 St01 的描述，筆電沒另外量網頁別處有沒有擋）：網頁 MODAL_POLICY 沒有 observer 這一列，等級不夠的人也打得開 Observer、按 Yield 的 Clear 清掉良率圖；golden 要等級 5 才進得去。選項：**A（建議）** 照 golden：page-access-policy.md §3 加第 7 列 observer＝等級 5，St01 照表接；**B** 維持現狀（網頁不擋）。 | St01 FROM_STEVEN §3 01:30 ⑤ |
| 22 | 🟡 **V912 量產碼 `mtRowAMouseUp` 可能越界寫入，要不要修**（St01 01:30 ①，量產碼，筆電沒動） | golden（V912 cObserver.cpp）點 Yield 分頁的 site 格子時，沒檢查 `ConvertIndexCells` 回傳的 -1：點在 1 像素的格線上時 Y 會留在像素值，接著寫 `bShowYieldSeries[Tag][Y-1]`。**例**（St01 算的）：860×200 的格子、YItem＝5，點到格線時 Y 留 77，寫到 `[row][76]` —— 陣列外。後果看那塊記憶體是什麼，可能沒事、也可能改到別的全域。網頁只送格子中心，**移植樹重現不了**；只有 BCB6 的機台上用滑鼠點才會中。選項：**A（建議）** 下一版 V912 在那一段加「-1 就 return」（兩行，`ht9045-v912` 做）；**B** 不修（機台上沒回報過）。 | St01 FROM_STEVEN §3 01:30 ① |
| 23 | 🟡 **Handler 要不要照 golden 自己啟動舊的 EventlogAnalyzer.exe**（St02 ELA G8，St02 待裁決 #31） | golden 912 在 O10「使用 EventLogSaver」打勾時，會自己啟動 `d:\EventlogAnalyzer\EventlogAnalyzer.exe`（main.cpp:18598-18630），某些情況還會關掉再重開（:18447-18490）。V906 這段在閘裡；St02 另外把分析器做進了 wb_serve（網頁 `/api/ela`，開機就啟動，今晚合進 main）。⇒ V906 從來不啟動舊 exe：Handler 送的事件紀錄指令會進 wb_serve 裡的分析器，舊 exe 只有被人手動打開時才收得到。舊 exe 還有六項工作是新版沒有的（FTP 上傳、客戶報表等，St02 #22）。**例**：某客戶機台 O10 打勾、靠舊 exe 每天把報表 FTP 回客戶 ⇒ 換成 V906 之後，沒人手動開舊 exe 就不會上傳。選項：**A（St02 建議）** 維持關著，只用新版分析器，那六項暫時沒有；**B** 照 golden，O10 打勾就啟動／重啟舊 exe，新舊兩個一起跑（同一筆指令兩邊都處理，會不會互相干擾沒量過）；**C** 正式下架舊 exe，六項等新版補上（#22）。我的建議：**A**，另把「O10 有打勾的機台」列進上線前的確認清單 —— B 的兩個一起跑沒人量過，C 要等 #22 | St02 FROM_STEVEN §3 03:40 |
| 24 | 🟡 **Handler 要不要照 golden 自己啟動 ESD 程式、並在程式開著時替 IonBar 送上電指令**（St02 ESD G5，St02 待裁決 #32） | golden 912（:18690-18732）在設定了 ESD_Monitor／NOVX3360／KASUGA_Fan／HT IonBar、又找不到 ESD 視窗時，會啟動 `D:\ESD_Program\EXE\ESD_Program.exe`；找到視窗之後，HT IonBar 那一支會跑上電流程（大約 450～500 個週期後，對應的感測器亮了就送 IonBar 控制器的上電指令）。今晚合進來的 G3 只做「每秒找一次 ESD 視窗」：ESD 程式開著的話，Handler 的 START／STOP／溫度／上下線通知現在送得到了（之前全部送不出去）；G5 仍在閘裡。**例**：機台裝了 HT IonBar，開機時 ESD 程式沒有自己開 ⇒ golden 會把它叫起來並替 IonBar 上電；V906 目前兩件都不做，IonBar 要有人手動上電。選項：**A** 維持關著；**B（St02 建議）** 翻 G5 但不啟動程式：ESD 程式開著時，照 golden 跑 IonBar 上電；**C** 整段照 golden（V906 會自己啟動／重啟 ESD_Program.exe）。我的建議：**B** —— IonBar 上電是對「已經開著的 ESD 程式」送指令，跟 G3 同一類；「Handler 自己啟動外部程式」跟第 23 題是同一個問題，建議兩題一起定 | St02 FROM_STEVEN §3 04:06 |
| 25 | 🟡 **單選設定的號碼超出選項時，要不要照 golden「那次存不進去」**（NB2 R87-RANGE；今晚接上的設定變更紀錄，第 46 列） | 存設定檔時，golden 會把舊值、新值換成畫面上的選項文字記一筆（例 `19200 ==> 4800`）。如果 ini 裡存的號碼比選項還多 —— 例：BaudRate 只有 3 個選項，ini 裡卻是 5（新版多了選項、機台又退回舊版，或有人手改 ini）—— golden 在查選項文字那一步就丟例外（NB2 用 BCB6 的 VCL 原始碼與實測確認）：**這個設定、以及同一次存檔後面的設定都存不進去**，畫面跳「List index out of bounds」，而且每次存都一樣 ⇒ 用畫面改不回來。移植樹現在照存，log 寫成「` ==> 4800`」。只有 7 個「單選鈕」設定會這樣（清潔盤種類、測試機介面、RS-232 的 BaudRate／位元數／同位檢查／停止位元、AutoClean 選哪支手臂）；5 個下拉選單 golden 本來就是空字串照存，跟移植樹一樣。選項：**A（建議）** 維持現在：照存、log 的選項文字留空 —— golden 這裡是一個會卡住操作員的失敗，重現它沒有好處；**B** 照 golden 丟例外、那次存檔中斷（還要先決定網頁怎麼顯示這個錯） | NB2 R87 §3 |
| 26 | 🟡 **開機時該跳給操作員看的訊息，移植樹要不要照 golden 跳**（NB2 R91） | 移植樹的設定讀寫程式把 golden 的訊息框改成「放進這一次網頁請求的回覆」；開機時沒有網頁請求，所以開機時的訊息**沒人看得到**（只印在主控台）。NB2 普查開機會跑到的 16 處：**3 處 golden 會暫停機台、停所有馬達、跳框等操作員按**（都是 AutoClean 設定：arm 2 不能用、模式不支援、X／Y Division 數量錯）；**2 處 golden 會直接結束程式**（找不到任何 DIO 設定檔、GPIB 機種讀不到）；其餘 11 處 golden 自己也不顯示。**例**：AutoClean 設成用 arm 2、site Y pitch ≥ 65 ⇒ golden 開機完暫停並跳「arm 2 不能用」，操作員知道設定被改成 arm 1；移植樹同樣改成 arm 1，但沒有任何提示。選項：**A（NB2 建議）** 開機時的訊息先排隊，開機完照 golden 重播（那 3 處會暫停＋停馬達＋網頁跳框）；兩處「結束程式」改成照常服務、但開機後跳一個關不掉的警告；**B** A 之外，兩處「結束程式」改成 wb_serve 拒絕啟動（跟沒有工單時一樣）；**C** 維持現況。我的建議：**A** —— 3 處照 golden 跳是翻譯問題；「結束程式」照做的話網頁會連不上、操作員連原因都看不到，用關不掉的框比較好。A 會在開機時停馬達、跳框，屬於警報／模式的變更，**夜間不做，你選了之後白天做** | NB2 R91 |
| 27 | 🟡 **St02 那台沒有 golden 906 可讀，要不要給、怎麼給**（St02 06:36、NB2 R93） | 規定翻譯一律對照 golden 906（RULINGS_20260925 第 15 條），但 St02（STEVEN-NB3）只有加密的 `HT9011UC_Code_V3.33.906.0_20260618.7z`、沒有密碼（St02 不猜），所以 **St02 過去的翻譯都是照 V912 讀的**。NB2 R93 的新工具找到全樹 19 處註解引的是 V912 行號，全部在 Steven 兩台寫的檔。NB2 抽查的幾支 906 與 912 本體逐行相同，所以翻譯多半沒錯；但只要哪一支 906 與 912 不同，St02 就會照 912 翻而沒人發現。**例**：St02 翻 `spbClearRecordClick` 照的是 912 `main.cpp:31109`，906 是 `:30062`，這一支內容剛好相同。St02 已主動要做一次比對，把 912 與 906 不同的函式列出來。選項：**A（建議）** 請 Steven 在他那台用正常管道拿到解開的 906（密碼當面給，**不經 git、信件或任何文件**）；**B** St02 繼續照 912 翻，每一支由 NB2 事後對 906；**C** 不處理 | St02 FROM_STEVEN §3 06:36、NB2 R93 |
| 28 | ✅ 已裁：A（留 912）—— 使用者 10:2x（原話寫「第 30 題」，筆電理解成第 28 題的題號打錯）；RULINGS_20260927 第 6 條。🟡 **銦片壽命（Head Contact Count）的檢查與設定頁：留 912 的「依手臂」還是照 0926 Q2 改回 906 的「依列／行」**（NB2 R96-ARM，R97 建議補題） | 移植樹的 `CheckContactOver`（`csystem.cpp:15690-15724`，Steven `9b816dfd`）與 StartCondition 頁（`FileRW/StartCondition.gen.inc:908-910`）都已經是 912 的 `ContactSet[x][手臂][位置]`；golden 906 是 `[x][列][行]`。0926 Q2 是「底層照 906」⇒ 對不上。**只改一邊會算錯是哪個測試頭到壽命**（例：第 2 支手臂的計數被當成第 2 列）。選項：**A 留 912，在 RULINGS 記成 Q2 的例外**（不用改程式；跟第 19 題警報碼照 912 同一塊、同一方向）／B 兩邊都改回 906。**建議 A** | NB2 README R96 §0、R97 §0 |
| 29 | ✅ 已裁：暫緩、相關警報先 mark —— 使用者 10:2x；RULINGS_20260927 第 6 條。🟡 **ATC 溫控器（`USE_ATC_MODE=6`）報的警報要不要接**（NB2 R84-ATC，R97 建議補題） | 移植樹從收封包開始整條都沒有（連線、解析、200 ms 輪詢、佇列、對照表、`ShowATCAlarmMessage`）⇒ 溫控器報錯時畫面不會跳警報。**新量到的**：動作流程對照用的真機紀錄（HT9046_LS、工單 FT005054）那台 `Gerneral.ini:380` 就是 `USE_ATC_MODE=6`（`system\ATC.ini` 的 `iATC_MODE_TYPE=35`）。選項：A 現在做（約 200 行 golden、只收不送、ctest 可驗）／B A 之後再把送指令那半也接／C 先不做，等量到 HT9050 自己的 `USE_ATC_MODE`。NB2 原本建議 C；**我建議改成 A**：第 5 條說工作目標以真機紀錄為準，而紀錄那台是 6 | NB2 README R84 §0、R97 §0 |
| 30 | ✅ 已裁：A、用真實路徑 —— 使用者 10:2x；RULINGS_20260927 第 6 條。🔴 **動作流程對照第 ③ 步：可不可以把真機紀錄的設定暫時換進 `D:\HT9045`**（INBOX 第 49 列；0927 07:5x 問過，還沒回） | 要跟工單 FT005054 比，模擬就要讀那台機的設定（HT9046_LS、機台 ID HZ6I9）。現在 `D:\HT9045` 是另一台（PMLD1019）。**A**：跑之前把目前的 `system`／`config` 整個改名擱在旁邊、`setup.inf` 另存，換上快照裡的那份，並把 `IniData\Data\FT005054` 加進去；跑完刪掉換上的、改名改回來，再用 gate 那套 sysguard 逐檔比對 MD5 確認一模一樣（改名是瞬間的、完全可逆；換著的期間不跑 gate、不開 F5）。**B**：改程式讓 wb_serve 讀別的根目錄（寫死 `D:\HT9045\…` 的地方有上百處，要先做轉向接點，很大）。**建議 A**。⚠ AGENTS.md 規定複製進 `system/` 要你同意，所以要你一個字。沒回之前我先用目前這台的設定跑（今天 08:4x 第一次已經在跑，量工具本身） | `D:\HT9045\Staterecord\2025-12-11 17_47_57\HT9045\` |
| 31 | ✅ 已裁：A（維持 912 送 1）—— 使用者 10:2x；RULINGS_20260927 第 6 條。🟡 **P65 ARM-QA 重測時送給測試機的批次狀態碼：照 912 送 1，還是照 906 送 2**（St02 07:09） | St02 翻測試機通訊時是照 912 寫的：912 在「QA 抽測中而且這顆要測」時送 1（0x42），906 是「QA 重測」時送 2（`RunTestProgram` 912 :19037-19040）。0926 14:3x 你裁過「906 為底、補上 912 的新東西，目標是 912」，其他 5 處差異都在這條裡；**只有這一處是「改掉」906 的行為、不是「補上」，而且測試機看得到**，所以 St02 要你確認。例：客戶的測試程式如果靠這個碼判斷「這是 QA 重測」，送 1 跟送 2 它的反應會不一樣。選項：**A 涵蓋，維持 912 送 1**／B 改回 906 送 2。**建議 A**（跟 0926 那條裁決同方向；目前程式就是 A，選 A 不用改） | `FROM_STEVEN.md` §3 07:09、`docs/ST02_GOLDEN906_AUDIT.md` |
| 32 | ✅ 已裁：照 golden（A）—— 使用者 0927 16:5x；RULINGS_20260927 第 7 條。🔴 **面板實體鍵（START／PAUSE／HOME…）要不要照 golden 接上**（INBOX 第 86 列） | 白話：機台面板上那排實體按鍵，在移植樹只有「警報框開著」時有作用；**平常主畫面時按 PAUSE 不會停機、按 START／HOME 也沒反應**，只能從網頁按。golden 是用一支定時器一直讀面板鍵（`TfMain::ScanKey`，main.cpp:2379-2656，14 顆鍵）。例：生產中操作員看到異常按面板 PAUSE，golden 會暫停；移植樹現在不會。選項：**A** 全部照 golden 接（START 走跟網頁 START 同一條 `StartFromWeb`，普查數字跟著更新）／**B** 先接「停下來」那幾顆（PAUSE、ALARM RESET、POWER OFF），START／HOME／ONE CYCLE 等之後／**C** 維持現狀（只能網頁操作）。**建議 A**（V906 要上線，面板鍵是操作員的主要操作方式；真機紀錄也是用面板鍵），至少要 B（面板 PAUSE 不停機是安全問題） | `INBOX_QUEUE.md` 第 86 列 |
| 33 | ✅ 已裁：照 golden（A，沒被寫過的輸入視為亮）—— 使用者 0927 16:5x；RULINGS_20260927 第 7 條。🔴 **模擬時感測器預設要「全亮」（照 golden）還是「全暗」（現在）**（NB2 R112；動作流程對照為什麼沒有料流的第二層原因） | 白話：golden 在沒插卡的電腦上跑模擬，讀卡失敗時它寫了「就當成亮」，所以**每個感測器都是亮的**；移植樹的模擬用記憶體裡的一張表，**每個感測器都是暗的**。例：按 Initial Start 時 golden 看到「台車有盤」是亮的，補料流程就開始跑、入料臂、測試、出料臂一路動下去；移植樹看到暗的，台車永遠是空的，料流不進來（今天第四次對照就是這樣）。選項：**A** 照 golden，沒被寫過的輸入一律亮／**B** 維持全暗，另加一條模擬專用指令一顆一顆打開／**C** A＋B（預設全亮，測試要暗的時候用指令關）。**建議 C**（對照要比的就是「golden 模擬會怎麼走」；`IOBackend.h:15-19` 自己的設計說明本來就寫要照 BCB6 的模擬）。⚠ 影響：所有模擬流程都會變，sim 那 18 項既有失敗與對照基準都要重量；只影響模擬組態，不影響真機 | INBOX 第 49 列、NB2 R112 |
| 34 | ✅ 已裁：照 golden（A，不跳過）—— 使用者 0927 16:5x；RULINGS_20260927 第 7 條。🟡 **溫控器設成「沒有加熱器」的通道，要不要讓溫控迴圈跳過它**（St01 13:45，Q34 方案 D 的底層第 4 點） | 白話：golden 的溫控迴圈是一個通道一個通道輪流問；如果某個已安裝的通道廠牌設成「沒有加熱器」（3），golden 四個廠牌都對不上、**不送命令也不換下一個通道**，結果後面所有通道都不再被問，溫度停在最後一次的值。例：72 個通道裡第 5 個設成沒有加熱器 ⇒ 第 6～72 個的溫度都不再更新。移植樹照 Q15 把缺鍵預設成 3 之後這種情況會變多。選項：**A** 照 golden（不跳）／**B** 偏離 golden：廠牌不是有效溫控器（含 3）就換下一個通道。**建議 B**（golden 那樣是缺陷，會讓溫度顯示卡住；V912 要不要一起修另外判斷）。⚠ 移植樹的溫控迴圈今天還沒在 wb_serve 跑起來，這題是「接起來那一波」要用的 | TO_STEVEN §4 14:3x、St01 decisions-pending Q34 ⑥-4 |
| 35 | ✅ 已裁：依據建議（A，只算 Head1～4＋Index 32 區共 36 個）—— 使用者 0927 17:1x；RULINGS_20260927 第 7 條。🟡 **Q34 方案 D 的「Index 位置」要算哪些溫控通道**（St01 13:45 D-2，要照機構確認） | 白話：方案 D 讓「Index 位置」的溫控器跟「其他位置」分開選廠牌。St01 建議 Index 位置＝Head1～4＋Index 32 區（golden V912 MachineType.h:638-645，共 36 個）；要不要把 Socket、DUT1～4、IndexESD、Door1～2 也算進 Index 位置，要看機構上它們是不是跟 Index 用同一種溫控器。選項：**A** 只算那 36 個（St01 建議）／**B** 36 個＋Socket／DUT1～4／IndexESD／Door1～2（請指出哪幾個）。筆電這邊沒有機構資訊可以判斷，**請你照機構回答** | St01 decisions-pending Q34 D-2 |

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
| 14 | **golden 的 IO 輸出快取越界修掉**（`6367d599`，0925 第 18／29 條「要修」，INBOX 第 16 列）：`OutPortData[4][64][4]` 裝不下 1203 的輸出位址，HT9050 表上 14 組不同的輸出點位共用同一個快取位元 ⇒ 開 A 會讓 B 的「是否已開」也讀成開（`SW[].Status()`／氣缸 `GetOutBit()`）。只放大快取、MotionNet 規則不動；⚠ 原本想在範圍檢查一律擋 1203，量表時發現輸入端真空列的 Port 是 128～135 ⇒ 改成只擋輸出（不然機台上每次讀真空都會跳框）⇒ 推 main `10936285`；**GitHub 更新包 11 `updates/10936285/`**（`5744569`，8 檔，要全量重編；掃描 0 處） | `LaneIORoute` [5]：修正前紅 16 條（對照組）、修正後 96／96；兩組態 gate＝基準 |
| 15 | **ctest 不再能寫到機台的 `system\lastdata*.dat`**（`c265f087`，OPMODE 波次的前置）：`cprod.cpp` 15 處寫死路徑經過 `W906_LastDataPath`，只有測試執行檔（`tests/test_bootstrap.cpp`）會把自己行程轉進 build dir 裡的沙盒；變數名綁行程 PID，正式的 wb_serve 轉不走（跟 `--dry` 退場同一個理由）。⚠ 做的時候我自己的新測試寫壞了一次真實 `config.ini`（見 §6），已還原 | 新 ctest `LastDataSandbox` 兩組態 20／20（含對照組）；兩組態 gate＝基準 |
| 16 | **ctest 也不寫真實 `config.ini`**（`0d0a423b`）：`WriteLastDataFile` 寫 config.ini 那四節的路徑（`cprod.cpp:2085`）也經同一個鉤子 | `LastDataSandbox` 20／20；兩組態 gate＝基準 |
| 17 | **多分頁的視窗總表不再互相蓋掉**（`19844f8e`，St01 報的 bug）：`WebCommand.connId` 以前永遠是 0 ⇒ 每個分頁都登記成同一條連線 ⇒ 推 main、**GitHub 更新包 13**（`20dfaf1`） | `WB_Server` 第 6 節，對照組（改回 0）紅 3 條 |
| 18 | **OPMODE：`UpdateMainOperateMode` 整支照翻**（`7304dcef`，你 0922 的裁決「都要，全部動作都要執行」）＋`ChangeATCSiteUse`／`TemperatureEditDisable`／`SetNormalOrPrime`，約 1,200 行 golden。**wb_serve 開機就會跑真本體**（照 golden：依溫度模式切加熱器繼電器、送 ATC 命令、寫 lastdata）。審查抓到並已改的兩個錯：① ATC 那支第一版把「沒連線」當「已連線」，後面 430 行一律執行；② 分層（本體改放 sm、經 hook 呼叫）。ctest 不受影響（沒裝 hook＝原本的計數樁）| 新 ctest `OpModeBody` 15／15；wb_serve 開機煙霧測試兩組態都正常；寫到的真實檔量過（lastdata 只多不少、config.ini 只多空行）並已從快照還原 |
| 19 | **NB2 R70 覆核我的第 9 條，抓到的兩個 🟠 已修**（`d0e4e5a0`）：① 警報框開著時，網頁送的 IO 輸出／軸停止會被執行（框裡的 1203 Poll 跑了「輸出優先」的 hook；HT9050 實彈上約一半的點擊）⇒ 改回一律拒絕；② 關框後面板 START／PAUSE 燈停在閃的那一相 ⇒ 關框時設回暗（照 golden 翻 ProcessKeyFlush 排在 INBOX 第 64 列）；另照 golden 補兩處 `bAlarmReset` 清除、是否框也收模擬 DI ⇒ 推 main `c65ddd85`、**GitHub 更新包 15**（`21b44bb`，兩支掃描都 0） | 兩組態 gate＝基準 |
| 20 | **`ChangeATCSiteUse` 接上另外兩處**（`75b87a88`）：三溫機 TriTemp 的 5 個呼叫點（原本是空樁）、Home（經 hook，同 UpdateMainOperateMode） ⇒ **GitHub 更新包 16**（`8803ab2`） | 兩組態 gate＝基準；`OpModeBody` 16／16 |
| 21 | **框開著時照 golden 跑 Index 吸嘴的 IC 掉落檢查**（`10539eef`，NB2 R70 MW-F，第 9 條之前就缺的）：golden 任何框開著時每一拍都跑 `CheckIndexAllSuckICFallDown`（main.cpp:3125-3128），移植樹三個等待迴圈都沒跑；另照 golden 清兩個 SECS 旗標（YN-4）⇒ **GitHub 更新包 17**（`35a9a51`） | 兩組態 gate＝基準 |
| 22 | **警報框用 START 答掉後，恢復運轉照 golden 先讓手臂回 Z 安全位**（`d31147db`，NB2 R71 C1）：golden 的警報框一出現就設一整組暫停標記（`bHandlerPause`、`bPauseInMotor`…），移植樹沒設 ⇒ 用 START 答掉時，手臂從停下的地方直接接著走、tester 逾時把框開著的時間也算進去。筆電逐行對過 golden 7 處都是無條件設 ⇒ **GitHub 更新包 18**（`41ab6e0`） | 兩組態 gate＝基準 |
| 23 | **St02 測試通訊 P2f／P7 合進 main**（`5716de33`＋筆電編譯修正 `04c72d84`）：機台停著、測區沒有 IC 時，On/Off-Line、工單、GPIB 位址、bin 數有變就照 golden 同步給測試機橋接程式（以前 `MSG_CMD_ChangeGpib` 從來沒送 ⇒ 橋接那邊的 Off-Line 模擬／位址／bin 數一直是它自己 ini 的值）；測試通訊頁 400 ms 內同一顆鈕只算一次。St02 那台沒有 MinGW，P2f 有一行型別不合編不過，筆電照既有三處的寫法改傳 `nullptr` ⇒ **GitHub 更新包 19**（`bba60c4`） | 兩組態 gate＝基準；`D:\GPIB9045\system\general.ini` 前後 MD5 相同 |
| 24 | 🔴 **修掉我 0926 自己帶進來的當機**（`ec5905c9`，NB2 R72 ATC-1／R73）：`USE_ATC_MODE=4`（Hontech ATC）、測試模式 Single／Dual 的機台，OPMODE 之後換配方、登入、切運轉模式、按 HOME 都會當 —— golden 開機會先 `InitialATC` 建 4 個 ATC 通道，移植樹那個呼叫是死碼，通道表永遠是空的。照 golden `main.cpp:9357-9361` 補在開機 InitialHandler 之前。修好之後**不會**開始跟 ATC 通訊（真正收送的計時器移植樹沒人驅動，另案）；這台筆電是 5、不受影響 | 新 ctest `AtcBootInit`（重現當機路徑）；兩組態 gate＝基準 |
| 25 | **Jam 次數照 golden 在關警報框時累加**（`0cc90186`，St01 J2）：以前網頁 prod 的 Jam 數、SECS SV 1036、MTBA／Jam rate 字串永遠是 0。條件照 golden：JAM 碼、正式跑、不是答 TRAY END、不是重複警報、Unit < 9（ADI 菲律賓全算）。已知差異：開 MTBA 的客戶那一支閘著（缺 `sJamArea`）、kcode＝0 的通知框不計 | 新 ctest `NoteJamCount` 21 項 |
| 26 | **生產資料三件**：Index 時間平均恢復累計（`d8e9d553`，J6，閘的理由 0818 就過期）；UPH 表寫進真的畫面物件（`58b01200`，J3，網頁與遠端 UPH 查詢以前讀到空的）；one cycle 結束照 golden 存各 Site 計數 `Arm*.dat`（`bb6857e2`，J4；ctest 經 `W906_MACHINERECORD_DIR` 寫 scratch）；測試中的秒數照 golden 每秒加一（`57027ca3`，J12，狀態列 [5] 的發布歸 St01） ⇒ 第 24～26 列＝**GitHub 更新包 20**（`9699df0`） | 新 ctest `Timer2TestSeconds`；兩組態 gate＝基準、system／config／IniData 0 變動 |
| 27 | **St02 第三批合進 main**（`626f405b`）：cMyDB P1（golden 的 log 物件、SaveEventLog、CSV 寫入閘解開；wb_serve 開機建、關機收）、Qorvo 的 Tester Pause 照 912 等逾時才響、SetTestTimeOutTimer 照 golden（On-Line 一送 SOT 就判逾時的問題修掉）、P6 暫定子集。St02 那台沒有 MinGW，**又有三處編不過**，筆電同行修掉（`f640ce18`：`cMyDB.cpp` 三處 BCB 的 `__FUNC__`、`LogObjects.cpp:50` 對 `->Text` 直接 `AnsiPos`、`FileRW/TestIF_File_TesterIF.gen.inc:1101` 解閘後的 `__FUNC__`——在 include 它的 .cpp 空行補 shim）。同一輪另做：觀察頁的 bin 歷史矩陣照 golden 每顆更新（`364ea435`，St01 S116 ⑤）、bin 顏色表還原 golden 值（`e0750059`，⑥②） ⇒ **GitHub 更新包 21**（`4e8b9c1`） | 兩組態 gate＝基準；`D:\HT9045_Log` 前後相同 |
| 28 | **開機紀錄 BootLog 照 golden 接上**（`1bf5ce5b`）：golden 在 `WinMain` 寫 `D:\HT9045\Error\BootLog.txt`（「24V／硬體沒好時查當機用」，超過 512 KB 輪替），移植樹從來沒呼叫。接了入口、進主迴圈前、正常結束三個點（另外四個在 wb_serve 沒有對應：沒有 ExePath 檢查、沒有 VCL Application、`main()` 沒有 catch）。另：OPMODE 相關四處過期註解更正（NB2 R72 OPM-4）、absence 哨兵抓到的 `cBinSel.cpp:3221` 註記 ⇒ **GitHub 更新包 22**（`f55ea00`） | 兩組態 gate＝基準；模擬組態 wb_serve 實跑，BootLog 三行都在 |
| 29 | **St02 第四批合進 main**（`387c407a`，這次一次就編過）：GPIB 程式的額外 RS232 port 跟配方走（P6 Q2(a)，你的裁決）、TTL 卡重送測試模式的條件改讀網頁視窗總表（我 21:3x 提的觀察）、cMyDB P3（開機照 golden 讀／建 `D:\HT9045\Error\AlarmCodeList.txt`、AlarmCode 快取）、Event Log Analyzer 轉 web 的帳本 ⇒ **GitHub 更新包 23**（`d10d86d`） | 兩組態 gate＝基準 |
| 30 | **面板 Alarm Reset 照 golden 送 SECS 事件**（`addd1a89`，CEID 30 DoAlarmReset，NB2 R70 YN-4 那一行）：以前只寫成註解「SECS 未移植」。SECS 開著的機台，host 會收到「按下 Alarm Reset」；筆電 SECS 關著＝沒變化 ⇒ **GitHub 更新包 24**（`33f76d3`） | 兩組態 gate＝基準 |
| 31 | **觀察頁的測試／Index 時間表照 golden 每次測完更新**（`d3c93dea`，St01 S116 ①，golden `RecordTimeInfo` 290 行）：以前是空樁 ⇒ 表一直是空的，遠端 IndexTime 查詢、SECS 的 TestTime／IndexCycleTime、每顆 IC 生產紀錄的時間欄都沒值。⚠ 照 golden，`dTestSec` 從此每測完更新（ATC 的測試時間補償讀它） ⇒ **GitHub 更新包 25**（`617e7a4`） | 新 ctest `RecordTimeInfo` 14 項；兩組態 gate＝基準 |
| 32 | **每一支 ctest 都拿到路徑轉向變數**（`260a29ca`，St02 查到的洞）：tests/CMakeLists.txt 的全域 ENVIRONMENT 只給到那一行之前加的測試，檔尾十幾支（包括我今晚加的 4 支）一個轉向都沒有、走到就寫真的 `D:\HT9045_Log`／system。改成在目錄結尾統一補（`cmake_language(DEFER …)`），201 支全部帶上；St02 的兩個 log 根目錄變數放 `_ht9045_env_extra` 就好。第一輪抓到我自己 `W906_LastDataPath` 的 bug（只認反斜線），同一行修掉（`012fbc13`） ⇒ **GitHub 更新包 26**（`58c6f77`） | 兩組態 gate＝基準、子檢查 0 差異 |
| 33 | **St02 第五批合進 main**（`5cfb4b04`，0 個編譯錯）：log 根目錄的三處改成讀環境變數（接我的 CMake 那一半；變數本身由 St02 放進 `_ht9045_env_extra`）、S93 的站況 log 轉接函式 ⇒ **GitHub 更新包 27**（`c2b3d4d`） | 兩組態 gate＝基準 |
| 34 | **主畫面 Shuttle／AutoClean 兩個 log 出口照 golden 翻**（`722d12da`，golden main.cpp:30228-30261）：以前是兩個空函式。⚠ 今天在機台上看不出差別：寫進去的 memo 都是 `TfMainMemo` 替身（不存任何行）。要真的記、每 2048 行存 `d:\AutoCleanLogs\*.csv`，要把 memo 換成存得住行的版本 —— `memoAutoClean` 在 golden 的 Main.Record 分頁上，是 St02 認領的 S119 範圍，所以留給他（INBOX 第 75 列） | 兩組態 gate＝基準、子檢查 0 差異 |
| 35 | **觀察頁 Yield 圖帶 golden dfm 預設值**（`870ff02e`，St01 指出）：Y 軸 -5～105、上下限框 100／0、32 條線標題 Site Aa～Dh（以前全是 0／空的）；**wb_serve 開機時把有設的 `W906_*` 轉向變數逐一印出來**（`f0fc6c82`，NB2 R79 只印不擋那一半，擋不擋是決策第 20 題）；**St02 的 log 根目錄 CMake 半邊**（`6d7e8d23`＝`f5b2d378` 挑過來）：201 支 ctest 的 `W906_HT9045LOG_ROOT`／`W906_SAVEEVENTLOG_ROOT` 都指到 build 裡的 `machine_log_scratch`。⚠ St02 同時推的 Event Log Analyzer（`9b4b33dc`）**沒合**：他的新測試 `ELA_Core` 58 過 4 敗，根因線索已回給他（INBOX 第 79 列） | 兩組態 gate＝基準、子檢查 0 差異；轉向變數實跑兩次（A 印兩個、B 對照不印，都 exit 2），跑完 0 變動；第 34～35 列 ⇒ **GitHub 更新包 28**（`167922a`，main `95c2c26a`） |
| 36 | **開機照 golden 呼叫 `InitialMemory()`**（`bf45351f`）：以前沒人呼叫它。現在在讀機台設定**之前**呼叫（跟 golden 同順序，不會清掉讀過的值）；它清零的 48 行（47 個陣列；`iAutoHasHod` golden 也清兩次，NB2 R83）量過都本來就是 0。⚠ **效果比我原本寫的小**（見 §6）：站點行列表 `SiteData` 其實開機時已由 St01 的 `SeedSiteData()`（`wb_serve.cpp:4158` → `W906_DoReadLastData` → `FileRW_Setup_Boot`）補好；這一顆讓它更早有值、在沒有有效配方路徑（`pathReady` 為 false、St01 那段不跑）時也有值，並照 golden 在開機時啟動「Handler 停機時間」計時（`lHandlerStopTime`） | 新 ctest `InitialMemory` 16 條；兩組態 gate＝基準 |
| 37 | **「能啟動機台的呼叫點」普查修正＋接成 ctest**（NB2 R80）：工具原本只認字面 `#if 0`，把 St02 加的 GPIB START（`HandlerGpibMsg.cpp:717`，其實被 `#define W906_REMOTE_START_WIRED 0` 閘住）算成活的，所以從 0926 14:56 起一直是紅的、沒人發現。改成 34 個／活 30／閘 4（**活的路徑數沒變**），CLAUDE.md 等三處同步，新 ctest `START_SitesCensus` 以後數字一變 gate 就紅；另 St01 指出的兩件：Observer 一段錯的註解、mtRowA～D 照 dfm 的大小 | 工具改前改後逐行比：只有 `:717` 一列從 LIVE 變 GATED；兩組態 gate 見 §5 |
| 38 | **St02 的 Event Log Analyzer 核心合進 main**（`cb58ed9f`，含日期修正 `58643309`）：新 library `ht9045_ela`＋ctest `ELA_Core`，**沒連進 wb_serve**、`/api/ela` 仍暫停，機台上跑的程式不受影響。上一版 58 過 4 敗是筆電抓到的（日期與時間分開四捨五入，拆出「04/01 24:00:00」）；St02 照 Delphi 的拆法修好後 **65 條全過**。另 INITMEM 用模擬組態 wb_serve 實跑 15 秒：開機三個檢查點都寫出來、正常結束，St01 的站點表補值那一行 0 次（`FileRW_Setup_Boot` 有跑，看到表已經有值就不動）；看門狗「LoadMachineConfig 5 秒沒前進」0926 那次實跑就有，不是這批帶進來的 | 兩組態 gate＝基準、子檢查只多 3 支新測試；ELA 的檔只在 `%TEMP%\ht9045_ela_core`，`D:\HT9045_Log` 0 變動；實跑後 6 個真實檔照快照還原；第 36～38 列 ⇒ **GitHub 更新包 29**（`1543c21`，main `3f166785`） |
| 39 | **完成度普查（census）量法修正**（`d6bae87f`）：它數大括號時連註解與字串裡的都算，一個不成對的就把後面的函式全吞掉 ⇒ 移植樹 `Command.cpp` 已翻好的 43 支遠端查詢（813 行）一直被算成「沒翻」。修正後：非表單 95.9%→**96.1%**（缺 13,825→13,012 行；分母 golden 336,509 行）；把「翻在別的檔」也算進來，非表單 96.3%、表單 23.2%→**25.2%**、全部 64.2%→**65.2%**（分母 598,371 行）。**非表單真正剩下的都不是夜間能做的**：SECS 從主機下載配方（S7F2／S7F4／S7F6，1,202 行，會寫配方）、log 函式（等 log 物件或存得住行的 memo）、參數修改紀錄（等 St02）、cMyDB 四支（St02）、`CheckMotorValue`（馬達）、ASE／畫面各一支 | 被「少算」的 5 支 golden 函式逐一查過都在 golden 的 `/* … */` 裡（沒被編譯）；新承認的 17 支抽查，移植樹行數都跟 golden 相近 |
| 40 | **良率計算兩支放進活的檔**（`8250e828`，golden `GetTotalYield_double`／`_Str`）：PAT 即時報表會呼叫它、可是整棵樹都沒有定義（用 `nm` 量到的未解析參照），今天沒連進 wb_serve 所以沒爆，接上那天會連結失敗 —— 現在先補好；另把 INITMEM 檔頭那句說過頭的註解改成實情。**三個解閘標記的 golden 行號更正**（`66db2979`，NB2 R82，只改整行註解） | 兩組態 gate＝基準、`InitialMemory` 19／19；R82 那顆剝掉註解後逐字相同（沒另跑 gate）；⇒ **GitHub 更新包 30**（`d4e1616`，main `b635f32d`） |
| 41 | **文件裡被吃掉的反斜線還原**（28 處）：路徑或正規表示式裡的 `\b`、`\a`、`\14`、`\v` 曾被當成跳脫字元，寫成了退格／響鈴／跳頁等看不見的控制字元（例 CLAUDE.md 的 `D:\HT9045\Obj\V906\build_dbg` 顯示成「V906uild_dbg」、pt-wave-loop 的 `C:\MinGW\bin` 少了 b）。這次寫腳本掃全部被追蹤的文字檔抓出來：CLAUDE.md、pt-wave-loop 與 ht9045-config 兩個 skill、KNOWLEDGE／MIGRATION_ROADMAP、INBOX（0925 留下的 4 處＋今晚的 1 處）、cObserver.cpp 4 行註解（`rg -n "\b…\b"`）。St01 的 `docs/handoff/AUDIT_IO_20260926.md:3` 也有一處，是他的檔，已在 TO_STEVEN 告訴他 | 掃描器 `backup\night_tools_20260927\ctrlchar_scan.py`（排除建置 log／產生的手冊）；cObserver.cpp 剝註解後跟 HEAD 逐字相同；⇒ **GitHub 更新包 31**（`fad653f`，main `fe03a1e7`） |
| 42 | **擋住「送進 Python 的反斜線被減半」的 hook**（`scripts/ops/deny-heredoc-backslash.ps1`，只掛在這台的 `.claude/settings.local.json`，不進 git、不影響 Steven 那兩台）：inline Python（heredoc 或 `-c`）裡有 `\\` 就擋下，叫我改用 Write 工具寫成檔；單一反斜線（例 commit 訊息的路徑）不擋。CLAUDE.md 的 hook 段落記了這件 | 8 組假輸入全符合（擋 3、放 5，含壞輸入放行）；掛上後實測一個符合條件的指令被擋下 |
| 43 | **RecordErrorLog 照 golden 寫日期 log 檔**（golden cpublic.cpp:1651-1670）：0626 放的替身只把訊息印到主控台，golden 是寫 `D:\HT9045_Log\<分類>\YYYY\MM\YYYYMMDDHH.txt`。呼叫點跟 golden 一樣是 21 處、全部是 `SiteUseMgr`（cSiteUseManager 20、ainarm9045_1x4_4 1）⇒ 開了 Site Use Manager 的機台，運轉時會開始在 `D:\HT9045_Log\SiteUseMgr\` 底下產生每小時一個檔（跟 BCB6 版一樣）。兩處偏離都標在行上：根目錄用 `as9045LogPath`（值相同，ctest 由 D5 轉進建置資料夾）、`case 1`（寫 ListBox14 畫面）照舊閘著。新 ctest `RecordErrorLog`（4 項，路徑不含 `machine_log_scratch` 就拒跑）；`W7_L3_SiteUseMgr` 原本從 stdout 抓 log，改成讀那個檔（斷言不變） | `2bab8e7c`；兩組態全量 gate＝基準（出貨 3、模擬 18），subfails 只多 4 支新測試；sysguard 0 變動、`D:\HT9045_Log` 只有 QtyData；第一輪 `W7_L3_SiteUseMgr` 約 20 項 CHECK 失敗（抓 stdout）⇒ 改讀檔後通過 |
| 44 | **另外 6 處被吃掉的反斜線**（NB2 R85；第 41 列那 28 處之外）：`scratchpad/extract_page_map.py:36` 與 `.claude/skills/ht9045-html-version/scripts/extract_page_map.py:36`、`scratchpad/obs_check.py:75` 是**正則永遠不中**的功能錯（開頭是退格字元，原本是 `\b`）；`ainarm9045.cpp:8500`（2 個）、`build.bat:7`、`tools/enum_cite_check.ps1:105` 是註解。逐檔先斷言控制字元個數再換；`build.bat` 等 gate 跑完才改（cmd 按位元組偏移讀），改完仍是純 CRLF、無 BOM。Steven 的 Copilot 副本 `.github/skills/…/extract_page_map.py:36` 同一個錯，是他的目錄，已在 TO_STEVEN §4 告訴他 | NB2 的 `ctrl_char_scan.py --worktree`：剩 2 處 ⇒ 改完 build.bat 後剩 1 處（就是 Steven 那份）；與上一列同一次 gate |
| 45 | **St02 的事件紀錄分析器接上 wb_serve＋ESD 指令送得出去了**（合 `v906/steven-gpib-widget` 到 `3ce47956`，merge `8ed776ad`）：Handler 送的事件紀錄指令改送進 wb_serve 裡的分析器（網頁 `/api/ela`，開機啟動、關站停止；`HT9045_ELA=0` 整個關掉）；**開機會照 golden 寫真檔**：`config.ini` 缺的鍵、`Gerneral.ini` 缺的 Machine ID、當天查詢時 `Error\English\JAM0000.dat` 缺的碼。golden 往舊 `EventlogAnalyzer.exe` 送的訊息照舊保留（手動開著的舊 exe 照樣收得到）；要不要自動啟動舊 exe 是 §0 第 23 題。ESD G3：之前 V906 **所有**送給 ESD 程式的指令（START／STOP／溫度／上下線／關站）都送不出去，因為沒人去找 ESD 視窗；現在照 golden 每秒找一次（只找、不啟動程式）；IonBar 上電與自動啟動 ESD 程式是 §0 第 24 題。網頁外殼加了 eventlog／testercomm 兩個工作列視窗（第一次打開才載入） | 兩組態全量 gate＝基準（出貨 207 項失敗 3；模擬 207 項失敗 18）；新 ctest `ELA_Hub` 15／15、`ELA_Service` 30／30（它自己比對真的 config.ini／JAM0000.dat／Gerneral.ini／EventLogTxt 前後一樣）、`TesterComm_Handler` 30／30（含 ESD 第 7 節）；sysguard 0 變動、`D:\HT9045_Log` 只有 QtyData、`D:\HT9045\Error` 不存在；⇒ **GitHub 更新包 33**（`24615fc`，main `f91f793e`） |
| 46 | **設定變更紀錄（ChangeLog）的輸入端照 golden 接上**（`752130a5`，St02 03:06 確認可以做）：golden 每次寫設定檔前，會先讀舊值，值有變且開機完成（`InitialOK`）就記一筆「`群組_名稱 change Value`＋`舊==>新`」（選項類的欄位記選項文字，例 `Kit ==> Tray`、`GP-IB ==> TCP/IP`）。移植樹五支 `WriteIniData` 的這一段一直在閘裡，現在五支照 golden 接上：`common.cpp`（最底層）只放接口，golden 原文放新檔 `common_ChangeLog.cpp`（產生器直接從 golden 抽，另一支腳本把 32 個標記行還原後，7 段 golden 原文逐字相同），wb_serve 開機時裝上。**要知道的**：① 紀錄目前還到不了檔案 —— 最後一站 `MyDBIProcess("ChangeLog")` 在 cMyDB P4 之前是只計數的替身（St02 03:06）；② double 那一支：golden 用 `double != AnsiString` 判斷有沒有改；NB2 用這台沒有的 BCB6 實測（R87），它是**數值**比較（1.5 對 "1.5000" 算相同，客戶 106 筆紀錄 0 筆同值），移植樹照字面寫會變成字串比較、每存一次 double 就多記一筆 ⇒ 寫成 `ret!=Str.ToDouble()`；另含 golden 的扭力標準值重設（扭力模式開著、Test Arm 的 Contact 偏移改了就重學）；③ 選項文字：golden 讀表單元件，這裡改用 golden .dfm 的原文清單（執行期不會改的那些）、`fContactForm`（接觸模式）、依客戶代碼選的 GPIB 清單、DIO 設定檔的檔名，超出清單回空字串（下拉選單跟 golden 一樣；單選鈕 golden 會丟例外、那次存不進去 ⇒ §0 第 25 題）；④ 依批號記錄（`RecordChangeLogByLot`，客戶選項）照舊閘著 | 兩組態全量 gate＝基準（出貨 208 項失敗 3、模擬 208 項失敗 18）；新 ctest `ChangeLog` 兩組態全過（81 項）；sysguard 0 變動；第一輪我的測試用錯鍵名（`ATCTempOffset_3`，配方檔實際是 `ATCTempOffset[3]`）63／65，改正後重跑 |
| 47 | **St02 的 #33「ELA 開機實跑」在這台做了**（St02／St01 兩台要等 Steven 同意才能改機台真檔，所以都沒跑）：模擬組態 wb_serve（main `231fffa6` 的 gate 產物）開機約 30 秒，照 0918 規則「備份 → 驗證 → 還原」。**結果**：開機印出 `[ELA] Event Log Analyzer hub started`；`GET /api/ela` 三次都 200（內容有「ReadConfig done」與當天查詢 0 份 EventLog）；`HT9045_ELA=0` 時回 503。**開機寫了哪些真檔**：`Gerneral.ini`（`Program Close` 0→1）、`lastdata.dat`／`lastdata_backup.dat`／`machinerecord.dat`（執行期資料）、`config.ini`／`teach.ini`（區段間補空行，照 golden，INBOX 第 27 列）、`D:\HT9045\Error\AlarmCodeList.txt`＋`BootLog.txt`、`D:\HT9045_Log\MDB_UpdateLog\…`、GPIB 的 log 與 general.ini ——**ELA 開與關逐檔比對完全相同**，所以在這台 ELA 沒有多寫任何檔（config.ini 的鍵本來就齊；當天沒有 EventLog 可查，所以沒建 `JAM0000.dat`）。另外看到今晚 RECERR 的第一筆真實 log：`D:\HT9045_Log\SiteUseMgr\2026\09\2026092705.txt` 一行「`2026-09-27 05:19:27 795, :, Init CompactSearch=OFF`」，格式同 golden | 三次實跑都「回到基準：True」（system／config／IniData 逐檔 MD5 等於 sysguard 基準，Error 目錄與新 log 都刪掉、GPIB general.ini 還原） |
| 48 | **St02 的 S119（主畫面 Record 分頁）合進 main**（merge `0faa75be`，St02 到 `5aed449f`）：網頁的 CLEAR 鈕與 AseRecordMemo 雙擊現在會送到 C++（golden `spbClearRecordClick` main.cpp:31109-31138、`meShuttle2DblClick` :29471-29475）。CLEAR 照 golden 只在開了 `bShowMainDebugRecord` 的機台做事（只有 CC_SIGURD_HUKOU／CC_RICHTEK 會開）；重畫畫面那一行 St02 先閘著，等 St01 的 S113 進 main。另含 St02 的 ELA 文件更正：wb_serve 的 `allowCmd` 從 0918 起一律是 true，網頁 POST `/api/ela/query` 不會回 403 | 兩組態全量 gate＝基準（出貨 209 項失敗 3、模擬 18）；新 ctest `MainRecord_Clear` 10／10；sysguard 0 變動 |
| 49 | **數字跟文字比較的地方，照 BCB6 的實際語意改**（`3f1882fc`，NB2 R89）：golden 有些地方拿數字跟 AnsiString 用 `==`／`!=` 比。NB2 用 BCB6 實測：數字在左時 BCB6 當成**數值**比較；`0`／`NULL` 在右時 BCB6 跟 `"0"` 比。移植樹兩種都變成跟字串比（後者是跟 `""` 比），行為不同。這次改了：① **`Command.cpp:2901`**：回給測試機的 `CHKSETUP?`，golden 會濾掉 Yield＝0 的 bin，移植樹沒濾（**這一條會改變送給測試機的內容，改回 golden**）；② `asortarm.cpp` 三處：條碼結果碼是 0 的 IC，盤圖照 golden 記成一般 IC、提示寫「2DID: ERROR」（原本記成條碼錯誤 IC）；③ `cpublic.cpp:1759`（測試時間佇列）、`Command.cpp:17220`（遠端 HTSET,403 訊息剛好是 "0" 時）；④ `EJ1N/uSocketServerClient.cpp:634`：port 設定寫成 `"05000"` 之類時不再每拍斷線重連；⑤ `fContact.cpp` 的 Dimension 比較改數值；三段說錯 golden 行為的註解改正。另外兩句在同事的檔（`DeviceForm_File.gen.inc`、`Rs232Support.cpp`），已在 TO_STEVEN §4 交給他們 | 兩支普查互相印證：筆電的編譯器探針（488 TU，「數字在左」8 處＝NB2 A 類逐處相同）；改完用 NB2 工具 #30 重跑：A 8→5、B 17→11，剩下的都是 NB2 列為「可不改」或同事的檔；兩組態全量 gate＝基準（出貨 3、模擬 18，subfails 0 變化 —— 現有 ctest 沒有直接量到這 9 句）；sysguard 0 變動 |
| 50 | **主畫面的 log 小視窗真的會記了，Shuttle 感測 log 照 golden 存檔**（`42ea830b`，使用者 07:2x「記憶體無上限由你建議方式改、硬碟 log 忠於翻譯」）：以前 `meShuttle1/2`、`memoAutoClean`、AGV 的 `mmE84Log` 都是不存任何行的替身。現在真的存行，另加一道 **4096 行**的防呆上限（滿了丟最舊的）—— 比 golden 每一個清空門檻都大（Shuttle 2048、存檔 1024、AutoClean 2048、E84 500），所以 golden 自己的清空／存檔照樣先發生，上限只防萬一。硬碟 log 忠於 golden：主畫面「Show Shuttle Sensor」照 golden 預設勾選，`OutShuttleLog` 在 memo 超過 1024 行（或關站時）寫 `D:\HT9045_Log\ShuttleLog\YYYYMM\SH2_／SH1_<時間>.logs`；AutoClean 超過 2048 行寫 `d:\AutoCleanLogs\*.csv`（golden 寫死的路徑）。ctest 新增 `MemoLog`（27 項）。⚠ 更正 INBOX 第 75 列：`mmTesterLog` 其實**有**清空門檻（`TesterTCP_Socket.cpp:387-394`，2000 行），我 06:4x 寫「沒找到」是只看了一行（§6）。**上機時要看**：跑一段生產後 `D:\HT9045_Log\ShuttleLog\<年月>\` 有 SH1_／SH2_ 檔、內容是 Shuttle 感測訊息、每檔最後一行是時間；長時間運轉 wb_serve 的記憶體不會一直長（工作管理員看 wb_serve.exe）。 | 兩組態 gate：出貨＝基準；模擬＝基準＋`WebMotorAccess` 逾時一次（不是 MEMO 造成的，見 §3） |
| 51 | **動作流程對照開始有東西可比了：task 紀錄照 golden 登錄**（`2e576e6d`，INBOX 第 49 列、RULINGS_20260927 第 5 條）：第一次模擬實跑（08:45，START → 模擬歸零 → Running）量到移植樹存的 `Task_ListWithTime.csv` **整份是空的**（15 萬個逗號）—— golden 開機畫面（`FormShow` main.cpp:9717-10036）登錄 281 個 task 的那一段沒翻。照翻：264 列登錄、13 列閘著（那個 task 在移植樹真的不存在，例 Tray 步進馬達的 `SetStepMotorTask`）、4 列 golden 自己註解掉；golden 的 Contact 表單對到移植樹真的表單物件 `fContactForm`、條碼的 8 個對到移植樹搬成全域的變數。另外 `act.main.stateRecord` 加一個只給比對工具用的參數，模擬組態也存得出 task 紀錄（網頁的鈕行為不變）。第二次實跑：60 秒時 18 個 task 有步序、15 個跟真機紀錄重疊；300 秒時 19 個。NB2 R100 獨立找到同一個缺口（它建議補宣告，我選閘著，已請它覆核，REQUESTS Q14）。⚠ 目前 `D:\HT9045` 是另一台（PMLD1019）的設定，差異多半是機種不同（例：歸零走 300／310 那一支）；要比得準要 §0 第 30 題。⚠ 已看到的真缺口：真機紀錄裡 `SetStepMotorTask` 跑了 123 次，移植樹整個 Tray 步進馬達模組（golden `Motor/TrayStepMotor`）沒翻 —— 馬達控制，列 §2 不動。**上機時要看**：按 State Record 後 zip 裡的 `Task_ListWithTime.csv` 每一列開頭有 task 名稱、有時間與步序 | 兩組態 gate＝基準；ctest `TaskListRegister`（新）＋`SjsonChan` |
| 52 | **流程對照找到的第一個狀態機缺口修好了：測試頭（Index）主流程照 golden 補回約 290 行**（`caf9b389`＋`33fd6570`）：先寫了一支靜態比對（真機紀錄走過的每個步序，golden／移植樹的狀態機有沒有那個 case）⇒ 真機有動的 35 個 task 裡 **33 個在移植樹都有真機走過的每一步**；真缺口是 Tray 步進馬達（整個沒翻，INBOX 85）與測試頭的步序 11。移植樹的測試頭 600000 以前是早期波次的簡化版，直接跳 9，跳過了：開機後吸嘴初始狀態檢查（不對就回 1）、F16 飛梭感測器斷線檢查、**第一顆 IC 的起測延遲旗標**（影響起測溫度補償）、ESD 衰減測試、「先放飛梭」與 D71／D69「這次不做 Index Check」的分支。照翻後模擬實跑：測試頭走 1→200000→300000→400000→600000→**11**→9→10→15→20→21→100→120，跟真機紀錄同一條路。另外 **DAQ 型 Index 吸嘴（`INDEX_SUCKER_TYPE=1`）的真空泵** golden 有、移植樹以前 6 個檔各自用「一律回 true」頂著（真機紀錄那台就是 type 1）：照翻、接上、16 個閘解開；type 0 機台不設旗標、行為不變。⚠ ~~目前是待命的~~（`ffd74fd8` 已解開，見第 53 列）：移植樹讀 `INDEX_SUCKER_TYPE` 的那段（`FileRW/HSys.cpp:147`，GATE W906-CTORKEYS-SUCKER，0925）還閘著，執行期一直是預設 0 ⇒ 連 type 1 的機台現在也走 type 0 的路，泵不會被用到、行為完全不變（10:2x 模擬實跑量到：TestHeadMotorTask 仍是 400000→600000，沒有 500000）。那個閘的解閘條件是「泵＋`bNeedCheck` 讀取端都翻好」（NB2 預勘 `docs/nb2_assist/RD5軟體_NB2預勘_D44泵_開機序列_翻譯佇列_20260925_031105.md` §1、§3 P1：先載入會讓 WAR1604 被 bIndexCheck1 永久關掉）—— 泵這次翻好了，**下一步是 bNeedCheck 讀取端＋解那個閘**。**上機時要看**：開機第一顆的起測延遲照配方設定生效；F16 開著的機台起動時兩個入料飛梭會先往右移再回來（感測器斷線檢查） | 兩組態 gate＝基準（兩顆各跑一次）；模擬實跑兩次、真實檔逐檔還原 CLEAN |
| 53 | **用真機那台的設定跑對照（第 30 題 A，真實路徑）：測試頭從開機到 120 跟真機一步不差**（`ffd74fd8`＋工具 `cb94360f`）：真機紀錄那台（HT9046_LS）的 system／config／setup.inf／FT005054 直接放進 `D:\HT9045` 跑模擬，跑完換回、逐檔比 MD5 一模一樣（兩次都 CLEAN）。同時把 `INDEX_SUCKER_TYPE` 照 golden 從 Gerneral.ini 讀（以前那段閘著、一直是 0；那台是 1），DAQ 型吸嘴的真空自檢從這顆起真的會走。結果：測試頭 1→200000→300000→400000→**500000**→600000→**11**→9→10→15→20→21→100→120 跟真機完全相同；120 之後的扭力幾步（12000／12101…）模擬下 golden 自己也直接跳過；真機 14101 之後回到 1 是扭力檢查 NG（真機硬體）。另外 OutArm、BinTray[0]、AutoSHT1、LoadNewICTray、TrayZLoad 五個 task 步序也跟真機一樣。**還比不到的**是操作員觸發的（AutoClean、Initial Start、Color 盤、Loader 補料）：真機是按面板 HOME／START，移植樹主畫面的面板鍵沒接（§0 第 32 題）。工具收進 `tools/flowcmp/`。**上機時要看**：Gerneral.ini `INDEX_SUCKER_TYPE=1` 的機台，Index Check 時兩排吸嘴會依序開真空、破真空再往下；D44（`bD44CheckIndexICDestroy`）開著時放完料回黏檢查會報 JAM0327 | 兩組態 gate＝基準；真機設定模擬實跑 2 次 |
| 54 | **主畫面「起動模式」下拉照 golden 翻**（`5997abda`，已推 main＋GitHub 第 49 包）：golden `cbRunStartModeChange`（main.cpp:23726-23825）照翻，wb_serve 多一個 WS 指令 `main.runStartMode`（value＝模式名稱；空字串＝只查詢；回覆帶目前模式與清單）。golden 的「改回原值」靠 VCL 設 ItemIndex 連帶改 Text，移植樹的下拉不會 ⇒ 補一步同步，不然會「畫面說改回去了，其實照新選的跑」。用真機設定照真機順序先切 Initial Start 再 START：**被 golden 自己擋下**（「Auto區有tray盤, 請先執行tray feed」並改回原值）—— 換進來的真機快照是 17:47 收的，那時機台裡有料；真機 17:29 能切是因為那時是空的 ⇒ 這包快照重現不了開頭的 Initial Start（INBOX 第 49 列）。同時發現對照工具不會按 golden 的 ShowMyMessage 框，那次卡住白跑、已停掉並換回（CLEAN），工具已補。網頁的下拉還是舊 C# 模擬器那條，已請 Steven 改接（TO_STEVEN §1）| 兩組態 gate＝基準；真機設定模擬實跑 1 次（CLEAN） |
| 55 | **Z 軸安全互鎖照 golden 翻**（`a4408436`，NB2 R105 ①）：`InArmZSafe`／`OutArmZSafe`／`SortArmZSafe` 以前一律回「安全」—— 入料臂 Z 還在下面時飛梭照樣動（`acarry.cpp:4170` 等十幾處互鎖全部放行）；`CheckIn/OutArmZNeedHome` 以前一律回「0 號馬達要回原點」—— START 時每次多做一次入／出料臂回原點、SortingBinTray 的 X／Y 移動一律被擋。8 支照 golden 翻（相依全在）。**唯一和 golden 不同**：讀 `Motor->Enable` 前先擋 NULL（移植樹與 golden 自己 :759 的慣例）—— 第一輪 gate 有 9 支沒建馬達的測試 SegFault，正式機馬達開機就建好，行為不變。**上機時要看**：Z 軸沒回到 home（LED 沒亮）或位置為負時，飛梭／出料臂的移動會被擋、START 會要求回原點 —— 這是 golden 本來的行為 | 兩組態 gate＝基準 |
| 56 | **起動模式的選項清單照 golden 填，開機把目前模式種回去**（`ff639260`）：golden `SetStartModeData`（main.cpp:24118-24234）取代空殼 —— 清單依機種／設定（`LastSet.iStartMode`、Auto Site Mapping、QA、ART、FIFO、MRT）填，最後把 `LastSet.iRunStartMode` 種回下拉。以前開機沒走這條，START／Clean Out／ATC 自檢讀到的起動模式在第一次換模式之前都是空字串（WebStart.cpp:2945 記過的缺口）。換配方、SECS、SCK ART 那幾個呼叫點也照 golden 生效。**上機時要看**：開機後主畫面起動模式應顯示上次的模式（`config` 的 LastSet）；St01 那邊開機寫 `RunMode.txt` 會跟著生效 | 兩組態 gate＝基準 |
| 57 | **出料臂會下去吸料了**（`4b2ec4f3`，NB2 R105 ②③⑦⑧）：`InitialOutArmNeedSuck` 以前一律回 false ⇒ **所有機型的出料臂都停在飛梭上方、不會下去吸**；另有 10 支 variant 連到「吸料時不標記要吸」的空樁、14 支連到多繞一圈的樁（clean-out 整盤、Magazine buffer、自動測高、ESD 讓位那幾個分支因此消失）。全部改成 golden 的原型與本體，連同 Fix 盤滿換盤（`VerifyFixTrayLink`）、手動 Step／Auto Offset／Setup Teach 時停下（`OutArmNeedCheckOffset`），以及 TfOffSet 的 Setup Teach 一對（入料臂那支以前也是回 false 的樁）。**上機時要看**：A30（Setup Teach）開著、需要教點位時，入／出料臂到每個點位第一次會停下 | 兩組態 gate＝基準 |
| 58 | **出料臂會找對的盤、移得過去**（`e1dbad7c`，NB2 R105 ④⑤）：`SearchTrayToPlace_9045` 以前一律回 0 ⇒ 永遠放第一個 Auto 盤、不分 bin；`SetOutArm_9045` 那一條以前一律回 false ⇒ 移不到放料位置。四支照 golden 翻（含 FIFO、ATK AMR、Magazine 分支），0 個閘 | 兩組態 gate＝基準 |
| 59 | **出料臂真的放料了**（`15df16ab`，NB2 R105 ⑥）：`DoOutArmPlaceToAuto`（golden 551 行）以前是回 true 的樁 ⇒ 「放料」立刻成功，沒有 Z 下降、破真空、放料後檢查、寫盤資料、計數、警報。照 golden 翻；8 段客戶／選配分支照移植樹既有的先例閘住或做型別轉接（Open/Short 結果放盤、多批次計數、OCR log、良率警報、AQL 結束關框）。⇒ 出料臂第一次能完整走完「吸 → 找盤 → 移過去 → 放」。**上機時要看**：出料臂放料會真的下 Z、破真空；放料後的檢查失敗會照 golden 報警 | 兩組態 gate＝基準 |
| 60 | **出料臂另外 4 種排列的 Setup Teach 也接回**（`14d274ea`）：ARM1 翻了 `UseOutArmSetupTeach` 之後，variant 裡 11 處「因為沒有這個方法」才寫死 false 的替身前提就死了，全部照 golden 接回（同一行）。2x2_4_23 那三段還差 `fMain->Pause()` 無參數版，另列 | 兩組態 gate＝基準 |
| 61 | **Index 位置出錯會照 golden 停機了**（`ebc8e41c`，46 個呼叫者）：`ShowIndexMotorError` 以前是空的 ⇒ Index 四軸位置檢查判定出錯時，錯誤被吞掉、機台照樣跑。現在照 golden 停所有馬達、跳「Index Position error, Index 4 Axis Need home」、要求回原點 | 兩組態 gate＝基準 |
| 62 | **單軸回原點真的會動了**（`23c264b9`，27 個呼叫者）：`ProcessSingleMotorHome` 以前一律回「已經回好」⇒ AOI 四軸、Fix3 氣缸流程等單軸回原點實際上沒動。照 golden 翻（uhome.cpp:415-644）。gate 時 sim 有一支 `BAD_COMMAND`（執行檔剛連結完就被拿去跑），單獨重跑通過 | 兩組態 gate＝基準（見左） |
| 63 | **Fix3 盤滿換盤照 golden 做了**（`e593dede`，NB2 R103）：`UseFix3Cylinder`（393 行）以前一律回「氣缸已到位」、`DoFix3FullTray` 被一支 static 樁遮住（真本體早就在 SortingBinTray.cpp）⇒ Fix3 盤滿的整段流程被跳過。照 golden 翻，並接上 SMHOME 的單軸回原點。**上機時要看**：Fix3 盤滿時會照 golden 動氣缸、換盤 | 兩組態 gate＝基準 |
| 64 | **入料臂的附加功能照 golden 做了**（`7a78e2ba`，Preciser 位置精修／Die Clean／底部 2D 條碼／入料旋轉站）：以前判斷式一律「不需要」、預設一律「已做完」⇒ 不管機台有沒有裝，入料臂都不會做這幾項。照 golden 翻（ainarm9045.cpp:3107-3340），本體都在移植樹、只補宣告；只有「ART 重測不旋轉」那一段閘住（旋轉站設定頁沒翻）。**上機時要看**：裝了這些選配的機台，入料臂會多做位置精修／旋轉 | 兩組態 gate＝基準 |
| 65 | **出料臂的附加功能照 golden 做了（出料旋轉站）**（`1f425fa2`）：入料臂那一側的鏡像。出料旋轉站照 golden 做；**AOI 與 Fix AI CCD 那幾項的動作本體移植樹沒有**（TFrmAOI、矽格 Fix AI），判斷式裡只拿掉那一項、動作閘住 —— 開了 AOI 的機台不會卡住，但也還是不做 AOI（跟以前一樣），列在 INBOX 第 89 列 | 兩組態 gate＝基準 |

### 接下來

* 📋 **排在白天的大件（我刻意沒在夜間做）**：① `ShowTestHeadComp1`（golden 980 行，會存測試模式、寫 lastdata、改各 site 的使用旗標，START 也會呼叫）＋`ShowTestHeadComp` 其餘（NB2 R72 ATC-2）；② St01 讀寫檔普查的 ② 類（有本體沒人呼叫）逐個查過：`InitialGaliDelayCount`／`WriteAutoTeachTable`／`bOffsetClean` 跟馬達或教導綁在一起、`AddByLotCount` 在 J1 的出料臂樁裡、`N23UseLotInfoFile`／`TransformTemperature_AirStream` 是 St01 的 LotInfo、`md5_Folder` 已接（S92）、`RotateBootLogIfNeeded` 這一輪接了；③ J10（關程式存 machinerecord 等 3 項）跟 St01 的 S95 在同一段關機碼，提議併給 St01（TO_STEVEN §4 22:3x）。
* ⚠ **你要知道（不用決定）**：St01 盤點的 J1 —— 用 `aoutarm9045.cpp` 的機種，**出料臂放 IC 到 Auto 盤的整段動作是一個回「放好了」的樁**（`:220`；golden 是 551 行、含 15 處馬達動作）。這是運動控制的翻譯，我沒在夜間動（INBOX 第 66 列）；HT9050（HT9046_LS 家族）走不走這支檔還要用連結結果確認。
* ✅ **第 9 條做完**（`66c2480e`）：阻塞框在等的時候，塔燈／蜂鳴器／面板鍵燈照 golden 框自己的 Timer1Timer 動；30 秒沒有網頁就自動開 Edge 正式版畫面（之後每 2 分鐘最多一次、一個框最多 3 次）；是／否框可用面板 Alarm Reset 消音。3 個審查 agent 找到 11 條（2 條 medium：鍵燈沒閃、**HT9050 上等待期間 1203 的 DI 凍住 ⇒ 沒辦法消音**），全修；兩組態 gate＝基準。⚠ 在機台上要看的：框開著時 Retry／Skip 鍵燈閃、按 Alarm Reset 會停叫、按 Start 會啟動（這三件移植樹的 ctest 測不到）。
* 安全 PLC 五個閘 ✅（`4abcdf0b`）。下一個：第 8 條（網頁硬體鈕 `hw.access`）—— 卡在上面「新的要你決定的」第 4、5 題；St01 的分支（`3a7a252f`，S57／S86／S88～S90）Steven 說「暫時先不要 build」，等他說再代跑。

### §3 紅燈／雜訊（這一段）

* **實跑 wb_serve 會寫到測試機橋接程式的檔**（0926 22:37 BootLog 實跑時量到，不是新問題）：wb_serve 內嵌的 GPIB 引擎（St02 `801f3a0d` 起開機自動啟動）關站時照 golden GPIB 程式寫 `D:\GPIB9045\system\general.ini` 的 `LastFile=`、`GpibString.dat`、`D:\GPIBLOG\Log\<時間>.txt`。這次 general.ini 用 gate 前的備份還原、新 log 刪掉；`GpibString.dat`（22 bytes，內容 `Fullsites 00000000`）沒有事前備份，留著。之後實跑的備份清單加這兩個目錄。另：MachineMotors_HT9050 一次 BAD_COMMAND（同一類，單獨重跑通過）。
* **哨兵 0926 22:1x**：接線 selftest OK；配方 65 個目錄／64 個有 Contact.Data（＝新筆電基準）；system／config／IniData 每輪 gate 前後 0 變動。absence-claim 哨兵抓到 **1 條過期的「不存在」**：`cBinSel.cpp:3221` 說 `SetNormalOrPrime` 不存在，OPMODE 之後已經有了 —— 查過它在移植樹整支都是網頁顯示閘（呼叫＝no-op），`SetPrimeButton` 靠畫面元件那一半理由照樣閘著、行為不變，只同行補 `AI(W906-NL-ABSENCE)` 註記；重跑轉綠。
* **BAD_COMMAND 兩次（同一類）**：模擬組態的 `W7_HotplateLoaderVibrate`（18:37）、`W7_C1_CleanOutFinish`（19:4x）各一次「Not Run（BAD_COMMAND）」—— 都是 **130 MB 以上、剛連結好**的測試執行檔，檔還在、單獨重跑都通過（1.76 s／0.44 s）。看起來是防毒掃描剛寫出的大檔時擋住啟動。不是回歸，但會讓 gate 看起來多一項失敗；每次都要單獨重跑確認。
* `IniFiles_Win32Diff`：兩組態 ctest 都「Process not started [operation not permitted]」，之後 exe 消失 —— 看起來是防毒／EDR 把它隔離了；重新連結後兩組態都通過（9.9 s／10.1 s）。它大量呼叫 `WritePrivateProfileStringA`，可能被當成可疑行為。之後每輪 gate 再看一次。
* `WebMotorAccess`：模擬組態 `-j 8` 下逾時一次（159 s），單獨重跑 0.82 s 通過；上一輪同樣負載是通過的。下一輪 gate 若再出現就認真查（可能是測試裡有等待牆鐘的迴圈）。
  **0927 08:0x 第二次**（MEMO 那一輪，126.94 s）。查了：它是純邏輯測試（假後端，`SleepMs` 是假的、沒有等牆鐘的迴圈，只連 `WebMotorAccess.cpp`／`JsonWriter`／`cJSON`），單獨重跑 0.82 s 過、**8 路並行重跑 64 次全過**，重現不出來。log 看起來停在「W5B-11: non-1203 accept」那一行，但那不可信：stdout 接管線是全緩衝，被殺時最後約 6.6 KB 沒寫出來。⇒ `42ea830b` 把這支測試的 stdout 改成不緩衝，**下次再逾時，log 的最後一行就是真正停住的地方**，到時候再查。

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

* **0927 03:41 `TransformFuntion` 在模擬組態 ctest 回「Not Run」、0 秒、沒有輸出**：exe 在（137 MB，03:31 剛連結完），直接執行 15／15 全過 ⇒ 啟動時的偶發失敗，不是回歸；同類的還有 0926 的 `ContactForceLoad`。下一輪 gate 若再出現同一支，就要查是不是防毒在掃剛連結好的大 exe。
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

* **0927 12:0x 背景 gate 的孤兒又疊跑了（記憶裡已經有的坑）**：ZSAFE 第一輪 SegFault 後我只 `kill` 了外層 bash，`build.bat` 的 cmd 子程序沒跟著停，12:00 自己開了 sim ctest（跑的是沒擋 NULL 的舊 binary），跟重跑那輪的 ship ctest 同時讀寫真實 system 檔 11 分鐘，而新那輪的 sim 同時在同一個 build_sim 連結。12:13 發現後用 PID 停掉（ctest 25004、build.bat 34700）。那輪 ship 的數字只拿來證明「SegFault 消失」（同時跑只會多出假失敗、不會造出假通過）；sysguard 見第 55 列的驗證欄。**下次停 gate 要停整棵程序樹**（`taskkill /T /PID <build.bat 的 cmd>`），停完用 `Get-CimInstance Win32_Process` 確認沒有 ctest 才重跑。
* **0927 11:4x 用真機快照照「開頭的操作順序」跑，前提錯了**：我以為換進來真機那台的設定就能照它 17:29 的順序切 Initial Start；其實那包快照是 17:47 收尾時收的（機台裡有料），golden 本來就會擋。golden 擋下來證明 RSMODE 翻對了，但這一段對照要換思路（INBOX 第 49 列）。另外 flow_run 只會回 query 類警報框、不會按 ShowMyMessage 框，那次白跑了 2 分鐘，已補。
* **0927 09:5x 把一批對的改動當成會卡死而退回**（09:5x～10:0x 更正、重做）：我看到 `aHotPlateSubstrate.cpp:137` 的 `TMySucker::Suck()` 永遠回 false，就斷定吸嘴泵接上後 type 1 會卡死，整批退回、還把這個「阻擋點」寫進 INBOX 84 與 `caf9b389` 的 commit 訊息。其實那一段在同檔 `:79` 的 `#if 0` 裡，0924 A4-6 已經退役；連進去的是 `mykitsuck.cpp:2207` 的真翻譯（`nm` 量到 aHotPlateSubstrate 的 obj 0 個 TMySucker 定義）。「這段碼是不是活的」要先量（`#if 0` 範圍或 nm），不是看到函式本體就算。`caf9b389` 的訊息那一段是錯的，以 `33fd6570` 的訊息與本條為準。
* **0927 06:4x INBOX 第 75 列寫「`mmTesterLog` 還沒找到清空的門檻」是錯的**（08:2x MEMO 收尾時改正）：我只看了 `Interface/TesterTCP_Socket.cpp:399` 那一行 `Add`，沒往上看 —— 同一個函式 :387-394 在 `mmTCPIPCommLog` 超過 2000 行時就連 `mmTesterLog` 一起清（golden 同）；而且它 0925 起已經是 St01 真的存行的 `TfLotInfoLogMemo`。這句話當時讓 MEMO 看起來要多一個「另案」，差點把 `mmTesterLog` 排成要另外處理。說「沒有 X」之前，要看完整個函式，不是只看命中的那一行。
* **0927 05:1x 同一個坑第 11 次，而且第 42 列的 hook 沒擋到**：寫 CHGLOG 的 commit 訊息時又用了 `python - "<檔>" <<'EOF'` 帶 `\\`。這次減半後剛好是我要的文字（commit `752130a5` 的訊息內容正確、沒寫壞檔），但 hook 該擋沒擋 —— 它的 regex 只認得 `python <<`／`python - <<`，heredoc 前面多一個參數就漏了。已改成「同一行 python 之後任何位置出現 `<<`」（`scripts/ops/deny-heredoc-backslash.ps1`）：10 組假輸入，改前 2 組該擋沒擋、改後 0 組不符。教訓同第 42 列：守門員自己也要拿「剛漏掉的那一個」當測試案例。
* **0927 03:2x 同一個坑第 10 次（寫 night-loop skill 的哨兵列）**：剛寫下「含反斜線一律用 Write 工具」四十分鐘後又用了 heredoc，5 個控制字元寫進 skill（當場修回）。筆記擋不住習慣 ⇒ 改機制：第 42 列的 hook。
* **0927 03:1x heredoc 第 9 次，這次真的寫壞了 INBOX 第 82 列**（當場修回）：Python 字串裡的 `D:\\HT9045\\backup…\\recipe_outliers.py` 被 shell 吃成單反斜線，Python 再把 `\b`／`\n`／`\r` 解成退格、換行、CR 寫進檔案。Python 印了 `SyntaxWarning` 才發現。順著這個查下去，才找到 0925 以來同一種錯留在各文件的 27 處（第 41 列）。規則不變但要真的做到：**含反斜線的字串一律用 Write／Edit 工具寫進 .py，不經過 heredoc**；用 `chr(92)` 組反斜線。
* **0927 02:38 開機實跑留下一個空目錄 `D:\HT9045_Log\MDB_UpdateLog`**（03:08 gate 後檢查發現、已刪）：smoke 會刪掉這次新寫的**檔**，但不刪這次新建的**目錄**。裡面是空的，對機台沒有影響；smoke 產生器已改成也記下目錄清單、跑完刪掉新建的空目錄（`backup\night_tools_20260927\mk_smoke_initmem.py`）。
* **0927 02:38 INITMEM 實跑後 `D:\GPIB9045\system\general.ini` 被留在改過的狀態約 3 分鐘**（已還原）：我用腳本從 smoke_bootlog.py 衍生新 smoke，把「比對＋還原 GPIB 兩個檔」接在檔尾，但原腳本最後一行是 `sys.exit(...)` ⇒ 我加的段落根本沒跑到，輸出也就少了那兩段。發現輸出不完整後當場查：`general.ini` 只有 `LastFile=` 一行變了，用 0926 的備份（MD5 就是跑之前的 `54bf26be…`）還原；`GpibString.dat` 內容相同（22 bytes「Fullsites 00000000」），只是修改時間變了。產生器已改成先拿掉原本的 `sys.exit` 再接、最後補回（並斷言原檔結尾真的是它）。另：同一段修正又被 heredoc 吃了一次 `\\n`（今晚第 7 次）。
* **0927 02:3x INITMEM 的效果說過頭了**（`bf45351f` 的 commit 訊息、本報告第 36 列初版、TO_STEVEN 給 St01 那列）：我寫「`SiteData` 要等打開 Setup 頁才補、在那之前遠端指令與 LotInfo 讀到的都是 0」。寫更新包 README 時回頭查 `SeedSiteData()` 的呼叫點才發現：它在開機 `wb_serve.cpp:4158` 就跑了。我只讀了 St01 的註解（「FormShow 時 SiteData[13]=0x0」是他加補值之前量的），沒查呼叫點。程式沒錯、呼叫位置也沒錯，錯的是對效果的描述 —— 第 36 列、TO_STEVEN 已更正；`bf45351f` 的訊息壓在後面 5 顆 commit（含一個 merge）底下，不為了一句話改寫歷史，在這裡記下；`cmydef_InitialMemory.cpp` 檔頭同一句下一批改。
* **0927 02:1x 用 shell 的 `nm` 查符號，但 `nm` 不在 PATH**：指令是 `nm … 2>/dev/null | grep`，錯誤被吞掉 ⇒ 印出「沒有定義、也沒有參照」，差點當成「`GetTotalYield_Str` 沒人用」。改用 `C:\MinGW\bin\nm.exe` 重量，才看到 `uPAT_Function.cpp.obj` 有未解析的參照（INBOX 第 81 列）。規則：**`nm` 一律寫完整路徑，不准配 `2>/dev/null`**。
* **0927 02:0x mtRow 的新斷言寫錯了**（出貨組態 `RecordTimeInfo` 20／21）：我斷言建構完成後 mtRowA 是 420×180、YItem 9，但 golden 建構子自己在最後（cObserver.cpp:526）呼叫 `SetSiteYieldDiagram()`，執行期會把 mtRowA／B 的 Width 改成 860（iShtRow≤2）、每個 mtRow 的 YItem 改成 iShtCol+1 —— 這是 golden 行為，程式沒錯。mtRowD 那條會過只是因為 iShtCol+1 剛好也是 9。改成只斷言執行期不會動的 XItem 11、Height 180，外加 Width 符合 golden 的規則；只重建、重跑這一支。
* **START 呼叫點普查從 0926 14:56 紅到 0927 01:3x，我每輪 gate 都沒跑它**（NB2 R80 抓到）：CLAUDE.md 寫「CI 形式 `--check 33 30 3`」，但它從來不是 ctest，我的 gate 腳本也沒叫它 ⇒ St02 `6fff0960` 加了 `HandlerGpibMsg.cpp:717` 之後，工具回 FAIL 一整天沒人看到。所幸那一處其實是閘住的（`#define W906_REMOTE_START_WIRED 0`），沒有多出會啟動機台的路。處置：工具照 NB2 的 patch 也認「同檔 #define 成 0 的巨集」、數字改 34／30／4，並接成 ctest `START_SitesCensus`（下一批）—— 之後 gate 自己會紅。
* **0926 寫的 `check_env_all.py` 從來沒量到東西**（0927 01:3x 發現）：它用 `add_test([=[名稱]=]` 找測試，但這個產生器寫的是 `add_test(名稱 "…")` ⇒ 永遠印「tests 0 without ENVIRONMENT 0」，看起來是綠的。0926「201 支都帶轉向變數」那句話當時的證據其實是空的；所幸 NB2 R78 用別的方法獨立量過 201 支，結論成立。新的 `check_env_d5.py` 兩種寫法都認，而且 0 支就回失敗；備份到 `D:\HT9045\backup\night_tools_20260927\`。教訓同 memory「不可能當掉的 gate 不是 gate」：**量出 0 的時候要先懷疑量法**。
* **0927 00:4x 第一版 LOGSINK 重複宣告了 `meShuttle1/2`**：以為 `fMain.h` 沒有這兩個成員，同一行另外加了 `TMemo *meShuttle1/2`，其實 :222-223 早就有（型別是 `TfMainMemo` 替身）。兩組態語法檢查當場擋下（redeclaration），改成用既有成員，沒進任何 commit。教訓：加成員前先 grep 那個名字在整個 header。
* **0927 01:1x ENV-BANNER 的呼叫點放錯行，F5 契約探針抓到**（`WB_F5Contract` 紅）：我把呼叫接在 `wb_serve.cpp:3755` `recipeRedirect` 那一行尾，而 `tools/webprobe/f5_contract_probe.cjs:173` 會把「`recipeRedirect` 到 `return 2; }`」那一段原封抽出來單獨編譯 ⇒ 抽出來的程式沒有 helper 本體，連結失敗。改接在上面那個大 printf 的最後一行（:3749，沒有探針會抽它），探針四節全過。另：我手動重跑探針時一直「g++ 靜默回 1」，是我的 shell PATH 沒有 `C:\MinGW\bin`（cc1plus 找不到 DLL，rc 127），不是編譯器壞了；ctest 裡有這個 PATH。
* **0927 01:0x～01:1x 反斜線又被 shell 吃掉兩次**：① 用 `sed` 把 `server\run_wb_serve.cmd` 塞進一般的 Python 字串 ⇒ `\r` 變成真的 CR 寫進本報告，當場用位元組比對找到、修回（報告裡 0 個單獨 CR）；② 用 heredoc 追加測試片段 ⇒ `\\n` 變 `\n`，執行前讀回腳本時發現、改用 Edit 工具修。到今晚為止同一個坑 6 次 —— 規則再寫一次：**凡是含反斜線的字串，一律用 Write／Edit 工具寫進 .py，不經過 shell**。
* **0926 18:0x 寫的 lastdata／config.ini 沙盒 helper 只認反斜線**（`W906_LastDataPath`，`cprod.cpp:4277`）：`config.ini` 的路徑是 `AuthPath`＋檔名，而 ctest 的 `W906_AUTH_PATH` 是正斜線 ⇒ 整串被接到沙盒後面、寫入失敗。當時那支測試剛好沒有 ENVIRONMENT，所以沒量到；0927 00:0x 補上轉向變數後 `LastDataSandbox` :84 紅了才發現。沒寫到任何真實檔（無效路徑）。已修（`012fbc13`）。教訓：路徑工具的測試要同時餵正斜線與反斜線。
* **0926 21:4x heredoc 吃反斜線又發生三次**（`mkpkg22.py` 產生器、`stage_cmake_atc.py`、`fix_func_color.py`）：`\n`／`\r\n` 被 bash 吃成真的換行。三次都在寫壞任何專案檔之前被斷言或語法錯擋下（mkpkg22 的 assert、另外兩支直接重寫）。之後**凡是 Python 腳本一律用 Write 工具寫檔**，heredoc 只留給不含反斜線的片段。
* **0926 19:5x 一顆 commit 的訊息寫多了**（`85a9542e`）：同一個指令裡的 heredoc Python 因反斜線被吃掉而語法錯誤、沒有執行，但後面用換行接的 `git add`／`git commit`／`push` 照跑 ⇒ 那顆實際只含 INBOX 第 60 列，訊息卻說也做了第 61 列與 NIGHT_REPORT。已推上去不改歷史，由下一顆補上並註明。教訓：寫檔的腳本一律用 Write 工具，commit 一律接在 `&&` 後面。
* **0926 19:3x 推 GitHub 更新包 14 之前漏跑了 7z 密碼掃描**（權杖／私鑰那支有跑）：推完才補掃，0 處。之後照順序：兩支掃描都跑完才 push。
* **0926 18:0x 我自己的新測試寫壞了一次真實的 `D:\HT9045\config\config.ini`（已還原）**：以為 `WriteLastDataFile()` 只寫 lastdata，新測試 `LastDataSandbox` 直接呼叫它 ⇒ 它另外一定會寫 `AuthPath+"config.ini"`（Vibrate_Time、P65_QAMode、SocketContact，再加 O_Count 接觸壽命計數，`cprod.cpp:2082-2175`），把 `O_14～16ContactSet` 從 6000 寫成 0、`iVibratorUnloader` 寫成 0。sysguard 抓到（`CHANGED OTHER config/config.ini`，修改時間＝那支測試跑完的時刻），從 `D:\HT9045\backup\gate_sysguard\gbtrial\` 的快照還原兩次（第二次是我改成 `bNotContact=true` 還是寫），MD5 回到快照值。現在的測試照 `test_ga1_cprod.cpp:354` 先把 `AuthPath` 指到沙盒。教訓：**呼叫一個會寫檔的 golden 函式之前，先把它整支讀完**，不要看名字猜它寫哪裡。這也證明 OPMODE 翻活之前，`config.ini` 那條也得圍（INBOX 第 55 列）。
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
