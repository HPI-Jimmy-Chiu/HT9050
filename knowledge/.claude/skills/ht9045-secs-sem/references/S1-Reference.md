# Stream 1：設備狀態 (Equipment Status)

> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`
> 資料來源：Excel `SECS_20260401_Steven.xlsx` + 原始碼分析
> 實作檔案：`uHGemClass.cpp`、`uHGemEquipment.cpp`、`uHGemHT9045.cpp`

## 概述

Stream 1 處理設備與主機之間的連線建立、狀態查詢與變數名稱查詢。

## 訊息一覽

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 處理函式 |
|------|------|:----:|:-----:|:------:|----------|
| S1F1 | Are You There Request | S,H→E,reply | 1 | ✅ | `S1F1_AreYouThereRequest()` |
| S1F2 | On Line Data (D) | S,H→E,reply | 1 | ✅ | `S1F2_OnLineData()` |
| S1F3 | Selected Equipment Status Request (SSR) | S,H→E,reply | 1 | ✅ | — |
| S1F4 | Selected Equipment Status Data (SSD) | M,H←E | 0 | ✅ | `S1F4_SelectedStatusReply()` |
| S1F11 | Status Variable Namelist Request (SVNR) | S,H→E,reply | 1 | ✅ | — |
| S1F12 | Status Variable Namelist Reply (SVNRR) | M,H←E | 0 | ✅ | `S1F12_StatusVariableNamelistReply()` |
| S1F13 | Establish Communications Request (CR) | S,H↔E,reply | 1 | ✅ | `S1F13_EstablishCommunicationsRequest()` |
| S1F14 | Establish Communications Request Acknowledge (CRA) | M,H←E | 0 | ✅ | `S1F14_ConnectRequestAcknowledge()` |
| S1F15 | Request OFF-LINE (ROFL) | S,H→E,reply | 1 | ✅ | — |
| S1F16 | OFF-LINE Acknowledge (OFLA) | S,H←E | 0 | ✅ | `S1F16_OFFLINEAcknowledge()` |
| S1F17 | Request ON-LINE (RONL) | S,H→E,reply | 1 | ✅ | — |
| S1F18 | ON-LINE Acknowledge (ONLA) | S,H←E | 0 | ✅ | `S1F18_ONLINEAcknowledge()` |

## 詳細說明

### S1F1 / S1F2 — Are You There Request / On Line Data (D)

| 屬性 | S1F1 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ✅ |
| **處理函式** | `S1F1_AreYouThereRequest()` |
| | |
| 屬性 | S1F2 |
| **方向** | S,H→E,reply |
| **HT9045** | ✅ |
| **處理函式** | `S1F2_OnLineData()` |

**說明**

Establishes if the equipment is on-line. A function 0 response to this message means the communication is inoperative. In the  
equipment, a function 0 is equivalent to a timeout on the receive timer after issuing S1,F1 to the host.

**資料結構**

*S1F1*:
```
Header only
```

*S1F2*:
```
L,2
1. <MDLN>
2. <SOFTREV>
```

**變數**

| 變數 | 說明 |
|------|------|
| MDLN | |
| SOFTREV | |

---

### S1F3 / S1F4 — Selected Equipment Status Request (SSR) / Selected Equipment Status Data (SSD)

| 屬性 | S1F3 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S1F4 |
| **方向** | M,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S1F4_SelectedStatusReply()` |

**說明**

A request to the equipment to report selected values of its status.

**資料結構**

*S1F3*:
```
The following structure is approved for all item formats and should be used by all new
implementations:
L,n
1. <SVID1>
.
.
n. <SVIDn>
The following structure is included for compatibility with previous implementations
and may only be used for items of format 3() and 5():
<SVID1,...,SVIDn>
```

*S1F4*:
```
L,n
1. <SV1>
.
.
n. <SVn>
```

**變數**

| 變數 | 說明 |
|------|------|
| SVID | |
| SV | |

**例外**

S1F3: A zero-length list (structure 1) or item (structure 2) means report all SVIDs.  
S1F4: None

---

### S1F11 / S1F12 — Status Variable Namelist Request (SVNR) / Status Variable Namelist Reply (SVNRR)

| 屬性 | S1F11 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S1F12 |
| **方向** | M,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S1F12_StatusVariableNamelistReply()` |

**說明**

A request to the equipment to identify certain status variables.

**資料結構**

*S1F11*:
```
L,n
1. <SVID1>
.
.
n. <SVIDn>
```

*S1F12*:
```
L,n
1. L,3
1. <SVID1>
2. <SVNAME1>
3. <UNITS1>
2. L,3
.
.
n. L,3
1. <SVIDn>
2. <SVNAMEn>
3. <UNITSn>
```

**變數**

| 變數 | 說明 |
|------|------|
| SVID | |
| SVCNAME | |
| UNITS | |

**例外**

S1F11: A zero length means report all SVIDs.  
S1F12: None

---

### S1F13 / S1F14 — Establish Communications Request (CR) / Establish Communications Request Acknowledge (CRA)

| 屬性 | S1F13 |
|------|---|
| **方向** | S,H↔E,reply |
| **HT9045** | ✅ |
| **處理函式** | `S1F13_EstablishCommunicationsRequest()` |
| | |
| 屬性 | S1F14 |
| **方向** | M,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S1F14_ConnectRequestAcknowledge()` |

**說明**

The purpose of this message is to provide a formal means of initializing communications at a logical application level both on  
power-up and following a break in communications. It should be the following any period where host and Equipment SECS  
applications are unable to communicate. An attempt to send an Establish Communications Request (S1,F13) should be repeated  
at programmable intervals until an Establish Communications Acknowledge(S1,F14) is received within the transaction timeout  
period with an acknowledgement code accepting the establishment

**資料結構**

*S1F13*:
```
L,2
1. <MDLN>
2. <SOFTREV>
```

*S1F14*:
```
L,2
1. <COMMACK>
2. L,2
1. <MDLN>
2. <SOFTREV>
```

**變數**

| 變數 | 說明 |
|------|------|
| MDLN | |
| SOFTREV | |
| COMMACK | |

**例外**

S1F13: The host sends a zero-length list to the equipment.  
S1F14: The host sends a zero-length list for item 2 to the equipment.

---

### S1F15 / S1F16 — Request OFF-LINE (ROFL) / OFF-LINE Acknowledge (OFLA)

| 屬性 | S1F15 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S1F16 |
| **方向** | S,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S1F16_OFFLINEAcknowledge()` |

**說明**

The host requests that the equipment transition to the OFF-LINE state.

**資料結構**

*S1F15*:
```
Header only
```

*S1F16*:
```
<OFLACK>
```

**變數**

| 變數 | 說明 |
|------|------|
| OFLACK | |

---

### S1F17 / S1F18 — Request ON-LINE (RONL) / ON-LINE Acknowledge (ONLA)

| 屬性 | S1F17 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S1F18 |
| **方向** | S,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S1F18_ONLINEAcknowledge()` |

**說明**

The host requests that the equipment transition to the ON-LINE state

**資料結構**

*S1F17*:
```
Header only
```

*S1F18*:
```
<ONLACK>
```

**變數**

| 變數 | 說明 |
|------|------|
| ONLACK | |

---
---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S1F1 | Are You Online? | H↔E | 1 | ✅ |  |
| S1F2 | On Line Data | H↔E | 0 | ✅ |  |
| S1F3 | Selected Equipment Status Request | H→E | 1 | ✅ |  |
| S1F4 | Selected Equipment Status Data | H←E | 0 | ✅ |  |
| S1F5 | Formatted Status Request | H→E | 1 | ❌ | [Web] |
| S1F6 | Formatted Status Data | H←E | 0 | ❌ | [Web] |
| S1F7 | Fixed Form Request | H→E | 0 | ❌ | [Web] |
| S1F8 | Fixed Form Data | H←E | 0 | ❌ | [Web] |
| S1F9 | Material Transfer Status Request | H→E | 1 | ❌ | [Web] |
| S1F10 | Material Transfer Status Data | H←E | 0 | ❌ | [Web] |
| S1F11 | Status Variable Namelist Request | H→E | 1 | ✅ |  |
| S1F12 | Status Variable Namelist Reply | H←E | 0 | ✅ |  |
| S1F13 | Establish Communications Request | H↔E | 1 | ✅ |  |
| S1F14 | Establish Communications Request Acknowledge | H↔E | 0 | ✅ |  |
| S1F15 | Request OFF-LINE | H→E | 1 | ✅ |  |
| S1F16 | OFF-LINE Acknowledge | H←E | 0 | ✅ |  |
| S1F17 | Request ON-LINE | H→E | 1 | ✅ |  |
| S1F18 | ON-LINE Acknowledge | H←E | 0 | ✅ |  |
| S1F19 | Get Attribute | H↔E | 1 | ❌ | [Web] |
| S1F20 | Attribute Data | H↔E | 0 | ❌ | [Web] |
| S1F21 | Data Variable Namelist Request | H→E | 1 | ❌ | [Web] |
| S1F22 | Data Variable Namelist Reply | H←E | 0 | ❌ | [Web] |
| S1F23 | Collection Event Namelist Request | H→E | 1 | ❌ | [Web] |
| S1F24 | Collection Event Namelist Reply | H←E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S1F1 — Are You Online?

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |
| **處理函式** | `S1F1_AreYouThereRequest()` |

**資料結構**

```
header only
```

---

#### S1F2 — On Line Data

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S1F2_OnLineData()` |

**說明**

Equipment sends the reply to indicate an online control state, otherwise an abort reply is seen.

**資料結構**

```
{L:2 MDLN
SOFTREV
*E→H:*
{L:2 MDLN
SOFTREV
*H→E:*
L:0
```

---

#### S1F3 — Selected Equipment Status Request

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**資料結構**

```
{L:n SVID
```

---

#### S1F4 — Selected Equipment Status Data

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S1F4_SelectedStatusReply()` |

**說明**

zero length value returned for unknown SVID

**資料結構**

```
{L:n SV
```

---

#### S1F5 — Formatted Status Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
SFCD
```

---

#### S1F6 — Formatted Status Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

message structure varies, superseded by dynamic reports

**資料結構**

```
{L:n SV
```

---

#### S1F7 — Fixed Form Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
SFCD
```

---

#### S1F8 — Fixed Form Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

format varies, superseded by dynamic reports

**資料結構**

```
{L:n {L:2 SVNAME
SV0
```

---

#### S1F9 — Material Transfer Status Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
header only
```

---

#### S1F10 — Material Transfer Status Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

An L:0 reply can be sent if there are no material ports

**資料結構**

```
{L:2 TSIP
TSOP
```

---

#### S1F11 — Status Variable Namelist Request

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**說明**

Host sends L:0 to request all SVIDs.

**資料結構**

```
{L:n SVID
```

---

#### S1F12 — Status Variable Namelist Reply

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S1F12_StatusVariableNamelistReply()` |

**說明**

A:0 for SVNAME and UNITS indicates unknown SVID

**資料結構**

```
{L:n {L:3 SVID
SVNAME
UNITS
```

---

#### S1F13 — Establish Communications Request

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |
| **處理函式** | `S1F13_EstablishCommunicationsRequest()` |

**資料結構**

```
{L:2 MDLN
SOFTREV
*E→H:*
{L:2 MDLN
SOFTREV
*H→E:*
L:0
```

---

#### S1F14 — Establish Communications Request Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S1F14_ConnectRequestAcknowledge()` |

**說明**

MDLN and SOFTREV may not be valid unless COMMACK value is 0

**資料結構**

```
{L:2 COMMACK
{L:2 MDLN
SOFTREV
*E→H:*
{L:2 COMMACK
{L:2 MDLN
SOFTREV
*H→E:*
{L:2 COMMACK
L:0
```

---

#### S1F15 — Request OFF-LINE

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**資料結構**

```
header only
```

---

#### S1F16 — OFF-LINE Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S1F16_OFFLINEAcknowledge()` |

**資料結構**

```
OFLACK
```

---

#### S1F17 — Request ON-LINE

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**資料結構**

```
header only
```

---

#### S1F18 — ON-LINE Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S1F18_ONLINEAcknowledge()` |

**資料結構**

```
ONLACK
```

---

#### S1F19 — Get Attribute `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

L:m = L:0 for all objects, L:n = L:0 for all attributes

**資料結構**

```
{L:3 OBJTYPE
{L:m OBJID
```

---

#### S1F20 — Attribute Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Ordered per request., m=0 means OBJTYPE unknown, n=0 means instance not found, error associations are problematic

**資料結構**

```
{L:2 {L:m {L:n ATTRDATA
```

---

#### S1F21 — Data Variable Namelist Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Host sends L:0 to request all DVVALs. VIDs are limited to DVVAL variables only. (proposed ballot item 4824B, Oct 2011)

**資料結構**

```
{L:n VID
```

---

#### S1F22 — Data Variable Namelist Reply `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

A:0 for DVVALNAME and UNITS indicates unknown VID or that VID is not a DVVAL. (proposed ballot item 4824B, Oct 2011)

**資料結構**

```
{L:n {L:3 VID
DVVALNAME
UNITS
```

---

#### S1F23 — Collection Event Namelist Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Host sends L:0 to imply all CEIDs. (proposed ballot item 4824B, Oct 2011)

**資料結構**

```
{L:n CEID
```

---

#### S1F24 — Collection Event Namelist Reply `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `S1F24_CollectionEventNamelist()` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Only associated DVVAL VIDs are listed. A:0 for CENAME and L:0 for L:a indicates non-existent CEID. (proposed ballot item 4824B, Oct 2011)

**資料結構**

```
{L:n {L:3 CEID
CENAME
{L:a VID
```

---
