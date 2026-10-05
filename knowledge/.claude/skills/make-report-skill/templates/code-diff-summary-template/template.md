# code diff summary template
# 由 generate_code_diff_summary.py 自動產生，或手動填寫
# 通常嵌入 ops daily/weekly worklog 中

## Code Diff Summary — {YYYY-MM-DD} — {HT9045 / GPIB9045}

異動檔案數：**{N}**

| 模組 | 檔案 | 風險等級 | 影響函式 |
|------|------|----------|----------|
| {模組} | `{filename.cpp}` | {⚠ High / ℹ Normal} | {函式1, 函式2, ...} |
| {模組} | `{filename.h}` | {⚠ High / ℹ Normal} | {函式1} |

> 掃描路徑：`{D:\HT9045 / D:\GPIB9045}`
> 掃描時間：`{YYYY-MM-DD HH:MM}`
> 掃描模式：mtime（比對修改時間 > {cutoff datetime}）

---

### 高風險項目說明（若有）

| 項目 | 函式 | 風險說明 |
|------|------|----------|
| `{filename.cpp}` | `{函式名}` | {說明：含 Motor/Sucker/Alarm 等關鍵字，需特別複核} |

---

### 腳本指令（重新產生）

```powershell
python "D:\HT9045\.claude\skills\make-report-skill\scripts\generate_code_diff_summary.py" `
  --project {HT9045|GPIB9045} `
  --range {daily|weekly} `
  --date {YYYY-MM-DD}
```
