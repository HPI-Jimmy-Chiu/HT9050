# Stream 3/4：物料傳輸 (Material Transfer)

> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`

## Stream 3 — 物料狀態

| SxFy | 名稱 | 方向 | 處理函式 |
|------|------|:----:|----------|
| S3F1 | Material Status Request | H→E | — |
| S3F2 | Material Status Data | E→H | — |
| S3F11 | Material ID Request | H→E | — |
| S3F12 | Material ID Request Ack | E→H | — |
| S3F13 | Material ID Send | H→E | — |
| S3F14 | Material ID Acknowledge | E→H | — |

## Stream 4 — 物料交接

| SxFy | 名稱 | 方向 | 處理函式 |
|------|------|:----:|----------|
| S4F1 | Ready to Send Material | E→H | — |
| S4F2 | Ready to Send Ack | H→E | — |
| S4F3 | Handshake Complete | E→H | — |
| S4F5 | Not Ready to Send | E→H | — |
| S4F17 | Request to Receive Material | H→E | — |
| S4F18 | Request to Receive Material Ack | E→H | — |

> **注意**：S3/S4 在 `SFCodeAndMean[]` 中有定義但目前程式碼無實際 dispatch 處理。
---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S4F1 | Ready to Send Materials | H↔E | 1 | ❌ | [Web] |
| S4F2 | Ready to Send Ack | H↔E | 0 | ❌ | [Web] |
| S4F3 | Send Material | H↔E | 0 | ❌ | [Web] |
| S4F5 | Handshake Complete | H↔E | 0 | ❌ | [Web] |
| S4F7 | Not Ready to Send | H↔E | 0 | ❌ | [Web] |
| S4F9 | Stuck in Sender | H↔E | 0 | ❌ | [Web] |
| S4F11 | Stuck in Receiver | H↔E | 0 | ❌ | [Web] |
| S4F13 | Send Incomplete Timeout | H↔E | 0 | ❌ | [Web] |
| S4F15 | Material Received | H↔E | 0 | ❌ | [Web] |
| S4F17 | Request to Receive | H↔E | 1 | ❌ | [Web] |
| S4F18 | Request to Receive Ack | H↔E | 0 | ❌ | [Web] |
| S4F19 | Transfer Job Create | H→E | 1 | ❌ | [Web] |
| S4F20 | Transfer Job Acknowledge | H←E | 0 | ❌ | [Web] |
| S4F21 | Transfer Job Command | H→E | 1 | ❌ | [Web] |
| S4F22 | Transfer Job Command Ack | H←E | 0 | ❌ | [Web] |
| S4F23 | Transfer Command Alert | H←E | 0 | ❌ | [Web] |
| S4F24 | Transfer Alert Ack | H→E | 0 | ❌ | [Web] |
| S4F25 | Multi-block Inquire | H→E | 1 | ❌ | [Web] |
| S4F26 | Multi-block Grant | H←E | 0 | ❌ | [Web] |
| S4F27 | Handoff Ready | H↔E | 0 | ❌ | [Web] |
| S4F29 | Handoff Command | H↔E | 0 | ❌ | [Web] |
| S4F31 | Handoff Command Complete | H↔E | 0 | ❌ | [Web] |
| S4F33 | Handoff Verified | H↔E | 0 | ❌ | [Web] |
| S4F35 | Handoff Cancel Ready | H↔E | 0 | ❌ | [Web] |
| S4F37 | Handoff Cancel Ready Ack | H↔E | 0 | ❌ | [Web] |
| S4F39 | Handoff Halt | H↔E | 0 | ❌ | [Web] |
| S4F41 | Handoff Halt Ack | H↔E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S4F1 — Ready to Send Materials `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 PTN
MID
```

---

#### S4F2 — Ready to Send Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
RSACK
```

---

#### S4F3 — Send Material `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 PTN
MID
```

---

#### S4F5 — Handshake Complete `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 PTN
MID
```

---

#### S4F7 — Not Ready to Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 PTN
MID
```

---

#### S4F9 — Stuck in Sender `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

standard does not show as bidirectional

**資料結構**

```
{L:2 PTN
MID
```

---

#### S4F11 — Stuck in Receiver `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

standard does not show as bidirectional

**資料結構**

```
{L:2 PTN
MID
```

---

#### S4F13 — Send Incomplete Timeout `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 PTN
MID
```

---

#### S4F15 — Material Received `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 PTN
MID
```

---

#### S4F17 — Request to Receive `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 PTN
MID
```

---

#### S4F18 — Request to Receive Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
RRACK
```

---

#### S4F19 — Transfer Job Create `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 DATAID
{L:2 TRJOBNAME
{L:n {L:12 TRLINK
TRPORT
TROBJNAME
TROBJTYPE
TRROLE
TRRCP
TRPTNR
TRPTPORT
TRDIR
TRTYPE
TRLOCATION
TRAUTOSTART
```

---

#### S4F20 — Transfer Job Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 TRJOBID
{L:m TRATOMCID
```

---

#### S4F21 — Transfer Job Command `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 TRJOBID
TRCMDNAME
{L:n {L:2 CPNAME
CPVAL
```

---

#### S4F22 — Transfer Job Command Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 TRACK
{L:n {L:2 ERRCODE
ERRTEXT
```

---

#### S4F23 — Transfer Command Alert `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 TRJOBID
TRJOBNAME
TRJOBMS
{L:2 TRACK
{L:n {L:2 ERRCODE
ERRTEXT
```

---

#### S4F24 — Transfer Alert Ack `[來源: hume.com]`

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

#### S4F25 — Multi-block Inquire `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

for SECS-I, not required for HSMS

**資料結構**

```
{L:2 DATAID
DATALENGTH
```

---

#### S4F26 — Multi-block Grant `[來源: hume.com]`

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

#### S4F27 — Handoff Ready `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 EQNAME
{L:11 TRLINK
TRPORT
TROBJNAME
TROBJTYPE
TRROLE
TRPTNR
TRPTPORT
TRDIR
TRTYPE
TRLOCATION
```

---

#### S4F29 — Handoff Command `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 TRLINK
MCINDEX
HOCMDNAME
{L:n {L:2 CPNAME
CPVAL
```

---

#### S4F31 — Handoff Command Complete `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 TRLINK
MCINDEX
{L:2 HOACK
{L:n {L:2 ERRCODE
ERRTEXT
```

---

#### S4F33 — Handoff Verified `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 TRLINK
{L:2 HOACK
{L:n ERRCODE
ERRTEXT
```

---

#### S4F35 — Handoff Cancel Ready `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
TRLINK
```

---

#### S4F37 — Handoff Cancel Ready Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 TRLINK
HOCANCELACK
```

---

#### S4F39 — Handoff Halt `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
TRLINK
```

---

#### S4F41 — Handoff Halt Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 TRLINK
HOHALTACK
```

---
