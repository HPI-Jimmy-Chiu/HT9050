# ATC Handler 通訊指令實作對照報告

**報告日期**：2026-04-14（更新加入 ATC_6.0）  
**參考版本**：
- ATC_3.5 → `ATC35_Program_20260407`
- ATC_Rogers (3.2/3.3) → `ATC32_Program_Rogers_20260325`
- ATC_6.0 → `d:\ATC\ATC60`（`ATC_Server_Ver 2017-11-09`，`ATC_MaxCommand=1137`）

---

## 說明

| 符號 | 意義 |
|------|------|
| ✅ | 已實作（具有實際邏輯或完整 ACK） |
| ⚠️ | 存在但為空實作（`break;` 無邏輯），或呼叫 `SendCommandToHandler` 但目標 case 不存在導致無 ACK |
| ❌ | 該版本無此 case |

**方向說明**：
- **H→A**：Handler 傳送給 ATC，由 `ProcessHandlerCommand()` 接收處理
- **A→H**：ATC 回傳給 Handler，由 `SendCommandToHandler()` 發送

> **ATC_3.5 架構備注**：當 `iATC_MODE_TYPE == 35` 時，`ProcessHandlerCommand()` 第一段會將收到的指令直接轉入
> `SendToATCBufferQueue`，交由後段 `ProcessATC_BufferCommand()` 實際處理。本表以最終實際處理結果為準。

> **ATC_6.0 架構備注**：ATC_6.0 以 `CommandTranslate()` 作為中介，再依 `iATC_MODE_TYPE`（預設 31/51/60）轉發至
> `Translate_ATC31`、`Translate_ATC51` 或 `Translate_ATC70`。`ATC_MAX_SITE = 64`（大於 3.5/Rogers 的 32）。

---

## 一、三版本共用指令（ATC_3.5、Rogers、ATC_6.0 均有）

### 1.1 基礎控溫指令

| Code | 常數名稱 | 說明 | 3.5 H→A | 3.5 A→H | Rogers H→A | Rogers A→H | 6.0 H→A | 6.0 A→H | 備注 |
|------|---------|------|---------|---------|-----------|-----------|--------|--------|------|
| 1001 | `ATC_RECIPE_FILE` | 切換 Recipe 檔案 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 3.5 有 AutoSave/AMD_SG 分支；6.0 解析方式用 `LastDelimiter` |
| 1002 | `ATC_SET_TEMP` | 設定執行溫度（all ch） | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 3.5/Rogers 單位 0.1°C；6.0 不除以10（呼叫端負責） |
| 1003 | `ATC_SET_TOFS` | 設定溫度 Offset（all ch） | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1004 | `ATC_SITE_ENABLED` | 啟用/停用 Site | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 3.5 有 NvidiaType / SingleSiteType 特殊鎖定 |
| 1005 | `ATC_SET_TRM` | 溫度讀取來源模式 | ⚠️ 空 | ✅ | ✅ | ✅ | ✅ | ✅ | 3.5 ProcessHandlerCommand 為空 `break` |
| 1006 | `ATC_SET_MAX_TEMP` | 設定最高工作溫度 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1007 | `ATC_RUN_STOP` | ATC 啟動/停止 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 3.5 額外有 PID 值檢查 |
| 1008 | `ATC_INITIAL_SET` | 初始化全部設定 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1009 | `ATC_MODE_TYPE` | 查詢 ATC Mode（型號） | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 回傳 `iATC_MODE_TYPE` |
| 1010 | `ATC_READ_TEMP` | 讀取溫度（TC+TJ 每 ch） | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 回傳 `Tc,Tj...ThreadCount`；6.0 用 `%6.2f` 格式，ATC_MAX_SITE=64 |
| 1015 | `ATC_SET_SINGLE_TEMP` | 設定單一 Site 溫度 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 格式：`@1015,2,Site,Temp#` |
| 1016 | `ATC_SET_SINGLE_OFFSET` | 設定單一 Site Offset | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1018 | `ATC_CHILLER_STATUS` | 查詢 Chiller 狀態 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1019 | `ATC_SET_PID` | 設定 PID 參數 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 每組 3 筆(P/I/D)，除以10.0 |
| 1020 | `ATC_READ_PID` | 讀取 PID 參數 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 3.5/Rogers 透過 flag 延遲回傳；6.0 用 `asATC_SendData` 即時回傳 |
| 1023 | `ATC_READ_TEMP_2` | 讀取第二感溫點溫度 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 6.0 回傳時也支援 bTwoDecimalPlaces（×10） |
| 1024 | `ATC_READ_SETTEMP` | 讀取目前設定溫度 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1025 | `ATC_STATE` | 讀取 ATC 運行狀態 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 回傳 Running=1/Stop=0 |
| 1038 | `ATC_SET_TjMode` | 設定 Tj 控制模式（AMD） | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 0:TC 1:TJ 2:TS；3.5 額外更新 TDC for TSMC |
| 1039 | `ATC_SET_CHILLER_TEMP` | 設定 Chiller 目標溫度 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |

### 1.2 Handler 通知類指令

| Code | 常數名稱 | 說明 | 3.5 H→A | 3.5 A→H | Rogers H→A | Rogers A→H | 6.0 H→A | 6.0 A→H | 備注 |
|------|---------|------|---------|---------|-----------|-----------|--------|--------|------|
| 1011 | `ATC_HANDLER_ARM` | 告知使用哪隻手臂 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1012 | `ATC_TEST_START` | 測試開始/結束通知 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 3.5 支援 2DID（TSMC）；包含電壓硬限制檢查 |
| 1013 | `ATC_SEND_TJ` | 傳送 TJ 給 ATC（已廢棄） | ⚠️ 空 | ✅ | ⚠️ 空 | ✅ | ⚠️ 空 | ✅ | 三版本 ProcessHandlerCommand 均為空 `break` |
| 1014 | `ATC_ERROR` | ATC 傳送錯誤訊息 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 3.5/Rogers 固定解析 `_OK`/`_NG` 位於第7位；6.0 動態長度 `Length()-2` |
| 1017 | `ATC_CANCEL_MESSAGE` | 關閉告警視窗 | ✅ | ❌ | ✅ | ❌ | ✅ | ❌ | 三版本均無 ACK 回傳 |
| 1033 | `ATC_TIME_SYNC` | Handler 進行時間同步 | ✅ | ❌ | ✅ | ❌ | ✅ | ❌ | ATC 只接受更新時間，不回覆 ACK |
| 1048 | `HANDLER_STATUS` | Handler 狀態查詢（Ping） | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1050 | `ATC_LOT_START` | Lot 開始通知（含 Lot ID） | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1051 | `ATC_LOT_END` | Lot 結束通知 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1053 | `HANDLER_ABNORMAL_SITE` | Handler 回報異常 Site | ✅ | ❌ | ✅ | ❌ | ✅ | ❌ | 三版本均僅標記 `iSelfTestResult[]`，無 ACK |

### 1.3 版本 / 設定查詢指令

| Code | 常數名稱 | 說明 | 3.5 H→A | 3.5 A→H | Rogers H→A | Rogers A→H | 6.0 H→A | 6.0 A→H | 備注 |
|------|---------|------|---------|---------|-----------|-----------|--------|--------|------|
| 1031 | `ATC_SW_VER` | 軟體版本 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1032 | `ATC_FW_VER` | 韌體版本 | ⚠️ 空 | ✅ | ✅ | ✅ | ✅ | ✅ | 3.5 ProcessHandlerCommand 為空 `break` |
| 1036 | `ATC_GET_NOW_RECIPE` | 查詢目前 Recipe 檔名 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1037 | `ATC_GET_RECIPE_LIST` | 查詢 Recipe 清單 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1054 | `ATC_GET_SN` | 查詢電源供應器序號 | ⚠️ 空 | ✅ | ✅ | ✅ | ✅ | ✅ | 3.5 ProcessHandlerCommand 為空 `break` |

### 1.4 第二感溫點指令

| Code | 常數名稱 | 說明 | 3.5 H→A | 3.5 A→H | Rogers H→A | Rogers A→H | 6.0 H→A | 6.0 A→H | 備注 |
|------|---------|------|---------|---------|-----------|-----------|--------|--------|------|
| 1028 | `ATC_SET_2ND_RANGE_P` | 設定第二感溫點正極限 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1029 | `ATC_SET_2ND_RANGE_N` | 設定第二感溫點負極限 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1030 | `ATC_SET_2ND_TIME` | 設定第二感溫點間隔時間 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1034 | `ATC_SET_2ND_ENABLED` | 啟用第二感溫點功能 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1035 | `ATC_READ_OFFSET` | 讀取各 ch Offset 值 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 6.0 額外解析 `sDataList->Strings[3]` 擴充參數 |
| 1041 | `ATC_SITE_2ND_CHECK` | 各 Site 第二感溫點 Check 啟停 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1045 | `ATC_GET_2ND_STATUS` | 查詢第二感溫點設定狀態 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 6.0 回傳用 `asATC_SendData` |

### 1.5 Self-Test 指令

| Code | 常數名稱 | 說明 | 3.5 H→A | 3.5 A→H | Rogers H→A | Rogers A→H | 6.0 H→A | 6.0 A→H | 備注 |
|------|---------|------|---------|---------|-----------|-----------|--------|--------|------|
| 1042 | `ATC_SET_TJ_OFFSET` | 設定 Tj Offset（all ch） | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1043 | `ATC_SET_SINGLE_TJ_OFFSET` | 設定單一 Site Tj Offset | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1044 | `ATC_RUN_SELFTEST` | 啟動自動 Self-Test | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | **Rogers / 6.0 的 case 1044 均缺 `break;` → fall-through 到 1045！** |
| 1046 | `ATC_MANUAL_SELFTEST` | 手動 Self-Test | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1047 | `ATC_SELFTEST_STATUS` | 查詢 Self-Test 進行狀態 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1052 | `ATC_SELFTEST_RESULT` | 查詢 Self-Test 結果 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 回傳 0:Fail 1:Pass |

### 1.6 進階功能指令

| Code | 常數名稱 | 說明 | 3.5 H→A | 3.5 A→H | Rogers H→A | Rogers A→H | 6.0 H→A | 6.0 A→H | 備注 |
|------|---------|------|---------|---------|-----------|-----------|--------|--------|------|
| 1021 | `ATC_USE_TSD` | 啟用 TSD 功能（ATC 7.0） | ✅ | ❌ | ✅ | ❌ | ✅ | ❌ | 三版本均無 ACK |
| 1022 | `ATC_EMG_UP_DOWN` | Handler EMG 升降（ATC 7.0） | ⚠️ 空 | ❌ | ✅ | ❌ | ✅ | ❌ | 3.5 為空 `break`；Rogers/6.0 有 `CommandTranslate` |
| 1026 | `ATC_RUN_MODULE` | 啟動 ATC 模組（ATC 5.0） | ⚠️ 空 | ✅ | ✅ | ✅ | ✅ | ✅ | 3.5 ProcessHandlerCommand 為空 `break` |
| 1027 | `ATC_STOP_MODULE` | 停止 ATC 模組（ATC 5.0） | ⚠️ 空 | ✅ | ✅ | ✅ | ✅ | ✅ | 3.5 ProcessHandlerCommand 為空 `break` |
| 1049 | `ATC_AIRMACHINE_STATUS` | 查詢冷機狀態（ATC 5.1） | ⚠️ 空 | ✅ | ✅ | ✅ | ✅ | ✅ | 3.5 ProcessHandlerCommand 為空 `break` |
| 1062 | `ATC_READ_SOCKETTEMP` | 讀取 Socket 溫度 | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | **6.0 無此 case**；3.5/Rogers 回傳 4 ch Socket 溫度 |
| 1063 | `HANDLER_FFC_SETTING` | FFC 資料設定 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1065 | `HANDLER_FFC_ENABLED` | 啟用/停用 FFC | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | |
| 1066 | `HANDLER_FFC_TRIGGER` | FFC 觸發（各 Site） | ✅ | ⚠️ | ✅ | ✅ | ✅ | ✅ | **3.5 不呼叫 SendCommandToHandler（無 ACK）**；Rogers/6.0 有 ACK |
| 1067 | `ATC_GET_CONTROL_MODE` | 讀取各 ch 控制模式 | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | **6.0 無此 case** |
| 1072 | `ATC_READ_FUNCTION_STATUS` | 讀取 ATC Function 狀態 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 6.0 case 1066+1072+1073 共用同一 CommandTranslate 呼叫塊 |
| 1074 | `ATC_AUTO_TCTS_ENABLED` | 自動 Tc/Ts 切換功能 | ✅ | ⚠️ | ✅ | ⚠️ | ❌ | ❌ | **6.0 無此 case**；3.5/Rogers 均呼叫 `SendCommandToHandler(1074)` 但無對應 case → ACK 不發出 |

### 1.7 PFC 功能

| Code | 常數名稱 | 說明 | 3.5 H→A | 3.5 A→H | Rogers H→A | Rogers A→H | 6.0 H→A | 6.0 A→H | 備注 |
|------|---------|------|---------|---------|-----------|-----------|--------|--------|------|
| 1068 | `HANDLER_PFC_ENABLED` | 啟停各 ch PFC | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | **6.0 無此 case** |
| 1069 | `ATC_GET_PFC_PARAMETER` | 讀取 PFC 參數（280組） | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | **6.0 無此 case** |
| 1070 | `ATC_SET_PFC_PARAMETER` | 設定 PFC 參數 | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | **6.0 無此 case** |

### 1.8 水閥 / I/O 讀取

| Code | 常數名稱 | 說明 | 3.5 H→A | 3.5 A→H | Rogers H→A | Rogers A→H | 6.0 H→A | 6.0 A→H | 備注 |
|------|---------|------|---------|---------|-----------|-----------|--------|--------|------|
| 1040 | `ATC_SEND_TEMP_READY` | ATC 主動上報溫度就緒 | ❌ | ✅ | ❌ | ✅ | ❌ | ✅ | **A→H 主動發送**；Handler 不傳此指令 |
| 1071 | `ATC_READ_WATER_VALVE` | 讀取水閥開度 | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | **6.0 無此 case**；回傳 `ctrlPara.iMotorPos` |

### 1.9 TJ 斜率 / Offset 校正

| Code | 常數名稱 | 說明 | 3.5 H→A | 3.5 A→H | Rogers H→A | Rogers A→H | 6.0 H→A | 6.0 A→H | 備注 |
|------|---------|------|---------|---------|-----------|-----------|--------|--------|------|
| 1076 | `ATC_SET_TJ_ENABLED` | 啟用 TJ 自動切換功能 | ✅ | ⚠️ | ✅ | ✅ | ✅ | ❌ | 6.0 H→A：`bATC_SlopeSaveOnHandler` 啟用時送出（uLotInfo case 13）；**6.0 ATC server 無此 case → ACK 不發出**；3.5 SendCommandToHandler 無 case → ACK 不發出 |
| 1077 | `ATC_SET_TJ_PARAMETER` | 設定 TJ Slope / Offset | ✅ | ⚠️ | ✅ | ✅ | ✅ | ❌ | 6.0 H→A：`bATC_SlopeSaveOnHandler` 啟用時送出（uLotInfo case 14）；**6.0 ATC server 無此 case → ACK 不發出**；3.5 同上 ACK 問題 |
| 1084 | `ATC_GET_SLOPE_OFFSET` | 讀取 Slope/Offset 值 | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | **6.0 無此 case** |
| 1085 | `ATC_SET_SLOPE_OFFSET` | 設定 Slope/Offset 值 | ✅ | ⚠️ | ✅ | ⚠️ | ❌ | ❌ | **6.0 無此 case**；3.5/Rogers 只設 flag 無立即 ACK |
| 1087 | `ATC_RECORD_TJ_TEMP` | Arm 別 Tj(Max/Min/Avg) 記錄 | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | **6.0 無此 case** |
| 1088 | `ATC_QUERY_TJ_TEMP` | 查詢 Tj 統計數據 | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | **6.0 無此 case** |

### 1.10 Chiller 查詢

| Code | 常數名稱 | 說明 | 3.5 H→A | 3.5 A→H | Rogers H→A | Rogers A→H | 6.0 H→A | 6.0 A→H | 備注 |
|------|---------|------|---------|---------|-----------|-----------|--------|--------|------|
| 1103 | `ATC_READ_CHILLER_TEMP` | 讀取 Chiller 目前溫度 | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | **6.0 已定義但 ProcessHandlerCommand 無此 case** |
| 1104 | `ATC_SET_PF_PARAMETER` | 設定 PF Slope / Watt-Gain | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | **6.0 已定義但 ProcessHandlerCommand 無此 case** |

### 1.11 水閥控制（TC）

| Code | 常數名稱 | 說明 | 3.5 H→A | 3.5 A→H | Rogers H→A | Rogers A→H | 6.0 H→A | 6.0 A→H | 備注 |
|------|---------|------|---------|---------|-----------|-----------|--------|--------|------|
| 1125 | `ATC_SET_TC_WATER_VALVE` | 設定 TC 水閥 | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | **6.0 無此 case**（6.0 代碼庫早於此功能加入） |

---

## 二、僅 ATC_Rogers 有實作的指令

| Code | 常數名稱 | 說明 | Rogers H→A | Rogers A→H | 備注 |
|------|---------|------|-----------|-----------|------|
| 1080 | `ATC_SWAP_CH2CH3_DATALOG` | 交換 Log 中 Ch2/Ch3 資料位置 | ✅ | ⚠️ | 呼叫 `SendCommandToHandler(1080)` 但 SendCommandToHandler 無此 case，ACK 不發出 |
| 1081 | `ATC_READ_TCTJ_WATER_VALVE` | 讀取 TC+TJ 水閥值 | ✅ | ✅ | 回傳格式 `@1081,2,TcWV,TjWV#` |
| 1082 | `ATC_READ_TJ_PARAMETER` | 讀取 TJ Slope/Offset | ✅ | ✅ | 回傳格式 `@1082,2,Slope,Offset#` |
| 1092 | `ATC_GET_HEATER_POWER` | 讀取加熱器輸出功率 | ✅ | ✅ | Eliot 20230427 |
| 1098 | `ATC_LOAD_RECIPE_SINGLE` | 載入單一 ch 的 Recipe 設定 | ✅ | ✅ | Eliot 20230809；格式：`@1098,3,File,Temp,CH#` |
| 1102 | `ATC_SAVE_RECIPE_PROHIBIT` | 禁止儲存 Recipe | ✅ | ✅ | Eliot 20231213；設 `bSaveRecipeProhibited` |
| 1105 | `ATC_SET_T2OFS` | 設定 Tc2（Ts）Offset（all ch） | ✅ | ✅ | Eliot 20240312 |
| 1106 | `ATC_SET_SINGLE_T2OFS` | 設定單一 Site Tc2 Offset | ✅ | ✅ | Eliot 20240312 |
| 1108 | `ATC_GET_CTRL_MODE` | 取得各 ch 控制模式 | ✅ | ✅ | Eliot 20240426；格式 `@1108,n,mode1,...#` |
| 1110 | `ATC_READ_WATER_FLOW` | 讀取水流量 | ✅ | ✅ | Eliot 20240514；透過 `iWaterFlowRate_ADAM[]` |
| 1112 | `ASIF_TJ_EFUSED` | MTK ASIF：Tester 讀出晶片校正資料 | ⚠️ 空 | ❌ | 僅 case break，無邏輯 |
| 1113 | `ASIF_TJ_REQUEST` | MTK ASIF：要求 ATC 傳送 Tj 資料 | ✅ | ✅ | 呼叫 `SendASIF_TJ(ASIF_TJ_REQUEST)` |
| 1114 | `ASIF_TJ_FB` | MTK ASIF：ATC 回傳 Tj 資料 | ✅ | ✅ | 呼叫 `SendASIF_TJ(ASIF_TJ_FB)` |
| 1120 | `HANDLER_2DID` | Device 2DID 與 Thermo Profile 記錄 | ✅ | ❌ | 設定 `as2DID[]` / `flagMonitor_2DID[]`，無 ACK |
| 1130 | `ATC_CHILLER_WATERWARNING` | Chiller 水警告（A→H 主動發送） | ❌ | ✅ | **A→H 主動發送**；Victor 20250915 |

---

## 三、僅 ATC_3.5 有實作的指令

| Code | 常數名稱 | 說明 | 3.5 H→A | 3.5 A→H | 備注 |
|------|---------|------|---------|---------|------|
| 1086 | `ATC_GET_TJ_VOLTAGE` | 讀取 Tj 電壓 | ✅ | ✅ | 透過 flag；Rogers SendCommandToHandler 已標為 comment；6.0 無 |
| 1122 | `ATC_Recipe_By_Channel` | 讀取 Recipe 各 ch 參數（分包） | ✅ | ✅ | Eliot 20241025；透過 flag 延遲回傳 |
| 1123 | `ATC_Recipe_By_Public` | 讀取 Recipe 公共參數（分包） | ✅ | ✅ | Eliot 20241025；透過 flag 延遲回傳 |
| 1126 | `HANDLER_ALARM` | Handler 傳送告警代碼 | ✅ | ✅ | Evan 20250428；僅處理 16113/16114/16115/16119（水漏） |
| 1127 | `ATC_SET_DYNAMIC_PID` | 設定動態 PID 組別（TSMC） | ✅ | ✅ | Dustin 20250512 |
| 1128 | `ATC_READ_DYNAMIC_PID` | 讀取動態 PID 目前組別（TSMC） | ✅ | ✅ | Dustin 20250522 |
| 1129 | `ATC_SET_MULTI_TC_OFFSET` | 設定多區 Tc Offset | ✅ | ✅ | Evan 20250812；目前支援4區 |
| 1131 | `ATC_Multi_Temperature_Control` | 啟停多點溫度控制（MTC） | ✅ | ✅ | Evan 20251010 |
| 1132 | `ATC_Handler_Transmit_Recipe` | Handler 傳送 Recipe 給 ATC（檔案傳輸） | ✅ | — | Evan 20251024；透過 `frmFileTransfer->ProcessCommand` |
| 1134 | `ATC_Star_Transmit_Recipe` | Handler 要求 ATC 送出 Recipe | ✅ | ✅ | Evan 20260122；呼叫 `CheckAndSendRecipe()` |
| 1137 | `ATC_HulkMode` | 設定 HulkMode 開/關（TSMC） | ✅ | ✅ | Evan 20260403；寫入 FunctionData.INI |

---

## 四、僅 ATC_6.0 有實作的指令

### 4.1 ATC 5.1 / 6.0 冷凍系統指令

| Code | 常數名稱 | 說明 | 6.0 H→A | 6.0 A→H | 備注 |
|------|---------|------|--------|--------|------|
| 1055 | `ATC_SHUTDOWN` | ATC 主機強制關機 | ✅ | ⚠️ | H→A via `CommandTranslate`；SendCommandToHandler 無 case 1055，ACK 依 Translate_ATC51 內部決定 |
| 1056 | `ATC_51_FREONRECOVER` | 冷媒回收（ATC 5.1） | ✅ | ⚠️ | 同上，via CommandTranslate |
| 1058 | `ATC_51_SET_REF_ENABLED` | 啟停冷凍機（ATC 5.1） | ✅ | ✅ | SendCommandToHandler case 1058 回傳 `@1058,1,N#` |
| 1059 | `ATC_51_GET_REF_STATUS` | 查詢冷凍機狀態（ATC 5.1） | ✅ | ✅ | H→A 額外讀取 `sDataList->Strings[3]`；A→H 回傳 `asATC_RefStatus` |
| 1060 | `ATC_51_SET_DEFROST` | 設定除霜（ATC 5.1） | ✅ | ✅ | SendCommandToHandler case 1060 |
| 1061 | `ATC_60_SET_AIRVALVE` | 控制氣閥（ATC 6.0，Qualcomm） | ✅ | ✅ | SendCommandToHandler case 1061 |

### 4.2 ATC 6.0 分析 / 讀取指令

| Code | 常數名稱 | 說明 | 6.0 H→A | 6.0 A→H | 備注 |
|------|---------|------|--------|--------|------|
| 1064 | `HANDLER_ATO_RECORD` | 記錄 Auto Temperature Offset 資料 | ✅ | ✅ | SendCommandToHandler case 1064；回傳 `@1064,1,N#` |
| 1073 | `ATC_GET_HEAT_OUTPUT` | 讀取加熱輸出百分比 | ✅ | ✅ | SendCommandToHandler case 1073；回傳 `asATC_SendData` |
| 1083 | `ATC_READ_HEATING` | 讀取加熱狀態（Rogers 亦有） | ✅ | ✅ | SendCommandToHandler case 1083 回傳 `asFlagRunning`；Rogers 亦實作（Victor 20250708） |
| 1091 | `HANDLER_SLK_LAYOUT` | Handler 傳入 SLK 露點資料 | ✅ | ✅ | H→A 儲存 `dDewPointTemp[]`；A→H case 1091 回傳 `@1091,1,N#` |
| 1097 | `ATC_READ_TEMP_3` | 讀取多感測器溫度（TC1/2/3/4） | ✅ | ✅ | A→H 回傳每 ch 四個感測點；格式 `@1097,N*issCtrlSwGroupNum,...#` |
| 1101 | `ATC_AIRMACHINE_TEMP` | 讀取冷機（Air Stream）溫度 | ✅ | ✅ | A→H 回傳 `dATC_AIRSTREAM[]`；支援 bTwoDecimalPlaces（×10） |

### 4.3 KL 控制指令

| Code | 常數名稱 | 說明 | 6.0 H→A | 6.0 A→H | 備注 |
|------|---------|------|--------|--------|------|
| 1095 | `ATC_KL_TRIGGER` | KL 控制啟動觸發 | ✅ | ✅ | SendCommandToHandler case 1095；2023-06-12 Zek |
| 1096 | `ATC_KL_TRIGGER_STATUS` | KL 控制觸發狀態回報 | ✅ | ✅ | SendCommandToHandler case 1096；2023-06-12 Zek |

### 4.4 ATC 6.0 特殊讀取

| Code | 常數名稱 | 說明 | 6.0 H→A | 6.0 A→H | 備注 |
|------|---------|------|--------|--------|------|
| 1099 | `ATC_GETDEWPOINTTEMP` | 讀取 Handler 端露點溫度 | ✅ | ⚠️ | H→A：case 1099 呼叫 `SendCommandToHandler(ATC_GETDEWPOINTTEMP, N)`；但 SendCommandToHandler **無 case 1099** → **ACK 不發出** |

---

## 五、已定義但三版本均未實作的指令

以下指令在 `ATC_Server.h` 中已定義，但在三版本的 `ProcessHandlerCommand` / `SendCommandToHandler` 中均無對應 case（或僅定義未使用）：

| Code | 常數名稱 | 說明 | 備注 |
|------|---------|------|------|
| 1057 | `HANDLER_CAN_SELFTEST` | 詢問 Handler 是否可執行 Self-Test | 三版本均未實作 |
| 1075 | `HANDLER_TEST_SITEMAPPING` | Handler 傳送 Site Mapping | 三版本均未實作 |
| 1078 | `ATC_51_SET_ExtraHeatGun` | 設定加熱棒（ATC 5.1） | 三版本均未實作 |
| 1079 | `ATC_51_READ_ExtraHeatGun` | 讀取加熱棒狀態（ATC 5.1） | 三版本均未實作 |
| 1089 | `ATC_51_SET_REF_ADJUST` | 設定冷凍機閥門（ATC 5.1） | 6.0 已定義；3.5/Rogers 尚未定義 |
| 1090 | `ATC_51_AUTOLOAD_RECIPE` | 自動載入 Golden Recipe | 6.0 已定義；3.5/Rogers 尚未定義 |
| 1093 | `ATC_KL_SV` | KL 傳送設定溫度給 Handler | 6.0 已定義；3.5/Rogers 尚未定義 |
| 1094 | `ATC_KL_OFFSET` | KL 傳送 Offset 給 Handler | 6.0 已定義；3.5/Rogers 尚未定義 |
| 1098 | `ATC_LOAD_RECIPE_SINGLE` | 載入單一 ch Recipe（Rogers 已實作） | 6.0 已定義但 ProcessHandlerCommand 無 case |
| 1100 | `ATC_51_SET_TJ_WATCHDOG` | 設定 TJ Watchdog 延遲/電壓範圍 | 6.0 H→A：`bATC_SlopeSaveOnHandler` 且 `bEnableTJFunction` 時送出（uLotInfo case 16）；**6.0 ATC server 無此 case → ACK 不發出** |
| 1102 | `ATC_SAVE_RECIPE_PROHIBIT` | 禁止儲存 Recipe（Rogers 已實作） | 6.0 已定義但 ProcessHandlerCommand 無 case |
| 1111 | `ATC_SET_CoolingValue` | 設定冷卻值 | 6.0 已定義；未實作 |
| 1115–1119 | `ASIF_TJ_CPU/SOC/GPU/FPC/EOT` | MTK ASIF 指定感測器 Tj 資料 | Rogers 已定義但未實作；6.0 未定義 |
| 1121 | `ATC_SET_WValve_ENABLED` | 啟停水閥（Water Valve Enabled） | 2024/10/08 Cheng；已定義未實作 |
| 1124 | `ATC_SET_CHILLER_ENABLED` | 啟停 Chiller | 2025/01/16 Cheng；已定義未實作 |
| 1133 | `ATC_DEFROSTING` | ATC 除霜（Defrosting） | 20251217 Victor；已定義未實作 |
| 1135 | `ATC_SET_SINGLE_OFFSET_MTP` | 設定單點 Offset（MTP） | 20260226 Victor；已定義未實作 |
| 1136 | `ATC_51_ATO_AdjustValve` | Auto K-Temp 自動調整閥門（ATC 5.1） | 6.0 已定義（2026-02-10 Allen）；未實作 |

---

## 六、已知實作問題（Bug / 遺漏）

| # | 問題描述 | 影響版本 |
|---|---------|---------|
| 1 | **1074 (`ATC_AUTO_TCTS_ENABLED`)**：H→A 處理後呼叫 `SendCommandToHandler(1074,1)`，但 `SendCommandToHandler` 無 case 1074 → ACK 不發出，Handler 收不到回應 | ATC_3.5 & Rogers |
| 2 | **1076/1077 (`ATC_SET_TJ_ENABLED/PARAMETER`)**：3.5 ProcessHandlerCommand 中呼叫 `SendCommandToHandler(1076/1077)`，但 ATC_3.5 的 `SendCommandToHandler` 無對應 case → ACK 不發出（Rogers 已修正，有 case 1076/1077） | ATC_3.5 |
| 3 | **1080 (`ATC_SWAP_CH2CH3_DATALOG`)**：Rogers ProcessHandlerCommand 呼叫 `SendCommandToHandler(1080,1)`，但 `SendCommandToHandler` 無 case 1080 → ACK 不發出 | Rogers |
| 4 | **1044 (`ATC_RUN_SELFTEST`)**：Rogers 與 ATC_6.0 的 case 1044 均缺少 `break;`，直接 fall-through 執行 case 1045 (`ATC_GET_2ND_STATUS`) 的 `CommandTranslate` | Rogers & ATC_6.0 |
| 5 | **1085 (`ATC_SET_SLOPE_OFFSET`)**：3.5/Rogers 均只設 flag，不直接發 ACK，需等待 Timer 才回傳 Slope/Offset 資料給 Handler | ATC_3.5 & Rogers |
| 6 | **1099 (`ATC_GETDEWPOINTTEMP`)**：ATC_6.0 case 1099 呼叫 `SendCommandToHandler(ATC_GETDEWPOINTTEMP, N)`，但 `SendCommandToHandler` 無 case 1099 → ACK 不發出 | ATC_6.0 |
| 7 | **1066 (`HANDLER_FFC_TRIGGER`)**：ATC_3.5 中 case 1066 呼叫 `tSystem->SendStartTrigger(...)` 後未呼叫 `SendCommandToHandler` → Handler 收不到 ACK（Rogers 有 ACK，6.0 有 ACK） | ATC_3.5 |

---

## 附錄：三版本指令支援差異摘要

| 功能類別 | ATC_3.5 獨有 | Rogers 獨有 | ATC_6.0 獨有 |
|---------|------------|------------|-------------|
| TSMC 功能 | 1127 Dynamic PID、1131 MTC、1137 HulkMode | — | — |
| Recipe 傳輸 | 1122/1123 分包、1132/1134 檔案傳輸 | 1098 單 ch 載入、1102 禁止儲存 | — |
| Handler Alarm | 1126 （水漏 16113~16119） | — | — |
| 多區 Offset | 1129 Multi Tc Offset | 1105/1106 Tc2(Ts) Offset | — |
| TJ 完整支援 | 1076/1077（ACK 有 bug） | 1076/1077（ACK 正常）、1081/1082 | — |
| 水流 / 水警 | — | 1110 水流量、1130 水警告 | — |
| MTK ASIF | — | 1112~1114、1120 2DID | — |
| 加熱器功率 | 1086 TJ 電壓 | 1092 加熱器功率、1083 加熱狀態 | 1083 加熱狀態、1073 加熱輸出% |
| 控制模式查詢 | — | 1108 Ctrl Mode | — |
| 冷凍系統 | — | — | 1055 關機、1056 冷媒回收、1058~1060 冷凍機控制 |
| 氣閥控制 | — | — | 1061 氣閥（Qualcomm） |
| 多感測器溫度 | — | — | 1097 TC1/2/3/4 四路溫度 |
| 冷機氣溫 | — | — | 1101 Air Stream 溫度 |
| KL 控制 | — | — | 1095/1096 KL Trigger |
| 露點 | — | — | 1091/1099 露點溫度 |
| ATO 記錄 | — | — | 1064 ATO Record |
| PFC/FFC | 1068/1069/1070 PFC 完整 | 1068/1069/1070 PFC 完整 | ❌（6.0 無此功能） |
| 水閥進階 | 1071 TC 水閥讀取、1125 TC 水閥設定 | 1071+1081 TC/TJ 水閥讀取、1125 TC 水閥設定 | ❌（6.0 無此功能） |
