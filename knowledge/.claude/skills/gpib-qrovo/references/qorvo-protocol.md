# Qorvo GPIB 通訊協定詳細規格

> 來源：`SPE-001712 Rev(A)_Gpib command.docx`（Qorvo GTS，James Migliaccio，2018/09/20）
> 補充：`GPIB Custormize for Qorvo.pdf`、`SES20-086 Improvement...pdf`、`message_2020_07_30_10.TXT`

---

## 1. 協定概述

- **Handler 角色**：Listener / Talker（無 Controller 功能）
- **GPIB 位址**：Address 1（固定）
- **物理層**：IEEE 488.2，含 Service Request Line（SRQ）
- **規格版本**：SPE-001712 Rev A

| 縮寫 | 全名 |
|------|------|
| SOT | Start of Test（測試開始）|
| ESC | Empty Socket Check（空 Socket 檢查）|
| SRQ | Service Request |
| CONTACTOR | 壓頭/接觸頭編號 |

---

## 2. CONFIGURE 指令詳細規格

每個 CONFIGURE 指令收到後，Handler 清除進行中指令，以 `:OK` 或 `:NG` 回應。

### `CONFIGURE,SRQ=[n]`

| 參數 | 說明 |
|------|------|
| `Y` | 啟用 SRQ 流程【預設】|
| `N` | 不使用 SRQ |

```
回應格式：CONFIGURE,SRQ=[n]:OK
         CONFIGURE,SQR=[n]:NG（注意：規格書原文 NG 為 SQR 錯字）
```

### `CONFIGURE,FULLSITES?=[n]`

| 參數 | 說明 |
|------|------|
| `Y` | 執行 FULLSITES? 流程【預設】|
| `N` | 不執行 FULLSITES?（若 `CONFIGURE,SRQ=N` 則必須同時設 N）|

### `CONFIGURE,CONTACTOR=[n]`

| 參數 | 說明 |
|------|------|
| `Y` | FULLSITES 回應包含 CONTACTOR 資訊 |
| `N` | 不包含 CONTACTOR 資訊 |

---

## 3. FULLSITES? — SOT 後查詢 Site 狀態

在 Tester 偵測到 `SRQ41(H)` 後送出。

**Handler 回應（CONTACTOR=Y）：**
```
FULLSITES xxxxxxxx;CONTACTOR y,xxxxxxxx
```

**Handler 回應（CONTACTOR=N）：**
```
FULLSITES xxxxxxxx
```

**Bit 格式**：`xxxxxxxx` 為 8 位十六進位，最右 bit = Site 1，bit=1 表示有 IC。

範例：`Fullsites 00000003;CONTACTOR 1,00000003` → Site 1、Site 2 有 IC，CONTACTOR=1

---

## 4. BINON / BINOFF 指令

### BINON — 含 ECHO 確認

格式：`BINON:xxxxxxxx,xxxxxxxx,xxxxxxxx,xxxxxxxx;`

4 組 8 位十六進位，最右 `x` = Site 1，共 32 Site。

**BIN 值對照表：**

| GPIB 值 | Bin | 說明 |
|---------|-----|------|
| 0 | BIN 0 | Flush Bin |
| 1~9 | BIN 1~9 | |
| A | — | Retract & Re-plunge（不分 BIN，重新壓頭）|
| B | BIN 10 | |
| C | BIN 11 | |
| D | BIN 12 | |
| E | BIN 13 | |
| F | BIN 14 | |
| G | BIN 15 | |

**Handler 回應：**
```
ECHO:xxxxxxxx,xxxxxxxx,xxxxxxxx,xxxxxxxx;
```

**ECHO 失敗重試流程：**
1. Tester 未收到 ECHO → 送 `ECHONG`
2. Tester 再送一次 `BINON`
3. 若第二次仍 ECHONG → 所有零件送至 Flush Bin

### BINOFF — 無 ECHO

與 BINON 格式相同，但 Handler **不需**回應 ECHO。

---

## 5. ECHOOK / ECHONG

| 指令 | 說明 |
|------|------|
| `ECHOOK` | 確認 BINON ECHO 正確，Handler 可開始分料；或確認 CHECKEMPTY 結果為「空」|
| `ECHONG` | BINON ECHO 不符（Tester 重送 BINON）；或 CHECKEMPTY 結果「非空」（Tester 停止）|

> **重要**：Handler 必須等到收到 `ECHOOK` 後才能排料（分 BIN）。

---

## 6. QRM? / QRC? — 2D Barcode

### QRM? — 查詢支援能力（Session 開始時送一次）

```
QRM:1   支援 2D Barcode
QRM:0   不支援
```

### QRC? — 查詢各 Site 結果（收到 FULLSITES 後送出）

回應格式：`QRC:[x1],[x2],...`

| 回應值 | 意義 |
|--------|------|
| `[barcode]` | 讀取成功的 Barcode（Site 1 在最左）|
| `@` | 此 Site 無 IC |
| `$` | 此 Site 有 IC 但未讀到值 |
| `#` | 讀取失敗 |

範例：`QRC:F14110JV01A102,F14110JV01A20D,@,@,@,@,@,@,@,@,@,@,@,@,@,@`

---

## 7. PAUSE / RESUME

| 指令 | 說明 |
|------|------|
| `PAUSE` | 請求 Handler 暫停，防止 Tester 停機時 GPIB 超時；下一道指令必須是 `RESUME` |
| `RESUME` | 請求 Handler 恢復；僅在收到 `PAUSE` 後才能送出 |

---

## 8. REQUEST,CHECKEMPTY — ESC 流程

格式：`REQUEST,CHECKEMPTY`

### 觸發條件

| 條件 |
|------|
| New Lot 開始前 |
| Rear Door 開啟後又關閉 |
| 零件被 Flush 後 |
| Contactor Jam —零件在 Contactor 上遺失 |
| 操作者手動選擇 |
| Tester 送出 `REQUEST,CHECKEMPTY` |

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
    ↓ (空)          ↓ (非空)
ECHOOK            ECHONG
繼續測試           Tester 停止
```

> 若等待逾時，Handler 收到 CONFIGURE 指令後應中止 ESC 並重新執行初始化。

---

## 9. SRQ 狀態位元

| Position | 用途 |
|----------|------|
| bit 7 (MSB) | Not used |
| bit 6 | Service Request |
| bit 5~3 | Not used |
| bit 2 | Empty Socket Check |
| bit 1 | Manual Test Start |
| bit 0 (LSB) | Auto Test Start |

| SRQ 值 | Hex | 事件 |
|--------|-----|------|
| `SRQ41(H)` | 0x41 | SOT Auto Test Start |
| `SRQ42(H)` | 0x42 | Manual Test Start |
| `SRQ44(H)` | 0x44 | Empty Socket Check（ESC）|

---

## 10. 通訊流程範例

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
Response: 0x44
Rx: CHECKEMPTY
Tx: ECHOOK
Response: 0x41
Tx: FULLSITES?
Rx: Fullsites 00000003;Contactor 1,00000003
Tx: QRC?
Rx: QRC:F14110JV01A102,F14110JV01A20D,@,@,@,...
Tx: PAUSE
Tx: RESUME
Tx: BINON:00000000,00000000,00000000,00000012;
Rx: ECHO:00000000,00000000,00000000,00000012;
Tx: ECHOOK
Response: 0x41
Tx: FULLSITES?
Rx: Fullsites 00000003;Contactor 2,00000003
Tx: QRC?
Rx: QRC:F14110JV01A50B,F14110JV01A40H,@,@,@,...
Tx: BINON:00000000,00000000,00000000,00000011;
Rx: ECHO:00000000,00000000,00000000,00000011;
Tx: ECHOOK
```

---

## 11. 備註

- BINON 中 Bin 排列最右側為 Site 1（與 HT9xxx Standard BINON **相同**）
- `BINON` vs `BINOFF`：需 ECHO 確認用 BINON，不需確認用 BINOFF
- SRQ41(H) = `0x41`，同時設定 bit6(SRQ)=1 + bit0(Auto Start)=1
