---
name: gpib-qrovo
description: >
  Qorvo 客製 GPIB 通訊協定知識庫。涵蓋 CONFIGURE 流程、FULLSITES? 回應格式、
  BINON/BINOFF 排列、ECHOOK/ECHONG 重試機制、QRM?/QRC? 2D Barcode、
  PAUSE/RESUME、REQUEST,CHECKEMPTY (ESC) Empty Socket Check。
  關鍵字：Qorvo, CONFIGURE, FULLSITES?, BINON, BINOFF, ECHOOK, ECHONG,
  QRM?, QRC?, PAUSE, RESUME, CHECKEMPTY, SPE-001712, SRQ41, SRQ42, SRQ44,
  SOT, ESC, Empty Socket Check
---

# SKILL: gpib-qrovo

## 描述

Qorvo 客製 GPIB 通訊協定完整知識庫，基於 Qorvo 官方規格文件
（`SPE-001712 Rev(A)_Gpib command.docx`）與實際通訊記錄。
當使用者詢問 Qorvo GPIB 指令格式、CONFIGURE 流程、FULLSITES 回應格式、
BINON/BINOFF 排列順序、ECHOOK/ECHONG 重試機制、QRM?/QRC? 2D Barcode 查詢、
PAUSE/RESUME、REQUEST,CHECKEMPTY（ESC）、SRQ 狀態位元等問題時，應載入此 SKILL。

關鍵字：Qorvo, GPIB, CONFIGURE,SRQ, CONFIGURE,FULLSITES?, CONFIGURE,CONTACTOR,
FULLSITES?, BINON, BINOFF, ECHOOK, ECHONG, QRM?, QRC?, PAUSE, RESUME,
REQUEST,CHECKEMPTY, CHECKEMPTY, SRQ41, SRQ42, SRQ44, SOT, ESC,
Empty Socket Check, Pick & Place, IEEE488.2, SPE-001712, Qorvo Protocol

## 原始文件

| 檔案 | 說明 |
|------|------|
| `d:\GPIB9045\.github\skills\gpib-qrovo\SPE-001712 Rev(A)_Gpib command.docx`（二進位規格，不在 git：`D:\GPIB9045` 不是 git repo，只在 GPIB9045 工作區） | **主要規格書**：Qorvo Pick & Place Handler GPIB Command 規格 Rev A（2018/09/20，James Migliaccio） |
| `d:\GPIB9045\.github\skills\gpib-qrovo\GPIB Custormize for Qorvo.pdf`（二進位規格，不在 git：`D:\GPIB9045` 不是 git repo，只在 GPIB9045 工作區） | Qorvo GPIB 客製化規格（HonTech 內部文件） |
| `d:\GPIB9045\.github\skills\gpib-qrovo\SES20-086 Improvement of additional GPIB command for QORVO.pdf`（二進位規格，不在 git：`D:\GPIB9045` 不是 git repo，只在 GPIB9045 工作區） | SES20-086：Qorvo 附加 GPIB 指令改良說明 |
| `.claude/skills/hpi-gpib/references/gpib-qrovo/message_2020_07_30_10.TXT` | 2020/07/30 實際通訊記錄（含 QRC?/BINON/ESC 流程） |
| `d:\GPIB9045\.github\skills\gpib-qrovo\簡報1.pptx`（二進位規格，不在 git：`D:\GPIB9045` 不是 git repo，只在 GPIB9045 工作區） | Qorvo GPIB 功能說明簡報 |

---

## 協定概述

- **Handler 角色**：Listener / Talker（無 Controller 功能）
- **GPIB 位址**：Address 1（固定）
- **物理層**：IEEE 488.2，含 Service Request Line（SRQ）
- **開發者**：GTS 部門，James Migliaccio（Qorvo）
- **規格版本**：SPE-001712 Rev A

---

## 術語定義

| 縮寫 | 全名 |
|------|------|
| SOT | Start of Test（測試開始）|
| ESC | Empty Socket Check（空 Socket 檢查）|
| SRQ | Service Request |
| CONTACTOR | 壓頭/接觸頭編號 |

---

## 命令分類概覽

| 類別 | 指令 | 說明 |
|------|------|------|
| CONFIG | `CONFIGURE,SRQ=[n]` | 設定 SRQ 模式 |
| CONFIG | `CONFIGURE,FULLSITES?=[n]` | 設定 FULLSITES 流程 |
| CONFIG | `CONFIGURE,CONTACTOR=[n]` | 設定是否回報 CONTACTOR 資訊 |
| SOT | `FULLSITES?` | 查詢各 Site IC 在否 |
| BINON | `BINON:xxxxxxxx,...;` | 送出 BIN 指令（含 ECHO 回應）|
| BINOFF | `BINOFF:xxxxxxxx,...;` | 送出 BIN 指令（不含 ECHO 回應）|
| BINON | `ECHOOK` | 確認 BINON ECHO 正確 |
| BINON | `ECHONG` | 宣告 BINON ECHO 錯誤 |
| 2D | `QRM?` | 查詢是否支援 2D Barcode |
| 2D | `QRC?` | 查詢各 Site 2D Barcode 結果 |
| FLOW | `PAUSE` | 暫停 Handler |
| FLOW | `RESUME` | 恢復 Handler |
| ESC | `REQUEST,CHECKEMPTY` | 要求 Handler 執行空 Socket 檢查 |
| ESC | `CHECKEMPTY` | Handler 告知 Tester 執行 ESC |

## 詳細參照

> **完整 CONFIGURE 指令、FULLSITES bit 格式、BINON BIN 值對照表、ECHO 失敗重試流程、
> QRM?/QRC? 回應格式、ESC 流程、SRQ 狀態位元、通訊流程範例（標準 + 2D Barcode）**：
> [references/qorvo-protocol.md](references/qorvo-protocol.md)

---

## 詳細指令規格

### CONFIGURE 指令（測試 Session 開始時送出）

每個 CONFIGURE 指令收到後，Handler 應清除其他進行中的指令或計時器，
然後以 `:OK` 或 `:NG` 結尾回應。

---

#### `CONFIGURE,SRQ=[n]<CR><LF>`

設定 SRQ 使用模式。

| 參數 | 說明 |
|------|------|
| `[n] = "Y"` | 啟用 SRQ 流程 **[預設值]** |
| `[n] = "N"` | 不使用 SRQ |

**Handler 回應：**
```
CONFIGURE,SRQ=[n]:OK<CR><LF>   [成功]
CONFIGURE,SQR=[n]:NG<CR><LF>   [失敗]（注意：規格書 NG 回應原文有 SQR 拼寫錯誤）
```

---

#### `CONFIGURE,FULLSITES?=[n]<CR><LF>`

設定是否執行 FULLSITES 查詢流程。

| 參數 | 說明 |
|------|------|
| `[n] = "Y"` | 執行 `FULLSITES?` 流程 **[預設值]** |
| `[n] = "N"` | 不執行 `FULLSITES?` |

> **注意**：若 `CONFIGURE,SRQ=N`，必須同時設定 `CONFIGURE,FULLSITES?=N`

**Handler 回應：**
```
CONFIGURE,FULLSITES?=[n]:OK<CR><LF>    [成功]
CONFIGURE,FULLSITES?=[n]:NG<CR><LF>    [失敗]
```

---

#### `CONFIGURE,CONTACTOR=[n]<CR><LF>`

設定 FULLSITES 回應中是否包含 CONTACTOR（壓頭）資訊。

| 參數 | 說明 |
|------|------|
| `[n] = "Y"` | FULLSITES 回應包含 CONTACTOR 資訊 |
| `[n] = "N"` | 不包含 CONTACTOR 資訊 |

**Handler 回應：**
```
CONFIGURE,CONTACTOR=[n]:OK<CR><LF>     [成功]
CONFIGURE,CONTACTOR=[n]:NG<CR><LF>     [失敗]
```

---

### FULLSITES? — SOT 後查詢 Site 狀態

**格式**：`FULLSITES?<CR><LF>`

在 Tester 偵測到 `SRQ41(H)` 後送出，查詢各 Site 是否有 IC。

**Handler 回應（CONTACTOR=Y）：**
```
FULLSITES xxxxxxxx;CONTACTOR y,xxxxxxxx<CR><LF>
```

**Handler 回應（CONTACTOR=N）：**
```
FULLSITES xxxxxxxx<CR><LF>
```

**Bit 格式說明：**

| 說明 | 值 |
|------|-----|
| `xxxxxxxx` | 8 位十六進位數字，最右 bit = Site 1 |
| bit = 1 | 該 Site 有 Device |
| bit = 0 | 該 Site 無 Device |
| `y` | CONTACTOR 編號 |

**範例：**
```
Fullsites 00000003;CONTACTOR 1,00000003
```
→ Site 1 與 Site 2 均有 IC，CONTACTOR 編號 = 1

---

### BINON — BIN 指令（含 ECHO 確認）

**格式**：`BINON:xxxxxxxx,xxxxxxxx,xxxxxxxx,xxxxxxxx;<CR><LF>`

4 組 8 位十六進位數字，最右 `x` = Site 1，共可表示 32 Site。

**BIN 值對照表：**

| GPIB 值 | 十進制 BIN | 說明 |
|---------|-----------|------|
| 0 | BIN 0 | Flush Bin（不支援時排至此）|
| 1 | BIN 1 | |
| 2 | BIN 2 | |
| 3 | BIN 3 | |
| 4 | BIN 4 | |
| 5 | BIN 5 | |
| 6 | BIN 6 | |
| 7 | BIN 7 | |
| 8 | BIN 8 | |
| 9 | BIN 9 | |
| A | — | Retract & Re-plunge（重新壓頭）|
| B | BIN 10 | |
| C | BIN 11 | |
| D | BIN 12 | |
| E | BIN 13 | |
| F | BIN 14 | |
| G | BIN 15 | |

> `A`（十六進位）代表此 IC 需要縮回後再壓一次，不分類至任何 BIN。

**Handler 回應：**
```
ECHO:xxxxxxxx,xxxxxxxx,xxxxxxxx,xxxxxxxx;<CR><LF>
```

**ECHO 失敗重試流程：**
1. 若 Handler 未在規定時間內回 `ECHO:`，Tester 送出 `ECHONG`
2. Tester 再送一次 `BINON` 指令（第二次）
3. Handler 回 `ECHO:`
4. 若第二次仍 `ECHONG` → 所有零件送至 **Flush Bin**

---

### BINOFF — BIN 指令（無 ECHO）

**格式**：`BINOFF:xxxxxxxx,xxxxxxxx,xxxxxxxx,xxxxxxxx;<CR><LF>`

與 `BINON` 完全相同，但 Handler **不需回應 ECHO**。

---

### ECHOOK / ECHONG

#### `ECHOOK<CR><LF>`

- 確認 `BINON ECHO` 正確，Handler 可開始分料
- 確認 `CHECKEMPTY` 結果為「空」

> **重要**：Handler 必須等到收到 `ECHOOK` 後才能排料（分 BIN）。

#### `ECHONG<CR><LF>`

- 宣告 `BINON ECHO` 不符合，Tester 會重送 `BINON`
- 宣告 `CHECKEMPTY` 結果為「非空」，Tester 停止

---

### QRM? — 查詢 2D Barcode 支援

**格式**：`QRM?<CR><LF>`

**時機**：Session 開始時送出一次。

**Handler 回應：**
```
QRM:1<CR><LF>   [支援 2D Barcode 讀取]
QRM:0<CR><LF>   [不支援]
```

---

### QRC? — 查詢各 Site 2D Barcode 結果

**格式**：`QRC?<CR><LF>`

**時機**：收到 FULLSITES 資訊後送出。

**Handler 回應：**
```
QRC:[x1],[x2],...<CR><LF>
```

| 回應值 | 意義 |
|--------|------|
| `[barcode string]` | 讀取成功的 Barcode（Site 1 在最左）|
| `@` | 此 Site 無 IC（Part not present）|
| `$` | 此 Site 有 IC 但未讀到值（Value not read）|
| `#` | 讀取失敗（Read failure）|

**範例：**
```
QRC:F14110JV01A102,F14110JV01A20D,@,@,@,@,@,@,@,@,@,@,@,@,@,@
```
→ Site 1 = `F14110JV01A102`，Site 2 = `F14110JV01A20D`，Site 3~16 無 IC

---

### PAUSE / RESUME

#### `PAUSE<CR><LF>`

請求 Handler 暫停，防止 Tester 停機時 GPIB 超時。

> **限制**：送出 PAUSE 後，下一道指令必須是 `RESUME`。

#### `RESUME<CR><LF>`

請求 Handler 恢復處理。僅在收到 `PAUSE` 後才能送出。

---

### REQUEST,CHECKEMPTY — 空 Socket 檢查觸發

**格式**：`REQUEST,CHECKEMPTY<CR><LF>`

由 Tester 主動要求 Handler 執行空 Socket 檢查（ESC）。

**流程：**
```
Tester → REQUEST,CHECKEMPTY
Handler → SRQ44(H)
Handler → CHECKEMPTY
Tester → ECHOOK   [Socket 確認為空，繼續測試]
      or → ECHONG  [Socket 非空，Tester 停止]
```

---

## SRQ 狀態位元

SRQ Status Byte 格式（十六進位通知，以 `SRQxx(H)` 表示）：

| Bit | 用途 |
|-----|------|
| bit 7 (MSB) | Not used |
| bit 6 | Service Request |
| bit 5 | Not used |
| bit 4 | Not used |
| bit 3 | Not used |
| bit 2 | Check Empty Socket |
| bit 1 | Manual Test Start |
| bit 0 (LSB) | Auto Test Start |

**Handler 必須實作的 SRQ：**

| SRQ 值 | 十六進位 | 事件 |
|--------|---------|------|
| `SRQ41(H)` | 0x41 | Start of Test（SOT）Auto Test Start |
| `SRQ42(H)` | 0x42 | Manual Test Start |
| `SRQ44(H)` | 0x44 | Empty Socket Check（ESC）|

---

## ESC（Empty Socket Check）詳細說明

### 觸發條件

| 條件 | 說明 |
|------|------|
| New Lot 開始 | 每批新 Lot 開始前 |
| 後門開關 | Rear Door 開啟後又關閉 |
| Contactor 終止 | 零件被 Flush 後 |
| Contactor Jam | 零件在 Contactor 上遺失 |
| 手動 ESC | 操作者手動選擇 |
| Tester 要求 | Tester 送出 `REQUEST,CHECKEMPTY` |

### ESC 流程

```
Handler 偵測到 ESC 條件
    ↓
Handler 縮回已壓頭的 IC（若有）
    ↓
Handler 以空頭壓下（無 IC）
    ↓
Handler 送出 SRQ44(H) + CHECKEMPTY 給 Tester
    ↓
Tester 檢查 Test Site 是否為空
    ↓ (空)               ↓ (非空)
ECHOOK                 ECHONG
Handler 繼續測試         Tester 停止
```

> **注意**：若流程中 Tester 或 Handler 停止造成等待逾時，
> 一旦 Handler 收到 `CONFIGURE` 指令，應中止 ESC 流程並重新執行初始化。

---

## 通訊流程範例

### 標準流程（無 2D Barcode，雙 Site）

```
Tx: CONFIGURE,SRQ=Y
Rx: CONFIGURE,SRQ=Y:OK
Tx: CONFIGURE,FULLSITES?=Y
Rx: CONFIGURE,FULLSITES?=Y:OK
Tx: CONFIGURE,CONTACTOR=Y
Rx: CONFIGURE,CONTACTOR=Y:OK
Tx: QRM?
Rx: QRM:0
Tx: REQUEST,CHECKEMPTY
Response: 0x44 (ESC)
Rx: CHECKEMPTY
Tx: ECHOOK
Response: 0x41 (SOT)
Tx: FULLSITES?
Rx: Fullsites 00000003;Contactor 1,00000003
Tx: BINON:00000000,00000000,00000000,00000021;
Rx: ECHO:00000000,00000000,00000000,00000021;
Tx: ECHOOK
Response: 0x41
Tx: FULLSITES?
Rx: Fullsites 00000003;Contactor 2,00000003
Tx: BINON:00000000,00000000,00000000,00000021;
Rx: ECHO:00000000,00000000,00000000,00000021;
Tx: ECHOOK
```

### 2D Barcode + PAUSE/RESUME 流程（雙 Site）

```
Tx: CONFIGURE,SRQ=Y
Rx: CONFIGURE,SRQ=Y:OK
Tx: CONFIGURE,FULLSITES?=Y
Rx: CONFIGURE,FULLSITES?=Y:OK
Tx: CONFIGURE,CONTACTOR=Y
Rx: CONFIGURE,CONTACTOR=Y:OK
Tx: QRM?
Rx: QRM:1
Tx: REQUEST,CHECKEMPTY
Response: 0x44 (EmptySocketChk)
Rx: CHECKEMPTY
Tx: ECHOOK
Response: 0x41
Tx: FULLSITES?
Rx: Fullsites 00000003;Contactor 1,00000003
Tx: QRC?
Rx: QRC:F14110JV01A102,F14110JV01A20D,@,@,@,@,@,@,@,@,@,@,@,@,@,@
Tx: PAUSE
Tx: RESUME
Tx: BINON:00000000,00000000,00000000,00000012;
Rx: ECHO:00000000,00000000,00000000,00000012;
Tx: ECHOOK
Response: 0x41 (ReadyAutoStart)
Tx: FULLSITES?
Rx: Fullsites 00000003;Contactor 2,00000003
Tx: QRC?
Rx: QRC:F14110JV01A50B,F14110JV01A40H,@,@,@,@,@,@,@,@,@,@,@,@,@,@
Tx: BINON:00000000,00000000,00000000,00000011;
Rx: ECHO:00000000,00000000,00000000,00000011;
Tx: ECHOOK
```

---

## 備註

- 此 SKILL 對應 gpib-command-list 中的 `### Qorvo GPIB Protocol` 章節
- BINON 中 Bin 排列最右側為 Site 1（與 HT9xxx Standard BINON **相同**）
- `BINON` vs `BINOFF`：如需 ECHO 確認用 BINON，不需確認用 BINOFF
- SRQ41(H) = `0x41`，同時設定 bit6(SRQ)=1 + bit0(Auto Start)=1

---

## 搬移說明（St02 20260926）

本 skill 是從 `D:\GPIB9045\.github\skills\gpib-qrovo` 複製進 `D:\HT9045\.claude\skills` 的（原處保留）。
下列二進位檔（客戶／廠商文件、手冊、截圖）**沒有一起搬進版控**：main 會同步到 GitHub。要看原檔請到原處：

- `GPIB Custormize for Qorvo.pdf`
- `SES20-086 Improvement of additional GPIB command for QORVO.pdf`
- `SPE-001712 Rev(A)_Gpib command.docx`
- `Thumbs.db`
- `簡報1.pptx`
