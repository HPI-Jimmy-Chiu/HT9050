---
name: ht9045-atc-interface
description: >
  HT9045 Handler ↔ ATC 溫控設備 TCP 通訊協定知識庫。涵蓋 137 個 ATC 命令（1001-1137）、封包格式、Handler Side 類別、溫度/Offset 傳送流程、Site Mapping、Recipe、Self-Test、TJ/FFC/PFC、MTK ASIF、TSMC Hulk 等。Use when: 分析 ATC 通訊 log、追 @1002/@1003/@1004 等封包內容、除錯 ATC 不回應、新增/修改 ATC 命令、解析 ATC hangup 資料夾 TXT 檔。關鍵字：ATC, ATC Interface, ATC_SET_TEMP, ATC_SET_TOFS, ATC_SITE_ENABLED, ATC_Handler_Side, SendCommand, GetData, @1001, @1002, @1003, @1004, @1016, @1105, ATC_Multi_Temperature_Control, ATC_SET_T2OFS, HANDLER_2DID, ATC Recipe, Self-Test, TJ Offset, FFC, PFC, MTK ASIF, TSMC Hulk。
  （20261001：V906 現況與 ATC.ini 位置 → ht9045-temperature；本 skill 與 ht9045-atc 重疊，見 ht9045-temperature references/temperature-facts-index.md §1）
---

# HT9045 Handler ↔ ATC Interface 知識庫

來源：`ATC Interface Define.xlsx`（完整 137 個命令，1001-1137）+ `ATC/ATC_Handler_Side.cpp` 實作。

## 1. 通訊協定格式

### 封包結構（TCP Socket）
```
@<command>,<count>,<data1>,<data2>,...,#
```
- `@` 起始符、`#` 結尾符、`,` 分隔符
- `<command>`: 4 碼命令編號（0001、1001~1137）
- `<count>`: 資料筆數
- `<data>`: 參數，整數/字串
- 溫度值傳送慣例：**放大 10 倍**（例 `55` 代表 5.5°C）

### 雙向命名慣例
| 命令前綴 | 方向 |
|---|---|
| `ATC_*` | Handler 送給 ATC（或 ATC 主動回報） |
| `HANDLER_*` | Handler 主動通知 / ATC 查詢 Handler |
| `ASIF_*` | MTK ASIF 專用（雙向） |

## 2. 核心程式檔案

| 檔案 | 說明 |
|------|------|
| `ATC/ATC_Handler_Side.cpp` | Handler 端主控（TATC_InterfaceForm），封包收發、SendCommand、GetData、SetOffset、SetTemp 等 |
| `ATC/ATC_Handler_Side.h` | 類別宣告 |
| `ATC/ATCInterface.cpp` | 舊版單站 ATC 介面 |
| `ATC/ATCSystem.cpp` | ATC 系統層封裝 |
| `ATC/TCPData.cpp` | 底層 TCP 封包解析 |
| `ATC/WinWaySetting.cpp` | WinWay 變種設定 |
| `uTemp_Set.cpp` | 溫度/Offset UI + `iSiteToATC[][][]` / `iSiteToOfs[][][]` Channel Mapping |
| `TempCtrl/TriTemp.cpp` | TriTemp（冷熱凍三區）整合 |

### 命令編號定義位置
`ATC/ATC_Handler_Side.cpp` L21-L150 有完整的 `#define ATC_XXX <num>` 列表（等同於 xlsx 規格）。

---

## 3. 命令分類速查（137 個）

詳細命令表請見 [references/atc-commands.md](references/atc-commands.md)。以下為分類總覽：

### 3.1 基礎控制（1001-1017）
| CODE | Name | 用途 |
|------|------|------|
| 0001 | ATC_NO_THIS_COMMAND | ATC 不支援該命令 |
| **1001** | ATC_RECIPE_FILE | 切換 Recipe |
| **1002** | ATC_SET_TEMP | 設定 ATC 工作溫度（`@1002,ch,n,n,n...` 55=5.5°C） |
| **1003** | ATC_SET_TOFS | **Temp Offset（批次）** |
| **1004** | ATC_SITE_ENABLED | 啟用/關閉 Site（`@1004,ch,n,n,...` 0=關 1=開） |
| 1005 | ATC_SET_TRM | Temp Reading Mode |
| 1006 | ATC_SET_MAX_TEMP | ATC 最大工作溫度 |
| **1007** | ATC_RUN_STOP | ATC Run/Stop（n=0 停止 1 執行） |
| 1008 | ATC_INITIAL_SET | ATC Initial All Setting |
| 1009 | ATC_MODE_TYPE | ATC Mode Type |
| **1010** | ATC_READ_TEMP | 讀取現在溫度 |
| 1011 | ATC_HANDLER_ARM | 告知 Handler 使用 Arm |
| **1012** | ATC_TEST_START | 通知 Testing 開始 / 結束（n=0 開始 n=1 結束） |
| 1013 | ATC_SEND_TJ | 送 TJ 溫度（未使用） |
| **1014** | ATC_ERROR | ATC Error Message |
| **1015** | ATC_SET_SINGLE_TEMP | 設定單站溫度 |
| **1016** | ATC_SET_SINGLE_OFFSET | **設定單站 Offset** |
| 1017 | ATC_CANCEL_MESSAGE | 關閉 ATC Alarm Form |

### 3.2 Chiller / PID / TSD（1018-1034）
- 1018 ATC_CHILLER_STATUS（ATC 3.0/6.0）
- 1019 ATC_SET_PID / 1020 ATC_READ_PID
- 1021 ATC_USE_TSD（ATC 7.0）
- 1022 ATC_EMG_UP_DOWN（ATC 7.0）
- 1023 ATC_READ_TEMP_2（第二感溫點）
- 1024 ATC_READ_SETTEMP / 1025 ATC_STATE
- 1026 ATC_RUN_MODULE / 1027 ATC_STOP_MODULE（ATC 5.0）
- 1028-1030 第二感溫點 Alarm 範圍正/負/時間
- 1031 SW_VER / 1032 FW_VER / 1033 TIME_SYNC / 1034 第二感溫點啟用

### 3.3 Recipe / Self-Test / Lot（1035-1057）
- 1035 ATC_READ_OFFSET
- 1036 ATC_GET_NOW_RECIPE / 1037 ATC_GET_RECIPE_LIST
- 1038 ATC_SET_TjMode（AMD TC/TJ 切換）
- 1039 ATC_SET_CHILLER_TEMP
- 1040 ATC_SEND_TEMP_READY
- 1041 ATC_SITE_2ND_CHECK
- 1042 ATC_SET_TJ_OFFSET / 1043 ATC_SET_SINGLE_TJ_OFFSET
- 1044 ATC_RUN_SELFTEST / 1045 ATC_GET_2ND_STATUS / 1046 ATC_MANUAL_SELFTEST / 1047 ATC_SELFTEST_STATUS
- 1048 HANDLER_STATUS（"Are you there?"）
- 1049 ATC_AIRMACHINE_STATUS（ATC 5.1）
- 1050 ATC_LOT_START / 1051 ATC_LOT_END
- 1052 ATC_SELFTEST_RESULT（0 失敗 1 成功）
- 1053 HANDLER_ABNORMAL_SITE
- 1054 ATC_GET_SN
- 1055 ATC_SHUTDOWN（1028c）
- 1056 ATC_51_FREONRECOVER（勝麗）
- 1057 HANDLER_CAN_SELFTEST

### 3.4 ATC 5.1 / FFC / PFC（1058-1074）
- 1058/1059 ATC_51_SET/GET_REF_ENABLED（HT3012CT 冷凍機）
- 1060 ATC_51_SET_DEFROST（ASE_CL）
- 1061 ATC_60_SET_AIRVALVE（Qualcomm 氣流閥）
- 1062 ATC_READ_SOCKETTEMP（ANST）
- **1063 HANDLER_FFC_SETTING** / 1064 HANDLER_ATO_RECORD
- 1065 HANDLER_FFC_ENABLED / 1066 HANDLER_FFC_TRIGGER
- 1067 ATC_GET_CONTROL_MODE
- 1068 HANDLER_PFC_ENABLED / 1069 GET / 1070 SET（8-site PFC）
- 1071 ATC_READ_WATER_VALVE
- 1072 ATC_READ_FUNCTION_STATUS（CheckNetwork）
- 1073 ATC_GET_HEAT_OUTPUT（%5.2f）
- 1074 ATC_AUTO_TCTS_ENABLED（自動 Tc/Ts 切換）

### 3.5 TJ / Slope / Site Mapping（1075-1092）
- 1075 HANDLER_TEST_SITEMAPPING
- 1076 ATC_SET_TJ_ENABLED / 1077 ATC_SET_TJ_PARAMETER（slope, offset）
- 1078/1079 ATC_51_SET/READ_ExtraHeatGun
- 1080 ATC_SWAP_CH2CH3_DATALOG
- 1081 ATC_READ_TCTJ_WATER_VALVE
- 1082 ATC_READ_TJ_PARAMETER
- 1083 ATC_READ_HEATING
- 1084/1085 ATC_GET/SET_SLOPE_OFFSET
- 1086 ATC_GET_TJ_VOLTAGE（8 組）
- 1087 ATC_RECORD_TJ_TEMP（Max/Min/Avg）
- 1088 ATC_QUERY_TJ_TEMP
- 1089 ATC_51_SET_REF_ADJUST（8 組 -8~+8）
- 1090 ATC_51_AUTOLOAD_RECIPE
- 1091 HANDLER_SLK_LAYOUT
- 1092 ATC_GET_HEATER_POWER

### 3.6 KL Control / Multi Temp（1093-1103）
- 1093 ATC_KL_SV / 1094 ATC_KL_OFFSET
- 1095 ATC_KL_TRIGGER / 1096 ATC_KL_TRIGGER_STATUS
- 1097 ATC_READ_TEMP_3（Multi Temperature）
- 1098 ATC_LOAD_RECIPE_SINGLE
- 1099 ATC_GETDEWPOINTTEMP（Index/InShuttle/OutShuttle）
- 1100 ATC_51_SET_TJ_WATCHDOG
- 1101 ATC_AIRMACHINE_TEMP（ATC 5.1）
- 1102 ATC_SAVE_RECIPE_PROHIBIT
- 1103 ATC_READ_CHILLER_TEMP

### 3.7 PF / Tc2 / Heat Output Limit（1104-1115）
- 1104 ATC_SET_PF_PARAMETER
- **1105 ATC_SET_T2OFS** / **1106 ATC_SET_SINGLE_T2OFS**
- 1107 ATC_SET_HEAT_OUTPUT_LIMIT（0~100%）
- 1108 ATC_GET_CTRL_MODE
- 1109 ATC_READ_HYGON_STATUS
- 1110 ATC_READ_WATER_FLOW
- 1111 ATC_SET_CoolingValue（8 組，範圍 0%~）
- 1112-1119 MTK ASIF（EFUSED / REQUEST / FB / CPU / SOC / GPU / FPC / EOT）

### 3.8 2DID / Recipe / Dynamic PID（1120-1131）
- 1120 HANDLER_2DID（AMD-US，thermo profile）
- 1121 ATC_SET_WValve_ENABLED
- 1122 ATC_Recipe_By_Channel（channel 1~16）
- 1123 ATC_Recipe_By_Public
- 1124 ATC_SET_CHILLER_ENABLED
- 1125 ATC_SET_TC_WATER_VALVE
- 1126 HANDLER_ALARM（HandlerAlarmCode）
- 1127 ATC_SET_DYNAMIC_PID
- 1128 ATC_READ_DYNAMIC_PID
- 1129 ATC_SET_MULTI_TC_OFFSET（ATC3.5/3.6 四區 Tc1~Tc4）
- 1130 ATC_CHILLER_WATERWARNING（水位 <85 回 1）
- 1131 ATC_Multi_Temperature_Control（開關 TC1~TC4）

### 3.9 Recipe Transfer / Defrosting / 客製（1132-1137）
- 1132 ATC_Handler_Transmit_Recipe（搭配 `FileTransfer.cpp/.h`）
- 1133 ATC_DEFROSTING（max 12 sites）
- 1134 ATC_Star_Transmit_Recipe（觸發 1132）
- 1135 ATC_SET_SINGLE_OFFSET_MTP（MTP 多 offset）
- 1136 ATC_51_ATO_AdjustValve（自動 K 溫 HotPlate 降開度，統一減 5，下限 65）
- **1137 ATC_HulkMode**（TSMC Hulk 開關）

---

## 4. 常見封包分析

### 4.1 SET_TEMP（@1002）
```
@1002,4,800,800,800,800,#
  → 4 channels, 全部設 80.0°C
```

### 4.2 SET_TOFS（@1003）
```
@1003,4,30,0,30,0,#
  → 4 channels, offset = +3.0, 0, +3.0, 0
```
由 `TfLotInfo::SetATCOffset()` → `ATC_InterfaceForm->SetOffset()` 送出。內部 mapping 由 `uTemp_Set.cpp::iSiteToATC[2][Row][Col]` 與 `iSiteToOfs[2][Row][Col]` 決定（**詳見 `ht9045-contact-force` / V899 iSiteToOfs typo bug**）。

### 4.3 SITE_ENABLED（@1004）
```
@1004,4,1,0,1,0,#
  → Site1/3 啟用, Site2/4 關閉
```

### 4.4 TEST_START（@1012）
```
@1012,1,0,# → 測試開始
@1012,1,1,# → 測試結束
```

---

## 5. Site / Channel Mapping

`uTemp_Set.cpp::InitSiteToATC()`（約 L6337~）依 `iATC_Use_Heat_Count` 與 `FTestSuck.iShtRow/iShtCol` 配置兩張查表：

| 變數 | 維度 | 意義 |
|---|---|---|
| `iSiteToATC[Arm][Row][Col]` | [2][MAX_Row][MAX_Col] | Site 對應 ATC 物理 Channel（0-based） |
| `iSiteToOfs[Arm][Row][Col]` | [2][MAX_Row][MAX_Col] | Site 對應 `Temperature.dATCTempOffset[]` 的 index |
| `iATCToAddr[ch]` | [ATC_HEAD_COUNT] | Channel → tcAa1/tcAa2/.../tcHead1 枚舉 |
| `iAddrToATC[tc]` | [tcTotalCount] | 反查 |
| `iATCToSiteArm[ch]` | [ATC_HEAD_COUNT] | 0=Arm1 1=Arm2 |
| `iATCToSiteRow[ch]` / `iATCToSiteCol[ch]` | | Channel 對應的 Site 座標 |

### dATCTempOffset[] 索引編排（32 組）
| 索引 | 對應 Temperature Channel |
|---|---|
| 0~7 | Arm1 Row1（Aa1~Ad1, Ba1~Bd1） |
| 8~15 | Arm1 Row2（Aa1~Ad1, Ba1~Bd1 第二列）*實際視機台* |
| 16~23 | Arm2 Row1（Aa2~Ad2, Ba2~Bd2） |
| 24~31 | Arm2 Row2 |

公式（`InitSiteToATC` L6343）：
```cpp
iSiteToOfs[0][i][j] = j + (i*8);        // Arm1
iSiteToOfs[1][i][j] = (j + 16) + (i*8); // Arm2
```
> ? **已知 bug（V899 L6440 已修）**：`iATC_Use_Heat_Count==4 && iShtRow==1` 分支有 typo `iSiteToOfs[1][0][0]=24` 重複寫入，造成 Arm2 col0 offset 讀到 `Ba2` 而非 `Aa2`。已修為 `iSiteToOfs[1][0][1]=24`。

### Temperature Channel 枚舉（`MachineType.h` L673-674）
| Enum | 值 | 意義 |
|---|---|---|
| `tcAa1..tcAd1` | 11-14 | Arm1 Row1 a~d 欄 |
| `tcBa1..tcBd1` | 15-18 | Arm1 Row2 a~d 欄 |
| `tcAa2..tcAd2` | 19-22 | Arm2 Row1 a~d 欄 |
| `tcBa2..tcBd2` | 23-26 | Arm2 Row2 a~d 欄 |
| `tcHead1..tcHead4` | - | 4Heater 模式 |

> 命名慣例：**前綴 A/B=Row1/Row2**，**後綴 1/2=Arm1/Arm2**，中間 a/b/c/d=Column。

---

## 6. 常見除錯場景

### 6.1 客戶反應「Offset 沒寫進 ATC」
1. 先看 hangup 資料夾中的 `2026_MM_DD_*.TXT` 通訊 log，統計 `@1003` 出現次數與內容
2. 對照 `EventLogTxt_YYYYMMDD.csv` 的 `ChangeLog` 條目，確認 UI 操作哪幾個欄位
3. 檢查 `uTemp_Set.cpp::InitSiteToATC()` 的 Mapping 是否正確（注意 V899 L6440 typo）
4. 驗證 `TestIF.iTestMode` 與 `Temperature.bMultiZoneEnable/bUseSecondATCTempOffset` 旗標

### 6.2 ATC 不回應
1. 先發 `@1048` HANDLER_STATUS 測試連線
2. 檢查 `TATC_InterfaceForm::Connected` 狀態
3. 確認 ATC 端 TCP IP/Port 設定（config.ini / ATC.ini）

### 6.3 Recipe 下載失敗
1. 使用 `@1036` 查詢目前 Recipe
2. `@1037` 取得 Recipe List
3. `@1001` 切換 Recipe
4. 新版 `@1132` + `@1134` 為檔案傳輸（搭配 `FileTransfer.cpp`）

---

## 7. 封包送出呼叫路徑

```
UI (uTemp_Set.cpp)
  ↓
Temperature.dATCTempOffset[0..31]
  ↓
TfLotInfo::SetATCOffset()        (uLotInfo.cpp L9006)
  ├─ 填 dbATCOffstBuffer[0..31]
  ├─ 依 iSiteToATC / iSiteToOfs 打包 dbATC_Offset[]
  └─ ATC_InterfaceForm->SetOffset(iTotal_Channel, dbATC_Offset)
        ↓
TATC_InterfaceForm::SetOffset()  (ATC_Handler_Side.cpp L2027)
  └─ SendCommand(ATC_SET_TOFS)
        ↓
SendCommand → 組 "@1003,count,d1,d2,...,#" → TCP send
```

---

## 8. 相關技能交叉索引

- `ht9045-contact-force`：Contact Force 與 Temperature 耦合
- `ht9045-lotinfo-flow`：SetLotStart → ATC_LOT_START (@1050) 觸發
- `ht9045-secsgem`：ATC SV 透過 SECS/GEM 上報 MES
- `ht9045-v899`：V899 版本 iSiteToOfs typo bug 詳解

---

## 9. 版本紀錄

| 日期 | 作者 | 變更 |
|---|---|---|
| 20260417 | Jimmy | 依 `ATC Interface Define.xlsx`（137 commands, 1001-1137）建立技能 |
