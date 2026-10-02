---
description: "Use when: 執行每週治理盤點、維護 Harness、檢查邊界漂移。適用於 weekly review、治理巡檢、維護週報。"
---

# 執行每週治理盤點

請針對 HT9045 專案做一次每週治理審查，並更新對應文件。

## 目標路徑

- 週檢：docs/ops/weekly/YYYY-Www.md
- 模板：docs/ops/templates/weekly-harness-review.tmpl.md
- Skill 登錄：docs/ops/registers/skill-registry.md

## 執行步驟

1. 依週檢模板建立或更新本週審查檔。
2. 檢查 Agent、Skill、Instruction、Command/Prompt、Hook、Script、日誌是否有過時或漂移。
3. 檢查是否發生以下情況：
   - 寫入邊界被放寬（檢查 .github/ops/write-boundary-policy.json）
   - 備份規則被跳過（system/、config/、CFG/ 的 .ini/.csv/.dat）
   - description 失效導致 Skill 不易觸發
   - Command/Prompt 與實際工作流脫節
   - `.claude/skills/` 與 `.agents/skills/` 兩邊內容不一致
4. 若有需要，更新 skill-registry.md。
5. 產出下週優先動作清單。

## HT9045 重點巡檢項目

- 確認舊版目錄仍為唯讀（readonlyRoots 中的所有版本資料夾）
- 確認 AGENTS.md 禁改清單中的驅動定義檔未被觸及
- 確認 V899 AI 註解規範（//AI(AgentName) YYYYMMDD: 描述）有被遵守
- 確認 BCB6 編碼規範（Big5、無 C++11 語法）無漂移

## 輸出要求

- 先寫風險，再寫改善建議。
- 只列出需要行動的問題，不堆砌流水帳。
