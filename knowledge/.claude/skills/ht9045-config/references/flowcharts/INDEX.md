# Config 功能流程圖索引

> 將每個 IniConfig 功能的內部運作流程、跨模組互動、UI 顯示控制、客戶相依條件
> 以 Mermaid 圖示拆解，方便工程師與 AI Agent 快速理解。

## 撰寫規範

| 項目 | 規範 |
|------|------|
| **檔名** | `<Section>-<short-name>.md`，例如 `L43-power-follow.md` |
| **目錄** | 依群組字母分子目錄：`flowcharts/L/`、`flowcharts/N/` 等 |
| **編碼** | UTF-8 (no BOM) |
| **必備章節** | 1.功能概述、2.IniConfig 變數、3.UI 元件、4.控制流程、5.互動序列、6.啟用條件、7.相關函式、8.除錯建議、9.相關文件 |
| **流程圖類型** | flowchart / sequenceDiagram / stateDiagram / gantt（依功能性質選用，可多種混用） |
| **Caption / IniConfig** | 對應到 per-letter md 的「Caption」與「變數名」欄位 |

## 圖示類型選擇

| 場景 | 推薦類型 | 範例 |
|------|----------|------|
| 開機 / 啟用 / 條件分支 | `flowchart` | 設定載入流程 |
| Handler ↔ ATC / GPIB / FTP 互動 | `sequenceDiagram` | L43、N06、N14 |
| 狀態機（Normal / Retest / Error） | `stateDiagram-v2` | A10 ART、L11 ATC Protect |
| 時序型（升降溫、Soak Time） | `gantt` | L32 除霜、L43 升溫曲線 |

---

## 索引

### [A] Function

| 區段 | 主題 | 流程圖 |
|------|------|--------|
| _待建立_ | _e.g._ A10 ART 自動重測 | _e.g._ `A/A10-art.md` |

### [L] Temperature

| 區段 | 主題 | 流程圖 |
|------|------|--------|
| L43 | ATC Power Follow | [`L/L43-power-follow.md`](L/L43-power-follow.md) ← 範本 |

### [N] Network

| 區段 | 主題 | 流程圖 |
|------|------|--------|
| _待建立_ | _e.g._ N06 FTP Recipe | _e.g._ `N/N06-ftp-recipe.md` |

---

## 與 per-letter md 的整合

建議在 `config-fields-X.md` 的對應列加上連結：

```markdown
| L43 | Enable ATC Power Follow | `cbL43` | `bL43EnableATCPowerFollow` | bool | `bL43EnableATCPowerFollow` | -- | -- | Enable ATC Power Follow | [流程圖](flowcharts/L/L43-power-follow.md) |
```

或在 SKILL.md 的「群組分類總表」欄位加上「流程圖數」欄。
