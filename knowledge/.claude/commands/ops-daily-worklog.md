---
description: "Use when: 記錄每日工作日誌、工作交接、Session 收尾。適用於 daily worklog、日報（RD5 統一格式）、handover、今日進度、明日接續。"
---

# 更新每日工作日誌

請在 HT9045 專案中新增或更新當日工作日誌。

## 目標路徑

- 日誌：`<repo>/public/Docs/Daily/<EnglishName>/YYYYMMDD.md`（檔名一定是 YYYYMMDD.md；`<repo>`＝自己本機 RD5 入口網站 repo 的位置、`<EnglishName>`＝組織表上的英文名，見 `.claude/skills/make-report-skill/SKILL.md`「輸出位置設定」；寫完照那一節開分支＋MR。St01 記錄員暫時照舊寫 `docs/ops/daily/YYYY-MM-DD.md`，Steven 20260928「先不改，明天再說」）
- 格式：**20261003 起一律用 RD5 統一格式**（Steven 20261003 定案，全員適用）。說明 `.claude/skills/make-report-skill/references/ops-daily-worklog/ops-daily-worklog.md`；正本在入口網站 repo 的 `.claude/skills/rd5-daily-report/SKILL.md` §2～§3.5，不一致時以正本為準。
- 模板：`.claude/skills/make-report-skill/templates/ops-daily-worklog-template/template.md`（`docs/ops/templates/daily-worklog.tmpl.md` 還是舊的 8 段格式，不要用）

## 執行步驟

1. 先檢查今天的日誌檔是否存在；若不存在，依模板建立（一天一份）。
2. 蒐集當天內容：使用者口述 → 當天 git log / diff → 當天 ChangeLog → 對話紀錄。只寫實際做了、實際驗證過的事。
3. 照統一格式寫，第一行 `# YYYYMMDD 工作日誌 — <英文名>`、第二行 `客戶／機種：…　版本／分支：…`，接著**只有這四個 `##` 段落**，一頁內（最多 8 KB）：
   - `## 一句話`：2 行內、180 字以內，今天的主線與結果
   - `## 今日完成`：表格 項目｜客戶｜結果｜狀態｜連結；狀態只用 完成／待上機／待客戶／待裁決／進行中（另可用「取消」）
   - `## 卡點／需要協助`：表格 項目｜卡在哪｜從哪天開始｜需要誰；沒有就寫「無」
   - `## 明天接續`：最多 3 項
4. 根因推理、程式行號、測試數據、Skill／Command／Hook 異動細節放 ChangeLog 或報告，日報只寫一句結論加連結；不寫 session／視窗經過，不寫帳號密碼、token。
5. commit 之後、推送之前，在入口網站 clone 跑 `py tools/check_daily.py --changed` 自我檢查，列出的項目改好再推（不擋同事的報告：MR 照常自動合併）。
6. 如果任務會跨 Session 延續，同步更新 memories/repo/ACTIVE.md。

## HT9045 專案脈絡

- 目前版本：V3.33.899.0_20260323_Jimmy_20260422
- 版本目錄：HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/
- 共用設定：system/、config/、CFG/、IniData/
- Skill 位於 .claude/skills/（Copilot 鏡像在 .agents/skills/）；斜線指令位於 .claude/commands/

## 輸出要求

- 保持精簡（一頁內），但要讓明天的人能直接接手。
- 不寫空話，只寫已完成、已確認、待處理與卡點；延後的事從第二天起標「（第 N 天）」。
- 如果需要沉澱成 ADR、Skill 或規則，寫進 ChangeLog 或交接檔，不要在日報多開段落。
