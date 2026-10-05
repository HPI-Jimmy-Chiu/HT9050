# hpi-rs232 通用行為（common）

兩支舊 skill 寫的都是協定與硬體規格，沒有任何依機型（`MachineTypeChoice`、`Type_HT9045`／`Type_HT9050` 等機型旗標）分流的段落，所以內容全部放在本檔，沒有 `ht9045.md`／`ht9050.md`。規格標示「適用 HT-9xxx Series（HT9045, HT9046 等）」。§1.7 依 Test Mode 分流，指的是測試站的排列（1x4、2x4 等），不是機型分流。

每段後面〔 〕裡是來源代號：

| 代號 | 檔案 |
|---|---|
| STD | `.claude/skills/hpi-rs232/references/rs232-standard-interface/rs232-standard-interface.md`（舊 rs232-standard-interface 的 SKILL.md，整理版） |
| STD-原文 | `.claude/skills/hpi-rs232/references/rs232-standard-interface/references/rs232-interface-extracted.md`（V12.11.843 .doc 抽出的文字） |
| TTL | `.claude/skills/hpi-rs232/references/rs232-ttl-communication/rs232-ttl-communication.md`（舊 rs232-ttl-communication 的 SKILL.md，整理版） |
| TTL-原文 | `.claude/skills/hpi-rs232/references/rs232-ttl-communication/references/ttl-communication-extracted.md`（VER.07082201 .pdf 抽出的文字） |

## 1. 標準 RS232 指令介面（RS232Standard）

### 1.1 COM Port 與接線

Baud Rate 9600、Bit Length 7、Stop Bit 1、Parity Even；所有參數都要跟 Tester 的 COM Port 設定一致。〔STD「COM Port 設定」；STD-原文「COM Port Setting」〕

接線是同名腳位對接，Handler COM 與 Tester COM 兩邊同腳：RD Pin 2、TD Pin 3、SG Pin 5、RTS Pin 7、CTS Pin 8。〔STD「Wiring（接線對應）」；STD-原文「Wiring」〕

### 1.2 格式

每筆都是 `[STX] + Command + [ETX]`。沒有資料可回時回 NULL 字串（"no data"）；收到未定義的指令也回 NULL。〔STD「通訊協定格式」與「指令清單」CA 列；STD-原文「Command List」CA 列〕

### 1.3 指令清單

共 22 個：BA、BARCODE?、CA、CB、CD、CE、CF、CH、CI、CK、CN、CZ all masstemp?、CZ doublecontact?、CZ id?、CZ jam?、CZ jamnumber?、CZ sitemap?、CZ soaktime?、CZ status?、CZ testerbin?、CZ which?、GET2DID?。功能與說明見 STD「指令清單」。歸納幾點：

- `CA`（Inhibits）、`CH`（Sleeves Full）、`CI`（Input Empty）一律回 NULL。
- 同義的指令：`CB` 與 `CZ all masstemp?` 都是 Index 接觸 Socket 的溫度；`CD` 與 `CZ which?` 都回機台 ID；`CK` 與 `CZ status?` 都回 16-bit Handler 狀態；`BARCODE?` 與 `GET2DID?` 都回每顆 IC 的 2DID。
- `CZ id?` 回 Handler 型號。
- `CZ jamnumber?` 在規格裡只有指令名稱，功能與說明欄都是空的。
- `CF`（Number of Sites）回最大測試站數 1～8，格式 `[STX]<SiteCount>[ETX]`。

〔STD「指令清單」「CF – Number of Sites」；STD-原文「Command List」「Number of Sites (CF)」〕

### 1.4 一次測試的交握：CE → BA

Tester 送 `CE` 問可以測哪些 Site：Handler 還不需要測時回 `[STX]NULL[ETX]`；準備好時回 Site 號碼，例如 `[STX]1,2,3,4[ETX]`。〔STD「CE – Start of Test」；STD-原文「Start of Test (CE)」〕

測完 Tester 送 `BA` 回分類結果：`[STX]BA<Site#1>,<DeviceID#1>,<Bin#1>;<Site#2>,<DeviceID#2>,<Bin#2>[ETX]`，每個 Site 一組、組與組之間以 `;` 分隔。〔STD「BA – Bin Data」；STD-原文「Bin Data (BA)」〕

RS232 程式在「Handler 下壓 Socket、Tester 送 CE」時把 `Has CE` flag 設成 TRUE，原文說這代表 SOT（start of test）；Tester 之後送的 BA 被當成 EOT。§1.9 兩個例外都靠這個 flag 判斷。〔STD「例外錯誤處理」；STD-原文「Exception Error」〕

### 1.5 Handler Status（`CK`／`CZ status?`）

回 `[STX]integer number[ETX]`，每個 bit 代表一種狀態，位元表（Bit 0～16）見 STD「CK / CZ status? – Handler Status（16-bit）」。原文比 STD 多出的細節：

- Bit 1 Output Full 對應的告警碼：MES1021（Empty tray 上沒盤）、MES1421（Color tray 上沒盤）、MES1120／MES1220／MES1320（Auto 1／2／3 tray unloader 疊滿）、MES1720（Fix tray 1 裝滿）、MES1820（Fix tray 2 裝滿）、MES1920（原文同樣寫「Fix tray 2 is filled with devices」，照抄未改）。
- Bit 4 Diagnostics：找 Home 中、做 contact test 或 auto height、在 Teach 畫面、在 Motor test 畫面、Pause 中且停在任何可設定 Handler 參數的畫面。
- Bit 5 Index Action Without Testing：Handler 在跑但不在測試流程，例如找 Home、Index Check、Tray Feed、等 guard band 或 soak time（STD 只寫等 Guard Band）。
- Bit 7 Guard Band：等溫度升到設定值；Bit 10 Soak：等 soak time。兩者在 Ambient 模式下永遠是 0。
- Bit 8 Jam：Handler 告警，且該告警碼把 bit 8 設成 1 時才是 1。
- Bit 0（Reboot）、Bit 6、Bit 13（OK）、Bit 16 永遠是 0；Bit 6 與 Bit 16 原文名稱寫 "Reversed"。

〔STD-原文「Handler Status (CZ status?)」〕

原文有兩處對不起來，照抄未改，待原作者（Steven）確認：

- 標題寫 16-bit，表格卻列了 Bit 0～16 共 17 列。
- 範例「`514` means output full (bit-2) and handler stopped (bit-9)」：依位元表計算，514＝2^9＋2^1＝Bit 9（Handler Stop）＋Bit 1（Output Full，值 2）。原文寫的 "bit-2" 在位元表裡是 Input Empty（值 4），兩者不符。

〔STD「CK / CZ status?」範例；STD-原文「Handler Status (CZ status?)」〕

### 1.6 Sorting Count（`CZ testerbin?`）

`[STX]Bin#1-Count#1,Bin#2-Count#2,...,U-0[ETX]`，每個 Bin 一組數量；數量 0 的 Bin 不會出現。原文的範例情境是 Bin1 有 10 顆、Bin2 有 12 顆。〔STD「CZ testerbin? – Sorting Count」；STD-原文「Sorting Count (CZ testerbin?)」〕

### 1.7 依 Test Mode 排列：溫度、Site Map、CN

這裡分流的是 Test Mode 的站點排列，不是機型。

- `CZ all masstemp?`：每個 Site 一個溫度，以空白分隔，例如 `[STX]Site1Temp Site2Temp[ETX]`；Site 關閉時該格是 NULL。原文表格列了 10 種 Test Mode：Single、Dual 1x2、Tri 1x3、Quad 1x4、Dual 2x1、Quad 2x2、6 Site 2x3、Octal 2x4、12 Site 2x6、16 Site 2x8。STD 的表格少了 Tri 1x3、Dual 2x1、Quad 2x2 三種，格式要看原文。〔STD「CZ all masstemp? – Index Heater Temperature」；STD-原文「Index Heater Temperature (CZ all masstemp?)」〕
- `CZ sitemap?`：以逗號分隔，例如 `[STX]Site1,Site2[ETX]`；關閉的 Site 回 `0` 或 `-1`；同樣是上述 10 種 Test Mode。〔STD「CZ sitemap? – Site Mapping」；STD-原文「Site Mapping (CZ sitemap?)」〕
- 站號排列：原文 sitemap 表裡排成兩排的模式是上排奇數、下排偶數（例 2x4：1 3 5 7／2 4 6 8；2x2：1 3／2 4），跟 `CN` 的 2x4、2x8 圖一致；masstemp 表的 Site Map 欄則是依序排（例 2x4：1 2 3 4／5 6 7 8；2x2：1 2／3 4）。原文是從 .doc 表格抽出的文字，表格框線已經不見，有疑問請看原 .doc（§4）。〔STD-原文「Index Heater Temperature」「Site Mapping」「Site Map (CN)」；STD「CN – Site Map」〕
- `CN`：`[xx,yy,z][x2,yy2,z2]…`，xx＝Site 號、yy＝Channel 號（沒設定時是 `--`）、最後一欄 `00`＝停用、`01`＝啟用（STD 寫 `z`、原文寫 `zz`，值都是兩位）。範例 a（1x4）：`[STX][01,01,01][02,03,00][03,02,01][04,--,01][ETX]`。範例 b（2x4，只有原文有）：`[STX][01,01,01][02,02,01][03,04,00][04,03,01][05,05,01][06,06,01][07,08,01][08,07,01][ETX]`。〔STD「CN – Site Map（含 Channel 對應）」；STD-原文「Site Map (CN)」〕

### 1.8 其他查詢

- `CZ jam?`：`[STX]JamCode[ETX]`，沒有 Jam 時回 `0`。
- `CZ soaktime?`：`[STX]SoakTime[ETX]`，溫度模式是 Ambient 時回 `NONE`。
- `CZ doublecontact?`：`[STX]number[ETX]`；`0`＝功能關閉、`1`＝接觸 2 次、`2`＝3 次、`3`＝4 次。
- `BARCODE?`：`[STX]BARCODE:Site32_2D,Site31_2D,…,Site2_2D,Site1_2D[ETX]`，倒序（最右邊是 Site 1）；未使用的 Site 或功能關閉時回 `0`，讀取錯誤回 `ERROR`。`GET2DID?` 在原文只出現在指令清單，沒有另外列格式。

〔STD「CZ jam?」「CZ soaktime?」「CZ doublecontact?」「BARCODE? / GET2DID?」；STD-原文 5.8、5.9、5.11、5.12〕

### 1.9 例外處理（SKILL.md「絕不能漏」有摘要）

- **測試逾時後按 Skip**：Handler 觸發 test time out 告警、使用者按 Skip 後，Handler 把 Socket 內所有 IC 設成 Error Bin，送 `Halt test and Skip` 給 RS232 程式，`Has CE` 改成 FALSE。原文另外補了一句「Handler might change arm contact to socket」（Handler 之後可能換手臂去下壓 Socket）。Tester 測完若仍送 BA（EOT），RS232 程式因為 flag 是 FALSE，送 `BA without CE Error` 給 Handler，Handler 告警並把 Socket 內所有 IC 設成 Error Bin。
- **關閉的 Site 收到 Bin**：例如 Handler 只下壓 1 顆 IC，Tester 卻回了 2 個 Bin，RS232 程式報 `Closed Site have Bin Error`，Handler 告警並把 Socket 內所有 IC 設成 Error Bin。

〔STD「例外錯誤處理」；STD-原文「Exception Error」〕

### 1.10 規格版本

V7.00.550.0（2018/03/14）、V7.00.715.0（2018/03/15）、V12.01.622.0（2019/06/27）、V12.04.657.0（2020/07/09）、V12.06.700.0（2021/09/01）、V12.11.843.0（2024/09/18），工程師都是 Steven。舊 skill 從 V12.11.843 轉換而來，最後驗證日期 2026-04-09。〔STD「版本歷史（Change List）」「驗證狀態」；STD-原文「Change List」〕

## 2. TTL 通訊板（TTL Communication with IPC by RS-232）

### 2.1 硬體

- IPC 與 TTL 板之間：RS-232，115200、N、8、1。
- TTL 介面：4 Site（10 bit）／8 Site（5 bit）；SOT、DATA、DUT、EOT 的 Active Logic 可以分別設定；以光耦合器（optocoupler）隔離。

〔TTL「硬體規格」；TTL-原文「HARDWARE」〕

### 2.2 帧格式與 CRC

- SET 或 ASK（IPC → TTL 板）：`SOF | W/R | CMD | DATA | CRC | EOF`；ACK（TTL 板 → IPC）：`SOF | CMD | DATA | CRC | EOF`。
- 欄位：SOF 是 `@`（1 byte）；W/R 是 `W`（0x57）或 `R`（0x52）（1 byte）；CMD 4 bytes ASCII；DATA 8 bytes ASCII；CRC 2 bytes，CRC16 Modbus；EOF 是 `#`（1 byte）。
- CRC 的計算範圍從 SOF 開始，到 DATA 結束（含 SOF、W/R、CMD、DATA）。

〔TTL「帧格式（Frame Format）」；TTL-原文「SETTING FRAME FORMAT」〕

### 2.3 TTL 板主動回報與錯誤碼

格式是 `@ErrXXX'CRC'#`：

- 開機時送 `@Runing'CRC'#`，沒收到 INIT 之前每秒送一次。
- `ErrSOF`、`ErrCMD`、`ErrCRC`、`ErrEOF`：對應欄位錯誤。
- `ErrSOT`：前一流程還沒結束，又收到 SOT 發送要求。
- `ErrDAT`：DATA 錯誤。
- `ErrSIT`：未使用的 Site 收到 BIN。
- `ErrBIN`：Bit Bit 模式下，單一 Site 收到兩個 BIN。

〔TTL「錯誤回傳格式」；TTL-原文第 1～9 點〕

### 2.4 INIT 開機初始化

TTL 板每次上電都要先送 INIT；沒送或內容錯誤，TTL 功能都無法正常執行。

| 位置 | 內容 |
|---|---|
| SOF／R/W／Command | `@`／`W`／`INIT` |
| DATA[0] | TS+5V：On `0x31`、Off `0x30` |
| DATA[1] | MODE：5 BitBit `0x30`、10 BitBit `0x31`、5 BitBinary `0x32` |
| DATA[2～9] | SOT Active Logic，依序對應 Site1～8；Low `0x30`、High `0x31` |
| DATA[10～17] | DATA Active Logic（同上） |
| DATA[18～25] | EOT Active Logic（同上） |
| DATA[26～33] | DUT Active Logic（同上） |
| DATA[34～37] | SOT 發送寬度，單位 ms，上限 1,000 ms；例：528 ms 依序送 `0x30 0x35 0x32 0x38` |
| DATA[38～41] | DUT 發送寬度，單位與上限同 SOT，例子也相同 |
| DATA[42～47] | BIN 等待逾時上限，單位 ms，上限 500,000 ms，從第一個收到 BIN 的 Site 開始計時；設 0 表示無限等待 BIN（依序送 6 個 `0x30`） |
| CRC[48～49] | CRC16 Modbus，從 SOF 算到 DATA 結束 |
| EOF | `#` |

INIT 的 DATA 是 DATA[0]～[47] 共 48 碼，比其他指令的 8 bytes DATA 長。內容確認正確後，TTL 板 echo 收到的內容，並送韌體版本號 `@VERS 年月日次序'CRC'#`。〔TTL「INIT 開機初始化指令」；TTL-原文 INIT 段〕

### 2.5 其他指令

`MODE`、`SOTL`、`DATL`、`EOTL`、`DUTL`、`SOTT`、`DUTT`、`SOTS`、`DUTS`、`RBIN`、`PWON`、`OTME`、`CSOT`、`VERS` 這 14 個指令的 DATA 都是 8 bytes，逐 byte 定義見 TTL「其他指令清單」。歸納：

- Active Logic 四個（`SOTL`／`DATL`／`EOTL`／`DUTL`）：DATA[0～7] 對應 Site1～8，High `0x31`、Low `0x30`。
- `SOTS`（送 SOT 給 Tester）、`DUTS`（送 DUT 給 Tester）：DATA[0～7] 對應 Site1～8，`0x30` 等待、`0x31` 發送。
- 時間類（`SOTT`／`DUTT`／`OTME`）都是 ASCII、單位 ms、靠右對齊。`SOTT`／`DUTT` 上限 1,000 ms（例：330 ms 送 `0x33 0x33 0x30`）；`OTME` 上限 500 s（例：5000 ms 送 `0x35 0x30 0x30 0x30`），設 0 表示無限等待。
- `RBIN`：8 個 byte 依序是 Site1～8 的 BIN，`0x00` 無 BIN、`0x01`～`0x0A` 是 bin1～bin10；所有 Site 收完才回傳。
- `PWON`（電源 +TS5V）：On `0x31`、Off `0x30`，Byte2～8 填 `0x00`；`MODE` 的 Byte2～8 也填 `0x00`。
- `CSOT`（Clear SOT）：DATA 全為 0 時等同清除指令。BIN 沒回、又不想再等時，要先清除才能重新送 SOT。
- `VERS`：例如 `07022701`，07＝民國 107 年、02＝2 月、27＝27 號、01＝第 01 版。

〔TTL「其他指令清單（Other Command List）」；TTL-原文「Other Command List」〕

舊 SKILL.md 沒抄到、只在原文出現的三個指令：

- `FCRC`（Fail crc）：Handler 收到 BIN 後驗 CRC 發現錯誤時送出，TTL 板會再送一次 BIN 別，回 `RSEN:bin……`（韌體 07041301 新增）。
- `ENBU`（Enable Button）：`0x00` disable、`0x01` enable；板上的手動 SOT 按鈕要先用它解鎖，否則按鈕沒有作用（07082201 新增）。
- `FIRM`（Firm Ware upgrade）：進入韌體燒錄流程；Bootloader 教學用的是 `@RFIRM00000000'CRC'#`，W/R 段是 `R`，History 稱為 RFIRM 指令（07082201 新增）。
- 抽出文字裡另外有「READ ONLY」與「通訊格式中的 W/R 段需使用 R」兩格，表格已經打散，無法確定屬於哪個指令，以原 .pdf 為準。

〔TTL-原文「Other Command List」「History」「Bootloader 韌體更新教學」〕

### 2.6 工作模式

| MODE 碼 | 模式 | 最多 Site | 說明 |
|---|---|---|---|
| `0x30` | 5 Bit Bit | 8 | 每個 Site 獨立 1 bit 信號 |
| `0x31` | 10 Bit Bit | 4 | 每個 Site 用 10 bit 信號（較高解析度） |
| `0x32` | 5 Bit Binary | 8 | BIN 結果用 Binary 編碼 |

〔TTL「工作模式說明」〕

板上「作動模式燈號」的編號順序跟 MODE 碼不同：Mode1＝5 bit bit、Mode2＝5 bit binary、Mode3＝10 bit bit、Mode4＝保留。看燈號時不要直接拿 MODE 碼去對。〔TTL-原文「通訊式 TTL 板燈號說明」〕

### 2.7 交握範例（原文狀況 A～E）

前提：已設定 mode 5bitbit，測 Site 1～4。

- 起測：PC → 板 `@WSOTS11110000'CRC'#`，板回同樣內容。
- 狀況 A：Site 1～4 都收到 BIN（bin 1、5、2、3），板回 `@RBIN"0x01 0x05 0x02 0x03 0x00 0x00 0x00 0x00"CRC#`。
- 狀況 B：只有 Site 3 沒有 BIN，且到了 TTL 板的逾時設定，板回 `@RBIN"0x01 0x05 0x00 0x03 0x00 0x00 0x00 0x00"CRC#`。
- 狀況 C：PC 一直等不到 BIN，可以查詢目前已完成的 BIN：PC → 板 `@RRBIN00000000'CRC'#`，板回目前的 RBIN。
- 狀況 D：PC 一直等不到 BIN、想重發 SOT，要先用 CSOT 強制結束前一流程：`@WCSOT00000000'CRC'#`（板回同樣內容），再送 `@WSOTS11110000'CRC'#`。改送 `@WSOTS00000000'CRC'#` 的效果與 CSOT 相同。
- 狀況 E：Handler 收 BIN 後驗 CRC 發現錯誤，送 `@WFCRC00000000'CRC'#`，板回 `@RSEN11110000'CRC'#`，再送一次 BIN 別。

〔TTL-原文「範例舉例」〕

### 2.8 板子外觀、跳線、燈號

以下取自 .pdf 圖片標註的抽出文字，位置對應以原 .pdf 為準。

- 跳線與接頭：TTL 板啟動用 Jump（一般使用都需短路）、工程用接頭、RS-232（Baud rate 115200）、24Vdc、SOT 手動按鈕（需指令解鎖後使用，即 ENBU）、強制 5Vdc 供電、測試機 JUMP。
- Site 接頭的標示文字，照抽出順序：SITE 7-8 (5 bits)、SITE 4 (10bits)、SITE 5-8 (5 bits)、SITE 3-4 (10bits)、SITE 3-4 (5 bits)、SITE 2 (10bits)、SITE 1-4 (5 bits)、SITE 1-2 (10bits)。
- 燈號：Brink 運行狀態燈（閃爍）、RX 通訊接收、TX 通訊發送、5V／3.3V 內部電源、GETSTART（等待 BIN 別時恆亮）、EOT 訊號燈、BIN 訊號燈、DUT 訊號燈、SOT 訊號燈、作動模式燈號 Mode1～4（見 §2.6）。

〔TTL-原文「通訊式 TTL 板說明」「通訊式 TTL 板燈號說明」〕

### 2.9 Bootloader 韌體更新

先開超級終端機或 Tera Term 等支援 Y modem 傳輸協定的 console 程式，Baud rate 設 115200。

1. 進入韌體更新模式：方法 1，按住 SW8 再重開 TTL 板電源；方法 2，在正常、沒有生產時送 `@RFIRM00000000'CRC'#`。
2. 終端機顯示「Download image to the internal Flash」。
3. 用 Y MODEM 傳輸，選擇新韌體 `.bin` 檔。
4. 傳完會顯示「Start program execution......」，並自動執行 TTL 板功能。
5. 完成後，到測試機軟體的 RS-232 頁面確認韌體版本號已經變更。
6. 過程中有任何異常，重開 TTL 板後重做上述步驟。

注意：誤入韌體更新流程時，只要重開 TTL 電源就能恢復正常；更新過程中不可拔除 RS-232，也不可關閉電源。〔TTL-原文「Bootloader 韌體更新教學」〕

### 2.10 韌體與文件版本

- 07022701：新增韌體版本詢問指令，並在 INIT 後主動告知；新增 Bit Bit 模式下接收信號異常通知；新增未開 Site 收到 BIN 的異常通知；over time 改成「某 Site 收到 BIN 後開始計時」，用來監控各 Site 收 BIN 是否逾時，時間長度設 0 則不使用。
- 07041301：新增 FCRC 指令。
- 07082201：新增 ENBU（解鎖手動 SOT 按鈕）、RFIRM（進入韌體燒錄流程）、button 8 特殊功能（TTL 板開電時長按，進入韌體燒錄流程）、TTL 板外觀與燈號說明、Bootloader 更新韌體教學。
- 文件版本 VER.07082201＝民國 107 年 8 月 22 日第 1 版；韌體版本格式 `YYMMDDNN`（例：`07022701`＝民國 107 年 2 月 27 日第 1 版）。舊 skill 從這一版轉換而來，最後驗證日期 2026-04-09。

〔TTL「說明書版本與時間戳」「驗證狀態」；TTL-原文「History」〕

## 3. 機型分流

舊 skill 沒有任何依機型分流的內容，因此不開 `ht9045.md`／`ht9050.md`／`ht9046-ls.md`，SKILL.md 路由表寫「同 common」。HT9050 的 RS232 沒有資料來源（ST01 重整提案 §2.1 第 18 列的 HT9050 欄是「—」），不要自行推論。

## 4. 原始文件與檔案注意事項

- 原始規格是二進位檔，沒有進版控（main 會同步到 GitHub），要看原檔請到原處：`D:\RS232Standard\.github\skills\rs232-standard-interface\references\HT9xxx RS232_Interface_V12.11.843.doc`、`D:\RS232Standard\.github\skills\rs232-ttl-communication\references\20180822_TTL Communication with IPC by RS-232.pdf`。兩支舊 skill 是 St02 在 20260926 從 `D:\RS232Standard\.github\skills\` 複製進 `D:\HT9045\.claude\skills` 的，原處保留。〔STD、TTL「原始文件」「搬移說明（St02 20260926）」〕
- 兩份抽出原文含有 Word／PDF 抽字留下的控制字元，搬移時保持逐位元組不變：`rs232-interface-extracted.md` 有 563 個 0x07、20 個 0x01（檔頭第一個位元組就是）、9 個 0x0C，git 把它判定為非文字檔（`-text`）；`ttl-communication-extracted.md` 有 8 個 0x0C（換頁）。控制字元哨兵掃到這兩份屬於既有內容，不要用編輯器「整理」它們。
- 舊 TTL skill 的關鍵字列了 `uSocketServerClient`、`TTL_Rev585`、`RS232Standard`，但兩份舊文件內文都沒有說明它們是什麼。V906 移植樹裡 RS232／TTL 的實作見 SKILL.md 路由表指向 `hpi-gpib` 的兩列。
