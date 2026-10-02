# Ops Daily Worklog（每日工作紀錄）

## 適用時機
- 每日記錄 HT9045 / GPIB9045 開發工作
- 作為週報、除錯報告的原始素材來源

## 輸出路徑與命名

| 目錄 | 檔名格式 |
|---|---|
| `<repo>\public\Docs\Daily\<EnglishName>\` | `{YYYYMMDD}.md` |

> 所有專案（HT9045 / GPIB9045 / RS232Standard）共用同一目錄，不再依專案分目錄。

## 輸出格式
- 純 **Markdown**（`.md`），**不放 Logo**，**不輸出 HTML**

## 8 段固定結構

| # | 段落 | 說明 |
|---|---|---|
| 1 | 今日目標 | 計畫完成的事項（項目清單）|
| 2 | 今日完成 | 程式碼/文件/治具/驗證 結果表格 |
| 3 | 變更觸及範圍 | 模組/檔案/設定/Skill/Prompt/Instruction/Hook |
| 4 | 關鍵決策 | 主題、決策內容、原因 |
| 5 | 風險與阻塞 | 風險說明、需要誰協助 |
| 6 | 經驗留存 | 確認有效規則、失效假設、值得沉澱的知識 |
| 7 | 明日接續點 | 隔天要繼續的 3 個項目 |
| 8 | 收尾檢查 | checklist（ACTIVE.md、ADR、Skill 更新）|

## metadata 表格（必填）

```markdown
| 項目 | 內容 |
|------|------|
| 日期 | YYYY-MM-DD |
| 專案 | HT9045 / GPIB9045 |
| 版本 / 分支 | {版本號} |
| 工作模式 | 開發 / 除錯 / 導入 / 維護 |
```

## AI 自動補充規則
1. 若使用者未提供完整工作內容，主動讀取當日對應 ops 檔案（若存在）作補充
2. 若 ops 檔案不存在，依 VS Code 工作紀錄或對話內容填入
3. 「今日完成」中「結果」欄優先填入：`完成` / `進行中` / `待確認` / `取消`
4. 「收尾檢查」若有跨 Session 任務，更新 `memories/repo/ACTIVE.md`

## 模板
`../../templates/ops-daily-worklog-template/template.md`
