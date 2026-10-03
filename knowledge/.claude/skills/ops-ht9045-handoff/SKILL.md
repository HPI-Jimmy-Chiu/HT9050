---
name: ops-ht9045-handoff
description: >
  HT9045／HT9050 三方交接（Jimmy 的筆電、Steven01、Steven02）的協調與登記作業：交接檔在哪個分支（TO_STEVEN／CHAT_JIMMY 在 main；
  FROM_STEVEN／CHAT_ST01／CHAT_ST02 在 v906/steven-handoff）、每個檔各節寫什麼、20 分鐘巡檢步驟（先 pull、MR 對 main 試合）、轉達規則
  （給 St01 的轉 ST01-E、給 Steven 的報使用者、Jimmy 的派工卡只報告不認領）、改別人登記的檔看區段不看檔名、
  共用工作樹 D:\HT9045 先知會 ST01-E、寫給 Steven 看的文件一律絕對路徑，以及替 St02／Jimmy 登記
  ht9050-construction 的 todo／done／decisions-pending／decisions-decided。附三支 helper：handoff_commit.sh（不 checkout 直接改
  handoff 分支並推）、refresh_handoff.sh（更新 D:\HT9045_handoff 唯讀快照）、watch_skill.sh（盯 skill 變動）。
  Use when：交接巡檢、FROM_STEVEN、TO_STEVEN、CHAT_ST01、CHAT_ST02、CHAT_JIMMY、steven-handoff 分支、St01／St02 協調、
  ST01-M／ST01-E、代登記 todo／done、decisions 檔、Steven 回 Q／R／W 題、handoff_commit、refresh_handoff。
  關鍵字：交接, handoff, 巡檢, FROM_STEVEN, TO_STEVEN, CHAT_ST01, CHAT_ST02, CHAT_JIMMY, CHAT_合併, v906/steven-handoff,
  St01, St02, Steven01, Steven02, ST01-M, ST01-E, 代登記, todo.md, done.md, decisions-pending, decisions-decided,
  RULINGS, 區段規則, 共用工作樹, 絕對路徑, handoff_commit.sh, refresh_handoff.sh, watch_skill.sh。
  交接檔格式與轉達規則 → references/protocol.md；todo／done／decisions 登記規則 → references/registrar.md
---

# HT9045 三方交接：協調與登記

## 1. 誰是誰

| 名稱 | 機台／session | 做什麼 | 分支 |
|---|---|---|---|
| **Jimmy 的筆電** | Jimmy 的電腦 | 整合、合 main、夜間迴圈、筆電 gate | `main` |
| **St01**（Steven01，主機名 Steven-NB） | **ST01-M**（Steven01-Manager）＝協調與登記（本 skill）；**ST01-E**（Steven01-Engineer）＝工程線 | 資料讀寫轉檔、畫面讀寫 | `v906/steven-cbridge-review6` |
| **St02**（Steven02，STEVEN-NB3） | **ST02-M**（Steven02-Manager）／**ST02-E**（Steven02-Engineer） | 測試通訊（GPIB／RS232）、cMyDB、Event Log 分析器 | `v906/steven-gpib-widget`；借 St01 分支做的在 `v906/steven-st02-on-cbridge` |

St02 只能透過 git 交接檔聯絡；St01 的工程線在同一台，用 SendMessage。

> **組織圖＋各角色目前的 session**：正本在 RD5 入口網站人員組織頁「AI session 派遣現況」（Steven 20261001），指標見 [references/org-chart.md](references/org-chart.md)；角色或 session 一變，ST01-M 當天更新正本。

> ⚠ **角色名稱**（使用者 20260927 定）：Steven01-Manager＝**ST01-M**、Steven01-Engineer＝**ST01-E**，St02 比照 **ST02-M／ST02-E**。
> 文件與訊息一律寫角色名稱。github-xx 是 Claude Code session 的名字，重開就換（20260927 13:0x 重開後，原本叫 github-02 的 ST01-M、叫 github-70 的 ST01-E 都換了名字）；傳訊息前先 ListAgents，認不出哪個 session 是哪個角色就問使用者，不要猜。

## 2. 交接檔

| 檔 | 分支 | 誰寫 | 內容 |
|---|---|---|---|
| `docs/handoff/TO_STEVEN.md` | `main` | Jimmy | §1 筆電在改的檔、§3 派工卡（S-01～S-06，**先聽著、不認領**）、§4 回答 |
| `docs/handoff/CHAT_JIMMY.md` | `main` | Jimmy | 聊天 |
| `docs/handoff/FROM_STEVEN.md` | `v906/steven-handoff` | St01、St02 | §1 認領、§2 完成、§3 問 Jimmy、§4 St01 ↔ St02；**每列標 St01／St02** |
| `docs/handoff/CHAT_ST01.md`／`CHAT_ST02.md` | `v906/steven-handoff` | 各自 | 聊天 |

唯讀快照：`D:\HT9045_handoff\`（`scripts/refresh_handoff.sh` 產生，含三方合併的 `CHAT_合併.md`）。

## 3. 巡檢（每 20 分鐘，先 pull）

0. **pull**（Steven 20261002 17:5x「使用20分鐘的, 然後要做pull」；ST01-M 的排程在每小時 :03／:23／:43）：
   - `D:\HT9045`：先 `git fetch origin`，再看 `git rev-list --left-right --count origin/v906/steven-cbridge-review6...HEAD`。左邊（遠端多的）不是 0、右邊是 0 才 `git pull --ff-only`；右邊不是 0＝ST01-E 有本機還沒推的 commit，不 pull、不 rebase，等它推（20261002 20:3x `fb31e431` 多 58 顆就是這樣）。
   - `D:\RD5-Portal`：`git pull --ff-only`。ST01-M 在那裡只改檔、不 commit，由 ST01-E3 推；ST01-E3 的功能分支合進 main 後遠端分支會刪掉，pull 會說「no such ref was fetched」，那是正常的，換分支交給 ST01-E3，未提交的日報／組織圖改動會跟著走。
1. `sh scripts/refresh_handoff.sh`（fetch＋更新快照）。
2. 讀 TO_STEVEN 新列：給 St01 的轉 ST01-E；給 Steven 的報使用者；§3 新卡只報告。
3. 讀 FROM_STEVEN §4 裡 St02 → St01 的新列、CHAT_ST02 新行：牽涉 St01 的檔先對 St01 分支與 main 的差異再回（看區段）。
4. St02／Jimmy 送來的 done／todo 照 `references/registrar.md` 登記。
5. `origin/v906/steven-cbridge-review6` 有新的程式 commit → 更新 FROM_STEVEN §1／§2 的 St01 列（ST01-E 回報內容）。
5a. **St01／St02 已推的分支、MR 對 main 試合**（Steven 20261002 18:2x「確認我們發上去的MR有沒有衝突, 有衝突就想辦法修正」）：`git merge-tree --write-tree origin/main <tip> | grep ^CONFLICT`（git 2.38 起；要只看檔名加 `--name-only`）。
   - 只有 `HT9011UC_Cpp_V3.33.906.0/tests/CMakeLists.txt` 檔尾：合 main 時用 ops-ht9045-proxy-build 的 `scripts/cmake_rebase_union.py`。
   - 分支主人正在合 main 的（例 review6 合併在本機還沒推）：拿它本機的頭再試一次，把新冒出來的衝突告訴它。
   - 同事的 MR 有衝突（20261002 MR !115，Ifor：測試清單檔尾＋`wb_serve.cpp` 三行開機長行，兩邊在同一行各加東西）：St01 在獨立 worktree 合 main、兩邊的插入都留，跑全量 gate，不先動對方分支。Steven 20:4x「我們直接整合好之後, 幫她合併就好」——gate 綠了才推到 MR 的來源分支、幫忙合併。
6. Monitor 跑 `bash scripts/watch_skill.sh`（timeout 1800000，到期重開）；事件提醒使用者，自己改的忽略。
   - 5 小時檢查點（Steven 20260927：不寄信、每 5 小時一次）：SendMessage 叫 ST01-E「記錄員補到現在＋push/pull」並附 ST01-M 這段做的事；ChangeLog／日報／rulings-index.md 由 ST01-E 的記錄員寫（skill `ops-st01-clerk-report`），ST01-M 只給文字、不直接改這三份檔。例外：Steven 的入口網站日報 `D:\RD5-Portal\public\Docs\Daily\Steven\YYYYMMDD.md` 由 ST01-M 編、ST01-E3 晚上推（memory「做完就更新 skill」）。
7. 沒有新東西只回一行「無變化」。
   - skill 快查表：`python scripts/skill_index.py <輸出檔>` 產生全表，貼回 memory 的 skill-quick-index.md（AUTO TABLE 段）。

## 4. 寫交接檔：一律用 helper

```
bash scripts/handoff_commit.sh <edit.py 絕對路徑> <commit 訊息檔>
```

- edit.py 在暫存資料夾執行（cwd 裡有 FROM_STEVEN.md、CHAT_ST01.md），用 LF 讀寫，只改自己的列。
- commit 訊息開頭「FROM_STEVEN（St01）：」，結尾 `Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>`。
- helper 不 checkout 工作樹；被拒會在最新版上重跑 edit.py（最多 5 次）；有控制字元會拒推。
- ⛔ **第二個參數是「訊息檔」的路徑，不是訊息字串**：先把訊息寫進 scratchpad 的 .txt 再傳檔名。20260930 00:3x ST01-M 傳了字串，commit-tree 失敗、`$C` 變空，舊版照樣 `git push origin :refs/heads/v906/steven-handoff`，**等於刪掉遠端交接分支**；當下用前一個 BASE `bfe5d127` 推回（`git push origin bfe5d127:refs/heads/v906/steven-handoff`，看到 `[new branch]` 表示空窗期沒人推），沒有遺失。現在 helper 會先檢查檔案存在（不存在 exit 2）、commit-tree 失敗或 `$C` 空就不推。**萬一分支又不見**：`git reflog show origin/v906/steven-handoff` 或 helper 印的 `pushed <BASE>..` 找最後的 hash 推回去，再通知 St02 與筆電。
- **edit.py 用 Write 工具寫，不要用 Bash heredoc**：heredoc 會把 `\\` 收成 `\`，`\n`、`\a` 變成跳脫字元（20260927 弄斷一列表格、讓一筆推送失敗）；腳本裡用 `chr(92)` 組反斜線。

## 5. 硬規則（細節見 references/protocol.md）

- **派工都要提醒更新 skill＋日報**（Steven 20261002 08:1x：「記得通知大家要更新skill跟日報」「你每次安排工作的時候,都要做這個提醒, 記錄到 instruction裡面好了」）：ST01-M 每一則有新工作的訊息——給 ST01-E／ST01-E2／ST01-E3 的 SendMessage、CHAT_ST02 與 FROM_STEVEN §4 的 St02 工作卡、給子代理的 prompt——最後都加一行 `Reminder (Steven): when done, update the related skill and the daily (ChangeLog / repo daily / Steven's portal daily via ST01-M).`；給 ST01-E、St02-M 的派工另加「也轉告你的工程師／helper」。純轉告、回覆、FYI 不強制。
- **只做 906（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20261002.md` 第 20 條，Steven 20261002 18:0x）**：原話「我還有看到912版，這是錯的，現在分工處理只能做906 C++專案，能理解?」。golden＝906 `D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260625_Steven`，912 只拿來查 906 漏了什麼，沒有「Steven 點名」的例外；派工單要寫明。**例外 20a（Jimmy 19:0x 依 Steven）：溫控必須參考 V912**（溫控器、HandlerSys Heater 分頁 E-029、HT9050 通道表、bthermo；E-027／E-029 的 V912 引用保留）。St01 分支用到 912 的稽核在 FROM_STEVEN §3 `f1208ecf`（全文在 handoff 分支 `docs/handoff/ST01_912_AUDIT_20261002.md`；Q-A／Q-B／Q-C 等 Steven）。
  - **912 比較好就照 912（Steven 20261003 05:3x～05:4x 常設規則）**：原話「以後我這邊遇到這個問題，如果是912比較好，就是註記906的行號跟做法　然後增加註記912已修正或更新的行號」「不需要一直糾結在這邊　Jimmy弄了天條擋住不是906 cpp版的項目　我們這邊就是讓他接受+繞過這一個限制」。906 與 912 不同、912 是修 bug 或明顯比較好 ⇒ 程式照 912，註解寫 906 的行號＋做法、912 修正／更新的行號、「#20 例外（Steven 1003 常設規則）」；不再寫進 decisions-pending 問 Steven，在 FROM_STEVEN §3 公開告訴 Jimmy、human-review C 區登一筆；判斷不出哪個好、或客戶專用／行為改變很大才問 Steven。第一批：Q78（A02 存檔保護）、Q79（GPIB 力量字串）、W70（ELA 拆欄）、Data.Observer 事件記錄檢視。**Jimmy 已接受**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20261003.md` 第 1 條＝§0 #78 B：做的人自己判斷、不用等 Jimmy；保留 912 的地方兩邊行號都寫，帳本記一列理由——St01 記在 ht9050-construction registry）。**golden 基準是 906 的 0618**（同檔第 2 條：用共用區 7z 密碼解開，0625 只做對照；St01 的重核是 todo E-032）；產生器的 golden 來源由 St01 決定（第 4 條，todo E-031）。
- **先聽著、不接單**：Jimmy 的派工卡 S-01～S-06 不認領，除非 Steven 說要接。
- **看區段、不看檔名**：別人登記的檔，不同區段就直接做並在 FROM_STEVEN 寫明；同一段等對方回覆。`aHotPlateSubstrate.*` 要先問 Steven。
- **共用工作樹**：ST01-E 也用 `D:\HT9045`。commit 登記前先 fetch、確認 `origin/v906/steven-cbridge-review6..HEAD` 為空、先 SendMessage 問它，只 commit 自己的檔。
- **給 Steven 看的文件寫絕對路徑**（`D:\HT9045\...`，標明哪棵樹）；commit hash 後面附主要檔案全路徑。
- Steven 的答案要**照原話**轉，不改意思；理解不確定就標出來再問。
- **語言**（RULINGS_20260927 第 8 條＋Steven 18:1x）：內部作業與 SendMessage 用英文；FROM_STEVEN、CHAT_ST01、給 Jimmy 的列可以寫英文；skill（含 ht9050-construction 的 todo／done／decisions）與 ChangeLog／日報維持繁體中文（UTF-8）；回 Steven 本人一律繁體中文。
- **St01 分支合 main**：由 Steven 決定；用 ops-ht9045-proxy-build 的全量 gate 當證據，在 FROM_STEVEN §2 寫「到哪一顆可以合」給筆電（例 20260927 17:40 到 `6bd0f5a4`）；同事分支合 main 一律要有 MR（RULINGS_20260927 第 8 條）。**每一批都要對得上 Steven 的一個裁決**：Q56（20260929）只涵蓋 `10cac033` 那一輪，加上當晚在 §2 講好的 `b5710386`／`1e5316eb` 修正版；新功能的批次要看 Q59（「全套 gate 綠了就直接請 Jimmy 合」，20260930 登記，等 Steven 回）。還沒有裁決時，gate 綠了只在 §3 貼證據，並寫「合併列等 Steven」。筆電催的時候也一樣。**§2 寫過「合最新版也可以」之後，分支又進了沒 gate 的 commit，要馬上改那一列**（20260930 01:3x 的例子：`699dc06d` 進來後改成「最多到 `4d4495f3`」）。
- **20261001 起的合併規則（取代上一條「等 Q59」那段）**：Q59（20260930「ok」）＝兩組態 gate 綠了 ST01-M 直接寫 §2；**上機才驗得出來的（加熱器／馬達／IO、改接觸氣壓這類）先由 EastSun 上機驗證再寫 §2**（Steven 1001 09:4x「需要上機驗證的, 都是請Eastsun處理」，Q62／Q63／Q64 都是 B）。照 golden 補齊、接上的功能不用再問 Steven（Jimmy RULINGS_20261001 第 0 條，跟 Steven 的方向一致）；只有**新設計／跟 golden 不同**或**動別人認領的檔**才問。「只是改了存檔內容」不算例外（20261001 04:4x ST01-M 更正）。
- **St01 兩條分支**：`v906/st01-q59`＝不用上機、gate 綠就寫 §2 cap 的工作；`v906/steven-cbridge-review6`＝要 EastSun 上機驗的（D-021～D-025 等），驗過才寫 §2。新的大件（例 D-026）先開側分支，完成後再合進對應那條。共用目錄 `D:\HT9045` 只在 review6 上，其他分支用各自的 worktree（`D:\AI_TempFile\st01e-q59` 等）。§3 告訴筆電「review6 過了某顆不要合」時，要寫清楚停在哪顆。
- **上機驗證清單**：`ht9050-construction/references/human-review.md` 的 A 區＝EastSun 的清單；新增 A 項同一輪在 §3 請 Jimmy 轉 EastSun，附分支／commit 和每一項要看什麼（先 gate 再給）。
- **代跑 St02 的 MR 之前先看 main**：筆電已經合進 main（它自己跑過兩組態 gate）的 MR，St01 的代跑改成可省；正在跑的確認性 gate 可以只留 SIM＋真實檔檢查就停，讓位給還沒合的工作（20261001 MR !20、!21 的例子）。
- **時間標記一律先跑 `date`**：20261001 ST01-M 兩次寫早（「05:15」實際 04:57、「09:5x」實際 09:46），ST01-E 也寫早 30 分鐘；標錯當輪就改。
- 用量 ≥95% 減少多工、以收尾為主；**子代理回報「到週上限」時先請 Steven 看 /usage 再下結論**（20261001 08:52 一個子代理被擋，ST01-M 就通知大家停工，實際本週只用 4%）；真的到上限時：先把各分支頭與 WIP 狀態寫進記憶、停派子代理，gate 腳本照跑（不耗用量）。**到上限前排喚醒**（Steven 20261002 17:4x「在reset之後, 把大家喚醒」、「提早一點到4:00啟動」）：/usage 顯示 reset 時間時，用 CronCreate 排一次性工作（`recurring: false`，約在 reset 那一刻，例 1002 18:30、10/03 04:00）。工作內容：ListAgents，SendMessage 叫 ST01-E／ST01-E3 接著做、ST01-E2 只轉訊息，CronList 確認 20 分鐘巡檢等排程還在，再報給 Steven：喚醒了誰、各自接什麼、還在等哪些題目。
