---
name: rs232-standard-interface
description: >
  HT-9xxx RS-232 標準通訊介面知識庫。涵蓋指令格式（BA/CE/CF/CZ/CN/BARCODE?）、
  COM Port 設定、Handler 狀態位元、Bin 資料格式、Site Mapping、
  測試逾時處理、溫度回傳、Alarm Code、Soak Time。
  關鍵字：RS232, RS-232, Standard Command, BA, CE, CF, CZ, CN, CA, CB, CD, CH, CI,
  BARCODE?, GET2DID?, COM Port, STX ETX, Handler Status, Bin Data, Site Mapping,
  Alarm Code, Soak Time, 9600, Number of Sites, Start of Test
---

# SKILL: rs232-standard-interface

## 描述

HT-9xxx IC Test Handler RS-232 標準通訊介面知識庫（基於 `HT9xxx RS232_Interface_V12.11.843.doc`）。
當使用者詢問 RS-232 通訊協定格式、指令清單（BA/CE/CF/CZ/CN/BARCODE?）、
COM Port 設定、Pin 腳接線、Handler 狀態位元定義、Bin 資料格式、Site Mapping 格式、
Site Map 回傳格式、測試逾時處理、BA without CE 錯誤、Closed Site Bin 錯誤、
溫度回傳格式、Alarm Code 查詢、Soak Time 設定等問題時，應載入此 SKILL。

關鍵字：RS232, RS-232, Standard Command, BA, CE, CF, CZ, CK, CN, CA, CB, CD, CH, CI,
BARCODE?, GET2DID?, CZ status?, CZ testerbin?, CZ sitemap?, CZ jam?, CZ soaktime?,
CZ all masstemp?, CZ doublecontact?, CZ id?, CZ which?, COM Port, Baud Rate 9600,
STX ETX, Handler Status, Bin Data, Site Mapping, Number of Sites, Start of Test,
Sorting Count, Alarm Code, Soak Time, Index Heater Temperature, 2DID, Barcode,
Multi Double Contact, 測試逾時, Test Timeout, BA without CE, Closed Site Bin,
HT-9xxx, HT9045, HT9046, RS232Standard

## 原始文件

- **路徑**：`d:\RS232Standard\.github\skills\rs232-standard-interface\references\HT9xxx RS232_Interface_V12.11.843.doc`
- **版本**：V12.11.843（更新日期：2024/09/18，Steven）
- **適用機台**：HT-9xxx Series（HT9045, HT9046 等）

---

## COM Port 設定

| 參數 | 值 |
|------|-----|
| Baud Rate | 9600 |
| Bit Length | 7 |
| Stop Bit | 1 |
| Parity | Even |

> 所有參數需與 Tester COM Port 設定一致。

---

## Wiring（接線對應）

| Handler COM | Tester COM |
|-------------|------------|
| RD – Pin 2  | RD – Pin 2 |
| TD – Pin 3  | TD – Pin 3 |
| SG – Pin 5  | SG – Pin 5 |
| RTS – Pin 7 | RTS – Pin 7 |
| CTS – Pin 8 | CTS – Pin 8 |

---

## 通訊協定格式

```
[STX] + Command + [ETX]
```

---

## 指令清單（Command List）

| 指令 | 功能 | 說明 |
|------|------|------|
| `BA` | Bin Data | 接收 Tester 的測試分類資料 |
| `BARCODE?` | 2DID Data | 回傳每顆 IC 的 2DID（倒序，最右為 Site 1） |
| `CA` | Inhibits | 回傳 NULL String（未知指令也回傳 NULL） |
| `CB` | Index Heater Temp | 回傳 Index 接觸 Socket 的溫度 |
| `CD` | Machine ID | 回傳機台 ID |
| `CE` | Start of Test | 回傳測試 Site 號碼；無可測 Site 時回傳 NULL |
| `CF` | Number of Sites | 回傳最大測試站點數（1~8） |
| `CH` | Sleeves Full | 回傳 NULL String |
| `CI` | Input Empty | 回傳 NULL String |
| `CK` | Handler Status | 回傳 16-bit 整數（每 bit 代表特定狀態） |
| `CN` | Site Enabled | 取得各 Site 啟停狀態與 Channel 對應 |
| `CZ all masstemp?` | Index Heater Temp | 依 Test Mode 回傳各站溫度 |
| `CZ doublecontact?` | Multi Double Contact | 回傳多次接觸次數（0=關閉，1=2次，2=3次，3=4次） |
| `CZ id?` | Handler Model | 回傳 Handler 型號 |
| `CZ jam?` | Alarm Code | 回傳目前 Jam Code；無 Jam 時回傳 0 |
| `CZ jamnumber?` | — | — |
| `CZ sitemap?` | Site Mapping | 依 Test Mode 回傳各站 Site Map |
| `CZ soaktime?` | Soak Time | 回傳目前 Soak Time 設定 |
| `CZ status?` | Handler Status | 回傳 16-bit Handler 狀態整數 |
| `CZ testerbin?` | Sorting Count | 回傳各 Bin 的 IC 數量 |
| `CZ which?` | Machine ID | 回傳機台 ID |
| `GET2DID?` | 2DID Data | 回傳每顆 IC 的 2DID |

---

## 指令詳細定義

### CF – Number of Sites
- Tester 詢問 Handler 的測試站點數
- 回傳：`[STX]<SiteCount>[ETX]`（1~8）

---

### CE – Start of Test
- Tester 請求 Handler 回傳可測試的 Site 號碼

**Handler 無需測試時**：
```
[STX]NULL[ETX]
```

**Handler 準備好測試時**（例：Site 1, 2, 3, 4）：
```
[STX]1,2,3,4[ETX]
```

---

### BA – Bin Data
- 格式：
  ```
  [STX]BA<Site#1>,<DeviceID#1>,<Bin#1>;<Site#2>,<DeviceID#2>,<Bin#2>[ETX]
  ```
- 範例（請求測試 Site 1~4，收到 Tester Bin）

---

### CK / CZ status? – Handler Status（16-bit）

| Bit | 狀態名稱 | Response=1 時說明 |
|-----|---------|----------------|
| 0 | Handler Reboot | 永遠為 0 |
| 1 | Handler Output Full | 無空 Tray 時（MES1021/1421/1120/1220/1320/1720/1820/1920） |
| 2 | Handler Input Empty | Loader 上無 Tray 時 |
| 3 | Contact Cleaning | 自動 Clean 流程執行中 |
| 4 | Handler Diagnostics | 找 Home / 接觸測試 / 自動高度 / 教導 / 馬達測試 / Pause 中 |
| 5 | Index Action Without Testing | 非測試流程（找 Home / Index Check / Tray Feed / 等待 Guard Band）|
| 6 | Reversed | 永遠為 0 |
| 7 | Guard Band | 等待溫度升至設定值（Ambient 模式永遠為 0）|
| 8 | Handler Jam | 觸發 Alarm 且該 Alarm 為 Bit8 = 1 |
| 9 | Handler Stop | Handler 已停止 |
| 10 | Handler Soak | 等待 Soak Time（Ambient 模式永遠為 0）|
| 11 | Handler Door Open | 安全門開啟 |
| 12 | Handler Empty | 機台內無 IC 或 Tray |
| 13 | Handler OK | 永遠為 0 |
| 14 | Unloading Tray | 托盤送料流程執行中 |
| 15 | Loading Trays | 正在將 Tray 載入 Loader |
| 16 | Reversed | 永遠為 0 |

> 範例：`"514"` = output full (bit-2) + handler stopped (bit-9)

---

### CZ testerbin? – Sorting Count
- 格式：`[STX]Bin#1-Count#1,Bin#2-Count#2,...,U-0[ETX]`
- Count = 0 的 Bin 不會出現

---

### CZ all masstemp? – Index Heater Temperature

| Test Mode | 格式 |
|-----------|------|
| Single Site | `[STX]Site1Temp[ETX]` |
| Dual Site 1x2 | `[STX]Site1Temp Site2Temp[ETX]` |
| Quad Site 1x4 | `[STX]Site1Temp ... Site4Temp[ETX]` |
| 6 Site 2x3 | `[STX]Site1Temp ... Site6Temp[ETX]` |
| Octal Site 2x4 | `[STX]Site1Temp ... Site8Temp[ETX]` |
| 12 Site 2x6 | `[STX]Site1Temp ... Site12Temp[ETX]` |
| 16 Site 2x8 | `[STX]Site1Temp ... Site16Temp[ETX]` |

---

### CZ sitemap? – Site Mapping

| Test Mode | 格式 |
|-----------|------|
| Single Site | `[STX]Site1[ETX]` |
| Dual Site 1x2 | `[STX]Site1,Site2[ETX]` |
| Tri Site 1x3 | `[STX]Site1,Site2,Site3[ETX]` |
| Quad Site 1x4 | `[STX]Site1,Site2,Site3,Site4[ETX]` |
| Dual Site 2x1 | `[STX]Site1,Site2[ETX]` |
| Quad Site 2x2 | `[STX]Site1,Site2,Site3,Site4[ETX]` |
| 6 Site 2x3 | `[STX]Site1,Site2,Site3,Site4,Site5,Site6[ETX]` |
| Octal Site 2x4 | `[STX]Site1,...,Site8[ETX]` |
| 12 Site 2x6 | `[STX]Site1,...,Site12[ETX]` |
| 16 Site 2x8 | `[STX]Site1,...,Site16[ETX]` |

> 關閉的 Site 回傳 `0` 或 `-1`

---

### CN – Site Map（含 Channel 對應）

格式：`"[xx,yy,z][x2,yy2,z2]…"`  
- `xx` = Site 號碼  
- `yy` = Channel 號碼（無設定時為 `--`）  
- `z` = `00`（停用）, `01`（啟用）

**Handler Site Layout（1x4 模式）**：
```
Site:  1  2  3  4
```

**Handler Site Layout（2x4 模式）**：
```
     a  b  c  d
A:   1  3  5  7
B:   2  4  6  8
```

**Handler Site Layout（2x8 模式）**：
```
     a  b  c  d  e  f  g  h
A:   1  3  5  7  9  11 13 15
B:   2  4  6  8  10 12 14 16
```

範例（1x4）：
```
[STX][01,01,01][02,03,00][03,02,01][04,--,01][ETX]
```

---

### CZ jam? – Alarm Code
- 格式：`[STX]JamCode[ETX]`
- 無 Jam 時回傳 `0`

---

### CZ soaktime? – Soak Time
- 格式：`[STX]SoakTime[ETX]`
- 溫度模式為 Ambient 時回傳 `NONE`

---

### CZ doublecontact? – Multi Double Contact
- 格式：`[STX]number[ETX]`
- `0` = 功能關閉；`1` = 2次；`2` = 3次；`3` = 4次

---

### BARCODE? / GET2DID? – 2DID Data
- `BARCODE?` 格式：
  ```
  [STX]BARCODE:Site32_2D,Site31_2D,...,Site2_2D,Site1_2D[ETX]
  ```
  - **倒序**（最右為 Site 1）
  - 未用 Site 或功能關閉時回傳 `0`
  - 讀取錯誤回傳 `ERROR`

---

## 例外錯誤處理

### Test Timeout and Skip（測試逾時略過）
1. RS232 程式在 Handler 下壓後設定 `Has CE` flag = TRUE（Tester 送 CE）
2. 若 Handler 觸發測試逾時且使用者按下 Skip → Handler 設所有 IC 為 Error Bin，送 `Halt test and Skip`
3. `Has CE` flag 改為 FALSE
4. Tester 送 BA（EOT）時，因 flag = FALSE → RS232 程式送 `BA without CE Error` 給 Handler
5. Handler 警報並將 Socket 內所有 IC 設為 Error Bin

### Closed Site Has Bin（關閉站點收到 Bin）
- 例：Handler 只下壓 1 顆 IC，但 Tester 回傳 2 個 Bin
- RS232 程式觸發 `Closed Site have Bin Error`
- Handler 警報並將 Socket 內所有 IC 設為 Error Bin

---

## 版本歷史（Change List）

| 版本 | 日期 | 工程師 |
|------|------|--------|
| V7.00.550.0 | 2018/03/14 | Steven |
| V7.00.715.0 | 2018/03/15 | Steven |
| V12.01.622.0 | 2019/06/27 | Steven |
| V12.04.657.0 | 2020/07/09 | Steven |
| V12.06.700.0 | 2021/09/01 | Steven |
| V12.11.843.0 | 2024/09/18 | Steven |

---

## 驗證狀態

- **最後驗證**：2026-04-09（由原始文件 V12.11.843 轉換生成）
- **驗證狀態**：Active

---

## 搬移說明（St02 20260926）

本 skill 是從 `D:\RS232Standard\.github\skills\rs232-standard-interface` 複製進 `D:\HT9045\.claude\skills` 的（原處保留）。
下列二進位檔（客戶／廠商文件、手冊、截圖）**沒有一起搬進版控**：main 會同步到 GitHub。要看原檔請到原處：

- `references/HT9xxx RS232_Interface_V12.11.843.doc`
