# ATC H→A 命令封包內文（Command Payload Reference）

**資料來源**：`ATC/ATC_Handler_Side.cpp` `SendCommand()` switch  
**更新日期**：2026-04-15  
**封包基本格式**：`@<Code>,<DataCount>,<D1>,<D2>,...,#`  
**注意**：數值型溫度傳輸前乘以 10（整數，0.1°C 精度），除非個別說明。

---

## 封包格式說明總表

**保護欄位說明**：
- `—` 無版本限制，所有版本均可送出
- `⛔ 6.0/7.0` 函式開頭 early return，6.0/7.0 完全封鎖（Steven 20260415）
- `⚠️ 6.0/7.0(cond)` 條件包圍，僅封鎖 SendCommand，flag 等邏輯仍執行（Steven 20260415）
- `🔒 TYPE_33` 限制僅 Rogers（TYPE_33）可送出（Steven 20260415）
- `📌 6.0 H→A` 有意設計的 6.0 送出路徑，但 ATC server 目前無 ACK

| Code | 常數名稱 | H→A 封包格式 | 欄位說明 | 6.0/7.0 保護 |
|------|---------|-------------|---------|-------------|
| 1001 | `ATC_RECIPE_FILE` | `@1001,2,<FileName>,<SelTemp>#` | FileName: Recipe 檔名字串；SelTemp: 選擇溫度（整數） | — |
| 1002 | `ATC_SET_TEMP` | `@1002,N,D1,D2,...,DN,#` | N=iDataCount；Di=目標溫度值（×10） | — |
| 1003 | `ATC_SET_TOFS` | `@1003,N,D1,...,DN,#` | N=iDataCount；Di=Offset 值（×10） | — |
| 1004 | `ATC_SITE_ENABLED` | `@1004,N,D1,...,DN,#` | N=iDataCount；Di=0/1（停用/啟用） | — |
| 1005 | `ATC_SET_TRM` | `@1005,1,<Mode>,#` | Mode: 溫度讀取來源模式代碼 | — |
| 1006 | `ATC_SET_MAX_TEMP` | `@1006,1,<MaxTemp>,#` | MaxTemp=最高工作溫度（×10） | — |
| 1007 | `ATC_RUN_STOP` | `@1007,1,<State>,#` | State: 1=Run, 0=Stop | — |
| 1008 | `ATC_INITIAL_SET` | `@1008,1,<Value>,#` | 初始化全部設定 | — |
| 1009 | `ATC_MODE_TYPE` | `@1009,1,<Value>,#` | 查詢 ATC 型號（回傳 iATC_MODE_TYPE） | — |
| 1010 | `ATC_READ_TEMP` | `@1010,1,<SiteCount>,#` | SiteCount=iHandlerSiteCount | — |
| 1011 | `ATC_HANDLER_ARM` | `@1011,1,<ArmIndex>,#` | ArmIndex: 0=Front/1=Rear | — |
| 1012 | `ATC_TEST_START` | `@1012,N,D1,...,DN,#` | N=iDataCount；含測試開始/結束旗標 | — |
| 1013 | `ATC_SEND_TJ` | `@1013,1,<Value>,#` | 已廢棄，不再使用 | — |
| 1014 | `ATC_ERROR` | `@1014,1,ATC_ERROR_OK,#` | 固定格式；告知 ATC 錯誤已確認 | — |
| 1015 | `ATC_SET_SINGLE_TEMP` | `@1015,2,<Site>,<Temp>#` | Site: 1-based；Temp: ×10 | — |
| 1016 | `ATC_SET_SINGLE_OFFSET` | `@1016,2,<Site>,<Offset>#` | Site: 1-based；Offset: ×10 | — |
| 1017 | `ATC_CANCEL_MESSAGE` | `@1017,0#` | 無資料欄位，通知 ATC 關閉告警視窗 | — |
| 1018 | `ATC_CHILLER_STATUS` | `@1018,1,<Value>,#` | 查詢 Chiller 狀態 | — |
| 1019 | `ATC_SET_PID` | `@1019,N*3,P1,I1,D1,...,PN,IN,DN,#` | N=ch 數；每組 P/I/D 均除以 10.0；共 N×3 筆 | — |
| 1020 | `ATC_READ_PID` | `@1020,1,1,#` | 要求回傳目前 PID 值 | — |
| 1021 | `ATC_USE_TSD` | `@1021,N,D1,...,DN,#` | 啟用 TSD 功能（ATC 7.0） | — |
| 1022 | `ATC_EMG_UP_DOWN` | `@1022,N,D1,...,DN,#` | Handler EMG 升降（ATC 7.0） | — |
| 1023 | `ATC_READ_TEMP_2` | `@1023,1,<SiteCount>,#` | 讀取第二感溫點溫度 | — |
| 1024 | `ATC_READ_SETTEMP` | `@1024,1,<Value>,#` | 讀取目前設定溫度 | — |
| 1025 | `ATC_STATE` | `@1025,1,1,#` | 讀取 ATC 運行狀態（iSendData[0]=1） | — |
| 1026 | `ATC_RUN_MODULE` | `@1026,1,<Value>,#` | 啟動 ATC 模組（ATC 5.0） | — |
| 1027 | `ATC_STOP_MODULE` | `@1027,1,<Value>,#` | 停止 ATC 模組（ATC 5.0） | — |
| 1028 | `ATC_SET_2ND_RANGE_P` | `@1028,N,D1,...,DN,#` | 設定各 ch 第二感溫點正極限 | — |
| 1029 | `ATC_SET_2ND_RANGE_N` | `@1029,N,D1,...,DN,#` | 設定各 ch 第二感溫點負極限 | — |
| 1030 | `ATC_SET_2ND_TIME` | `@1030,1,<Seconds>,#` | 第二感溫點量測間隔時間（秒） | — |
| 1031 | `ATC_SW_VER` | `@1031,1,<VersionString>#` | VersionString=`asHandlerVer`（Handler 版本字串） | — |
| 1032 | `ATC_FW_VER` | `@1032,1,<Value>,#` | 查詢 ATC 韌體版本 | — |
| 1033 | `ATC_TIME_SYNC` | `@1033,1,MM/DD/YYYY HH:NN:SS AM/PM#` | Windows 格式時間字串；每 60 秒由 SendReadTempComm 自動送出 | — |
| 1034 | `ATC_SET_2ND_ENABLED` | `@1034,1,<Enable>,#` | 0/1 啟停第二感溫點功能 | — |
| 1035 | `ATC_READ_OFFSET` | `@1035,1,<Value>,#` | 讀取各 ch Offset | — |
| 1036 | `ATC_GET_NOW_RECIPE` | `@1036,1,<Value>,#` | 查詢目前 Recipe 檔名 | — |
| 1037 | `ATC_GET_RECIPE_LIST` | `@1037,1,<Value>,#` | 查詢 Recipe 清單 | — |
| 1038 | `ATC_USE_TJ_MODE` | `@1038,N,D1,...,DN,#` | N=iDataCount；Di=0(TC)/1(TJ)/2(TS)，來自 ATCCONTROLMODEMode | ⚠️ 6.0/7.0(cond) |
| 1039 | `ATC_SET_CHILLER_TEMP` | `@1039,1,<ChillerTemp>,#` | Chiller 目標溫度（×10） | — |
| 1040 | `ATC_SEND_TEMP_READY` | `@1040,N,D1,...,DN,#` | 通知溫度就緒（A→H 為主動發送方向）| — |
| 1041 | `ATC_SITE_2ND_CHECK` | `@1041,N,D1,...,DN,#` | 各 Site 第二感溫點啟停 | — |
| 1042 | `ATC_SET_TJ_OFFSET` | `@1042,N,D1,...,DN,#` | N=ch 數；Di=TJ Offset（×10） | — |
| 1043 | `ATC_SET_SINGLE_TJ_OFFSET` | `@1043,N,D1,...,DN,#` | 設定單一 Site TJ Offset | — |
| 1044 | `ATC_RUN_SELFTEST` | `@1044,1,<Value>,#` | 啟動自動 Self-Test | — |
| 1045 | `ATC_GET_2ND_STATUS` | `@1045,1,<Value>,#` | 查詢第二感溫點設定狀態 | — |
| 1046 | `ATC_MANUAL_SELFTEST` | `@1046,1,<Value>,#` | 手動 Self-Test | — |
| 1047 | `ATC_SELFTEST_STATUS` | `@1047,1,<Value>,#` | 查詢 Self-Test 進行狀態 | — |
| 1048 | `HANDLER_STATUS` | `@1048,1,<Value>,#` | Handler Ping / 狀態查詢 | — |
| 1049 | `ATC_AIRMACHINE_STATUS` | 一般：`@1049,3,<UseAir>,D1,D2,#`<br>TriTemp：`@1049,6,D1,...,D6,#` | 依 iATC_MODE_TYPE==61 切換格式；含 bUseAirMachine 旗標 | — |
| 1050 | `ATC_LOT_START` | `@1050,1,<LotID>#` | LotID=`asLotID` 字串 | — |
| 1051 | `ATC_LOT_END` | `@1051,1,<LotID>#` | LotID=`asLotID` 字串 | — |
| 1052 | `ATC_SELFTEST_RESULT` | `@1052,1,<Value>,#` | 查詢 Self-Test 結果（0=Fail/1=Pass） | — |
| 1053 | `HANDLER_ABNORMAL_SITE` | `@1053,N,D1,...,DN,#` | 各 Site 異常旗標 | — |
| 1054 | `ATC_GET_SN` | `@1054,1,<Value>,#` | 查詢電源供應器序號 | — |
| 1058 | `ATC_51_SET_REF_ENABLED` | `@1058,<CompNum>,D1,...,D_CompNum,#` | Total_Compressor 個值；0/1 啟停冷凍機 | — |
| 1059 | `ATC_51_GET_REF_STATUS` | 簡易：`@1059,1,<Mode>,#`<br>全量：`@1059,2,<Mode>,<StatusStr>#` | bReadRefrigerantMode_AllStatus 決定格式 | — |
| 1060 | `ATC_51_SET_DEFROST` | `@1060,1,<Value>,#` | 設定除霜（ATC 5.1） | — |
| 1061 | `ATC_60_SET_AIRVALVE` | `@1061,N,D1,...,DN,#` | 控制氣閥開關（ATC 6.0 Qualcomm） | — |
| 1063 | `HANDLER_FFC_SETTING` | `@1063,N,D1,...,DN#` | N=iDataCount；FFC 資料陣列（最後一筆不加逗號） | — |
| 1064 | `HANDLER_ATO_RECORD` | `@1064,N,D1,...,DN,#` | Auto K-Temp 資料紀錄 | — |
| 1065 | `HANDLER_FFC_ENABLED` | `@1065,1,<Enable>#` | iSendData[1]; 0/1 啟停 FFC | — |
| 1066 | `HANDLER_FFC_TRIGGER` | `@1066,16,D1,...,D16,#` | 固定 16CH；各 Site FFC 觸發旗標 | — |
| 1067 | `ATC_GET_CONTROL_MODE` | `@1067,1,<Value>,#` | 讀取各 ch 控制模式 | ⚠️ 6.0/7.0(cond) |
| 1069 | `ATC_GET_PFC_PARAMETER` | `@1069,1,1#` | 讀取 PFC 參數（固定送 DataCount=1, Value=1） | — |
| 1070 | `ATC_SET_PFC_PARAMETER` | `@1070,21,<asSetPFCData>#` | 固定 21 筆 PFC 資料；格式範例：`80,100,80,70,...` | — |
| 1071 | `ATC_READ_WATER_VALVE` | `@1071,1,<SiteCount>,#` | 讀取水閥開度 | ⛔ 6.0/7.0 |
| 1072 | `ATC_READ_FUNCTION_STATUS` | `@1072,1,<Value>,#` | 讀取 ATC Function 狀態（eKeep 判斷） | — |
| 1075 | `HANDLER_TEST_SITEMAPPING` | `@1075,N,D1,...,DN,#` | Handler 傳送 Site Mapping | — |
| 1076 | `ATC_SET_TJ_ENABLED` | `@1076,1,<Enable>,#` | Enable=0/1；6.0 H→A 在 case 13 觸發（**6.0 ATC server 無 ACK**） | 📌 6.0 H→A |
| 1077 | `ATC_SET_TJ_PARAMETER` | `@1077,2,<Slope>,<Offset>#` | Slope/Offset 為字串格式（由 asSendData[0]/[1] 組成）；6.0 case 14 觸發（**6.0 ATC server 無 ACK**） | 📌 6.0 H→A |
| 1084 | `ATC_GET_SLOPE_OFFSET` | `@1084,1,<SiteCount>#` | iDataCount=SiteCount；讀取 Slope/Offset | ⛔ 6.0/7.0 |
| 1085 | `ATC_SET_SLOPE_OFFSET` | `@1085,<N*2>,<asSetPFCData>#` | N=iATC_Use_Heat_Count；每 ch 含 Slope+Offset 共 N×2 筆 | ⛔ 6.0/7.0 |
| 1086 | `ATC_GET_TJ_VOLTAGE` | `@1086,1,<SiteCount>#` | 讀取 Tj 電壓（透過 flag 延遲回傳） | — |
| 1087 | `ATC_RECORD_TJ_TEMP` | `@1087,2,<ArmSel>,<StartStop>#` | ArmSel: -1=none/0=Arm1/1=Arm2/2=Both；StartStop: 0=Stop/1=Start | ⛔ 6.0/7.0 |
| 1088 | `ATC_QUERY_TJ_TEMP` | `@1088,1,1#` | 讀取 1 組 Tj 統計（Max/Min/Avg） | ⛔ 6.0/7.0 |
| 1091 | `HANDLER_SLK_LAYOUT` | `@1091,3,<D1>,<D2>,<D3>,#` | 三點露點資料（含 Index、InShuttle、OutShuttle） | — |
| 1095 | `ATC_KL_TRIGGER` | `@1095,3,<FileName>,<D1>,<D0>#` | KL 控制觸發；asChangeFile=Recipe 檔名 | — |
| 1096 | `ATC_KL_TRIGGER_STATUS` | `@1096,1,<Value>,#` | 查詢 KL 觸發狀態 | — |
| 1097 | `ATC_READ_TEMP_3` | `@1097,1,<SiteCount>,#` | 讀取多感測器溫度（TC1~TC4 per ch） | — |
| 1099 | `ATC_GETDEWPOINTTEMP` | `@1099,3,<D1>,<D2>,<D3>,#` | 三點露點溫度數值 | — |
| 1100 | `ATC_51_SET_TJ_WATCHDOG` | `@1100,5,<Enable>,<Delay>,<VLow>,<VHigh>,<Continue>#` | 5 欄位均為字串格式（asSendData[0~4]）；6.0 case 16 觸發（**6.0 ATC server 無 ACK**） | 📌 6.0 H→A |
| 1101 | `ATC_AIRMACHINE_TEMP` | 依 iATC_MODE_TYPE 格式不同，同 1049 | （僅 ATC 6.0 有實作） | — |
| 1104 | `ATC_SET_PF_PARAMETER` | `@1104,5,<Enable>,<FullPower>,<PFSlope>,<WGain>,<Many2one>#` | Enable/FullPower/Many2one 為整數；PFSlope/WGain 為浮點字串 | ⛔ 6.0/7.0 |
| 1105 | `ATC_SET_T2OFS` | `@1105,N,D1,...,DN,#` | N=ch 數；Di=Tc2 Offset（×10） | 🔒 TYPE_33 |
| 1106 | `ATC_SET_SINGLE_T2OFS` | `@1106,N,D1,...,DN,#` | 設定單一 Site Tc2 Offset | — |
| 1112 | `ASIF_TJ_EFUSED` | `@1112,N,ASIF_TJ_EFUSED,"D1","D2",...#` | N=iDataCount；各欄位用雙引號包覆 | — |
| 1113 | `ASIF_TJ_REQUEST` | `@1113,1,ASIF_TJ_REQUEST#` | 固定格式；要求 ATC 傳送 Tj | — |
| 1114 | `ASIF_TJ_FB` | `@1114,1,ASIF_TJ_FB#` | 固定格式 | — |
| 1120 | `HANDLER_2DID` | 單雙站：`@1120,<UseCount>,<ArmSel>,<2DIDStr>#`<br>停止：`@1120,1,-1#` | iSendData[0]=use site count；iSendData[1]=0/1/其他 决定格式 | — |
| 1125 | `ATC_SET_TC_WATER_VALVE` | `@1125,N,<asTCWaterValue>#` | N=iDataCount；asTCWaterValue 含各 ch 水閥目標開度，以逗號分隔 | ⛔ 6.0/7.0 |
| 1127 | `ATC_SET_DYNAMIC_PID` | `@1127,N,D1,...,DN,#` | N=iDataCount；各 ch 動態 PID 組別 | — |
| 1128 | `ATC_READ_DYNAMIC_PID` | `@1128,1,<SiteCount>#` | iDataCount=SiteCount；讀取動態 PID 組別 | — |
| 1129 | `ATC_SET_MULTI_TC_OFFSET` | `@1129,N,D1,...,DN#` | N=iDataCount；各區 Tc Offset（最後一筆不加逗號） | — |
| 1131 | `ATC_Multi_Temperature_Control` | `@1131,N,D1,...,DN,#` | N=iDataCount；各區 MTC 啟停旗標 | — |

---

## 注意事項

1. **`iSendData[]`** 陣列在呼叫 `SendCommand()` 前由各 API 函式填入。
2. **`asSendData[]`** 陣列用於字串格式資料（如 Slope/Offset 浮點字串）。
3. **`iDataCount`** 多數情況為 `iATC_Use_Heat_Count`（加熱 ch 數），部分指令固定值。
4. 標示「**已封鎖**」的指令在 `iATC_MODE_TYPE==ATC_TYPE_60` 或 `ATC_TYPE_70` 時函式開頭 `return`，不會送出封包（kevin 20260415）。
5. **1076/1077/1100** 在 ATC 6.0 有 H→A 傳送路徑（uLotInfo case 13~16），但 ATC server 端無對應 case，不回 ACK。

---

## A→H 回傳備注

A→H 方向（ATC 主動發送或 ACK）的封包格式記錄在 `ProcessReceiveString_ATC()` switch 的各 case，非本文件範圍。   
詳見 [Handler_Command_Report.md](Handler_Command_Report.md) 各欄位「A→H」與備注說明。
