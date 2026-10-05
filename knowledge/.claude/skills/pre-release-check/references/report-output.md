# 報告產出（兩階段）

> 統一依 [`make-report-skill`](../../make-report-skill/SKILL.md) 規格產出（類型：除錯報告 / 使用報告 / 上線前掃描）。

---

## 階段一：MD 草稿（Track B，與編譯平行）

| 項目 | 規格 |
|------|------|
| 報告類型 | 上線前掃描（除錯報告） |
| Template | 不套用（純 Markdown） |
| 格式 | **MD 僅** |
| 輸出路徑 | `<入口網站 repo>\public\Docs\PreReleaseCheck\{YYYY}\` |
| 命名 | `{YYYYMMDD}_{HHmm}_{作者}_PreReleaseRiskCheck_{專案}_{基線}_{主題}[_r{nn}]_draft.md` |

### 草稿區段（Build 驗證結果欄位留空）

1. **執行摘要**：掃描範圍、發現總數（Critical / High / Low）
2. **P1–P9 風險發現表**：File \| Line \| Pattern \| Severity \| Description \| Suggested Fix
3. **P6 除法安全替換統計**（若有）：候選數、已保護排除數、實際替換數、修改檔案數
4. **格式化問題彙整**（F1–F11）：按規則分組
5. **Build 驗證結果**：【待補填 — 編譯完成後更新】

### sub-agent prompt 範本

```
你是技術報告撰寫專家。
請依據以下掃描結果，產生 Pre-Release Risk Check 的 Markdown 草稿報告。

【掃描結果】
<貼入 P1–P9 + F1–F11 + DFM 的 findings 彙整>

【輸出格式】
包含以下區段（Build 驗證結果留空，待編譯完成後補填）：
1. 執行摘要（掃描範圍、發現總數 Critical/High/Low）
2. P1–P9 風險發現表（File | Line | Pattern | Severity | Description | Suggested Fix）
3. P6 除法安全替換統計（若有）
4. 格式化問題彙整（F1–F11，按規則分組）
5. Build 驗證結果：【待補填 — 編譯完成後更新】

【命名規則】
{YYYYMMDD}_{HHmm}_{作者}_PreReleaseRiskCheck_{專案}_{基線}_{主題}[_r{nn}].md

【輸出路徑】
<入口網站 repo>\public\Docs\PreReleaseCheck\{YYYY}\
```

> ⚠️ **草稿完成後，等待使用者確認內容無誤**，再進行階段二。

---

## 階段二：完整 MD + HTML（Track A + 使用者確認 均完成後）

| 項目 | 規格 |
|------|------|
| 報告類型 | 上線前掃描（除錯報告） |
| Template | `honprec-blue-template` |
| 格式 | **MD + HTML** |
| 輸出路徑 | `<入口網站 repo>\public\Docs\PreReleaseCheck\{YYYY}\` |
| 命名 | `{YYYYMMDD}_{HHmm}_{作者}_PreReleaseRiskCheck_{專案}_{基線}_{主題}[_r{nn}][_v{n}].md/html` |

### 操作

1. 將 Build 驗證結果（Exit Code、Error 數、Warning 數）填入草稿第 5 區段。
2. 移除 `_draft` 後綴。
3. 轉換 HTML：

```powershell
python "D:\HT9045\.claude\skills\make-report-skill\scripts\md_to_html.py" <output.md> --template blue
```

---

## 命名欄位說明

| 欄位 | 說明 | 範例 |
|------|------|------|
| `{專案}` | 專案代號 | `HT1032`、`HT9045`、`GPIB9045` |
| `{基線}` | 版本資料夾、Rev 或 revision | `V3.32.859.0`、`Rev12.13.902.0` |
| `{主題}` | 短主題（建議 ASCII 英文） | `OffsetSaveReload`、`BlockTrayDivision`、`WholeProject` |
| `[_r{nn}]` | 同分鐘同主題重跑時追加 | `_r02` |
| `[_v{n}]` | 修訂版本（階段二後續修訂） | `_v2` |

### 範例

- 草稿：`20260423_1015_Steven_PreReleaseRiskCheck_HT1032_V3.32.859.0_OffsetSaveReload_draft.md`
- 完整：`20260423_1015_Steven_PreReleaseRiskCheck_HT1032_V3.32.859.0_OffsetSaveReload.md` + `.html`
- 重跑：`20260423_1410_Steven_PreReleaseRiskCheck_HT1032_V3.32.859.0_BlockTrayDivision_r02.md`

---

*最後更新：2026-04-24*
