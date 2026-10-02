# 台達 Delta DTM 多迴路模組化溫控器（重點 DTME08／DTMN08）（溫控器手冊參照）

> 手冊放在 Steven01 本機的 `E:\HT9045W_相關料件技術文件\溫控器\Delta溫控器\`（不在 repo 裡；其他電腦沒有 E: 這顆硬碟，要看原檔請到 Steven01）。

樹的代號：**golden V912**＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（BCB6，Big5）；**移植樹 V906**＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`（UTF-8）。

---

## 0. 結論先講：DTME08 是台達的

**DTME08＝台達 DTM 系列「Ethernet 型主機、8 通道」**，不是 Panasonic、也不是 Omron。證據：

| # | 證據 | 出處 |
|---|---|---|
| 1 | 型號表：主機 RS-485 型 `DTMR08／DTMR04`、**Ethernet 型 `DTME08／DTME04`**；量測擴充機 `DTMN08／DTMN04／DTMN02-x` | `DELTA_IA-SSM_DTM_C_EN_20211022_Web.pdf` PDF p.4（印刷頁 3）「Host: Ethernet Type DTME08 / DTME04」、PDF p.20（印 19）訂購表「DTME04 4-channel、DTME08 8-channel，RS-485 + Ethernet」；繁中型錄 `DELTA_IA-SSM_DTM_C_TC_20200730_web.pdf` PDF p.4、p.20 同 |
| 2 | 操作手冊直接用 DTME08 舉例：「欲使用 **DTME08** 的 Ethernet Modbus 讀取 8 通道的 PV 值，可下指令 `[FF 03 0268 0008]`」 | `DELTA_IA-TC_DTM_OM_SC_20210223.pdf` PDF p.17（印 2-7）§2.6.3 |
| 3 | 程式用的暫存器**逐一對得上** DTM 手冊（SV `0x000`、感測器 `0x028`、輸出 2 選擇 `0x0D0`、輸出 1 週期 `0x0F8`、AT `0x250`、PV `0x268`、狀態 `0x288`；站別 `x×0x1000`；PV 錯誤碼 `0x80xx`；感測器 0＝K、12＝Pt100） | 見 §3、§4 |
| 4 | golden 程式自己的命名：`mapDTMInfo`、`DTMTimeout`、主畫面鈕文字 `"DTM Temp."`、註解「add for DTM 32CH」 | golden V912 `main.cpp:10963`、`EJ1N/fDTME08.cpp:61`、`EJ1N/uDTME08Control.h:138` |
| 5 | HT9050 開發機資料（`HP-9050開發機資料-20260717.xlsx` 分頁 `04_溫控器站號`）寫「台達、Ethernet、DTME08／DTMN08」 | `D:\HT9045\.claude\skills\ht9050-hw\references\temp-dtm-map.md` §1、`..\data\HT9050-TempMap.json:10-12` |

反例（都是誤植）：golden V912 `EJ1N/fDTME08.cpp:163` 註解寫「Omron DTME08」（這個模組是從 Omron EJ1N 表單複製來的，面板類別也還叫 `TMyOmronPanel`）；移植樹 V906 `EJ1N/uDTME08Control.h:2`、`EJ1N/uDTME08Control.cpp:2` 檔頭寫「**Panasonic** DTME08」——**應改成 Delta（台達）DTM**（改檔不在本文件範圍）。

---

## 1. HT9045 在哪裡用

### 1.1 設定鍵與值

| 項目 | 內容 | 程式位置 |
|---|---|---|
| 啟用 | `Gerneral.ini [System] USE_16_HEATER`＝**5**（`eht16HeaterDTME08`，Index 16 組）或 **6**（`eht32HeaterDTME08`，Index 32 組） | golden V912 `MachineType.h:717-723`、`database.cpp:625`、`HandlerSys.cpp:260`、`:777` |
| 通道數 | 5→16 通道、6→32 通道（`SetChannelNumber`） | golden V912 `EJ1N/fDTME08.cpp:61-64` |
| 連線設定 | `DTME08_Control.ini [SocketSetting] asAddress／asPort`；**路徑＝exe 所在資料夾**（`ExtractFilePath(Application->ExeName)`），`D:\HT9045\system\` 那行是註解掉的 | golden V912 `EJ1N/uDTME08Control.h:54-57`、`EJ1N/uSocketServerClient.cpp:32-45`；程式內預設 `127.0.0.1:59999`（`uSocketServerClient.cpp:26-30`） |
| 本機檔 | `D:\HT9045\EXE\DTME08_Control.ini`：`asAddress=172.16.8.50`、`asPort=502`（`D:\HT9045\system\DTME08_Control.ini` **不存在**） | Steven01 |

本機 `D:\HT9045\system\Gerneral.ini:19` 目前 `USE_16_HEATER=2`（EJ1N），這台沒有啟用 DTME08。

### 1.2 通訊介面與參數

- **Modbus/TCP**，一條 socket（`uSocketClient`），Unit ID（SlaveID）固定 **1**（`EJ1N/uDTME08Control.cpp:345-351`）。
- 讀：功能碼 **03**、數量 **8**（`GetClientEncodeSingleTCP`，`EJ1N/uModbusCommand.cpp:30-41`）；寫：功能碼 **0x10（16）**、8 個 word（`GetClientEncodeWriteMultipleTCP`，`:43-59`）。
- 逾時 2 秒、斷線重連間隔 10 秒、輸出週期寫 2 秒（`EJ1N/fDTME08.h:164-166`）。

### 1.3 站號規則

- `GetMaxStationNumber()=4`、`GetChannelNumberPerStation()=8`（`EJ1N/uDTME08Control.h:107-108`）。
- **起始位址＝站別 × 0x1000＋暫存器**（`GetStationCode`：`istate*pow(16,3)`，`EJ1N/uDTME08Control.cpp:34-38`；站別 <0 或 >4 時變 0）。站別從 **0** 開始，16 通道掃 0、1，32 通道掃 0～3（`DoDTME08Cycle` case 9000，`EJ1N/fDTME08.cpp:656-668`）。
- 全域通道 `iCh` → 站別 `iCh/8`、站內 `iCh%8`（`GetStationAndNumber`，`EJ1N/fDTME08.cpp:266-270`）。
- Index 加熱器對應：`iTempCode[i]` 的第 i 個就是 DTM 全域通道 i：`tcAa1, tcBa1, tcAb1, tcBb1…`（前後排交錯，golden V912 `cmydef.cpp:111-117`）。

### 1.4 程式位置

| 動作 | golden V912 | 移植樹 V906 |
|---|---|---|
| 驅動（暫存器、編解碼） | `EJ1N/uDTME08Control.{h,cpp}`（380 行）、`EJ1N/uModbusCommand.{h,cpp}` | 同名檔已翻譯（`EJ1N/uDTME08Control.cpp:127-130` 站別、`:421-433` 暫存器、`:441` SlaveID）；設定檔路徑改成 `D:\HT9045\system\`（`EJ1N/uDTME08Control.h:135-142`，GATE 1） |
| 表單與輪詢狀態機 | `EJ1N/fDTME08.cpp`：建構 `:16-27`、`TimerUpdateTimer` `:158-176`（ATC6.0／主動冷卻時不跑）、`DoDTME08Cycle` `:475-676` | `forms/fDTME08.cpp`：大多是 `#if 0`（GATE E-1～E-33） |
| 啟動輪詢 | `main.cpp:10959-10966`（鈕文字改 `"DTM Temp."`、`TimerUpdate` 開） | — |
| 寫 SV／讀 PV 給溫控 | `bthermo.cpp:1212-1215` 呼叫 `DoSetSVOfDTME08`（`:4879-4958`）：對 32 個 Index 通道算 SV，`frmDTME08->SetSettingSV(i, Temp)`、`UN150Read = GetConvertTemp(GetPV())`（`:4953-4954`） | `bthermo.cpp:1311` 呼叫、`:5302` 定義；**寫 SV 與讀 PV 兩行在 `#if 0 // TODO(W7-UI G29)`（`:5396-5399`）**，所以移植樹不會控 DTM |
| 序列迴圈跳過 Index | `bthermo.cpp:1331-1367`：DTME08 版時 Index 32 區 `Task=300` 不走溫控 COM 埠 | — |

### 1.5 輪詢順序（golden V912 `EJ1N/fDTME08.cpp:475-676`）

1. 第一次：依序對每個站寫 **感測器種類 → 輸出 1 控制週期 → 輸出 2 控制選擇 → SV**（`:511-548`；感測器 `:352-368`、週期 `:397-406`、輸出 2 `:408-417`）。
2. 之後每 10 輪：第 1 輪讀狀態（0x288）、第 2 輪讀 SV（0x000）、第 3 輪「SV 有變就寫、沒變就讀 PV」、其餘讀 PV（0x268）（`:568-600`）。
3. 每站一個指令，站間 `Task=100→200→9000` 輪轉。

---

## 2. 手冊清單

| 絕對路徑 | 語言 | 版本／日期 | 內容 | 先讀哪幾頁 |
|---|---|---|---|---|
| `E:\HT9045W_相關料件技術文件\溫控器\Delta溫控器\DELTA_IA-TC_DTM_OM_SC_20210223.pdf` | 簡體中文 | V5.2（2021/2/05，改版紀錄 PDF p.2） | **DTM 溫度控制器操作手冊**（97 頁）：規格、站號、RS-485 DIP、Ethernet、輸入／輸出／警報、PID、**附錄通訊位址全表**、封包、LRC／CRC、DCISoft 設 IP | **PDF p.13～19（印 2-3～2-9）**、p.22～23（印 3-2～3-3）、p.28～29（印 4-1～4-2）、**p.60～64（印 7-1～7-5）**、p.71～73（印 7-12～7-14）、p.81～95（印 7-22～7-36，DCISoft） |
| `E:\HT9045W_相關料件技術文件\溫控器\Delta溫控器\DELTA_IA-SSM_DTM_C_EN_20211022_Web.pdf` | 英文 | 2021-10-22 | DTM 型錄（24 頁）：模組介紹、RS-485 DIP、站號旋鈕、Ethernet 規格、規格表、訂購資訊、DT 系列總覽 | PDF p.4～5（印 3～4）、**p.10（印 9）**、p.11（印 10）、p.16（印 15）、**p.20（印 19）**、p.21（印 20） |
| `E:\HT9045W_相關料件技術文件\溫控器\Delta溫控器\DELTA_IA-SSM_DTM_C_TC_20200730_web.pdf` | 繁體中文 | 2020-07-30 | 同上（繁中版） | PDF p.4、p.20 |
| `E:\HT9045W_相關料件技術文件\溫控器\Delta溫控器\DELTA_IA-SS_DT_C_TC_20180208_web.pdf` | 繁體中文 | 2018-02-08 | DT 系列型錄：DT3、DTK、DTA、DTB、DTC、DTD、DTE、DTV；**沒有 DTM**（DTM 是之後的產品） | 比較用：DTE（8 組 TC／6 組 RTD 模組化，RS-485 最高 115,200 bps）、DTC 模組化：PDF p.16～18、p.21（印 20 訂購） |

⚠ pdftotext 抓不到 OM、繁中型錄、DT 型錄、DTB 操作手冊這幾份中文 PDF 的中文字（字型沒有 Unicode 對應，只剩數字與英文），本文件的中文內容是把頁面算圖後看的。

---

## 3. 通訊重點（手冊內容）

出處縮寫：**OM**＝`DELTA_IA-TC_DTM_OM_SC_20210223.pdf`；**EN**＝`DELTA_IA-SSM_DTM_C_EN_20211022_Web.pdf`。頁碼寫「PDF 頁（印刷頁）」。

### 3.1 系統架構與站號

- 一個 DTM 群組＝**1 台主機＋最多 7 台量測擴充機＋8 台 I/O 擴充機，最多 64 點**；多群組用 RS-485 或 Ethernet 串（OM p.6（1-1）；EN p.2）。
- 主機 `DTMR`（RS-485）／`DTME`（RS-485＋Ethernet）各有 4／8 通道；量測擴充機 `DTMN08／04` **本身不通訊**（EN p.5（4）「Communication: N/A」），經主機內部匯流排讀寫。
- **內部站號 x**：主機固定 **0** 不能改；量測擴充機用旋鈕設 **1～F**（0＝工程模式，不要用）；同類型擴充機不能重複，不同類型可相同（EN p.10（9）表格與註 *2～*5；§2.4 站號設定從 OM p.13（2-3）起，p.14（2-4）範例：DTME08 內部站號 0、兩台 DTMN08 旋鈕 2 與 A）。
- 主機的 **RS-485 站號**＝旋鈕 1～F（旋鈕 0 → 站號 16）；DIP Bit 8 ON 時再＋64（EN p.10（9）；OM p.15（2-5））。
- 開機後主機約 **30 秒**收集各擴充機參數，**這段期間 RS-485、USB、Ethernet 都不能通訊**（OM p.18（2-8）§2.7）。ERR 燈閃＝量測擴充機超過 7 台、內部站號重複、內部匯流排讀寫錯（同頁）。

### 3.2 協定與預設值

| 介面 | 內容 | 出處 |
|---|---|---|
| RS-485 | Modbus **ASCII／RTU**（DIP Bit 1）；4,800～115,200 bps（Bit 2～4）；格式 7E1／7O1／7N1／8E1／8O1／8N1／7N2／8N2（Bit 5～7）；**出廠全 OFF＝ASCII、38,400、7,E,1** | OM p.15（2-5）；EN p.10（9） |
| Ethernet（DTME） | **Modbus TCP**（Server）、EtherNet/IP；**預設 IP 192.168.1.5、Port 502**；遮罩 255.255.255.0、閘道 192.168.1.1 | OM p.16（2-6）、p.88（7-29） |
| Modbus TCP 限制 | **最多 4 條連線**、單筆最多 **100 words** | OM p.16（2-6） |
| 閒置斷線 | Modbus TCP 連線保持時間 10～65,535 秒，**預設 30 秒**，閒置超過就斷 | OM p.88（7-29） |
| Unit ID | DTME 用 IP 分辨，「RS-485 站號」欄位**可填任意值**（範例填 FF） | OM p.17（2-7） |
| 設 IP | DCISoft（UDP 20006，PC 防火牆要放行；手冊的 PC 範例 IP 192.168.0.1 跟出廠 192.168.1.5 不同網段） | OM p.81～95（7-22～7-36） |
| 功能碼 | **H03 讀，最多 64 word；H06 寫 1 word；H10 寫多筆，最多 64 word** | OM p.60（7-1） |

### 3.3 通訊位址（OM p.60～64（7-1～7-5）；`Hx###` 的 **x＝內部站號**，主機 x=0；CH1～CH8 依序＋0～＋7）

| 位址 | 名稱 | 說明 |
|---|---|---|
| **Hx000～007** | **SV 值（讀寫）** | 0.1 °C/°F |
| Hx008／Hx010 | SV 上限／下限 | |
| Hx018／Hx020 | 輸入誤差調整／增益 | −999～+999 |
| **Hx028～02F** | **輸入感測器** | 代碼見 §3.4 |
| Hx040～Hx087 | 警報 1～3 模式／延遲／功能 | |
| Hx088～Hx0B7 | 警報 1～3 上下限 | |
| Hx0B8 | 控制方式 | 0 PID、1 ON-OFF、2 可程式 PID |
| Hx0C0 | 手動開關 | 0 自動、1 手動 |
| Hx0C8／**Hx0D0** | 輸出 1／**輸出 2 控制選擇** | 0 加熱（預設）、1 冷卻；**§4.1.3 另有 2＝通道禁能**（OM p.28（4-1）） |
| **Hx0F8～0FF** | **輸出 1 控制週期** | 0.1 秒、1～600、預設 5 秒（RELAY 20 秒）（OM p.62（7-3）） |
| Hx138 | 輸出 2 控制週期 | 同上 |
| **Hx248** | **執行／停止** | 0 停止、1 執行、2 程序結束、3 程序暫停（OM p.63（7-4）） |
| **Hx250** | **自整定 AT** | 0 停止、1 執行中 |
| Hx258 | 通道禁能 | Bit0～7＝CH1～8（0 關、1 開）（OM p.64（7-5）） |
| **Hx268～26F** | **PV 值** | 0.1 |
| Hx270～277 | SV 值（唯讀） | 0.1；可程式控制時是動態 SV |
| Hx278／Hx280 | 輸出 1／2 操作量 | 0.1% |
| **Hx288～28F** | **輸入通道狀態** | 1＝開啟：b7 自整定、b6 輸出 1、b5 輸出 2、b4 警報 1、b3 °F、b2 °C、b1 警報 2、b0 警報 3（OM p.63～64） |
| Hx2E1… | PID 參數 | OM p.64（7-5） |

OM p.23（3-3）附注 2：Hx000 可讀寫 SV，Hx270 只讀；從 Hx268 一次讀 16 筆就能同時拿到 PV 與 SV。

### 3.4 感測器代碼（OM p.22～23（3-2～3-3））與 PV 錯誤碼（OM p.23（3-3））

- 0 K、1 J、2 T、3 E、4 N、5 R、6 S、7 B、8 L、9 U、10 TXK、11 JPt100、**12 Pt100**（−200～850 °C）、13 Ni120、14 Cu50、15～19 類比、20 C；**出廠 K**。範例：把內部站號 2 的 CH3 設成 Pt100＝寫 `H000C` 到 `H202A`（OM p.22（3-2））。
- PV 錯誤：**H8001** EEPROM 無法寫入、**H8002** 感測器斷線或未接、**H8003** ADC 讀取失敗、**H8004** 內部通訊錯誤、**H8005** 輸入錯誤、**H8006** 通道禁能、**H8007** 輸入資料未穩定。

### 3.5 檢查碼

- ASCII：LRC＝「機器位址」加到「資料內容」、捨去進位、取 2 補數；例 `H01+H03+H41+HFF+H00+H02=H146`→`H46`→**`HBA`**（OM p.72（7-13））。封包格式表 OM p.71（7-12）。
- RTU：CRC 初值 FFFFH、多項式 A001H、低位元組先送，附 C 範例（OM p.73（7-14））。
- Modbus TCP 不用 LRC／CRC。**手冊沒有列 Modbus 例外回應碼。**

---

## 4. 與程式對照（golden V912 `EJ1N\`）

| # | 項目 | 手冊 | 程式 | 判斷 |
|---|---|---|---|---|
| 1 | 站別位址 | `Hx###`，x＝內部站號，主機 0 | `x*0x1000`，x＝0～3 | ✓ 但隱含**接線規則**：32 通道＝DTME08（x=0）＋3 台 DTMN08 旋鈕 **1、2、3**；16 通道＝DTME08＋DTMN08 旋鈕 **1**。golden 沒寫，換機或換模組要照設。 |
| 2 | 功能碼／數量 | H03／H10 最多 64 word；TCP 單筆 100 words | 03 讀 8、0x10 寫 8（`uModbusCommand.cpp:30-59`） | ✓；MBAP 長度欄（6；13+2n−6）也正確 |
| 3 | Unit ID | 任意值 | 1（`uDTME08Control.cpp:348`） | ✓ |
| 4 | SV | Hx000 讀寫，0.1 | 寫 `0x000`、讀回也用 `0x000`（`:83`、`:237`、`:325-330`） | ✓。`DoGetSV` 用了 `efc_SetSV`、`DoSetSV` 用了 `efc_GetSV`，兩個值都是 0x000，對調不影響。`SetSettingSV` 用 `(int)(dvalue*10)` **截斷**不四捨五入（`uDTME08Control.h:33`），可能差 0.1。 |
| 5 | PV | Hx268，0.1，有號 | `0x268`、`TwoBytes2Short`、×0.1（`uDTME08Control.cpp:151-152`） | ✓ |
| 6 | PV 錯誤碼 | H8001～H8007 | 高位元組 0x80 時查 `ErrorCodeDescription`（`:143-147`、`:353-364`） | ⚠ **`:361` 把 0x02 寫了第二次**（應是 0x07「輸入資料未穩定」），H8007 會顯示「未知異常」。 |
| 7 | 回應太短 | 手冊無例外碼說明 | 長度 <25 時把最後一個位元組丟進 PV 錯誤表（`:135-140`） | ⚠ Modbus 例外回應（功能碼加 0x80＋例外碼，Modbus 規範，非本手冊）會被誤顯示成 PV 錯誤，例外碼 02 會變「感測器斷線」。 |
| 8 | 狀態位元 | Hx288：b0 ALM3、b1 ALM2、b2 °C、b3 °F、b4 ALM1、b5 OUT2、b6 OUT1、b7 AT | `GetStatus` 陣列順序完全相同（`:312-319`） | ✓。但 `AnalysisChannelStatus` 迴圈條件用 OR（`while(iInput!=0 \|\| inum<iBoolLen)`，`:304`），狀態字若有 b8 以上的位元會寫出 8 格陣列外（手冊只定義 b0～b7）。 |
| 9 | 感測器 | 0＝K、12＝Pt100 | `esstKType=0`、`esstPT100=12`（`uDTME08Control.h:74-78`） | ✓。**每站 8 個通道一律設成同一種**（`fDTME08.cpp:352-368`，看 `rgSensorType`）。 |
| 10 | 輸出 1 週期 | Hx0F8，0.1 秒，1～600 | 寫 20（2.0 秒）（`fDTME08.cpp:397-406`、`uDTME08Control.h:38`） | ✓ 單位相符 |
| 11 | 輸出 2 選擇 | Hx0D0：7-2 表只寫 0／1；4-1 寫 2＝通道禁能 | 寫 `ecaDisable`＝2（`fDTME08.cpp:408-417`） | ✓（以 OM p.28（4-1）為準；手冊兩處不一致） |
| 12 | AT | Hx250：0 停、1 執行 | `0x250`，0／1（`:240-251`、`:327-328`） | ✓ |
| 13 | RUN／STOP | Hx248 | **程式完全不寫 Hx248** | 依賴模組本身狀態。⛔ 20261001 更正（ST01-E3 對手冊）：出廠是開機就 RUN（OM 2.7.2，Hx1E6 bit0＝0，PDF 19）；RUN 燈只代表任一通道在跑，逐通道要用通訊讀 Hx248。DFM 上有「Set RUN / STOP」鈕但 `uDTME08Control` 沒有對應功能碼。 |
| 14 | 開機 30 秒 | 期間不能通訊 | 第一輪初始設定（感測器、週期、輸出 2、SV）每站逾時 2 秒就往下走，`iDoInitialCommandStatus++` 不管成敗（`fDTME08.cpp:511-548`）；`DoSetValue` 只等「有回應」不看內容（`uDTME08Control.cpp:200-225`） | ⚠ DTM 還在 30 秒收集期時，初始設定可能整批沒寫進去且不會重送（之後只有 SV 會因 `IsNeedSetSV` 重寫）。 |
| 15 | IP／Port | 預設 192.168.1.5:502 | ini 讀 exe 資料夾的 `DTME08_Control.ini`；本機 172.16.8.50:502 | Port ✓；IP 要先用 DCISoft 改。 |
| 16 | 設定檔路徑 | — | golden：**exe 資料夾**（`uDTME08Control.h:54`）→ `D:\HT9045\EXE\DTME08_Control.ini`；移植樹 V906：`D:\HT9045\system\`（`EJ1N/uDTME08Control.h:135-142`） | ⚠ 兩樹讀不同檔；`D:\HT9045\system\DTME08_Control.ini` 在 Steven01 不存在，移植樹會落到預設 `127.0.0.1:59999`。 |
| 17 | 連線數 | Modbus TCP 最多 4 條 | 1 條 | ✓；但 DCISoft／其他 PC 同時連時要留額度。閒置 30 秒會被斷，程式持續輪詢所以不會閒置。 |
| 18 | 啟動條件 | — | `main.cpp:10959-10961`：`TC401HeaterControl!=NoHeater && USE_16_HEATER==5 \|\| USE_16_HEATER==6` | 程式問題（與手冊無關）：AND 比 OR 先算，`USE_16_HEATER==6` 時不看 NoHeater。 |

---

## 5. 注意事項（含 HT9050）

1. **HT9050 用 DTM 的方式比 HT9045 多**（`D:\HT9045\.claude\skills\ht9050-hw\references\temp-dtm-map.md`）：3 站 24 通道——站 1 DTME08（Hotplate1/2、In Shuttle1/2、DUT1～4，PT100，SSR）、站 2 DTMN08（Chamber PT100 SCR、Hot Air 1/2 **K-type** SCR）、站 3 DTME08（SLK-1～8，PT100）。HT9045 的 DTM 只管 Index 32 組（`iTempCode[]`），HP／Shuttle／DUT／Chamber 走序列埠；HT9050 要把它們也搬到 DTM。Steven 20260928（R113）：「新的機台架構是改用全機 DTM」（移植樹 V906 `FileRW/HSys.cpp:632-633`；`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md:1120`）。
2. ⚠ **HT9050 表上「站 3＝DTME08」與手冊架構衝突**：一個群組只有一台主機（OM p.6（1-1）），主機內部站號固定 0（EN p.10（9）註 *3）。第二台 DTME08 是**另一個群組、另一個 IP**，目前 `uDTME08Control` 只有一條 socket、用 `x*0x1000` 分站，**定址不到它**。要嘛站 3 其實是 DTMN08（那它的旋鈕要設 2，站 2 的 DTMN08 旋鈕要設 1，才會對上 `iCh/8` 的算法），要嘛程式要多開一條連線。要問硬體確認。
3. ⚠ HT9050 站 2 混用 PT100 與 K-type，但現有程式每站 8 通道只能設同一種感測器（§4 #9）；要逐通道寫 Hx028～02F。
4. HT9050-TempMap.json 把 DTMN08 的介面寫成 Ethernet；實際上 DTMN08 沒有通訊埠，是透過 DTME08 主機（EN p.5（4））。
5. 上機前檢查：DTME08 IP／遮罩（DCISoft）、DTMN08 旋鈕（1、2、3…不可重複、不可 0）、RUN 燈（出廠開機就 RUN；逐通道只能用通訊讀 Hx248）、感測器種類（出廠是 K）、開機等 30 秒再連。
6. DT 系列總覽（EN p.21（印 20））：DTM（多迴路模組）、DTA（標準）、DTB（進階）、DTV（閥控）、DT3（高階模組化）、DTC（模組化串接）、DTE（多通道模組）、DTK（智能型）。2018 繁中 DT 型錄沒有 DTM。
