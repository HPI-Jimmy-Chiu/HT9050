# Stream 13：資料庫訊息 (Data Set Transfer)

> 資料來源：hume.com/secs (SEMI E5)
> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`

---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S13F1 | Send Data Set Send | H↔E | 1 | ❌ | [Web] |
| S13F2 | Send Data Set Ack | H↔E | 0 | ❌ | [Web] |
| S13F3 | Open Data Set Request | H↔E | 1 | ❌ | [Web] |
| S13F4 | Open Data Set Data | H↔E | 0 | ❌ | [Web] |
| S13F5 | Read Data Set Request | H↔E | 1 | ❌ | [Web] |
| S13F6 | Read Data Set Data | H↔E | 0 | ❌ | [Web] |
| S13F7 | Close Data Set Send | H↔E | 1 | ❌ | [Web] |
| S13F8 | Close Data Set Ack | H↔E | 0 | ❌ | [Web] |
| S13F9 | Reset Data Set Send | H↔E | 1 | ❌ | [Web] |
| S13F10 | Reset Data Set Ack | H↔E | 0 | ❌ | [Web] |
| S13F11 | Data Set Obj Multi-Block Inquire | H↔E | 1 | ❌ | [Web] |
| S13F12 | Data Set Obj Multi-Block Grant | H↔E | 0 | ❌ | [Web] |
| S13F13 | Table Data Send | H↔E | 1 | ❌ | [Web] |
| S13F14 | Table Data Ack | H↔E | 0 | ❌ | [Web] |
| S13F15 | Table Data Request | H↔E | 1 | ❌ | [Web] |
| S13F16 | Table Data | H↔E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S13F1 — Send Data Set Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

S13F1 seems to have the L: wrapper that S13F2 is missing. Be prepared to receive DSNAME without the L:

**資料結構**

```
{L:1 DSNAME
```

---

#### S13F2 — Send Data Set Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

The standards have had an erroneous structure for years - the L:2 has been missing. Unfortunately some implementations have not realized it was an error. The latest Hume versions automagically create the L:2 wrapper when it is missing.

**資料結構**

```
{L:2 DSNAME
ACKC13
```

---

#### S13F3 — Open Data Set Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Sent by the receiver to open a data set for reading

**資料結構**

```
{L:3 HANDLE
DSNAME
CKPNT
```

---

#### S13F4 — Open Data Set Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:5 HANDLE
DSNAME
ACKC13
RTYPE
RECLEN
```

---

#### S13F5 — Read Data Set Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 HANDLE
READLN
```

---

#### S13F6 — Read Data Set Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 HANDLE
ACKC13
CKPNT
{L:n FILDAT
```

---

#### S13F7 — Close Data Set Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:1 HANDLE
```

---

#### S13F8 — Close Data Set Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 HANDLE
ACKC13
```

---

#### S13F9 — Reset Data Set Send `[來源: hume.com]`

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

#### S13F10 — Reset Data Set Ack `[來源: hume.com]`

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

#### S13F11 — Data Set Obj Multi-Block Inquire `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 DATAID
OBJSPEC
DATALENGTH
```

---

#### S13F12 — Data Set Obj Multi-Block Grant `[來源: hume.com]`

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

#### S13F13 — Table Data Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

The first element of every row is a primary key value which identifies the row. The row items correspond in sequence to the column headers. E58 uses attributes NumCols, NumRows, and DataLength

**資料結構**

```
{L:8 DATAID
OBJSPEC
TBLTYP
TBLID
TBLCMD
{L:n {L:2 ATTRID
ATTRDATA
```

---

#### S13F14 — Table Data Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 TBLACK
{L:p {L:2 ERRCODE
ERRTEXT
```

---

#### S13F15 — Table Data Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Either p or q or both are 0.

**資料結構**

```
{L:7 DATAID
OBJSPEC
TBLTYP
TBLID
TBLCMD
{L:p COLHDR
```

---

#### S13F16 — Table Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:6 TBLTYP
TBLID
{L:n {L:2 ATTRID
ATTRDATA
```

---
