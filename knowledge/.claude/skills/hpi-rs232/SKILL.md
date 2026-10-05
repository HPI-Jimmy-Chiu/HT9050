---
name: hpi-rs232
description: >
  HT-9xxx Handler 與 Tester 之間的 RS-232 通訊主題 skill（併入 rs232-standard-interface、rs232-ttl-communication，舊內容一字未刪）。
  兩條線：(1) 標準 RS232 指令介面（RS232Standard 程式，規格 HT9xxx RS232_Interface V12.11.843）：[STX]Command[ETX]、
  COM Port 9600/7/1/Even、RD/TD/SG/RTS/CTS 接線、BA/CE/CF/CK/CN/CA/CB/CD/CH/CI、CZ status?/testerbin?/sitemap?/jam?/soaktime?/all masstemp?/doublecontact?、
  BARCODE?/GET2DID?、CE→BA 交握、Handler Status 位元、Site Mapping、溫度回傳、Alarm Code、Soak Time、
  測試逾時 Skip（BA without CE）與 Closed Site Bin 例外；(2) TTL 通訊板（TTL Communication with IPC by RS-232，VER.07082201）：
  115200/N/8/1、SOF/W-R/CMD/DATA/CRC/EOF 帧、CRC16 Modbus、INIT 開機初始化、SOT/DATA/EOT/DUT Active Logic、
  Bit Bit／Bit Binary 模式、SOTS→RBIN 交握、CSOT 重發、FCRC/ENBU/FIRM、Err 回傳碼、光耦隔離、4/8 Site、燈號、Bootloader 韌體更新。
  關鍵字：RS232, RS-232, RS232Standard, TTL, TTL Board, Standard Command, STX ETX, COM Port, 9600, 115200,
  CRC16, Modbus CRC, INIT, SOT, EOT, DUT, BA, CE, CF, CK, CN, CZ, BARCODE?, GET2DID?, Handler Status,
  Site Mapping, Alarm Code, Soak Time, BA without CE, Closed Site Bin, Bit Bit Mode, Bit Binary Mode,
  RBIN, CSOT, optocoupler, 光耦隔離, uSocketServerClient, TTL_Rev585, HT9045, HT9046
---

# hpi-rs232：Handler 與 Tester 的 RS-232 通訊

## 這是什麼

HT-9xxx Handler 跟 Tester 之間走 RS-232 有兩條線，各有一份廠內規格。兩支舊 skill 原封不動搬進 `references/`，本檔只做路由與安全提醒。

| 線 | 誰跟誰講話 | 序列埠參數 | 規格 |
|---|---|---|---|
| 標準 RS232 指令介面 | Tester 用 `[STX]Command[ETX]` 文字指令，經 RS232 程式（RS232Standard）跟 Handler 溝通 | 9600／7／1／Even | HT9xxx RS232_Interface V12.11.843（2024/09/18，Steven） |
| TTL 通訊板 | IPC 用 RS-232 指令設定 TTL 板，TTL 板對 Tester 送 SOT／DUT、收 BIN（TTL 介面光耦隔離） | 115200／N／8／1 | TTL Communication with IPC by RS-232，VER.07082201（2018/08/22） |

## 路由表

| 問題或機型 | 先讀 | 要細節再讀 |
|---|---|---|
| 標準 RS232：COM Port、接線、`[STX]…[ETX]`、指令清單、CE→BA 交握 | `.claude/skills/hpi-rs232/references/common.md` §1.1～1.4 | `.claude/skills/hpi-rs232/references/rs232-standard-interface/rs232-standard-interface.md` |
| Handler Status 位元（`CK`／`CZ status?`）、`514` 怎麼解 | `.claude/skills/hpi-rs232/references/common.md` §1.5 | `.claude/skills/hpi-rs232/references/rs232-standard-interface/references/rs232-interface-extracted.md`（.doc 原文，含 MES 告警碼） |
| 各 Test Mode 的溫度、Site Map、`CN` 排列 | `.claude/skills/hpi-rs232/references/common.md` §1.7 | 同上原文（含 Tri 1x3／Dual 2x1／Quad 2x2 溫度格式、`CN` 2x4 範例） |
| `CZ jam?`／`soaktime?`／`doublecontact?`、`BARCODE?`／`GET2DID?`、`CZ testerbin?` | `.claude/skills/hpi-rs232/references/common.md` §1.6、§1.8 | `.claude/skills/hpi-rs232/references/rs232-standard-interface/rs232-standard-interface.md` |
| 測試逾時按 Skip、`BA without CE`、`Closed Site have Bin` | 本檔「絕不能漏」＋ `.claude/skills/hpi-rs232/references/common.md` §1.9 | 同上 |
| TTL 板：硬體、帧格式、CRC、Err 碼、INIT、指令、模式 | `.claude/skills/hpi-rs232/references/common.md` §2.1～2.6 | `.claude/skills/hpi-rs232/references/rs232-ttl-communication/rs232-ttl-communication.md` |
| TTL 板：SOTS→RBIN 範例、FCRC／ENBU／FIRM、跳線燈號、Bootloader、韌體歷史 | `.claude/skills/hpi-rs232/references/common.md` §2.5、§2.7～2.10 | `.claude/skills/hpi-rs232/references/rs232-ttl-communication/references/ttl-communication-extracted.md`（.pdf 原文） |
| HT9045／HT9046／HT9046LS／HT9050 等機型差異 | 同 common（舊 skill 沒有機型分流；規格寫「適用 HT-9xxx Series」） | HT9050 沒有 RS232 資料來源，不要自己推 |
| 客戶專屬行為 | `.claude/skills/hpi-rs232/references/customers.md`（目前沒有） | — |
| V906 移植樹裡的 RS232／TTL 程式與 P8 機邊上機步驟 | `hpi-gpib`：`.claude/skills/hpi-gpib/references/ht9045-gpib-bridge/references/gb-p8-bringup-plan.md` §6 RS232（Standard）、§7 TTL 板（DIO） | — |
| GPIB_RS232 三介面整合（`g_iTestType`、TTL_MODE／RS232_MODE、COM port 開關） | `hpi-gpib`：`.claude/skills/hpi-gpib/references/gpib-rs232-merge/gpib-rs232-merge.md` | — |
| 原始 .doc／.pdf | `.claude/skills/hpi-rs232/references/common.md` §4（在 `D:\RS232Standard\.github\skills\`，不進版控） | — |
| Panasonic 伺服驅動器的 RS232（不是這條線） | `.claude/skills/ht9045-motor-control/references/panasonic-rs232/overview.md` | — |

## 絕不能漏的安全事項

1. **測試逾時按 Skip 之後的 BA**：Skip 會讓 Handler 把 Socket 內所有 IC 設成 Error Bin 並送 `Halt test and Skip`，`Has CE` 變 FALSE；Tester 之後再送 BA，RS232 程式送 `BA without CE Error`，Handler 告警並把 Socket 內所有 IC 設成 Error Bin。（`rs232-standard-interface.md`「例外錯誤處理」）
2. **關閉的 Site 收到 Bin**：「Handler 只下壓 1 顆 IC，但 Tester 回傳 2 個 Bin」，RS232 程式報 `Closed Site have Bin Error`，Handler 告警並把 Socket 內所有 IC 設成 Error Bin。（同上）
3. **COM Port**：「所有參數需與 Tester COM Port 設定一致」。（`rs232-standard-interface.md`「COM Port 設定」）
4. **`BARCODE?` 是倒序**：「最右為 Site 1」；未用 Site 或功能關閉回 `0`，讀取錯誤回 `ERROR`。順序弄反，2DID 就會對到錯的 Site。（`rs232-standard-interface.md`「BARCODE? / GET2DID?」）
5. **TTL 板每次上電都要 INIT**：「未完成初始化時無法正常執行 TTL 功能」；沒收到 INIT 之前每秒送一次 `@Runing'CRC'#`。（`rs232-ttl-communication.md`「INIT 開機初始化指令」「錯誤回傳格式」）
6. **前一流程沒結束不可重送 SOT**：會回 `@ErrSOT`；要重發，先送 `CSOT`（或 `SOTS` 全 0）強制結束前一流程。（`rs232-ttl-communication.md` CSOT 列；`ttl-communication-extracted.md` 狀況 D）
7. **BIN 等待逾時設 0 等於無限等待**（INIT DATA[42～47]、`OTME`）。（`rs232-ttl-communication.md`）
8. **韌體更新**：「韌體更新過程中請勿拔除 RS-232 及關閉電源」；誤入更新流程時，只要重開 TTL 電源就能恢復。（`ttl-communication-extracted.md`「Bootloader 韌體更新教學」）
9. **手動 SOT 按鈕**「需指令解鎖後使用」（ENBU），沒解鎖時按了不會動作。（`ttl-communication-extracted.md`）
10. **原始 .doc／.pdf 不進版控**：main 會同步到 GitHub，客戶／廠商文件不可帶進來。（兩份舊 skill 的「搬移說明（St02 20260926）」）

## 完整關鍵字

舊 skill 兩份的 frontmatter 與內文關鍵字聯集（去重，100 個）：

RS232, RS-232, Standard Command, BA, CE, CF, CZ, CN, CA, CB, CD, CH, CI, BARCODE?, GET2DID?, COM Port, STX ETX, Handler Status, Bin Data, Site Mapping, Alarm Code, Soak Time, 9600, Number of Sites, Start of Test, CK, CZ status?, CZ testerbin?, CZ sitemap?, CZ jam?, CZ soaktime?, CZ all masstemp?, CZ doublecontact?, CZ id?, CZ which?, Baud Rate 9600, Sorting Count, Index Heater Temperature, 2DID, Barcode, Multi Double Contact, 測試逾時, Test Timeout, BA without CE, Closed Site Bin, HT-9xxx, HT9045, HT9046, RS232Standard, TTL, TTL Board, TTL Communication, CRC16, Modbus CRC, INIT, SOT, EOT, DUT, Bit Bit Mode, Bit Binary Mode, RBIN, PWON, OTME, CSOT, VERS, optocoupler, 光耦隔離, 4 Site, 8 Site, TTL by RS-232, IPC, SOF, EOF, SOT Active Logic, DATA Active Logic, EOT Active Logic, DUT Active Logic, 5bit, 10bit, MODE, SOTL, DATL, EOTL, DUTL, SOTT, DUTT, SOTS, DUTS, TS+5V, 開機初始化, ErrSOF, ErrCMD, ErrCRC, ErrEOF, ErrSOT, ErrDAT, ErrSIT, ErrBIN, uSocketServerClient, TTL_Rev585

另外補充（舊 skill 的 references 原文有，舊關鍵字沒列）：115200, CZ jamnumber?, Has CE, Halt test and Skip, FCRC, ENBU, FIRM, RFIRM, RSEN, Runing, Bootloader, Y modem, GETSTART, MES1021, Error Bin。
