# SVID / ECID 統一參照表（含差異標記）

> **來源**：`SECS_20260416_Steven.xlsx` (SV & EC sheet, HT9045=V) + 程式碼 `uHGemHT9045_SV.cpp` / `uHGemHT9045_EC.cpp`
> **程式碼目錄**：`HT9011UC_Code_V3.33.902.0_20260410\SECSGEM`
> **總筆數**：2900 筆 (Excel HT9045=V: 2839, Code SV: 809, Code EC: 1738)

## 版本控制

| 版本 | 日期 | 更新者 | 說明 |
|------|------|--------|------|
| V1.00 | 2026-04-01 | Steven | 初版：SV 808 / EC 1738，依 V3.33.900.0 同步 |
| V1.01 | 2026-04-16 | Steven (AI) | 加入版本控制機制 |
| V1.02 | 2026-04-16 | Steven (AI) | 同步 SECS_20260416_Steven.xlsx + V3.33.902.0 Code：SV 809 / EC 1738，Excel HT9045=V 增至 2839 |

## 差異標記說明

| 標記 | 說明 |
|------|------|
| 🟡 程式獨有 | 僅存在於程式碼中，Excel 文件未定義 |
| 🔴 文件獨有 | 僅存在於 Excel 文件中，程式碼未註冊 |
| ⚠️ 名稱 | 同一 ID 在 Excel 與程式碼中名稱不同 |
| ⚠️ 型別 | 同一 ID 在 Excel 與程式碼中資料型別不同 |

## 差異統計

| 類別 | 數量 |
|------|------|
| 🟡 程式獨有 | 61 |
| 🔴 文件獨有 | 354 |
| ⚠️ 名稱差異 | 359 |
| ⚠️ 型別差異 | 56 |
| ✅ 完全一致 | 2097 |
| **合計（去重）** | **2900** |

## GEM 系統 (0-99)
> ⚠️ 此區段有 **22** 筆差異

| ID | 類型 | Name | DataType | Unit | Max | Min | Default | Description | Code | 差異 |
|----|------|------|----------|------|-----|-----|---------|-------------|------|------|
| 3 | SV | GemClock | ASCII |  |  |  |  | Equipment clock |  | 🔴 文件獨有 |
| 4 | SV | GemControlState | UINT_1 |  |  |  |  | Equipment connection status:  1: OffLine;  2:OnLine Local;  3:OnLine Remote; |  | 🔴 文件獨有 |
| 5 | SV | GemLinkState | UINT_1 |  |  |  |  | Current connection status     0:Disabled;  1:Enabled/Not Communicating;  2: Communicating; |  | 🔴 文件獨有 |
| 6 | SV | SECSCommunicationMode | INT_1 |  |  |  |  | 0:HSMS Mode    1:SECS Mode (Set By AP) |  | 🔴 文件獨有 |
| 9 | SV | PreviousGemControlState | UINT_1 |  |  |  |  | Pre-Equipment connection status:  1: OffLine;  2:OnLine Local;  3:OnLine Remote; |  | 🔴 文件獨有 |
| 10 | SV | CPU Frequence | INT_4 |  |  |  |  | CPU operate MHZ |  | 🔴 文件獨有 |
| 11 | SV | CPU  Manufacturer | ASCII |  |  |  |  | CPU  Manufacturer |  | 🔴 文件獨有 |
| 12 | SV | CPU  Type | ASCII |  |  |  |  | CPU Type |  | 🔴 文件獨有 |
| 13 | SV | Total Space Of Disk C | INT_4 |  |  |  |  | Total Space Of Disk C |  | 🔴 文件獨有 |
| 14 | SV | Total Space Of Disk D | INT_4 |  |  |  |  | Total Space Of Disk D |  | 🔴 文件獨有 |
| 15 | SV | Total FreeSpace Of Disk C | INT_4 |  |  |  |  | Total FreeSpace Of Disk C |  | 🔴 文件獨有 |
| 16 | SV | Total FreeSpace Of Disk D | INT_4 |  |  |  |  | Total FreeSpace Of Disk D |  | 🔴 文件獨有 |
| 17 | SV | Memory Load Percent | UINT_4 |  |  |  |  | Memory Load Percent |  | 🔴 文件獨有 |
| 18 | SV | Memory Total Physic | UINT_4 |  |  |  |  | Memory Total Physic |  | 🔴 文件獨有 |
| 19 | SV | Memory Avail Physic | UINT_4 |  |  |  |  | Memory Avail Physic |  | 🔴 文件獨有 |
| 24 | SV | GemMDLN | ASCII |  |  |  |  | Handler Model |  | 🔴 文件獨有 |
| 25 | SV | GemSOFTREV | ASCII |  |  |  |  | Software revision |  | 🔴 文件獨有 |
| 54 | SV | GemSpoolCountActual | UINT_4 |  |  |  |  | Spool enable or disable |  | 🔴 文件獨有 |
| 57 | SV | GemSpoolStartTime | ASCII |  |  |  |  | Spool start time |  | 🔴 文件獨有 |
| 68 | SV/EC | Time Format | UINT_1 |  | 3 | 0 | 0 | Time Format |  | 🔴 文件獨有 |
| 70 | SV | Receipe Struct | UINT_1 |  |  |  |  | It is used to define the format of current setup file. 0: a singla file.;  1: multi-file such as the group of BLD .OFF ;  2: the directory as the setup file; |  | 🔴 文件獨有 |
| 71 | SV | Receipe Extend | ASCII |  |  |  |  | Receipe extend filename |  | 🔴 文件獨有 |

## 料管 / 通道 / 掃碼 (1000-1199)
> ⚠️ 此區段有 **43** 筆差異

| ID | 類型 | Name | DataType | Unit | Max | Min | Default | Description | Code | 差異 |
|----|------|------|----------|------|-----|-----|---------|-------------|------|------|
| 1000 | SV | Machine Define | ASCII |  |  |  |  | Handler define | ✓ |  |
| 1001 | SV | Machine Model | ASCII |  |  |  |  | Handler model | ✓ |  |
| 1002 | SV | Machine ID | ASCII |  |  |  |  | Handler ID | ✓ |  |
| 1003 | SV | Software Version | ASCII |  |  |  |  | Software Version | ✓ |  |
| 1004 | SV | Software Release Date | ASCII |  |  |  |  | Software Release Date | ✓ |  |
| 1005 | SV | Factory | ASCII |  |  |  |  | Factory | ✓ |  |
| 1006 | EC | Lot ID | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1007 | SV/EC | Operator ID | ASCII |  |  |  |  | Operator ID | ✓ |  |
| 1009 | SV | Lot Start Time | ASCII |  |  |  |  | Lot Start Time | ✓ |  |
| 1010 | SV | Machine Pre State | ASCII |  |  |  |  | Pre-handler status | ✓ |  |
| 1011 | SV | Machine State | ASCII |  |  |  |  | Handler status "LOCK"  "EMG 1"  "EMG 2"  "EMG 3"  "EMG 4"  "Power Off"  "Homing"  "Auto Retest"  "HP Check"  "Cleaning"  "OCR Insp"  "Reseting"  "Piggy Back"  "QA Mode"  "Onecycle Cleaning"  "No Tray"  "Running"  "Heater Wait"  "Cooling Wait"  "PAUSE"  (Device in handler, only detect Hot Plate、Shuttle、Index Sucker、In\Out Arm Sucker) "HALT" (except PAUSE) "Index Check" "RUN CHECK" "Defrosting" "ATC Self Test" "Restarting" "FixDoorLock" "FastCool" "RTC Mode" "Decay Test" "Drying Wait" "Cooling Wait" "Ref Wait" | ✓ |  |
| 1012 | SV | Index Arm1 Torque | ASCII |  |  |  |  | Index Arm1 torque value | ✓ |  |
| 1013 | SV | Index Arm2 Torque | ASCII |  |  |  |  | Index Arm2 torque value | ✓ |  |
| 1014 | SV | Tower Light Red | INT_4 |  |  |  |  | Tower Light-Red; 0:Off; 1:Light On; 2:Blink | ✓ |  |
| 1015 | SV | Tower Light Yellow | INT_4 |  |  |  |  | Tower Light-Yellow; 0:Off; 1:Light On; 2:Blink | ✓ |  |
| 1016 | SV | Tower Light Green | INT_4 |  |  |  |  | Tower Light-Green; 0:Off; 1:Light On; 2:Blink | ✓ |  |
| 1017 | SV | Number of RT | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1018 | SV | Galil_Driver_Version | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1019 | SV | Machine Pre Status | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1020 | SV | Machine Pre State | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1021 | SV | UPH | INT_4 |  |  |  |  | UPH | ✓ |  |
| 1023 | SV | Index Time | ASCII |  |  |  |  | Index Time | ✓ |  |
| 1024 | SV | Index Cycle Time | ASCII |  |  |  |  | Index Cycle Time | ✓ |  |
| 1025 | SV | Test Time | ASCII |  |  |  |  | Test Time | ✓ |  |
| 1026 | SV | MTBF | ASCII |  |  |  |  | MTBF | ✓ | ⚠️ SV名稱: Code=`MTBA` |
| 1027 | SV | System Time | ASCII |  |  |  |  | System Time | ✓ |  |
| 1028 | SV | Avg UPH | ASCII |  |  |  |  | Avg UPH | ✓ |  |
| 1031 | SV | MUBF | ASCII |  |  |  |  | MUBF | ✓ | ⚠️ SV名稱: Code=`MUBA` |
| 1032 | SV | Power On Time | INT_4 | Second |  |  |  | Power On Time | ✓ |  |
| 1033 | SV | Running Time | INT_4 | Second |  |  |  | Running Time | ✓ |  |
| 1034 | SV | Production Time | INT_4 | Second |  |  |  | Production Time | ✓ |  |
| 1035 | SV | Pause Time | INT_4 | Second |  |  |  | Pause Time | ✓ |  |
| 1036 | SV | Jam Count | INT_4 |  |  |  |  | Jam Count | ✓ |  |
| 1037 | SV | 10 Times of UPH | ASCII |  |  |  |  | Return 10 times of UPH;  CSV Format include Time and UPH information; |  | 🔴 文件獨有 |
| 1040 | SV | ATC SYSTEM | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1041 | SV | EP Penconder | ASCII |  |  |  |  | EP Penconder | ✓ |  |
| 1042 | SV | ATC SYSTEM | INT_4 |  |  |  |  | ATC SYSTEM |  | 🔴 文件獨有 |
| 1043 | SV | ATC State | INT_4 |  |  |  |  | ATC State | ✓ | ⚠️ SV型別: Code=BOOLEAN |
| 1044 | SV | ATC RUN | BOOLEAN |  |  |  |  | ATC RUN | ✓ |  |
| 1045 | SV | ATC Chiller | BOOLEAN |  |  |  |  | ATC Chiller | ✓ | ⚠️ SV型別: Code=ASCII |
| 1046 | SV | Chiller Temp | ASCII |  |  |  |  | Chiller Temp | ✓ |  |
| 1047 | SV | Read KG | FT_8 |  |  |  |  | Read KG | ✓ |  |
| 1048 | SV | Test Arm EP value transform to Kg | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1049 | SV | Which Arm is in socket area | INT_4 |  |  |  |  | Which Arm is in socket area, 1: Arm1, 2: Arm2, 0: Neither | ✓ |  |
| 1050 | SV | Lot End Time | ASCII |  |  |  |  | Lot End Time | ✓ |  |
| 1051 | SV | Hot Plate1 Temperature | ASCII | Celsius |  |  |  | Hot Plate1 Temperature | ✓ |  |
| 1052 | SV | Hot Plate2 Temperature | ASCII | Celsius |  |  |  | Hot Plate2 Temperature | ✓ |  |
| 1053 | SV | Shuttle1 Temperature | ASCII | Celsius |  |  |  | Shuttle1 Temperature | ✓ |  |
| 1054 | SV | Shuttle2 Temperature | ASCII | Celsius |  |  |  | Shuttle2 Temperature | ✓ |  |
| 1055 | SV | Head1 Temperature | ASCII | Celsius |  |  |  | Head1 Temperature | ✓ |  |
| 1056 | SV | Head2 Temperature | ASCII | Celsius |  |  |  | Head2 Temperature | ✓ |  |
| 1057 | SV | Head5 Temperature | ASCII | Celsius |  |  |  | Head5 Temperature | ✓ |  |
| 1058 | SV | Head6 Temperature | ASCII | Celsius |  |  |  | Head6 Temperature | ✓ |  |
| 1059 | SV | DUT1 Temperature | ASCII | Celsius |  |  |  | DUT1 Temperature | ✓ |  |
| 1060 | SV | DUT2 Temperature | ASCII | Celsius |  |  |  | DUT2 Temperature | ✓ |  |
| 1061 | SV | Chamber Temperature | ASCII | Celsius |  |  |  | Chamber Temperature | ✓ |  |
| 1062 | SV | CCD Temperature | ASCII | Celsius |  |  |  | CCD Temperature | ✓ |  |
| 1063 | SV | Aa 1 Temperature | ASCII | Celsius |  |  |  | Aa 1 Temperature | ✓ |  |
| 1064 | SV | Ab 1 Temperature | ASCII | Celsius |  |  |  | Ab 1 Temperature | ✓ |  |
| 1065 | SV | Ac 1 Temperature | ASCII | Celsius |  |  |  | Ac 1 Temperature | ✓ |  |
| 1066 | SV | Ad 1 Temperature | ASCII | Celsius |  |  |  | Ad 1 Temperature | ✓ |  |
| 1067 | SV | Ba 1 Temperature | ASCII | Celsius |  |  |  | Ba 1 Temperature | ✓ |  |
| 1068 | SV | Bb 1 Temperature | ASCII | Celsius |  |  |  | Bb 1 Temperature | ✓ |  |
| 1069 | SV | Bc 1 Temperature | ASCII | Celsius |  |  |  | Bc 1 Temperature | ✓ |  |
| 1070 | SV | Bd 1 Temperature | ASCII | Celsius |  |  |  | Bd 1 Temperature | ✓ |  |
| 1071 | SV | Aa 2 Temperature | ASCII | Celsius |  |  |  | Aa 2 Temperature | ✓ |  |
| 1072 | SV | Ab 2 Temperature | ASCII | Celsius |  |  |  | Ab 2 Temperature | ✓ |  |
| 1073 | SV | Ac 2 Temperature | ASCII | Celsius |  |  |  | Ac 2 Temperature | ✓ |  |
| 1074 | SV | Ad 2 Temperature | ASCII | Celsius |  |  |  | Ad 2 Temperature | ✓ |  |
| 1075 | SV | Ba 2 Temperature | ASCII | Celsius |  |  |  | Ba 2 Temperature | ✓ |  |
| 1076 | SV | Bb 2 Temperature | ASCII | Celsius |  |  |  | Bb 2 Temperature | ✓ |  |
| 1077 | SV | Bc 2 Temperature | ASCII | Celsius |  |  |  | Bc 2 Temperature | ✓ |  |
| 1078 | SV | Bd 2 Temperature | ASCII | Celsius |  |  |  | Bd 2 Temperature | ✓ |  |
| 1079 | SV | Heat Gun 1 Temperature | ASCII | Celsius |  |  |  | Heat Gun 1 Temperature | ✓ |  |
| 1080 | SV | Heat Gun 2 Temperature | ASCII | Celsius |  |  |  | Heat Gun 2 Temperature | ✓ |  |
| 1081 | SV | DUT 3 Temperature | ASCII | Celsius |  |  |  | DUT 3 Temperature | ✓ |  |
| 1082 | SV | DUT 4 Temperature | ASCII | Celsius |  |  |  | DUT 4 Temperature | ✓ |  |
| 1083 | SV | Socket Temperature | ASCII | Celsius |  |  |  | Socket Temperature | ✓ |  |
| 1084 | SV | Ae 1 Temperature | ASCII | Celsius |  |  |  | Ae 1 Temperature | ✓ |  |
| 1085 | SV | Af 1 Temperature | ASCII | Celsius |  |  |  | Af 1 Temperature | ✓ |  |
| 1086 | SV | Ag 1 Temperature | ASCII | Celsius |  |  |  | Ag 1 Temperature | ✓ |  |
| 1087 | SV | Ah 1 Temperature | ASCII | Celsius |  |  |  | Ah 1 Temperature | ✓ |  |
| 1088 | SV | Be 1 Temperature | ASCII | Celsius |  |  |  | Be 1 Temperature | ✓ |  |
| 1089 | SV | Bf 1 Temperature | ASCII | Celsius |  |  |  | Bf 1 Temperature | ✓ |  |
| 1090 | SV | Bg 1 Temperature | ASCII | Celsius |  |  |  | Bg 1 Temperature | ✓ |  |
| 1091 | SV | Bh 1 Temperature | ASCII | Celsius |  |  |  | Bh 1 Temperature | ✓ |  |
| 1092 | SV | Ae 2 Temperature | ASCII | Celsius |  |  |  | Ae 2 Temperature | ✓ |  |
| 1093 | SV | Af 2 Temperature | ASCII | Celsius |  |  |  | Af 2 Temperature | ✓ |  |
| 1094 | SV | Ag 2 Temperature | ASCII | Celsius |  |  |  | Ag 2 Temperature | ✓ |  |
| 1095 | SV | Ah 2 Temperature | ASCII | Celsius |  |  |  | Ah 2 Temperature | ✓ |  |
| 1096 | SV | Be 2 Temperature | ASCII | Celsius |  |  |  | Be 2 Temperature | ✓ |  |
| 1097 | SV | Bf 2 Temperature | ASCII | Celsius |  |  |  | Bf 2 Temperature | ✓ |  |
| 1098 | SV | Bg 2 Temperature | ASCII | Celsius |  |  |  | Bg 2 Temperature | ✓ |  |
| 1099 | SV | Bh 2 Temperature | ASCII | Celsius |  |  |  | Bh 2 Temperature | ✓ |  |
| 1101 | SV | Loader Count | INT_4 |  |  |  |  | Loader Count | ✓ |  |
| 1102 | SV | Output Total Count | INT_4 |  |  |  |  | Output Total Count | ✓ |  |
| 1103 | SV | Auto1 Count | INT_4 |  |  |  |  | Auto1 Count | ✓ |  |
| 1104 | SV | Auto2 Count | INT_4 |  |  |  |  | Auto2 Count | ✓ |  |
| 1105 | SV | Auto3 Count | INT_4 |  |  |  |  | Auto3 Count | ✓ |  |
| 1106 | SV | Fix1 Count | INT_4 |  |  |  |  | Fix1 Count | ✓ |  |
| 1107 | SV | Fix2 Count | INT_4 |  |  |  |  | Fix2 Count | ✓ |  |
| 1108 | SV | Fix3 Count | INT_4 |  |  |  |  | Fix3 Count | ✓ |  |
| 1109 | SV | Fix4 Count | INT_4 |  |  |  |  | Fix4 Count | ✓ |  |
| 1110 | SV | Fix5 Count | INT_4 |  |  |  |  | Fix5 Count | ✓ |  |
| 1111 | SV | Fix6 Count | INT_4 |  |  |  |  | Fix6 Count | ✓ |  |
| 1112 | SV | Loader Count_ART | INT_4 |  |  |  |  | Loader Count_ART | ✓ | ⚠️ SV名稱: Code=`Loader Count ART` |
| 1113 | SV | Output Total Count_ART | INT_4 |  |  |  |  | Output Total Count_ART | ✓ | ⚠️ SV名稱: Code=`Output Total Count ART` |
| 1114 | SV | Auto1 Count_ART | INT_4 |  |  |  |  | Auto1 Count_ART | ✓ | ⚠️ SV名稱: Code=`Auto1 Count ART` |
| 1115 | SV | Auto2 Count_ART | INT_4 |  |  |  |  | Auto2 Count_ART | ✓ | ⚠️ SV名稱: Code=`Auto2 Count ART` |
| 1116 | SV | Auto3 Count_ART | INT_4 |  |  |  |  | Auto3 Count_ART | ✓ | ⚠️ SV名稱: Code=`Auto3 Count ART` |
| 1117 | SV | Fix1 Count_ART | INT_4 |  |  |  |  | Fix1 Count_ART | ✓ | ⚠️ SV名稱: Code=`Fix1 Count ART` |
| 1118 | SV | Fix2 Count_ART | INT_4 |  |  |  |  | Fix2 Count_ART | ✓ | ⚠️ SV名稱: Code=`Fix2 Count ART` |
| 1119 | SV | Fix3 Count_ART | INT_4 |  |  |  |  | Fix3 Count_ART | ✓ | ⚠️ SV名稱: Code=`Fix3 Count ART` |
| 1120 | SV | Fix4 Count_ART | INT_4 |  |  |  |  | Fix4 Count_ART | ✓ | ⚠️ SV名稱: Code=`Fix4 Count ART` |
| 1121 | SV | Fix5 Count_ART | INT_4 |  |  |  |  | Fix5 Count_ART | ✓ | ⚠️ SV名稱: Code=`Fix5 Count ART` |
| 1122 | SV | Fix6 Count_ART | INT_4 |  |  |  |  | Fix6 Count_ART | ✓ | ⚠️ SV名稱: Code=`Fix6 Count ART` |
| 1123 | SV | Bin 0 Count_ART | INT_4 |  |  |  |  | Bin 0 Count_ART | ✓ | ⚠️ SV名稱: Code=`Bin 0 Count ART` |
| 1124 | SV | Bin 1 Count_ART | INT_4 |  |  |  |  | Bin 1 Count_ART | ✓ | ⚠️ SV名稱: Code=`Bin 1 Count ART` |
| 1125 | SV | Bin 2 Count_ART | INT_4 |  |  |  |  | Bin 2 Count_ART | ✓ | ⚠️ SV名稱: Code=`Bin 2 Count ART` |
| 1126 | SV | Bin 3 Count_ART | INT_4 |  |  |  |  | Bin 3 Count_ART | ✓ | ⚠️ SV名稱: Code=`Bin 3 Count ART` |
| 1127 | SV | Bin 4 Count_ART | INT_4 |  |  |  |  | Bin 4 Count_ART | ✓ | ⚠️ SV名稱: Code=`Bin 4 Count ART` |
| 1128 | SV | Bin 5 Count_ART | INT_4 |  |  |  |  | Bin 5 Count_ART | ✓ | ⚠️ SV名稱: Code=`Bin 5 Count ART` |
| 1129 | SV | Bin 6 Count_ART | INT_4 |  |  |  |  | Bin 6 Count_ART | ✓ | ⚠️ SV名稱: Code=`Bin 6 Count ART` |
| 1130 | SV | Bin 7 Count_ART | INT_4 |  |  |  |  | Bin 7 Count_ART | ✓ | ⚠️ SV名稱: Code=`Bin 7 Count ART` |
| 1131 | SV | Bin 8 Count_ART | INT_4 |  |  |  |  | Bin 8 Count_ART | ✓ | ⚠️ SV名稱: Code=`Bin 8 Count ART` |
| 1132 | SV | Bin 9 Count_ART | INT_4 |  |  |  |  | Bin 9 Count_ART | ✓ | ⚠️ SV名稱: Code=`Bin 9 Count ART` |
| 1133 | SV | Bin 10 Count_ART | INT_4 |  |  |  |  | Bin 10 Count_ART | ✓ | ⚠️ SV名稱: Code=`Bin 10 Count ART` |
| 1134 | SV | Bin 11 Count_ART | INT_4 |  |  |  |  | Bin 11 Count_ART | ✓ | ⚠️ SV名稱: Code=`Bin 11 Count ART` |
| 1135 | SV | Bin 12 Count_ART | INT_4 |  |  |  |  | Bin 12 Count_ART | ✓ | ⚠️ SV名稱: Code=`Bin 12 Count ART` |
| 1136 | SV | Bin 13 Count_ART | INT_4 |  |  |  |  | Bin 13 Count_ART | ✓ | ⚠️ SV名稱: Code=`Bin 13 Count ART` |
| 1137 | SV | Bin 14 Count_ART | INT_4 |  |  |  |  | Bin 14 Count_ART | ✓ | ⚠️ SV名稱: Code=`Bin 14 Count ART` |
| 1138 | SV | Bin 15 Count_ART | INT_4 |  |  |  |  | Bin 15 Count_ART | ✓ | ⚠️ SV名稱: Code=`Bin 15 Count ART` |
| 1139 | SV | ATR FT PassCount | INT_4 |  |  |  |  | ATR FT PassCount | ✓ |  |
| 1140 | SV | ATR FT FailCount | INT_4 |  |  |  |  | ATR FT FailCount | ✓ |  |
| 1141 | SV | ATR FT LoaderCount | INT_4 |  |  |  |  | ATR FT LoaderCount | ✓ |  |
| 1142 | SV | ATR RT PassCount | INT_4 |  |  |  |  | ATR RT PassCount | ✓ |  |
| 1143 | SV | ATR RT FailCount | INT_4 |  |  |  |  | ATR RT FailCount | ✓ |  |
| 1144 | SV | ATR RT LoaderCount | INT_4 |  |  |  |  | ATR RT LoaderCount | ✓ |  |
| 1151 | SV | Auto1 Yield | ASCII | Percentage |  |  |  | Auto1 Yield | ✓ |  |
| 1152 | SV | Auto2 Yield | ASCII | Percentage |  |  |  | Auto2 Yield | ✓ |  |
| 1153 | SV | Auto3 Yield | ASCII | Percentage |  |  |  | Auto3 Yield | ✓ |  |
| 1154 | SV | Fix1 Yield | ASCII | Percentage |  |  |  | Fix1 Yield | ✓ |  |
| 1155 | SV | Fix2 Yield | ASCII | Percentage |  |  |  | Fix2 Yield | ✓ |  |
| 1156 | SV | Fix3 Yield | ASCII | Percentage |  |  |  | Fix3 Yield | ✓ |  |
| 1157 | SV | Fix4 Yield | ASCII | Percentage |  |  |  | Fix4 Yield | ✓ |  |
| 1158 | SV | Fix5 Yield | ASCII | Percentage |  |  |  | Fix5 Yield | ✓ |  |
| 1159 | SV | Fix6 Yield | ASCII | Percentage |  |  |  | Fix6 Yield | ✓ |  |
| 1160 | SV | PassTotalCount | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1161 | SV | FailTotalCount | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1164 | SV | Bin 0 Count | INT_4 |  |  |  |  | Bin 0 Count | ✓ |  |
| 1165 | SV | Bin 1 Count | INT_4 |  |  |  |  | Bin 1 Count | ✓ |  |
| 1166 | SV | Bin 2 Count | INT_4 |  |  |  |  | Bin 2 Count | ✓ |  |
| 1167 | SV | Bin 3 Count | INT_4 |  |  |  |  | Bin 3 Count | ✓ |  |
| 1168 | SV | Bin 4 Count | INT_4 |  |  |  |  | Bin 4 Count | ✓ |  |
| 1169 | SV | Bin 5 Count | INT_4 |  |  |  |  | Bin 5 Count | ✓ |  |
| 1170 | SV | Bin 6 Count | INT_4 |  |  |  |  | Bin 6 Count | ✓ |  |
| 1171 | SV | Bin 7 Count | INT_4 |  |  |  |  | Bin 7 Count | ✓ |  |
| 1172 | SV | Bin 8 Count | INT_4 |  |  |  |  | Bin 8 Count | ✓ |  |
| 1173 | SV | Bin 9 Count | INT_4 |  |  |  |  | Bin 9 Count | ✓ |  |
| 1174 | SV | Bin 10 Count | INT_4 |  |  |  |  | Bin 10 Count | ✓ |  |
| 1175 | SV | Bin 11 Count | INT_4 |  |  |  |  | Bin 11 Count | ✓ |  |
| 1176 | SV | Bin 12 Count | INT_4 |  |  |  |  | Bin 12 Count | ✓ |  |
| 1177 | SV | Bin 13 Count | INT_4 |  |  |  |  | Bin 13 Count | ✓ |  |
| 1178 | SV | Bin 14 Count | INT_4 |  |  |  |  | Bin 14 Count | ✓ |  |
| 1179 | SV | Bin 15 Count | INT_4 |  |  |  |  | Bin 15 Count | ✓ |  |
| 1190 | SV | GET OS Tester Data | ASCII |  |  |  |  | Data= Workfile, ClampV, UpLimit, LowLimit, Current, delayms | ✓ |  |
| 1191 | EC | USE_RFID_READER | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |

## 溫控 ATC (1200-1499)
> ⚠️ 此區段有 **84** 筆差異

| ID | 類型 | Name | DataType | Unit | Max | Min | Default | Description | Code | 差異 |
|----|------|------|----------|------|-----|-----|---------|-------------|------|------|
| 1220 | SV | Loader Count_AUTO | INT_4 |  |  |  |  | V+L206:M206 | ✓ | ⚠️ SV名稱: Code=`Loader Count AUTO` |
| 1221 | SV | Output Total Count_AUTO | INT_4 |  |  |  |  | Output Total Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Output Total Count AUTO` |
| 1222 | SV | Auto1 Count_AUTO | INT_4 |  |  |  |  | Auto1 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Auto1 Count AUTO` |
| 1223 | SV | Auto2 Count_AUTO | INT_4 |  |  |  |  | Auto2 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Auto2 Count AUTO` |
| 1224 | SV | Auto3 Count_AUTO | INT_4 |  |  |  |  | Auto3 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Auto3 Count AUTO` |
| 1225 | SV | Fix1 Count_AUTO | INT_4 |  |  |  |  | Fix1 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Fix1 Count AUTO` |
| 1226 | SV | Fix2 Count_AUTO | INT_4 |  |  |  |  | Fix2 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Fix2 Count AUTO` |
| 1227 | SV | Fix3 Count_AUTO | INT_4 |  |  |  |  | Fix3 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Fix3 Count AUTO` |
| 1228 | SV | Fix4 Count_AUTO | INT_4 |  |  |  |  | Fix4 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Fix4 Count AUTO` |
| 1229 | SV | Fix5 Count_AUTO | INT_4 |  |  |  |  | Fix5 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Fix5 Count AUTO` |
| 1230 | SV | Fix6 Count_AUTO | INT_4 |  |  |  |  | Fix6 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Fix6 Count AUTO` |
| 1231 | SV | Bin 0 Count_AUTO | INT_4 |  |  |  |  | Bin 0 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Bin 0 Count AUTO` |
| 1232 | SV | Bin 1 Count_AUTO | INT_4 |  |  |  |  | Bin 1 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Bin 1 Count AUTO` |
| 1233 | SV | Bin 2 Count_AUTO | INT_4 |  |  |  |  | Bin 2 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Bin 2 Count AUTO` |
| 1234 | SV | Bin 3 Count_AUTO | INT_4 |  |  |  |  | Bin 3 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Bin 3 Count AUTO` |
| 1235 | SV | Bin 4 Count_AUTO | INT_4 |  |  |  |  | Bin 4 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Bin 4 Count AUTO` |
| 1236 | SV | Bin 5 Count_AUTO | INT_4 |  |  |  |  | Bin 5 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Bin 5 Count AUTO` |
| 1237 | SV | Bin 6 Count_AUTO | INT_4 |  |  |  |  | Bin 6 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Bin 6 Count AUTO` |
| 1238 | SV | Bin 7 Count_AUTO | INT_4 |  |  |  |  | Bin 7 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Bin 7 Count AUTO` |
| 1239 | SV | Bin 8 Count_AUTO | INT_4 |  |  |  |  | Bin 8 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Bin 8 Count AUTO` |
| 1240 | SV | Bin 9 Count_AUTO | INT_4 |  |  |  |  | Bin 9 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Bin 9 Count AUTO` |
| 1241 | SV | Bin 10 Count_AUTO | INT_4 |  |  |  |  | Bin 10 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Bin 10 Count AUTO` |
| 1242 | SV | Bin 11 Count_AUTO | INT_4 |  |  |  |  | Bin 11 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Bin 11 Count AUTO` |
| 1243 | SV | Bin 12 Count_AUTO | INT_4 |  |  |  |  | Bin 12 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Bin 12 Count AUTO` |
| 1244 | SV | Bin 13 Count_AUTO | INT_4 |  |  |  |  | Bin 13 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Bin 13 Count AUTO` |
| 1245 | SV | Bin 14 Count_AUTO | INT_4 |  |  |  |  | Bin 14 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Bin 14 Count AUTO` |
| 1246 | SV | Bin 15 Count_AUTO | INT_4 |  |  |  |  | Bin 15 Count_AUTO | ✓ | ⚠️ SV名稱: Code=`Bin 15 Count AUTO` |
| 1247 | SV/EC | Reset Loader Count_AUTO | BOOLEAN |  | True | False | True | TRUE:Enalbe; FALSE:Disable |  | 🔴 文件獨有 |
| 1250 | SV | Enabled Site Count | INT_4 |  |  |  |  | The count of site enabled | ✓ |  |
| 1259 | SV | Auto 4 Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1260 | SV | Auto 5 Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1261 | SV | Auto 6 Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1262 | SV | Fix 7 Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1263 | SV | Fix 8 Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1264 | SV | Fix 9 Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1265 | SV | Fix 10 Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1266 | SV | Fix 11 Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1267 | SV | Fix 12 Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1268 | SV | Auto 4 Count ART | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1269 | SV | Auto 5 Countunt ART | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1271 | SV | Fix 7 Count ART | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1272 | SV | Fix 8 Count ART | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1273 | SV | Fix 9 Count ART | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1274 | SV | Fix 10 Count ART | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1275 | SV | Fix 11 Count ART | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1276 | SV | Fix 12 Count ART | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1277 | SV | Auto 4 Yield | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1278 | SV | Auto 5 Yield | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1279 | SV | Auto 6 Yield | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1280 | SV | Fix 7 Yield | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1281 | SV | Fix 8 Yield | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1282 | SV | Fix 9 Yield | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1283 | SV | Fix 10 Yield | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1284 | SV | Fix 11 Yield | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1285 | SV | Fix 12 Yield | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1286 | SV | Auto 4 Count AUTO | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1287 | SV | Auto 5 Count AUTO | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1288 | SV | Auto 6 Count AUTO | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1289 | SV | Fix 7 Count AUTO | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1290 | SV | Fix 8 Count AUTO | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1291 | SV | Fix 9 Count AUTO | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1292 | SV | Fix 10 Count AUTO | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1293 | SV | Fix 11 Count AUTO | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1294 | SV | Fix 12 Count AUTO | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1351 | SV | ATC Arm1 Head 1 | ASCII |  |  |  |  | ATC Arm1 Head 1 | ✓ |  |
| 1352 | SV | ATC Arm1 Head 2 | ASCII |  |  |  |  | ATC Arm1 Head 2 | ✓ |  |
| 1353 | SV | ATC Arm1 Head 3 | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1354 | SV | ATC Arm1 Head 4 | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1355 | SV | ATC Arm2 Head 1 | ASCII |  |  |  |  | ATC Arm2 Head 1 | ✓ |  |
| 1356 | SV | ATC Arm2 Head 2 | ASCII |  |  |  |  | ATC Arm2 Head 2 | ✓ |  |
| 1357 | SV | ATC Arm2 Head 3 | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1358 | SV | ATC Arm2 Head 4 | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1359 | SV | ATC Arm1 Ref Head 1 | ASCII |  |  |  |  | ATC Arm1 Ref Head 1 | ✓ |  |
| 1360 | SV | ATC Arm1 Ref Head 2 | ASCII |  |  |  |  | ATC Arm1 Ref Head 2 | ✓ |  |
| 1361 | SV | ATC Arm1 Ref Head 3 | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1362 | SV | ATC Arm1 Ref Head 4 | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1363 | SV | ATC Arm2 Ref Head 1 | ASCII |  |  |  |  | ATC Arm2 Ref Head 1 | ✓ |  |
| 1364 | SV | ATC Arm2 Ref Head 2 | ASCII |  |  |  |  | ATC Arm2 Ref Head 2 | ✓ |  |
| 1365 | SV | ATC Arm2 Ref Head 3 | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1366 | SV | ATC Arm2 Ref Head 4 | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1420 | SV | Site 1 to 32 Test Result | ASCII |  |  |  |  | Site 1 to 32 Test Result;  Bin -1 is no test;  CSV Format | ✓ |  |
| 1430 | EC | Arm 1 Contact times for alarm | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1431 | EC | Arm 2 Contact times for alarm | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1432 | SV | Arm 1 Contact Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1433 | SV | Arm 2 Contact Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1451 | SV | Auto1 Test Bin | ASCII |  |  |  |  | Auto1 Test Bin | ✓ |  |
| 1452 | SV | Auto2 Test Bin | ASCII |  |  |  |  | Auto2 Test Bin | ✓ |  |
| 1453 | SV | Auto3 Test Bin | ASCII |  |  |  |  | Auto3 Test Bin | ✓ |  |
| 1454 | SV | Fix1 Test Bin | ASCII |  |  |  |  | Fix1 Test Bin | ✓ |  |
| 1455 | SV | Fix2 Test Bin | ASCII |  |  |  |  | Fix2 Test Bin | ✓ |  |
| 1456 | SV | Fix3 Test Bin | ASCII |  |  |  |  | Fix3 Test Bin | ✓ |  |
| 1457 | SV | Fix4 Test Bin | ASCII |  |  |  |  | Fix4 Test Bin | ✓ |  |
| 1458 | SV | Fix5 Test Bin | ASCII |  |  |  |  | Fix5 Test Bin | ✓ |  |
| 1459 | SV | Fix6 Test Bin | ASCII |  |  |  |  | Fix6 Test Bin | ✓ |  |
| 1460 | SV | Auto4 Test Bin | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1461 | SV | Auto5 Test Bin | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1462 | SV | Auto6 Test Bin | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1463 | SV | Fix7 Test Bin | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1464 | SV | Fix8 Test Bin | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1465 | SV | Fix9 Test Bin | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1466 | SV | Fix10 Test Bin | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1467 | SV | Fix11 Test Bin | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1468 | SV | Fix12 Test Bin | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1470 | SV | CCD2 Temperature | ASCII | Celsius |  |  |  | CCD2 Temperature | ✓ |  |
| 1471 | SV | 2DID Temperature | ASCII | Celsius |  |  |  | 2DID Temperature | ✓ |  |
| 1472 | SV | LB Temperature | ASCII | Celsius |  |  |  | LB Temperature | ✓ |  |
| 1473 | SV | ESD Air Temperature | ASCII | Celsius |  |  |  | ESD Air Temperature | ✓ |  |

## 動作 / 位置 / 感測 (1500-1999)
> ⚠️ 此區段有 **58** 筆差異

| ID | 類型 | Name | DataType | Unit | Max | Min | Default | Description | Code | 差異 |
|----|------|------|----------|------|-----|-----|---------|-------------|------|------|
| 1501 | SV/EC | Setup File | ASCII |  |  |  |  | Setup file name | ✓ |  |
| 1502 | SV/EC | User Level | INT_4 |  | 3 | 0 | 0 | 0:Operator;  1:Engineer;  2:Supervisor;  3:Hontech; | ✓ |  |
| 1513 | SV/EC | Tester On/Off | INT_4 |  | 1 | 0 | 1 | 0:Off-Line;  1:On-Line 2: 2DID Sort | ✓ |  |
| 1514 | SV/EC | Temperature Mode | INT_4 |  | 1 | 0 | 0 | 0:Ambient;  1:Hot 2:AmbientHot | ✓ |  |
| 1517 | SV/EC | Start Mode For HT9045 | INT_4 |  | 8 | 0 | 0 | 0:Continuous Start;   1:Initial Start;  2:Re-Test Continuous;  3:Re-Test Initial Start;  4:Site Mapping Check;  5:QA Mode;  6:Continuous EQC;  7:Initial EQC;  8:rsmInitial_ART; 9:rsmContinuStart_ART 10:rsmContinuRetest_ART 11:Auto Retest | ✓ |  |
| 1518 | SV/EC | Real/Dummy For HT9045 | INT_4 |  | 2 | 0 | 1 | 0:Dummy;  1:Tray Only;  2:Real; | ✓ |  |
| 1519 | SV/EC | Temperature Default for HT9045 | FT_8 |  | 150 | 20 | 30 | Temperature Default | ✓ |  |
| 1520 | SV/EC | Temperature Soak Time for HT9045 | FT_8 | Second |  |  | 90 | Temperature Soak Time | ✓ |  |
| 1530 | SV/EC | Site Aa to Dh Status | ASCII |  |  |  |  | Site Aa to Dh Status;  0:Close; 1:Open | ✓ |  |
| 1531 | SV/EC | Site Aa to Dh Status for Arm 2 | ASCII |  |  |  |  | Site Aa to Dh Status for Arm 2;  0:Close; 1:Open | ✓ |  |
| 1552 | EC | FlowID | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1553 | EC | Insertion | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1554 | EC | Customer Device | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1581 | SV/EC | Lot Information Customer | ASCII |  |  |  |  | Lot Information Customer | ✓ |  |
| 1582 | SV/EC | Lot Information Inner Lot ID | ASCII |  |  |  |  | Lot Information Inner Lot ID | ✓ |  |
| 1583 | SV/EC | Lot Information Customer Lot ID | ASCII |  |  |  |  | Lot Information Customer Lot ID | ✓ |  |
| 1584 | SV/EC | Lot Information Customer Device Group | ASCII |  |  |  |  | Lot Information Customer Device Group | ✓ |  |
| 1585 | SV/EC | Lot Information Device Name | ASCII |  |  |  |  | Lot Information Device Name | ✓ |  |
| 1586 | SV/EC | Lot Information Stage | ASCII |  |  |  |  | Lot Information Stage | ✓ |  |
| 1587 | SV/EC | Lot Information Step | ASCII |  |  |  |  | Lot Information Step | ✓ |  |
| 1588 | SV/EC | Lot Information Report Count | ASCII |  |  |  |  | Lot Information Report Count | ✓ |  |
| 1589 | SV/EC | Lot Information Program Name | ASCII |  |  |  |  | Lot Information Program Name | ✓ |  |
| 1590 | SV/EC | Lot Information Test Bin No | ASCII |  |  |  |  | Lot Information Test Bin No | ✓ |  |
| 1591 | SV/EC | Lot Information Tester ID | ASCII |  |  |  |  | Lot Information Tester ID | ✓ |  |
| 1592 | SV/EC | Lot Information Handler ID | ASCII |  |  |  |  | Lot Information Handler ID | ✓ |  |
| 1593 | SV/EC | Lot Information Temperauture | ASCII |  |  |  |  | Lot Information Temperauture | ✓ |  |
| 1594 | SV/EC | Lot Information Curr Quantity | ASCII |  |  |  |  | Lot Information Curr Quantity | ✓ |  |
| 1595 | SV/EC | Lot Information Operator ID | ASCII |  |  |  |  | Lot Information Operator ID | ✓ |  |
| 1596 | SV/EC | Lot Information Barcode Recipe Name | ASCII |  |  |  |  | Lot Information Barcode Recipe Nam |  | 🔴 文件獨有 |
| 1597 | SV/EC | Use barcode Multi Recipe Function | BOOLEAN |  | 1 | 0 | 0 | Use barcode Multi Recipe Function |  | 🔴 文件獨有 |
| 1600 | SV/EC | Use Individual Temperature | BOOLEAN |  | 1 | 0 | 0 | Use Individual Temperature | ✓ |  |
| 1601 | SV/EC | Hot Plate1 Individual Temperature | FT_8 | Celsius |  |  |  | Hot Plate1 Individual Temperature | ✓ | ⚠️ EC名稱: Code=`Plate1 Individual Temperature` |
| 1602 | SV/EC | Hot Plate2 Individua Temperature | FT_8 | Celsius |  |  |  | Hot Plate2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Plate2 Individual Temperature` |
| 1603 | SV/EC | Shuttle1 Individua Temperature | FT_8 | Celsius |  |  |  | Shuttle1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Shuttle1 Individual Temperature` |
| 1604 | SV/EC | Shuttle2 Individua Temperature | FT_8 | Celsius |  |  |  | Shuttle2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Shuttle2 Individual Temperature` |
| 1605 | SV/EC | Head 1 Individua Temperature | FT_8 | Celsius |  |  |  | Head 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Head1 Individual Temperature` |
| 1606 | SV/EC | Head 2 Individua Temperature | FT_8 | Celsius |  |  |  | Head 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Head2 Individual Temperature` |
| 1607 | SV/EC | Head 5 Individua Temperature | FT_8 | Celsius |  |  |  | Head 5 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Head5 Individual Temperature` |
| 1608 | SV/EC | Head 6 Individua Temperature | FT_8 | Celsius |  |  |  | Head 6 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Head6 Individual Temperature` |
| 1609 | SV/EC | DUT 1 Individua Temperature | FT_8 | Celsius |  |  |  | DUT 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`DUT1 Individual Temperature` |
| 1610 | SV/EC | DUT 2  IndividuaTemperature | FT_8 | Celsius |  |  |  | DUT 2  IndividuaTemperature | ✓ | ⚠️ EC名稱: Code=`DUT2 Individual Temperature` |
| 1611 | SV/EC | Chamber Individua Temperature | FT_8 | Celsius |  |  |  | Chamber Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Chamber Individual Temperature` |
| 1612 | SV/EC | Aa 1 Individua Temperature | FT_8 | Celsius |  |  |  | Aa 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`CCD Individual Temperature` |
| 1613 | SV/EC | Ab 1 Individua Temperature | FT_8 | Celsius |  |  |  | Ab 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Aa 1 Individual Temperature` |
| 1614 | SV/EC | Ac 1 Individua Temperature | FT_8 | Celsius |  |  |  | Ac 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Ab 1 Individual Temperature` |
| 1615 | SV/EC | Ad 1 Individua Temperature | FT_8 | Celsius |  |  |  | Ad 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Ac 1 Individual Temperature` |
| 1616 | SV/EC | Ba 1 Individua Temperature | FT_8 | Celsius |  |  |  | Ba 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Ad 1 Individual Temperature` |
| 1617 | SV/EC | Bb 1 Individua Temperature | FT_8 | Celsius |  |  |  | Bb 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Ba 1 Individual Temperature` |
| 1618 | SV/EC | Bc 1 Individua Temperature | FT_8 | Celsius |  |  |  | Bc 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Bb 1 Individual Temperature` |
| 1619 | SV/EC | Bd 1 Individua Temperature | FT_8 | Celsius |  |  |  | Bd 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Bc 1 Individual Temperature` |
| 1620 | SV/EC | Aa 2 Individua Temperature | FT_8 | Celsius |  |  |  | Aa 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Bd 1 Individual Temperature` |
| 1621 | SV/EC | Ab 2 Individua Temperature | FT_8 | Celsius |  |  |  | Ab 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Aa 2 Individual Temperature` |
| 1622 | SV/EC | Ac 2 Individua Temperature | FT_8 | Celsius |  |  |  | Ac 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Ab 2 Individual Temperature` |
| 1623 | SV/EC | Ad 2 Individua Temperature | FT_8 | Celsius |  |  |  | Ad 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Ac 2 Individual Temperature` |
| 1624 | SV/EC | Ba 2 Individua Temperature | FT_8 | Celsius |  |  |  | Ba 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Ad 2 Individual Temperature` |
| 1625 | SV/EC | Bb 2 Individua Temperature | FT_8 | Celsius |  |  |  | Bb 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Ba 2 Individual Temperature` |
| 1626 | SV/EC | Bc 2 Individua Temperature | FT_8 | Celsius |  |  |  | Bc 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Bb 2 Individual Temperature` |
| 1627 | SV/EC | Bd 2 Individua Temperature | FT_8 | Celsius |  |  |  | Bd 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Bc 2 Individual Temperature` |
| 1628 | SV/EC | Heat Gun 1 Individua Temperature | FT_8 | Celsius |  |  |  | Heat Gun 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Bd 2 Individual Temperature` |
| 1629 | SV/EC | Heat Gun 2 Individua Temperature | FT_8 | Celsius |  |  |  | Heat Gun 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Heat Gun 1 Individual Temperature` |
| 1630 | SV/EC | DUT 3 Individua Temperature | FT_8 | Celsius |  |  |  | DUT 3 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Heat Gun 2 Individual Temperature` |
| 1631 | SV/EC | DUT 4 Individua Temperature | FT_8 | Celsius |  |  |  | DUT 4 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`DUT 3 Individual Temperature` |
| 1632 | SV/EC | Socket Individua Temperature | FT_8 | Celsius |  |  |  | Socket Individua Temperature | ✓ | ⚠️ EC名稱: Code=`DUT 4 Individual Temperature` |
| 1633 | SV/EC | Ae 1 Individua Temperature | FT_8 | Celsius |  |  |  | Ae 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Socket Individual Temperature` |
| 1634 | SV/EC | Af 1 Individua Temperature | FT_8 | Celsius |  |  |  | Af 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Ae 1 Individual Temperature` |
| 1635 | SV/EC | Ag 1 Individua Temperature | FT_8 | Celsius |  |  |  | Ag 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Af 1 Individual Temperature` |
| 1636 | SV/EC | Ah 1 Individua Temperature | FT_8 | Celsius |  |  |  | Ah 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Ag 1 Individual Temperature` |
| 1637 | SV/EC | Be 1 Individua Temperature | FT_8 | Celsius |  |  |  | Be 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Ah 1 Individual Temperature` |
| 1638 | SV/EC | Bf 1 Individua Temperature | FT_8 | Celsius |  |  |  | Bf 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Be 1 Individual Temperature` |
| 1639 | SV/EC | Bg 1 Individua Temperature | FT_8 | Celsius |  |  |  | Bg 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Bf 1 Individual Temperature` |
| 1640 | SV/EC | Bh 1 Individua Temperature | FT_8 | Celsius |  |  |  | Bh 1 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Bg 1 Individual Temperature` |
| 1641 | SV/EC | Ae 2 Individua Temperature | FT_8 | Celsius |  |  |  | Ae 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Bh 1 Individual Temperature` |
| 1642 | SV/EC | Af 2 Individua Temperature | FT_8 | Celsius |  |  |  | Af 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Ae 2 Individual Temperature` |
| 1643 | SV/EC | Ag 2 Individua Temperature | FT_8 | Celsius |  |  |  | Ag 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Af 2 Individual Temperature` |
| 1644 | SV/EC | Ah 2 Individua Temperature | FT_8 | Celsius |  |  |  | Ah 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Ag 2 Individual Temperature` |
| 1645 | SV/EC | Be 2 Individua Temperature | FT_8 | Celsius |  |  |  | Be 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Ah 2 Individual Temperature` |
| 1646 | SV/EC | Bf 2 Individua Temperature | FT_8 | Celsius |  |  |  | Bf 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Be 2 Individual Temperature` |
| 1647 | SV/EC | Bg 2 Individua Temperature | FT_8 | Celsius |  |  |  | Bg 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Bf 2 Individual Temperature` |
| 1648 | SV/EC | Bh 2 Individua Temperature | FT_8 | Celsius |  |  |  | Bh 2 Individua Temperature | ✓ | ⚠️ EC名稱: Code=`Bg 2 Individual Temperature` |
| 1649 | EC | Bh 2 Individual Temperature | FT_8 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1701 | EC | Check 2DID allow list function | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1702 | SV | Auto1 car Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1703 | SV | Auto2 car Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1704 | SV | Auto3 car Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1705 | SV | Auto4 car Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1706 | SV | Auto5 car Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 1707 | SV | Auto6 car Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |

## 站位 / 吸嘴 / 測試 (2000-2999)
> ⚠️ 此區段有 **84** 筆差異

| ID | 類型 | Name | DataType | Unit | Max | Min | Default | Description | Code | 差異 |
|----|------|------|----------|------|-----|-----|---------|-------------|------|------|
| 2001 | SV/EC | Arm Total Force KG | FT_8 | KG |  |  |  | Arm Total Force KG | ✓ |  |
| 2003 | SV | Force Per Device KG | FT_8 | KG |  |  |  | Force Per Device KG | ✓ |  |
| 2004 | SV | Force Per Pin N | FT_8 | N |  |  |  | Force Per Pin N | ✓ |  |
| 2005 | SV | Arm Total Force N | FT_8 | N |  |  |  | Arm Total Force N | ✓ |  |
| 2007 | SV | Force Per Device N | FT_8 | N |  |  |  | Force Per Device N | ✓ |  |
| 2008 | SV | Die Force Device KG | FT_8 | KG |  |  |  | Force Per Device KG |  | 🔴 文件獨有 |
| 2009 | SV | Die Force of pins(balls) | FT_8 |  |  |  |  | Die Force of pins(balls) |  | 🔴 文件獨有 |
| 2010 | SV/EC | Head Device Count | INT_4 |  | 6 | 2 | 2 | 2: 1 Device with 1 Compliance Unit;  3: 2 Device with 1 Compliance Unit;  4: 4 Device with 1 Compliance Unit;  5: 2 Device with 4 Compliance Unit;  6: 8 Device with 1 Compliance Unit; | ✓ |  |
| 2011 | SV/EC | X-Dimension | FT_8 | mm |  |  |  | X-Dimension | ✓ |  |
| 2012 | SV/EC | Y-Dimension | FT_8 | mm |  |  |  | Y-Dimension | ✓ |  |
| 2013 | SV/EC | Kit Diameter | FT_8 |  |  |  |  | 30mm = 3.0 40mm = 4.0 60mm = 6.0 | ✓ |  |
| 2014 | SV/EC | Die Force Kit Diameter | FT_8 |  |  |  |  | 20mm = 2.0 30mm =3.0 40mm =4.0 |  | 🔴 文件獨有 |
| 2015 | SV | Contact Alarm Count Setting grop 1 | INT_4 |  |  |  |  | Contact Alarm Count Setting grop 1 | ✓ |  |
| 2016 | SV | Contact Alarm Count Setting grop 2 | INT_4 |  |  |  |  | Contact Alarm Count Setting grop 2 | ✓ |  |
| 2017 | SV | Contact Alarm Count Setting grop 3 | INT_4 |  |  |  |  | Contact Alarm Count Setting grop 3 | ✓ |  |
| 2018 | SV | Contact Air Force | FT_8 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2019 | SV | Contact Air Force Feedback | FT_8 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2020 | SV | Contact Set KG | FT_8 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2021 | SV/EC | Die Force KG | FT_8 |  |  |  |  | Die Force KG | ✓ |  |
| 2022 | SV | Use Die Force | BOOLEAN |  |  |  |  | Use Die Force | ✓ | ✅ 20261003 ST02-C16（Steven Q83＝A，照 V912 uHGemHT9045_EC.cpp:158／:161）：程式改成 EC 2022＝Use Die Force（BOOLEAN）、EC 2025＝The no of pins on die（INT_4），SV 2022 拿掉（golden 906 0618 是 SV 2022＝Use Die Force、EC 2022＝pin 數，同一個號碼兩樣東西） |
| 2023 | SV | Die Force Kit Diameter | INT_4 |  |  |  |  | Die Force Kit Diameter | ✓ |  |
| 2024 | SV | Die Force Feedback | FT_8 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2050 | SV/EC | Product Type | ASCII |  |  |  |  | Product Type |  | 🔴 文件獨有 |
| 2051 | SV/EC | Customer lot | ASCII |  |  |  |  | Customer lot |  | 🔴 文件獨有 |
| 2052 | SV/EC | Normal test program | ASCII |  |  |  |  | Normal test program |  | 🔴 文件獨有 |
| 2053 | SV/EC | Process | ASCII |  |  |  |  | Process |  | 🔴 文件獨有 |
| 2101 | SV | Arm1 Contact Height | FT_8 | mm |  |  |  | Arm1 Contact Height | ✓ |  |
| 2102 | SV | Arm2 Contact Height | FT_8 | mm |  |  |  | Arm2 Contact Height | ✓ |  |
| 2103 | SV | Shuttle1 Pick Height | FT_8 | mm |  |  |  | Shuttle1 Pick Height | ✓ |  |
| 2104 | SV | Shuttle2 Pick Height | FT_8 | mm |  |  |  | Shuttle2 Release Height | ✓ |  |
| 2105 | SV | Shuttle1 Release Height | FT_8 | mm |  |  |  | Shuttle1 Release Height | ✓ |  |
| 2106 | SV | Shuttle2 Release Height | FT_8 | mm |  |  |  | Shuttle2 Release Height | ✓ |  |
| 2200 | SV | Magazine 1 Count | INT_4 |  |  |  |  | Magazine 1 Count |  | 🔴 文件獨有 |
| 2201 | SV | Magazine 2 Count | INT_4 |  |  |  |  | Magazine 2 Count |  | 🔴 文件獨有 |
| 2202 | SV | Magazine 3 Count | INT_4 |  |  |  |  | Magazine 3 Count |  | 🔴 文件獨有 |
| 2203 | SV | Magazine 4 Count | INT_4 |  |  |  |  | Magazine 4 Count |  | 🔴 文件獨有 |
| 2204 | SV | Magazine 5 Count | INT_4 |  |  |  |  | Magazine 5 Count |  | 🔴 文件獨有 |
| 2205 | SV | Magazine 6 Count | INT_4 |  |  |  |  | Magazine 6 Count |  | 🔴 文件獨有 |
| 2206 | SV | Magazine 7 Count | INT_4 |  |  |  |  | Magazine 7 Count |  | 🔴 文件獨有 |
| 2207 | SV | Magazine 8 Count | INT_4 |  |  |  |  | Magazine 8 Count |  | 🔴 文件獨有 |
| 2208 | SV | Magazine 9 Count | INT_4 |  |  |  |  | Magazine 9 Count |  | 🔴 文件獨有 |
| 2209 | SV | Magazine 10 Count | INT_4 |  |  |  |  | Magazine 10 Count |  | 🔴 文件獨有 |
| 2210 | SV | Magazine 11 Count | INT_4 |  |  |  |  | Magazine 11 Count |  | 🔴 文件獨有 |
| 2211 | SV | Magazine 12 Count | INT_4 |  |  |  |  | Magazine 12 Count |  | 🔴 文件獨有 |
| 2212 | SV | Magazine 13 Count | INT_4 |  |  |  |  | Magazine 13 Count |  | 🔴 文件獨有 |
| 2213 | SV | Magazine 14 Count | INT_4 |  |  |  |  | Magazine 14 Count |  | 🔴 文件獨有 |
| 2221 | SV | Magazine 2 Count ART | INT_4 |  |  |  |  | Magazine 2 Count ART |  | 🔴 文件獨有 |
| 2222 | SV | Magazine 3 Count ART | INT_4 |  |  |  |  | Magazine 3 Count ART |  | 🔴 文件獨有 |
| 2223 | SV | Magazine 4 Count ART | INT_4 |  |  |  |  | Magazine 4 Count ART |  | 🔴 文件獨有 |
| 2224 | SV | Magazine 5 Count ART | INT_4 |  |  |  |  | Magazine 5 Count ART |  | 🔴 文件獨有 |
| 2225 | SV | Magazine 6 Count ART | INT_4 |  |  |  |  | Magazine 6 Count ART |  | 🔴 文件獨有 |
| 2226 | SV | Magazine 7 Count ART | INT_4 |  |  |  |  | Magazine 7 Count ART |  | 🔴 文件獨有 |
| 2227 | SV | Magazine 8 Count ART | INT_4 |  |  |  |  | Magazine 8 Count ART |  | 🔴 文件獨有 |
| 2228 | SV | Magazine 9 Count ART | INT_4 |  |  |  |  | Magazine 9 Count ART |  | 🔴 文件獨有 |
| 2229 | SV | Magazine 10 Count ART | INT_4 |  |  |  |  | Magazine 10 Count ART |  | 🔴 文件獨有 |
| 2230 | SV | Magazine 11 Count ART | INT_4 |  |  |  |  | Magazine 11 Count ART |  | 🔴 文件獨有 |
| 2231 | SV | Magazine 12 Count ART | INT_4 |  |  |  |  | Magazine 12 Count ART |  | 🔴 文件獨有 |
| 2232 | SV | Magazine 13 Count ART | INT_4 |  |  |  |  | Magazine 13 Count ART |  | 🔴 文件獨有 |
| 2233 | SV | Magazine 14 Count ART | INT_4 |  |  |  |  | Magazine 14 Count ART |  | 🔴 文件獨有 |
| 2240 | SV | Magazine 1 Test Bin | ASCII |  |  |  |  | Magazine 1 Test Bin |  | 🔴 文件獨有 |
| 2241 | SV | Magazine 2 Test Bin | ASCII |  |  |  |  | Magazine 2 Test Bin |  | 🔴 文件獨有 |
| 2242 | SV | Magazine 3 Test Bin | ASCII |  |  |  |  | Magazine 3 Test Bin |  | 🔴 文件獨有 |
| 2243 | SV | Magazine 4 Test Bin | ASCII |  |  |  |  | Magazine 4 Test Bin |  | 🔴 文件獨有 |
| 2244 | SV | Magazine 5 Test Bin | ASCII |  |  |  |  | Magazine 5 Test Bin |  | 🔴 文件獨有 |
| 2245 | SV | Magazine 6 Test Bin | ASCII |  |  |  |  | Magazine 6 Test Bin |  | 🔴 文件獨有 |
| 2246 | SV | Magazine 7 Test Bin | ASCII |  |  |  |  | Magazine 7 Test Bin |  | 🔴 文件獨有 |
| 2247 | SV | Magazine 8 Test Bin | ASCII |  |  |  |  | Magazine 8 Test Bin |  | 🔴 文件獨有 |
| 2248 | SV | Magazine 9 Test Bin | ASCII |  |  |  |  | Magazine 9 Test Bin |  | 🔴 文件獨有 |
| 2249 | SV | Magazine 10 Test Bin | ASCII |  |  |  |  | Magazine 10 Test Bin |  | 🔴 文件獨有 |
| 2250 | SV | Magazine 11 Test Bin | ASCII |  |  |  |  | Magazine 11 Test Bin |  | 🔴 文件獨有 |
| 2251 | SV | Magazine 12 Test Bin | ASCII |  |  |  |  | Magazine 12 Test Bin |  | 🔴 文件獨有 |
| 2252 | SV | Magazine 13 Test Bin | ASCII |  |  |  |  | Magazine 13 Test Bin |  | 🔴 文件獨有 |
| 2253 | SV | Magazine 14 Test Bin | ASCII |  |  |  |  | Magazine 14 Test Bin |  | 🔴 文件獨有 |
| 2266 | EC | ContiFail(Socket) Auto Site Off Func | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2267 | EC | ContiFail(Head) Auto Site Off Func | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2268 | EC | By Arm Per Site Differ Yields Auto Site Off Func | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2269 | EC | By Socket Compare Yield Auto Site Off Func | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2270 | EC | By Picker Compare Yield Auto Site Off Func | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2501 | SV/EC | Input Arm Vacuum Wait | FT_8 | Second | 10 | 0.01 | 0.01 | Input Arm vacuum wait time | ✓ |  |
| 2502 | SV/EC | Index Arm Vacuum Wait | FT_8 | Second | 10 | 0.01 | 0.01 | Index Arm vacuum wait time | ✓ |  |
| 2503 | SV/EC | Output Arm Vacuum Wait | FT_8 | Second | 10 | 0.01 | 0.01 | Output Arm vacuum wait time | ✓ |  |
| 2504 | SV/EC | Tray Arm Vacuum Wait | FT_8 | Second | 10 | 0.01 | 0.01 | Tray Arm vacuum wait time | ✓ |  |
| 2511 | SV/EC | Input Arm Destroy Wait | FT_8 | Second | 10 | 0.01 | 0.01 | Input Arm destroy wait time | ✓ |  |
| 2512 | SV/EC | Index Arm Destroy Wait | FT_8 | Second | 10 | 0.01 | 0.01 | Index Arm destroy wait time | ✓ |  |
| 2513 | SV/EC | Output Arm Destroy Wait | FT_8 | Second | 10 | 0.01 | 0.01 | Output Arm destroy wait time | ✓ |  |
| 2514 | SV/EC | Tray Arm Destroy Wait | FT_8 | Second | 10 | 0.01 | 0.01 | Tray Arm destroy wait time | ✓ |  |
| 2521 | SV/EC | Input Arm Vacuum Pre On | BOOLEAN |  |  |  |  | Input Arm Vacuum Pre On | ✓ |  |
| 2522 | SV/EC | Index Arm Vacuum Pre On | BOOLEAN |  |  |  |  | Index Arm Vacuum Pre On | ✓ |  |
| 2523 | SV/EC | Output Arm Vacuum Pre On | BOOLEAN |  |  |  |  | Output Arm Vacuum Pre On | ✓ |  |
| 2609 | SV/EC | Device ID | ASCII |  |  |  |  | Device ID | ✓ |  |
| 2611 | SV/EC | Second Speed | INT_4 | Percentage | 100 | 1 | 1 | Second Speed | ✓ |  |
| 2613 | SV/EC | Arm1 Drop Height | FT_8 | mm | 30 | 0 | 0 | Arm1 Drop Height | ✓ |  |
| 2614 | SV/EC | Arm2 Drop Height | FT_8 | mm | 30 | 0 | 0 | Arm2 Drop Height | ✓ |  |
| 2615 | SV/EC | Drop Wait Time | FT_8 | Second | 60 | 0 | 0 | Drop Wait Time | ✓ |  |
| 2616 | SV/EC | Contact Test Mode for HT9045 | INT_4 |  | 6 | 0 | 0 | 0: Direct Contact Mode;  1: Drop Contact;  2: Direct & Soft Contact Mode;  3: TMOVE;  4: TMOVE Drop;  5: Direct Soft Contact;  6: Drop Soft Contact ;  7: Shift Contact for 8Site 1x4 8: Drop & Slow Contact 9: TMOVE Slow Contact 10:TMOVE Drop & Slow Contact | ✓ |  |
| 2617 | SV/EC | Vacuum Mode | INT_4 |  | 1 | 0 | 0 | 0: Vacuum ON Mode;  1: Vacuum OFF Mode; | ✓ |  |
| 2618 | SV/EC | Drop Wait Time | FT_8 | Second | 60 | 0 | 0 | Up Wait Time | ✓ | ⚠️ EC名稱: Code=`Up Wait Time` |
| 2619 | SV/EC | Up Second Speed | INT_4 | Percentage | 100 | 1 | 1 | Up Second Speed | ✓ |  |
| 2621 | SV/EC | Pins per device | INT_4 |  | 10000 | 0 | 50 | Pins per device | ✓ |  |
| 2622 | SV/EC | Force per pin | FT_8 | G | 1000 | 0.1 | 10 | Force per pin (gf) | ✓ |  |
| 2623 | SV/EC | Arm1 Up Height | FT_8 | mm | 30 | 0 | 0 | Arm1 Up Height | ✓ |  |
| 2624 | SV/EC | Arm2 Up Height | FT_8 | mm | 30 | 0 | 0 | Arm2 Up Height | ✓ |  |
| 2631 | SV | Height Calibration Mode | INT_4 |  | 3 | 0 | 0 | 0:Normal;  1:Auto Height;  2:Contact Test;  3:Manual Test; | ✓ |  |
| 2632 | SV/EC | Height Include Shuttle | BOOLEAN |  | 1 | 0 | 1 | Height Include Shuttle | ✓ |  |
| 2633 | SV/EC | Shuttle Wait Outside Chamber | BOOLEAN |  | 1 | 0 | 0 | 0: OFF; 1: ON | ✓ |  |
| 2634 | SV/EC | Pick Shuttle Device After Tested | BOOLEAN |  | 1 | 0 | 0 | 0: OFF; 1: ON | ✓ |  |
| 2635 | SV/EC | Pick Shuttle Device Together (32Site_N) | BOOLEAN |  | 1 | 0 | 0 | 0: OFF; 1: ON | ✓ |  |
| 2636 | EC | Socket IC vacuum check mode | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2637 | EC | Above socket position z offset | FT_8 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2650 | EC | Auto 4 Tray Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2651 | EC | Auto 5 Tray Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2652 | EC | Auto 6 Tray Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2653 | EC | Auto 4 Tray Type for RT | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2654 | EC | Auto 5 Tray Type for RT | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2655 | EC | Auto 6 Tray Type for RT | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2656 | EC | Auto 4 Tray Direction | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2657 | EC | Auto 5 Tray Direction | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2658 | EC | Auto 6 Tray Direction | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2659 | EC | Fix 4 Tray Direction | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2660 | EC | Fix 5 Tray Direction | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2661 | EC | Fix 6 Tray Direction | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2662 | EC | Fix 4 Tray Type Index | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2663 | EC | Fix 5 Tray Type Index | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2664 | EC | Fix 6 Tray Type Index | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2665 | SV | Auto 4 Tray From Alias | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2666 | SV | Auto 5 Tray From Alias | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2667 | SV | Auto 6 Tray From Alias | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2701 | SV/EC | Tray Type | INT_4 |  | 1 | 0 | 0 | 0:Same; 1:Defferent | ✓ |  |
| 2702 | SV/EC | Loader Tray Type | INT_4 |  | 2 | 0 | 0 | 0:Empty; 1:Color; 2: Auto2 | ✓ |  |
| 2703 | SV/EC | Auto1 Tray Type | INT_4 |  | 2 | 0 | 0 | 0:Empty; 1:Color; 2: Auto2 | ✓ |  |
| 2704 | SV/EC | Auto2 Tray Type | INT_4 |  | 2 | 0 | 0 | 0:Empty; 1:Color; 2: Auto2 | ✓ |  |
| 2705 | SV/EC | Auto3 Tray Type | INT_4 |  | 2 | 0 | 0 | 0:Empty; 1:Color; 2: Auto2 | ✓ |  |
| 2706 | SV/EC | Loader Tray Type for RT | INT_4 |  | 2 | 0 | 0 | 0:Empty; 1:Color; 2: Auto2 | ✓ |  |
| 2707 | SV/EC | Auto1 Tray Type for RT | INT_4 |  | 2 | 0 | 0 | 0:Empty; 1:Color; 2: Auto2 | ✓ |  |
| 2708 | SV/EC | Auto2 Tray Type for RT | INT_4 |  | 2 | 0 | 0 | 0:Empty; 1:Color; 2: Auto2 | ✓ |  |
| 2709 | SV/EC | Auto3 Tray Type for RT | INT_4 |  | 2 | 0 | 0 | 0:Empty; 1:Color; 2: Auto2 | ✓ |  |
| 2710 | SV/EC | Fix Tray Mode | INT_4 |  | 1 | 0 | 0 | 0: Full Bin;  1: Up Down; | ✓ |  |
| 2711 | SV/EC | Loader Tray Direction | INT_4 |  | 3 | 0 | 0 | Loader Tray Direction | ✓ |  |
| 2712 | SV/EC | Loader Tray Mode | INT_4 |  | 2 | 0 | 1 | 0: None;  1: When Loader had skip device need remove loader tray manually;  2: When Loader had skip device need open the door to check the tray; | ✓ |  |
| 2713 | SV/EC | Auto Tray Feed | BOOLEAN |  | 1 | 0 | 1 |  | ✓ |  |
| 2721 | SV/EC | Auto1 Tray Direction | INT_4 |  | 3 | 0 | 0 | Auto1 Tray Direction | ✓ |  |
| 2722 | SV/EC | Auto2 Tray Direction | INT_4 |  | 3 | 0 | 0 | Auto2 Tray Direction | ✓ |  |
| 2723 | SV/EC | Auto3 Tray Direction | INT_4 |  | 3 | 0 | 0 | Auto3 Tray Direction | ✓ |  |
| 2731 | SV/EC | Fix1 Tray Direction | INT_4 |  | 3 | 0 | 0 | Fix1 Tray Direction | ✓ |  |
| 2732 | SV/EC | Fix2 Tray Direction | INT_4 |  | 3 | 0 | 0 | Fix2 Tray Direction | ✓ |  |
| 2733 | SV/EC | Fix3 Tray Direction | INT_4 |  | 3 | 0 | 0 | Fix3 Tray Direction | ✓ |  |
| 2734 | SV/EC | Loader Tray Type Index | INT_4 |  | 2 | 0 | 0 | 0:Type 1; 1:Type 2; 2:Type 3 | ✓ |  |
| 2735 | SV/EC | Empty Tray Type Index | INT_4 |  | 2 | 0 | 0 | 0:Type 1; 1:Type 2; 2:Type 3 | ✓ |  |
| 2736 | SV/EC | Color Tray Type Index | INT_4 |  | 2 | 0 | 0 | 0:Type 1; 1:Type 2; 2:Type 3 | ✓ |  |
| 2737 | SV/EC | Fix1 Tray Type Index | INT_4 |  | 2 | 0 | 0 | 0:Type 1; 1:Type 2; 2:Type 3 | ✓ |  |
| 2738 | SV/EC | Fix2 Tray Type Index | INT_4 |  | 2 | 0 | 0 | 0:Type 1; 1:Type 2; 2:Type 3 | ✓ |  |
| 2739 | SV/EC | Fix3 Tray Type Index | INT_4 |  | 2 | 0 | 0 | 0:Type 1; 1:Type 2; 2:Type 3 | ✓ |  |
| 2754 | SV | Auto 1 Tray From Alias | ASCII |  |  |  |  | Auto 1 Tray From Alias | ✓ |  |
| 2755 | SV | Auto 2 Tray From Alias | ASCII |  |  |  |  | Auto 2 Tray From Alias | ✓ |  |
| 2756 | SV | Auto 3 Tray From Alias | ASCII |  |  |  |  | Auto 3 Tray From Alias | ✓ |  |
| 2758 | SV/EC | Type 1 Tray Pitch X | FT_8 | mm |  |  |  | Type 1 Tray Pitch X | ✓ |  |
| 2759 | SV/EC | Type 1 Tray Pitch Y | FT_8 | mm |  |  |  | Type 1 Tray Pitch Y | ✓ |  |
| 2760 | SV/EC | Type 1 Tray Start Position X | FT_8 | mm |  |  |  | Type 1 Tray Start Position X | ✓ |  |
| 2761 | SV/EC | Type 1 Tray Start Position Y | FT_8 | mm |  |  |  | Type 1 Tray Start Position Y | ✓ |  |
| 2762 | SV/EC | Type 1 Tray Division X | INT_4 |  |  |  |  | Type 1 Tray Division X | ✓ |  |
| 2763 | SV/EC | Type 1 Tray Division Y | INT_4 |  |  |  |  | Type 1 Tray Division Y | ✓ |  |
| 2764 | SV/EC | Type 1 Tray Pick Up | FT_8 | mm |  |  |  | Type 1 Tray Pick Up | ✓ |  |
| 2766 | SV/EC | Type 1 Tray Block Num X | INT_4 |  |  |  |  | Type 1 Tray Block Num X | ✓ |  |
| 2767 | SV/EC | Type 1 Tray Block Num Y | INT_4 |  |  |  |  | Type 1 Tray Block Num Y | ✓ |  |
| 2768 | SV/EC | Type 1 Tray Block Pitch X | FT_8 | mm |  |  |  | Type 1 Tray Block Pitch X | ✓ |  |
| 2769 | SV/EC | Type 1 Tray Block Pitch Y | FT_8 | mm |  |  |  | Type 1 Tray Block Pitch Y | ✓ |  |
| 2771 | SV/EC | Type 2 Tray Pitch X | FT_8 | mm |  |  |  | Type 2 Tray Pitch X | ✓ |  |
| 2772 | SV/EC | Type 2 Tray Pitch Y | FT_8 | mm |  |  |  | Type 2 Tray Pitch Y | ✓ |  |
| 2773 | SV/EC | Type 2 Tray Start Position X | FT_8 | mm |  |  |  | Type 2 Tray Start Position X | ✓ |  |
| 2774 | SV/EC | Type 2 Tray Start Position Y | FT_8 | mm |  |  |  | Type 2 Tray Start Position Y | ✓ |  |
| 2775 | SV/EC | Type 2 Tray Division X | INT_4 |  |  |  |  | Type 2 Tray Division X | ✓ |  |
| 2776 | SV/EC | Type 2 Tray Division Y | INT_4 |  |  |  |  | Type 2 Tray Division Y | ✓ |  |
| 2777 | SV/EC | Type 2 Tray Pick Up | FT_8 | mm |  |  |  | Type 2 Tray Pick Up | ✓ |  |
| 2778 | EC | Type 2 Tray Memo | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2779 | SV/EC | Type 2 Tray Block Num X | INT_4 |  |  |  |  | Type 2 Tray Block Num X | ✓ |  |
| 2780 | SV/EC | Type 2 Tray Block Num Y | INT_4 |  |  |  |  | Type 2 Tray Block Num Y | ✓ |  |
| 2781 | SV/EC | Type 2 Tray Block Pitch X | FT_8 | mm |  |  |  | Type 2 Tray Block Pitch X | ✓ |  |
| 2782 | SV/EC | Type 2 Tray Block Pitch Y | FT_8 | mm |  |  |  | Type 2 Tray Block Pitch Y | ✓ |  |
| 2783 | EC | Type 3 Tray Name | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2784 | SV/EC | Type 3 Tray Pitch X | FT_8 | mm |  |  |  | Type 3 Tray Pitch X | ✓ |  |
| 2785 | SV/EC | Type 3 Tray Pitch Y | FT_8 | mm |  |  |  | Type 3 Tray Pitch Y | ✓ |  |
| 2786 | SV/EC | Type 3 Tray Start Position X | FT_8 | mm |  |  |  | Type 3 Tray Start Position X | ✓ |  |
| 2787 | SV/EC | Type 3 Tray Start Position Y | FT_8 | mm |  |  |  | Type 3 Tray Start Position Y | ✓ |  |
| 2788 | SV/EC | Type 3 Tray Division X | INT_4 |  |  |  |  | Type 3 Tray Division X | ✓ |  |
| 2789 | SV/EC | Type 3 Tray Division Y | INT_4 |  |  |  |  | Type 3 Tray Division Y | ✓ |  |
| 2790 | SV/EC | Type 3 Tray Pick Up | FT_8 | mm |  |  |  | Type 3 Tray Pick Up | ✓ |  |
| 2791 | EC | Type 3 Tray Memo | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2792 | SV/EC | Type 3 Tray Block Num X | INT_4 |  |  |  |  | Type 3 Tray Block Num X | ✓ |  |
| 2793 | SV/EC | Type 3 Tray Block Num Y | INT_4 |  |  |  |  | Type 3 Tray Block Num Y | ✓ |  |
| 2794 | SV/EC | Type 3 Tray Block Pitch X | FT_8 | mm |  |  |  | Type 3 Tray Block Pitch X | ✓ |  |
| 2795 | SV/EC | Type 3 Tray Block Pitch Y | FT_8 | mm |  |  |  | Type 3 Tray Block Pitch Y | ✓ |  |
| 2796 | EC | Bin Box Name | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2797 | SV/EC | Bin Box Alarm Count | INT_4 |  | 5000 | 100 | 100 | Bin Box Alarm Count | ✓ |  |
| 2798 | SV/EC | Bin Box Current Count | INT_4 |  |  | 5 |  | Bin Box Current Count | ✓ |  |
| 2799 | SV/EC | Device Direction | INT_4 |  |  |  |  | 0: 0 deg;  1: 90 deg;  2: 180 deg;  3: 270 deg; | ✓ |  |
| 2800 | SV/EC | Tray Direction | INT_4 |  |  |  |  | 0: 0 deg;  1: 180 deg; | ✓ |  |
| 2801 | SV/EC | Hot Plate Form Select | ASCII |  |  |  |  | Hot Plate Form Select | ✓ |  |
| 2802 | SV/EC | Hot Plate Use Select | INT_4 |  | 2 | 0 | 0 | 0:Dual Plate; 1:Plate1 Only; 2:Plate2 Only | ✓ |  |
| 2804 | SV/EC | Use Wide Hotplate | BOOLEAN |  |  |  |  | Use Wide Hotplate | ✓ |  |
| 2811 | SV/EC | Hot Plate Pitch X | FT_8 | mm | 160 | 1 |  | Hot Plate Pitch X | ✓ |  |
| 2812 | SV/EC | Hot Plate Pitch Y | FT_8 | mm | 200 | 1 |  | Hot Plate Pitch Y | ✓ |  |
| 2813 | SV/EC | Hot Plate Start Position X | FT_8 | mm | 160 | 1 |  | Hot Plate Start Position X | ✓ |  |
| 2814 | SV/EC | Hot Plate Start Position Y | FT_8 | mm | 200 | 1 |  | Hot Plate Start Position Y | ✓ |  |
| 2815 | SV/EC | Hot Plate Division X | INT_4 |  | 20 | 1 |  | Hot Plate Division X | ✓ |  |
| 2816 | SV/EC | Hot Plate Division Y | INT_4 |  | 40 | 1 |  | Hot Plate Division Y | ✓ |  |
| 2819 | EC | Type 1 Tray Thickness | FT_8 | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2820 | EC | Type 2 Tray Thickness | FT_8 | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 2821 | EC | Type 3 Tray Thickness | FT_8 | mm |  |  |  |  | ✓ | 🟡 程式獨有 |

## 擴充 / 客製 (3000-4999)
> ⚠️ 此區段有 **168** 筆差異

| ID | 類型 | Name | DataType | Unit | Max | Min | Default | Description | Code | 差異 |
|----|------|------|----------|------|-----|-----|---------|-------------|------|------|
| 3400 | SV/EC | Interface Type for HT9045 | INT_4 |  | 2 | 0 | 1 | 0: DIO;  1: GP-IB;  2: RS232; 3: TCP/IP | ✓ |  |
| 3401 | SV/EC | GPIB Mode for HT9045 | INT_4 |  | 4 | 0 | 0 | 0: Advan type1;  1: 256 Bin 2: 16 Bin 3: 32 Bin 4: SPEA Type | ✓ |  |
| 3402 | SV/EC | RS232 Mode for HT9045 | INT_4 |  | 0 | 0 | 0 | 0: Standard; | ✓ |  |
| 3403 | SV/EC | RS232 Max Bin Count | INT_4 |  | 99 | 15 | 15 | RS232 Max Bin Count | ✓ |  |
| 3404 | SV/EC | RS232 Need Send VSOT | BOOLEAN |  | 1 | 0 | 0 | For SLT Test |  | 🔴 文件獨有 |
| 3405 | SV/EC | Initial Start Delay Time | FT_8 | Second |  |  |  | Initial Start Delay Time | ✓ |  |
| 3406 | SV/EC | Initial Start Delay Count | INT_4 |  |  |  |  | Initial Start Delay Count | ✓ |  |
| 3407 | SV/EC | Initial Testing Stop Wait Time | FT_8 | Second |  |  |  | Testing Need Stop All Motor | ✓ |  |
| 3408 | SV/EC | Testing Stop Wait Time | FT_8 | Second |  |  |  | Testing Need Stop All Motor | ✓ |  |
| 3409 | SV/EC | Enable Start Delay in Every First Device | BOOLEAN |  | 1 | 0 | 0 | Enable Start Delay in Every First Device | ✓ |  |
| 3410 | SV/EC | Enable Start Delay After Show Alarm Message | BOOLEAN |  | 1 | 0 | 0 | Enable Start Delay After Show Alarm Message | ✓ |  |
| 3411 | SV/EC | Enable Start Delay When Test Time is Fast | BOOLEAN |  | 1 | 0 | 0 | Enable Start Delay When Test Time is Fast | ✓ |  |
| 3412 | SV/EC | Enable Start Delay Test Time | FT_8 | Second |  |  |  | Enable Start Delay Test Time | ✓ |  |
| 3413 | SV/EC | Enable Start Delay Time | FT_8 | Second |  |  |  | Enable Start Delay Time | ✓ |  |
| 3414 | EC | After Tested Delay | FT_8 | Second |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3450 | SV/EC | Test Mode for HT9045 | ASCII |  |  |  |  | Single Site;  2-Site;  2-Site Busy Shuttle;  In-Line 4-Site(1X4);  2-Site (2x1);  Square 4-Site(2X2);  Square 4-Site(2X2) Busy Shuttle;  6-Site; 8-Site;  12-Site;  16-Site;   32-Site N Mode; 32-Site M Mode; | ✓ |  |
| 3451 | SV/EC | Site X Pitch for HT9045 | FT_8 |  | 120 | 10 | 40 | Site X Pitch for HT9045 | ✓ |  |
| 3452 | SV/EC | Site Y Pitch for HT9045 | FT_8 |  | 80 | 10 | 60 | Site Y Pitch for HT9045 | ✓ |  |
| 3453 | SV/EC | In Arm Y-Pitch | INT_4 |  | 1 | 0 | 0 | 0: 60mm;  1: 63.5mm / 36mm; | ✓ |  |
| 3454 | SV/EC | Enable Reat Time CCD | BOOLEAN |  | 1 | 0 | 0 | Enable Reat Time CCD 0:啟動 Reat Time CCD 1:關閉 Reat Time CCD | ✓ |  |
| 3455 | SV/EC | NS7000 Bias Kit | BOOLEAN |  | 1 | 0 | 0 | NS7000 Bias Kit | ✓ |  |
| 3456 | SV/EC | NS7000 Change Socket | BOOLEAN |  | 1 | 0 | 0 | NS7000 Change Socket | ✓ |  |
| 3457 | SV/EC | Rotate Shuttle | BOOLEAN |  | 1 | 0 | 0 | Rotate Shuttle | ✓ |  |
| 3458 | SV/EC | NS8000H Change Kit | BOOLEAN |  | 1 | 0 | 0 | NS8000H Change Kit | ✓ |  |
| 3459 | SV/EC | Octal Site X Pitch 80mm | BOOLEAN |  | 1 | 0 | 0 | Octal Site X Pitch 80mm | ✓ |  |
| 3460 | SV/EC | Octal Site Use 16 Site Layout Kit | BOOLEAN |  | 1 | 0 | 0 | Octal Site Use 16 Site Layout Kit | ✓ |  |
| 3461 | SV/EC | Shuttle Mode | INT_4 |  | 1 | 0 | 0 | 0: Normal;  1: One Side; | ✓ |  |
| 3462 | SV/EC | Shuttle Select | INT_4 |  | 1 | 0 | 0 | 0: Use Shuttle 1;  1: Use Shuttle 2; | ✓ |  |
| 3463 | SV/EC | 2x2 site use 2x4 Site Layout Kit | BOOLEAN |  | 1 | 0 | 0 | 2x2 site use 2x4 Site Layout Kit | ✓ |  |
| 3464 | SV/EC | Octal site use 12 Site Layout Kit | BOOLEAN |  | 1 | 0 | 0 | Octal site use 12 Site Layout Kit | ✓ |  |
| 3465 | SV/EC | 12_16 Site Direct Heater Layout | BOOLEAN |  | 1 | 0 | 0 | 12_16 Site Direct Heater Layout | ✓ |  |
| 3466 | SV/EC | 12 Site 10 Direct Heater Layout | BOOLEAN |  | 1 | 0 | 0 | 12 Site 10 Direct Heater Layout | ✓ |  |
| 3467 | SV/EC | Enable Arm 1 Pick Arm 2 Test Function | BOOLEAN |  | 1 | 0 | 0 | Enable Arm 1 Pick Arm 2 Test Function by Setup File | ✓ |  |
| 3468 | SV/EC | Check Arm 2 Vacuum | BOOLEAN |  | 1 | 0 | 0 | In arm 1 pick arm 2 test function need check arm 2 vacuum | ✓ |  |
| 3469 | EC | sSiteLayout | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3470 | EC | sSiteMap | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3471 | SV/EC | Use Socket Sensor | BOOLEAN |  | 1 | 0 | 0 | Use Socket Sensor | ✓ | ⚠️ EC名稱: Code=`Socket sensor` |
| 3472 | SV/EC | Socket Sensor Count | INT_4 |  | 8 | 1 | 1 | Socket Sensor Count | ✓ |  |
| 3473 | SV/EC | 2 Cable Layout Kit | BOOLEAN |  | 1 | 0 | 0 | 2 Cable Layout Kit | ✓ |  |
| 3474 | SV/EC | 1 Cable Layout Kit | BOOLEAN |  | 1 | 0 | 0 | 1 Cable Layout Kit | ✓ |  |
| 3511 | SV/EC | TTL Mode | ASCII |  |  |  |  | TTL Mode | ✓ |  |
| 3512 | SV/EC | TTL Anti-Signal | BOOLEAN |  | 1 | 0 | 0 | FALSE: The Start signal will be output in the positive format.; TRUE: The Start signal will be output in the negative format. | ✓ |  |
| 3517 | SV/EC | GPIB Address | INT_4 |  | 30 | 1 | 1 | GPIB Address | ✓ |  |
| 3521 | SV/EC | Maximun Test Time | FT_8 | Second |  |  |  | Maximun Test Time | ✓ |  |
| 3522 | SV/EC | Initial Maximun Test Time | FT_8 | Second |  |  |  | Initial Maximun Test Time | ✓ |  |
| 3523 | SV/EC | Start Delay Time | FT_8 | Second | 100 | 0.01 | 0.1 | Start Signal Delay Time | ✓ |  |
| 3524 | SV/EC | Dummy Test Time | FT_8 | Second |  |  |  | Dummy Test Time | ✓ |  |
| 3540 | SV/EC | Site Aa to Dh Mapping | ASCII |  |  |  |  | Site Aa to Dh Mapping;  0 is close site;  CSV format | ✓ |  |
| 3541 | SV/EC | Testing Stop Time - Initial Wait Time | FT_8 | Second |  |  |  | While testing must stop all motor. |  | 🔴 文件獨有 |
| 3542 | SV/EC | Testing Stop Time - Testing Wait Time | FT_8 | Second |  |  |  | While testing must stop all motor. |  | 🔴 文件獨有 |
| 3543 | SV/EC | Use initial start delay in socket - Every first devices | BOOLEAN |  |  |  |  | Use initial start delay in socket - Every first devices | ✓ |  |
| 3544 | SV/EC | Use initial start delay in socket - Every first devices Delay Time | FT_8 | Second |  |  |  | Use initial start delay in socket - Every first devices Delay Time | ✓ |  |
| 3545 | SV/EC | Use initial start delay in socket - After Show Alarm Message | BOOLEAN |  |  |  |  | Use initial start delay in socket - After Show Alarm Message | ✓ |  |
| 3546 | SV/EC | Use initial start delay in socket - After Show Alarm Message Delay Time | FT_8 | Second |  |  |  | Use initial start delay in socket - After Show Alarm Message Delay Time | ✓ |  |
| 3547 | SV/EC | Use initial start delay in socket - After Auto Clean Function | BOOLEAN |  |  |  |  | Use initial start delay in socket - After Auto Clean Function | ✓ |  |
| 3548 | SV/EC | Use initial start delay in socket - After Auto Clean Function Delay Time | FT_8 | Second |  |  |  | Use initial start delay in socket - After Auto Clean Function Delay Time | ✓ |  |
| 3549 | SV/EC | Use initial start delay in socket - After Open HeatDoor | BOOLEAN |  |  |  |  | Use initial start delay in socket - After Open HeatDoor | ✓ |  |
| 3550 | SV/EC | Use initial start delay in socket - After Open HeatDoor Delay Time | FT_8 | Second |  |  |  | Use initial start delay in socket - After Open HeatDoor Delay Time | ✓ |  |
| 3551 | SV/EC | Use initial start delay in socket - When happen short tested time | BOOLEAN |  |  |  |  | Use initial start delay in socket - When happen short tested time |  | 🔴 文件獨有 |
| 3552 | SV/EC | Use initial start delay in socket - When happen short tested time | FT_8 | Second |  |  |  | Use initial start delay in socket - When happen short tested time |  | 🔴 文件獨有 |
| 3553 | SV/EC | Use initial start delay in socket - When happen short tested time Delay Time | FT_8 | Second |  |  |  | Use initial start delay in socket - When happen short tested time Delay Time | ✓ |  |
| 3554 | SV/EC | Use initial start delay in socket - Use when press stop over time | BOOLEAN |  |  |  |  | Use initial start delay in socket - Use when press stop over time | ✓ |  |
| 3555 | SV/EC | Use initial start delay in socket - When press stop over time setting | FT_8 | Second |  |  |  | Use initial start delay in socket - When press stop over time setting | ✓ |  |
| 3556 | SV/EC | Use initial start delay in socket - Delay time when press stop over time | FT_8 | Second |  |  |  | Use initial start delay in socket - Delay time when press stop over time | ✓ |  |
| 3557 | SV/EC | Use initial start delay in socket - Use when test site is no full | BOOLEAN |  |  |  |  | Use initial start delay in socket - Use when test site is no full | ✓ |  |
| 3558 | SV/EC | Use initial start delay in socket - Delay time when test site is no full | FT_8 | Second |  |  |  | Use initial start delay in socket - Delay time when test site is no full | ✓ |  |
| 3559 | SV/EC | Use initial start delay in socket - Use when EOT to SOT over time | BOOLEAN |  |  |  |  | Use initial start delay in socket - Use when EOT to SOT over time | ✓ |  |
| 3560 | SV/EC | Use initial start delay in socket - EOT to SOT over time setting | FT_8 | Second |  |  |  | Use initial start delay in socket - EOT to SOT over time setting | ✓ |  |
| 3561 | SV/EC | Use initial start delay in socket - Delay time of EOT to SOT over time | FT_8 | Second |  |  |  | Use initial start delay in socket - Delay time of EOT to SOT over time | ✓ |  |
| 3562 | SV/EC | Use initial start delay in socket - Use when OTD unlock | BOOLEAN |  |  |  |  | Use initial start delay in socket - Use when OTD unlock | ✓ |  |
| 3563 | SV/EC | Use initial start delay in socket - Delay time when OTD unlock | FT_8 | Second |  |  |  | Use initial start delay in socket - Delay time when OTD unlock | ✓ |  |
| 3564 | SV/EC | Use initial start delay in socket - Use when SOT to EOT over time | BOOLEAN |  |  |  |  | Use initial start delay in socket - Use when SOT to EOT over time | ✓ |  |
| 3565 | SV/EC | Use initial start delay in socket - SOT to EOT over time setting | FT_8 | Second |  |  |  | Use initial start delay in socket - SOT to EOT over time setting | ✓ |  |
| 3566 | SV/EC | Use initial start delay in socket - Delay time of SOT to EOT over time | FT_8 | Second |  |  |  | Use initial start delay in socket - Delay time of SOT to EOT over time | ✓ |  |
| 3567 | SV/EC | Use initial start delay in socket - Enable Start Delay Time_RT | FT_8 | Second |  |  |  | Use initial start delay in socket - Enable Start Delay Time_RT | ✓ |  |
| 3568 | SV/EC | Use initial start delay in socket - After Show Alarm Message Delay Time_RT | FT_8 | Second |  |  |  | Use initial start delay in socket - After Show Alarm Message Delay Time_RT | ✓ |  |
| 3569 | SV/EC | Use initial start delay in socket - After Auto Clean Function Delay Time_RT | FT_8 | Second |  |  |  | Use initial start delay in socket - After Auto Clean Function Delay Time_RT | ✓ |  |
| 3570 | SV/EC | Use initial start delay in socket - After Open HeatDoor Delay Time_RT | FT_8 | Second |  |  |  | Use initial start delay in socket - After Open HeatDoor Delay Time_RT | ✓ |  |
| 3571 | SV/EC | Use initial start delay in socket - After Open HeatDoor Delay Time_RT | FT_8 | Second |  |  |  | Use initial start delay in socket - After Open HeatDoor Delay Time_RT | ✓ | ⚠️ EC名稱: Code=`Use initial start delay in socket - Delay Time when happen short tested time_RT` |
| 3572 | SV/EC | Use initial start delay in socket - Delay time when press stop over time_RT | FT_8 | Second |  |  |  | Use initial start delay in socket - Delay time when press stop over time_RT | ✓ |  |
| 3573 | SV/EC | Use initial start delay in socket - Delay time when test site is no full_RT | FT_8 | Second |  |  |  | Use initial start delay in socket - Delay time when test site is no full_RT | ✓ |  |
| 3574 | SV/EC | Use initial start delay in socket - Delay time when OTD unlock_RT | FT_8 | Second |  |  |  | Use initial start delay in socket - Delay time when OTD unlock_RT | ✓ |  |
| 3575 | SV/EC | Use initial start delay in socket - Delay time of EOT to SOT over time_RT | FT_8 | Second |  |  |  | Use initial start delay in socket - Delay time of EOT to SOT over time_RT | ✓ |  |
| 3576 | SV/EC | Use initial start delay in socket - Delay time of SOT to EOT over time_RT | FT_8 | Second |  |  |  | Use initial start delay in socket - Delay time of SOT to EOT over time_RT | ✓ |  |
| 3577 | EC | Enable Temperature Offset Function | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3578 | EC | Contact count for offset period | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3579 | EC | Contact count for cool down | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3580 | EC | Enable chamber boost function | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3581 | EC | Boost period (Min, 1 to 30) | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3582 | EC | Boost temperature offset (°C, 0 to 30) | INT_4 | Celsius |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3616 | EC | Error Bin Tray Select | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3617 | EC | Bin 0-255 Tray Select | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3618 | EC | Bin tray select for all | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3636 | SV/EC | Bin 0-255 Contact | ASCII |  |  |  |  | 0:Single; 1:Double CSV Format for Bin 0 to 255 | ✓ |  |
| 3640 | EC | Bin 0 Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3641 | EC | Bin 1 Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3642 | EC | Bin 2 Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3643 | EC | Bin 3 Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3644 | EC | Bin 4 Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3645 | EC | Bin 5 Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3646 | EC | Bin 6 Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3647 | EC | Bin 7 Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3648 | EC | Bin 8 Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3649 | EC | Bin 9 Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3650 | EC | Bin 10 Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3651 | EC | Bin 11 Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3652 | EC | Bin 12 Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3653 | EC | Bin 13 Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3654 | EC | Bin 14 Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3655 | EC | Bin 15 Type | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3656 | SV/EC | Bin 0-255 Type | ASCII |  |  |  |  | 0:Pase; 1:Fail;  CSV Format for Bin 0 to 255 | ✓ |  |
| 3676 | SV/EC | Bin 0-255 Continut Error Alarm Select | ASCII |  |  |  |  | 0:Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 3677 | EC | Tray for Fail Bin | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3678 | EC | Tray for Need Auto Retest | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3716 | EC | Error Bin Tray Select for RT | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3717 | EC | Bin 0-255 Tray Select for RT | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3718 | SV/EC | Bin 0-255 Contact for RT | ASCII |  |  |  |  | 0:Single; 1:Double;  CSV Format for Bin 0 to 255 | ✓ | ⚠️ EC名稱: Code=`Bin 0-255 Contact RT` |
| 3719 | SV/EC | Bin 0-255 Type for RT | ASCII |  |  |  |  | 0:Pase; 1:Fail;  CSV Format for Bin 0 to 255 | ✓ |  |
| 3720 | SV/EC | Bin 0-255 Continut Error Alarm Select for RT | ASCII |  |  |  |  | Continut Error Alarm 0:Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 3721 | EC | Error Bin Tray Select for OFF-Line | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3722 | EC | Bin 0-255 Tray Select for OFF-Line | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3723 | SV/EC | Bin 0-255 Contact for OFF-Line | ASCII |  |  |  |  | 0:Single; 1:Double;  CSV Format for Bin 0 to 255 | ✓ |  |
| 3724 | SV/EC | Bin 0-255 Type for OFF-Line | ASCII |  |  |  |  | 0:Pase; 1:Fail;  CSV Format for Bin 0 to 255 | ✓ |  |
| 3725 | SV/EC | Bin 0-255 Continut Error Alarm Select for OFF-Line | ASCII |  |  |  |  | Continut Error Alarm 0:Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 3726 | EC | Tray for Fail Bin (RT) | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3727 | EC | Tray for Need Auto Retest (RT) | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3728 | EC | Tray for Fail Bin (Off Line) | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3729 | EC | Tray for Need Auto Retest (Off Line) | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3800 | EC | Error Bin Tray Select for ART_FT | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3801 | EC | Bin 0-255 Tray Select for ART_FT | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3802 | SV/EC | Bin 0-255 Contact for ART_FT | ASCII |  |  |  |  | 0:Single; 1:Double;  CSV Format for Bin 0 to 255 | ✓ |  |
| 3803 | SV/EC | Bin 0-255 Type for ART_FT | ASCII |  |  |  |  | 0:Pase; 1:Fail;  CSV Format for Bin 0 to 255 | ✓ |  |
| 3804 | SV/EC | Bin 0-255 Continut Error Alarm Select for ART_FT | ASCII |  |  |  |  | 0:Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 3805 | EC | Tray for Fail Bin (ART_FT) | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3806 | EC | Tray for Need Auto Retest (ART_FT) | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3900 | EC | Error Bin Tray Select for ART_RT | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3901 | EC | Bin 0-255 Tray Select for ART_RT | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3902 | SV/EC | Bin 0-255 Contact for ART_RT | ASCII |  |  |  |  | 0:Single; 1:Double;  CSV Format for Bin 0 to 255 | ✓ |  |
| 3903 | SV/EC | Bin 0-255 Type for ART_RT | ASCII |  |  |  |  | 0:Pase; 1:Fail;  CSV Format for Bin 0 to 255 | ✓ |  |
| 3904 | SV/EC | Bin 0-255 Continut Error Alarm Select for ART_RT | ASCII |  |  |  |  | 0:Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 3905 | EC | Tray for Fail Bin (ART_RT) | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 3906 | EC | Tray for Need Auto Retest (ART_RT) | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4000 | EC | Error Bin Tray Select for MRT_FT | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4001 | EC | Bin 0-255 Tray Select for MRT_FT | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4002 | SV/EC | Bin 0-255 Contact for MRT_FT | ASCII |  |  |  |  | 0:Single; 1:Double;  CSV Format for Bin 0 to 255 | ✓ |  |
| 4003 | SV/EC | Bin 0-255 Type for MRT_FT | ASCII |  |  |  |  | 0:Pase; 1:Fail;  CSV Format for Bin 0 to 255 | ✓ |  |
| 4004 | SV/EC | Bin 0-255 Continut Error Alarm Select for MRT_FT | ASCII |  |  |  |  | 0:Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 4005 | EC | Tray for Fail Bin (MRT_FT) | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4006 | EC | Tray for Need Auto Retest (MRT_FT) | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4009 | EC | Use MRT Bin Tray Mode | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4100 | EC | Error Bin Tray Select for MRT_RT | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4101 | EC | Bin 0-255 Tray Select for MRT_RT | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4102 | SV/EC | Bin 0-255 Contact for MRT_RT | ASCII |  |  |  |  | 0:Single; 1:Double;  CSV Format for Bin 0 to 255 | ✓ |  |
| 4103 | SV/EC | Bin 0-255 Type for MRT_RT | ASCII |  |  |  |  | 0:Pase; 1:Fail;  CSV Format for Bin 0 to 255 | ✓ |  |
| 4104 | SV/EC | Bin 0-255 Continut Error Alarm Select for MRT_RT | ASCII |  |  |  |  | 0:Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 4105 | EC | Tray for Fail Bin (MRT_RT) | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4106 | EC | Tray for Need Auto Retest (MRT_RT) | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4201 | SV/EC | In Arm to Loader Offset | ASCII | mm |  |  |  | CSV Format with 7 data:  X; Y; Pitch; Pick; Place; PitchY; PitchX2; | ✓ |  |
| 4202 | SV/EC | In Arm to Hot Plate 1 Offset | ASCII | mm |  |  |  | CSV Format with 7 data:  X; Y; Pitch; Pick; Place; PitchY; PitchX2; | ✓ |  |
| 4203 | SV/EC | In Arm to Hot Plate 2 Offset | ASCII | mm |  |  |  | CSV Format with 7 data:  X; Y; Pitch; Pick; Place; PitchY; PitchX2; | ✓ |  |
| 4204 | SV/EC | In Arm to Shuttle 1 Offset | ASCII | mm |  |  |  | CSV Format with 7 data:  X; Y; Pitch; Pick; Place; PitchY; PitchX2; | ✓ |  |
| 4205 | SV/EC | In Arm to Shuttle 2 Offset | ASCII | mm |  |  |  | CSV Format with 7 data:  X; Y; Pitch; Pick; Place; PitchY; PitchX2; | ✓ |  |
| 4206 | SV/EC | In Arm to Auto Clean Kit Offset | ASCII | mm |  |  |  | CSV Format with 7 data:  X; Y; Pitch; Pick; Place; PitchY; PitchX2; | ✓ |  |
| 4208 | SV/EC | In Arm to Rotater Offset | ASCII | mm |  |  |  | CSV Format with 7 data:  X; Y; Pitch; Pick; Place; PitchY; PitchX2; | ✓ |  |
| 4251 | SV/EC | Out Arm to Shuttle 1 Offset | ASCII | mm |  |  |  | CSV Format with 7 data:  X; Y; Pitch; Pick; Place; PitchY; PitchX2; | ✓ |  |
| 4252 | SV/EC | Out Arm to Shuttle 2 Offset | ASCII | mm |  |  |  | CSV Format with 7 data:  X; Y; Pitch; Pick; Place; PitchY; PitchX2; | ✓ |  |
| 4253 | SV/EC | Out Arm to Auto 1 Offset | ASCII | mm |  |  |  | CSV Format with 7 data:  X; Y; Pitch; Pick; Place; PitchY; PitchX2; | ✓ |  |
| 4254 | SV/EC | Out Arm to Auto 2 Offset | ASCII | mm |  |  |  | CSV Format with 7 data:  X; Y; Pitch; Pick; Place; PitchY; PitchX2; | ✓ |  |
| 4255 | SV/EC | Out Arm to Auto 3 Offset | ASCII | mm |  |  |  | CSV Format with 7 data:  X; Y; Pitch; Pick; Place; PitchY; PitchX2; | ✓ |  |
| 4256 | SV/EC | Out Arm to Fix 1 Offset | ASCII | mm |  |  |  | CSV Format with 7 data:  X; Y; Pitch; Pick; Place; PitchY; PitchX2; | ✓ |  |
| 4257 | SV/EC | Out Arm to Fix 2 Offset | ASCII | mm |  |  |  | CSV Format with 7 data:  X; Y; Pitch; Pick; Place; PitchY; PitchX2; | ✓ |  |
| 4258 | SV/EC | Out Arm to Fix 3 Offset | ASCII | mm |  |  |  | CSV Format with 7 data:  X; Y; Pitch; Pick; Place; PitchY; PitchX2; | ✓ |  |
| 4259 | SV/EC | Out Arm to Rotater Offset | ASCII | mm |  |  |  | CSV Format with 7 data:  X; Y; Pitch; Pick; Place; PitchY; PitchX2; | ✓ |  |
| 4260 | EC | Out Arm to Auto 4 Offset | ASCII | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4261 | EC | Out Arm to Auto 5 Offset | ASCII | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4262 | EC | Out Arm to Auto 6 Offset | ASCII | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4263 | EC | Out Arm to Fix 4 Offset | ASCII | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4264 | EC | Out Arm to Fix 5 Offset | ASCII | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4265 | EC | Out Arm to Fix 6 Offset | ASCII | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4301 | SV/EC | In Arm to Loader Pick Up Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4302 | SV/EC | In Arm to Hot Plate 1 Pick Up Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4303 | SV/EC | In Arm to Hot Plate 2 Pick Up Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4304 | EC | In Arm to Loader Pick Up Offset(ALL Z) | FT_8 | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4308 | SV/EC | In Arm to Rotater Pick Up Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4351 | SV/EC | Out Arm to Shuttle 1 Pick Up Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4352 | SV/EC | Out Arm to Shuttle 2 Pick Up Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4353 | EC | Out Arm to Shuttle 1 Pick Up Offset(ALL Z) | FT_8 | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4354 | EC | Out Arm to Shuttle 2 Pick Up Offset(ALL Z) | FT_8 | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4359 | SV/EC | Out Arm to Rotater Pick Up Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4402 | SV/EC | In Arm to Hot Plate 1 Place Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4403 | SV/EC | In Arm to Hot Plate 2 Place Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4404 | SV/EC | In Arm to Shuttle 1 Place Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4405 | SV/EC | In Arm to Shuttle 2 Place Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4406 | SV/EC | In Arm to Auto Clean Kit Place Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4408 | SV/EC | In Arm to Rotater Place Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4453 | SV/EC | Out Arm to Auto 1 Place Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4454 | SV/EC | Out Arm to Auto 2 Place Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4455 | SV/EC | Out Arm to Auto 3 Place Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4456 | SV/EC | Out Arm to Fix 1 Place Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4457 | SV/EC | Out Arm to Fix 2 Place Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4458 | SV/EC | Out Arm to Fix 3 Place Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4459 | SV/EC | Out Arm to Rotater Place Offset | ASCII | mm |  |  |  | CSV Format with 8 data:  Pick A; B; C; D; E; F; G; H | ✓ |  |
| 4460 | EC | Out Arm to Auto 4 Place Offset | ASCII | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4461 | EC | Out Arm to Auto 5 Place Offset | ASCII | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4462 | EC | Out Arm to Auto 6 Place Offset | ASCII | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4463 | EC | Out Arm to Fix 4 Place Offset | ASCII | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4464 | EC | Out Arm to Fix 5 Place Offset | ASCII | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4465 | EC | Out Arm to Fix 6 Place Offset | ASCII | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4490 | SV | EESUG Offest Data select location | INT_4 |  | 0 | 0 | 0 | EESUG Offest Address |  | 🔴 文件獨有 |
| 4491 | SV | EESUG Offest Data modify location | INT_4 |  | 0 | 0 | 0 | 0:Noraml 1:X 2:Y 3:Pitch 4:Pick 5:Place 6:PitchY 7:PitchX2 8:Contact Height Offset |  | 🔴 文件獨有 |
| 4492 | SV | EESUG Offset Unit | ASCII |  | 0 | 0 | 0 | EESUG Offest Unit |  | 🔴 文件獨有 |
| 4527 | SV/EC | Hot Plate2 Pick Offset | FT_8 | mm |  |  |  | Hot Plate2 Pick Offset |  | 🔴 文件獨有 |
| 4528 | SV/EC | Hot Plate2 Place Offset | FT_8 | mm |  |  |  | Hot Plate2 Place Offset |  | 🔴 文件獨有 |
| 4529 | SV/EC | Hot Plate1 Pick Offset | FT_8 | mm |  |  |  | Hot Plate1 Pick Offset |  | 🔴 文件獨有 |
| 4530 | SV/EC | Hot Plate1 Place Offset | FT_8 | mm |  |  |  | Hot Plate1 Place Offset |  | 🔴 文件獨有 |
| 4568 | SV/EC | Loader Tray Pick Offset | FT_8 | mm |  |  |  | Loader Tray Pick Offset |  | 🔴 文件獨有 |
| 4661 | SV/EC | Test Arm2 Pick Offset | FT_8 | mm |  |  |  | Test Arm2 Pick Offset | ✓ |  |
| 4662 | SV/EC | Test Arm2 Place Offset | FT_8 | mm |  |  |  | Test Arm2 Place Offset | ✓ |  |
| 4663 | SV/EC | Test Arm2 Contact Height Offset | FT_8 | mm |  |  |  | Test Arm2 Contact Height Offset | ✓ |  |
| 4664 | SV/EC | Test Arm2 Busy Shuttle Half Offset | FT_8 | mm |  |  |  | Test Arm2 Busy Shuttle Half Offset | ✓ |  |
| 4665 | SV/EC | Test Arm2 Shuttle Right Offset | FT_8 | mm | 2 | -2 | 0 | Test Arm2 Shuttle Right Offset | ✓ |  |
| 4666 | SV/EC | Test Arm2 Shuttle Left Offset | FT_8 | mm | 2 | -2 | 0 | Test Arm2 Shuttle Left Offset | ✓ |  |
| 4671 | SV/EC | Test Arm1 Pick Offset | FT_8 | mm |  |  |  | Test Arm1 Pick Offset | ✓ |  |
| 4672 | SV/EC | Test Arm1 Place Offset | FT_8 | mm |  |  |  | Test Arm1 Place Offset | ✓ |  |
| 4673 | SV/EC | Test Arm1 Contact Height Offset | FT_8 | mm |  |  |  | Test Arm1 Contact Height Offset | ✓ |  |
| 4674 | SV/EC | Test Arm1 Busy Shuttle Half Offset | FT_8 | mm |  |  |  | Test Arm1 Busy Shuttle Half Offset | ✓ |  |
| 4675 | SV/EC | Test Arm1 Shuttle Right Offset | FT_8 | mm | 2 | -2 | 0 | Test Arm1 Shuttle Right Offset | ✓ |  |
| 4676 | SV/EC | Test Arm1 Shuttle Left Offset | FT_8 | mm | 2 | -2 | 0 | Test Arm1 Shuttle Left Offset | ✓ |  |
| 4682 | SV/EC | Tray Arm Pick Loader Offset | FT_8 | mm |  |  |  | Tray Arm Pick Loader Offset | ✓ |  |
| 4683 | SV/EC | Tray Arm Pick Buffer Offset | FT_8 | mm |  |  |  | Tray Arm Pick Buffer Offset | ✓ |  |
| 4684 | SV/EC | Tray Arm Pick Color Offset | FT_8 | mm |  |  |  | Tray Arm Pick Color Offset | ✓ |  |
| 4685 | SV/EC | Tray Arm Place Auto1 Offset | FT_8 | mm |  |  |  | Tray Arm Place Auto1 Offset | ✓ | ⚠️ EC名稱: Code=`Tray Arm Place Auto 1 Offset` |
| 4686 | SV/EC | Tray Arm Place Auto2 Offset | FT_8 | mm |  |  |  | Tray Arm Place Auto2 Offset | ✓ | ⚠️ EC名稱: Code=`Tray Arm Place Auto 2 Offset` |
| 4687 | SV/EC | Tray Arm Place Auto3 Offset | FT_8 | mm |  |  |  | Tray Arm Place Auto3 Offset | ✓ | ⚠️ EC名稱: Code=`Tray Arm Place Auto 3 Offset` |
| 4688 | EC | Tray Arm Place Auto 4 Offset | FT_8 | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4689 | EC | Tray Arm Place Auto 5 Offset | FT_8 | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4690 | EC | Tray Arm Place Auto 6 Offset | FT_8 | mm |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4751 | SV/EC | Plate1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Plate1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Plate1 Temperature Offset` |
| 4752 | SV/EC | Plate2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Plate2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Plate2 Temperature Offset` |
| 4753 | SV/EC | Shuttle1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Shuttle1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Shuttle1 Temperature Offset` |
| 4754 | SV/EC | Shuttle2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Shuttle2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Shuttle2 Temperature Offset` |
| 4755 | SV/EC | Head1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Head1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Head1 Temperature Offset` |
| 4756 | SV/EC | Head2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Head2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Head2 Temperature Offset` |
| 4757 | SV/EC | Head5 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Head5 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Head5 Temperature Offset` |
| 4758 | SV/EC | Head6 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Head6 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Head6 Temperature Offset` |
| 4759 | SV/EC | Dut Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Dut Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Dut Temperature Offset` |
| 4760 | SV/EC | Dut1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Dut1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Dut1 Temperature Offset` |
| 4761 | SV/EC | Hot Gun1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Hot Gun1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Hot Gun1 Temperature Offset` |
| 4762 | SV/EC | Hot Gun2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Hot Gun2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Hot Gun2 Temperature Offset` |
| 4763 | SV/EC | Chamber Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Chamber Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Chamber Temperature Offset` |
| 4764 | SV/EC | CCD Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | CCD Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`CCD Temperature Offset` |
| 4765 | SV/EC | DUT3 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | DUT3 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`DUT3 Temperature Offset` |
| 4766 | SV/EC | DUT4 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | DUT4 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`DUT4 Temperature Offset` |
| 4767 | SV/EC | Socket Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Socket Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Socket Temperature Offset` |
| 4770 | SV/EC | ATC Head1 Tempearture Offset | FT_8 | Celsius |  |  |  | ATC Head1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`ATC Head1 Temperature Offset` |
| 4771 | SV/EC | ATC Head2 Tempearture Offset | FT_8 | Celsius |  |  |  | ATC Head2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`ATC Head2 Temperature Offset` |
| 4772 | SV/EC | ATC Head3 Tempearture Offset | FT_8 | Celsius |  |  |  | ATC Head3 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`ATC Head3 Temperature Offset` |
| 4773 | SV/EC | ATC Head4 Tempearture Offset | FT_8 | Celsius |  |  |  | ATC Head4 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`ATC Head4 Temperature Offset` |
| 4788 | SV/EC | Active Heater Gun | BOOLEAN |  |  |  |  | Active Heater Gun | ✓ |  |
| 4789 | SV/EC | Active ATC Colling | BOOLEAN |  |  |  |  | Active ATC Colling | ✓ |  |
| 4790 | SV/EC | Chiller Temp | INT_4 |  | 30 | 15 | 25 | Chiller Temp | ✓ |  |
| 4791 | EC | ATC Use PID Offset | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4792 | EC | ATC PID Kp min Offset | FT_8 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4793 | EC | ATC PID Ki min Offset | FT_8 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4794 | EC | ATC PID Kd min Offset | FT_8 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4795 | EC | ATC PID Kp max Offset | FT_8 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4796 | EC | ATC PID Ki max Offset | FT_8 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4797 | EC | ATC PID Kd max Offset | FT_8 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 4801 | SV/EC | Aa1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Aa1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Aa1 Temperature Offset` |
| 4802 | SV/EC | Ab1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Ab1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Ab1 Temperature Offset` |
| 4803 | SV/EC | Ac1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Ac1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Ac1 Temperature Offset` |
| 4804 | SV/EC | Ad1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Ad1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Ad1 Temperature Offset` |
| 4805 | SV/EC | Ba1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Ba1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Ba1 Temperature Offset` |
| 4806 | SV/EC | Bb1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Bb1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Bb1 Temperature Offset` |
| 4807 | SV/EC | Bc1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Bc1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Bc1 Temperature Offset` |
| 4808 | SV/EC | Bd1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Bd1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Bd1 Temperature Offset` |
| 4809 | SV/EC | Aa2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Aa2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Aa2 Temperature Offset` |
| 4810 | SV/EC | Ab2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Ab2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Ab2 Temperature Offset` |
| 4811 | SV/EC | Ac2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Ac2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Ac2 Temperature Offset` |
| 4812 | SV/EC | Ad2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Ad2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Ad2 Temperature Offset` |
| 4813 | SV/EC | Ba2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Ba2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Ba2 Temperature Offset` |
| 4814 | SV/EC | Bb2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Bb2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Bb2 Temperature Offset` |
| 4815 | SV/EC | Bc2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Bc2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Bc2 Temperature Offset` |
| 4816 | SV/EC | Bd2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Bd2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Bd2 Temperature Offset` |
| 4817 | SV/EC | Ae1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Ae1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Ae1 Temperature Offset` |
| 4818 | SV/EC | Af1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Af1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Af1 Temperature Offset` |
| 4819 | SV/EC | Ag1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Ag1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Ag1 Temperature Offset` |
| 4820 | SV/EC | Ah1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Ah1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Ah1 Temperature Offset` |
| 4821 | SV/EC | Be1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Be1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Be1 Temperature Offset` |
| 4822 | SV/EC | Bf1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Bf1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Bf1 Temperature Offset` |
| 4823 | SV/EC | Bg1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Bg1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Bg1 Temperature Offset` |
| 4824 | SV/EC | Bh1 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Bh1 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Bh1 Temperature Offset` |
| 4825 | SV/EC | Ae2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Ae2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Ae2 Temperature Offset` |
| 4826 | SV/EC | Af2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Af2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Af2 Temperature Offset` |
| 4827 | SV/EC | Ag2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Ag2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Ag2 Temperature Offset` |
| 4828 | SV/EC | Ah2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Ah2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Ah2 Temperature Offset` |
| 4829 | SV/EC | Be2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Be2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Be2 Temperature Offset` |
| 4830 | SV/EC | Bf2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Bf2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Bf2 Temperature Offset` |
| 4831 | SV/EC | Bg2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Bg2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Bg2 Temperature Offset` |
| 4832 | SV/EC | Bh2 Tempearture Offset | FT_8 | Celsius | 60 | -60 | 0 | Bh2 Tempearture Offset | ✓ | ⚠️ EC名稱: Code=`Bh2 Temperature Offset` |
| 4850 | SV | Use pre-offset when change work temp | BOOLEAN |  |  |  |  | Use pre-offset when change work temp |  | 🔴 文件獨有 |
| 4851 | SV | Use Test complete, awaiting temperature | BOOLEAN |  |  |  |  | Use Test complete, awaiting temperature |  | 🔴 文件獨有 |
| 4852 | SV | Use Test time below Setting, Next Contact Need Delay. | BOOLEAN |  |  |  |  | Use Test time below Setting, Next Contact Need Delay. |  | 🔴 文件獨有 |
| 4853 | SV | Below Test Time Setting | FT_8 |  |  |  |  | Below Test Time Setting |  | 🔴 文件獨有 |
| 4854 | SV | Next Contact Need Delay Time Setting | FT_8 |  |  |  |  | Next Contact Need Delay Time Setting |  | 🔴 文件獨有 |
| 4880 | SV/EC | Temp Low OffSet | ASCII |  |  |  |  | Temp Low OffSet |  | 🔴 文件獨有 |
| 4881 | SV/EC | Temp Mid OffSet | ASCII |  |  |  |  | Temp Mid OffSet |  | 🔴 文件獨有 |
| 4882 | SV/EC | Temp High OffSet | ASCII |  |  |  |  | Temp High OffSet |  | 🔴 文件獨有 |
| 4883 | SV/EC | Temp User OffSet | ASCII |  |  |  |  | Temp User OffSet |  | 🔴 文件獨有 |
| 4884 | SV/EC | Temp Single Limit | ASCII |  |  |  |  | Temp Single Limit |  | 🔴 文件獨有 |
| 4885 | SV/EC | Temp HotLow OffSet | ASCII |  |  |  |  | Temp HotLow OffSet |  | 🔴 文件獨有 |
| 4886 | SV/EC | Temp HotMid OffSet | ASCII |  |  |  |  | Temp HotMid OffSet |  | 🔴 文件獨有 |
| 4887 | SV/EC | Temp Init OffSet | ASCII |  |  |  |  | Temp Init OffSet |  | 🔴 文件獨有 |
| 4888 | SV/EC | Temp EOT OffSet | ASCII |  |  |  |  | Temp EOT OffSet |  | 🔴 文件獨有 |
| 4900 | SV/EC | Temperature Mode | INT_4 |  | 2 | 0 | 2 | 0: High;  1: Ambient;  2: Ambient/High; | ✓ |  |
| 4901 | SV/EC | Jam Soak Time | FT_8 | Second |  |  |  | Jam Soak Time | ✓ |  |
| 4902 | SV/EC | Initial Wait Time | FT_8 | Second |  |  |  | Initial Wait Time | ✓ |  |
| 4903 | SV/EC | Initial Start 1 Time | FT_8 | Second |  |  |  | Initial Start 1 Time | ✓ | ⚠️ EC型別: Code=INT_4 |
| 4904 | SV/EC | Shuttle Soak Time | FT_8 | Second |  |  |  | Shuttle Soak Time | ✓ | ⚠️ EC型別: Code=INT_4 |
| 4905 | SV/EC | Chamber coolingTemp | FT_8 | Celsius |  |  |  | Chamber coolingTemp | ✓ |  |
| 4906 | SV/EC | Index Soak Time | FT_8 | Second |  |  |  | Index Soak Time | ✓ | ⚠️ EC型別: Code=INT_4 |
| 4907 | SV/EC | O/S Time Soak Time | FT_8 | Second |  |  |  | O/S Time Soak Time | ✓ | ⚠️ EC型別: Code=INT_4 |
| 4908 | SV/EC | Ambient Check | BOOLEAN |  |  |  |  | Ambient Check | ✓ |  |
| 4909 | SV/EC | Ambient Check Timing | INT_4 |  | 1 | 0 | 0 | 0: Start;  1: Running; | ✓ |  |
| 4910 | SV/EC | Ambient Temp | FT_8 | Celsius | 60 | -60 | 0 | Ambient Temp. | ✓ |  |
| 4911 | SV/EC | Using chamber fan in ambient mode | BOOLEAN |  |  |  |  | Using chamber fan in ambient mode | ✓ |  |
| 4912 | SV/EC | Initial Wait Time in Ambient | FT_8 | Second |  |  |  | Initial Wait Time in Ambient | ✓ |  |
| 4913 | SV/EC | Cooling Time in Ambient | FT_8 | Second |  |  |  | Cooling Time in Ambient | ✓ |  |
| 4914 | SV/EC | Index Heat Mode | INT_4 |  | 5 | 0 | 0 | 0: HeadOnly;  1: ChamberOnly;  2: Head+Chamber;  3: Socket+Chamber;  4: Head+Socket;  5: Head+Chamber+Socket; | ✓ |  |
| 4915 | SV/EC | Enable L/B temp monitoring function | BOOLEAN |  |  |  |  | Enable L/B temp monitoring function | ✓ |  |
| 4916 | SV/EC | Precondition LB Temperature Min | FT_8 |  |  |  |  | Precondition LB Temperature Min | ✓ |  |
| 4917 | SV/EC | Precondition LB boost temperature offset | FT_8 |  |  |  |  | Precondition LB boost temperature offset | ✓ |  |
| 4918 | SV/EC | Enable boost offset | BOOLEAN |  |  |  |  | Enable boost offset | ✓ |  |
| 4919 | SV/EC | LB Temperature Min | FT_8 |  |  |  |  | LB Temperature Min | ✓ |  |
| 4920 | SV/EC | LB boost temperature offset | FT_8 |  |  |  |  | LB boost temperature offset | ✓ |  |
| 4921 | SV/EC | Enable initial temperature offset | BOOLEAN |  |  |  |  | Enable initial temperature offset | ✓ |  |
| 4922 | SV/EC | Idle time | FT_8 |  |  |  |  | Idle time | ✓ |  |
| 4923 | SV/EC | Idle time LB boost temperature offset | FT_8 |  |  |  |  | Idle time LB boost temperature offset | ✓ |  |
| 4924 | SV/EC | Idle time LB boost duration | FT_8 |  |  |  |  | Idle time LB boost duration | ✓ |  |
| 4925 | SV/EC | Idle time LB post boost | FT_8 |  |  |  |  | Idle time LB post boost | ✓ |  |
| 4926 | SV/EC | Shuttle no need to heat up | BOOLEAN |  |  |  |  | Shuttle no need to heat up |  | 🔴 文件獨有 |

## BinCT 統計 (5000-9999)
> ⚠️ 此區段有 **17** 筆差異

| ID | 類型 | Name | DataType | Unit | Max | Min | Default | Description | Code | 差異 |
|----|------|------|----------|------|-----|-----|---------|-------------|------|------|
| 6001 | SV | Arm1 Total Contact Count | INT_4 |  |  |  |  | Arm1 Total Contact Count | ✓ |  |
| 6002 | SV | Arm2 Total Contact Count | INT_4 |  |  |  |  | Arm2 Total Contact Count | ✓ |  |
| 6501 | SV/EC | Temp. Use Single Limit | BOOLEAN |  | 1 | 0 | 0 | TRUE:Enalbe; FALSE:Disable | ✓ |  |
| 6551 | SV/EC | Plate1 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Plate1 Temperature Single Limit | ✓ |  |
| 6552 | SV/EC | Plate2 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Plate2 Temperature Single Limit | ✓ |  |
| 6553 | SV/EC | Shuttle1 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Shuttle1 Temperature Single Limit | ✓ |  |
| 6554 | SV/EC | Shuttle2 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Shuttle2 Temperature Single Limit | ✓ |  |
| 6555 | SV/EC | Head1 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Head1 Temperature Single Limit | ✓ |  |
| 6556 | SV/EC | Head2 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Head2 Temperature Single Limit | ✓ |  |
| 6557 | SV/EC | Head5 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Head5 Temperature Single Limit | ✓ |  |
| 6558 | SV/EC | Head6 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Head6 Temperature Single Limit | ✓ |  |
| 6559 | SV/EC | DUT1 Temp. Limit | FT_8 |  | 60 | -60 | 0 | DUT1 Temperature Single Limit | ✓ |  |
| 6560 | SV/EC | DUT2 Temp. Limit | FT_8 |  | 60 | -60 | 0 | DUT2 Temperature Single Limit | ✓ |  |
| 6561 | SV/EC | Chamber Temp. Limit | FT_8 |  | 60 | -60 | 0 | Chamber Temp. Limit | ✓ |  |
| 6562 | SV/EC | CCD Temp. Limit | FT_8 |  | 60 | -60 | 0 | CCD Temp. Limit | ✓ |  |
| 6563 | SV/EC | Aa 1 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Aa 1 Temp. Limit | ✓ |  |
| 6564 | SV/EC | Ab 1 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Ab 1 Temp. Limit | ✓ |  |
| 6565 | SV/EC | Ac 1 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Ac 1 Temp. Limit | ✓ |  |
| 6566 | SV/EC | Ad 1 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Ad 1 Temp. Limit | ✓ |  |
| 6567 | SV/EC | Ba 1 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Ba 1 Temp. Limit | ✓ |  |
| 6568 | SV/EC | Bb 1 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Bb 1 Temp. Limit | ✓ |  |
| 6569 | SV/EC | Bc 1 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Bc 1 Temp. Limit | ✓ |  |
| 6570 | SV/EC | Bd 1 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Bd 1 Temp. Limit | ✓ |  |
| 6571 | SV/EC | Aa 2 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Aa 2 Temp. Limit | ✓ |  |
| 6572 | SV/EC | Ab 2 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Ab 2 Temp. Limit | ✓ |  |
| 6573 | SV/EC | Ac 2 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Ac 2 Temp. Limit | ✓ |  |
| 6574 | SV/EC | Ad 2 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Ad 2 Temp. Limit | ✓ |  |
| 6575 | SV/EC | Ba 2 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Ba 2 Temp. Limit | ✓ |  |
| 6576 | SV/EC | Bb 2 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Bb 2 Temp. Limit | ✓ |  |
| 6577 | SV/EC | Bc 2 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Bc 2 Temp. Limit | ✓ |  |
| 6578 | SV/EC | Bd 2 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Bd 2 Temp. Limit | ✓ |  |
| 6579 | SV/EC | Heat Gun 1 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Heat Gun 1 Temp. Limit | ✓ |  |
| 6580 | SV/EC | Heat Gun 2 Temp. Limit | FT_8 |  | 60 | -60 | 0 | Heat Gun 2 Temp. Limit | ✓ |  |
| 6581 | SV/EC | DUT 3 Temp. Limit | FT_8 |  | 60 | -60 | 0 | DUT 3 Temp. Limit | ✓ |  |
| 6582 | SV/EC | DUT 4 Temp. Limit | FT_8 |  | 60 | -60 | 0 | DUT 4 Temp. Limit | ✓ |  |
| 6583 | SV/EC | Socket Temp. Limit | FT_8 |  | 60 | -60 | 0 | Socket Temp. Limit | ✓ |  |
| 6901 | SV/EC | Tower Light Running Green | INT_4 |  | 2 | 0 | 0 | Tower Light Running Green | ✓ |  |
| 6902 | SV/EC | Tower Light Running Yellow | INT_4 |  | 2 | 0 | 0 | Tower Light Running Yellow | ✓ |  |
| 6903 | SV/EC | Tower Light Running Red | INT_4 |  | 2 | 0 | 0 | Tower Light Running Red | ✓ |  |
| 6904 | SV/EC | Tower Light Error/Jam Green | INT_4 |  | 2 | 0 | 0 | Tower Light Error/Jam Green | ✓ |  |
| 6905 | SV/EC | Tower Light Error/Jam Yellow | INT_4 |  | 2 | 0 | 0 | Tower Light Error/Jam Yellow | ✓ |  |
| 6906 | SV/EC | Tower Light Error/Jam Red | INT_4 |  | 2 | 0 | 0 | Tower Light Error/Jam Red | ✓ |  |
| 6907 | SV/EC | Tower Light Pause Green | INT_4 |  | 2 | 0 | 0 | Tower Light Pause Green | ✓ |  |
| 6908 | SV/EC | Tower Light Pause Yellow | INT_4 |  | 2 | 0 | 0 | Tower Light Pause Yellow | ✓ |  |
| 6909 | SV/EC | Tower Light Pause Red | INT_4 |  | 2 | 0 | 0 | Tower Light Pause Red | ✓ |  |
| 6910 | SV/EC | Tower Light Message Green | INT_4 |  | 2 | 0 | 0 | Tower Light Message Green | ✓ |  |
| 6911 | SV/EC | Tower Light Message Yellow | INT_4 |  | 2 | 0 | 0 | Tower Light Message Yellow | ✓ |  |
| 6912 | SV/EC | Tower Light Message Red | INT_4 |  | 2 | 0 | 0 | Tower Light Message Red | ✓ |  |
| 6913 | SV/EC | Tower Light Heating Green | INT_4 |  | 2 | 0 | 0 | Tower Light Heating Green | ✓ |  |
| 6914 | SV/EC | Tower Light Heating Yellow | INT_4 |  | 2 | 0 | 0 | Tower Light Heating Yellow | ✓ |  |
| 6915 | SV/EC | Tower Light Heating Red | INT_4 |  | 2 | 0 | 0 | Tower Light Heating Red | ✓ |  |
| 6916 | SV/EC | Tower Light Homing Green | INT_4 |  | 2 | 0 | 0 | Tower Light Homing Green | ✓ |  |
| 6917 | SV/EC | Tower Light Homing Yellow | INT_4 |  | 2 | 0 | 0 | Tower Light Homing Yellow | ✓ |  |
| 6918 | SV/EC | Tower Light Homing Red | INT_4 |  | 2 | 0 | 0 | Tower Light Homing Red | ✓ |  |
| 6921 | SV/EC | Music Select Running | INT_4 |  | 4 | 0 | 0 | 0: Silent;  1: Music1;  2: Music2;  3: Music3;  4: Music4; | ✓ |  |
| 6922 | SV/EC | Music Select Error/Jam | INT_4 |  | 4 | 0 | 0 | 0: Silent;  1: Music1;  2: Music2;  3: Music3;  4: Music5; | ✓ |  |
| 6923 | SV/EC | Music Select Pause | INT_4 |  | 4 | 0 | 0 | 0: Silent;  1: Music1;  2: Music2;  3: Music3;  4: Music6; | ✓ |  |
| 6924 | SV/EC | Music Select Message | INT_4 |  | 4 | 0 | 0 | 0: Silent;  1: Music1;  2: Music2;  3: Music3;  4: Music7; | ✓ |  |
| 6925 | SV/EC | Music Select Heating | INT_4 |  | 4 | 0 | 0 | 0: Silent;  1: Music1;  2: Music2;  3: Music3;  4: Music8; | ✓ |  |
| 6926 | SV/EC | Music Select Homing | INT_4 |  | 4 | 0 | 0 | 0: Silent;  1: Music1;  2: Music2;  3: Music3;  4: Music9; | ✓ |  |
| 6927 | SV/EC | Music Select Off-Line | INT_4 |  | 4 | 0 | 0 | 0: Silent;  1: Music1;  2: Music2;  3: Music3;  4: Music10; | ✓ |  |
| 6928 | SV/EC | Music Select Auto Retest | INT_4 |  | 4 | 0 | 0 | 0: Silent;  1: Music1;  2: Music2;  3: Music3;  4: Music11; | ✓ |  |
| 6930 | SV/EC | Tower Light Off-Line Green | INT_4 |  | 2 | 0 | 0 | Tower Light Homing Red | ✓ |  |
| 6931 | SV/EC | Tower Light Off-Line Yellow | INT_4 |  | 2 | 0 | 0 | Tower Light Homing Red | ✓ |  |
| 6932 | SV/EC | Tower Light Off-Line Red | INT_4 |  | 2 | 0 | 0 | Tower Light Homing Red | ✓ |  |
| 6933 | SV/EC | Tower Light Auto Retest Green | INT_4 |  | 2 | 0 | 0 | Tower Light Homing Red | ✓ |  |
| 6934 | SV/EC | Tower Light Auto Retest Yellow | INT_4 |  | 2 | 0 | 0 | Tower Light Homing Red | ✓ |  |
| 6935 | SV/EC | Tower Light Auto Retest Red | INT_4 |  | 2 | 0 | 0 | Tower Light Homing Red | ✓ |  |
| 7001 | SV/EC | LevelSet 9045 [00] Main - Tools | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7002 | SV/EC | LevelSet 9045 [01] Main - Config | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7003 | SV/EC | LevelSet 9045 [02] Main - Offset | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7004 | SV/EC | LevelSet 9045 [03] Main - Speed | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7005 | SV/EC | LevelSet 9045 [04] Main - IO | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7006 | SV/EC | LevelSet 9045 [05] Main - Message | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7007 | SV/EC | LevelSet 9045 [06] Main - Exit | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7008 | SV/EC | LevelSet 9045 [07] Main - Hot Mode | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7009 | SV/EC | LevelSet 9045 [08] Main - Tester On/Off line | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7010 | EC | LevelSet 9045 [09] Main -Setup File | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 7011 | SV/EC | LevelSet 9045 [10] Main - Site Select | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7012 | SV/EC | LevelSet 9045 [11] Main - Real/Dummy | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7013 | SV/EC | LevelSet 9045 [12] Main - Start Mode | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7015 | SV/EC | LevelSet 9045 [14] Tools - Tray Form | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7016 | SV/EC | LevelSet 9045 [15] Tools - Plate Form | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7017 | SV/EC | LevelSet 9045 [16] Tools - Tray Assign | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7018 | SV/EC | LevelSet 9045 [17] Tools - Temp.OffSet | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7019 | SV/EC | LevelSet 9045 [18] Tools - Contact | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7020 | SV/EC | LevelSet 9045 [19] Tools - Test IF | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7021 | SV/EC | LevelSet 9045 [20] Tools - Bin | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7022 | SV/EC | LevelSet 9045 [21] Tools - Set Up | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7023 | SV/EC | LevelSet 9045 [22] Tools - Load/Unld | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7024 | SV/EC | LevelSet 9045 [23] Config - Builder | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7025 | SV/EC | LevelSet 9045 [24] Config - Start Mode | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7026 | SV/EC | LevelSet 9045 [25] Config - C.Select | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7027 | SV/EC | LevelSet 9045 [26] Config - C.Clear | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7029 | SV/EC | LevelSet 9045 [28] Config - Tower Light | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7030 | SV/EC | LevelSet 9045 [29] Config - Password | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7031 | SV/EC | LevelSet 9045 [30] Config - Configuration | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7032 | SV/EC | LevelSet 9045 [31] Config - DIO Setting | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7033 | SV/EC | LevelSet 9045 [32] Main - Temperature Deg Setup | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7036 | SV/EC | LevelSet 9045 [35] Alarm - Trouble Shooting | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7038 | SV/EC | LevelSet 9045 [37] Tools - CCD | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7039 | SV/EC | LevelSet 9045 [38] Contact - Contact Force | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7040 | SV/EC | LevelSet 9045 [39] Tools - Yield Monitoring | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7041 | SV/EC | LevelSet 9045 [40] Setup - Shuttle Mode | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7042 | SV/EC | LevelSet 9045 [41] Main - Change SetUp File | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7043 | SV/EC | LevelSet 9045 [42] Config - SECS\GEM | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7044 | SV/EC | LevelSet 9045 [43] Tools - Auto Clean | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7045 | SV/EC | LevelSet 9045 [44] Tools - ATC | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7046 | SV/EC | LevelSet 9045 [45] Tools - Sensor Adj. | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7047 | SV/EC | LevelSet 9045 [46] Config - Sensor Latch | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7048 | SV/EC | LevelSet 9045 [47] Config - Omron Temp. | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7049 | SV/EC | LevelSet 9045 [48] Tools - OCR | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7050 | SV/EC | LevelSet 9045 [49] Config - Auto Temp. | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7051 | SV/EC | LevelSet 9045 [50] Temp.OffSet - Temp. Mode | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7052 | SV/EC | LevelSet 9045 [51] Temp.OffSet - Default Offset | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7053 | SV/EC | LevelSet 9045 [52] Temp.OffSet - Base Point | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7054 | SV/EC | LevelSet 9045 [53] Temp.OffSet - User Offser | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7055 | SV/EC | LevelSet 9045 [54] Temp.OffSet - Single Limits | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7056 | SV/EC | LevelSet 9045 [55] Temp.OffSet - Hot Mode | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7057 | SV/EC | LevelSet 9045 [56] Temp.OffSet - Amb Mode | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7058 | SV/EC | LevelSet 9045 [57] Temp.OffSet - Index Mode | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7059 | SV/EC | LevelSet 9045 [58] IO - STACK 1 | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7060 | SV/EC | LevelSet 9045 [59] IO - STACK 2 | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7061 | SV/EC | LevelSet 9045 [60] IO - SUCKER | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7062 | SV/EC | LevelSet 9045 [61] IO - SHUTTLE | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7063 | SV/EC | LevelSet 9045 [62] IO - KEY PAD | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7064 | SV/EC | LevelSet 9045 [63] IO - SYSTEM | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7065 | SV/EC | LevelSet 9045 [64] IO - INDEX | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7066 | SV/EC | LevelSet 9045 [65] IO - TTL | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7067 | SV/EC | LevelSet 9045 [66] IO - TOOLS | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7068 | SV/EC | LevelSet 9045 [67] Configure - A [Function] | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7069 | SV/EC | LevelSet 9045 [68] Configure - C [Hardware] | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7070 | SV/EC | LevelSet 9045 [69] Configure - D [Index] | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7071 | SV/EC | LevelSet 9045 [70] Configure - E [In/Out Arm] | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7072 | SV/EC | LevelSet 9045 [71] Configure - F [Shuttle] | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7073 | SV/EC | LevelSet 9045 [72] Configure - G [Visible] | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7074 | SV/EC | LevelSet 9045 [73] Configure - I [Tester] | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7075 | SV/EC | LevelSet 9045 [74] Configure - L [Temperature] | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7076 | SV/EC | LevelSet 9045 [75] Configure - O [Count] | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7077 | SV/EC | LevelSet 9045 [76] Configure - N [Network] | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7078 | SV/EC | LevelSet 9045 [77] Configure - P [Tray] | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7079 | SV/EC | LevelSet 9045 [78] Configure - Tray Data | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7080 | SV/EC | LevelSet 9045 [79] Configure - Hot Plate Data | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7081 | SV/EC | LevelSet 9045 [80] Tools - Dyna. Temperature | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7082 | SV/EC | LevelSet 9045 [81] Yield - Yield Alarm | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7083 | SV/EC | LevelSet 9045 [82] Yield - Piggy-Back Funtions | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7084 | SV/EC | LevelSet 9045 [83] Yield - Alarm | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7085 | SV/EC | LevelSet 9045 [84] Reserved | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech |  | 🔴 文件獨有 |
| 7086 | SV/EC | LevelSet 9045 [85] Main - QA Mode | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7087 | SV/EC | LevelSet 9045 [86] Config - Motion View | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7088 | SV/EC | LevelSet 9045 [87] Main  - Teaching | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7089 | SV/EC | LevelSet 9045 [88] Main  - BarCode | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7090 | SV/EC | LevelSet 9045 [89] Tools - Rotate | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7091 | SV/EC | LevelSet 9045 [90] Config - Air Con. | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7092 | SV/EC | LevelSet 9045 [91] Other - Test Time | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ | ⚠️ EC名稱: Code=`LevelSet 9045 [91] Tools - Test Time` |
| 7093 | SV/EC | LevelSet 9045 [92] Contact - Contact parameter | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7094 | SV/EC | LevelSet 9045 [93] Contact - Handler Mode | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7095 | SV/EC | LevelSet 9045 [94] Setup - Site Map | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7096 | SV/EC | LevelSet 9045 [95] Reserved | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech |  | 🔴 文件獨有 |
| 7097 | SV/EC | LevelSet 9045 [96] Configure - Monitor Function | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7098 | SV/EC | LevelSet 9045 [97] Other - Auto Clean Clear Count | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7099 | SV/EC | LevelSet 9045 [98] Other - QA Mode Device Count | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7100 | SV/EC | LevelSet 9045 [99] Tools - Socket | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7101 | SV/EC | LevelSet 9045 [100] Tools - Laser Sensor | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7102 | SV/EC | LevelSet 9045 [101] Contact - Height | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7103 | SV/EC | LevelSet 9045 [102] Other - Loader Tray Mode | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7104 | SV/EC | LevelSet 9045 [103] Contact - Test Socket IC check | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7105 | SV/EC | LevelSet 9045 [104] Other - Tray Edit | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ |  |
| 7106 | SV/EC | LevelSet 9045 [105] Main  - Big Fan | INT_4 |  | 3 | 0 | 0 | 0:Operator; 1:Engineer; 2: Supervisor; 3:Hontech | ✓ | ⚠️ EC名稱: Code=`LevelSet 9045 [105] Main - Big Fan` |
| 7107 | EC | LevelSet 9045 [106] Temp.OffSet - Initial Temp Offset | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 7108 | EC | LevelSet 9045 [107] Other - Yield Count Clean | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 7109 | EC | LevelSet 9045 [108] Other - Bin Clean Count | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 7110 | EC | LevelSet 9045 [109] Contact - Sensor Adjustment | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 8501 | SV/EC | In Arm Speed | INT_4 | Percentage | 100 | 1 | 80 | In Arm Speed | ✓ |  |
| 8502 | SV/EC | Shuttle Speed | INT_4 | Percentage | 100 | 1 | 80 | Shuttle Speed | ✓ |  |
| 8503 | SV/EC | Index Arm Speed | INT_4 | Percentage | 100 | 1 | 80 | Index Arm Speed | ✓ |  |
| 8504 | SV/EC | Out Arm Speed | INT_4 | Percentage | 100 | 1 | 80 | Out Arm Speed | ✓ |  |
| 8505 | SV/EC | Tray Arm Speed | INT_4 | Percentage | 100 | 1 | 80 | Tray Arm Speed | ✓ |  |
| 8507 | SV/EC | Shuttle 2 Speed | INT_4 | Percentage | 100 | 1 | 80 | Shuttle 2 Speed | ✓ |  |
| 8508 | SV/EC | In Rotate Speed | INT_4 | Percentage | 100 | 1 | 80 | In Rotate Speed | ✓ |  |
| 8509 | SV/EC | Out Rotate Speed | INT_4 | Percentage | 100 | 1 | 80 | Out Rotate Speed | ✓ |  |
| 8510 | SV/EC | In Arm Z Speed | INT_4 | Percentage | 100 | 1 | 80 | In Arm Z Speed | ✓ |  |
| 8511 | SV/EC | In Arm X Pitch Speed | INT_4 | Percentage | 100 | 1 | 80 | In Arm X Pitch Speed | ✓ |  |
| 8513 | SV/EC | Out Arm Z Speed | INT_4 | Percentage | 100 | 1 | 80 | Out Arm Z Speed | ✓ |  |
| 8514 | SV/EC | Out Arm X Pitch Speed | INT_4 | Percentage | 100 | 1 | 80 | Out Arm X Pitch Speed | ✓ |  |
| 8527 | SV/EC | Step Shuttle | INT_4 |  | 2 | 0 | 1 | 0: OFF;  1: ON; | ✓ |  |
| 8528 | SV/EC | Tray Arm Retry Count | INT_4 |  | 5 | 0 | 1 | Tray Arm Retry Count | ✓ |  |
| 8529 | SV/EC | Tray Arm Head Down Time | FT_8 | Second | 10 | 0.01 | 0.01 | Tray Arm Head Down Time | ✓ |  |
| 8530 | SV/EC | Auto Speed | BOOLEAN |  | 1 | 0 | 1 | 0: OFF;  1: ON; | ✓ |  |
| 8534 | SV/EC | Index Retry Count | INT_4 |  | 5 | 0 | 1 | Index Retry Count | ✓ |  |
| 8535 | SV/EC | Index Retry Down Distance | FT_8 | mm | -0.01 | -0.3 | -0.01 | Index Retry Down Distance | ✓ |  |
| 8536 | SV/EC | Index Destroy Again Interval Timre | FT_8 | Second | 10 | 0.01 | 0.01 | Index Destroy Again Interval Timre | ✓ |  |
| 8537 | SV/EC | Index Destroy Again Count | INT_4 |  | 5 | 0 | 0 | Index Destroy Again Count | ✓ |  |
| 8538 | SV/EC | Socket Device Floating Check | INT_4 |  | 2 | 0 | 0 | 0: OFF;  1: ON; | ✓ |  |
| 8540 | SV/EC | In Arm ADC Speed | INT_4 | Percentage | 100 | 1 | 80 | In Arm ADC Speed | ✓ |  |
| 8541 | SV/EC | Shuttle ADC Speed | INT_4 | Percentage | 100 | 1 | 80 | Shuttle ADC Speed | ✓ |  |
| 8542 | SV/EC | Index Arm ADC Speed | INT_4 | Percentage | 100 | 1 | 80 | Index Arm ADC Speed | ✓ |  |
| 8543 | SV/EC | Out Arm ADC Speed | INT_4 | Percentage | 100 | 1 | 80 | Out Arm ADC Speed | ✓ |  |
| 8544 | SV/EC | Tray Arm ADC Speed | INT_4 | Percentage | 100 | 1 | 80 | Tray Arm ADC Speed | ✓ |  |
| 8546 | SV/EC | Shuttle 2 ADC Speed | INT_4 | Percentage | 100 | 1 | 80 | Shuttle 2 ADC Speed | ✓ |  |
| 8547 | SV/EC | In Rotate ADC Speed | INT_4 | Percentage | 100 | 1 | 80 | In Rotate ADC Speed | ✓ |  |
| 8548 | SV/EC | Out Rotate ADC Speed | INT_4 | Percentage | 100 | 1 | 80 | Out Rotate ADC Speed | ✓ |  |
| 8549 | SV/EC | In Arm Z ADC Speed | INT_4 | Percentage | 100 | 1 | 80 | In Arm Z ADC Speed | ✓ |  |
| 8550 | SV/EC | In Arm X Pitch ADC Speed | INT_4 | Percentage | 100 | 1 | 80 | In Arm X Pitch ADC Speed | ✓ |  |
| 8552 | SV/EC | Out Arm Z ADC Speed | INT_4 | Percentage | 100 | 1 | 80 | Out Arm Z ADC Speed | ✓ |  |
| 8553 | SV/EC | Out Arm X Pitch ADC Speed | INT_4 | Percentage | 100 | 1 | 80 | Out Arm X Pitch ADC Speed | ✓ |  |
| 8560 | SV/EC | In Arm Retry Count | INT_4 |  | 5 | 0 | 1 | In Arm Retry Count | ✓ |  |
| 8561 | SV/EC | In Arm Retry Down Distance | FT_8 | mm | -0.01 | -0.3 | -0.01 | In Arm Retry Down Distance | ✓ |  |
| 8562 | SV/EC | In Arm Destroy Again Interval Timre | FT_8 | Second | 10 | 0.01 | 0.01 | In Arm Destroy Again Interval Timre | ✓ |  |
| 8563 | SV/EC | In Arm Destroy Again Count | INT_4 |  | 5 | 0 | 0 | In Arm Destroy Again Count | ✓ |  |
| 8564 | SV/EC | In Arm Pitch Function | INT_4 |  | 1 | 0 | 0 | 0: Open/Close;  1: Fixed | ✓ |  |
| 8565 | SV/EC | In Arm Destroy Check Time | FT_8 | Second | 1 | 0.1 | 0.1 | In Arm Destroy Check Time | ✓ |  |
| 8566 | SV/EC | In Arm Destroy Check Need Pause | BOOLEAN |  | 1 | 0 | 0 | In Arm Destroy Check Need Pause | ✓ |  |
| 8567 | SV/EC | In Arm Two Speed Move Down | INT_4 |  | 1 | 0 | 0 | In Arm Two Speed Move Down | ✓ |  |
| 8568 | SV/EC | In Arm Two Speed Move Down Speed | INT_4 | Percentage | 100 | 1 | 10 | In Arm Two Speed Move Down Speed | ✓ |  |
| 8569 | SV/EC | In Arm Two Speed Move Down ADC | INT_4 | Percentage | 100 | 1 | 10 | In Arm Two Speed Move Down ADC | ✓ |  |
| 8570 | SV/EC | In Arm Two Speed Move Down Distance | FT_8 | mm | 10 | 1 | 3 | In Arm Two Speed Move Down Distance | ✓ |  |
| 8571 | SV/EC | In Arm Auto Skip | INT_4 |  | 1 | 0 | 0 | 0: OFF;  1: ON; | ✓ |  |
| 8572 | SV/EC | In Arm Auto Skip Count | INT_4 |  | 50 | 10 | 10 | In Arm Auto Skip Count | ✓ |  |
| 8573 | SV/EC | In Arm Wait On Shuttle Time | FT_8 |  | 10 | 0.01 | 0.01 |  | ✓ |  |
| 8574 | SV/EC | Out Arm Retry Count | INT_4 |  |  |  |  | Out Arm Retry Count | ✓ |  |
| 8575 | SV/EC | Out Arm Retry Down Distance | FT_8 | mm |  |  |  | Out Arm Retry Down Distance | ✓ |  |
| 8576 | SV/EC | Out Arm Destroy Again Interval Timre | FT_8 | Second |  |  |  | Out Arm Destroy Again Interval Timre | ✓ |  |
| 8577 | SV/EC | Out Arm Destroy Again Count | INT_4 |  |  |  |  | Out Arm Destroy Again Count | ✓ |  |
| 8578 | SV/EC | Out Arm Pitch Function | INT_4 |  | 1 | 0 | 0 | 0: Open/Close;  1: Fixed | ✓ |  |
| 8579 | SV/EC | Out Arm Destroy Check Time | FT_8 | Second | 1 | 0.1 | 0.1 | Out Arm Destroy Check Time | ✓ |  |
| 8580 | SV/EC | Out Arm Destroy Check Need Pause | BOOLEAN |  | 1 | 0 | 0 | Out Arm Destroy Check Need Pause | ✓ |  |
| 8581 | SV/EC | Out Arm Two Speed Move Down | INT_4 |  | 1 | 0 | 0 | Out Arm Two Speed Move Down | ✓ |  |
| 8582 | SV/EC | Out Arm Two Speed Move Down Speed | INT_4 | Percentage | 100 | 1 | 10 | Out Arm Two Speed Move Down Speed | ✓ |  |
| 8583 | SV/EC | Out Arm Two Speed Move Down ADC | INT_4 | Percentage | 100 | 1 | 10 | Out Arm Two Speed Move Down ADC | ✓ |  |
| 8584 | SV/EC | Out Arm Two Speed Move Down Distance | FT_8 | mm | 10 | 1 | 3 | Out Arm Two Speed Move Down Distance | ✓ |  |
| 9001 | SV | Auto Clean Cleaning Count | INT_4 |  |  |  |  | Auto Clean Cleaning Count | ✓ |  |
| 9003 | SV | Auto Clean Arm Total Force(KG) | ASCII | KG |  |  |  | Auto Clean Arm Total Force(KG) | ✓ |  |
| 9004 | SV | Auto Clean Arm Total Force(N) | ASCII | N |  |  |  | Auto Clean Arm Total Force(N) | ✓ |  |
| 9501 | SV/EC | Auto Clean Function | INT_4 |  | 1 | 0 | 0 | 0:Close; 1:Open | ✓ |  |
| 9502 | SV | Auto Clean Trigger Condition | INT_4 |  | 7 | 0 | 0 | 0: Initial/Finish 1: Start Auto Cleaning By Initial Start 2: Start Auto Cleaning By Inital Retest Start 3: Start Auto Cleaning By Interval 4: Start Auto Cleaning By Manual 5: Start Auto Cleaning By Socket Alarm 6: Start Auto Cleaning By Clean Out Finish 7: Start Auto Cleaning By SECS GEM Command |  | 🔴 文件獨有 |
| 9511 | SV/EC | Auto Clean Initial Start Mode | BOOLEAN |  |  |  |  | Auto Clean Initial Start Mode | ✓ |  |
| 9512 | SV/EC | Auto Clean Initial Retest Start Mode | BOOLEAN |  |  |  |  | Auto Clean Initial Retest Start Mode | ✓ |  |
| 9513 | SV/EC | Auto Clean Finish Mode | BOOLEAN |  |  |  |  | Auto Clean Finish Mode | ✓ |  |
| 9514 | SV/EC | Auto Clean Manual Mode | BOOLEAN |  |  |  |  | Auto Clean Manual  Mode | ✓ |  |
| 9515 | SV/EC | Auto Clean Socket Alarm Fail Mode | BOOLEAN |  |  |  |  | Auto Clean Socket Alarm Fail Mode | ✓ |  |
| 9516 | SV/EC | Auto Clean Interval Contact Mode | BOOLEAN |  |  |  |  | Auto Clean Interval Contact Mode | ✓ |  |
| 9517 | SV/EC | Auto Clean Interval Contact Count | INT_4 |  |  |  |  | Auto Clean Interval Contact Count | ✓ |  |
| 9518 | SV | Auto Clean Mode | INT_4 |  |  |  |  | Auto Clean Mode |  | 🔴 文件獨有 |
| 9521 | SV/EC | Auto Clean Device Number of pices | INT_4 |  |  |  |  | Auto Clean Device Number of pices | ✓ |  |
| 9523 | SV/EC | Auto Clean Alarm Count | INT_4 |  |  |  |  | Auto Clean Alarm Count | ✓ |  |
| 9524 | SV/EC | Auto Clean Pad Deviation | INT_4 |  |  |  |  | Auto Clean Pad Deviation |  | 🔴 文件獨有 |
| 9525 | SV/EC | Auto Clean Shuttle Detect | INT_4 |  | 1 | 0 | 0 | 0: OFF;  1: ON; | ✓ |  |
| 9526 | SV/EC | Auto Clean Device Position | INT_4 |  | 4 | 1 | 2 | 1: Hot Plate 2;  2: Clean Kit;  3: Clean Air; | ✓ |  |
| 9531 | SV/EC | Auto Clean Contact Mode | INT_4 |  |  |  |  | Auto Clean Contact Mode | ✓ |  |
| 9532 | SV/EC | Auto Clean Contact Time | FT_8 | Second |  |  |  | Auto Clean Contact Time(Second) | ✓ |  |
| 9533 | SV/EC | Auto Clean Contact Count | INT_4 |  |  |  |  | Auto Clean Contact Count | ✓ |  |
| 9534 | SV/EC | Auto Clean Pins Count | INT_4 |  |  |  |  | Auto Clean Pins Count | ✓ |  |
| 9535 | SV/EC | Auto Clean Force per Pin(N) | FT_8 | N |  |  |  | Auto Clean Force per Pin(N) | ✓ | ⚠️ EC型別: Code=FT_4 |
| 9536 | SV/EC | Auto Clean Force per Pin(gf) | FT_8 | G |  |  |  | Auto Clean Force per Pin(gf) | ✓ | ⚠️ EC型別: Code=FT_4 |
| 9537 | SV/EC | Auto Clean Drop High | INT_4 | mm |  |  |  | Auto Clean Drop High | ✓ |  |
| 9538 | SV/EC | Auto Clean Inital Contact Count | INT_4 |  |  |  |  | Auto Clean Inital Contact Count |  | 🔴 文件獨有 |
| 9541 | SV/EC | Auto Clean In Arm Speed | INT_4 | Percentage | 100 | 1 | 10 | Auto Clean In Arm Speed(%) | ✓ |  |
| 9543 | SV/EC | Auto Clean Shuttle Speed | INT_4 | Percentage | 100 | 1 | 10 | Auto Clean Shuttle Speed(%) | ✓ |  |
| 9544 | SV/EC | Auto Clean Index Arm Speed | INT_4 | Percentage | 100 | 1 | 10 | Auto Clean Index Arm Speed(%) | ✓ |  |
| 9545 | SV/EC | Auto Clean In Arm Z Speed | INT_4 | Percentage | 100 | 1 | 10 | Auto Clean In Arm Z Speed(%) | ✓ |  |
| 9546 | SV/EC | Auto Clean Force per Pin(N) | FT_4 | N |  |  |  | Auto Clean Force per Pin(N) |  | 🔴 文件獨有 |
| 9547 | SV/EC | Auto Clean Force per Pin(gf) | FT_4 | G |  |  |  | Auto Clean Force per Pin(gf) |  | 🔴 文件獨有 |
| 9552 | SV/EC | Auto Clean Form X Pitch | FT_8 | mm |  |  |  | Auto Clean Form X Pitch | ✓ |  |
| 9553 | SV/EC | Auto Clean Form Y Pitch | FT_8 | mm |  |  |  | Auto Clean Form Y Pitch | ✓ |  |
| 9554 | SV/EC | Auto Clean Form X Start | FT_8 | mm |  |  |  | Auto Clean Form X Start | ✓ |  |
| 9555 | SV/EC | Auto Clean Form Y Start | FT_8 | mm |  |  |  | Auto Clean Form Y Start | ✓ |  |
| 9556 | SV/EC | Auto Clean Form X Division | INT_4 |  |  |  |  | Auto Clean Form X Division | ✓ |  |
| 9557 | SV/EC | Auto Clean Form Y Division | INT_4 |  |  |  |  | Auto Clean Form Y Division | ✓ |  |
| 9558 | SV/EC | Auto Clean Kit Mode for HT9045 | INT_4 |  | 1 | 0 | 0 | 0: Kit;  1: Tray; | ✓ |  |
| 9559 | SV/EC | Auto Clean Use Index Arm 2 | BOOLEAN |  | 1 | 0 | 0 | Auto Clean Use Index Arm 2 | ✓ |  |
| 9560 | SV/EC | Auto Clean Use Index Mode | INT_4 |  | 2 | 0 | 0 | 0: Arm1 Only;  1: Arm2 Only;  2: Arm1 & Arm2; | ✓ |  |
| 9561 | SV/EC | Auto Clean Low Yield Alarm | BOOLEAN |  | 1 | 0 | 0 | Auto Clean Low Yield Alarm | ✓ |  |
| 9562 | SV/EC | Auto Clean Low Yield Limit | INT_4 | Percentage | 100 | 0 | 0 | Auto Clean Low Yield Limit | ✓ |  |
| 9563 | SV/EC | Auto Clean Low Yield Ignore Count | INT_4 |  | 100000 | 1 | 100 | Auto Clean Low Yield Ignore Count | ✓ |  |
| 9564 | SV/EC | Auto Clean Site Yield Alarm | BOOLEAN |  | 1 | 0 | 0 | Auto Clean Site Yield Alarm | ✓ |  |
| 9565 | SV/EC | Auto Clean Site Yield Limit | INT_4 | Percentage | 100 | 0 | 0 | Auto Clean Site Yield Limit | ✓ |  |
| 9566 | SV/EC | Auto Clean Site Yield Ignore Count | INT_4 |  | 100000 | 1 | 100 | Auto Clean Site Yield Ignore Count | ✓ |  |
| 9567 | SV/EC | Auto Clean Consecutive Fail by Socket for FT | BOOLEAN |  | 1 | 0 | 0 | Auto Clean Consecutive Fail by Socket for FT | ✓ |  |
| 9568 | SV/EC | Auto Clean Consecutive Fail by Socket Count for FT | INT_4 |  | 100000 | 1 | 100 | Auto Clean Consecutive Fail by Socket Count for FT | ✓ |  |
| 9569 | SV/EC | Auto Clean Consecutive Fail by Socket for RT | BOOLEAN |  | 1 | 0 | 0 | Auto Clean Consecutive Fail by Socket for RT | ✓ |  |
| 9570 | SV/EC | Auto Clean Consecutive Fail by Socket Count for RT | INT_4 |  | 100000 | 1 | 100 | Auto Clean Consecutive Fail by Socket Count for RT | ✓ |  |
| 9571 | SV/EC | Auto Clean Consecutive Fail by Head for FT | BOOLEAN |  | 1 | 0 | 0 | Auto Clean Consecutive Fail by Head for FT | ✓ |  |
| 9572 | SV/EC | Auto Clean Consecutive Fail by Head Count for FT | INT_4 |  | 100000 | 1 | 100 | Auto Clean Consecutive Fail by Head Count for FT | ✓ |  |
| 9573 | SV/EC | Auto Clean Consecutive Fail by Head for RT | BOOLEAN |  | 1 | 0 | 0 | Auto Clean Consecutive Fail by Head for RT | ✓ |  |
| 9574 | SV/EC | Auto Clean Consecutive Fail by Head Count for RT | INT_4 |  | 100000 | 1 | 100 | Auto Clean Consecutive Fail by Head Count for RT | ✓ |  |
| 9601 | SV/EC | Auto Clean Contact Shift Height | FT_8 | mm |  |  |  | Auto Clean Contact Shift Height | ✓ |  |
| 9602 | SV/EC | Auto Clean Contact Height Offset | FT_8 | mm |  |  |  | Auto Clean Contact Height Offset | ✓ |  |
| 9671 | SV/EC | Auto Clean In Shuttle 2 X Offset | FT_8 | mm |  |  |  | Auto Clean In Shuttle 2 X Offset | ✓ |  |
| 9672 | SV/EC | Auto Clean In Shuttle 2 Y Offset | FT_8 | mm |  |  |  | Auto Clean In Shuttle 2 Y Offset | ✓ |  |
| 9689 | SV/EC | Auto Clean In Shuttle Pick Offset | FT_8 | mm |  |  |  | Auto Clean In Shuttle Pick Offset | ✓ |  |
| 9690 | SV/EC | Auto Clean In Shuttle Place Offset | FT_8 | mm |  |  |  | Auto Clean In Shuttle Place Offset | ✓ |  |

## BinCT 分站 (10000-14999)
> ⚠️ 此區段有 **77** 筆差異

| ID | 類型 | Name | DataType | Unit | Max | Min | Default | Description | Code | 差異 |
|----|------|------|----------|------|-----|-----|---------|-------------|------|------|
| 10000 | SV/EC | ART Max Count | INT_4 |  |  |  |  | Maximum count for doing ART | ✓ |  |
| 10001 | SV/EC | ART Fail Yield Rate | INT_4 |  | 100 | 1 |  | Yield for doing ART | ✓ |  |
| 10609 | SV/EC | Alarm1 Normal Bin 0 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 Normal Bin 0 Yield over Limit Enable | ✓ |  |
| 10610 | SV/EC | Alarm1 Normal Bin 0 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 Normal Bin 0 Yield over Limit (%) | ✓ |  |
| 10611 | SV/EC | Alarm1 Normal Bin 1 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 Normal Bin 1 Yield over Limit Enable | ✓ |  |
| 10612 | SV/EC | Alarm1 Normal Bin 1 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 Normal Bin 1 Yield over Limit (%) | ✓ |  |
| 10613 | SV/EC | Alarm1 Normal Bin 2 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 Normal Bin 2 Yield over Limit Enable | ✓ |  |
| 10614 | SV/EC | Alarm1 Normal Bin 2 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 Normal Bin 2 Yield over Limit (%) | ✓ |  |
| 10615 | SV/EC | Alarm1 Normal Bin 3 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 Normal Bin 3 Yield over Limit Enable | ✓ |  |
| 10616 | SV/EC | Alarm1 Normal Bin 3 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 Normal Bin 3 Yield over Limit (%) | ✓ |  |
| 10617 | SV/EC | Alarm1 Normal Bin 4 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 Normal Bin 4 Yield over Limit Enable | ✓ |  |
| 10618 | SV/EC | Alarm1 Normal Bin 4 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 Normal Bin 4 Yield over Limit (%) | ✓ |  |
| 10619 | SV/EC | Alarm1 Normal Bin 5 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 Normal Bin 5 Yield over Limit Enable | ✓ |  |
| 10620 | SV/EC | Alarm1 Normal Bin 5 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 Normal Bin 5 Yield over Limit (%) | ✓ |  |
| 10621 | SV/EC | Alarm1 Normal Bin 6 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 Normal Bin 6 Yield over Limit Enable | ✓ |  |
| 10622 | SV/EC | Alarm1 Normal Bin 6 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 Normal Bin 6 Yield over Limit (%) | ✓ |  |
| 10623 | SV/EC | Alarm1 Normal Bin 7 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 Normal Bin 7 Yield over Limit Enable | ✓ |  |
| 10624 | SV/EC | Alarm1 Normal Bin 7 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 Normal Bin 7 Yield over Limit (%) | ✓ |  |
| 10625 | SV/EC | Alarm1 Normal Bin 8 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 Normal Bin 8 Yield over Limit Enable | ✓ |  |
| 10626 | SV/EC | Alarm1 Normal Bin 8 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 Normal Bin 8 Yield over Limit (%) | ✓ |  |
| 10627 | SV/EC | Alarm1 Normal Bin 9 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 Normal Bin 9 Yield over Limit Enable | ✓ |  |
| 10628 | SV/EC | Alarm1 Normal Bin 9 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 Normal Bin 9 Yield over Limit (%) | ✓ |  |
| 10629 | SV/EC | Alarm1 Normal Bin 10 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 Normal Bin 10 Yield over Limit Enable | ✓ |  |
| 10630 | SV/EC | Alarm1 Normal Bin 10 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 Normal Bin 10 Yield over Limit (%) | ✓ |  |
| 10631 | SV/EC | Alarm1 Normal Bin 11 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 Normal Bin 11 Yield over Limit Enable | ✓ |  |
| 10632 | SV/EC | Alarm1 Normal Bin 11 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 Normal Bin 11 Yield over Limit (%) | ✓ |  |
| 10633 | SV/EC | Alarm1 Normal Bin 12 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 Normal Bin 12 Yield over Limit Enable | ✓ |  |
| 10634 | SV/EC | Alarm1 Normal Bin 12 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 Normal Bin 12 Yield over Limit (%) | ✓ |  |
| 10635 | SV/EC | Alarm1 Normal Bin 13 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 Normal Bin 13 Yield over Limit Enable | ✓ |  |
| 10636 | SV/EC | Alarm1 Normal Bin 13 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 Normal Bin 13 Yield over Limit (%) | ✓ |  |
| 10637 | SV/EC | Alarm1 Normal Bin 14 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 Normal Bin 14 Yield over Limit Enable | ✓ |  |
| 10638 | SV/EC | Alarm1 Normal Bin 14 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 Normal Bin 14 Yield over Limit (%) | ✓ |  |
| 10639 | SV/EC | Alarm1 Normal Bin 15 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 Normal Bin 15 Yield over Limit Enable | ✓ |  |
| 10640 | SV/EC | Alarm1 Normal Bin 15 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 Normal Bin 15 Yield over Limit (%) | ✓ |  |
| 10641 | SV/EC | Alarm Normal Bin 0-255 Yield Over Limit Enable | ASCII |  |  |  |  | Alarm Normal Bin 0-255 Yield Over Limit Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 10642 | SV/EC | Alarm Normal Bin 0-255 Yield Over Limit | ASCII | Percentage |  |  |  | Alarm Normal Bin 0-255 Yield Over Limit 0 to 100 CSV Format for Bin 0 to 255 | ✓ |  |
| 10643 | SV/EC | Alarm Normal Bin 0-255 Yield Over Limit Ignore | ASCII | Percentage |  |  |  | Alarm Normal Bin 0-255 Yield Over Limit Ignore CSV Format for Bin 0 to 255 | ✓ |  |
| 10644 | SV/EC | Alarm Normal Bin 0-255 Count Over Limit Enable | ASCII |  |  |  |  | Alarm Normal Bin 0-255 Count Over Limit Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 10645 | SV/EC | Alarm Normal Bin 0-255 Count Over Limit | ASCII |  |  |  |  | Alarm Normal Bin 0-255 Count Over Limit CSV Format for Bin 0 to 255 | ✓ |  |
| 10646 | SV/EC | Alarm Normal Bin 0-255 Count Over Limit Ignore | ASCII |  |  |  |  | Alarm Normal Bin 0-255 Count Over Limit Ignore CSV Format for Bin 0 to 255 | ✓ |  |
| 10647 | SV/EC | Alarm Normal Bin 0-255 Special Bin By Arm | ASCII |  |  |  |  | Alarm Normal Bin 0-255 Special Bin By Arm 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 10648 | SV/EC | Alarm Normal Bin 0-255 Special Bin Count By Arm | ASCII |  |  |  |  | Alarm Normal Bin 0-255 Special Bin Count By Arm 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 10649 | SV/EC | Alarm Normal Bin 0-255 Special Bin By Socket | ASCII |  |  |  |  | Alarm Normal Bin 0-255 Special Bin By Socket 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 10650 | SV/EC | Alarm Normal Bin 0-255 Special Bin Count By Socket | ASCII |  |  |  |  | Alarm Normal Bin 0-255 Special Bin Count By Socket 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 10680 | SV/EC | LowYield | BOOLEAN |  |  |  |  | LowYield; CSV Format for Bin 0 to 255 | ✓ |  |
| 10681 | SV/EC | ArmYield | BOOLEAN |  |  |  |  | ArmYield; CSV Format for Bin 0 to 255 | ✓ |  |
| 10682 | SV/EC | SiteYield | BOOLEAN |  |  |  |  | SiteYield; CSV Format for Bin 0 to 255 | ✓ |  |
| 10683 | SV/EC | By Bin per site cleaning | INT_4 |  | 10000 | 0 | 0 | By Bin per site cleaning; CSV Format for Bin 0 to 255 | ✓ | ⚠️ EC型別: Code=ASCII |
| 10684 | SV/EC | By Bin count cleaning | INT_4 |  | 10000 | 0 | 0 | By Bin count cleaning; CSV Format for Bin 0 to 255 | ✓ | ⚠️ EC型別: Code=ASCII |
| 10685 | SV/EC | Alarm Normal Bin 0-255 By Site By Bin Compare Enable | ASCII |  |  |  |  | Alarm Normal Bin 0-255 By Site By Bin Compare Percent Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10686 | SV/EC | Alarm Normal Bin 0-255 By Site By Bin Compare Ignore | ASCII |  |  |  |  | Alarm Normal Bin 0-255 By Site By Bin Compare Ignore CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10687 | SV/EC | Alarm Normal Bin 0-255 By Site By Bin Compare Percent | ASCII |  |  |  |  | Alarm Normal Bin 0-255 By Site By Bin Compare Percent CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10688 | SV/EC | Alarm Normal Bin 0-255 By Arm By Site By Bin Compare Enable | ASCII |  |  |  |  | Alarm Normal Bin 0-255 By Arm By Site By Bin Compare Percent Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10689 | SV/EC | Alarm Normal Bin 0-255 By Arm By Site By Bin Compare Ignore | ASCII |  |  |  |  | Alarm Normal Bin 0-255 By Arm By Site By Bin Compare Ignore CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10690 | SV/EC | Alarm Normal Bin 0-255 By Arm By Site By Bin Compare Percent | ASCII |  |  |  |  | Alarm Normal Bin 0-255 By Arm By Site By Bin Compare Percent CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10691 | SV/EC | Alarm1 Normal By Bin Yield over Limit Ignore count | INT_4 |  |  |  |  | Alarm1 Normal By Bin Yield over Limit Ignore count | ✓ |  |
| 10698 | SV/EC | Bin Tray Linked | ASCII |  |  |  |  | Bin Tray Linked |  | 🔴 文件獨有 |
| 10699 | SV/EC | Bin Linked | ASCII |  |  |  |  | Bin Linked |  | 🔴 文件獨有 |
| 10700 | SV/EC | Magazine Setup File | BOOLEAN |  |  |  |  | Magazine Setup File |  | 🔴 文件獨有 |
| 10709 | SV/EC | Alarm1 RT Bin 0 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 RT Bin 0 Yield over Limit Enable | ✓ |  |
| 10710 | SV/EC | Alarm1 RT Bin 0 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 RT Bin 0 Yield over Limit (%) | ✓ |  |
| 10711 | SV/EC | Alarm1 RT Bin 1 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 RT Bin 1 Yield over Limit Enable | ✓ |  |
| 10712 | SV/EC | Alarm1 RT Bin 1 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 RT Bin 1 Yield over Limit (%) | ✓ |  |
| 10713 | SV/EC | Alarm1 RT Bin 2 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 RT Bin 2 Yield over Limit Enable | ✓ |  |
| 10714 | SV/EC | Alarm1 RT Bin 2 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 RT Bin 2 Yield over Limit (%) | ✓ |  |
| 10715 | SV/EC | Alarm1 RT Bin 3 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 RT Bin 3 Yield over Limit Enable | ✓ |  |
| 10716 | SV/EC | Alarm1 RT Bin 3 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 RT Bin 3 Yield over Limit (%) | ✓ |  |
| 10717 | SV/EC | Alarm1 RT Bin 4 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 RT Bin 4 Yield over Limit Enable | ✓ |  |
| 10718 | SV/EC | Alarm1 RT Bin 4 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 RT Bin 4 Yield over Limit (%) | ✓ |  |
| 10719 | SV/EC | Alarm1 RT Bin 5 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 RT Bin 5 Yield over Limit Enable | ✓ |  |
| 10720 | SV/EC | Alarm1 RT Bin 5 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 RT Bin 5 Yield over Limit (%) | ✓ |  |
| 10721 | SV/EC | Alarm1 RT Bin 6 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 RT Bin 6 Yield over Limit Enable | ✓ |  |
| 10722 | SV/EC | Alarm1 RT Bin 6 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 RT Bin 6 Yield over Limit (%) | ✓ |  |
| 10723 | SV/EC | Alarm1 RT Bin 7 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 RT Bin 7 Yield over Limit Enable | ✓ |  |
| 10724 | SV/EC | Alarm1 RT Bin 7 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 RT Bin 7 Yield over Limit (%) | ✓ |  |
| 10725 | SV/EC | Alarm1 RT Bin 8 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 RT Bin 8 Yield over Limit Enable | ✓ |  |
| 10726 | SV/EC | Alarm1 RT Bin 8 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 RT Bin 8 Yield over Limit (%) | ✓ |  |
| 10727 | SV/EC | Alarm1 RT Bin 9 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 RT Bin 9 Yield over Limit Enable | ✓ |  |
| 10728 | SV/EC | Alarm1 RT Bin 9 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 RT Bin 9 Yield over Limit (%) | ✓ |  |
| 10729 | SV/EC | Alarm1 RT Bin 10 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 RT Bin 10 Yield over Limit Enable | ✓ |  |
| 10730 | SV/EC | Alarm1 RT Bin 10 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 RT Bin 10 Yield over Limit (%) | ✓ |  |
| 10731 | SV/EC | Alarm1 RT Bin 11 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 RT Bin 11 Yield over Limit Enable | ✓ |  |
| 10732 | SV/EC | Alarm1 RT Bin 11 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 RT Bin 11 Yield over Limit (%) | ✓ |  |
| 10733 | SV/EC | Alarm1 RT Bin 12 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 RT Bin 12 Yield over Limit Enable | ✓ |  |
| 10734 | SV/EC | Alarm1 RT Bin 12 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 RT Bin 12 Yield over Limit (%) | ✓ |  |
| 10735 | SV/EC | Alarm1 RT Bin 13 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 RT Bin 13 Yield over Limit Enable | ✓ |  |
| 10736 | SV/EC | Alarm1 RT Bin 13 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 RT Bin 13 Yield over Limit (%) | ✓ |  |
| 10737 | SV/EC | Alarm1 RT Bin 14 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 RT Bin 14 Yield over Limit Enable | ✓ |  |
| 10738 | SV/EC | Alarm1 RT Bin 14 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 RT Bin 14 Yield over Limit (%) | ✓ |  |
| 10739 | SV/EC | Alarm1 RT Bin 15 Yield over Limit Enable | BOOLEAN |  |  |  |  | Alarm1 RT Bin 15 Yield over Limit Enable | ✓ |  |
| 10740 | SV/EC | Alarm1 RT Bin 15 Yield over Limit | FT_8 | Percentage |  |  |  | Alarm1 RT Bin 15 Yield over Limit (%) | ✓ |  |
| 10741 | SV/EC | Alarm RT Bin 0-255 Yield over Limit Enable | ASCII |  |  |  |  | Alarm RT Bin 0-255 Yield over Limit Enable CSV Format for Bin 0 to 255 | ✓ |  |
| 10742 | SV/EC | Alarm RT Bin 0-255 Yield over Limit | ASCII | Percentage |  |  |  | Alarm RT Bin 0-255 Yield over Limit (%) CSV Format for Bin 0 to 255 | ✓ |  |
| 10743 | SV/EC | Alarm RT Bin 0-255 Yield over Limit Ignore | ASCII | Percentage |  |  |  | Alarm RT Bin 0-255 Yield over Limit (%) Ignore CSV Format for Bin 0 to 255 | ✓ |  |
| 10744 | SV/EC | Alarm RT Bin 0-255 Count Over Limit Enable | ASCII |  |  |  |  | Alarm RT Bin 0-255 Count Over Limit Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 10745 | SV/EC | Alarm RT Bin 0-255 Count Over Limit | ASCII |  |  |  |  | Alarm RT Bin 0-255 Count Over Limit CSV Format for Bin 0 to 255 | ✓ |  |
| 10746 | SV/EC | Alarm RT Bin 0-255 Count Over Limit Ignore | ASCII |  |  |  |  | Alarm RT Bin 0-255 Count Over Limit Ignore CSV Format for Bin 0 to 255 | ✓ |  |
| 10747 | SV/EC | Alarm RT Bin 0-255 Special Bin By Arm | ASCII |  |  |  |  | Alarm RT Bin 0-255 Special Bin By Arm 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 10748 | SV/EC | Alarm RT Bin 0-255 Special Bin Count By Arm | ASCII |  |  |  |  | Alarm RT Bin 0-255 Special Bin Count By Arm 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 10749 | SV/EC | Alarm RT Bin 0-255 Special Bin By Socket | ASCII |  |  |  |  | Alarm RT Bin 0-255 Special Bin By Socket 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 10750 | SV/EC | Alarm RT Bin 0-255 Special Bin Count By Socket | ASCII |  |  |  |  | Alarm RT Bin 0-255 Special Bin Count By Socket 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 10780 | SV/EC | LowYield RT | BOOLEAN |  |  |  |  | LowYield RT; CSV Format for Bin 0 to 255 | ✓ |  |
| 10781 | SV/EC | ArmYield RT | BOOLEAN |  |  |  |  | ArmYield RT; CSV Format for Bin 0 to 255 | ✓ |  |
| 10782 | SV/EC | SiteYield RT | BOOLEAN |  |  |  |  | SiteYield RT; CSV Format for Bin 0 to 255 | ✓ |  |
| 10783 | SV/EC | By Bin per site cleaning RT | INT_4 |  | 10000 | 0 | 0 | By Bin per site cleaning RT; CSV Format for Bin 0 to 255 | ✓ | ⚠️ EC型別: Code=ASCII |
| 10784 | SV/EC | By Bin count cleaning RT | INT_4 |  | 10000 | 0 | 0 | By Bin count cleaning RT; CSV Format for Bin 0 to 255 | ✓ | ⚠️ EC型別: Code=ASCII |
| 10785 | SV/EC | Alarm RT Bin 0-255 By Site By Bin Compare Enable | ASCII |  |  |  |  | Alarm RT Bin 0-255 By Site By Bin Compare Percent Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10786 | SV/EC | Alarm RT Bin 0-255 By Site By Bin Compare Ignore | ASCII |  |  |  |  | Alarm RT Bin 0-255 By Site By Bin Compare Ignore CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10787 | SV/EC | Alarm RT Bin 0-255 By Site By Bin Compare Percent | ASCII |  |  |  |  | Alarm RT Bin 0-255 By Site By Bin Compare Percent CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10788 | SV/EC | Alarm RT Bin 0-255 By Arm By Site By Bin Compare Enable | ASCII |  |  |  |  | Alarm RT Bin 0-255 By Arm By Site By Bin Compare Percent Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10789 | SV/EC | Alarm RT Bin 0-255 By Arm By Site By Bin Compare Ignore | ASCII |  |  |  |  | Alarm RT Bin 0-255 By Arm By Site By Bin Compare Ignore CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10790 | SV/EC | Alarm RT Bin 0-255 By Arm By Site By Bin Compare Percent | ASCII |  |  |  |  | Alarm RT Bin 0-255 By Arm By Site By Bin Compare Percent CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10791 | SV/EC | Alarm1 RT By Bin Yield over Limit Ignore count | INT_4 |  |  |  |  | Alarm1 RT By Bin Yield over Limit Ignore count | ✓ |  |
| 10798 | SV/EC | Bin Tray Linked | ASCII |  |  |  |  | Bin Tray Linked |  | 🔴 文件獨有 |
| 10799 | SV/EC | Bin Linked | ASCII |  |  |  |  | Bin Linked |  | 🔴 文件獨有 |
| 10800 | SV/EC | Magazine Setup File | BOOLEAN |  |  |  |  | Magazine Setup File |  | 🔴 文件獨有 |
| 10941 | SV/EC | Alarm OffLine Bin 0-255 Yield over Limit Enable | ASCII |  |  |  |  | Alarm OffLine Bin 0-255 Yield over Limit Enable CSV Format for Bin 0 to 255 | ✓ |  |
| 10942 | SV/EC | Alarm OffLine Bin 0-255 Yield over Limit | ASCII | Percentage |  |  |  | Alarm OffLine Bin 0-255 Yield over Limit (%) CSV Format for Bin 0 to 255 | ✓ |  |
| 10943 | SV/EC | Alarm OffLine Bin 0-255 Yield over Limit Ignore | ASCII | Percentage |  |  |  | Alarm OffLine Bin 0-255 Yield over Limit (%) Ignore CSV Format for Bin 0 to 255 | ✓ |  |
| 10944 | SV/EC | Alarm OffLine Bin 0-255 Count Over Limit Enable | ASCII |  |  |  |  | Alarm OffLine Bin 0-255 Count Over Limit Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 10945 | SV/EC | Alarm OffLine Bin 0-255 Count Over Limit | ASCII |  |  |  |  | Alarm OffLine Bin 0-255 Count Over Limit CSV Format for Bin 0 to 255 | ✓ |  |
| 10946 | SV/EC | Alarm OffLine Bin 0-255 Count Over Limit Ignore | ASCII |  |  |  |  | Alarm OffLine Bin 0-255 Count Over Limit Ignore CSV Format for Bin 0 to 255 | ✓ |  |
| 10947 | SV/EC | Alarm OffLine Bin 0-255 Special Bin By Arm | ASCII |  |  |  |  | Alarm OffLine Bin 0-255 Special Bin By Arm 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 10948 | SV/EC | Alarm OffLine Bin 0-255 Special Bin Count By Arm | ASCII |  |  |  |  | Alarm OffLine Bin 0-255 Special Bin Count By Arm 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 10949 | SV/EC | Alarm OffLine Bin 0-255 Special Bin By Socket | ASCII |  |  |  |  | Alarm OffLine Bin 0-255 Special Bin By Socket 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 10950 | SV/EC | Alarm OffLine Bin 0-255 Special Bin Count By Socket | ASCII |  |  |  |  | Alarm OffLine Bin 0-255 Special Bin Count By Socket 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 10980 | SV/EC | LowYield OffLine | BOOLEAN |  |  |  |  | LowYield OffLine; CSV Format for Bin 0 to 255 | ✓ |  |
| 10981 | SV/EC | ArmYield OffLine | BOOLEAN |  |  |  |  | ArmYield OffLine; CSV Format for Bin 0 to 255 | ✓ |  |
| 10982 | SV/EC | SiteYield OffLine | BOOLEAN |  |  |  |  | SiteYield OffLine; CSV Format for Bin 0 to 255 | ✓ |  |
| 10983 | SV/EC | By Bin per site cleaning OffLine | INT_4 |  | 10000 | 0 | 0 | By Bin per site cleaning OffLine; CSV Format for Bin 0 to 255 | ✓ | ⚠️ EC型別: Code=ASCII |
| 10984 | SV/EC | By Bin count cleaning OffLine | INT_4 |  | 10000 | 0 | 0 | By Bin count cleaning OffLine; CSV Format for Bin 0 to 255 | ✓ | ⚠️ EC型別: Code=ASCII |
| 10985 | SV/EC | Alarm OffLine Bin 0-255 By Site By Bin Compare Enable | ASCII |  |  |  |  | Alarm OffLine Bin 0-255 By Site By Bin Compare Percent Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10986 | SV/EC | Alarm OffLine Bin 0-255 By Site By Bin Compare Ignore | ASCII |  |  |  |  | Alarm OffLine Bin 0-255 By Site By Bin Compare Ignore CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10987 | SV/EC | Alarm OffLine Bin 0-255 By Site By Bin Compare Percent | ASCII |  |  |  |  | Alarm OffLine Bin 0-255 By Site By Bin Compare Percent CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10988 | SV/EC | Alarm OffLine Bin 0-255 By Arm By Site By Bin Compare Enable | ASCII |  |  |  |  | Alarm OffLine Bin 0-255 By Arm By Site By Bin Compare Percent Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10989 | SV/EC | Alarm OffLine Bin 0-255 By Arm By Site By Bin Compare Ignore | ASCII |  |  |  |  | Alarm OffLine Bin 0-255 By Arm By Site By Bin Compare Ignore CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10990 | SV/EC | Alarm OffLine Bin 0-255 By Arm By Site By Bin Compare Percent | ASCII |  |  |  |  | Alarm OffLine Bin 0-255 By Arm By Site By Bin Compare Percent CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 10998 | SV/EC | Bin Tray Linked | ASCII |  |  |  |  | Bin Tray Linked |  | 🔴 文件獨有 |
| 10999 | SV/EC | Bin Linked | ASCII |  |  |  |  | Bin Linked |  | 🔴 文件獨有 |
| 11000 | SV/EC | Magazine Setup File | BOOLEAN |  |  |  |  | Magazine Setup File |  | 🔴 文件獨有 |
| 11041 | SV/EC | Alarm ART_FT Bin 0-255 Yield over Limit Enable | ASCII |  |  |  |  | Alarm ART_FT Bin 0-255 Yield over Limit Enable CSV Format for Bin 0 to 255 | ✓ |  |
| 11042 | SV/EC | Alarm ART_FT Bin 0-255 Yield over Limit | ASCII | Percentage |  |  |  | Alarm ART_FT Bin 0-255 Yield over Limit (%) CSV Format for Bin 0 to 255 | ✓ |  |
| 11043 | SV/EC | Alarm ART_FT Bin 0-255 Yield over Limit Ignore | ASCII | Percentage |  |  |  | Alarm ART_FT Bin 0-255 Yield over Limit (%) Ignore CSV Format for Bin 0 to 255 | ✓ |  |
| 11044 | SV/EC | Alarm ART_FT Bin 0-255 Count Over Limit Enable | ASCII |  |  |  |  | Alarm ART_FT Bin 0-255 Count Over Limit Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11045 | SV/EC | Alarm ART_FT Bin 0-255 Count Over Limit | ASCII |  |  |  |  | Alarm ART_FT Bin 0-255 Count Over Limit CSV Format for Bin 0 to 255 | ✓ |  |
| 11046 | SV/EC | Alarm ART_FT Bin 0-255 Count Over Limit Ignore | ASCII |  |  |  |  | Alarm ART_FT Bin 0-255 Count Over Limit Ignore CSV Format for Bin 0 to 255 | ✓ |  |
| 11047 | SV/EC | Alarm ART_FT Bin 0-255 Special Bin By Arm | ASCII |  |  |  |  | Alarm ART_FT Bin 0-255 Special Bin By Arm 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11048 | SV/EC | Alarm ART_FT Bin 0-255 Special Bin Count By Arm | ASCII |  |  |  |  | Alarm ART_FT Bin 0-255 Special Bin Count By Arm 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11049 | SV/EC | Alarm ART_FT Bin 0-255 Special Bin By Socket | ASCII |  |  |  |  | Alarm ART_FT Bin 0-255 Special Bin By Socket 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11050 | SV/EC | Alarm ART_FT Bin 0-255 Special Bin Count By Socket | ASCII |  |  |  |  | Alarm ART_FT Bin 0-255 Special Bin Count By Socket 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11080 | SV/EC | LowYield ART_FT | BOOLEAN |  |  |  |  | LowYield ART_FT; CSV Format for Bin 0 to 255 | ✓ |  |
| 11081 | SV/EC | ArmYield ART_FT | BOOLEAN |  |  |  |  | ArmYield ART_FT; CSV Format for Bin 0 to 255 | ✓ |  |
| 11082 | SV/EC | SiteYield ART_FT | BOOLEAN |  |  |  |  | SiteYield ART_FT; CSV Format for Bin 0 to 255 | ✓ |  |
| 11083 | SV/EC | By Bin per site cleaning ART_FT | INT_4 |  | 10000 | 0 | 0 | By Bin per site cleaning ART_FT; CSV Format for Bin 0 to 255 | ✓ | ⚠️ EC型別: Code=ASCII |
| 11084 | SV/EC | By Bin count cleaning ART_FT | INT_4 |  | 10000 | 0 | 0 | By Bin count cleaning ART_FT; CSV Format for Bin 0 to 255 | ✓ | ⚠️ EC型別: Code=ASCII |
| 11085 | SV/EC | Alarm ART_FT Bin 0-255 By Site By Bin Compare Enable | ASCII |  |  |  |  | Alarm ART_FT Bin 0-255 By Site By Bin Compare Percent Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11086 | SV/EC | Alarm ART_FT Bin 0-255 By Site By Bin Compare Ignore | ASCII |  |  |  |  | Alarm ART_FT Bin 0-255 By Site By Bin Compare Ignore CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11087 | SV/EC | Alarm ART_FT Bin 0-255 By Site By Bin Compare Percent | ASCII |  |  |  |  | Alarm ART_FT Bin 0-255 By Site By Bin Compare Percent CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11088 | SV/EC | Alarm ART_FT Bin 0-255 By Arm By Site By Bin Compare Enable | ASCII |  |  |  |  | Alarm ART_FT Bin 0-255 By Arm By Site By Bin Compare Percent Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11089 | SV/EC | Alarm ART_FT Bin 0-255 By Arm By Site By Bin Compare Ignore | ASCII |  |  |  |  | Alarm ART_FT Bin 0-255 By Arm By Site By Bin Compare Ignore CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11090 | SV/EC | Alarm ART_FT Bin 0-255 By Arm By Site By Bin Compare Percent | ASCII |  |  |  |  | Alarm ART_FT Bin 0-255 By Arm By Site By Bin Compare Percent CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11098 | SV/EC | Bin Tray Linked | ASCII |  |  |  |  | Bin Tray Linked |  | 🔴 文件獨有 |
| 11099 | SV/EC | Bin Linked | ASCII |  |  |  |  | Bin Linked |  | 🔴 文件獨有 |
| 11100 | SV/EC | Magazine Setup File | BOOLEAN |  |  |  |  | Magazine Setup File |  | 🔴 文件獨有 |
| 11141 | SV/EC | Alarm ART_RT Bin 0-255 Yield over Limit Enable | ASCII |  |  |  |  | Alarm ART_RT Bin 0-255 Yield over Limit Enable CSV Format for Bin 0 to 255 | ✓ |  |
| 11142 | SV/EC | Alarm ART_RT Bin 0-255 Yield over Limit | ASCII | Percentage |  |  |  | Alarm ART_RT Bin 0-255 Yield over Limit (%) CSV Format for Bin 0 to 255 | ✓ |  |
| 11143 | SV/EC | Alarm ART_RT Bin 0-255 Yield over Limit Ignore | ASCII | Percentage |  |  |  | Alarm ART_RT Bin 0-255 Yield over Limit (%) Ignore CSV Format for Bin 0 to 255 | ✓ |  |
| 11144 | SV/EC | Alarm ART_RT Bin 0-255 Count Over Limit Enable | ASCII |  |  |  |  | Alarm ART_RT Bin 0-255 Count Over Limit Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11145 | SV/EC | Alarm ART_RT Bin 0-255 Count Over Limit | ASCII |  |  |  |  | Alarm ART_RT Bin 0-255 Count Over Limit CSV Format for Bin 0 to 255 | ✓ |  |
| 11146 | SV/EC | Alarm ART_RT Bin 0-255 Count Over Limit Ignore | ASCII |  |  |  |  | Alarm ART_RT Bin 0-255 Count Over Limit Ignore CSV Format for Bin 0 to 255 | ✓ |  |
| 11147 | SV/EC | Alarm ART_RT Bin 0-255 Special Bin By Arm | ASCII |  |  |  |  | Alarm ART_RT Bin 0-255 Special Bin By Arm 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11148 | SV/EC | Alarm ART_RT Bin 0-255 Special Bin Count By Arm | ASCII |  |  |  |  | Alarm ART_RT Bin 0-255 Special Bin Count By Arm 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11149 | SV/EC | Alarm ART_RT Bin 0-255 Special Bin By Socket | ASCII |  |  |  |  | Alarm ART_RT Bin 0-255 Special Bin By Socket 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11150 | SV/EC | Alarm ART_RT Bin 0-255 Special Bin Count By Socket | ASCII |  |  |  |  | Alarm ART_RT Bin 0-255 Special Bin Count By Socket 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11180 | SV/EC | LowYield ART_RT | BOOLEAN |  |  |  |  | LowYield ART_RT; CSV Format for Bin 0 to 255 | ✓ |  |
| 11181 | SV/EC | ArmYield ART_RT | BOOLEAN |  |  |  |  | ArmYield ART_RT; CSV Format for Bin 0 to 255 | ✓ |  |
| 11182 | SV/EC | SiteYield ART_RT | BOOLEAN |  |  |  |  | SiteYield ART_RT; CSV Format for Bin 0 to 255 | ✓ |  |
| 11183 | SV/EC | By Bin per site cleaning ART_RT | INT_4 |  | 10000 | 0 | 0 | By Bin per site cleaning ART_RT; CSV Format for Bin 0 to 255 | ✓ | ⚠️ EC型別: Code=ASCII |
| 11184 | SV/EC | By Bin count cleaning ART_RT | INT_4 |  | 10000 | 0 | 0 | By Bin count cleaning ART_RT; CSV Format for Bin 0 to 255 | ✓ | ⚠️ EC型別: Code=ASCII |
| 11185 | SV/EC | Alarm ART_RT Bin 0-255 By Site By Bin Compare Enable | ASCII |  |  |  |  | Alarm ART_RT Bin 0-255 By Site By Bin Compare Percent Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11186 | SV/EC | Alarm ART_RT Bin 0-255 By Site By Bin Compare Ignore | ASCII |  |  |  |  | Alarm ART_RT Bin 0-255 By Site By Bin Compare Ignore CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11187 | SV/EC | Alarm ART_RT Bin 0-255 By Site By Bin Compare Percent | ASCII |  |  |  |  | Alarm ART_RT Bin 0-255 By Site By Bin Compare Percent CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11188 | SV/EC | Alarm ART_RT Bin 0-255 By Arm By Site By Bin Compare Enable | ASCII |  |  |  |  | Alarm ART_RT Bin 0-255 By Arm By Site By Bin Compare Percent Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11189 | SV/EC | Alarm ART_RT Bin 0-255 By Arm By Site By Bin Compare Ignore | ASCII |  |  |  |  | Alarm ART_RT Bin 0-255 By Arm By Site By Bin Compare Ignore CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11190 | SV/EC | Alarm ART_RT Bin 0-255 By Arm By Site By Bin Compare Percent | ASCII |  |  |  |  | Alarm ART_RT Bin 0-255 By Arm By Site By Bin Compare Percent CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11198 | SV/EC | Bin Tray Linked | ASCII |  |  |  |  | Bin Tray Linked |  | 🔴 文件獨有 |
| 11199 | SV/EC | Bin Linked | ASCII |  |  |  |  | Bin Linked |  | 🔴 文件獨有 |
| 11200 | SV/EC | Magazine Setup File | BOOLEAN |  |  |  |  | Magazine Setup File |  | 🔴 文件獨有 |
| 11241 | SV/EC | Alarm MRT_FT Bin 0-255 Yield over Limit Enable | ASCII |  |  |  |  | Alarm MRT_FT Bin 0-255 Yield over Limit Enable CSV Format for Bin 0 to 255 | ✓ |  |
| 11242 | SV/EC | Alarm MRT_FT Bin 0-255 Yield over Limit | ASCII | Percentage |  |  |  | Alarm MRT_FT Bin 0-255 Yield over Limit (%) CSV Format for Bin 0 to 255 | ✓ |  |
| 11243 | SV/EC | Alarm MRT_FT Bin 0-255 Yield over Limit Ignore | ASCII | Percentage |  |  |  | Alarm MRT_FT Bin 0-255 Yield over Limit (%) Ignore CSV Format for Bin 0 to 255 | ✓ |  |
| 11244 | SV/EC | Alarm MRT_FT Bin 0-255 Count Over Limit Enable | ASCII |  |  |  |  | Alarm MRT_FT Bin 0-255 Count Over Limit Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11245 | SV/EC | Alarm MRT_FT Bin 0-255 Count Over Limit | ASCII |  |  |  |  | Alarm MRT_FT Bin 0-255 Count Over Limit CSV Format for Bin 0 to 255 | ✓ |  |
| 11246 | SV/EC | Alarm MRT_FT Bin 0-255 Count Over Limit Ignore | ASCII |  |  |  |  | Alarm MRT_FT Bin 0-255 Count Over Limit Ignore CSV Format for Bin 0 to 255 | ✓ |  |
| 11247 | SV/EC | Alarm MRT_FT Bin 0-255 Special Bin By Arm | ASCII |  |  |  |  | Alarm MRT_FT Bin 0-255 Special Bin By Arm 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11248 | SV/EC | Alarm MRT_FT Bin 0-255 Special Bin Count By Arm | ASCII |  |  |  |  | Alarm MRT_FT Bin 0-255 Special Bin Count By Arm 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11249 | SV/EC | Alarm MRT_FT Bin 0-255 Special Bin By Socket | ASCII |  |  |  |  | Alarm MRT_FT Bin 0-255 Special Bin By Socket 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11250 | SV/EC | Alarm MRT_FT Bin 0-255 Special Bin Count By Socket | ASCII |  |  |  |  | Alarm MRT_FT Bin 0-255 Special Bin Count By Socket 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11280 | SV/EC | LowYield MRT_FT | BOOLEAN |  |  |  |  | LowYield MRT_FT; CSV Format for Bin 0 to 255 | ✓ |  |
| 11281 | SV/EC | ArmYield MRT_FT | BOOLEAN |  |  |  |  | ArmYield MRT_FT; CSV Format for Bin 0 to 255 | ✓ |  |
| 11282 | SV/EC | SiteYield MRT_FT | BOOLEAN |  |  |  |  | SiteYield MRT_FT; CSV Format for Bin 0 to 255 | ✓ |  |
| 11283 | SV/EC | By Bin per site cleaning MRT_FT | INT_4 |  | 10000 | 0 | 0 | By Bin per site cleaning MRT_FT; CSV Format for Bin 0 to 255 | ✓ | ⚠️ EC型別: Code=ASCII |
| 11284 | SV/EC | By Bin count cleaning MRT_FT | INT_4 |  | 10000 | 0 | 0 | By Bin count cleaning MRT_FT; CSV Format for Bin 0 to 255 | ✓ | ⚠️ EC型別: Code=ASCII |
| 11285 | SV/EC | Alarm MRT_FT Bin 0-255 By Site By Bin Compare Enable | ASCII |  |  |  |  | Alarm MRT_FT Bin 0-255 By Site By Bin Compare Percent Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11286 | SV/EC | Alarm MRT_FT Bin 0-255 By Site By Bin Compare Ignore | ASCII |  |  |  |  | Alarm MRT_FT Bin 0-255 By Site By Bin Compare Ignore CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11287 | SV/EC | Alarm MRT_FT Bin 0-255 By Site By Bin Compare Percent | ASCII |  |  |  |  | Alarm MRT_FT Bin 0-255 By Site By Bin Compare Percent CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11288 | SV/EC | Alarm MRT_FT Bin 0-255 By Arm By Site By Bin Compare Enable | ASCII |  |  |  |  | Alarm MRT_FT Bin 0-255 By Arm By Site By Bin Compare Percent Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11289 | SV/EC | Alarm MRT_FT Bin 0-255 By Arm By Site By Bin Compare Ignore | ASCII |  |  |  |  | Alarm MRT_FT Bin 0-255 By Arm By Site By Bin Compare Ignore CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11290 | SV/EC | Alarm MRT_FT Bin 0-255 By Arm By Site By Bin Compare Percent | ASCII |  |  |  |  | Alarm MRT_FT Bin 0-255 By Arm By Site By Bin Compare Percent CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11298 | SV/EC | Bin Tray Linked | ASCII |  |  |  |  | Bin Tray Linked |  | 🔴 文件獨有 |
| 11299 | SV/EC | Bin Linked | ASCII |  |  |  |  | Bin Linked |  | 🔴 文件獨有 |
| 11300 | SV/EC | Magazine Setup File | BOOLEAN |  |  |  |  | Magazine Setup File |  | 🔴 文件獨有 |
| 11341 | SV/EC | Alarm MRT_RT Bin 0-255 Yield over Limit Enable | ASCII |  |  |  |  | Alarm MRT_RT Bin 0-255 Yield over Limit Enable CSV Format for Bin 0 to 255 | ✓ |  |
| 11342 | SV/EC | Alarm MRT_RT Bin 0-255 Yield over Limit | ASCII | Percentage |  |  |  | Alarm MRT_RT Bin 0-255 Yield over Limit (%) CSV Format for Bin 0 to 255 | ✓ |  |
| 11343 | SV/EC | Alarm MRT_RT Bin 0-255 Yield over Limit Ignore | ASCII | Percentage |  |  |  | Alarm MRT_RT Bin 0-255 Yield over Limit (%) Ignore CSV Format for Bin 0 to 255 | ✓ |  |
| 11344 | SV/EC | Alarm MRT_RT Bin 0-255 Count Over Limit Enable | ASCII |  |  |  |  | Alarm MRT_RT Bin 0-255 Count Over Limit Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11345 | SV/EC | Alarm MRT_RT Bin 0-255 Count Over Limit | ASCII |  |  |  |  | Alarm MRT_RT Bin 0-255 Count Over Limit CSV Format for Bin 0 to 255 | ✓ |  |
| 11346 | SV/EC | Alarm MRT_RT Bin 0-255 Count Over Limit Ignore | ASCII |  |  |  |  | Alarm MRT_RT Bin 0-255 Count Over Limit Ignore CSV Format for Bin 0 to 255 | ✓ |  |
| 11347 | SV/EC | Alarm MRT_RT Bin 0-255 Special Bin By Arm | ASCII |  |  |  |  | Alarm MRT_RT Bin 0-255 Special Bin By Arm 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11348 | SV/EC | Alarm MRT_RT Bin 0-255 Special Bin Count By Arm | ASCII |  |  |  |  | Alarm MRT_RT Bin 0-255 Special Bin Count By Arm 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11349 | SV/EC | Alarm MRT_RT Bin 0-255 Special Bin By Socket | ASCII |  |  |  |  | Alarm MRT_RT Bin 0-255 Special Bin By Socket 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11350 | SV/EC | Alarm MRT_RT Bin 0-255 Special Bin Count By Socket | ASCII |  |  |  |  | Alarm MRT_RT Bin 0-255 Special Bin Count By Socket 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 | ✓ |  |
| 11380 | SV/EC | LowYield MRT_RT | BOOLEAN |  |  |  |  | LowYield MRT_RT; CSV Format for Bin 0 to 255 | ✓ |  |
| 11381 | SV/EC | ArmYield MRT_RT | BOOLEAN |  |  |  |  | ArmYield MRT_RT; CSV Format for Bin 0 to 255 | ✓ |  |
| 11382 | SV/EC | SiteYield MRT_RT | BOOLEAN |  |  |  |  | SiteYield MRT_RT; CSV Format for Bin 0 to 255 | ✓ |  |
| 11383 | SV/EC | By Bin per site cleaning MRT_RT | INT_4 |  | 10000 | 0 | 0 | By Bin per site cleaning MRT_RT; CSV Format for Bin 0 to 255 | ✓ | ⚠️ EC型別: Code=ASCII |
| 11384 | SV/EC | By Bin count cleaning MRT_RT | INT_4 |  | 10000 | 0 | 0 | By Bin count cleaning MRT_RT; CSV Format for Bin 0 to 255 | ✓ | ⚠️ EC型別: Code=ASCII |
| 11385 | SV/EC | Alarm MRT_RT Bin 0-255 By Site By Bin Compare Enable | ASCII |  |  |  |  | Alarm MRT_RT Bin 0-255 By Site By Bin Compare Percent Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11386 | SV/EC | Alarm MRT_RT Bin 0-255 By Site By Bin Compare Ignore | ASCII |  |  |  |  | Alarm MRT_RT Bin 0-255 By Site By Bin Compare Ignore CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11387 | SV/EC | Alarm MRT_RT Bin 0-255 By Site By Bin Compare Percent | ASCII |  |  |  |  | Alarm MRT_RT Bin 0-255 By Site By Bin Compare Percent CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11388 | SV/EC | Alarm MRT_RT Bin 0-255 By Arm By Site By Bin Compare Enable | ASCII |  |  |  |  | Alarm MRT_RT Bin 0-255 By Arm By Site By Bin Compare Percent Enable 0: Disable; 1:Enable;  CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11389 | SV/EC | Alarm MRT_RT Bin 0-255 By Arm By Site By Bin Compare Ignore | ASCII |  |  |  |  | Alarm MRT_RT Bin 0-255 By Arm By Site By Bin Compare Ignore CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11390 | SV/EC | Alarm MRT_RT Bin 0-255 By Arm By Site By Bin Compare Percent | ASCII |  |  |  |  | Alarm MRT_RT Bin 0-255 By Arm By Site By Bin Compare Percent CSV Format for Bin 0 to 255 |  | 🔴 文件獨有 |
| 11398 | SV/EC | Bin Tray Linked | ASCII |  |  |  |  | Bin Tray Linked |  | 🔴 文件獨有 |
| 11399 | SV/EC | Bin Linked | ASCII |  |  |  |  | Bin Linked |  | 🔴 文件獨有 |
| 11400 | SV/EC | Magazine Setup File | BOOLEAN |  |  |  |  | Magazine Setup File |  | 🔴 文件獨有 |

## Bin 設定 (15000-19999)
> ⚠️ 此區段有 **16** 筆差異

| ID | 類型 | Name | DataType | Unit | Max | Min | Default | Description | Code | 差異 |
|----|------|------|----------|------|-----|-----|---------|-------------|------|------|
| 15500 | SV/EC | Pre Alarm Message | INT_4 |  | 0 | 0 | 0 | 1:LD 2:Auto1 3:Auto2 4:Auto3 5:Fix1 6:Fix2 7:Fix3 8:Empty 9:Color | ✓ |  |
| 16000 | SV/EC | Consecutive Failure Alarm by Socket | INT_4 |  | 1 | 0 | 0 | 0: ON;  1: OFF; | ✓ |  |
| 16001 | SV/EC | Consecutive Failure Alarm Count by Socket | INT_4 |  | 100 | 1 | 100 | Consecutive Failure Alarm Count by Socket | ✓ |  |
| 16002 | SV/EC | Consecutive Failure Alarm by Head | INT_4 |  | 1 | 0 | 0 | 0: ON;  1: OFF; | ✓ |  |
| 16003 | SV/EC | Consecutive Failure Alarm Count by Head | INT_4 |  | 100 | 1 | 100 | Consecutive Failure Alarm Count by Head | ✓ |  |
| 16004 | SV/EC | Fail Rate Alarm | INT_4 |  | 1 | 0 | 0 | 0: ON;  1: OFF; |  | 🔴 文件獨有 |
| 16005 | SV/EC | Fail Rate Alarm Ignored IC | INT_4 |  | 100000 | 1 | 100 | Fail Rate Alarm Ignored IC |  | 🔴 文件獨有 |
| 16006 | SV/EC | Fail Rate Alarm Type | INT_4 |  | 1 | 0 | 0 | Fail Rate Alarm Type |  | 🔴 文件獨有 |
| 16007 | SV/EC | Low Yield Alarm | BOOLEAN |  | 1 | 0 | 0 | Low Yield Alarm | ✓ |  |
| 16008 | EC | Low Yield Alarm Percentage | INT_4 | Percentage |  |  |  |  | ✓ | 🟡 程式獨有 |
| 16009 | SV/EC | Low Yield Alarm Ignore Count | INT_4 |  | 100000 | 1 | 100 | Low Yield Alarm Ignore Count | ✓ |  |
| 16010 | SV/EC | Site Different Alarm | BOOLEAN |  | 1 | 0 | 0 | Site Different Alarm | ✓ |  |
| 16011 | EC | Site Different Alarm Percentage | INT_4 | Percentage |  |  |  |  | ✓ | 🟡 程式獨有 |
| 16012 | SV/EC | Site Different Alarm Ignore Count | INT_4 |  | 100000 | 1 | 100 | Site Different Alarm Ignore Count | ✓ |  |
| 16013 | SV/EC | Piggy Back Function Action | INT_4 |  | 1 | 0 | 0 | 0: Alarm;  1: Index Check; | ✓ |  |
| 16014 | SV/EC | Piggy Back Continual Pass Bin Enable (Total) | BOOLEAN |  | 1 | 0 | 0 | Piggy Back Continual Pass Bin Enable (Total) | ✓ |  |
| 16015 | SV/EC | Piggy Back Continual Pass Bin Setting (Total) | INT_4 |  | 255 | 0 | 1 | Piggy Back Continual Pass Bin Setting (Total) | ✓ |  |
| 16016 | SV/EC | Piggy Back Continual Pass Bin Count (Total) | INT_4 |  | 100000 | 1 | 100 | Piggy Back Continual Pass Bin Count (Total) | ✓ |  |
| 16017 | SV/EC | Piggy Back Continual Pass Bin Enable (Socket) | BOOLEAN |  | 1 | 0 | 0 | Piggy Back Continual Pass Bin Enable (Socket) | ✓ |  |
| 16018 | SV/EC | Piggy Back Continual Pass Bin Count (Socket) | INT_4 |  | 100000 | 1 | 100 | Piggy Back Continual Pass Bin Count (Socket) | ✓ |  |
| 16019 | SV/EC | Piggy Back Continual Loader Enable | BOOLEAN |  | 1 | 0 | 0 | Piggy Back Continual Loader Enable | ✓ |  |
| 16020 | SV/EC | Piggy Back Continual Loader Count | INT_4 |  | 100000 | 1 | 100 | Piggy Back Continual Loader Count | ✓ |  |
| 16021 | SV/EC | Piggy Back Continual Contact Enable | BOOLEAN |  | 1 | 0 | 0 | Piggy Back Continual Contact Enable | ✓ |  |
| 16022 | SV/EC | Piggy Back Continual Contact Count | INT_4 |  | 100000 | 1 | 100 | Piggy Back Continual Contact Count | ✓ |  |
| 16023 | SV/EC | By Site Compare Yield Enable | BOOLEAN |  | 1 | 0 | 0 | By Site Compare Yield Enable | ✓ |  |
| 16024 | SV/EC | By Site Compare Yield Percentage | INT_4 | Percentage | 100 | 1 | 100 | By Site Compare Yield Percentage | ✓ |  |
| 16025 | SV/EC | By Site Compare Yield Ignore Count | INT_4 |  | 100000 | 1 | 100 | By Site Compare Yield Ignore Count | ✓ |  |
| 16026 | SV/EC | Low Yields%(By Total) Enable | BOOLEAN |  | 1 | 0 | 0 | Low Yields%(By Total) Enable | ✓ |  |
| 16027 | SV/EC | Low Yields%(By Total) Percentage | INT_4 | Percentage | 100 | 1 | 100 | Low Yields%(By Total) Percentage | ✓ |  |
| 16028 | SV/EC | Low Yields%(By Total) Ignore Count | INT_4 |  | 100000 | 1 | 100 | Low Yields%(By Total) Ignore Count | ✓ |  |
| 16029 | SV/EC | All Site Fail Enable | BOOLEAN |  | 1 | 0 | 0 | All Site Fail Enable | ✓ |  |
| 16030 | SV/EC | All Site Fail Count | INT_4 |  | 100 | 1 | 0 | All Site Fail Count | ✓ |  |
| 16031 | SV/EC | Enable Yield Alarm by Bin Setting | BOOLEAN |  | 1 | 0 | 0 | Enable Yield Alarm by Bin Setting | ✓ |  |
| 16050 | SV/EC | Consecutive Failure Alarm by Socket for RT | INT_4 |  | 1 | 0 | 0 | 0: ON;  1: OFF; | ✓ | ⚠️ EC型別: Code=BOOLEAN |
| 16051 | SV/EC | Consecutive Failure Alarm Count by Socket for RT | INT_4 |  | 100 | 1 | 100 | Consecutive Failure Alarm Count by Socket for RT | ✓ |  |
| 16052 | SV/EC | Consecutive Failure Alarm by Head for RT | INT_4 |  | 1 | 0 | 0 | 0: ON;  1: OFF; | ✓ | ⚠️ EC型別: Code=BOOLEAN |
| 16053 | SV/EC | Consecutive Failure Alarm Count by Head for RT | INT_4 |  | 100 | 1 | 100 | Consecutive Failure Alarm Count by Head for RT | ✓ |  |
| 16054 | SV/EC | Fail Rate Alarm for RT | INT_4 |  | 1 | 0 | 0 | 0: ON;  1: OFF; |  | 🔴 文件獨有 |
| 16055 | SV/EC | Fail Rate Alarm Ignored IC for RT | INT_4 |  | 100000 | 1 | 100 | Fail Rate Alarm Ignored IC for RT |  | 🔴 文件獨有 |
| 16056 | SV/EC | Fail Rate Alarm Type for RT | INT_4 |  | 1 | 0 | 0 | Fail Rate Alarm Type for RT |  | 🔴 文件獨有 |
| 16057 | SV/EC | Low Yield Alarm for RT | BOOLEAN |  | 1 | 0 | 0 | Low Yield Alarm for RT | ✓ |  |
| 16058 | EC | Low Yield Alarm Percentage for RT | INT_4 | Percentage |  |  |  |  | ✓ | 🟡 程式獨有 |
| 16059 | SV/EC | Low Yield Alarm Ignore Count for RT | INT_4 |  | 100000 | 1 | 100 | Low Yield Alarm Ignore Count for RT | ✓ |  |
| 16060 | SV/EC | Site Different Alarm for RT | BOOLEAN |  | 1 | 0 | 0 | Site Different Alarm for RT | ✓ |  |
| 16061 | EC | Site Different Alarm Percentage for RT | INT_4 | Percentage |  |  |  |  | ✓ | 🟡 程式獨有 |
| 16062 | SV/EC | Site Different Alarm Ignore Count for RT | INT_4 |  | 100000 | 1 | 100 | Site Different Alarm Ignore Count for RT | ✓ |  |
| 16063 | SV/EC | Piggy Back Function Action for RT | INT_4 |  | 1 | 0 | 0 | 0: Alarm;  1: Index Check; | ✓ |  |
| 16064 | SV/EC | Piggy Back Continual Pass Bin Enable (Total) for RT | BOOLEAN |  | 1 | 0 | 0 | Piggy Back Continual Pass Bin Enable (Total) for RT | ✓ |  |
| 16065 | SV/EC | Piggy Back Continual Pass Bin Setting (Total) for RT | INT_4 |  | 255 | 0 | 1 | Piggy Back Continual Pass Bin Setting (Total) for RT | ✓ |  |
| 16066 | SV/EC | Piggy Back Continual Pass Bin Count (Total) for RT | INT_4 |  | 100000 | 1 | 100 | Piggy Back Continual Pass Bin Count (Total) for RT | ✓ |  |
| 16067 | SV/EC | Piggy Back Continual Pass Bin Enable (Socket) for RT | BOOLEAN |  | 1 | 0 | 0 | Piggy Back Continual Pass Bin Enable (Socket) for RT | ✓ |  |
| 16068 | SV/EC | Piggy Back Continual Pass Bin Count (Socket) for RT | INT_4 |  | 100000 | 1 | 100 | Piggy Back Continual Pass Bin Count (Socket) for RT | ✓ |  |
| 16069 | SV/EC | Piggy Back Continual Loader Enable for RT | BOOLEAN |  | 1 | 0 | 0 | Piggy Back Continual Loader Enable for RT | ✓ |  |
| 16070 | SV/EC | Piggy Back Continual Loader Count for RT | INT_4 |  | 100000 | 1 | 100 | Piggy Back Continual Loader Count for RT | ✓ |  |
| 16071 | SV/EC | Piggy Back Continual Contact Enable for RT | BOOLEAN |  | 1 | 0 | 0 | Piggy Back Continual Contact Enable for RT | ✓ |  |
| 16072 | SV/EC | Piggy Back Continual Contact Count for RT | INT_4 |  | 100000 | 1 | 100 | Piggy Back Continual Contact Count for RT | ✓ |  |
| 16073 | SV/EC | By Site Compare Yield Enable for RT | BOOLEAN |  | 1 | 0 | 0 | By Site Compare Yield Enable for RT | ✓ |  |
| 16074 | SV/EC | By Site Compare Yield Percentage for RT | INT_4 | Percentage | 100 | 1 | 100 | By Site Compare Yield Percentage for RT | ✓ |  |
| 16075 | SV/EC | By Site Compare Yield Ignore Count for RT | INT_4 |  | 100000 | 1 | 100 | By Site Compare Yield Ignore Count for RT | ✓ |  |
| 16076 | SV/EC | Low Yields%(By Total) Enable for RT | BOOLEAN |  | 1 | 0 | 0 | Low Yields%(By Total) Enable for RT | ✓ |  |
| 16077 | SV/EC | Low Yields%(By Total) Percentage for RT | INT_4 | Percentage | 100 | 1 | 100 | Low Yields%(By Total) Percentage for RT | ✓ |  |
| 16078 | SV/EC | Low Yields%(By Total) Ignore Count for RT | INT_4 |  | 100000 | 1 | 100 | Low Yields%(By Total) Ignore Count for RT | ✓ |  |
| 16079 | SV/EC | All Site Fail Enable for RT | BOOLEAN |  | 1 | 0 | 0 | All Site Fail Enable for RT | ✓ |  |
| 16100 | SV/EC | QA Mode Device Count | INT_4 |  | 10000 | 50 | 50 | QA Mode Device Count | ✓ |  |
| 16101 | SV/EC | Running Mode After QA Mode Finish | INT_4 |  | 1 | 0 | 0 | 0: Clean out every devices without test automatically;  1: One cycle and alarm then normal production; | ✓ |  |
| 16102 | SV/EC | QA Mode Use Bin | INT_4 |  | 15 | 1 | 1 | QA Mode Use Bin | ✓ | ⚠️ EC名稱: Code=`Setted bin for untest devices` |
| 16103 | EC | Tray end need to do QA mode again | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 16110 | SV/EC | Active Rotate Function | INT_4 |  | 1 | 0 | 0 | Active Rotate Function | ✓ |  |
| 16111 | SV/EC | Rotate Dut Number | INT_4 |  | 1 | 0 | 0 | 0: 4 Kit;  1: 8 Kit; | ✓ |  |
| 16112 | SV/EC | Rotate X Pitch | FT_8 | mm | 80 | 10 | 40 | Rotate X Pitch | ✓ |  |
| 16113 | SV/EC | Rotate Y Pitch | FT_8 | mm | 80 | 10 | 60 | Rotate Y Pitch | ✓ |  |
| 16114 | SV/EC | Aa Rotate Angle | FT_8 | Degree | 360 | 0 | 0 | Aa Rotate Angle | ✓ |  |
| 16115 | SV/EC | Ab Rotate Angle | FT_8 | Degree | 360 | 0 | 0 | Ab Rotate Angle | ✓ |  |
| 16116 | SV/EC | Ac Rotate Angle | FT_8 | Degree | 360 | 0 | 0 | Ac Rotate Angle | ✓ |  |
| 16117 | SV/EC | Ad Rotate Angle | FT_8 | Degree | 360 | 0 | 0 | Ad Rotate Angle | ✓ |  |
| 16122 | SV/EC | Ba Rotate Angle | FT_8 | Degree | 360 | 0 | 0 | Ba Rotate Angle | ✓ |  |
| 16123 | SV/EC | Bb Rotate Angle | FT_8 | Degree | 360 | 0 | 0 | Bb Rotate Angle | ✓ |  |
| 16124 | SV/EC | Bc Rotate Angle | FT_8 | Degree | 360 | 0 | 0 | Bc Rotate Angle | ✓ |  |
| 16125 | SV/EC | Bd Rotate Angle | FT_8 | Degree | 360 | 0 | 0 | Bd Rotate Angle | ✓ |  |
| 16148 | EC | In Rotate Angle | ASCII | Degree |  |  |  |  | ✓ | 🟡 程式獨有 |
| 16149 | EC | Out Rotate Angle | ASCII | Degree |  |  |  |  | ✓ | 🟡 程式獨有 |
| 16150 | SV/EC | Loader Tray Arrival Lock Time | FT_8 | Second | 10 | 0.1 | 0.1 | Loader Tray Arrival Lock Time | ✓ |  |
| 16151 | SV/EC | Loader Lock OK Time | FT_8 | Second | 10 | 0.1 | 0.1 | Loader Lock OK Time | ✓ |  |
| 16152 | SV/EC | Loader Lifter to Middle Time | FT_8 | Second | 10 | 0.1 | 0.1 | Loader Lifter to Middle Time | ✓ |  |
| 16153 | SV/EC | Loader Lifter to Down Time | FT_8 | Second | 10 | 0.1 | 0.1 | Loader Lifter to Down Time | ✓ |  |
| 16154 | SV/EC | Unloader Tray Arrival Lock Time | FT_8 | Second | 10 | 0.1 | 0.1 | Unloader Tray Arrival Lock Time | ✓ |  |
| 16155 | SV/EC | Unloader Lock OK Time | FT_8 | Second | 10 | 0.1 | 0.1 | Unloader Lock OK Time | ✓ |  |
| 16156 | SV/EC | Unloader Lifter to Middle Time | FT_8 | Second | 10 | 0.1 | 0.1 | Unloader Lifter to Middle Time | ✓ |  |
| 16157 | SV/EC | Unloader Lifter to Down Time | FT_8 | Second | 10 | 0.1 | 0.1 | Unloader Lifter to Down Time | ✓ |  |
| 16170 | SV/EC | Low Yield Alarm Percentage | FT_8 | Percentage | 100 | 0 | 0 | Low Yield Alarm Percentage | ✓ |  |
| 16171 | SV/EC | Site Different Alarm Percentage | FT_8 | Percentage | 100 | 0 | 0 | Site Different Alarm Percentage | ✓ |  |
| 16172 | SV/EC | Low Yield Alarm Percentage for RT | FT_8 | Percentage | 100 | 0 | 0 | Low Yield Alarm Percentage for RT | ✓ |  |
| 16173 | SV/EC | Site Different Alarm Percentage for RT | FT_8 | Percentage | 100 | 0 | 0 | Site Different Alarm Percentage for RT | ✓ |  |
| 16174 | SV/EC | By Site Compare Yield Percentage | FT_8 | Percentage | 100 | 0 | 0 | By Site Compare Yield Percentage | ✓ |  |
| 16175 | SV/EC | Low Yields%(By Total) Percentage | FT_8 | Percentage | 100 | 0 | 0 | Low Yields%(By Total) Percentage | ✓ |  |
| 16176 | SV/EC | By Site Compare Yield Percentage for RT | FT_8 | Percentage | 100 | 0 | 0 | By Site Compare Yield Percentage for RT | ✓ |  |
| 16177 | SV/EC | Low Yields%(By Total) Percentage for RT | FT_8 | Percentage | 100 | 0 | 0 | Low Yields%(By Total) Percentage for RT | ✓ |  |
| 16200 | SV | Site Aa Bin 0-255 Total | ASCII |  |  |  |  | Site Aa Bin 0-255 Total CSV Format | ✓ |  |
| 16201 | SV | Site Ab Bin 0-255 Total | ASCII |  |  |  |  | Site Ab Bin 0-255 Total CSV Format | ✓ |  |
| 16202 | SV | Site Ac Bin 0-255 Total | ASCII |  |  |  |  | Site Ac Bin 0-255 Total CSV Format | ✓ |  |
| 16203 | SV | Site Ad Bin 0-255 Total | ASCII |  |  |  |  | Site Ad Bin 0-255 Total CSV Format | ✓ |  |
| 16204 | SV | Site Ae Bin 0-255 Total | ASCII |  |  |  |  | Site Ae Bin 0-255 Total CSV Format | ✓ |  |
| 16205 | SV | Site Af Bin 0-255 Total | ASCII |  |  |  |  | Site Af Bin 0-255 Total CSV Format | ✓ |  |
| 16206 | SV | Site Ag Bin 0-255 Total | ASCII |  |  |  |  | Site Ag Bin 0-255 Total CSV Format | ✓ |  |
| 16207 | SV | Site Ah Bin 0-255 Total | ASCII |  |  |  |  | Site Ah Bin 0-255 Total CSV Format | ✓ |  |
| 16208 | SV | Site Ba Bin 0-255 Total | ASCII |  |  |  |  | Site Ba Bin 0-255 Total CSV Format | ✓ |  |
| 16209 | SV | Site Bb Bin 0-255 Total | ASCII |  |  |  |  | Site Bb Bin 0-255 Total CSV Format | ✓ |  |
| 16210 | SV | Site Bc Bin 0-255 Total | ASCII |  |  |  |  | Site Bc Bin 0-255 Total CSV Format | ✓ |  |
| 16211 | SV | Site Bd Bin 0-255 Total | ASCII |  |  |  |  | Site Bd Bin 0-255 Total CSV Format | ✓ |  |
| 16212 | SV | Site Be Bin 0-255 Total | ASCII |  |  |  |  | Site Be Bin 0-255 Total CSV Format | ✓ |  |
| 16213 | SV | Site Bf Bin 0-255 Total | ASCII |  |  |  |  | Site Bf Bin 0-255 Total CSV Format | ✓ |  |
| 16214 | SV | Site Bg Bin 0-255 Total | ASCII |  |  |  |  | Site Bg Bin 0-255 Total CSV Format | ✓ |  |
| 16215 | SV | Site Bh Bin 0-255 Total | ASCII |  |  |  |  | Site Bh Bin 0-255 Total CSV Format | ✓ |  |
| 16216 | SV | Site Ca Bin 0-255 Total | ASCII |  |  |  |  | Site Ca Bin 0-255 Total CSV Format | ✓ |  |
| 16217 | SV | Site Cb Bin 0-255 Total | ASCII |  |  |  |  | Site Cb Bin 0-255 Total CSV Format | ✓ |  |
| 16218 | SV | Site Cc Bin 0-255 Total | ASCII |  |  |  |  | Site Cc Bin 0-255 Total CSV Format | ✓ |  |
| 16219 | SV | Site Cd Bin 0-255 Total | ASCII |  |  |  |  | Site Cd Bin 0-255 Total CSV Format | ✓ |  |
| 16220 | SV | Site Ce Bin 0-255 Total | ASCII |  |  |  |  | Site Ce Bin 0-255 Total CSV Format | ✓ |  |
| 16221 | SV | Site Cf Bin 0-255 Total | ASCII |  |  |  |  | Site Cf Bin 0-255 Total CSV Format | ✓ |  |
| 16222 | SV | Site Cg Bin 0-255 Total | ASCII |  |  |  |  | Site Cg Bin 0-255 Total CSV Format | ✓ |  |
| 16223 | SV | Site Ch Bin 0-255 Total | ASCII |  |  |  |  | Site Ch Bin 0-255 Total CSV Format | ✓ |  |
| 16224 | SV | Site Da Bin 0-255 Total | ASCII |  |  |  |  | Site Da Bin 0-255 Total CSV Format | ✓ |  |
| 16225 | SV | Site Db Bin 0-255 Total | ASCII |  |  |  |  | Site Db Bin 0-255 Total CSV Format | ✓ |  |
| 16226 | SV | Site Dc Bin 0-255 Total | ASCII |  |  |  |  | Site Dc Bin 0-255 Total CSV Format | ✓ |  |
| 16227 | SV | Site Dd Bin 0-255 Total | ASCII |  |  |  |  | Site Dd Bin 0-255 Total CSV Format | ✓ |  |
| 16228 | SV | Site De Bin 0-255 Total | ASCII |  |  |  |  | Site De Bin 0-255 Total CSV Format | ✓ |  |
| 16229 | SV | Site Df Bin 0-255 Total | ASCII |  |  |  |  | Site Df Bin 0-255 Total CSV Format | ✓ |  |
| 16230 | SV | Site Dg Bin 0-255 Total | ASCII |  |  |  |  | Site Dg Bin 0-255 Total CSV Format | ✓ |  |
| 16231 | SV | Site Dh Bin 0-255 Total | ASCII |  |  |  |  | Site Dh Bin 0-255 Total CSV Format | ✓ |  |
| 16232 | SV | Arm 1 Site Aa Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Aa Bin 0-255 Total CSV Format | ✓ |  |
| 16233 | SV | Arm 1 Site Ab Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Ab Bin 0-255 Total CSV Format | ✓ |  |
| 16234 | SV | Arm 1 Site Ac Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Ac Bin 0-255 Total CSV Format | ✓ |  |
| 16235 | SV | Arm 1 Site Ad Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Ad Bin 0-255 Total CSV Format | ✓ |  |
| 16236 | SV | Arm 1 Site Ae Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Ae Bin 0-255 Total CSV Format | ✓ |  |
| 16237 | SV | Arm 1 Site Af Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Af Bin 0-255 Total CSV Format | ✓ |  |
| 16238 | SV | Arm 1 Site Ag Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Ag Bin 0-255 Total CSV Format | ✓ |  |
| 16239 | SV | Arm 1 Site Ah Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Ah Bin 0-255 Total CSV Format | ✓ |  |
| 16240 | SV | Arm 1 Site Ba Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Ba Bin 0-255 Total CSV Format | ✓ |  |
| 16241 | SV | Arm 1 Site Bb Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Bb Bin 0-255 Total CSV Format | ✓ |  |
| 16242 | SV | Arm 1 Site Bc Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Bc Bin 0-255 Total CSV Format | ✓ |  |
| 16243 | SV | Arm 1 Site Bd Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Bd Bin 0-255 Total CSV Format | ✓ |  |
| 16244 | SV | Arm 1 Site Be Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Be Bin 0-255 Total CSV Format | ✓ |  |
| 16245 | SV | Arm 1 Site Bf Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Bf Bin 0-255 Total CSV Format | ✓ |  |
| 16246 | SV | Arm 1 Site Bg Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Bg Bin 0-255 Total CSV Format | ✓ |  |
| 16247 | SV | Arm 1 Site Bh Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Bh Bin 0-255 Total CSV Format | ✓ |  |
| 16248 | SV | Arm 1 Site Ca Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Ca Bin 0-255 Total CSV Format | ✓ |  |
| 16249 | SV | Arm 1 Site Cb Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Cb Bin 0-255 Total CSV Format | ✓ |  |
| 16250 | SV | Arm 1 Site Cc Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Cc Bin 0-255 Total CSV Format | ✓ |  |
| 16251 | SV | Arm 1 Site Cd Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Cd Bin 0-255 Total CSV Format | ✓ |  |
| 16252 | SV | Arm 1 Site Ce Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Ce Bin 0-255 Total CSV Format | ✓ |  |
| 16253 | SV | Arm 1 Site Cf Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Cf Bin 0-255 Total CSV Format | ✓ |  |
| 16254 | SV | Arm 1 Site Cg Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Cg Bin 0-255 Total CSV Format | ✓ |  |
| 16255 | SV | Arm 1 Site Ch Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Ch Bin 0-255 Total CSV Format | ✓ |  |
| 16256 | SV | Arm 1 Site Da Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Da Bin 0-255 Total CSV Format | ✓ |  |
| 16257 | SV | Arm 1 Site Db Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Db Bin 0-255 Total CSV Format | ✓ |  |
| 16258 | SV | Arm 1 Site Dc Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Dc Bin 0-255 Total CSV Format | ✓ |  |
| 16259 | SV | Arm 1 Site Dd Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Dd Bin 0-255 Total CSV Format | ✓ |  |
| 16260 | SV | Arm 1 Site De Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site De Bin 0-255 Total CSV Format | ✓ |  |
| 16261 | SV | Arm 1 Site Df Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Df Bin 0-255 Total CSV Format | ✓ |  |
| 16262 | SV | Arm 1 Site Dg Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Dg Bin 0-255 Total CSV Format | ✓ |  |
| 16263 | SV | Arm 1 Site Dh Bin 0-255 Total | ASCII |  |  |  |  | Arm 1 Site Dh Bin 0-255 Total CSV Format | ✓ |  |
| 16264 | SV | Arm 2 Site Aa Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Aa Bin 0-255 Total CSV Format | ✓ |  |
| 16265 | SV | Arm 2 Site Ab Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Ab Bin 0-255 Total CSV Format | ✓ |  |
| 16266 | SV | Arm 2 Site Ac Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Ac Bin 0-255 Total CSV Format | ✓ |  |
| 16267 | SV | Arm 2 Site Ad Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Ad Bin 0-255 Total CSV Format | ✓ |  |
| 16268 | SV | Arm 2 Site Ae Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Ae Bin 0-255 Total CSV Format | ✓ |  |
| 16269 | SV | Arm 2 Site Af Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Af Bin 0-255 Total CSV Format | ✓ |  |
| 16270 | SV | Arm 2 Site Ag Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Ag Bin 0-255 Total CSV Format | ✓ |  |
| 16271 | SV | Arm 2 Site Ah Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Ah Bin 0-255 Total CSV Format | ✓ |  |
| 16272 | SV | Arm 2 Site Ba Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Ba Bin 0-255 Total CSV Format | ✓ |  |
| 16273 | SV | Arm 2 Site Bb Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Bb Bin 0-255 Total CSV Format | ✓ |  |
| 16274 | SV | Arm 2 Site Bc Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Bc Bin 0-255 Total CSV Format | ✓ |  |
| 16275 | SV | Arm 2 Site Bd Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Bd Bin 0-255 Total CSV Format | ✓ |  |
| 16276 | SV | Arm 2 Site Be Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Be Bin 0-255 Total CSV Format | ✓ |  |
| 16277 | SV | Arm 2 Site Bf Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Bf Bin 0-255 Total CSV Format | ✓ |  |
| 16278 | SV | Arm 2 Site Bg Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Bg Bin 0-255 Total CSV Format | ✓ |  |
| 16279 | SV | Arm 2 Site Bh Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Bh Bin 0-255 Total CSV Format | ✓ |  |
| 16280 | SV | Arm 2 Site Ca Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Ca Bin 0-255 Total CSV Format | ✓ |  |
| 16281 | SV | Arm 2 Site Cb Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Cb Bin 0-255 Total CSV Format | ✓ |  |
| 16282 | SV | Arm 2 Site Cc Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Cc Bin 0-255 Total CSV Format | ✓ |  |
| 16283 | SV | Arm 2 Site Cd Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Cd Bin 0-255 Total CSV Format | ✓ |  |
| 16284 | SV | Arm 2 Site Ce Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Ce Bin 0-255 Total CSV Format | ✓ |  |
| 16285 | SV | Arm 2 Site Cf Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Cf Bin 0-255 Total CSV Format | ✓ |  |
| 16286 | SV | Arm 2 Site Cg Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Cg Bin 0-255 Total CSV Format | ✓ |  |
| 16287 | SV | Arm 2 Site Ch Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Ch Bin 0-255 Total CSV Format | ✓ |  |
| 16288 | SV | Arm 2 Site Da Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Da Bin 0-255 Total CSV Format | ✓ |  |
| 16289 | SV | Arm 2 Site Db Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Db Bin 0-255 Total CSV Format | ✓ |  |
| 16290 | SV | Arm 2 Site Dc Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Dc Bin 0-255 Total CSV Format | ✓ |  |
| 16291 | SV | Arm 2 Site Dd Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Dd Bin 0-255 Total CSV Format | ✓ |  |
| 16292 | SV | Arm 2 Site De Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site De Bin 0-255 Total CSV Format | ✓ |  |
| 16293 | SV | Arm 2 Site Df Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Df Bin 0-255 Total CSV Format | ✓ |  |
| 16294 | SV | Arm 2 Site Dg Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Dg Bin 0-255 Total CSV Format | ✓ |  |
| 16295 | SV | Arm 2 Site Dh Bin 0-255 Total | ASCII |  |  |  |  | Arm 2 Site Dh Bin 0-255 Total CSV Format | ✓ |  |
| 16296 | SV | Total Pass Count by Site Aa to Dh | ASCII |  |  |  |  | Pass Count by Site Aa to Dh CSV Format | ✓ |  |
| 16297 | SV | Total Fail Count by Site Aa to Dh | ASCII |  |  |  |  | Fail Count by Site Aa to Dh CSV Format | ✓ |  |
| 16298 | SV | Total IF Error Count by Site Aa to Dh | ASCII |  |  |  |  | IF Error Count by Site Aa to Dh CSV Format | ✓ |  |
| 16299 | SV | Total Count by Site Aa to Dh | ASCII |  |  |  |  | Total Count by Site Aa to Dh CSV Format | ✓ |  |
| 16300 | SV | Arm 1 Pass Count by Site Aa to Dh | ASCII |  |  |  |  | Arm 1 Pass Count by Site Aa to Dh CSV Format | ✓ |  |
| 16301 | SV | Arm 1 Fail Count by Site Aa to Dh | ASCII |  |  |  |  | Arm 1 Fail Count by Site Aa to Dh CSV Format | ✓ |  |
| 16302 | SV | Arm 1 IF Error Count by Site Aa to Dh | ASCII |  |  |  |  | Arm 1 IF Error Count by Site Aa to Dh CSV Format | ✓ |  |
| 16303 | SV | Arm 1 Total Count by Site Aa to Dh | ASCII |  |  |  |  | Arm 1 Total Count by Site Aa to Dh CSV Format | ✓ |  |
| 16304 | SV | Arm 2 Pass Count by Site Aa to Dh | ASCII |  |  |  |  | Arm 2 Pass Count by Site Aa to Dh CSV Format | ✓ |  |
| 16305 | SV | Arm 2 Fail Count by Site Aa to Dh | ASCII |  |  |  |  | Arm 2 Fail Count by Site Aa to Dh CSV Format | ✓ |  |
| 16306 | SV | Arm 2 IF Error Count by Site Aa to Dh | ASCII |  |  |  |  | Arm 2 IF Error Count by Site Aa to Dh CSV Format | ✓ |  |
| 16307 | SV | Arm 2 Total Count by Site Aa to Dh | ASCII |  |  |  |  | Arm 2 Total Count by Site Aa to Dh CSV Format | ✓ |  |
| 16308 | SV | Site Aa Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Aa Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16309 | SV | Site Ab Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Ab Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16310 | SV | Site Ac Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Ac Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16311 | SV | Site Ad Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Ad Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16312 | SV | Site Ae Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Ae Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16313 | SV | Site Af Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Af Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16314 | SV | Site Ag Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Ag Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16315 | SV | Site Ah Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Ah Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16316 | SV | Site Ba Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Ba Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16317 | SV | Site Bb Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Bb Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16318 | SV | Site Bc Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Bc Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16319 | SV | Site Bd Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Bd Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16320 | SV | Site Be Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Be Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16321 | SV | Site Bf Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Bf Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16322 | SV | Site Bg Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Bg Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16323 | SV | Site Bh Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Bh Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16324 | SV | Site Ca Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Ca Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16325 | SV | Site Cb Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Cb Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16326 | SV | Site Cc Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Cc Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16327 | SV | Site Cd Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Cd Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16328 | SV | Site Ce Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Ce Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16329 | SV | Site Cf Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Cf Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16330 | SV | Site Cg Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Cg Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16331 | SV | Site Ch Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Ch Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16332 | SV | Site Da Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Da Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16333 | SV | Site Db Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Db Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16334 | SV | Site Dc Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Dc Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16335 | SV | Site Dd Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Dd Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16336 | SV | Site De Bin 0-255 Total (History) | ASCII |  |  |  |  | Site De Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16337 | SV | Site Df Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Df Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16338 | SV | Site Dg Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Dg Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16339 | SV | Site Dh Bin 0-255 Total (History) | ASCII |  |  |  |  | Site Dh Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16340 | SV | Arm 1 Site Aa Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Aa Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16341 | SV | Arm 1 Site Ab Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Ab Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16342 | SV | Arm 1 Site Ac Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Ac Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16343 | SV | Arm 1 Site Ad Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Ad Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16344 | SV | Arm 1 Site Ae Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Ae Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16345 | SV | Arm 1 Site Af Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Af Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16346 | SV | Arm 1 Site Ag Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Ag Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16347 | SV | Arm 1 Site Ah Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Ah Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16348 | SV | Arm 1 Site Ba Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Ba Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16349 | SV | Arm 1 Site Bb Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Bb Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16350 | SV | Arm 1 Site Bc Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Bc Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16351 | SV | Arm 1 Site Bd Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Bd Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16352 | SV | Arm 1 Site Be Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Be Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16353 | SV | Arm 1 Site Bf Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Bf Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16354 | SV | Arm 1 Site Bg Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Bg Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16355 | SV | Arm 1 Site Bh Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Bh Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16356 | SV | Arm 1 Site Ca Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Ca Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16357 | SV | Arm 1 Site Cb Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Cb Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16358 | SV | Arm 1 Site Cc Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Cc Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16359 | SV | Arm 1 Site Cd Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Cd Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16360 | SV | Arm 1 Site Ce Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Ce Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16361 | SV | Arm 1 Site Cf Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Cf Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16362 | SV | Arm 1 Site Cg Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Cg Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16363 | SV | Arm 1 Site Ch Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Ch Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16364 | SV | Arm 1 Site Da Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Da Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16365 | SV | Arm 1 Site Db Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Db Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16366 | SV | Arm 1 Site Dc Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Dc Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16367 | SV | Arm 1 Site Dd Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Dd Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16368 | SV | Arm 1 Site De Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site De Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16369 | SV | Arm 1 Site Df Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Df Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16370 | SV | Arm 1 Site Dg Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Dg Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16371 | SV | Arm 1 Site Dh Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 1 Site Dh Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16372 | SV | Arm 2 Site Aa Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Aa Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16373 | SV | Arm 2 Site Ab Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Ab Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16374 | SV | Arm 2 Site Ac Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Ac Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16375 | SV | Arm 2 Site Ad Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Ad Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16376 | SV | Arm 2 Site Ae Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Ae Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16377 | SV | Arm 2 Site Af Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Af Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16378 | SV | Arm 2 Site Ag Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Ag Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16379 | SV | Arm 2 Site Ah Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Ah Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16380 | SV | Arm 2 Site Ba Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Ba Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16381 | SV | Arm 2 Site Bb Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Bb Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16382 | SV | Arm 2 Site Bc Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Bc Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16383 | SV | Arm 2 Site Bd Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Bd Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16384 | SV | Arm 2 Site Be Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Be Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16385 | SV | Arm 2 Site Bf Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Bf Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16386 | SV | Arm 2 Site Bg Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Bg Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16387 | SV | Arm 2 Site Bh Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Bh Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16388 | SV | Arm 2 Site Ca Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Ca Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16389 | SV | Arm 2 Site Cb Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Cb Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16390 | SV | Arm 2 Site Cc Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Cc Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16391 | SV | Arm 2 Site Cd Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Cd Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16392 | SV | Arm 2 Site Ce Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Ce Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16393 | SV | Arm 2 Site Cf Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Cf Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16394 | SV | Arm 2 Site Cg Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Cg Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16395 | SV | Arm 2 Site Ch Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Ch Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16396 | SV | Arm 2 Site Da Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Da Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16397 | SV | Arm 2 Site Db Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Db Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16398 | SV | Arm 2 Site Dc Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Dc Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16399 | SV | Arm 2 Site Dd Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Dd Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16400 | SV | Arm 2 Site De Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site De Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16401 | SV | Arm 2 Site Df Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Df Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16402 | SV | Arm 2 Site Dg Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Dg Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16403 | SV | Arm 2 Site Dh Bin 0-255 Total (History) | ASCII |  |  |  |  | Arm 2 Site Dh Bin 0-255 Total (History) CSV Format | ✓ |  |
| 16404 | SV | Total Pass Count by Site Aa to Dh (History) | ASCII |  |  |  |  | Pass Count by Site Aa to Dh (History) CSV Format | ✓ |  |
| 16405 | SV | Total Fail Count by Site Aa to Dh (History) | ASCII |  |  |  |  | Fail Count by Site Aa to Dh (History) CSV Format | ✓ |  |
| 16406 | SV | Total IF Error Count by Site Aa to Dh (History) | ASCII |  |  |  |  | IF Error Count by Site Aa to Dh (History) CSV Format | ✓ |  |
| 16407 | SV | Total Count by Site Aa to Dh (History) | ASCII |  |  |  |  | Total Count by Site Aa to Dh (History) CSV Format | ✓ |  |
| 16408 | SV | Arm 1 Pass Count by Site Aa to Dh (History) | ASCII |  |  |  |  | Arm 1 Pass Count by Site Aa to Dh (History) CSV Format | ✓ |  |
| 16409 | SV | Arm 1 Fail Count by Site Aa to Dh (History) | ASCII |  |  |  |  | Arm 1 Fail Count by Site Aa to Dh (History) CSV Format | ✓ |  |
| 16410 | SV | Arm 1 IF Error Count by Site Aa to Dh (History) | ASCII |  |  |  |  | Arm 1 IF Error Count by Site Aa to Dh (History) CSV Format | ✓ |  |
| 16411 | SV | Arm 1 Total Count by Site Aa to Dh (History) | ASCII |  |  |  |  | Arm 1 Total Count by Site Aa to Dh (History) CSV Format | ✓ |  |
| 16412 | SV | Arm 2 Pass Count by Site Aa to Dh (History) | ASCII |  |  |  |  | Arm 2 Pass Count by Site Aa to Dh (History) CSV Format | ✓ |  |
| 16413 | SV | Arm 2 Fail Count by Site Aa to Dh (History) | ASCII |  |  |  |  | Arm 2 Fail Count by Site Aa to Dh (History) CSV Format | ✓ |  |
| 16414 | SV | Arm 2 IF Error Count by Site Aa to Dh (History) | ASCII |  |  |  |  | Arm 2 IF Error Count by Site Aa to Dh (History) CSV Format | ✓ |  |
| 16415 | SV | Arm 2 Total Count by Site Aa to Dh (History) | ASCII |  |  |  |  | Arm 2 Total Count by Site Aa to Dh (History) CSV Format | ✓ |  |

## Lot 歷史 (20000-24999)
> ⚠️ 此區段有 **14** 筆差異

| ID | 類型 | Name | DataType | Unit | Max | Min | Default | Description | Code | 差異 |
|----|------|------|----------|------|-----|-----|---------|-------------|------|------|
| 20001 | SV | EC Change ID | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 20002 | SV | EC Change Origina Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 20003 | SV | EC Change New Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 21023 | SV | Enable Tester Dry Air Control | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 21504 | SV | Setting Tri Temp Out Shuttle Heater Temperature | ASCII |  |  |  |  | Setting Tri Temp Out Shuttle Heater Temperature |  | 🔴 文件獨有 |
| 22505 | SV | Energy Saving Start Time | ASCII |  |  |  |  | ATC Energy Saving Start Time |  | 🔴 文件獨有 |
| 22506 | SV | Energy Saving End Time | ASCII |  |  |  |  | ATC Energy Saving End Time |  | 🔴 文件獨有 |
| 22520 | SV | ATC Energy Saving State | BOOLEAN |  |  |  |  | ATC Energy Saving State |  | 🔴 文件獨有 |
| 22521 | SV | Heater Energy Saving State | BOOLEAN |  |  |  |  | Heater Energy Saving State |  | 🔴 文件獨有 |
| 22522 | SV | Motor Energy Saving State | BOOLEAN |  |  |  |  | Motor Energy Saving State |  | 🔴 文件獨有 |
| 22523 | SV | Tester Purge Kit Energy Saving State | BOOLEAN |  |  |  |  | Tester Purge Kit Energy Saving State |  | 🔴 文件獨有 |
| 22524 | SV | Dry Air Energy Saving State | BOOLEAN |  |  |  |  | Dry Air Energy Saving State |  | 🔴 文件獨有 |
| 22525 | SV | Energy Saving State | BOOLEAN |  |  |  |  | Energy Saving State |  | 🔴 文件獨有 |
| 22526 | SV | Handler Power Measurement Values | ASCII |  |  |  |  | Handler Power Measurement Values |  | 🔴 文件獨有 |

## IO / 數位訊號 (30000-39999)
> ⚠️ 此區段有 **330** 筆差異

| ID | 類型 | Name | DataType | Unit | Max | Min | Default | Description | Code | 差異 |
|----|------|------|----------|------|-----|-----|---------|-------------|------|------|
| 35000 | SV/EC | [A01] Auto switch to operator when idle | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35001 | SV/EC | [A01] Auto switch to operator when idle sec. | INT_4 | Second |  | 10 |  |  | ✓ |  |
| 35002 | SV/EC | [A01_1] Press start auto switch to operator | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35003 | SV/EC | [A03] After Home Suck And Carry Ic To InterFace Error | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35004 | SV/EC | [A04] Loader Magazine ,Tray split fail can skip | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35005 | SV/EC | [A05] Use Auto Docking | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35008 | SV/EC | [A07] Auto Speed | BOOLEAN |  |  |  |  |  |  | 🔴 文件獨有 |
| 35009 | SV/EC | [A08] Loader no tray Cleanout  and Cleanout finish  check again | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35010 | SV/EC | [A09] By Arm Close Site | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35011 | SV/EC | [A10] Auto Retest | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35012 | SV/EC | [A11] Barcode Reader | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35013 | SV/EC | [A11] Barcode Reader over Sec. | INT_4 | Second |  | 10 |  |  | ✓ |  |
| 35014 | EC | [A02] Can select Normal or Prime bin data | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 35015 | SV/EC | [A17] Disable RESET button | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35016 | SV/EC | [A21] Rotate Detect Error Need Shake | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35017 | SV/EC | [A22-2]Show Message and keep running distance | FT_8 | Second | 10 | 0.01 | 0.01 |  | ✓ |  |
| 35018 | SV/EC | [A22-3] Show Message and Stop running | FT_8 | Second | 10 | 0.01 | 0.01 |  | ✓ |  |
| 35019 | SV/EC | [A23] Check 'lot no' in SLT report | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35020 | SV/EC | [A26] Motor Speed Sort Display | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35021 | SV/EC | [A27] Enable Light Scale | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35022 | SV/EC | [A27-1] Log Enable Light Scale Data | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35031 | SV/EC | [A58]  Bundle Info | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[A65] Support Bundle Info` |
| 35032 | EC | [A68] Enable Automatic Loading and Unloading | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 35051 | SV/EC | [C02] Enable CCD | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35052 | SV/EC | [C03] Use CatchTrayFix | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35072 | SV/EC | [C08] Use socket sensor | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35073 | SV/EC | [C09] Jam After Start Car Record | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35074 | SV/EC | [C09] Jam After Start Car Record Delay Time | INT_4 | Second |  |  |  |  | ✓ |  |
| 35075 | SV/EC | [C10] Enable ESD connect error report function | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35100 | SV/EC | [D01] Enable ReadTorque  (1/per ) | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[D01] Enable ReadTorque  (1/per)` |
| 35101 | SV/EC | [D02] Off Read Torque Function During Test | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35103 | SV/EC | [D03] Read Toqrue Count | INT_4 |  |  |  |  |  | ✓ |  |
| 35104 | SV/EC | [D10] Manual height use Z1,Z2 button to UP/Down | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35105 | SV/EC | [D11] If shuttle no device, no need to do auto height. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35106 | SV/EC | [D12] Shuttle auto hieight by setting force value | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35107 | SV/EC | [D21] Enable Finish Test up && wait | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[D21] Enable Finish Test up and wait` |
| 35108 | SV/EC | [D21] Enable Finish Test up && wait Height | INT_4 | 0.01mm | 1000 | 0 |  |  | ✓ | ⚠️ EC名稱: Code=`[D21] Enable Finish Test up and wait Height` |
| 35109 | SV/EC | [D21] Enable Finish Test up && wait Time | INT_4 | MSecond | 100000 | 100 |  |  | ✓ | ⚠️ EC名稱: Code=`[D21] Enable Finish Test up and wait Time` |
| 35110 | SV/EC | [D22] Support Multi Double Contact | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35111 | SV/EC | [D22] Support Multi Double Contact Count | INT_4 |  |  |  |  |  | ✓ |  |
| 35112 | SV/EC | [D24] Enable EP check funtion | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35113 | SV/EC | [D25]EP Load Rate for 60mm | FT_8 |  | 1.5 | 0.5 | 1 |  | ✓ |  |
| 35114 | SV/EC | [D25]EP Load Rate for 40mm | FT_8 |  | 1.5 | 0.5 | 1 |  | ✓ |  |
| 35115 | SV/EC | [D25]EP Load Rate for 30mm | FT_8 |  | 1.5 | 0.5 | 1 |  | ✓ |  |
| 35116 | SV/EC | [D26] Enable EP Encoder Range | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35117 | SV/EC | [D26] EP Encoder Range | INT_4 |  |  |  |  |  | ✓ |  |
| 35118 | SV/EC | [D27] Enable single site 85 kg | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35119 | SV/EC | [D28] Double Contact No Need Re-Contact | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[D22-2] Double Contact No Need Re-Contact` |
| 35120 | SV/EC | [D29] Stable Contact Mode | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35121 | SV/EC | [D30]  Enable Site Mode Select | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35122 | SV/EC | [D31]  RTC Change Recipe Need reCreate RTC Model | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35123 | SV/EC | [D32] Tray pitch > 35mm, the counter air on time must >0.5 Sec. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35124 | SV/EC | [D33]  RTC Initial Start Need Verify | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35125 | SV/EC | [D34] Enable Galil Protection Function. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35126 | SV/EC | [D40] IC miss of testhead, skip enabled after Z1 pressed | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35127 | SV/EC | [D41] Test Socket IC check Skip | BOOLEAN |  |  |  |  |  |  | 🔴 文件獨有 |
| 35128 | SV/EC | [D41] Test Socket IC check Skip Position | INT_4 |  | 1 | 0 | 1 | 0 : Inside socket 1 : Above Socket | ✓ |  |
| 35129 | SV/EC | [D41] Test Socket IC check Skip Height | FT_4 | mm | 10 | 0 | 3 |  | ✓ |  |
| 35130 | SV/EC | [D42] Index && Shuttle jam need pause | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[D42] Index and Shuttle jam need pause` |
| 35131 | SV/EC | [D43] Index && Shuttle jam can retry or skip | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[D43] Index and Shuttle jam can retry or skip` |
| 35132 | SV/EC | [D44] Check vacumn after test head purge | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35133 | SV/EC | [D44] Check vacumn after test head purge time | INT_4 | MSecond | 100000 | 100 |  |  | ✓ |  |
| 35134 | SV/EC | [D45] CheckIndex | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35135 | SV/EC | [D46] Index Destroy Delay | INT_4 | Second |  |  |  |  | ✓ |  |
| 35139 | SV/EC | [D48] Disable Z1,Z2 function when power off,or EMG press | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35140 | SV/EC | [D51] OneCycle and Clean Out finish Test Arm at rear position | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35141 | SV/EC | [D52]Before Show Iterrface Error Test Head Need  Up | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35142 | SV/EC | [D53] Index Light Always On | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35143 | SV/EC | [D53] Index Light Off Delay Time | INT_4 | Min |  |  |  |  | ✓ |  |
| 35144 | SV/EC | [D54]Index Pick && Place Shuttle Slow Down | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[D54]Index Pick and Place Shuttle Slow Down` |
| 35145 | SV/EC | [D54]Index Pick && Place Shuttle Slow Down Speed | INT_4 |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[D54]Index Pick and Place Shuttle Slow Down Speed` |
| 35146 | SV/EC | [D55] When RTC Enabled, Disable Index Check. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35147 | SV/EC | [D56] Forced Enable Piggy-Back Function | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35148 | SV/EC | [D13] Check index home sensor after find home | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35149 | SV/EC | [D14] Auto height use setting torque (For > 300KG Model) | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35150 | SV/EC | [D14] Auto height use setting torque value | INT_4 |  | 30 | 10 | 15 |  | ✓ |  |
| 35151 | SV/EC | [D15] Continuous auto contact test | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35152 | SV/EC | [D16] Step by step contact test | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35153 | SV/EC | [D17] Get hardware height in high calbration. | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[D17] Get hardware height in high calbration` / ⚠️ EC型別: Code=INT_4 |
| 35154 | SV/EC | [D23] Every device do multi contact before test | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35155 | SV/EC | [D23] Every device do multi contact before test. | INT_4 |  | 30 | 2 | 2 |  | ✓ | ⚠️ EC名稱: Code=`[D23] Every device do multi contact before test Count` |
| 35156 | SV/EC | [D26] Enable EP log | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35157 | SV/EC | [D26] Show EP encoder | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35158 | SV/EC | [D38] Index release device to shuttle no wait motion | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35159 | SV/EC | [D43] Enable auto retry when index pick up error | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[D43-1] Enable auto retry when index pick up error` |
| 35160 | SV/EC | [D43] Check vacuum in socket when index pick up error | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[D43-2] Check vacuum in socket when index pick up error` |
| 35161 | SV/EC | [D44] Check open 4 Site check | BOOLEAN |  |  |  |  |  |  | 🔴 文件獨有 |
| 35162 | SV/EC | [D57] Close site need display chanel number | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35163 | SV/EC | [D58] Arm 1 for pick and place,  Arm 2 for testing | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35164 | SV/EC | [D61] encountered  Vacuum sensor OFF error, must do piggyback check | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35165 | EC | [D62] Pick up shuttle error, need to purge one time | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 35166 | SV/EC | [D64] Pick up shuttle error, only SKIP | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35167 | SV/EC | [D65] Enable check socket sensor function | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35176 | EC | [D74] RTC Auto Tuning | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 35200 | SV/EC | [E30] In Arm Use Different Scale | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35201 | SV/EC | [E30] In Arm Use Different Scale Loader X | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35202 | SV/EC | [E30] In Arm Use Different Scale Loader Y | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35203 | SV/EC | [E30] In Arm Use Different Scale Hot Plate 1 X | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35204 | SV/EC | [E30] In Arm Use Different Scale Hot Plate 1 Y | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35205 | SV/EC | [E30] In Arm Use Different Scale Hot Plate 2 X | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35206 | SV/EC | [E30] In Arm Use Different Scale Hot Plate 2 Y | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35207 | SV/EC | [E31] Out Arm Use Different Scale | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35208 | SV/EC | [E31] Out Arm Use Different Scale Auto 1 X | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35209 | SV/EC | [E31] Out Arm Use Different Scale Auto 1 Y | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35210 | SV/EC | [E31] Out Arm Use Different Scale Auto 2 X | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35211 | SV/EC | [E31] Out Arm Use Different Scale Auto 2 Y | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35212 | SV/EC | [E31] Out Arm Use Different Scale Auto 3 X | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35213 | SV/EC | [E31] Out Arm Use Different Scale Auto 3 Y | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35214 | SV/EC | [E31] Out Arm Use Different Scale Fix 1 X | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35215 | SV/EC | [E31] Out Arm Use Different Scale Fix 1 Y | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35216 | SV/EC | [E31] Out Arm Use Different Scale Fix 2 X | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35217 | SV/EC | [E31] Out Arm Use Different Scale Fix 2 Y | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35218 | SV/EC | [E31] Out Arm Use Different Scale Fix 3 X | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35219 | SV/EC | [E31] Out Arm Use Different Scale Fix 3 Y | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35220 | SV/EC | [E32] Shuttle Use Different Scale | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35221 | SV/EC | [E32] Shuttle Use Different Scale In Shuttle 1 X | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35222 | SV/EC | [E32] Shuttle Use Different Scale In Shuttle 1 Y | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35223 | SV/EC | [E32] Shuttle Use Different Scale In Shuttle 2 X | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35224 | SV/EC | [E32] Shuttle Use Different Scale In Shuttle 2 Y | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35225 | SV/EC | [E32] Shuttle Use Different Scale Out Shuttle 1 X | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35226 | SV/EC | [E32] Shuttle Use Different Scale Out Shuttle 1 Y | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35227 | SV/EC | [E32] Shuttle Use Different Scale Out Shuttle 2 X | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35228 | SV/EC | [E32] Shuttle Use Different Scale Out Shuttle 2 Y | FT_8 |  | 1.05 | 0.95 | 1 |  | ✓ |  |
| 35229 | SV/EC | [E33] In && Out Arm Z Offset  same one | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[E33] In and Out Arm Z Offset  same one` |
| 35230 | SV/EC | [E34]In && Out Arm Pitch && Pick/Release Offset  same one | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[E34]In and Out Arm Pitch and Pick/Release Offset  same one` |
| 35231 | SV/EC | [E35]In && Out arm place device disable Vacuum detect Off | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[E35]In and Out arm place device disable Vacuum detect Off` |
| 35232 | SV/EC | [E36] In Arm Y Pitch 60mm Offset | INT_4 | 0.01mm | 100 | -100 | 0 |  | ✓ |  |
| 35233 | SV/EC | [E37] Out Arm Y Pitch 60mm Offset | INT_4 | 0.01mm | 100 | -100 | 0 |  | ✓ |  |
| 35234 | SV/EC | [E38] Check Hot Plate while initial start. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35235 | SV/EC | [E39] Check Hot Plate after clean out and before tray feed. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35236 | SV/EC | [E39-1] Put the devices to Error Bin | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35237 | SV/EC | [E40]  Clear All Hot IC then Pick Load IC | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35238 | SV/EC | [E41] Tray pitch > 35mm, In out arm speed must small than 80%. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35239 | SV/EC | [E42]  OutShuttle Alarm InArm Servo Off | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35240 | SV/EC | [E45] All setup file use one offset data | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35241 | SV/EC | [E46] Loader use 2 offset for each row | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35242 | SV/EC | [E47] Shuttle use 4 offset for each row and col | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35243 | SV/EC | [E48] Auto clean shuttle use 4 offset for each row and col | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35244 | SV/EC | [E49] Loader pick up error only RETRY and CLEAN OUT | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35245 | SV/EC | [E50] Out arm pick up error only RETRY | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC型別: Code=INT_4 |
| 35246 | SV/EC | [E51] In arm Z ADC speed | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35247 | SV/EC | [E51] In arm Z ADC speed value | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC型別: Code=INT_4 |
| 35248 | SV/EC | [E52] Out arm Z ADC speed | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35249 | SV/EC | [E52] Out arm Z ADC speed value | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC型別: Code=INT_4 |
| 35250 | SV/EC | [E53] When low yield do auto clean and close site | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35251 | SV/EC | [E54] Check close site can not have IC when pick from hot plate | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35252 | SV/EC | [E55] Use Fix3 Full Tray Function | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35253 | SV/EC | [E56] When loader have pick up error, to retry at same position | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35254 | SV/EC | [E57] Hot Plate can use another vacuum delay time | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35255 | SV/EC | [E58] InOut Arm Y pitch home  check | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35256 | SV/EC | [E59] Offset file group by [###] | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35257 | SV/EC | [E60] In arm pick from loader drop error auto skip | BOOLEAN |  |  |  |  |  |  | 🔴 文件獨有 |
| 35258 | SV/EC | [E61] In arm standby postion on loader | BOOLEAN |  |  |  |  |  |  | 🔴 文件獨有 |
| 35259 | SV/EC | [E62] Search last row when auto skip count over limit. | BOOLEAN |  |  |  |  |  |  | 🔴 文件獨有 |
| 35260 | SV/EC | [E63] In arm retry to pick up the loader device before alarm takeout tray message. | BOOLEAN |  |  |  |  |  |  | 🔴 文件獨有 |
| 35261 | SV/EC | [E64]Tray pitch > 50mm or Tray  X-Division=1 , in out arm speed must small than 50%. | BOOLEAN |  |  |  |  |  |  | 🔴 文件獨有 |
| 35300 | SV/EC | [F01] Shake shuttle when jam happen. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35301 | SV/EC | [F01] Shake shuttle speed when jam happen. | INT_4 | Persent | 100 | 50 |  |  | ✓ |  |
| 35302 | SV/EC | [F03] Output shuttle skip detect  IC miss | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35303 | SV/EC | [F05] Enable Shuttlet clean function | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35304 | SV/EC | [F05] Enable Shuttlet clean count | INT_4 |  |  |  |  |  | ✓ |  |
| 35305 | SV/EC | [F06] Enable Initial IC Check | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35306 | SV/EC | [F07] Out Shuttle Sensor Detect Mode | INT_4 |  |  |  |  | 1. Detect Have IC;  2. Detect Double Device; | ✓ |  |
| 35307 | SV/EC | [F09] Check IC which first time load | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35308 | SV/EC | [F11] Out Shuttle Use Front Rear Sensor Detect  superfluous IC | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35309 | SV/EC | [F12] Rotate Shuttle need check if the rotation is done. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35310 | SV/EC | [F12] Rotate Shuttle check delay time | FT_8 | Second |  |  |  |  | ✓ |  |
| 35311 | SV/EC | [F13] Rotate Shuttle ADC Speed | INT_4 | Persent | 100 | 10 |  |  | ✓ |  |
| 35312 | SV/EC | [F13] Rotate Shuttle Initial Speed | INT_4 | Persent | 100 | 10 |  |  | ✓ |  |
| 35313 | SV/EC | [F13] Rotate Shuttle High Speed | INT_4 |  | 5000 | 10 |  |  | ✓ |  |
| 35314 | SV/EC | [F14] Knock shuttle when jam happen. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35315 | SV/EC | [F14] Knock shuttle interval | FT_8 | Second |  |  |  |  | ✓ |  |
| 35316 | SV/EC | [F14] Knock shuttle count | INT_4 |  |  |  |  |  | ✓ |  |
| 35317 | SV/EC | [F15] OutShuttle Lose IC Need Password | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35318 | SV/EC | [F16] Check In Shuttle Sensor I/O | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35319 | SV/EC | [F14-1] Knock shuttle first. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35320 | SV/EC | [F14-1] Knock shuttle first interval | FT_8 | Second | 1 | 0.01 | 0.1 |  | ✓ |  |
| 35321 | SV/EC | [F14-1] Knock shuttle first count | INT_4 |  | 100 | 3 | 3 |  | ✓ |  |
| 35322 | SV/EC | [F17] Always shuttle 1 first after one cycle in hot mode | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35323 | SV/EC | [F18] In shuttle product detect | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35324 | SV/EC | [F19] Out shuttle lose IC need do piggyback check | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35325 | SV/EC | [F20] In shuttle product prominent detect | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35326 | SV/EC | [F21] IN/OUT Arm Z motor on home sensor ,shuttle move | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35327 | SV/EC | [I21-9] Site Mapping Fail Bin setting Bin | INT_4 |  | 16 | 2 |  |  | ✓ | ⚠️ EC名稱: Code=`[F22] In shuttle check has device from Index Arm.` / ⚠️ EC型別: Code=BOOLEAN |
| 35328 | SV/EC | [I25] Format for get handler testing arm temperature. | INT_4 |  | 1 | 0 | 0 |  | ✓ | ⚠️ EC名稱: Code=`[F23] Enable shuttle vibration` / ⚠️ EC型別: Code=BOOLEAN |
| 35329 | SV/EC | [F23] Enable shuttle vibration second | INT_4 | 0.1Second | 100 | 20 | 20 |  | ✓ |  |
| 35330 | SV/EC | [F24] Out shuttle lose IC must open index door and push Z1 | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35400 | SV/EC | [G01]  Show Test Rate | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[G01] Show Test Rate` |
| 35401 | SV/EC | [G04]  Show Fail Alarm Count | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[G04] Show Fail Alarm Count` |
| 35402 | SV/EC | [G05]  Show Motor Speed | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[G05] Show Motor Speed` |
| 35403 | SV/EC | [G06] Home push Z1 Start initial | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35404 | SV/EC | [G07] Support multi color for error bin | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35405 | SV/EC | [G08] Show 'Are you sure' message after alarm. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35406 | SV/EC | [G09] Need password when edit site map | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35407 | SV/EC | [G10] Show immediate UPH" | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[G10] Show immediate UPH` |
| 35408 | SV/EC | [G11] ASE Report  record | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35409 | SV/EC | [G12] Contract high manual send test message | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35500 | SV/EC | [I01] Enable Tester Finish After Homing | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35501 | SV/EC | [I04] Enable change bin during pause | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35502 | SV/EC | [I07] Reset GPIB after OneCycle or CleanOut | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35503 | SV/EC | [I12] Tester time out ,don't send START signal again | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35504 | SV/EC | [I16] Use New TTL Board | BOOLEAN |  |  |  |  |  |  | 🔴 文件獨有 |
| 35505 | SV/EC | [I18] Can Receive ECHOSTOP | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35506 | SV/EC | [I20] The alphabet of the error bin. | INT_4 |  | 5 | 0 |  | 0 : 0;  1 : 16;  2 : E;  3 : Err;  4 : Error;  5 : Accroding to Tester; | ✓ |  |
| 35507 | SV/EC | [I21] Enable Auto Site Mapping function | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35508 | SV/EC | [I21] Skip Soak Time | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35509 | SV/EC | [I21] Use Same Soak Time | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35510 | SV/EC | [I21] Soak Time | FT_8 |  | 360 | 0 |  |  | ✓ |  |
| 35511 | SV/EC | [I21] Check every dut should be open. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35512 | SV/EC | [I21] Remove Loader Tray  Manually. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35513 | SV/EC | [I22] Test time out can use 'SKIP' | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35514 | SV/EC | [I23] Hot Test Waiting Mode | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35515 | SV/EC | [I24] Testing Stop All Motor | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35516 | SV/EC | [I02] After home,set socket IC to error bin | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35517 | SV/EC | [I03] Enable temperature control function | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35518 | SV/EC | [I08] Check 2DID function when initial start. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35519 | SV/EC | [I13] Initial start delay fuinction setting different when FT and RT. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35520 | SV/EC | [I16] TTL setting save to setup file. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35521 | SV/EC | [I18] Can receive ECHOSTOP | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35522 | SV/EC | [I19] Auto site map pause change SIMULATE test bin data | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35523 | SV/EC | [I21-6] Run time check | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35524 | SV/EC | [I21-7] Bin IC combine place to Fix 2 | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35525 | SV/EC | [I21-8] Auto Site Mapping  Use Hotplate | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35526 | SV/EC | [I21-9] Enable Site Mapping Fail Bin setting | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35527 | EC | [I21-9] Site Mapping Fail Bin setting Bin | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 35528 | EC | [I25] Format for get handler testing arm temperature. | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 35529 | SV/EC | [I26] Close site have bin data need manual remove device. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35530 | SV/EC | [I27] Manual sort mode | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35531 | SV/EC | [I28] On off sites on the fly | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35532 | SV/EC | [I29] Enable yield record | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35533 | SV/EC | [I29] Enable yield record second | FT_8 | Second | 3000 | 1 |  |  | ✓ |  |
| 35534 | SV/EC | [I29-1] Save yield data by socket by bin | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35535 | SV/EC | [I30] Reset bin fail count number over. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35536 | SV/EC | [I31-1] GPIB Lot End Command | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35537 | SV/EC | [I31-2] GPIB Lot Start Command | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35538 | SV/EC | [I31-3] GPIB Reset Command | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35539 | SV/EC | [I32] Disable error bin setting. Error devices should take out manually. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35540 | SV/EC | [I33] Enable error bin box setting. Error devices put to bin box. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35541 | SV/EC | [I34] In the test relust all site are specific fail bin, show alarm. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35542 | SV/EC | [I35] Use Third Test Site (Engineer Access) | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35543 | SV/EC | [I36] Tester timer out, manual take out on arm device. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35544 | SV/EC | [I37-1] Enable  FIFO mode. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35545 | SV/EC | [I37-2] Enable site order link. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35546 | SV/EC | [I37-3] Lock loader sort direction | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35548 | SV/EC | [I38] SETTEMP? Respond Settemp +25.0.. Data. | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[I38] Format of SETTEMP?` / ⚠️ EC型別: Code=INT_4 |
| 35549 | SV/EC | [I39] Enabled Spirox Lot End and Full lot end command. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35550 | SV/EC | [I40] Operator mode ->ON line | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35551 | EC | [I06] Turn on the [I01] function after home is complete | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 35600 | EC | [L04] Temperature range | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 35601 | EC | [L05] Chamber temperature range | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 35602 | EC | [L06] Ambient temperature range | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 35603 | EC | [L07] Use single limit | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 35604 | SV/EC | [L08] Socket temperature over range | INT_4 |  | 30 | 2 |  |  | ✓ |  |
| 35605 | SV/EC | [L08] Socket temperature under range | INT_4 |  | 30 | 2 |  |  | ✓ |  |
| 35606 | SV/EC | [L09] Tempture Position Shift Shuttle 1 Left | INT_4 |  |  |  |  |  | ✓ |  |
| 35607 | SV/EC | [L09] Tempture Position Shift Shuttle 1 Right | INT_4 |  |  |  |  |  | ✓ |  |
| 35608 | SV/EC | [L09] Tempture Position Shift Shuttle 2 Left | INT_4 |  |  |  |  |  | ✓ |  |
| 35609 | SV/EC | [L09] Tempture Position Shift Shuttle 2 Right | INT_4 |  |  |  |  |  | ✓ |  |
| 35610 | SV/EC | [L10] Temperature record Interval | INT_4 |  | 5 | 0 |  | 0 : 5  Sec;  1 : 15 Sec;  2 : 30 Sec;  3 : 1  Min;  4 : 5  Min;  5 : 10 Min; | ✓ |  |
| 35611 | SV/EC | [L11-1] Use ATC temperature range | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35612 | SV/EC | [L11-1] ATC temperature range | INT_4 |  | 30 | 2 |  |  | ✓ |  |
| 35613 | SV/EC | [L11-1] ATC temperature range check interval | FT_8 | Second |  |  |  |  | ✓ |  |
| 35614 | SV/EC | [L11-2] Enable Chiller Auto Close Protected. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35615 | SV/EC | [L11-2] Enable Chiller Auto Close Delay Time | INT_4 | Min |  |  |  |  | ✓ |  |
| 35616 | SV/EC | [L11-3] ATC Run Ambient  Temperature | FT_8 |  | 50 | 10 |  |  | ✓ | ⚠️ EC名稱: Code=`[L11-3] ATC Run Ambient Temperature` |
| 35617 | SV/EC | [L12]  Temp Error No Close Heater | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35618 | SV/EC | [L13] Hot Plate and Shuttle Use One Temperature Offset | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35620 | SV/EC | [L15] Chamber mode even blow need wait initial wait time(Temp Form) | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35621 | SV/EC | [L03] Enable Socket Air Cooling contact count trun on | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35622 | SV/EC | [L03] Socket Air Cooling contact count trun on count | INT_4 |  | 50 | 2 |  |  | ✓ |  |
| 35623 | SV/EC | [L10] Enable Index Test logTemperature | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35624 | SV/EC | [L11-4] ATC max temperature limit | INT_4 |  | 130 | 2 | 130 |  | ✓ |  |
| 35625 | SV/EC | [L11-6] Enable ATC Temperature continuous outside Alarm | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35626 | SV/EC | [L11-6] ATC Temperature outside range | INT_4 |  | 120 | 1 | 2 |  | ✓ |  |
| 35627 | SV/EC | [L11-6] ATC Temperature outside continous second | INT_4 |  | 120 | 1 | 2 |  | ✓ |  |
| 35628 | SV/EC | [L11-7] Enable ATC max peak alarm | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35629 | SV/EC | [L11-7] ATC max peak value | INT_4 |  | 120 | 1 | 2 |  | ✓ |  |
| 35630 | SV/EC | [L11-8] Use temperature difference over setting alarm | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35631 | SV/EC | [L17] Turn on all head heater when close site. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35632 | SV/EC | [L18] No full site add temperature offset | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35633 | SV/EC | [L19] Keep heating when chamber door open without chamber heat | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35634 | SV/EC | [L20] Abiemt guard band check | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35635 | SV/EC | [L21] Power OFF open chamber door, break all temperature power. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35700 | SV/EC | [O01]  [ RESET ] Clear and auto check Hot Plate matrix | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[O01] [ RESET ] Clear and auto check Hot Plate matrix` |
| 35701 | SV/EC | [O02]  [ RESET ] do not need clear hot plate | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[O02] [ RESET ] do not need clear hot plate` |
| 35702 | SV/EC | [O05]  [ RESET ] Need remove all Tray on the Handler | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[O05] [ RESET ] Need remove all Tray on the Handler` |
| 35703 | SV/EC | [O06] Enable Save Event log | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35704 | SV/EC | [O06] Event log Path | ASCII |  |  |  |  |  | ✓ |  |
| 35705 | SV/EC | [O06] Enable Save Alarm Histroy | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35706 | SV/EC | [O06] Alarm Histroy Path | ASCII |  |  |  |  |  | ✓ |  |
| 35707 | SV/EC | [O06] Enable Save Alarm Statist | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35708 | SV/EC | [O06] Alarm Statist Path | ASCII |  |  |  |  |  | ✓ |  |
| 35709 | SV/EC | [O06] Last  Record Date Time | ASCII |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[O06] Last Record Date Time` |
| 35710 | SV/EC | [O06] Use Net Drive | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35711 | SV/EC | [O06] Remote Directory | ASCII |  |  |  |  |  | ✓ |  |
| 35712 | SV/EC | [O06] Local Directory | ASCII |  |  |  |  |  | ✓ |  |
| 35713 | SV/EC | [O06] Auto Save Log Sunday | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35714 | SV/EC | [O06] Auto Save Log Monday | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35715 | SV/EC | [O06] Auto Save Log Tuesday | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35716 | SV/EC | [O06] Auto Save Log Wednesday | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35717 | SV/EC | [O06] Auto Save Log Thursday | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35718 | SV/EC | [O06] Auto Save Log Friday | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35719 | SV/EC | [O06] Auto Save Log Saturday | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35720 | SV/EC | [O07]  [FT ]Can't Off continue | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[O07] [FT ]Can't Off continue` |
| 35721 | SV/EC | [O07]  [FT ]Can't Off continue Max Count | INT_4 |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[O07] [FT ]Can't Off continue Max Count` |
| 35723 | SV/EC | [O09] Initial start need ask | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35724 | SV/EC | [O11] Enable Record Jam Rate By Time | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35725 | SV/EC | [O11] Record Jam Rate By Time minutes | INT_4 |  | 999999 | 1 |  |  | ✓ |  |
| 35726 | SV/EC | [O12] Use Head Condition1  Life Time Control | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35727 | SV/EC | [O12] Use Head Condition1  Life Time Control ContactConditionName | ASCII |  |  |  |  |  | ✓ |  |
| 35728 | SV/EC | [O13] Use Head Condition1  Life Time Control | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35729 | SV/EC | [O13] Use Head Condition1  Life Time Control ContactConditionName | ASCII |  |  |  |  |  | ✓ |  |
| 35730 | SV/EC | [O14] Use Head Condition1  Life Time Control | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35731 | SV/EC | [O14] Use Head Condition1  Life Time Control ContactConditionName | ASCII |  |  |  |  |  | ✓ |  |
| 35732 | SV/EC | [O15-1] Saving file period | INT_4 |  | 6 | 0 | 3 |  | ✓ |  |
| 35733 | SV/EC | [O15-2] File name include machine ID | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35800 | SV/EC | [N05] Enable Automation connection | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35801 | SV/EC | [N05] Automation Upload Path | ASCII |  |  |  |  |  | ✓ |  |
| 35802 | SV/EC | [N05] Automation Download Path | ASCII |  |  |  |  |  | ✓ |  |
| 35803 | SV/EC | [N05] Enable Automation Check File | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35804 | SV/EC | [N05] Ambient Temperaure Define | FT_8 |  |  |  |  |  | ✓ |  |
| 35805 | SV/EC | [N06] Enable FTP | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35806 | SV/EC | [N06] FTP User Name | ASCII |  |  |  |  |  | ✓ |  |
| 35807 | SV/EC | [N06] FTP Password | ASCII |  |  |  |  |  | ✓ |  |
| 35808 | SV/EC | [N06] FTP Host | ASCII |  |  |  |  |  | ✓ |  |
| 35809 | SV/EC | [N06] FTP Download Path | ASCII |  |  |  |  |  | ✓ |  |
| 35810 | SV/EC | [N06] FTP Upload Path | ASCII |  |  |  |  |  | ✓ |  |
| 35811 | SV/EC | [N06] FTP Tester Map | ASCII |  |  |  |  |  | ✓ |  |
| 35812 | SV/EC | [N06] FTP List | ASCII |  |  |  |  |  | ✓ |  |
| 35813 | SV/EC | [N06] FTP HD level setup | INT_4 |  | 3 | 0 |  | 0 : Operator;  1 : Engineer;  2 : Supervisor;  3 : HonPrec; | ✓ |  |
| 35814 | SV/EC | [N06] FTP Server level setup | INT_4 |  | 3 | 0 |  | 0 : Operator;  1 : Engineer;  2 : Supervisor;  3 : HonPrec; | ✓ |  |
| 35815 | SV/EC | [N06] Use System Call to UnZip | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35816 | SV/EC | [N07-2] Enable Host Control Start | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35817 | SV/EC | [N07-2] Start Run Check After Sec  Alarm | INT_4 |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[N07-2] Start Run Check After Sec Alarm` |
| 35818 | SV/EC | [N07-3] Enable Secs Gem Disable One Cycle | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35819 | SV/EC | [N10-1] Enable temperature && EP && ESD log upload to server. | BOOLEAN |  |  |  |  |  | ✓ | ⚠️ EC名稱: Code=`[N10-1] Enable temperature and EP and ESD log upload to server.` |
| 35820 | SV/EC | [N10-2] Enable upload summary to server | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35821 | SV/EC | [N10-3] Enable daily upload production status to server | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35822 | SV/EC | [N10-3-1] Upload Time Priod Method | INT_4 |  | 1 | 0 |  | 0:00:00~24:00 1:08:00~20:00 | ✓ |  |
| 35823 | SV/EC | [N10-4] Upload Method | INT_4 |  |  |  |  |  | ✓ |  |
| 35824 | SV/EC | [N10-5] FTP Setting User Name | ASCII |  |  |  |  |  | ✓ |  |
| 35825 | SV/EC | [N10-5] FTP Setting Password | ASCII |  |  |  |  |  | ✓ |  |
| 35826 | SV/EC | [N10-5] FTP Setting Host | ASCII |  |  |  |  |  | ✓ |  |
| 35827 | SV/EC | [N10-5] FTP Setting Upload Path | ASCII |  |  |  |  |  | ✓ |  |
| 35828 | SV/EC | [N10-6] Interval time for upload to host second | INT_4 |  | 3600 | 5 | 5 |  | ✓ |  |
| 35829 | SV/EC | [N10-7] Upload date folder type | INT_4 |  | 2 | 0 | 0 |  | ✓ |  |
| 35830 | SV/EC | [N10-8] Net derive path | ASCII |  |  |  |  |  | ✓ |  |
| 35831 | SV/EC | [N16] Offset FTP Enable Offset FTP | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35832 | SV/EC | [N16] Offset FTP User Name | ASCII |  |  |  |  |  | ✓ |  |
| 35833 | SV/EC | [N16] Offset FTP Password | ASCII |  |  |  |  |  | ✓ |  |
| 35834 | SV/EC | [N16] Offset FTP Host | ASCII |  |  |  |  |  | ✓ |  |
| 35835 | SV/EC | [N16] Offset FTP Download Path | ASCII |  |  |  |  |  | ✓ |  |
| 35836 | SV/EC | [N16] Offset FTP Upload Path | ASCII |  |  |  |  |  | ✓ |  |
| 35855 | SV/EC | [N22-1] Enable FTP Function | BOOLEAN |  |  |  |  |  |  | 🔴 文件獨有 |
| 35856 | SV/EC | [N22] FTP User Name | ASCII |  |  |  |  |  |  | 🔴 文件獨有 |
| 35857 | SV/EC | [N22] FTP Password | ASCII |  |  |  |  |  |  | 🔴 文件獨有 |
| 35858 | SV/EC | [N22] FTP Host | ASCII |  |  |  |  |  |  | 🔴 文件獨有 |
| 35859 | SV/EC | [N22] FTP Upload Path | ASCII |  |  |  |  |  |  | 🔴 文件獨有 |
| 35860 | SV/EC | [N22-3] Upload JHT format log | BOOLEAN |  |  |  |  |  |  | 🔴 文件獨有 |
| 35861 | SV/EC | [N22-3] FTP User Name | ASCII |  |  |  |  |  |  | 🔴 文件獨有 |
| 35862 | SV/EC | [N22-3] FTP Password | ASCII |  |  |  |  |  |  | 🔴 文件獨有 |
| 35863 | SV/EC | [N22-3] FTP Host | ASCII |  |  |  |  |  |  | 🔴 文件獨有 |
| 35864 | SV/EC | [N22-3] FTP Upload Path | ASCII |  |  |  |  |  |  | 🔴 文件獨有 |
| 35865 | EC | [N35] Upload JHT format log | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 35866 | EC | [N35] FTP User Name | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 35867 | EC | [N35] FTP Password | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 35868 | EC | [N35] FTP Host | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 35869 | EC | [N35] FTP Upload Path | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 35900 | SV/EC | [P04] Color stage regard as empty tray unloader to use. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35901 | SV/EC | [P10] Fixed Tary loads new Tary need propose the initial Question. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35902 | SV/EC | [P13] Enable Auto Tray Edge Push Cylinder Loop Function. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35903 | SV/EC | [P13-1] Loop Delay Time | INT_4 | 0.1Sec |  |  |  |  | ✓ |  |
| 35904 | SV/EC | [P14] Enable Auto Tray  Receive Delay Function. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35905 | SV/EC | [P14-1] Delay Edge Push Cylinder Count | INT_4 |  |  |  |  |  | ✓ |  |
| 35906 | SV/EC | [P14-2] Receive Loop Delay Time | INT_4 | 0.1Sec |  |  |  |  | ✓ |  |
| 35907 | SV/EC | [P15] Unload Tray Cylinder free when Open door | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35908 | SV/EC | [P16] Enable Hotplate Edge Push Cylinder Loop Function. | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35909 | SV/EC | [P16-1] Loop Delay Time | INT_4 |  |  |  |  |  | ✓ |  |
| 35910 | SV/EC | [P17] In arm picker must wait loader tray. (full pick up) | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35911 | SV/EC | [P18] Autotray is fail bin must manual put Tray | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35912 | SV/EC | [P19] Catch Tray goes up then check if had catched tray | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35913 | SV/EC | [P20] Must Manual Clear Fix Tray After Initial Start | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35914 | SV/EC | [P21] TrayFeed Take out Fix Tray | BOOLEAN |  |  |  |  |  | ✓ |  |
| 35915 | SV | USE BARCODE MODE | INT_4 |  |  |  |  | USE BARCODE MODE | ✓ |  |
| 35916 | EC | Enable Bar Code Function | BOOLEAN |  |  |  |  | Enable Bar Code Function | ✓ |  |
| 35917 | EC | [P25] Empty or Color tray no supple Auto 1 2 3,no load one tray | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 35930 | SV/EC | Enable TrayID 2 Function | BOOLEAN |  |  |  |  | Enable TrayID 2 Function |  | 🔴 文件獨有 |
| 36165 | SV/EC | [D62] Pick up shuttle error, need to purge one time | BOOLEAN |  |  |  |  |  |  | 🔴 文件獨有 |
| 37000 | SV | Enable LoaderTrayCount_ART | BOOLEAN |  |  |  |  | Is handler in ART mode | ✓ |  |
| 37001 | SV | LoaderTrayCount_ART | INT_4 |  |  |  |  | The current amount of Loader tray in ART mode. | ✓ |  |
| 37002 | SV | AutoRetestCount_ART | INT_4 |  |  |  |  | Auto Retest Count | ✓ |  |
| 37003 | SV | UnloaderTrayCount_ART_Auto1 | INT_4 |  |  |  |  | Auto 1 tray counts (Only calculate the amount that set to retest). | ✓ |  |
| 37004 | SV | UnloaderTrayCount_ART_Auto2 | INT_4 |  |  |  |  | Auto 2 tray counts (Only calculate the amount that set to retest). | ✓ |  |
| 37005 | SV | UnloaderTrayCount_ART_Auto3 | INT_4 |  |  |  |  | Auto 3 tray counts (Only calculate the amount that set to retest). | ✓ |  |
| 37006 | SV | InputLoaderCount | INT_4 |  |  |  |  | Unused at present. | ✓ | ⚠️ SV名稱: Code=`Input Loader Count` |
| 37007 | SV/EC | LoaderTotalTray | INT_4 |  |  |  |  | The tray amount in Load Magazine. | ✓ |  |
| 37008 | SV | Enter Barcode Reader | ASCII |  |  |  |  | Enter Barcode Reader | ✓ |  |
| 37009 | SV | USE AUTO RETEST | BOOLEAN |  |  |  |  | USE AUTO RETEST | ✓ |  |
| 37010 | SV/EC | Enter Skip IC Count |  |  |  |  |  | Enter Skip IC Count | ✓ |  |
| 37011 | SV/EC | HISILICON ESD data reporting interval | INT_4 | Second |  |  |  | ESD Report Even | ✓ | ⚠️ EC型別: Code=FT_8 |
| 37012 | SV | Enter User Password | ASCII |  |  |  |  | Enter User Password | ✓ | ⚠️ SV名稱: Code=`Enter Barcode Reader Password` |
| 37013 | SV | UnloaderTrayCount_ART_Auto4 | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37014 | SV | UnloaderTrayCount_ART_Auto5 | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37015 | SV | UnloaderTrayCount_ART_Auto6 | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37200 | SV | SIMCO Ion fan Data | ASCII |  |  |  |  | SIMCO Ion fan Data | ✓ |  |
| 37201 | SV | SIMCO Ion fan Decay Data | ASCII |  |  |  |  | SIMCO Ion fan Decay Data | ✓ |  |
| 37202 | SV | ATC Software Version | ASCII |  |  |  |  | ATC Software Version | ✓ |  |
| 37203 | SV | GPIB Software Version | ASCII |  |  |  |  | GPIB Software Version | ✓ |  |
| 37204 | SV | ION FAN 1 Status | INT_4 |  |  |  |  | ION FAN 1 Status(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37205 | SV | ION FAN 2 Status | INT_4 |  |  |  |  | ION FAN 2 Status(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37206 | SV | ION FAN 3 Status | INT_4 |  |  |  |  | ION FAN 3 Status(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37207 | SV | ION FAN 4 Status | INT_4 |  |  |  |  | ION FAN 4 Status(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37208 | SV | ION FAN 5 Status | INT_4 |  |  |  |  | ION FAN 5 Status(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37209 | SV | ION FAN 6 Status | INT_4 |  |  |  |  | ION FAN 6 Status(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37210 | SV | ION FAN 7 Status | INT_4 |  |  |  |  | ION FAN 7 Status(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37211 | SV | ION FAN 8 Status | INT_4 |  |  |  |  | ION FAN 8 Status(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37212 | SV | ION FAN 9 Status | INT_4 |  |  |  |  | ION FAN 9 Status(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37213 | SV | ION FAN 10 Status | INT_4 |  |  |  |  | ION FAN 10 Status(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37214 | SV | ION FAN 11 Status | INT_4 |  |  |  |  | ION FAN 11 Status(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37215 | SV | ION FAN 12 Status | INT_4 |  |  |  |  | ION FAN 12 Status(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37216 | SV | ION FAN 1 Power | INT_4 |  |  |  |  | ION FAN 1 Power(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37217 | SV | ION FAN 2 Power | INT_4 |  |  |  |  | ION FAN 2 Power(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37218 | SV | ION FAN 3 Power | INT_4 |  |  |  |  | ION FAN 3 Power(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37219 | SV | ION FAN 4 Power | INT_4 |  |  |  |  | ION FAN 4 Power(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37220 | SV | ION FAN 5 Power | INT_4 |  |  |  |  | ION FAN 5 Power(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37221 | SV | ION FAN 6 Power | INT_4 |  |  |  |  | ION FAN 6 Power(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37222 | SV | ION FAN 7 Power | INT_4 |  |  |  |  | ION FAN 7 Power(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37223 | SV | ION FAN 8 Power | INT_4 |  |  |  |  | ION FAN 8 Power(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37224 | SV | ION FAN 9 Power | INT_4 |  |  |  |  | ION FAN 9 Power(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37225 | SV | ION FAN 10 Power | INT_4 |  |  |  |  | ION FAN 10 Power(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37226 | SV | ION FAN 11 Power | INT_4 |  |  |  |  | ION FAN 11 Power(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37227 | SV | ION FAN 12 Power | INT_4 |  |  |  |  | ION FAN 12 Power(-1：Disabled；0：OFF；1：ON) | ✓ |  |
| 37228 | SV | ESD Software Version | ASCII |  |  |  |  | ESD Software Version | ✓ |  |
| 37230 | SV | Run Mode | INT_4 |  |  |  |  | Run Mode |  | 🔴 文件獨有 |
| 37231 | SV | Map Error Count | INT_4 |  |  |  |  | Map Error Count |  | 🔴 文件獨有 |
| 37233 | SV | Load port read Cassette ID | ASCII |  |  |  |  | Load port read Cassette ID |  | 🔴 文件獨有 |
| 37234 | SV | Buffer6 read Cassette ID | ASCII |  |  |  |  | Buffer6 read Cassette ID |  | 🔴 文件獨有 |
| 37235 | SV | Catch Arm Cassette ID | ASCII |  |  |  |  | Catch Arm Cassette ID |  | 🔴 文件獨有 |
| 37237 | SV | Buffer1 Cassette ID | ASCII |  |  |  |  | Buffer1 Cassette ID |  | 🔴 文件獨有 |
| 37238 | SV | Buffer2 Cassette ID | ASCII |  |  |  |  | Buffer2 Cassette ID |  | 🔴 文件獨有 |
| 37239 | SV | Buffer3 Cassette ID | ASCII |  |  |  |  | Buffer3 Cassette ID |  | 🔴 文件獨有 |
| 37240 | SV | Buffer4 Cassette ID | ASCII |  |  |  |  | Buffer4 Cassette ID |  | 🔴 文件獨有 |
| 37241 | SV | Buffer5 Cassette ID | ASCII |  |  |  |  | Buffer5 Cassette ID |  | 🔴 文件獨有 |
| 37242 | SV | Buffer6 Cassette ID | ASCII |  |  |  |  | Buffer6 Cassette ID |  | 🔴 文件獨有 |
| 37243 | SV | Buffer7 Cassette ID | ASCII |  |  |  |  | Buffer7 Cassette ID |  | 🔴 文件獨有 |
| 37244 | SV | Buffer8 Cassette ID | ASCII |  |  |  |  | Buffer8 Cassette ID |  | 🔴 文件獨有 |
| 37245 | SV | Buffer9 Cassette ID | ASCII |  |  |  |  | Buffer9 Cassette ID |  | 🔴 文件獨有 |
| 37246 | SV | Buffer10 Cassette ID | ASCII |  |  |  |  | Buffer10 Cassette ID |  | 🔴 文件獨有 |
| 37247 | SV | Buffer1 Lot ID | ASCII |  |  |  |  | Buffer1 Lot ID |  | 🔴 文件獨有 |
| 37248 | SV | Buffer2 Lot ID | ASCII |  |  |  |  | Buffer2 Lot ID |  | 🔴 文件獨有 |
| 37249 | SV | Buffer3 Lot ID | ASCII |  |  |  |  | Buffer3 Lot ID |  | 🔴 文件獨有 |
| 37250 | SV | Buffer4 Lot ID | ASCII |  |  |  |  | Buffer4 Lot ID |  | 🔴 文件獨有 |
| 37251 | SV | Buffer5 Lot ID | ASCII |  |  |  |  | Buffer5 Lot ID |  | 🔴 文件獨有 |
| 37252 | SV | Buffer6 Lot ID | ASCII |  |  |  |  | Buffer6 Lot ID |  | 🔴 文件獨有 |
| 37253 | SV | Buffer7 Lot ID | ASCII |  |  |  |  | Buffer7 Lot ID |  | 🔴 文件獨有 |
| 37254 | SV | Buffer8 Lot ID | ASCII |  |  |  |  | Buffer8 Lot ID |  | 🔴 文件獨有 |
| 37255 | SV | Buffer9 Lot ID | ASCII |  |  |  |  | Buffer9 Lot ID |  | 🔴 文件獨有 |
| 37256 | SV | Buffer10 Lot ID | ASCII |  |  |  |  | Buffer10 Lot ID |  | 🔴 文件獨有 |
| 37257 | SV | Buffer1 Data Type | INT_4 |  |  |  |  | Buffer1 Data Type |  | 🔴 文件獨有 |
| 37258 | SV | Buffer2 Data Type | INT_4 |  |  |  |  | Buffer2 Data Type |  | 🔴 文件獨有 |
| 37259 | SV | Buffer3 Data Type | INT_4 |  |  |  |  | Buffer3 Data Type |  | 🔴 文件獨有 |
| 37260 | SV | Buffer4 Data Type | INT_4 |  |  |  |  | Buffer4 Data Type |  | 🔴 文件獨有 |
| 37261 | SV | Buffer5 Data Type | INT_4 |  |  |  |  | Buffer5 Data Type |  | 🔴 文件獨有 |
| 37262 | SV | Buffer6 Data Type | INT_4 |  |  |  |  | Buffer6 Data Type |  | 🔴 文件獨有 |
| 37263 | SV | Buffer7 Data Type | INT_4 |  |  |  |  | Buffer7 Data Type |  | 🔴 文件獨有 |
| 37264 | SV | Buffer8 Data Type | INT_4 |  |  |  |  | Buffer8 Data Type |  | 🔴 文件獨有 |
| 37265 | SV | Buffer9 Data Type | INT_4 |  |  |  |  | Buffer9 Data Type |  | 🔴 文件獨有 |
| 37266 | SV | Buffer10 Data Type | INT_4 |  |  |  |  | Buffer10 Data Type |  | 🔴 文件獨有 |
| 37267 | SV | Buffer1 Pass Count | INT_4 |  |  |  |  | Buffer1 Pass Count |  | 🔴 文件獨有 |
| 37268 | SV | Buffer2 Pass Count | INT_4 |  |  |  |  | Buffer2 Pass Count |  | 🔴 文件獨有 |
| 37269 | SV | Buffer3 Pass Count | INT_4 |  |  |  |  | Buffer3 Pass Count |  | 🔴 文件獨有 |
| 37270 | SV | Buffer4 Pass Count | INT_4 |  |  |  |  | Buffer4 Pass Count |  | 🔴 文件獨有 |
| 37271 | SV | Buffer5 Pass Count | INT_4 |  |  |  |  | Buffer5 Pass Count |  | 🔴 文件獨有 |
| 37272 | SV | Buffer6 Pass Count | INT_4 |  |  |  |  | Buffer6 Pass Count |  | 🔴 文件獨有 |
| 37273 | SV | Buffer7 Pass Count | INT_4 |  |  |  |  | Buffer7 Pass Count |  | 🔴 文件獨有 |
| 37274 | SV | Buffer8 Pass Count | INT_4 |  |  |  |  | Buffer8 Pass Count |  | 🔴 文件獨有 |
| 37275 | SV | Buffer9 Pass Count | INT_4 |  |  |  |  | Buffer9 Pass Count |  | 🔴 文件獨有 |
| 37276 | SV | Buffer10 Pass Count | INT_4 |  |  |  |  | Buffer10 Pass Count |  | 🔴 文件獨有 |
| 37277 | SV | Buffer1 Fail Count | INT_4 |  |  |  |  | Buffer1 Fail Count |  | 🔴 文件獨有 |
| 37278 | SV | Buffer2 Fail Count | INT_4 |  |  |  |  | Buffer2 Fail Count |  | 🔴 文件獨有 |
| 37279 | SV | Buffer3 Fail Count | INT_4 |  |  |  |  | Buffer3 Fail Count |  | 🔴 文件獨有 |
| 37280 | SV | Buffer4 Fail Count | INT_4 |  |  |  |  | Buffer4 Fail Count |  | 🔴 文件獨有 |
| 37281 | SV | Buffer5 Fail Count | INT_4 |  |  |  |  | Buffer5 Fail Count |  | 🔴 文件獨有 |
| 37282 | SV | Buffer6 Fail Count | INT_4 |  |  |  |  | Buffer6 Fail Count |  | 🔴 文件獨有 |
| 37283 | SV | Buffer7 Fail Count | INT_4 |  |  |  |  | Buffer7 Fail Count |  | 🔴 文件獨有 |
| 37284 | SV | Buffer8 Fail Count | INT_4 |  |  |  |  | Buffer8 Fail Count |  | 🔴 文件獨有 |
| 37285 | SV | Buffer9 Fail Count | INT_4 |  |  |  |  | Buffer9 Fail Count |  | 🔴 文件獨有 |
| 37286 | SV | Buffer10 Fail Count | INT_4 |  |  |  |  | Buffer10 Fail Count |  | 🔴 文件獨有 |
| 37287 | EC | Lot Quantity | ASCII |  |  |  |  | Lot Quantity |  | 🔴 文件獨有 |
| 37288 | EC | Pre-Send ID | ASCII |  |  |  |  | Pre-Send ID |  | 🔴 文件獨有 |
| 37289 | SV | Empty Buffer Count | INT_4 |  |  |  |  | Empty Buffer Count |  | 🔴 文件獨有 |
| 37290 | SV | Run QA Mode | INT_4 |  |  |  |  | Run QA Mode |  | 🔴 文件獨有 |
| 37291 | EC | Access Mode | INT_4 |  |  |  |  | Access Mode |  | 🔴 文件獨有 |
| 37292 | SV | Load Port Transfer State | INT_4 |  |  |  |  | Load Port Transfer State |  | 🔴 文件獨有 |
| 37293 | SV | Ready To Load Mode | INT_4 |  |  |  |  | Ready To Load Mode |  | 🔴 文件獨有 |
| 37294 | EC | Software Bin | ASCII |  |  |  |  | Software Bin |  | 🔴 文件獨有 |
| 37295 | EC | Index Arm Down | INT_4 |  |  |  |  | Index Arm Down |  | 🔴 文件獨有 |
| 37296 | EC | Cassette Out Data | ASCII |  |  |  |  | Cassette Out Data |  | 🔴 文件獨有 |
| 37297 | EC | Enable FIFO Function | BOOLEAN |  |  |  |  | Enable FIFO Function |  | 🔴 文件獨有 |
| 37298 | SV | Fail RT | INT_4 |  |  |  |  | Fail RT |  | 🔴 文件獨有 |
| 37299 | SV | Fail No RT | INT_4 |  |  |  |  | Fail No RT |  | 🔴 文件獨有 |
| 37300 | SV/EC | Site Aa Socket ID | ASCII |  |  |  |  | Site Aa Socket ID | ✓ |  |
| 37301 | SV/EC | Site Ab Socket ID | ASCII |  |  |  |  | Site Ab Socket ID | ✓ |  |
| 37302 | SV/EC | Site Ac Socket ID | ASCII |  |  |  |  | Site Ac Socket ID | ✓ |  |
| 37303 | SV/EC | Site Ad Socket ID | ASCII |  |  |  |  | Site Ad Socket ID | ✓ |  |
| 37304 | SV/EC | Site Ae Socket ID | ASCII |  |  |  |  | Site Ae Socket ID | ✓ |  |
| 37305 | SV/EC | Site Af Socket ID | ASCII |  |  |  |  | Site Af Socket ID | ✓ |  |
| 37306 | SV/EC | Site Ag Socket ID | ASCII |  |  |  |  | Site Ag Socket ID | ✓ |  |
| 37307 | SV/EC | Site Ah Socket ID | ASCII |  |  |  |  | Site Ah Socket ID | ✓ |  |
| 37308 | SV/EC | Site Ba Socket ID | ASCII |  |  |  |  | Site Ba Socket ID | ✓ |  |
| 37309 | SV/EC | Site Bb Socket ID | ASCII |  |  |  |  | Site Bb Socket ID | ✓ |  |
| 37310 | SV/EC | Site Bc Socket ID | ASCII |  |  |  |  | Site Bc Socket ID | ✓ |  |
| 37311 | SV/EC | Site Bd Socket ID | ASCII |  |  |  |  | Site Bd Socket ID | ✓ |  |
| 37312 | SV/EC | Site Be Socket ID | ASCII |  |  |  |  | Site Be Socket ID | ✓ |  |
| 37313 | SV/EC | Site Bf Socket ID | ASCII |  |  |  |  | Site Bf Socket ID | ✓ |  |
| 37314 | SV/EC | Site Bg Socket ID | ASCII |  |  |  |  | Site Bg Socket ID | ✓ |  |
| 37315 | SV/EC | Site Bh Socket ID | ASCII |  |  |  |  | Site Bh Socket ID | ✓ |  |
| 37316 | SV/EC | Site Ca Socket ID | ASCII |  |  |  |  | Site Ca Socket ID | ✓ |  |
| 37317 | SV/EC | Site Cb Socket ID | ASCII |  |  |  |  | Site Cb Socket ID | ✓ |  |
| 37318 | SV/EC | Site Cc Socket ID | ASCII |  |  |  |  | Site Cc Socket ID | ✓ |  |
| 37319 | SV/EC | Site Cd Socket ID | ASCII |  |  |  |  | Site Cd Socket ID | ✓ |  |
| 37320 | SV/EC | Site Ce Socket ID | ASCII |  |  |  |  | Site Ce Socket ID | ✓ |  |
| 37321 | SV/EC | Site Cf Socket ID | ASCII |  |  |  |  | Site Cf Socket ID | ✓ |  |
| 37322 | SV/EC | Site Cg Socket ID | ASCII |  |  |  |  | Site Cg Socket ID | ✓ |  |
| 37323 | SV/EC | Site Ch Socket ID | ASCII |  |  |  |  | Site Ch Socket ID | ✓ |  |
| 37324 | SV/EC | Site Da Socket ID | ASCII |  |  |  |  | Site Da Socket ID | ✓ |  |
| 37325 | SV/EC | Site Db Socket ID | ASCII |  |  |  |  | Site Db Socket ID | ✓ |  |
| 37326 | SV/EC | Site Dc Socket ID | ASCII |  |  |  |  | Site Dc Socket ID | ✓ |  |
| 37327 | SV/EC | Site Dd Socket ID | ASCII |  |  |  |  | Site Dd Socket ID | ✓ |  |
| 37328 | SV/EC | Site De Socket ID | ASCII |  |  |  |  | Site De Socket ID | ✓ |  |
| 37329 | SV/EC | Site Df Socket ID | ASCII |  |  |  |  | Site Df Socket ID | ✓ |  |
| 37330 | SV/EC | Site Dg Socket ID | ASCII |  |  |  |  | Site Dg Socket ID | ✓ |  |
| 37331 | SV/EC | Site Dh Socket ID | ASCII |  |  |  |  | Site Dh Socket ID | ✓ |  |
| 37400 | SV | Site Aa Socket Contact Count | INT_4 |  |  |  |  | Site Aa Socket Contact Count | ✓ |  |
| 37401 | SV | Site Ab Socket Contact Count | INT_4 |  |  |  |  | Site Ab Socket Contact Count | ✓ |  |
| 37402 | SV | Site Ac Socket Contact Count | INT_4 |  |  |  |  | Site Ac Socket Contact Count | ✓ |  |
| 37403 | SV | Site Ad Socket Contact Count | INT_4 |  |  |  |  | Site Ad Socket Contact Count | ✓ |  |
| 37404 | SV | Site Ae Socket Contact Count | INT_4 |  |  |  |  | Site Ae Socket Contact Count | ✓ |  |
| 37405 | SV | Site Af Socket Contact Count | INT_4 |  |  |  |  | Site Af Socket Contact Count | ✓ |  |
| 37406 | SV | Site Ag Socket Contact Count | INT_4 |  |  |  |  | Site Ag Socket Contact Count | ✓ |  |
| 37407 | SV | Site Ah Socket Contact Count | INT_4 |  |  |  |  | Site Ah Socket Contact Count | ✓ |  |
| 37408 | SV | Site Ba Socket Contact Count | INT_4 |  |  |  |  | Site Ba Socket Contact Count | ✓ |  |
| 37409 | SV | Site Bb Socket Contact Count | INT_4 |  |  |  |  | Site Bb Socket Contact Count | ✓ |  |
| 37410 | SV | Site Bc Socket Contact Count | INT_4 |  |  |  |  | Site Bc Socket Contact Count | ✓ |  |
| 37411 | SV | Site Bd Socket Contact Count | INT_4 |  |  |  |  | Site Bd Socket Contact Count | ✓ |  |
| 37412 | SV | Site Be Socket Contact Count | INT_4 |  |  |  |  | Site Be Socket Contact Count | ✓ |  |
| 37413 | SV | Site Bf Socket Contact Count | INT_4 |  |  |  |  | Site Bf Socket Contact Count | ✓ |  |
| 37414 | SV | Site Bg Socket Contact Count | INT_4 |  |  |  |  | Site Bg Socket Contact Count | ✓ |  |
| 37415 | SV | Site Bh Socket Contact Count | INT_4 |  |  |  |  | Site Bh Socket Contact Count | ✓ |  |
| 37416 | SV | Site Ca Socket Contact Count | INT_4 |  |  |  |  | Site Ca Socket Contact Count | ✓ |  |
| 37417 | SV | Site Cb Socket Contact Count | INT_4 |  |  |  |  | Site Cb Socket Contact Count | ✓ |  |
| 37418 | SV | Site Cc Socket Contact Count | INT_4 |  |  |  |  | Site Cc Socket Contact Count | ✓ |  |
| 37419 | SV | Site Cd Socket Contact Count | INT_4 |  |  |  |  | Site Cd Socket Contact Count | ✓ |  |
| 37420 | SV | Site Ce Socket Contact Count | INT_4 |  |  |  |  | Site Ce Socket Contact Count | ✓ |  |
| 37421 | SV | Site Cf Socket Contact Count | INT_4 |  |  |  |  | Site Cf Socket Contact Count | ✓ |  |
| 37422 | SV | Site Cg Socket Contact Count | INT_4 |  |  |  |  | Site Cg Socket Contact Count | ✓ |  |
| 37423 | SV | Site Ch Socket Contact Count | INT_4 |  |  |  |  | Site Ch Socket Contact Count | ✓ |  |
| 37424 | SV | Site Da Socket Contact Count | INT_4 |  |  |  |  | Site Da Socket Contact Count | ✓ |  |
| 37425 | SV | Site Db Socket Contact Count | INT_4 |  |  |  |  | Site Db Socket Contact Count | ✓ |  |
| 37426 | SV | Site Dc Socket Contact Count | INT_4 |  |  |  |  | Site Dc Socket Contact Count | ✓ |  |
| 37427 | SV | Site Dd Socket Contact Count | INT_4 |  |  |  |  | Site Dd Socket Contact Count | ✓ |  |
| 37428 | SV | Site De Socket Contact Count | INT_4 |  |  |  |  | Site De Socket Contact Count | ✓ |  |
| 37429 | SV | Site Df Socket Contact Count | INT_4 |  |  |  |  | Site Df Socket Contact Count | ✓ |  |
| 37430 | SV | Site Dg Socket Contact Count | INT_4 |  |  |  |  | Site Dg Socket Contact Count | ✓ |  |
| 37431 | SV | Site Dh Socket Contact Count | INT_4 |  |  |  |  | Site Dh Socket Contact Count | ✓ |  |
| 37490 | EC | Kit No 1 | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37491 | EC | Kit No 2 | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37492 | EC | Kit No 3 | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37501 | SV | ATC Power Supply Serial Number 01 | ASCII |  |  |  |  | ATC Power Supply Serial Number 01 | ✓ |  |
| 37502 | SV | ATC Power Supply Serial Number 02 | ASCII |  |  |  |  | ATC Power Supply Serial Number 02 | ✓ |  |
| 37503 | SV | ATC Power Supply Serial Number 03 | ASCII |  |  |  |  | ATC Power Supply Serial Number 03 | ✓ |  |
| 37504 | SV | ATC Power Supply Serial Number 04 | ASCII |  |  |  |  | ATC Power Supply Serial Number 04 | ✓ |  |
| 37505 | SV | ATC Power Supply Firmware Number 01 | ASCII |  |  |  |  | ATC Power Supply Firmware Number 01 | ✓ | ⚠️ SV名稱: Code=`ATC Power Supply Firmware Version 01` |
| 37506 | SV | ATC Power Supply Firmware Number 02 | ASCII |  |  |  |  | ATC Power Supply Firmware Number 02 | ✓ | ⚠️ SV名稱: Code=`ATC Power Supply Firmware Version 02` |
| 37507 | SV | ATC Power Supply Firmware Number 03 | ASCII |  |  |  |  | ATC Power Supply Firmware Number 03 | ✓ | ⚠️ SV名稱: Code=`ATC Power Supply Firmware Version 03` |
| 37508 | SV | ATC Power Supply Firmware Number 04 | ASCII |  |  |  |  | ATC Power Supply Firmware Number 04 | ✓ | ⚠️ SV名稱: Code=`ATC Power Supply Firmware Version 04` |
| 37529 | SV | OCR Software Version | ASCII |  |  |  |  | OCR Software Version |  | 🔴 文件獨有 |
| 37530 | SV | 2D Barcode Software Version | ASCII |  |  |  |  | 2D Barcode Software Version |  | 🔴 文件獨有 |
| 37531 | SV | RTC Software Version | ASCII |  |  |  |  | RTC Software Version |  | 🔴 文件獨有 |
| 37532 | SV | AOA Software Version | ASCII |  |  |  |  | AOA Software Version |  | 🔴 文件獨有 |
| 37533 | SV | Handler Software Minor Version | ASCII |  |  |  |  | Handler Software Minor Version |  | 🔴 文件獨有 |
| 37534 | SV | 2DID String Format | INT_4 |  | 5 | 0 |  | 0: Normal ( XXXXXXXXX ) 1: Include Dash ( XXXX-XXXX ) 2: Include Dot ( XXXX.XXXX ) 3: Include Dot and Dash ( XXXX.XXXX-XXXX ) 4: Include Dash and Dot ( XXXX-XXXX.XXXX ) 5: [a-zA-Z0-9-: _.] |  | 🔴 文件獨有 |
| 37535 | SV | ATC Channel 01 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37536 | SV | ATC Channel 02 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37537 | SV | ATC Channel 03 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37538 | SV | ATC Channel 04 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37539 | SV | ATC Channel 05 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37540 | SV | ATC Channel 06 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37541 | SV | ATC Channel 07 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37542 | SV | ATC Channel 08 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37543 | SV | ATC Channel 09 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37544 | SV | ATC Channel 10 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37545 | SV | ATC Channel 11 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37546 | SV | ATC Channel 12 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37547 | SV | ATC Channel 13 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37548 | SV | ATC Channel 14 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37549 | SV | ATC Channel 15 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37550 | SV | ATC Channel 16 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37551 | SV | ATC Channel 17 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37552 | SV | ATC Channel 18 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37553 | SV | ATC Channel 19 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37554 | SV | ATC Channel 20 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37555 | SV | ATC Channel 21 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37556 | SV | ATC Channel 22 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37557 | SV | ATC Channel 23 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37558 | SV | ATC Channel 24 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37559 | SV | ATC Channel 25 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37560 | SV | ATC Channel 26 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37561 | SV | ATC Channel 27 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37562 | SV | ATC Channel 28 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37563 | SV | ATC Channel 29 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37564 | SV | ATC Channel 30 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37565 | SV | ATC Channel 31 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37566 | SV | ATC Channel 32 Parameters By Recipe | ASCII |  |  |  |  | TC_P,TC_I,TC_D,TS_P,TS_I,TS_D,TJ_P,TJ_I,TJ_D,TC Water Valve,TJ Water Valve,Check Water Flow function |  | 🔴 文件獨有 |
| 37567 | SV | ATC Public Parameters By Recipe | ASCII |  |  |  |  |  |  | 🔴 文件獨有 |
| 37578 | SV | Reserve 11 |  |  |  |  |  |  |  | 🔴 文件獨有 |
| 37600 | SV | In Arm Picker A Count | INT_4 |  |  |  |  | In Arm Picker A Count | ✓ |  |
| 37601 | SV | In Arm Picker C Count | INT_4 |  |  |  |  | In Arm Picker C Count | ✓ |  |
| 37602 | SV | In Arm Picker E Count | INT_4 |  |  |  |  | In Arm Picker E Count | ✓ |  |
| 37603 | SV | In Arm Picker G Count | INT_4 |  |  |  |  | In Arm Picker G Count | ✓ |  |
| 37604 | SV | In Arm Picker B Count | INT_4 |  |  |  |  | In Arm Picker B Count | ✓ |  |
| 37605 | SV | In Arm Picker D Count | INT_4 |  |  |  |  | In Arm Picker D Count | ✓ |  |
| 37606 | SV | In Arm Picker F Count | INT_4 |  |  |  |  | In Arm Picker F Count | ✓ |  |
| 37607 | SV | In Arm Picker H Count | INT_4 |  |  |  |  | In Arm Picker H Count | ✓ |  |
| 37650 | SV | Out Arm Picker A Count | INT_4 |  |  |  |  | Out Arm Picker A Count | ✓ |  |
| 37651 | SV | Out Arm Picker C Count | INT_4 |  |  |  |  | Out Arm Picker C Count | ✓ |  |
| 37652 | SV | Out Arm Picker E Count | INT_4 |  |  |  |  | Out Arm Picker E Count | ✓ |  |
| 37653 | SV | Out Arm Picker G Count | INT_4 |  |  |  |  | Out Arm Picker G Count | ✓ |  |
| 37654 | SV | Out Arm Picker B Count | INT_4 |  |  |  |  | Out Arm Picker B Count | ✓ |  |
| 37655 | SV | Out Arm Picker D Count | INT_4 |  |  |  |  | Out Arm Picker D Count | ✓ |  |
| 37656 | SV | Out Arm Picker F Count | INT_4 |  |  |  |  | Out Arm Picker F Count | ✓ |  |
| 37657 | SV | Out Arm Picker H Count | INT_4 |  |  |  |  | Out Arm Picker H Count | ✓ |  |
| 37700 | SV | PC_NAME | ASCII |  |  |  |  | PC_NAME | ✓ |  |
| 37701 | SV | Index Arm 1 Picker Aa Count | INT_4 |  |  |  |  | Index Arm 1 Picker Aa Count | ✓ |  |
| 37702 | SV | Index Arm 1 Picker Ab Count | INT_4 |  |  |  |  | Index Arm 1 Picker Ab Count | ✓ |  |
| 37703 | SV | Index Arm 1 Picker Ac Count | INT_4 |  |  |  |  | Index Arm 1 Picker Ac Count | ✓ |  |
| 37704 | SV | Index Arm 1 Picker Ad Count | INT_4 |  |  |  |  | Index Arm 1 Picker Ad Count | ✓ |  |
| 37705 | SV | Index Arm 1 Picker Ae Count | INT_4 |  |  |  |  | Index Arm 1 Picker Ae Count | ✓ |  |
| 37706 | SV | Index Arm 1 Picker Af Count | INT_4 |  |  |  |  | Index Arm 1 Picker Af Count | ✓ |  |
| 37707 | SV | Index Arm 1 Picker Ag Count | INT_4 |  |  |  |  | Index Arm 1 Picker Ag Count | ✓ |  |
| 37708 | SV | Index Arm 1 Picker Ah Count | INT_4 |  |  |  |  | Index Arm 1 Picker Ah Count | ✓ |  |
| 37709 | SV | Index Arm 1 Picker Aa Count | INT_4 |  |  |  |  | Index Arm 1 Picker Aa Count | ✓ | ⚠️ SV名稱: Code=`Index Arm 1 Picker Ba Count` |
| 37710 | SV | Index Arm 1 Picker Ab Count | INT_4 |  |  |  |  | Index Arm 1 Picker Ab Count | ✓ | ⚠️ SV名稱: Code=`Index Arm 1 Picker Bb Count` |
| 37711 | SV | Index Arm 1 Picker Ac Count | INT_4 |  |  |  |  | Index Arm 1 Picker Ac Count | ✓ | ⚠️ SV名稱: Code=`Index Arm 1 Picker Bc Count` |
| 37712 | SV | Index Arm 1 Picker Ad Count | INT_4 |  |  |  |  | Index Arm 1 Picker Ad Count | ✓ | ⚠️ SV名稱: Code=`Index Arm 1 Picker Bd Count` |
| 37713 | SV | Index Arm 1 Picker Ae Count | INT_4 |  |  |  |  | Index Arm 1 Picker Ae Count | ✓ | ⚠️ SV名稱: Code=`Index Arm 1 Picker Be Count` |
| 37714 | SV | Index Arm 1 Picker Af Count | INT_4 |  |  |  |  | Index Arm 1 Picker Af Count | ✓ | ⚠️ SV名稱: Code=`Index Arm 1 Picker Bf Count` |
| 37715 | SV | Index Arm 1 Picker Ag Count | INT_4 |  |  |  |  | Index Arm 1 Picker Ag Count | ✓ | ⚠️ SV名稱: Code=`Index Arm 1 Picker Bg Count` |
| 37716 | SV | Index Arm 1 Picker Ah Count | INT_4 |  |  |  |  | Index Arm 1 Picker Ah Count | ✓ | ⚠️ SV名稱: Code=`Index Arm 1 Picker Bh Count` |
| 37717 | SV | Index Arm 2 Picker Aa Count | INT_4 |  |  |  |  | Index Arm 2 Picker Aa Count | ✓ |  |
| 37718 | SV | Index Arm 2 Picker Ab Count | INT_4 |  |  |  |  | Index Arm 2 Picker Ab Count | ✓ |  |
| 37719 | SV | Index Arm 2 Picker Ac Count | INT_4 |  |  |  |  | Index Arm 2 Picker Ac Count | ✓ |  |
| 37720 | SV | Index Arm 2 Picker Ad Count | INT_4 |  |  |  |  | Index Arm 2 Picker Ad Count | ✓ |  |
| 37721 | SV | Index Arm 2 Picker Ae Count | INT_4 |  |  |  |  | Index Arm 2 Picker Ae Count | ✓ |  |
| 37722 | SV | Index Arm 2 Picker Af Count | INT_4 |  |  |  |  | Index Arm 2 Picker Af Count | ✓ |  |
| 37723 | SV | Index Arm 2 Picker Ag Count | INT_4 |  |  |  |  | Index Arm 2 Picker Ag Count | ✓ |  |
| 37724 | SV | Index Arm 2 Picker Ah Count | INT_4 |  |  |  |  | Index Arm 2 Picker Ah Count | ✓ |  |
| 37725 | SV | Index Arm 2 Picker Aa Count | INT_4 |  |  |  |  | Index Arm 2 Picker Aa Count | ✓ | ⚠️ SV名稱: Code=`Index Arm 2 Picker Ba Count` |
| 37726 | SV | Index Arm 2 Picker Ab Count | INT_4 |  |  |  |  | Index Arm 2 Picker Ab Count | ✓ | ⚠️ SV名稱: Code=`Index Arm 2 Picker Bb Count` |
| 37727 | SV | Index Arm 2 Picker Ac Count | INT_4 |  |  |  |  | Index Arm 2 Picker Ac Count | ✓ | ⚠️ SV名稱: Code=`Index Arm 2 Picker Bc Count` |
| 37728 | SV | Index Arm 2 Picker Ad Count | INT_4 |  |  |  |  | Index Arm 2 Picker Ad Count | ✓ | ⚠️ SV名稱: Code=`Index Arm 2 Picker Bd Count` |
| 37729 | SV | Index Arm 2 Picker Ae Count | INT_4 |  |  |  |  | Index Arm 2 Picker Ae Count | ✓ | ⚠️ SV名稱: Code=`Index Arm 2 Picker Be Count` |
| 37730 | SV | Index Arm 2 Picker Af Count | INT_4 |  |  |  |  | Index Arm 2 Picker Af Count | ✓ | ⚠️ SV名稱: Code=`Index Arm 2 Picker Bf Count` |
| 37731 | SV | Index Arm 2 Picker Ag Count | INT_4 |  |  |  |  | Index Arm 2 Picker Ag Count | ✓ | ⚠️ SV名稱: Code=`Index Arm 2 Picker Bg Count` |
| 37732 | SV | Index Arm 2 Picker Ah Count | INT_4 |  |  |  |  | Index Arm 2 Picker Ah Count | ✓ | ⚠️ SV名稱: Code=`Index Arm 2 Picker Bh Count` |
| 37800 | EC | ScanInterval | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37801 | EC | Alarm_Continuous_Time | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37802 | EC | Alarm_Enable | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37803 | EC | PassWord | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37804 | EC | iESDReporTimer | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37805 | EC | Active[0] | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37806 | EC | Alarm[0][0] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37807 | EC | ProxOnOff[0][0] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37816 | EC | Alarm[0][1] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37817 | EC | ProxOnOff[0][1] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37826 | EC | Alarm[0][2] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37827 | EC | ProxOnOff[0][2] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37835 | EC | Active[1] | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37836 | EC | Alarm[1][0] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37837 | EC | ProxOnOff[1][0] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37846 | EC | Alarm[1[1] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37847 | EC | ProxOnOff[1][1] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37856 | EC | Alarm[1][2] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37857 | EC | ProxOnOff[1][2] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37865 | EC | Active[2] | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37866 | EC | Alarm[2][0] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37867 | EC | ProxOnOff[2][0] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37876 | EC | Alarm[2[1] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37877 | EC | ProxOnOff[2][1] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37886 | EC | Alarm[2][2] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 37887 | EC | ProxOnOff[2][2] | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38002 | EC | Tray RFID | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38097 | SV | Catch Form Auto | INT_4 |  |  |  |  | Catch Form Auto |  | 🔴 文件獨有 |
| 38098 | SV | FT Lane Device Count | ASCII |  |  |  |  | FT Lane Device Count |  | 🔴 文件獨有 |
| 38099 | SV | RT Lane Device Count | ASCII |  |  |  |  | RT Lane Device Count |  | 🔴 文件獨有 |
| 38100 | SV | Cassette Tray 1 Data | ASCII |  |  |  |  | Cassette Tray 1 Data |  | 🔴 文件獨有 |
| 38101 | SV | Cassette Tray 2 Data | ASCII |  |  |  |  | Cassette Tray 2 Data |  | 🔴 文件獨有 |
| 38102 | SV | Cassette Tray 3 Data | ASCII |  |  |  |  | Cassette Tray 3 Data |  | 🔴 文件獨有 |
| 38103 | SV | Cassette Tray 4 Data | ASCII |  |  |  |  | Cassette Tray 4 Data |  | 🔴 文件獨有 |
| 38104 | SV | Cassette Tray 5 Data | ASCII |  |  |  |  | Cassette Tray 5 Data |  | 🔴 文件獨有 |
| 38105 | SV | Cassette Tray 6 Data | ASCII |  |  |  |  | Cassette Tray 6 Data |  | 🔴 文件獨有 |
| 38106 | SV | Cassette Tray 7 Data | ASCII |  |  |  |  | Cassette Tray 7 Data |  | 🔴 文件獨有 |
| 38107 | SV | Cassette Tray 8 Data | ASCII |  |  |  |  | Cassette Tray 8 Data |  | 🔴 文件獨有 |
| 38108 | SV | Cassette Tray 9 Data | ASCII |  |  |  |  | Cassette Tray 9 Data |  | 🔴 文件獨有 |
| 38109 | SV | Cassette Tray 10 Data | ASCII |  |  |  |  | Cassette Tray 10 Data |  | 🔴 文件獨有 |
| 38110 | SV | Cassette Tray 11 Data | ASCII |  |  |  |  | Cassette Tray 11 Data |  | 🔴 文件獨有 |
| 38111 | SV | Cassette Tray 12 Data | ASCII |  |  |  |  | Cassette Tray 12 Data |  | 🔴 文件獨有 |
| 38112 | SV | Cassette Tray 13 Data | ASCII |  |  |  |  | Cassette Tray 13 Data |  | 🔴 文件獨有 |
| 38113 | SV | Cassette Tray 14 Data | ASCII |  |  |  |  | Cassette Tray 14 Data |  | 🔴 文件獨有 |
| 38114 | SV | Cassette Tray 15 Data | ASCII |  |  |  |  | Cassette Tray 15 Data |  | 🔴 文件獨有 |
| 38115 | SV | Cassette Tray 16 Data | ASCII |  |  |  |  | Cassette Tray 16 Data |  | 🔴 文件獨有 |
| 38116 | SV | Cassette Tray 17 Data | ASCII |  |  |  |  | Cassette Tray 17 Data |  | 🔴 文件獨有 |
| 38117 | SV | Cassette Tray 18 Data | ASCII |  |  |  |  | Cassette Tray 18 Data |  | 🔴 文件獨有 |
| 38118 | SV | Cassette Tray 19 Data | ASCII |  |  |  |  | Cassette Tray 19 Data |  | 🔴 文件獨有 |
| 38119 | SV | Cassette Tray 20 Data | ASCII |  |  |  |  | Cassette Tray 20 Data |  | 🔴 文件獨有 |
| 38120 | SV | Load Port Transfer State - LOADER | INT_4 |  |  |  |  | 1: Ready To Load : 指定的Port軌道已鎖定，等待上料 2: High WIP : Load Port已達最高水位，不可再上bundle 3: Tray Arrived : Load Port有bundle 4: Low WIP : Load Port需求派料上機 5: Empty : Load Port無TRAY盤 6: Error : Load port sensor狀態異常 Note: AGV上下料須確認當下狀態為Read To Load | ✓ |  |
| 38121 | SV | Load Port Transfer State - EMPTY | INT_4 |  |  |  |  | 暫不支援,由人工上下料 | ✓ |  |
| 38122 | SV | Load Port Transfer State - COLOR | INT_4 |  |  |  |  | 暫不支援,由人工上下料 | ✓ |  |
| 38123 | SV | Load Port Transfer State - AUTO1 | INT_4 |  |  |  |  | 1: Ready To Unload : 指定的Port軌道已鎖定，等待下料 2: 滿Bundle : 完成1 bundle量需求取出或是tray feed前需求取出 3: TRAY Arrived : Unload Port有bundle 4: Empty : Unload Port無TRAY盤 5: Error : Unload port sensor狀態異常 Note: AGV上下料須確認當下狀態為Ready To Unload | ✓ |  |
| 38124 | SV | Load Port Transfer State - AUTO2 | INT_4 |  |  |  |  | same as above | ✓ |  |
| 38125 | SV | Load Port Transfer State - AUTO3 | INT_4 |  |  |  |  | same as above | ✓ |  |
| 38126 | SV | Load Port Transfer State - FIX1 | INT_4 |  |  |  |  | 1: Ready To Unload滿Bundle需求下料 2: FULL TRAY: 滿TRAY但尚未滿BUNDLE，需求下料 3: TRAY Arrived : 機台感應有TRAY, 但尚未刷入Bundle ID 4: Empty : Fix port無tray盤 5: Error :  Fix port sensor狀態與資料不匹配之異常 | ✓ |  |
| 38127 | SV | Load Port Transfer State - FIX2 | INT_4 |  |  |  |  | same as above | ✓ |  |
| 38128 | SV | Load Port Transfer State - FIX3 | INT_4 |  |  |  |  | same as above | ✓ |  |
| 38129 | SV | Load Port Transfer State - AUTO4 | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38130 | SV | Load Port Transfer State - AUTO5 | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38131 | SV | Load Port Transfer State - AUTO6 | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38132 | SV | Load Port Transfer State - FIX4 | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38133 | SV | Load Port Transfer State - FIX5 | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38134 | SV | Load Port Transfer State - FIX6 | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38199 | EC | Output 4 Tray ID | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38200 | EC | Output 5 Tray ID | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38201 | EC | Output 6 Tray ID | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38205 | EC | Output 1 Tray ID | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38206 | EC | Output 2 Tray ID | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38207 | EC | Output 3 Tray ID | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38214 | EC | Bundle Information | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38215 | EC | In port Bundle ID | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38216 | EC | Loader Bundle ID | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38217 | EC | Unload Bundle ID | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38218 | EC | Process end report | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38250 | SV | Die QTY of Bundle | INT_4 |  |  |  |  | Die QTY of Bundle | ✓ |  |
| 38300 | EC | Port number for tray input stacker | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38301 | EC | Port status for tray input stacker | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38302 | EC | Dcc Data | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38303 | SV | Empty Tray Count | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38304 | SV | Output BIN Code | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38305 | SV | Input Port State | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38306 | SV | Empty Port State | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38307 | SV | Output 1 Port State | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38308 | SV | Output 2 Port State | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38309 | SV | Output 3 Port State | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38310 | SV | Output 4 Port State | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38311 | SV | Output 5 Port State | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38312 | SV | Output 6 Port State | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38313 | SV | CoverTray Port State | INT_4 |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38314 | SV | PC_NAME | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38601 | SV/EC | Alarm2 Normal Bin 0 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 Normal Bin 0 By Bin Site Gap over Limit Enable | ✓ |  |
| 38602 | SV/EC | Alarm2 Normal Bin 0 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 Normal Bin 0 By Bin Site Gap over Limit (%) | ✓ |  |
| 38603 | SV/EC | Alarm2 Normal Bin 1 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 Normal Bin 1 By Bin Site Gap over Limit Enable | ✓ |  |
| 38604 | SV/EC | Alarm2 Normal Bin 1 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 Normal Bin 1 By Bin Site Gap over Limit (%) | ✓ |  |
| 38605 | SV/EC | Alarm2 Normal Bin 2 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 Normal Bin 2 By Bin Site Gap over Limit Enable | ✓ |  |
| 38606 | SV/EC | Alarm2 Normal Bin 2 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 Normal Bin 2 By Bin Site Gap over Limit (%) | ✓ |  |
| 38607 | SV/EC | Alarm2 Normal Bin 3 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 Normal Bin 3 By Bin Site Gap over Limit Enable | ✓ |  |
| 38608 | SV/EC | Alarm2 Normal Bin 3 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 Normal Bin 3 By Bin Site Gap over Limit (%) | ✓ |  |
| 38609 | SV/EC | Alarm2 Normal Bin 4 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 Normal Bin 4 By Bin Site Gap over Limit Enable | ✓ |  |
| 38610 | SV/EC | Alarm2 Normal Bin 4 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 Normal Bin 4 By Bin Site Gap over Limit (%) | ✓ |  |
| 38611 | SV/EC | Alarm2 Normal Bin 5 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 Normal Bin 5 By Bin Site Gap over Limit Enable | ✓ |  |
| 38612 | SV/EC | Alarm2 Normal Bin 5 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 Normal Bin 5 By Bin Site Gap over Limit (%) | ✓ |  |
| 38613 | SV/EC | Alarm2 Normal Bin 6 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 Normal Bin 6 By Bin Site Gap over Limit Enable | ✓ |  |
| 38614 | SV/EC | Alarm2 Normal Bin 6 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 Normal Bin 6 By Bin Site Gap over Limit (%) | ✓ |  |
| 38615 | SV/EC | Alarm2 Normal Bin 7 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 Normal Bin 7 By Bin Site Gap over Limit Enable | ✓ |  |
| 38616 | SV/EC | Alarm2 Normal Bin 7 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 Normal Bin 7 By Bin Site Gap over Limit (%) | ✓ |  |
| 38617 | SV/EC | Alarm2 Normal Bin 8 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 Normal Bin 8 By Bin Site Gap over Limit Enable | ✓ |  |
| 38618 | SV/EC | Alarm2 Normal Bin 8 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 Normal Bin 8 By Bin Site Gap over Limit (%) | ✓ |  |
| 38619 | SV/EC | Alarm2 Normal Bin 9 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 Normal Bin 9 By Bin Site Gap over Limit Enable | ✓ |  |
| 38620 | SV/EC | Alarm2 Normal Bin 9 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 Normal Bin 9 By Bin Site Gap over Limit (%) | ✓ |  |
| 38621 | SV/EC | Alarm2 Normal Bin 10 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 Normal Bin 10 By Bin Site Gap over Limit Enable | ✓ |  |
| 38622 | SV/EC | Alarm2 Normal Bin 10 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 Normal Bin 10 By Bin Site Gap over Limit (%) | ✓ |  |
| 38623 | SV/EC | Alarm2 Normal Bin 11 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 Normal Bin 11 By Bin Site Gap over Limit Enable | ✓ |  |
| 38624 | SV/EC | Alarm2 Normal Bin 11 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 Normal Bin 11 By Bin Site Gap over Limit (%) | ✓ |  |
| 38625 | SV/EC | Alarm2 Normal Bin 12 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 Normal Bin 12 By Bin Site Gap over Limit Enable | ✓ |  |
| 38626 | SV/EC | Alarm2 Normal Bin 12 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 Normal Bin 12 By Bin Site Gap over Limit (%) | ✓ |  |
| 38627 | SV/EC | Alarm2 Normal Bin 13 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 Normal Bin 13 By Bin Site Gap over Limit Enable | ✓ |  |
| 38628 | SV/EC | Alarm2 Normal Bin 13 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 Normal Bin 13 By Bin Site Gap over Limit (%) | ✓ |  |
| 38629 | SV/EC | Alarm2 Normal Bin 14 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 Normal Bin 14 By Bin Site Gap over Limit Enable | ✓ |  |
| 38630 | SV/EC | Alarm2 Normal Bin 14 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 Normal Bin 14 By Bin Site Gap over Limit (%) | ✓ |  |
| 38631 | SV/EC | Alarm2 Normal Bin 15 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 Normal Bin 15 By Bin Site Gap over Limit Enable | ✓ |  |
| 38632 | SV/EC | Alarm2 Normal Bin 15 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 Normal Bin 15 By Bin Site Gap over Limit (%) | ✓ |  |
| 38633 | SV/EC | Alarm2 Normal By Bin Site Gap over Limit Ignore count | INT_4 |  |  |  |  | Alarm2 Normal By Bin Site Gap over Limit Ignore count | ✓ |  |
| 38651 | SV/EC | Alarm3 Normal Bin 0 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 Normal Bin 0 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38652 | SV/EC | Alarm3 Normal Bin 0 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 Normal Bin 0 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38653 | SV/EC | Alarm3 Normal Bin 1 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 Normal Bin 1 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38654 | SV/EC | Alarm3 Normal Bin 1 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 Normal Bin 1 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38655 | SV/EC | Alarm3 Normal Bin 2 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 Normal Bin 2 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38656 | SV/EC | Alarm3 Normal Bin 2 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 Normal Bin 2 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38657 | SV/EC | Alarm3 Normal Bin 3 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 Normal Bin 3 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38658 | SV/EC | Alarm3 Normal Bin 3 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 Normal Bin 3 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38659 | SV/EC | Alarm3 Normal Bin 4 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 Normal Bin 4 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38660 | SV/EC | Alarm3 Normal Bin 4 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 Normal Bin 4 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38661 | SV/EC | Alarm3 Normal Bin 5 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 Normal Bin 5 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38662 | SV/EC | Alarm3 Normal Bin 5 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 Normal Bin 5 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38663 | SV/EC | Alarm3 Normal Bin 6 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 Normal Bin 6 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38664 | SV/EC | Alarm3 Normal Bin 6 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 Normal Bin 6 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38665 | SV/EC | Alarm3 Normal Bin 7 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 Normal Bin 7 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38666 | EC | Alarm3 Normal Bin 7 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38667 | SV/EC | Alarm3 Normal Bin 7 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 Normal Bin 7 By Arm By Bin Site Gap over Limit (%) | ✓ | ⚠️ EC名稱: Code=`Alarm3 Normal Bin 8 By Arm By Bin Site Gap over Limit Enable` / ⚠️ EC型別: Code=BOOLEAN |
| 38668 | SV/EC | Alarm3 Normal Bin 8 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 Normal Bin 8 By Arm By Bin Site Gap over Limit Enable | ✓ | ⚠️ EC名稱: Code=`Alarm3 Normal Bin 8 By Arm By Bin Site Gap over Limit` / ⚠️ EC型別: Code=FT_8 |
| 38669 | SV/EC | Alarm3 Normal Bin 8 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 Normal Bin 8 By Arm By Bin Site Gap over Limit (%) | ✓ | ⚠️ EC名稱: Code=`Alarm3 Normal Bin 9 By Arm By Bin Site Gap over Limit Enable` / ⚠️ EC型別: Code=BOOLEAN |
| 38670 | SV/EC | Alarm3 Normal Bin 9 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 Normal Bin 9 By Arm By Bin Site Gap over Limit Enable | ✓ | ⚠️ EC名稱: Code=`Alarm3 Normal Bin 9 By Arm By Bin Site Gap over Limit` / ⚠️ EC型別: Code=FT_8 |
| 38671 | SV/EC | Alarm3 Normal Bin 9 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 Normal Bin 9 By Arm By Bin Site Gap over Limit (%) | ✓ | ⚠️ EC名稱: Code=`Alarm3 Normal Bin 10 By Arm By Bin Site Gap over Limit Enable` / ⚠️ EC型別: Code=BOOLEAN |
| 38672 | SV/EC | Alarm3 Normal Bin 10 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 Normal Bin 10 By Arm By Bin Site Gap over Limit Enable | ✓ | ⚠️ EC名稱: Code=`Alarm3 Normal Bin 10 By Arm By Bin Site Gap over Limit` / ⚠️ EC型別: Code=FT_8 |
| 38673 | SV/EC | Alarm3 Normal Bin 11 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 Normal Bin 11 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38674 | SV/EC | Alarm3 Normal Bin 11 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 Normal Bin 11 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38675 | SV/EC | Alarm3 Normal Bin 12 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 Normal Bin 12 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38676 | SV/EC | Alarm3 Normal Bin 12 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 Normal Bin 12 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38677 | SV/EC | Alarm3 Normal Bin 13 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 Normal Bin 13 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38678 | SV/EC | Alarm3 Normal Bin 13 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 Normal Bin 13 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38679 | SV/EC | Alarm3 Normal Bin 14 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 Normal Bin 14 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38680 | SV/EC | Alarm3 Normal Bin 14 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 Normal Bin 14 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38681 | SV/EC | Alarm3 Normal Bin 15 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 Normal Bin 15 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38682 | SV/EC | Alarm3 Normal Bin 15 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 Normal Bin 15 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38683 | SV/EC | Alarm3 Normal By Arm By Bin Site Gap over Limit Ignore count | INT_4 |  |  |  |  | Alarm3 Normal By Arm By Bin Site Gap over Limit Ignore count | ✓ |  |
| 38701 | SV/EC | Alarm2 RT Bin 0 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 RT Bin 0 By Bin Site Gap over Limit Enable | ✓ |  |
| 38702 | SV/EC | Alarm2 RT Bin 0 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 RT Bin 0 By Bin Site Gap over Limit (%) | ✓ |  |
| 38703 | SV/EC | Alarm2 RT Bin 1 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 RT Bin 1 By Bin Site Gap over Limit Enable | ✓ |  |
| 38704 | SV/EC | Alarm2 RT Bin 1 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 RT Bin 1 By Bin Site Gap over Limit (%) | ✓ |  |
| 38705 | SV/EC | Alarm2 RT Bin 2 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 RT Bin 2 By Bin Site Gap over Limit Enable | ✓ |  |
| 38706 | SV/EC | Alarm2 RT Bin 2 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 RT Bin 2 By Bin Site Gap over Limit (%) | ✓ |  |
| 38707 | SV/EC | Alarm2 RT Bin 3 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 RT Bin 3 By Bin Site Gap over Limit Enable | ✓ |  |
| 38708 | SV/EC | Alarm2 RT Bin 3 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 RT Bin 3 By Bin Site Gap over Limit (%) | ✓ |  |
| 38709 | SV/EC | Alarm2 RT Bin 4 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 RT Bin 4 By Bin Site Gap over Limit Enable | ✓ |  |
| 38710 | SV/EC | Alarm2 RT Bin 4 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 RT Bin 4 By Bin Site Gap over Limit (%) | ✓ |  |
| 38711 | SV/EC | Alarm2 RT Bin 5 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 RT Bin 5 By Bin Site Gap over Limit Enable | ✓ |  |
| 38712 | SV/EC | Alarm2 RT Bin 5 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 RT Bin 5 By Bin Site Gap over Limit (%) | ✓ |  |
| 38713 | SV/EC | Alarm2 RT Bin 6 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 RT Bin 6 By Bin Site Gap over Limit Enable | ✓ |  |
| 38714 | SV/EC | Alarm2 RT Bin 6 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 RT Bin 6 By Bin Site Gap over Limit (%) | ✓ |  |
| 38715 | SV/EC | Alarm2 RT Bin 7 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 RT Bin 7 By Bin Site Gap over Limit Enable | ✓ |  |
| 38716 | SV/EC | Alarm2 RT Bin 7 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 RT Bin 7 By Bin Site Gap over Limit (%) | ✓ |  |
| 38717 | SV/EC | Alarm2 RT Bin 8 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 RT Bin 8 By Bin Site Gap over Limit Enable | ✓ |  |
| 38718 | SV/EC | Alarm2 RT Bin 8 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 RT Bin 8 By Bin Site Gap over Limit (%) | ✓ |  |
| 38719 | SV/EC | Alarm2 RT Bin 9 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 RT Bin 9 By Bin Site Gap over Limit Enable | ✓ |  |
| 38720 | SV/EC | Alarm2 RT Bin 9 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 RT Bin 9 By Bin Site Gap over Limit (%) | ✓ |  |
| 38721 | SV/EC | Alarm2 RT Bin 10 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 RT Bin 10 By Bin Site Gap over Limit Enable | ✓ |  |
| 38722 | SV/EC | Alarm2 RT Bin 10 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 RT Bin 10 By Bin Site Gap over Limit (%) | ✓ |  |
| 38723 | SV/EC | Alarm2 RT Bin 11 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 RT Bin 11 By Bin Site Gap over Limit Enable | ✓ |  |
| 38724 | SV/EC | Alarm2 RT Bin 11 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 RT Bin 11 By Bin Site Gap over Limit (%) | ✓ |  |
| 38725 | SV/EC | Alarm2 RT Bin 12 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 RT Bin 12 By Bin Site Gap over Limit Enable | ✓ |  |
| 38726 | SV/EC | Alarm2 RT Bin 12 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 RT Bin 12 By Bin Site Gap over Limit (%) | ✓ |  |
| 38727 | SV/EC | Alarm2 RT Bin 13 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 RT Bin 13 By Bin Site Gap over Limit Enable | ✓ |  |
| 38728 | SV/EC | Alarm2 RT Bin 13 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 RT Bin 13 By Bin Site Gap over Limit (%) | ✓ |  |
| 38729 | SV/EC | Alarm2 RT Bin 14 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 RT Bin 14 By Bin Site Gap over Limit Enable | ✓ |  |
| 38730 | SV/EC | Alarm2 RT Bin 14 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 RT Bin 14 By Bin Site Gap over Limit (%) | ✓ |  |
| 38731 | SV/EC | Alarm2 RT Bin 15 By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm2 RT Bin 15 By Bin Site Gap over Limit Enable | ✓ |  |
| 38732 | SV/EC | Alarm2 RT Bin 15 By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm2 RT Bin 15 By Bin Site Gap over Limit (%) | ✓ |  |
| 38733 | SV/EC | Alarm2 RT By Bin Site Gap over LimitIgnore count | INT_4 |  |  |  |  | Alarm2 RT By Bin Site Gap over LimitIgnore count | ✓ | ⚠️ EC名稱: Code=`Alarm2 RT By Bin Site Gap over Limit Ignore count` |
| 38751 | SV/EC | Alarm3 RT Bin 0 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 RT Bin 0 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38752 | SV/EC | Alarm3 RT Bin 0 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 RT Bin 0 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38753 | SV/EC | Alarm3 RT Bin 1 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 RT Bin 1 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38754 | SV/EC | Alarm3 RT Bin 1 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 RT Bin 1 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38755 | SV/EC | Alarm3 RT Bin 2 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 RT Bin 2 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38756 | SV/EC | Alarm3 RT Bin 2 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 RT Bin 2 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38757 | SV/EC | Alarm3 RT Bin 3 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 RT Bin 3 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38758 | SV/EC | Alarm3 RT Bin 3 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 RT Bin 3 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38759 | SV/EC | Alarm3 RT Bin 4 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 RT Bin 4 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38760 | SV/EC | Alarm3 RT Bin 4 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 RT Bin 4 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38761 | SV/EC | Alarm3 RT Bin 5 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 RT Bin 5 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38762 | SV/EC | Alarm3 RT Bin 5 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 RT Bin 5 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38763 | SV/EC | Alarm3 RT Bin 6 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 RT Bin 6 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38764 | SV/EC | Alarm3 RT Bin 6 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 RT Bin 6 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38765 | SV/EC | Alarm3 RT Bin 7 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 RT Bin 7 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38766 | EC | Alarm3 RT Bin 7 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  |  | ✓ | 🟡 程式獨有 |
| 38767 | SV/EC | Alarm3 RT Bin 7 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 RT Bin 7 By Arm By Bin Site Gap over Limit (%) | ✓ | ⚠️ EC名稱: Code=`Alarm3 RT Bin 8 By Arm By Bin Site Gap over Limit Enable` / ⚠️ EC型別: Code=BOOLEAN |
| 38768 | SV/EC | Alarm3 RT Bin 8 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 RT Bin 8 By Arm By Bin Site Gap over Limit Enable | ✓ | ⚠️ EC名稱: Code=`Alarm3 RT Bin 8 By Arm By Bin Site Gap over Limit` / ⚠️ EC型別: Code=FT_8 |
| 38769 | SV/EC | Alarm3 RT Bin 8 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 RT Bin 8 By Arm By Bin Site Gap over Limit (%) | ✓ | ⚠️ EC名稱: Code=`Alarm3 RT Bin 9 By Arm By Bin Site Gap over Limit Enable` / ⚠️ EC型別: Code=BOOLEAN |
| 38770 | SV/EC | Alarm3 RT Bin 9 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 RT Bin 9 By Arm By Bin Site Gap over Limit Enable | ✓ | ⚠️ EC名稱: Code=`Alarm3 RT Bin 9 By Arm By Bin Site Gap over Limit` / ⚠️ EC型別: Code=FT_8 |
| 38771 | SV/EC | Alarm3 RT Bin 9 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 RT Bin 9 By Arm By Bin Site Gap over Limit (%) | ✓ | ⚠️ EC名稱: Code=`Alarm3 RT Bin 10 By Arm By Bin Site Gap over Limit Enable` / ⚠️ EC型別: Code=BOOLEAN |
| 38772 | SV/EC | Alarm3 RT Bin 10 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 RT Bin 10 By Arm By Bin Site Gap over Limit Enable | ✓ | ⚠️ EC名稱: Code=`Alarm3 RT Bin 10 By Arm By Bin Site Gap over Limit` / ⚠️ EC型別: Code=FT_8 |
| 38773 | SV/EC | Alarm3 RT Bin 11 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 RT Bin 11 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38774 | SV/EC | Alarm3 RT Bin 11 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 RT Bin 11 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38775 | SV/EC | Alarm3 RT Bin 12 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 RT Bin 12 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38776 | SV/EC | Alarm3 RT Bin 12 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 RT Bin 12 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38777 | SV/EC | Alarm3 RT Bin 13 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 RT Bin 13 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38778 | SV/EC | Alarm3 RT Bin 13 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 RT Bin 13 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38779 | SV/EC | Alarm3 RT Bin 14 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 RT Bin 14 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38780 | SV/EC | Alarm3 RT Bin 14 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 RT Bin 14 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38781 | SV/EC | Alarm3 RT Bin 15 By Arm By Bin Site Gap over Limit Enable | BOOLEAN |  |  |  |  | Alarm3 RT Bin 15 By Arm By Bin Site Gap over Limit Enable | ✓ |  |
| 38782 | SV/EC | Alarm3 RT Bin 15 By Arm By Bin Site Gap over Limit | FT_8 | Percentage |  |  |  | Alarm3 RT Bin 15 By Arm By Bin Site Gap over Limit (%) | ✓ |  |
| 38783 | SV/EC | Alarm3 RT By Arm By Bin Site Gap over Limit Ignore count | INT_4 |  |  |  |  | Alarm3 RT By Arm By Bin Site Gap over Limit Ignore count | ✓ |  |
| 38800 | SV | Has RTC Module | BOOLEAN |  |  |  |  | Has RTC Module |  | 🔴 文件獨有 |
| 38801 | SV | Has Dual EP Module | BOOLEAN |  |  |  |  | Has Dual EP Module |  | 🔴 文件獨有 |
| 38802 | SV | Has 2DBarcode Module | BOOLEAN |  |  |  |  | Has 2DBarcode Module |  | 🔴 文件獨有 |
| 38803 | SV | Has Heat Gun Module | BOOLEAN |  |  |  |  | Has Heat Gun Module |  | 🔴 文件獨有 |
| 38804 | SV | Has Rotate Module | BOOLEAN |  |  |  |  | Has Rotate Module |  | 🔴 文件獨有 |
| 38805 | SV | Has ATC System Module | BOOLEAN |  |  |  |  | Has ATC System Module |  | 🔴 文件獨有 |
| 38806 | SV | Has OCR Module | BOOLEAN |  |  |  |  | Has OCR Module |  | 🔴 文件獨有 |
| 38807 | SV | Has PRECISER Module | BOOLEAN |  |  |  |  | Has PRECISER Module |  | 🔴 文件獨有 |
| 38808 | SV | Has Shuttle Vibration Module | BOOLEAN |  |  |  |  | Has Shuttle Vibration Module |  | 🔴 文件獨有 |
| 38809 | SV | Has Dew point meter Module | BOOLEAN |  |  |  |  | Has Dew point meter Module |  | 🔴 文件獨有 |
| 38810 | SV | Has Loader Vibration Module | BOOLEAN |  |  |  |  | Has Loader Vibration Module |  | 🔴 文件獨有 |
| 38811 | SV | Has TRAY Vibration Module | BOOLEAN |  |  |  |  | Has TRAY Vibration Module |  | 🔴 文件獨有 |
| 38812 | SV | Has Loader Press Tray Module | BOOLEAN |  |  |  |  | Has Loader Press Tray Module |  | 🔴 文件獨有 |
| 38814 | SV | Has Purge Kit Module | BOOLEAN |  |  |  |  | Has Purge Kit Module |  | 🔴 文件獨有 |
| 38815 | SV | Initial temperature offset function Enable | BOOLEAN |  |  |  |  | Initial temperature offset function Enable |  | 🔴 文件獨有 |
| 38816 | SV | HAS AUTO ALIGNMENT CCD Module | BOOLEAN |  |  |  |  | HAS AUTO ALIGNMENT CCD Module |  | 🔴 文件獨有 |
| 38817 | SV | AUTO ALIGNMENT CCD function Enable | BOOLEAN |  |  |  |  | AUTO ALIGNMENT CCD function Enable |  | 🔴 文件獨有 |
| 38818 | SV | Has Precisor Module | INT_4 |  | 2 | 0 |  | 0 :Not Installed 1 : Installed on Shuttle 2 : Installed on Hotplate |  | 🔴 文件獨有 |
| 38819 | SV | Precisor function Enable | BOOLEAN |  |  |  |  | Precisor function Enable |  | 🔴 文件獨有 |
| 38820 | SV | Precisor RT function Enable | BOOLEAN |  |  |  |  | Precisor RT function Enable |  | 🔴 文件獨有 |
| 38821 | SV | Handling without 2DID | INT_4 |  | 2 | 0 |  | 0 :Follow the test bin 1 : Test and put to error bin 2 : No-test and put to error bin |  | 🔴 文件獨有 |
| 38822 | SV | Has Hotplate vibration Module | BOOLEAN |  |  |  |  | Has Hotplate vibration Module |  | 🔴 文件獨有 |
| 38823 | SV | Has Shuttle vibration Module | INT_4 |  |  |  |  | 0: Un Install 1: Install |  | 🔴 文件獨有 |
| 38824 | SV | Loader vibration  function Enable | BOOLEAN |  |  |  |  | Loader vibration  function Enable |  | 🔴 文件獨有 |
| 38833 | SV | Has Magazine Module | BOOLEAN |  |  |  |  | Has Magazine Module |  | 🔴 文件獨有 |
| 38835 | SV | Base-point | INT_4 |  |  |  |  | 1: 1 Points 2: 2 Points 4: 3 Points 8 5 Points |  | 🔴 文件獨有 |
| 38880 | SV | SECS GEM Version | ASCII |  |  |  |  | SECS GEM Version |  | 🔴 文件獨有 |

## Head / 觸碰 (40000+)
> ⚠️ 此區段有 **11** 筆差異

| ID | 類型 | Name | DataType | Unit | Max | Min | Default | Description | Code | 差異 |
|----|------|------|----------|------|-----|-----|---------|-------------|------|------|
| 62105 | SV | SafeDoorStatus | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 62106 | SV | SenSafeDoor_1 | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 62107 | SV | SenSafeDoor_2 | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 62108 | SV | SenSafeDoor_3 | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 62109 | SV | SenSafeDoor_4 | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 62110 | SV | SenSafeDoor_5 | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 62111 | SV | SenSafeDoor_6 | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 62112 | SV | SenSafeDoor_7 | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 62113 | SV | SenSafeDoor_8 | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 62114 | SV | SenSafeDoor_9 | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 62115 | SV | SenSafeDoor_10 | BOOLEAN |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 65000 | SV | HeadCondition1 Head 1 Contact Count Aa | INT_4 |  |  |  |  |  | ✓ |  |
| 65001 | SV | HeadCondition1 Head 1 Contact Count Ab | INT_4 |  |  |  |  |  | ✓ |  |
| 65002 | SV | HeadCondition1 Head 1 Contact Count Ac | INT_4 |  |  |  |  |  | ✓ |  |
| 65003 | SV | HeadCondition1 Head 1 Contact Count Ad | INT_4 |  |  |  |  |  | ✓ |  |
| 65004 | SV | HeadCondition1 Head 1 Contact Count Ae | INT_4 |  |  |  |  |  | ✓ |  |
| 65005 | SV | HeadCondition1 Head 1 Contact Count Af | INT_4 |  |  |  |  |  | ✓ |  |
| 65006 | SV | HeadCondition1 Head 1 Contact Count Ag | INT_4 |  |  |  |  |  | ✓ |  |
| 65007 | SV | HeadCondition1 Head 1 Contact Count Ah | INT_4 |  |  |  |  |  | ✓ |  |
| 65008 | SV | HeadCondition1 Head 1 Contact Count Ba | INT_4 |  |  |  |  |  | ✓ |  |
| 65009 | SV | HeadCondition1 Head 1 Contact Count Bb | INT_4 |  |  |  |  |  | ✓ |  |
| 65010 | SV | HeadCondition1 Head 1 Contact Count Bc | INT_4 |  |  |  |  |  | ✓ |  |
| 65011 | SV | HeadCondition1 Head 1 Contact Count Bd | INT_4 |  |  |  |  |  | ✓ |  |
| 65012 | SV | HeadCondition1 Head 1 Contact Count Be | INT_4 |  |  |  |  |  | ✓ |  |
| 65013 | SV | HeadCondition1 Head 1 Contact Count Bf | INT_4 |  |  |  |  |  | ✓ |  |
| 65014 | SV | HeadCondition1 Head 1 Contact Count Bg | INT_4 |  |  |  |  |  | ✓ |  |
| 65015 | SV | HeadCondition1 Head 1 Contact Count Bh | INT_4 |  |  |  |  |  | ✓ |  |
| 65016 | SV | HeadCondition1 Head 2 Contact Count Aa | INT_4 |  |  |  |  |  | ✓ |  |
| 65017 | SV | HeadCondition1 Head 2 Contact Count Ab | INT_4 |  |  |  |  |  | ✓ |  |
| 65018 | SV | HeadCondition1 Head 2 Contact Count Ac | INT_4 |  |  |  |  |  | ✓ |  |
| 65019 | SV | HeadCondition1 Head 2 Contact Count Ad | INT_4 |  |  |  |  |  | ✓ |  |
| 65020 | SV | HeadCondition1 Head 2 Contact Count Ae | INT_4 |  |  |  |  |  | ✓ |  |
| 65021 | SV | HeadCondition1 Head 2 Contact Count Af | INT_4 |  |  |  |  |  | ✓ |  |
| 65022 | SV | HeadCondition1 Head 2 Contact Count Ag | INT_4 |  |  |  |  |  | ✓ |  |
| 65023 | SV | HeadCondition1 Head 2 Contact Count Ah | INT_4 |  |  |  |  |  | ✓ |  |
| 65024 | SV | HeadCondition1 Head 2 Contact Count Ba | INT_4 |  |  |  |  |  | ✓ |  |
| 65025 | SV | HeadCondition1 Head 2 Contact Count Bb | INT_4 |  |  |  |  |  | ✓ |  |
| 65026 | SV | HeadCondition1 Head 2 Contact Count Bc | INT_4 |  |  |  |  |  | ✓ |  |
| 65027 | SV | HeadCondition1 Head 2 Contact Count Bd | INT_4 |  |  |  |  |  | ✓ |  |
| 65028 | SV | HeadCondition1 Head 2 Contact Count Be | INT_4 |  |  |  |  |  | ✓ |  |
| 65029 | SV | HeadCondition1 Head 2 Contact Count Bf | INT_4 |  |  |  |  |  | ✓ |  |
| 65030 | SV | HeadCondition1 Head 2 Contact Count Bg | INT_4 |  |  |  |  |  | ✓ |  |
| 65031 | SV | HeadCondition1 Head 2 Contact Count Bh | INT_4 |  |  |  |  |  | ✓ |  |
| 65032 | SV | HeadCondition2 Head 1 Contact Count Aa | INT_4 |  |  |  |  |  | ✓ |  |
| 65033 | SV | HeadCondition2 Head 1 Contact Count Ab | INT_4 |  |  |  |  |  | ✓ |  |
| 65034 | SV | HeadCondition2 Head 1 Contact Count Ac | INT_4 |  |  |  |  |  | ✓ |  |
| 65035 | SV | HeadCondition2 Head 1 Contact Count Ad | INT_4 |  |  |  |  |  | ✓ |  |
| 65036 | SV | HeadCondition2 Head 1 Contact Count Ae | INT_4 |  |  |  |  |  | ✓ |  |
| 65037 | SV | HeadCondition2 Head 1 Contact Count Af | INT_4 |  |  |  |  |  | ✓ |  |
| 65038 | SV | HeadCondition2 Head 1 Contact Count Ag | INT_4 |  |  |  |  |  | ✓ |  |
| 65039 | SV | HeadCondition2 Head 1 Contact Count Ah | INT_4 |  |  |  |  |  | ✓ |  |
| 65040 | SV | HeadCondition2 Head 1 Contact Count Ba | INT_4 |  |  |  |  |  | ✓ |  |
| 65041 | SV | HeadCondition2 Head 1 Contact Count Bb | INT_4 |  |  |  |  |  | ✓ |  |
| 65042 | SV | HeadCondition2 Head 1 Contact Count Bc | INT_4 |  |  |  |  |  | ✓ |  |
| 65043 | SV | HeadCondition2 Head 1 Contact Count Bd | INT_4 |  |  |  |  |  | ✓ |  |
| 65044 | SV | HeadCondition2 Head 1 Contact Count Be | INT_4 |  |  |  |  |  | ✓ |  |
| 65045 | SV | HeadCondition2 Head 1 Contact Count Bf | INT_4 |  |  |  |  |  | ✓ |  |
| 65046 | SV | HeadCondition2 Head 1 Contact Count Bg | INT_4 |  |  |  |  |  | ✓ |  |
| 65047 | SV | HeadCondition2 Head 1 Contact Count Bh | INT_4 |  |  |  |  |  | ✓ |  |
| 65048 | SV | HeadCondition2 Head 2 Contact Count Aa | INT_4 |  |  |  |  |  | ✓ |  |
| 65049 | SV | HeadCondition2 Head 2 Contact Count Ab | INT_4 |  |  |  |  |  | ✓ |  |
| 65050 | SV | HeadCondition2 Head 2 Contact Count Ac | INT_4 |  |  |  |  |  | ✓ |  |
| 65051 | SV | HeadCondition2 Head 2 Contact Count Ad | INT_4 |  |  |  |  |  | ✓ |  |
| 65052 | SV | HeadCondition2 Head 2 Contact Count Ae | INT_4 |  |  |  |  |  | ✓ |  |
| 65053 | SV | HeadCondition2 Head 2 Contact Count Af | INT_4 |  |  |  |  |  | ✓ |  |
| 65054 | SV | HeadCondition2 Head 2 Contact Count Ag | INT_4 |  |  |  |  |  | ✓ |  |
| 65055 | SV | HeadCondition2 Head 2 Contact Count Ah | INT_4 |  |  |  |  |  | ✓ |  |
| 65056 | SV | HeadCondition2 Head 2 Contact Count Ba | INT_4 |  |  |  |  |  | ✓ |  |
| 65057 | SV | HeadCondition2 Head 2 Contact Count Bb | INT_4 |  |  |  |  |  | ✓ |  |
| 65058 | SV | HeadCondition2 Head 2 Contact Count Bc | INT_4 |  |  |  |  |  | ✓ |  |
| 65059 | SV | HeadCondition2 Head 2 Contact Count Bd | INT_4 |  |  |  |  |  | ✓ |  |
| 65060 | SV | HeadCondition2 Head 2 Contact Count Be | INT_4 |  |  |  |  |  | ✓ |  |
| 65061 | SV | HeadCondition2 Head 2 Contact Count Bf | INT_4 |  |  |  |  |  | ✓ |  |
| 65062 | SV | HeadCondition2 Head 2 Contact Count Bg | INT_4 |  |  |  |  |  | ✓ |  |
| 65063 | SV | HeadCondition2 Head 2 Contact Count Bh | INT_4 |  |  |  |  |  | ✓ |  |
| 65064 | SV | HeadCondition3 Head 1 Contact Count Aa | INT_4 |  |  |  |  |  | ✓ |  |
| 65065 | SV | HeadCondition3 Head 1 Contact Count Ab | INT_4 |  |  |  |  |  | ✓ |  |
| 65066 | SV | HeadCondition3 Head 1 Contact Count Ac | INT_4 |  |  |  |  |  | ✓ |  |
| 65067 | SV | HeadCondition3 Head 1 Contact Count Ad | INT_4 |  |  |  |  |  | ✓ |  |
| 65068 | SV | HeadCondition3 Head 1 Contact Count Ae | INT_4 |  |  |  |  |  | ✓ |  |
| 65069 | SV | HeadCondition3 Head 1 Contact Count Af | INT_4 |  |  |  |  |  | ✓ |  |
| 65070 | SV | HeadCondition3 Head 1 Contact Count Ag | INT_4 |  |  |  |  |  | ✓ |  |
| 65071 | SV | HeadCondition3 Head 1 Contact Count Ah | INT_4 |  |  |  |  |  | ✓ |  |
| 65072 | SV | HeadCondition3 Head 1 Contact Count Ba | INT_4 |  |  |  |  |  | ✓ |  |
| 65073 | SV | HeadCondition3 Head 1 Contact Count Bb | INT_4 |  |  |  |  |  | ✓ |  |
| 65074 | SV | HeadCondition3 Head 1 Contact Count Bc | INT_4 |  |  |  |  |  | ✓ |  |
| 65075 | SV | HeadCondition3 Head 1 Contact Count Bd | INT_4 |  |  |  |  |  | ✓ |  |
| 65076 | SV | HeadCondition3 Head 1 Contact Count Be | INT_4 |  |  |  |  |  | ✓ |  |
| 65077 | SV | HeadCondition3 Head 1 Contact Count Bf | INT_4 |  |  |  |  |  | ✓ |  |
| 65078 | SV | HeadCondition3 Head 1 Contact Count Bg | INT_4 |  |  |  |  |  | ✓ |  |
| 65079 | SV | HeadCondition3 Head 1 Contact Count Bh | INT_4 |  |  |  |  |  | ✓ |  |
| 65080 | SV | HeadCondition3 Head 2 Contact Count Aa | INT_4 |  |  |  |  |  | ✓ |  |
| 65081 | SV | HeadCondition3 Head 2 Contact Count Ab | INT_4 |  |  |  |  |  | ✓ |  |
| 65082 | SV | HeadCondition3 Head 2 Contact Count Ac | INT_4 |  |  |  |  |  | ✓ |  |
| 65083 | SV | HeadCondition3 Head 2 Contact Count Ad | INT_4 |  |  |  |  |  | ✓ |  |
| 65084 | SV | HeadCondition3 Head 2 Contact Count Ae | INT_4 |  |  |  |  |  | ✓ |  |
| 65085 | SV | HeadCondition3 Head 2 Contact Count Af | INT_4 |  |  |  |  |  | ✓ |  |
| 65086 | SV | HeadCondition3 Head 2 Contact Count Ag | INT_4 |  |  |  |  |  | ✓ |  |
| 65087 | SV | HeadCondition3 Head 2 Contact Count Ah | INT_4 |  |  |  |  |  | ✓ |  |
| 65088 | SV | HeadCondition3 Head 2 Contact Count Ba | INT_4 |  |  |  |  |  | ✓ |  |
| 65089 | SV | HeadCondition3 Head 2 Contact Count Bb | INT_4 |  |  |  |  |  | ✓ |  |
| 65090 | SV | HeadCondition3 Head 2 Contact Count Bc | INT_4 |  |  |  |  |  | ✓ |  |
| 65091 | SV | HeadCondition3 Head 2 Contact Count Bd | INT_4 |  |  |  |  |  | ✓ |  |
| 65092 | SV | HeadCondition3 Head 2 Contact Count Be | INT_4 |  |  |  |  |  | ✓ |  |
| 65093 | SV | HeadCondition3 Head 2 Contact Count Bf | INT_4 |  |  |  |  |  | ✓ |  |
| 65094 | SV | HeadCondition3 Head 2 Contact Count Bg | INT_4 |  |  |  |  |  | ✓ |  |
| 65095 | SV | HeadCondition3 Head 2 Contact Count Bh | INT_4 |  |  |  |  |  | ✓ |  |

## 接地偵測 / 氣缸計時 (43000-43999)
> ⚠️ 此區段全數 **48** 筆為 🟡 程式獨有（V3.33.902.0 新增）

| ID | 類型 | Name | DataType | Unit | Max | Min | Default | Description | Code | 差異 |
|----|------|------|----------|------|-----|-----|---------|-------------|------|------|
| 43300 | SV | Ground man CH1 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43301 | SV | Ground man CH2 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43302 | SV | Ground man CH3 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43303 | SV | Ground man CH4 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43304 | SV | Ground man CH5 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43305 | SV | Ground man CH6 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43306 | SV | Ground man CH7 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43307 | SV | Ground man CH8 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43308 | SV | Ground man CH9 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43309 | SV | Ground man CH10 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43310 | SV | Ground man CH11 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43311 | SV | Ground man CH12 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43312 | SV | Ground man CH13 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43313 | SV | Ground man CH14 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43314 | SV | Ground man CH15 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43315 | SV | Ground man CH16 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43316 | SV | Ground man CH17 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43317 | SV | Ground man CH18 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43318 | SV | Ground man CH19 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43319 | SV | Ground man CH20 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43320 | SV | Ground man CH21 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43321 | SV | Ground man CH22 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43322 | SV | Ground man CH23 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43323 | SV | Ground man CH24 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43324 | SV | Ground man CH25 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43325 | SV | Ground man CH26 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43326 | SV | Ground man CH27 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43327 | SV | Ground man CH28 Value | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43350 | SV | C_LoaderEdgePush AVG push time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43351 | SV | C_TrayY_Fixer AVG push time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43352 | SV | C_Empty_Fix  AVG push time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43353 | SV | C_Color_Fix AVG push time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43354 | SV | C_Auto1EdgePush AVG push time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43355 | SV | C_Auto1Side_Fixer AVG push time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43356 | SV | C_Auto2EdgePush AVG push time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43357 | SV | C_Auto2Side_Fixer AVG push time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43358 | SV | C_Auto3EdgePush AVG push time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43359 | SV | C_Auto3Side_Fixer AVG push time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43360 | SV | C_LoaderEdgePush AVG pop time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43361 | SV | C_TrayY_Fixer AVG pop time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43362 | SV | C_Empty_Fix  AVG pop time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43363 | SV | C_Color_Fix AVG pop time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43364 | SV | C_Auto1EdgePush AVG pop time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43365 | SV | C_Auto1Side_Fixer AVG pop time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43366 | SV | C_Auto2EdgePush AVG pop time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43367 | SV | C_Auto2Side_Fixer AVG pop time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43368 | SV | C_Auto3EdgePush AVG pop time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
| 43369 | SV | C_Auto3Side_Fixer AVG pop time | ASCII |  |  |  |  |  | ✓ | 🟡 程式獨有 |
