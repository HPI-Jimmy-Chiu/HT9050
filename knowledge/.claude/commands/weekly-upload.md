---
description: "上傳週報：本週週報貼上 RD5 入口網站（內容檢查乾淨才推，有命中才停下來問）＋照上一封週報信的格式做好本週的信（預設存草稿開視窗）。關鍵字：上傳週報, 貼週報, 寄週報, 週報上網站"
---

# 上傳週報（使用者 20260930）

使用者說「**上傳週報**」就直接執行，不用先問（使用者原話：「當我說關鍵字[上傳週報]，就自動幫我執行」）。
工具在 Weekly_AI：`tools/weekly_upload.py`（總指揮）→ `tools/publish_portal.py`（入口網站）＋ `tools/weekly_mail.ps1`（週報信）。

## 1. 執行

```
cd /d/Work-jimmychiu/document/WeeklyReport/Weekly_AI && PYTHONIOENCODING=utf-8 python -X utf8 tools/weekly_upload.py
```
（Bash timeout 給 600000；會跑 1～6 分鐘，大部分在等 MR 自動合併）

它會依序做：
1. 用 `weekly_data.json` 重產本週 Excel（不回寫 JSON）
2. 週報 md 貼上入口網站：`jimmychiu/<日期>-weekly` 分支 → MR（指派 steven、檢查過自動合併、合併後刪分支）
   - **內容檢查**（使用者 20260930 Q2＝A）：信箱、內網 IP、手機號碼、密碼／權杖、金鑰——**乾淨才推，有命中就不推**
3. 週報信：收件人／副本照抄上一封週報信、主旨＝Excel 檔名、附 Excel、內文＝上一封的「Hi Sirs,」＋標題＋簽名檔，
   中間的圖換成本週的「表頭＋紅字列」（Excel 自己畫的，等於以前手動貼的那張）。**預設存草稿並打開視窗，不寄出**
4. 等 MR 合併，印出入口網站頁面網址

## 2. 看結果、回報使用者

| 輸出 | 怎麼處理 |
|---|---|
| `[網站] 已合併上線` | 回報頁面網址與 MR 網址 |
| `[網站] 內容跟網站上的一樣，不用推` | 照實回報 |
| `[網站] ⛔ 沒有推：內容檢查擋下 N 處` | **停下來問使用者**（決策題，用回覆文字，不用彈窗）：用表格列出「哪一筆（客戶／事件）、抓到什麼、為什麼不該給全公司看」，給選項 **A 放行**（`weekly_upload.py --no-mail --guard-allow "<命中的字>"`）／**B 改資料**（用 `fix_brief.py` 或更新那一筆，再跑 `--no-mail`），附建議 |
| `[網站] ⛔ 沒有推：`（其他原因） | 照錯誤訊息處理（例：入口網站 repo 有未 commit 的修改＝Steven 信裡的第 2 點情況）；處理不了就回報 |
| `[信] ✅ 草稿已存好` | **先用 Read 打開「內文圖」那張 PNG 看一眼**：要有表頭＋N 列紅字、不能是空白。再回報主旨／收件者／副本／附件，問「要寄出嗎？」 |
| `[信] ❌ this week was already sent …` | 本週已寄過；照實回報，使用者明講才加 `--force-mail` |
| `[信] ❌ recipients differ …` | 收件人跟上一封不一樣，**不要硬寄**，回報給使用者 |

使用者說「**寄出**」→
```
cd /d/Work-jimmychiu/document/WeeklyReport/Weekly_AI && PYTHONIOENCODING=utf-8 python -X utf8 tools/weekly_upload.py --send-draft <EntryID>
```
（EntryID 在上一步輸出的「EntryID」那行；它會再核對一次收件人才寄）→ 回報寄出時間與收件人。

## 3. 界線

- 入口網站被擋的時候，信照樣做草稿：信的內容就是主管一直以來收到的那份，檢查只是為了「全公司都看得到」的網站。
- 不要改 `weekly_data.json`、不要推週（`copy_next_week.py`）——那是另外的事。
- 入口網站只推工具寫的那一個 md（不碰 `site-data.js`、不推 main）；這**不是** HT9045 repo 的推送，**不觸發** GitHub 機台更新包。
- 畫圖會用到剪貼簿（Excel 的 CopyPicture），使用者剪貼簿裡的東西會被蓋掉；回報時提一句。
- 結尾照慣例寫完成狀態（草稿待寄出／已寄出／網站被擋待決定）。
