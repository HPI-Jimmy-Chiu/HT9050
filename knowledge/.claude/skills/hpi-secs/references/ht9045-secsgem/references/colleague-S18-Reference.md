# Stream 18：子設備服務 (Sub-Equipment Services)

> 資料來源：hume.com/secs (SEMI E5)
> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`

---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S18F1 | Read Attribute Req | H→E | 1 | ❌ | [Web] |
| S18F2 | Read Attribute Data | H←E | 0 | ❌ | [Web] |
| S18F3 | Write Attribute Req | H→E | 1 | ❌ | [Web] |
| S18F4 | Write Attribute Ack | H←E | 0 | ❌ | [Web] |
| S18F5 | Read Request | H→E | 1 | ❌ | [Web] |
| S18F6 | Read Data | H←E | 0 | ❌ | [Web] |
| S18F7 | Write Data Request | H→E | 1 | ❌ | [Web] |
| S18F8 | Write Data Ack | H←E | 0 | ❌ | [Web] |
| S18F9 | Read ID Req | H→E | 1 | ❌ | [Web] |
| S18F10 | Read ID Data | H←E | 0 | ❌ | [Web] |
| S18F11 | Write ID Req | H→E | 1 | ❌ | [Web] |
| S18F12 | Write ID Ack | H←E | 0 | ❌ | [Web] |
| S18F13 | Subsystem Command | H→E | 1 | ❌ | [Web] |
| S18F14 | Subsystem Command Ack | H←E | 0 | ❌ | [Web] |
| S18F15 | Read 2D Code Cond Req | H→E | 1 | ❌ | [Web] |
| S18F16 | Read 2D Code Cond Data | H←E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S18F1 — Read Attribute Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 TARGETID
{L:n ATTRID
```

---

#### S18F2 — Read Attribute Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

E5 differs from OEM tools

**資料結構**

```
{L:4 TARGETID
SSACK
{L:n ATTRDATA
```

---

#### S18F3 — Write Attribute Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 TARGETID
{L:n {L:2 ATTRID
ATTRDATA
```

---

#### S18F4 — Write Attribute Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

fixed E5 mistake

**資料結構**

```
{L:3 TARGETID
SSACK
{L:s STATUS
```

---

#### S18F5 — Read Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 TARGETID
DATASEG
DATALENGTH
```

---

#### S18F6 — Read Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 TARGETID
SSACK
DATA
{L:s STATUS
```

---

#### S18F7 — Write Data Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 TARGETID
DATASEG
DATALENGTH
DATA
```

---

#### S18F8 — Write Data Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 TARGETID
SSACK
{L:s STATUS
```

---

#### S18F9 — Read ID Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
TARGETID
```

---

#### S18F10 — Read ID Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 TARGETID
SSACK
MID
{L:s STATUS
```

---

#### S18F11 — Write ID Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 TARGETID
MID
```

---

#### S18F12 — Write ID Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 TARGETID
SSACK
{L:s STATUS
```

---

#### S18F13 — Subsystem Command `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 TARGETID
SSCMD
{L:n CPVAL
```

---

#### S18F14 — Subsystem Command Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 TARGETID
SSACK
{L:s STATUS
```

---

#### S18F15 — Read 2D Code Cond Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
TARGETID
```

---

#### S18F16 — Read 2D Code Cond Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:5 TARGETID
SSACK
MID
{L:s STATUS
```

---
