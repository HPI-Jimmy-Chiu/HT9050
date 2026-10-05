---
name: gpib-command-list
description: >
  HT9xxx GPIB Command List 完整指令知識庫。
  涵蓋 GPIB 指令格式、BINON/BINOFF 排列、SRQ 流程、BIN Mapping、
  溫度控制指令、ART/ATC 功能、2DID/Barcode 指令、Device Map、
  Remote Control 指令、Qorvo/SIGURD 客製 GPIB 協定。
  關鍵字：GPIB Command, BINON, BINOFF, SRQ, BIN Mapping, FULLSITES,
  ECHO, BARCODE?, GET2DID?, SETSITEMAP, BINMAP, SETTEMP, PAUSE, STOP_01,
  CHKSTATUS, SETSTARTMODE, ART, SETTESTTEMP, OVERDRIVE, RECONTACT
---

# SKILL: gpib-command-list

## 描述

HT9xxx GPIB Command List 完整知識庫（基於 `HT9xxx GPIB_Command_FullVersion_V12.13.884.docx`）。
當使用者詢問 GPIB 指令格式、BINON/BINOFF 排列、SRQ 流程、BIN Mapping、
溫度控制指令、ART/ATC 功能、2DID/Barcode 指令、Device Map、Remote Control 指令、
Qorvo/SIGURD 客製 GPIB 協定、SRQ Request Table 等問題時，應載入此 SKILL。

關鍵字：GPIB Command, BINON, BINOFF, FULLSITES, SRQ, ECHO, ECHOOK, ECHONG,
BARCODE?, GET2DID?, QRC?, GETBARCODENUMBER, SETSITEMAP, BINMAP, CHKSTATUS,
SETSTARTMODE, SETUP_, SETTEMP, SETSOAK, ASIF_TJ_EFUSED, ASIF_TJ_REQUEST,
TEST ARM?, TEMPARM?, GETNOWALLTEMP, FORCE?, TESTARMEP?, TESTARMPOS?,
PAUSE, STOP_01, ONECYCLE, AUTO_CLEAN, OVERDRIVE, RECONTACT, PICKLOAD,
PLACETOLOAD, TRAYFEED, ART, SETTESTTEMP, NEXTSTEP, READDIODE, READTEMP,
READDAQ, SENDERROR, DEVICETEMP_, SETTESTOFFSET_, SETCOOLINGVALUE_,
CHECKLIST, SGFTP_, NONDOUBLEBIN_, BINCOUNT_, SGOSBIN_, SGCONTFAIL,
SETTESTERID_, BINPOS_, SETAICCD_, SVID, ECID, RCMD, *IDN?, CHKMATCH?,
HANDLERID?, Version?, SETUPFILENAME_, BIN 0~254, 32BIN, GS16/GS32 BIN,
Advan T6577, Qorvo Protocol, SIGURD Protocol, 32-site, HT9045, HT9046

## 原始文件

- **路徑**：`d:\GPIB9045\.github\skills\gpib-command-list\references\HT9xxx GPIB_Command_FullVersion_V12.13.884.docx`
- **版本**：V12.13.884（最後更新：2026/01/14，RogerYang）
- **適用機台**：HT-9xxx 系列（HT9045, HT9046）

## 文件章節索引

### Standard GPIB Command

> 詳細規格請參閱：`d:\GPIB9045\.github\skills\gpib-command-list\references\standard-gpib-command.md`

### GPIB Command with 2DID

> 詳細規格請參閱：`d:\GPIB9045\.github\skills\gpib-command-list\references\standard-gpib-command.md`（章節：GPIB Command with 2DID）

| 指令 | 排列順序 | 說明 |
|------|---------|------|
| `BARCODE?` | 倒序（最右為 Site 1）| 查詢條碼，支援 HT-9xxx/7xxx/1xxx |
| `GET2DID?` | 正序（最左為 Site 1）| 查詢 2D ID，僅支援 HT-9xxx |
| `QRC?` | 正序（最左為 Site 1）| 查詢 QR Code，支援 HT-9xxx/7xxx/1xxx |
| `GETBARCODENUMBER` | — | 查詢條碼，帶 DataLength + Checksum |

### Additional GPIB Command / Site Mapping

- Site Mapping for GPIB Command
- GPIB Test Mode and Site Map（HT9045 & HT9046）

### Handler Information（第 1 章）

| 指令 | 說明 |
|------|------|
| `*IDN?` | Get Machine Type |
| `CHKMATCH?` / `CHKMACH?` | Get Machine Model |
| `HANDLERID?` / `HANDLER ID?` / `ID?` | Get Machine ID |
| `Version?` | Get Software Version |

### Recipe Control（第 2 章）

| 指令 | 說明 |
|------|------|
| `SETUPFILENAME?` | Get Setup File Name |
| `SETUPFILENAME_` | Set Setup File Name |

### Site Map Commands（第 3 章）

| 指令 | 說明 |
|------|------|
| `SETSITEMAP?` / `ASSIGN?` / `MAP?` | Get Handler Site Map |
| `SETSITEMAP_` | Set Handler Site Map |

### Bin Map and Yield Control（第 4 章）

| 指令 | 說明 |
|------|------|
| `START MODE?` | Get Start Mode |
| `TESTMODE?` | Get Bin Mode |
| `CHKSTATUS?` | Get Start Status / Start Mode |
| `SETSTARTMODE_` | Set Start Mode |
| `GETBINCATEGORY?` | Get Bin Category |
| `BINMAP?` | Get Bin Map |
| `BINMAP_` | Set Bin Map |
| `CHKSETUP?` | Get Setting of Bin Alarm |
| `SETUP_` | Set Bin Alarm |

### Temperature & Soak Time（第 5 章）

| 指令 | 說明 |
|------|------|
| `SETTEMP?` / `SETPOINT?` | Get Handler Temperature |
| `SETTEMP` | Set Handler Temperature |
| `SETSOAK?` / `SOAK?` | Get Handler Soak Time |
| `SETSOAK_` | Set Handler Soak Time |
| `ASIF_TJ_EFUSED` | Send EFUSED data from tester |
| `ASIF_TJ_REQUEST` | Send Tj data from ATC |

### Index Arm Information（第 6 章）

| 指令 | 說明 |
|------|------|
| `TEST ARM?` | Get Current Test Arm |
| `TEMPARM?` / `ACTUALTEMP?` | Get Temperature of Current Testing Arm |
| `GETNOWALLTEMP?` | Get Temperature of All Index Arms |
| `TMP?` | Get Temperature of Handler |
| `Contact Force?` / `FORCE?` | Get Contact Force of Current Testing Arm |
| `TESTARMEP?` | Get EP Value of Current Testing Arm |
| `TESTARMPOS?` | Get Encoder Position of Test Arm |

### Remote Control（第 7 章）

| 指令 | 說明 |
|------|------|
| `PAUSE` | 暫停 |
| `SETHANDLERDOPAUSE` | 暫停（第二種）|
| `PAUSE_01` | Pause and Alarm |
| `STOP_01` | Stop and Alarm |
| `ONECYCLE` | One Cycle |
| `AUTO_CLEAN` | Auto Clean |
| `OVERDRIVE` | Over Drive |
| `RECONTACT` | Recontact |

### Device Map Function（第 8 章）

| 指令 | 說明 |
|------|------|
| `PICKLOAD` | Pick Loader |
| `PLACETOLOAD` | Place to Loader |
| `RECONTACT` | Recontact |
| `TRAYFEED` | Tray Feed |

### ART Function（第 9 章）— Flow of ART & Command List

### ATC Control Command（第 10 章）

| 指令 | 說明 |
|------|------|
| `SETTESTTEMP` | Set ATC Target Temperature |
| `NEXTSTEP` | Switch ATC TC/TJ Temperature Control Mode |
| `READDIODE` | Read Current Diode Temperature |
| `READTEMP` | Read Current TC Temperature |
| `READDAQ` | Read Current TJ Temperature |
| `SENDERROR` | Tj Temperature Compare to Defined Value |

### ATC 6.0 Control Command（第 11 章）

| 指令 | 說明 |
|------|------|
| `DEVICETEMP_` | Set Tj Offset for ATC6.0 |
| `SETTESTOFFSET_` | Set Tcase Offset for ATC6.0 |
| `SETCOOLINGVALUE_` | Set Cooling Value for ATC6.0 |

### ASIF Function（第 12 章）

| 指令 | 說明 |
|------|------|
| `ASIF_TJ_EFUSED` | Send EFUSED data（LVTS cal offset data）|
| `ASIF_TJ_REQUEST` | Send Tj data（all LVTS sensors）|
| `ASIF_TJ_FB` | Send Tj data（all LVTS sensors）|
| `ASIF_TJ_EOT` | Send Tj data（all LVTS sensors）|

### Data Collection（第 13 章）

- Tester Send Software Bin to Handler

### SIGURD Control GPIB Command

| 指令 | 說明 |
|------|------|
| `CHECKLIST` | Check List |
| `SGFTP_STATUS` | Get FTP Automation Function State |
| `SGFTP_` | Set FTP Automation Function On/Off |
| `NONDOUBLEBIN_` | Set NONDOUBLEBIN to Check List |
| `BINCOUNT_` | Set Count parameter to Check List |
| `SGOSBIN_` | Set OS Bin to Check List |
| `SGCONTFAIL` | Set CONTFAIL parameter to Check List |
| `SETTESTERID?` | Get Tester ID |
| `SETTESTERID_` | Set Tester ID |
| `BINPOS_` | Bin Position |
| `SETAICCD_` | Set AI CCD |

### Tester Control GPIB Command（SECS/GEM-like）

| 指令 | 說明 |
|------|------|
| `SVID` | Query Data |
| `ECID` | Set Data |
| `RCMD` | Command |
| `CEID` | Event（尚未支援）|
| S5F1 Alarm Report（尚未支援）|

### SRQ Request Table

- 完整 SRQ 請求對照表（見原始文件第 53 頁）

### Qorvo GPIB Protocol

> 完整 Qorvo 指令規格請參閱：`d:\GPIB9045\.github\skills\gpib-qrovo\SKILL.md`
> 原始規格文件：`SPE-001712 Rev(A)_Gpib command.docx`（Qorvo GTS，Rev A，2018/09/20）

| 指令 | 說明 |
|------|------|
| `CONFIGURE,SRQ=[n]` | 設定 SRQ 啟用（Y/N，預設 Y）|
| `CONFIGURE,FULLSITES?=[n]` | 設定 FULLSITES 流程（Y/N，預設 Y）|
| `CONFIGURE,CONTACTOR=[n]` | FULLSITES 回應是否含 CONTACTOR 資訊（Y/N）|
| `FULLSITES?` | SRQ41(H) 後查詢各 Site IC 狀態 |
| `BINON:xx,xx,xx,xx;` | BIN 指令（最右 = Site 1），回應 ECHO:... |
| `BINOFF:xx,xx,xx,xx;` | BIN 指令（無 ECHO 回應）|
| `ECHOOK` | 確認 BINON ECHO 正確 / ESC 為空 |
| `ECHONG` | BINON ECHO 不符 / ESC 非空，重試或 Flush |
| `QRM?` | Session 開始時查詢 2D Barcode 支援 |
| `QRC?` | 查詢各 Site 2D Barcode 結果 |
| `PAUSE` | 暫停 Handler（下一道必須是 RESUME）|
| `RESUME` | 恢復 Handler（僅在 PAUSE 後）|
| `REQUEST,CHECKEMPTY` | Tester 要求 Handler 執行 ESC |

## 版本修訂歷程

| 版本 | 日期 | 負責人 |
|---------|------------|---------|
| V12.06.676 | 2020/10/27 | Steven |
| V12.09.715 | 2022/01/12 | Sam |
| V12.10.727 | 2023/02/08 | Steven |
| V12.11.802 | 2023/11/03 | Jimmychiu |
| V12.11.843 | 2024/09/24 | Steven |
| V12.13.883 | 2026/01/14 | RogerYang |

---

## 搬移說明（St02 20260926）

本 skill 是從 `D:\GPIB9045\.github\skills\gpib-command-list` 複製進 `D:\HT9045\.claude\skills` 的（原處保留）。
下列二進位檔（客戶／廠商文件、手冊、截圖）**沒有一起搬進版控**：main 會同步到 GitHub。要看原檔請到原處：

- `references/Barcode.png`
- `references/GET2DID.png`
- `references/HT9xxx GPIB_Command_FullVersion_V12.13.884.docx`
- `references/QRC.png`
- `references/Standard GPIB Command.png`
