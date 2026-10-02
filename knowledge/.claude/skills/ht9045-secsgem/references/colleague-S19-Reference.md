# Stream 19：設備資訊 (Equipment Info)

> 資料來源：hume.com/secs (SEMI E5)
> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`

---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S19F1 | Request Process Definition Element (PDE) Directory | H↔E | 1 | ❌ | [Web] |
| S19F2 | PDE Directory Data | H↔E | 0 | ❌ | [Web] |
| S19F3 | PDE Delete Request | H→E | 1 | ❌ | [Web] |
| S19F4 | PDE Delete Acknowledge | H←E | 0 | ❌ | [Web] |
| S19F5 | PDE Header Data Request | H↔E | 1 | ❌ | [Web] |
| S19F6 | PDE Header Data Reply | H↔E | 0 | ❌ | [Web] |
| S19F7 | request the transfer of PDEs via Stream 13 | H↔E | 1 | ❌ | [Web] |
| S19F8 | PDE Transfer Reply | H↔E | 0 | ❌ | [Web] |
| S19F9 | Request to Send PDE | H↔E | 1 | ❌ | [Web] |
| S19F10 | Initiate PDE transfer Reply | H↔E | 0 | ❌ | [Web] |
| S19F11 | Send PDE | H↔E | 1 | ❌ | [Web] |
| S19F12 | Send PDE Acknowledge | H↔E | 0 | ❌ | [Web] |
| S19F13 | TransferContainer Report | H↔E | 1 | ❌ | [Web] |
| S19F14 | TransferContainer Report Ack | H↔E | 0 | ❌ | [Web] |
| S19F15 | Request PDE Resolution | H→E | 1 | ❌ | [Web] |
| S19F16 | PDE Resolution Data | H←E | 0 | ❌ | [Web] |
| S19F17 | Verify PDE Request | H→E | 1 | ❌ | [Web] |
| S19F18 | PDE Verification Result | H←E | 0 | ❌ | [Web] |
| S19F19 | S19 Multi-block Inquire | H↔E | 1 | ❌ | [Web] |
| S19F20 | S19 Multi-block Grant | H↔E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S19F1 — Request Process Definition Element (PDE) Directory `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

SEE SEMI E139. If m=0, all PDE's are returned. If n=0, no additional attributes are returned. Selection conditions are ANDed together.

**資料結構**

```
{L:2 {L:m {L:3 PDEATTRIBUTENAME
COMPARISONOPERATOR
PDEATTRIBUTEVALUE
```

---

#### S19F2 — PDE Directory Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

the list of PDEs, and their attributes matching the request

**資料結構**

```
{L:3 DIRRSPSTAT
STATUSTXT
{L:m {L:2 UID
{L:n {L:2 PDEATTRIBUTE
PDEATTRIBUTEVALUE
```

---

#### S19F3 — PDE Delete Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

L:0 is not allowed. Surprisingly the command is only defined for the host despite S19F1R being for both.

**資料結構**

```
{L:n UID
```

---

#### S19F4 — PDE Delete Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Surprisingly L:0 is specified as the reply for L:0 input instead of S9F7.

**資料結構**

```
{L:n {L:3 UID
DELRSPSTAT
STATUSTXT
```

---

#### S19F5 — PDE Header Data Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

n = 0 is not allowed

**資料結構**

```
{L:n UID
```

---

#### S19F6 — PDE Header Data Reply `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

A zero length TCID is sent if there are no code 0 PDEs. If L:0 S19F5R input then n=0 reply instead of S9F7!

**資料結構**

```
{L:2 TCID
{L:n {L:3 UID
GETRSPSTAT
STATUSTXT
```

---

#### S19F7 — request the transfer of PDEs via Stream 13 `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

n = 0 is not allowed

**資料結構**

```
{L:n UID
```

---

#### S19F8 — PDE Transfer Reply `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Each PDE data set with the GETRSPSTAT response code of 0 will be sent in a Stream 13 TransferContainer. A zero length TCID is sent if there are no code 0 PDEs. If L:0 S19F7R input then n=0 reply instead of S9F7!

**資料結構**

```
{L:2 TCID
{L:n {L:3 UID
GETRSPSTAT
STATUSTXT
```

---

#### S19F9 — Request to Send PDE `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Request permission to initiate PDE transfer using S19F11R.

**資料結構**

```
{L:2 TCID
TRANSFERSIZE
```

---

#### S19F10 — Initiate PDE transfer Reply `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 TCID
RTSRSPSTAT
STATUSTXT
```

---

#### S19F11 — Send PDE `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

tells the receiver to initiate a Stream 13 transfer with the DSNAME = TCID

**資料結構**

```
TCID
```

---

#### S19F12 — Send PDE Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Header only. The transfer result status is sent in S19F13.

**資料結構**

```
header only
```

---

#### S19F13 — TransferContainer Report `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Acknowledges the receipt of a TransferContainer using S13. Verification of transferred PDEs is rrequired when received by equipment.

**資料結構**

```
{L:n {L:4 UID
SENDRSPSTAT
VERIFYRSPSTAT
STATUSTXT
```

---

#### S19F14 — TransferContainer Report Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

header only acknowledges the receipt S19F13R

**資料結構**

```
header only
```

---

#### S19F15 — Request PDE Resolution `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Request the equipment to resolve PDEs in the target. n can be 0 for no InputMap

**資料結構**

```
{L:2 TARGETPDE
{L:n {L:2 PDEREF
RESOLUTION
```

---

#### S19F16 — PDE Resolution Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

The output map of the recipe structure. L:m has resolved PDEREF. n can be 0, n >= m

**資料結構**

```
{L:2 {L:m {L:2 PDEREF
RESOLUTION
```

---

#### S19F17 — Verify PDE Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

n can be 0 when there is no InputMap

**資料結構**

```
{L:4 TARGETPDE
{L:n {L:2 PDEREF
RESOLUTION
```

---

#### S19F18 — PDE Verification Result `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 VERIFYSUCCESS
{L:n {L:3 UID
VERIFYRSPSTAT
STATUSTXT
```

---

#### S19F19 — S19 Multi-block Inquire `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

SECS-I request permission to send multi-block S19F1,3,5,6,13,15,17. Not required for HSMS.

**資料結構**

```
DATALENGTH
```

---

#### S19F20 — S19 Multi-block Grant `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Usage is not required by the standard. Should not have been included in the standard.

**資料結構**

```
GRANT
```

---
