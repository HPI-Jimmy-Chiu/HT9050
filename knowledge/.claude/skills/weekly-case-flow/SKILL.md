---
name: weekly-case-flow
description: >
  Weekly_AI 異常案件處理流程技能。涵蓋客戶異常 → 週報登錄 → Case 歸檔 → 看板總覽 → Release Note 完整鏈路。
  適用關鍵字：客戶異常、log 歸檔、case、CASE-ID、週報新增、看板、case registry、release note。
applyTo: "**/*"
---

# weekly-case-flow — Weekly_AI 異常案件處理流程

> 完全自包，無外部 Skill 依賴。模板與 Logo 內建於 `assets/`。

## 核心識別碼

| 識別 | 用途 | 變動性 |
|---|---|---|
| **`CASE-<EngCode>-YYYYMMDD-NNN`** | 長期主鍵，跨 session 溝通用（新格式，全域唯一） | 永久不變 |
| `CASE-YYYYMMDD-NNN`（舊案） | 2026-06-22 前建立的舊案，永久保留不改名 | 永久不變 |
| `weekly_data.json row=N` | 週報臨時索引 | 重整週報時可能變動 |
| `Customer/<folder>/CASE-*/` | 案件實體位置 | 永久 |

> **原則**：對話與檔名以 **CASE-ID** 為主鍵；row 為輔助。
> **CASE-ID 格式（ADR-007）**：新案 = `CASE-<EngCode>-YYYYMMDD-NNN`，`<EngCode>` 取自 `MachineType.h` 官方 `CC_*` 客戶代碼（如 力成PTI→`PTI`、超豐→`Greatek`、盛合晶微→`SJ_Semiconductor`），由 `tools/customer_code_map.json` 對照。`archive_issue.py` 自動生成；查無代號則 fallback 舊格式並提醒補對照表。舊案 `CASE-YYYYMMDD-NNN` 永久相容、不改名。
> **事件型 case（ADR-009，2026-09-23 起）**：非客訴的開發／維護／工具導入／版本定錨等事件也要有 case id，一事一案，代號段前面加 `EVT_`：`CASE-EVT_<EngCode>-YYYYMMDD-NNN`（例 `CASE-EVT_PTI-20260909-001`；HT160 系列 `CASE-EVT_HT160S_KYEC_CHEN-…`；查無代號 `CASE-EVT-…`）。建案加 `--event`，收尾**不跑 close_case.py**，見下方 ADR-009。

## 標準 SOP

### Step 1：登週報 + 歸檔

```powershell
# 已有檔案：建 case 並複製到 01_intake/
python tools\archive_issue.py <row> <log_or_screenshot...> --desc "<短描述>" \
    [--category P|R|B|E|Q] [--serial PPLS1266] \
    [--tags secs-gem,retry-logic] [--component SECS_INTERFACE] [--severity P2]

# 還沒檔案：先建殼（issue.md + 4 子夾）
python tools\archive_issue.py <row> --skeleton-only --desc "<短描述>" \
    [--category ...] [--serial ...] [--tags ...] [--component ...] [--severity ...]
```

**欄位說明**：
- `--category`：`P`=Problem `R`=Request `B`=Bug `E`=Enhancement `Q`=Question（預設 P）
- `--serial`：客戶機台序號（如代理商給的 PPLS1266）；未提供則填 `Default`

執行後：
- 建立 `Customer/<archive_name>/CASE-<EngCode>-YYYYMMDD-NNN_<desc>/`（代號化，見 ADR-007；查無代號則 fallback `CASE-YYYYMMDD-NNN`）
- 4 個子夾：`01_intake / 02_analysis / 03_fix / 04_release`
- `issue.md` 寫 metadata + intake timeline
- `weekly_data.json` row=N 的 `notes` 寫入 `case=CASE-<EngCode>-YYYYMMDD-NNN`
- stdout 必須列出 `case 根目錄`、`異常資料放置`、`分析筆記放置`、`修正證據放置`、`release 產物放置`；回覆使用者時至少提供 `01_intake/` 路徑。

### Step 2：產看板

```powershell
python tools\case_registry.py                # 全部
python tools\case_registry.py --status open  # 只看未結
python tools\case_registry.py --customer 超豐
python tools\case_registry.py --tag secs-gem # 跨客戶聖 tag
python tools\case_registry.py --component InArm
python tools\case_registry.py --category R   # 只看需求
python tools\case_registry.py --serial PPLS1266  # 查某台機歷史
python tools\case_registry.py --print        # 同時印 stdout
```

輸出：`Customer/_index/case-registry.md`

### Step 2.5：跨 case 聚合（Lessons Learned）

```powershell
python tools\build_lessons.py        # 生成 Customer/_index/lessons-learned.md
python tools\build_lessons.py --print
```

依 **component / tag / category / 客戶 / 機台序號** 多維聚合，抓出重複 root cause。
- **客戶×模組熱點矩陣**：數字 ≥2 = 該客戶反覆中招 → 建 Checklist 防呆
- **機台累犯分析**：同一 `machine_serial` 出現 ≥2 件 = 該機台個體故障，可能是硬體；
  若同型號多台都中招 = 軟體通病，必出 release note
- **類別統計**：P/R/B/E/Q 數量分佈，看本週工作重心是除錯多還是接需求多

### Step 3：分析與修復過程

新檔案直接拖到對應子夾，或同 case 加新檔：
```powershell
python tools\archive_issue.py <row> <new_file>   # 自動 reuse 既有 case
```

子夾用途：
| 夾 | 內容 |
|---|---|
| `01_intake/` | 客戶持續補的原始資料（log / 截圖 / 影片） |
| `02_analysis/` | 工程師分析、root cause doc、推論文件 |
| `03_fix/` | code diff、patch 摘要、修正版 EXE |
| `04_release/` | 最終 release_note .md/.html、客戶交付物 |

> ⚠ **跨 case 搜 log 不要用 Grep 工具（ripgrep）**（20261002 CASE-PTI-20261002-001 實測）：Weekly_AI 是 git repo，
> `.gitignore:27` 忽略 `Customer/**/01_intake/**`，ripgrep 預設**靜默跳過**被忽略的檔——同一個字串
> （`Initial Start clear data`）Python `os.walk` 掃到 11 個 EventLog，Grep 工具只回 3 個，沒有任何警告。
> 掃歸檔的 EventLog／Automation log／State Record 一律用 Python `os.walk`（讀檔 `decode("cp950","replace")`），
> 數字要拿 os.walk 的結果。另：OLP log 多數壓在 `Automation\yyyy_mm.7z`，沒解壓的月份掃不到，報數字時要寫掃了哪幾個月。

若事件判讀含老手工程師手法，請建立 `02_analysis/YYYYMMDD_<slug>_triage.md`，至少記錄：事實、待確認、老手工程師手法、建議下一步。典型手法包含：重啟既有 case 優先於另開、同一信件中不同模式需拆開判讀、主旨機型與歷史 case 不一致時先標待確認而非覆蓋 metadata、原始資料與推論分別放 `01_intake/` 與 `02_analysis/`。

### Step 4：產 Release Note 三版本

```powershell
python tools\make_release_note.py CASE-20260422-001 [CASE-...] \
    --version V3.33.899.9 \
    [--customer 超豐] \
    [--code-folder HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422] \
    [--from 2026-04-15] [--to 2026-04-22] \
    [--no-html]
```

> ⚠️ 同一 CASE-ID 可能出現在多客戶資料夾（每客戶 NNN 獨立計數），重複時必加 `--customer`。
>
> ⚠️ **`--code-folder` 必填情境**：當 release 的版本（例如 V899）**不是** `HT9011UC_Code_V*_Jimmy_*` 中名稱排序最新的那一個（例如目前最新是 V903），工具會掃錯版本資料夾、抓不到 //AI 註解。**修舊版本一定要顯式加 `--code-folder` 鎖定**。判斷準則：當 `--version` 與 `detect_active_version()` 抓到的版本不同 → 加 `--code-folder`。
> 範例（修 V899 但 workspace 已有 V903 資料夾）：
> ```powershell
> python tools\make_release_note.py CASE-20260508-001 \
>     --version V3.33.899.19 \
>     --code-folder HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422 \
>     --from 2026-05-12 --to 2026-05-12
> ```

執行後（在第一個 case 的 `04_release/`）：
- `{CASE-ID}_internal_zh-TW.md` + `.html` — 廠內版（含函式名 + `//AI` 註解全文）
- `{CASE-ID}_distributor_<lang>.md` + `.html` — 代理商版（中等，無原始碼；無代理商時略過）
- `{CASE-ID}_customer_<lang>.md` + `.html` — 客戶版（精簡，僅操作影響）

工具會自動：
1. 讀 case `issue.md` 抽 metadata
2. 偵測 `d:\HT9045\HT9011UC_Code_V*_Jimmy_*` 中**最新**版資料夾（依名稱 desc 排序）
3. 掃日期範圍內 mtime 變動的 `.cpp/.h`，正則抓 `//AI(AgentName) YYYYMMDD: 描述`
4. 套內建鴻勁模板（CSS + Logo Base64）：`*_internal_*` 使用鴻勁藍，`*_customer_*` / `*_distributor_*` 使用鴻勁紅

### Step 4.5：產 Customer Request 提案表（功能新增需求）

當 case 是 `category=R`（客戶需求）時，用此工具產提案表給客戶 / 代理商審視：

```powershell
python tools\make_proposal.py CASE-20260422-001 \
    --customer 南茂科技 \
    --version V3.33.899.9 \
    --topic "AutoContact 速度可調"
```

執行後（在第一個 case 的 `04_release/proposals/`）：
- `{date}_{author}_{machine}_{topic}_internal_zh-TW.md` + `.html` — 廠內版（含 P1~P6 風險清單、02_analysis + 03_fix 全文）
- `{date}_{author}_{machine}_{topic}_distributor_<lang>.md` + `.html` — 代理商版（無代理商略過）
- `{date}_{author}_{machine}_{topic}_customer_<lang>.md` + `.html` — 客戶版（含 4 項待客戶確認問題清單）

> 客戶版資訊不足時，依 Steven 規範自動列出 4 項提問清單（操作流程 / 期望結果 / 環境限制 / 驗收標準）。

### Step 5：週報狀態更新

修復完成後將該 row 的 status 改為 `done`。下週週報該項自動轉黑字。

## 結案 SOP（必跑完整鏈路，不可只更新週報）

當使用者說「結案」「出貨」「安裝包已給客戶」等表示完成時，**優先使用 `tools\close_case.py`**，不要手動拆步驟，也不要只更新週報：

```powershell
python tools\close_case.py --row <weekly_row> --version V<x.x.x.x> `
    [--desc "<結案描述>"] `
    [--installer "d:\path\HT9045_Vx.x.x.x_setup.exe"] `
    [--code-folder HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422]
```

`close_case.py` 會固定執行並在 stdout 回報下列六步；若 release note 失敗會中止，不可宣稱結案完成：

1. `weekly_data.json` status → `done`，補 `actual_completion` / `version_number`，新增 action（seq = max+1，新→舊排列）
2. `python tools\generate_report.py` 重產 Excel
3. `Customer/<客戶>/CASE-*/issue.md` status → `done`，補 `resolved_version` / `resolved_date`
4. **`python tools\make_release_note.py CASE-* --version V<x.x.x.x> --customer <客戶> --from <date> --to <date>`** ← 不可遺漏
5. `python tools\case_registry.py` 重生看板
6. 自動建立 `Customer/<客戶>/CASE-*/04_release/installer/` 空資料夾（使用者手動放安裝包進去）

如客戶交付安裝包，將 `.exe` / `.zip` 放到 `Customer/<客戶>/CASE-*/04_release/installer/`（同夾與 release note 共存，方便對應）。命名建議 `HT9045_V<version>_setup.exe`。

> **結案回覆必含（規則，不可省略）**：跑完 `close_case.py` 後，回覆使用者的**最後**一定要附上**安裝包放置資料夾的絕對路徑** `…\04_release\installer\`（完整 Windows 絕對路徑、可直接複製貼上），並明確請使用者把 `.exe` / `.zip` 安裝包放進去。這是結案回覆的固定收尾。

> 修舊版（例如 V899）但 `d:\HT9045` 最新資料夾已是 V903 時，`close_case.py` 也要加 `--code-folder`，避免 release note 掃錯版本。

## ADR（Architecture Decision Records）

長期決策記錄於 `Customer/_index/decisions/`：

| 編號 | 標題 |
|---|---|
| ADR-001 | CASE-ID 為長期主鍵 |
| ADR-002 | 不啟用 PreToolUse Hook |
| ADR-003 | Release Note 三版本 |
| ADR-004 | Issue Metadata 加 category + machine_serial |
| ADR-005 | CASE-ID 跨客戶歧義以 --customer 消歧（ADR-007 後退化為選填） |
| ADR-006 | HT172 Weekly Case Management |
| ADR-007 | CASE-ID 加入官方客戶英文代號（`CASE-<EngCode>-YYYYMMDD-NNN`） |
| ADR-008 | HT160S Weekly Case Management（客訴一案一列＋機型前綴；收尾不跑 close_case）——「非客訴併入量產維護列」一段已被 ADR-009 取代 |
| ADR-009 | 非客訴事件型 case：`CASE-EVT_<EngCode>-YYYYMMDD-NNN`，一事一案、不產 release note |

新決策請依 README 模板新增 `ADR-NNN-<slug>.md`。已 Accepted 的不可修改，僅可標 Superseded。

### ADR-009 事件型 case（Accepted，2026-09-23）

全文：`Customer/_index/decisions/ADR-009-event-record-case.md`。重點：

- 非客訴事件（開發、工具導入、版本定錨、技術債…）一事一案，id = `CASE-EVT_<EngCode>-YYYYMMDD-NNN`；不再開長期 in-progress 的量產維護列。
- 事件身分看 `CASE-EVT` 前綴或 tag `event-record`；**category 依性質填**（開發／改善多半 `E`），統計事件用 tag／前綴，不要用 category。
- 建案：`update_report.py add` → `archive_issue.py <row> --skeleton-only --event --expect-customer "<正式名>" --category <依性質> --desc "<短描述>" [--case-date YYYYMMDD]`
- 收尾（**不跑 close_case.py**，順序同 ADR-008 §5）：
  1. `update_report.py update --search … --desc … --brief "<產品面一句>" --status done --allow-direct-done`
  2. 補 `actual_completion`、`version_number="N/A（事件紀錄）"`（`update` 不寫這兩欄）
  3. `generate_report.py`（重排 row）
  4. `archive_issue.py <重排後 row> --skeleton-only --expect-customer …`（同步 status 與結案四欄）
  5. issue.md「客戶影響說明」寫事實
  6. `case_registry.py`（**不帶篩選**）→ `check_case_integrity.py` → `verify_close.py --case <id>`（事件型自動略過 release note／installer）
- ⚠ `case_registry.py` 帶任何篩選時只印不寫（2026-09-23 起）；在那之前的版本帶篩選會把全域看板蓋成篩選結果。
- ADR-008 第 2、6 點（量產維護列只收非客訴）由本 ADR 取代；HT160S／HT172 兩 repo 的週報指令需各自同步。

## 工具一覽

| 工具 | 功能 | 狀態 |
|---|---|---|
| `tools/archive_issue.py` | 歸檔 + 建 case + 寫 weekly notes（含 tags/component/severity/category/serial） | ✅ |
| `tools/case_registry.py` | 看板總覽（含 --tag --component --category --serial 過濾） | ✅ |
| `tools/build_lessons.py` | 跨 case 聚合（含類別統計、機台累犯分析） | ✅ |
| `tools/make_release_note.py` | Release Note 三版本（含 HTML，支援 --customer 防歧義） | ✅ |
| `tools/make_proposal.py` | 客戶需求提案表三版本（含 HTML，依 Steven customer-request 規格） | ✅ |
| `tools/close_case.py` | 結案總控：週報、Excel、issue.md、release note、case registry、installer 六步驟 | ✅ |
| `tools/update_report.py` | 週報 CRUD | ✅ |

## 客戶語言/代理商對照

維護於 `tools/make_release_note.py` 的 `CUSTOMER_PROFILE`：
- Greatek（超豐）：繁中，廠內直銷
- SPIL（SPIL-CN）：簡中，HPI-CHINA 代理
- 南茂科技 / 力成 / 確安：繁中，廠內直銷
- 新增客戶請編輯該字典

## HT9045 版本動態偵測

工具不寫死版本路徑，而是 glob `HT9011UC_Code_V*_Jimmy_*` 並取**字典序最大**（即最新版）。
未來升 V900/V901，無需改工具——資料夾命名遵守規範即可自動跟上。
舊版本資料夾為唯讀參考，不會被 release note 工具掃到。

## 對話慣例

| 格式 | 範例 | AI 行為 |
|---|---|---|
| 客戶+問題+檔案 | 「超豐 MA 指令無回應，log 在 xxx」 | 找 row → archive_issue |
| CASE-ID | 「CASE-20260422-001 加新 log xxx」 | 用 CASE-ID 解析 → archive_issue |
| row | 「row=2 加 xxx」 | 直接 archive_issue 2 xxx |
| 結案 / 出貨 / 已提供安裝包 | 「row=9 已修正並提供 V3.33.899.19，結案」 | close_case.py |

## `//AI()` 註解規範（HT9045 改 code 時）

JimmyChiu 在 HT9045 改 code 時遵循：
```cpp
//AI(AgentName) YYYYMMDD: 動作 + 原因
iRetryCount = 0;  //AI(ht9045-v899) 20260422: reset after MAIN_STATUS retry
```
- 規則同 `d:\HT9045\.github\instructions\ht9045-v899-ai-annotation.instructions.md`
- 此註解 = release note 自動素材，寫得好 → 三版本都有料

## 編碼規範

- 所有 Python 工具一律 UTF-8 讀寫（cp950 讀 HT9045 程式碼會 fallback 到 big5）
- 含中文 .md 檔禁用 PowerShell `Set-Content`，必須用 Python 直接寫
- weekly_data.json 為單一資料源（Single Source of Truth）

## 模板來源

模板（CSS、Logo）內嵌於本 skill / tools：
- `assets/honprec-logo.png` — 鴻勁精密商標（21KB）
- CSS_RED / CSS_BLUE 樣式內建於 `tools/make_release_note.py` 與 `tools/md_to_pdf.py`
- 正式產物路徑固定在 `Customer/<客戶>/<CASE>/04_release/`；回覆使用者時需貼出紅 / 藍實體檔案路徑

## 版本記錄

- 2026-04-22 v1：初版；CASE-ID 設計、archive_issue + case_registry 雙工具落地
- 2026-04-22 v2：補 --skeleton-only 模式；補建 4 缺 case；新增 make_release_note.py（三版本 + HTML 自包模板）；移除所有外部 Skill 依賴
- 2026-04-22 v3：**橫向聚合層**上線：issue.md 加 tags/component/severity；archive_issue / case_registry 同步支援；新增 tools/build_lessons.py（lessons-learned.md）；建 Customer/_index/decisions/ 並寫 3 份 ADR
- 2026-04-22 v3.1：借鑑代理商編碼，加 `category` (P/R/B/E/Q) + `machine_serial`（預設 Default）；build_lessons 加「類別統計」「機台累犯分析」；新增 ADR-004
- 2026-04-22 v3.2：新增 tools/make_proposal.py（依 Steven customer-request 規格，三版本 + HTML）；修復 find_case_dir 多客戶同名歧義 bug（make_release_note + make_proposal 加 --customer 參數 + 歧義保護）
- 2026-05-12 v3.3：補 `--code-folder` 必填情境警告（修舊版本但 workspace 已有更新版資料夾時 detect_active_version 會抓錯，需顯式鎖定）
- 2026-05-14 v3.4：新增 `tools/close_case.py` 結案總控；結案優先跑 6-step 工具並固定回報 installer 路徑與檔案狀態
- 2026-06-22 v3.5：**CASE-ID 代號化（ADR-007）**：新案 = `CASE-<EngCode>-YYYYMMDD-NNN`，代號取自 `MachineType.h` 官方 `CC_*`；新增 `tools/customer_codes.json`(217 筆) + `tools/customer_code_map.json`；10 支工具正則改相容版（非捕獲選填，舊案零異動）；新增 `customer_code()`/`case_id_from_dir()` 於 `_customers.py`；補 ADR-006/007 進索引
