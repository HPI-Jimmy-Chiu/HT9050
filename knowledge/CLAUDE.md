# CLAUDE.md — HT9045（Claude Code 入口）

本檔是 Claude Code 的專案記憶入口。完整專案說明沿用既有的 `AGENTS.md`，以下用 import 串接，避免兩份漂移。

@AGENTS.md

---

## Claude Code 資產對照（由 Copilot 設定轉接）

| 類型 | Claude Code 位置 | Copilot 原始位置（鏡像，須同步） |
|------|------------------|-----------------------------------|
| Skills | `.claude/skills/<name>/SKILL.md` | **無鏡像**（見下方 20260918 註） |
| Agents（子代理） | `.claude/agents/<name>.md` | `.github/agents/<name>.agent.md` |
| Commands（斜線指令） | `.claude/commands/<name>.md` | `.github/prompts/<name>.prompt.md` |
| Hooks（寫入邊界） | `.claude/settings.json` → `scripts/ops/check-write-boundary.ps1` | `.github/hooks/pretool-write-boundary.json` |
| 寫入邊界政策 | `.claude/ops/write-boundary-policy.json`（兩邊共用同一份；1005 從 `.github/ops` 搬過來，MR !208、RULINGS_20261005 第 20 條） | `.github/hooks/pretool-write-boundary.json` 的 `OPS_WRITE_POLICY` 指同一份 |

> 維護提醒：修改任一 Skill / Agent / Command 後，若仍同時使用 Copilot，請同步更新對應的鏡像檔。

> **Steven 20260918 — `.agents/skills` 已退場。**
> 本表原本寫「Skills 的 Copilot 鏡像在 `.agents/skills/`」，實測那是過時的。
> 三個 skills 目錄的真實歸屬：
>
> | 目錄 | 誰在用 | git |
> |---|---|---|
> | `.claude/skills/` | Claude Code | ✅ 追蹤（332 檔） |
> | `.github/skills/` | **Steven 個人的 Copilot**（29 個 skill） | ❌ **未追蹤，0 檔** |
> | `.agents/skills/` | **沒有人** → 本日退場 | 曾追蹤 54 檔 |
>
> `.agents/skills` 既不是 Claude Code 的（那是 `.claude/skills`），
> 也不是 Copilot 的（那是 `.github/skills`），而且全樹沒有任何設定檔引用它
> （`*.json` / `*.code-workspace` / `*.yml` 全掃過，零命中）。它是孤兒。
>
> ⚠ 另注意 `D:\HT9045_ref\.github\`（Jimmy 那份，來自 git）**底下沒有 `skills/`** ——
> Steven 的 `.github/skills` 因為未追蹤，不會出現在 Jimmy 那邊。**20260926 Steven 裁決：`D:\HT9045_ref` 已退場**（`git worktree remove`；St01 的產生器改指主 repo 的 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy`，`3e0ebb92`），本段只剩史料意義。
>
> 退場前已逐層驗證它不是純冗餘：目錄層與檔案層確實是 `.claude/skills` 的子集
> （獨有 skill 0 個、獨有檔案 0 個），但逐行比對後 7 個 skill 有獨有行。
> 追查結果只有 **`ht9045-motor-home` 的 §11 HomeClass（72 行）是真的獨有**，
> 已併入 `.claude/skills/ht9045-motor-home/SKILL.md`；其餘為假陽性
> （措辭改寫、指向不存在的 `FlowChart\` 死指標、或只是「五個 vs 六個陷阱」的舊數字）。
>
> ⇒ **Skills 現在只有 `.claude/skills/` 一處權威，不需要再同步任何鏡像。**
> 完整稽核：`D:\docs\ops\weekly\2026\09\20260918\20260918_Steven_skills_duplication_audit.md`（20261005 18:1x 註：這份稽核只在 Steven 那台的 `D:/docs`，沒有搬進入口網站；Steven 1005 17:0x 起 ChangeLog／日報／週報改放入口網站 repo 的 `public/Docs/…`（MR !211），那天的 ChangeLog 在 `public/Docs/ChangeLog/Steven/CHANGES_20260918_Steven.md`）

### 可用子代理（Task 工具呼叫）
- `ht9045-v912` — **目前的量產維護目標**（20260909 起），鎖定 `HT9011UC_Code_V3.33.912.0_20260908_Jimmy`（公司 20260908 出貨版）。新客戶案件預設走這支。
- `ht9045-v899` — V3.33.899.0 版本**唯讀分析**（20260909 起樹已凍結）。用途只剩「客戶機台跑 899.x，要對照它實際在跑的碼」；修正一律做在 V912。
- `ht9045-v906` — **HT9045 專屬 C++ 代理**（V906 移植樹，**實驗機**），鎖定 `HT9011UC_Cpp_V3.33.906.0`。所有 C++ 工作都歸它：翻譯波次、翻完後的新功能、CMake/ctest、MinGW+MSVC 雙 oracle、WebBridge 與瀏覽器 HMI。C++17 / UTF-8 / CMake，與 V899 的 BCB6 / Big5 / pre-C++11 規則完全相反。目標架構（20260812 定案）：**UI 用 web 開發、底層邏輯與控制是 C++**；MFC/Gate A 只是翻譯驗證 harness。
- `case-coordinator` — 客戶異常案件協調入口，分派 intake/analysis/closure。
- `weekly-report` — 週報與客戶異常 case 管理（Hub 模式，操作 Weekly_AI 工作區 `d:\Work-jimmychiu\document\WeeklyReport\Weekly_AI` 的 Python 工具）。破壞性動作（建下週週報、建 case、重產 Excel）執行前先確認。

### 可用斜線指令
- HT9045：`/ht9045-debug`、`/ht9045-v912-build`（**預設**）、`/ht9045-v899-build`（V899 已唯讀，僅供重現舊版建置基準）、`/ht9045-skill-factory`
- V906 純翻譯戰役：`/pt-wave`（**已完成**——20260817 PT-W10 量測非表單 0 行未翻；
  政策見 `pt-wave-loop` skill，其五個陷阱與硬邊界仍是後續戰役的單一出處）
- V906 DFM→WEB 戰役：`/fw-wave`（執行一個完整波次；搭 `/loop /fw-wave` 夜間自動連續推進。
  政策見 `fw-wave-loop` skill，計畫書 `HT9011UC_Cpp_V3.33.906.0/docs/DFM2WEB_CAMPAIGN_PLAN.md`。
  **唯讀方向；write path 是安全關鍵，佇列等使用者**。
  ⚠ 20260916 裁決：**FW-3 表單波已退役**，剩餘價值集中到 C++ 的 tag 發布側；
  tag 串流要接上手工 HMI，且由我們直接動同事的 `.js`）
- V906 START 戰役：`/st-wave`（**推進一波**；`/loop 25m /st-wave` 連續跑）。
  把 golden `TfMain::Start()` 的 1,875 行分波翻進 `TfMainWeb::StartFromWeb()`。
  政策見 `st-wave-loop` skill，計畫書 `HT9011UC_Cpp_V3.33.906.0/docs/START_CAMPAIGN_PLAN.md`。
  **⛔ 做完 ST-W7 硬停；S1／S3（武裝）絕不自動做 —— 那一步機台會動。**
- V906 1203 導入戰役：`/bu-wave`（**推進一波**；`/loop 30m /bu-wave` 連續跑）。
  把同事的 PCIE-1203 成果（`D:\HT9045\backup\HT9050_PCI1203_20260916`）導入 A 樹。
  政策見 `bu-wave-loop` skill，計畫書 `HT9011UC_Cpp_V3.33.906.0/docs/BU_C_CAMPAIGN_PLAN.md`。
  **⛔ 做完 BU-C3 硬停；BU-C4（write path）/ C5（網頁）/ C6（機邊）全是 🟡/🔴。**
  ⚠ `docs/BU_CAMPAIGN_PLAN.md` 是 **B 樹史料**（A 樹 `git log --grep="BU-"` = 0 顆），不是待辦。
- 夜間例行：`/night-loop`（**下班前掛 `/loop 20m /night-loop`**；清垃圾／收攏未 commit 的
  日間工作／處理刻意標記未處理的項目／衛生哨兵。**07:00 起收尾，07:30 前寫完
  `HT9011UC_Cpp_V3.33.906.0/docs/NIGHT_REPORT.md` 後靜默**，早上只讀那一份。
  政策見 `night-loop` skill。⚠ session-scoped —— 視窗關掉或機器重開就沒了）
- 治理：`/ops-daily-worklog`、`/ops-weekly-review`、`/ops-skill-maintenance`、`/ops-new-project-bootstrap`
- 週報/案件（Hub，操作 Weekly_AI）：`/update-weekly`、`/weekly-status`、`/weekly-case-intake`、`/weekly-case-integrity`、`/weekly-next-week`、`/weekly-help`、`/weekly-upload`（關鍵字「**上傳週報**」）、`/daily-upload`（關鍵字「**上傳日報**」）

> Weekly_AI 工作區為 Hub 模式接入：agent/指令在 HT9045，實際 Python 工具與 `weekly_data.json`、`Customer/` 資料留在 Weekly_AI。修改 weekly-report agent 或指令時，Weekly_AI 的 `.github/` 原始定義為鏡像，視需要同步。

---

## 與使用者協作的方式（使用者 20260925）

**紀錄寫在哪裡**：專案的知識、裁決、踩過的坑，一律寫進專案檔案並 commit（裁決 → `HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_YYYYMMDD.md`；
工具與流程的坑 → 本檔或對應 skill；進度 → `docs/NIGHT_REPORT.md`／`docs/INBOX_QUEUE.md`）。
**Claude 的個人記憶（`~/.claude/projects/.../memory/`）只在非常特殊的需求才寫**——它只在這台電腦、這個帳號看得到，
不進 git，舊電腦與同事都看不到。使用者原話：「除非是非常特殊的需求才寫入個人記憶，我不是所有任務都要做這些」。

**什麼要問、怎麼問**：
- 驗收、量測、確認問題的紀錄**不需要使用者回覆**，直接做、直接記。
- 只有**決策題**才問，而且用**回覆裡的條列文字**（白話＋舉例＋選項＋建議），**不要用中斷式的彈窗詢問**（AskUserQuestion）。
- 必須問的，**趁使用者在的時候一次問完**；使用者不在（夜間／週末迴圈）時**不可以因為沒人回答而停住**——
  照「保持現行行為 → 可逆 → 樹編得起來」選安全預設值繼續，並把決策題寫進 `NIGHT_REPORT.md` §0。
- **等 EastSun（機台端）的題目一律不等**（使用者 20261004 22:1x：「等EastSun決定->這問題一律不要等他，沒有回答就立刻也一份給ST01處理，他也能回覆」）：問 EastSun 的同一顆 commit 也在 `docs/handoff/TO_STEVEN.md` §4 給 St01 一份，請 St01 直接回答能答的；需要人在機台旁量的才留給 EastSun。RULINGS_20261004 第 4 條。
- **推公司 GitLab main 之後，一律同步推 GitHub 機台更新包**（使用者 20260926：「你推公司的main後，也要自動推github，必須的」）：
  repo `github.com/HPI-Jimmy-Chiu/HT9050`（公開，第 18 條），只推更新包內容、不推 GitLab 歷史（第 22 條 B）；每包放 `updates/<rev>/`（相對上一包，附 check_and_copy＋base＋README_MACHINE_AI），
  **舊包不動**（機台可能正在套），根目錄 README.md 的「更新包清單」補一列；推之前掃過權杖／私鑰／7z 密碼 0 處。機台要不要套由使用者決定，推上去不代表要機台馬上 pull。
  ⚠ 20260930 補註：這條只指 **HT9045 repo** 的 main。RD5 入口網站（`D:\HT9045-Index`，分支＋MR 自動合併進它自己的 main）與 Weekly_AI（私人 GitHub `weeklyreport`）各有自己的推送規則，**不觸發**機台更新包。
- **跟 Steven 分工走 git 交接檔**（使用者 20260926）：不急的工作寫在 main 的 `docs/handoff/TO_STEVEN.md`（只有 Jimmy 這邊寫：工作卡、我們正在改的檔、回答），
  Steven 的認領／完成／問題寫在他 `v906/steven-*` 分支的 `docs/handoff/FROM_STEVEN.md`（只有他寫）。兩邊都「**開工前先認領、先推再做**」，避免重工。
  急的事用電話。夜間迴圈每一輪都讀 FROM_STEVEN.md（`night-loop` skill 第 5b 步）。
  ⚠ 20260926 13:4x 起：Steven 有兩台（**St01**＝資料讀寫轉檔 `v906/steven-cbridge-review6`、**St02**＝測試介面 `v906/steven-gpib-widget`），FROM_STEVEN.md **只讀 `origin/v906/steven-handoff`**；另有三方聊天 `docs/handoff/CHAT_JIMMY.md`（我們寫，在 main）／`CHAT_ST01.md`／`CHAT_ST02.md`（他們寫，在 steven-handoff）。
- **工作語言**（使用者 20260927 17:4x，為了省算力）：內部作業、agent prompt 與 agent 回傳用英文；**問使用者、對使用者說明、給使用者看的文件一律繁體中文**。
- **同事分支合進 main 走 MR**（使用者 20260927 18:0x 選 A，`RULINGS_20260927.md` 第 8 條）：同事沒開 MR 的，筆電合併時用 push option（`git push origin <合併結果>:refs/heads/v906/jimmy-merge-<主題> -o merge_request.create -o merge_request.target=main -o merge_request.title=…`）開一張；
  gate 綠了照舊推 main，GitLab 自動標成已合併（沒有 CI，網頁按 Merge 不會跑 gate）。筆電自己的小 commit 照舊直接推 main。
- **筆電的角色**（使用者 20261003 11:3x，`RULINGS_20261003.md` 第 13 條：「你現在角色除了解惑我的疑問外，還有就是有效分配工作給其他人處理，其他人也可以協助思考問題」「因為你一旦停擺，所有人都會停下來無法運作」）：新功能與分析寫成工作卡交給接案的人（TO_STEVEN／TO_IFOR／TO_FRANK／TO_JERRY／TO_KEVIN／TO_ES02；NB2 走 CHAT_JIMMY），問題也可以請同事一起想；筆電只留回答 Jimmy、分派追蹤、合 MR＋gate、出 GitHub 機台包、寫裁決與報告；筆電自己的子代理只用在整合時非修不可的小修補，**同時最多 1～2 個**（下面那條 ≤5 是上限，不是目標）。
- **Agent／workflow 扇出：同時在跑的 agent「總數」≤ 5**（所有 workflow 加起來，不是每個 workflow 各 5），ultracode 不解除。
  開新 workflow 前先數還在跑的；要多個問題就讓一個 workflow 內**依序**跑。
  20260925 違反過兩次（IO 調查 4＋R28 三件 3 同時跑到 8、P25 審查 3＋其他 3 到 6），當場停掉並改成依序。
- 工具層也不可以跳詢問：寫入邊界 hook 對 Claude 的 scratchpad 與記憶目錄放行（`externalAllowedRoots`），
  並把 `.claude/worktrees/<名稱>/` 當成一棵完整的樹來判斷（見下方 hook 那一段）。
- **開工前先對正本**（使用者 20261005「S1、S2、V1、V2 都照建議 A 做」，RULINGS_20261005 第 2 條）：`D:\HT9045` 主資料夾**沒有人會自動換新**
  （整合在 worktree 做、直接推 GitLab；1005 量到它停在 10/02、落後 origin/main 1,225 顆，在裡面開的 session 讀到舊規則而給錯建議）。
  V1：每個 session 開場的 SessionStart 檢查（`scripts/ops/check_stale_checkout.py`，`.claude/settings.json`）落後就提醒（main 落後就提醒，其他分支落後 100 顆以上才提醒），不自動改；
  V2：夜間迴圈每一輪跑 `scripts/ops/ff_main_checkout.py` 安全快轉（不在 main／有本機 commit／有建置或 wb_serve 從這裡在跑／git 拒絕覆寫時就略過，不 stash、不 reset）。
  看到提醒時：讀規則、技能、交接檔之前先更新，或用 `git show origin/main:<路徑>` 讀正本。

---

## 專有名詞（使用者講這些詞時的意思）

### ST 戰役 —— 讓瀏覽器的 START 真的啟動機台

| 詞 | 意思 |
|---|---|
| **ST 戰役** | 把 golden `TfMain::Start()`（`main.cpp:4385-6259`，**1,875 行**）翻進移植樹，讓網頁的 START 按鈕真的能啟動機台。計畫書 `HT9011UC_Cpp_V3.33.906.0/docs/START_CAMPAIGN_PLAN.md` |
| **ST-W1 … ST-W7** | 翻譯波次。W1-W6 各約 300 行，W7 是 6 個小相依＋補完擋啟動的洞 |
| **S1** | FW-W3 單一操作權 token（`control.acquire/release`）。使用者 20260819 裁決的第 3 條，至今未做 |
| **S2** | 翻譯本身 = ST-W1..W7。**`/st-wave` 的全部範圍** |
| **S3** | **武裝**：掛 wb_serve 分派 + 網頁按鈕送命令。**不可逆，機台會動，要使用者在機台旁** |
| **`StartFromWeb()`** | 翻譯的落點（`WebStart.cpp` 的 `TfMainWeb::StartFromWeb`）。**刻意不是 `TfMain::Start()` 的 override** —— 全樹 **34 個** `fMain->Start()` 家族呼叫點只有 **4 個**被閘住，override 一次就把 SECS/GEM 遠端 START、clean-out／one-cycle／ART 自動重啟等 **30 條**路徑全部武裝（20260924 使用者授權更正：8841abc 照翻 `ProcessSensorScan` 多了一個 `fMain->Start("AMR")`，原本寫 32／29／3；20260927 NB2 R80 再更正：St02 `6fff0960` 加的 `TesterComm/Handler/HandlerGpibMsg.cpp:717` GPIB START 在同檔 :176 `#define W906_REMOTE_START_WIRED 0` 底下＝閘住，工具原本只認字面 `#if 0` 才把它算成活的 ⇒ 34／30／4，活的仍是 30；之後 St02 把 HTSET,333 經 `W906_RemoteRunStart` 接上 ⇒ 34／31／3；20261001 St02 MR !33 把 GPIB 遠端 START 也接上 ⇒ 34／32／2；20261003 第 130 包 St01 review6 照 golden 把 Contact 頁 START（CT-3b）與自動 Offset START（OS-1b，`FileRW/Offset_File.cpp:846`）經 `W906_RemoteRunStart` 接上 ⇒ 36／34／2；20261003 第 132 包 St02 c912-1（`2298e664`）照 golden 906 拿掉 GPIB 遠端 START／STOP（906 沒有 MSG_CMD 204／205）⇒ **35／33／2**，CT-3b 在 `FileRW/DeviceForm_File.cpp:710`）。⚠ 這個數字**不要人工 grep**（20260915 與 20260921 兩次人工盤點都錯），跑 `HT9011UC_Cpp_V3.33.906.0/tools/start_sites_census.py`；CI 形式 `--check 35 33 2`（20260927 起是 ctest `START_SitesCensus`，數字變了 gate 就紅；⚠ 20260930 更正：0927 St02 `aeea58e5`（W10＝B，打開 TCP 7016 命令伺服器）讓 `Command.cpp:17108` HTSET,333 變活的，從那時起就是 34／31／3，本檔原寫 34／30／4 已過期；另注意工具只掃 `*.cpp`／`*.h`，`.gen.inc` 裡活的 `fMain->Start` 看不到）。清單與三次量錯的原因見 `WebStart.h` 檔頭與 `docs/DUET3D_REFERENCE_ANALYSIS.md` §9.1.1 |
| **🔴 / 🟡 gate** | 計畫書 §5.1 的還債清單。🔴 = golden 用它擋啟動（S3 之前必須補完或裁決）；🟡 = 純顯示 |

**使用者只要說「下一波」「推進 ST」「跑 st-wave」，就是 `/st-wave`。**
要連續跑：`/loop 25m /st-wave`。

### 其他常用語

| 詞 | 意思 |
|---|---|
| **夜間迴圈** | `/loop 20m /night-loop`。晨間報告在 `HT9011UC_Cpp_V3.33.906.0/docs/NIGHT_REPORT.md` |
| **上傳週報** | `/weekly-upload`（使用者 20260930：「當我說關鍵字[上傳週報]，就自動幫我執行」，直接跑、不先問）：本週週報 md 貼上 RD5 入口網站（內容檢查乾淨才推，有命中才停下來問）＋照上一封週報信做好本週的信（預設存草稿開視窗，使用者說「寄出」才寄）。工具在 Weekly_AI `tools/weekly_upload.py` |
| **上傳日報** | `/daily-upload`（使用者 20261005：結案補一列＋關鍵字）：說了就開始做、不先問要不要做。**兩段式**——當天的日報（12:00 前＝前一個工作天）照入口網站統一格式寫好、`check_daily.py`＋內容檢查都過，**先把全文貼給使用者看，使用者回「推」才推**上 daily.html（入口網站規定 AI 代寫的要本人看過）。結案時 `close_case.py` 會自動在當天草稿補一列（不推）。工具在 Weekly_AI `tools/daily_upload.py`。起因：20261005 量到網站上 JimmyChiu 日報 0 篇——之前只有週報與 Release Note 有發佈工具 |
| **接線 / wire** | 網頁欄位 ↔ 配方文件的對照（`tools/pagewire/`）。三元組是 `[文件, 區段, 鍵]` |
| **golden** | `HT9011UC_Code_V3.33.906.0_20260618`（BCB6、Big5、唯讀）。翻譯的對照原文 |
| **移植樹 / A 樹** | `HT9011UC_Cpp_V3.33.906.0`。唯一的 C++ 開發目標 |
| **B 樹** | `D:\HT9050`，20260915 起凍結 |

---

## 路徑範圍指令（取代 Copilot `applyTo`）

Claude Code 無原生路徑範圍指令機制，故將原 `.github/instructions/*.instructions.md` 的關鍵守則統整於此。詳細表格仍可參考對應 instruction 檔。

> **⚠️ 規則相反的兩類樹，先確認你在哪一棵。**
> **BCB6 類**（Big5 / pre-C++11 / .bpr）：
> `HT9011UC_Code_V3.33.912.0_20260908_Jimmy` — **目前的量產維護目標**（20260909 起）
> `HT9011UC_Code_V3.33.899.0_...` — 前一個量產維護版，**已唯讀**（20260909 關閉，見下）
> **C++ 類**（C++17 / UTF-8 / CMake）：
> `HT9011UC_Cpp_V3.33.906.0`（**實驗機**）
> 另注意 `HT9011UC_Code_V3.33.906.0_20260618` 是**另一棵 BCB6 樹**，同樣叫 906 但唯讀；差別在 `_Code_` 與 `_Cpp_`。
>
> **V899 已於 20260909 關閉**（使用者裁決）：加入 `readonlyRoots`，hook 實測回 deny。
> 未結案的週報 row 3/6/8/9 一律**改由 V912 判斷與交付**。可行性已查證：
> 四件涉及的機種（Type_HT9046LS / Type_HT9045 / Type_HT9046A）與程式區域
> （`bCleanOutCanTrayEnd` 5 處、`CleanOutFinish` 73→77 處、`SaveSiteYield` 3 處）
> 在 V912 都有對應。
> **唯讀只擋「改」不擋「讀」** —— 分析客戶跑 899.x 的問題時，照樣要開 V899 對照，
> 那是機台實際在跑的碼；只是修正與交付一律出 V912。
> 實務後果：row 3 客戶正在跑 899.33，若需改程式即等於請客戶跨版升級。
>
> 若日後要重開 V899：必須從 `readonlyRoots` **移除**並加回 `allowedWriteRoots`。
> 只從 `readonlyRoots` 拿掉不夠也不對稱 —— `confirmOutsideAllowed:false`
> 會讓「兩份清單都沒列到」變成靜默放行，不是詢問也不是阻擋。
>
> **權威的寫入邊界在 `.claude/ops/write-boundary-policy.json`，不在本文。**（1005 從 `.github/ops` 搬過來，MR !208；`.github/ops` 只剩一行說明。1005 之前開的 session 的 hook 指令還帶舊路徑，守門腳本會自動改讀新位置，`AI(W906-WBPOLICY-MOVE)`）
> Claude Code 端由 `.claude/settings.json` 的 PreToolUse hook 強制
> （20260909 接上；在那之前腳本第 73 行有語法錯誤，從未執行過）。
>
> ⚠⚠ **20260925：hook 從 09/23 起又靜默失效過一次，已修。** 這台的 Claude Code 用 **Git Bash**
> 執行 hook 命令列，原本寫的 `-File .\\scripts\\ops\\check-write-boundary.ps1` 反斜線被吃掉，
> 變成找不到 `.scriptsopscheck-write-boundary.ps1` ⇒ `hook_non_blocking_error` ⇒ **放行**
> （舊 session 累計 4,857 次，V899／golden 唯讀全程沒擋）。
> 現在的寫法是 `"$CLAUDE_PROJECT_DIR/scripts/ops/…"`（正斜線；session 的 cwd 常在
> `.claude/worktrees/<wt>/…` 裡，相對路徑不可靠），實彈驗證對 V899 的 Edit 回 `Readonly path blocked`。
> **懷疑守門失效時，數逐字稿裡的 `hook_non_blocking_error`，不要只看「有沒有被擋過」**——
> hook 失敗是非阻斷＝fail-open，壞掉的守門員不會吵。
> 測 PreToolUse:Edit 要用**真的存在**的 `old_string`：Edit 先驗字串，不存在就在 hook 之前失敗，等於沒測到。
>
> ⚠ **20260927：Bash 工具會把送進 Python 的 `\\` 減半**（`python - <<'EOF'` 與 `python -c "…"` 都會）⇒ Python 把 `\b` `\n` `\r` `\a` `\14`
> 解成退格／換行／CR／響鈴／跳頁寫進檔案，畫面上看不出來。0926～27 一晚 10 次，清出 28 個控制字元（本檔 `V906\build_dbg` 曾顯示成「V906uild_dbg」）。
> **含反斜線的 Python 一律用 Write 工具寫成檔再跑**；反斜線用 `chr(92)`。筆電這台另在 `.claude/settings.local.json`（不進 git）
> 掛了 `scripts/ops/deny-heredoc-backslash.ps1`：inline Python 含 `\\` 就擋（逃生口 `ALLOW-PY-BACKSLASH`）；別台要用自己接。
> 掃描器 `D:\HT9045\backup\night_tools_20260927\ctrlchar_scan.py`（night-loop 線 D 哨兵）。
>
> 同日另修兩處（`AI(W906-HOOK-WT)`，使用者：「我不希望周末不在因為沒人回而停住」）：
> ①路徑在 `<repo>\.claude\worktrees\<名稱>\` 底下時，**以那棵 worktree 為根**判斷（V899／golden 在任何 worktree 裡照樣 deny；
> cwd 在 worktree A 時寫 worktree B 不再被當成「工作區外」而詢問）；
> ②政策新增 `externalAllowedRoots`（`%TEMP%\claude\`、`%LOCALAPPDATA%\Temp\claude\`、`%USERPROFILE%\.claude\projects\`），
> Claude 的 scratchpad 與記憶目錄不再詢問。三種 cwd × 14 種路徑共 42 組決策實測全部符合預期。
>
> **MG 戰役（V899→V910）已結束**：目標樹 `HT9011UC_Code_V3.33.910.0_20260716_Jimmy`
> 已從工作目錄移除，內容保存在 tag `v910-mg-final`。公司 V912 已整併其成果
> （27 個 `//AI(mg899to910)` 標記 + 655 個 `//AI(ht9045-v899)`）。
> 史料：`docs/MG899TO910_CAMPAIGN_PLAN.md`、`docs/MG_PORT_LEDGER.md`。

### 編輯 V912 C/C++（`HT9011UC_Code_V3.33.912.0_20260908_Jimmy/**/*.{cpp,h,hpp}`）

> 目前的量產維護目標。語言規則與 V899 那節**完全相同**（BCB6 / Big5 / pre-C++11），
> 差別只在寫入邊界、AgentName、建置輸出。

**寫入邊界**
- 可寫：本目錄（以及 `HT9011UC_Cpp_V3.33.906.0/`，那是 V906 代理的地盤）。
  **V899 已唯讀**，其他版本樹一律唯讀。
- `system/`、`config/`、`CFG/`、`IniData/`、`setup.inf`、`CurrentSetupData.txt` 為共用參數，預設只讀。
- 不得修改 `AGENTS.md` 禁改清單中的驅動與定義檔。

**BCB6 / VCL / 編碼**：同 V899 那節（pre-C++11、`AnsiString`、Big5 最小差異、`.dfm` 不手動重排）。

**AI 修改註解**：`//AI(ht9045-v912) YYYYMMDD: 描述`。
CASE-ID 要放在**冒號後面**（`make_release_note.py` 的正則會吞掉日期後的括號）。
若日後再跑一次 899→912 回搬戰役，campaign tag 用 `mg899to912`；
**不要重用 `mg899to910`** —— V912 裡那 27 筆是前一次搬移的簽名。
注意這棵樹的 AI tag 命名空間**與公司工程師共用**（例：`uTemp_Set.cpp:5676`
的 `//AI(ht9045-atc) 20260904 (RogerYang)`），所以 harness 前綴要保持可辨識。

**建置（第一次動手前必讀）**
- 912 的 `HT9045.bpr` 把 `PROJECT` 指向 `D:\HT9045\EXE\HT9045.exe`、301 個 obj 與
  PCH 指向 `D:\HT9045\Obj` —— **與 V899 出貨路徑相同**。
- `build_bcb.bat` 自 20260909 起**預設**把 bpr2mak 產出的 .mak 導向
  `<樹>\Obj912` / `<樹>\Out912`（`.claude/skills/bcb_build/scripts/redirect_mak.ps1`，含驗證 gate）。
  忘記帶參數只會得到安全的私有建置。要寫共用目錄必須第 4 個參數明講 `shared`。
- `MachineType.h:43` 的 `SOFT_SIMULTE` 在 vendor drop 裡是**開著的**
  （連帶開 `DEBUG_ATC`、`DEBUG_HANGUP_NO_HOME`）。出貨前務必關掉。

**已知的 V912 自身問題（不是我們搬壞的）**
- `acatchtray.cpp:2449` JAM0610 防抖被換成 KYEC-only 閘門，而 `iTrayDetectCount`
  全樹只有定義 `=0`、從未賦值 → 對所有客戶（含 KYEC）防抖等於失效。
- 「Eastsun 20260521」ESD 讓位整合在 shuttle-2 側少了內層 `else`，
  **21 支 `aoutarm*.cpp` 全中**；沒裝 ESD/BU5 的機台只要有 Auto 盤要換，
  整個 case 2x00 變 no-op。
- `cmydef.h` 有三個非 inline 的檔案範圍函式定義（`IsSafePLCIOType_Schneider` /
  `IsSafePLCIOType_ReeR` / `IsSafePLCIOInstall`），被 239 支 .cpp include。
  **20260909 實測：全量建置 290 個單元、294 個 .obj，ilink32 連結成功、0 個重複符號錯誤。**
  也就是 BCB6 不拒絕這個寫法。寫法仍不合慣例，但**不要再當成建置風險重新爭論**。

### 編輯 V899 C/C++（`HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/**/*.{cpp,h,hpp}`）

> 前一個量產維護版，**20260909 起唯讀**。以下規則保留作為歷史與對照用；
> 這棵樹現在只能讀，任何修正都做在 V912。

**寫入邊界**
- **本目錄已唯讀**（在 `readonlyRoots` 裡，hook 會 deny）。可寫的只有 V912 與 `HT9011UC_Cpp_V3.33.906.0`。
- `system/`、`config/`、`CFG/`、`IniData/`、`setup.inf`、`CurrentSetupData.txt` 為共用參數，預設只讀，使用者明確要求才改。
- 不得修改 `AGENTS.md` 禁改清單中的驅動與定義檔（如 `CSmc.h`、`sqlite3.h` 等）。

**BCB6 / VCL / 編碼**
- 以 pre-C++11 為準：不用 `auto`、`nullptr`、lambda、range-for；不引入新 STL/第三方依賴。
- 字串優先用 VCL `AnsiString`，非必要不引入 `std::string`。
- 原始碼為 Big5：最小化變更範圍，避免整檔重寫成 UTF-8，審查不得出現亂碼（U+FFFD）。
- `.dfm` 為 Delphi 格式，不手動重排，除非使用者明確要求處理表單。

**AI 修改註解**（格式：`//AI(AgentName) YYYYMMDD: 描述`）
- 日期固定 `YYYYMMDD`；V899 新變更 AgentName 預設 `ht9045-v899`。
- 描述寫「動作 + 原因」，不寫 fix/update 等空泛字樣。
- 新增行：在前一行放獨立註解；修改單行：行尾加註解；多行區塊：只在區塊第一行前加一則。
- 不重寫既有歷史 AI 註解格式。

**完成後自我審查**
- 確認落在 V899 目錄內、未碰禁改檔、最小差異、無編碼亂碼。
- 回覆需說明：影響函式/狀態機、風險、驗證方式。
- 修改共用標頭、核心狀態機、跨模組函式、全域變數時，優先對 V899 根目錄 `HT9045.bpr` 做 BCB6 build 檢查；環境不足無法編譯時要明說，不可假設成功。

### 編輯 V906 C++ 移植版（`HT9011UC_Cpp_V3.33.906.0/**/*.{cpp,h,hpp}`）

> 本節規則**與上面 V899 那節相反**，不要混用。專屬代理：`ht9045-v906`。

**寫入邊界**
- 只允許修改 `HT9011UC_Cpp_V3.33.906.0/`。**BCB6 那邊要改請交回 `ht9045-v912`**
  （V912 是量產維護目標；V899 自 20260909 起整棵唯讀，誰都不能改）。
- `D:\HT9045\EXE\` 是 BCB6 量產建置的輸出目錄，V906 的 exe 永不複製或覆寫進去。
- `D:\HT9045\EXE_V906\` 是 V906 自己的輸出目錄（`BA_MIGRATION_PLAN.md:800` 已決的分家）。
- ⚠⚠ **20260918 裁決：`--dry` 與 `--allow-system-write` 不再是閘門。一律實際讀寫真實檔。**
  使用者原話：「**不用擋，一律確實讀寫檔案**」「**沒有實際修改我都無法手動驗證**」。
  這**推翻**了 20260917 之前「跑 `wb_serve` / `wb_publish` 啟動時一律加 `--dry`」那條。

  **取代它的是「備份 → 驗證 → 刪備份」**（使用者 20260918 指定的做法）：
  1. 動到真實檔之前，先備份會被碰到的那些（作用中工單、`system\*.ini`、`D:\HT9045_Log\`）
  2. 跑驗證，**比對檔案內容**，不要只看 ack 回 `ok`
  3. **驗證通過就把備份刪掉** —— 使用者明確要求不要累積備份垃圾
  4. 驗證有問題就用備份還原，並回報

  > 為什麼改：擋住寫入等於讓驗證跑在假路徑上，**等於沒驗到**。
  > 真正要防的是「不可逆的損害」，而備份把它變成可逆的。

  ⚠ 仍然要知道的事實（不是禁令，是風險來源）：
  * `common.cpp:89` 的 `asGeneralPath` 指向量產機共用的 `system\Gerneral.ini`，
    而 `LoadMachineConfig()` 會把缺的鍵**補寫回去**（實證：20260817 該檔被整檔重寫）。
    ⇒ 那個檔是備份清單的第一項。
  * web API 的讀寫**本來就**解析到真實檔（`RealRecipeDir()`，20260917 裁決 A1），
    `--dry` 從來就沒擋過它。「帶了 `--dry` 就不會寫真檔」一直是錯的。

- ⛔⛔ **20260923 裁決：`--dry` 已完全退場。行為由「建置期」的 `SOFT_SIMULTE` 決定，
  不由任何執行期旗標決定。**（使用者原話：「`--dry` 不再使用」「比照 #8，行為由建置期的
  `SOFT_SIMULTE` 決定，不由執行期旗標決定」）

  上面 20260918 那條說的是「`--dry` 不再是**閘門**」；這一條更進一步：**它不再存在於任何啟動路徑**。

  | | |
  |---|---|
  | 三個啟動器已移除 `--dry` | commit `a332397`（20260923）。`HT9045_Web.cmd:13-21` 原文：**`Do not re-add it.`** |
  | exe 預設 | `f005a7c`（20260918 裁決 W906-ZEROARG）起 `dry=false`（`wb_serve.cpp:2534`） |
  | ~~`wb_serve.cpp:2556` 還認得 `--dry`~~ | **20260924 起完全移除**（使用者：「全面用 `#define SOFT_SIMULTE` 來卡控模擬或實際機台」）：傳 `--dry` 會**被拒絕並 exit 2**；行程層級的 `dry` 旗標與兩段暫存導向已關掉（`AI(W906-NODRY)`），`tools/exe_startup_gate.ps1` 不再傳它，F5 契約探針改釘「必須拒絕」。⚠ 網頁 API 的 `dryRun`（寫入配方前先預覽、操作員確認後才寫）是另一件事：**使用者 20260924 確認「事實是有這樣的需求」—— 保留，不可跟著 `--dry` 一起拆**；它不是模擬／真機開關 |
  | 模擬 vs 真機怎麼分 | **只看 `MachineType.h` 的 `#define SOFT_SIMULTE`**（V906 預設開；出貨組態用 CMake 的 `-DW906_NO_SOFT_SIMULTE=ON`）。使用者 20260918 原話：「軟體模擬僅有開啟 `SOFT_SIMULTE`，否則就是機台上能跑，**就這兩種**」 |

  ⇒ **不要再建議、也不要再加 `--dry`。**要在「模擬」與「真機」之間切換，改建置組態，不是加參數。
  ⚠ 理由不是潔癖：`--dry` 的失敗模式是**存檔 ack 回 ok、重讀「正常」、伺服器一停就整批蒸發**
    （review A1，`wb_serve.cpp:996-1006`）—— 一個看起來成功的假成功，是最難查的那種。

- ⚠⚠⚠ **20260918 裁決「甲」：這棵樹現在是實彈的。**
  使用者原話：「**我會跑機台，而且必須**」「**現在要實彈開發**」。
  `MachineType.h` 同時開了 `WB_PUMP_1203_CONTROL` 與 `WB_PUMP_1203_CONTROL_LIVE`。

  **在有 `HAVE_PCI1203` 的建置上，網頁上一次點擊可以讓線圈通電、讓伺服轉動**，
  沒有第二道 opt-in、沒有參數、沒有確認對話框。

  這**推翻**了 1203 同事寫在 `tools/pci1203_control_gate.ps1` 裡的規則
  （「不可留在 committed tree，要開就當場在 session 裡開」）。兩種安全模型都有道理，
  但不能同時成立，使用者選了前者，理由是他的另一條常設規則：
  **武裝狀態不可在機器之間分岔**，所以它必須在 git 裡。

  | | |
  |---|---|
  | **沒裝 Advantech SDK 的機器**（例：這台筆電 JIMMYCHIU-NB） | `HAVE_PCI1203` 關 ⇒ `Pci1203ControlEnable()` 回 `not linked`，指令被誠實拒絕。⚠ 20260925 更正（RULINGS 第 34 條）：判準是「有沒有裝 SDK」不是「是不是筆電」—— 舊筆電 NB2 裝了 SDK、沒有卡，建出來的 wb_serve 是武裝的（wb_serve 找得到 SDK 就開 HAVE_PCI1203，0923 Q34 裁決） |
  | 同事的機台端 | `HAVE_PCI1203=1` ⇒ **真的會動** |

  ⇒ **在任何有卡的機器上碰這棵樹之前，先確認你知道機台周圍有沒有人。**
  要反轉：`MachineType.h` 把 `WB_PUMP_1203_CONTROL_LIVE` 註解掉，**同時**把
  `pci1203_control_gate.ps1` 的 `$expectActive` 改回 `$false`，兩邊要一起動。

**語言 / 編碼 / 工具鏈**
- **C++17**（`CMAKE_CXX_STANDARD 17`）；新寫的基礎建設開 `-Wall -Wextra`。
- 原始碼 **UTF-8**。EOL 是**混合**的，逐檔保持原樣；`build.bat` 必須純 CRLF 無 BOM（cmd 掃 `goto` 承重）。
- 主 oracle 是 **MinGW g++**（唯一重現 BCB6 x87 算術，不可替換）；MSVC 是次 oracle，且**只有 MSVC 能編 MFC UI**。
- 建置一律走 `build.bat`（quick / gate / test / ui / run / msvc / clean / prune），不要自己另組 cmake 指令。
- **建置輸出不在原始碼樹裡**（20260925 起，`AI(W906-OBJROOT)`）：相對的 build dir（`build`、`build_dbg`、`V906_BUILD_DIR=build_x`）一律落在 `<repo>\Obj\V906\`（主 checkout＝`D:\HT9045\Obj\V906`，worktree＝`<worktree>\Obj\V906`；可用 `V906_OBJ_ROOT` 覆寫）。F5 的 exe 在 `D:\HT9045\Obj\V906\build_dbg\wb_serve.exe`。`build_nonoracle.bat`／`build_x64.bat`（EastSun 的 1203 路線）沒改，仍在樹內。

**AI 修改註解**：`//AI(W906-<工作代號>) YYYYMMDD: 描述`（例 `AI(W906-PT-W1)`、`AI(W906-GateA-0)`、`AI(W906-WebBridge-Tcp)`）。

**完成後自我審查**
- 翻譯以**忠實**優先於「寫得更好」；golden 不合理處照翻並在註解說明，改行為須使用者決定。
- 驗收比對 **ctest 失敗清單**，不看數字或百分比。常駐失敗是**五個**（非回歸）：`config_db`、`IniFiles`、`ini_helpers`、`config_loaders`、`GA1_ReadGeneralIni`。
  ⚠ 20260917 更正：`dfm2rc_idempotent` 已從清單拿掉 —— 20260916 夜間跑了四輪雙 gate，它**每一輪都通過**（Debug 153.79 s／Release 49.38 s）。把一個會過的測試留在「既有失敗」清單上，等於授權自己未來忽略它的真回歸。
- 「build 綠」不等於「接上了」；交付數字只在全新 build dir 量，收工最後一個動作是 build 不是 commit。
- 引用完成度百分比必附分母與單位。

### 安全關鍵變更（運動控制 / IO / 互鎖 / 模式切換 / 警報 / 執行期設定）
- 修改前先描述風險與影響範圍，修改後提供回歸與驗證建議。
- 觸及 `system/`、`config/`、`CFG/` 的 `.ini/.csv/.dat` 等執行期設定屬高風險，預設只讀並需備份確認（見寫入邊界 hook）。

### 報告 / 文件輸出
- 報告檔名須以 `RD5軟體` 前綴開頭，並含 `YYYYMMDD_HHMMSS`。
- `.svn` 資料夾一律不搜尋、不修改。

---

## HT9050 開發知識與極限規則（Ifor01 20261002）

> 已搬到 `.claude/skills/ht9050-hw/references/dev-knowledge-and-limits.md`（Jimmy 1002 22:4x §0 第 57 項＝A：CLAUDE.md 每個 session 都整份讀，越長越慢；內容照原文搬，`RULINGS_20261002.md` 第 23 條）。做 HT9050 的溫控／序列埠／1203／ctest 沙盒之前先讀那一份。

---

## Agent 分流

> Steven 20261005 11:5x 放行、12:0x 核准（ST01-E session「要，照 ST01-M 說的做」）；Jimmy RULINGS_20261005 第 5 條同意（主路由寫在本檔、不另建主路由 agent 檔；三支區域 agent 照提案 §1）。依據 `docs/handoff/ST01_AGENT_SKILL_REORG_PROPOSAL_20261005.md`（v906/steven-handoff 分支）§1 與 §4（FROM_STEVEN §3 1005 13:2x 更正列）。
> Steven 定的新前綴是 `hpi-`（不是 `ht-`）；放行範圍只有 §1 agent 架構與通訊類 skill（ST02-C22）。主題 skill 重構、客戶橫切層、封存等 Steven／Jimmy 討論後再做——在那之前 skill 一律用目前的名字。

先判斷問題屬於哪一區，再交給對的 agent：

| 問題 | 交給 | agent 檔 |
|---|---|---|
| HT9045 程式（V899／V906／V912）、流程、馬達、IO、通訊、網頁 HMI、溫控、告警、建置出貨、客訴、週報 | **ht9045-agent**，再依版本派 ht9045-v899／ht9045-v906／ht9045-v912／case-coordinator／weekly-report | `D:\HT9045\.claude\agents\ht9045-agent.md` |
| HT9050 機台事實：硬體、馬達參數、IO 表、1203 回原點、MotionView 9050、自動測高、乾跑、跟 9045 的差異 | **ht9050-agent** | `D:\HT9045\.claude\agents\ht9050-agent.md` |
| 跨 session／跨機台：交接檔、巡檢、代跑 build、todo／done 登記、記錄員、日報、派工 | **co-work-agent** | `D:\HT9045\.claude\agents\co-work-agent.md` |
| RD5 入口網站（repo 9050motionview）：頁面、索引產生器、日報發布、部署 | **rd5-portal-agent**（入口網站歸 ST02-M，走 MR） | 入口網站 repo 的 `.claude\agents\rd5-portal-agent.md`（St01／St02：`D:\RD5-Portal`；筆電：`D:\HT9045-Index`） |

**跨區工作的先後**
- HT9050 機台問題要改程式：先 **ht9050-agent** 查事實（附函式＋Task、出處），再由 **ht9045-v906** 改 C++。不要讓改程式的 agent 自己猜機台事實。
- 改完要登記、通知或交接：最後交 **co-work-agent**（FROM_STEVEN、todo／done、日報）。
- 要放上入口網站的文件：內容在 HT9045 做完，發布交 **rd5-portal-agent**。
- 一個問題跨兩區時，先做「事實／規格」那一區，再做「改動」那一區；不要兩區同時改同一件事。

**每支 agent 開頭第一張表就是 skill 清單**：「開工必讀」那一欄開工就用 **Skill 工具**載入，「依情境再讀」看主題再載。St02 正在整理的 hpi-gpib／hpi-secs／hpi-rs232 還沒進樹前，標「(整理中，舊名照用)」，照用 gpib-*／ht9045-secsgem／ht9045-secs-sem／rs232-* 舊名。

既有 5 支 agent（ht9045-v899、ht9045-v906、ht9045-v912、case-coordinator、weekly-report）保留不改，掛在 ht9045-agent 底下。
