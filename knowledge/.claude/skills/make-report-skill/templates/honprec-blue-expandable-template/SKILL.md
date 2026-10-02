---
name: honprec-blue-expandable-template
description: >
  多層次可展開的鴻勁藍 HTML 報告模板。
  用於架構圖、Agent/Skill 索引、技術總覽、知識庫分類、版本對照等
  需要「分層展開 + 卡片互動」的視覺化 HTML 報告。
  與 honprec-blue-template（靜態 MD→HTML）不同，本模板需以 Python Generator 腳本生成。
applyTo: "**/*"
---

# honprec-blue-expandable-template — 多層次可展開 HTML 模板

## 適用報告類型

| 報告類型 | 範例 |
|----------|------|
| 軟體架構圖 / Agent-Skill 索引 | `Steven_AI軟體架構圖.html` |
| 多層次技術總覽 | 多工具生態索引、模組關係圖 |
| 知識庫索引（可展開 Chip） | Skill References 可互動索引 |
| 版本演進對照 | 歷版本記錄 + 差異描述 |

> 靜態文字型除錯/週報請用 `honprec-blue-template`。  
> 本模板適合「需要分層展開、Chip 互動、統計卡片、視覺同步圖」的場景。

---

## HTML 生成方式（Python Generator）

```
1. 複製 template.py 至目標工作路徑（建議 D:\AI_TempFile\）
2. 依需求修改 Data 區（替換 LAYER_1/2/3/4 資料）
3. 執行腳本：
   python D:\AI_TempFile\<your_script_name>.py
4. 輸出 HTML 至指定路徑（OUT_PATH）
```

> ⚡ **Logo**：腳本自動讀取 `D:\AI_TempFile\_logo_b64_arch.txt`（Base64）；  
> 若不存在請先執行：  
> `python -c "import base64; open('_logo_b64_arch.txt','w').write(base64.b64encode(open(r'C:\path\to\logo.png','rb').read()).decode())"`

---

## 色彩系統

| 用途 | 顏色代碼 | 說明 |
|------|----------|------|
| 主色（標題 / Badge / 按鈕）| `#003e7e` | 鴻勁深藍 |
| 輔助藍（連結 / h2）| `#005a9e` | 鴻勁標準藍 |
| 漸層終點 | `#0078d4` | Microsoft Blue |
| 背景 | `#f4f6fa` | 淺灰藍底 |
| 白板底 | `#ffffff` | 白色區塊 |
| 表格深色行 | `#f0f4fb` | 交替行色 |
| Hover 行 | `#dce8f8` | 表格 hover |
| 備注底色 | `#fffbf0` | 黃色備注框 |
| 備注邊框 | `#e8c840` | 金黃邊線 |

---

## 版面元件清單

### 1. Header（頁首）
- 漸層藍背景 `135deg #003e7e → #005a9e → #0078d4`
- Logo + 標題 + 右側 meta 資訊

### 2. Meta Block（元資料區）
- 白底 + 左側深藍邊條
- 表格式呈現：版本、日期、作者、說明

### 3. Stats Bar（統計卡片列）
- 多個 `.stat-card`，每個卡片有大數字 + 說明文字
- Hover 時微上浮

### 4. Section Header（層次標題）
- 圓形 Layer Badge（深藍底白字）
- 主標題 + 副標題

### 5. Tool Card（可展開卡片）
- `.tool-card > .tool-hd（標題列）+ .tool-body（展開內容）`
- 標題列有 Badge + 名稱 + meta + ＋/－按鈕
- 點擊標題列呼叫 `toggleCard(cid)`

### 6. Chip Grid（技能 Chip 格）
- `.chip-grid` 容納多個 `.chip`
- 每個 Chip 有類型色彩（flow/hw/config/comm/qa/merge/mgmt/core/art）
- 點擊呼叫 `chipClick(this, skillId)` → 展開 `.skill-panel`

### 7. Skill Panel（知識庫展開面板）
- `.skill-panel > .sp-inner`（動態注入 HTML）
- 包含：sp-head（標題+關閉）+ sp-refs-label + sp-refs（ref-pill 陣列）

### 8. Sync Visual（同步關係圖）
- `.sync-nodes`：多個 `.sync-node` + `.sync-arrows` 排列
- `.sync-hub-box`：中央 Hub + `.sync-item`（si-cc / si-msg / si-mode）

### 9. SVN Flow Diagram（SVN 流程圖）
- `.svn-flow`：來源框 + 箭頭 + commit box + report box

### 10. Standard Table（標準資料表）
- `.std` class：深藍 th + 交替行 + hover 行

---

## 類型色彩對照（Chip 類別）

| 類別代碼 | 中文 | 背景色 | 文字色 |
|---------|------|--------|--------|
| `flow` | 流程 | `#dbeafe` | `#1d4ed8` |
| `hw` | 硬體 | `#ffedd5` | `#9a3412` |
| `config` | 設定 | `#dcfce7` | `#166534` |
| `comm` | 通訊 | `#f3e8ff` | `#6b21a8` |
| `qa` | 品質 | `#fee2e2` | `#991b1b` |
| `merge` | 合併 | `#fef9c3` | `#854d0e` |
| `mgmt` | 管理 | `#e0e7ff` | `#3730a3` |
| `core` | 核心 | `#ccfbf1` | `#134e4a` |
| `art` | ART | `#fef3c7` | `#78350f` |

---

## 核心 JavaScript 函式

| 函式 | 說明 |
|------|------|
| `chipClick(btn, skillId)` | Chip 點擊展開 skill-panel；走訪 btn → chip-grid → container 找 .skill-panel |
| `closeSkillPanel(btn)` | 關閉最近的 .skill-panel；使用 btn.closest('.skill-panel') |
| `toggleCard(cid)` | 切換 tool-card 展開；嘗試 body-{cid} 或 body2-{cid} |
| `toggleCard2(cid)` | toggleCard 別名，供 LAYER 2 卡片使用 |

---

## 使用此模板的提示指令

### 從零建立架構圖

```
請用 honprec-blue-expandable-template 幫我產生一份 [主題] 架構圖 HTML，
包含以下層次：
  LAYER 1：[說明，例如：個人調度中樞 2 個 Agent]
  LAYER 2：[說明，例如：3 個專案 Agent，每個有多個 Sub-Skill Chip]
  LAYER 3：[說明，例如：4 個工具 Agent，含詳細路徑表]
  LAYER 4：[說明，例如：5 個全域 Skill，可展開 References]
輸出至 <repo>\public\Docs\other\2026\[檔名].html
```

### 更新現有架構圖

```
請參考 honprec-blue-expandable-template，
修改 D:\AI_TempFile\[gen_script.py]，
新增以下 LAYER X 資料：[描述新增內容]
重新產生 HTML。
```

### 僅調整樣式 / 新增 Chip 類別

```
請在 honprec-blue-expandable-template 的 CSS 中，
新增 chip 類別 [類別名]，顏色為背景 [#xxx] / 文字 [#yyy]，
並更新 CAT_LABELS 對應。
```

---

## 參考腳本

- **完整原型**：`D:\AI_TempFile\gen_arch_html_v2.py`  
  （包含 HT9045/GPIB9045/RS232Standard 完整架構圖資料）
- **模板基底**：[template.py](./template.py)  
  （空資料、可替換，包含所有 CSS/JS/Helper 函式）

---

## 注意事項

- 腳本以 **UTF-8 無 BOM** 儲存（Python 標準）
- HTML 字元需用 `eh()` 轉義；JS 字串用 `ej()` 轉義
- `body-{cid}` ID 命名必須與 `toggleCard(cid)` 一致
- Chip 的 `data-skill` 必須對應 `SKILL_DATA` 中的 key
- `OUT_PATH` 若含中文路徑，請用 unicode 字串（`u'...'`）
