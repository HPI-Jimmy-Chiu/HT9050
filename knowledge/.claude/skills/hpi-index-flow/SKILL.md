---
name: hpi-index-flow
description: "整合 HT9045、HT9050 與其他 Handler 機型的 Index 下壓與測試流程：DoTestHeadMotor、FinePitch、DoTestY、前後 Test Head、Index1/Index2、EP D24/D26、Contact Test／Pick interlock、Auto Height、Cycle Time 與 RTC。分析共同機制與機型差異、追 Task／安全互鎖／客戶分流時使用。"
---

# Handler Index 流程

推送基準與後續裁決：[最新main整合](references/main-integration-20261006.md)。

同一主題共用此入口。先確認機型、正在執行的版本及問題路徑，再讀對應 reference；不要一次載入整棵樹。

## 必要限制

- 先讀 [共同與差異](references/common.md)，分清「共同介面、各機型行為、裁決、實作狀態」。
- HT9050 僅 Z1，與有 Index Y／Z2 的機型不同；FinePitch 函式存在不代表目前 dispatcher 已呼叫它。
- 下壓前的 socket 外安全區依 Steven W-44 原裁決與實作狀態判斷；讀 [HT9050](references/machines/ht9050/index.md)。
- Contact/Pick 互鎖解除前須確認真實 IC 狀態；不得只清 IC 資料來解鎖。舊 V899 案例保留版本標記。
- EP 容差單位與 alarm gate 依版本確認；V906 live stub 不可當成已執行的完整回授流程。
- 修改仍遵守專案版本、編碼與硬體邊界；本技能未授權執行機台或改設定。

## 按問題選路

| 問題 | 讀取 |
|---|---|
| 共通呼叫介面、機型對照、證據版本 | [共同與差異](references/common.md) |
| HT9045／雙 Index／前後臂／32 Site | [HT9045 流程樹](references/machines/ht9045/index.md) |
| HT9050 FinePitch／Z1／W-44 安全區 | [HT9050 流程樹](references/machines/ht9050/index.md) |
| EP D24/D26、kPa、回授與 Alarm | [EP 分支](references/ep/index.md) |
| 客戶條件與版本差異 | [客戶對照](references/customers.md) |
| Cycle Time、UPH、RTC、其他子狀態機 | [流程與量測分支](references/flow/index.md) |
| 舊 Contact/Pick 互鎖案例 | [V899 原始案例](references/history/ht9045-contact-pick-interlock/index.md) |

## 查證與交付

1. 用機台版本選來源碼；歷史 reference 的行號只是該版本錨點，回到目前程式核對。
2. 分別記錄命令、Task cursor、HasIC 容器、實際 I/O 與互鎖條件；裁決尚未實作時明列缺口。
3. 回報來源版本、入口／case、影響機型、客戶開關與驗證方式；每批更新 Change Log。

原技能名稱及舊引用路徑保留為相容入口。完整原文依主題拆分於 references，保留原始機型、日期與裁決。
