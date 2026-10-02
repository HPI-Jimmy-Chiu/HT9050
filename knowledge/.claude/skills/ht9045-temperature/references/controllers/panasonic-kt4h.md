# Panasonic KT4H（溫控器手冊參照）

> 手冊放在 Steven01 本機 `E:\HT9045W_相關料件技術文件\溫控器\`（不在 repo；其他電腦沒有這顆 E 槽，引用頁碼前先確認自己在 Steven01）。
> 頁碼：`kt4h_f412e-2.pdf` 的 PDF 頁＝手冊印刷頁（第 1 頁封面沒有字），本檔「p.」一律指這本。
> 程式：golden＝V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（BCB6，cp950，用 `iconv -f BIG5 -t UTF-8` 讀）；移植樹＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`（UTF-8），行號以 `D:\HT9045` 工作樹分支 `v906/steven-cbridge-review6`、HEAD `e0e7c66c`（20261001 11:00）為準。
> 整理方式（20261001）：手冊用 `pdftotext` 抽字對照（Read 工具缺 pdftoppm，看不到圖）；表格欄位是從抽字結果重排的，圖裡的按鍵符號抽不出來。

## 1. HT9045 在哪裡用

### 1.1 設定鍵與值

| 設定 | 值 | 意思 | 程式位置（golden） |
|---|---|---|---|
| `[TempCtrl] HEATER_CTRL_TYPE`（`D:\HT9045\system\Gerneral.ini`） | `1` | 整台廠牌＝KT4H；**缺鍵時的預設就是 1** | 常數 `cmydef.cpp:223`（`const int KT4H=1`）、全域初值 `cmydef.cpp:227`（`TC401HeaterControl=1`）、讀檔 `database.cpp:433`（預設 `KT4H`）、HandlerSys 讀檔 `HandlerSys.cpp:52`、`:263`；顯示字 "Panasonic KT4H" `MachineTypeUtility.cpp:21` |
| `[TempCtrl] HeaterInsOpt_<通道>` | `1` | 該通道是 KT4H（V912 逐通道；-9999＝跟著 HEATER_CTRL_TYPE） | 判斷 `IsValEqual_HeaterInsOpt(Addr, KT4H)`：`bthermo.cpp:2524`（寫）、`:2558`（讀）、`:2587`（下一個 Task）、`:3219`（警報值） |
| `[System] USE_16_HEATER` | `1` eht16Heater／`4` eht32HeaterKT4H | Index 區用 KT4H 16 組／32 組，走同一個溫控 COM 埠輪詢 | enum `MachineType.h:718`、`:721`；分支 `bthermo.cpp:1331-1367`（值 1：Aa1～Bd2＝序號 11～26 照輪詢；值 4：再加 Ae1～Bh2＝33～48） |
| `IniConfig.bEnableKT4HAlarm1`（不是 ini 鍵） | true／false | 加熱時順便寫 KT4H Alarm 1 值當過溫保護 | 宣告 `Config.h:180`；預設 false `CosFunction.cpp:4113`（`InitialCosFunction`）；只有 `FUNC_CC_HONPREC_QC`（`:32`）、`FUNC_CC_Allegro_Philippines`（`:3682`）、`FUNC_CC_Elmos_Germany`（`:3748`）設 true |

逐通道廠牌、存檔留 -9999、沒有 else 的缺陷等，見 `D:\HT9045\.claude\skills\ht9045-heater-control\SKILL.md` §2、§3。

### 1.2 通訊埠與參數

- 埠：`[TempCtrl] COM_PORT` → `HSys.sTempComPort`（golden `database.cpp:522`，預設 `COM2`）。TC401／KT4H／E5DC／DTK4848 全部共用這一個埠。
- 參數：golden `rs232.cpp:266-283` 開 `Comm2`：**9600 bps、8 data bits、無同位、1 stop**（`:269-273`）。條件 `USE_NEW_TEMPCTRL_FUNCTION==false`（`database.cpp:340` 強制 false）。開埠失敗訊息寫死 "Temperature KT4H : <埠> port error"（`:174`、`:281`），不管實際廠牌。
- 整台 No Heater（`IsNoHeaterMachine()`）時不開這個埠（`rs232.cpp:165-169`）。
- 收資料（`rs232.cpp:742-762`）：通道廠牌不是 TC401／E5DC 時，第一個字不是 `:` 就丟掉，否則整串放進 `Com2Buffer`、設 `Com2ReceiveOK`。

### 1.3 站號規則

- 站號＝通道序號（`eTempControll`，golden `MachineType.h:638-655`，0～70）＋1，用 **兩位十六進位** 寫進訊框（`cpublic.cpp:185`、`:196` 的 `%02X`）。`tcTotalCount`＝71，所以站號 01H～47H（1～71）。
- KT4H 面板的 `[20] Instrument number` 是十進位 1～99（p.16），要設成「序號＋1」的十進位值；Modbus 位址欄位就是那個數字的十六進位（p.47 範圍 0～95＝00H～5FH）。

| 通道 | 序號 | 面板 [20] | 訊框站號 |
|---|---|---|---|
| tcHotPlate1 | 0 | 1 | `01` |
| tcShuttle1 | 2 | 3 | `03` |
| tcHead1 | 4 | 5 | `05` |
| tcCCD | 10 | 11 | `0B` |
| tcAa1（Index 第一區） | 11 | 12 | `0C` |
| tcBd2 | 26 | 27 | `1B` |
| tcAe1（32 組的第 17 區） | 33 | 34 | `22` |
| tcBh2 | 48 | 49 | `31` |

### 1.4 程式位置（golden V912）

| 函式／段落 | 位置 | 內容 |
|---|---|---|
| `UT100WordWriteNoSucm(Addr, Command, Value)` | `cpublic.cpp:179-190` | `":%02X%02X%04X%4s00\r\n"`，fc 06；`IntToHex(Value,4)`；LRC 算前 12 字、蓋在第 13、14 字 |
| `UT100WordReadNoSucm(Addr, Command)` | `cpublic.cpp:192-201` | 同格式，fc 03，筆數寫死 `0001` |
| `A_Create_LCR` | `EJ1N\TextProcess.cpp:563-580` | 每兩個 ASCII 字轉一個 byte 相加，取 2 補數 |
| `A_Check_LRC`／`A_Get_Function_Code`／`A_Get_MEM_Word` | `bthermo.cpp:1040-1060`／`:1067-1074`／`:1086-1094` | 收到的 LRC 比對（假設尾巴是 CR LF）；取 fc；取 `READBUFF[7..10]` 為 16 位元**無號**值 |
| `A_Check_Addr` | `bthermo.cpp:1024-1038` | 有定義，**全樹沒人呼叫** |
| `DoThermoReal` case 100 寫 SV | `bthermo.cpp:2492-2537` | 設定溫度變了才寫；`WriteCommand=0x0001`（`:2512`）；`UT100WordWriteNoSucm(Addr, WriteCommand, Temp*10)`（`:2526`，`Temp` 是 double） |
| case 100 讀 PV | `bthermo.cpp:2544-2561` | `ReadCommand=0x0080`（`:2546`）；`UT100WordReadNoSucm`（`:2560`）；等回應 `Com2Delay` 0.5 秒（`:2572`）；`Task=250`（`:2587-2590`） |
| case 250 收 PV | `bthermo.cpp:2687-2801` | LRC 錯或逾時→先重試（`MAX_RETRY`=2），用完就 `StopComm`、`CommunCTErr++`，超過 5 就 `UN150CommError=true`、`UN150Read=999`（`:2694-2717`、`:2779-2800`）；fc 不是 3 → `Task=300`（`:2721-2725`）；`Read=A_Get_MEM_Word()/10.0`（`:2726`）；加熱中且 `bEnableKT4HAlarm1` 且不是 CCD／HeatGun／2D／LB 類 → `Task=500`（`:2754-2771`） |
| case 500／510 寫 Alarm 1 值 | `bthermo.cpp:3218-3293` | `WriteCommand=0x000B`（`:3226`）；`OldTemp==0`→0、`0<OldTemp<120`→1400（140.0）、`>=120`→`(OldTemp+10)*10`（`:3228-3240`）；等 0.6 秒（`:3241`）；510 收到就 `Task=300` |
| case 520／530 寫 Alarm 1 種類 | `bthermo.cpp:3294-3357` | `WriteCommand=0x0023`（`:3296`）；未安裝或 0 度寫 0、否則寫 1（註解 "0001H = High Limit Alarm"，`:3299`、`:3303`）。**全檔沒有 `Task=520`**（只有 500／510／530 被指定）⇒ 這段走不到 |
| case 300 換通道 | `bthermo.cpp:3209-3217` | `Addr++`、`g_iHeaterTypeIdx_SendCmd=Addr` |
| Configuration 頁手動通訊 | `cConfiguration.cpp:5623-5626`（寫 0x0001，註解 "Steven 20120831 : 6 -> 0x0001"）、`:5643-5646`（讀 0x0080）、`:5699-5712`（解析） | 解析用 `GetEveryCode`＋`Change_Tempture_Value`（`cpublic.cpp:147-175`）：看第一個 hex 字是不是 F 判負數，**不驗 LRC** |
| `TMyPanasonic` 類別 | `TempCtrl\MyTempture_KT4H.cpp` | 舊的「新溫控架構」，**不在 `HT9045.bpr`**（`TempCtrl\` 只編 `TriTemp.cpp`），沒編譯；有處理負值與 +0.5 四捨五入（`:71-82`、`:228`），可當對照 |

### 1.5 移植樹 V906 現況

- 通訊函式在 `#if 0` 裡：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cpublic.cpp:292-349`（`UT100WordWriteNoSucm` `:326`、`UT100WordReadNoSucm` `:339`）。
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\bthermo.cpp`：寫 SV 閘在 `:2674-2683`、讀 PV 閘在 `:2736-2745`（判斷用全域 `TC401HeaterControl==KT4H`，不是逐通道）；case 250 在 `:2904`（`Task=500` 在 `:2999`）；case 500 在 `:3502`；case 520 在 `:3603`，同樣沒有 `Task=520`。
- 溫控 COM 埠（Comm2）沒翻（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\rs232.cpp:245-255` 的閘住清單）。溫控迴圈在 wb_serve 裡也沒跑（heater-control skill §4）。

## 2. 手冊清單

| 檔案（絕對路徑） | 語言 | 版本／日期 | 內容 | 先看哪幾頁 |
|---|---|---|---|---|
| `E:\HT9045W_相關料件技術文件\溫控器\kt4h_f412e-2.pdf` | English | ARCT1F412E-2，**第 3 版，April 2007**（改版紀錄 p.65）；66 頁 | 完整 User's Manual（Matsushita Electric Works） | 目錄 p.2；通訊 12 章 p.36-56（12.1 架構 p.36、12.2 配線 p.36-37、12.3 參數 p.38、12.4 程序 p.39、12.5 MEWTOCOL p.40-46、12.6 Modbus p.47-52、Modbus 資料表 p.53-56）；14.4 通訊排錯 p.64；輔助設定模式 p.16-17；Setup 模式 p.18-22；警報動作 p.30；選配規格 p.60-61；型號 p.5 |
| `E:\HT9045W_相關料件技術文件\溫控器\Panasonic KT4H溫度控制器操作手冊(English).pdf` | English（檔名中文） | ARCT1F412E-1，第 2 版，'05.10（封面 p.1）；63 頁 | 同一本 User's Manual 的舊版 | 通訊 p.33-53（12.3 p.35、12.6 Modbus p.44、資料表 p.50-53） |
| `E:\HT9045W_相關料件技術文件\溫控器\KT4H_inst_e.pdf` | English | Installation Instructions No. KT4HE3，2006.08；1 頁（A3 雙面） | 面板、尺寸、配線端子、操作流程圖（設定項 [1]～[56]）、輸入種類表、警報種類圖、規格 | 流程圖與端子圖；沒有通訊協定細節 |

新舊兩版比對：Modbus 一節（舊 p.44-53、新 p.47-56）逐行 diff 只有措辭不同，另外 RTU 寫入範例的 CRC 由 `DB90H` 改成 `D890H`；資料位址表完全相同。新版多了 11.8～11.11（冷卻側動作、SV rise/fall），所以頁碼往後 3 頁。**以 `kt4h_f412e-2.pdf` 為準。**

## 3. 通訊重點（頁碼＝kt4h_f412e-2.pdf）

### 3.1 介面與接線
- RS-485、半雙工、start-stop 同步；一台主機最多 **31 台**；最長 1,000 m、線阻 50Ω 以內（p.61；架構圖 Fig.12.1-1 p.36 畫的是主機 RS-232C → 轉換器 → RS-485 → No.1～No.31）。
- 要有序列通訊選配：型號第 (7) 碼＝1（p.5）；序列通訊跟 Contact input 互斥（p.5、p.61）；有序列通訊時 SV2～SV4 不能用（p.13）。
- 終端電阻：KT4H 內建 pull-up／pull-down，**不要接**；若有反射非接不可，接在 PLC（主機）側、120Ω 以上（p.37）。屏蔽線只有一端接 FG（p.37）。
- 序列通訊跟 Tool port（USB 工具線 AKT4H820）不能同時用，做序列通訊要拔掉工具線（p.36）。
- 時序：從站回應前至少空 5 ms（可設 5～99 ms）；主站送完要在 1 個字元時間內放開發送器（p.39）。

### 3.2 通訊參數（輔助功能設定模式 [19]～[24]，p.16-17、p.38）

| 項目 | 選項 | 出廠值 | HT9045 要設 |
|---|---|---|---|
| [19] 協定 | Modbus ASCII／Modbus RTU／MEWTOCOL | Modbus ASCII | Modbus ASCII（同出廠） |
| [20] 站號 | 1～99 | 1 | 通道序號＋1（§1.3） |
| [21] 速度 | 2400／4800／9600／19200 | 9600 | 9600（同出廠） |
| [22] 資料位元／同位 | 8N、7N、8E、7E、8O、7O | **7 bits／Even** | **8 bits／No parity**（程式開 8N1） |
| [23] 停止位元 | 1／2 | 1 | 1 |
| [24] 回應時間 | 5～99 ms | 5 ms | 不用改（程式等 0.5 秒） |

站號與通訊速度不能用通訊改，只能面板設（p.56）。進入輔助功能設定模式的按鍵組合在 p.16 與 `KT4H_inst_e.pdf` 流程圖（抽字看不到按鍵符號，要看原檔）。

### 3.3 Modbus ASCII 訊框（p.47-49）
- 資料格式：start 1、data 7 bits（p.47 寫 7；p.60-61 規格表寫 Modbus ASCII 可 7 或 8）、同位 Even／無／Odd、stop 1／2；錯誤檢查 LRC；字元間隔 1 秒以內。
- 訊框：`:`（3AH）＋站號 2 字＋功能碼 2 字＋資料＋LRC 2 字＋CR LF。
- 站號 0～95（00H～5FH）；0＝廣播，從站不回（p.47）。
- 功能碼只有 **03H 讀、06H 寫**；一次只處理 1 筆，筆數固定 `0001`，回應 byte 數固定 `02`；資料範圍 -32768～32767（8000H～7FFFH）（p.47）。
- LRC：把站號到資料結尾的 byte 相加，位元反相再加 1，轉成兩個 ASCII 字（p.48）。
- 範例（站號 1）：讀 PV `:010300800001` `7B` CRLF，回 `:0103020258` `A0` CRLF（PV=600＝0258H，p.48）；讀 SV `:010300010001` `FA`（p.49）；寫 SV=600 `:010600010258` `9E`，正常回應原樣回傳（p.49）；例外回應 `:018302` `7A`（位址錯）、`:018603` `76`（超出範圍）（p.49）。
- RTU（p.50-52）：8 bits、CRC-16（多項式 A001H、初值 FFFFH）、前後 3.5 字元靜默；功能碼與位址同 ASCII。HT9045 的 KT4H 不用 RTU。

### 3.4 例外與不回應（p.39、p.47、p.56）
- 例外回應＝功能碼 MSB 設 1（83H／86H）＋例外碼：01H 不存在的功能、02H 不存在的位址、03H 超出設定範圍、11H 目前狀態不能設（例如 AT 中）、12H 面板正在按鍵設定模式（p.47）。
- 不回應：廣播、framing／parity 錯、LRC 不符、CRC 不符（p.39）。
- p.56 的特例：OUT/OFF 鍵功能與 Auto/Manual 衝突、自動時設 Manual MV、PI／ON-OFF 動作時做 AT → 01H；AT 重複執行／取消 → 11H。

### 3.5 Modbus 資料位址（p.53-56；MEWTOCOL 的 DT 編號＝100＋2×Modbus 位址，p.43-46）

| 位址 | 功能碼 | 項目 | 資料 | HT9045 |
|---|---|---|---|---|
| 0000H | 03H | Use in the system | **不可用 06H 寫**，寫了 KT4H 可能不能動（p.53） | — |
| 0001H | 03H/06H | SV | 設定值，小數點忽略 | 寫（`bthermo.cpp:2512`） |
| 0003H | 03H/06H | AT／Auto-reset | 0000H 取消、0001H 執行 | — |
| 0004H／0006H／0007H | 03H/06H | OUT1 比例帶／積分／微分 | 設定值 | — |
| 000BH | 03H/06H | Alarm 1 value | 設定值，小數點忽略 | 寫（task 500） |
| 000CH | 03H/06H | Alarm 2 value | 同上 | — |
| 0012H | 03H/06H | Set value lock | 0000H Unlock、0001H Lock 1、0002H Lock 2、0003H Lock 3 | — |
| 0015H | 03H/06H | Sensor correction | 設定值，小數點忽略 | — |
| 0018H／0019H | 03H/06H | Scaling 上限／下限 | 設定值 | — |
| 001AH | 03H/06H | Decimal point place | 0000H xxxx、0001H xxx.x、0002H xx.xx、0003H x.xxx（**只有 DC 輸入可選**，p.18） | — |
| 0023H | 03H/06H | Alarm 1 type | 0000H 無、0001H High limit（偏差）、0002H Low limit、0003H High/Low limits、0004H High/Low limit range、0005H **Process high（絕對值）**、0006H Process low、0007H～0009H 各型 with standby | 寫 0／1，但走不到（task 520） |
| 0024H／0025H／0029H | 03H/06H | Alarm 2 type／Alarm 1 遲滯／Alarm 1 延遲 | — | — |
| 0037H／0038H／0039H | 03H/06H | 輸出 ON/OFF／Auto-Manual／Manual MV | 0000H ON、0001H OFF 等 | — |
| 0040H | 03H/06H | Alarm 1 Energized/Deenergized | 0000H Energized、0001H Deenergized | — |
| 0044H | 03H/06H | Input type | 0000H K -200～1370、0001H K -200.0～400.0、000BH Pt100 -200.0～850.0 … 0023H 0～10V（共 36 種） | — |
| 0045H | 03H/06H | Direct/Reverse | 0000H Reverse（加熱）、0001H Direct | — |
| 0070H | 06H | 按鍵變更旗標清除 | 0001H 全清 | — |
| 0080H | 03H | PV | 小數點忽略 | 讀（`bthermo.cpp:2546`） |
| 0081H／0082H／0083H | 03H | OUT1 MV／OUT2 MV／爬升中的 SV | 小數點忽略 | — |
| 0085H | 03H | Status flag | b0 OUT1、b1 OUT2、b2 Alarm 1 輸出、b3 Alarm 2 輸出、b6 斷線警報、b8 Overscale、b9 Underscale、b10 控制輸出 OFF、b11 AT 中、b12 OUT/OFF 鍵功能、b14 Manual、b15 有按鍵變更 | — |
| 0086H／0087H | 03H | CT1／CT2 電流 | 小數點忽略 | — |
| 00A1H | 03H | Instrument specification flag | b1＝有序列通訊選配等 | — |

### 3.6 小數點、範圍與寫入限制
- 所有資料是整數的十六進位，小數點拿掉（"Decimal point ignored"）；負數用 2 補數（p.56）。
- 熱電偶／RTD 的小數位數由**輸入種類**決定（[25] Input type，p.18，例如 K -200.0～400.0 有 1 位、K -200～1370 沒有）；[28] Decimal point place 只對 DC 輸入有效（p.18）。出廠輸入是 K -200～1370、Scaling 上限 1370、下限 -200（p.18）；SV 範圍＝Scaling 下限～上限（p.13）。
- Alarm 1 值範圍依種類（Table 6.6-1，p.15）：偏差型 ±輸入跨距、Process 型＝輸入範圍；設 0 等於關掉（Process high／low 除外）。
- 動作點：High limit alarm＝**SV＋A1 值**；Process high alarm＝**A1 值本身**（p.30 圖；`KT4H_inst_e.pdf` 警報圖相同）。
- 改 Alarm 1／2 種類時，警報值會被清成 0、警報輸出重設（p.56）。
- 面板鎖住時通訊照樣能寫（p.56）。
- 非揮發記憶體寫入上限 1,000,000 次，不建議頻繁用通訊寫（p.56）；「通訊寫入的值跟原值相同就不寫非揮發記憶體」這句只出現在 Lock 3 的說明裡（p.16）。

## 4. 與程式對照

| # | 項目 | 手冊 | 程式（golden） | 判定 |
|---|---|---|---|---|
| 1 | 協定與 LRC | Modbus ASCII、LRC＝byte 和的 2 補數（p.47-48） | `cpublic.cpp:185`、`:196`；`EJ1N\TextProcess.cpp:563-580` | **一致**。手冊範例 `:010300800001`→`7B`、`:010600010258`→`9E` 用程式的算法算出來相同 |
| 2 | 功能碼／筆數 | 03H／06H，筆數 0001（p.47） | `cpublic.cpp:182`、`:194`、`:196` | 一致 |
| 3 | SV 位址 | 0001H（p.53） | `bthermo.cpp:2512`、`cConfiguration.cpp:5625` | 一致。`cConfiguration.cpp:5625` 註解「6 -> 0x0001」：20120831 以前寫的是 0006H＝積分時間 |
| 4 | PV 位址 | 0080H（p.55） | `bthermo.cpp:2546`、`cConfiguration.cpp:5645` | 一致 |
| 5 | Alarm 1 值位址 | 000BH（p.53） | `bthermo.cpp:3226` | 位址一致 |
| 6 | Alarm 1 種類 | 0023H；0001H＝High limit（偏差型，動作點 SV＋值）、0005H＝Process high（絕對值）（p.53、p.30） | `bthermo.cpp:3296-3304` 寫 1；寫的警報值是絕對溫度（註解「小於120的一律設定為140度」「大於120的一律比設定多10度」，`:3233-3239`） | ⚠ **語意不符＋走不到**。task 520 沒人進得去，所以程式從來沒寫過種類，全靠面板設定。⛔ 20261001 更正（ST01-E3 對手冊）：[38] 出廠是 `----`（No alarm action，p.20），照出廠值時寫進去的警報值**完全沒有作用**。面板若改成 High limit alarm（`H`），實際跳脫點是 SV＋140（SV<120）或 2×SV＋10（SV≥120），保護等於沒作用；要符合註解的意思，面板要設 Process high alarm（0005H）。若有人把 520 接上：寫 1 會變偏差型，而且 p.56「改種類會把警報值清 0」，520 排在 500 後面會把剛寫的值清掉 |
| 7 | 資料位元／同位 | 出廠 7 bits／Even／1（p.17、p.38、p.60） | 8N1（`rs232.cpp:269-273`） | ⚠ **跟出廠值不同**：每台 KT4H 的 [22] 都要改成 8 bits／No parity。p.47 寫 ASCII 用 7 bits，但 p.60-61 規格表寫 Modbus ASCII 可 7 或 8，所以 8N1 可行 |
| 8 | 站號範圍 | Modbus 0～95、面板 1～99（p.47、p.16） | `Addr+1`、`%02X`，最大 71（47H） | 一致；面板要設十進位的序號＋1 |
| 9 | 台數 | 一台主機最多 31 台（p.61） | 所有通道共用一個 COM 埠（`database.cpp:522`）；`USE_16_HEATER=4` 時 Index 就有 32 區，再加其他通道 | ⚠ 推論（沒查配線）：eht32HeaterKT4H 一條線會超過 31 台，現場是否分段／中繼要問硬體 |
| 10 | ×10 換算 | 整數、小數點忽略（p.56） | 寫 `Temp*10`（`:2526`）、讀 `/10.0`（`:2726`）、警報值 `*10`（`:3235`、`:3239`） | 成立的前提：**輸入種類要選 1 位小數的**（例如 0001H K -200.0～400.0、000BH Pt100 -200.0～850.0）。若是出廠的 K -200～1370，設 125 度會寫 1250、而且在 Scaling 1370 內不會被拒 ⇒ 實際設成 1250 度。程式沒有讀 0044H 檢查 |
| 11 | 負數 | 2 補數（p.56） | `A_Get_MEM_Word` 回無號值（`bthermo.cpp:1086-1094`）⇒ -1.0 度（FFF6H）會讀成 6552.6；寫入用 `IntToHex(Value,4)`（`cpublic.cpp:184`），負值會超過 4 位（推論，未實測）把訊框弄壞。Configuration 頁的 `Change_Tempture_Value` 有處理 F 開頭負數（`cpublic.cpp:157-175`）；沒編譯的 `TMyPanasonic::WriteData` 也有（`TempCtrl\MyTempture_KT4H.cpp:71-82`） | 不符，只影響 0 度以下；KT4H 用在加熱，影響小 |
| 12 | 例外回應 | 83H／86H＋例外碼（p.47、p.49） | case 250：fc≠3 直接 `Task=300`，不計錯、不記錄（`:2721-2725`）；case 510／530：fc 讀進 `i` 就丟掉（`:3271`、`:3335`） | ⚠ 程式不處理 NAK：例如面板正在按鍵設定（12H）時寫 SV 失敗不會報；讀 PV 收到例外時溫度停在上一次的值、`UN150CommError` 不會設 |
| 13 | 回應站號 | 從站回自己的站號（p.47） | `A_Check_Addr` 有定義沒呼叫（`bthermo.cpp:1024-1038`） | 不檢查回應站號 |
| 14 | 回應 LRC | p.48 | `A_Check_LRC`（`bthermo.cpp:1040-1060`），以 `strlen-4` 找 LRC（假設 CR LF 結尾） | 一致。Configuration 頁的解析不驗 LRC |
| 15 | 寫入頻率 | 寫入上限 100 萬次，不建議頻繁寫（p.56） | SV 只在變動時寫（`:2492`）；但開了 `bEnableKT4HAlarm1` 時，**每一次讀 PV 成功都寫一次 Alarm 1 值**（`:2754-2771` → `:3218-3241`） | ⚠ 手冊只在 Lock 3 說明提到同值不寫記憶體（p.16），一般狀態沒寫；要不要改成值變了才寫，請 Jimmy 判斷 |
| 16 | 回應時間 | 從站 5～99 ms（p.39） | 等 0.5 秒（`:2572`）、警報寫入等 0.6 秒（`:3241`） | 足夠 |
| 17 | 四捨五入 | — | `Temp*10` 從 double 轉 int 是截斷；沒編譯的舊類別寫 `*10.0+0.5`、註解「避免浮點運算造成資料不正確」（`TempCtrl\MyTempture_KT4H.cpp:228`） | 推論：可能差 0.1 度，未實測 |

移植樹 V906 的 KT4H 路徑整段閘住（§1.5），上面的對照只對 V912 有意義；照 RULINGS_20260927 第 1 條，V912 的問題只通報 Jimmy，不改 V912。

## 5. 注意事項

- **新機或換 KT4H 的面板檢查**：[19] Modbus ASCII、[20]＝通道序號＋1（十進位）、[21] 9600、[22] **8 bits／No parity**、[23] 1、輸入種類選**有 1 位小數**的；有 `bEnableKT4HAlarm1` 的客戶（HONPREC_QC、Allegro_Philippines、Elmos_Germany）另外把 [38] Alarm 1 type 設成 Process high（`AS`，絕對值）；出廠是 `----`（無警報，p.20），不改就沒有過溫保護。[38] 在 Setup mode：PV/SV 顯示時按住 [△] 再按 [▽] 約 3 秒進 [25]，按 [MODE] 往下（p.12 流程圖）。
- 要有序列通訊選配（型號第 7 碼＝1）；有序列通訊就不能有 Contact input、不能用 SV2～SV4。
- 做序列通訊時拔掉 USB 工具線；KT4H 端不要接終端電阻。
- 畫面溫度顯示 999＝那一通道收不到或 LRC 錯、重試用完（`MAX_RETRY`=2，`bthermo.cpp:1246`）的次數累積超過 5 次（`CommunCTErr`，`:2708-2714`、`:2790-2796`；讀成功就歸零 `:2753`）；收到例外回應不會變 999。
- 函式名 `UT100WordWriteNoSucm`／`bUT150Install` 是沿用 Yokogawa UT100／UT150 時代的命名，協定其實是 KT4H 的 Modbus ASCII，不要拿 Yokogawa 手冊對。
- 2012 年 Index 16 組 KT4H 被評估換成 Omron EJ1N、2018 年評估換成台達 DTK（見同資料夾的 index.md §2）。
- `bthermo.cpp`／`cpublic.cpp`／`rs232.cpp`／`cConfiguration.cpp` 是 Jimmy 的檔（heater-control skill §8）；St01 只整理與通報。
