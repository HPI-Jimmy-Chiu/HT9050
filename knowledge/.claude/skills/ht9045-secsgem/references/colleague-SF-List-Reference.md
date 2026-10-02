# SECS/GEM SxFy 訊息總表

> 來源：`uHGemEquipment.cpp` `SFCodeAndMean[]` + `ProcessReceiceData()` dispatch + Excel `SF_List` sheet
> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`

## 訊息統計

| Stream | 範圍 | 訊息數 | 說明 |
|:------:|------|:------:|------|
| S1 | 設備狀態 (Equipment Status) | 12 | |
| S2 | 設備控制 (Equipment Control) | 38 | |
| S5 | 警報管理 (Alarm Management) | 8 | |
| S6 | 資料收集 (Data Collection) | 12 | |
| S7 | 程序管理 (Process Program Management) | 26 | |
| S9 | 錯誤處理 (System Errors) | 7 | |
| S10 | 終端服務 (Terminal Services) | 7 | |
| S14 | 2DID 擴充 (Object Services — Custom) | 2 | |
| S15 | 工作檔管理 (Recipe Management) | 18 | |
| S103 | SVID 含值查詢 (Custom SV Query) | 2 | |
| S110 | Recipe 校驗 (Custom Recipe Verify) | 4 | |
| S120 | Setup File (Custom Setup) | 2 | |
| S125 | EC 啟停 (Custom EC Control) | 4 | |
| **合計** | | **142** | |

## 完整 SxFy 清單

| SxFy | 名稱 | 方向 | HT-704X | HT9045 | HT-9040 | HT-9080A |
|------|------|:----:|:------:|:------:|:------:|:-------:|
| S1F1 | Are You There Request | S,H→E,reply | ✅ | ✅ |  |  |
| S1F2 | On Line Data (D) | S,H→E,reply | ✅ | ✅ |  |  |
| S1F3 | Selected Equipment Status Request (SSR) | S,H→E,reply | ✅ | ✅ |  |  |
| S1F4 | Selected Equipment Status Data (SSD) | M,H←E | ✅ | ✅ |  |  |
| S1F11 | Status Variable Namelist Request (SVNR) | S,H→E,reply | ✅ | ✅ |  |  |
| S1F12 | Status Variable Namelist Reply (SVNRR) | M,H←E | ✅ | ✅ |  |  |
| S1F13 | Establish Communications Request (CR) | S,H↔E,reply | ✅ | ✅ |  |  |
| S1F14 | Establish Communications Request Acknowledge (CRA) | M,H←E | ✅ | ✅ |  |  |
| S1F15 | Request OFF-LINE (ROFL) | S,H→E,reply | ✅ | ✅ |  |  |
| S1F16 | OFF-LINE Acknowledge (OFLA) | S,H←E | ✅ | ✅ |  |  |
| S1F17 | Request ON-LINE (RONL) | S,H→E,reply | ✅ | ✅ |  |  |
| S1F18 | ON-LINE Acknowledge (ONLA) | S,H←E | ✅ | ✅ |  |  |
| S2F13 | Equipment Constant Request (ECR) | S,H→E,reply | ✅ | ✅ |  |  |
| S2F14 | Equipment Constant Data (ECD) | M,H←E | ✅ | ✅ |  |  |
| S2F15 | New Equipment Constant Send (ECS) | S,H→E,reply | ✅ | ✅ |  |  |
| S2F16 | New Equipment Constant Acknowledge (ECA) | S,H←E | ✅ | ✅ |  |  |
| S2F17 | Date and Time Request (DTR) | S,H↔E,reply | ✅ | ✅ |  |  |
| S2F18 | Date and Time Data (DTD) | S,H↔E | ✅ | ✅ |  |  |
| S2F19 | Reset/Initialize Send (RIS) | S,H→E,reply |  |  |  |  |
| S2F20 | Reset Acknowledge (RIA) | S,H←E |  |  |  |  |
| S2F21 | Remote Command Send (RCS) | S,H→E,[reply] |  |  |  |  |
| S2F22 | Remote Command Acknowledge (RCA) | S,H←E |  |  |  |  |
| S2F23 | Trace Initialize Send (TIS) | M,H→E,reply | ✅ | ✅ |  |  |
| S2F24 | Trace Initialize Acknowledge (TIA) | S,H←E | ✅ | ✅ |  |  |
| S2F25 | Loopback Diagnostic Request (LDR) | S,H↔E,reply | ✅ | ✅ |  |  |
| S2F26 | Loopback Diagnostic Data (LDD) | S,H↔E | ✅ | ✅ |  |  |
| S2F27 | Initiate Processing Request (IPR) | S,H→E,reply |  |  |  |  |
| S2F28 | Initiate Processing Acknowledge (IPA) | S,H←E |  |  |  |  |
| S2F29 | Equipment Constant Namelist Request (ECNR) | S,H→E,reply | ✅ | ✅ |  |  |
| S2F30 | Equipment Constant Namelist (ECN) | M,H←E | ✅ | ✅ |  |  |
| S2F31 | Date and Time Set Request (DTS) | S,H→E,reply | ✅ | ✅ |  |  |
| S2F32 | Date and Time Set Acknowledge (DTA) | S,H←E | ✅ | ✅ |  |  |
| S2F33 | Define Report (DR) | M,H→E,reply | ✅ | ✅ |  |  |
| S2F34 | Define Report Acknowledge (DRA) | S,H←E | ✅ | ✅ |  |  |
| S2F35 | Link Event Report (LER) | M,H→E,reply | ✅ | ✅ |  |  |
| S2F36 | Link Event Report Acknowledge (LERA) | S,H←E | ✅ | ✅ |  |  |
| S2F37 | Enable/Disable Event Report (EDER) | S,H→E,reply | ✅ | ✅ |  |  |
| S2F38 | Enable/Disable Event Report Acknowledge (EERA) | S,H←E | ✅ | ✅ |  |  |
| S2F39 | Multi-block Inquire (DMBI) | S,H→E,reply |  |  |  |  |
| S2F40 | Multi-block Grant (DMBG) | S,H←E |  |  |  |  |
| S2F41 | Host Command Send (HCS) | S,H→E,reply |  |  |  |  |
| S2F42 | Host Command Acknowledge (HCA) | S,H←E |  |  |  |  |
| S2F43 | Reset Spooling Streams and Functions (RSSF) | S,H→E,reply |  |  |  |  |
| S2F44 | Reset Spooling Acknowledge (RSA) | M,H←E |  |  |  |  |
| S2F45 | Define Variable Limit Attributes (DVLA) | M,H→E,reply |  |  |  |  |
| S2F46 | Variable Limit Attribute Acknowledge (VLAA) | M,H←E |  |  |  |  |
| S2F47 | Variable Limit Attribute Request (VLAR) | S,H→E,reply |  |  |  |  |
| S2F48 | Variable Limit Attributes Send (VLAS) | M,H←E |  |  |  |  |
| S2F49 | Enhanced Remote Command | M,H→E |  |  |  |  |
| S2F50 | Enhanced Remote Command Acknowledge | M,H←E |  |  |  |  |
| S5F1 | Alarm Report Send (ARS) | S,H←E,[reply] | ✅ | ✅ |  |  |
| S5F2 | Alarm Report Acknowledge (ARA) | S,H→E | ✅ | ✅ |  |  |
| S5F3 | Enable/Disable Alarm Send (EAS) | S,H→E,[reply] | ✅ | ✅ |  |  |
| S5F4 | Enable/Disable Alarm Acknowledge (EAA) | S,H←E | ✅ | ✅ |  |  |
| S5F5 | List Alarms Request (LAR) | S,H→E,reply | ✅ | ✅ |  |  |
| S5F6 | List Alarm Data (LAD) | M,H←E | ✅ | ✅ |  |  |
| S5F7 | List Enabled Alarm Request (LEAR) | S,H→E,reply | ✅ | ✅ |  |  |
| S5F8 | List Enabled Alarm Data (LEAD) | M,H←E | ✅ | ✅ |  |  |
| S6F1 | Trace Data Send (TDS) | M,H←E,[reply] |  |  |  |  |
| S6F2 | Trace Data Acknowledge (TDA) | S,H→E |  |  |  |  |
| S6F5 | Multi-block Data Send Inquire (MBI) | S,H←E,reply |  |  |  |  |
| S6F6 | Multi-block Grant (MBG) | S,H→E |  |  |  |  |
| S6F11 | Event Report Send (ERS) | M,H←E, reply | ✅ | ✅ |  |  |
| S6F12 | Event Report Acknowledge (ERA) | S,H→E | ✅ | ✅ |  |  |
| S6F15 | Event Report Request (ERR) | S,H→E, reply | ✅ | ✅ |  |  |
| S6F16 | Event Report Data (ERD) | M,H←E | ✅ | ✅ |  |  |
| S6F19 | Individual Report Request (IRR) | S,H→E,reply | ✅ | ✅ |  |  |
| S6F20 | Individual Report Data (IRD) | M,H←E | ✅ | ✅ |  |  |
| S6F23 | Request Spooled Data (RSD) | S,H→E,reply |  |  |  |  |
| S6F24 | Request Spooled Data Acknowledgement Send (RSDAS) | S,H←E |  |  |  |  |
| S7F1 | Process Program Load Inquire (PPI) | S,H↔E,reply |  | ✅ |  |  |
| S7F2 | Process Program Load Grant (PPG) | S,H↔E |  | ✅ |  |  |
| S7F3 | Process Program Send (PPS) | M,H↔E,reply |  | ✅ |  |  |
| S7F4 | Process Program Acknowledge (PPA) | S,H↔E |  | ✅ |  |  |
| S7F5 | Process Program Request (PPR) | S,H↔E,reply |  | ✅ |  |  |
| S7F6 | Process Program Data (PPD) | M,H↔E |  | ✅ |  |  |
| S7F17 | Delete Process Program Send (DPS) | S,H→E,reply |  | ✅ |  |  |
| S7F18 | Delete Process Program Acknowledge (DPA) | S,H←E |  | ✅ |  |  |
| S7F19 | Current EPPD Request (RER) | S,H→E,reply |  | ✅ |  |  |
| S7F20 | Current EPPD Data (RED) | M,H←E |  | ✅ |  |  |
| S7F23 | Formatted Process Program Send (FPS) | M,H↔E,reply |  |  |  |  |
| S7F24 | Formatted Process Program Acknowledge (FPA) | S,H↔E |  |  |  |  |
| S7F25 | Formatted Process Program Request (FPR) | S,H↔E,reply |  |  |  |  |
| S7F26 | Formatted Process Program Data (FPD) | M,H↔E |  |  |  |  |
| S7F27 | Process Program Verification Send (PVS) | S,H←E,reply |  |  |  |  |
| S7F28 | Process Program Verification Acknowledge (PVA) | S,H→E |  |  |  |  |
| S7F29 | Process Program Verification Inquire (PVI) | S,H←E,reply |  |  |  |  |
| S7F30 | Process Program Verification Grant (PVG) | S,H→E |  |  |  |  |
| S7F37 | Large Process Program Send (LPPS) | S,H <→ E,reply |  |  |  |  |
| S7F38 | Large Process Program Acknowledge(LPPA) | S,H <→ E |  |  |  |  |
| S7F39 | Large Formatted Process Program Send (LFPPS) | S,H <→ E, reply |  |  |  |  |
| S7F40 | Large Formatted Process Program Acknowledge (LFPPA) | S,H <→ E |  |  |  |  |
| S7F41 | Large Process Program Request(LPPR) | S,H <→ E, reply |  |  |  |  |
| S7F42 | Large Process Program Acknowledge (LPPA) | S,H <→ E |  |  |  |  |
| S7F43 | Large Formatted Process Program Request (LFPPR) | S,H <→ E, reply |  |  |  |  |
| S7F44 | Large Formatted Process Program Acknowledge (LFPPA) | S,H <→ E |  |  |  |  |
| S9F1 | Unrecognized Device ID (UDN) | S,H←E |  |  |  |  |
| S9F3 | Unrecognized Stream Type (USN) | S,H←E |  |  |  |  |
| S9F5 | Unrecognized Function Type (UFN) | S,H←E |  |  |  |  |
| S9F7 | Illegal Data (IDN) | S,H←E |  |  |  |  |
| S9F9 | Transaction Timer Timeout (TTN) | S,H←E |  |  |  |  |
| S9F11 | Data Too Long (DLN) | S,H←E |  |  |  |  |
| S9F13 | Conversation Timeout (CTN) | S,H←E |  |  |  |  |
| S10F1 | Terminal Request (TRN) | S,H←E,[reply] |  |  |  |  |
| S10F2 | Terminal Request Acknowledge (TRA) | S,H→E |  |  |  |  |
| S10F3 | Terminal Display, Single (VTN) | S,H→E, [reply] |  | ✅ |  |  |
| S10F4 | Terminal Display, Single Acknowledge (VTA) | S,H←E |  | ✅ |  |  |
| S10F5 | Terminal Display, Multi-Block (VTN) | M,H→E,[reply] |  | ✅ |  |  |
| S10F6 | Terminal Display, Multi-block Acknowledge (VMA) | S,H←E |  | ✅ |  |  |
| S10F7 | Multi-block Not Allowed (MNN) | S,H←E |  |  |  |  |
| S14F1 | GetAttr Request (GAR) | S,H↔E,reply |  |  |  |  |
| S14F2 | GetAttr Data (GAD) | M,H↔E |  |  |  |  |
| S15F1 | Recipe Management Multi-block Inquire | S,H↔E,reply |  |  |  |  |
| S15F2 | Recipe Management Multi-block Grant | S,H↔E |  |  |  |  |
| S15F21 | Recipe Action Request | M,H↔E,reply |  |  |  |  |
| S15F22 | Recipe Action Acknowledge | M,H↔E |  |  |  |  |
| S15F27 | Recipe Download Request | M,H→E,reply |  |  |  |  |
| S15F28 | Recipe Download Acknowledge | M,H←E |  |  |  |  |
| S15F29 | Recipe Verify Request | M,H→E,reply |  |  |  |  |
| S15F30 | Recipe Verify Acknowledge | M,H←E |  |  |  |  |
| S15F31 | Recipe Upload Request | S,H→E,reply |  |  |  |  |
| S15F32 | Recipe Upload Data | M,H←E |  |  |  |  |
| S15F35 | Recipe Delete Request | M,H→E,reply |  |  |  |  |
| S15F36 | Recipe Delete Acknowledge | M,H←E |  |  |  |  |
| S15F49 | Large Recipe Download Request (LRDR) | S,H→E,reply |  |  |  |  |
| S15F50 | Large Recipe Download Acknowledge (LRDA) | S,H←E |  |  |  |  |
| S15F51 | Large Recipe Upload Request (LRUR) | S,H→E,reply |  |  |  |  |
| S15F52 | Large Recipe Upload Acknowledge (LRUA) | S,H←E |  |  |  |  |
| S15F53 | Recipe Verification Send (RVS) | M,H←E,reply |  |  |  |  |
| S15F54 | Recipe Verification Acknowledge (RVA) | S,H→E |  |  |  |  |
| S103F11 | Status Variable Namelist Request with Value (SVNR) | S,H→E,reply |  |  |  |  |
| S103F12 | Status Variable Namelist Reply with Value (SVNRR) | M,H←E |  |  |  |  |
| S110F5 | Customer Name List Acknowledge | S,H←E,reply |  |  |  |  |
| S110F6 | List Customer Name | S,H→E |  |  |  |  |
| S110F7 | Receipe Information Acknowledge | S,H←E,reply |  |  |  |  |
| S110F8 | Receipe Information Send | S,H→E |  |  |  |  |
| S120F1 | Setup File Information Acknowledge | S,H←E,reply |  |  |  |  |
| S120F2 | Setup File Information Send | S,H→E |  |  |  |  |
| S125F1 | Enable / Disable EC Change Report | S,H→E,reply | ✅ | ✅ |  |  |
| S125F2 | Enable / Disable EC Change Report Acknowledge | M,H←E | ✅ | ✅ |  |  |
| S125F3 | Level Setting Change Request | S,H→E,reply |  |  |  |  |
| S125F4 | Level Setting Change Acknowledge | M,H←E |  |  |  |  |

---

## 方向代碼對照

| 代碼 | 意義 |
|:----:|------|
| S | 送出 (Send) |
| M | 多播 (Multicast) |
| H→E | Host 傳送至 Equipment |
| H←E | Equipment 傳送至 Host |
| H↔E | 雙向 |
| reply | 需要回覆（W-Bit=1） |