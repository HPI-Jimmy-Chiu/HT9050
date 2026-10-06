---
description: "上傳日報：把當天（12:00 前＝前一個工作天）的日報照入口網站統一格式寫好、檢查過就直接推上 RD5 入口網站 daily.html，不問（使用者 20261006）。結案時已自動補的列會帶進來。關鍵字：上傳日報, 推日報, 寫日報, 補日報, 日報上網站"
---

# 上傳日報（使用者 20261005）

使用者 20261005 選「**結案補一列＋上傳日報關鍵字**」：說「上傳日報」就開始做，不用先問要不要做。
起因：入口網站 daily.html 上 12 人 173 篇日報，JimmyChiu 0 篇——我們這邊只有週報與 Release Note 的發佈工具，日報從來沒有機制；
網站日報頁上 Jimmy 那一列只看得到 Release Note，是因為 build_portal.py 把 release note 掛在每人的日報欄（`add_release_to_daily`）。

**直接推、不問**（使用者 20261006：「以後上傳日報能夠自動推嗎？不要詢問」，取代 20261005 的兩段式）：草稿寫好、`check` 過就 `push`，
推完把**草稿全文與頁面網址**貼給使用者。唯一會停下來問的是**內容檢查命中**（§4 表格，不該給全公司看的字）。
入口網站 SOP 寫 AI 代寫的日報要本人看過（`sop.html`、rd5-daily-report skill §3.5）——Jimmy 選擇推完再看。
推了之後再改要多開一張 MR（網站規定一天推一次），所以第一次就照證據寫對，不要寫證據裡沒有的事。

工具在 Weekly_AI：`tools/daily_upload.py`（借用 `publish_portal.py` 的分支／MR、`_publish_guard.py` 的內容檢查）。
格式規則的權威是入口網站的 `.claude/skills/rd5-daily-report/SKILL.md` 與 `tools/check_daily.py`（工具直接載入 origin/main 那份來檢查）。

```
W=/d/Work-jimmychiu/document/WeeklyReport/Weekly_AI
cd $W && PYTHONIOENCODING=utf-8 python -X utf8 tools/daily_upload.py <子指令> [--date YYYYMMDD]
```

## 1. 蒐集（collect）

`daily_upload.py collect` → 印出證據並存 `daily/<YYYYMMDD>_evidence.md`；第一次跑會產生草稿骨架 `daily/<YYYYMMDD>.md`
（已存在就沿用；`--fresh` 重產會蓋掉草稿）。

- 日期：使用者沒講就用預設（12:00 以後＝今天；12:00 以前＝前一個工作天）。「補 10/02 的」就 `--date 20261002`。
- 證據六段：**A** 結案／手動記的列（`close_case.py` 自動補的）、**B** 週報 action 日期＝這天、
  **C** HT9045 的 commit（所有分支；含夜間迴圈、NB2、心跳等同一個 `jimmychiu` 身分的 AI session）、
  **D** 這天的使用者裁決（`RULINGS_<日期>.md` 標題＝Jimmy 親自做的決定）、**E** 這天推上入口網站的東西、
  **F** 前一篇日報（網站上的或本機已寫好的草稿）的卡點與明天接續。

## 2. 寫草稿（Claude 用 Edit 改 `daily/<YYYYMMDD>.md`）

照 rd5-daily-report 統一格式，**只能有四個 `##`**：一句話 → 今日完成 → 卡點／需要協助 → 明天接續。一頁內（約 3 KB，上限 8 KB）。

- **標題下一行**：`客戶／機種：…　版本／分支：…`（這天主要碰到的）。
- **一句話**（≤180 字、2 行內）：今天的主線與結果。**補寫**（早於前一個工作天）要以「（補寫）」開頭——
  不要寫「補寫：」，`build_portal.py` 把短的「標籤：」段落當中繼資料，首頁摘要會抓錯段。
- **今日完成**（表格：項目｜客戶｜結果｜狀態｜連結）：
  - A 段的結案列保留（結果可改成白話）；B 段骨架已填好，一案一列。
  - D 段的裁決挑重要的寫成列（狀態多半是「完成」或「待裁決」）。
  - C 段的 commit **歸納成 2～5 列主線**（例：V912 修正哪一件、V906 合併哪幾個 MR、跟 Steven 交接什麼），不逐顆列；
    寫「做成了什麼」，不寫視窗或對話經過。
  - 狀態只用：完成／待上機／待客戶／待裁決／進行中（另可用取消）。連結寫 MR 編號、commit hash、報告檔名。
  - 只寫證據裡有的事；沒有證據的不要補。
- **卡點／需要協助**（表格：項目｜卡在哪｜從哪天開始｜需要誰），沒有就寫「無」。F 段前一篇的卡點還沒解的要帶過來並標「（第 N 天）」。
- **明天接續**：最多 3 項。
- 跨人寫名字（Jimmy／Steven），不寫你我他。不寫客戶聯絡人信箱／電話、內網 IP、帳密、token、**交付包 7z 密碼**
  （網站全公司內網免登入；交接檔與 RULINGS 裡有這個密碼的字面值，引用那些檔的句子時特別注意）。

寫完跑 `daily_upload.py check`（不推）——格式沒過就照列出的項目改，不用問使用者。

## 3. 不用等使用者（20261006 起）

`check` 過了就直接進第 4 步，**不要**貼草稿問「要推嗎」。使用者要改，看完網站上的再說，照改草稿再推一次。

## 4. 推（push）

`daily_upload.py push`（Bash timeout 給 600000；大部分時間在等 MR 自動合併）：
再檢查一次（「【待填】」、`check_daily.py`、內容檢查）→ `jimmychiu/<今天>-daily` 分支 → 本機 `build_portal.py --ci` →
只 commit `public/Docs/Daily/JimmyChiu/<YYYYMMDD>.md` → MR（指派 steven、檢查通過自動合併）→ 等合併。

| 輸出 | 怎麼處理 |
|---|---|
| `[網站] 已合併上線` | 回報頁面網址與 MR，並貼上推上去的日報全文 |
| `日報格式沒過` | 照列出的項目改草稿再推（不用問使用者） |
| `內容檢查擋下 N 處` | **停下來問使用者**（回覆文字，不用彈窗）：列出命中的字與為什麼不該給全公司看，選項 **A 放行**（`push --guard-allow "<字>"`）／**B 改草稿**，附建議 |
| `跟網站上的一樣，不用推` | 照實回報 |
| `入口網站 repo 有未 commit 的修改` | 不要動那些修改；回報給使用者 |
| `N 秒內還沒合併` | 開 MR 頁看 pipeline；20261001 Frank 的 MR !75 也發生過，不合就請使用者到 MR 頁按 Merge |

## 5. 補寫好幾天

`daily_upload.py status [--since YYYYMMDD]` 列出還沒有日報的工作天（預設從日報功能上線的 20260928 起算，只算週一到週五）。
每天各跑 `collect --date <d>` 寫好草稿 → `check` → `push --date d1,d2,…`：**一個 commit、一個 MR**（不用先貼給使用者看；推完列出各篇的一句話與網址）。
每篇的「一句話」都以「（補寫）」開頭。週末做的事（裁決、案件）併進下一個工作天，除非使用者要單獨一篇。

## 6. 其他子指令

- `daily_upload.py add --item "…" --customer "…" --result "…" --status 完成 [--link "…"]`：白天手動記一列（不是結案的事也可以）。

## 7. 界線

- 入口網站只推日報 md（不碰 `site-data.js`、不推 main、不轉 html——部署時 CI 會轉）；這**不是** HT9045 repo 的推送，**不觸發** GitHub 機台更新包。
- 一天推一次（Steven 20260929）。結案時 `close_case.py` 只補列、不推。
- 不改 `weekly_data.json`。`daily/` 的草稿、證據、`rows.json` 是 Weekly_AI 的檔，`sync.py push` 會一起推進私人 GitHub `weeklyreport`；
  Weekly_AI「一次只在一台動」的規矩對它們一樣適用。
- 結尾照慣例寫完成狀態（已上線／被擋待決定）。
- 同一台有別的 session 正在推（入口網站 repo 有 `index.lock`、或有 `daily_upload.py push` 的程序在跑）就先等它結束，不要同時推（20261006 NB2 兩個 session 撞過一次）。
