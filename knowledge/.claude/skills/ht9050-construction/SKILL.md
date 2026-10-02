---
name: ht9050-construction
description: "以 HT9045 為行為基底，接續開發採 HTML + C++ 架構的 HT9050（HP-9050）機台。用於 HT9050 新機種建構、既有半成品續作、需求差異盤點、Loader 入料、InArm、Shuttle、Index、OutArm、Unloader、Tester 測試通訊與完整 Auto 料流、IO／Motor／溫控、C++ 狀態機、WebBridge、HTML HMI、Recipe、Alarm、通訊、安全互鎖、bring-up、驗收與完成度追蹤。觸發關鍵字：HT9050 construction, 開發 HT9050, HT9045 當基底, 新機種, 機台設計, 整機動作流程, Loader, InArm, Shuttle, Index, OutArm, Unloader, Tester, 測試通訊, GPIB, RS232, SOT, EOT, Bin, Site map, HTML + C++, HTML+C++, 半完成, 接續開發, bring-up, commissioning, 完成進度, phase gate。"
---

# HT9050 Construction

把 HT9045 的已驗證行為當作 golden，以 **C++ 負責機台控制、HTML 負責 HMI** 的架構接續完成 HT9050。這不是從零重寫，也不是只做畫面；每次工作都要先辨識目前完成度，再做最小且可驗證的垂直切片。

## 權威順序

遇到互相矛盾的資訊時，依序採信：

1. 使用者最新裁決與 `HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_YYYYMMDD.md`
2. 真實程式、設定載入路徑、編譯產物與機台量測
3. `docs/handoff/FROM_STEVEN.md`、`docs/INBOX_QUEUE.md`、`docs/NIGHT_REPORT.md`
4. [references/current-progress.md](references/current-progress.md) 的進度快照
5. 既有 Skill、提案與設計文件

**進度快照不是永久事實。** 開始工作前先讀最新裁決／交接，再更新該 reference；不可只因 build 綠或 UI 能顯示就標成完成。

## 先分類再工作

| 類別 | 內容 | 優先讀取 |
|---|---|---|
| A 需求與產品 | CT、UPH、設備選配、客戶介面、驗收標準 | `design-domains.md` |
| B 機構與料流 | Loader、Hot plate、In/Out P&P、Shuttle、Index、Tray、AOI | `inheritance-map.md` + 對應流程 Skill |
| C 電控與硬體 | IO、Motor、1203、溫控、真空、氣缸、安全元件 | `ht9050-hw` |
| D C++ 控制 | 初始化、狀態機、互鎖、Home、Alarm、資料持久化 | `inheritance-map.md` |
| E HTML HMI | 頁面、元件狀態、命令、警報、可及性 | `ht9050-motionview-layout` 或相關 web 文件 |
| F 橋接契約 | tag、snapshot/patch、command/ack、HTTP API、liveness | `phase-gates.md` |
| G 設定與 Recipe | Gerneral.ini、IO/Mot table、Data 檔、機種能力集 | `ht9045-recipe`、`ht9045-json-bridge` |
| H 外部整合 | GPIB、RS232、SECS/GEM、ATC、Barcode/AOI/MES | 對應通訊 Skill |
| I 安全與上機 | EMG、門禁、煞車、Soft Limit、互斥、回復策略 | `phase-gates.md` + safety instruction |
| J 驗證與交付 | build、test、SIM、dry run、機邊驗證、追溯文件 | `phase-gates.md` |

完整項目清單見 [references/design-domains.md](references/design-domains.md)。

## Todo／Done 台帳

- 權威待辦：[todo.md](todo.md)；權威完成紀錄：[done.md](done.md)。兩檔都固定使用上面的 A–J 分類。
- 待 Steven 決定的題目：未決斷 [references/decisions-pending.md](references/decisions-pending.md)、已決斷 [references/decisions-decided.md](references/decisions-decided.md)（20260927 從 todo.md ★ 節拆出）。
- **需要 Steven／人工審核的項目**（Steven 20260930 17:1x「需要人工審核的要通知ST01-M寫到skill的參照裡面喔!」；St01＋St02，ST01-M 維護；上機要看／看得到的行為改變／規則例外）：[references/human-review.md](references/human-review.md)。
- **寫給 Steven 的決策題格式**（Steven 20260927 21:xx：「給我的決策文件裡面，相關的檔案都要使用絕對路徑，不要使用代號；功能也是要白話說明，淺顯易懂的方式」）：decisions-pending／decided 與任何請 Steven 決定的題目——① 檔案每一處都寫完整絕對路徑＋行號（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\...` 移植樹、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\...` golden V912、`D:\HT9045\web\...` 網頁），只寫 `:行號` 或 `tools\wb_serve.cpp` 這種半路徑都不行；commit hash 後面附主要檔案的絕對路徑。② 不用代號當說明（TS-8、CC-E2、S-a、D-f、HTSET,354、form.event、C 路…）：先白話講它是什麼，代號放括號。③ 先講功能在機台／畫面上做什麼、操作員看到什麼，再講程式；每個選項附一個具體例子。題號（Q44、R60…）保留，那是 Steven 回覆用的把手。
- 裁決進度表（第 1～21 條、S22 起的 S 編號、R927；每條的狀態與原文連結，ST01-E 維護）：[references/rulings-index.md](references/rulings-index.md)（20260927 從 `D:\docs\ops\registers\HT9045_裁決進度表.md` 搬進來）。
- 讀寫檔普查（20260926 一次性快照，歸 Jimmy／St02 的清單）：[references/io-audit-20260926.md](references/io-audit-20260926.md)。
- 開工前先選一個主分類並建立唯一 ID（如 `F-001`）；跨領域依賴只寫在同一列，不複製成多份工作。
- `todo.md` 每列必須有狀態、負責、風險、依賴／阻塞、完成證據與下一步；資訊未知時寫 `UNKNOWN`，不可留白。
- 若 ID 的範圍就是「完成實作」，程式落地且該 ID 的靜態／建置證據齊備後，可用 `IMPLEMENTED` 搬入 done；若 ID 的完成條件含 runtime、真檔或機邊驗證，缺任一證據就仍留在 todo。
- 搬移時保留 ID，補日期、commit／檔案、驗證環境與剩餘限制；同一 ID 不得同時存在兩檔。
- `VERIFIED` 必須有環境相符的實測；歷史測試須標明「歷史」，旁支／其他 worktree 成果在合入目標 HEAD 前不得列入 done。
- 完成後若發現回歸或需求擴大，保留原 done 紀錄，另開新 todo ID 並雙向引用，禁止竄改歷史結論。

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

每一段都必須同時具備：C++ 狀態機、真實 IO／Motor／Sensor／Vacuum 動作、跨機構互鎖、IC／Tray 在籍資料轉移、WebBridge tag／command、HTML 狀態與操作、timeout／alarm、retry／abort／home／recovery，以及 SIM、整機空跑、帶料驗證證據。Tester 通訊另須證明協定資料、時序、Index 狀態與 Bin routing 一致。詳細完成矩陣見 [references/current-progress.md](references/current-progress.md)，驗收順序見 [references/phase-gates.md](references/phase-gates.md)。

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

1. **讀進度**：讀 [references/current-progress.md](references/current-progress.md)，再讀最新 `RULINGS`、`INBOX_QUEUE`、`FROM_STEVEN`；記錄 HEAD、分支與未提交變更。
2. **選垂直切片**：一次只選一條可端到端驗證的鏈，例如「IO 點位 → C++ 物件 → tag → HTML LED」。
3. **做差異分類**：依 [references/inheritance-map.md](references/inheritance-map.md) 標成 `REUSE`、`ADAPT`、`REPLACE`、`NEW` 或 `REMOVE`；不得把 HT9045 多站／多吸嘴假設直接帶入。
4. **先定契約**：寫清條件、資料形狀、成功／失敗行為、alarm、timeout、rollback 與驗收證據。
5. **由底往上實作**：硬體表／驅動 → C++ primitive → 互鎖 → 狀態機 → tag/command → HTML。
6. **逐 Gate 驗證**：依 [references/phase-gates.md](references/phase-gates.md)；前一 Gate 沒證據不得宣稱後一 Gate 完成。
7. **更新進度**：只更新有證據的項目，附日期、commit、測試或機台量測；不確定就標 `UNKNOWN`，不要猜。
8. **更新台帳**：依主分類把未完成工作更新到 [todo.md](todo.md)；證據齊備後才整列搬到 [done.md](done.md)，並同步更新 [references/current-progress.md](references/current-progress.md)。

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
- 實機驗證採低能量、單致動器、單方向、可急停、有人監看；未通過前不得跑完整 Auto cycle。

## 相關 Skill 路由

- 硬體事實：`ht9050-hw`
- Motion View：`ht9050-motionview-layout`
- UPH：`ht9050-uph-model`
- JSON／C++ 橋接：`ht9045-json-bridge`
- IO／Motor／Home：`ht9045-io-control`、`ht9045-motor-control`、`ht9045-motor-home`
- Loader／Unloader：`ht9045-tray-group-mechanism`、`ht9045-load-y-use-motor`、`ht9045-catchtray-flow`
- InArm／OutArm／Shuttle／Index：對應 `ht9045-*-flow`
- GPIB／SECS／ATC：對應通訊 Skill

不要把上述 Skill 的詳細內容複製進本 Skill；本 Skill 只負責跨領域整合、順序、Gate 與進度真相。
