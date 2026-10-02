# HT9050 目前進度快照

> 快照日期：2026-09-26  
> 本檔是施工索引，不是永久真相。每次接續工作前，必須重新查閱 `RULINGS_YYYYMMDD.md`、`FROM_STEVEN.md`、`INBOX_QUEUE.md`、`NIGHT_REPORT.md`、實際 code／設定與測試結果。
> 分類工作明細以 [../todo.md](../todo.md) 與 [../done.md](../done.md) 為權威。

## Git 分支工作上下文

> 最後查證：2026-09-26，於 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0` 執行 `git fetch origin --prune` 後取得。分支是動態資訊；開始施工前仍須重新 fetch，並記錄當次 HEAD 與 dirty files。

| 分支／ref | 類型 | 目前用途或判讀 | 查證狀態 |
|---|---|---|---|
| `v906/steven-cbridge-review6` | 本機＋`origin` | 目前工作分支；最新查證 HEAD `9ffb0e5e004b16a4e46120656644838dae136005`，相對 `origin/main` ahead 33／behind 0 | `VERIFIED`（Git ref）；目標子樹有 3 個 tracked dirty、65 個 untracked |
| `origin/main` | 遠端追蹤 | 主整合線；當時 HEAD `b5fb53be0d2ae97ebe420f149994209d14da11f9` | `VERIFIED`（Git ref） |
| `feat/v912-port`／`origin/feat/v912-port` | 本機＋遠端追蹤 | 舊移植／整合分支；不可因 `origin/HEAD` 指向它就視為目前 HT9050 施工基線 | `VERIFIED`（Git ref） |
| `origin/v906/nb2-assist` | 遠端追蹤 | NB2 協作與查證工作 | `VERIFIED`（Git ref） |
| `origin/v906/steven-gpib-widget` | 遠端追蹤 | TesterComm／GPIB 工作線；P0 commit `299283bc` 已是目前 HEAD 祖先；P1 `3e8534c9` 不是目前 HEAD 祖先 | `VERIFIED`（Git ancestry） |
| `origin/v906/steven-handoff` | 遠端追蹤 | Steven 交接紀錄工作線 | `VERIFIED`（Git ref） |
| `backup/steven-20260922` | 本機 | 2026-09-22 保全點；只作回溯，不作整合基線 | `VERIFIED`（Git ref） |
| `machine/integ-ioweb` | 日誌提及，Git ref 不可見 | 本次 fetch 後，本機與 `origin` 都找不到；可能位於其他 repo／機台、尚未 push，或名稱已退場 | `UNKNOWN`；取得 repo、remote 或 commit 前不得引用為完成證據 |

分支名稱只代表工作位置，不代表功能完成。進度升級仍須依 commit 內容、build/test 與機邊證據判定。

## 狀態定義

- `VERIFIED`：有本次適用環境的測試或機台量測證據。
- `IMPLEMENTED`：程式已存在且至少可建置／靜態檢查，但尚未取得足夠機邊證據。
- `INTEGRATING`：跨層接線中，部分 producer／contract／consumer 或副作用未閉環。
- `SPEC_ONLY`：有資料、提案或 Skill，尚未證明產品實作。
- `BLOCKED`：缺設備、依賴或決策，且無安全預設可繼續。
- `NOT_STARTED`：已知需要，但尚未施工。
- `RETIRED`：舊路線已裁決退場，不得當成目前架構。
- `UNKNOWN`：證據不足，禁止推定。

## 已確立的架構裁決

| 項目 | 狀態 | 目前判斷 | 權威來源／下一步 |
|---|---|---|---|
| UI 與控制分工 | `VERIFIED`（設計） | HTML/JS 是 HMI；C++ 是真實狀態、流程、互鎖與副作用 | `CLAUDE.md`、V906 rulings；持續以垂直切片驗證 |
| 目標 C++ 樹 | `VERIFIED`（治理） | `HT9011UC_Cpp_V3.33.906.0`，C++17／UTF-8／CMake | `AGENTS.md`、`CLAUDE.md` |
| 模擬邊界 | `VERIFIED`（契約） | 只由建置期 `SOFT_SIMULTE` 決定；CLI `--dry` 已退場 | `RULINGS_20260925.md`；不得重加 `--dry` |
| Recipe 預覽 | `VERIFIED`（契約） | API `dryRun` 是存檔前預覽，必須保留 | `ht9045-json-bridge` 與 rulings |
| HT9045 繼承策略 | `VERIFIED`（設計） | golden 行為需逐切片分類 `REUSE/ADAPT/REPLACE/NEW/REMOVE` | 本 Skill `inheritance-map.md` |

## 已有基礎資產

| 項目 | 狀態 | 已有內容 | 缺口／下一 Gate |
|---|---|---|---|
| HT9050 機種識別 | `IMPLEMENTED` | `Type_HT9050` 與 `9050GPIB` 已有對應知識與程式痕跡 | 驗證真實設定選入與 capability 分流，不靠 hardcode |
| HT9050 硬體資料 | `SPEC_ONLY` | 既有 `ht9050-hw` Skill 整理 IO、Motor、DTM、PCIe-1203 等資料 | 對回實際 `machines/HT9050`／現場設定，完成 Gate 2 |
| Motion View | `IMPLEMENTED` | `Main.MotionView9050.html` 與 layout JSON／機種切換規則已有專用 Skill | 將位置、狀態、command 全部接到真實 C++；以 null/stale 顯示未知 |
| UPH 模型 | `IMPLEMENTED` | 單 Picker／單 Index 的事件排程與動畫同步已有專用 Skill | 用實機 StateRecord 校正動作時間與瓶頸 |
| WebBridge 基礎 | `IMPLEMENTED` | HTTP／WebSocket、tag、snapshot／patch、command／ack 已有框架 | 逐功能做 producer→contract→consumer→副作用閉環 |
| Recipe／Struct bridge | `INTEGRATING` | C 路整合 35 個結構（20260927，`ht9045-html-json/references/route-c-golden-bridge.md` §6：24 個有頁、ContactForce 有 PageDesc 但頁未接、10 個沒有頁；A 形狀只剩 HotPlate）＋手寫 FileRW（MainBoot／MainBackup／MainClose／MainRecord／CfgTrayPlate…，`json-bridge/references/write-inventory.md`）；owner route、`dryRun` 預覽、read-back 與 backup/restore 已大幅實作；伺服器防連點 WebCmdGuard | 10 個結構尚無頁面（要不要現在做見 todo ★ Q41）；St01 這條線只做語法檢查、沒有 build，目前 HEAD 尚未重跑完整 S-01／S-03 |
| PCIe-1203 控制 | `INTEGRATING` | V906 已有 1203 控制／監控戰役與建置能力差異規則 | 安全鏈、實卡 IO、單軸與機邊證據未能由 build 取代 |
| HTML 表單資產 | `INTEGRATING` | 已有大量 DFM→WEB／手工頁面與 pagewire 資產 | 表單存在不等於接線完成；依功能價值逐頁接真實 tag／command |

## 不可推定完成的高風險項目

| 項目 | 狀態 | 原因 | 下一步 |
|---|---|---|---|
| EMG／安全門／安全 PLC 完整鏈 | `UNKNOWN` | 未取得本快照適用機台的逐點與失效模式證據 | 完成 Gate 5，禁止以 UI／build 代替 |
| Motor brake／limit／方向 | `UNKNOWN` | 靜態 mapping 不等於實際運動與 safe stop | 清場後逐軸 Gate 6 |
| 全軸 Home | `UNKNOWN` | Home path 涉及 sensor、速度、timeout、機構順序 | 逐軸及單機構 Gate 7 |
| DTM 24-channel 實機控制 | `UNKNOWN` | 規格與 mapping 已知，但通訊、channel、過溫尚需現場證據 | 先 read-only，再單 channel，最後 Gate 9 |
| 完整 Auto 物料流 | `NOT_STARTED`／`INTEGRATING` | 各子模組與資料轉移尚未證明閉環 | 先單機構，再 Gate 8 空跑 |
| Tester／GPIB 完整生產循環 | `INTEGRATING` | 目標 HEAD 僅有 P0 骨架；歷史筆電雙組態 `TesterComm_IPC` 通過，但 P1 commit 未合入，Handler、Web 與真機 handshake 尚未完成 | 依 `TESTERCOMM_PORT_LEDGER.md` 推進 P1～P5；先協定 replay，再進 Gate 9 |
| FAT／SAT 與目標 UPH | `NOT_STARTED` | 必須等安全、運動、溫控、帶料流程穩定 | Gate 10 |

## 整機動作與測試通訊必完工矩陣

> 下列項目是最終交付範圍，不是目前完成聲明。各段在取得可重跑證據前，不得標成 `VERIFIED`；沒有足夠證據可判定目前實作程度時維持 `UNKNOWN`。

| 必完工項目 | 目前狀態 | 功能邊界 | 升級為 `VERIFIED` 的最低證據 |
|---|---|---|---|
| Loader 入料 | `UNKNOWN` | 供盤／升降／進盤、Tray 有無與到位、空料／補料、與 InArm 取料條件 | C++ 狀態機＋IO/Motor/Sensor 實測＋互鎖＋alarm/recovery＋HTML 接線；SIM、空跑、帶料皆通過 |
| InArm 動作 | `INTEGRATING` | Loader 取料、吸取確認、搬送、必要的 Hot plate／方向處理、放至 Shuttle、IC 資料交接 | 取放與在籍資料一致；掉料／真空不足／timeout 可復歸；C++→Bridge→HTML 閉環；SIM、空跑、帶料皆通過 |
| Shuttle 動作 | `INTEGRATING` | 接收 InArm、搬送／定位、與 Index 交接、離站與下一循環條件 | 位置回授與 sensor 真實；與 InArm／Index 互鎖；殘料／浮料／定位失敗可處理；SIM、空跑、帶料皆通過 |
| Index 動作 | `INTEGRATING` | 進站、Socket 定位、下壓／接觸、Tester 流程、退站、交給出料側 | 壓合與安全條件可追溯；SOT/EOT/Bin/timeout 一致；alarm/recovery 不破壞資料；SIM、空跑、帶料皆通過 |
| OutArm 動作 | `INTEGRATING` | 從出料 Shuttle／Index 取料、吸取確認、Bin routing、放至 Unloader、IC 資料交接 | Bin 與實體落點一致；掉料／滿盤／路由失敗可復歸；C++→Bridge→HTML 閉環；SIM、空跑、帶料皆通過 |
| Unloader 動作 | `UNKNOWN` | 收料、Tray 定位、滿盤／空盤交換、出盤與補盤、完成條件 | C++ 狀態機＋IO/Motor/Sensor 實測＋換盤互鎖＋alarm/recovery＋HTML 接線；SIM、空跑、帶料皆通過 |
| Tester 測試通訊 | `INTEGRATING` | 目標 HEAD 只有 GB P0：`TesterComm/` 同步信箱、單一通訊執行緒、Hub、引擎介面與版本契約；尚未接 GPIB／RS232／TCPIP 引擎、Handler、wb_serve 與真機 | P0 `299283bc` 有歷史雙組態 `TesterComm_IPC` 證據；P1 `3e8534c9` 不在目標 HEAD。後續須完成協定 replay、斷線／逾時／重送、Web 接線與真實 Tester handshake |
| 六段＋Tester 端到端完整循環 | `NOT_STARTED` | Loader → InArm → Shuttle → Index／Tester → OutArm → Unloader，含連續循環、Pause／Resume、Clean Out 與異常復歸 | Gate 8 完整空跑、Gate 9 dummy／真品與 Tester、StateRecord／Motion View／在籍／測試結果一致，並完成長時間循環證據 |

### 每段共同完成條件

每一段都必須逐項關閉以下缺口，任一項缺失都只能是 `IMPLEMENTED` 或 `INTEGRATING`：

1. **C++ 控制**：entry condition、狀態機、命令、到位、timeout、結束條件與對稱分支完整。
2. **硬體動作**：IO、Motor、Sensor、Cylinder、Vacuum、Brake、Limit 與 safe state 使用真實 backend 驗證。
3. **安全互鎖**：與前後段、共用空間、門禁、EMG、手動模式及維修模式的互鎖成立。
4. **資料一致性**：IC／Tray 在籍、Barcode／2DID、Site、Bin、Lot、計數在交接成功後才轉移，失敗可回復。
5. **Web 契約**：tag、command、ack/error、null/stale、權限與 liveness 在 C++／WebBridge／HTML 三側一致。
6. **操作與告警**：HMI 能顯示真實狀態；timeout、alarm、retry、skip、abort、home、recovery 行為明確。
7. **驗證階梯**：建置／契約測試 → `SOFT_SIMULTE` 流程 → Gate 7 單機構 → Gate 8 整機空跑 → Gate 9 帶料；不得跨級宣稱完成。

### Tester 測試通訊完成條件

Tester 通訊須另做 3D 閉環，不得以 thread 可啟動、socket 已連線或單一命令收到回覆視為完成：

1. **條件錨點**：Tester 介面／模式、連線狀態、Lot／Recipe、Index ready、Site enable、timeout 與 retry 上限。
2. **資料錨點**：命令／回覆格式、SOT／EOT、Site map、Bin、測試結果、錯誤碼、序號／2DID 與 correlation 資訊。
3. **行為錨點**：實際送收、Index 准入／退站、Bin routing、斷線重連、重送去重、alarm 與 recovery。

GB P0 已有可重跑自動測試，因此 P0 可視為 `IMPLEMENTED`；整個 Tester 生產循環仍是 `INTEGRATING`，在 P1～P5、協定 replay 與機邊 handshake 完成前不得升級為 `VERIFIED`。

## 目前接續工作的選擇原則

1. 先讀 `D:\HT9045\docs\handoff\FROM_STEVEN.md`，確認是否有人已認領同一區域。
2. 以能完成「C++ producer／handler → WebBridge 契約 → HTML consumer → 測試證據」的最小垂直切片為單位。
3. 優先補上會阻擋 Gate 的真實缺口，不再堆只顯示但沒有 producer 的頁面。
4. 運動與 IO 工作依 Gate 5→6→7 順序；未有安全鏈證據，不進行動作測試。
5. 完成後更新本檔：日期、狀態、commit、測試、機台／建置模式、證據路徑與下一步。

## 更新紀錄模板

| 日期 | 切片 | 原狀態 → 新狀態 | Commit／檔案 | 驗證證據 | 機台／模式 | 剩餘風險 |
|---|---|---|---|---|---|---|
| YYYY-MM-DD | 例：Loader Y read-only monitor | `INTEGRATING → VERIFIED` | commit / files | test log / measurement | HT9050 S/N、SOFT_SIMULTE on/off | 尚未測 Home |

## 狀態升級規則

- `SPEC_ONLY → IMPLEMENTED`：必須指出實作檔案與 build／static evidence。
- `IMPLEMENTED → INTEGRATING`：跨層已有部分接線，但缺 producer、consumer、command 或副作用閉環。
- `IMPLEMENTED/INTEGRATING → VERIFIED`：必須有符合目標環境的測試或機邊量測。
- Build 綠、HTTP 200、ack=`ok`、畫面有值，任何一項單獨都不能升級成 `VERIFIED`。
- 若新證據推翻舊結論，立即降級狀態並保留原因，不得為了好看維持完成標記。
