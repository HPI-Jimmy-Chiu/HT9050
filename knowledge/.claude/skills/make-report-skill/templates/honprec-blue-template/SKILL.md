---
name: honprec-blue-template
description: >
  鴻勁精密內部 HTML 報告模板（藍色系）。
  適用：使用報告、除錯報告、上線前風險掃描、完整建置報告、技術稽核報告、部門週報 HTML 版。
  主色 #005a9e，HTML 轉換請執行 md_to_html.py --template blue。
applyTo: "**/*"
---

# honprec-blue-template（內部用 HTML 模板）

## 適用報告類型
- 使用報告（Agent / Skill / 工具）
- 除錯報告（debug report）
- 上線前風險掃描（pre-release risk check）
- 完整建置報告（full build report）
- 技術稽核報告
- 部門週報 HTML 版

## HTML 生成規則（強制）

> ⚡ **效能規則**：AI **禁止**直接讀取 Base64 參考檔（58KB）來輸出 HTML。
> 正確流程：AI 先將報告存為 `.md`，再執行腳本轉換（腳本自動讀 PNG，無需 AI 讀 Base64）：
>
> ```
> python "D:\HT9045\.claude\skills\make-report-skill\scripts\md_to_html.py" <output.md> --template blue
> ```

## 色彩方案

| 元件 | 顏色值 | 用途 |
|------|--------|------|
| 主題色（強） | `#003e7e` | th 背景、h1 底線 |
| 主題色（輕） | `#005a9e` | h2 左邊框、meta 強調文字 |
| 背景色 | `#f5f5f5` | .meta 背景 |
| 表格偶數列 | `#f0f4fb` | tr:nth-child(even) |
| 表格 hover | `#dce8f8` | tr:hover |
| 摘要底色 | `#fffbf0` | .summary 背景 |
| 摘要邊框 | `#e8c840` | .summary border |
| 頁腳文字 | `#999` | footer |
| 正文 | `#222` | body |

## 尺寸規範

| 項目 | 值 |
|------|-----|
| 主字體 | "Microsoft JhengHei", Arial, sans-serif |
| body 字體大小 | 13px |
| body margin | 24px 40px |
| Logo max-height | 56px |
| h1 | 20px, color #c00, border-bottom 2px solid #c00 |
| h2 | 15px, color #005a9e, border-left 4px solid #005a9e, padding-left 8px |
| h3 | 13px, color #333 |
| 表格 th | 5px 7px padding, 白字 |
| 表格 td | 4px 7px padding |

## CSS 完整樣式

```css
body {
  font-family: "Microsoft JhengHei", Arial, sans-serif;
  font-size: 13px;
  margin: 24px 40px;
  color: #222;
}
img.logo { max-height: 56px; margin-bottom: 10px; }
h1 { font-size: 20px; color: #c00; border-bottom: 2px solid #c00;
     padding-bottom: 4px; margin-bottom: 12px; }
h2 { font-size: 15px; color: #005a9e; border-left: 4px solid #005a9e;
     padding-left: 8px; margin-top: 22px; }
h3 { font-size: 13px; color: #333; margin-top: 14px; }
.meta {
  background: #f5f5f5;
  border: 1px solid #ddd;
  padding: 8px 14px;
  border-radius: 4px;
  margin-bottom: 16px;
  line-height: 1.8;
}
.meta strong { color: #005a9e; }
table { border-collapse: collapse; width: 100%; margin-top: 8px; font-size: 12px; }
th { background: #003e7e; color: #fff; padding: 5px 7px;
     text-align: center; white-space: nowrap; }
td { border: 1px solid #bbb; padding: 4px 7px; vertical-align: top; }
tr:nth-child(even) td { background: #f0f4fb; }
tr:hover td { background: #dce8f8; }
.summary {
  background: #fffbf0;
  border: 1px solid #e8c840;
  padding: 10px 16px;
  border-radius: 4px;
  margin-top: 8px;
  line-height: 1.7;
}
ul { margin: 4px 0 4px 18px; padding: 0; }
li { margin-bottom: 3px; }
footer { margin-top: 32px; color: #999; font-size: 11px;
         border-top: 1px solid #ddd; padding-top: 6px; }
```

## HTML 骨架

```html
<!DOCTYPE html>
<html lang="zh-TW">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>{標題}</title>
  <style>/* 上方 CSS */</style>
</head>
<body>
  <div style="padding: 12px 24px 0 24px;">
    <img class="logo" alt="鴻勁精密 Logo" src="data:image/png;base64,{BASE64_FROM_REFERENCE}" />
  </div>

  <h1>{報告標題}</h1>

  <div class="meta">
    <strong>人員</strong>：{人員}　
    <strong>部門</strong>：{部門}　
    <strong>日期</strong>：{YYYY-MM-DD}　
    <strong>報告類型</strong>：{類型}
  </div>

  <h2>{章節標題}</h2>
  <!-- 表格或內容 -->

  <div class="summary">
    <!-- 摘要文字 -->
  </div>

  <footer>
    Generated: {YYYY-MM-DD} | Developer: {作者} | {主題}
  </footer>
</body>
</html>
```

## 模板 A：使用報告 HTML 結構
1. Logo + 標題
2. .meta 基本資訊（人員、工具名稱、版本、依據文件）
3. `<h2>功能範圍</h2>` — 表格
4. `<h2>使用流程</h2>` — 有序清單
5. `<h2>使用範例</h2>` — code block
6. `<h2>注意事項與風險</h2>` — 條列
7. footer

## 模板 B：除錯報告 HTML 結構
1. Logo + 標題
2. .meta（問題標題、影響版本、開發者、環境）
3. `<h2>問題描述</h2>`（現象、影響範圍、錯誤訊息）
4. `<h2>根本原因分析</h2>` — 表格（直接原因/根本原因/程式碼位置）
5. `<h2>修正方案</h2>` — code diff + 修改位置表
6. `<h2>驗證結果</h2>` — 表格
7. `<h2>後續建議</h2>` — 條列
8. footer

## 模板 C：上線前風險掃描 HTML 結構
1. Logo + 標題
2. .meta（專案、版本、掃描範圍、日期、開發者）
3. `<h2>掃描摘要（P1–P6）</h2>` — 表格，✅ Clean 或 ⚠ 說明
4. `<h2>掃描細節</h2>` — 依檔案分小節
5. `<h2>格式化與完整性檢查</h2>` — 表格
6. `<h2>修正優先序</h2>` — 表格
7. `<h2>編譯驗證</h2>` — 表格（Exit Code、EXE 路徑/時間戳）
8. footer

## 注意事項
- Markdown（`.md`）版本**不放 Logo**，改為純文字 metadata 表格
