# SECS/GEM SF Code 文件模板

> 此模板定義每支 SxFy 訊息的標準文件格式，可用於產生客戶手冊或內部規格書。
> 版本：v1.0

---

## 模板欄位說明

| 欄位 | 說明 |
|------|------|
| **SxFy** | Stream x, Function y 編號 |
| **名稱 (Mnemonic)** | 訊息名稱與助記符 |
| **方向 (Direction)** | S=傳送, M=多播, H=Host, E=Equipment, reply=需回覆 |
| **說明 (Description)** | 訊息用途說明 |
| **資料結構 (Structure)** | SECS-II 資料項結構（L=List, A=ASCII, U4=Unsigned 4-byte, B=Binary 等） |
| **變數 (Variables)** | 結構中使用的 Data Item 名稱 |
| **例外 (Exception)** | 特殊情況或錯誤處理 |
| **HT9045 支援** | ✅ = 已實作, ❌ = 未實作 |
| **處理函式** | 程式碼中的處理函式名稱 |

---

## 單支 SF Code 模板

```markdown
### SxFy — 名稱 (Mnemonic)

| 屬性 | 值 |
|------|---|
| **方向** | S, H→E, reply |
| **HT9045 支援** | ✅ |
| **處理函式** | `FunctionName()` |

**說明**

（此訊息的用途與操作情境）

**資料結構**

\```
L,n
  1. <ITEM1>
  2. <ITEM2>
\```

**變數**

| 變數 | 說明 |
|------|------|
| ITEM1 | 說明 |
| ITEM2 | 說明 |

**例外**

（例外處理說明，或 "None"）
```

---

## SF Code 對照表模板（快速索引）

```markdown
| SxFy | 名稱 | 方向 | W-Bit | HT9045 | 處理函式 |
|------|------|:----:|:-----:|:------:|----------|
| S1F1 | Are You There | H→E | 1 | ✅ | `S1F1_AreYouThereRequest()` |
```

---

## 方向代碼對照

| 代碼 | 意義 |
|:----:|------|
| S | 送出 (Send) |
| M | 多播 (Multicast) |
| H | Host |
| E | Equipment |
| H→E | Host 傳送至 Equipment |
| H←E | Equipment 傳送至 Host |
| H↔E | 雙向 |
| reply | 需要回覆（W-Bit=1） |

## SECS-II 資料格式對照

| 格式 | 說明 | 範例 |
|------|------|------|
| L,n | List，n 個子項 | `L,2` |
| A | ASCII 字串 | `<A MDLN>` |
| B | Binary | `<B COMMACK>` |
| U1 | Unsigned 1-byte | `<U1 GRANT>` |
| U2 | Unsigned 2-byte | |
| U4 | Unsigned 4-byte | `<U4 SVID>` |
| I1 | Signed 1-byte | |
| I2 | Signed 2-byte | |
| I4 | Signed 4-byte | |
| Boolean | 布林值 | `<Boolean ALED>` |
