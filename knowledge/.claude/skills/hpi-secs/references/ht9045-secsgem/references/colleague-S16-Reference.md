# Stream 16：啟動/停止控制 (Control Function)

> 資料來源：hume.com/secs (SEMI E5)
> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`

---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S16F1 | Process Job Data MBI | H→E | 1 | ❌ | [Web] |
| S16F2 | PJD MBI Grant | H←E | 0 | ❌ | [Web] |
| S16F3 | Process Job Create Req | H→E | 1 | ❌ | [Web] |
| S16F4 | Process Job Create Ack | H←E | 0 | ❌ | [Web] |
| S16F5 | Process Job Cmd Req | H→E | 1 | ❌ | [Web] |
| S16F6 | Process Job Cmd Ack | H←E | 0 | ❌ | [Web] |
| S16F7 | Process Job Alert Notify | H←E | 0 | ❌ | [Web] |
| S16F8 | Process Job Alert Ack | H→E | 0 | ❌ | [Web] |
| S16F9 | Process Job Event Notify | H←E | 0 | ❌ | [Web] |
| S16F10 | Process Job Event Ack | H→E | 0 | ❌ | [Web] |
| S16F11 | PRJobCreateEnh | H→E | 1 | ❌ | [Web] |
| S16F12 | PRJobCreateEnh Ack | H←E | 0 | ❌ | [Web] |
| S16F15 | PRJobMultiCreate | H→E | 1 | ❌ | [Web] |
| S16F16 | PRJobMultiCreate Ack | H←E | 0 | ❌ | [Web] |
| S16F17 | PRJobDequeue | H→E | 1 | ❌ | [Web] |
| S16F18 | PRJobDequeue Ack | H←E | 0 | ❌ | [Web] |
| S16F19 | PRJob List Req | H→E | 1 | ❌ | [Web] |
| S16F20 | PRJob List Data | H←E | 0 | ❌ | [Web] |
| S16F21 | PRJob Create Limit Req | H→E | 1 | ❌ | [Web] |
| S16F22 | PRJob Create Limit Data | H←E | 0 | ❌ | [Web] |
| S16F23 | PRJob Recipe Variable Set | H→E | 1 | ❌ | [Web] |
| S16F24 | PRJob Recipe Variable Ack | H→E | 0 | ❌ | [Web] |
| S16F25 | PRJob Start Method Set | H→E | 1 | ❌ | [Web] |
| S16F26 | PRJob Start Method Ack | H←E | 0 | ❌ | [Web] |
| S16F27 | Control Job Command | H→E | 1 | ❌ | [Web] |
| S16F28 | Control Job Command Ack | H←E | 0 | ❌ | [Web] |
| S16F29 | PRSetMtrlOrder | H→E | 0 | ❌ | [Web] |
| S16F30 | PRSetMtrlOrder Ack | H←E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S16F1 — Process Job Data MBI `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

SECS-I multiblock inquire/grant for all process management messages, optional for HSMS

**資料結構**

```
{L:2 DATAID
DATALENGTH
```

---

#### S16F2 — PJD MBI Grant `[來源: hume.com]`

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

#### S16F3 — Process Job Create Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:5 DATAID
MF
{L:n MID
```

---

#### S16F4 — Process Job Create Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 PRJOBID
{L:2 ACKA
{L:n {L:2 ERRCODE
ERRTEXT
```

---

#### S16F5 — Process Job Cmd Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 DATAID
PRJOBID
PRCMDNAME
{L:n {L:2 CPNAME
CPVAL
```

---

#### S16F6 — Process Job Cmd Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 PRJOBID
{L:2 ACKA
{L:n {L:2 ERRCODE
ERRTEXT
```

---

#### S16F7 — Process Job Alert Notify `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Unlike S5F1 there is no message to enable/disable selected alerts. ACKA false indicates failure.

**資料結構**

```
{L:4 TIMESTAMP
PRJOBID
PRJOBMILESTONE
{L:2 ACKA
{L:n {L:2 ERRCODE
ERRTEXT
```

---

#### S16F8 — Process Job Alert Ack `[來源: hume.com]`

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

#### S16F9 — Process Job Event Notify `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

There is no message to define the VID list or enable/disable. Less featured than Stream 6 and superseded by Stream 17.

**資料結構**

```
{L:4 PREVENTID
TIMESTAMP
PRJOBID
{L:n {L:2 VID
V
```

---

#### S16F10 — Process Job Event Ack `[來源: hume.com]`

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

#### S16F11 — PRJobCreateEnh `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

The L:n list can be {L:n MID} depending on MF, {L:j SLOTID} might be an array instead

**資料結構**

```
{L:7 DATAID
PRJOBID
MF
{L:n {L:2 CARRIERID
{L:j SLOTID
```

---

#### S16F12 — PRJobCreateEnh Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 PRJOBID
{L:2 ACKA
{L:n {L:2 ERRCODE
ERRTEXT
```

---

#### S16F15 — PRJobMultiCreate `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

The L:n list can be {L:n MID} depending on MF, {L:j SLOTID} might be an array instead

**資料結構**

```
{L:2 DATAID
{L:p {L:6 PRJOBID
MF
{L:n {L:2 CARRIERID
{L:j SLOTID
```

---

#### S16F16 — PRJobMultiCreate Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 {L:m PRJOBID
```

---

#### S16F17 — PRJobDequeue `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

If m = 0 then the request is for all jobs that have not begun processing.

**資料結構**

```
{L:m PRJOBID
```

---

#### S16F18 — PRJobDequeue Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 {L:m PRJOBID
```

---

#### S16F19 — PRJob List Req `[來源: hume.com]`

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

#### S16F20 — PRJob List Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:m {L:2 PRJOBID
PRSTATE
```

---

#### S16F21 — PRJob Create Limit Req `[來源: hume.com]`

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

#### S16F22 — PRJob Create Limit Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
PRJOBSPACE
```

---

#### S16F23 — PRJob Recipe Variable Set `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 PRJOBID
{L:m {L:2 RCPPARNM
RCPPARVAL
```

---

#### S16F24 — PRJob Recipe Variable Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 ACKA
{L:n {L:2 ERRCODE
ERRTEXT
```

---

#### S16F25 — PRJob Start Method Set `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 {L:m PRJOBID
```

---

#### S16F26 — PRJob Start Method Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 {L:m PRJOBID
```

---

#### S16F27 — Control Job Command `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Defined by E94, used in replacement of S16F5 when Control Jobs are used

**資料結構**

```
{L:3 CTLJOBID
CTLJOBCMD
{L:2 CPNAME
CPVAL
```

---

#### S16F28 — Control Job Command Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 ACKA
{L:2 ERRCODE
ERRTEXT
```

---

#### S16F29 — PRSetMtrlOrder `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
PRMTRLORDER
```

---

#### S16F30 — PRSetMtrlOrder Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKA
```

---
