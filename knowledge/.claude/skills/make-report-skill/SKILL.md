---
name: make-report-skill
description: >
  統一的報告產生技能。涵蓋客戶提案表、Release Note、個人/部門週報、
  ops 工作紀錄、除錯報告、程式碼差異摘要、出差報告、change log 等報告類型。
  適用關鍵字：日報（20261003 起 RD5 統一格式）、週報、提案表、release note、除錯報告、使用報告、
  上線前掃描、ops log、daily worklog、weekly review、code diff summary、travel report、
  change log、變更紀錄、變更日誌。
applyTo: "**/*"
---

# make-report-skill — 統一報告產生技能

> ## ⚠ 這個 skill 有兩份，正本在 `D:\.github`
>
> | 位置 | 角色 | 版控 |
> |---|---|---|
> | `D:\.github\skills\make-report-skill\` | **正本**，所有修改都改這裡 | ❌ 無（`D:\.github\.git` 已損壞） |
> | `D:\HT9045\.claude\skills\make-report-skill\` | 鏡像，給 Jimmy 經 git 取得 | ✅ `ht9045.git` / `feat/v912-port` |
>
> **兩份必須逐位元組相同。** 改完正本要重新複製一次，不要只改其中一邊
> —— `D:\HT9045` 底下 `.agents` / `.claude` / `.github` 三個 skills 目錄已經有
> **24 個 skill 內容漂移**（2026-09-18 實測），這個 skill 不要變成第 25 個。
>
> ⚠ 檔內 `d:\.github\skills\make-report-skill\scripts\...` 這類腳本路徑**刻意指向正本**，
> 不隨鏡像改寫。Jimmy 那台若沒有 `D:\.github`，報告產生腳本不可用（但 reference 文件照樣可讀）。


## ⚙ 輸出位置設定（20260929 起：所有報告與手冊都直接寫進 RD5 入口網站 repo 的 `public\Docs\`）

> Steven 20260928 22:3x：「另外也要建議修改 skill, 讓日報跟 change log跟release bote自動寫到這邊的資料夾裡面」「通知st01-m把上面的報表 skill 進行修改」。
> Steven 20260929 11:15：「未來報告跟手冊的寫入路徑都要放入 D:\RD5-Portal\public\Docs\ 裡面」（週報、手冊、Merge Report 等也一起搬）。
> 目的：產出的報告直接寫進入口網站 repo，同事不用再把檔案搬過去，就能建置、開 MR。

**兩個設定（每位同事各自不同，寫報告前先確認）**

| 設定 | 意思 | Steven 的值 |
|---|---|---|
| `<repo>` | 自己本機 RD5 入口網站 repo 的 clone 位置（GitLab `honprec/rd/rd5/9050motionview`） | `D:\RD5-Portal`（SOP 給同事的例子是 `D:\HT9045-Index`） |
| `<EnglishName>` | 組織表上的英文名 | `Steven` |

> **其他 skill 寫的 `<入口網站 repo>` 就是上表的 `<repo>`**（St02 20261005，Steven「其他文件也要跟著換路徑」）：St01／St02＝`D:\RD5-Portal`、筆電＝`D:\HT9045-Index`。共用 skill 不寫死某一台的路徑；會自己寫檔的腳本（pre-release-check `scan_and_report_pre_release.py`、ht9045-io-control `gen_io_alias_doc.py`）依序找環境變數 `RD5_PORTAL_REPO`（與 `generate_code_diff_summary.py` 同一個）→ `D:\RD5-Portal` → `D:\HT9045-Index`，都沒有才退回舊的 `D:\docs\…` 並印警告。

**寫到哪裡**（Steven 20260929 11:15：「未來報告跟手冊的寫入路徑都要放入 D:\RD5-Portal\public\Docs\ 裡面」——所有報告與手冊都寫進 `<repo>\public\Docs\`）

| 報告 | 以前 | 現在 |
|---|---|---|
| 日報 | `D:\docs\ops\daily\{YYYYMMDD}.md` | `<repo>\public\Docs\Daily\<EnglishName>\{YYYYMMDD}.md`（**20261003 起用 RD5 統一格式**：一句話／今日完成／卡點／需要協助／明天接續四段、一頁內；見 [ops-daily-worklog.md](./references/ops-daily-worklog/ops-daily-worklog.md)） |
| Change Log | `D:\docs\ChangeLog\CHANGES_{YYYYMMDD}_{作者}.md` | `<repo>\public\Docs\ChangeLog\<EnglishName>\CHANGES_{YYYYMMDD}_<EnglishName>.md` |
| Release Note／提案／Bug 修正 | `D:\docs\customers\{代理商}\{客戶代碼}_{客戶名稱}\release-notes\|proposals\{YYYY}\` | `<repo>\public\Docs\customers\…`（底下結構不變） |
| 個人週報 | `D:\docs\ops\weekly\{YYYY}\{MM}\{YYYYMMDD}\` | `<repo>\public\Docs\weekly\<EnglishName>\{YYYYMMDD}.md`（**小寫 `weekly`**；日期＝週報日（週三）；最省事：`py tools/weekly2md.py <個人週報.xlsx> --name <EnglishName> --zh <中文名>`，.xlsx／.msg 不進 git） |
| 除錯／風險報告、工程報告、出差報告、部門週報 | `D:\docs\ops\weekly\{YYYY}\{MM}\{YYYYMMDD}\` | `<repo>\public\Docs\weekly\{YYYY}\{MM}\{YYYYMMDD}\` |
| 手冊／說明書 | `D:\docs\manual\` | `<repo>\public\Docs\manual\`（子資料夾照舊，如 `GPIB_Manual\`） |
| Merge Report | `D:\docs\release-notes\MergeReport\<年度>\` | `<repo>\public\Docs\MergeReport\<年度>\` |
| 廠內提案 | `D:\docs\proposals\{YYYY}\` | `<repo>\public\Docs\proposals\{YYYY}\` |
| 上線前掃描報告 | `D:\docs\PreReleaseCheck\{YYYY}\` | `<repo>\public\Docs\PreReleaseCheck\{YYYY}\` |
| 其他報告 | `D:\docs\other\{YYYY}\` | `<repo>\public\Docs\other\{YYYY}\` |
| 程式碼差異摘要（腳本自動寫） | `D:\docs\ops\daily\{日期}_code_diff.md` | `<repo>\public\Docs\Daily\<EnglishName>\{YYYYMMDD}_code_diff.md` |

- 檔名照舊。**日報一定要是 `YYYYMMDD.md`**（附檔用 `YYYYMMDD_*.*`），入口網站的建置不收 `YYYY-MM-DD.md`。
- Merge Report **不要**放進 `Docs\Release Note\`：入口網站把那一層當代理商／地區資料夾，會多出新警告。
- 內容照原樣上線：Steven 20260929 11:26「這網站只有公司內部人士能看, 我後續會請MIS加入權限管控, 目前先全部上線」。只有格式限制：文件只收 .html／.pdf／.md（.xlsx、.msg、.txt、錄音檔不進 `public\`，要轉成 .md 或 .html）。
- 20260929 之前的舊檔還在 `D:\docs\…` 對應位置；週報讀取來源時兩邊都讀。
- 腳本 `scripts\generate_code_diff_summary.py` 已改寫到 `<repo>\public\Docs\Daily\<EnglishName>\`（環境變數 `RD5_PORTAL_REPO`，預設 `D:\RD5-Portal`；`REPORT_ENGLISH_NAME`，預設 Windows 登入名）；資料夾不存在就不寫。

**寫完之後（照 Steven 20260928 的推送規則「要推的時候要改用先開branch, 然後才能push + mr要求」）**

1. （可選）`node tools/md2html.js` 產一份 HTML。
2. `py tools/build_portal.py`，不能多出新的警告。
3. **日報**：commit 之後、推送之前，在入口網站 clone 跑 `py tools/check_daily.py --changed` 自我檢查（看這個分支相對 origin/main 改到、日期 ≥ 20261003 的日報；還沒 commit 就用 `py tools/check_daily.py <檔>`），列出的項目照 [ops-daily-worklog.md](./references/ops-daily-worklog/ops-daily-worklog.md) 改好再推。
   這是自我檢查（Steven 20261003：「不用阻擋同事的報告」）：GitLab MR 的 check-portal 與 git pre-push 只列出不合格項目，MR 照常自動合併；Claude 在入口網站 repo 推送時會暫停一次，讓 Claude 照 rd5-daily-report 調整後再推（同一份內容再推一次就放行）。
4. 開分支 `<name>/<YYYYMMDD>-<topic>`，然後推送並開 MR：
   `git push -u origin HEAD -o merge_request.create -o merge_request.target=main -o merge_request.remove_source_branch`
   由 Steven 審完合併。建議節奏：早上推前一天的，或下班前推當天的。不可以直接推 main。
5. 日報格式與流程的正本：`<repo>\.claude\skills\rd5-daily-report\SKILL.md`（§2 格式、§3 寫作規則、§3.5 推送前的格式檢查；Steven 的 clone 是 `D:\RD5-Portal`），範本 `<repo>\.claude\skills\rd5-daily-report\references\template.md` §統一格式。20261003 起全員用統一格式；本 skill 的 ops-daily-worklog 照抄正本，不一致時以正本為準。

> **St01 的日報（Steven 的每日工作日誌）**：20260929 起由 ST01-M 編輯（Steven「日報一律通報 ST01-M做內容編輯」），寫在 `D:\RD5-Portal\public\Docs\Daily\Steven\YYYYMMDD.md`（Steven 20260929 15:2x：「今天的日報…要改寫到 D:\RD5-Portal\public\Docs\Daily\Steven 裡面」），一天一份，20261003 起同樣用統一格式；改好由 **ST01-E3** 推送、開 MR（Steven：「改好通知 st01-e3 push」「他負責 protal的管理」）。20260928 以前的舊日報與寄信存查仍在 `D:\docs\ops\daily\`。~~St01 記錄員（`D:\HT9045\.claude\skills\ops-st01-clerk-report\`）的 ChangeLog 仍寫 `D:\docs\ChangeLog\`、repo 日報仍寫 `docs\ops\daily\`，到 Steven 另外說為止。~~
> **Steven 20261005 17:0x 另外說了**：「D:\docs\ChangeLog\ 不是應該改存放到 D:\RD5-portal\public\Docs\ChangeLog 嗎?」⇒ **ChangeLog 一律寫入口網站**，`D:\docs\ChangeLog\` 不再寫新檔（舊檔留著當史料，不搬也不刪）：
> - St01（記錄員）：`D:\RD5-Portal\public\Docs\ChangeLog\Steven\CHANGES_{YYYYMMDD}_Steven.md`（記錄員 skill 的路徑由 ST01-M 改）。
> - St02（STEVEN-NB3 的 St02-E）：`D:\RD5-portal\public\Docs\ChangeLog\Steven02\CHANGES_{YYYYMMDD}_Steven02.md`。**另開 `Steven02\` 資料夾**：入口網站 `tools/build_portal.py` 以「資料夾名＋日期」當一筆，放進 `Steven\` 會跟 St01 同一天的 ChangeLog 互相蓋掉；`Steven02` 對不到組織表，建置時印一行警告、照樣列出。0927～1005 的 9 份已從 `D:\docs\ChangeLog\` 複製過去（原檔留著）。
> - 入口網站歸 ST02-M：commit／MR（含 md2html 轉出的 .html）由 ST02-M 做，寫檔的人只寫 .md。
> - repo 日報（`docs\ops\daily\`）這次沒有改，照舊。

## 報告類型路由表

| 報告類型 | 版本 | 參考文件 | Template | 格式 |
|----------|------|----------|----------|------|
| 軟體功能新增提案表（對內詳細版）| 含程式碼 diff、完整技術說明 | [customer-request.md](./references/customer-request/customer-request.md) | [honprec-red-template](./templates/honprec-red-template/SKILL.md) | MD |
| 軟體功能新增提案表（代理商版）| 含對應客戶資訊、流程圖，無原始碼 | [customer-request.md](./references/customer-request/customer-request.md) | [honprec-red-template](./templates/honprec-red-template/SKILL.md) | HTML |
| 軟體功能新增提案表（客戶版）| 僅流程圖 + 功能說明；不足時向客戶提問 | [customer-request.md](./references/customer-request/customer-request.md) | [honprec-red-template](./templates/honprec-red-template/SKILL.md) | HTML |
| **軟體異常修正紀錄表**（Bug Fix Report，廠內版）| 含根本原因分析、程式碼位置、修改建議；**MD 不放 Logo** | [bug-fix-report.md](./references/bug-fix-report/bug-fix-report.md) | — | MD only |
| **軟體異常修正紀錄表**（Bug Fix Report，客戶版）| 僅流程說明 + 修改摘要；不含原始碼 | [bug-fix-report.md](./references/bug-fix-report/bug-fix-report.md) | [honprec-red-template](./templates/honprec-red-template/SKILL.md) | MD + HTML |
| Release Note（廠內版）| 完整技術細節：含 .cpp/.h 程式碼、函式名、變數、RCA | [release-note.md](./references/release-note/release-note.md) | [honprec-blue-template](./templates/honprec-blue-template/SKILL.md) | MD + HTML |
| Release Note（代理商版）| 業務描述、流程圖；不含原始碼 | [release-note.md](./references/release-note/release-note.md) | [honprec-red-template](./templates/honprec-red-template/SKILL.md) | MD + HTML |
| Release Note（客戶版） | 僅該客戶相關修復 + 升級建議；不含原始碼 | [release-note.md](./references/release-note/release-note.md) | [honprec-red-template](./templates/honprec-red-template/SKILL.md) | MD + HTML |
| 個人週報 | — | [personal-weekly-report.md](./references/personal-weekly-report/personal-weekly-report.md) | [personal-weekly-report-template](./templates/personal-weekly-report-template/) | MD + HTML |
| 部門週報 | — | [group-weekly-report.md](./references/group-weekly-report/group-weekly-report.md) | [group-weekly-report-template](./templates/group-weekly-report-template/) | HTML |
| 除錯報告 / 使用報告 / 上線前掃描 | 預設 MD+HTML；若使用者指定「MD only」則僅輸出 MD | [debug-and-risk-report.md](./references/debug-and-risk-report/debug-and-risk-report.md) | [honprec-blue-template](./templates/honprec-blue-template/SKILL.md) | **MD 或 MD+HTML（可選）**|
| 其他報告 | — | [other-report.md](./references/other-report/other-report.md) | 視情況 | MD / HTML |
| ops daily worklog（日報）| **20261003 起 RD5 統一格式**（一句話／今日完成／卡點／需要協助／明天接續，一頁內）；輸出 `<repo>\public\Docs\Daily\<EnglishName>\{YYYYMMDD}.md`；推送前 `py tools/check_daily.py --changed` 自我檢查 | [ops-daily-worklog.md](./references/ops-daily-worklog/ops-daily-worklog.md) | [ops-daily-worklog-template](./templates/ops-daily-worklog-template/template.md) | MD only |
| ops weekly review | — | [ops-weekly-review.md](./references/ops-weekly-review/ops-weekly-review.md) | [ops-weekly-review-template](./templates/ops-weekly-review-template/template.md) | MD only |
| code-diff-summary | — | [code-diff-summary.md](./references/code-diff-summary/code-diff-summary.md) | [code-diff-summary-template](./templates/code-diff-summary-template/template.md) | MD only |
| 合併報告（Merge Report）| 含 Section 1–9、功能來源追蹤（Section 8）、SVN Commit Message（Section 9）；合併流程、衝突解決、build 驗證 | [merge-report.md](./references/merge-report/merge-report.md) | — | MD only |
| 個人出差報告 | — | [personal-travel-report.md](./references/personal-travel-report/personal-travel-report.md) | 視情況 | MD |
| **GPIB Command Manual**（通訊指令手冊）| EN / ZH；Standard 或 Full（含 ART / 溫控 / Qorvo 等）| [gpib-manual.md](./references/gpib-manual/gpib-manual.md) | [honprec-blue-template](./templates/honprec-blue-template/SKILL.md) | MD + HTML |
| **Change Log**（當日變更紀錄）| 技術交接用；以異動內容為單位，非時間軸；含標頭標記慣例、已知落差、備份位置 | [change-log.md](./references/change-log/change-log.md) | [change-log-template](./templates/change-log-template/template.md) | MD only |

> **合併報告（Merge Report）注意**：報告結構與 Section 定義詳見 [references/merge-report/merge-report.md](./references/merge-report/merge-report.md)。若有專案特例，應優先回寫到 make-report-skill 的 reference 或模板，而非在個別 agent 重複定義。
> **模板別名**：「鴻勁紅」= `honprec-red-template`（客戶/代理商用）；「鴻勁藍」= `honprec-blue-template`（除錯/工程報告用）。
> **⚠ Change Log 輸出路徑**：一律 `<repo>\public\Docs\ChangeLog\<EnglishName>\CHANGES_{YYYYMMDD}_<EnglishName>.md`（例：`CHANGES_20260918_Steven.md`）。**所有專案共用這一個目錄，不依專案分子目錄，也不放在專案樹底下**（如 `D:\HT9045\`）。完整規則見 [references/change-log/change-log.md](./references/change-log/change-log.md) §輸出路徑與命名。

---

## HTML 報告生成規則（強制）

> AI 禁止直接讀取 Base64 參考檔來生成 HTML。正確流程是先產出 `.md`，再用腳本轉換。

### MD → HTML 轉換流程

```text
1. AI 產出 MD 報告（含 YAML frontmatter），儲存至目標路徑
2. 執行腳本轉換 HTML：
   python "d:\.github\skills\make-report-skill\scripts\md_to_html.py" <output.md> [--template red|blue]
```

| 報告格式 | Logo 處理 |
|----------|-----------|
| HTML 報告 | 執行 `md_to_html.py` 腳本，自動讀取 PNG |
| Markdown 報告 | 不放 Logo，改用純文字 metadata 表格 |

---

## 報告版本命名轉換

- `廠內版`、`代理商版`、`客戶版` 既是 audience 分類，**也直接作為檔名尾碼的固定字樣**。
- 檔名一律以 `{YYYYMMDD}` 為前綴；尾碼固定使用 `廠內版` / `代理商版` / `客戶版`（不再使用 `{代理商名稱}`/`{客戶名稱}` 或 `廠內版本`/`代理商版本`/`客戶版本`）。
- 代理商 / 客戶識別改由目錄 `{客戶代碼}_{客戶名稱}` 承載；目錄與檔名變數分開處理。
- 輸出路徑：Release Note → `<repo>\public\Docs\customers\{代理商}\{客戶代碼}_{客戶名稱}\release-notes\{YYYY}\`；提案報告 → 同客戶 `proposals\{YYYY}\`；手冊 → `<repo>\public\Docs\manual\`；**Change Log → `<repo>\public\Docs\ChangeLog\<EnglishName>\`**（不分專案、不分年份子目錄）。
- ⚠️ **Change Log 不套用上面的「版本尾碼」規則**：它是技術交接文件，沒有廠內版／代理商版／客戶版之分。檔名固定 `CHANGES_{YYYYMMDD}_{作者}.md`，`{作者}` 用使用者本人英文名（例：Steven），不用專案名。
- ⚠️ **常見錯誤**：Release Note（含廠內版）一律放客戶資料夾的 `release-notes\{YYYY}\`，**不可**放在 `D:\docs\release-notes\` 或其他暫存路徑。產出前先用客戶代碼定位客戶資料夾（如 TFME CC_916 → `<repo>\public\Docs\customers\鴻勁興業\916_TFME_CHINA\`）。
- **Release Note 模板對應**：廠內版用 `honprec-blue-template`（工程藍）；代理商版 / 客戶版用 `honprec-red-template`（客戶紅）。
- 完整規格、範例與判斷順序統一以 [report-version-naming.md](./references/report-version-naming/report-version-naming.md) 為準。

---

## 客戶代碼管理

客戶代碼定義與分發規則詳見：
- [customer-code-manager/SKILL.md](./customer-code/customer-code-manager/SKILL.md)（報告/索引端；與 HT9045 端 `ht9045-customer-code-manager` 雙向同步）
- 客戶縮寫/代理商/語言唯一來源：[customer-code-table.instructions.md](../../instructions/customer-code-table.instructions.md)

---

## 腳本工具

| 腳本 | 用途 |
|------|------|
| [generate_release_note.py](./scripts/generate_release_note.py) | 多語言 Release Note 自動產生 |
| [method-a_excel_to_md_python.py](./scripts/method-a_excel_to_md_python.py) | Excel → Markdown 週報轉換 |
| [method-d_summary_optimizer.py](./scripts/method-d_summary_optimizer.py) | 週報摘要最佳化 |
| [method-e_date_standardizer.py](./scripts/method-e_date_standardizer.py) | 日期格式標準化 |
| [generate_code_diff_summary.py](./scripts/generate_code_diff_summary.py) | mtime 模式程式碼差異摘要產生 |
| [generate_personal_travel_report.ps1](./scripts/generate_personal_travel_report.ps1) | 個人出差報告產生 |
