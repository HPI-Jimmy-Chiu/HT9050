# 部門週報（Group Weekly Report）

## 適用時機
- 產生研五部門週報（彙總版、主管會議版）
- 多人資料合併為部門週報
- 輸出 `.md` + `.html` 雙格式

## 輸出路徑
- 主要：`<repo>\public\Docs\weekly\{YYYY}\{MM}\{YYYYMMDD}\`
- 備份：生成後 robocopy 至 `R:\研五_共用區\AI\Weekly Report\` 對應路徑

## 輸出格式
- **Markdown** `.md`：純文字 metadata 表格，**不放 Logo**
- **HTML** `.html`：使用 **honprec-blue-template**（主色 `#005a9e`），含 Logo（Base64 from reference）

## 版本類型

| 命名後綴 | 用途 |
|---|---|
| （無後綴）| 一般彙總 |
| `_彙總版` | 多人資料合併 |
| `_會議版` | 主管會議簡報格式|

## 命名規則
```
鴻勁_研五部週報_{民國年}-{月}-{日}.md/.html
鴻勁_研五部週報_{民國年}-{月}-{日}_彙總版.md/.html
鴻勁_研五部週報_{民國年}-{月}-{日}_會議版.md/.html
```

## 固定輸出結構
1. 部門標題
2. 週期 metadata
3. 各成員工作彙整（依人員分節）
4. 部門摘要（本週重點、下週行動、待決議事項）

## 固定表頭（10 欄）
```
| 序號 | 部門 | 客戶 | 事件/議題 | 行動/解決方案 |
| 負責人(主) | 負責人(協) | 預定完成日 | 實際完成日 | 備註 |
```

## 多人合併規則
1. 同一週多人資料整併至單一主表，不拆成多個表格
2. `負責人(主)` 欄標注個人姓名以維持可追溯性
3. 摘要需反映整部門整體進度，而非某一人的進度

## 模板
`../../templates/group-weekly-report-template/`
HTML 樣式詳見：`../../templates/honprec-blue-template/SKILL.md`
