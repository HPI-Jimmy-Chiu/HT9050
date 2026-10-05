# SEMI E87-0301 Carrier Management (CMS) Reference

> 規格來源：SEMI E87-0301 Provisional Specification for Carrier Management (CMS)  
> 版本：2001 年 3 月出版（2000 年 10/11 月審核通過）  
> 適用：HT9045 GEM300 Carrier/Port 通訊功能開發參考

---

## 1. 概覽 (Overview)

CMS（Carrier Management Standard）定義設備與 Host 之間的 Carrier 傳輸通訊行為，涵蓋：

- Load Port 外部 Carrier 搬入/搬出
- 內部緩衝區（Internal Buffer）的 Carrier 存取
- Load Port 存取模式切換（MANUAL/AUTO）
- Carrier 與 Load Port 的關聯（Association）
- CarrierID 驗證（Verification）
- Carrier Slot Map 驗證

**實作前提**：CMS 需搭配 GEM (SEMI E30) 一同實作，不需要獨立通訊連線。

---

## 2. 術語與縮寫

| 術語 | 說明 |
|------|------|
| AMHS | Automated Material Handling System，自動搬運系統 |
| Carrier | 承載基板的容器，如 FOUP、開放式料匣 |
| CarrierID | Carrier 的唯一可讀識別碼 |
| Docked Position | Carrier 準備好取放晶圓的停靠位置 |
| Fixed Buffer Equipment | 只有固定 Load Port 無內部緩衝的設備 |
| FIMS Port | FOUP 開關門的基板存取口 |
| Internal Buffer | 設備內部存放 Carrier 的區域（不含 Load Port） |
| Internal Buffer Equipment | 具有 Internal Buffer 的設備 |
| Load Port | 設備上 Carrier 裝卸的介面位置 |
| LocationID | Carrier 位置的唯一名稱 |
| PIO | Parallel Input/Output Interface |
| Slot Map | 記錄 Carrier 中各 Slot 基板位置與狀態的資訊 |
| Transfer Unit | 單次傳輸服務中允許的最大 Carrier 數量 |

---

## 3. 需求（Requirements）

CMS 合規設備必須符合以下標準：

| 標準 | 功能 |
|------|------|
| SEMI E30 (GEM) | Event Notification、Status Data Collection、Equipment Constants、Alarm Management、Equipment Control |
| SEMI E39 (OSS) | Object Services Standard |
| SEMI E53 | Event Reporting |
| SEMI E41 | Exception Management |

---

## 4. Load Port

### 4.1 Load Port 編號規則

從設備正面看，由左下到右下、再左上到右上，依序遞增編號（1, 2, 3…）。

### 4.2 Carrier Slot 編號規則

Carrier 內 Slot 從最底部開始，以 **1** 起算，向上遞增。

### 4.3 Load Port Transfer 狀態模型

每個 Load Port 各自維護獨立的 Transfer State Model 實例。

#### 狀態定義

| 狀態 | 說明 |
|------|------|
| **OUT OF SERVICE** | 此 Load Port 的傳輸功能已停用 |
| **IN SERVICE** | 此 Load Port 的傳輸功能已啟用 |
| **TRANSFER READY** | Load Port 可進行 Carrier 傳輸（IN SERVICE 子狀態） |
| **READY TO LOAD** | Load Port 上無 Carrier，可接受放入 Carrier（TRANSFER READY 子狀態） |
| **READY TO UNLOAD** | Load Port 上有 Carrier，可進行取出（TRANSFER READY 子狀態） |
| **TRANSFER BLOCKED** | Load Port 正在進行相關操作，目前無法傳輸 |

#### 狀態轉換表（Table 5）

| # | 前一狀態 | 觸發條件 | 新狀態 | 說明 |
|:--:|---------|---------|--------|------|
| 1 | (無狀態) | 系統重置 | OUT OF SERVICE 或 IN SERVICE (History) | 依系統重置前的狀態恢復 |
| 2 | OUT OF SERVICE | Host/操作員執行 `ChangeServiceStatus` → IN SERVICE | IN SERVICE | Load Port 恢復可用 |
| 3 | IN SERVICE | Host/操作員執行 `ChangeServiceStatus` → OUT OF SERVICE | OUT OF SERVICE | Load Port 停用；強行使用將觸發 Alarm |
| 4 | IN SERVICE | ChangeServiceStatus / 系統重置 → IN SERVICE | TRANSFER READY 或 TRANSFER BLOCKED | 預設進入 IN SERVICE 的子狀態 |
| 5 | TRANSFER READY | 進入 TRANSFER READY 時 | READY TO LOAD 或 READY TO UNLOAD | 有 Carrier → READY TO UNLOAD；無 Carrier → READY TO LOAD |
| 6 | READY TO LOAD | Manual: 偵測到 Carrier；Auto: PIO READY 訊號；Internal Buffer: CarrierOut 開始 | TRANSFER BLOCKED | — |
| 7 | READY TO UNLOAD | Manual: 偵測到取出開始；Auto: PIO READY 訊號；Internal Buffer: CarrierIn 開始 | TRANSFER BLOCKED | — |
| 8 | TRANSFER BLOCKED | Carrier 已取出（無 Carrier），傳輸完成 | READY TO LOAD | Carrier 可被放入 |
| 9 | TRANSFER BLOCKED | Carrier 處理完成或取消（CancelCarrier/CancelCarrierAtPort），Carrier 回到裝卸位置 | READY TO UNLOAD | Carrier 可被取走 |
| 10 | TRANSFER BLOCKED | 傳輸失敗，Carrier 未完成裝卸 | TRANSFER READY | 子狀態依 #5 判斷 |

> **事件資料**：所有狀態轉換均需產生對應 Collection Event，事件資料至少包含 **PortID**（部分轉換需包含 **CarrierID**）。

---

## 5. Carrier Object

### 5.1 Object 實例化（Instantiation）

Carrier Object 在以下情況被建立：

1. Host 執行 **Bind** 或 **CarrierNotification** Service
2. 設備成功讀取 CarrierID（且目前不存在相同 CarrierID 的 Object）
3. 在 UNASSOCIATED Port 上執行 **ProceedWithCarrier** 或 **CancelCarrier**（CarrierID 讀取失敗情況）

### 5.2 Object 銷毀（Destruction）

Carrier Object 在以下情況被銷毀：

1. Carrier 從設備卸載（Unload）
2. 收到 **CancelBind** 或 **CancelCarrierNotification** Service（Carrier 未到達前）
3. 設備的 Carrier 驗證失敗（由 Bind Service 建立的 Object）

### 5.3 Carrier Object ID（ObjID）

Carrier Object ID 等同於 **CarrierID**，設備需確保 Carrier ID 的唯一性。

### 5.4 Carrier 屬性（Attribute）定義（Table 6）

| 屬性名稱 | 說明 | 存取 | 必要 | 格式/值 |
|---------|------|:----:|:----:|--------|
| `ObjType` | 物件型別 | RO | Y | 文字 = "Carrier" |
| `ObjID` | 物件識別碼（= CarrierID） | RO | Y | 文字，1~80 字元 |
| `Capacity` | Carrier 最大基板容量 | RO | Y | 正整數 |
| `SubstrateCount` | 目前 Carrier 內基板數量 | RO | Y | 非負整數（≤ Capacity） |
| `CarrierIDStatus` | CarrierID 驗證狀態 | RO | Y | ID NOT READ / ID READ / ID VERIFICATION OK / ID VERIFICATION FAILED |
| `CarrierAccessingStatus` | 設備存取 Carrier 的當前狀態 | RO | Y | NOT ACCESSED / IN ACCESS / CARRIER COMPLETE / CARRIER STOPPED |
| `ContentMap` | 各 Slot 的 LotID + SubstrateID 清單 | RO | Y | List of n (LotID, SubstrateID)；Slot 1~n 對應清單位置 |
| `LocationID` | 目前 Carrier 所在位置 | RO | Y | 文字，1~80 字元 |
| `SlotMap` | 各 Slot 基板狀態清單 | RO | Y | UNDEFINED / EMPTY / NOT EMPTY / CORRECTLY OCCUPIED / DOUBLE SLOTTED / CROSS SLOTTED |
| `SlotMapStatus` | Slot Map 驗證狀態 | RO | Y | SLOT MAP NOT READ / SLOT MAP READ / SLOT MAP VERIFICATION OK / SLOT MAP VERIFICATION FAILED |
| `Usage` | Carrier 內材料類型（TEST/DUMMY/PRODUCT/FILLER 等） | RO | Y | 設備定義文字 |

### 5.5 LocationID 命名慣例

| 位置類型 | 命名格式 | 範例 |
|---------|---------|------|
| Load Port 裝卸位 | `LP`n | `LP1`, `LP2` |
| FIMS Port 位置 | `FIMS`n | `FIMS1` |
| 緩衝區位置 | `BUF`n | `BUF1`, `BUF2` |

> Carrier 在移動過程中，LocationID 維持為**來源位置**直到 Carrier 靜止在目的地。

---

## 6. Carrier 狀態模型

Carrier State Model 由三個**並行**（AND / Orthogonal）子狀態組成：

```
CARRIER
 ├─ CARRIER ID STATUS
 │    ├─ ID NOT READ
 │    ├─ WAITING FOR HOST
 │    ├─ ID VERIFICATION OK  (Final)
 │    └─ ID VERIFICATION FAILED  (Final)
 ├─ CARRIER SLOT MAP STATUS
 │    ├─ SLOT MAP NOT READ
 │    ├─ WAITING FOR HOST
 │    ├─ SLOT MAP VERIFICATION OK  (Final)
 │    └─ SLOT MAP VERIFICATION FAIL  (Final)
 └─ CARRIER ACCESSING STATUS
      ├─ NOT ACCESSED
      ├─ IN ACCESS
      ├─ CARRIER COMPLETE  (Final)
      └─ CARRIER STOPPED  (Final)
```

### 6.1 CARRIER ID STATUS 說明

| 狀態 | 說明 |
|------|------|
| **ID NOT READ** | 設備尚未讀取 CarrierID（Bind 或 CarrierNotification 建立 Object 時的初始狀態） |
| **WAITING FOR HOST** | CarrierID 已讀取（成功或失敗）但尚未驗證，等待 Host 指示 |
| **ID VERIFICATION OK** | CarrierID 已通過驗證（Final State） |
| **ID VERIFICATION FAILED** | CarrierID 驗證失敗（Final State） |

#### CARRIER ID STATUS 建立初始子狀態

| 建立方式 | 初始子狀態 |
|---------|-----------|
| Bind / CarrierNotification | ID NOT READ |
| 成功讀取 CarrierID | WAITING FOR HOST |
| ProceedWithCarrier（ID 讀取失敗） | ID VERIFICATION OK |
| CancelCarrier（ID 讀取失敗） | ID VERIFICATION FAILED |

### 6.2 CARRIER SLOT MAP STATUS 說明

| 狀態 | 說明 |
|------|------|
| **SLOT MAP NOT READ** | Carrier 剛進入設備，尚未在 Substrate Port 讀取 Slot Map（預設初始狀態） |
| **WAITING FOR HOST** | Slot Map 已讀取，等待 Host 驗證或確認（含讀取失敗/設備驗證失敗/Slot 位置異常情況） |
| **SLOT MAP VERIFICATION OK** | Slot Map 驗證通過（Final State） |
| **SLOT MAP VERIFICATION FAIL** | Slot Map 驗證失敗（Final State） |

### 6.3 CARRIER ACCESSING STATUS 說明

| 狀態 | 說明 |
|------|------|
| **NOT ACCESSED** | 設備尚未開始存取 Carrier；Carrier 可以被移出 |
| **IN ACCESS** | 設備正在存取 Carrier 中；不應移出 Carrier |
| **CARRIER COMPLETE** | 設備已正常完成對 Carrier 的存取；Carrier 應被移出（Final State） |
| **CARRIER STOPPED** | 設備異常停止對 Carrier 的存取；Carrier 應被移出（Final State） |

#### CarrierAccessingStatus 的 CARRIER COMPLETE 使用情境範例

| Usage | CARRIER COMPLETE 定義 |
|-------|----------------------|
| PRODUCT | 基板已完成處理 |
| DUMMY | 基板已用完，不可重複使用 |
| TEST | 基板已處理完成，Carrier 將為空 |
| REJECT | 設備已完成存取，Carrier 將為滿 |

### 6.4 Carrier 狀態轉換表（Table 7，精要）

| # | 前一狀態 | 觸發條件 | 新狀態 |
|:--:|---------|---------|--------|
| 1-5 | (無狀態) | Object 實例化 | CARRIER / ID NOT READ / WAITING FOR HOST / ID VERIFICATION OK / ID VERIFICATION FAILED |
| 6 | ID NOT READ | ID 成功讀取且設備驗證通過 | ID VERIFICATION OK |
| 7 | ID NOT READ | ID 讀取失敗 | WAITING FOR HOST |
| 8 | WAITING FOR HOST | 收到 ProceedWithCarrier | ID VERIFICATION OK |
| 9 | WAITING FOR HOST | 收到 CancelCarrier | ID VERIFICATION FAILED |
| 10 | ID NOT READ | BypassReadID=FALSE，ID Reader 不可用 | WAITING FOR HOST |
| 11 | ID NOT READ | BypassReadID=TRUE，ID Reader 不可用 | ID VERIFICATION OK（使用 Bind 中的 CarrierID） |
| 13 | SLOT MAP NOT READ | Slot Map 讀取並由設備驗證通過 | SLOT MAP VERIFICATION OK |
| 14 | SLOT MAP NOT READ | Slot Map 讀取（Host 驗證/設備失敗/讀取失敗/位置異常）| WAITING FOR HOST |
| 15 | WAITING FOR HOST | 收到 ProceedWithCarrier | SLOT MAP VERIFICATION OK |
| 16 | WAITING FOR HOST | 收到 CancelCarrier | SLOT MAP VERIFICATION FAIL |
| 17 | (無狀態) | Object 實例化 | NOT ACCESSED |
| 18 | NOT ACCESSED | 設備開始存取 Carrier | IN ACCESS |
| 19 | IN ACCESS | 設備正常完成存取 | CARRIER COMPLETE |
| 20 | IN ACCESS | 設備異常停止存取 | CARRIER STOPPED |
| 21 | CARRIER | Carrier 卸載 / CancelBind / 設備驗證失敗自發 CancelBind | (無狀態)，Object 銷毀 |

---

## 7. Access Mode 狀態模型

每個 Load Port 各有獨立的 Access Mode State Model。

### 7.1 狀態定義

| 狀態 | 說明 |
|------|------|
| **MANUAL** | 只允許人工（非 AMHS）的 Carrier 搬運；若有自動化搬運嘗試，設備應觸發 Alarm |
| **AUTO** | 只允許自動化（AMHS）的 Carrier 搬運；若有人工搬運嘗試，設備應觸發 Alarm |

### 7.2 狀態轉換表（Table 9）

| # | 前一狀態 | 觸發條件 | 新狀態 | 說明 |
|:--:|---------|---------|--------|------|
| 1 | (無狀態) | 系統重啟 | MANUAL 或 AUTO (History) | 恢復重置前的模式 |
| 2 | MANUAL | 執行 `ChangeAccess` → AUTO | AUTO | 傳輸中不可切換 |
| 3 | AUTO | 執行 `ChangeAccess` → MANUAL | MANUAL | 傳輸中不可切換 |

> **重要**：Access Mode 需在重初始化後記憶並恢復。Access Mode **不可**在 Carrier 傳輸期間切換（限制時間窗口見 Table 8）。

### 7.3 Carrier 傳輸邊界（Table 8）

| 傳輸類型 | 方法 | 起始邊界 | 結束邊界 |
|---------|------|---------|---------|
| LOAD | MANUAL | 設備偵測到 Carrier 存在 | 操作員邏輯通知傳輸完成 |
| LOAD | AUTO | PIO READY 訊號（裝載）啟動 | PIO 傳輸完成訊號 |
| UNLOAD | MANUAL | 設備偵測到卸載開始的邏輯訊號 | 操作員邏輯通知傳輸完成 |
| UNLOAD | AUTO | PIO READY 訊號（卸載）啟動 | PIO 傳輸完成訊號 |

---

## 8. Reservation 狀態模型

Reservation State Model 為 **Internal Buffer Equipment**（必要實作），Fixed Buffer Equipment（選用）。

### 8.1 狀態定義

| 狀態 | 說明 |
|------|------|
| **NOT RESERVED** | 該 Load Port 無任何預約 |
| **RESERVED** | 該 Load Port 已被預約，等待未來的 Carrier 到達；此狀態下不可更改 Access Mode |

### 8.2 狀態轉換表（Table 10）

| # | 前一狀態 | 觸發條件 | 新狀態 |
|:--:|---------|---------|--------|
| 1 | (無狀態) | 系統重置 | NOT RESERVED |
| 2 | NOT RESERVED | 收到 ReserveAtPort 或 Bind；或設備物理啟動 CarrierOut | RESERVED |
| 3 | RESERVED | 收到 CancelBind 或 CancelReservationAtPort；或 Carrier 到達預約 Port | NOT RESERVED |

> 進入 RESERVED 狀態時，設備應以可視訊號（如閃燈）標示，直到離開 RESERVED 狀態為止。

---

## 9. Load Port/Carrier Association 狀態模型

每個 Load Port 各自維護獨立的 Association State Model。

### 9.1 狀態定義

| 狀態 | 說明 |
|------|------|
| **NOT ASSOCIATED** | 此 Load Port 無 Carrier 關聯 |
| **ASSOCIATED** | 已有 CarrierID 與此 Load Port 關聯；不可再接受新的關聯 |

### 9.2 狀態轉換表（Table 11）

| # | 前一狀態 | 觸發條件 | 新狀態 |
|:--:|---------|---------|--------|
| 1 | (無狀態) | 系統重置 | NOT ASSOCIATED |
| 2 | NOT ASSOCIATED | Bind Service / CarrierID 讀取成功 / Known Carrier（CarrierOut 初始化） | ASSOCIATED |
| 3 | ASSOCIATED | CancelBind / 卸載 Carrier / 設備移動 Carrier 至 Internal Buffer | NOT ASSOCIATED |
| 4 | ASSOCIATED | 設備驗證失敗，Carrier 以讀取的 ID 更新；或 Internal Buffer 卸載且排隊的 CarrierOut 開始 | ASSOCIATED（更新 CarrierID） |

> Transition #4：設備應暫停後續動作，直到收到 CancelCarrier 或 ProceedWithCarrier；僅在 Bind Service 被使用時發生。

---

## 10. Verification（驗證機制）

### 10.1 CarrierID 驗證方式（Table 12）

| 驗證方式 | Host 在裝載前的動作 | 設備在 Carrier 到達時的動作 | Host 在裝載後的動作 |
|---------|-----------------|--------------------------|------------------|
| **設備驗證** | 執行 Bind Service（提供 PortID + CarrierID） | 讀取 CarrierID 並與 Bind 中的 ID 比對；通過 → Transition 6；失敗 → 銷毀原 Object，建立新 Object（讀取到的 ID），等待 PWC | 通過：無需動作；失敗：發送 CancelCarrier（強制卸載）或 ProceedWithCarrier（接受非預期 Carrier） |
| **Host 驗證** | 可執行 ReserveAtPort（選用） | 讀取 CarrierID 並回報給 Host（Transition 3）；等待 ProceedWithCarrier | 驗證通過：發送 ProceedWithCarrier；驗證失敗：發送 CancelCarrier 或 CancelCarrierAtPort |

### 10.2 Slot Map 驗證方式（Table 13）

| 驗證方式 | Host 在裝載前的動作 | 設備動作 | Host 在裝載後的動作 |
|---------|-----------------|---------|------------------|
| **設備驗證** | 透過 Bind 或 ProceedWithCarrier 提供 Slot Map | 比對讀取到的 Slot Map；Transition 13 或 14 | 通過：無需動作；失敗：可發送 CancelCarrier 或 ProceedWithCarrier |
| **Host 驗證** | 無需提前提供 | 讀取 Slot Map 並回報給 Host | 通過：發送 ProceedWithCarrier；失敗：發送 CancelCarrier 或 ProceedWithCarrier |

### 10.3 BypassReadID 機制

`BypassReadID`（RW Boolean，Variable Data 之一）控制 ID Reader 不可用時的行為：

| 值 | 行為 |
|:--:|----|
| FALSE（預設） | Carrier 轉入 WAITING FOR HOST，等待 Host 發送 ProceedWithCarrier；使用 ProceedWithCarrier 中的 ID |
| TRUE | 自動使用 Bind Service 提供的 CarrierID，直接進入 ID VERIFICATION OK |

---

## 11. Carrier Release Control

### 11.1 CarrierHold Trigger

適用於使用 Carrier Read/Write 技術的設備（Tag 讀寫）：

| CarrierHold 設定 | 行為 |
|:---:|------|
| **Host Release** | 設備保持 Carrier 在 Read/Write 位置，直到收到 `CarrierRelease` Service |
| **Equipment Release** | 設備在 Carrier 進入 CARRIER COMPLETE 或 CARRIER STOPPED 時自動釋放 |

### 11.2 UnclampControl（Fixed Buffer Equipment，AUTO 模式）

| UnclampControl 設定 | 行為 |
|:--:|------|
| CARRIERCOMPLETE/CARRIERSTOPPED Triggered | Carrier Status 轉為 CARRIERCOMPLETE/CARRIERSTOPPED 時自動 Unclamp |
| AMHS Triggered | 保持 Carrier 在夾持位置，直到 AMHS 到達並開始 PIO Unload 序列 |

---

## 12. Services（服務指令）

### 12.1 服務一覽（Table 15）

| Service 名稱 | Type | 說明 |
|-------------|:----:|------|
| `Bind` | R | 將 CarrierID 與 Load Port 關聯，Load Port → RESERVED |
| `CancelAllCarrierOut` | R | 取消排隊中的全部 CarrierOut |
| `CancelBind` | R | 取消 CarrierID 與 Load Port 的關聯，Load Port → NOT RESERVED |
| `CancelCarrier` | R | 取消當前 Carrier 操作；若在 Load Port 則退到裝卸位置，若在 Internal Buffer 則留在Buffer |
| `CancelCarrierAtPort` | R | 取消指定 Port 上的 Carrier（不需要知道 CarrierID） |
| `CancelCarrierNotification` | R | 銷毀由 CarrierNotification 建立的 Carrier Object |
| `CancelCarrierOut` | R | 取消排隊中的特定 CarrierOut |
| `CancelReservationAtPort` | R | 取消 Port 的預約並停用可視訊號（注意：CarrierOut 觸發的預約不可用此取消） |
| `CarrierIn` | R | 讓 Internal Buffer 設備將 Carrier 收入緩衝區（僅用於 CarrierOut 後的異常情況） |
| `CarrierNotification` | R | 通知設備特定 CarrierID 的 Carrier 即將到達（不指定 Port，不關聯 Load Port） |
| `CarrierOut` | R | 請求 Internal Buffer 設備將 Carrier 移到 Load Port；可排隊處理（FIFO） |
| `CarrierRelease` | R | 通知設備 Carrier 可從 Read/Write 位置移走 |
| `CarrierTagReadData` | R | 從 Carrier ID Tag 讀取資料 |
| `CarrierTagWriteData` | R | 寫入資料至 Carrier ID Tag |
| `ChangeAccess` | R | 變更指定 Port 清單的 Access Mode（全部成功才生效，否則全部不變） |
| `ChangeServiceStatus` | R | 變更 Load Port 的 Transfer 服務狀態（IN SERVICE / OUT OF SERVICE） |
| `ProceedWithCarrier` | R | 指示設備繼續處理指定 Carrier；Host 驗證通過時使用 |
| `ReserveAtPort` | R | 預約 Load Port，等待未來 Carrier 到達（Host 驗證使用） |

### 12.2 重要 Service 參數定義（Table 16～34）

#### `Bind`（Table 17）
| 參數 | 方向 | 必/選 | 說明 |
|------|:----:|:-----:|------|
| PortID | Req | M | 預期 Carrier 到達的 Port ID |
| CarrierID | Req | M | 預期的 CarrierID |
| PropertiesList | Req | C | Carrier Object 屬性清單（選用） |
| CMStatus | Rsp | M | 服務結果 |

#### `ProceedWithCarrier`（Table 33）
| 參數 | 方向 | 必/選 | 說明 |
|------|:----:|:-----:|------|
| PortID | Req | C | 允許繼續處理的 Port ID |
| CarrierID | Req | M | 允許繼續處理的 CarrierID |
| PropertiesList | Req | C | Carrier Object 屬性清單（選用） |
| CMStatus | Rsp | M | 服務結果 |

#### `CancelCarrier`（Table 20）
| 參數 | 方向 | 必/選 | 說明 |
|------|:----:|:-----:|------|
| CarrierID | Req | M | 要取消的 CarrierID |
| PortID | Req | C | Carrier 所在 Port（若 Object 已實例化則不需要） |
| CMStatus | Rsp | M | 服務結果 |

#### `CarrierOut`（Table 27）
| 參數 | 方向 | 必/選 | 說明 |
|------|:----:|:-----:|------|
| CarrierID | Req | M | 要移出的 CarrierID |
| PortID | Req | C | 目標 Port（省略則設備自行挑選） |
| CMStatus | Rsp | M | 服務結果 |

#### `ChangeAccess`（Table 31）
| 參數 | 方向 | 必/選 | 說明 |
|------|:----:|:-----:|------|
| AccessMode | Req | M | 新的 Access Mode（AUTO / MANUAL） |
| PortList | Req | M | 要套用的 Port 清單 |
| CMStatus | Rsp | M | 服務結果 |

### 12.3 CMStatus/CMAcknowledge

| 值 | 說明 |
|:--:|------|
| 0 | Acknowledge，命令已執行 |
| — | Invalid command |
| — | Cannot perform now |
| — | Invalid data or argument |
| — | Acknowledge，請求將非同步完成，以事件通知 |
| — | Rejected，invalid state |

### 12.4 ErrorCode 列表（精要）

通用錯誤（所有 Service）：
- Unsupported option requested
- Command not valid for current state
- Insufficient parameters specified
- Parameters improperly specified

特定 Service 錯誤：
- Bind：Load port does not exist / Load port already in use / Object identifier in use, Duplicate CarrierID
- CancelCarrier / CancelBind：Load port does not exist / Unknown object instance – Unknown CarrierID
- CarrierNotification：Object identifier in use, Duplicate CarrierID / Invalid attribute value / Unknown attribute name

---

## 13. 各狀態下可執行的 Services（Table 35，精要）

| 狀態 | 可執行 Service |
|------|--------------|
| OUT OF SERVICE | ChangeServiceStatus, ChangeAccess, CarrierNotification, CancelCarrierNotification |
| READY TO LOAD | ChangeServiceStatus, CancelBind, Bind, ChangeAccess, CancelCarrierOut, CarrierOut, CancelCarrierAtPort, CarrierNotification, CancelCarrierNotification, ReserveAtPort, CancelReservationAtPort |
| READY TO UNLOAD | + ProceedWithCarrier（全部 READY TO LOAD 可用的 + PWC）|
| TRANSFER BLOCKED | ChangeServiceStatus, ProceedWithCarrier, ChangeAccess, CancelAllCarrierOut, CancelCarrierOut, CarrierOut, CancelCarrierAtPort, CancelCarrier, CarrierNotification, CancelCarrierNotification |
| ASSOCIATED | ChangeServiceStatus, ProceedWithCarrier, CancelBind, ChangeAccess, CancelAllCarrierOut, CancelCarrierOut, CarrierOut, CancelCarrierAtPort, CancelCarrier, CarrierNotification |
| RESERVED | ChangeServiceStatus, CancelBind, CancelAllCarrierOut, CancelCarrierOut, ReserveAtPort, CancelReservationAtPort |

---

## 14. 附加事件（Additional Events）

### 14.1 特定 Collection Events（CMS 定義，Table 18 節）

| 事件 | 觸發時機 | 必要資料 |
|------|---------|---------|
| Buffer Capacity Changed | 內部 Buffer 容量變更 | BufferPartitionInfo |
| Carrier Approaching Complete | Carrier 即將完成存取 | CarrierID |
| Carrier Clamped | Carrier 第一次夾持啟動 | PortID, CarrierID（若可用）, LocationID |
| Carrier Closed | Carrier 門關閉（若有門） | CarrierID, LocationID, PortID（若有效）|
| Carrier Location Change | Carrier 位置改變 | CarrierID, LocationID（新目的地）, CarrierLocationMatrix |
| Carrier Opened | Carrier 門開啟（若有門） | CarrierID, LocationID, PortID（若有效）|
| Carrier Unclamped | Carrier 全部夾持裝置解除 | PortID, CarrierID（若可用）, LocationID |
| CarrierID Read Fail | 在 NOT ASSOCIATED 狀態下讀取 CarrierID 失敗 | PortID |
| ID Reader Available | ID Reader 恢復可用 | PortID |
| ID Reader Unavailable | ID Reader 變為不可用 | PortID |

> 所有 CMS 狀態轉換亦需有對應的 Collection Event Report。

---

## 15. Variable Data（Table 36，精要）

| 變數名稱 | 說明 | Type | Access |
|---------|------|------|:------:|
| `AccessMode`i | 第 i 個 Load Port 的 Access Mode | Enum: MANUAL/AUTO | RO |
| `BypassReadID` | ID Reader 不可用時的自動接受設定 | Boolean | **RW** |
| `CarrierID` | Carrier 的 ID | Text | RO |
| `CarrierLocationMatrix` | 設備上所有 Carrier 的位置清單（LocationID + CarrierID） | List | RO |
| `LocationID`i | 第 i 個 Carrier 位置的 ID | Text | RO |
| `PortAssociationState`i | 第 i 個 Load Port 的 Association State | Enum: ASSOCIATED/NOT ASSOCIATED | RO |
| `PortStateInfo`i | 第 i 個 Load Port 的 Association + Transfer State 組合 | List | RO |
| `PortStateInfoList` | 所有 Load Port 的狀態清單 | List | RO |
| `PortTransferState`i | 第 i 個 Load Port 的 Transfer State | Enum: OUT OF SERVICE/TRANSFER BLOCKED/READY TO LOAD/READY TO UNLOAD | RO |
| `PortTransferStateList` | 所有 Load Port 的 Transfer State 清單 | List | RO |
| `Reason` | Transition 14（SLOT MAP NOT READ → WAITING FOR HOST）的原因 | Enum: VERIFICATION NEEDED / VERIFICATION BY EQUIPMENT UNSUCCESSFUL / READ FAIL / IMPROPER WAFER POSITION | RO |
| `AvailPartitionCapacity`i | 第 i 個 Buffer 分區的可用容量（Internal Buffer 設備專用） | 非負整數 | RO |
| `BufferCapacityList` | 所有 Buffer 分區的容量資訊（Internal Buffer 設備專用） | List | RO |
| `BufferPartitionInfo`i | 第 i 個 Buffer 分區的詳細資訊（PartitionID/Type/Avail/Total/Unallocated）（Internal Buffer 設備專用） | Struct | RO |

---

## 16. 警報（Alarms，Table 37）

### 16.1 必要警報清單

| 適用設備 | 警報文字 | 危險等級 | 影響對象 |
|---------|---------|---------|---------|
| Fixed & Internal Buffer | PIO Failure | Potential | Operator, Equipment, Material |
| Fixed & Internal Buffer | Access Mode Violation | Potential | Operator, Equipment, Material |
| Fixed & Internal Buffer | Carrier Verification Failure | Potential | Material |
| Fixed & Internal Buffer | Slot Map Read Failed | Potential | Operator, Equipment, Material |
| Fixed & Internal Buffer | Slot Map Verification Failed | Potential | Equipment, Material |
| Fixed & Internal Buffer | Attempt To Use Out Of Service Load Port | Potential | Equipment, Material |
| Fixed & Internal Buffer | Carrier Presence Error | Potential | Operator, Equipment, Material |
| Fixed & Internal Buffer | Carrier Placement Error | Potential | Operator, Equipment, Material |
| Fixed & Internal Buffer | Carrier Dock/UnDock Failure | Potential | Equipment, Material |
| Fixed & Internal Buffer | Carrier Open/Close Failure | Potential | Equipment, Material |
| Fixed & Internal Buffer | Carrier Removal Error | Potential | Operator, Equipment, Material |
| Fixed & Internal Buffer | Duplicate CarrierID | Potential | Material |
| Internal Buffer Only | Internal Buffer Carrier Move Failure | Potential | Equipment, Material |

### 16.2 Duplicate CarrierID 處理規則

當設備收到與已在設備上的 Carrier 相同 CarrierID 時：
1. 第二個具相同 CarrierID 的 Carrier 不應被處理
2. 若第一個 Carrier 尚未開始處理，亦不應被處理
3. 若第一個 Carrier 已在處理中，應發出 Duplicate Carrier ID In Process 事件通知 Host

---

## 17. CMS 合規檢查表（Table 38）

### 基本必要項目（Fundamental）

| 項目 | CMS 章節 |
|------|---------|
| Load Port 編號規則 | 9.1 |
| Carrier Slot 編號規則 | 9.2 |
| Load Port Transfer 狀態模型 | 9.3–9.4.3 |
| Carrier Object 實作 | 10 |
| Load Port Reservation 狀態模型（Internal Buffer 設備必要） | 12 |
| Load Port/Carrier Association 狀態模型 | 13 |
| CarrierID 驗證支援 | 14.2 |
| Slot Map 驗證支援 | 14.3 |
| Services 實作 | 16 |
| Additional Events 實作 | 18 |
| Variable Data 定義 | 19 |
| Alarms 實作 | 20 |

### 選用項目（Additional Capabilities）

| 項目 | CMS 章節 |
|------|---------|
| Load Port Reservation 狀態模型（Fixed Buffer 選用） | 12 |
| Reservation Visible Signal（可視訊號） | 12.2 |

---

## 18. 典型場景（Normal Roundtrip 摘要）

### 場景 1：Fixed Buffer，Host 驗證

```
Initial: READY TO LOAD
→ Carrier 到達 → TRANSFER BLOCKED
→ CarrierID 讀取 → WaitingForHost (CIDS)
→ ProceedWithCarrier → ID_VERIFICATION_OK
→ Carrier Docked → Slot Map 讀取 → WaitingForHost (CSMS)
→ ProceedWithCarrier → SLOT_MAP_VERIFICATION_OK
→ 處理 → 完成
→ Carrier Undocked → READY TO UNLOAD
→ Carrier 取走 → TRANSFER BLOCKED → READY TO LOAD
```

### 場景 2：Fixed Buffer，設備驗證（Bind）

```
Initial: READY TO LOAD
→ Host 發送 Bind(PortID, CarrierID) → RESERVED, ASSOCIATED, ID_NOT_READ
→ Carrier 到達 → TRANSFER BLOCKED
→ CarrierID 讀取 → 設備驗證通過 → ID_VERIFICATION_OK
→ Slot Map 讀取 → 設備驗證通過 → SLOT_MAP_VERIFICATION_OK
→ 處理 → 完成
→ READY TO UNLOAD → TRANSFER BLOCKED（卸載） → READY TO LOAD
```

### 場景 3：Internal Buffer，Host 驗證

```
→ Carrier 到達，CarrierID 讀取 → WaitingForHost
→ ProceedWithCarrier → ID_VERIFICATION_OK
→ Carrier-in（進入 Buffer），BufferCapacityChange 事件
→ FIMS Port 讀取 Slot Map → WaitingForHost
→ ProceedWithCarrier → SLOT_MAP_VERIFICATION_OK
→ 處理完成 → CarrierComplete 事件
→ Host 發送 CarrierOut → Carrier 移至 Load Port
→ BufferCapacityChange 事件 → READY TO UNLOAD
→ Carrier 取走 → READY TO LOAD
```

---

## 19. Reservation 與 Association 關係

以下整理 Reservation、Association 與各種 Carrier 到達情況的狀態流程：

| 情況 | Reservation 觸發 | Association 觸發 | Association 解除 |
|------|:--------------:|:--------------:|:--------------:|
| Load w/o Bind, w/o Reserve | 無 | ID Read（Carrier 到達時） | Unload 或 Carrier 進 Buffer 完成 |
| Load w/ Reserve | ReserveAtPort | ID Read（Carrier 到達後）| 同上 |
| Load w/ Bind | Bind | Bind（立即）| CancelBind 或 Unload |
| Carrier Out（Internal Buffer） | CarrierOut 開始 | CarrierOut 開始 | Unload 完成 |

---

## 20. 在 HT9045 中的對應實作說明

HT9045 的 CMS/GEM300 功能主要在 `uHGemHT9045.cpp` 的 S125F4（GEM300 Service）及系列 CEID（94~135 OHT/Cassette 事件）實作。

### 已對應的 CEID（Load Port / Carrier 事件區段）

| CEID 範圍 | 說明 |
|:--------:|------|
| 94-135 | OHT/Cassette 事件：TransferBlocked、CassetteLoad/Out、SlotMap、CHECK_IN/OUT |
| 136-211 | Tray/Port 擴充，Auto1-6/Fix1-6 Unload/Full/NoTray |
| 217-231 | Port 狀態變更 PortStatusChanged |
| 38100-38300 | Load Port Transfer State（SVID） |

### 對應的 RCMD

| RCMD | E87 對應 |
|------|---------|
| `DISCHARGE_OUTPUT_PORT` | CarrierOut（指定 Port） |
| `DISCHARGE_OUTPUT_ALL_PORT` | CarrierOut（所有 Port） |
| `STOP_LOAD_PORT` | ChangeServiceStatus → OUT OF SERVICE |
| `RESTART_LOAD_PORT` | ChangeServiceStatus → IN SERVICE |
| `TRAY_CHECK` | Bind / ProceedWithCarrier / CancelCarrier 之邏輯 |

> 詳細 SECS/GEM 實作參照：[SECS-Dev-Procedures.md](SECS-Dev-Procedures.md) 與 [SECS-Programming-Guide.md](SECS-Programming-Guide.md)
