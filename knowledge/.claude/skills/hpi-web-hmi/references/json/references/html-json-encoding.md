> 保存來源：`.claude/skills/ht9045-html-json/references/html-json-encoding.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
---
description: >
  HT9045 HTML 模擬版本（page/*.html、background.html／release.html／debug.html）與
  開站 JSON 資料（JSON/*.json）的編碼與技能優先規則。建立或修改這些檔案時自動生效。
  與 AGENTS.md「BCB6 原始碼維持 Big5(CP950)」規則不同，此範圍一律使用 UTF-8。
  觸發關鍵字：HTML version, HTML 模擬, background.html, release.html, debug.html,
  page/*.html, JSON, UTF-8, 編碼, ht9045-html-json, ht9045-html-version
applyTo: "**/page/**, **/JSON/**, **/background.html, **/release.html, **/debug.html"
---

# HTML 模擬版本／JSON 資料 — 編碼與技能優先規則

> **每次建立或修改 `D:\HT9045\page\*`、`D:\HT9045\JSON\*`、
> `background.html`／`release.html`／`debug.html` 時自動生效。**

## 編碼規則（覆蓋 AGENTS.md 的 Big5 規則）

- 這個範圍的檔案**不是 BCB6 編譯輸出**，一律使用 **UTF-8**（無 BOM）：
  - `D:\HT9045\page\*.html`／`*.js`／`*.css`
  - `D:\HT9045\background.html`／`release.html`／`debug.html`
  - `D:\HT9045\JSON\*.json`（含 `JSON\offline\*`、`JSON\js\*.js` 墊片）
- Python 腳本讀寫這些檔案一律用 `encoding='utf-8'`；**不要**用 `cp950`／`Big5`。
- 讀 **BCB6 原始碼**（`.cpp`/`.h`/`.dfm` 等）取資料轉成 JSON 時，來源檔仍用
  `encoding='cp950', errors='replace'`，只有**輸出的 JSON/HTML/JS 檔**才轉存成 UTF-8——
  兩套編碼不可在同一支轉換腳本中混淆方向。

## 技能優先順序

建立或修改 HTML 模擬版本／開站 JSON 資料時，**優先參照**（依此順序）：

1. [`ht9045-html-json`](../index.md) — 開站需要哪些 JSON、
   資料項為何、載入順序（四大分類：硬體設定檔／Config 檔／Setup 檔(Recipe)／生產記錄檔）
2. [`ht9045-html-version`](../../pages/index.md) — 頁面／元件轉 HTML
   規則、產生器、release 模式、離線 JSON 寫入（`json-writer.js`）

若牽涉 BCB6 原始碼的權威資料結構（`Gerneral.ini`／`config.ini`／Recipe `.Data`），
仍需交叉核對對應 skill（`ht9045-general-ini`／`ht9045-config`／`ht9045-recipe`）取得欄位定義，
但 **HTML/JSON 端「該轉什麼、放哪裡、什麼順序載入」以上述兩個 skill 為準**，
不要在別處另立規則。

<!-- preserved-content:end -->
