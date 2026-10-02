# Ops Weekly Review（每週治理審查）

## 適用時機
- 每週末對 HT9045 / GPIB9045 兩專案進行治理資產巡檢
- 確認 Skill / Instruction / Prompt / Hook 是否仍有效

## 輸出路徑與命名

| 專案 | 目錄 | 檔名範例 |
|---|---|---|
| HT9045 | `D:\HT9045\docs\ops\weekly\` | `{YYYY}-W{ww}.md` |
| GPIB9045 | `D:\GPIB9045\docs\ops\weekly\` | `{YYYY}-W{ww}.md` |

## 輸出格式
- 純 **Markdown**（`.md`），**不放 Logo**，**不輸出 HTML**

## 6 段固定結構

| # | 段落 | 說明 |
|---|---|---|
| 1 | 本週摘要 | 主要變更、主要風險、主要收穫 |
| 2 | 治理資產巡檢 | Agent/Skill/Instruction/Prompt/Hook/記憶日誌 逐項檢查 |
| 3 | 邊界與安全檢查 | 超出白名單例外、備份規則、唯讀規則、安全審查點 |
| 4 | 過時與待整理項目 | Skill/Prompt/Instruction/文件各類型 |
| 5 | 下週行動 | 3 個行動項目 |
| 6 | 審查結論 | checklist（無重大風險 / 需立即處理 / 已更新文件）|

## metadata 表格（必填）

```markdown
| 項目 | 內容 |
|------|------|
| 週次 | YYYY-Www |
| 專案 | HT9045 / GPIB9045 / 兩者 |
| 審查者 | {人員} |
```

## 治理巡檢項目清單

每週至少檢查：
- Agent：寫入邊界是否仍正確
- Skill：description 是否仍可被觸發
- Instruction：applyTo 與關注點是否過寬
- Prompt：是否仍符合實際流程
- Hook / Script：是否仍可執行且規則未漂移
- 記憶 / 日誌：是否有持續更新

## AI 自動補充規則
1. 讀取當週 ops daily 日誌作為本週摘要素材
2. 掃描 `d:\.github\skills\` 最後修改時間，標記異動的 Skill
3. 若有 ADR 相關決策，連結至對應 ADR 文件

## 模板
`../../templates/ops-weekly-review-template/template.md`
