---
name: rs232-ttl-communication
description: >
  HT-9xxx TTL 通訊板（RS-232 實體層）知識庫。涵蓋 TTL Board 硬體規格、
  帧格式（SOF/W-R/CMD/DATA/CRC/EOF）、CRC16 Modbus 計算、
  INIT 初始化、SOT/DATA/EOT/DUT 邏輯設定、Bit Bit/Bit Binary 模式。
  關鍵字：TTL, TTL Board, TTL Communication, RS-232, CRC16, Modbus CRC,
  INIT, SOT, EOT, DUT, Bit Bit Mode, Bit Binary Mode, RBIN, PWON, OTME,
  CSOT, VERS, optocoupler, 光耦隔離, 4 Site, 8 Site
---

# SKILL: rs232-ttl-communication

## 描述

HT-9xxx IC Test Handler TTL 通訊板（RS-232 實體層）知識庫（基於 `20180822_TTL Communication with IPC by RS-232.pdf`，VER.07082201）。
當使用者詢問 TTL 通訊板（TTL Board）的硬體規格、RS-232 接口參數、帧格式（SOF/W-R/CMD/DATA/CRC/EOF）、
CRC16 Modbus 計算、INIT 初始化流程、SOT/DATA/EOT/DUT 主動邏輯設定、Bit Bit / Bit Binary 模式、
其他指令（MODE/SOTL/DATL/EOTL/DUTL/SOTT/DUTT/SOTS/DUTS/RBIN/PWON/OTME/CSOT/VERS）、
錯誤回傳格式（ErrSOF/ErrCMD/ErrCRC/ErrEOF/ErrSOT/ErrDAT/ErrSIT/ErrBIN）、
TTL 板開機初始化流程、光耦隔離設計、多 Site 支援（4 Site / 8 Site）等問題時，應載入此 SKILL。

關鍵字：TTL, TTL Board, TTL Communication, RS-232, TTL by RS-232, IPC, SOF, EOF, CRC16,
Modbus CRC, INIT, SOT, EOT, DUT, SOT Active Logic, DATA Active Logic, EOT Active Logic,
DUT Active Logic, Bit Bit Mode, Bit Binary Mode, 5bit, 10bit, MODE, SOTL, DATL, EOTL,
DUTL, SOTT, DUTT, SOTS, DUTS, RBIN, PWON, OTME, CSOT, VERS, TS+5V, optocoupler,
光耦隔離, 開機初始化, ErrSOF, ErrCMD, ErrCRC, ErrEOF, ErrSOT, ErrDAT, ErrSIT, ErrBIN,
HT-9xxx, HT9045, HT9046, uSocketServerClient, TTL_Rev585, RS232Standard

## 原始文件

- **路徑**：`d:\RS232Standard\.github\skills\rs232-ttl-communication\references\20180822_TTL Communication with IPC by RS-232.pdf`
- **版本**：VER.07082201（2018/08/22）
- **適用機台**：HT-9xxx Series（使用 TTL 通訊板的機型）

---

## 硬體規格

### 通訊介面

| 項目 | 規格 |
|------|------|
| 物理層介面 | RS-232 |
| Baud Rate | 115200 |
| Parity | N（None）|
| Data bits | 8 |
| Stop bits | 1 |

### TTL 介面規格

| 項目 | 規格 |
|------|------|
| Channel | 4 Site（10 bit）/ 8 Site（5 bit）|
| Active Logic Level | Programmable（SOT, DATA, DUT, EOT 可分別設定）|
| 隔離方式 | Optocoupler（光耦合器隔離）|

---

## 帧格式（Frame Format）

### SET 或 ASK 帧（IPC → TTL Board）

```
SOF  | W/R | CMD  | DATA     | CRC (crc16 modbus) | EOF
```

### ACK 帧（TTL Board → IPC）

```
SOF  | CMD  | DATA | CRC | EOF
```

### 欄位定義

| 欄位 | 說明 | 長度 | 資料型別 | 值範例 |
|------|------|------|----------|--------|
| SOF | 起始段 | 1 byte | ASCII | `@` |
| W/R | WRITE 或 READ | 1 byte | ASCII | W=`0x57`, R=`0x52` |
| CMD | 選擇功能 | 4 bytes | ASCII | — |
| DATA | 參數值 | 8 bytes | ASCII | — |
| CRC | CRC16 Modbus 校驗碼 | 2 bytes | CRC16 | — |
| EOF | 結束段 | 1 byte | ASCII | `#` |

> **CRC 計算範圍**：由 SOF 開始計算到 DATA 結束（含 SOF, W/R, CMD, DATA）

---

## 錯誤回傳格式

TTL Board 在偵測到以下錯誤時，主動回傳對應錯誤碼（格式：`@ErrXXX'CRC'#`）：

| 情況 | 錯誤碼 |
|------|--------|
| TTL 板開機 | `@Runing'CRC'#`（未收到 INIT 時每秒傳送一次）|
| SOF 錯誤 | `@ErrSOF'CRC'#` |
| CMD 錯誤 | `@ErrCMD'CRC'#` |
| CRC 錯誤 | `@ErrCRC'CRC'#` |
| EOF 錯誤 | `@ErrEOF'CRC'#` |
| 流程未完卻收到 SOT 發送要求 | `@ErrSOT'CRC'#` |
| DATA 錯誤 | `@ErrDAT'CRC'#` |
| 未使用 SITE 收到 BIN | `@ErrSIT'CRC'#` |
| BitBit 模式單 SITE 收到雙 BIN | `@ErrBIN'CRC'#` |

---

## INIT 開機初始化指令

TTL Board **每次上電都必須執行 INIT 指令**，未完成初始化時無法正常執行 TTL 功能。

### 指令格式

| 欄位 | 值 |
|------|-----|
| SOF | `@` |
| R/W | `W`（Write）|
| CMD | `INIT` |
| DATA[0] | TS+5V：On=`0x31`, Off=`0x30` |
| DATA[1] | MODE：5 BitBit=`0x30`, 10 BitBit=`0x31`, 5 BitBinary=`0x32` |
| DATA[2~9] | SOT Active Logic（Site1~8）：Low=`0x30`, High=`0x31` |
| DATA[10~17] | DATA Active Logic（Site1~8）：Low=`0x30`, High=`0x31` |
| DATA[18~25] | EOT Active Logic（Site1~8）：Low=`0x30`, High=`0x31` |
| DATA[26~33] | DUT Active Logic（Site1~8）：Low=`0x30`, High=`0x31` |
| DATA[34~37] | SOT 發送寬度（unit: ms，上限 1,000ms）|
| DATA[38~41] | DUT 發送寬度（unit: ms，上限 1,000ms）|
| DATA[42~47] | BIN 別等待 Timeout 上限（unit: ms，上限 500,000ms；設 0 則無限等待）|
| CRC[48~49] | CRC16 Modbus（由 SOF 計算到 DATA 結束）|
| EOF | `#` |

**確認正確後**：TTL Board echo 接收內容 + 韌體版本號（格式：`@VERS 年月日次序'CRC'#`）

---

## 其他指令清單（Other Command List）

| 指令 | 說明 | DATA 長度 | 資料內容 | 備註 |
|------|------|-----------|----------|------|
| `MODE` | 接收模式設定 | 8 bytes | DATA[0]: 5 BitBit=`0x30`, 10 BitBit=`0x31`, 5 BitBinary=`0x32`；Byte2~8: `0x00` | — |
| `SOTL` | SOT Active Logic（啟動信號主動邏輯）| 8 bytes | DATA[0~7]: Site1~8，High=`0x31`, Low=`0x30` | — |
| `DATL` | DATA Active Logic（資料信號主動邏輯）| 8 bytes | DATA[0~7]: Site1~8，High=`0x31`, Low=`0x30` | — |
| `EOTL` | EOT Active Logic（結束信號主動邏輯）| 8 bytes | DATA[0~7]: Site1~8，High=`0x31`, Low=`0x30` | — |
| `DUTL` | DUT Active Logic（DUT 信號主動邏輯）| 8 bytes | DATA[0~7]: Site1~8，High=`0x31`, Low=`0x30` | — |
| `SOTT` | SOT 持續發送時間 | 8 bytes | ASCII，靠右對齊，上限 1,000ms（例：330ms → `0x33 0x33 0x30`）| — |
| `DUTT` | DUT 持續發送時間 | 8 bytes | ASCII，靠右對齊，上限 1,000ms（例：330ms → `0x33 0x33 0x30`）| — |
| `SOTS` | 發送 SOT 給 Tester | 8 bytes | `0x30`=等待, `0x31`=發送；DATA[0~7] 對應 Site1~8 | — |
| `DUTS` | 發送 DUT 給 Tester | 8 bytes | `0x30`=等待, `0x31`=發送；DATA[0~7] 對應 Site1~8 | — |
| `RBIN` | 回傳 BIN 結果 | 8 bytes | `0x00`=無 bin, `0x01`=bin1, `0x02`=bin2, ..., `0x0A`=bin10；8個 byte 分別代表 site1~8 的分 BIN 結果，收完後回傳 | — |
| `PWON` | 電源開啟（+TS5V）| 8 bytes | On=`0x31`, Off=`0x30`；Byte2~8: `0x00` | — |
| `OTME` | 取得 BIN Over Time 時間 | 8 bytes | ASCII，靠右對齊，上限 500s（例：5000ms → `0x35 0x30 0x30 0x30`）；設定為 0 則無限等待 | — |
| `CSOT` | Clear SOT | 8 bytes | DATA 全為 0 時等同清除指令；無 bin 別回傳但不等待時，需清除後才可重新發送 SOT | — |
| `VERS` | 韌體版本號 | 8 bytes | ASCII，例：`07022701`（107年2月27日） | 韌體版本別：07=107年, 02=2月, 27=27號 |

---

## 工作模式說明

### 5 Bit Bit Mode（`MODE=0x30`）
- 最多 8 Site
- 每個 Site 獨立 1 bit 信號

### 10 Bit Bit Mode（`MODE=0x31`）
- 最多 4 Site
- 每個 Site 使用 10 bit 信號（較高解析度）

### 5 Bit Binary Mode（`MODE=0x32`）
- 最多 8 Site
- Binary 編碼 BIN 結果

---

## 說明書版本與時間戳

- **文件版本**：VER.07082201（民國 107 年 8 月 22 日，第 1 版）
- **韌體版本格式**：`YYMMDDNN`（例：`07022701` = 民國107年2月27日第1版）

---

## 驗證狀態

- **最後驗證**：2026-04-09（由原始文件 VER.07082201 轉換生成）
- **驗證狀態**：Active

---

## 搬移說明（St02 20260926）

本 skill 是從 `D:\RS232Standard\.github\skills\rs232-ttl-communication` 複製進 `D:\HT9045\.claude\skills` 的（原處保留）。
下列二進位檔（客戶／廠商文件、手冊、截圖）**沒有一起搬進版控**：main 會同步到 GitHub。要看原檔請到原處：

- `references/20180822_TTL Communication with IPC by RS-232.pdf`
