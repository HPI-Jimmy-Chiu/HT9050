# 軟體異常修正紀錄表（Bug Fix Report）

## 適用時機

- 客戶回報軟體問題，HonPrec 提供根本原因分析與修正方案
- 與「軟體功能新增提案表」的差別：此類型針對 **既有功能錯誤修正**，並非功能新增請求
- 含程式碼分析、根本原因追溯、修改建議（廠內版）

## 輸出路徑

- 主要：`<repo>\public\Docs\customers\{代理商}\{客戶代碼}_{客戶名稱}\proposals\{YYYY}\`
- 軟體異常修正紀錄表視同提案報告，歸檔於 `proposals\{YYYY}`；來源檔（.msg/.pptx/.doc/.pdf/.png 等）可留於客戶資料夾根層以追蹤進行中的 Bug。

## 輸出格式

| 報告格式 | Logo | 說明 |
|---------|------|------|
| **MD（廠內版）** | **不放 Logo** | 純文字 metadata 表格 |
| **HTML（代理商 / 客戶版）** | 執行 `md_to_html.py` 腳本，自動讀取 PNG | 使用 `honprec-red-template` |

> ⚠️ Markdown 報告嚴禁插入 `![Logo](...)` — Logo 只由 `md_to_html.py` 在 HTML 轉換時自動植入。

## 命名規則

### 標準格式

```
{YYYYMMDD}_軟體異常修正紀錄表_{專案名稱}_{主題}_廠內版.md          ← 廠內版
{YYYYMMDD}_軟體異常修正紀錄表_{專案名稱}_{主題}_客戶版.md          ← 客戶版 MD
{YYYYMMDD}_軟體異常修正紀錄表_{專案名稱}_{主題}_客戶版.html        ← 客戶版 HTML
```

- `{版本別}` 為固定字樣：`廠內版` / `代理商版` / `客戶版`。
- `{YYYYMMDD}` — 報告製作日（8 碼，例：`20260504`）
- `{專案名稱}` — 機型 / 專案識別（例：`HT9045`、`HT9046LS`）
- `{主題}` — 簡短 ASCII 主題，底線分隔，例：`IndexPositionError_Z1UpZ2Down1`、`AutoCleanCount`

### 範例

```
20260504_軟體異常修正紀錄表_HT9045_IndexPositionError_Z1UpZ2Down1_廠內版.md
20260504_軟體異常修正紀錄表_HT9045_IndexPositionError_Z1UpZ2Down1_客戶版.html
20260427_軟體異常修正紀錄表_HT9045_AutoCleanCount_廠內版.md
20260428_軟體異常修正紀錄表_HT9045_AutoClean_InArm_Pickup_Collision_廠內版.md
```

## 報告結構（必填）

```markdown
# 軟體異常修正紀錄表 / Software Bug Fix Report

## Basic Information

| 項目 | 內容 |
|------|------|
| 問題編號 / Issue No. | #P{YYMMDD}-{客戶代碼}-H9L-{NN} |
| Equipment Model | HT-9xxx（平台型號）|
| Issue Version | V{X.XX.XXX.X}（問題影響版本）|
| Last Known Good Version | V{X.XX.XXX} |
| Report Source | ■ Customer / ☐ Field Service |
| Customer | {客戶代碼}（代理商：TeraTech Korea）|
| Report Date | YYYY/MM/DD |
| Author | {作者} |
| Issue Date | YYYY/MM/DD |

## Issue Description
### 客戶現象
### 影響範圍

## Root Cause Analysis
### Bug 位置
### 問題程式碼（code block）
### 根本原因

## 修改建議 / Proposed Fix
（可含多個方案 A/B/C，說明優缺點）

## 暫時性 Workaround（可選）

## Modified Files（預計）

| File | Lines | Description |
|------|-------|-------------|

## Build Verification
- 待修改後執行 BCB6 full rebuild 驗證

## Acceptance

| 項目 | 狀態 |
|------|------|
| HonPrec | 待補 |
| Author | 待補 |
| Approval | 待補 |
```

## 問題編號格式

與 `customer-request` 提案表使用相同格式，見 [issue-number-format](../issue-number-format/issue-number-format.md)：

```
#P{YYMMDD}-{客戶代碼}-{機型縮寫}-{NN}
例：#P260504-SCK-H9L-01
```

## Template

- 廠內 MD 版：純文字，**不使用 template**
- 客戶 HTML 版：使用 `honprec-red-template`（主色 `#c0392b`）

HTML 轉換指令：
```
python "D:\HT9045\.claude\skills\make-report-skill\scripts\md_to_html.py" <output.md> --template red
```

## 相關參考

- 問題編號格式：[`../issue-number-format/issue-number-format.md`](../issue-number-format/issue-number-format.md)
- 全域問題索引：`<repo>\public\Docs\customers\issue-index.md`（20260929 之前的在 `D:\docs\customers\issue-index.md`，內容相同）
- 提案表（功能新增）：[`../customer-request/customer-request.md`](../customer-request/customer-request.md)
