# HT9045 → HT9050 繼承決策圖

HT9045 是行為基底，不是可直接複製的機構答案。每個垂直切片都必須先分類，再決定實作與驗證方式。

## 五類決策

| 類型 | 定義 | 典型例子 | 必要證據 |
|---|---|---|---|
| `REUSE` | 行為與資料契約可原樣沿用 | Alarm code 格式、共用 log 格式 | code reference、相容性測試 |
| `ADAPT` | 流程概念保留，但機構／站數／資料形狀不同 | Auto flow、Tray routing、UPH 模型 | 差異表、契約測試、機台驗證 |
| `REPLACE` | HT9050 有不同硬體或控制方法 | PCIe-1203、DTM channel mapping | datasheet、設定、driver 測試 |
| `NEW` | HT9045 沒有的機構、頁面或客戶需求 | HT9050 專用 profile／畫面 | 規格、hazard review、完整 Gate |
| `REMOVE` | HT9050 不存在或不可達的 HT9045 能力 | 多 Picker／多 Index 專用分支 | absence proof、dead-path 檢查 |

## 每個垂直切片的紀錄模板

### 切片

- 名稱：
- 使用者／機台目標：
- HT9045 golden 來源：
- HT9050 現況來源：
- 類型：`REUSE | ADAPT | REPLACE | NEW | REMOVE`

### 三個錨點

- 條件：機種、模式、狀態、權限、interlock、feature flag
- 資料：tag／payload／設定鍵、型別、單位、來源、null／stale
- 行為：C++ 呼叫、硬體副作用、Alarm、UI 結果、rollback

### 差異與風險

- 機構差異：
- IO／Motor／Temp 差異：
- 狀態機差異：
- Recipe／持久化差異：
- 安全風險：
- 相容風險：

### 契約與實作

- C++ producer／command handler：
- WebBridge schema：
- HTML consumer／control：
- Error／timeout／retry：
- 實作檔案與 commit：

### 驗證

- Build／static check：
- Contract／unit test：
- Simulation／replay：
- 機邊測試：
- 證據位置：
- 結論：`VERIFIED | IMPLEMENTED | INTEGRATING | SPEC_ONLY | BLOCKED | UNKNOWN`

## 禁止的繼承方式

- 因為檔名或函式名相同，就假設機構行為相同。
- 將多臂／多 Index 的 index、array size 或 site mapping 原封不動帶入單臂機。
- 只複製 HTML，不確認 C++ 是否真的發布資料或執行副作用。
- 只看 ack=`ok`，不驗證真實 struct、檔案、IO 或 Motor 狀態。
- 以 SOFT_SIMULTE 成功推論真機安全。
- 把 UI 隱藏當成能力移除；C++ command path 仍可能可達。
- 為了先跑而 hardcode 成功、吞掉 Alarm、跳過 interlock 或創造假資料。
