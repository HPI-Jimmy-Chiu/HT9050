---
name: ops-st01-clerk-report
description: >
  St01（Steven01）工程線的「記錄員整點回報」：ST01-E（Steven01-Engineer）每小時或每段落派一位記錄員子代理，把 D:\HT9045
  分支 v906/steven-cbridge-review6 的新 commit、工程師交件、Steven 的裁決、ST01-M 給的交接項目，累加寫進三份檔——
  ChangeLog D:\docs\ChangeLog\CHANGES_20260926_Steven.md（§11 摘要表加一列＋§11.NN 新節、§12 目前狀態整段覆蓋）、
  日報 D:\docs\ops\daily\20260926.md（檔尾「09-27 HH:MM 更新」段）、裁決進度表
  D:\HT9045\.claude\skills\ht9050-construction\references\rulings-index.md（最後更新、主表狀態、狀態計數、最近變動 20 行）。
  記錄員不做 git 寫入、不 build；ST01-E 核對後把 §12 四塊轉給 Steven、只 commit rulings-index.md；ST01-M 只給文字、
  不直接寫這三份檔，並用 5 小時檢查點提醒 ST01-E。含範圍指令、⛔ 更正寫法、§12 範本、派工與交件範本、常見錯誤
  （Q34 誤寫成 levelset.dat、已裁決寫成待決、Q4 重選沒跟上、§12.4 漏列 R 題、R 題寫成「定案」、機台資料夾誤接到移植樹底下）
  與核對清單（顆數、題號、條目數、路徑存在、狀態計數、diff 只刪在允許的地方）。
  Use when：派記錄員、整點回報、每段落回報、寫 ChangeLog §11／§12、日報更新段、更新 rulings-index／裁決進度表、
  核對記錄員交件、5 小時檢查點、「無新進度」、要把 §12 轉給 Steven。
  關鍵字：記錄員, 文書記錄員, clerk, 整點回報, 每小時, 每段落, ChangeLog, CHANGES_20260926_Steven.md, 日報, daily,
  rulings-index, 裁決進度表, 狀態計數, 最近變動, §11, §12, 12.1 已推送, 12.2 進行中, 12.3 排隊, 12.4 待 Steven 決定,
  12.5 交給 Jimmy, ⛔ 更正, 只增不改, 絕對路徑, ST01-E, ST01-M, S53, 5 小時檢查點, 無新進度, clerk_state。
  長期規則 → references/standing-rules.md；三份檔格式與例子 → references/format-rules.md；§12 範本 → references/section12-template.md；
  派工與交件範本 → references/dispatch-and-handin.md；常見錯誤與核對清單 → references/checklist.md
---

# St01 記錄員整點回報

> **由來**：Steven 20260926（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md` S53）：「派一個 sonnet 文書記錄員，持續記錄 change log，並且整理 skill」「每個小時或是每個段落就要回報一次」。
> **跟 `D:\HT9045\.claude\skills\ops-ht9045-handoff\` 的分工**：那份是 ST01-M 的交接協定（TO_STEVEN／FROM_STEVEN／CHAT、30 分鐘巡檢、todo／done／decisions 代登記）；本 skill 只管 ChangeLog／日報／rulings-index 三份檔的累加記錄。交會點只有兩個：ST01-M 的交接動作由 ST01-M 給文字、經 ST01-E 交給記錄員（§1）；ST01-M 的 5 小時檢查點觸發本流程（§2）。

## 0. 檔案位置

| 用途 | 絕對路徑 | 誰改 |
|---|---|---|
| ChangeLog（讀者 Jimmy 與研五軟體組；不在 git） | `D:\docs\ChangeLog\CHANGES_20260926_Steven.md` | 記錄員 |
| 日報（不在 git） | `D:\docs\ops\daily\20260926.md` | 記錄員 |
| 裁決進度表（在 git，三方都看得到） | `D:\HT9045\.claude\skills\ht9050-construction\references\rulings-index.md` | 記錄員寫、ST01-E commit；ST01-M 不改內容 |
| 舊裁決進度表（20260927 13:0x 凍結） | `D:\docs\ops\registers\HT9045_裁決進度表.md` | 不再改 |
| 待決／已決（只讀） | `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`／`decisions-decided.md` | Q／R＝ST01-E，W＝ST01-M |
| 裁決原文（只讀） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md`（S 編號）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md`（第 N 條，筆電寫的） | ST01-E／筆電 |

檔名 20260926 是這一段工作開頭那天；跨到 20260927 仍沿用同一份檔（見 references/standing-rules.md §5）。

## 1. 誰做什麼

| 角色 | 做什麼 | 不做什麼 |
|---|---|---|
| **ST01-E** | 定範圍（`<上一輪終點>..<本輪終點>`，終點寫死 hash）、列在跑的工程師與交件、附上 Steven 本輪的回覆與 ST01-M 的文字、派記錄員；核對交件；把 §12 四塊轉給 Steven；commit＋push rulings-index.md；新發現的錯誤型態補進本 skill | 不讓記錄員自己猜範圍或猜誰在跑 |
| **記錄員**（ST01-E 派的子代理，S53 原話是 sonnet） | 只改三份檔；照本 skill 寫；交件 | 不做 git 寫入（add／commit／push／fetch／pull／checkout）、不 build、不跑 ctest／wb_serve、不開子代理、不改 decisions／todo／done／RULINGS／skill、不寄信 |
| **ST01-M** | 5 小時檢查點提醒 ST01-E；自己的交接動作（FROM_STEVEN 轉達、替 St02 代編、代登記）整理成文字給 ST01-E | 不直接寫這三份檔 |
| **Steven** | 看 ST01-E 轉的 §12 四塊，回 §12.4 的題 | — |

「§12 四塊」＝12.1 已推送、12.2 進行中、12.3 排隊、12.4 待 Steven 決定；12.5（交給 Jimmy／其他 Steven session）是累加清單，本輪有新條目才一起轉。

## 2. 節奏

- 每小時（S53 當時排在每小時 :17）或每段落（一批 commit 推完、工程師交件、Steven 回一批題目）派一次。
- **5 小時檢查點**：ST01-M 的排程 `23 */5 * * *`（session 內建立，換 session 要重建）發訊息給 ST01-E → ST01-E 派記錄員補三份檔，並 push／pull。
- **沒有新東西**：`<上一輪終點>..HEAD` 0 顆、沒有工程師交件、ST01-M 沒給文字 ⇒ ST01-E 不派；已經派了，記錄員只回一行「無新進度」，三份檔都不動。
- 用量 ≥95% 減少多工，但記錄員照跑；98% 收尾時最後一輪記錄跑完才停。Steven 沒回題目不停工，待決題累積在 §12.4 等 Steven 回來一次問。

## 3. 一輪的步驟（記錄員）

> 20260928 起 ChangeLog 與日報**一天一份**（`D:\docs\ChangeLog\CHANGES_YYYYMMDD_Steven.md`、`D:\docs\ops\daily\YYYYMMDD.md`），§12 只放最新一天，規則見 references/standing-rules.md §11。上面說明裡寫死的 0926 檔名，照當天日期換。
> ⛔ 20260929 11:1x 起 **Steven 日報 `D:\docs\ops\daily\YYYYMMDD.md` 由 ST01-M 編輯，記錄員不寫**（Steven「日報一律通報 ST01-M做內容編輯」）；記錄員只寫 ChangeLog、repo 日報 `D:\HT9045\docs\ops\daily\YYYY-MM-DD.md`、rulings-index。見 references/standing-rules.md §12。（20260929 15:2x 起 Steven 日報在 `D:\RD5-Portal\public\Docs\Daily\Steven\YYYYMMDD.md`，也不是記錄員寫。）

> 20260927 19:3x 起：每一輪也要併 St02 的 `docs/handoff/ST02_DAILY_<YYYYMMDD>.md`／`ST02_CHANGELOG_<YYYYMMDD>.md`（交接分支），規則見 references/standing-rules.md §10。

1. 讀 references/standing-rules.md、references/checklist.md 的 C 段（常見錯誤）。
2. 重讀三份檔**現在的實際內容**（ST01-E 可能剛改過）：ChangeLog 最後一個 §11.NN 與 §12、日報最後一段、rulings-index 表頭與最近變動。
3. 範圍：`git -C /d/HT9045 log --first-parent --format="%h %ad %an %s" --date=format:%H:%M <上一輪終點>..<本輪終點>`；每顆主要檔案 `git -C /d/HT9045 show --name-only --format= <hash>`（前面加 `D:\HT9045\`、`/` 換 `\`）。其餘只讀指令見 references/format-rules.md §1。
4. 每個 Q／R／W／S 題號都去 decisions-pending.md／decisions-decided.md／RULINGS 對題目、狀態、最新選項。
5. ChangeLog：§11 摘要表加一列 → `# §12.` 前面加 `## 11.NN` → §12.1～12.4 整段重寫 → §12.5 頂端加新條目、結案的原地標 ⛔。
6. 日報：檔尾加一段「**09-27 HH:MM 更新**」。
7. rulings-index：最後更新 → 主表改到的列 → 重算狀態計數 → 最近變動頂端加 1 行、砍最舊 1 行（維持 20 行）。
8. 照 references/checklist.md A 段自查 → 照 references/dispatch-and-handin.md 交件。

## 4. 格式速記（細節與例子見 references/format-rules.md）

- ChangeLog §1～§11、日報：**只增不改**；錯了在原處加「⛔ 更正（誰 時間 核對）：」，不刪原字。§12.1～12.4 每輪整段重寫；§12.5 累加，結案的標 ⛔ 已完成／⛔ 已結案，舊字加刪除線不刪。
- §11.NN 標題＝`commit 首～尾（09-27 hh:mm～hh:mm，N 顆）：重點`；下一行「> 來源：」＝範圍指令＋顆數＋「有沒有工程師交件、有沒有新裁決、Steven 有沒有新回覆」；小節 a、b、c 照時間排，ST01-M 的項目放最後一節。
- 題目一律寫「題號＝選項（S 編號）＋一句意思」，例：**Q31＝A′**（S152，906 單邊修 AOI.Data 兩個 golden 缺陷）。R 題是「已照建議先做、可推翻」，Steven 點頭前不寫「定案」。
- 路徑一律絕對路徑並標樹；commit hash 後面接主要檔案的絕對路徑；一條路徑不拆成兩行。機台資料夾是 `D:\HT9045\system\`、`D:\HT9045\config\`、`D:\HT9045\IniData\`，**不在移植樹底下**。
- 用人名（Steven、Jimmy），不用你我他；角色寫 ST01-E／ST01-M／ST02-E／ST02-M，不寫 github-xx。
- 時間以 commit 時間為準；跨日標「09-27 hh:mm」。
- rulings-index「最近變動」最多 20 行、最新在最上面；寫「狀態計數不變」之前一定先重算。

## 5. ST01-E：派工、核對、轉達、commit

1. 派之前：`git -C /d/HT9045 fetch`、定本輪終點 hash、把三份檔各複製一份到 session 暫存資料夾（交件後 diff 用）。派工訊息範本 → references/dispatch-and-handin.md §1。
2. 核對：references/checklist.md B 段（顆數、題號對題目、§12.4 條目數＝decisions-pending 標題數、工程師名單、路徑存在、狀態計數、diff 只刪在允許的地方）。
3. 錯的地方：自己在原處加「⛔ 更正（ST01-E hh:mx 核對）：」改掉，或退回記錄員重寫；錯誤型態是新的，補進 references/checklist.md C 段。
4. 轉 Steven：§12.1～12.4 原文；12.4 附 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`。
5. commit（只有 rulings-index.md 在 git 裡，`D:\docs` 不是 repo）：
   `git -C /d/HT9045 commit -m "rulings-index: clerk round <上一輪終點>..<本輪終點>" -- .claude/skills/ht9050-construction/references/rulings-index.md`（訊息結尾照 session 規定加 Co-Authored-By），再 push。例：`23efc733`（20260927 14:25，範圍 `46e2d7f9..9f6c178c` 含 ST01-E 更正）。只 commit 這一個檔；ST01-M 要在 `D:\HT9045` commit 前會先問 ST01-E（`D:\HT9045\.claude\skills\ops-ht9045-handoff\references\protocol.md`「共用工作樹」）。

## 6. 交件格式（記錄員 → ST01-E）

五項，範本在 references/dispatch-and-handin.md §3：①改了哪些檔（節號、列號、砍掉的最近變動那行）②狀態計數（和上一輪比）③§12 四塊原文（12.5 只貼新增／改標的）④待確認（來源互相矛盾、寫不準的地方，附兩邊出處）⑤新教訓（建議補進本 skill 的）。

## 7. 記錄員狀態檔（暫存）

20260926～27 每輪交接靠一份記錄員狀態檔，放在 ST01-E session 的暫存資料夾，session 結束就不見。裡面長期有效的規則與核對更正已搬進 references/standing-rules.md 與 references/checklist.md，**以本 skill 為準**。「上一輪停在哪」直接從三份檔讀：§12.1 標題的 hash、rulings-index「最後更新」的 commit、ChangeLog 最後一個 §11.NN 的節號。暫存檔若還在，只當本輪臨時備註；裡面出現新的長期規則，由 ST01-E 搬進本 skill。

## references

- references/standing-rules.md — 每輪都要套用的長期規則（權限、只增不改、路徑與樹、人名與角色、時間、題目與裁決、寫檔技術）
- references/format-rules.md — 三份檔的格式與真實例子（§11 摘要表列、§11.NN、⛔ 的六種寫法、日報更新段、rulings-index 各區）
- references/section12-template.md — §12.1～12.5 範本與 12.4 的組法
- references/dispatch-and-handin.md — ST01-E 派工訊息、ST01-M 給的文字、記錄員交件、「無新進度」
- references/checklist.md — 記錄員自查、ST01-E 抽查、常見錯誤（附出處）、檢查指令
