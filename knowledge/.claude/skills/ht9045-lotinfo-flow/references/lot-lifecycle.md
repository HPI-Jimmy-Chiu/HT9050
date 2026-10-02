# Lot 生命週期詳細流程

## 概述

HT9045 的批次生命週期由 `TfLotInfo` 管理，包含：**Lot Start → 生產中 → Lot End** 三個主要階段。

---

## 1. Lot Start 詳細流程

### 1.1 完整時序圖

```
                    [ 手動 / OLP / SECS/GPIB ]
                           ↓
              sbSECSLotStartClick() / SetLotStart(__FUNC__)
                           ↓
         ┌─────────────────────────────────────────┐
         │           前置驗證階段                    │
         │  - 特殊字元檢查（Lot ID / OP ID）          │
         │  - 2DID sorting list 下載（若啟用）         │
         │  - KYEC / VTEST / ASE 等客製驗證           │
         └─────────────────────────────────────────┘
                           ↓
              SetLotID(edtSysLotID->Text)
              │  └─ WriteIniData(config.ini, [Lot Info], "Lot ID", ...)
              │  └─ RunInfo.LotNo = edtSysLotID->Text
              │  └─ RunInfo.LotStartTime = 當前時間戳
              ↓
              ReadWriteLotInfo(false)
              └─ 寫入 OP ID / Run Mode / Customer Lot ID / Station 等
                           ↓
              ArmDataLot[i]->ClearALLCT()   ← 清除 By-Lot 統計
                           ↓
              SetTesterStartTimeByB03()      ← PTI ART 起始時間
                           ↓
       CosFunction.bRunModeFollowLotInfo ?
       ├─ true → 依 cbRunMode 切換 FT / RT / EQC
       └─ false → 不動
                           ↓
              EventReport(SECS_EVENT.DoLotStart)
              [條件: 非 TSMC + RunInfo.bLotStart==false + bEnable_SECS_GEM]
                           ↓
              SetLotComponents(false)   ← Lock UI（不可再改 Lot ID 等）
                           ↓
       TCP_IP_MODE?
       └─ 送 LOTNUMBER / LOTSTART / OPERATORID → Tester
                           ↓
              客製 log 寫入（OEE / SLT Summary / KYEC / CyuEan 等）
                           ↓
                    [ Lot Start 完成 ]
```

### 1.2 SetLotStart 參數說明

```cpp
void __fastcall TfLotInfo::SetLotStart(AnsiString sFunc, bool bReadFromFile);
```

| 參數 | 值 | 用途 |
|------|-----|------|
| `sFunc` | 呼叫者函式名（`__FUNC__`） | 日誌追蹤 |
| `bReadFromFile` | `true` | **還原模式**：從 config.ini 讀取（FormShow 時呼叫） |
| `bReadFromFile` | `false` | **正常 Lot Start**：寫入新批次資料 |

### 1.3 呼叫者對照表

| 呼叫者 | 條件 | bReadFromFile |
|--------|------|---------------|
| `auto9045.cpp::SetLotInfo()` L1023 | OLP `LotInfo_REQUEST` 指令 | false |
| `BarcodeXML.cpp` L807 | 條碼掃描觸發 | false |
| `SCK_ART.cpp` L4194 | ART 完成觸發 | false |
| `HANA_ART.cpp` L451 | HANA ART | false |
| `Command.cpp` L9405 | Command 指令 | false |
| `Command.cpp` L14190 | Command 指令 | false |
| `uLotInfo.cpp::FormShow` L1067 | 開機後還原 | true |
| `uLotInfo.cpp` L8089 | Open Short 完成 | false |
| `uLotInfo.cpp` L10157 | CheckList 驗證通過 | false |

---

## 2. Lot End 詳細流程

### 2.1 流程圖

```
              [ 手動按下 Lot End 按鈕 ]
                       ↓
              sbSECSLotEndClick()
                       ↓
       ┌───────────────────────────────┐
       │        前置守衛條件            │
       │  SystemStart==true → 拒絕     │
       │  CheckCanChangeRealDummy()    │
       │  iTestHeadMotorTask==1 → 拒絕 │
       └───────────────────────────────┘
                       ↓
       2DID Sorting?
       ├─ true → EventReport(DoVisualSortLotEnd)
       └─ false → EventReport(DoLotEnd)
                       ↓
       PTI bB03_TesterReport?
       └─ true → 上傳 JamAlarmLog 到 FTP
                       ↓
              SaveJamRateByLot()
                       ↓
              SetEndTime()
              └─ MyDBULotEndTime()
              └─ edLotID->Text = ""
              └─ lbStartTime->Caption = ""
                       ↓
              CyuEan 模式?
              └─ SetLotInfo_CYUEAN(2, false) → 寫入 CSV Log
                       ↓
              SetLotComponents(true)  ← 開放 UI 輸入
                       ↓
                [ Lot End 完成 ]
```

---

## 3. Config.ini 資料結構

**路徑**：`AuthPath + "config.ini"` → Section `[Lot Info]`

```ini
[Lot Info]
Lot ID=xxxxx
LotStartTime=2025-01-01 12:00:00
Customer Lot ID=xxxxx
OP ID=OP001
Run Mode=FT
Station=HT9045
Station Number=1
Job Sequence=001           ; SLT Summary 用
Start Time=                ; CyuEan 用
End Time=                  ; CyuEan 用
Tester OS Ver=
Tester ID=
Operator=                  ; CyuEan OperatorID
Customer=
Test Program=
Device Name=
Lot No=                    ; CyuEan Lot No
Sub Lot No=
Mode Code=
Test Code=
Test Bin No=
```

---

## 4. SECS/GEM 事件觸發條件

### DoLotStart 條件

```cpp
if(CUSTOMER_CODE != CC_TSMC_TAINAN &&
   IniConfig.bEnable_SECS_GEM == true &&
   RunInfo.bLotStart == false)              // ← 防重複觸發
    EventReport(SECS_EVENT.DoLotStart);
```

### DoLotEnd 條件

```cpp
if(2DID Sorting)
    EventReport(SECS_EVENT.DoVisualSortLotEnd);
else
    EventReport(SECS_EVENT.DoLotEnd);
```

---

## 5. CyuEan 特殊 CSV Log

**觸發條件**：`CUSTOMER_CODE == CC_CYUEAN`

```
路徑：D:\HT9045_Log\LotInfo\YYYY\YYYYMM_LotInfo.csv
```

**CSV 欄位（14 欄）**：
```
StartTime, EndTime, TesterOsVer, TesterID, Operator,
Customer, TestProg, DeviceName, LotNo, SubLotNo,
ModeCode, TestCode, TestBinNo, MachineID
```

| iLotStatus 值 | 觸發時機 | 動作 |
|-------------|---------|------|
| `0` | 讀取 | 從 config.ini 讀回所有欄位 |
| `1` | Lot Start | 記錄 StartTime 到 lbledtStarTime |
| `2` | Lot End | 記錄 EndTime → 輸出 CSV 行 → 清空所有欄位 |

---

## 6. RunInfo 關鍵欄位

```cpp
RunInfo.LotNo         = edtSysLotID->Text;              // 在 SetLotID 中賦值
RunInfo.LotStartTime  = "YYYY-MM-DD HH:MM:SS";          // 在 SetLotID 中賦值
RunInfo.bLotStart     = true/false;                     // Lot 啟動狀態旗標
RunInfo.ATCSoftwareVersion = sATCVerRead;               // ATC 版本（ShowATCThermo 中更新）
```

---

## 7. 特殊客戶行為差異

| 客戶代碼 | Lot Start 特殊行為 |
|---------|-----------------|
| `CC_TSMC_TAINAN` | 不觸發 `EventReport(DoLotStart)` |
| `CC_VTEST` / `CC_VTEST_Shanghai` | 檢查 VT ENG mode；依 Run Mode 切換 rsmInitialStart / rsmCInitialRetest |
| `CC_SCC` / `CC_Murata` / `CC_CYUEAN` | Lot Start 後鎖定 Run Mode 切換按鈕 |
| `CC_KYEC_LEE` / `CC_KYEC_XILINX` | 呼叫 `SaveLotOperatorID()` |
| `CC_ASE_CL` | 使用 `edtASECL_LotID` 而非 `edtSysLotID` |
| 2DID Sorting | 需先下載 sorting list；觸發 `DoVisualSortLotStart` 而非 `DoLotStart` |
