# 其他報告（Other Report）

## 適用時機
- 上述分類無法覆蓋的報告需求
- 代理人使用報告
- SKILL 應用說明
- 日報（個人/主管版）
- 代理商客戶服務出差報告

## 輸出路徑
- 工程報告：`<repo>\public\Docs\weekly\{YYYY}\{MM}\{YYYYMMDD}\`
- 備份：生成後 robocopy 至 `R:\研五_共用區\AI\Weekly Report\`

## 輸出格式
- 視情況選擇 `honprec-blue-template`（HTML）或純 Markdown
- Markdown 版本：**不放 Logo**，純文字 metadata 表格

## 命名規則

| 類型 | 命名格式 |
|---|---|
| 代理人使用報告 | `{YYYYMMDD}_{人員}_RD5軟體_{代理名稱}代理使用報告.md` |
| SKILL 應用說明 | `{YYYYMMDD}_{人員}_RD5軟體_{SKILL名稱}_SKILL應用說明.md` |
| 個人日報 | `{YYYYMMDD}_{人員}_RD5_個人日報.md` |
| 日報（精簡版） | `{YYYYMMDD}_{人員}_RD5_個人日報_精簡版.md` |
| 日報（軟體格式） | `{YYYYMMDD}_{人員}_RD5軟體_daily_report.md` |

## 通用 Markdown 骨架
```markdown
# {報告標題}

| 項目 | 內容 |
|------|------|
| 報告日期 | {YYYY-MM-DD} |
| 報告類型 | {類型} |
| 工具/主題 | {名稱} |
| 版本 | {X.Y.Z} |
| 維護者 | {人員} |

> {一到兩句摘要}

---

## 1. {內容章節}

---

*Generated: {YYYY-MM-DD} | Developer: {作者} | {主題}*
```

## 模板
HTML 樣式詳見：`../../templates/honprec-blue-template/SKILL.md`
