# Stream 2：設備控制 (Equipment Control)

> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`
> 資料來源：Excel `SECS_20260401_Steven.xlsx` + 原始碼分析
> 實作檔案：`uHGemClass.cpp`、`uHGemEquipment.cpp`、`uHGemHT9045.cpp`

## 概述

Stream 2 處理設備常數讀寫、遠端指令、動態報告定義、警報啟停、時間同步與追蹤資料設定。

## 訊息一覽

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 處理函式 |
|------|------|:----:|:-----:|:------:|----------|
| S2F13 | Equipment Constant Request (ECR) | S,H→E,reply | 1 | ✅ | — |
| S2F14 | Equipment Constant Data (ECD) | M,H←E | 0 | ✅ | `S2F14_EquipmentConstantData()` |
| S2F15 | New Equipment Constant Send (ECS) | S,H→E,reply | 1 | ✅ | — |
| S2F16 | New Equipment Constant Acknowledge (ECA) | S,H←E | 0 | ✅ | `S2F16_NewEquipmentConstantAcknowledge()` |
| S2F17 | Date and Time Request (DTR) | S,H↔E,reply | 1 | ✅ | — |
| S2F18 | Date and Time Data (DTD) | S,H↔E | 0 | ✅ | `S2F18_DateTimeData()` |
| S2F19 | Reset/Initialize Send (RIS) | S,H→E,reply | 1 | — | — |
| S2F20 | Reset Acknowledge (RIA) | S,H←E | 0 | — | — |
| S2F21 | Remote Command Send (RCS) | S,H→E,[reply] | 1 | — | — |
| S2F22 | Remote Command Acknowledge (RCA) | S,H←E | 0 | — | `S2F22_RemoteCommandAcknowledge()` |
| S2F23 | Trace Initialize Send (TIS) | M,H→E,reply | 1 | ✅ | — |
| S2F24 | Trace Initialize Acknowledge (TIA) | S,H←E | 0 | ✅ | `S2F24_TraceInitializeAcknowledge()` |
| S2F25 | Loopback Diagnostic Request (LDR) | S,H↔E,reply | 1 | ✅ | — |
| S2F26 | Loopback Diagnostic Data (LDD) | S,H↔E | 0 | ✅ | — |
| S2F27 | Initiate Processing Request (IPR) | S,H→E,reply | 1 | — | — |
| S2F28 | Initiate Processing Acknowledge (IPA) | S,H←E | 0 | — | — |
| S2F29 | Equipment Constant Namelist Request (ECNR) | S,H→E,reply | 1 | ✅ | — |
| S2F30 | Equipment Constant Namelist (ECN) | M,H←E | 0 | ✅ | `S2F30_EquipmentConstantNamelist()` |
| S2F31 | Date and Time Set Request (DTS) | S,H→E,reply | 1 | ✅ | — |
| S2F32 | Date and Time Set Acknowledge (DTA) | S,H←E | 0 | ✅ | `S2F32_DateTimeSetAcknowledge()` |
| S2F33 | Define Report (DR) | M,H→E,reply | 1 | ✅ | — |
| S2F34 | Define Report Acknowledge (DRA) | S,H←E | 0 | ✅ | `S2F34_DefineReportAcknowledge()` |
| S2F35 | Link Event Report (LER) | M,H→E,reply | 1 | ✅ | — |
| S2F36 | Link Event Report Acknowledge (LERA) | S,H←E | 0 | ✅ | `S2F36_LinkEventReportAcknowledge()` |
| S2F37 | Enable/Disable Event Report (EDER) | S,H→E,reply | 1 | ✅ | — |
| S2F38 | Enable/Disable Event Report Acknowledge (EERA) | S,H←E | 0 | ✅ | `S2F38_EnableAlarmAcknowledge()` |
| S2F39 | Multi-block Inquire (DMBI) | S,H→E,reply | 1 | ❌ | — |
| S2F40 | Multi-block Grant (DMBG) | S,H←E | 0 | ❌ | — |
| S2F41 | Host Command Send (HCS) | S,H→E,reply | 1 | ❌ | — |
| S2F42 | Host Command Acknowledge (HCA) | S,H←E | 0 | ❌ | `S2F42_HostCommandAcknowledge()` |
| S2F43 | Reset Spooling Streams and Functions (RSSF) | S,H→E,reply | 1 | ❌ | — |
| S2F44 | Reset Spooling Acknowledge (RSA) | M,H←E | 0 | ❌ | `S2F44_ResetSpool()` |
| S2F45 | Define Variable Limit Attributes (DVLA) | M,H→E,reply | 1 | ❌ | — |
| S2F46 | Variable Limit Attribute Acknowledge (VLAA) | M,H←E | 0 | ❌ | — |
| S2F47 | Variable Limit Attribute Request (VLAR) | S,H→E,reply | 1 | ❌ | — |
| S2F48 | Variable Limit Attributes Send (VLAS) | M,H←E | 0 | ❌ | — |
| S2F49 | Enhanced Remote Command | M,H→E | 0 | ❌ | — |
| S2F50 | Enhanced Remote Command Acknowledge | M,H←E | 0 | ❌ | — |

## 詳細說明

### S2F13 / S2F14 — Equipment Constant Request (ECR) / Equipment Constant Data (ECD)

| 屬性 | S2F13 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S2F14 |
| **方向** | M,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S2F14_EquipmentConstantData()` |

**說明**

Constants such as for calibration, servo gain, alarm limits, data collection mode, and other values that are changed infrequently  
can be obtained using this message.  
from the primary message.

**資料結構**

*S2F13*:
```
The following structure is approved for all item formats and should be used by all new
implementations:
L,n
   1. <ECID1>
    .
    .
    n. <ECIDn>
The following structure is included for compatibility with previous implementations
and may only be used for items of format 3() and 5():
<ECID1, . . . , ECIDn>
```

*S2F14*:
```
L,n
       1. <ECV1>
       2. <ECV2>
       .
       .
       n. <ECVn>
```

**變數**

| 變數 | 說明 |
|------|------|
| ECID | |
| ECV | |

**例外**

S2F13: A zero-length list (structure1) or item (structure2) means report all ECVs according to a predefined order.  
S2F14: A zero-length list item for ECVi means that ECIDi does not exist. The list format for this data item is not allowed, except in this  
case.

---

### S2F15 / S2F16 — New Equipment Constant Send (ECS) / New Equipment Constant Acknowledge (ECA)

| 屬性 | S2F15 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S2F16 |
| **方向** | S,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S2F16_NewEquipmentConstantAcknowledge()` |

**說明**

Change one or more equipment constants.

**資料結構**

*S2F15*:
```
L,n
1. L,2
        1. <ECID1>
        2. <ECV1>
 2. L,2
  .
  .
 n. L,2
        1. <ECIDn>
        2. <ECVn>
```

*S2F16*:
```
<EAC>
```

**變數**

| 變數 | 說明 |
|------|------|
| ECID | |
| ECV | |
| EAC | |

---

### S2F17 / S2F18 — Date and Time Request (DTR) / Date and Time Data (DTD)

| 屬性 | S2F17 |
|------|---|
| **方向** | S,H↔E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S2F18 |
| **方向** | S,H↔E |
| **HT9045** | ✅ |
| **處理函式** | `S2F18_DateTimeData()` |

**說明**

Useful to check equipment time base or for equipment to synchronize with the host time base.

**資料結構**

*S2F17*:
```
Header only
```

*S2F18*:
```
<TIME>
```

**變數**

| 變數 | 說明 |
|------|------|
| TIME | |

**例外**

A zero-length item means no time exists.

---

### S2F19 / S2F20 — Reset/Initialize Send (RIS) / Reset Acknowledge (RIA)

| 屬性 | S2F19 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | — |
| | |
| 屬性 | S2F20 |
| **方向** | S,H←E |
| **HT9045** | — |

**說明**

Causes equipment to reach one of several predetermined initialized conditions.

**資料結構**

*S2F19*:
```
<RIC>
```

*S2F20*:
```
<RAC>
```

**變數**

| 變數 | 說明 |
|------|------|
| RIC | |
| RAC | |

---

### S2F21 / S2F22 — Remote Command Send (RCS) / Remote Command Acknowledge (RCA)

| 屬性 | S2F21 |
|------|---|
| **方向** | S,H→E,[reply] |
| **HT9045** | — |
| | |
| 屬性 | S2F22 |
| **方向** | S,H←E |
| **HT9045** | — |
| **處理函式** | `S2F22_RemoteCommandAcknowledge()` |

**說明**

Similar to pressing buttons on the front panel or causes some equipment activity to commence or to cease.

**資料結構**

*S2F21*:
```
<RCMD>
```

*S2F22*:
```
<CMDA>
```

**變數**

| 變數 | 說明 |
|------|------|
| RCMD | |
| CMDA | |

---

### S2F23 / S2F24 — Trace Initialize Send (TIS) / Trace Initialize Acknowledge (TIA)

| 屬性 | S2F23 |
|------|---|
| **方向** | M,H→E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S2F24 |
| **方向** | S,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S2F24_TraceInitializeAcknowledge()` |

**說明**

Status variables exist at all times. This function provides a way to sample a subset of those status variables as a function of time.  
The trace data is returned on S6,F1 and is related to the original request by the TRID Multiple trace requests may be made to that  
equipment allowing it. If equipment receives S2,F23 with the same TRID as a trace function that is currently in progress, the  
equipment should terminate the old trace and then initiate the new trace. A trace function currently in progress may be  
terminated by S2,F23 with TRID of that trace and TOTSMP = 0.  
If S2,F23 is multi-block, it must be preceded by the S2,F39/S2,F40 Inquire/Grant transaction. Some equipment may support  
only single-Block S6,F1, and may refuse a S2,F23 message which would cause a multi-block S6,F1.  
Each equipment shall document its trace performance limits. The Host Computer shall not send an S2,F23 which exceeds the  
equipment’s performance limits, or the equipment may operate incorrectly.

**資料結構**

*S2F23*:
```
The following structure is approved for all item formats and should be used by all new
implementations:
L,5
     1. <TRID>                   // Trace request ID 
     2. <DSPER>               // Data sample period
     3. <TOTSMP>           // Total samples to be made
     4. <REPGSZ>            // Reporting group size 
     5. L,n
            1. <SVID1>
            .
            .
            n. <SVIDn>
The following structure is included for compatibility with previous implementations
and may only be used for items whose SVID is format 3() and 5():
L,5
     1. <TRID>
     2. <DSPER>
     3. <TOTSMP>
     4. <REPGSZ>
     5. <SVID1, . . . , SVIDn>
```

*S2F24*:
```
<TIAACK>
```

**變數**

| 變數 | 說明 |
|------|------|
| TRID | |
| DSPER | |
| TOTSMP | |
| REPGSZ | |
| SVID | |
| TIAACK | |

---

### S2F25 / S2F26 — Loopback Diagnostic Request (LDR) / Loopback Diagnostic Data (LDD)

| 屬性 | S2F25 |
|------|---|
| **方向** | S,H↔E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S2F26 |
| **方向** | S,H↔E |
| **HT9045** | ✅ |

**說明**

A diagnostic message for checkout of protocol and communication circuits. The binary string sent is echoed back.

**資料結構**

*S2F25*:
```
<ABS>
```

*S2F26*:
```
<ABS>
```

**變數**

| 變數 | 說明 |
|------|------|
| ABS | |

---

### S2F27 / S2F28 — Initiate Processing Request (IPR) / Initiate Processing Acknowledge (IPA)

| 屬性 | S2F27 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | — |
| | |
| 屬性 | S2F28 |
| **方向** | S,H←E |
| **HT9045** | — |

**說明**

Host requests equipment to initiate processing of the identified material at the specified location in the machine using the  
specified process program.

**資料結構**

*S2F27*:
```
L,3
    1. <LOC>
    2. <PPID>
    3. L,n
            1. <MID1>
            .
            .
            n. <MIDn>
```

*S2F28*:
```
<CMDA>
```

**變數**

| 變數 | 說明 |
|------|------|
| LOC | |
| PPID | |
| MID | |
| CMDA | |

**例外**

S2F27: A zero-length PPID indicates no process program is being specified and the equipment is to take whatever action is appropriate  
for it to determine the proper program to use. A zero-length MID list indicates no MID is to be associated with the material to be  
processed.  
S2F28: None

---

### S2F29 / S2F30 — Equipment Constant Namelist Request (ECNR) / Equipment Constant Namelist (ECN)

| 屬性 | S2F29 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S2F30 |
| **方向** | M,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S2F30_EquipmentConstantNamelist()` |

**說明**

This function allows the host to retrieve basic information about what equipment constants are available in the equipment.

**資料結構**

*S2F29*:
```
L,n
    1. <ECID1>
    .
    .
    n. <ECIDn>
```

*S2F30*:
```
L,n (number of equipment constants)
1. L,6
    1. <ECID1>
    2. <ECNAME1>
    3. <ECMIN1>
    4. <ECMAX1>
    5. <ECDEF1>
    6. <UNITS1>
2. L,6
        .
        .
        n. L,6
        1. <ECIDn>
        2. <ECNAMEn>
        3. <ECMINn>
        4. <ECMAXn>
        5. <ECDEFn>
        6. <UNITSn>
```

**變數**

| 變數 | 說明 |
|------|------|
| ECID | |
| ECNAME | |
| ECMIN | |
| ECMAX | |
| ECDEF | |
| UNITS | |

**例外**

S2F29: A zero-length list means send information for all ECIDs.  
S2F30: Zero-length ASCII items for ECNAMEi , ECMINi, ECMAXi, ECDEFi, and UNITSi indicates that the ECID does not exist.

---

### S2F31 / S2F32 — Date and Time Set Request (DTS) / Date and Time Set Acknowledge (DTA)

| 屬性 | S2F31 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S2F32 |
| **方向** | S,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S2F32_DateTimeSetAcknowledge()` |

**說明**

Useful to synchronize the equipment time with the host time base.

**資料結構**

*S2F31*:
```
<TIME>
```

*S2F32*:
```
<TIACK>
```

**變數**

| 變數 | 說明 |
|------|------|
| TIME | |
| TIACK | |

---

### S2F33 / S2F34 — Define Report (DR) / Define Report Acknowledge (DRA)

| 屬性 | S2F33 |
|------|---|
| **方向** | M,H→E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S2F34 |
| **方向** | S,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S2F34_DefineReportAcknowledge()` |

**說明**

The purpose of this message is for the host to define a group of reports for the equipment.  
The type of report to be transmitted is designated by a Boolean “Equipment Constant.” An “Equipment Constant Value” of  
“False” means that an “Event Report”(S6,F11) will be sent, and a value of “True” means that an “Annotated Event  
Report”(S6,F13) will be sent.If S2,F33 is Multi-block, it must be preceded by the S2,F39/S2,F40 Inquire/Grant transaction.

**資料結構**

*S2F33*:
```
L,2
    1. <DATAID>
    2. L,a # reports
          1. L,2 report 1
                  1. <RPTID1>
                  2. L,b # VIDs this report
                          1. <VID1>
                           .
                           .
                           b.<VIDb>
           a. L,2 report a
                  1. <RPTIDa>
                  2. L,c # VIDs this report
                         1. <VID1>
                          .
                          .
                         c. <VIDc>
```

*S2F34*:
```
<DRACK>
```

**變數**

| 變數 | 說明 |
|------|------|
| VID | |
| DATAID | |
| RPTID | |
| DRACK | |

**例外**

S2F33: 1. A list of zero-length following <DATAID> deletes all report definitions and associated links. See S2,F35 (Link  
Event/Report).  
2. A list of zero-length following <RPTID> deletes report type RPTID. All CEID links to this RPTID are also deleted.  
S2F34: None

**DRACK 回應碼（SEMI E30）**

| DRACK | 意義 | 處理方向 |
|:-----:|------|----------|
| `0x00` | Accepted — Define Report 成功 | 無 |
| `0x01` | Denied — 設備空間不足 | 減少 report 數量 |
| `0x02` | Denied — **至少一個 VID（SVID/ECID）不存在** | **修正 EAP 端 VID 清單** |
| `0x03` | Denied — 至少一個 RPTID 已被定義 | 先清除 report |
| `0x04` | Denied — RPTID 已存在且 VID 清單不同 | 清除/重命名 RPTID |

> ⚠️ **DRACK = `0x02` 是 atomic 拒絕**：S2F33 中只要有任一 VID 不存在，**整個訊息**被拒絕，
> 所有 RPTID 都不會被建立。後續 S6F11 事件報告會被壓制（log：`CEID be disabled, abort send`）。

**重要原則：SVID/ECID 是全客戶共用的固定集合**

HT9045 / HT9046 的 SVID/ECID 號碼是**同一套、全客戶共用**（號碼範圍約 1000～65095）。
機台**不會也不应該**為個別客戶變更 SVID/ECID 的號碼。當客戶 EAP 回報 DRACK=0x02 時，
正確解法是**客戶依據該機台軟體版本的官方 SVID/ECID 清單，去調整 EAP 端的 mapping**，
而非要求原廠修改機台程式去配合某一客戶的號碼。詳見 [Known-Issues-and-Workarounds.md](Known-Issues-and-Workarounds.md) Issue #4。

---

### S2F35 / S2F36 — Link Event Report (LER) / Link Event Report Acknowledge (LERA)

| 屬性 | S2F35 |
|------|---|
| **方向** | M,H→E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S2F36 |
| **方向** | S,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S2F36_LinkEventReportAcknowledge()` |

**說明**

The purpose of this message is for the host to link n reports to an event (CEID). These linked event reports will default to  
‘disabled’ upon linking. That is, the occurrence of an event would not cause the report to be sent until enabled. See S2,F37 for  
enabling reports.  
If S2,F35 is Multi-block, it must be preceded by the S2,F39/S2,F40 Inquire/Grant transaction.

**資料結構**

*S2F35*:
```
L,2
    1. <DATAID>
    2. L,a # events
      1. L,2 event 1
          1. <CEID1>
          2. L,b
               1. <RPTID1>
               .
               .
               b. <RPTIDb>
               .
               .
          a. L,2 event a
          1. <CEIDa> # RPTIDS this event
          2.L,c
              1.<RPTID1>
              .
              .
              c. <RPTIDc>
```

*S2F36*:
```
<LRACK>
```

**變數**

| 變數 | 說明 |
|------|------|
| DATAID | |
| CEID | |
| RPTID | |
| LRACK | |

**例外**

S2F35: A list of zero length following CEID deletes all report links to that event.  
S2F36: None

---

### S2F37 / S2F38 — Enable/Disable Event Report (EDER) / Enable/Disable Event Report Acknowledge (EERA)

| 屬性 | S2F37 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S2F38 |
| **方向** | S,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S2F38_EnableAlarmAcknowledge()` |

**說明**

The purpose of this message is for the host to enable or disable reporting for a group of events (CEIDs).

**資料結構**

*S2F37*:
```
L,2
1. <CEED> enable/disable
2. L,n #CEIDs
    1. <CEID1>
      .
      .
    n. <CEIDn>
```

*S2F38*:
```
<ERACK>
```

**變數**

| 變數 | 說明 |
|------|------|
| CEED | |
| CEID | |
| ERACK | |

**例外**

S2F37: A list of zero length following <CEED> means all CEIDs.  
S2F38: None

---

### S2F39 / S2F40 — Multi-block Inquire (DMBI) / Multi-block Grant (DMBG)

| 屬性 | S2F39 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S2F40 |
| **方向** | S,H←E |
| **HT9045** | ❌ |

**說明**

If a S2,F23 S2,F33, S2,F35, S2,F45, or S2,F49 message is more than one block, this transaction must precede the message.

**資料結構**

*S2F39*:
```
L,2
    1. <DATAID>
    2. <DATALENGTH>
```

*S2F40*:
```
<GRANT>
```

**變數**

| 變數 | 說明 |
|------|------|
| DATAID | |
| DATALENGTH | |
| GRANT | |

---

### S2F41 / S2F42 — Host Command Send (HCS) / Host Command Acknowledge (HCA)

| 屬性 | S2F41 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S2F42 |
| **方向** | S,H←E |
| **HT9045** | ❌ |
| **處理函式** | `S2F42_HostCommandAcknowledge()` |

**說明**

The Host requests the Equipment perform the specified remote command with the associated parameters.

**資料結構**

*S2F41*:
```
L,2
1. <RCMD>
2. L,n # of parameters
       1. L,2
              1. <CPNAME1> parameter 1 name
              2. <CPVAL1> parameter 1 value
      .
      .
      n. L,2
             1. <CPNAMEn> parameter n name
             2. <CPVALn> parameter n value
```

*S2F42*:
```
L,2
    1. <HCACK>
    2. L,n # of parameters
        1. L,2
                1. <CPNAME1> parameter 1 name
                2. <CPACK1> parameter 1 reason
     .
     .
     n. L,2
            1. <CPNAMEn> parameter n name
            2. <CPACKn> parameter n reason
```

**變數**

| 變數 | 說明 |
|------|------|
| RCMD="PPSIGNALTOWER" 方式
L,2
    <A[] "PPSIGNALTOWER">
    L,3                                                                    ; 若是 L0 表示回到 Handler 自行控制
        L,2
              <A[n] "RED">
              <U4[1] m>                                          ; m=0  表示 Off  ,  m=1 On  ,  m=2 閃爍
        L,2
              <A[n] "GREEN">
              <U4[1] m>                                          ; m=0  表示 Off  ,  m=1 On  ,  m=2 閃爍
        L,2
              <A[n] "YELLOW">
              <U4[1] m>                                          ; m=0  表示 Off  ,  m=1 On  ,  m=2 閃爍 | |
| CPNAME | |
| RCMD | |
| CPVAL | |
| HCACK | |
| CPACK | |

**例外**

S2F41: None  
S2F42: If there are no invalid parameters, then a list of zero length will be sent for item 2.

---

### S2F43 / S2F44 — Reset Spooling Streams and Functions (RSSF) / Reset Spooling Acknowledge (RSA)

| 屬性 | S2F43 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S2F44 |
| **方向** | M,H←E |
| **HT9045** | ❌ |
| **處理函式** | `S2F44_ResetSpool()` |

**說明**

This message allows the host to select specific streams and functions to be spooled whenever spooling is active.

**資料結構**

*S2F43*:
```
L,m
1. L,2
        1. <STRID1>
        2. L,n
               1. <FCNID1>
               .
               .
               n. <FCNIDn>
               .
               .
              m. L,2
                      1. <STRIDm>
                      2. L,n
                          1. <FCNID1>
                          .
                          .
                         n. <FCNIDn>
```

*S2F44*:
```
L,2
1. <RSPACK> (accept or reject)
2. L,m (m = number of streams with errors)
    1. L,3
           1. <STRID1>
           2. <STRACK1> (error in stream)
           3. L,n (n = number of functions in error)
                   1. <FCNID1>
                   .
                   .
                  n. <FCNIDn>
                   .
                   .
                  m. L,3
                          1. <STRIDm>
                          2. <STRACKm> (error in stream)
                          3. L,n (n = number of functions in error)
                                 1. <FCNID1>
                                 .
                                 .
                                n. <FCNIDn>
```

**變數**

| 變數 | 說明 |
|------|------|
| STRID | |
| FCNID | |
| RSPACK | |

**例外**

S2F43: 1. A zero-length list, m = 0, turns off spooling for all streams and functions.  
2. A zero-length list, n = 0, turns on spooling for all functions for the associated stream.  
Notes:  
1. Turning off spooling for all functions for a specific stream is achieved by omitting reference to the stream from this message.  
2. Spooling for Stream 1 is not allowed.  
3. Equipment must allow host to spool all primary messages for a stream (except Stream 1).  
4. A defined list of functions for a stream in this message will replace any previously selected functions.  
S2F44: 1. If RSPACK = 0, a zero-length list, m = 0, is given, indicating no streams or functions in error.  
2. A zero-length list, n = 0, indicates no functions in error for specified stream.

---

### S2F45 / S2F46 — Define Variable Limit Attributes (DVLA) / Variable Limit Attribute Acknowledge (VLAA)

| 屬性 | S2F45 |
|------|---|
| **方向** | M,H→E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S2F46 |
| **方向** | M,H←E |
| **HT9045** | ❌ |

**說明**

（無說明）

**資料結構**

*S2F45*:
```
L,2
1. <DATAID>
2. L,m (m = # of variables in this definition)
1. L,2
1. <VID1>
2. L,n (n = # of limits being defined/changed for VID1)
1. L,2
1. <LIMITID1>
2. L,p (p = {0,2})
1. <UPPERDB1>
2. <LOWERDB1>
.
.
n. L,2
1. <LIMITIDn>
2. L,p (p = {0,2})
1. <UPPERDBn>
2. <LOWERDBn>
.
.
m.L,2
1. <VIDm>
2. L,n (n = # of limits being defined/changed for VIDm)
1. L,2
1. <LIMITID1>
2. L,p (p = {0,2})
1. <UPPERDB1>
2. <LOWERDB1>
.
.
n. L,2
1. <LIMITIDn>
2. L,p (p = {0,2})
1. <UPPERDBn>
2. <LOWERDBn>
```

*S2F46*:
```
L,2
   1. <VLAACK>
   2. L,m (m = number of invalid parameters)
       1. L,3
              1. <VID1> (VID with error)
              2. <LVACKp> (reason)
              3. L,n {n = 0,2}
                     1. <LIMITID1> (1st limit in error for VIDp)
                     2. <LIMITACK1> (reason)
                     .
                     .
                    m. L,3
                            1. <VIDm> (VID with error)
                            2. <LVACKm> (reason)
                            3. L,n {n = 0,2}
                                   1. <LIMITID1> (1st limit in error for VIDx)
                                   2. <LIMITACK1> (reason)
```

**變數**

| 變數 | 說明 |
|------|------|
| DATAID | |
| VID | |
| LIMITID | |
| UPPERDB | |
| LOWERDB | |
| VLAACK | |
| LVACK | |
| LIMITACK | |

**例外**

S2F45: 1. A zero-length list, m = 0, sets all limit values for all monitored VIDs to “undefined.”  
2. A zero-length list, n = 0, sets all limits values for that VID to “undefined.”  
3. A zero-length list, p = 0, sets that limit to “undefined.”  
S2F46: 1. A zero-length list, m = 0 indicates no invalid variable limit attributes.  
2. A zero-length list, n = 0 indicates no invalid limit values for that VID.

---

### S2F47 / S2F48 — Variable Limit Attribute Request (VLAR) / Variable Limit Attributes Send (VLAS)

| 屬性 | S2F47 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S2F48 |
| **方向** | M,H←E |
| **HT9045** | ❌ |

**說明**

This message allows the host to query the equipment for current variable limit attribute definitions.

**資料結構**

*S2F47*:
```
L,m (m = # of VIDs this request)
    1. <VID1>
    
    .
    m. <VIDm>
```

*S2F48*:
```
L,m (m = # of VIDs this request)
1. L,2
       1. <VID1>
       2. L,p {p = 0,4}
              1. <UNITS1>
              2. <LIMITMIN1>
              3. <LIMITMAX1>
              4. L,n (n = # of limits defined for this VID)
                  1. L,3
                  1. <LIMITID1>
                  2. <UPPERDB1>
                  3. <LOWERDB1>
                   .
                   .
                   n. L,3
                          1. <LIMITIDn>
                          2. <UPPERDBn>
                          3. <LOWERDBn>
                          .
                          .
                         m. L,2
                                 1.<VIDm>
                                 2. L,p {p = 0,4}
                                        1. <UNITSm>
                                        2. <LIMITMINm>
                                        3. <LIMITMAXm>
                                        4. L,n (n = # of limits defined for this VID)
                                             1. L,3
                                                     1. <LIMITID1>
                                                     2. <UPPERDB1>
                                                     3. <LOWERDB1>
                                                     .
                                                     .
                                                     n. L,3
                                                             1. <LIMITIDn>
                                                             2. <UPPERDBn>
                                                             3. <LOWERDBn>
```

**變數**

| 變數 | 說明 |
|------|------|
| VID | |
| UNITS | |
| LIMITMIN | |
| LIMITMAX | |
| LIMITID | |
| UPPERDB | |
| LOWERDB | |
| 1. A zero-length list, p = 0, indicates that limits are not supported for the VID.
2. A zero-length list, n = 0, means no limits are currently defined for the specified variable. | |

**例外**

A zero-length list, m = 0, requests a list of all VID values that can have variable limit attributes.

---

### S2F49 / S2F50 — Enhanced Remote Command / Enhanced Remote Command Acknowledge

| 屬性 | S2F49 |
|------|---|
| **方向** | M,H→E |
| **HT9045** | ❌ |
| | |
| 屬性 | S2F50 |
| **方向** | M,H←E |
| **HT9045** | ❌ |

**說明**

The host requests an object to perform the specified remote command with its associated parameters. If multi-block, it shall be  
preceded by the S2,F39/S2,F40 Multi-Block Inquire/Grant transaction.

**資料結構**

*S2F49*:
```
L,4
   1. <DATAID>
   2. <OBJSPEC>
   3. <RCMD>
   4. L,m # of parameter groups
       1. L,2
              1. <CPNAME1> command parameter 1 name
              2. <CEPVAL1> command-enhanced parameter 1 value
       2. L,2
               1. <CPNAME2> command parameter 2 name
               2. <CEPVAL2> command-enhanced parameter 2 value
               .
               .
              m. L,2
                      1. <CPNAMEm> command parameter m name
                      2. <CEPVALm> command enhanced parameter m value
If a specific value of CPNAME is defined to have a CEPVAL defined as a LIST, it shall
always be a LIST. If the CEPVAL that is associated to that specific value of CPNAME
is defined to be anything other than LIST, it will result in a format error.
```

*S2F50*:
```
L,2
   1. <HCACK>
   2. L,n # of parameter groups
       1. L,2
              1. <CPNAME1>
              2. <CEPACK1>
              .
              .
              n. L,2
                     1. <CPNAMEn>
                     2. <CEPACKn>
```

**變數**

| 變數 | 說明 |
|------|------|
| DATAID | |
| OBJSPEC | |
| RCMD | |
| CPNAME | |
| CEPVAL | |
| HCACK | |
| CEPACK | |

**例外**

S2F49: A zero length list, m = 0, indicates that no parameter groups are sent with the command. OBJSPEC can be a null length item.  
Notes:  
1. If CEPVAL is a LIST, the items that make up that list shall take on one of the following forms: (1) a list of items with an  
identical format, (2) a LIST of CPNAME, CEPVAL pairs, as illustrated below.  
A) L,2  
1. <CPNAMEa>  
2. L,m  
1. <CPVALa1>  
2. <CPVALa2>  
.  
.  
m. <CPVALam>  
B) L,2  
1. <CPNAMEb>  
2. L,n  
1. L,2  
1. <CPNAMEb1>  
2. <CEPVALb1>  
.  
.  
n. L,2  
1. <CPNAMEbn>  
2. <CEPVALbn>  
S2F50: None

---
---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S2F1 | Service Program Load Inquire | H↔E | 0 | ❌ | [Web] |
| S2F2 | Service Program Load Grant | H↔E | 0 | ❌ | [Web] |
| S2F3 | Service Program Send | H↔E | 0 | ❌ | [Web] |
| S2F4 | Service Program Send Acknowledge | H↔E | 0 | ❌ | [Web] |
| S2F5 | Service Program Load Request | H↔E | 0 | ❌ | [Web] |
| S2F6 | Service Program Load Data | H↔E | 0 | ❌ | [Web] |
| S2F7 | Service Program Run Send | H↔E | 0 | ❌ | [Web] |
| S2F8 | Service Program Run Acknowledge | H↔E | 0 | ❌ | [Web] |
| S2F9 | Service Program Results Request | H↔E | 0 | ❌ | [Web] |
| S2F10 | Service Program Results Data | H↔E | 0 | ❌ | [Web] |
| S2F11 | Service Program Directory Request | H↔E | 0 | ❌ | [Web] |
| S2F12 | Service Program Directory Data | H↔E | 0 | ❌ | [Web] |
| S2F13 | Equipment Constant Request | H→E | 1 | ✅ |  |
| S2F14 | Equipment Constant Data | H←E | 0 | ✅ |  |
| S2F15 | New Equipment Constant Send | H→E | 1 | ✅ |  |
| S2F16 | New Equipment Constant Ack | H←E | 0 | ✅ |  |
| S2F17 | Date and Time Request | H↔E | 1 | ✅ |  |
| S2F18 | Date and Time Data | H↔E | 0 | ✅ |  |
| S2F19 | Reset/Initialize Send | H→E | 1 | ❌ | [Web] |
| S2F20 | Reset Acknowledge | H←E | 0 | ❌ | [Web] |
| S2F21 | Remote Command Send | H→E | 0 | ❌ | [Web] |
| S2F22 | Remote Command Acknowledge | H←E | 0 | ❌ | [Web] |
| S2F23 | Trace Initialize Send | H→E | 1 | ✅ |  |
| S2F24 | Trace Initialize Acknowledge | H←E | 0 | ✅ |  |
| S2F25 | Loopback Diagnostic Request | H↔E | 1 | ✅ |  |
| S2F26 | Loopback Diagnostic Data | H↔E | 0 | ✅ |  |
| S2F27 | Initiate Processing Request | H→E | 1 | ❌ | [Web] |
| S2F28 | Initiate Processing Acknowledge | H←E | 0 | ❌ | [Web] |
| S2F29 | Equipment Constant Namelist Request | H→E | 1 | ✅ |  |
| S2F30 | Equipment Constant Namelist | H←E | 0 | ✅ |  |
| S2F31 | Date and Time Set Request | H→E | 1 | ✅ |  |
| S2F32 | Date and Time Set Acknowledge | H←E | 0 | ✅ |  |
| S2F33 | Define Report | H→E | 1 | ✅ |  |
| S2F34 | Define Report Acknowledge | H←E | 0 | ✅ |  |
| S2F35 | Link Event Report | H→E | 1 | ✅ |  |
| S2F36 | Link Event Report Acknowledge | H←E | 0 | ✅ |  |
| S2F37 | Enable/Disable Event Report | H→E | 1 | ✅ |  |
| S2F38 | Enable/Disable Event Report Acknowledge | H←E | 0 | ✅ |  |
| S2F39 | Multi-block Inquire | H→E | 1 | ❌ | [Web] |
| S2F40 | Multi-block Grant | H←E | 0 | ❌ | [Web] |
| S2F41 | Host Command Send | H→E | 1 | ❌ | [Web] |
| S2F42 | Host Command Acknowledge | H←E | 0 | ❌ | [Web] |
| S2F43 | Configure Spooling | H→E | 1 | ❌ | [Web] |
| S2F44 | Configure Spooling Acknowledge | H←E | 0 | ❌ | [Web] |
| S2F45 | Define Variable Limit Attributes | H→E | 1 | ❌ | [Web] |
| S2F46 | Define Variable Limit Attributes Acknowledge | H←E | 0 | ❌ | [Web] |
| S2F47 | Variable Limit Attribute Request | H→E | 1 | ❌ | [Web] |
| S2F48 | Variable Limit Attribute Send | H←E | 0 | ❌ | [Web] |
| S2F49 | Enhanced Remote Command | H→E | 1 | ❌ | [Web] |
| S2F50 | Enhanced Remote Command Acknowledge | H←E | 0 | ❌ | [Web] |
| S2F51 | Request Report Identifiers | H→E | 1 | ❌ | [Web] |
| S2F52 | Return Report Identifiers | H←E | 0 | ❌ | [Web] |
| S2F53 | Request Report Definitions | H→E | 1 | ❌ | [Web] |
| S2F54 | Return Report Definitions | H←E | 0 | ❌ | [Web] |
| S2F55 | Request Event Report Links | H→E | 1 | ❌ | [Web] |
| S2F56 | Return Event Report Links | H←E | 0 | ❌ | [Web] |
| S2F57 | Request Enabled Events | H→E | 1 | ❌ | [Web] |
| S2F58 | Return Enabled Events | H←E | 0 | ❌ | [Web] |
| S2F59 | Request Spool Streams and Functions | H→E | 1 | ❌ | [Web] |
| S2F60 | Return Spool Streams and Functions | H←E | 0 | ❌ | [Web] |
| S2F61 | Request Trace Identifiers | H→E | 1 | ❌ | [Web] |
| S2F62 | Return Trace Identifiers | H←E | 0 | ❌ | [Web] |
| S2F63 | Request Trace Definitions | H→E | 1 | ❌ | [Web] |
| S2F64 | Return Trace Definitions | H←E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S2F1 — Service Program Load Inquire `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 SPID
LENGTH
```

---

#### S2F2 — Service Program Load Grant `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
GRANT
```

---

#### S2F3 — Service Program Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
SPD
```

---

#### S2F4 — Service Program Send Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
SPAACK
```

---

#### S2F5 — Service Program Load Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
SPID
```

---

#### S2F6 — Service Program Load Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
SPD
```

---

#### S2F7 — Service Program Run Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
SPID
```

---

#### S2F8 — Service Program Run Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
CSAACK
```

---

#### S2F9 — Service Program Results Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
SPID
```

---

#### S2F10 — Service Program Results Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
SPR
```

---

#### S2F11 — Service Program Directory Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
header only
```

---

#### S2F12 — Service Program Directory Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:n SPID
```

---

#### S2F13 — Equipment Constant Request

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**說明**

Host sends L:0 to receive all in predefined order

**資料結構**

```
{L:n ECID
```

---

#### S2F14 — Equipment Constant Data

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S2F14_EquipmentConstantData()` |

**說明**

An L:0 is substituted if an ECID does not exist, L format is not allowed for an ECV.

**資料結構**

```
{L:n ECV
```

---

#### S2F15 — New Equipment Constant Send

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**說明**

Constant is a misnomer, they are settable parameters.

**資料結構**

```
{L:n {L:2 ECID
ECV
```

---

#### S2F16 — New Equipment Constant Ack

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S2F16_NewEquipmentConstantAcknowledge()` |

**說明**

if any input value is not proper, none of the input values are set

**資料結構**

```
EAC
```

---

#### S2F17 — Date and Time Request

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**說明**

NTP servers should be used instead for better accuracy and to avoid lack of time zone specification.

**資料結構**

```
header only
```

---

#### S2F18 — Date and Time Data

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S2F18_DateTimeData()` |

**說明**

The value should not be used for system clock synchronization.

**資料結構**

```
TIME
```

---

#### S2F19 — Reset/Initialize Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
RIC
```

---

#### S2F20 — Reset Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
RAC
```

---

#### S2F21 — Remote Command Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Compare to S2F41R. Arguments can be passed as you would on a command line.

**資料結構**

```
RCMD
```

---

#### S2F22 — Remote Command Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `S2F22_RemoteCommandAcknowledge()` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
CMDA
```

---

#### S2F23 — Trace Initialize Send

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**說明**

Set TOTSMP=0 to terminate a trace. Set zero-length TRID to delete all traces.

**資料結構**

```
{L:5 TRID
DSPER
TOTSMP
REPGSZ
{L:n SVID
```

---

#### S2F24 — Trace Initialize Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S2F24_TraceInitializeAcknowledge()` |

**資料結構**

```
TIAACK
```

---

#### S2F25 — Loopback Diagnostic Request

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**說明**

it is useful to test the telnet escape sequence

**資料結構**

```
ABS
```

---

#### S2F26 — Loopback Diagnostic Data

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |

**資料結構**

```
ABS
```

---

#### S2F27 — Initiate Processing Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

made obsolete by S2F41R ?

**資料結構**

```
{L:3 LOC
PPID
{L:n MID
```

---

#### S2F28 — Initiate Processing Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
CMDA
```

---

#### S2F29 — Equipment Constant Namelist Request

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**說明**

Host sends L:0 for all ECIDs

**資料結構**

```
{L:n ECID
```

---

#### S2F30 — Equipment Constant Namelist

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S2F30_EquipmentConstantNamelist()` |

**資料結構**

```
{L:n {L:6 ECID
ECNAME
ECMIN
ECMAX
ECDEF
UNITS
```

---

#### S2F31 — Date and Time Set Request

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**說明**

Modern equipment should use NTP servers to synchronize the system clock. SECS is inaccurate and does not include timezone info.

**資料結構**

```
TIME
```

---

#### S2F32 — Date and Time Set Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S2F32_DateTimeSetAcknowledge()` |

**資料結構**

```
TIACK
```

---

#### S2F33 — Define Report

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**說明**

a=0 means delete all reports and event links, b=0 means delete the RPTID type and its event links

**資料結構**

```
{L:2 DATAID
{L:a {L:2 RPTID
{L:b VID
```

---

#### S2F34 — Define Report Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S2F34_DefineReportAcknowledge()` |

**資料結構**

```
DRACK
```

---

#### S2F35 — Link Event Report

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**說明**

b=0 disassociates the CEID from all report links. The message links reports to an event, it does not imply a particular output sequence in the event report.

**資料結構**

```
{L:2 DATAID
{L:a {L:2 CEID
{L:b RPTID
```

---

#### S2F36 — Link Event Report Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S2F36_LinkEventReportAcknowledge()` |

**資料結構**

```
LRACK
```

---

#### S2F37 — Enable/Disable Event Report

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**說明**

n=0 means all CEIDs

**資料結構**

```
{L:2 CEED
{L:n CEID
```

---

#### S2F38 — Enable/Disable Event Report Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S2F38_EnableAlarmAcknowledge()` |

**資料結構**

```
ERACK
```

---

#### S2F39 — Multi-block Inquire `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

for SECS-I mullti-block S2F23 F33, F35, F45, or F49, not required for HSMS

**資料結構**

```
{L:2 DATAID
DATALENGTH
```

---

#### S2F40 — Multi-block Grant `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
GRANT
```

---

#### S2F41 — Host Command Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RCMD
{L:n {L:2 CPNAME
CPVAL
```

---

#### S2F42 — Host Command Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `S2F42_HostCommandAcknowledge()` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

n is equal to the number of parameters having errors

**資料結構**

```
{L:2 HCACK
{L:n {L:2 CPNAME
CPACK
```

---

#### S2F43 — Configure Spooling `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

m=0 turns off all streams and fns, n=0 turns on all functions in the stream, stream 1 is not spooled

**資料結構**

```
{L:m {L:2 STRID
{L:n FCNID
```

---

#### S2F44 — Configure Spooling Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `S2F44_ResetSpool()` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

m=0 when there are no errors; n is the number of functions in error in a stream

**資料結構**

```
{L:2 RSPACK
{L:m {L:3 STRID
STRACK
{L:n FCNID
```

---

#### S2F45 — Define Variable Limit Attributes `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

L:2* can be L:2 or L:0

**資料結構**

```
{L:2 DATAID
{L:m {L:2 VID
{L:n {L:2 LIMITID
{L:2* UPPERDB
LOWERDB
```

---

#### S2F46 — Define Variable Limit Attributes Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

L:2* can be L:2 or L:0, varies with VID

**資料結構**

```
{L:2 VLAACK
{L:m {L:3 VID
LVACK
{L:2* LIMITID
LIMITACK
```

---

#### S2F47 — Variable Limit Attribute Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Host sends L:0 to discover VIDs that support limits

**資料結構**

```
{L:m VID
```

---

#### S2F48 — Variable Limit Attribute Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

L:4* can be L:4 or L:0

**資料結構**

```
{L:m {L:2 VID
{L:4* UNITS
LIMITMIN
LIMITMAX
{L:n {L:3 LIMITID
UPPERDB
LOWERDB
```

---

#### S2F49 — Enhanced Remote Command `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

the standard fails to show the message is sent with reply requested; CEPVAL can be a list of same format values or CPNAME CEPVAL pairs, possible nested list inclusion is implied

**資料結構**

```
{L:4 DATAID
OBJSPEC
RCMD
{L:m {L:2 CPNAME
CEPVAL
```

---

#### S2F50 — Enhanced Remote Command Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

CEPACK can be a list structure with possible nesting of additional list structures

**資料結構**

```
{L:2 HCACK
{L:n {L:2 CPNAME
CEPACK
```

---

#### S2F51 — Request Report Identifiers `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Host sends S2F51R to discover defined RPTIDs, new 8/2021

**資料結構**

```
header only
```

---

#### S2F52 — Return Report Identifiers `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:n RPTID
```

---

#### S2F53 — Request Report Definitions `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Host sends L:0 for all defined reports, new 8/2021

**資料結構**

```
{L:n RPTID
```

---

#### S2F54 — Return Report Definitions `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

a is 0 for unknown RPTID, n is 0 for no reports defined and L:0 request, new 8/2021

**資料結構**

```
{L:n {L:2 RPTID
{L:a VID
```

---

#### S2F55 — Request Event Report Links `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Host sends L:0 to discover linked events and links, new 8/2021

**資料結構**

```
{L:n CEID
```

---

#### S2F56 — Return Event Report Links `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

CENAME is A:0 for unknown CEID, a is 0 for no linked reports, L:0 reply to L:0 if no links defined, new 8/2021

**資料結構**

```
{L:n {L:3 CEID
CENAME
{L:a RPTID
```

---

#### S2F57 — Request Enabled Events `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Host sends header-only to discover events that are enabled for reporting, new 8/2021

**資料結構**

```
header only
```

---

#### S2F58 — Return Enabled Events `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:n CEID
```

---

#### S2F59 — Request Spool Streams and Functions `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Host sends header-only to discover SFs configured for spooling, new 8/2021

**資料結構**

```
header only
```

---

#### S2F60 — Return Spool Streams and Functions `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

n=0 indicates spooling disabled, b=0 implies all Fs in stream, new 8/2021

**資料結構**

```
{L:n {L:2 STRID
{L:a FCNID
```

---

#### S2F61 — Request Trace Identifiers `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Host sends header-only to discover S2F23 trace reports, new 8/2021

**資料結構**

```
header only
```

---

#### S2F62 — Return Trace Identifiers `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Host configures traces with S2F23, new message 8/2021

**資料結構**

```
{L:n TRID
```

---

#### S2F63 — Request Trace Definitions `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Host sends L:0 for all traces setup with S2F23, new 8/2021

**資料結構**

```
{L:n TRID
```

---

#### S2F64 — Return Trace Definitions `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

zero-length items sent for unknown TRID, n=0 for no traces defined and L:0 query, new 8/2021

**資料結構**

```
{L:n {L:5 TRID
DSPER
TOTSMP
REPGSZ
{L:a SVID
```

---
