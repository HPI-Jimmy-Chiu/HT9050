# Stream 17：設備自我描述 (Equipment Self-Description)

> 資料來源：hume.com/secs (SEMI E5)
> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`

---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S17F1 | Data Report Create Req | H→E | 1 | ❌ | [Web] |
| S17F2 | Data Report Create Ack | H←E | 0 | ❌ | [Web] |
| S17F3 | Data Report Delete Req | H→E | 1 | ❌ | [Web] |
| S17F4 | Data Report Del Ack | H←E | 0 | ❌ | [Web] |
| S17F5 | Trace Create Req | H→E | 1 | ❌ | [Web] |
| S17F6 | Trace Create Ack | H←E | 0 | ❌ | [Web] |
| S17F7 | Trace Delete Req | H→E | 1 | ❌ | [Web] |
| S17F8 | Trace Delete Ack | H←E | 0 | ❌ | [Web] |
| S17F9 | Collection Event Link Req | H→E | 1 | ❌ | [Web] |
| S17F10 | Collection Event Link Ack | H←E | 0 | ❌ | [Web] |
| S17F11 | Collection Event Unlink | H→E | 1 | ❌ | [Web] |
| S17F12 | Collection Event Unlink Ack | H←E | 0 | ❌ | [Web] |
| S17F13 | Trace Reset Req | H→E | 1 | ❌ | [Web] |
| S17F14 | Trace Reset Ack | H←E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S17F1 — Data Report Create Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 DATAID
RPTID
DATASRC
{L:n VID
```

---

#### S17F2 — Data Report Create Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RPTID
ERRCODE
```

---

#### S17F3 — Data Report Delete Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

L:0 means delete all reports

**資料結構**

```
{L:n RPTID
```

---

#### S17F4 — Data Report Del Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 ACKA
{L:m {L:3 RPTID
ERRCODE
ERRTEXT
```

---

#### S17F5 — Trace Create Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

we recommend the host always provides the L:8 values

**資料結構**

```
{L:6 DATAID
TRID
CEED
{L:n RPTID
```

---

#### S17F6 — Trace Create Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 TRID
ERRCODE
```

---

#### S17F7 — Trace Delete Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Surprisingly, L:0 is not specified as a means to indicate all, but this feature has to be provided because there is no means to discover the existing traces.

**資料結構**

```
{L:n TRID
```

---

#### S17F8 — Trace Delete Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 ACKA
{L:m {L:3 TRID
ERRCODE
ERRTEXT
```

---

#### S17F9 — Collection Event Link Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 DATAID
EVNTSRC
CEID
{L:n RPTID
```

---

#### S17F10 — Collection Event Link Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 EVNTSRC
CEID
ERRCODE
```

---

#### S17F11 — Collection Event Unlink `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 EVNTSRC
CEID
RPTID
```

---

#### S17F12 — Collection Event Unlink Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 EVNTSRC
CEID
RPTID
ERRCODE
```

---

#### S17F13 — Trace Reset Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:n TRID
```

---

#### S17F14 — Trace Reset Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 ACKA
{L:m {L:3 TRID
ERRCODE
ERRTEXT
```

---
