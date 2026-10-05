# Stream 7：程序管理 (Process Program Management)

> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`
> 資料來源：Excel `SECS_20260401_Steven.xlsx` + 原始碼分析
> 實作檔案：`uHGemClass.cpp`、`uHGemEquipment.cpp`、`uHGemHT9045.cpp`

## 概述

Stream 7 處理程序（Recipe / Process Program）的上傳、下載、刪除與格式化程序管理。

## 訊息一覽

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 處理函式 |
|------|------|:----:|:-----:|:------:|----------|
| S7F1 | Process Program Load Inquire (PPI) | S,H↔E,reply | 1 | ✅ | — |
| S7F2 | Process Program Load Grant (PPG) | S,H↔E | 0 | ✅ | `S7F2_ProcessProgramLoadInquire()` |
| S7F3 | Process Program Send (PPS) | M,H↔E,reply | 1 | ✅ | — |
| S7F4 | Process Program Acknowledge (PPA) | S,H↔E | 0 | ✅ | `S7F4_ProcessProgramSend()` |
| S7F5 | Process Program Request (PPR) | S,H↔E,reply | 1 | ✅ | — |
| S7F6 | Process Program Data (PPD) | M,H↔E | 0 | ✅ | `S7F6_ProcessProgramRequest()` |
| S7F17 | Delete Process Program Send (DPS) | S,H→E,reply | 1 | ✅ | — |
| S7F18 | Delete Process Program Acknowledge (DPA) | S,H←E | 0 | ✅ | `S7F18_DeleteProcessProgram()` |
| S7F19 | Current EPPD Request (RER) | S,H→E,reply | 1 | ✅ | — |
| S7F20 | Current EPPD Data (RED) | M,H←E | 0 | ✅ | `S7F20_CurrentEPPD()` |
| S7F23 | Formatted Process Program Send (FPS) | M,H↔E,reply | 1 | ❌ | `S7F23_FormattedProcessProgramSend() [HT9045]` |
| S7F24 | Formatted Process Program Acknowledge (FPA) | S,H↔E | 0 | ❌ | `S7F24_FormattedProcessProgramAck() [HT9045]` |
| S7F25 | Formatted Process Program Request (FPR) | S,H↔E,reply | 1 | ❌ | `S7F25_FormattedProcessProgramRequest() [HT9045]` |
| S7F26 | Formatted Process Program Data (FPD) | M,H↔E | 0 | ❌ | `S7F26_FormattedProcessProgramData() [HT9045]` |
| S7F27 | Process Program Verification Send (PVS) | S,H←E,reply | 1 | ❌ | — |
| S7F28 | Process Program Verification Acknowledge (PVA) | S,H→E | 0 | ❌ | — |
| S7F29 | Process Program Verification Inquire (PVI) | S,H←E,reply | 1 | ❌ | — |
| S7F30 | Process Program Verification Grant (PVG) | S,H→E | 0 | ❌ | — |
| S7F37 | Large Process Program Send (LPPS) | S,H <→ E,reply | 1 | ❌ | — |
| S7F38 | Large Process Program Acknowledge(LPPA) | S,H <→ E | 0 | ❌ | — |
| S7F39 | Large Formatted Process Program Send (LFPPS) | S,H <→ E, reply | 1 | ❌ | — |
| S7F40 | Large Formatted Process Program Acknowledge (LFPPA) | S,H <→ E | 0 | ❌ | — |
| S7F41 | Large Process Program Request(LPPR) | S,H <→ E, reply | 1 | ❌ | — |
| S7F42 | Large Process Program Acknowledge (LPPA) | S,H <→ E | 0 | ❌ | — |
| S7F43 | Large Formatted Process Program Request (LFPPR) | S,H <→ E, reply | 1 | ❌ | — |
| S7F44 | Large Formatted Process Program Acknowledge (LFPPA) | S,H <→ E | 0 | ❌ | — |

## 詳細說明

### S7F1 / S7F2 — Process Program Load Inquire (PPI) / Process Program Load Grant (PPG)

| 屬性 | S7F1 |
|------|---|
| **方向** | S,H↔E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S7F2 |
| **方向** | S,H↔E |
| **HT9045** | ✅ |
| **處理函式** | `S7F2_ProcessProgramLoadInquire()` |

**說明**

This message is used to initiate the transfer of a process program or to select from stored programs. The message may be used to  
initiate the transfer of an unformatted process program (S7,F3/S7,F4) or a formatted process program  
(S7,F23/S7,F24),(S7,F31/S7,F32).

**資料結構**

*S7F1*:
```
L,2
    1. <PPID>
    2. <LENGTH>
```

*S7F2*:
```
<PPGNT>
```

**變數**

| 變數 | 說明 |
|------|------|
| PPID | |
| LENGTH | |
| PPGNT | |

---

### S7F3 / S7F4 — Process Program Send (PPS) / Process Program Acknowledge (PPA)

| 屬性 | S7F3 |
|------|---|
| **方向** | M,H↔E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S7F4 |
| **方向** | S,H↔E |
| **HT9045** | ✅ |
| **處理函式** | `S7F4_ProcessProgramSend()` |

**說明**

The program is sent. If S7,F3 is multi-block, it must be preceded by the S7,F1/S7,F2 Inquire/Grant transaction.

**資料結構**

*S7F3*:
```
L,2
    1. <PPID>
    2. <PPBODY>
```

*S7F4*:
```
<ACKC7>
```

**變數**

| 變數 | 說明 |
|------|------|
| PPID | |
| PPBODY | |
| ACK7 | |

---

### S7F5 / S7F6 — Process Program Request (PPR) / Process Program Data (PPD)

| 屬性 | S7F5 |
|------|---|
| **方向** | S,H↔E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S7F6 |
| **方向** | M,H↔E |
| **HT9045** | ✅ |
| **處理函式** | `S7F6_ProcessProgramRequest()` |

**說明**

This message is used to request the transfer of a process program.

**資料結構**

*S7F5*:
```
<PPID>
```

*S7F6*:
```
L,2
    1. <PPID>
    2. <PPBODY>
```

**變數**

| 變數 | 說明 |
|------|------|
| PPID | |
| PPBODY | |

**例外**

S7F5: None  
S7F6: A zero-length list means request denied.

---

### S7F17 / S7F18 — Delete Process Program Send (DPS) / Delete Process Program Acknowledge (DPA)

| 屬性 | S7F17 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S7F18 |
| **方向** | S,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S7F18_DeleteProcessProgram()` |

**說明**

This message is used by the host to request the equipment to delete process programs from equipment storage.

**資料結構**

*S7F17*:
```
L,n     (Number of process programs to be deleted)
    1. <PPID1>
     .
     .
    n. <PPIDn>
```

*S7F18*:
```
<ACKC7>
```

**變數**

| 變數 | 說明 |
|------|------|
| PPID | |
| ACKC | |

**例外**

S7F17: If n = 0, then delete all.  
S7F18: None

---

### S7F19 / S7F20 — Current EPPD Request (RER) / Current EPPD Data (RED)

| 屬性 | S7F19 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ✅ |
| | |
| 屬性 | S7F20 |
| **方向** | M,H←E |
| **HT9045** | ✅ |
| **處理函式** | `S7F20_CurrentEPPD()` |

**說明**

This message is used to request the transmission of the current equipment process program directory (EPPD). This is a list of all  
the PPIDs of the process programs stored in the equipment.

**資料結構**

*S7F19*:
```
Header only
```

*S7F20*:
```
L,n     (number of process programs in the directory)
    1. <PPID1>
     .
     .
    n. <PPIDn>
```

**變數**

| 變數 | 說明 |
|------|------|
| PPID | |

---

### S7F23 / S7F24 — Formatted Process Program Send (FPS) / Formatted Process Program Acknowledge (FPA)

| 屬性 | S7F23 |
|------|---|
| **方向** | M,H↔E,reply |
| **HT9045** | ❌ |
| **處理函式** | `S7F23_FormattedProcessProgramSend() [HT9045]` |
| | |
| 屬性 | S7F24 |
| **方向** | S,H↔E |
| **HT9045** | ❌ |
| **處理函式** | `S7F24_FormattedProcessProgramAck() [HT9045]` |

**說明**

This message allows movement of formatted process programs between a piece of equipment and its host system. The values of  
MDLN and SOFTREV are obtained from the PCD used to generate the process program. If S7,F23 is multi-block, it must be  
preceded by the S7F1/F2 Inquire/Grant transaction.

**資料結構**

*S7F23*:
```
L,4
    1. <PPID>
    2. <MDLN>
    3. <SOFTREV>
    4. L,c                           (c = Number of Process Commands)
        1. L,2
            1. <CCODE>
            2. L,p                  (p = Number of Parameters)
                1. <PPARM1>
                 .
                 .
                p. <PPARMp>
        2. L,2
         .
         .
        c. L,2
```

*S7F24*:
```
<ACKC7>
```

**變數**

| 變數 | 說明 |
|------|------|
| PPID | |
| MDLN | |
| SOFTREV | |
| CCODE | |
| PPARM | |
| ACKC | |

---

### S7F25 / S7F26 — Formatted Process Program Request (FPR) / Formatted Process Program Data (FPD)

| 屬性 | S7F25 |
|------|---|
| **方向** | S,H↔E,reply |
| **HT9045** | ❌ |
| **處理函式** | `S7F25_FormattedProcessProgramRequest() [HT9045]` |
| | |
| 屬性 | S7F26 |
| **方向** | M,H↔E |
| **HT9045** | ❌ |
| **處理函式** | `S7F26_FormattedProcessProgramData() [HT9045]` |

**說明**

This message is used by either equipment or host to request a particular process program from the other.

**資料結構**

*S7F25*:
```
<PPID>
```

*S7F26*:
```
L,4
    1. <PPID>
    2. <MDLN>
    3. <SOFTREV>
    4. L,c                      (c = Number of Process Commands)
        1. L,2
            1. <CCODE>
            2. L,p             (p = Number of Parameters)
                1. <PPARM1>
                 .
                 .
                p. <PPARMp>
        2. L,2
         .
         .
        c. L,2
```

**變數**

| 變數 | 說明 |
|------|------|
| PPID | |
| MDLN | |
| SOFTREV | |
| CCODE | |
| PPARM | |

**例外**

S7F25: None  
S7F26: A zero length list indicates the request was denied.

---

### S7F27 / S7F28 — Process Program Verification Send (PVS) / Process Program Verification Acknowledge (PVA)

| 屬性 | S7F27 |
|------|---|
| **方向** | S,H←E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S7F28 |
| **方向** | S,H→E |
| **HT9045** | ❌ |

**說明**

This message indicates to the host that a process program has been received and checked by the equipment. The result of the  
check is specified by the list of errors. An empty error list (list of zero-length) or a one-element list with ACKC7A having a  
value of zero (0) indicates no errors were found in the process program. The equipment may report as many errors as it seems  
appropriate. The equipment is responsible for sending a single copy of this message to the host after any reception of a  
formatted process program (S7,F23; S7,F26; S7,F31) or a large process program that was transferred via Stream 13 Data set  
Transfer Protocol (S7,F37; S7,F39; S7,F41; S7,F43). The verification of large unformatted process programs checks that the  
received process program is intact and was not corrupted by the Stream 13 transfer (e.g. by trying to load it).If S7,F27 is multiblock,  
it must be preceded by the S7,F29/S7,F30 Inquire/Grant Transaction.

**資料結構**

*S7F27*:
```
L,2
    1. <PPID>
    2. L,n      (n = number of errors being reported)
        1. L,3
            1. <ACKC7A>
            2. <SEQNUM>
            3. <ERRW7>
       2. L,3
        .
        .
       n. L,3
```

*S7F28*:
```
Header only
```

**變數**

| 變數 | 說明 |
|------|------|
| PPID | |
| ACKC7A | |
| SEQNUM | |
| ERRW7 | |

---

### S7F29 / S7F30 — Process Program Verification Inquire (PVI) / Process Program Verification Grant (PVG)

| 屬性 | S7F29 |
|------|---|
| **方向** | S,H←E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S7F30 |
| **方向** | S,H→E |
| **HT9045** | ❌ |

**說明**

This message allows a piece of equipment to ask a host for permission to send a multi-block PVS.

**資料結構**

*S7F29*:
```
<LENGTH>
```

*S7F30*:
```
<PPGNT>
```

**變數**

| 變數 | 說明 |
|------|------|
| LENGTH | |
| PPGNT | |

---

### S7F37 / S7F38 — Large Process Program Send (LPPS) / Large Process Program Acknowledge(LPPA)

| 屬性 | S7F37 |
|------|---|
| **方向** | S,H <→ E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S7F38 |
| **方向** | S,H <→ E |
| **HT9045** | ❌ |

**說明**

This is a request to send a process program via the Data Set Transfer protocol. The Data Set name, DSNAME, is the text string  
identifier of the process program, PPID. The Data Set is subsequently transferred as a Stream with the following internal SECSII  
structured data:  
<PPBODY>

**資料結構**

*S7F37*:
```
<DSNAME>
```

*S7F38*:
```
<ACKC7>
```

**變數**

| 變數 | 說明 |
|------|------|
| PPBODY | |
| DSNAME | |
| ACKC | |

---

### S7F39 / S7F40 — Large Formatted Process Program Send (LFPPS) / Large Formatted Process Program Acknowledge (LFPPA)

| 屬性 | S7F39 |
|------|---|
| **方向** | S,H <→ E, reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S7F40 |
| **方向** | S,H <→ E |
| **HT9045** | ❌ |

**說明**

This is a request to send a formatted process program via the Data Set Transfer Protocol. The Data Set name, DSNAME, is the  
text string identifier of the process program, PPID. The Data Set is subsequently transferred as a Stream with the following  
internal SECSII structured data:  
L,4  
    1. <PPID>  
    2. <MDLN>  
    3. <SOFTREV>  
    4. L,c (c = Number of Process Commands)  
        1. L,2  
            1. <CCODE>  
            2. L,p (p = Number of Parameters)  
                1. <PPARM1>  
                 .  
                 .  
                p. <PPARMp>  
     2. L,2  
      .  
      .  
     c. L,2

**資料結構**

*S7F39*:
```
<DSNAME>
```

*S7F40*:
```
<ACKC7>
```

**變數**

| 變數 | 說明 |
|------|------|
| PPID | |
| MDLN | |
| SOFTREV | |
| CCODE | |
| PPARM | |
| DSNAME | |
| ACKC | |

**例外**

S7F39: none  
S7F40: None

---

### S7F41 / S7F42 — Large Process Program Request(LPPR) / Large Process Program Acknowledge (LPPA)

| 屬性 | S7F41 |
|------|---|
| **方向** | S,H <→ E, reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S7F42 |
| **方向** | S,H <→ E |
| **HT9045** | ❌ |

**說明**

This message is used to request the transfer of a process program via the Stream 13 Data set Transfer protocol. The Data Set  
name, DSNAME, is the text string identifier of the process program, PPID. The Data Set is subsequently transferred as a Stream  
with the following internal SECSII structured data:  
<PPBODY>

**資料結構**

*S7F41*:
```
<DSNAME>
```

*S7F42*:
```
<ACKC7>
```

**變數**

| 變數 | 說明 |
|------|------|
| DSNAME | |
| PPBODY | |
| ACKC | |

**例外**

S7F41: None  
S7F42: It is possible to use the ACKC7 code “command will be performed with completion signaled later” for this message.

---

### S7F43 / S7F44 — Large Formatted Process Program Request (LFPPR) / Large Formatted Process Program Acknowledge (LFPPA)

| 屬性 | S7F43 |
|------|---|
| **方向** | S,H <→ E, reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S7F44 |
| **方向** | S,H <→ E |
| **HT9045** | ❌ |

**說明**

This message is used to request the transfer of a formatted process program via the Data set Transfer protocol. The Data Set  
name, DSNAME, is the text string identifier of the process program, PPID. The Data Set is subsequently transferred as a Stream  
with the following internal SECSII structured data:  
L,4  
    1. <PPID>  
    2. <MDLN>  
    3. <SOFTREV>  
    4. L,c                 (c = Number of Process Commands)  
        1. L,2  
            1. <CCODE>  
           2. L,p         (p = Number of Parameters)  
               2. <PPARM1>  
                .  
                .  
               p. <PPARMp>  
        2. L,2  
         .  
         .  
        c. L,2

**資料結構**

*S7F43*:
```
<DSNAME>
```

*S7F44*:
```
<ACKC7>
```

**變數**

| 變數 | 說明 |
|------|------|
| PPID | |
| MDLN | |
| SOFTREV | |
| CCODE | |
| PPARM | |
| DSNAME | |
| ACKC | |

**例外**

S7F43: None  
S7F44: It is possible to use the ACKC7 code “command will be performed with completion signaled later” for this message.

---
---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S7F1 | Process Program Load Inquire | H↔E | 1 | ✅ |  |
| S7F2 | Process Program Load Grant | H↔E | 0 | ✅ |  |
| S7F3 | Process Program Send | H↔E | 1 | ✅ |  |
| S7F4 | Process Program Send Acknowledge | H↔E | 0 | ✅ |  |
| S7F5 | Process Program Request | H↔E | 1 | ✅ |  |
| S7F6 | Process Program Data | H↔E | 0 | ✅ |  |
| S7F7 | Process Program ID Request | H←E | 1 | ❌ | [Web] |
| S7F8 | Process Program ID Data | H→E | 0 | ❌ | [Web] |
| S7F9 | Matl/Process Matrix Request | H↔E | 1 | ❌ | [Web] |
| S7F10 | Matl/Process Matrix Data | H↔E | 0 | ❌ | [Web] |
| S7F11 | Matl/Process Matrix Update Send | H→E | 0 | ❌ | [Web] |
| S7F12 | Matl/Process Matrix Update Ack | H←E | 0 | ❌ | [Web] |
| S7F13 | Matl/Process Matrix Delete Entry Send | H→E | 0 | ❌ | [Web] |
| S7F14 | Delete Matl/Process Matrix Entry Acknowledge | H←E | 0 | ❌ | [Web] |
| S7F15 | Matrix Mode Select Send | H→E | 1 | ❌ | [Web] |
| S7F16 | Matrix Mode Select Ack | H←E | 0 | ❌ | [Web] |
| S7F17 | Delete Process Program Send | H→E | 1 | ✅ |  |
| S7F18 | Delete Process Program Acknowledge | H←E | 0 | ✅ |  |
| S7F19 | Current Process Program Dir Request | H→E | 1 | ✅ |  |
| S7F20 | Current Process Program Data | H←E | 0 | ✅ |  |
| S7F21 | Process Capabilities Request | H→E | 0 | ❌ | [Web] |
| S7F22 | Process Capabilities Data | H←E | 0 | ❌ | [Web] |
| S7F23 | Formatted Process Program Send | H↔E | 1 | ❌ | [Web] |
| S7F24 | Formatted Process Program Acknowledge | H↔E | 0 | ❌ | [Web] |
| S7F25 | Formatted Process Program Request | H↔E | 1 | ❌ | [Web] |
| S7F26 | Formatted Process Program Data | H↔E | 0 | ❌ | [Web] |
| S7F27 | Process Program Verification Send | H←E | 1 | ❌ | [Web] |
| S7F28 | Process Program Verification Acknowledge | H→E | 0 | ❌ | [Web] |
| S7F29 | Process Program Verification Inquire | H←E | 1 | ❌ | [Web] |
| S7F30 | Process Program Verification Grant | H→E | 0 | ❌ | [Web] |
| S7F31 | Verification Request Send | H→E | 1 | ❌ | [Web] |
| S7F32 | Verification Request Acknowledge | H←E | 0 | ❌ | [Web] |
| S7F33 | Process Program Available Request | H↔E | 1 | ❌ | [Web] |
| S7F34 | Process Program Availability Data | H↔E | 0 | ❌ | [Web] |
| S7F35 | Process Program for MID Request | H↔E | 1 | ❌ | [Web] |
| S7F36 | Process Program for MID Data | H↔E | 0 | ❌ | [Web] |
| S7F37 | Large PP Send | H↔E | 1 | ❌ | [Web] |
| S7F38 | Large PP Send Ack | H↔E | 0 | ❌ | [Web] |
| S7F39 | Large Formatted PP Send | H↔E | 1 | ❌ | [Web] |
| S7F40 | Large Formatted PP Ack | H↔E | 0 | ❌ | [Web] |
| S7F41 | Large PP Req | H↔E | 1 | ❌ | [Web] |
| S7F42 | Large PP Req Ack | H↔E | 0 | ❌ | [Web] |
| S7F43 | Large Formatted PP Req | H↔E | 1 | ❌ | [Web] |
| S7F44 | Large Formatted PP Req Ack | H↔E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S7F1 — Process Program Load Inquire

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**資料結構**

```
{L:2 PPID
LENGTH
```

---

#### S7F2 — Process Program Load Grant

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S7F2_ProcessProgramLoadInquire()` |

**資料結構**

```
PPGNT
```

---

#### S7F3 — Process Program Send

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**資料結構**

```
{L:2 PPID
PPBODY
```

---

#### S7F4 — Process Program Send Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S7F4_ProcessProgramSend()` |

**資料結構**

```
ACKC7
```

---

#### S7F5 — Process Program Request

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**資料結構**

```
PPID
```

---

#### S7F6 — Process Program Data

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S7F6_ProcessProgramRequest()` |

**資料結構**

```
{L:2 PPID
PPBODY
```

---

#### S7F7 — Process Program ID Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
MID
```

---

#### S7F8 — Process Program ID Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 PPID
MID
```

---

#### S7F9 — Matl/Process Matrix Request `[來源: hume.com]`

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

#### S7F10 — Matl/Process Matrix Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:n {L:2 PPID
{L:a MID
```

---

#### S7F11 — Matl/Process Matrix Update Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:n {L:2 PPID
{L:a MID
```

---

#### S7F12 — Matl/Process Matrix Update Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC7
```

---

#### S7F13 — Matl/Process Matrix Delete Entry Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:n {L:2 PPID
{L:a MID
```

---

#### S7F14 — Delete Matl/Process Matrix Entry Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC7
```

---

#### S7F15 — Matrix Mode Select Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
MMODE
```

---

#### S7F16 — Matrix Mode Select Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC7
```

---

#### S7F17 — Delete Process Program Send

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**說明**

Host can send L:0 to delete all.

**資料結構**

```
{L:n PPID
```

---

#### S7F18 — Delete Process Program Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S7F18_DeleteProcessProgram()` |

**資料結構**

```
ACKC7
```

---

#### S7F19 — Current Process Program Dir Request

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ✅ |

**資料結構**

```
header only
```

---

#### S7F20 — Current Process Program Data

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ✅ |
| **處理函式** | `S7F20_CurrentEPPD()` |

**資料結構**

```
{L:n PPID
```

---

#### S7F21 — Process Capabilities Request `[來源: hume.com]`

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

#### S7F22 — Process Capabilities Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

L:x can be L:9, L:5, or L:4 with different data items

**資料結構**

```
{L:5 MDLN
SOFTREV
CMDMAX
BYTMAX
{L:c {L:11 CCODE
CNAME
RQCMD
BLKDEF
BCDS
IBCDS
NBCDS
ACDS
IACDS
NACDS
{L:p L:x
```

---

#### S7F23 — Formatted Process Program Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **處理函式** | `S7F23_FormattedProcessProgramSend() [HT9045]` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 PPID
MDLN
SOFTREV
{L:c {L:2 CCODE
{L:p PPARM
```

---

#### S7F24 — Formatted Process Program Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `S7F24_FormattedProcessProgramAck() [HT9045]` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC7
```

---

#### S7F25 — Formatted Process Program Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **處理函式** | `S7F25_FormattedProcessProgramRequest() [HT9045]` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
PPID
```

---

#### S7F26 — Formatted Process Program Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `S7F26_FormattedProcessProgramData() [HT9045]` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 PPID
MDLN
SOFTREV
{L:c {L:2 CCODE
{L:p PPARM
```

---

#### S7F27 — Process Program Verification Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 PPID
{L:n {L:3 ACKC7A
SEQNUM
ERRW7
```

---

#### S7F28 — Process Program Verification Acknowledge `[來源: hume.com]`

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

#### S7F29 — Process Program Verification Inquire `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

This is a multiblock inquire for S7F27 so the LENGTH is the S7F27 message length an not the PP length.

**資料結構**

```
LENGTH
```

---

#### S7F30 — Process Program Verification Grant `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
PPGNT
```

---

#### S7F31 — Verification Request Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 PPID
MDLN
SOFTREV
{L:c {L:2 CCODE
{L:p PPARM
```

---

#### S7F32 — Verification Request Acknowledge `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC7
```

---

#### S7F33 — Process Program Available Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
PPID
```

---

#### S7F34 — Process Program Availability Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 PPID
UNFLEN
FRMLEN
```

---

#### S7F35 — Process Program for MID Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
MID
```

---

#### S7F36 — Process Program for MID Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 MID
PPID
PPBODY
```

---

#### S7F37 — Large PP Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
DSNAME
```

---

#### S7F38 — Large PP Send Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC7
```

---

#### S7F39 — Large Formatted PP Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
DSNAME
```

---

#### S7F40 — Large Formatted PP Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC7
```

---

#### S7F41 — Large PP Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
DSNAME
```

---

#### S7F42 — Large PP Req Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC7
```

---

#### S7F43 — Large Formatted PP Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
DSNAME
```

---

#### S7F44 — Large Formatted PP Req Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC7
```

---
