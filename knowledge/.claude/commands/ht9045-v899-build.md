---
description: "HT9045 V899 BCB6 編譯檢查模板。Use when: 驗證 V899 程式碼修改後是否可編譯、需要 build/rebuild、檢查 HT9045.bpr。關鍵字：V899 build, BCB6 build, HT9045.bpr, rebuild, compile check"
argument-hint: "[build|rebuild|clean] [變更焦點] [期望輸出]"
---

> 本指令的執行請交給 **ht9045-v899** 子代理（用 Task 工具啟動），由它負責 V899 版本的編譯檢查。

請針對固定版本目錄 `HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422` 執行編譯檢查。

使用者輸入：$ARGUMENTS

> 若未提供，預設 build 模式並向使用者確認：
> - 建構模式：build / rebuild / clean
> - 變更焦點：例如 ainarm9045_2x4_16.cpp 修改後
> - 期望輸出：例如 回報可編譯性 + 失敗點 + 最小回歸建議

## 固定目標

- 版本根目錄：[HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422](HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422)
- 專案檔：[HT9045.bpr](HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/HT9045.bpr)
- 參考技能：bcb_build（`.claude/skills/bcb_build/SKILL.md`）

## 執行要求

1. 先確認 BCB6 工具是否存在，包含 `bpr2mak.exe`、`make.exe`、`bcc32.exe`。
2. 只允許對 V899 根目錄的 `HT9045.bpr` 做 build 檢查，不得切到其他版本。
3. 若是 `build` 或 `rebuild`，先檢查必要輸出目錄與前置條件是否存在；缺少時要明確指出。
4. 若環境不足導致無法編譯，不可假設成功，必須直接列出阻塞原因。
5. 若編譯成功，補上與本次變更焦點相符的最小回歸建議。

## 輸出格式

1. 前置條件確認
2. 實際執行的建構模式與目標專案
3. Build 結果（成功 / 失敗 / 未執行）
4. 關鍵錯誤或警告摘要
5. 針對本次變更焦點的最小回歸建議
