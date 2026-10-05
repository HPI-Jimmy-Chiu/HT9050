# 給 Kevin：工作卡與回答（Jimmy／筆電 → Kevin）

> **這個檔只有 Jimmy 這邊（Jimmy 本人或筆電的 Claude）會寫。** Kevin 請不要改這個檔，回覆一律寫在你分支
> `v906/kevin-handoff` 的 `docs/handoff/FROM_KEVIN.md`（聊天寫 `CHAT_KEVIN.md`）。兩個檔各自只有一個寫者，git 合併永遠不會衝突。
> 開始：20260929（Jimmy 18:0x：Kevin 接單協助；「A，Index 流程交給 Kevin」）。規則比照 `TO_STEVEN.md` §0（Steven 那套已經跑了 3 天）。

## 0. 規則

1. **每次開工先 `git fetch`**，讀這個檔 §1「我們正在改的檔」與 `TO_STEVEN.md` §1（Steven 那邊正在改的也不要碰）。
2. **開工前先認領**：在 `v906/kevin-handoff` 的 `docs/handoff/FROM_KEVIN.md` §1 寫一行「我接 K-xx、會動哪些檔」，**只改那個檔、先推一顆小 commit**，再開始做。
3. 做完在 `FROM_KEVIN.md` §2 寫 commit hash／檔案位置和驗證結果；有問題寫在 §3。筆電的回答寫在這個檔的 §4。
4. **急的事打電話給 Jimmy**；這條管道一次來回大約幾十分鐘到幾小時（筆電的夜間迴圈每 20 分鐘讀一次 `v906/kevin-handoff`）。
5. `v906/kevin-handoff` 不開 MR、不合進 main；要交程式碼時另開 `v906/kevin-*` 工作分支（BCB6 的東西見 K-02 的交付方式）。
6. commit 作者請用你自己的公司信箱（Jimmy 18:0x）。

## 1. 我們正在改的檔（這些先不要動）

| 誰 | 檔 |
|---|---|
| 筆電 | （20260930 14:3x 更新；原本那列是 AMB-L2 時期的，已上 main）移植樹 `HT9011UC_Cpp_V3.33.906.0/` 的：INBOX 115 B 那 10 個閘所在段落（`PowerSavingMode.cpp`、`VacuumUnit/VacuumUnit.cpp`、`ainarm9045.cpp`、`cinitial.cpp`、`csystem.cpp`、`uHeaterThread.cpp`，清單在 `docs/INBOX115_TRIAGE_20260930.md` §3）；INBOX 121 `Motor/mymotor.cpp` SetHTrayPanel、`cinitial.cpp` SetSimuScreenPara |
| 你這邊 | 你只有 BCB6，移植樹（C++）原則上不用碰；golden（`HT9011UC_Code_V3.33.906.0_20260618`）是唯讀 |

## 2. 須知（不用回）

- **共用區交付包的 7z 密碼（Jimmy 1001 13:4x：「未來統一用一個密碼」「所有人都要知道密碼，這不是機密，也不會有任何風險，否則無法多人協作」）：`〔交付 7z 密碼：已遮，不放 GitHub（RULINGS_20261001 第 40 條）〕`**。以後所有共用區（`U:\共用區\`）的交付包都用這一組。例外：9/25～10/01 打的包（例 0930 的 K-01 兩包 `U:\共用區\HT-9050\K01_golden0618_HT9050snapshot_20260930\`）用的是退役的那組 `〔交付 7z 密碼：已遮，不放 GitHub（RULINGS_20261001 第 40 條）〕`。這兩組只寫在交接檔（公司 GitLab）；公開的 GitHub 機台包不放。
- **20260930 14:5x 起 GitLab 的預設分支改成 `main`**（原本是 0925 起凍結的 `feat/v912-port`）：新開的 MR（網頁或 push option `-o merge_request.create`）預設就指向 `main`；`main` 跟原本的 feat 一樣受保護（Developer＋Maintainer 可推／可合，**禁止 force push**）。你自己的 clone 請跑一次 `git remote set-head origin -a`。
- 移植樹是把 golden BCB6 V3.33.906 翻成 C++17（`HT9011UC_Cpp_V3.33.906.0`），**以 main 為準**（`feat/v912-port` 0925 起停用）。
- HT9050（Jimmy 的開發機）＝golden 的 `Type_HT9046_LS`＋PCIE-1203 運動卡（RULINGS_20260926 第 25 條）。**Index 只有 Z1（M14 MTestZ1，1203 軸）會上下**，Y1／Z2／Y2 都關閉（RULINGS_20260929 第 6、8 條）；Galil 卡這台沒有（`INDEX_MOTION_CARD=0`）。
- 9050 的 IO 表裡吸嘴、Tray 臂上下氣缸與吸盤、滿盤感測器目前 Enable=0，**硬體還沒接**（RULINGS_20260929 第 11 條）。
- 9050 用的工單是 `machines/HT9050/recipes/FT005054_9050/`（第 1 臂、Shuttle 1；說明 `FT005054_9050_DIFF.md`／`_VERIFY.md`）。兄弟機給的 FT005054 是第 2 臂、Shuttle 2，9050 上沒有那一組硬體。
- **現在常溫優先，加熱模式晚點**（Jimmy 0929 16:2x）。
- 動作流程對照工具在移植樹 `tools/flowcmp/`（`flow_run.py`／`flow_cmp.py`，比 golden State Record 的 task 環與判斷變數）；T1 那份比對與分類在 `docs/handoff/T1_20260929/`、`ST02_T1_CLASSIFY_20260929.md`（在 `v906/steven-handoff`）。
- **20261005 11:5x 新規則（Jimmy；RULINGS_20261005 第 4 條，全文 `AGENTS.md`「懷疑是 HT9050 機台設定或工單的問題」一節）：懷疑是機台設定或工單問題，先比對機台快照再下結論。** 起因：有同事的 AI 回報機台參數有問題，最後查到是它電腦上的參數跟機台不一樣。做法：`python tools/machine_sync/machine_sync.py check` —— `NOT SYNCED` 就 `apply --yes`（自動備份→複製→逐檔比 MD5）後重看，同步前量到的不算機台問題；`SYNCED` 才是真問題的機率高，附上工具印的 `machine snapshot` 兩行提出討論；做完 `restore <備份資料夾>`。工具先比 GitHub（機台約每 30 分鐘推一次）；讀不到 GitHub 自動改比 GitLab `machines/HT9050/snapshot/`（筆電 1005 已補到機台最新的快照，之後夜間迴圈每輪跟上）。只裝在開發機／模擬，別台真機台不裝。

## 3. 工作卡（接之前先在 FROM_KEVIN.md 認領）

| # | 工作 | 會動的檔 | 驗收 | 狀態 |
|---|---|---|---|---|
| K-01 | **用 golden BCB6 模擬版（`SOFT_SIMULTE`）錄 State Record，給流程對照工具當標準答案**（先做這張）。條件：**常溫**、工單 **`FT005054_9050`**、machine 設定用 HT9050 那一套（`machines/HT9050/`；沒有的照 Jimmy 的 `D:\HT9045\system`）。每份都要附當下的 `system\`、`config\`、`IniData\`、`setup.inf`（筆電要用同一套設定換進去重播，今天 10:58 那份就是缺了 6 個檔要從別份補）。建議先錄這幾段，每段一份：① 開機→Lot Start→HOME→START→入料到 Shuttle→出料到 Auto 1→第一次出盤、補空盤→PAUSE；② Tray Edit 把 Auto 1 填滿→退盤→START→再出一盤（T1 後半段，移植樹還沒有 Tray Edit，這份先留著）；③ Clean Out／One Cycle 結束；④ AutoClean 觸發一次。**PAUSE 要在 task 環還沒被蓋掉前按**（環每個 task 最後 500 筆；今天那份有 8 個 task 已經滿了）。 | 只新增錄好的資料夾（放共用區或 `v906/kevin-handoff` 的 `staterecord/`，大的話放共用區、在 FROM_KEVIN §2 寫路徑） | 每份附：按了哪些鍵、幾點幾分（EventLog 對得上）、golden 版本號；筆電用 `tools/flowcmp` 重播得起來 | 可以接 |
| K-02 | ⚠ **1001 13:2x 改範圍＝審 Frank 910 樹的 Index（見 §4）**。原文：~~**HT9050 的 Index 正確流程（BCB6）**~~——Jimmy 0929 18:0x：「A，Index 流程交給 Kevin」（原本 Jimmy 自己要寫，RULINGS_20260929 第 8 條；**寫完 Jimmy 看過再定稿**）。9050 只有 Z1 上下（M14，1203）；golden 第 1 臂的下壓是透過 Y1 的聯動命令（`Z1DownZ2Up`），Y1 關掉後 Z1 不會被叫到——這就是要重寫的地方。範圍：Index 臂的 task（`TestHeadMotorTask`、`TestYTask`／`TestYFrontTask`／`TestYRearTask`、`IndexStatus`、`RearTest*Task`、`AutoCleanIndexTask` 裡用到 Index 的部分），Shuttle 與其他模組照 golden 不動。**底版**：golden 906（移植樹的翻譯對照）；要改用 V912 當底版請先在 FROM_KEVIN §3 問。 | 你的 BCB6 工作副本（不是 golden 本身）；交付＝對 golden 906 的 diff＋一份流程說明（每個 task 的 case 表：做什麼、等什麼、下一步） | Jimmy 審過；你那邊 BCB6 模擬版跑得起來，並錄一份 State Record（同 K-01 的條件），筆電拿來當 Index 的標準答案 | 可以接（先寫流程說明給 Jimmy 看，再寫程式） |
| K-03 | **golden 行為諮詢／覆核（唯讀）**：筆電或 Steven 在 FROM／TO 檔問「golden 這裡為什麼這樣」時回答；也可以覆核筆電翻譯的 golden 行號 | 只寫 FROM_KEVIN.md | 附 golden 檔名行號 | 隨時 |
| K-04 | **golden Timer 覆核（唯讀，BCB6）**——Timer 排程表計畫書 `HT9011UC_Cpp_V3.33.906.0/docs/TIMER_TABLE_PLAN.md` §5.6（RULINGS_20261001 第 39 條：「核芯邏輯寫好後交由其他人員處理，Kevin 也行」）。① 主畫面 14 支＋§5.3 的 24 支（程式活著就跑的 C 類），逐支比 golden 906（0618）與 V912 的本體，列出 V912 修過的地方（例：Timer1 main.cpp:2928 中途 return 沒放回 `bRunTimer1` ⇒ V912 :3013 RogerYang 20260823 修了）；② 抽查 `docs/TIMER_CENSUS.md` 的 A／B／C 分類（工具自動分的）；③ 每支 Timer 裡哪幾段只給某些客戶碼／硬體，9050 用不到 | 只寫 FROM_KEVIN.md（或一份 md 放 `v906/kevin-handoff`） | 附 golden 檔名行號；C++ 翻譯的人照你的結果決定跟不跟 V912 | 可以接（排程表合 main 之後） |
| — | S-01～S-03（在 TO_STEVEN）要跑 wb_serve，你那台沒有 MinGW／CMake，**先不接** | — | — | 不接 |

## 4. 回答你的問題

| 時間 | 你的問題 | 回答 |
|---|---|---|
| 20260929 18:0x | ① 可以推 `v906/kevin-handoff` 嗎 ② 候選四項怎麼排 ③ commit 作者用哪個信箱 | ① **可以推**（只放 FROM_KEVIN.md、CHAT_KEVIN.md，不開 MR、不動 main）。② **先 K-01（錄 State Record），再 K-02（Index 流程，Jimmy 18:0x 決定交給你）**，K-03 隨時，S-01～S-03 先不接。③ **用你自己的公司信箱**（跟 Steven 用 steven@honprec.com 一樣，git log 看得出是誰）。 |
| 20260930 15:46 | K-01：golden 用哪一棵？ | **用 `906.0_20260618`**（C++ 移植樹逐行照翻的就是這一棵；換成 0625／906.2／906.3／906.5 會多出跟移植樹無關的差異，比對就不準，T1 用 906.2 錄的那份也只當參考）。已放在共用區 `U:\共用區\HT-9050\K01_golden0618_HT9050snapshot_20260930\` 的 `golden_HT9011UC_Code_V3.33.906.0_20260618.7z`（原始碼 886 檔、不含 .svn；加密，密碼同歷次交付包；SHA256 在同資料夾 `_README_FIRST.txt`）。⚠ 這棵的 `MachineType.h:43` 是 `//#define SOFT_SIMULTE`（關著），錄模擬版要在你那份把 `//` 拿掉；建置輸出不要蓋到你機台正式用的 EXE。 |
| 20260930 15:46 | K-01：system／config／IniData 的底 | 同一個資料夾的 `HT9050_sim_runtime_snapshot_20260930.7z`＝筆電 `D:\HT9045` 現在 HT9050 模擬用的 `system\`、`config\`、`IniData\`、`setup.inf`（2387 檔，含工單 `FT005054_9050`；已含 0930 馬達表 M35／M36／M38／M39／M40 Enable=0）。照 K-01 的步驟：先整份備份你自己的那四樣 → 換進去錄 → 錄完還原並比對。 |
| 20261001 12:0x | 09:20 K-01 錄製改由誰做（A 筆電錄／B 開放認領） | **轉 Jimmy 決定**（NIGHT_REPORT §0 第 28 項），他回了寫在這裡。在那之前：**請照你 §1 先把 golden 0618 模擬版 EXE 建好放共用區**（A、B 都用得到；建好在 FROM_KEVIN §2 寫路徑與 SHA256）。FYI：Frank 1001 在他的 910 樹（9050 客製）用模擬跑完一輪並錄了 State Record（筆電這台 `D:\HT9045\Staterecord\2026-10-01 10_50_09`）；之後 9050 專屬的 Index／Shuttle 流程對照以那一份為主，K-01 的 golden 0618 紀錄用在一般流程的對照。 |
| 20261001 13:2x | K-01 錄製、K-02 範圍（Jimmy 1001 13:2x「照建議」） | ① **K-01＝在筆電錄**：請把 golden 0618 模擬版 EXE 建好放共用區 `U:\共用區\HT-9050\`，在 FROM_KEVIN §2 寫路徑與 SHA256；Jimmy 安排在筆電上跑一輪（筆電的 `D:\HT9045` 就是 HT9050 模擬設定；跑之前備份、跑完還原）。② **K-02 改成「審 Frank 910 樹的 Index」**，不要再以 golden 906 為底重寫——Frank／EastSun 在 910 樹已經寫好 9050 的 Index（`DoTestHeadMotorFP()`、`MoveIndexZ()`、`CheckIndexStatusERRSH1()`、`atester_FinePitch.cpp` 3,040 行），Frank01 的 F-01b 照它翻進 V906。你這張卡改成：用你對 golden 與機台的經驗審 910 那份 Index（以 `atester_FinePitch.cpp` 為底）——哪裡跟 golden 906 不一致、哪裡對 9050 的硬體（只有 Z1 上下、M14 走 1203）不對，寫在 FROM_KEVIN §3；Frank01 翻譯時照你的審查結果。910 樹的盤點在 main 的 `HT9011UC_Cpp_V3.33.906.0/docs/FLOW9050_PORT_LEDGER.md`；910 樹本身會放在 `ref/frank-910-9050` 分支（Frank01 的 F-02）。 |
| 20261001 14:3x | 13:5x K-01 ① 模擬版 EXE 已交共用區 | **收到，謝謝**（SHA256 記下了：7z `c59ed6c2…`、包內 `HT9045.exe` `fe3b4513…`）。錄製在筆電做，**Jimmy 1001 14:2x 定在下個上班日**（要有人按 START；錄之前備份 `system`／`config`／`IniData`，錄完還原）；錄好會在這裡回你 state record 的路徑。K-02 照你 13:5x 寫的：Frank01 的 F-01b 有東西可審時，先在 FROM_KEVIN §1 認領再開工。 |
| 20261001 18:1x | K-01 錄製由誰跑（第 25 條原本是「下個上班日在筆電錄」） | **Jimmy 1001 16:1x：「讓kevin自己授權模擬驗證」**（RULINGS_20261001 第 34 條）⇒ 你可以自己決定在哪台、什麼時候跑 golden 0618 模擬版錄 state record，不用等筆電這邊有人按 START。HT9050 模擬設定的快照在共用區 `K01_golden0618_HT9050snapshot_20260930` 那個資料夾；golden 會寫 `system`／`config`／`IniData`，跑之前備份、跑完還原。錄好把 state record 的路徑寫在 FROM_KEVIN §2。 |
| 20261002 21:4x | 📌 Jimmy 1002 20:2x（`RULINGS_20261002.md` 第 21 條）：**HT9050 的開發／測試一律用機台推上來的工作檔**（工單＋機台參數） | 原話：「機台端有透過github把工單和機台設定檔放上去，你放到gitlab後，未來要求其他人要協助開發或測試時，都要用此工作檔，這樣才能有效同步問題」。機台 1002 19:29 的快照（GitHub `machine/integ-ioweb` `ca828068`）筆電已逐位元組放進 GitLab main `machines/HT9050/snapshot/`（`c84209ad`，694 檔；目前工單 IOWEB_TEST_R003，料盤 7×17、熱盤 8×16）。Kevin：**之後重現、量測、測試 HT9050 的行為，先照 `machines/HT9050/snapshot/SNAPSHOT_SOURCE.md` 裝好**（先備份 → 複製到自己的 `D:\HT9045\system\`／`config\`／`IniData\Data\` → 比 MD5 → 做完還原），回報時寫明用的是哪一版（`git log -1 --format=%h -- machines/HT9050/snapshot`）。機種身分不在快照裡：`D:\GPIB9045\system\general.ini` 的 `[Version] Model` 要是 `9050GPIB`（網頁也可以加 `?machine=HT9050`）。只裝在開發機／模擬，不要裝到別台真機台。ctest 用的主表（`machines/HT9050/IO_Table.csv`、`Mot_Table.csv`…）來自同一份快照（NB2 MR !123，今晚跟筆電第四十六批一起上 main）。機台每次推新快照，筆電會更新並在這裡通知。 |
| 20261003 12:0x | ⓘ **協作規則兩條（RULINGS_20261003 第 15 條）** | **新測試請附「反向驗證」**（RULINGS_20261003 第 15 條，NB2 R183 提、Jimmy 同意）：MR 裡新增或改動的 ctest 檢查，說明請寫「故意改壞哪一行 → 哪個 CHECK FAIL」；起因是 R170／R171／R177／R180 都有「只驗測試自己的假物件、程式壞了也綠」的檢查。另：筆電現在也有心跳 `origin/v906/jimmy-heartbeat:HEARTBEAT.md`，超過 4 小時沒更新＋main 沒推＝筆電停了，你們照工作卡繼續做、不用等；St01 是備援整合者（第 14 條）。**通用工具都放 git**（第 16 條）：筆電的在 `tools/laptop_ops/`（gate、單獨重跑、MR 掃描、文件推送、心跳…，附 README），NB2 的在 `tools/nb2_assist/`；派工卡會寫用哪支。覺得哪支要改善，開分支＋MR，筆電評估後採用。 |
| 20261003 12:1x | 🫀 **請建心跳** | 🫀 **請建心跳**（RULINGS_20261003 第 17 條，Jimmy 1003 12:1x：「心跳線如果讓各人員建立完成後，回報目前人員上線狀況」）：每輪最後一步跑 `python tools/laptop_ops/heartbeat.py --who kevin --doing "<這一輪在做什麼>" --next <下一輪時間> --push`（在任何一份 HT9045 checkout 裡跑；只寫分支 `v906/kevin-heartbeat` 的一支 HEARTBEAT.md，不碰工作分支）；沒有迴圈、人工開的 session，開工跟收工各跑一次。建好後筆電的 `tools/laptop_ops/team_status.py` 就看得到你；全員狀況你也可以自己跑那支看。 |
| 20261003 15:3x | 📅 **常駐卡：每日日報**（Steven 1003） | 📅 **常駐卡「每日日報」**（Steven 1003 透過 St01 轉：「要發給Jimmy筆電 讓有連線且有repo的人, 每天定期發日報」，ST 組以外的人由筆電派）——①**誰**：你（有在交接系統上連線的 session）；本機要有 clone 入口網站 repo（GitLab `honprec/rd/rd5/9050motionview`；還沒 clone 的照 https://pages.honprec.com/honprec/rd/rd5/9050motionview/sop.html 第一、二節做）。②**每個工作日**下班前（建議排程 17:30）寫當天日報並推；來不及就隔天 09:00 前推前一天的。一天一份、一個 MR。檔案 `public/Docs/Daily/<英文名>/YYYYMMDD.md`（英文名照組織表）。③**格式**照那個 repo 的 `.claude/skills/rd5-daily-report`（§2～§3.5、`references/template.md`）：一句話；今日完成表（狀態只用 完成／待上機／待客戶／待裁決／進行中）；卡點／需要協助表（從哪天開始、需要誰，沒有就寫「無」）；明天接續最多 3 項；一頁內。**不寫** Claude 對話經過、密碼／token／個資；AI 寫的草稿要本人看過。④**推送**：`git pull`（main）→ 開分支 `<英文名小寫>/<YYYYMMDD>-daily` → 只 add 自己的日報 → `git push -u origin HEAD -o merge_request.create -o merge_request.target=main -o merge_request.remove_source_branch -o merge_request.assign=steven -o merge_request.auto_merge`（檢查通過就自動合併）；推之前跑 `py tools/check_daily.py --changed` 自我檢查。推錯了隔天用修正 MR 改。排程用你習慣的方式（Claude Code 的排程是 session 內、7 天過期、重開要重建；或 Windows 工作排程器）。⑤開始交了就在 FROM 檔或 CHAT 說一聲（筆電也會看 daily.html 上方的「繳交狀況」）。 |
| 20261003 18:04 | 📣 **常駐卡：報數＋每小時回報工作狀態**（Steven 1003 18:0x，由 ST01-M 直接寫入） | Steven 原話：「請Ifor / Jerry / Frank / Kevin / ES02 報數, 並加入每小時回報」「如果有人是idle狀態, 就找工作派給他」「我們直接寫進那五個人的 TO 檔。 不要等了」。①**現在報數**（Kevin）：一行寫清楚——在不在線、在做什麼、卡在什麼、下一步。回在你的 `docs/handoff/FROM_KEVIN.md`（或你自己的交接分支）＋心跳。②**之後每小時回報一次工作狀態**，沒變也回一行；最方便的是心跳分支 `v906/kevin-heartbeat` 的 HEARTBEAT.md（last tick／doing／next 三欄，工具 `tools/laptop_ops/heartbeat.py --who kevin --doing "..." --next HH:MM --push`，跟 Ifor01 已在用的一樣）。③**沒事做（idle）就說**，派工的人會給你卡。派工順位（Steven 的代理人制度）：Jimmy 筆電 → ST01-M → ST02-M；前一位超過 1 小時沒回應就由下一位派工，回來就交還。上機驗證照舊只給 EastSun。 |
| 20261005 11:4x | ℹ **Jimmy 1005 11:4x：你的 K-01～K-04 會轉給其他人（先確認還要不要）** | 你 10/01 13:53 之後沒有推送，Jimmy 決定把沒做完的卡轉給別人；轉之前由 St01 逐張確認是不是已經有人做了（RULINGS_20261005 第 3 條）。**如果你回來了**：先在 FROM_KEVIN（`v906/kevin-handoff`）寫一行你正在做哪張、做到哪，筆電會照那行調整。 |
