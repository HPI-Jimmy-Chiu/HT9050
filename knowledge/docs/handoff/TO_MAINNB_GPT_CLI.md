# 給 MainNB-GPT-CLI：工作卡與回答（Jimmy／筆電 → MainNB-GPT-CLI）

> **這個檔只有 Jimmy 這邊（Jimmy 本人或筆電的 Claude）會寫。** 你的回覆寫在分支 `v906/mainnb-gpt-cli-handoff` 的 `docs/handoff/FROM_MAINNB_GPT_CLI.md`。兩個檔各自只有一個寫者，git 合併不會衝突。
> 開始：20261006 19:2x。Jimmy 19:1x 轉達：「MainNB-GPT-CLI 已實測 GitLab 推送成功並啟動接案 LOOP，後續透過 GitLab 工作池與交接分支認領、交付，不碰編譯；GitHub 推送由你負責，首件接續 MAINNB-T1 測試 exe 合併方案分析」。
> 主機：JIMMYCHIU-NB（跟筆電的 Claude 同一台）。之前的本機信箱 `D:\HT9045\docs\handoff\TO_MAIN_CLAUDE_LOCAL.md`／`FROM_MAIN_CLAUDE_LOCAL.md` 從這一刻起**退場**（留著當紀錄，雙方都不再寫）。

## 0. 規則

1. **每次開工先 `git fetch origin`**，用 `git show origin/main:<路徑>` 讀這個檔、`docs/handoff/POOL.md`、`docs/handoff/WAITING_REPLIES.md`，以及 `AGENTS.md` 開頭的 Codex 入口與 `CLAUDE.md`。
2. **開工前先認領**：在 `v906/mainnb-gpt-cli-handoff` 的 `docs/handoff/FROM_MAINNB_GPT_CLI.md` §1 寫「我接 X、會動哪些檔」，先推一顆小 commit 再做；完成寫 §2、問題寫 §3；筆電的回答在這個檔 §4。第一次請從 `origin/main` 開這個分支並建檔（可以把你本機那份 `FROM_MAINNB_GPT_CLI.md` 的內容搬進來當開頭）。工作池 `POOL.md` 的卡也是在 FROM §1 認領。
3. **不碰編譯、不跑 ctest、不跑 wb_serve、不碰機台**——你的角色是分析與文件。要改程式的發現寫成建議（檔:行、golden 出處、怎麼驗），由筆電轉成卡給有編譯環境的人。
4. **交付**：文件推你的交接分支或 `v906/mainnb-gpt-<主題>` 分支；要進 main 的開 MR（目標 main），筆電合。**GitHub（HT9050 機台更新包）只有筆電推**，你不要推 GitHub。
5. commit 作者名用 `MainNB-GPT-CLI (Codex)`，訊息開頭 `MainNB-GPT:`。
6. **心跳**（每輪最後一步）：`python tools/laptop_ops/heartbeat.py --who mainnbgpt --doing "<在做什麼>" --push`（推到 `v906/mainnbgpt-heartbeat`）。沒卡了就在 doing 寫 `idle`，筆電會派。
7. **引用機台快照**：`Gerneral.ini` 看 `machines/HT9050/snapshot/machine_params/D_HT9045_system/`（機台讀這份）；`teach.ini`、`SetUp.inf`、`config\` 看 `machine_params/runcfg/`（規則在 `machines/HT9050/snapshot/SNAPSHOT_SOURCE.md`）。Frank01 1006 16:1x 就是在這裡看錯檔。
8. 給人看的文字用**繁體中文**（不要簡體字）；行號引用寫「**檔名:行號**」；寫「沒有／不存在／找不到」之前，在 main 最新版重查一次並附查法（你 18:13 寫 `FLOW9050_STATUS_20261005.md` 不在 main——它其實在 repo 根目錄的 `docs/handoff/`）。
9. 只有需要 Jimmy 裁決的才問（寫 §3）；動作流程題照 RULINGS_20261006 第 20 條（Steven 本人 → Frank）；同事本人回覆的就是定案（第 21 條）。
10. 寫入邊界讀 `.claude/ops/write-boundary-policy.json`。不要動 `D:\HT9045` 主資料夾裡別人沒提交的檔（那裡有 Jimmy 的在製工作）；在你自己的 clone（`D:\HT9045\tools\mainnb-gpt-cli-loop-repo`）或另開的工作樹裡做。

## 1. 我們正在改的檔（這些先不要動）

| 誰 | 檔 |
|---|---|
| 筆電（第 80 批 gate 中；第 81 批排隊） | 批次內容見 main 的 `HT9011UC_Cpp_V3.33.906.0/docs/NIGHT_REPORT.md`；你只讀不改，不會衝突 |

## 2. 須知（不用回）

- 你的協作約定原本寫在本機 `docs/handoff/MAINNB_GPT_CLI.md`（沒進 git）；以後以本檔為準。
- 1006 你交的三件已處理：Light／FAN 在 release 模式沒接上 → St02 修好（MR !277，第 81 批）；Index 放料 Z 教點＝0 → EastSun（W-129）；料盤 9050 專用 Z 欄位 → Frank01（W-128）。

## 3. 工作卡

### MAINNB-T1　📋 測試 exe 合併方案（唯讀、不編譯）

- **背景**（1006 17:1x～17:3x 筆電實測）：gate 從 10/01 的約 45 分鐘變成約 100 分鐘，主因是公司防毒——沒看過的 30 MB 測試 exe **第一次被打開要 45～79 秒，第二次 0.7 秒**；gate 每次從頭重編，445 個測試 exe 每個都要卡一次。1006 18:5x 再量：「先平行預讀新 exe」**反而更慢**（防毒同時只處理約 8 個檔，預讀把 ctest 要的檔擠到後面）——所以**減少 exe 個數**是不靠 IT 例外時剩下唯一的方向。
- **範圍（只讀）**：`HT9011UC_Cpp_V3.33.906.0/tests/CMakeLists.txt` 與 `tests/*.cpp`（`git show origin/main:<路徑>` 讀最新 main）。不改檔、不編譯、不跑 ctest。
- **要回答的**：①盤點：每個 `add_test` → exe 目標、連結的程式庫（`LINK_GROUP` 那串）、參數、`TIMEOUT`；共有幾個不同的 exe。②**不能合**的：會寫真實檔的、靠 `abort`／當機／`exit` 判結果的、依賴靜態初始化順序或同名全域符號會撞的、需要獨立行程的（socket、子行程、執行緒）——每類舉 2～3 個實例（檔:行）。③**能合**的：連結同一組程式庫、可以放進同一個「分派器」exe（`main(argc, argv)` 依名稱呼叫各測試原本的 `main`）——提出分組、估計 exe 數會從多少降到多少。④依上面的量測估算省多少時間（新 exe 第一次開 45～79 秒、ctest `-j 5`）。⑤風險與驗證（合併前後失敗集合要一樣、反向驗證怎麼做）。
- **產出**：推你的交接分支 `docs/handoff/MAINNB_T1_TEST_EXE_GROUPING_20261006.md`（繁體中文、檔:行），FROM §2 寫一列。讀完我決定派給誰實作。

## 4. 回答與通知（筆電寫）

| 時間 | 標題 | 內容 |
|---|---|---|
| 20261006 19:2x | 📋 **開張：改走 GitLab；首件 MAINNB-T1（W-130）** | 照 §0 開 `v906/mainnb-gpt-cli-handoff`、在 FROM §1 認領 MAINNB-T1、推心跳。本機信箱退場。 |
| 20261006 20:0x | ✅ **MAINNB-T1 收到——品質很好**；C3-119 認領收到 | 我抽驗你的數字：main `7c4bf541` 的 `tests/CMakeLists.txt` 443 個 add_test＋兩份 include 2＋1＝446、add_executable 378＋2（＋三個 foreach 多出的 3）＝383，**全對**；「每個測項仍各自開行程」的設計、TIMEOUT 被 `:4127` 覆蓋、DEFER 環境、RUN_SERIAL／SKIP_RETURN_CODE 這些坑都點到了。試點已派給 Ifor01（有編譯環境，W-131），他會量合併後大 exe 的首次開啟時間。W-130 關。 |
| 20261006 21:0x | ✅ **C3-005（Abort Home）、C3-119（HotPlate）兩份唯讀複核收到——兩份都對，POOL-3 已改列「已接，待執行期驗收」** | 我抽驗：C3-005 的 `a16a0697`（10/02 HOMEMON）確實補了網頁接線與 C++ 分派；C3-119「`cbSelectHPFromDB` 是選資料庫的事件、不是存檔鍵」對。你 20:26 自己更正「20:25 那輪沒跑、計時器不能當持續工作的證據」——這種更正寫得很好，照舊。下一張照你的規則從 POOL 挑；⚠ 若是 POOL-2（`#if 0`），認領前先查 `docs/handoff/IF0_CENSUS_20261006_linked.tsv`（22 支檔整支沒連進 wb_serve，解開沒效果；RogerYang 20:4x 發現、筆電量）。 |
