---
name: gpib-rs232-merge
description: >
  GPIB_RS232 三介面整合設計知識庫。將 GPIB_RS232 程式從純 GPIB
  擴展為支援 TTL/GPIB/RS232 三種介面的設計與實作。
  涵蓋 g_iTestType 架構、COM port 開關、startup ini 還原、
  iTesterMode 職責分離、DIO TypeName log、Qorvo guard。
  關鍵字：g_iTestType, TTL_MODE, GPIB_MODE, RS232_MODE,
  MSG_CMD_ChangeGpib, iTestType, OpenTesterComm, CloseTesterComm,
  iLotStatus, SendMessageToGpibProg, ProcessHVisionConnect, eRs232Mode
---

# SKILL: gpib-rs232-merge

## 描述

GPIB_RS232 通訊模式整合知識庫，紀錄將 GPIB_RS232 程式從純 GPIB 擴展為支援
TTL / GPIB / RS232 三種介面的完整設計與實作。涵蓋：
- 三介面架構設計（`g_iTestType` / `iLotStatus`）
- Log 路徑統一（`D:\GPIBLOG\` 下集中）
- RS232 log 同步寫入 GPIB log（`SerialPoll->WriteLog`）
- ProcessHVisionConnect 首次連線通知修正
- SendMSG_TestMode `iLotStatus` 未設定 bug 修正
- FormDestroy / FormClose 記憶體安全修正

關鍵字：g_iTestType, TTL_MODE, GPIB_MODE, RS232_MODE, iLotStatus, MSG_CMD_ChangeGpib,
MSG_CMD_TesterMode, SendMessageToGpibProg, iTestType, OpenTesterComm, CloseTesterComm,
LoadSetupData, g_sRecipePath, WriteLastDataFile, ReadLastDataFile, bGpibMode,
SendMSG_TestMode, ProcessHVisionConnect, sLastHVisionWnd, FormDestroy, FormClose,
slRS232Log, AppException, MY_DUT_PAL, double-free, dangling pointer, crash on close,
DIO TypeName, eRs232Standard, eRs23232Bin, SerialPoll WriteLog, ShowCommData

---

## 專案對照表

| 端 | 專案路徑 | 關鍵檔案 |
|----|---------|---------|
| **GPIB_RS232（受端）** | `D:\GPIB9045\GPIB_RS232_Code_32Site_V12.13.900.0_20260331\` | `Main.cpp`, `cmydef.h`, `RS232Aux.cpp`, `RS232Std.cpp` |
| **HT9045（送端）** | `D:\HT9045\HT9011UC_Code_V3.33.901.0_20260408_Steven\` | `main.cpp`, `cTesterIF.cpp` |

---

## 架構概念（簡要）

HT9045 透過 `WM_COPYDATA` 將 `iTestType`（介面種類）經 `iLotStatus` 欄位傳送至 GPIB_RS232，
GPIB_RS232 據此切換 TTL/GPIB/RS232 通訊模式。Cold start 從 `general.ini` 還原。

核心資料流：`HT9045 SendMessageToGpibProg() → MSG_CMD_ChangeGpib → g_iTestType → OpenTesterComm()`

→ 完整架構圖與程式碼：[references/rs232-merge-details.md](references/rs232-merge-details.md)

---

## 核心設計規則

| 變數 | 職責 | 值域 |
|------|------|------|
| `g_iTestType` | **介面種類**（誰在通訊）| `TTL_MODE(0) / GPIB_MODE(1) / RS232_MODE(2)` |
| `LastSet.iTesterMode` | **GPIB 子協定**（GPIB 時用什麼格式）| `InterfaceType_ADVAN_Type1` … `InterfaceType_Delta_Castle` |

- `iTesterMode` 只在 GPIB 模式下有意義；Qorvo guard 必須用 `g_iTestType == GPIB_MODE`
- **禁止**新增 `InterfaceType_RS232Standard` 等混用兩種概念的 define
- 每個 `WM_COPYDATA` 送出點都**必須**設定 `iLotStatus`（POD 預設 0 = TTL_MODE → BUG-001）

---

## 修改清單索引（C1–C10，2026-04-09）

| 代號 | 檔案 | 摘要 |
|------|------|------|
| C1 | `cmydef.h` | `TTL_MODE/GPIB_MODE/RS232_MODE` 常數 + `extern g_iTestType` |
| C2 | `Main.cpp` | `g_iTestType = GPIB_MODE` 全域變數宣告 |
| C3 | `Main.cpp` | `ReadLastDataFile()` startup ini 還原 |
| C4 | `RS232Aux` | `LoadSetupData()` 讀 Tester.Data RS-232C |
| C5 | `RS232Aux` | `OpenTesterComm()` 依 `g_iTestType` 分流 |
| C6 | `RS232Std` | `iUseRS232Mode` 來自 `g_iTestType` |
| C7 | `Main.cpp` | `MSG_CMD_TesterMode` log 分流 + Qorvo guard + COM port |
| C8 | `Main.cpp` | `MSG_CMD_ChangeGpib` 讀取 `iLotStatus` |
| C9 | HT9045 `main.cpp` | `SendMSG_TestMode` 補設 `iLotStatus`（Bug Fix） |
| C10 | `RS232Std` | `ShowCommData` 同步 `SerialPoll->WriteLog` + log 路徑統一 |

→ 完整程式碼：[references/rs232-merge-details.md § 關鍵修改清單](references/rs232-merge-details.md)

---

## 參考文件索引

| 文件 | 內容 |
|------|------|
| [references/rs232-merge-details.md](references/rs232-merge-details.md) | 架構圖、全域變數、C1–C10 程式碼、HT9045 端修改、log 路徑、ini 持久化、注意事項 |
| [references/bug-log.md](references/bug-log.md) | BUG-001（iLotStatus 未設定 → TTL 誤判）、BUG-002（FormDestroy 崩潰） |
| [references/design-decisions.md](references/design-decisions.md) | DD-001（g_iTestType vs bGpibMode）、DD-002（log 路徑統一）、DD-003（RS232 log 同步）、DD-004（static HWND 首次連線）、DD-005（iLotStatus 複用） |

---

## 注意事項

- `TestGPIB()` 在 RS232/TTL 模式下不執行，`iTesterMode` 判斷不需額外 guard
- `GPIBVersionCheck` 是硬體版本保護，與 `g_iTestType` 無關
- Ini 持久化：`D:\GPIB9045\system\general.ini [SystemSetup] iTestType`
