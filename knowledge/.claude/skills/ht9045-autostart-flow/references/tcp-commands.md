# AutoStart TCP 指令規格參考

> **重要**：規格書中的 `HTSET, XXX` 代碼為暫定值。HT9045 Handler 內部使用 OLP 字串命令處理通訊，兩者的對應關係需與供應商確認。以下同時列出規格書定義與 HT9045 實際 OLP 命令。

## 完整指令對照表（規格書 vs 實際 OLP 命令）

| 規格書指令 | 暫定代碼 | HT9045 OLP 命令 | 發送方 | 接收方 | 處理函式 | 狀態 |
|------------|----------|-----------------|--------|--------|---------|------|
| Ready (Handshake) | — | — | GTK Loader | Agent | — | 已定義 |
| End Lot | `HTSET, 700` | `CLEAR_REPORT_REQUEST [1111]` | Agent | HT9045 | `DoClearReportRequest(1111)` | 部分實作 |
| Start Lot (批次資訊) | `HTSET, 702` (part) | `LotInfo_REQUEST` | Agent | HT9045 | `SetLotInfo()` | 已實作 |
| Start Lot (Setup File) | `HTSET, 702` (part) | `PP_DL_REQUEST` | Agent | HT9045 | `DoDLRequest()` | 已實作 |
| Auto Start | `HTSET, 333` | `START_REQUEST` | Agent | HT9045 | `fMain->Start()` | 已實作（有條件） |
| Setting OK | — | — | Agent | GTK Loader | — | 已定義 |
| SOT | — | — | HT9045 | GTK Loader | — | 已存在 |
| EOT | — | GTK Loader | HT9045 | — | — | 已存在 |
| Safety Lock | TBD | `PAUSE_REQUEST` | Agent | HT9045 | `bLockByServer=true; SoftStop=true;` | 已實作 |
| Safety Unlock | TBD | `RESUME_REQUEST` | Agent | HT9045 | `bLockByServer=false;` | 已實作 |
| One Cycle | TBD | `ONECYCLE_REQUEST` | Agent | HT9045 | `DoOneCycle()` | **僅 DEBUG 模式** |

---

## 各指令詳細說明

### Auto Start（HTSET 333 → START_REQUEST）

**Agent → HT9045**，無攜帶資料

```cpp
// automation.cpp L1565-1580
else if(Command=="START_REQUEST")
{
    if(fMain->palMainStatus->Caption=="HALT" && SystemStart==false)
    {
        fMain->Start("TfAutomation::ProcessBuffer");
        // SoftStart=true;  ← 已 comment out，目前不使用
    }
    Data[0]=0;
    CommandProcess("START_REPLY", 1, Data, bClient, iHandle);
}
```

**執行條件**：
- `palMainStatus` 顯示 `"HALT"`
- `SystemStart == false`
- `bLockByServer == false`（Safety Interlock 為 Unlock）

---

### Safety Lock（PAUSE_REQUEST）

```cpp
// automation.cpp L1550-1557
else if(Command=="PAUSE_REQUEST")
{
    bLockByServer = true;
    SoftStop = true;
    Data[0]=0;
    CommandProcess("PAUSE_REPLY", 1, Data, bClient, iHandle);
}
```

`SoftStop=true` 會在下一個狀態機週期停止機台動作。

---

### Safety Unlock（RESUME_REQUEST）

```cpp
// automation.cpp L1558-1564
else if(Command=="RESUME_REQUEST")
{
    bLockByServer = false;
    Data[0]=0;
    CommandProcess("RESUME_REPLY", 1, Data, bClient, iHandle);
}
```

---

### End Lot（HTSET 700 → CLEAR_REPORT_REQUEST）

**Agent → HT9045**，攜帶 `iSwitchCase`

```cpp
// auto9045.cpp DoClearReportRequest()
if(iSwitchCase == 1111)  // 全清
{
    fCounterClear->ClearCount(ctTimeData);
    fCounterClear->ClearCount(ctLoadingCounts);
    fCounterClear->ClearCount(ctTraySortCount);
    fCounterClear->ClearCount(ctContactCounts);
    fCounterClear->ClearCount(ctTesterCategory);
}
```

| iSwitchCase | 清除內容 |
|-------------|---------|
| 1111 | 全部（時間資料、裝載計數、Sort 計數、Contact 計數、分類計數） |
| 2 | 時間資料（ctTimeData） |
| 3 | 裝載計數（ctLoadingCounts） |
| 4 | Sort 計數（ctTraySortCount） |
| 5 | Contact 計數（ctContactCounts） |
| 6 | 測試分類（ctTesterCategory） |

---

### Start Lot — 批次資訊（LotInfo_REQUEST）

**Agent → HT9045**，攜帶 `LotID, OperatorID, RunMode`

```cpp
// auto9045.cpp L1017-1038
int SetLotInfo(AnsiString *Data)
{
    // 前提：機台可切換 FT/RT（非運行中、無 IC 在機器下）
    if(fMain->CheckCanChangeRealDummy())
    {
        fLotInfo->edtSysOperatorID->Text = Data[1];   // OperatorID
        fLotInfo->SetLotID(Data[0], false);            // LotID
        fLotInfo->SetLotStart(__FUNC__);               // 啟動 Lot 計數
        if(Data[2]=="0") fMain->palFTClick(fMain);     // FT 模式
        else             fMain->palRTClick(fMain);     // RT 模式
        return 0;  // 成功
    }
    return 2;  // 失敗（機台運行中或有 IC）
}
```

Data 格式：`Data[0]=LotID`, `Data[1]=OperatorID`, `Data[2]=0(FT)/1(RT)`

---

### Start Lot — Setup File 下載（PP_DL_REQUEST）

**Agent → HT9045**，攜帶 Setup File 的 .zip 資料

`DoDLRequest()` 執行流程：
1. 建立目錄（`DataPath + sDLFileName`）
2. 解壓 `DataPath + sDLFileName.zip` 至目標目錄
3. 依 `Security_new.def` 判斷哪些參數需要保留（Contact 高度、HotPlate、Shuttle 模式等）
4. 恢復保護項目，移除暫存 zip

回傳：`1`（成功）、`0`（失敗）

> ⚠️ 此函式使用 `system()` 呼叫 7z.exe 解壓，7z.exe 須存在於 `d:\HT9045\7z.exe`

---

### One Cycle（ONECYCLE_REQUEST）

```cpp
// automation.cpp L1612-1615 (包在 #ifdef DEBUG_DUTONOFF 內)
else if(Command=="ONECYCLE_REQUEST")
{
    DoOneCycle();    // → InitOneCycle() + BtnOneCycle->Down=true
    bOneCycle=true;
}
```

```cpp
// auto9045.cpp L2174-2178
void DoOneCycle()
{
    InitOneCycle("DoOneCycle");
    fMain->BtnOneCycle->Down=true;
}
```

> ⚠️ **限制**：`ONECYCLE_REQUEST` 包在 `#ifdef DEBUG_DUTONOFF` 預處理條件內。正式 Release 版本不含此功能，需供應商開通 `DEBUG_DUTONOFF` 或提供替代方案。

---

## Info Mismatch 驗證（Agent 端，Phase 3）

此邏輯在 **Agent** 執行，非 HT9045：

```
Phase 1 暫存值（Golden Sample）：{ MO, Flow, Ticket }    ← Handshake 收到
Phase 3 Start Lot Ack 回傳：     { Ack.Status, Ack.MO, Ack.FLOW, Ack.TICKET }

if Ack.Status == "Fail":
    → 報錯中止（Setup File 下載或套用失敗）

if Ack.MO != Handshake.MO
   OR Ack.FLOW != Handshake.FLOW
   OR Ack.TICKET != Handshake.TICKET:
    → 報錯中止（Info Mismatch，顯示 Expected vs Actual）

if all match:
    → 發送 "Setting OK" 給 GTK Loader
    → 繼續執行 Auto Start
```

---

## Handshake 重試邏輯（GTK Loader 端）

此邏輯在 **GTK Loader** 執行，非 HT9045：

```
loop (Max 3 times):
    Send Ready [MO, Flow, Ticket]
    Wait 3s for Ack
    if Ack received: break (Handshake 成功)
    else: Retry Count +1
if Retry Count == 3:
    ABORT: 通知操作人員（網路異常或 Agent 未啟動）
```

