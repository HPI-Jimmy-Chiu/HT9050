# Stream 10：終端服務 (Terminal Services)

> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`
> 資料來源：Excel `SECS_20260401_Steven.xlsx` + 原始碼分析
> 實作檔案：`uHGemClass.cpp`、`uHGemEquipment.cpp`、`uHGemHT9045.cpp`

## 概述

Stream 10 處理終端顯示服務，包括單行/多行顯示、廣播訊息。

## 訊息一覽

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 處理函式 |
|------|------|:----:|:-----:|:------:|----------|
| S10F1 | Terminal Request (TRN) | S,H←E,[reply] | 1 | ❌ | — |
| S10F2 | Terminal Request Acknowledge (TRA) | S,H→E | 0 | ❌ | `S10F2_TerminalRequestAcknowledge()` |
| S10F3 | Terminal Display, Single (VTN) | S,H→E, [reply] | 1 | ✅ | — |
| S10F4 | Terminal Display, Single Acknowledge (VTA) | S,H←E | 0 | ✅ | `S10F4_TerminalDisplaySingle()` |
| S10F5 | Terminal Display, Multi-Block (VTN) | M,H→E,[reply] | 1 | ✅ | — |
| S10F6 | Terminal Display, Multi-block Acknowledge (VMA) | S,H←E | 0 | ✅ | `S10F6_TerminalDisplayMulti()` |
| S10F7 | Multi-block Not Allowed (MNN) | S,H←E | 0 | ❌ | — |

## 詳細說明

### S10F1 / S10F2 — Terminal Request (TRN) / Terminal Request Acknowledge (TRA)

| 屬性 | S10F1 |
|------|---|
| **方向** | S,H←E,[reply] |
| **HT9045** | ❌ |
| | |
| 屬性 | S10F2 |
| **方向** | S,H→E |
| **HT9045** | ❌ |
| **處理函式** | `S10F2_TerminalRequestAcknowledge()` |

**說明**

A terminal text message to the host.

**資料結構**

*S10F1*:
```
L, 2
    1. <TID>
    2. <TEXT>
```

*S10F2*:
```
<ACKC10>
```

**變數**

| 變數 | 說明 |
|------|------|
| TID | |
| TEXT | |
| ACKC10 | |

---

### S10F3 / S10F4 — Terminal Display, Single (VTN) / Terminal Display, Single Acknowledge (VTA)

| 屬性 | S10F3 |
|------|---|
| **方向** | S,H→E, [reply] |
| **HT9045** | ✅ |
| | |
| 屬性 | S10F4 |
| **方向** | S,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S10F4_TerminalDisplaySingle()` |

**說明**

Data to be displayed.

**資料結構**

*S10F3*:
```
L, 2
    1. <TID>
    2. <TEXT>
```

*S10F4*:
```
<ACKC10>
```

**變數**

| 變數 | 說明 |
|------|------|
| TID | |
| TEXT | |
| ACKC10 | |

---

### S10F5 / S10F6 — Terminal Display, Multi-Block (VTN) / Terminal Display, Multi-block Acknowledge (VMA)

| 屬性 | S10F5 |
|------|---|
| **方向** | M,H→E,[reply] |
| **HT9045** | ✅ |
| | |
| 屬性 | S10F6 |
| **方向** | S,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S10F6_TerminalDisplayMulti()` |

**說明**

Data to be displayed on the equipment’s terminal.

**資料結構**

*S10F5*:
```
L, 2
    1. <TID>
    2. L,n
        1. <TEXT1>
        .
        .
        n.<TEXTn>
```

*S10F6*:
```
<ACKC10>
```

**變數**

| 變數 | 說明 |
|------|------|
| TID | |
| TEXT | |
| ACKC10 | |

---

### S10F7 — Multi-block Not Allowed (MNN)

| 屬性 | S10F7 |
|------|---|
| **方向** | S,H←E |
| **HT9045** | ❌ |

**說明**

An error message from a terminal that cannot handle a multi-block message from S10,F5.

**資料結構**

```
<TID>
```

**變數**

| 變數 | 說明 |
|------|------|
| TID | |

---
---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S10F1 | Terminal Request | H←E | 0 | ❌ | [Web] |
| S10F2 | Terminal Request Acknowledge | H→E | 0 | ❌ | [Web] |
| S10F3 | Terminal Display, Single | H→E | 0 | ✅ |  |
| S10F4 | Terminal Display, Single Acknowledge | H←E | 0 | ✅ |  |
| S10F5 | Terminal Display, Multi-Block | H→E | 0 | ✅ |  |
| S10F6 | Terminal Display, Multi-Block Acknowledge | H←E | 0 | ✅ |  |
| S10F7 | Multi-block Not Allowed | H←E | 0 | ❌ | [Web] |
| S10F9 | Broadcast | H→E | 0 | ❌ | [Web] |
| S10F10 | Broadcast Acknowledge | H←E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S10F1 — Terminal Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 TID
TEXT
```

---

#### S10F2 — Terminal Request Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `S10F2_TerminalRequestAcknowledge()` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC10
```

---

#### S10F3 — Terminal Display, Single

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |

**資料結構**

```
{L:2 TID
TEXT
```

---

#### S10F4 — Terminal Display, Single Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S10F4_TerminalDisplaySingle()` |

**資料結構**

```
ACKC10
```

---

#### S10F5 — Terminal Display, Multi-Block

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |

**資料結構**

```
{L:2 TID
{L:n TEXT
```

---

#### S10F6 — Terminal Display, Multi-Block Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S10F6_TerminalDisplayMulti()` |

**資料結構**

```
ACKC10
```

---

#### S10F7 — Multi-block Not Allowed `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
TID
```

---

#### S10F9 — Broadcast `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
TEXT
```

---

#### S10F10 — Broadcast Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `S10F10_BroadcastAcknowledge()` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC10
```

---
