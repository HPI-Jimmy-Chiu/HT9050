# 自訂 Stream 訊息 (Custom Streams)

> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`
> 資料來源：Excel `SECS_20260401_Steven.xlsx` + 原始碼分析
> 實作檔案：`uHGemClass.cpp`、`uHGemEquipment.cpp`、`uHGemHT9045.cpp`

此檔案彙整 HT9045 自訂擴充的 Stream 訊息（S14, S100~S125）。

## Stream 14：2DID 擴充 (Object Services — Custom)

| SxFy | 名稱 | 方向 | HT9045 | 處理函式 |
|------|------|:----:|:------:|----------|
| S14F1 | GetAttr Request (GAR) | S,H↔E,reply | ❌ | — |
| S14F2 | GetAttr Data (GAD) | M,H↔E | ❌ | — |

### S14F1 — GetAttr Request (GAR)

| 屬性 | 值 |
|------|---|
| **方向** | S,H↔E,reply |
| **HT9045** | ❌ |

**說明**

This message is used to request a set of specified attributes for one or more objects. It consists of an “object specifier” for the  
owner of the target objects (the objects of interest), the target object type, a list of identifiers of the target objects, a filter (a list of  
qualifying relationships) that limits the target objects of interest to those that meet all of the qualifications in and the specific  
attributes whose values are requested.  
The object specifier provides a specification of the owner of the target object(s). It contains a sequence of hierarchical object  
relationships. Each element of the object specifier identifies a specific object instance that is the superior of the following object  
instance in the sequence. The last object instance in the sequence is in a hierarchical relationship to the target objects. The target  
object type designates the type of the target object, and the list of object identifiers indicates the specific instance of that type that  
are of interest. The target type may be omitted only if object identifiers are unique across all object types and the list of  
identifiers is not empty.  
The object filter is an optional list of qualifications, each of which provides a condition to be applied to the object instances of  
interest. Each qualification objects of interest are those that meet all of the specified qualifications.  
The attribute relationship quantifier is a logical binary relationship ATTRRELNi that the specified qualifying value  
ATTRDATAi has to the corresponding attribute of each instance of the desired object type(s). The objects that are to be  
qualified with this filter have an attribute value Vi such that the statement “ATTRDATAi ATTRRELNi Vi” is TRUE. If  
ATTRRELNi is omitted, the relationship of equality is intended.  
For ASCII attribute values ATTRDATAi, the characters for question mark “?” and asterisk “*” are used as “wild characters” to  
provide filtering for certain object types. The character “?” may be used in any attribute or key attribute value with an ASCII  
format to represent “any single character” and may be repeated. The asterisk character “*” may be similarly used to represent a  
variable-length string, including a null string. The string “*x” represents a string of any length that ends in “x”, the string “x*”  
represents any string that begins with “x”, and the string “*” represents any string of any non-zero length. The comparison for  
text characters is case insensitive.  
Equipment is not required to support wild characters in particular, or attribute filters in general.

**資料結構**

```
L, 5
    1. <OBJSPEC>
    2. <OBJTYPE>
    3. L, i                                                  i = identifiers of the object instances requested
            1. <OBJID1>
            .
            .
            i. <OBJIDi>
    4. L, q                                                q = # object qualifiers to match
           1. L, 3
                   1. <ATTRID1>
                   2. <ATTRDATA1>
                   3. <ATTRRELN1>
           .
           .
          q. L, 3
                   1. <ATTRIDq>
                   2. <ATTRDATAq>
                   3. <ATTRRELNq>
    5. L, a                                               a = # attributes requested
           1. <ATTRID1>
            .
            .
           a. <ATTRIDa>
```

**變數**

| 變數 | 說明 |
|------|------|
| OBJSPEC | |
| OBJTYPE | |
| OBJID | |
| ATTRID | |
| ATTRDATA | |
| ATTRRELN | |

**例外**

If OBJSPEC is a zero-length item, no object specifier is provided. If i = 0,only the filter is to be applied. If q = 0, no filter is  
specified. If both i and q = 0, information for all instances of the objects are requested. If a = 0, all attributes are requested.

---

### S14F2 — GetAttr Data (GAD)

| 屬性 | 值 |
|------|---|
| **方向** | M,H↔E |
| **HT9045** | ❌ |

**說明**

This message is used to transfer the set of requested attributes for the specified object(s). The order of attributes is retained from  
the primary message.

**資料結構**

```
L, 2
    1. L, n                                                        n = number of objects
             1. L, 2
                      1. <OBJID1>
                      2. L, a                                      a= number of attributes
                              1. L, 2
                                       1. <ATTRID1>
                                       2. <ATTRDATA1>
                              .
                              .
                             a. L,2
                                      1. <ATTRIDa>
                                      2. <ATTRDATAa>
             .
             .
            n. L,2
                    1. <OBJIDn>
                    2. L, b                                        b = number of attributes
                            1. L, 2
                                    1. <ATTRID1>
                                    2. <ATTRDATA1>
                            .
                            .
                            b. L, 2
                                    1. <ATTRIDb>
                                    2. <ATTRDATAb>
    2. L, 2
            1. <OBJACK>
            2. L, p                                              p = number of errors reported
                    1. L, 2
                            1. <ERRCODE1>
                            2. <ERRTEXT1>
                    .
                    .
                    p. L, 2
                            1. <ERRCODEp>
                            2. <ERRTEXTp>
```

---

## Stream 15：工作檔管理 (Recipe Management)

| SxFy | 名稱 | 方向 | HT9045 | 處理函式 |
|------|------|:----:|:------:|----------|
| S15F1 | Recipe Management Multi-block Inquire | S,H↔E,reply | ❌ | — |
| S15F2 | Recipe Management Multi-block Grant | S,H↔E | ❌ | — |
| S15F21 | Recipe Action Request | M,H↔E,reply | ❌ | — |
| S15F22 | Recipe Action Acknowledge | M,H↔E | ❌ | — |
| S15F27 | Recipe Download Request | M,H→E,reply | ❌ | — |
| S15F28 | Recipe Download Acknowledge | M,H←E | ❌ | — |
| S15F29 | Recipe Verify Request | M,H→E,reply | ❌ | — |
| S15F30 | Recipe Verify Acknowledge | M,H←E | ❌ | — |
| S15F31 | Recipe Upload Request | S,H→E,reply | ❌ | — |
| S15F32 | Recipe Upload Data | M,H←E | ❌ | — |
| S15F35 | Recipe Delete Request | M,H→E,reply | ❌ | — |
| S15F36 | Recipe Delete Acknowledge | M,H←E | ❌ | — |
| S15F49 | Large Recipe Download Request (LRDR) | S,H→E,reply | ❌ | — |
| S15F50 | Large Recipe Download Acknowledge (LRDA) | S,H←E | ❌ | — |
| S15F51 | Large Recipe Upload Request (LRUR) | S,H→E,reply | ❌ | — |
| S15F52 | Large Recipe Upload Acknowledge (LRUA) | S,H←E | ❌ | — |
| S15F53 | Recipe Verification Send (RVS) | M,H←E,reply | ❌ | — |
| S15F54 | Recipe Verification Acknowledge (RVA) | S,H→E | ❌ | — |

### S15F1 — Recipe Management Multi-block Inquire

| 屬性 | 值 |
|------|---|
| **方向** | S,H↔E,reply |
| **HT9045** | ❌ |

**說明**

This message requests permission to send a multi-block message based upon a maximum length of the total message.

**資料結構**

```
L, 3
    1. <DATAID>
    2. <RCPSPEC>
    3. <RMDATASIZE>
```

**變數**

| 變數 | 說明 |
|------|------|
| DATAID | |
| RCPSPEC | |
| RMDATASIZE | |

**例外**

If RCPSPEC is zero-length, the multi-block message for which permission to send is requested does not contain a recipe.

---

### S15F2 — Recipe Management Multi-block Grant

| 屬性 | 值 |
|------|---|
| **方向** | S,H↔E |
| **HT9045** | ❌ |

**說明**

This message grants or denies permission to send a multi-block message.

**資料結構**

```
<RMGRNT>
```

**變數**

| 變數 | 說明 |
|------|------|
| RMGRNT | |

---

### S15F21 — Recipe Action Request

| 屬性 | 值 |
|------|---|
| **方向** | M,H↔E,reply |
| **HT9045** | ❌ |

**說明**

This message is used to acknowledge the request to perform an action in one or more recipes within a namespace.

**資料結構**

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

**變數**

| 變數 | 說明 |
|------|------|
| DATAID | |
| RCPCMD | |
| RMNSSPEC | |
| OPID | |
| AGENT | |
| RCPID | |

**例外**

AGENT may be a zero-length item except for requests for certify, de-certify, download, and upload.

---

### S15F22 — Recipe Action Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | M,H↔E |
| **HT9045** | ❌ |

**說明**

This message is used to acknowledge the request to originate a new recipe.

**資料結構**

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
| AGENT | |
| LINKID | |
| RCPCMD | |
| RMACK | |
| ERRCODE | |
| ERRTEXT | |

**例外**

LINKID is zero if and only if all requested actions have been completed. p = 0 if and only if RMACK indicates no errors.

---

### S15F27 — Recipe Download Request

| 屬性 | 值 |
|------|---|
| **方向** | M,H→E,reply |
| **HT9045** | ❌ |

**說明**

This message is used to send a recipe to a recipe executor. If multi-block, it shall be preceded by the S15,F1/S15,F2  
inquire/grant transaction.

**資料結構**

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

**變數**

| 變數 | 說明 |
|------|------|
| DATAID | |
| RCPOWCODE | |
| RCPSPEC | |
| RCPATTRID | |
| RCPATTRDATA | |
| RCPBODY | |

---

### S15F28 — Recipe Download Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | M,H←E |
| **HT9045** | ❌ |

**說明**

This message is used to acknowledge that a recipe has been received by the recipe executor. If the recipe was successfully  
verified, the results are returned to the sender. RCPID contains the identifier of a derived object form recipe if created during  
verification.

**資料結構**

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
| RCPID | |
| RCPATTRID | |
| RCPATTRDATA | |
| RMACK | |
| ERRCODE | |
| ERRTEXT | |

**例外**

If item is a zero length item, no derived object form recipe was originated. n = 0 if and only if the recipe was not verified or  
failed verification. p = 0 if and only if RMACK indicates no errors.

---

### S15F29 — Recipe Verify Request

| 屬性 | 值 |
|------|---|
| **方向** | M,H→E,reply |
| **HT9045** | ❌ |

**說明**

This message is used to request verification of one or more recipes by a recipe executor. If multi-block, it shall be preceded by  
the S15F1,F2 inquire/grant transaction. The operation identifier OPID, used where multiple verification requests may be  
outstanding, may be zero if no further verifications will be requested before all current verification requests are completed by the  
recipe executor. Otherwise, OPID is generated to be unique for the requestor. RESPEC is the object specifier for the recipe  
executor.

**資料結構**

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

**變數**

| 變數 | 說明 |
|------|------|
| DATAID | |
| OPID | |
| RESPEC | |
| RCPID | |

**例外**

If RESPEC is a zero length item, the target is the recipient of the message.

---

### S15F30 — Recipe Verify Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | M,H←E |
| **HT9045** | ❌ |

**說明**

This message is used to acknowledge the request to verify one or more recipes. If a single recipe verification was requested and  
the recipe was successfully verified, the results are returned to the sender in this message, and RCPID contains the identifier of a  
derived object form recipe if created during verification. If multiple recipe verifications were requested, then LINKID shall be  
non-zero.

**資料結構**

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
| OPID | |
| LINKID | |
| RCPID | |
| RCPATTRID | |
| RCPATTRDATA | |
| RMACK | |
| ERRCODE | |
| ERRTEXT | |

**例外**

LINKID is zero if and only if a single recipe verification was requested and has been completed. If item 3 is zero length item, no  
derived object form recipe was originated. n = 0 if and only if the recipe was not verified or failed verification. p = 0 if and only  
if RMACK indicates no errors.

---

### S15F31 — Recipe Upload Request

| 屬性 | 值 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ❌ |

**說明**

This message is used to request an execution recipe from a recipe executor.

**資料結構**

```
<RCPSPEC>
```

**變數**

| 變數 | 說明 |
|------|------|
| RCPSPEC | |

---

### S15F32 — Recipe Upload Data

| 屬性 | 值 |
|------|---|
| **方向** | M,H←E |
| **HT9045** | ❌ |

**說明**

This message is used to send an execution recipe from a recipe executor.

**資料結構**

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

p = 0 if and only if RMACK indicates no errors.

---

### S15F35 — Recipe Delete Request

| 屬性 | 值 |
|------|---|
| **方向** | M,H→E,reply |
| **HT9045** | ❌ |

**說明**

This message is used to request that one or more recipes be deleted or deselected. If multi-block, it shall be preceded by the  
S15,F1/S15,F2 inquire/grant transaction.

**資料結構**

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

**變數**

| 變數 | 說明 |
|------|------|
| DATAID | |
| RESPEC | |
| RCPDEL | |
| RCPID | |

**例外**

If n = 0 and recipes are to be deselected (RCPDEL = 1), then all currently-selected recipes are indicated.

---

### S15F36 — Recipe Delete Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | M,H←E |
| **HT9045** | ❌ |

**說明**

This message is used to acknowledge the request that recipes be deleted or deselected.

**資料結構**

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
| RMACK | |
| ERRCODE | |
| ERRTEXT | |

**例外**

p = 0 if and only if RMACK indicates no errors.

---

### S15F49 — Large Recipe Download Request (LRDR)

| 屬性 | 值 |
|------|---|
| **方向** | S,H→E,reply |
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

```
L, 2
    1. <DSNAME>
    2. <RCPOWCODE>
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

---

### S15F50 — Large Recipe Download Acknowledge (LRDA)

| 屬性 | 值 |
|------|---|
| **方向** | S,H←E |
| **HT9045** | ❌ |

**說明**

Acknowledge or error. A returned status of “accepted” means only that the message is understood. Upon completion of the  
large recipe download request (Stream 13 Data Set transfer scenario) the equipment initiates a separate verification transaction  
(S15,F53/S15,F54) that provides the result of the verification.

**資料結構**

```
<ACKC15>
```

**變數**

| 變數 | 說明 |
|------|------|
| ACKC15 | |

---

### S15F51 — Large Recipe Upload Request (LRUR)

| 屬性 | 值 |
|------|---|
| **方向** | S,H→E,reply |
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

```
<DSNAME>
```

**變數**

| 變數 | 說明 |
|------|------|
| RCPSPEC | |
| RCPATTRID | |
| RCPATTRDATA | |
| RCPBODY | |
| DSNAME | |

---

### S15F52 — Large Recipe Upload Acknowledge (LRUA)

| 屬性 | 值 |
|------|---|
| **方向** | S,H←E |
| **HT9045** | ❌ |

**說明**

Acknowledge or error. A returned status of “accepted” means only that the message is understood. The completion of the  
request is signaled by an event report.

**資料結構**

```
<ACKC15>
```

**變數**

| 變數 | 說明 |
|------|------|
| ACKC15 | |

**例外**

It is possible to use the ACKC15 code “command will be performed with completion signaled later” for this message.

---

### S15F53 — Recipe Verification Send (RVS)

| 屬性 | 值 |
|------|---|
| **方向** | M,H←E,reply |
| **HT9045** | ❌ |

**說明**

This message indicates to the host that a large recipe that was transferred via Stream 13 Data Set Transfer Protocol had been  
received and checked by the equipment. RCPID contains the identifier of a derived object form recipe if created during  
verification. The result of the check is specified by the list of errors. An empty error list indicates no errors were found in the  
recipe. The equipment is responsible for sending a single copy of this message to the host after any reception of a recipe through  
S15,F49.

**資料結構**

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

### S15F54 — Recipe Verification Acknowledge (RVA)

| 屬性 | 值 |
|------|---|
| **方向** | S,H→E |
| **HT9045** | ❌ |

**說明**

Reply by host to equipment providing response to Recipe Verification Send (RVS).

**資料結構**

```
Header only
```

---

## Stream 103：SVID 含值查詢 (Custom SV Query)

| SxFy | 名稱 | 方向 | HT9045 | 處理函式 |
|------|------|:----:|:------:|----------|
| S103F11 | Status Variable Namelist Request with Value (SVNR) | S,H→E,reply | ❌ | — |
| S103F12 | Status Variable Namelist Reply with Value (SVNRR) | M,H←E | ❌ | — |

### S103F11 — Status Variable Namelist Request with Value (SVNR)

| 屬性 | 值 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ❌ |

**說明**

A request to the equipment to identify certain status variables.

**資料結構**

```
L,n
1. <SVID1>
.
.
n. <SVIDn>
```

**變數**

| 變數 | 說明 |
|------|------|
| SVID | |

**例外**

A zero length means report all SVIDs.

---

### S103F12 — Status Variable Namelist Reply with Value (SVNRR)

| 屬性 | 值 |
|------|---|
| **方向** | M,H←E |
| **HT9045** | ❌ |

**說明**

The equipment reports to the host the name and units of the requested SVs.

**資料結構**

```
L,n
1. L,4
1. <SVID1>
2. <SVNAME1>
3. <UNITS1>
4. <SV1>
2. L,4
.
.
n. L,4
1. <SVIDn>
2. <SVNAMEn>
3. <UNITSn>
4. <SVn>
```

**變數**

| 變數 | 說明 |
|------|------|
| SVID | |
| SVCNAME | |
| UNITS | |

---

## Stream 110：Recipe 校驗 (Custom Recipe Verify)

| SxFy | 名稱 | 方向 | HT9045 | 處理函式 |
|------|------|:----:|:------:|----------|
| S110F5 | Customer Name List Acknowledge | S,H←E,reply | — | `Process_S110F5() [HT9045]` |
| S110F6 | List Customer Name | S,H→E | — | — |
| S110F7 | Receipe Information Acknowledge | S,H←E,reply | — | — |
| S110F8 | Receipe Information Send | S,H→E | — | — |

### S110F5 — Customer Name List Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | S,H←E,reply |
| **HT9045** | — |
| **處理函式** | `Process_S110F5() [HT9045]` |

**說明**

The equipment request  Product of customer List.

**資料結構**

```
L,1
1. <MDLN >
```

**變數**

| 變數 | 說明 |
|------|------|
| MDLN | |

**例外**

Model mean equipment Type.

---

### S110F6 — List Customer Name

| 屬性 | 值 |
|------|---|
| **方向** | S,H→E |
| **HT9045** | — |

**說明**

Server Send All Customer Name

**資料結構**

```
L,n
1. <SCN1>
.
.
n. <SCNn>
```

**變數**

| 變數 | 說明 |
|------|------|
| SCN | |

---

### S110F7 — Receipe Information Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | S,H←E,reply |
| **HT9045** | — |

**說明**

The equipment request  Change Kit Name.

**資料結構**

```
L,1
1. <CKN >
```

**變數**

| 變數 | 說明 |
|------|------|
| CKN | |

**例外**

Model mean equipment Type.

---

### S110F8 — Receipe Information Send

| 屬性 | 值 |
|------|---|
| **方向** | S,H→E |
| **HT9045** | — |

**說明**

Server Send Change Kit Data

**資料結構**

```
L,25
1. <Tray Form X-Start>
2. <Tray Form X-Pitch>
3. <Tray Form Y-Start>
4. <Tray Form Y-Pitch>
5. <Tray Form X-Column>
6. <Tray Form Y-Column>
7. <Tray Form X-Width>
8. <Tray Form Y-Height>
9. <Tray Form Z-Height>
10. <Hot plate X-Start>
11. <Hot plate X-Pitch>
12. <Hot plate Y-Start>
13. <Hot plate Y-pitch>
14. <Hot plate X-Column>
15. <Hot plate Y-Column>
16. <Hot plate X-Width>
17. <Hot plate Y-Height>
18. <Hot plate Z-Height>
19. <Package Size >
20. <Lead>
21. <Package Type>
22. <No of Sites>
23. <Change Kit ID>
24. <Customer>
25. <Remarks>
```

---

## Stream 120：Setup File (Custom Setup)

| SxFy | 名稱 | 方向 | HT9045 | 處理函式 |
|------|------|:----:|:------:|----------|
| S120F1 | Setup File Information Acknowledge | S,H←E,reply | — | — |
| S120F2 | Setup File Information Send | S,H→E | — | — |

### S120F1 — Setup File Information Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | S,H←E,reply |
| **HT9045** | — |

**說明**

The equipment request  Change Kit Name.

**資料結構**

```
L,2
1. <PSNO >
2. <INSERTION>
```

---

### S120F2 — Setup File Information Send

| 屬性 | 值 |
|------|---|
| **方向** | S,H→E |
| **HT9045** | — |

**說明**

Server Send Change Kit Data

**資料結構**

```
L,15
1. <CUST_ID>
2. <DEVICE_TYPE>
3. <PKG_GROUP>
4. <LEAD_COUNT>
5. <INTERNAL_DEVICE>
6. <TEMPERATURE>
7. <TESTER_TYPE>
8. <TEST_PROGRAM>
9. <SOAKTIME>
10. <LOT_HOLD_YIELD>
11. <Good Bin>
12. <OS Bin>
13. <Non Retest>
14. <Reject Bin>
15. <Site Map>
```

---

## Stream 125：EC 啟停 (Custom EC Control)

| SxFy | 名稱 | 方向 | HT9045 | 處理函式 |
|------|------|:----:|:------:|----------|
| S125F1 | Enable / Disable EC Change Report | S,H→E,reply | ✅ | — |
| S125F2 | Enable / Disable EC Change Report Acknowledge | M,H←E | ✅ | — |
| S125F3 | Level Setting Change Request | S,H→E,reply | — | — |
| S125F4 | Level Setting Change Acknowledge | M,H←E | — | `Process_S125F4() [HT9045]` |

### S125F1 — Enable / Disable EC Change Report

| 屬性 | 值 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | ✅ |

**說明**

To enable or disable the report of the EC change.

**資料結構**

```
L,2
   1. <ECED >
   2. L,n
       1. <ECID1>
       .
       .
       n. <ECIDn>
```

**變數**

| 變數 | 說明 |
|------|------|
| ECED | |
| ECID | |

**例外**

A zero length means to elable or disable all EC change report.

---

### S125F2 — Enable / Disable EC Change Report Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | M,H←E |
| **HT9045** | ✅ |

**說明**

Acknowledge or error

**資料結構**

```
<ACKC5>
```

**變數**

| 變數 | 說明 |
|------|------|
| ACK5 | |

---

### S125F3 — Level Setting Change Request

| 屬性 | 值 |
|------|---|
| **方向** | S,H→E,reply |
| **HT9045** | — |

**說明**

To enable or disable the report of the EC change.

**資料結構**

```
L,n
    1. L,2
        1. <LSID1>
        2. <LEVEL1>
    2. L,2
        1. <LSID2>
        2. <LEVEL2>
    .
    .
    n. L,2
        1. <LSIDn>
        2. <LEVELn>
```

**變數**

| 變數 | 說明 |
|------|------|
| LSID | |
| LEVEL | |

**例外**

A zero length means to elable or disable all EC change report.

---

### S125F4 — Level Setting Change Acknowledge

| 屬性 | 值 |
|------|---|
| **方向** | M,H←E |
| **HT9045** | — |
| **處理函式** | `Process_S125F4() [HT9045]` |

**說明**

Acknowledge or error

**資料結構**

```
<ACKC5>
```

**變數**

| 變數 | 說明 |
|------|------|
| ACK5 | |

---
---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S14F1 | Get Attributes Request | H↔E | 1 | ❌ | [Web] |
| S14F2 | Attribute Data | H↔E | 0 | ❌ | [Web] |
| S14F3 | Set Attributes | H↔E | 1 | ❌ | [Web] |
| S14F4 | Set Attributes Reply | H↔E | 0 | ❌ | [Web] |
| S14F5 | Get Type Data | H↔E | 1 | ❌ | [Web] |
| S14F6 | Type Data | H↔E | 0 | ❌ | [Web] |
| S14F7 | Get Attribute Names for the types | H↔E | 1 | ❌ | [Web] |
| S14F8 | Attribute Names of the object types | H↔E | 0 | ❌ | [Web] |
| S14F9 | Create Obj Request | H↔E | 1 | ❌ | [Web] |
| S14F10 | Create Obj Ack | H↔E | 0 | ❌ | [Web] |
| S14F11 | Delete Obj Request | H↔E | 1 | ❌ | [Web] |
| S14F12 | Delete Obj Ack | H↔E | 0 | ❌ | [Web] |
| S14F13 | Object Attach Request | H↔E | 1 | ❌ | [Web] |
| S14F14 | Object Attach Ack | H↔E | 0 | ❌ | [Web] |
| S14F15 | Attached Obj Action Req. | H↔E | 1 | ❌ | [Web] |
| S14F16 | Attached Obj Action Ack | H↔E | 0 | ❌ | [Web] |
| S14F17 | Supervised Obj Action Req | H↔E | 1 | ❌ | [Web] |
| S14F18 | Supervised Obj Action Ack | H↔E | 0 | ❌ | [Web] |
| S14F19 | Generic Service Req | H→E | 1 | ❌ | [Web] |
| S14F20 | Generic Service Ack | H↔E | 0 | ❌ | [Web] |
| S14F21 | Generic Service Completion | H↔E | 1 | ❌ | [Web] |
| S14F22 | Generic Service Comp Ack | H↔E | 0 | ❌ | [Web] |
| S14F23 | Multi-block Generic Service Inquire | H↔E | 1 | ❌ | [Web] |
| S14F24 | Multi-block Generic Service Grant | H↔E | 0 | ❌ | [Web] |
| S14F25 | Service Name Request | H↔E | 1 | ❌ | [Web] |
| S14F26 | Service Name Data | H↔E | 0 | ❌ | [Web] |
| S14F27 | Service Parameter Name Req | H↔E | 1 | ❌ | [Web] |
| S14F28 | Service Parameter Name Data | H↔E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S14F1 — Get Attributes Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

List lengths can be 0, and OBJSPEC can be zero-length.

**資料結構**

```
{L:5 OBJSPEC
OBJTYPE
{L:i OBJID
```

---

#### S14F2 — Attribute Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 {L:n {L:2 OBJID
{L:a {L:2 ATTRID
ATTRDATA
```

---

#### S14F3 — Set Attributes `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 OBJSPEC
OBJTYPE
{L:i OBJID
```

---

#### S14F4 — Set Attributes Reply `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **處理函式** | `Process_S14F4() [HT9045]` |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 {L:i {L:2 OBJID
{L:n {L:2 ATTRID
ATTRDATA
```

---

#### S14F5 — Get Type Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

Asks for the types of objects owned by the type of specified object

**資料結構**

```
OBJSPEC
```

---

#### S14F6 — Type Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 {L:n OBJTYPE
```

---

#### S14F7 — Get Attribute Names for the types `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 OBJSPEC
{L:n OBJTYPE
```

---

#### S14F8 — Attribute Names of the object types `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 {L:n {L:2 OBJTYPE
{L:a ATTRID
```

---

#### S14F9 — Create Obj Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 OBJSPEC
OBJTYPE
{L:a {L:2 ATTRID
ATTRDATA
```

---

#### S14F10 — Create Obj Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 OBJSPEC
{L:b {L:2 ATTRID
ATTRDATA
```

---

#### S14F11 — Delete Obj Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 OBJSPEC
{L:a {L:2 ATTRID
ATTRDATA
```

---

#### S14F12 — Delete Obj Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 {L:b {L:2 ATTRID
ATTRDATA
```

---

#### S14F13 — Object Attach Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 OBJSPEC
{L:a {L:2 ATTRID
ATTRDATA
```

---

#### S14F14 — Object Attach Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 OBJTOKEN
{L:b {L:2 ATTRID
ATTRDATA
```

---

#### S14F15 — Attached Obj Action Req. `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 OBJSPEC
OBJCMD
OBJTOKEN
{L:a {L:2 ATTRID
ATTRDATA
```

---

#### S14F16 — Attached Obj Action Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 {L:b {L:2 ATTRID
ATTRDATA
```

---

#### S14F17 — Supervised Obj Action Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:4 OBJSPEC
OBJCMD
TARGETSPEC
{L:a {L:2 ATTRID
ATTRDATA
```

---

#### S14F18 — Supervised Obj Action Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 {L:b {L:2 ATTRID
ATTRDATA
```

---

#### S14F19 — Generic Service Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H→E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:5 DATAID
OPID
OBJSPEC
SVCNAME
{L:m {L:2 SPNAME
SPVAL
```

---

#### S14F20 — Generic Service Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

it is not a mistake that SVCACK is included twice

**資料結構**

```
{L:4 SVCACK
LINKID
{L:n {L:2 SPNAME
SPVAL
```

---

#### S14F21 — Generic Service Completion `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:5 DATAID
OPID
LINKID
{L:n {L:2 SPNAME
SPVAL
```

---

#### S14F22 — Generic Service Comp Ack `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
DATAACK
```

---

#### S14F23 — Multi-block Generic Service Inquire `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**說明**

You are advised not to implement this message.

**資料結構**

```
{L:2 DATAID
DATALENGTH
```

---

#### S14F24 — Multi-block Generic Service Grant `[來源: hume.com]`

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

#### S14F25 — Service Name Request `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 OBJSPEC
{L:n OBJTYPE
```

---

#### S14F26 — Service Name Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 {L:n {L:2 OBJTYPE
{L:a SVCNAME
```

---

#### S14F27 — Service Parameter Name Req `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 1 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:3 OBJSPEC
OBJTYPE
{L:n SVCNAME
```

---

#### S14F28 — Service Parameter Name Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
{L:2 {L:n {L:2 SVCNAME
{L:a SPNAME
```

---
