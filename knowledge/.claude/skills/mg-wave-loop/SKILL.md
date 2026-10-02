---
name: mg-wave-loop
description: HT9045 MG 戰役（V899→V910 功能搬移）的自動波次政策。定義範圍權威、final-state diff 搬移法、95% 決策規則、殭屍查證協議、額度中斷協議、模型政策、硬邊界。Use when：執行 /mg-wave、規劃搬移波次、判斷某案能不能自動搬、處理搬移衝突與客戶碼隔離。關鍵字：MG-W, 899轉910, 功能搬移, 搬移波次, PORT_LEDGER, splice, port_check, mg899to910
---

# MG 波次迴圈政策（V899 → V910 功能搬移）

**最終目標**：weekly 案件紀錄中「在 V899 樹上有程式變更」的功能，全數落地到
`HT9011UC_Code_V3.33.910.0_20260716_Jimmy`，V910 其他客戶行為不變。

計畫全文：`docs/MG899TO910_CAMPAIGN_PLAN.md`（含使用者 20260826 全部裁定）。
總帳：`docs/MG_PORT_LEDGER.md`。最終決策清單：`docs/MG_FINAL_DECISIONS.md`。
DEVLOG＋🔖RESUME：`docs/MG_DEVLOG.md`（檔尾 RESUME 是冷啟動唯一起點）。

`pt-wave-loop` skill 的以下段落**全部適用**：五個已付代價的陷阱／「不准自行停下」三規則／
冷啟動協議／額度中斷實測表。

## 基準點（不變量）

- V899 端點：`bb69c60`（pristine）→ `b515ed5`（在製收斂）。搬移期間 **V899 唯讀**。
- V910 基線：`e06524a`（pristine 846 檔）。
- 工作量權威：`docs/mg_ai_matrix_missing.csv`（337 條 MISSING）；重跑
  `python tools/port_tools/ai_comment_matrix.py` 可隨時重算（收工驗收也用它）。

## 搬移方法（鐵律）

1. **Final-state diff，不重播歷史**：以「V899 最終狀態（b515ed5） vs V910 現狀」為準。
   同主題跨日叢集（例 Multi EP 0429–0512）合併一波；被撤回的中間態（rolled back）不搬。
2. **Splice 不重打字**：新增行一律從 V899 檔案以位元組級抽出插入（cp950 安全），
   對齊天然保留。禁止用 Edit/Write 工具直改兩棵 Big5 樹。
3. **EOL 跟隨目標檔**：V910 有 ~34 檔是 LF；插入行的行尾轉成目標檔主流行尾，
   其餘位元組不動。**絕不對 V910 路徑 git checkout**（autocrlf 會改寫行尾）。
4. AI 註解：`//AI(mg899to910) YYYYMMDD: 描述 [CASE-ID或日期叢集]`；
   來源 899 註解**原樣保留**（它是矩陣驗收的簽章）。
5. 每波 gate（缺一不可）：port_check 全綠 → 觸及檔 bcc32 -c -H- 全過 →
   主腦逐條開檔複驗（agent 論證比程式碼更常錯）→ commit。
6. **commit 後清 V910 樹 `*.mgbak`**：port_check 與 bcc_syntax 的基準都取自
   .mgbak（首次觸碰時建立），逐波清掉才能讓下一波的差異報告與「既有錯誤集合」
   基準是波前狀態，而非跨波累積。

## 決策規則（使用者 20260826 裁定）

- 信心 **≥95%** → 直接執行，LEDGER 記決策與理由。
- **<95%**（錨點消失、V910 有反向刻意修改、邏輯衝突無安全解）→ 寫入
  `MG_FINAL_DECISIONS.md`，**跳下一案不停**。
- 影響其他客戶 → 用該案客戶碼（`CUSTOMER_CODE`／`FUNC_CC_*`）只包行為變更點，
  預設路徑＝V910 原行為。
- D 類（安全關鍵）：忠實搬運且錨點乾淨＝可執行；需改寫＝進最終決策清單。
- `.dfm` 案直接完成不留 review（文字級套用、不重排）。

## 殭屍查證協議（使用者 20260826 裁定：先查證後處置）

心跳（60 分鐘 cron）發現無進度時，**四項全部**確認無活動才能宣告死亡：
1. 背景任務清單還有沒有它
2. `tasklist` 查 bcc32／make／python（build 不經 API，額度中斷時照跑）
3. 產出檔／log **mtime** 有沒有前進（檔案存在 ≠ 在跑）
4. `git status` 對帳（agent 常寫完檔才死，殺前先收成果）

任一項有活動 → 不動它，記錄，下輪再看。確認死亡 → 收殮（驗證已落地工作、
commit／LEDGER）→ 照 🔖RESUME 重派。

## 額度中斷（實測背書，見 pt-wave-loop 實測表）

- 中斷期間：OS 進程照跑；ScheduleWakeup 鏈會斷；**cron 心跳下一次自動回來**＝復原路徑。
- 前提 session 不關。session 死＝commit+LEDGER+RESUME 是全部狀態，
  重開 `/loop /mg-wave` 冷啟動。
- 每波收工順序：port_check→bcc32→commit→LEDGER→DEVLOG/RESUME→**立即下一波**。

## 模型政策（使用者 20260826 裁定）

主腦=Fable（選案/複驗/裁決/commit）；分析 agent=**Opus 5**（上限，禁派 Fable）；
機械 agent=**Sonnet 5**。effort 不限（opus high/xhigh、sonnet high）。
mutation agent 不與主腦併行改同檔（snapshot+md5 對帳）。

## 硬邊界（永不觸碰）

- V899 與其他版本樹唯讀；`EXE/`、`Obj/` 不覆寫；`system/`、`config/`、`CFG/` 不動。
- `.svn` 不搜尋不修改；`.dfm` 不重排。
- 絕不 `git checkout` 復原自己的編輯（用備份+md5）。
- 波內只用 bcc32 -c 隔離編譯；全量 build 只在 Phase 3 且先查無 BCB IDE 開著。
