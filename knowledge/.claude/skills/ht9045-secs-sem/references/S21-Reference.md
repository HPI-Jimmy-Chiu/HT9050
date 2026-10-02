# Stream 21：擴充資料收集 (Item Transfer)

> 資料來源：hume.com/secs (SEMI E5)
> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`

---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S21F1 | Item Load Inquire | H↔E | 1 | ❌ | [Web] |
| S21F2 | Item Load Grant | H↔E | 0 | ❌ | [Web] |
| S21F3 | Item Send | H↔E | 1 | ❌ | [Web] |
| S21F4 | Item Send Acknowledge | H↔E | 0 | ❌ | [Web] |
| S21F5 | Item Request | H↔E | 1 | ❌ | [Web] |
| S21F6 | Item Data | H↔E | 0 | ❌ | [Web] |
| S21F7 | Item Type List Request | H↔E | 1 | ❌ | [Web] |
| S21F8 | Item Type List Results | H↔E | 0 | ❌ | [Web] |
| S21F9 | Supported Item Type List Request | H↔E | 1 | ❌ | [Web] |
| S21F10 | Supported Item Type List Result | H↔E | 0 | ❌ | [Web] |
| S21F11 | Item Delete | H→E | 0 | ❌ | [Web] |
| S21F12 | Item Delete Acknowledge | H←E | 0 | ❌ | [Web] |
| S21F13 | Request Permission To Send Item | H↔E | 1 | ❌ | [Web] |
| S21F14 | Grant Permission To Send Item | H↔E | 0 | ❌ | [Web] |
| S21F15 | Item Request | H↔E | 1 | ❌ | [Web] |
| S21F16 | Item Request Grant | H↔E | 0 | ❌ | [Web] |
| S21F17 | Send Item Part | H↔E | 1 | ❌ | [Web] |
| S21F18 | Send Item Part Acknowledge | H↔E | 0 | ❌ | [Web] |
| S21F19 | Item Type Feature Support | H↔E | 1 | ❌ | [Web] |
| S21F20 | Item Type Feature Support Results | H↔E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S21F1 — Item Load Inquire `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 ITEMTYPE
ITEMID
ITEMLENGTH
ITEMVERSION
```

---

#### S21F2 — Item Load Grant `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 ITEMACK
ITEMERROR
```

---

#### S21F3 — Item Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Each part except the last one is max size.

**資料結構**

```
{L:5 ITEMTYPE
ITEMID
ITEMLENGTH
ITEMVERSION
{L:n ITEMPART
```

---

#### S21F4 — Item Send Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 ITEMACK
ITEMERROR
```

---

#### S21F5 — Item Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 ITEMTYPE
ITEMID
```

---

#### S21F6 — Item Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:7 ITEMACK
ITEMERROR
ITEMTYPE
ITEMID
ITEMLENGTH
ITEMVERSION
{L:n ITEMPART
```

---

#### S21F7 — Item Type List Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ITEMTYPE
```

---

#### S21F8 — Item Type List Results `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 ITEMACK
ITEMERROR
ITEMTYPE
{L:n {L:3 ITEMID
ITEMLENGTH
ITEMVERSION
```

---

#### S21F9 — Supported Item Type List Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
header only
```

---

#### S21F10 — Supported Item Type List Result `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 ITEMACK
ITEMERROR
{L:n ITEMTYPE
```

---

#### S21F11 — Item Delete `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 ITEMTYPE
{L:n ITEMID
```

---

#### S21F12 — Item Delete Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 ITEMACK
ITEMTYPE
{L:n {L:3 ITEMID
ITEMACK
ITEMERROR
```

---

#### S21F13 — Request Permission To Send Item `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:5 ITEMTYPE
ITEMID
ITEMLENGTH
ITEMVERSION
ITEMPARTCOUNT
```

---

#### S21F14 — Grant Permission To Send Item `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 ITEMACK
ITEMERROR
```

---

#### S21F15 — Item Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 ITEMTYPE
ITEMID
```

---

#### S21F16 — Item Request Grant `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:7 ITEMACK
ITEMERROR
ITEMTYPE
ITEMID
ITEMLENGTH
ITEMVERSION
ITEMPARTCOUNT
```

---

#### S21F17 — Send Item Part `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Each part message is not sent until the reply for the previous part is received.

**資料結構**

```
{L:8 ITEMTYPE
ITEMID
ITEMLENGTH
ITEMVERSION
ITEMINDEX
ITEMPARTCOUNT
ITEMPARTLENGTH
ITEMPART
```

---

#### S21F18 — Send Item Part Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Non-zero ITEMACK causes retry (32) or abort

**資料結構**

```
{L:2 ITEMACK
ITEMERROR
```

---

#### S21F19 — Item Type Feature Support `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

a zero-length list requests for all supported item types, new 6/2022

**資料結構**

```
{L:n ITEMTYPE
```

---

#### S21F20 — Item Type Feature Support Results `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

On success, ITEMACK is zero and ITEMERROR shall be zero-length

**資料結構**

```
{L:n {L:4 ITEMACK
ITEMERROR
ITEMTYPE
ITEMTYPESUPPORT
```

---
