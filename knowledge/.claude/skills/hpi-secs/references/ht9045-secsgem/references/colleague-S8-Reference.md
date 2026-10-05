# Stream 8：程序管理 2 (Process Program II)

> 資料來源：hume.com/secs (SEMI E5)
> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`

---

## SEMI E5 完整定義 (來源: hume.com/secs)

> ⚠️ 以下標記說明：
> - ✅ = HT9045 程式碼已實作
> - ❌ = SEMI E5 標準定義，**HT9045 尚未實作** `[來源: hume.com]`

| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 來源 |
|------|------|:----:|:-----:|:------:|:----:|
| S8F1 | Boot Program Request | H↔E | 1 | ❌ | [Web] |
| S8F2 | Boot Program Data | H↔E | 0 | ❌ | [Web] |
| S8F3 | Executive Program Request | H↔E | 1 | ❌ | [Web] |
| S8F4 | Executive Program Data | H↔E | 0 | ❌ | [Web] |

### 訊息詳細定義

#### S8F1 — Boot Program Request `[來源: hume.com]`

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

#### S8F2 — Boot Program Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
BPD
```

---

#### S8F3 — Executive Program Request `[來源: hume.com]`

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

#### S8F4 — Executive Program Data `[來源: hume.com]`

| 屬性 | 值 |
|------|---|
| **方向** | H↔E |
| **W-Bit** | 0 |
| **HT9045** | ❌ |
| **來源** | hume.com/secs (SEMI E5 標準，HT9045 未實作) |

**資料結構**

```
EPD
```

---
