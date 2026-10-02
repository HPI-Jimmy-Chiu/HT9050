---
description: "Use when: 新專案導入、舊案快速套用治理骨架。適用於 new project bootstrap、project onboarding、pack install。"
---

# 新專案導入

請用最小可用方式，為新專案建立可持續維護的治理骨架。

## 目標路徑

- 啟動文件：docs/ops/adr/
- 模板：docs/ops/templates/new-project-bootstrap.tmpl.md

## 執行步驟

1. 先釐清專案基本資料、參考來源、寫入範圍與共享設定。
2. 用 bootstrap 模板建立啟動文件。
3. 指出最小必需的 Harness Core 元件：L0、L1、L3、L4、L5。
4. 安排 OPS Pack 的最小安裝內容：
   - Daily Worklog
   - Weekly Review
   - Skill Registry
   - Safety Checklist
   - Write Boundary Policy
5. 產出第一週最低交付清單。

## HT9045 可參考的導入經驗

- 版本化 Agent 策略（通用 + 版本子 Agent）
- Skill 工廠化生成（/ht9045-skill-factory）
- 寫入邊界三分法（可寫唯一版本 / 唯讀其他版本 / 備份設定檔）
- 多站點配置差異管理（1x1, 1x2, 2x4, 2x4_16）

## 輸出要求

- 不追求一次到位。
- 優先讓專案具備寫入邊界、經驗留存與週期維護。
