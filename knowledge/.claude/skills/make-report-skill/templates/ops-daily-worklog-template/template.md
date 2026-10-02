# ops daily worklog template
# 使用方式：複製此模板，填入 {佔位符}，存至 <repo>\public\Docs\Daily\<EnglishName>\{YYYYMMDD}.md
#
# 統一路徑：<repo>\public\Docs\Daily\<EnglishName>\{YYYYMMDD}.md（適用全專案）

# Daily Worklog

| 項目 | 內容 |
|------|------|
| 日期 | {YYYYMMDD} |
| 專案 | {HT9045 / GPIB9045} |
| 版本 / 分支 | {版本號，例：V3.33.900.0_20260331} |
| 工作模式 | {開發 / 除錯 / 導入 / 維護} |

## 今日目標

- {目標 1}
- {目標 2}
- {目標 3}

## 今日完成

| 類別 | 內容 | 結果 |
|------|------|------|
| 程式碼 | {描述修改內容} | {完成 / 進行中 / 待確認} |
| 文件 | {描述文件更新} | {完成 / N/A} |
| 治具 / Harness | {Skill/Prompt/Instruction 更新} | {完成 / N/A} |
| 驗證 | {編譯驗證 / 功能驗證} | {Pass / Fail / 待確認} |

## 變更觸及範圍

| 類型 | 項目 |
|------|------|
| 模組 | {inarm / outarm / shuttle / index /等} |
| 檔案 | {filename.cpp, filename.h} |
| 設定 | {General.ini / setup.inf / 等，若有} |
| Skill / Prompt / Instruction / Hook | {若有} |

## 關鍵決策

| 主題 | 決策 | 原因 |
|------|------|------|
| {主題} | {決定做什麼} | {為什麼這樣決定} |

## 風險與阻塞

| 類型 | 說明 | 需要誰協助 |
|------|------|-------------|
| 風險 | {風險描述，無則填「N/A」} | {誰 / N/A} |
| 阻塞 | {阻塞描述，無則填「N/A」} | {誰 / N/A} |

## 經驗留存

- 今天確認有效的規則：{描述}
- 今天發現失效的假設：{描述}
- 今天值得沉澱為 Skill / Instruction / ADR 的內容：{描述}

## 明日接續點

1. {接續任務 1}
2. {接續任務 2}
3. {接續任務 3}

## 收尾檢查

- [ ] 如果有跨 Session 任務，已更新 `memories/repo/ACTIVE.md`
- [ ] 如果有重要治理決策，已新增或更新 ADR
- [ ] 如果有新知識，已評估是否要更新 Skill / Instruction / Prompt
