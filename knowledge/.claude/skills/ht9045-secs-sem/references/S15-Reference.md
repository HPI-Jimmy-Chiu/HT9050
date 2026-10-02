# Stream 15：工作檔管理 (Recipe Management)

> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`
> 資料來源：Excel `SECS_20260401_Steven.xlsx` + 原始碼分析
> 實作檔案：`uHGemClass.cpp`、`uHGemEquipment.cpp`、`uHGemHT9045.cpp`

## 概述

Stream 15 自訂擴充訊息。

## 訊息一覽

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 處理函式 |
|------|------|:----:|:-----:|:------:|----------|
| S15F1 | Recipe Management Multi-block Inquire | S,H↔E,reply | 1 | ❌ | — |
| S15F2 | Recipe Management Multi-block Grant | S,H↔E | 0 | ❌ | — |
| S15F21 | Recipe Action Request | M,H↔E,reply | 1 | ❌ | — |
| S15F22 | Recipe Action Acknowledge | M,H↔E | 0 | ❌ | — |
| S15F27 | Recipe Download Request | M,H→E,reply | 1 | ❌ | — |
| S15F28 | Recipe Download Acknowledge | M,H←E | 0 | ❌ | — |
| S15F29 | Recipe Verify Request | M,H→E,reply | 1 | ❌ | — |
| S15F30 | Recipe Verify Acknowledge | M,H←E | 0 | ❌ | — |
| S15F31 | Recipe Upload Request | S,H→E,reply | 1 | ❌ | — |
| S15F32 | Recipe Upload Data | M,H←E | 0 | ❌ | — |
| S15F35 | Recipe Delete Request | M,H→E,reply | 1 | ❌ | — |
| S15F36 | Recipe Delete Acknowledge | M,H←E | 0 | ❌ | — |
| S15F49 | Large Recipe Download Request (LRDR) | S,H→E,reply | 1 | ❌ | — |
| S15F50 | Large Recipe Download Acknowledge (LRDA) | S,H←E | 0 | ❌ | — |
| S15F51 | Large Recipe Upload Request (LRUR) | S,H→E,reply | 1 | ❌ | — |
| S15F52 | Large Recipe Upload Acknowledge (LRUA) | S,H←E | 0 | ❌ | — |
| S15F53 | Recipe Verification Send (RVS) | M,H←E,reply | 1 | ❌ | — |
| S15F54 | Recipe Verification Acknowledge (RVA) | S,H→E | 0 | ❌ | — |

## 詳細說明

### S15F1 / S15F2 — Recipe Management Multi-block Inquire / Recipe Management Multi-block Grant

| 屬性 | S15F1 |
|------|---|
| **方向** | S,H↔E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S15F2 |
| **方向** | S,H↔E |
| **HT9045** | ❌ |

**說明**

This message requests permission to send a multi-block message based upon a maximum length of the total message.

**資料結構**

*S15F1*:
```
L, 3
    1. <DATAID>
    2. <RCPSPEC>
    3. <RMDATASIZE>
```

*S15F2*:
```
<RMGRNT>
```

**變數**

| 變數 | 說明 |
|------|------|
| DATAID | |
| RCPSPEC | |
| RMDATASIZE | |
| RMGRNT | |

**例外**

S15F1: If RCPSPEC is zero-length, the multi-block message for which permission to send is requested does not contain a recipe.  
S15F2: None

---

### S15F21 / S15F22 — Recipe Action Request / Recipe Action Acknowledge

| 屬性 | S15F21 |
|------|---|
| **方向** | M,H↔E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S15F22 |
| **方向** | M,H↔E |
| **HT9045** | ❌ |

**說明**

This message is used to acknowledge the request to perform an action in one or more recipes within a namespace.

**資料結構**

*S15F21*:
```
L, 6
    1. <DATAID>
    2. <RCPCMD>
    3. <RMNSSPEC>
    4. <OPID>
    5. <AGENT>
    6. L,n
            1. <RCPID1>
            .
            .
            n. <RCPIDn>
```

*S15F22*:
```
L, 4
    1. <AGENT>
    2. <LINKID>
    3. <RCPCMD>
    4. L, 2
            1. <RMACK>
            2. L, p
                    1. L, 2
                            1. <ERRCODE1>
                            2. <ERRTEXT1>
                    .
                    .
                    p. L, 2
                            1. <ERRCODEp>
                            2. <ERRTEXTp>
```

**變數**

| 變數 | 說明 |
|------|------|
| DATAID | |
| RCPCMD | |
| RMNSSPEC | |
| OPID | |
| AGENT | |
| RCPID | |
| LINKID | |
| RMACK | |
| ERRCODE | |
| ERRTEXT | |

**例外**

S15F21: AGENT may be a zero-length item except for requests for certify, de-certify, download, and upload.  
S15F22: LINKID is zero if and only if all requested actions have been completed. p = 0 if and only if RMACK indicates no errors.

---

### S15F27 / S15F28 — Recipe Download Request / Recipe Download Acknowledge

| 屬性 | S15F27 |
|------|---|
| **方向** | M,H→E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S15F28 |
| **方向** | M,H←E |
| **HT9045** | ❌ |

**說明**

This message is used to send a recipe to a recipe executor. If multi-block, it shall be preceded by the S15,F1/S15,F2  
inquire/grant transaction.

**資料結構**

*S15F27*:
```
L, 5
    1. <DATAID>
    2. <RCPOWCODE>
    3. <RCPSPEC>
    4. L, m
            1. L, 2
                    1. <RCPATTRID1>
                    2. <RCPATTRDATA1>
            .
            .
            m. L, 2
                    1. <RCPATTRIDm>
                    2. <RCPATTRDATAm>
    5. <RCPBODY>
```

*S15F28*:
```
L, 3
    1. <RCPID>
    2. L, n                                                       (n = # of attributes)
            1. L, 2
                    1. <RCPATTRID1>
                    2. <RCPATTRDATA1>
            .
            .
            n. L, 2
                    1. <RCPATTRIDn>
                    2. <RCPATTRDATAn>
    3. L, 2
            1. <RMACK>
            2. L, p
                    1. L, 2
                            1. <ERRCODE1>
                            2. <ERRTEXT1>
                    .
                    .
                    p. L, 2
                            1. <ERRCODEp>
                            2. <ERRTEXTp>
```

**變數**

| 變數 | 說明 |
|------|------|
| DATAID | |
| RCPOWCODE | |
| RCPSPEC | |
| RCPATTRID | |
| RCPATTRDATA | |
| RCPBODY | |
| RCPID | |
| RMACK | |
| ERRCODE | |
| ERRTEXT | |

**例外**

S15F27: None  
S15F28: If item is a zero length item, no derived object form recipe was originated. n = 0 if and only if the recipe was not verified or  
failed verification. p = 0 if and only if RMACK indicates no errors.

---

### S15F29 / S15F30 — Recipe Verify Request / Recipe Verify Acknowledge

| 屬性 | S15F29 |
|------|---|
| **方向** | M,H→E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S15F30 |
| **方向** | M,H←E |
| **HT9045** | ❌ |

**說明**

This message is used to request verification of one or more recipes by a recipe executor. If multi-block, it shall be preceded by  
the S15F1,F2 inquire/grant transaction. The operation identifier OPID, used where multiple verification requests may be  
outstanding, may be zero if no further verifications will be requested before all current verification requests are completed by the  
recipe executor. Otherwise, OPID is generated to be unique for the requestor. RESPEC is the object specifier for the recipe  
executor.

**資料結構**

*S15F29*:
```
L, 4
    1. <DATAID>
    2. <OPID>
    3. <RESPEC>
    4. L, m
             1. <RCPID1>
             .
             .
            m. <RCPIDm>
```

*S15F30*:
```
L, 5
    1. <OPID>
    2. <LINKID>
    3. <RCPID>
    4. L, n                                                 (n = # attributes)
            1. L, 2
                    1. <RCPATTRID1>
                    2. <RCPATTRDATA1>
            .
            .
            n. L, 2
                    1. <RCPATTRIDn>
                    2. <RCPATTRDATAn>
    5. L, 2
            1. <RMACK>
            2. L, p
                    1. L, 2
                            1. <ERRCODE1>
                            2. <ERRTEXT1>
                    .
                    .
                    p. L, 2
                            1. <ERRCODEp>
                            2. <ERRTEXTp>
```

**變數**

| 變數 | 說明 |
|------|------|
| DATAID | |
| OPID | |
| RESPEC | |
| RCPID | |
| LINKID | |
| RCPATTRID | |
| RCPATTRDATA | |
| RMACK | |
| ERRCODE | |
| ERRTEXT | |

**例外**

S15F29: If RESPEC is a zero length item, the target is the recipient of the message.  
S15F30: LINKID is zero if and only if a single recipe verification was requested and has been completed. If item 3 is zero length item, no  
derived object form recipe was originated. n = 0 if and only if the recipe was not verified or failed verification. p = 0 if and only  
if RMACK indicates no errors.

---

### S15F31 / S15F32 — Recipe Upload Request / Recipe Upload Data

| 屬性 | S15F31 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S15F32 |
| **方向** | M,H←E |
| **HT9045** | ❌ |

**說明**

This message is used to request an execution recipe from a recipe executor.

**資料結構**

*S15F31*:
```
<RCPSPEC>
```

*S15F32*:
```
L, 4
    1. <RCPSPEC>
    2. L, m                                                 (m = # attributes)
             1. L, 2
                     1. <RCPATTRID1>
                     2. <RCPATTRDATA1>
             .
             .
            m. L, 2
                      1. <RCPATTRIDm>
                      2. <RCPATTRDATAm>
    3. <RCPBODY>
    4. L, 2
            1. <RMACK>
            2. L, p
                    1. L, 2
                            1. <ERRCODE1>
                            2. <ERRTEXT1>
                    .
                    .
                    p. L, 2
                            1. <ERRCODEp>
                            2. <ERRTEXTp>
```

**變數**

| 變數 | 說明 |
|------|------|
| RCPSPEC | |
| RCPATTRID | |
| RCPATTRDATA | |
| RCPBODY | |
| RMACK | |
| ERRCODE | |
| ERRTEXT | |

**例外**

S15F31: None  
S15F32: p = 0 if and only if RMACK indicates no errors.

---

### S15F35 / S15F36 — Recipe Delete Request / Recipe Delete Acknowledge

| 屬性 | S15F35 |
|------|---|
| **方向** | M,H→E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S15F36 |
| **方向** | M,H←E |
| **HT9045** | ❌ |

**說明**

This message is used to request that one or more recipes be deleted or deselected. If multi-block, it shall be preceded by the  
S15,F1/S15,F2 inquire/grant transaction.

**資料結構**

*S15F35*:
```
L, 4
    1. <DATAID>
    2. <RESPEC>
    3. <RCPDEL>
    4. L, n                                                     (n = # recipes deselected)
            1. <RCPID1>
            .
            .
            n. <RCPIDn>
```

*S15F36*:
```
L, 2
    1. <RMACK>
    2. L, p
            1. L, 2
                    1. <ERRCODE1>
                    2. <ERRTEXT1>
            .
            .
            p. L, 2
                    1. <ERRCODEp>
                    2. <ERRTEXTp>
```

**變數**

| 變數 | 說明 |
|------|------|
| DATAID | |
| RESPEC | |
| RCPDEL | |
| RCPID | |
| RMACK | |
| ERRCODE | |
| ERRTEXT | |

**例外**

S15F35: If n = 0 and recipes are to be deselected (RCPDEL = 1), then all currently-selected recipes are indicated.  
S15F36: p = 0 if and only if RMACK indicates no errors.

---

### S15F49 / S15F50 — Large Recipe Download Request (LRDR) / Large Recipe Download Acknowledge (LRDA)

| 屬性 | S15F49 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S15F50 |
| **方向** | S,H←E |
| **HT9045** | ❌ |

**說明**

This is a request by the host for the equipment to request the download of a recipe via the Stream 13 Data Set Transfer protocol.  
The Data Set name, DSNAME, is the text string identifier of the recipe, RCPSPEC. The Data Set is subsequently transferred as  
a Stream with the following internal SECSII structured data:  
  
L, 4  
    1. <RCPSPEC>  
    2. <DATAID>  
    3. L, m                                                         (m = # of attributes)  
            1. L, 2  
                    1. <RCPATTRID1>  
                    2. <RCPATTRDATA1>  
            .  
            .  
            m. L, 2  
                    1. <RCPATTRIDm>  
                    2. <RCPATTRDATAm>  
    4. <RCPBODY>

**資料結構**

*S15F49*:
```
L, 2
    1. <DSNAME>
    2. <RCPOWCODE>
```

*S15F50*:
```
<ACKC15>
```

**變數**

| 變數 | 說明 |
|------|------|
| RCPSPEC | |
| DATAID | |
| RCPATTRID | |
| RCPATTRDATA | |
| RCPBODY | |
| DSNAME | |
| RCPOWCODE | |
| ACKC15 | |

---

### S15F51 / S15F52 — Large Recipe Upload Request (LRUR) / Large Recipe Upload Acknowledge (LRUA)

| 屬性 | S15F51 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S15F52 |
| **方向** | S,H←E |
| **HT9045** | ❌ |

**說明**

This is a request by the host for the equipment to request the host to upload a recipe via the Stream 13 Data Set Transfer  
protocol. The Data Set name, DSNAME, is the text string identifier of the recipe, RCPSPEC. The Data Set is subsequently  
transferred as a Stream with the following internal SECSII structured data:  
  
L, 3  
    1. <RCPSPEC>  
    2. L, m                                                             (m = # of attributes)  
            1. L, 2  
                    1. <RCPATTRID1>  
                    2. <RCPATTRDATA1>  
            .  
            .  
            m. L, 2  
                    1. <RCPATTRIDm>  
                    2. <RCPATTRDATAm>  
    3. <RCPBODY>

**資料結構**

*S15F51*:
```
<DSNAME>
```

*S15F52*:
```
<ACKC15>
```

**變數**

| 變數 | 說明 |
|------|------|
| RCPSPEC | |
| RCPATTRID | |
| RCPATTRDATA | |
| RCPBODY | |
| DSNAME | |
| ACKC15 | |

**例外**

It is possible to use the ACKC15 code “command will be performed with completion signaled later” for this message.

---

### S15F53 / S15F54 — Recipe Verification Send (RVS) / Recipe Verification Acknowledge (RVA)

| 屬性 | S15F53 |
|------|---|
| **方向** | M,H←E,reply |
| **HT9045** | ❌ |
| | |
| 屬性 | S15F54 |
| **方向** | S,H→E |
| **HT9045** | ❌ |

**說明**

This message indicates to the host that a large recipe that was transferred via Stream 13 Data Set Transfer Protocol had been  
received and checked by the equipment. RCPID contains the identifier of a derived object form recipe if created during  
verification. The result of the check is specified by the list of errors. An empty error list indicates no errors were found in the  
recipe. The equipment is responsible for sending a single copy of this message to the host after any reception of a recipe through  
S15,F49.

**資料結構**

*S15F53*:
```
L, 3
    1. <RCPSPEC>
    2. <RCPID>
    3. L, 2
            1. <RMACK>
            2. L, n                                                         (n = number of errors being reported)
                    1. L, 2
                                    1. <ERRCODE1>
                                    2. <ERRTEXT1>
                                    .
                    n. L, 2
                                    1. <ERRCODEn>
                                    2. <ERRTEXTn>
```

*S15F54*:
```
Header only
```

**變數**

| 變數 | 說明 |
|------|------|
| RCPSPEC | |
| RCPID | |
| RMACK | |
| ERRCODE | |
| ERRTEXT | |

**例外**

n = 0 if and only if RMACK indicates no error. If RCPSEPC is a zero length item, then the recipe was not verified or failed  
verification. If RCPID is zero length, then no derived object form recipe was originated.

---
---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S15F1 | Recipe Management Multi-Block Inquire | H↔E | 1 | ❌ | [Web] |
| S15F2 | Recipe Management Multi-block Grant | H↔E | 0 | ❌ | [Web] |
| S15F3 | Recipe Namespace Action Req | H↔E | 1 | ❌ | [Web] |
| S15F4 | Recipe Namespace Action | H↔E | 0 | ❌ | [Web] |
| S15F5 | Recipe Namespace Rename Req | H↔E | 1 | ❌ | [Web] |
| S15F6 | Recipe Namespace Rename Ack | H↔E | 0 | ❌ | [Web] |
| S15F7 | Recipe Space Req | H↔E | 1 | ❌ | [Web] |
| S15F8 | Recipe Space Data | H↔E | 0 | ❌ | [Web] |
| S15F9 | Recipe Status Request | H↔E | 1 | ❌ | [Web] |
| S15F10 | Recipe Status Data | H↔E | 0 | ❌ | [Web] |
| S15F11 | Recipe Version Request | H↔E | 1 | ❌ | [Web] |
| S15F12 | Recipe Version Data | H↔E | 0 | ❌ | [Web] |
| S15F13 | Recipe Create Req | H↔E | 1 | ❌ | [Web] |
| S15F14 | Recipe Create Ack | H↔E | 0 | ❌ | [Web] |
| S15F15 | Recipe Store Req | H↔E | 1 | ❌ | [Web] |
| S15F16 | Recipe Store Ack | H↔E | 0 | ❌ | [Web] |
| S15F17 | Recipe Retrieve Req | H↔E | 1 | ❌ | [Web] |
| S15F18 | Recipe Retrieve Data | H↔E | 0 | ❌ | [Web] |
| S15F19 | Recipe Rename Req | H↔E | 1 | ❌ | [Web] |
| S15F20 | Recipe Rename Ack | H↔E | 0 | ❌ | [Web] |
| S15F21 | Recipe Action Req | H↔E | 1 | ❌ | [Web] |
| S15F22 | Recipe Action Ack | H↔E | 0 | ❌ | [Web] |
| S15F23 | Recipe Descriptor Req | H↔E | 1 | ❌ | [Web] |
| S15F24 | Recipe Descriptor Data | H↔E | 0 | ❌ | [Web] |
| S15F25 | Recipe Parameter Update Req | H↔E | 1 | ❌ | [Web] |
| S15F26 | Recipe Parameter Update Ack | H↔E | 0 | ❌ | [Web] |
| S15F27 | Recipe Download Req | H→E | 1 | ❌ | [Web] |
| S15F28 | Recipe Download Ack | H←E | 0 | ❌ | [Web] |
| S15F29 | Recipe Verify Req | H→E | 1 | ❌ | [Web] |
| S15F30 | Recipe Verify Ack | H←E | 0 | ❌ | [Web] |
| S15F31 | Recipe Unload Req | H→E | 1 | ❌ | [Web] |
| S15F32 | Recipe Unload Data | H←E | 0 | ❌ | [Web] |
| S15F33 | Recipe Select Req | H→E | 1 | ❌ | [Web] |
| S15F34 | Recipe Select Ack | H←E | 0 | ❌ | [Web] |
| S15F35 | Recipe Delete Req | H→E | 1 | ❌ | [Web] |
| S15F36 | Recipe Delete Ack | H←E | 0 | ❌ | [Web] |
| S15F37 | DRNS Segment Approve Action Req | H↔E | 1 | ❌ | [Web] |
| S15F38 | DRNS Segment Approve Action Ack | H↔E | 0 | ❌ | [Web] |
| S15F39 | DRNS Recorder Seg Req | H↔E | 1 | ❌ | [Web] |
| S15F40 | DRNS Recorder Seg Ack | H↔E | 0 | ❌ | [Web] |
| S15F41 | DRNS Recorder Mod Req | H↔E | 1 | ❌ | [Web] |
| S15F42 | DRNS Recorder Mod Ack | H↔E | 0 | ❌ | [Web] |
| S15F43 | DRNS Get Change Req | H↔E | 1 | ❌ | [Web] |
| S15F44 | DRNS Get Change Ack | H↔E | 0 | ❌ | [Web] |
| S15F45 | DRNS Mgr Seg Aprvl Req | H↔E | 1 | ❌ | [Web] |
| S15F46 | DRNS Mgr Seg Aprvl Ack | H↔E | 0 | ❌ | [Web] |
| S15F47 | DRNS Mgr Rebuild Req | H↔E | 1 | ❌ | [Web] |
| S15F48 | DRNS Mgr Rebuild Ack | H↔E | 0 | ❌ | [Web] |
| S15F49 | Large Recipe Download Req | H→E | 1 | ❌ | [Web] |
| S15F50 | Large Recipe Download Ack | H←E | 0 | ❌ | [Web] |
| S15F51 | Large Recipe Upload Req | H→E | 1 | ❌ | [Web] |
| S15F52 | Large Recipe Upload Ack | H←E | 0 | ❌ | [Web] |
| S15F53 | Recipe Verification Send | H←E | 1 | ❌ | [Web] |
| S15F54 | Recipe Verification Ack | H→E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S15F1 — Recipe Management Multi-Block Inquire `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

E5 fails to mention the message type is optional for HSMS

**資料結構**

```
{L:3 DATAID
RCPSPEC
RMDATASIZE
```

---

#### S15F2 — Recipe Management Multi-block Grant `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
RMGRNT
```

---

#### S15F3 — Recipe Namespace Action Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RMNSSPEC
RMNSCMD
```

---

#### S15F4 — Recipe Namespace Action `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RMACK
{L:p {L:2 ERRCODE
ERRTEXT
```

---

#### S15F5 — Recipe Namespace Rename Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RMNSSPEC
RMNEWNS
```

---

#### S15F6 — Recipe Namespace Rename Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RMACK
{L:p {L:2 ERRCODE
ERRTEXT
```

---

#### S15F7 — Recipe Space Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
OBJSPEC
```

---

#### S15F8 — Recipe Space Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RMSPACE
{L:2 RMACK
{L:p {L:2 ERRCODE
ERRTEXT
```

---

#### S15F9 — Recipe Status Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
RCPSPEC
```

---

#### S15F10 — Recipe Status Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 RCPSTAT
RCPVERS
{L:2 RMACK
{L:p {L:2 ERRCODE
ERRTEXT
```

---

#### S15F11 — Recipe Version Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 RMNSSPEC
RCPCLASS
RCPNAME
AGENT
```

---

#### S15F12 — Recipe Version Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 AGENT
RCPVERS
{L:2 RMACK
{L:p {L:2 ERRCODE
ERRTEXT
```

---

#### S15F13 — Recipe Create Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:5 DATAID
RCPUPDT
RCPSPEC
{L:m {L:2 RCPATTRID
RCPATTRDATA
```

---

#### S15F14 — Recipe Create Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RMACK
{L:p {L:2 ERRCODE
ERRTEXT
```

---

#### S15F15 — Recipe Store Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

L:2* can be L:2 or L:0; E5 documentation is inadequate for L:q other than L:3

**資料結構**

```
{L:4 DATAID
RCPSPEC
RCPSECCODE
{L:3 {L:2* RCPSECNM
{L:g {L:2 RCPATTRID
RCPATTRDATA
```

---

#### S15F16 — Recipe Store Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RCPSECCODE
{L:2 RMACK
{L:p {L:2 ERRCODE
ERRTEXT
```

---

#### S15F17 — Recipe Retrieve Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RCPSPEC
RCPSECCODE
```

---

#### S15F18 — Recipe Retrieve Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 {L:q {L:r RCPSECNM
{L:g {L:2 RCPATTRID
RCPATTRDATA
```

---

#### S15F19 — Recipe Rename Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 RCPSPEC
RCPRENAME
RCPNEWID
```

---

#### S15F20 — Recipe Rename Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RMACK
{L:p {L:2 ERRCODE
ERRTEXT
```

---

#### S15F21 — Recipe Action Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:6 DATAID
RCPCMD
RMNSSPEC
OPID
AGENT
{L:n RCPID
```

---

#### S15F22 — Recipe Action Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 AGENT
LINKID
RCPCMD
{L:2 RMACK
{L:p {L:2 ERRCODE
ERRTEXT
```

---

#### S15F23 — Recipe Descriptor Req `[來源: hume.com]`

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
{L:n RCPID
```

---

#### S15F24 — Recipe Descriptor Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 {L:n {L:a {L:3* RCPDESCNM
RCPDESCTIME
RCPDESCLTH
```

---

#### S15F25 — Recipe Parameter Update Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 DATAID
RMNSSPEC
AGENT
{L:n {L:3 RCPPARNM
RCPPARVAL
RCPPARRULE
```

---

#### S15F26 — Recipe Parameter Update Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RMACK
{L:p {L:2 ERRCODE
ERRTEXT
```

---

#### S15F27 — Recipe Download Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:5 DATAID
RCPOWCODE
RCPSPEC
{L:m {L:2 RCPATTRID
RCPATTRDATA
```

---

#### S15F28 — Recipe Download Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 RCPID
{L:n {L:2 RCPATTRID
RCPATTRDATA
```

---

#### S15F29 — Recipe Verify Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 DATAID
OPID
RESPEC
{L:m RCPID
```

---

#### S15F30 — Recipe Verify Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:5 OPID
LINKID
RCPID
{L:n {L:2 RCPATTRID
RCPATTRDATA
```

---

#### S15F31 — Recipe Unload Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
RCPSPEC
```

---

#### S15F32 — Recipe Unload Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 RCPSPEC
{L:m {L:2 RCPATTRID
RCPATTRDATA
```

---

#### S15F33 — Recipe Select Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 DATAID
RESPEC
{L:r {L:2 RCPID
{L:p {L:2 RCPPARNM
RCPPARVAL
```

---

#### S15F34 — Recipe Select Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RMACK
{L:p {L:2 ERRCODE
ERRTEXT
```

---

#### S15F35 — Recipe Delete Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 DATAID
RESPEC
RCPDEL
{L:n RCPID
```

---

#### S15F36 — Recipe Delete Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RMACK
{L:p {L:2 ERRCODE
ERRTEXT
```

---

#### S15F37 — DRNS Segment Approve Action Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:6 RMSEGSPEC
OBJTOKEN
RMGRNT
OPID
RCPID
RMCHGTYPE
```

---

#### S15F38 — DRNS Segment Approve Action Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RMACK
{L:p {L:2 ERRCODE
ERRTEXT
```

---

#### S15F39 — DRNS Recorder Seg Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:5 DATAID
RMNSCMD
RMRECSPEC
RMSEGSPEC
OBJTOKEN
```

---

#### S15F40 — DRNS Recorder Seg Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RMACK
{L:p {L:2 ERRCODE
ERRTEXT
```

---

#### S15F41 — DRNS Recorder Mod Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

L:c is L:7 or L:1, no explanation of L:1 item

**資料結構**

```
{L:5 DATAID
RMRECSPEC
OBJTOKEN
RMNSCMD
{L:c RCPID
RCPNEWID
RMSEGSPEC
RMCHGTYPE
OPID
TIMESTAMP
RMREQUESTOR
```

---

#### S15F42 — DRNS Recorder Mod Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RMACK
{L:p {L:2 ERRCODE
ERRTEXT
```

---

#### S15F43 — DRNS Get Change Req `[來源: hume.com]`

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
TARGETSPEC
```

---

#### S15F44 — DRNS Get Change Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 {L:n {L:7 RCPID
RCPNEWID
RMSEGSPEC
RMCHGTYPE
OPID
TIMESTAMP
RMREQUESTOR
```

---

#### S15F45 — DRNS Mgr Seg Aprvl Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 DATAID
RCPSPEC
RCPNEWID
RMCHGTYPE
```

---

#### S15F46 — DRNS Mgr Seg Aprvl Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 RMCHGTYPE
RMGRNT
OPID
```

---

#### S15F47 — DRNS Mgr Rebuild Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:5 DATAID
OBJSPEC
RMNSSPEC
RMRECSPEC
{L:n RMSEGSPEC
```

---

#### S15F48 — DRNS Mgr Rebuild Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 RMACK
{L:p {L:2 ERRCODE
ERRTEXT
```

---

#### S15F49 — Large Recipe Download Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

The DSNAME is the RCPSPEC for Stream 13 transfer.

**資料結構**

```
{L:2 DSNAME
RCPOWCODE
```

---

#### S15F50 — Large Recipe Download Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC15
```

---

#### S15F51 — Large Recipe Upload Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

The DSNAME is the RCPSPEC used in Stream 13.

**資料結構**

```
DSNAME
```

---

#### S15F52 — Large Recipe Upload Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
ACKC15
```

---

#### S15F53 — Recipe Verification Send `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H←E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 RCPSPEC
RCPID
{L:2 RMACK
{L:n {L:2 ERRCODE
ERRTEXT
```

---

#### S15F54 — Recipe Verification Ack `[來源: hume.com]`

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
