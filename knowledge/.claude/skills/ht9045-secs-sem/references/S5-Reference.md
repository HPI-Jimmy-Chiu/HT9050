# Stream 5：警報管理 (Alarm Management)

> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`
> 資料來源：Excel `SECS_20260401_Steven.xlsx` + 原始碼分析
> 實作檔案：`uHGemClass.cpp`、`uHGemEquipment.cpp`、`uHGemHT9045.cpp`

## 概述

Stream 5 處理警報通知、警報啟用/停用與警報清單查詢。設備端發送 S5F1 通知 Host 警報事件。

## 訊息一覽

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 處理函式 |
|------|------|:----:|:-----:|:------:|----------|
| S5F1 | Alarm Report Send (ARS) | S,H←E,[reply] | 1 | ✅ | — |
| S5F2 | Alarm Report Acknowledge (ARA) | S,H→E | 0 | ✅ | `S5F2_AlarmReportAcknowledge()` |
| S5F3 | Enable/Disable Alarm Send (EAS) | S,H→E,[reply] | 1 | ✅ | — |
| S5F4 | Enable/Disable Alarm Acknowledge (EAA) | S,H←E | 0 | ✅ | `S5F4_EnableDisableAlarmAcknowledge()` |
| S5F5 | List Alarms Request (LAR) | S,H→E,reply | 1 | ✅ | — |
| S5F6 | List Alarm Data (LAD) | M,H←E | 0 | ✅ | `S5F6_ListAlarmReply()` |
| S5F7 | List Enabled Alarm Request (LEAR) | S,H→E,reply | 1 | ✅ | — |
| S5F8 | List Enabled Alarm Data (LEAD) | M,H←E | 0 | ✅ | `S5F8_EnableDisableAlarmSendReply()` |

## 詳細說明

### S5F1 / S5F2 — Alarm Report Send (ARS) / Alarm Report Acknowledge (ARA)

| 屬性 | S5F1 |
|------|---|
| **方向** | S,H←E,[reply] |
| **HT9045** | ✅ |
| | |
| 屬性 | S5F2 |
| **方向** | S,H→E |
| **HT9045** | ✅ |
| **處理函式** | `S5F2_AlarmReportAcknowledge()` |

**說明**

This message reports a change in or presence of an alarm condition. One message will be issued when the alarm is set and one  
message will be issued when the alarm is cleared. Irrecoverable errors and attention flags may not have a corresponding clear  
message.

**資料結構**

*S5F1*:
```
L,3
    1. <ALCD>
    2. <ALID>
    3. <ALTX>
```

*S5F2*:
```
<ACKC5>
```

**變數**

| 變數 | 說明 |
|------|------|
| ALCD | |
| ALID | |
| ALTX | |
| ACKC | |

---

### S5F3 / S5F4 — Enable/Disable Alarm Send (EAS) / Enable/Disable Alarm Acknowledge (EAA)

| 屬性 | S5F3 |
|------|---|
| **方向** | S,H→E,[reply] |
| **HT9045** | ✅ |
| | |
| 屬性 | S5F4 |
| **方向** | S,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S5F4_EnableDisableAlarmAcknowledge()` |

**說明**

This message will change the state of the enable bit in the equipment. The enable bit determines if the alarm will be sent to the  
host. Alarms which are not controllable in this way are unaffected by this message.

**資料結構**

*S5F3*:
```
L,2
   1. <ALED>
   2. <ALID>
```

*S5F4*:
```
<ACKC5>
```

**變數**

| 變數 | 說明 |
|------|------|
| ALED | |
| ALID | |
| ACKC5 | |

**例外**

S5F3: A zero-length item for ALID means all alarms.  
S5F4: None

---

### S5F5 / S5F6 — List Alarms Request (LAR) / List Alarm Data (LAD)

| 屬性 | S5F5 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S5F6 |
| **方向** | M,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S5F6_ListAlarmReply()` |

**說明**

This message requests the equipment to send binary and analog alarm information to the host.

**資料結構**

*S5F5*:
```
<ALID1, . . . ,ALIDn>
```

*S5F6*:
```
L,m
  1. L,3
    1. <ALCD1>
    2. <ALID1>
    3. <ALTX1>
  2. L,3
  .
  .
  m. L,3
    1. <ALCDm>
    2. <ALIDm>
    3. <ALTXm>
```

**變數**

| 變數 | 說明 |
|------|------|
| ALID | |
| ALCD | |
| ALTX | |

**例外**

S5F5: A zero-length item means send all possible alarms regardless of the state of ALED.  
S5F6: If m = 0, no response can be made. A zero-length item returned for ALCDi or ALTXi means that value does not exist.

---

### S5F7 / S5F8 — List Enabled Alarm Request (LEAR) / List Enabled Alarm Data (LEAD)

| 屬性 | S5F7 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S5F8 |
| **方向** | M,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S5F8_EnableDisableAlarmSendReply()` |

**說明**

List alarms which are enabled.

**資料結構**

*S5F7*:
```
Header only
```

*S5F8*:
```
Same as S5,F6
```

---
---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S5F1 | Alarm Report Send | H←E | 0 | ✅ |  |
| S5F2 | Alarm Report Ack | H→E | 0 | ✅ |  |
| S5F3 | Enable/Disable Alarm Send | H→E | 0 | ✅ |  |
| S5F4 | Enable/Disable Alarm Ack | H←E | 0 | ✅ |  |
| S5F5 | List Alarms Request | H→E | 1 | ✅ |  |
| S5F6 | List Alarm Data | H←E | 0 | ✅ |  |
| S5F7 | List Enabled Alarm Request | H→E | 1 | ✅ |  |
| S5F8 | List Enabled Alarm Data | H←E | 0 | ✅ |  |
| S5F9 | Exception Post Notify | H←E | 0 | ❌ | [Web] |
| S5F10 | Exception Post Confirm | H→E | 0 | ❌ | [Web] |
| S5F11 | Exception Clear Notify | H←E | 0 | ❌ | [Web] |
| S5F12 | Exception Clear Confirm | H→E | 0 | ❌ | [Web] |
| S5F13 | Exception Recover Request | H→E | 1 | ❌ | [Web] |
| S5F14 | Exception Recover Acknowledge | H←E | 0 | ❌ | [Web] |
| S5F15 | Exception Recovery Complete Notify | H←E | 0 | ❌ | [Web] |
| S5F16 | Exception Recovery Complete Confirm | H→E | 0 | ❌ | [Web] |
| S5F17 | Exception Recovery Abort Request | H→E | 1 | ❌ | [Web] |
| S5F18 | Exception Recovery Abort Ack | H←E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S5F1 — Alarm Report Send

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |

**資料結構**

```
{L:3 ALCD
ALID
ALTX
```

---

#### S5F2 — Alarm Report Ack

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S5F2_AlarmReportAcknowledge()` |

**資料結構**

```
ACKC5
```

---

#### S5F3 — Enable/Disable Alarm Send

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |

**資料結構**

```
{L:2 ALED
ALID
```

---

#### S5F4 — Enable/Disable Alarm Ack

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S5F4_EnableDisableAlarmAcknowledge()` |

**資料結構**

```
ACKC5
```

---

#### S5F5 — List Alarms Request

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**說明**

Host sends zero length item for all, otherwise ALID vector

**資料結構**

```
ALIDVECTOR
```

---

#### S5F6 — List Alarm Data

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S5F6_ListAlarmReply()` |

**資料結構**

```
{L:n {L:3 ALCD
ALID
ALTX
```

---

#### S5F7 — List Enabled Alarm Request

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

#### S5F8 — List Enabled Alarm Data

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S5F8_EnableDisableAlarmSendReply()` |

**資料結構**

```
{L:n {L:3 ALCD
ALID
ALTX
```

---

#### S5F9 — Exception Post Notify `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

see E41

**資料結構**

```
{L:5 TIMESTAMP
EXID
EXTYPE
EXMESSAGE
{L:n EXRECVRA
```

---

#### S5F10 — Exception Post Confirm `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
header only
```

---

#### S5F11 — Exception Clear Notify `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

see E41

**資料結構**

```
{L:4 TIMESTAMP
EXID
EXTYPE
EXMESSAGE
```

---

#### S5F12 — Exception Clear Confirm `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
header only
```

---

#### S5F13 — Exception Recover Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

see E41

**資料結構**

```
{L:2 EXID
EXRECVRA
```

---

#### S5F14 — Exception Recover Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

L:2* can be L:2 or L:0

**資料結構**

```
{L:2 EXID
{L:2 ACKA
{L:2* ERRCODE
ERRTEXT
```

---

#### S5F15 — Exception Recovery Complete Notify `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

L:2* can be L:2 or L:0; see E41

**資料結構**

```
{L:3 TIMESTAMP
EXID
{L:2 ACKA
{L:2* ERRCODE
ERRTEXT
```

---

#### S5F16 — Exception Recovery Complete Confirm `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
header only
```

---

#### S5F17 — Exception Recovery Abort Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

see E41

**資料結構**

```
EXID
```

---

#### S5F18 — Exception Recovery Abort Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

L:2* can be L:2 or L:0

**資料結構**

```
{L:2 EXID
{L:2 ACKA
{L:2* ERRCODE
ERRTEXT
```

---
