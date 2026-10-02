# 除錯報告 / 使用報告 / 上線前風險掃描

## 適用時機

| 類型 | 用途 |
|---|---|
| **除錯報告** | 記錄 Bug 根本原因、修正方案、驗證結果 |
| **使用報告** | 記錄 Agent / Skill / 工具的功能與使用方式 |
| **上線前風險掃描** | 發版前 P1~P6 掃描，確認無阻擋上線風險 |
| **完整建置報告** | BCB6 Full Rebuild 結果，含 EXE 大小/時間戳 |
| **技術稽核報告** | 非標準掃描（如 LastSet 陣列越界稽核）|

## 輸出路徑
- 主要：`<repo>\public\Docs\weekly\{YYYY}\{MM}\{YYYYMMDD}\`
- 備份：生成後 robocopy 至 `R:\研五_共用區\AI\Weekly Report\` 對應路徑

## 輸出格式（可選）

使用者可在請求時指定輸出模式；未指定時預設 **MD+HTML**。

| 模式 | 指定方式 | 輸出物 |
|------|----------|--------|
| **MD only** | 請求中包含「只要 MD」、「MD only」、「不需要 HTML」 | `.md` 一份 |
| **MD+HTML**（預設）| 未特別指定，或包含「HTML」、「雙格式」 | `.md` + `.html` 各一份 |

- HTML：使用 **honprec-blue-template**（主色 `#005a9e`），含 Logo（Base64 from reference）
- Markdown：純文字 metadata 表格，**不放 Logo**

## 命名規則

| 類型 | 命名格式 |
|---|---|
| 除錯報告 | `{YYYYMMDD}_{作者}_debug_report_{主題}.md` |
| 使用報告 | `{YYYYMMDD}_{作者}_RD5軟體_{工具名稱}使用報告.md` |
| 上線前風險掃描 | `{YYYYMMDD}_{HHmm}_{作者}_PreReleaseRiskCheck_{專案}_{基線}_{主題}[_r{nn}].md` |
| 完整建置報告 | `{YYYYMMDD}_{作者}_full_build_report.md` |
| 技術稽核報告 | `{YYYYMMDD}_{作者}_{主題}.md` |

### 上線前風險掃描命名補充

- 為避免同一天對不同程式碼切片重複掃描時撞名，`上線前風險掃描` 一律加入 `{HHmm}`。
- `{專案}` 建議使用短名稱，例如 `HT1032`、`HT9045`、`GPIB9045`。
- `{基線}` 使用版本資料夾、Rev 或 revision，例如 `V3.32.859.0`、`Rev12.13.902.0`。
- `{主題}` 使用固定短主題，建議採 ASCII 英文，例如 `OffsetSaveReload`、`BlockTrayDivision`、`WholeProject`。
- 同一分鐘、同一主題重跑時，才追加 `[_r{nn}]`，例如 `_r02`。

範例：

- `20260423_1015_Steven_PreReleaseRiskCheck_HT1032_V3.32.859.0_OffsetSaveReload.md`
- `20260423_1410_Steven_PreReleaseRiskCheck_HT1032_V3.32.859.0_BlockTrayDivision_r02.md`

## 固定輸出結構

### 除錯報告（必含）
1. metadata 表格（日期、版本、影響範圍、開發者）
2. `> 概述 blockquote`
3. `## 1. 問題描述`（現象、影響範圍、錯誤訊息/Log）
4. `## 2. 根本原因分析`（直接原因、根本原因、程式碼位置）
5. `## 3. 修正方案`（修改前後 diff + 修改位置表）
6. `## 4. 驗證結果`（編譯 Exit Code、功能驗證）
7. `## 5. 後續建議`
8. `*Generated: ... | Developer: ... | ...*`（斜體 footer）

### 上線前風險掃描（必含）
1. metadata 表格（專案、版本、掃描範圍、日期、開發者）
2. `## 1. 掃描摘要（P1–P6）`（表格，每項 ✅ Clean 或 ⚠ 說明）
3. `## 2. 掃描細節`（依檔案分小節）
4. `## 3. 格式化與完整性檢查`
5. `## 4. 修正優先序`
6. `## 5. 編譯驗證`（Exit Code、Compile Errors、EXE 路徑）

### P1–P6 掃描項目

| 項目 | 說明 |
|---|---|
| P1 | 鏈式布林條件中的裸值（Bare Value）|
| P2 | Enum 直接當布林值使用 |
| P3 | 多階段迴圈條件變更風險 |
| P4 | Save-Reload 路徑一致性 |
| P5 | 條件式誤用變數（Copy-Paste Error）|
| P6 | 除以零風險（Division by Zero）|

## 模板
HTML 樣式詳見：`../../templates/honprec-blue-template/SKILL.md`
