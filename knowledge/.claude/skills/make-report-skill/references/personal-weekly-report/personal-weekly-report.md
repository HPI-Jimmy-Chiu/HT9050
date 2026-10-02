# 個人週報（Personal Weekly Report）

## 適用時機
- 產生 RD5 個人週報 Markdown（`.md`）+ HTML（`.html`）
- 來源：Excel 週報表、出差服務案件、VS Code 工作紀錄（四種可混合）
  - VS Code 工作紀錄包含：VS Code Local History / Timeline（`1.7`）與 Workspace Session Memory（`1.8`）

## 輸出路徑
- 主要：`<repo>\public\Docs\weekly\<EnglishName>\{YYYYMMDD}.md`（小寫 `weekly`，日期＝週報日；入口網站週報頁讀這裡；Excel 版可用 `py tools/weekly2md.py <個人週報.xlsx> --name <EnglishName> --zh <中文名>` 轉成這個檔）
- 備份：生成後 robocopy 至 `R:\研五_共用區\AI\Weekly Report\` 對應路徑

## 輸出格式
- **Markdown**（`.md`）：純 metadata 表格 + 內容（AI 友好格式），**不放 Logo**
- **HTML**（`.html`）：完整版面，**Logo 使用 Base64 內嵌**（從舊版 HTML 或 `templates/personal-weekly-report-template/references/HonPrec_Logo_Base64_Reference.md` 取得）
- 兩種格式同時輸出

## 可設定變數

| 變數 | 預設值 |
|---|---|
| `PERSON_NAME` | `周廷瑋` |
| `DEPT_NAME` | `研五` |
| `WORK_DOMAIN` | `軟體` |
| `REPORT_DATE` | 當天日期 |

## 固定輸出結構（必須依序）

1. `# RD5 個人週報`
2. 基本欄位（行內粗體格式）：`**人員**：xxx`、`**部門**：xxx`、`**週期**：`、`**報告日期**：`、`**主要工作區**：`，後跟 `---`
3. `## 週報表`（10 欄表格）
4. `---`
5. `## 摘要`（本週工作總結 + 下週建議）
6. `## 行動/解決方案 歷史內容`（若有歷史資料，置最後）

## 固定表頭（10 欄）
```
| 序號 | 部門 | 客戶 | 事件/議題 | 行動/解決方案 |
| 負責人(主) | 負責人(協) | 預定完成日 | 實際完成日 | 備註 |
```

## 命名規則
```
RD5_個人週報_{人員}_{YYYY}_{MM}_{DD}.md
RD5_個人週報_{人員}_{YYYY}_{MM}_{DD}.html
```

## 工作紀錄來源

生成週報時，主動參考以下來源（按優先順序）：

### 1. ops 日誌（主要來源）
- `<repo>\public\Docs\Daily\<EnglishName>\{YYYYMMDD}.md` ← **主要日誌，優先讀取**（20260929 之前的舊日誌在 `D:\docs\ops\daily\`，一併讀）

### 1.5. CRM 出差紀錄（個人出差報告整合）
- 在生成週報前，**自動執行個人出差報告腳本**，取得本週 CRM 出差/客戶服務回覆紀錄
- 執行方式：
  ```powershell
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File "d:\.github\skills\make-report-skill\scripts\generate_personal_travel_report.ps1" -StartDate "{YYYYMMDD（週開始日）}"
  ```
- 將 CRM 查詢結果中每一筆「客戶服務明細檔」或出差記錄，整合進週報表中：
  - `客戶` 欄：填入 CRM 回傳的客戶/案件名稱（若無則填 `-`）
  - `事件/議題` 欄：填入 CRM `Issue`（問題說明）
  - `行動/解決方案` 欄：填入 CRM `Action`（問題處理）
  - `負責人(主)` 欄：填入 CRM `Owner`（處理人員）
  - `預定完成日` / `實際完成日`：填入 CRM `CreatedDate`（若有結案日期則分別填入）
  - `備註` 欄：填入 CRM `ReplyNo`（回覆單號）
- 若查無資料（API 回傳 `data` 為空），在摘要說明「本週無 CRM 出差服務記錄」，不捏造
- CRM 資料優先補充到 ops 日誌未涵蓋的客戶現場工作

### 1.7. VS Code Local History（Timeline）
掃描 VS Code 本機 Timeline 歷史紀錄，取得本週有實際編輯過的 workspace 檔案清單：

```powershell
$startMs = [DateTimeOffset]::Parse("{YYYY-MM-DD（週開始日）}").ToUnixTimeMilliseconds()
$endMs   = [DateTimeOffset]::Parse("{YYYY-MM-DD（週結束日）} 23:59:59").ToUnixTimeMilliseconds()
$histRoot = "$env:APPDATA\Code\User\History"
Get-ChildItem $histRoot -Recurse -Filter "entries.json" | ForEach-Object {
    $j = Get-Content $_.FullName -Raw | ConvertFrom-Json
    $hits = $j.entries | Where-Object { $_.timestamp -ge $startMs -and $_.timestamp -le $endMs }
    if ($hits) { [PSCustomObject]@{ File = $j.resource; EditCount = $hits.Count } }
} | Where-Object { $_.File -match "^file:///d:/" }  # 只取本 workspace 內的檔案
```

- 產出「本週編輯過的檔案 + 次數」列表
- 依檔案路徑推斷對應的子系統（HT9045、GPIB9045、RS232Standard、MDB Updater、.github）
- 每個子系統的編輯活動 → 補為週報表一列（事件/議題：`{子系統} 程式修改`；行動/解決方案：填入主要編輯檔案名稱）
- 若 ops 日誌已有同檔案的說明，**不重複新增列**，僅將檔案名補充到備註
- 若查無任何本週編輯紀錄，略過，不捏造

### 1.8. Workspace Session Memory（當次對話工作摘要）
讀取 `/memories/session/` 下所有 `.md` 檔案，將未完成任務或工作摘要整合入週報：

- 每筆有實質進度的任務 → 補為週報表一列（事件/議題 + 行動/解決方案）
- 若 session memory 為空或僅有暫存筆記，略過，不捏造
- 此來源優先級低於 ops 日誌；若內容重複，以 ops 日誌為準

### 2. 本週產出文件（<repo>\public\Docs\customers；20260929 之前的在 D:\docs\customers，一併讀）
**必須主動查閱**，補充週報中未記錄的正式產出：
- `<repo>\public\Docs\customers\{代理商}\{客戶代碼}\proposals\{YYYY}\` — 提案表
- `<repo>\public\Docs\customers\{代理商}\{客戶代碼}\release-notes\{YYYY}\` — Release Note
- `<repo>\public\Docs\proposals\{YYYY}\` — 廠內提案
- `<repo>\public\Docs\MergeReport\{YYYY}\` — 合併報告
- `<repo>\public\Docs\PreReleaseCheck\{YYYY}\` — 上線前掃描報告
- `<repo>\public\Docs\weekly\{YYYY}\{MM}\` — 同一週生成的報告
- 20260929 之前的廠內提案、合併報告、上線前掃描報告、週報在 `D:\docs\` 的對應位置，一併讀。

> 凡本週在上述路徑有新建檔案，應在週報「事件/議題」欄補入對應行目，並於「備註」欄標記產出路徑。

## 摘要自動生成規則
1. 必須根據當週週報表實際內容彙整，不可使用固定樣板
2. 至少涵蓋：主要完成項目、重點議題/風險、主要客戶/專案 三項中的兩項以上
3. 若資料不足，標示「待補」，不可捏造

## HTML 生成指令

```bash
python "d:\.github\skills\make-report-skill\scripts\md_to_html.py" <input.md> --template weekly
```

> ⚠️ 週報格式，必須使用 `--template weekly`。

## HTML 設計規格（鴻勁週報格式）

### CSS 色彩 Token

| 元素 | 樣式 |
|------|------|
| `h1` | `color: #c00; border-bottom: 2px solid #c00` |
| `h2` | `color: #005a9e; border-left: 4px solid #005a9e` |
| `th` | `background: #003e7e; color: #fff` |
| `.meta strong` | `color: #005a9e` |
| `.summary` | `background: #fffbf0; border: 1px solid #e8c840` |
| 偶數列 `td` | `background: #f0f4fb` |

### HTML 結構（與 honprec-red 的差異）

| 區塊 | 鴻勁週報格式 | honprec-red 格式 |
|------|------------|------------------|
| Logo | `<img width="100%">` 全寬顯示 | flex row 與 h1 同行 |
| 版頭 | Logo + `<h1>` 分行 | `<header>` flex block |
| Metadata | `<div class="meta">...<strong>人員</strong>...</div>` | `<p><strong>人員</strong>...</p>` |
| 摘要 | `<h2>摘要</h2><div class="summary">...</div>` | 無特殊包裝 |
| 外層容器 | 無 `.page` wrapper | `<div class="page">` |

### 自動後處理規則（md_to_html.py --template weekly）

1. **Metadata 段落**：第一個含 `**人員**` 的 `<p>` → 替換為 `<div class="meta">`
2. **摘要區塊**：`<h2>摘要</h2>` 之後的內容至 `<hr>` → 包裹於 `<div class="summary">`

## 腳本工具
- `../../scripts/method-a_excel_to_md_python.py` — Excel → Markdown
- `../../scripts/method-d_summary_optimizer.py` — 摘要最佳化
- `../../scripts/method-e_date_standardizer.py` — 日期格式標準化

## 模板
`../../templates/personal-weekly-report-template/`
