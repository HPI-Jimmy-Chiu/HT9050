---
name: hpi-tray-flow
description: "整合 HT9045、HT9050 及其他 Handler 的 Tray／CatchTray、TrayArm、Loader／Empty／Color／Auto 補盤取放、分盤與升降。追 DoCatchTray、DoLoad、DoReceiveAutoTray、LOAD_Z_USE_MOTOR、LOAD_Y_USE_MOTOR、DoLoad_9050、C_MobileTrayTableSelect、P04、P27、SortingBinTray／Fix3FullTray、JAM1101 與補退盤互鎖時使用；先比較共同項及機型差異。"
---

# Handler Tray 流程

推送基準與後續裁決：[最新main整合](references/main-integration-20261006.md)。

HT9050 與其他機型共用此入口；先確認機型、執行版本及流程分派，再按需要讀 references。

## 必要限制

- 先讀 [共同項與差異](references/common.md)，同名欄位／軸／Task 不能代替機構與版本查證。
- 通用三段舉升、HT9050 逐層 Z 馬達與交接氣缸、Double-Belt 四角爪是不同路徑。
- 核對層／軌陣列索引、教導值來源、Motor Enable、真實感測與 fHasTray；不要只清資料解鎖。
- 分清生產收料換盤、收工批次退盤與 P27 IC 整盤；同一 JAM code 可能對應不同觸發條件。
- 安全互鎖與原裁決完整保留。歷史案例維持來源版本；此Skill未授權啟動機台、HOME或更改設定。

## 按問題選路

| 問題 | Reference |
|---|---|
| 共用機制、機型對照、當前查證 | [共同與差異](references/common.md) |
| HT9045 系列主流程與來源版本 | [HT9045 分支](references/machines/ht9045.md) |
| HT9050 逐層 Z、取盤／放盤交接 | [HT9050 分支](references/machines/ht9050.md) |
| CatchTray／DoLoad／批次退盤／P04 | [取放與供退盤](references/flow/index.md) |
| Tray Z／Tray Y／分盤機構 | [機構樹](references/mechanisms/index.md) |
| P27／Fix3FullTray／OutArm IC整盤 | [整盤分支](references/sorting/index.md) |
| CC_SCC／其他客戶條件 | [客戶表](references/customers.md) |

## 查證與交付

從 dispatcher 追到 Task／case、命令與到位、資料交棒及 Alarm 條件。回報來源版本、機型／開關、軟體與實體狀態，並區分提案與已實作。每批更新 Change Log；舊名稱與路徑保留相容導覽。
