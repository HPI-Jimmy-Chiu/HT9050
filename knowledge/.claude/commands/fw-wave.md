---
description: "HT9045 V906 DFM→WEB 戰役：執行一個完整波次（選標的 → 交付三件套之一 → 整併 → 驗收 → commit → 更新 RESUME）。搭配 /loop 可整晚自動連續推進。關鍵字：FW-W, dfm轉web, 表單波次, tag 接線, formview"
argument-hint: "[目標（表單名/FW-0/FW-1/auto）] [單波行數上限，預設 15000]"
---

先載入 **fw-wave-loop** 與 **pt-wave-loop** 兩個 skill（`Skill` 工具）取得完整政策，
再照下面執行。權威計畫書：`HT9011UC_Cpp_V3.33.906.0/docs/DFM2WEB_CAMPAIGN_PLAN.md`。
使用者輸入：$ARGUMENTS

工作目錄：`HT9011UC_Cpp_V3.33.906.0`（web 渲染件在 `D:\HT9045\web`，同屬本工作區可寫範圍）。

---

## 步驟 0 — `git status` 先跑

**第一件事，不是先讀 RESUME**（這條規則已四次撿到樹上未 commit 的在製工作）。
在製工作先收完再開新波。然後讀 `docs/DEVLOG.md` 檔尾 🔖RESUME 與計畫書 §4 佇列。

## 步驟 1 — 選標的

- `$1` 指名（表單名或 FW-0/FW-1/FW-2）→ 就做那個。
- `$1` 空或 `auto` → 照佇列：**FW-0 沒做完先做 FW-0**（web/ 入版控、BinCount 兇手、
  webprobe 收編）；再 FW-1 tag 接線；再 FW-2 產生器＋pilot；之後 FW-3 表單波照
  計畫書 §4 優先序，每波開工重評。
- 表單波先列 golden 函式清單（名稱、golden 行號、span），總行數 ≤ `$2`（預設 15000）。
- **撞到「需要 write path 才有意義」的表單 → 佇列，換下一個**（§7，不做也不問）。

## 步驟 2 — 交付

依波次型態：

- **tag 接線波**：主迴圈自做或派 1 個 agent。每個 tag：來源全域已翻譯且可量測地載入
  → liveness predicate 與 value getter 分開寫進 `WebBridgeTags.cpp`；
  null=未載入、""=刻意空白、0=真的 0；分母規則照 `test_wb_tags.cpp`。
- **表單邏輯波**：照 pt-wave 的翻譯 workflow（agent 只准新增/append 鏡射檔；
  Big5 cp950 讀、UTF-8 寫、EOL 逐檔保持；banner/GATE REGISTER；五個陷阱原文入 prompt；
  交付前 `-fsyntax-only` 自檢）。**追加 FW 規則**：widget 存取改 UI-state 欄位
  （命名=dfm leaf name）；ctor 只塞欄位；event handler 本體翻譯但不接線。
- **產生器/前端波**：主迴圈自做（emit_web.py 吃 ir_out 的 IR，不自己開 .dfm；
  formview.js 防禦式渲染，未知 widget 畫佔位並列清單）。

## 步驟 3 — 整併（主迴圈自做，不委派）

CMakeLists 落點用 `nm` 量；連結收斂逐符號問「body 所在 .cpp 在 CMakeLists 嗎」；
stub 退役逐行 assert；過期 gate 前提逐條複驗（前提死≠退役）；
**行為變更留到下一顆 commit**。

## 步驟 4 — 驗收 gate（兩級制 + web 專屬）

- 先判有無行為變更：零行為變更 → `g++ -E` 比對（剝 `\r` 與 line marker，
  每行差異必須是空行）＋該檔單獨 `-O3` 探雷＋一次 Debug build；
  有行為變更 → **全新 dir、Debug+Release 各一次、最後一次整併之後量**，
  失敗集合逐項相同且 ⊆ 常駐六項（config_db/IniFiles/ini_helpers/config_loaders/
  dfm2rc_idempotent/GA1_ReadGeneralIni）。
- **tag/web 波加 e2e**：`wb_publish --pump`（要 config 值則 `--with-config --dry`）+
  `wb_gateway` + `tools/webprobe`；**不得與 ctest 並行**（WB_TcpLink 撞埠）。
- 收工比對 `D:\HT9045\system` 550 檔 MD5+mtime（BinCount 修好前每波必查）。
- 超出驗收線 → 停、根因、不 commit；同根因紅兩次 → 換標的並記錄。

## 步驟 5 — commit

一顆 commit 一件事；訊息寫「量到什麼」含自己犯的錯與更正；
`git add` **逐檔點名**（兩 session 共用 repo，嚴禁寬 glob 與 `git checkout` 還原）。

## 步驟 6 — DEVLOG + RESUME

`docs/DEVLOG.md` 附加一節（內容、數字、坑、刻意沒做的事），
更新檔尾 🔖RESUME：下一步具體到檔名行號；標明哪些數字已驗證、哪些沒有。

## 步驟 7 — 回報並立即開下一波

- 本波交付（型態/檔數/行數或 tag 數）、兩組 ctest 數字＋失敗集合、
  e2e 探針結果、三軸完成度（附分母與單位）、下一個標的。
- **回合結束前不准閒著**：沒有背景工作在跑就開下一波；ScheduleWakeup 只當保險，
  另掛 cron 心跳（20 分鐘守衛式）。額度中斷協議照 fw-wave-loop skill。

---

## 停止條件（僅此四個）

安全關鍵（write path/互鎖/動真機）→ 佇列不做不問；同根因紅兩次 → 換標的；
額度耗盡 → 寫完 DEVLOG+RESUME 再停；FW-3 清空且 FW-W 未定 → 停等使用者。
