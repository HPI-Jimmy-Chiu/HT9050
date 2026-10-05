---
name: honprec-red-template
description: >
  鴻勁精密外部客戶 HTML 報告模板（紅色系）。
  適用：軟體功能新增提案表、Release Note（客戶版）、代理商版文件。
  主色 #c0392b，HTML 轉換請執行 md_to_html.py --template red。
applyTo: "**/*"
---

# honprec-red-template（客戶用 HTML 模板）

## 適用報告類型
- 軟體功能新增提案表
- Release Note（廠內版 / 代理商版 / 客戶版）

## HTML 生成規則（強制）

> ⚡ **效能規則**：AI **禁止**直接讀取 Base64 參考檔（58KB）來輸出 HTML。
> 正確流程：AI 先將報告存為 `.md`，再執行腳本轉換（腳本自動讀 PNG，無需 AI 讀 Base64）：
>
> ```
> python "D:\HT9045\.claude\skills\make-report-skill\scripts\md_to_html.py" <output.md> --template red
> ```

## 色彩方案

| 元件 | 顏色值 | 用途 |
|------|--------|------|
| 主題色 | `#c0392b` | 標題、邊框、h2 背景、强調 |
| 背景色 | `#f4f6f9` | 頁面背景 |
| 表格頭 | `#f4f4f4` | th 背景 |
| 邊框色 | `#e0e0e0` | 表格邊線 |
| 文字主色 | `#2c2c2c` | 正文 |
| 輔助色 | `#2980b9` | `.node` 節點 |
| 指示色 | `#27ae60` | `.time` 時間/狀態 |
| 指令色 | `#8e44ad` | `.cmd` 命令 |

## 尺寸規範

| 項目 | 值 |
|------|-----|
| 頁面最大寬度 | 900px |
| 內外邊距 | 32px (上下) / 24px (左右) |
| 表格間距 | 9px / 14px |
| header 左邊框 | 6px solid #c0392b |
| flow-box 左邊框 | 4px solid #c0392b |
| 圓角 | 4px – 6px |
| 字體 | "微軟正黑體", "Calibri", sans-serif |

## CSS 完整樣式

```css
* { box-sizing: border-box; margin: 0; padding: 0; }
body {
  font-family: "微軟正黑體", "Calibri", sans-serif;
  background: #f4f6f9;
  padding: 32px 24px;
  color: #2c2c2c;
  line-height: 1.7;
}
.page { max-width: 900px; margin: 0 auto; }
header {
  display: flex;
  align-items: center;
  gap: 18px;
  border-left: 6px solid #c0392b;
  padding: 14px 18px;
  margin-bottom: 28px;
  background: #fff;
  border-radius: 0 6px 6px 0;
  box-shadow: 0 1px 4px rgba(0,0,0,.08);
}
header img { height: 56px; object-fit: contain; }
header h1 { font-size: 20px; color: #c0392b; margin-bottom: 4px; }
header p  { font-size: 12px; color: #888; }
h2 {
  font-size: 15px;
  background: #c0392b;
  color: #fff;
  padding: 6px 14px;
  border-radius: 4px;
  margin: 28px 0 12px 0;
}
table {
  width: 100%;
  border-collapse: collapse;
  background: #fff;
  border-radius: 6px;
  overflow: hidden;
  box-shadow: 0 1px 4px rgba(0,0,0,.06);
  margin-bottom: 8px;
}
th, td {
  border: 1px solid #e0e0e0;
  padding: 9px 14px;
  font-size: 14px;
  vertical-align: top;
}
th {
  background: #f4f4f4;
  font-weight: bold;
  width: 18%;
  white-space: nowrap;
  color: #444;
}
.flow-box {
  background: #fff;
  border: 1px solid #ddd;
  border-left: 4px solid #c0392b;
  padding: 16px 20px;
  font-family: Consolas, "Courier New", monospace;
  font-size: 14px;
  line-height: 2.2;
  border-radius: 4px;
  margin: 12px 0 16px 0;
}
.arr  { color: #c0392b; font-weight: bold; }
.node { color: #2980b9; font-weight: bold; }
.time { color: #27ae60; font-style: italic; }
.cmd  { color: #8e44ad; font-weight: bold; }
footer {
  margin-top: 36px;
  font-size: 11px;
  color: #bbb;
  border-top: 1px solid #ddd;
  padding-top: 10px;
  text-align: center;
}
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
<div class="page">
  <header>
    <img src="data:image/png;base64,{BASE64_FROM_REFERENCE}" alt="鴻勁精密 Logo">
    <div>
      <h1>{報告標題}</h1>
      <p>鴻勁精密股份有限公司 HONPREC INC.</p>
    </div>
  </header>

  <h2>提案資訊</h2>
  <table><!-- 參數表 --></table>

  <h2>修改摘要</h2>
  <p><!-- 簡略說明 --></p>

  <h2>技術流程</h2>
  <div class="flow-box"><!-- 程式碼或邏輯流程 --></div>

  <h2>詳細描述</h2>
  <p><!-- 完整說明 --></p>

  <footer><!-- 生成資訊 --></footer>
</div>
</body>
</html>
```

## 注意事項
- header `<p>` 副標題固定為 `鴻勁精密股份有限公司 HONPREC INC.`，不可改為文件描述
- Markdown（`.md`）版本**不放 Logo**，純文字 metadata 表格
