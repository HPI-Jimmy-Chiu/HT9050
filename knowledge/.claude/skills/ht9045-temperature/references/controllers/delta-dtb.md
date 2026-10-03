# 台達 Delta DTB 系列（重點 DTB4824）（溫控器手冊參照）

> 手冊放在 Steven01 本機的 `E:\HT9045W_相關料件技術文件\溫控器\Delta溫控器\`（不在 repo 裡；其他電腦沒有 E: 這顆硬碟，要看原檔請到 Steven01）。

樹的代號：**golden V912**＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（BCB6，Big5）；**移植樹 V906**＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`（UTF-8）。

---

## 1. HT9045 在哪裡用

### 1.1 用途：自動 K 溫（Auto Temperature Calibration）的「溫度計」

DTB4824 **不是拿來控溫**，而是 Steven 20210510 加的「自動 K 溫」表單 `TACTForm` 裡的**量測器**：把熱電偶貼在各量測點，HT9045 依序把機台設到 Low／Mid／High 三個基準溫度（`pnlBaseTemp01～03`），輪詢每顆 DTB4824 的 PV，依實測與基準的差自動調整 `Temperature.fTempOffSet[][]`（溫度偏移）。量測器另一個選項是 Agilent 34970A。

| 項目 | 內容 | 程式位置 |
|---|---|---|
| 量測器種類 | `enum eACTThermoType{Type_Agilent34970A=0, Type_DeltaDTB4824=1};` | golden V912 `AutoTemperature.h:32` |
| 進入點 | 主畫面 `sbAutoTemp` 鈕 → 權限 49 → `ACTForm->Show()` | golden V912 `main.cpp:29883-29890` |
| 按鈕顯示條件 | `CosFunction.bAutoKTemp`（依客戶碼設 true，例 `CosFunction.cpp:31`、`:846`、`:2804`） | golden V912 `CosFunction.cpp` |
| 設定檔 | `D:\HT9045\System\AutoTemperature.ini`：`[ACT] iThermoCtrlType`（預設 1＝DTB4824）、`CheckIntervalTime`、`CalibrationRange`、`dCalibrationRange[0..2]`、`ReadScanTime`、`OffsetLimit`、`SingleOffsetLimit`、`OffsetMethod`；`[Display] Colums`；`[COMPort] CommName／BaudRate／ByteSize／StopBits／Parity` | golden V912 `AutoTemperature.cpp:398`（路徑）、`:834-885`（`SaveACTData`／`LoadACTData`，`LoadACTData` 從 `:860` 起）、`:1275-1298`（`SaveCommData`／`LoadCommData`） |

哪一台機台實際裝了 DTB4824，程式裡看不出來（只看 `bAutoKTemp` 與 ini）。另一份資料 `E:\HT9045W_相關料件技術文件\溫控器\溫控器選用差異表20181025.pptx` slide 2 寫「DTB4824＝ATC3.2 水溫保護」（同資料夾 `index.md` §2.3 的摘錄；本文件未另外核對），那是 ATC 端的用途，跟 HT9045 程式裡的自動 K 溫無關。Steven01 本機 `D:\HT9045\system\AutoTemperature.ini` 沒有 `iThermoCtrlType` 鍵（→ 用預設 1＝DTB4824），`[COMPort]` 是 `COM3／9600／ByteSize=3（_8）／StopBits=0（_1）／Parity=0（None）`。

### 1.2 通訊介面與參數

- **獨立的序列埠** `ACTCom`（TComm），**不是**溫控共用的 `COM2->Comm2`。
- 預設 **COM3、9600 bps、8 data、None、1 stop**（`LoadCommData`，golden V912 `AutoTemperature.cpp:1287-1298`；DFM 設計值 `AutoTemperature.dfm:994-1011` 是 COM1／9600／8／None，開頁時被 ini 蓋掉）。可在表單上改並存回 ini（`btnUpdateClick`，`:692-742`）。
- 協定：**Modbus ASCII**，LRC 借用 DTK 的 `DTK4848_LRC`（golden V912 `cpublic.cpp:130-145`）。

### 1.3 站號規則

- 量測點清單（DTB4824 模式）共 **33 點**：`HP 1-1～1-5`、`HP 2-1～2-5`、`SHT 1-1／1-2`、`SHT 2-1／2-2`、`Base ×4`（依 `iSocketBaseTempCount` 對 DUT1～4 或 Socket）、`Chamber 1～3`、`HotGun 1／2`、`Reserved 1～10`（golden V912 `AutoTemperature.cpp:313-368`）。
- **站號＝量測點序號＋1**，`%02X`：點 0（HP 1-1）→ `01`，點 32（Reserved 10）→ `21`（`DTB4824_ReadPV`，`:1363-1373`）。
- 量測點各自對應到哪個溫控群組 `_Group`（例 HP 1-1～1-5 都算 `tcHotPlate1`），偏移以群組為單位計算（`:313-397` 的 `iGroupSet[]`；群組統計欄位 `dGroupMax／Min／Sum／Avg` 在 `AutoTemperature.h`）。

### 1.4 程式位置

| 動作 | golden V912 `AutoTemperature.cpp` | 說明 |
|---|---|---|
| 送讀 PV | `:1363-1373` `DTB4824_ReadPV`：`":%02X03%04d0002" + LRC + "\r\n"`，暫存器 `4700`、2 個 word；送出後 `tReadTimeOut` 1 秒（`:1371`） | 只讀，不寫 DTB |
| 輪詢 | `:935-967`（`TimerACTTimer` 內）：收到 → 下一點；逾時重送，**第 3 次逾時跳下一點**（`:957`）；33 點全掃，不管該點有沒有勾 Use | |
| 收資料 | `:583-594`（`ACTComReceiveData`）：取第 8～11 字元 `HexStrToInt(...)/10.0`；`<0.001` 當 0 | 不驗 LRC、不驗站號 |
| 顯示 | `UpdateTemperatureData`（`:809-832`）：≥200 顯示 `ERROR` | |
| 開頁 | `FormShow`（`:419-532`）：DTB4824 時把每點的 `cbChannel` 藏起來（`:495-499`） | |

⚠ golden 時序怪點：建構子 `:297` 先 `LoadACTData`，`:398` 才設 `FilePath`，所以開機那次讀不到檔、`iThermoCtrlType` 一定是預設 1 → **建構子永遠建 DTB4824 的 33 點面板**；開頁 `:428` 才真正讀 ini（移植樹 V906 `FileRW/ACTForm.cpp:18-23` 已記錄，照 golden 保留；St01 意見見 `D:\HT9045\.claude\skills\ht9050-construction\references\archive\decisions-decided-202609.md:669`）。

### 1.5 移植樹 V906 現況

- 只有「讀寫檔那一半」：`FileRW/ACTForm.cpp`（`FileRW_ACTForm_Boot／FormShow／btnUpdateClick／palSaveLogClick`），列舉與 DFM 字串在 `FileRW/ACTForm.gen.inc:30`、`:143`、`:210`。
- **通訊沒有移植**：`OpenCommPort`、`SendCommand`、`ACTComReceiveData`、`TimerACTTimer`、`DTB4824_ReadPV` 都列在「不在本檔，交 Jimmy」（移植樹 V906 `FileRW/ACTForm.cpp:31-35`）。也就是移植樹目前不會跟 DTB4824 通訊。

---

## 2. 手冊清單

| 絕對路徑 | 語言 | 版本／日期 | 內容 | 先讀哪幾頁 |
|---|---|---|---|---|
| `E:\HT9045W_相關料件技術文件\溫控器\Delta溫控器\DTB_Manual_Eng.pdf` | 英文 | 未標日期；顯示畫面寫 Firmware V1.50（PDF p.13） | DTB 使用手冊（13 頁）：選購、操作、參數流程、PID 程式控制、**RS-485 暫存器全表**、尺寸、**錯誤碼** | **PDF p.9（印刷頁 8）**、**p.10（印 9）**、**p.13（印 12）** |
| `E:\HT9045W_相關料件技術文件\溫控器\Delta溫控器\DTB 操作手冊.pdf` | **簡體中文**（檔名寫「操作手冊」） | 頁首條碼 `200412-10`、料號 5011628600-DBS0 | 單張 2 頁：選購、規格、參數、感測器表、警報、CT、EVENT、PID 程式控制、**RS-485（暫存器表＋ASCII／RTU 封包範例＋LRC／CRC 算法）**、尺寸、端子 | **PDF p.2**（中間欄 RS-485、右欄端子圖） |
| `E:\HT9045W_相關料件技術文件\溫控器\Delta溫控器\DELTA_IA-TC_DTB_I_TSET_20140421.pdf` | 繁中／簡中／英／土耳其文 | 2014-04-21，料號 5011628508-DBC8 | DTB 操作說明書（單張 2 頁），選購碼多了第 8 碼供電（D＝DC24V） | PDF p.1 **左半＝繁中**（「RS-485 通訊」在第 2 欄下方）；PDF p.2 左半＝英文 |
| `E:\HT9045W_相關料件技術文件\溫控器\Delta溫控器\DELTA_IA-SS_DT_C_TC_20180208_web.pdf` | 繁中 | 2018-02-08 | DT 系列型錄 | DTB：PDF p.15（印 14，電氣規格）、PDF p.21（印 20，選購碼與 DTB4824 備註） |

---

## 3. 通訊重點（手冊內容）

### 3.1 協定與通訊參數

- RS-485，**2,400／4,800／9,600／19,200／38,400 bps**；**不支援 7,N,1、8,O,2、8,E,2**（`DTB_Manual_Eng.pdf` PDF p.9，印 8，第 1～2 點；`DTB 操作手冊.pdf` PDF p.2 第 1 點）。
- **Modbus ASCII 或 RTU**（同上第 3 點）。
- 功能碼：**03H 讀暫存器（最多 8 word）、06H 寫 1 個 word、02H 讀位元（最多 16 bit）、05H 寫 1 個位元**（`DTB_Manual_Eng.pdf` PDF p.9 第 4 點；PDF p.10 第 7 點再列一次）。
- 參數名稱（設定模式）：通訊寫入許可、ASCII／RTU 選擇、通訊位址、鮑率、資料長度、同位元、停止位元（`DTB_Manual_Eng.pdf` PDF p.4～5，印 3～4 的流程圖）。文字層**沒寫站號範圍與出廠預設通訊格式**。
- DTB4824：1/32 DIN（W48×H24 mm），**沒有選購功能、沒有額外警報輸出，可把第 2 組輸出設成警報**（`DTB_Manual_Eng.pdf` PDF p.2 Note 1；型錄 PDF p.21，印 20）。RS-485 端子：DTB4824 為 11（DATA+）、12（DATA−）（`DTB 操作手冊.pdf` PDF p.2 右欄「連接端子」圖面判讀）。

### 3.2 字組暫存器（`DTB_Manual_Eng.pdf` PDF p.9，印 8；繁中版在說明書 PDF p.1）

| 位址 | 內容 | 說明 |
|---|---|---|
| **1000H** | PV 目前溫度 | 0.1 刻度，每 0.4 秒更新；錯誤時讀值為 **8002H** 初始中（還沒有溫度）、**8003H** 感測器未接、**8004H** 感測器輸入錯誤、**8006H** ADC 輸入錯誤、**8007H** 記憶體讀寫錯誤 |
| **1001H** | SV 設定值 | 0.1，°C 或 °F |
| 1002H／1003H | 溫度範圍上／下限 | 不得超出感測器範圍 |
| 1004H | 輸入感測器種類 | 見感測器表 |
| 1005H | 控制方式 | 0：PID、1：ON/OFF、2：手動、3：PID 程式控制 |
| 1006H | 加熱／冷卻選擇 | 0：加熱、1：冷卻、2：加熱/冷卻、3：冷卻/加熱 |
| 1007H／1008H | 第 1／2 組控制週期 | 0～99（0＝0.5 秒） |
| 1009H／100AH／100BH | P／Ti／Td | 0.1～999.9／0～9999／0～9999 |
| 1012H／1013H | 輸出 1／2 輸出量 | 0.1%，只在手動模式可寫 |
| 1016H | 溫度誤差調整值 | −999～+999，0.1 |
| 1020H～1029H | 警報 1～3 模式與上下限 | 見警報章節 |
| 102AH | LED 狀態 | b0 ALM3、b1 ALM2、b2 °F、b3 °C、b4 ALM1、b5 OUT2、b6 OUT1、b7 AT |
| 102CH | 面板鎖定 | 0 正常、1 全鎖、11 只開放 SV |
| 102DH／102FH | CT 讀值（0.1 A）／軟體版本（0x100＝V1.00） | |
| 2000H～20BFH | PID 程式控制樣式的溫度與時間 | PDF p.10（印 9） |

### 3.3 位元暫存器（`DTB_Manual_Eng.pdf` PDF p.10，印 9；寫 FF00H＝1、0000H＝0）

| 位址 | 內容 |
|---|---|
| **0810H** | **通訊寫入選擇：0＝禁止（預設）、1＝允許** |
| 0811H | 溫度單位：1＝°C／線性（預設）、0＝°F |
| 0812H | 小數點位置（B、S、R 型以外可設 0 或 1） |
| 0813H | AT：0＝OFF（預設）、1＝ON |
| **0814H** | **RUN／STOP：0＝停止、1＝執行（預設）** |
| 0815H／0816H | PID 程式控制停止／暫停 |

2014 版說明書的位元表沒列 0810H，只列 0811H、0813H～0816H（`DELTA_IA-TC_DTB_I_TSET_20140421.pdf` PDF p.1 繁中、p.2 英文），但參數表仍有「通訊寫入許可／禁止」。

### 3.4 4700H／4750H 相容位址與錯誤狀態（`DTB_Manual_Eng.pdf` PDF p.13，印 12「Error Acknowledge and Display」）

| Error status 102EH／4750H | PV read back 1000H／4700H | 意義 |
|---|---|---|
| 0001H | N/A | PV 不穩定 |
| 0002H | 8002H | 重新初始化，還沒有溫度 |
| 0003H | 8003H | 輸入感測器未接 |
| 0004H | 8004H | 輸入訊號錯誤 |
| 0005H | N/A | 超出輸入範圍 |
| 0006H | 8006H | ADC 失敗 |
| 0007H | N/A | EEPROM 讀寫錯誤 |

→ **4700H 是 1000H（PV）的另一個讀取位址，4750H 是 102EH（錯誤狀態）的另一個位址**。這是資料夾裡唯一提到 47xxH 的地方；另外兩份 DTB 說明書都沒有。

### 3.5 封包與檢查碼（`DTB 操作手冊.pdf` PDF p.2）

- ASCII：`':'`＋位址 2 字元＋功能碼 2 字元＋資料＋LRC 2 字元＋CR LF。
- **LRC**＝從「機器位址」加到「資料內容」，取 2 的補數。範例 `01H+03H+10H+00H+00H+02H=16H` → LRC `EA`。
- RTU 讀取範例：`01 03 10 00 00 02 C0 CB`；**CRC**＝初值 FFFFH、與 A001H 做 XOR 右移 8 次，傳送時低位元組在前。
- Modbus 例外回應碼：三份 DTB 手冊**都沒寫**。

---

## 4. 與程式對照

| # | 項目 | 手冊 | 程式（golden V912 `AutoTemperature.cpp`） | 判斷 |
|---|---|---|---|---|
| 1 | PV 位址 | 1000H；英文手冊另寫 4700H 為 PV 讀取別名（PDF p.13） | 讀 **4700H** 起 2 個 word（`:1365-1369`） | ✓ 4700H 有手冊依據（V1.50 版英文手冊）。但**第 2 個 word（4701H）沒有任何 DTB 手冊記載**；程式也沒用它（只取第 8～11 字元，`:587`），影響僅是多讀一個。若遇舊韌體不認 47xxH，改 1000H 最保險（三份手冊都有）。 |
| 2 | 位址字串 | 4 位十六進位 | `%04d` 印出 `4700` | 字元剛好等於 0x4700，碰巧正確（同 DTK 寫法）。 |
| 3 | 功能碼 | 03H 讀、最多 8 word | 03H、2 word | ✓ |
| 4 | 站號 | 文字層沒寫範圍 | `%02X`、量測點序號＋1（1～33） | 格式正確；33 顆 DTB 的 `通訊位址` 要依序設 1～33（0x01～0x21）。 |
| 5 | 比例 | 0.1 刻度 | `HexStrToInt(...)/10.0`（`:588`） | ✓ |
| 6 | 錯誤碼 | 8002H～8007H 表示錯誤 | 無號解析 → 8002H 變 3277.0，≥200 顯示 `ERROR`（`:809-832`） | 結果上會顯示 ERROR（剛好），但不分是哪種錯誤；負溫度也會變成 6000 多（例 FFF6H→6552.6）→ ERROR。 |
| 7 | 通訊格式 | 不支援 7N1、8O2、8E2；ASCII／RTU | 9600、8N1、ASCII | ✓ 8N1 在支援範圍。DTB 本體要設 ASCII、9600、8 bit、無同位、1 stop。 |
| 8 | LRC | 位址～資料加總取 2 補數 | 借 `DTK4848_LRC`（只算「:」後 12 字元，`cpublic.cpp:133`） | ✓ 讀命令剛好 12 字元，算法相符。 |
| 9 | 回應驗證 | ASCII 有 LRC；回應含站號 | 不驗 LRC、不驗站號、不看功能碼，假設整幀在一次 `OnReceiveData` 收齊（`:583-594`） | ⚠ 逾時後才到的晚回應會被算到下一點；分段收到的幀會解析錯。手冊沒規定，但是程式風險。 |
| 10 | 寫入 | 0810H 預設禁止寫入 | 程式不寫 DTB | 不受影響（純量測）。 |
| 11 | 移植樹 | — | 通訊整段未移植（移植樹 V906 `FileRW/ACTForm.cpp:31-35`） | 移植樹目前讀不到 DTB4824。 |

---

## 5. 注意事項

1. DTB4824 在 HT9045 只當**量測器**，不控溫；接在自己的 COM 埠（預設 COM3），跟溫控共用埠（`[TempCtrl] COM_PORT`）是兩回事。
2. 輪詢不管量測點有沒有勾 Use 都會問（`:935-967`），沒裝 DTB 的站號每次要等 3 次 × 1 秒逾時才跳過；33 點全掃一輪可能很久。現場只裝部分點時要留意掃描時間。
3. 自動 K 溫會**改溫度偏移**（`Temperature.fTempOffSet`，透過 `fMain->SetTemp`／`ChangeTempMode`，`:745-792`、`:982-1270` 的 `bAutoStart` 段，例 `:998`、`:1084`、`:1191`），移植樹 V906 把這段交 Jimmy（`FileRW/ACTForm.cpp:32-33`），移植時要當成「會動溫控參數」的功能處理。
4. 若要寫 DTB（例如改 SV），先把 0810H（通訊寫入）打開；手冊寫預設禁止。
5. 「DTB 操作手冊.pdf」實際是**簡體中文**、2004 年左右的舊版（頁首 `200412-10`）；暫存器以英文版（Firmware V1.50）和 2014 版說明書為準。
