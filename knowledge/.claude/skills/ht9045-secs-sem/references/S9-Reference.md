# Stream 9：錯誤處理 (System Errors)

> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`
> 資料來源：Excel `SECS_20260401_Steven.xlsx` + 原始碼分析
> 實作檔案：`uHGemClass.cpp`、`uHGemEquipment.cpp`、`uHGemHT9045.cpp`

## 概述

Stream 9 處理通訊錯誤報告，包括未識別的 Device ID、Stream、Function、資料格式錯誤與交易逾時。

## 訊息一覽

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 處理函式 |
|------|------|:----:|:-----:|:------:|----------|
| S9F1 | Unrecognized Device ID (UDN) | S,H←E | 0 | ❌ | `S9F1_UnrecognizedDeviceID()` |
| S9F3 | Unrecognized Stream Type (USN) | S,H←E | 0 | ❌ | `S9F3_UnrecognizedStreamType()` |
| S9F5 | Unrecognized Function Type (UFN) | S,H←E | 0 | ❌ | `S9F5_UnrecognizedFunctionType()` |
| S9F7 | Illegal Data (IDN) | S,H←E | 0 | ❌ | `S9F7_IllegalData()` |
| S9F9 | Transaction Timer Timeout (TTN) | S,H←E | 0 | ❌ | `S9F9_TransactionTimerTimeout()` |
| S9F11 | Data Too Long (DLN) | S,H←E | 0 | ❌ | `S9F11_DataTooLong()` |
| S9F13 | Conversation Timeout (CTN) | S,H←E | 0 | ❌ | — |

## 詳細說明

### S9F1 — Unrecognized Device ID (UDN)

| 屬性 | S9F1 |
|------|---|
| **方向** | S,H←E |
| **HT9045** | ❌ |
| **處理函式** | `S9F1_UnrecognizedDeviceID()` |

**說明**

The device ID in the message block header did not correspond to any known device ID in the node detecting the error.

**資料結構**

```
<MHEAD>
```

**變數**

| 變數 | 說明 |
|------|------|
| MHEAD | |

---

### S9F3 — Unrecognized Stream Type (USN)

| 屬性 | S9F3 |
|------|---|
| **方向** | S,H←E |
| **HT9045** | ❌ |
| **處理函式** | `S9F3_UnrecognizedStreamType()` |

**說明**

The equipment does not recognize the stream type in the message block header.

**資料結構**

```
<MHEAD>
```

**變數**

| 變數 | 說明 |
|------|------|
| MHEAD | |

---

### S9F5 — Unrecognized Function Type (UFN)

| 屬性 | S9F5 |
|------|---|
| **方向** | S,H←E |
| **HT9045** | ❌ |
| **處理函式** | `S9F5_UnrecognizedFunctionType()` |

**說明**

This message indicates that the function in the message ID is not recognized by the receiver.

**資料結構**

```
<MHEAD>
```

**變數**

| 變數 | 說明 |
|------|------|
| MHEAD | |

---

### S9F7 — Illegal Data (IDN)

| 屬性 | S9F7 |
|------|---|
| **方向** | S,H←E |
| **HT9045** | ❌ |
| **處理函式** | `S9F7_IllegalData()` |

**說明**

This message indicates that the stream and function were recognized, but the associated data format could not be interpreted.

**資料結構**

```
<MHEAD>
```

**變數**

| 變數 | 說明 |
|------|------|
| MHEAD | |

---

### S9F9 — Transaction Timer Timeout (TTN)

| 屬性 | S9F9 |
|------|---|
| **方向** | S,H←E |
| **HT9045** | ❌ |
| **處理函式** | `S9F9_TransactionTimerTimeout()` |

**說明**

This message indicates that a transaction (receive) timer has timed out and that the corresponding transaction has been aborted.  
It is up to the host to respond to this error in an appropriate manner to keep the system operational.

**資料結構**

```
<SHEAD>
```

**變數**

| 變數 | 說明 |
|------|------|
| SHEAD | |

---

### S9F11 — Data Too Long (DLN)

| 屬性 | S9F11 |
|------|---|
| **方向** | S,H←E |
| **HT9045** | ❌ |
| **處理函式** | `S9F11_DataTooLong()` |

**說明**

This message to the host indicates that the equipment has been sent more data than it can handle.

**資料結構**

```
<MHEAD>
```

**變數**

| 變數 | 說明 |
|------|------|
| MHEAD | |

---

### S9F13 — Conversation Timeout (CTN)

| 屬性 | S9F13 |
|------|---|
| **方向** | S,H←E |
| **HT9045** | ❌ |

**說明**

Data were expected but none were received within a reasonable length of time. Resources have been cleared.

**資料結構**

```
L, 2
    1. <MEXP>
    2. <EDID>
```

**變數**

| 變數 | 說明 |
|------|------|
| MEXP | |
| EDID | |

---
---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S9F1 | Unknown Device ID | H←E | 0 | ❌ | [Web] |
| S9F3 | Unknown Stream | H←E | 0 | ❌ | [Web] |
| S9F5 | Unknown Function | H←E | 0 | ❌ | [Web] |
| S9F7 | Illegal Data | H←E | 0 | ❌ | [Web] |
| S9F9 | Transaction Timeout | H←E | 0 | ❌ | [Web] |
| S9F11 | Data Too Long | H←E | 0 | ❌ | [Web] |
| S9F13 | Conversation Timeout | H←E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S9F1 — Unknown Device ID `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `S9F1_UnrecognizedDeviceID()` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
MHEAD
```

---

#### S9F3 — Unknown Stream `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `S9F3_UnrecognizedStreamType()` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
MHEAD
```

---

#### S9F5 — Unknown Function `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `S9F5_UnrecognizedFunctionType()` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
MHEAD
```

---

#### S9F7 — Illegal Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `S9F7_IllegalData()` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
MHEAD
```

---

#### S9F9 — Transaction Timeout `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `S9F9_TransactionTimerTimeout()` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
SHEAD
```

---

#### S9F11 — Data Too Long `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `S9F11_DataTooLong()` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
MHEAD
```

---

#### S9F13 — Conversation Timeout `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 MEXP
EDID
```

---
