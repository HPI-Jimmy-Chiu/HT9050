> 保存來源：`.claude/skills/ht9045-temperature/references/controllers/omron-ej1n.md`，main `84233b648`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../common.md)。

<!-- preserved-content:start -->
# Omron EJ1N（EJ1 模組式溫控器；附 EJ1G）（溫控器手冊參照）

> 手冊在 Steven01 本機 `E:\HT9045W_相關料件技術文件\溫控器\OMROM溫控器\`（資料夾名拼成 OMROM），不在 repo；其他電腦沒有 E: 這顆磁碟。頁碼寫「印刷頁（PDF 第幾頁）」；H142-E1-05 的 PDF 頁＝印刷頁＋26。

- golden V912＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（BCB6，Big5／cp950）
- 移植樹 V906＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`
- 只寫 `檔名:行號` 的都是 golden V912；`OmronEJ1N.cpp` 指 golden `EJ1N\OmronEJ1N.cpp`。

EJ1 的組成：TC4（4 通道）／TC2（2 通道）Basic Unit 負責量測與控制；EDU（End Unit）供 24 VDC 電源並提供通訊 Port A／Port B；一個 EDU 最多接 16 台 Basic Unit／HFU（H142 p.7，PDF 33）。HT9045 的 Index 加熱區用 4 台或 8 台 TC4。

## 1. HT9045 在哪裡用

### 1.1 設定鍵與值
| 鍵 | 值 | 位置（golden） |
|---|---|---|
| `[System] USE_16_HEATER` | `2`＝eht16HeaterEJ1N（`EJ1N_Count=4`，16 通道）；`3`＝eht32HeaterEJ1N（`EJ1N_Count=8`，32 通道）；其他值 `EJ1N_Count=0` | `MachineType.h:719-720`；`database.cpp:624-631` |
| `[TempCtrl] COM_PORT_OMRON` | EJ1N 專用 COM 埠（`HSys.sTempOmronComPort`，預設 COM7） | `database.cpp:523` |
| `[TempCtrl] HEATER_CTRL_TYPE` | 不能是 3（No Heater），否則 EJ1N 執行緒不啟動 | `main.cpp:10939-10942` |

這台機台 `D:\HT9045\system\Gerneral.ini`：`:19 USE_16_HEATER=2`、`:303 COM_PORT_OMRON=COM13`。`Omron 4Chanel溫控器連結測試步驟.pdf` 也寫 Comport 13。

### 1.2 COM 埠與通訊參數
- 開埠 `OmronEJ1N.cpp:328-355` `RS232_OpenDevice`：**38400 bps、7 data、Even、2 stop**（`ParityCheck=false`），元件是 `CommOmron`（TComm）。
- 埠檢查 `rs232.cpp:176-182`（`USE_16_HEATER` 是 2、3、6 時才檢查）。
- 38.4k／7／2／Even 剛好是 EDU **Port A 的固定值**；Port B 出廠是 9.6k（§3.1）。鴻勁的測試筆記（§2 的 `EJ1G\小記_研華 ADAM-4520…doc`）寫的也是接 EJ1C-EDUA-NFLK 的 PORT A ⇒ 推定 HT9045 接 Port A（從程式和筆記推的，配線沒看過）。

### 1.3 站號規則
- 送出：`%02X`＝Addr＋1，Addr＝0～`EJ1N_Count`−1，也就是站號 1～4（16 通道）或 1～8（32 通道）（`OmronEJ1N.cpp:992`、`:1081`、`:1193`、`:1305`、`:1386`、`:1477`、`:1661`）。
- 收回：`iChanel=atoi(asTStr.SubString(6,2))-1`（`:1733`），這裡用十進位解析。
- 每台 TC4 的 SW1／SW2 要設成 1～8，SW2 的 1、2 pin 都 OFF（測試步驟文件也寫 SW1＝1、2、3、4、SW2 OFF）。

### 1.4 程式位置（golden V912）
| 作用 | 位置 | 送出的 command text（不含 STX／ETX／BCC） |
|---|---|---|
| 讀型號 | `:976-1047` `DoGetComponent`，`:992` | `%02X` `00` `0` `0503` |
| 判 TC4／TC2 | `Main232` `:1811-1820` | 回覆含 `TC4A` → 4 通道、`TC2A` → 2 通道 |
| 讀 PV（TC4） | `:1050-1170` `DoGetPV`，`:1081` | `0104` + `C0 0000 00`、`C0 0100 00`、`C0 0200 00`、`C0 0300 00` |
| 讀狀態＋SP（TC4） | `:1172-1270` `DoGetSV`，`:1193` | `0104` + C0 `0001`／`0002`／`0101`／`0102`／`0201`／`0202`／`0301`／`0302`（各帶 `00`） |
| 寫 SV | `:1359-1437` `DoSetSV`，`:1386-1393` | `0113` + 每通道 `D4` `0i00` `00` `0000%04X`（i＝1～4，值＝畫面 SV×10） |
| 寫輸入種類 | `:1279-1347` `DoSetThermoType`，`:1305-1310` | `0113` + 每通道 `E0` `0i00` `00` `0000000%d`（1＝Pt100 −199.9～500.0，6＝K −20.0～500.0） |
| RUN／STOP | `:1634-1698` `DoRunStop`，`:1661`／`:1663` | `3005` `0B` `FF`（全部 STOP）／`3005` `0A` `FF`（全部 RUN） |
| AT | `:1448-1525` `DoAT`，`:1477`／`:1479`／`:1483` | `3005 0E FF`（40% AT）／`3005 0F FF`（100% AT）／`3005 10 FF`（取消） |
| 逐通道 AT | `:1527-1625`，`:1559` | `3005 0E %02d`／`3005 0F %02d`（通道 00～03） |
| RT | `:2041-2049` | 空函式，直接 return true |
| 送出 | `:365-391` `SendCommand` | `STX`＋text＋`ETX`＋BCC＋`\r\n`（`:376`） |
| 收資料 | `:2092-2136` `CommOmronReceiveData` | 把 byte 串成 `[STX]…[ETX](BCC)` 字串 `asTStr` |
| 解析 | `:1706-1876` `Main232` | 驗 BCC（`:1720-1730`）、End code `SubString(10,2)`、Response `SubString(16,4)`（`CheckEndAndResponseCode` `:412-469`）、MRC/SRC 在第 12 字（`:1701`）；長度 >70 當 SV 回覆（`:1826`） |
| PV 換算 | `:519-541` `GetPVValue` | 每個元素（類型 2＋資料 8）取資料**後 3 位** hex ÷10；Input error 時給 999 |
| 狀態換算 | `:543-578` `Get4in1SVValue` | Status 轉 32 字元：第 9 字 bit 23 AT、第 8 字 bit 24 RUN/STOP、第 18～20 字 bit 14～12 Alarm 3～1、第 26 字 bit 6 Input error；SP 也是取後 3 位 ÷10 |

執行緒與狀態機：
- `TOmronProcessThread::Execute`（`:55-62`）每 50 ms `Synchronize` → `Timer1Timer`（`:601-865`）；`TRS232Thread`（`:36-43`）每 5 ms → `Main232`。
- `Timer1Timer` 的優先序（`:644-713`）：ResetCom > GetComponent > SetSV（AT 中不寫）> RunStop > 手動送 > SetThermoType > AT > 逐通道 AT > RT；沒事時就輪流讀 PV → 讀 SV。
- 設輸入種類的流程是先 STOP、寫入、再恢復 RUN（`:718-748`）。
- 用 ATC 主動冷卻或 ATC 6.0 時整個不跑（`:614-620`）。
- 開機 `main.cpp:10939-10957`：按 ResetCOM、`bGetComponent`、`rgRunStop=1`（RUN）、`rgSensorType=0`（寫輸入種類 **1＝Pt100**）、`MyOmronThread->Resume()`。
- 逾時與重置：一般命令等 5 s（`iDelaySecc`，`:24`）；PV 等 10 s（`:1099`）、SV 等 4 s（`:1210`）。PV／SV 連 3 次失敗就 `bResetCom`，讀值全設 999（`:1130`、`:1242`）；SetSV 失敗 5 次重開 COM（`:1423`）；錯誤 frame 累計超過 10 次（`:1785`）或大約 10 秒沒收到東西（`:1847-1866`）也會重開。重開在 `DoResetCom`（`:1886-1934`）。

bthermo 端（golden `bthermo.cpp`）：
- `DoThermo` `:1207-1211` 每圈呼叫 `DoSetSVOfOmronEJ1N`（`:3501-3962`）。
- 算出每區溫度後寫進畫面 SV（`:3584-3618`），跟讀回的 `dSetValue` 比（`:3621-3812`），有不同就 `fOmron->btSetSV->Click()`（`:3848-3851`）。
- PV 用 `GetConvertTemp` 存進 `UN150Read`（`:3861-3962`）；還沒讀完型號或正在重開 COM 時，Aa1～Bd2 的讀值清成 0（`:3853-3860`）。
- 對應：第 1 台＝Aa1、Ba1、Ab1、Bb1；第 2 台＝Ac1、Bc1、Ad1、Bd1；第 3 台＝Aa2、Ba2、Ab2、Bb2；第 4 台＝Ac2、Bc2、Ad2、Bd2；第 5 台＝Ae1、Be1、Af1、Bf1；第 6 台＝Ag1、Bg1、Ah1、Bh1；第 7 台＝Ae2、Be2、Af2、Bf2；第 8 台＝Ag2、Bg2、Ah2、Bh2（只有 32 通道）。每台依序是 CH1～CH4；第 1～4 台 `:3903-3918`、第 5～8 台 `:3943-3958`（`dTempValue[台][CH]`）。
- 溫控 COM 埠那條輪詢（`DoThermoReal`）在 2／3 時跳過 Aa1～Bd2（3 時連 Ae1～Bh2），見 `bthermo.cpp:1331-1367`。
- 既有小錯：`:3635` `HeaterSVLog(tcAb1, Temp[tcBa1])`，只影響 log。

### 1.5 移植樹 V906 現況
- `EJ1N\` 只有 `MyOmronPanel`、`TextProcess`（`SetBCC` 在 `:530`）、`uDTME08Control`、`uModbusCommand`、`uSocketServerClient`。**沒有** `OmronEJ1N.cpp/.h/.dfm`、`OmronThermo.cpp`（DTME08 的外殼在 `forms\fDTME08.cpp`）。
- `bthermo.cpp:1306` 會呼叫 `DoSetSVOfOmronEJ1N`（`:3848`），但 G26a（`:3919-3924`）與 G26b（`:3972-4353`）都 `#if 0` ⇒ 不寫 SV、不讀 PV。
- `COM_PORT_OMRON` 仍會讀寫：`database.cpp:638`、`FileRW\HSys.gen.inc:3140`（開頁）、`:3718`（存檔）。

## 2. 手冊清單

路徑前綴都是 `E:\HT9045W_相關料件技術文件\溫控器\OMROM溫控器\`。

| 檔名 | 語言 | 版次／日期 | 內容 | 先讀哪幾頁 |
|---|---|---|---|---|
| `EJ1 MODULAR TEMPERATURE CONTROLLERS USER MANUAL(0212).pdf` | 英 | **H142-E1-05**（2012-02），314 頁 | 完整手冊（TC4／TC2／HFU／EDU、CompoWay/F、Modbus） | p.2-4 面板與 SW（PDF 28-30）；p.7 Port A／B（PDF 33）；p.18、23 EDU 端子與終端（PDF 44、49）；**Section 6 CompoWay/F p.171-192（PDF 197-218）**；p.235 規格（PDF 261）；p.239-257 參數表（PDF 265-283）；p.261 Status（PDF 287） |
| `EJ1 MODULAR TEMPERATURE CONTROLLERS USER MANUAL(0307).pdf` | 英 | H142-E1-02B（2007 年版），286 頁 | 舊版，沒有 V1.2 的 SW2 功能 | 只在比對舊機時看 |
| `EJ1 操作手册（中文）H142-CN5-01 (1206).pdf` | 簡中 | H142-CN5-01 | H142 中文版 | 文字層擷取不出來，本文以英文版為準 |
| `EJ1 MODULAR TEMPARATURE CONTROLLER DATASHEET(NEW_0614).pdf` | 英 | CSM_EJ1_DS_E_9_1，24 頁 | 型錄規格 | p.3 EEPROM 10 萬次；p.4 通訊規格表（Port A 固定 38.4k 7E2） |
| `EJ1 MODULAR TEMPARATURE CONTROLLER DATASHEET(NEW_0908).pdf` | 英 | 舊版，27 頁 | 同上 | 同上 |
| `EJ1N Data Sheet.pdf` | 中 | 16 頁 | EJ1N 型號（TC4A-QQ、TC4B-QQ、TC2A-QNHB、TC2B-QNHB、EDU、E58-CIFQ1） | 中文字多半擷取不出來 |
| `EJ1 中文目錄.pdf`、`SGTD-045A-1 EJ1 (150).pdf` | 中 | 8 頁、27 頁 | 中文型錄／簡介 | 文字層大多擷取不出來 |
| `Omron 4Chanel溫控器連結測試步驟.pdf` | 中 | 2 頁（2013-02） | 鴻勁的連線步驟：SW1＝1、2、3、4、SW2 OFF；ADAM-4520「(38400,N,2)」、jump 7 ON；CX-Thermo 檔 `H9045_Omron.cth`；HT9045 Handler System 的 Omron Comport 13；EJ1 要接 DC24V | 全部（中文字有缺漏） |
| `20140205鴻勁EJ1N-TC2報價.pdf` | 中 | 2014-02-05 | EJ1N-TC2A-CNB 1 台（金額見原檔） | — |
| `cj1w-cif11_ds_csm1972.pdf`／`cj1w-cif11_ds_j_1_1.pdf` | 英／日 | CSM_CJ1W-CIF11_DS_E_1_1 | CS/CJ PLC 用的 RS-232C→RS-422A/485 轉接頭（50 m） | HT9045 用不到（PC 端用 ADAM-4520） |
| `EJ1應用範例\` | 簡中 | 2011 | CP1H PLC 讀 PV／寫 SP 範例（CompoWay/F、Modbus-RTU、協議宏 .psw／.cxp／.doc）、`EJ1和NS进行SAP通讯.wmv` | 寫 PLC 時參考 |
| `EJ1G MODULAR … GRADIENT TEMPERATURE CONTROL USER MANUAL (1006).pdf` | 英 | H143-E1-01（2010-06），238 頁 | EJ1G | §6 |
| `EJ1G 操作手册（中文）H143-CN5-01(0914／1006).pdf`、`EJ1G 傾斜溫度控制用模組化溫度調節器.pdf`、`EJ1G Data Sheet.pdf` | 中 | — | EJ1G 中文版／型錄 | 文字層擷取不出來 |
| `EJ1G\CompoWayF_Example\` | VB6 | 2003／2012 | CompoWay/F 示範程式：預設 38400、E、7、2；送 `STX`＋text＋`ETX`＋BCC（沒有 CR LF）；`EJ1G TEST.txt`：CH1 PV `000000101C00000000001`、CH2 `000000101C00100000001` | **不要執行 `CompoWayFDemo.exe`**，看 `.frm` 原始碼就好 |
| `EJ1G\小記_研華 ADAM-4520 RS232 TO 422_485 使用--VER0.0--2011-.doc` | 中 | 2012-02-02 | ADAM-4520：SW1 資料格式用預設（1 ON、2 OFF）；SW2 鮑率 38.4k＝第 7 pin ON、其他 OFF；接 EJ1C-EDUA-NFLK 的 **PORT A**；轉接頭要另外供電 | 全部 |
| `EJ1G\小記_型號--VER0.0--2011-.doc`、`EJ1G\研華\` | 中 | 2011-2012 | 測試用型號（EJ1C-EDUA-NFLK、EJ1G-TC4A-QQ、EJ1G-HFUA-NFLK、ADAM-4520）；ADAM-4520 型錄與照片 | — |

`Omron 鴻勁科技溫控器曲線調整.pptx` 雖然放在同一層，內容講的是 **E5DC**（α 調整），收在 `omron-e5dc.md`。

## 3. 通訊重點（H142-E1-05 原文整理）

### 3.1 協定、Port A／B 與出廠值
- 通訊方式：RS-485 多點、起止同步、CompoWay/F 用 BCC、沒有流量控制，也沒有 retry（p.172，PDF 198）。Modbus 只能走 Port B（Section 7；datasheet p.4 註 *4：Basic Unit V1.1 以上）。
- **Port A：38.4 kbps、7 bit、2 stop、Even，固定不能改**；只能跑 CompoWay/F；回應等待 1～99 ms，預設 1 ms（p.172，PDF 198；datasheet p.4）。Port A 有接頭與端子台兩個出口，**兩個不能同時用**（p.18，PDF 44）。接頭那個是 CX-Thermo 的工具埠（E58-CIFQ1 USB 線）（p.7，PDF 33）。
- Port B（p.173，PDF 199）：通訊協定 0＝CompoWay/F（預設）／1＝Modbus；鮑率 3＝9.6（預設）、4＝19.2、5＝38.4、6＝57.6、7＝115.2；資料長度 7（預設）／8；停止位元 2（預設）／1；同位 Even（預設）／None／Odd；send wait 0～99 ms（預設 5 ms）。這些要透過通訊（E5 0020～）或 V1.2 的 SW2 pin 4、5 設定（4 OFF＋5 ON＝38.4k），改完重開電才生效（p.4，PDF 30；p.257，PDF 283）。
- 站號 00～63 用 SW1（0～F）＋SW2 pin 1、2 設定，出廠 01；要斷電才能改，上電時讀取（p.3，PDF 29）。可用 `XX` 廣播（不回覆）（p.174，PDF 200）。

### 3.2 Frame（p.174-177，PDF 200-203）
- Command：`STX(02H)` + 站號 2 + 子位址 `00` + SID `0` + FINS-mini text + `ETX(03H)` + BCC；BCC＝從站號到 ETX 逐 byte XOR（例 BCC＝35H）。frame 沒收到 ETX＋BCC 就不回覆。
- Response：`STX` + 站號 + `00` + end code 2 + text + `ETX` + BCC。End code：00 正常、0F FINS command error、10 parity、11 framing、12 overrun、13 BCC、14 format、16 sub-address、18 frame length（p.175，PDF 201）。
- 數值：double word 8 位／word 4 位 hex，負數用 2 的補數，拿掉小數點（105.0 → `0000041A`）（p.177，PDF 203）。
- 服務（p.177）：0101 讀、0102 寫、0104 複合讀、0113 複合寫、0110／0111／0112 複合讀登記、0503 型號、0601 狀態、0801 echoback、3005 operation command。上限：0101 double word 40 個（p.178）、0102 39 個（p.179）、0104 32 項（p.180）、0113 20 項（p.181）；0104 的回覆是「變數類型 2＋資料 8」依命令順序排（p.180，PDF 206）。
- 0503 回 Model 前 10 字（例 `EJ1N-TC4A-`）＋緩衝大小 4 字（p.185，PDF 211）。

### 3.3 PV／SV 與相關位址（參數表 p.239-257，PDF 265-283）
| 類型 位址 | 參數 | 屬性／預設 | 頁 |
|---|---|---|---|
| C0 0000 | Process Value CH1（CH2＝0100、CH3＝0200、CH4＝0300） | 唯讀 | p.239 |
| C0 0001 | Status CH1（…0101／0201／0301） | 唯讀，§3.4 | p.239 |
| C0 0002 | Internal SP CH1（…0102／0202／0302） | 唯讀 | p.239 |
| C1 0003 | Set Point CH1（…0103／0203／0303） | 運轉中可改 | p.240 |
| D4 0100 | **Present Bank Set Point CH1**（CH2＝0200…CH4＝0400），−1999～9999 | 運轉中可改，預設 0 | p.247（PDF 273） |
| D4 0111 | Present Bank Alarm Value 1 CH1 | 運轉中可改 | p.247 |
| E0 0100 | **Input Type CH1**（…0200／0300／0400）：0 Pt100 −200～850、1 Pt100 −199.9～500.0、5 K −200～1300、6 K −20.0～500.0… | **停止中才能改**，預設 5 | p.253（PDF 279） |
| E3 0100 | Alarm 1 Type CH1（CH2＝0200…） | 預設 2 | p.255（PDF 281） |
| E5 0020～0024 | Port B 通訊協定／鮑率／資料長度… | — | p.257（PDF 283） |

- SP 設定範圍固定 −1999～9999，不隨輸入種類改變（p.172，PDF 198）。1 EU 由輸入種類決定：−20.0～500.0 的範圍 1 EU＝0.1 ℃（前言 Conventions，羅馬數字頁 xx，PDF 20）。
- 輸入範圍表：p.238（PDF 264）。取樣週期 250 ms（p.235，PDF 261）。

### 3.4 Status（C0 0001）位元（p.261，PDF 287）
bit 0 Heater overcurrent、1 Heater current hold、**6 Input error**、8／9 控制輸出、10 HB alarm、**12～14 Alarm Output 1～3**、16／17 Event input、20 Write Mode（Backup／RAM write）、21 非揮發記憶體（RAM＝NV／RAM≠NV）、22 ST、**23 AT 執行中**、**24 RUN／STOP（0＝Run）**、25 Communications Writing（**永遠 ON**）、26 Auto／Manual、27 SP Mode、30 SP Ramp；其他 not used。

### 3.5 Operation Command（30 05 + 命令碼 + 相關資訊）（p.188-192，PDF 214-218）
| 碼 | 內容 | TC 的相關資訊 |
|---|---|---|
| 04 | Write Mode | 00 Backup／01 RAM |
| 06 | Software Reset | 00 |
| 0A／0B | Run／Stop | 00～03＝通道，FF＝全部 |
| 0C／0D | Manual／Auto | 同上 |
| 0E／0F／10 | 40% AT／100% AT／AT 取消 | 同上 |
| 1E～21 | Bank 0～3 | 同上 |
| 22／23 | Local SP／Remote SP | 同上 |
| 2D | Reset Error | 00 |
| 32～35 | Alarm 1～3 latch 取消／全部取消 | 00～03、FF |
| 37 | Save RAM Data（運轉中可改的設定寫進 NV） | FF |
| 38 | Parameter Initialization | 00 |
| 39 | Save RAM Data 2（全部寫進 NV，要全部停止） | FF |
| 3B | Register Unit Configuration（G3ZA） | 00／01 |

- AT 在 STOP 或 Manual 時會回操作錯誤（p.190）。
- Response code（p.178-192）：0000 正常、1001／1002 命令過長／過短、1101 變數類型錯、1003 數量不符、1100 參數錯、110B 回應過長、0402 型號／版本不符、2201 運轉中不可執行、2202 停止中不可執行、2203 模式不符、3003 寫唯讀、7011 裝置錯誤、7012 配置錯誤、7013 暫時不能收命令（要重試）、7014 正在寫非揮發記憶體（要重試）、7015 重置／開機中、7016 錯誤保持中、7020／7021／7030 控制或輸出模式不符、7041 手動模式、7042 調整模式。

### 3.6 EEPROM 與 write mode
- 非揮發記憶體只能寫 **100,000 次**（p.235，PDF 261；datasheet p.3）。
- Write Mode 只對 **Port B** 有效，出廠是 RAM write mode（運轉中改的設定不寫進 NV）。**從 Port A 改的設定一律寫進非揮發記憶體**（p.7，PDF 33；p.190 註 1，PDF 216）。
- EJ1 沒有「通訊寫入許可」的參數（Status bit 25 永遠 ON），不用像 E5DC 那樣先開 `cmwt`。

## 4. 與程式對照

| 項目 | 手冊 | golden V912 | 判定 |
|---|---|---|---|
| 通訊參數 | Port A 固定 38.4k／7／2／Even | 38400／7／2／Even（`:333-338`） | 一致（要接 Port A；接 Port B 就得先改鮑率） |
| Frame、BCC、End／Response code | §3.2、§3.5 | `SendCommand` `:376`、`Main232` 驗 BCC `:1720-1730`、碼表 `:123-155` | 一致；碼表跟手冊相同 |
| 站號編碼 | 00～63（手冊沒寫 frame 裡用 BCD 還是 hex；SW 對照表是十進位，E5□C 手冊寫 BCD） | 送 `%02X`、收 `atoi`（十進位） | 站號 1～8 時兩種寫法結果一樣；**站號 ≥10 就會不一致**（10 會送成 `0A`）——目前最多 8 台，碰不到 |
| PV 位址 | C0 0000／0100／0200／0300 | `:1081` | 一致 |
| 狀態＋SP 位址 | C0 0001／0002（每通道 +0100） | `:1193` | 一致；讀的是 **Internal SP**，不是寫進去的 SP |
| 寫 SV | D4 0100～0400＝Present Bank SP，0113 一次最多 20 項 | `:1386-1393`（4 項） | 一致 |
| 輸入種類 | E0 0100～0400；**停止中才能改** | 先 STOP→寫→RUN（`:718-748`） | 一致；每次開機都寫 1（Pt100）（`main.cpp:10952-10953`），Port A 會寫進 NV |
| RUN／STOP、AT | 0A／0B、0E／0F／10，FF＝全部、00～03＝通道 | `:1661-1663`、`:1477-1483`、`:1559` | 一致 |
| 0503 型號判斷 | 回型號前 10 字，例 `EJ1N-TC4A-`（p.185）；型號有 TC4A／TC4B／TC2A／TC2B（p.7，PDF 33），A＝M3 螺絲端子、B＝免螺絲端子（datasheet NEW_0614 p.1） | 只認 `TC4A`、`TC2A`（`:1813-1820`） | **疑點**：買到 TC4B／TC2B（免螺絲端子款）時 `bModulType` 會留 0，那台的通道都不會被輪詢 |
| TC2 讀 PV | 應讀 C0 0000／0100 | TC2 的「讀 PV」（`:1094`）跟「讀 SV」（`:1204`）是同一串（C0 0001／0002／0101／0102） | **疑點**：接 TC2 時，PV 欄位會被填進 Status 與 SP 的後 3 位。HT9045 現在用 TC4，但 2014 年有 TC2 的報價 |
| PV／SP 位數 | 8 位 hex、2 的補數 | 只取後 3 位（`:528`、`:562`） | 只能表示 0.0～409.5；**PV 超過 409.5 ℃ 會繞回小值**（例 410.0＝`0x1004` → 讀成 0.4），0 ℃ 以下會讀成約 409 ℃。Pt100（1）的指示範圍到 520.0（p.238） |
| SV 一直重寫的可能 | Internal SP 會受 SP 限制器／SP ramp 影響；SP 只到 0.1 | `bthermo.cpp:3621-3812` 拿 `Temp`（double）跟讀回的 `dSetValue` 比 | **疑點（從程式推的）**：設定溫度有 0.01 位、超過 409.5，或 Internal SP≠SP（ramp、限制器）時，每一圈都判成「有變」→ 每圈都重寫 SV。接 Port A 時每次都寫 NV（10 萬次） |
| SV 負值 | 8 位 2 的補數 | `0000%04X` | 負數會變成 12 位 → 1003；Index 只加熱，碰不到 |
| BCC 後的 CR LF | frame 到 BCC 結束 | 多送 `\r\n`（`:376`）；VB6 範例沒送 | 手冊沒定義；現場可用，推測被忽略 |
| Status 位元 | bit 6／12～14／23／24 | `:555-560` | 一致 |
| 逾時 | Port A 回應等待預設 1 ms | 等 4～10 s（`:1099`、`:1210`、`:24`） | 寬鬆、可用 |

## 5. 注意事項

1. **站號**：每台 TC4 斷電後設 SW1＝1～8、SW2 pin 1、2 OFF，上電才生效（p.3）。SW2 其他 pin：3 ON＝Port B 用 Modbus、4／5＝Port B 鮑率、6＝指示燈改顯示輸出、7＝G3ZA、8＝分散配置（p.4，PDF 30），HT9045 都不用，保持 OFF。
2. **配線（EDUA 螺絲端子）**：Port A 端子 1＝B(+)、2＝A(−)；Port B 端子 6＝B(+)、7＝A(−)；24 VDC 接 8(+)／9(−)（EDUC 是 Port B 7／8、電源 9／10）（p.18，PDF 44）。傳輸線兩端（含 PC 端）接 110～125 Ω 1/2 W 終端電阻，合成至少 54 Ω；用 Port A 接頭有雜訊問題時，在 Port A 端子 1、2 間跨終端電阻（p.23，PDF 49）。
3. **Port A 接頭與端子不能同時用**：接著 HT9045 時不要再插 CX-Thermo 的 USB 線到 Port A 接頭（p.18）。要用 CX-Thermo（例如載入 `H9045_Omron.cth`）就先拔掉 HT9045 那條。
4. **PC 端轉接**：ADAM-4520（RS-232→RS-485）的 SW2 第 7 pin ON＝38.4k、要另外供電（ADAM-4520 小記）。SW1（資料格式）：小記用預設 10 bits（pin 1 ON、pin 2 OFF），但 Port A 的 7E2 是 1＋7＋1＋2＝11 bits，照 ADAM Startup 手冊 p.2 應是 pin 1 OFF、pin 2 ON（20261001 ST01-E3 對手冊）。推論：SW1 決定 ADAM 自動切換收送的時間，設 10 bits 只會切掉第 2 個停止位元，接收端只看 1 個停止位元，所以 2012 年實機能用；兩種都待 EastSun 上機確認。測試步驟文件寫「(38400,N,2)」，但程式與 Port A 都是 7 bit／Even／2——以程式為準。
5. **EEPROM 壽命**：Port A 改設定一律寫 NV（10 萬次）。HT9045 每次開機寫 4×N 筆輸入種類，SV 只在變動時寫；若懷疑 §4「SV 一直重寫」發生，看 EJ1N 通訊 log（`WriteInfoToMemo` 存到 `asEJ1NLogPath`，`OmronEJ1N.cpp:393-409`）的 `DoSetSV` 次數。
6. **感測器**：程式開機固定寫輸入種類 1（Pt100 −199.9～500.0）；用 K 熱電偶的機台要把畫面 `rgSensorType` 改成 K（寫 6），否則讀值錯。PV ÷10 的前提是 0.1 解析度的輸入種類。
7. 電源：EDU 吃 24 VDC，運轉電壓 85%～110%（p.234，PDF 260）。

### EJ1G（傾斜溫控）跟 EJ1N 差在哪
- 型號：EJ1G-TC4A-QQ、EJ1G-TC2A-QNHB、**EJ1G-HFUA-NFLK（Advanced Unit，必須有）**、EJ1C-EDUA-NFLK（H143 PDF 13）。
- 控制：由 HFU 做「傾斜溫度控制」（同時控整面平均溫度與各點溫差，並抵銷加熱器互相干擾），或依群組做 2-PID；TC4／TC2 只當 I/O（H143 p.6-7，PDF 27-28）。
- 通訊：H143 只有 CompoWay/F 一章（沒有 Modbus）；EDU Port A 同樣固定 38.4k／7／2／Even，frame、BCC、服務碼跟 EJ1N 相同（H143 p.126-142，PDF 147-163）。
- SP 依群組設定，例：D0（90）0100「Bank 0 Set Point - CH1」、010D「Bank 0 Alarm Value - CH1」；調整用 GT（gradient tuning），要所有群組都在 STOP＋AUTO 才能執行（H143 p.41，PDF 62）。
- HT9045 golden 程式完全沒有 EJ1G（全樹搜尋 `EJ1G` 0 筆）。EJ1G 只在 2011～2012 年由 ADAM-4520＋VB6 範例測過（`EJ1G\` 資料夾），HT9045 的 `OmronEJ1N.cpp` 不能直接拿去控 EJ1G。

<!-- preserved-content:end -->
