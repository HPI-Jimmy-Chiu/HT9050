# /mg-wave — 執行一個 MG 搬移波次（V899→V910）

先載入 `mg-wave-loop` skill（政策單一出處），然後照下列步驟做**一個完整波次**。
搭 `/loop /mg-wave` 可夜間自動連續推進。

## 冷啟動對帳（每次 invocation 必做，不憑記憶）

1. `git log --oneline -5` — 真正做到哪。
2. `git status --porcelain -- HT9011UC_Code_V3.33.910.0_20260716_Jimmy/` —
   有未收在製 → 先收完（port_check → bcc32 → commit），不疊新波。
3. 讀 `docs/MG_DEVLOG.md` 檔尾 🔖RESUME 與 `docs/MG_PORT_LEDGER.md`，
   從「下一步」開始。RESUME 與 git log 對不上時信 git log。

## 波次步驟

1. **選叢集**：LEDGER 裡 status=pending 者，優先序＝(a) 同主題跨日叢集合併
   （見 LEDGER 註記）(b) 日期早者先。宣告本波 MG-W{N} 與範圍。
2. **分析**（Opus agent，effort high 以上）：讀該叢集在 V899 的 final-state diff
   （`git diff bb69c60..b515ed5 -- <檔>` 過濾該叢集 hunk）→ 在 V910 找錨點 →
   判讀漂移與客戶碼隔離需求 → 產搬移方案（hunk 清單＋錨點＋gate 策略）。
3. **主腦複驗方案**：對照 `docs/mg_ai_matrix_missing.csv` 該叢集條目，
   逐 hunk 開檔對字面。95% 規則裁決（見 skill）。
4. **執行 splice**（腳本或 Sonnet agent）：`tools/port_tools/` 工具位元組級插入，
   絕不重打字。
5. **Gate**：`python tools/port_tools/port_check.py <觸及檔>` 全綠 →
   觸及檔 bcc32 -c -H- 全過（V910 樹規則＝BCB6/Big5/pre-C++11）。
6. **收工**：commit（訊息帶 MG-W{N} 與 CASE-ID）→ LEDGER 狀態更新 →
   `docs/MG_DEVLOG.md` 補記＋檔尾 🔖RESUME 改寫 → 武裝 ScheduleWakeup 保險 →
   **立即開下一波，不准閒著結束回合**。

## 中斷處理

- 錨點消失／衝突 <95% → 寫 `docs/MG_FINAL_DECISIONS.md`，該叢集標
  queued-decision，跳下一叢集。
- 額度中斷／殭屍 → 照 skill 的協議（先查證後處置）。
