---
name: ht9045-atc
description: >
  HT9045 IC Test Handler ATC（溫度控制系統）通訊模組知識庫。
  當使用者詢問 ATC 溫控流程、ATC 通訊指令（code 1001~1137）、
  ATC_Handler_Side.cpp 指令實作、Handler↔ATC TCP 封包格式、
  ATC 版本型號差異（ATC_3.5 / ATC_Rogers / ATC_3.1 / 5.0 / 6.0 / 7.0）、
  iATC_MODE_TYPE 判斷邏輯、新增或修改 ATC 指令 case、
  ProcessHandlerCommand / SendCommandToHandler 空實作 bug、
  ATC Self-Test 流程、Lot Start/End 通知、FFC/PFC 功能、
  TJ Slope/Offset 校正、Dynamic PID（TSMC）、HulkMode、
  多區 Tc Offset、Recipe 傳輸、MTK ASIF 指令、2DID 設定、
  水閥/水流/水警告、Chiller 控制等問題時，應先載入此技能。
  觸發關鍵字：ATC, ATCInterface, ATC_Handler_Side, iATC_MODE_TYPE,
  SendCommand, ProcessHandlerCommand, ProcessATC_BufferCommand,
  ATC_RECIPE_FILE, ATC_RUN_STOP, ATC_READ_TEMP, ATC_TEST_START,
  ATC_LOT_START, ATC_SELFTEST, FFC, PFC, TJ Offset, Dynamic PID,
  HulkMode, ASIF, 2DID, Chiller, 水閥, 溫控, ATC指令, ATC通訊。
---

# HT9045 ATC 溫控通訊模組

## 架構概覽

| 檔案 | 說明 |
|------|------|
| `ATC/ATC_Handler_Side.cpp/.h` | **主通訊模組**：Handler 端 TCP 指令收發、所有 code 1001~1137 的 case switch |
| `ATC/ATCInterface.cpp/.h` | 舊版 ATC（ATC7.0 之前）相容介面、UI Panel（`TMyHonPrecATCPanel`） |
| `ATC/ATCSystem.h` | `HT_ATC` 類別、`ATCData`、`ATCChannel` 抽象層 |
| `ATC/TCPData.cpp/.h` | TCP 封包收發底層（`TCPClient`、`TCPIP`） |
| `ATC/WinWaySetting.cpp/.h` | WinWay 溫控設定 UI |

**全域物件**：`ATC_InterfaceForm`（`TATC_InterfaceForm*`）；所有上層呼叫均透過此物件。

---

## ATC 版本型號（`iATC_MODE_TYPE`）

| 常數 | 值 | 代表機型 |
|------|----|---------|
| `ATC_TYPE_20` | 20 | 舊版 ATC 2.0 |
| `ATC_TYPE_21` | 21 | ATC 2.1 |
| `ATC_TYPE_30` | 30 | ATC 3.0 |
| `ATC_TYPE_31` | 31 | ATC 3.1（Rogers / HISI） |
| `ATC_TYPE_32` | 32 | ATC Rogers |
| `ATC_TYPE_33` | 33 | ATC 3.3 |
| `ATC_TYPE_35` | 35 | **ATC 3.5**（JerryYang 20220408） |
| `ATC_TYPE_36` | 36 | ATC 3.6 |
| `ATC_TYPE_50` | 50 | ATC 5.0（含 Module 指令） |
| `ATC_TYPE_51` | 51 | ATC 5.1（冷凍機 / 除霜） |
| `ATC_TYPE_60` | 60 | ATC 6.0（Qualcomm；氣閥） |
| `ATC_TYPE_61` | 61 | ATC 6.1 |
| `ATC_TYPE_70` | 70 | ATC 7.0（TSD / EMG） |
| `ATC_TYPE_UNSET` | 9999 | 尚未連線取得型號 |

`iATC_MODE_TYPE` 由 Handler 連線後發送 `ATC_MODE_TYPE`（1009）取得，並存入 `Config\ATC.ini`（key：`System/iATC_MODE_TYPE`）。（⛔ 20261001 更正：是 `D:\HT9045\system\ATC.ini`——golden `main.cpp:9792-9798` 把 `ATCIniPath` 指到 system；`ATCInterface.cpp:44` 的 `Config\ATC.ini` 只是預設、開機就被蓋掉；`Config\ATC.ini` 只有 `[Setup] iCheckSameTempTime`。見 ht9045-temperature §3）

**ATC_3.5 架構特殊性**：當 `iATC_MODE_TYPE == 35` 時，`ProcessHandlerCommand()` 第一段將指令轉入 `SendToATCBufferQueue`，交由後段 `ProcessATC_BufferCommand()` 實際處理。

---

## 指令封包格式

```
H→A 送出：@<Code>,<DataCount>,<D1>,<D2>,...,#
A→H 回傳：@<Code>,<DataCount>,<D1>,<D2>,...,#
```

- Code 範圍：1001 ~ 1137（定義於 `ATC_Handler_Side.cpp` 最上方 `#define` 區段）
- `ProcessReceiveString_ATC()` 解析回傳封包，結果存入 `dATC_Result[]`
- 延遲回傳指令：Handler 設 `bCommFlag[Code-1000]=true` → Timer 輪詢 → ATC 回傳後清 flag

---

## 常用 API（上層呼叫）

```cpp
ATC_InterfaceForm->Run();                          // 1007 ATC Run
ATC_InterfaceForm->Stop();                         // 1007 ATC Stop
ATC_InterfaceForm->ChangeRecipe(asFile);           // 1001 切換 Recipe
ATC_InterfaceForm->SetAllTemp(dTemp);              // 1002 設定所有 ch 溫度
ATC_InterfaceForm->SetSingleTemp(iCh, dTemp);      // 1015 設定單一 ch 溫度
ATC_InterfaceForm->SetOffset(iCount, dOffset);     // 1003 設定 Offset
ATC_InterfaceForm->SetSingleOffset(iCh, dOffset);  // 1016 設定單一 ch Offset
ATC_InterfaceForm->EnablesChannel(iCount, bArr);   // 1004 啟停 Site
ATC_InterfaceForm->StartTesting();                 // 1012 測試開始
ATC_InterfaceForm->TestFinish();                   // 1012 測試結束
ATC_InterfaceForm->LotStart(asLotID);              // 1050 Lot Start（需 iATC_MODE_TYPE >= 31）
ATC_InterfaceForm->LotEnd(asLotID);                // 1051 Lot End
ATC_InterfaceForm->ReadTC(iCount, dTC);            // 讀 TC 溫度陣列（從 dATC_Result 取值）
ATC_InterfaceForm->ReadTJ(iCount, dTJ);            // 讀 TJ 溫度陣列
ATC_InterfaceForm->SendCommand(iCode);             // 直接發送任意 code（進階用）
```

---

## 指令分類與重點備注

詳細指令對照表（含 ATC_3.5 vs Rogers 實作狀態）請參考：
- [Handler_Command_Report.md](references/Handler_Command_Report.md)

各指令的 H→A 封包格式（DataCount、欄位順序、單位）請參考：
- [ATC_Command_Payload.md](references/ATC_Command_Payload.md)

Multi Zone 功能異常（EnablesChannel 未展開 Zone 通道）詳見：
- [MultiZone_EnableChannel_Bug.md](references/MultiZone_EnableChannel_Bug.md)

### 快速分類索引

| 分類 | 代碼範圍 | 說明 |
|------|---------|------|
| 基礎控溫 | 1001–1010, 1015–1016, 1019–1020, 1024–1025, 1038–1039 | 溫度/Offset/PID/Run-Stop/Recipe |
| Handler 通知 | 1011–1014, 1017, 1033, 1048, 1050–1053 | 手臂/測試開始/Lot/Ping/異常 Site |
| 版本/設定查詢 | 1031–1032, 1036–1037, 1054 | SW/FW版本、Recipe清單 |
| 第二感溫點 | 1028–1030, 1034–1035, 1041, 1045 | 2nd Sensor 範圍/時間/Offset |
| Self-Test | 1042–1044, 1046–1047, 1052 | TJ Offset、自動/手動 Self-Test |
| FFC/PFC | 1063, 1065–1066, 1068–1070 | Feed-Forward/Power-Following Control |
| 水閥/IO | 1040, 1071–1072, 1110, 1130 | 水閥開度、水流量、水警告 |
| TJ 斜率/Offset | 1076–1077, 1084–1085, 1087–1088 | TJ 校正、Slope/Offset |
| Chiller | 1018, 1039, 1103–1104, 1125 | Chiller 狀態/溫度/水閥 |
| ATC 3.5 獨有 | 1086, 1122–1123, 1126–1129, 1131–1132, 1134, 1137 | Dynamic PID、MTC、HulkMode、Recipe 傳輸 |
| Rogers 獨有 | 1080–1083, 1092, 1098, 1102, 1105–1106, 1108, 1112–1114, 1120 | 水流、MTK ASIF、2DID、加熱器功率 |

---

## 已知實作 Bug（修改前必讀）

| # | 代碼 | 問題 | 影響版本 |
|---|------|------|---------|
| 1 | 1074 | `SendCommandToHandler(1074)` 無 case → Handler 永遠收不到 ACK | 3.5 & Rogers |
| 2 | 1076/1077 | ATC_3.5 的 `SendCommandToHandler` 無 case 1076/1077 → ACK 不發出 | ATC_3.5 |
| 3 | 1080 | Rogers `SendCommandToHandler(1080)` 無 case → ACK 不發出 | Rogers |
| 4 | 1044 | Rogers case 1044 缺 `break;`，fall-through 到 1045 | Rogers |
| 5 | 1085 | 雙版本均只設 flag，不直接發 ACK，需等 Timer 才回傳 | 3.5 & Rogers |

修改上述 bug 時，需同時補齊 `SendCommandToHandler` 的對應 case。

---

## 新增 ATC 指令的標準步驟

1. **定義常數**（`ATC_Handler_Side.cpp` 頂部 `#define` 區段）：
   ```cpp
   #define ATC_NEW_CMD   1XXX   // 說明
   ```
2. **補 `SetCommandString()`**：
   ```cpp
   asCommString[ATC_NEW_CMD - 1000] = "Description(1XXX)";
   ```
3. **加入 `SendCommand()` 的 switch case**（H→A 封包組裝）。
4. **加入 `ProcessReceiveString_ATC()` 的 switch case**（A→H 回傳解析）。
5. **若需延遲回傳**：設 `bCommFlag[ATC_NEW_CMD-1000]=true`，在 Timer 輪詢中讀取。
6. **若 ACK 由 ATC 主動發送**（如 1040 `ATC_SEND_TEMP_READY`）：僅在接收 switch 加入，不須加 Send。
7. **更新本文件** `Handler_Command_Report.md` 的對應表格。

---

## 版本差異開發注意事項

- **修改共用指令**：先確認 `iATC_MODE_TYPE` 分支邏輯，避免影響其他版本。
- **ATC_3.5 專屬**：1127 Dynamic PID、1131 MTC、1137 HulkMode 僅在 `iATC_MODE_TYPE==35` 時啟用。
- **Rogers 專屬**：1076/1077 已修正 ACK，3.5 仍有 bug；修 3.5 時需補 `SendCommandToHandler` case。
- **ATC_MAX_COMMAND 限制**：目前定義為 150，代碼超過 1150 時需修改此值（`ATC_Handler_Side.h` 第 37 行）。
- **溫度單位**：Handler 儲存為 `double`（°C），傳給 ATC 時乘以 10（整數，0.1°C 精度）。

---

## ATC 3.5 / 3.6 版本關係（2026-04-21 確認）

> **RD6 確認**：ATC 3.6 硬體與 ATC 3.5 **共用同一套 ATC 軟體**（V1.2.1）。
> ATC 軟體右下角顯示 `ATC 3.5 Version:1.2.1`，即使機器安裝的是 ATC 3.6 硬體。

### 型號識別行為

- Handler 連線後發送 `ATC_MODE_TYPE`（code 1009）向 ATC 查詢型號代碼。
- ATC 3.5 硬體回傳 `35`；ATC 3.6 硬體亦回傳 `35`（因軟體相同）。
- `Config\ATC.ini` → `[System]/iATC_MODE_TYPE=35` **對 ATC 3.6 機台為正常值，無需修改**。（⛔ 20261001 更正：是 `D:\HT9045\system\ATC.ini`——golden `main.cpp:9792-9798` 把 `ATCIniPath` 指到 system；`ATCInterface.cpp:44` 的 `Config\ATC.ini` 只是預設、開機就被蓋掉；`Config\ATC.ini` 只有 `[Setup] iCheckSameTempTime`。見 ht9045-temperature §3）
- Handler 程式中 `ATC_TYPE_36`（值 36）目前**未被 ATC 3.5 軟體回傳觸發**，相關 case 分支保留備用。

### Multi Zone 功能（`cbMultiZoneFunction`）適用版本

- 此功能在 `iATC_MODE_TYPE == 35`（及 36）時均可使用，不受型號代碼影響。
- 功能是否正常取決於 Handler 端 `EnablesChannel` 是否為所有 Zone channel 都下達 Enable 指令。
- **已知問題（2026-04-21）**：SingleSite 模式下 `bUse[]` 只對映 1 個 socket → 1 個 ATC channel，
  導致其餘 Zone channel 維持 Disabled，`SetMultiZoneTemp()` 的溫度指令對其無效。
  修改目標：`main.cpp` EnablesChannel 呼叫前，當 `bMultiZoneEnable=true` 時展開 4 個 Zone channel。

---

## ATC 6.0 / 7.0 版本保護規則（kevin 20260415）

以下指令在 `iATC_MODE_TYPE==ATC_TYPE_60` 或 `ATC_TYPE_70` 時 **不得送出**，  
對應函式已在開頭加入 `if(...) return;` early return 保護。

### 函式層封鎖（ATC_Handler_Side.cpp）

| 封鎖指令碼 | 函式名稱 | 封鎖方式 |
|-----------|---------|---------|
| 1038 / 1067 | `ATCCONTROLMODEMode()` | 末段 SendCommand 包入 `if(!=60&&!=70)` 條件 |
| 1071 | `ReadTCWaterValue()` | early return |
| 1084 | `GetSLOPEOFFSET()` | early return |
| 1085 | `SetSLOPEOFFSET()` | early return |
| 1087 | `RECORDTJTEMP()` | early return |
| 1088 | `QUERYTJTEMP()` | early return |
| 1088（輪詢） | `SendReadTempComm()` `bQUERYTJ` 條件 | 條件加入版本判斷 |
| 1125 | `SetTCWaterValve()` | early return |

### 呼叫端封鎖（main.cpp / uLotInfo.cpp）

| 封鎖指令碼 | 檔案 / 位置 | 封鎖方式 |
|-----------|-----------|---------|
| 1087 / 1088 | `main.cpp` `MSG_CMD_RECODETJ` handler | 包入 `if(!=60&&!=70)` 條件，同時阻止 `bQUERYTJ=true` |
| 1104 | `uLotInfo.cpp` case 17 `Send_ATCSetPFParameter` | 包入 `if(!=60&&!=70)` 條件 |
| 1105 | `uLotInfo.cpp` `SetTC2Offset` 呼叫處 | 加入 `iATC_MODE_TYPE==ATC_TYPE_33` 限制（Rogers only） |

### 例外（有意設計的 6.0 H→A 送出，但 ATC server 尚未支援）

| 指令碼 | 觸發條件 | 說明 |
|-------|---------|------|
| 1076 | `bATC_SlopeSaveOnHandler` + uLotInfo case 13 | ATC server 無 case，ACK 不回 |
| 1077 | `bATC_SlopeSaveOnHandler` + uLotInfo case 14 | ATC server 無 case，ACK 不回 |
| 1100 | `bATC_SlopeSaveOnHandler + bEnableTJFunction` + uLotInfo case 16 | ATC server 無 case，ACK 不回 |

### Guard 樣板

```cpp
// 函式層 early return（最常用）
if(iATC_MODE_TYPE==ATC_TYPE_60 || iATC_MODE_TYPE==ATC_TYPE_70) return;

// 條件包圍（保留其他 flag 邏輯，僅封鎖 SendCommand）
if(iATC_MODE_TYPE!=ATC_TYPE_60 && iATC_MODE_TYPE!=ATC_TYPE_70)
{
    SendCommand(XXX);
}

// ATC_3.5 only
if(iATC_MODE_TYPE!=ATC_TYPE_35) return;

// Rogers (TYPE_33) only
if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33) { ... }
```

---

## 1128 / 1127 Dynamic PID 白名單守衛（2026-04-23）

> **ATK HT-9046LS Hybrid 現場問題**：Reporter: Matthew Han `<matthew@teratechkorea.com>` (TeraTech Korea)

### 問題

原始 guard 採黑名單（排除 ATC 3.2 / 6.0 / 7.0），Rogers / 3.3 等未列入，導致這些機型收到不支援的 1127 / 1128，ACK 永不回，佔用通訊頻寬。

### 正確做法（白名單）

1127 / 1128 Dynamic PID 為 **ATC 3.5 / 3.6 專屬**，一律改用白名單：

```cpp
// SetDynamicPID() / ReadDynamicPID() 開頭
if(iATC_MODE_TYPE!=ATC_TYPE_35 &&
   iATC_MODE_TYPE!=ATC_TYPE_36) return;   // 僅 3.5/3.6 支援

// SendReadTempComm() Timer 區 1128 排程
if(iCount%30==19)
{
    if(iATC_MODE_TYPE==ATC_TYPE_35 ||
       iATC_MODE_TYPE==ATC_TYPE_36)
        SendCommand(ATC_READ_DYNAMIC_PID);
}
```

> **規則**：凡新增「某版本專屬」指令，**優先用白名單**；黑名單只用於「某版本不支援但其他版本都支援」的情況。

---

## `SendReadTempComm()` 通訊密度問題與質數錯峰排程（2026-04-23）

### 問題根因

舊版 Timer 使用 `%9 / %11 / %13` 等非質數週期，公倍數 tick（如 t=99 為 11×9）會多個 Tier C 指令同時送出，造成爆量（peak ~6–8 /s）。  
另外 `1036 GET_NOW_RECIPE` 依賴 `bGetNowRecipeFlag` 輪詢，未回 ACK 前每秒重送，持續佔用頻寬。

### 設計原則：質數週期 + offset 錯峰

| 指令 | 週期 | offset | 備註 |
|------|------|--------|------|
| 1010 Read Temp | 1 s | 每 tick | 最高優先 |
| 1025 ATC State | 2 s | `%2==1` | 與 1010 錯峰 |
| 1024 Read SetTemp | 9 s | `%9==2` | |
| 1035 Read Offset | 9 s | `%9==5` | 與 1024 同週期但不同 tick |
| 1067 Control Mode | 30 s | `%30==7` | `!=60&&!=70` |
| 1128 Read Dyn PID | 30 s | `%30==19` | **`==35\|\|==36`（白名單）** |
| 1071 Water Valve | 60 s | `%60==23` | `!=60&&!=70` |
| 1036 Now Recipe | 60 s | `%60==41` | 取代每秒輪詢 |
| 1033 Time Sync | 300 s | `%300==137` | |

```
穩態平均 ~1.7 /s（舊版 ~3.5 /s，-51%）
同 tick 最多 3 個指令，無公倍數爆點
iCount >= 3600 重設（LCM(2,9,30,60,300)=900 的安全倍數）
```

### 1036 / `bGetNowRecipeFlag` 輪詢觸發鏈

```
uLotInfo case 2
  └─ if(bGetNowRecipeFlag==false || bFirstSetRecipe==false)
       └─ GetNowRecipeFile() → 送 1036
1036 ACK 回來 → bGetNowRecipeFlag = true

ChangeRecipe(1001) 送出
  └─ bFirstSetRecipe = false   ← 重置
  └─ bGetNowRecipeFlag = false ← 重置
  → 舊版 Timer 每秒再送 1036，形成連續輪詢
```

**修正**：1036 改為 `iCount%60==41` 定期送，不再依賴 flag 輪詢。  
1001 (`ChangeRecipe`) 本身**未動**，仍為 event-driven（`bRecipeFileChange==true` 時觸發）。

---

## 搬移說明（St02 20260926）

本 skill 是從 `D:\HT9045\.github\skills\ht9045-atc` 複製進 `D:\HT9045\.claude\skills` 的（原處保留）。
下列二進位檔（客戶／廠商文件、手冊、截圖）**沒有一起搬進版控**：main 會同步到 GitHub。要看原檔請到原處：

- `references/ATC Interface  Define.xlsx`

> 20261001（St01 ST01-E2）：V906 移植樹的 ATC／溫度現況（`W906_ReadATCIni`、`SetATCOffset` 沒有定義、ATC7 沒有呼叫端…）與 ATC.ini 的位置 → `D:\HT9045\.claude\skills\ht9045-temperature\SKILL.md` §2、§3；本 skill 與 ht9045-atc-interface 的重疊與合併建議（待 Jimmy）→ `D:\HT9045\.claude\skills\ht9045-temperature\references\temperature-facts-index.md` §1。
