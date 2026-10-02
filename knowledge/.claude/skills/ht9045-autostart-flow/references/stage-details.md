# AutoStart 階段詳細規格 + 供應商需求

## 目錄

- [階段一：握手（Handshake）](#階段一握手handshake)
- [階段二：清除批次（End Lot）](#階段二清除批次end-lot)
- [階段三：啟動批次與驗證（Start Lot）](#階段三啟動批次與驗證start-lot)
- [階段四：解除鎖定並啟動（Auto Start）](#階段四解除鎖定並啟動auto-start)
- [階段五：測試信號（SOT / EOT）](#階段五測試信號sot--eot)
- [供應商需求清單（Vendor Requirements）](#供應商需求清單vendor-requirements)
- [Open Issues 追蹤清單](#open-issues-追蹤清單)

---

## 階段一：握手（Handshake）

**目的：** 確認 GTK Loader 已就緒，將批次識別資訊傳遞給 Agent 作為後續驗證基準（Golden Sample）

| 項目 | 規格 |
|------|------|
| 觸發點 | GTK Loader 偵測到 Load test program 完成 + Set Lot 完成後，自動觸發 |
| 重試機制 | 發送 → 等待 3 秒 → 重試，最多 **3 次** |
| 失敗處理 | 3 次皆超時則中止流程，須通知操作人員 |
| 攜帶資料 | `MO`（工單號）、`Flow`（製程流程）、`Ticket`（批次票號） |

**前置條件：**

| 角色 | 前置狀態 |
|------|----------|
| Handler | 人員已放料，機台處於 Ready 狀態 |
| GTK Loader | 人員已手動載入測試程式並完成 Set Lot |

---

## 階段二：清除批次（End Lot）

**目的：** 確保 Handler 狀態歸零，避免上一批次數據殘留

| 項目 | 規格 |
|------|------|
| 指令 | `HTSET, 700`（暫定） |
| 執行者 | Agent 發送 → Handler 執行 |
| 動作 | ① LOTCLEAR（清計數器）② 清除暫存 Setup File 與 Config ③ 重置狀態機與 Flag |
| 回傳 | `Ack: End Lot OK`（所有清除動作完成後才回傳） |

> ⚠️ End Lot 是客製化 Macro，涵蓋範圍需與供應商確認（見 Requirement #5）

---

## 階段三：啟動批次與驗證（Start Lot）

**目的：** 命令 Handler 載入對應批次的 Setup File，並透過閉環驗證防止使用錯誤版本

| 項目 | 規格 |
|------|------|
| 指令 | `HTSET, 702`（暫定） |
| 攜帶參數 | `Qty`（數量）、`LotID`（批次ID） |

**Handler 端執行流程（客製）：**
1. 根據 `LotID` 至 FTP/Server 下載對應 `Setup File`
2. 解析檔案內容並套用 Machine Config
3. 驗證 Config 套用狀態
4. 回傳包含批次資訊的 Ack

**Ack 回傳格式：**
```
成功：OK;MO={Value};FLOW={Value};TICKET={Value}
失敗：Fail;MO={Value};FLOW={Value};TICKET={Value}
     或 Fail（Download 失敗時，無 Payload）
```

**Agent 驗證邏輯：**
```
if Ack.Status == "Fail":
    → 報錯中止（Setup File 下載或套用失敗）

if Ack.MO != Handshake.MO
   OR Ack.FLOW != Handshake.FLOW
   OR Ack.TICKET != Handshake.TICKET:
    → 報錯中止（Info Mismatch）

if all match:
    → 發送 "Setting OK" 給 GTK Loader → 繼續執行 Auto Start
```

---

## 階段四：解除鎖定並啟動（Auto Start）

**目的：** 遠端觸發 Handler 從 Idle 進入 Run 狀態

| 項目 | 規格 |
|------|------|
| 指令 | `HTSET, 333`（或對應指令） |
| 前提 | Safety Interlock 必須為 **Unlock（Cmd 1）** 狀態 |
| 回應 | `Ack: Running...` |
| 後續 | Handler 將首顆 IC 放入 Socket |

> ⚠️ 替代方案：若 Handler 不支援遠端啟動，改為 Handler 進入「等待 Start 信號」狀態，由 Agent 模擬按下 Start 鍵（見 Requirement #1）

---

## 階段五：測試信號（SOT / EOT）

**目的：** 進入標準 ART 測試循環

| 項目 | 規格 |
|------|------|
| SOT 觸發者 | Handler（IC 放入 Socket 後自動觸發） |
| EOT 觸發者 | GTK Loader（測試完成後回傳） |
| 後續流程 | 進入標準 ART 測試循環（Handler SRQ → Tester HTSET） |

---

## 供應商需求清單（Vendor Requirements）

### Requirement #1：Auto Start 指令開通

| 項目 | 說明 |
|------|------|
| 需求 | `HTSET, 333` 能實際觸發機台從 Idle → Run |
| 現況 | 指令存在於列表中，但**未生效（No Action）** |
| 優先級 | 🔴 高 |

### Requirement #2：One Cycle 模式設定

| 項目 | 說明 |
|------|------|
| 需求 | 能透過 TCP 指令將機台設定為 One Cycle 模式（執行單次循環後停止） |
| 指令形式 | 待供應商提供 |
| 優先級 | 🟡 中 |

### Requirement #3：Safety Interlock（安全互鎖）

| 項目 | 說明 |
|------|------|
| 需求 | 新增指令控制是否允許 Auto Start 生效 |
| Cmd 0 | Lock：強制鎖定， Auto Start 亦不啟動 |
| Cmd 1 | Unlock：解除鎖定，允許 Auto Start 正常運作 |
| 觸發情境 | 良率異常 / O/S 過高 → Agent 主動發送 Lock |
| 優先級 | 🔴 高 |

### Requirement #4：Start Lot 指令（客製化）

| 項目 | 說明 |
|------|------|
| 指令 | `HTSET, 702`（暫定） |
| 動作 1 | 根據 LotID 至 FTP/Server 下載對應 Setup File |
| 動作 2 | 解析並套用 Machine Config |
| 動作 3 | 驗證 Config 套用狀態 |
| 回傳（成功） | `OK;MO={Val};FLOW={Val};TICKET={Val}` |
| 回傳（失敗） | `Fail;MO={Val};FLOW={Val};TICKET={Val}` 或 `Fail` |
| 優先級 | 🔴 高 |

### Requirement #5：End Lot 指令（客製化）

| 項目 | 說明 |
|------|------|
| 指令 | `HTSET, 700`（暫定） |
| 動作 1 | 執行標準 LOTCLEAR（清除計數器） |
| 動作 2 | Clear Cache：移除上一批次 Setup File 與 Config |
| 動作 3 | Reset State：狀態機與 Flag 歸零 |
| 回傳 | `End Lot OK`（所有清除動作完成後才回傳） |
| 優先級 | 🟡 中（確認涵蓋範圍） |

---

## Open Issues 追蹤清單

| # | 類型 | 問題描述 | 負責方 | 優先級 |
|---|------|----------|--------|--------|
| 1 | 指令代碼 | `HTSET, 333`（Auto Start）最終代碼確認 | 設備供應商 | 🔴 高 |
| 2 | 指令代碼 | `HTSET, 700`（End Lot）最終代碼確認 | 設備供應商 | 🔴 高 |
| 3 | 指令代碼 | `HTSET, 702`（Start Lot）最終代碼確認 | 設備供應商 | 🔴 高 |
| 4 | 功能開通 | Auto Start 指令目前 No Action，需開通 | 設備供應商 | 🔴 高 |
| 5 | 新增功能 | Safety Interlock 指令需新增實作 | 設備供應商 | 🔴 高 |
| 6 | 新增功能 | One Cycle 模式 TCP 指令定義 | 設備供應商 | 🟡 中 |
| 7 | 範圍確認 | End Lot Macro 涵蓋動作最終確認 | 供應商 + 開發 | 🟡 中 |
| 8 | 替代方案 | 若 Handler 無遠端啟動，Start 鍵模擬方案 | 設備供應商 | 🟡 中 |
| 9 | 網路架構 | GTK Loader → Agent 的 TCP Port / 連線設定 | 開發團隊 | 🟡 中 |
| 10 | 錯誤處理 | End Lot 無回應時的超時與重試策略 | 開發團隊 | 🟢 低 |

---

## 開發實作注意事項

### Agent 開發重點
- 狀態機設計：維護當前流程階段，避免重複發送或跳過驗證
- Handshake 資料暫存：Phase 1 的 `{MO, Flow, Ticket}` 必須保存至 Phase 3 驗證完成
- 驗證邏輯獨立：Info Mismatch 檢查設計為獨立模組，方便擴展
- Safety Interlock 優先：Auto Start 前必須確認 Interlock 狀態為 Unlock

### GTK Loader 開發重點
- 自動觸發條件：Load test program 完成 + Set Lot 完成才允許發送 Ready
- 重試計數器：第 3 次失敗後明確停止並記錄 Log
- SOT 接收：SOT 信號到達後立即啟動測試，EOT 後回傳結果給 Handler

### Handler 介面開發重點
- 遠端啟動支援：透過 TCP 接收 Auto Start 並切換至 Run 模式
- Ack 時序：所有 Ack 必須在對應動作完成後才發送，不可提前回應
- Start Lot Ack 格式嚴格：`{Status};MO=...;FLOW=...;TICKET=...` 欄位不可缺少

### 通用建議
- Log 記錄：每個訊息收發、驗證結果、錯誤事件均記錄 Timestamp + 內容
- TCP 連線：建議實作 Keep-Alive 或心跳機制（偵測網路閃斷）
- 指令代碼：所有 `HTSET, XXX` 代碼為暫定，**開發前務必與供應商確認**
