---
name: ht9050-construction
description: "以 HT9045 為行為基底，接續開發採 HTML + C++ 架構的 HT9050（HP-9050）機台，並維護 HT9050 施工台帳（待辦／完成／待決與已決題目／人工審核清單／裁決進度表）。用於 HT9050 新機種建構、既有半成品續作、需求差異盤點、Loader 入料、InArm、Shuttle、Index、OutArm、Unloader、Tester 測試通訊與完整 Auto 料流、IO／Motor／溫控、C++ 狀態機、WebBridge、HTML HMI、Recipe、Alarm、通訊、安全互鎖、bring-up、驗收與完成度追蹤，以及新增待辦、結案搬 done、開題問 Steven、登記裁決、登記上機清單。觸發關鍵字：HT9050 construction, 開發 HT9050, HT9045 當基底, 新機種, 機台設計, 整機動作流程, Loader, InArm, Shuttle, Index, OutArm, Unloader, Tester, 測試通訊, GPIB, RS232, SOT, EOT, Bin, Site map, HTML + C++, HTML+C++, 半完成, 接續開發, bring-up, commissioning, 完成進度, phase gate, todo.md, done.md, human-review.md, decisions-pending, decisions-decided, 封存, archive, rulings-index, 裁決進度表, Q／R／W 題號, A–J 分類, IMPLEMENTED, VERIFIED, EastSun 上機清單。"
---

# HT9050 Construction

把 HT9045 的已驗證行為當作 golden，以 **C++ 負責機台控制、HTML 負責 HMI** 的架構接續完成 HT9050。這不是從零重寫，也不是只做畫面；每次工作都要先辨識目前完成度，再做最小且可驗證的垂直切片。

本 skill 有兩個用途：

1. **施工方法**：分類、權威順序、整機料流必完工項目、HTML／C++ 邊界、Gate 與狀態標準（本檔後半）。
2. **施工台帳**：`references/` 裡每天有好幾個 session 在寫的帳——待辦、完成、待決與已決題目、人工審核清單、裁決進度表。怎麼登記看〈快速上手〉與〈台帳規則〉。

## 什麼時候用

- 做任何 HT9050／V906 移植樹的功能：先查 todo／done 有沒有這件、誰在做、卡在哪。
- 要登記：新增待辦、結案搬 done、開一題問 Steven、登記 Steven（或 Jimmy）的回覆、登記要人看／要上機的項目。
- 要查：某個 ID（例 `D-021`）、題號（例 `Q34`、`R113`、`W58`）、裁決（例 `S166`、RULINGS 第 N 條）現在是什麼狀態。
- 要判斷「這件算不算完成」、能不能進下一個 Gate。

## 快速上手

| 我要… | 去哪個檔 | 怎麼做（細節見〈台帳規則〉） |
|---|---|---|
| 開工前看現況 | `references/todo.md` 該分類、`references/done.md`、`references/human-review.md`、`references/decisions-pending.md` | 再讀最新 RULINGS、INBOX_QUEUE、TO_STEVEN／FROM_STEVEN（路徑見〈權威順序〉） |
| 新增一筆待辦 | `references/todo.md` | 選 A–J 主分類，取該分類下一個號碼 `<分類>-<三位數>`，8 欄填滿，不知道寫 `UNKNOWN` |
| 結案一筆待辦 | `references/todo.md` → `references/done.md` | **合入 main**＋證據齊才整列搬；todo 刪列，done 註明「原 todo X-0nn」 |
| 開一題問 Steven | `references/decisions-pending.md` | 只問 BCB 沒答案或生意上的選擇；絕對路徑、白話、每個選項附例子 |
| 登記 Steven／Jimmy 的回覆 | pending → `references/decisions-decided.md` **檔尾** | 整題搬過去，寫原話與 RULINGS 編號；pending 原處留一行 |
| 補一題舊裁決的更正 | `references/decisions-decided.md` 檔尾 | 寫明題號；題目在封存檔也一樣加在主檔檔尾，封存檔不改 |
| 登記要人看的項目 | `references/human-review.md` | 上機要看→A（EastSun 清單）、看得到的行為改變→B、規則例外→C、等人→D |
| 查一條裁決（S 編號、第 N 條） | `references/rulings-index.md` | 狀態看進度表；原話看 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_*.md` |
| 查一個 Q／R／W 題號 | pending → decided → `references/archive/decisions-decided-202609.md` | 2026-09-30 以前裁決的（Q1～Q61、R1～R146、W1～W62）在封存檔 |
| 判斷算不算完成 | `references/current-progress.md`（完成條件）、`references/phase-gates.md` | 沒有環境相符的證據就不升級 |

大檔先 grep 再讀：`todo.md` 約 100 KB、`rulings-index.md` 約 110 KB、`human-review.md` 約 70 KB、封存檔約 540 KB。例：`grep -n "^| D-021 " references/todo.md references/done.md`、`grep -n "^#* Q34\." references/decisions-*.md references/archive/*.md`。

## 參照地圖

| 檔案 | 內容 | 誰寫 | 什麼時候讀 |
|---|---|---|---|
| [references/todo.md](references/todo.md) | 權威待辦：A–J 分類表（8 欄）；★ 節指到兩個 decisions 檔 | St01（ST01-M、ST01-E 與工程師）；St02、Jimmy 的項目由 ST01-M 代登記 | 開工前、接新工作、改狀態 |
| [references/done.md](references/done.md) | 權威完成紀錄：只收已合入 main、有證據的 | 同上 | 查某 ID 做完沒、證據在哪 |
| [references/human-review.md](references/human-review.md) | 要 Steven／人工看的項目：A 上機要看（＝EastSun 清單）、B 行為改變、C 規則例外、D 等人、已看過 | ST01-M（St01 從工程師回報整理；St02 照抄 St02-M 清單） | 交件有上機／行為改變／規則例外時；要給 EastSun 清單時 |
| [references/decisions-pending.md](references/decisions-pending.md) | 還要 Steven 回的 Q／R／W（Steven 直接讀這個檔） | Q／R：ST01-E（Steven 在 ST01-M 對話回的由 ST01-M 登記）；W：ST01-M 代 St02 | 問 Steven 前；記錄員寫 §12.4 時 |
| [references/decisions-decided.md](references/decisions-decided.md) | 2026-10-01 以後已裁決的 Q／R／W；新裁決加在檔尾 | 同上 | 查題號、查最新裁決 |
| [references/archive/decisions-decided-202609.md](references/archive/decisions-decided-202609.md) | 2026-09-30 以前裁決的 Q1～Q61、R1～R146、W1～W62 與早期「已裁決、不再問」清單（20261003 原樣搬來；封存不改；行號對照在檔頭） | 不再寫 | 查舊題號、舊的「decisions-decided.md:行號」引用 |
| [references/rulings-index.md](references/rulings-index.md) | 裁決進度表：第 1～21 條、S22 起的 S 編號、R927／R930／R1001…每條的狀態與原文連結 | ST01-E 的記錄員（`ops-st01-clerk-report`）；ST01-M 不改內容 | 查一條裁決做到哪 |
| [references/progress-st02.md](references/progress-st02.md) | St02 在 2026-09-26 的進度快照＋「待使用者裁決」備份 | 只有 Steven02 改 | 查 St02 早期背景；現況看 todo／done |
| [references/current-progress.md](references/current-progress.md) | 整機必完工矩陣、每段共同完成條件、Tester 完成條件、狀態升級規則；表內狀態停在 2026-09-26（檔頭有 20261003 查核說明） | 本 skill 維護者；只在有證據時改 | 判斷「算不算完成」 |
| [references/phase-gates.md](references/phase-gates.md) | Gate 0～10 的順序與每關最低證據 | 本 skill 維護者 | 宣稱過某個 Gate 之前 |
| [references/design-domains.md](references/design-domains.md) | HT9050 整機設計領域清單（A–J 的細項） | 本 skill 維護者 | 盤點需求、決定主分類 |
| [references/inheritance-map.md](references/inheritance-map.md) | HT9045→HT9050 五類繼承決策（REUSE／ADAPT／REPLACE／NEW／REMOVE）＋切片紀錄模板 | 本 skill 維護者 | 每個垂直切片開工前 |
| [references/io-audit-20260926.md](references/io-audit-20260926.md) | 讀寫檔普查（一次性快照，HEAD `8fad1522`）；todo G-022／G-023 指到這裡 | 不再更新 | 查 golden 讀寫點歸誰 |

## 台帳規則

這一節是台帳規則唯一的一份；todo.md、done.md 開頭只留指標。decisions-pending.md、human-review.md 開頭的說明是寫給 Steven 看的用法，應跟這裡一致；有出入以這裡為準，並順手修正那邊。

### 共通

- 只增不改：已登記的列與裁決紀錄不刪、不改字；要更正就加「⛔ 更正（日期、誰）」；結論被推翻就開新 ID／新題，兩邊互相引用。
- 列與標題的格式不要改（`| ID | … |`、`### Qnn.`／`#### Qnn.`），別人的腳本靠它切欄、數題。
- 檔案 UTF-8 無 BOM；repo 存 LF（`core.autocrlf=true`，工作樹看到 CRLF 是正常的）。
- 語言：繁體中文（Steven 讀）；FROM_STEVEN 與給 Jimmy 的列可以英文。寫給 Steven 的一律完整絕對路徑：移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`、golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`、網頁 `D:\HT9045\web\`。
- 這些檔大約每 20 分鐘就有人 commit：改之前先 pull；在共用工作樹 `D:\HT9045` commit 前照 `ops-ht9045-handoff` 的規則先確認、先知會 ST01-E。

### todo.md／done.md

- ID `<分類>-<三位數>`（例 `F-001`），分類用下面〈先分類再工作〉的 A–J。一件工作一個主分類、一個 ID；跨領域寫在「依賴／阻塞」，不複製成多列。
- todo 8 欄（`awk -F'|'` 的 NF＝10）：ID｜工作｜狀態｜負責｜風險｜依賴／阻塞｜完成證據｜下一步。每欄必填，未知寫 `UNKNOWN`。
- done 7 欄（NF＝9）：ID｜完成項目｜最終狀態｜日期｜commit／檔案｜驗證證據｜環境／限制。
- todo 的狀態：`INTEGRATING`、`SPEC_ONLY`、`BLOCKED`、`NOT_STARTED`、`UNKNOWN`；`IMPLEMENTED`＝程式已在，但證據不齊或還沒合入 main。
- **沒合入 main 的不進 done**：旁支、其他 worktree、未合入的 commit、口頭宣告都只能留在 todo（標 `IMPLEMENTED`）。合入 main 而且證據齊，才把整列搬到 done 的同一分類：todo 那列刪掉（不要只劃刪除線），done 註明「原 todo X-0nn」，補日期、commit／檔案、驗證環境與剩餘限制。同一 ID 不得同時在兩檔。
- 從沒進過 todo 的完成項目，用 x1xx 編號直接進 done（例 E-102、H-105）。
- 證據等級：ctest／建置／靜態檢查＝`IMPLEMENTED`；`VERIFIED` 要環境相符的實測，必填日期、commit、環境／模式、測試輸出；舊證據寫明「歷史」。完成條件含 runtime、真檔、瀏覽器、跟 BCB6 對照或機邊驗證的項目（驗證型），缺證據就留在 todo，即使程式已進 main。
- done 之後發現回歸或需求擴大：done 原列保留，另開新 todo ID，兩邊互相引用。

### decisions-pending.md／decisions-decided.md

- 題號：**Q**＝St01 要 Steven 決定；**R**＝St01 已照建議先做、Steven 可推翻；**W**＝St02 的題。題號不重用；pending、decided、封存檔用同一個題號。
- 少開題：照 BCB 就有答案的直接做、記一筆（Steven 20260929「你問題也太多了!」）；912 比較好就照 912，註解寫 906 行號與做法＋912 修正行號（Steven 20261003 常設規則；客戶專屬的仍要問）。只有 BCB 沒答案或生意上的選擇才開題。
- 寫法（Steven 20260927 21:xx）：每個檔案每一處都寫完整絕對路徑＋行號，commit 後面附主要檔案的絕對路徑；代號（TS-8、CC-E2、form.event、C 路…）先白話說是什麼，代號放括號；先講機台／畫面上會發生什麼、操作員看到什麼，再講程式；每個選項附一個具體例子。
- Steven（或 Jimmy）回了：整題從 pending 搬到 decided **檔尾**，前面加小標 `### YYYYMMDD HH:MM Steven 裁決：Qnn（從 decisions-pending.md 搬來）`；「目前狀態」改成 `**已裁決（日期時間，在哪邊，誰轉述）**：「原話」⇒ 意思`，附 RULINGS 編號；pending 原處留一行「（Qnn 已由 … 裁決，搬到 decisions-decided.md。）」。
- 舊題要補 ⛔ 更正或 ⚠ 追問：加在 decided 主檔檔尾，寫明題號（題目在封存檔就註明）。封存檔不改；記錄員 `grep "⚠ 還要 Steven 回"` 只掃主檔。
- 維護：Q／R＝ST01-E（Steven 在 ST01-M 對話裡回的，ST01-M 直接登記）；W＝ST01-M 代 St02 登記。兩邊都會改這兩個檔，動之前先說一聲。
- 再封存：等一個月份過完、而且沒人在補那個月的題，再把整月的題原樣搬到 `references/archive/decisions-decided-YYYYMM.md`，原處留一行指標，主檔檔尾不動。

### human-review.md

- ST01-M 維護。工程師推送時，有「照 golden 但要上機看」「看得到的行為改變」「規則例外」就在回報裡標出來（Steven 20260930 17:1x），ST01-M 同一輪加進來；St02 的照抄 St02-M 清單 `D:\HT9045\docs\handoff\ST02_HUMAN_REVIEW_20260930.md`（`v906/steven-handoff` 分支）。
- A 上機要看＝**EastSun 的上機清單**（Steven 20261001 09:4x「需要上機驗證的, 都是請Eastsun處理」）：新增 A 項同一輪在 FROM_STEVEN §3 請 Jimmy 轉 EastSun，附分支／commit，先跑完兩組態 gate。B 看得到的行為改變；C 規則例外（已跟規則擁有者講好）；D 還在等人的。
- 編號 A1、B1、C1… 往下接，不重用；每項寫做了什麼、為什麼要人看、要看什麼、程式在哪、commit。
- Steven 看過或上機驗過的，整列搬到「已看過」並寫日期，不要刪。

### rulings-index.md

- ST01-E 的記錄員每段落更新（最後更新、主表狀態、狀態計數、最近變動只留 20 行），規則在 `ops-st01-clerk-report`；其他人不改內容，發現錯誤告訴 ST01-E。

## 權威順序

遇到互相矛盾的資訊時，依序採信：

1. Steven／Jimmy 的最新裁決：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_YYYYMMDD.md`；每條的狀態看 [references/rulings-index.md](references/rulings-index.md)。
2. 真實程式、設定載入路徑、編譯產物與機台量測。
3. 交接與佇列：`D:\HT9045\docs\handoff\TO_STEVEN.md`（main）、`D:\HT9045\docs\handoff\FROM_STEVEN.md`（`v906/steven-handoff` 分支；唯讀快照在 `D:\HT9045_handoff\`）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\INBOX_QUEUE.md`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\NIGHT_REPORT.md`。
4. 台帳：todo／done／human-review／decisions 檔（每天在動，目前進度以這裡為準）。
5. [references/current-progress.md](references/current-progress.md) 的框架與 2026-09-26 快照、既有 Skill、提案與設計文件。

**台帳與快照都不是永久事實。** 開始工作前先讀最新裁決／交接；不可只因 build 綠或 UI 能顯示就標成完成。

## 先分類再工作

| 類別 | 內容 | 優先讀取 |
|---|---|---|
| A 需求與產品 | CT、UPH、設備選配、客戶介面、驗收標準 | `references/design-domains.md` |
| B 機構與料流 | Loader、Hot plate、In/Out P&P、Shuttle、Index、Tray、AOI | `references/inheritance-map.md` + 對應流程 Skill |
| C 電控與硬體 | IO、Motor、1203、溫控、真空、氣缸、安全元件 | `ht9050-hw` |
| D C++ 控制 | 初始化、狀態機、互鎖、Home、Alarm、資料持久化 | `references/inheritance-map.md` |
| E HTML HMI | 頁面、元件狀態、命令、警報、可及性 | `ht9050-motionview-layout` 或相關 web 文件 |
| F 橋接契約 | tag、snapshot/patch、command/ack、HTTP API、liveness | `references/phase-gates.md` |
| G 設定與 Recipe | Gerneral.ini、IO/Mot table、Data 檔、機種能力集 | `ht9045-recipe`、`ht9045-json-bridge` |
| H 外部整合 | GPIB、RS232、SECS/GEM、ATC、Barcode/AOI/MES | 對應通訊 Skill |
| I 安全與上機 | EMG、門禁、煞車、Soft Limit、互斥、回復策略 | `references/phase-gates.md` + safety instruction |
| J 驗證與交付 | build、test、SIM、dry run、機邊驗證、追溯文件 | `references/phase-gates.md` |

完整項目清單見 [references/design-domains.md](references/design-domains.md)。

## 不可省略的整機料流交付

下列八項全部是 HT9050 的**應完工項目**，不得因單一頁面、單一軸、單一機構、局部狀態機或單一通訊命令可運作，就宣告整機完成：

1. Loader 入料與供盤。
2. InArm 取料、搬送與交接。
3. Shuttle 搬送、定位與前後段交接。
4. Index 進站、測試定位、接觸／測試與退站。
5. OutArm 取料、Bin 路由與交接。
6. Unloader 收料、換盤、滿盤處理與出料。
7. Tester 測試通訊，包括介面選擇、連線、SOT／EOT、Site map、Bin、timeout、retry、斷線與復歸。
8. Loader → InArm → Shuttle → Index／Tester → OutArm → Unloader 的端到端完整循環。

每一段都必須同時具備：C++ 狀態機、真實 IO／Motor／Sensor／Vacuum 動作、跨機構互鎖、IC／Tray 在籍資料轉移、WebBridge tag／command、HTML 狀態與操作、timeout／alarm、retry／abort／home／recovery，以及 SIM、整機空跑、帶料驗證證據。Tester 通訊另須證明協定資料、時序、Index 狀態與 Bin routing 一致。完成矩陣見 [references/current-progress.md](references/current-progress.md)，驗收順序見 [references/phase-gates.md](references/phase-gates.md)。

## HTML + C++ 責任邊界

| C++ 擁有 | HTML 擁有 |
|---|---|
| 機台真實狀態、狀態機、互鎖、IO/Motor、警報判定、Recipe 實體讀寫 | 顯示、輸入、導覽、操作回饋、視覺化、可及性 |
| tag 的型別、liveness、來源與更新時機 | tag 到元件的映射、格式化、空值呈現 |
| command 驗證、權限、執行、ack/error | command 發送、等待狀態、錯誤呈現、防重複操作 |
| 所有會讓機台動或改變真實檔案的副作用 | 不得自行模擬「成功」或維護第二份機台真相 |

Web 工作強制做 3D check：

1. **條件錨點**：機種、模式、權限、狀態與 guard。
2. **資料錨點**：tag／payload／型別／null 語意／設定來源。
3. **行為錨點**：實際 API、C++ 副作用、UI 結果與錯誤路徑。

## 接續開發流程

1. **讀現況**：todo／done 的相關分類、human-review、decisions-pending，再讀最新 RULINGS、INBOX_QUEUE、TO_STEVEN／FROM_STEVEN；記錄 HEAD、分支與未提交變更。
2. **選垂直切片**：一次只選一條可端到端驗證的鏈，例如「IO 點位 → C++ 物件 → tag → HTML LED」。
3. **做差異分類**：依 [references/inheritance-map.md](references/inheritance-map.md) 標成 `REUSE`、`ADAPT`、`REPLACE`、`NEW` 或 `REMOVE`；不得把 HT9045 多站／多吸嘴假設直接帶入。
4. **先定契約**：寫清條件、資料形狀、成功／失敗行為、alarm、timeout、rollback 與驗收證據。
5. **由底往上實作**：硬體表／驅動 → C++ primitive → 互鎖 → 狀態機 → tag/command → HTML。
6. **逐 Gate 驗證**：依 [references/phase-gates.md](references/phase-gates.md)；前一 Gate 沒證據不得宣稱後一 Gate 完成。
7. **只記有證據的進度**：附日期、commit、測試或機台量測；不確定就標 `UNKNOWN`，不要猜。
8. **更新台帳**：照〈台帳規則〉更新 todo；合入 main 且證據齊才整列搬到 done；要人看的同一輪登記 human-review；整機矩陣某段真的升級時才改 current-progress.md，並附證據。

## 進度狀態標準

| 狀態 | 定義 |
|---|---|
| `VERIFIED` | 已在指定環境實測，證據可重跑 |
| `IMPLEMENTED` | 程式已存在且通過靜態／自動測試，尚未機邊驗證 |
| `INTEGRATING` | 部分鏈路完成，仍缺依賴或端到端證據 |
| `SPEC_ONLY` | 只有規格或介面，尚無可執行實作 |
| `BLOCKED` | 有明確外部依賴、決策或硬體阻塞 |
| `NOT_STARTED` | 尚未開始 |
| `RETIRED` | 已明確退場，不得當待辦復活 |
| `UNKNOWN` | 資訊互相矛盾或尚未查證 |

## 安全硬規則

- 修改運動、IO、互鎖、Home、Alarm 或執行期設定前，先列風險、對稱分支、rollback 與驗證方式。
- 真機動作前必須證明：真實 backend 已連上、感測器不是回讀輸出 cache、EMG／門禁／煞車／Soft Limit 可達且不是 stub。
- `SOFT_SIMULTE` 是建置期模擬／真機邊界；不得重新引入 `--dry` 當機台模式開關。Web API 的 `dryRun` 預覽語意是另一件事，必須保留。
- build 綠、HTTP 200、ack=`ok`、畫面數字會跳，都不等於硬體真的動作或真實檔真的寫入。
- 實機驗證採低能量、單致動器、單方向、可急停、有人監看；未通過前不得跑完整 Auto cycle。上機驗證一律經 Jimmy 請 EastSun（見 human-review A 區）。

## 相關 Skill 路由

- 硬體事實：`ht9050-hw`
- Motion View：`ht9050-motionview-layout`
- UPH：`ht9050-uph-model`
- JSON／C++ 橋接：`ht9045-json-bridge`
- IO／Motor／Home：`ht9045-io-control`、`ht9045-motor-control`、`ht9045-motor-home`
- Loader／Unloader：`ht9045-tray-group-mechanism`、`ht9045-load-y-use-motor`、`ht9045-catchtray-flow`
- InArm／OutArm／Shuttle／Index：對應 `ht9045-*-flow`
- GPIB／SECS／ATC：對應通訊 Skill
- 交接巡檢與代登記（handoff_commit、FROM_STEVEN／TO_STEVEN）：`ops-ht9045-handoff`；代登記的操作步驟在它的 `references/registrar.md`，規則以本檔〈台帳規則〉為準
- 裁決進度表、ChangeLog §11／§12、日報：`ops-st01-clerk-report`
- St01 評估報告（Q 題背後的分析）：`ht9050-st01-evaluations`
- St02 的工作流程：`ht9045-st02-workflow`

不要把上述 Skill 的詳細內容複製進本 Skill；本 Skill 只負責跨領域整合、順序、Gate、進度真相與台帳。
