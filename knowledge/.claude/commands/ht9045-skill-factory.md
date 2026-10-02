---
description: "HT9045 Skill 工廠：輸入模組名稱和版本資料夾，自動深度分析程式碼並生成或更新對應 SKILL.md。Use when: 批量建立/更新 skill、模組分析、版本升級後 skill 同步。關鍵字：skill factory, 建立 skill, 更新 skill, 模組分析"
argument-hint: "[版本資料夾] [模組清單或 ALL] [create|update|force]"
---

# HT9045 Skill Factory

請依以下流程，針對指定版本的指定模組自動建立或更新 Skill。

使用者輸入：$ARGUMENTS

> 若未提供，請向使用者補問：
> - 版本資料夾：例如 HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422
> - 模組清單：例如 InArm, OutArm, Index, Shuttle, CatchTray（逗號分隔，或 ALL 表示全部）
> - 模式：create = 新建（不覆蓋既有）/ update = 更新既有 / force = 強制覆蓋

## 執行流程

對每個模組依序執行：

### 1. 掃描階段
- 用 Explore 子代理讀取該模組的所有 .cpp/.h 原始碼
- 提取：狀態機 task 變數（switch-case）、報警觸發條件、與其他模組的互鎖關係

### 2. 分析階段
- 整理出完整狀態轉移表（task 值 → 動作 → 下一個 task 值）
- 記錄所有 Alarm / Error 觸發點及條件
- 記錄硬體操作（馬達、氣缸、真空、感測器）

### 3. 生成階段
- 在 `.claude/skills/ht9045-{模組小寫}-flow/` 建立或更新 SKILL.md
- frontmatter 的 description 需包含完整關鍵字（函式名、task 變數名、報警名稱）
- body 包含：狀態機流程、報警條件表、互鎖關係、常見問題排查指引
- 若內容超過 500 行，拆分到 references/ 子目錄
- 若仍維護 Copilot，請同步鏡像到 `.agents/skills/`（兩邊內容須一致）

### 4. 驗證階段
- 確認 SKILL.md 格式正確（YAML frontmatter 語法）
- 列出變更摘要

## 模組對應檔案

| 模組 | 主要檔案模式 | 狀態機函式 |
|------|------------|-----------|
| InArm | ainarm9045*.cpp/h, ainarm_Search*.cpp/h | DoInArm_9045, iArmTask |
| OutArm | aoutarm9045*.cpp/h | DoOutArm_9045, OutArmTask |
| Index | atester*.cpp/h | DoTestHeadMotor, DoTestY |
| Shuttle | uShuttleThread.cpp, ShuttleMove.cpp | Do_Auto_SHT1/SHT2, AutoSHT1Task |
| CatchTray | acatchtray.cpp/h | DoCatchTray, CatchTrayTask |
| Automation | Automation/*.cpp/h | auto9045, automation |
| TempCtrl | TempCtrl/*.cpp/h, uHeaterThread.cpp | 溫控執行緒 |
| SECSGEM | SECSGEM/*.cpp/h | SECS/GEM 通訊 |

請開始分析。
