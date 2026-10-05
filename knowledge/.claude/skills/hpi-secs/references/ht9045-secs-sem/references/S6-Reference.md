# Stream 6：資料收集 (Data Collection)

> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`
> 資料來源：Excel `SECS_20260401_Steven.xlsx` + 原始碼分析
> 實作檔案：`uHGemClass.cpp`、`uHGemEquipment.cpp`、`uHGemHT9045.cpp`

## 概述

Stream 6 處理事件報告（Event Report）、離散變數、個別報告與 Spool 資料。設備端透過 S6F11 主動報告事件。

## 訊息一覽

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 處理函式 |
|------|------|:----:|:-----:|:------:|----------|
| S6F1 | Trace Data Send (TDS) | M,H←E,[reply] | 1 | ❌ | — |
| S6F2 | Trace Data Acknowledge (TDA) | S,H→E | 0 | ❌ | — |
| S6F5 | Multi-block Data Send Inquire (MBI) | S,H←E,reply | 1 | ❌ | — |
| S6F6 | Multi-block Grant (MBG) | S,H→E | 0 | ❌ | — |
| S6F11 | Event Report Send (ERS) | M,H←E, reply | 1 | ✅ | — |
| S6F12 | Event Report Acknowledge (ERA) | S,H→E | 0 | ✅ | `S6F12_EventReportAcknowledge()` |
| S6F15 | Event Report Request (ERR) | S,H→E, reply | 1 | ✅ | — |
| S6F16 | Event Report Data (ERD) | M,H←E | 0 | ✅ | `S6F16_EventReportSendAcknowledge()` |
| S6F19 | Individual Report Request (IRR) | S,H→E,reply | 1 | ✅ | — |
| S6F20 | Individual Report Data (IRD) | M,H←E | 0 | ✅ | `S6F20_IndividualReportData()` |
| S6F23 | Request Spooled Data (RSD) | S,H→E,reply | 1 | ❌ | — |
| S6F24 | Request Spooled Data Acknowledgement Send (RSDAS) | S,H←E | 0 | ❌ | `S6F24_RequestSpoolData()` |

## 詳細說明

### S6F1 / S6F2 — Trace Data Send (TDS) / Trace Data Acknowledge (TDA)

| 屬性 | S6F1 |
|------|---|
| **方向** | M,H←E,[reply] |
| **HT9045** | ❌ |
| | |
| 屬性 | S6F2 |
| **方向** | S,H→E |
| **HT9045** | ❌ |

**說明**

This function sends samples to the host according to the trace setup done by S2,F23. Trace is a time-driven form of equipment  
status.  
Even if S6,F1 is multi-block, it is not preceded by an Inquire/Grant transaction, because the Host S2,F23 is an implicit grant.  
Some equipment may support only single-block S6,F1, and may refuse an S2,F23 (Trace Initiate Send) message which would  
cause a multi-block S6,F1.

**資料結構**

*S6F1*:
```
L,4
    1. <TRID>
    2. <SMPLN>
    3. <STIME>
    4. L,n
        1. <SV1>
        2. <SV2>
         .
         .
        n. <SVn>
```

*S6F2*:
```
<ACKC6>
```

**變數**

| 變數 | 說明 |
|------|------|
| TRID | |
| SMPLN | |
| STIME | |
| SV | |
| ACKC | |

**例外**

S6F1: A zero-length STIME means no value is given and that the time is to be derived from SMPLN along with knowledge of the  
request.  
S6F2: None

---

### S6F5 / S6F6 — Multi-block Data Send Inquire (MBI) / Multi-block Grant (MBG)

| 屬性 | S6F5 |
|------|---|
| **方向** | S,H←E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S6F6 |
| **方向** | S,H→E |
| **HT9045** | ❌ |

**說明**

If the discrete data report S6F3, F9, F11, F13 can involve more than one block, this transaction must precede the transmission.

**資料結構**

*S6F5*:
```
L,2
    1. <DATAID>
    2. <DATALENGTH>
```

*S6F6*:
```
<GRANT6>
```

**變數**

| 變數 | 說明 |
|------|------|
| DATAID | |
| DATALENGTH | |
| GRANT | |

---

### S6F11 / S6F12 — Event Report Send (ERS) / Event Report Acknowledge (ERA)

| 屬性 | S6F11 |
|------|---|
| **方向** | M,H←E, reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S6F12 |
| **方向** | S,H→E |
| **HT9045** | ✅ |
| **處理函式** | `S6F12_EventReportAcknowledge()` |

**說明**

The purpose of this message is for the equipment to send a defined, linked, and enabled group of reports to the host upon the  
occurrence of an event (CEID).  
If S6,F11 is Multi-block, it must be preceded by the S6,F5/S6,F6 Inquire/Grant transaction.

**資料結構**

*S6F11*:
```
L,3
    1. <DATAID>
    2. <CEID>
    3. L,a
        1. L,2
            1. <RPTID1>
            2. L,b
                1. <V1>
                 .
                 .
                b. <Vb>
            .
            .
        a. L,2               report a
           1. <RPTIDa>
           2. L,c            #Vs this report
               1. <V1>
                .
                .
               c. <Vc>
```

*S6F12*:
```
<ACKC6>
```

**變數**

| 變數 | 說明 |
|------|------|
| DATAID | |
| CEID | |
| RPTID | |
| V | |
| RPTID | |
| ACKC6 | |

**例外**

S6F11: If there are no reports linked to the event a ‘null’ report is assumed. A zero-length list for # of reports means there are no reports  
linked to the given CEID.  
S6F12: None

---

### S6F15 / S6F16 — Event Report Request (ERR) / Event Report Data (ERD)

| 屬性 | S6F15 |
|------|---|
| **方向** | S,H→E, reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S6F16 |
| **方向** | M,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S6F16_EventReportSendAcknowledge()` |

**說明**

The purpose of this message is for the host to demand a given report group from the equipment.

**資料結構**

*S6F15*:
```
<CEID>
```

*S6F16*:
```
Identical to structure of S6,F11.
```

**變數**

| 變數 | 說明 |
|------|------|
| CEID | |

---

### S6F19 / S6F20 — Individual Report Request (IRR) / Individual Report Data (IRD)

| 屬性 | S6F19 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S6F20 |
| **方向** | M,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S6F20_IndividualReportData()` |

**說明**

The purpose of this message is for the host to request a defined report from the equipment.

**資料結構**

*S6F19*:
```
<RPTID>
```

*S6F20*:
```
L,n            # of variable data items
    1. <V1>
     .
     .
    n. <Vn>
```

**變數**

| 變數 | 說明 |
|------|------|
| RPTID | |
| V | |

---

### S6F23 / S6F24 — Request Spooled Data (RSD) / Request Spooled Data Acknowledgement Send (RSDAS)

| 屬性 | S6F23 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S6F24 |
| **方向** | S,H←E |
| **HT9045** | ❌ |
| **處理函式** | `S6F24_RequestSpoolData()` |

**說明**

The purpose of this message is for the host to request transmission or deletion of the messages currently spooled by the  
equipment.

**資料結構**

*S6F23*:
```
<RSDC>
```

*S6F24*:
```
<RSDA>
```

**變數**

| 變數 | 說明 |
|------|------|
| RSDC | |
| RSDA | |

---
---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S6F1 | Trace Data Send | H←E | 0 | ❌ | [Web] |
| S6F2 | Trace Data Ack | H→E | 0 | ❌ | [Web] |
| S6F3 | Discrete Variable Data Send | H←E | 0 | ❌ | [Web] |
| S6F4 | Discrete Variable Data Send Ack | H→E | 0 | ❌ | [Web] |
| S6F5 | Multi-block Data Send Inquire | H←E | 1 | ❌ | [Web] |
| S6F6 | Multi-block Grant | H→E | 0 | ❌ | [Web] |
| S6F7 | Data Transfer Request | H→E | 1 | ❌ | [Web] |
| S6F8 | Data Transfer Data | H←E | 0 | ❌ | [Web] |
| S6F9 | Formatted Variable Send | H←E | 0 | ❌ | [Web] |
| S6F10 | Formatted Variable Ack | H→E | 0 | ❌ | [Web] |
| S6F11 | Event Report Send | H←E | 1 | ✅ |  |
| S6F12 | Event Report Ack | H→E | 0 | ✅ |  |
| S6F13 | Annotated Event Report Send | H←E | 1 | ❌ | [Web] |
| S6F14 | Annotated Event Report Ack | H→E | 0 | ❌ | [Web] |
| S6F15 | Event Report Request | H→E | 1 | ✅ |  |
| S6F16 | Event Report Data | H←E | 0 | ✅ |  |
| S6F17 | Annotated Event Report Request | H→E | 1 | ❌ | [Web] |
| S6F18 | Annotated Event Report Data | H←E | 0 | ❌ | [Web] |
| S6F19 | Individual Report Request | H→E | 1 | ✅ |  |
| S6F20 | Individual Report Data | H←E | 0 | ✅ |  |
| S6F21 | Annotated Individual Report Request | H→E | 1 | ❌ | [Web] |
| S6F22 | Annotated Individual Report Data | H←E | 0 | ❌ | [Web] |
| S6F23 | Request or Purge Spooled Data | H→E | 1 | ❌ | [Web] |
| S6F24 | Request or Purge Spooled Data Ack | H←E | 0 | ❌ | [Web] |
| S6F25 | Notification Report Send | H↔E | 0 | ❌ | [Web] |
| S6F26 | Notification Report Send Ack | H↔E | 0 | ❌ | [Web] |
| S6F27 | Trace Report Send | H←E | 0 | ❌ | [Web] |
| S6F28 | Trace Report Send Ack | H→E | 0 | ❌ | [Web] |
| S6F29 | Trace Report Request | H→E | 1 | ❌ | [Web] |
| S6F30 | Trace Report Data | H←E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S6F1 — Trace Data Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 TRID
SMPLN
STIME
{L:n SV
```

---

#### S6F2 — Trace Data Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC6
```

---

#### S6F3 — Discrete Variable Data Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Discrete implies event-based such as analysis completion. Use S2F15 to choose supported events.

**資料結構**

```
{L:3 DATAID
CEID
{L:n {L:2 DSID
{L:m {L:2 DVNAME
DVVAL
```

---

#### S6F4 — Discrete Variable Data Send Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `S6F4_DiscreteVariableData()` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC6
```

---

#### S6F5 — Multi-block Data Send Inquire `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

for SECS-I and S6F3 F9 F11 and F13, not required for HSMS

**資料結構**

```
{L:2 DATAID
DATALENGTH
```

---

#### S6F6 — Multi-block Grant `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
GRANT6
```

---

#### S6F7 — Data Transfer Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
DATAID
```

---

#### S6F8 — Data Transfer Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 DATAID
CEID
{L:n DSID
{L:m {L:2 DVNAME
DVVAL
```

---

#### S6F9 — Formatted Variable Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

like S6F3 without names

**資料結構**

```
{L:4 PFCD
DATAID
CEID
{L:n {L:2 DSID
{L:m DVVAL
```

---

#### S6F10 — Formatted Variable Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC6
```

---

#### S6F11 — Event Report Send

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**資料結構**

```
{L:3 DATAID
CEID
{L:a {L:2 RPTID
{L:b V
```

---

#### S6F12 — Event Report Ack

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S6F12_EventReportAcknowledge()` |

**資料結構**

```
ACKC6
```

---

#### S6F13 — Annotated Event Report Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 DATAID
CEID
{L:a {L:2 RPTID
{L:b {L:2 VID
V
```

---

#### S6F14 — Annotated Event Report Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC6
```

---

#### S6F15 — Event Report Request

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**資料結構**

```
CEID
```

---

#### S6F16 — Event Report Data

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S6F16_EventReportSendAcknowledge()` |

**資料結構**

```
{L:3 DATAID
CEID
{L:a {L:2 RPTID
{L:b V
```

---

#### S6F17 — Annotated Event Report Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
CEID
```

---

#### S6F18 — Annotated Event Report Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 DATAID
CEID
{L:a {L:2 RPTID
{L:b {L:2 VID
V
```

---

#### S6F19 — Individual Report Request

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**資料結構**

```
RPTID
```

---

#### S6F20 — Individual Report Data

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S6F20_IndividualReportData()` |

**資料結構**

```
{L:n V
```

---

#### S6F21 — Annotated Individual Report Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
RPTID
```

---

#### S6F22 — Annotated Individual Report Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `S6F22_AnnotatedIndividualReportData()` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:n {L:2 VID
V
```

---

#### S6F23 — Request or Purge Spooled Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
RSDC
```

---

#### S6F24 — Request or Purge Spooled Data Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `S6F24_RequestSpoolData()` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
RSDA
```

---

#### S6F25 — Notification Report Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:7 DATAID
OPID
LINKID
RCPSPEC
RMCHGSTAT
{L:m {L:2 RCPATTRID
RCPATTRDATA
```

---

#### S6F26 — Notification Report Send Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC6
```

---

#### S6F27 — Trace Report Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 DATAID
TRID
{L:n {L:p {L:2 RPTID
{L:m V
```

---

#### S6F28 — Trace Report Send Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
TRID
```

---

#### S6F29 — Trace Report Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
TRID
```

---

#### S6F30 — Trace Report Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

ERRCODE is set to zero length when there is no error.

**資料結構**

```
{L:3 TRID
{L:n {L:2 RPTID
{L:m V
```

---
