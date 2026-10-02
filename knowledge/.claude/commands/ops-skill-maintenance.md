---
description: "Use when: 維護 Skill 生態系、更新 description、盤點過時知識。適用於 skill maintenance、skill review、skill registry。"
---

# Skill 維護盤點

請針對 HT9045 專案的 Skill 生態系執行一次盤點與維護。

## 目標路徑

- Skill 登錄：docs/ops/registers/skill-registry.md
- Skill 目錄：.claude/skills/（Claude Code 實際載入位置；Copilot 鏡像在 .agents/skills/）

## 執行步驟

1. 掃描 .claude/skills/ 下所有 SKILL.md 的 description。
2. 檢查每個 Skill 的 description 是否包含足夠觸發詞。
3. 檢查是否有以下問題：
   - SKILL.md 超過 500 行
   - 多個無關主題混在一起
   - 長期未驗證（Last Verified 超過 30 天）
   - references/ 缺失
   - 已不再適用但仍被保留
   - `.claude/skills/` 與 `.agents/skills/` 兩邊內容漂移不一致
4. 更新 docs/ops/registers/skill-registry.md。
5. 對需要拆分、合併、淘汰或補資料的 Skill，列出後續動作。

## HT9045 Skill 分類參考

| 分類 | Skills |
|------|--------|
| 機構流程 | ht9045-inarm-flow, ht9045-outarm-flow, ht9045-index-flow, ht9045-shuttle-flow, ht9045-catchtray-flow |
| 深度邏輯 | ht9045-inarm-suck-logic, ht9045-contact-force |
| 通訊 | ht9045-secsgem, ht9045-autostart-flow, ht9045-lotinfo-flow |
| 版本 | ht9045-v899 |
| 工具 | bcb_build, cpp-pro, data-analysis, search-division, skill-creator |

## 輸出要求

- 以可執行建議為主。
- 明確標出 Active、Review、Split、Merge、Deprecated 狀態。
