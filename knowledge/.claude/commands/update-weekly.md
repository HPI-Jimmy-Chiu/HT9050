---
description: "快速更新週報進度。輸入自然語言描述工作進展，更新 weekly_data.json 與 Excel。"
argument-hint: "例如：甬矽 9016C OneByOne 已提供安裝包"
---

> 交給 **weekly-report** 子代理執行（Task 工具）。操作 Weekly_AI 工作區。

根據使用者輸入的自然語言，解析並更新週報。

使用者輸入：$ARGUMENTS

步驟：
1. 從輸入提取：客戶、機台型號、事件關鍵字、行動描述、日期。
2. 在 `d:\Work-jimmychiu\document\WeeklyReport\Weekly_AI\weekly_data.json` 搜尋匹配事件；模糊就向我確認是哪一筆。
3. 更新該 item：append action、設 `is_active_this_week=true`、自動推斷 status。
   **D 欄雙層制（必遵守）**：每筆 action 同時寫兩版——詳細版進 `--desc`（技術紀錄，留在 JSON），
   白話簡潔版進 `--brief`（主管看的 Excel D 欄；一句話講「客戶說什麼／做了什麼／結果」，
   禁檔名/行號/函式名/旗標名）。用 `update_report.py update --search ... --desc "..." --brief "..."`。
4. **確認後**重產 Excel：
   ```
   cd /d/Work-jimmychiu/document/WeeklyReport/Weekly_AI/tools && python generate_report.py
   ```
5. 簡述改了什麼、Excel 路徑。

## D 欄簡潔層守門員（20260901 起）

`--brief` 不再是「可省略、省略就用 desc」。寫入前會過 `tools/_brief_guard.py`：
單行顯示寬度需 ≤ 160，且不得含檔名／行號／函式名／編譯訊息等技術符號，
不合格會直接報錯並列出原因（逃生門 `--brief-force`，只印警告）。

收工前跑一次稽核：

```
python check_brief_quality.py
```

要事後改寫某筆簡潔層（不動詳細層）：

```
python fix_brief.py --row <N> --list
python fix_brief.py --row <N> --seq <S> --text "<一句白話>"
```
