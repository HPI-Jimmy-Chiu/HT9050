> 保存來源：`.claude/skills/ht9045-autostart-flow/SKILL.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../common.md)。

<!-- preserved-content:start -->

# HT9045 Auto Start 通訊流程知識庫

## 系統角色速覽

| 角色 | 所在位置 | 職責 | 程式實體 |
|------|----------|------|---------|
| **HT9045（Handler）** | Handler 機台（Localhost） | 執行機械動作、接收 OLP 指令、發送 SOT/EOT | `TfAutomation` (`automation.cpp`) |
| **Agent** | Handler PC（Localhost） | 接收 GTK 信號、驗證資料、發送指令給 Handler | 外部程式 |
| **GTK Loader** | Tester PC | 監控測試程式載入、主動發送 Ready 給 Agent | 外部程式 |

> **HT9045 = Handler（Executor）角色**。HT9045 以 OLP Server（`TServerSocket OLPServer`）收聽 TCP 連線，被動接受 Agent 下達的控制指令。

通訊方向：GTK Loader → Agent → **HT9045（Handler）** → GTK Loader（SOT/EOT 循環）

---

## 五階段流程總覽

```
GTK Loader ──①Ready──► Agent ──③EndLot──► HT9045 (Handler)
              ◄──②Ack──          ◄──④Ack──
                    ──⑤StartLot──►
                    ◄──⑥Ack+MO/Flow/Ticket──
                    [⑦ 驗證比對 → Setting OK / Error]
                    ──⑧AutoStart──►
                    ◄──⑨Running──
HT9045 ──⑩SOT──► GTK Loader ──⑪EOT──► HT9045
```

### 階段一：握手（Handshake）
- 觸發：GTK Loader 偵測 Load test program 完成 + Set Lot 完成
- GTK Loader 發送 `Ready [MO, Flow, Ticket]` 給 Agent
- Agent 回傳 `Handshake OK`
- **重試機制**：發送 → 等待 3 秒 → 最多重試 3 次；失敗則中止流程

### 階段二：清除批次（End Lot）
- Agent 發送指令給 HT9045，對應 OLP 指令：`CLEAR_REPORT_REQUEST`
- **實際執行**：`iSwitchCase=1111` → 全清（時間、裝載、Sort、Contact、測試分類計數器）
- 由 `DoClearReportRequest()` (`auto9045.cpp`) 執行清除
- **前提**：`CheckSystemStart()==false`（機台必須停止中）

### 階段三：啟動批次（Start Lot）
- Agent 發送指令給 HT9045
- **兩步驟組合**：
  1. `LotInfo_REQUEST [LotID, OperatorID, FT/RT]` → `SetLotInfo()` → 設定 LotID、Operator、啟動 Lot
  2. `PP_DL_REQUEST [filename]` → `DoDLRequest()` → 從本地解壓 Setup File，套用 Machine Config
- HT9045 回傳對應 Reply（含 MO/FLOW/TICKET 資訊供 Agent 驗證）

### 階段四：解除鎖定並啟動（Auto Start）
- **前提 1**：Safety Interlock 必須為 Unlock（先執行 `RESUME_REQUEST`）
- **前提 2**：`fMain->palMainStatus->Caption=="HALT"` 且 `SystemStart==false`
- Agent 發送 `START_REQUEST` 給 HT9045
- **實際執行**：`fMain->Start("TfAutomation::ProcessBuffer")` → 機台進入 Run 狀態
- HT9045 回傳 `START_REPLY`

### 階段五：測試信號（SOT / EOT）
- HT9045 將首顆 IC 放入 Socket 後自動發送 SOT
- GTK Loader 執行測試後回傳 EOT
- 進入標準 ART 測試循環

---

## HT9045 OLP 實際指令對應表

> HTSET 代碼為規格書「暫定」值。HT9045 內部使用以下 OLP 字串命令處理通訊。

| 規格書指令 | HT9045 OLP 命令字串 | 處理函式 | 前提條件 |
|------------|---------------------|---------|---------|
| Auto Start（HTSET 333） | `START_REQUEST` | `fMain->Start()` | 狀態=HALT 且 SystemStart==false |
| Safety Lock（Cmd 0） | `PAUSE_REQUEST` | `bLockByServer=true; SoftStop=true;` | — |
| Safety Unlock（Cmd 1） | `RESUME_REQUEST` | `bLockByServer=false;` | — |
| End Lot（HTSET 700） | `CLEAR_REPORT_REQUEST [1111]` | `DoClearReportRequest(1111)` | SystemStart==false |
| Start Lot 批次資訊（HTSET 702） | `LotInfo_REQUEST` | `SetLotInfo()` | CheckCanChangeRealDummy==true |
| Start Lot 下載 Setup File | `PP_DL_REQUEST` | `DoDLRequest()` | — |
| One Cycle 模式 | `ONECYCLE_REQUEST` | `DoOneCycle()` → `InitOneCycle()` | **僅在 DEBUG_DUTONOFF 模式有效** |

> ⚠️ `ONECYCLE_REQUEST` 包在 `#ifdef DEBUG_DUTONOFF` 內，**正式版本不支援**，需向供應商確認開通方式。

---

## Safety Interlock 實作（bLockByServer）

```cpp
// PAUSE_REQUEST → Lock（automation.cpp L1553）
bLockByServer = true;
SoftStop = true;

// RESUME_REQUEST → Unlock（automation.cpp L1561）
bLockByServer = false;
```

- `bLockByServer` 為 `extern bool`，影響 Handler 是否允許啟動
- Lock 狀態下即便收到 `START_REQUEST`，Handler 因 `SoftStop=true` 也不會進入 Run

---

## Auto Start 條件（START_REQUEST）

```cpp
// automation.cpp L1565-1580
if(fMain->palMainStatus->Caption=="HALT" && SystemStart==false)
{
    fMain->Start("TfAutomation::ProcessBuffer");
    // SoftStart=true;  ← 已被 comment out
}
```

- 只有在 **HALT 狀態 + 機台未啟動** 時才有效
- 若狀態不符，回傳 fail 訊息但仍送出 `START_REPLY`

---

## 關鍵驗證邏輯（Info Mismatch）

```
Phase 1 暫存: { MO, Flow, Ticket }         ← Handshake 收到（Agent 端）
Phase 3 接收: Ack.{MO, Flow, Ticket}       ← Start Lot / LotInfo Reply

if Ack.Status == "Fail"          → 中止（Download 或套用失敗）
if 任一欄位不符                  → 中止（版本錯誤防呆，僅 Agent 端執行）
if 全部符合                      → 發送 Setting OK → 繼續執行 Auto Start
```

---

## 錯誤處理決策表

| 情境 | 偵測點 | 動作 |
|------|--------|------|
| Handshake 超時（<3s） | GTK Loader | Retry，最多 3 次 |
| Handshake 3 次失敗 | GTK Loader | 中止，通知人員 |
| End Lot 無回應 | Agent | 超時重試 or 中止（待定義） |
| Start Lot / LotInfo 失敗 | HT9045 | 回傳錯誤碼 2（機台運行中）→ Agent 中止 |
| Setup File 解壓失敗 | HT9045 | `ShowErrorMessage("WAR1686")` → Agent 中止 |
| Info Mismatch | Agent | 立即中止，顯示 Expected vs Actual |
| Auto Start 狀態不符 | HT9045 | 不執行 Start，回傳 fail 訊息 |
| bLockByServer=true | HT9045 | SoftStop 阻止機台啟動 |

---

## OLP 通訊協定格式

HT9045 使用自定義 OLP 封包格式（非純 HTSET）：

```
Frame: STX + CMD + SOH + V_Total + [SOH + Data[i]]* + ETX
STX = 0x02, SOH = 0x01, ETX = 0x03

範例：STX + "START_REQUEST" + SOH + "0" + ETX
```

詳細 OLP 協定、TCP 連線架構與指令完整對照見
- [tcp-commands.md](references/tcp-commands.md) — 指令代碼與格式
- [stage-details.md](references/stage-details.md) — 各階段規格與供應商需求
- [olp-protocol.md](references/olp-protocol.md) — OLP 協定結構與模組說明

---

## 錯誤處理速查

| 情境 | 偵測點 | 動作 |
|------|--------|------|
| Handshake 超時 | GTK Loader | Retry，最多 3 次；失敗中止 |
| Start Lot Download 失敗 | Handler | 回傳 `Fail` → Agent 中止 |
| Info Mismatch | Agent | 立即中止，顯示 Expected vs Actual |
| Auto Start 被鎖定 | Handler | 拒絕執行，回傳 Lock 狀態 |

---

## 待確認事項（高優先）

1. `HTSET, 333` Auto Start 目前 **No Action**，需開通（V906 註記，AI(W906-W10) 20260927 (St02-E)：Handler 7016 的 `HTSET,333`／`334` 在 V906 已接通——golden Command.cpp 的條件照舊（CC_TERAPOWER 或 `CosFunction.bRemoteLotStart`、HALT、未啟動），經 `W906_RemoteRun` 到 `TfMainWeb::StartFromWeb`／`PauseFromWeb`，不碰基底 `TfMain::Start`；沒安裝或手動教導中回 NG。見 `D:\HT9045\.claude\skills\hpi-gpib\references\ht9045-gpib-bridge\references\tcp-command-server-7016.md`）
2. Safety Interlock 指令需**新增實作**
3. One Cycle 模式 TCP 指令定義（待供應商提供）

完整 Open Issues、Safety Interlock 規則、指令代碼速查、Vendor Requirements 見：
- [tcp-commands.md](references/tcp-commands.md)
- [stage-details.md](references/stage-details.md)
- [olp-protocol.md](references/olp-protocol.md)

<!-- preserved-content:end -->
