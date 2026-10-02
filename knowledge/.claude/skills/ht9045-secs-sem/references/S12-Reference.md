# Stream 12：晶圓圖像 (Wafer Mapping)

> 資料來源：hume.com/secs (SEMI E5)
> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`

---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S12F1 | Map Setup Data Send | H←E | 1 | ❌ | [Web] |
| S12F2 | Map Setup Data Acknowledge | H→E | 0 | ❌ | [Web] |
| S12F3 | Map Setup Data Request | H←E | 1 | ❌ | [Web] |
| S12F4 | Map Setup Data | H→E | 0 | ❌ | [Web] |
| S12F5 | Map Transmit Inquire | H←E | 1 | ❌ | [Web] |
| S12F6 | Map Transmit Grant | H→E | 0 | ❌ | [Web] |
| S12F7 | Map Data Send Type 1 | H←E | 1 | ❌ | [Web] |
| S12F8 | Map Data Ack Type 1 | H→E | 0 | ❌ | [Web] |
| S12F9 | Map Data Send Type 2 | H←E | 1 | ❌ | [Web] |
| S12F10 | Map Data Ack Type 2 | H→E | 0 | ❌ | [Web] |
| S12F11 | Map Data Send Type 3 | H←E | 1 | ❌ | [Web] |
| S12F12 | Map Data Ack Type 3 | H→E | 0 | ❌ | [Web] |
| S12F13 | Map Data Request Type 1 | H←E | 1 | ❌ | [Web] |
| S12F14 | Map Data Type 1 | H→E | 0 | ❌ | [Web] |
| S12F15 | Map Data Request Type 2 | H←E | 1 | ❌ | [Web] |
| S12F16 | Map Data Type 2 | H→E | 0 | ❌ | [Web] |
| S12F17 | Map Data Request Type 3 | H←E | 1 | ❌ | [Web] |
| S12F18 | Map Data Type 3 | H→E | 0 | ❌ | [Web] |
| S12F19 | Map Error Report Send | H↔E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S12F1 — Map Setup Data Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:15 MID
IDTYP
FNLOC
FFROT
ORLOC
RPSEL
{L:n REFP
```

---

#### S12F2 — Map Setup Data Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
SDACK
```

---

#### S12F3 — Map Setup Data Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:9 MID
IDTYP
MAPFT
FNLOC
FFROT
ORLOC
PRAXI
BCEQU
NULBC
```

---

#### S12F4 — Map Setup Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:15 MID
IDTYP
FNLOC
ORLOC
RPSEL
{L:n REFP
```

---

#### S12F5 — Map Transmit Inquire `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 MID
IDTYP
MAPFT
MLCL
```

---

#### S12F6 — Map Transmit Grant `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
GRNT1
```

---

#### S12F7 — Map Data Send Type 1 `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 MID
IDTYP
{L:n {L:2 RSINF
BINLT
```

---

#### S12F8 — Map Data Ack Type 1 `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
MDACK
```

---

#### S12F9 — Map Data Send Type 2 `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 MID
IDTYP
STRP
BINLT
```

---

#### S12F10 — Map Data Ack Type 2 `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
MDACK
```

---

#### S12F11 — Map Data Send Type 3 `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 MID
IDTYP
{L:n {L:2 XYPOS
BINLT
```

---

#### S12F12 — Map Data Ack Type 3 `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
MDACK
```

---

#### S12F13 — Map Data Request Type 1 `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 MID
IDTYP
```

---

#### S12F14 — Map Data Type 1 `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 MID
IDTYP
{L:n {L:2 RSINF
BINLT
```

---

#### S12F15 — Map Data Request Type 2 `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 MID
IDTYP
```

---

#### S12F16 — Map Data Type 2 `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 MID
IDTYP
STRP
BINLT
```

---

#### S12F17 — Map Data Request Type 3 `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 MID
IDTYP
SDBIN
```

---

#### S12F18 — Map Data Type 3 `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 MID
IDTYP
{L:n {L:2 XYPOS
BINLT
```

---

#### S12F19 — Map Error Report Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 MAPER
DATLC
```

---
