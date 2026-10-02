---
description: "HT9045 V906 MON1203-WEB 戰役：執行一個完整波次（冷啟動檢查 → 選標的 → 交付 → 驗收 gate → commit → 更新 RESUME → 開下一波）。目標是讓 1203 的 IO 與 Motor 在 web 上真的可監控，並修掉阻擋它的既有缺陷。搭配 /loop 可自動連續推進。關鍵字：MW-W, 1203, pci1203, 監控頁, EtherCAT, x64 heap, exe_startup_gate"
argument-hint: "[波次（MW-3/MW-4/MW-5/MW-6/auto）]"
---

先載入 **pt-wave-loop** skill 取得共用的翻譯／驗收／安全政策，再照下面執行。
權威計畫書：`HT9011UC_Cpp_V3.33.906.0/docs/MON1203_WEB_CAMPAIGN_PLAN.md`。
使用者輸入：$ARGUMENTS

工作目錄：`HT9011UC_Cpp_V3.33.906.0`。
⚠ **本戰役橫跨三個 repo，而它們是分開的**（20260908 實測）：
`HT9011UC_Cpp_V3.33.906.0`（C++／文件）、**`D:\HT9045\web`（自己一個 repo）**、
`D:\BCB6_1203_UI`（**不是 repo**，改動只在磁碟上）。
`D:\HT9045` 根目錄**不是** repo，所以 `CLAUDE.md`／`.claude\`／`installer\` 也不受版控。
**每一波收工時要逐 repo 分別 commit，不要以為一次 `git status` 看得到全部。**

---

## 步驟 0 — 冷啟動檢查（先跑，不是先讀 RESUME）

1. **三個 repo 各跑一次 `git status`**（見上）。在製工作先收完再開新波 ——
   這條規則在這棵樹已四次撿到未 commit 的在製工作。
   ⚠ `docs/FUSE_CAMPAIGN_PLAN.md` 若是 modified，那是**另一個 session 的**工作，
   **不要 commit、不要編輯**。
2. 讀 `docs/DEVLOG.md` 的**第一個** `### 🔖 RESUME（` 命中（本檔是「新波次插中段、
   往下遞減」的排法，**檔尾那則是一百多波以前的**），再讀計畫書 §4 佇列。
3. **確認沒有殘留監聽者**：
   ```powershell
   netstat -ano | Select-String ':8045|:8046'
   Get-Process | Where-Object { $_.ProcessName -match 'wb_publish|wb_gateway|wb_serve' }
   ```
   ⚠ 判準是「持有埠的 PID 等於你剛啟動的那一支」，**不是 HTTP 回 200** ——
   舊行程會回 200 ＋ 正確標題，給你一個來自錯二進位的綠燈。

## 步驟 1 — 選標的

- `$1` 指名 → 做那個。
- `$1` 空或 `auto` → 照計畫書 §4 佇列，但**先問「它的前提成立了嗎」**：

| 波次 | 前提 | 前提不成立時 |
|---|---|---|
| **MW-6** 軌道 A（`exe_startup_gate.ps1`） | 無前提，**現在就可做** | — |
| MW-6 軌道 B（找越界寫入） | 軌道 A 先落地（否則沒有紅燈可以判「修好了沒」） | 先做軌道 A |
| MW-3（馬達名上 wire） | `pci1203.axScan.byIdMode == true` **且** `axesOpened > 0` | **跳過**，不要為了畫面去改資料表 |
| MW-4（IO 位元命名） | `pci1203.scan.found > 0` | **跳過**，同上 |
| MW-5（`wb_serve` 旗標） | 無前提；或併入 FUSE（計畫書 D6） | — |

⚠ **MW-3／MW-4 的前提要「量」不要「猜」**：跑一次
`build_nonoracle\wb_publish.exe`（`WB_PUBLISH_SECONDS=25`）看它印的
`slavesFound` / `axByIdMode`。20260908 實測是 0 / NO。
⚠ **前提不成立時不要「先做一半」** —— `byIdMode=false` 時顯示馬達名是
「拿真實編碼器值配錯名字」，比空白危險。

## 步驟 2 — 交付

- **gate／工具波**（MW-6 軌道 A）：主迴圈自做。新 gate **必須先是紅的** ——
  第一次就綠要先懷疑它沒在測東西（本樹鐵則：不可能當掉的 gate 不是 gate）。
- **web 波**：邏輯寫進 `web/js/<page>/*.js` 的**純函式**（tag Map → DOM），
  接線留在頁面檔。判準：**探針要補幾個 stub 才碰得到邏輯？>0 就是黏在一起了。**
  每個 web 波都要有一支 `web/tools/jsprobe/probe_*.mjs`，且 fixture 用**真實擷取**
  （從 `tcp://127.0.0.1:8046` 抓，不要手寫 —— 手寫會把「我以為 wire 長怎樣」編碼進去）。
- **C++ 波**：照 pt-wave 的翻譯紀律。⚠ 觀測器的**唯讀白名單不得擴張**；
  真要加進入點必須同時更新 `tools/pci1203_readonly_gate.ps1` 的期望並說明它為何仍是唯讀。

## 步驟 3 — 驗收 gate

**必跑，逐項貼結果**：

```
tools\pci1203_readonly_gate.ps1        # 白名單未破，close 有 ownership 守衛
tools\macro_order_gate.ps1             # 新巨集的測試點必須是「安全」
tools\pe_truncation_check.ps1          # exit 0 不是產物完整的證據，這個才是
tools\production_audit.ps1             # ⚠ 跑之前先 -Snapshot
tools\devlog_resume_gate.ps1           # RESUME 與最新波次同號
web\tools\jsprobe\run.ps1 -All         # 7 支全跑＋總計；-Probe <單支> 只用來除錯
```

- **建置**：`build_x64` ＋ `build_nonoracle` 各一次，`Built target` 要看到。
  ⚠ **預設關閉的 `#ifdef` 臂從沒被型別檢查過** —— 動到 `#ifdef` 就把**每個組合**
  都 `-fsyntax-only` 編一次（`INSTALL_1203_MONITOR` × `WB_PUMP_WITH_CONFIG` × SIM 臂）。
  ⚠ `build_sim_nonoracle` **這台不存在**（實測只有兩個 build 目錄）；
  用 `-DSOFT_SIMULTE` 的語法檢查等價替代，**並在報告裡明說沒有做完整 SIM 建置**。
- **產物新鮮度用內容證明，不要只看 mtime**：新字串在不在、舊引導字串消失了沒。
- **e2e 用 32-bit 線**（`build_nonoracle`）。⚠ 理由不是偏好：
  **x64 的 `wb_publish.exe` 有一個 `main()` 之前的 heap 破壞**，觸發條件是
  環境區塊多了幾個位元組（計畫書 MW-6／CLAUDE.md 專章）。x64 上跑 e2e 會得到
  「gateway 起來、HTTP 200、feed=down」，那不是你這一波弄壞的。
- **`.js` 的 Content-Type 要驗**：錯了 ES module 頁面**整片空白**、只有 console 有錯。

## 步驟 4 — commit（逐 repo）

一顆 commit 一件事；訊息寫「量到什麼」，**含自己犯的錯與更正**；
`git add` **逐檔點名**（多 session 共用 repo，嚴禁寬 glob 與 `git checkout` 還原）。
⚠ web 的改動 commit 在 `D:\HT9045\web`；C++ 與文件在移植樹；
`D:\BCB6_1203_UI` 與 `D:\HT9045` 根目錄無法 commit，**在 DEVLOG 裡寫明改了什麼**。

## 步驟 5 — DEVLOG + RESUME

新段落**插在檔案中段**（在上一則 `## 2026...` 之前），編號遞增，
`### 🔖 RESUME（YYYYMMDD 羅馬編號）` **必須帶日期與編號**，否則
`devlog_resume_gate.ps1` 認不得（無日期標題會被刻意忽略）。收工跑那道 gate。

## 步驟 6 — 立刻開下一波

不要停下來問「要繼續嗎」。回到步驟 0。

---

## 硬邊界（不做、不問，直接跳過並記錄）

* **`GATE G9`**（`csystem.cpp:16913`，馬達煞車釋放）與 **`MachineType.h:48`
  的 `SOFT_SIMULTE`** —— 使用者「不做不問」佇列。
  ⚠ 現況：`:48` 是註解掉的、G9 仍關著，所以「電給了、煞車還咬著」是**潛在**風險
  （實測 `IO_CARD_TYPE`／`SHUTTLE_SENSOR_TYPE`／`VacuUnitType`／`MOTION_CARD_TYPE`
  全 0、`CardModel` 45/45 SMC、`ISABase` 無 3，所以走離線 sim backend）。
  **一次資料變更就會讓它變成現行。**
* **不要為了讓監控頁面有數字而改 `Mot_Table.csv` 的 `CardModel` 或 `IO_CARD_TYPE`。**
  `CMakeLists.txt:2560-2562` 自己寫著：武裝之後「一次資料變更就是機台與真實命令運動
  之間的唯一屏障」。那是使用者裁決的事。
* **不要打開 `INSTALL_ETHETCAT()`**（`SHUTTLE_SENSOR_TYPE` ∈ {6,7} 或 `VacuUnitType`=1）
  —— 它同時武裝 shuttle sensor 與 vacuum 的**輸出寫入**，爆炸半徑最大。
* **唯讀白名單不得加入任何會下命令的進入點**（Move／Jog／MoveHome／SetSvOn／
  DaqDoSet*／ResetError）。`Acm_AxResetError` 看起來像讀取而不是。
* **不要自行改 F5 預設線**（x64 取代是使用者 20260902 的裁決）。
* **不要用「把環境變數加長」當 x64 缺陷的修法** —— 那是藏起記憶體錯誤的現形條件。
* **不要在 `build\`／`build_x64`／`build_nonoracle` 裡做實驗**，另開探針目錄。
* **golden 樹唯讀**（Big5，UTF-8 存檔會損壞）；`D:\HT9045\EXE` 永不寫入。
* **不拿 `pci1203_linkprobe --enumerate` 當隨手煙霧測試**（它檔頭自己禁止）。

## 停止條件

⚠⚠ **停之前還有一項：MW-Z 轉移包同步（使用者 20260908 指定為「任務的最後」）。**
規格在計畫書 §MW-Z。要點：把當天改動重新打包進 `D:\HT9045_fromMachine`，
**尤其是「給 AI 看的內容」** —— `CLAUDE.md`／`.claude\`（含本檔與 `mw-wave-loop`）／
`README_FOR_AI.md`／`_claude_memory_from_machine\`。
⚠ **web 是獨立 repo**，移植樹的 bundle 不含它。
⚠ **提前做等於再打一次會過期的包** —— 只在所有波次都停下來之後做。

* 前提未成立的波次全部跳過，且**沒有任何前提成立的波次剩下** → **先做 MW-Z**，
  然後停，報告在等什麼（目前是「等使用者用 Common Motion Utility 掃匯流排，
  判硬體還是軟體」）。
* 同一個根因連紅兩次 → 停、換標的、記錄。
* 需要碰硬邊界才能推進 → 停，把裁決點寫清楚交還使用者。
