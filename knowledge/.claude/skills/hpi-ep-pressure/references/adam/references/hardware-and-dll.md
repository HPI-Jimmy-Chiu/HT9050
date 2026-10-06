> 保存來源：`.claude/skills/ht9045-adam6024/references/hardware-and-dll.md`，main `84233b648`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# ADAM-6024 硬體、EP 安裝型態、APAX、ADAMTCP.dll（細節）

> 本檔是 `../SKILL.md` §1～§2 的細節。頁碼 `p.N` 一律是 **PDF 第 N 頁**（Read 工具的 pages 參數），印刷頁碼 = N-10。
> 手冊：`E:\HT9045W_相關料件技術文件\ADAM\ADAM-6000_Series_Manual_Ed2.pdf`（下稱「手冊」）。
> golden 912 = `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（Big5，用 cp950 讀）。

## 1. ADAM-6024 本體（研華 12 通道萬用 I/O 模組）

| 項目 | 內容 | 出處 |
|---|---|---|
| 通道 | 6 AI（差動、16-bit）、2 AO（12-bit）、2 DI、2 DO | 手冊 p.41-43 |
| AI 範圍 | ±10 VDC、0~20 mA、4~20 mA；每通道可各自設定 | 手冊 p.41-42 |
| AO 範圍 | 0~10 VDC、0~20 mA、4~20 mA；精度 ±0.1% FSR；電流負載 0~500 Ω | 手冊 p.42 |
| DI | 乾接點／濕接點（0~3 V = 0，10~30 V = 1） | 手冊 p.43 |
| DO | 開集極 30 V／100 mA | 手冊 p.43 |
| 電源 | 10~30 VDC，4 W @24 VDC；內建 Watchdog | 手冊 p.43 |
| 跳線預設 | AI＝電壓、AO＝電流 | 手冊 p.44（Figure 4.8） |
| 通訊 | 10/100 Base-T；Modbus/TCP、TCP/IP、UDP、HTTP、ICMP、ARP | 手冊 p.42 |
| 出廠 IP | 第一次使用是 `10.0.0.1`，要用 ADAM.NET Utility 改到機台網段 | 手冊 p.77 |
| TCP 連線上限 | 一顆模組最多 **8 條** TCP 連線；Network 頁的 Host Idle（Timeout）到時會關掉閒置連線 | 手冊 p.79 |
| AO 開機值 | AO 的 startup value（上電輸出值）只能在 Utility 的 Output 頁設定 | 手冊 p.91 |
| 韌體 | 6024 **不支援網頁升級**（.html/.jar）；用 Utility 的 Firmware 頁更新（步驟見 `E:\HT9045W_相關料件技術文件\ADAM\FAE_SOP_ADAM-6000 Module Firmware upgrade.pdf` 第 1～5 頁；需要模組密碼，見手冊 p.80，本 skill 不抄密碼值） | 手冊 p.80 |

### 1.1 Modbus 位址（手冊 p.240，B.2.4 ADAM-6024）

| 位址 | 通道 | 內容 | 屬性 |
|---|---|---|---|
| 00001～00002 | DI0～DI1 | DI 值 | R |
| 00017～00018 | DO0～DO1 | DO 值 | R/W |
| 40001～40006 | AI0～AI5 | AI 值 | R |
| **40011** | **AO0** | AO 值 | R/W |
| **40012** | **AO1** | AO 值 | R/W |
| 40021～40026 | AI0～AI5 | AI 狀態（0 正常／1 過高／2 過低） | R |

golden 寫 AO 用 `ADAMTCP_WriteReg(ip, 1, 11+iAdd, 1, &data)`（912 `adam6024.cpp:1994`），起始位址 11 / 12 就是 40011 / 40012（4X 去掉前綴）：
`iAdd=0` → AO0，`iAdd=1`（預設）→ AO1。AO 是 12-bit，所以 golden 的滿刻度是 4095（`TransformFuntion` 的 `fMaxUnit`，912 `adam6024.cpp:1127-1130`）。

### 1.2 ASCII 指令（UDP，經 `ADAMTCP_SendReceive6KUDPCmd`）

| golden 送的字串 | 作用 | golden 912 行 | 手冊 |
|---|---|---|---|
| `$01M\r` | 讀模組名稱，回 `!01<名稱>`；golden 取第 3 字元起，要等於 `"16024-D"` | `adam6024.cpp:2768`、`:2779`、`:2915` | p.135-136（`$aaM`，範例 `!016050`） |
| `$01F\r` | 讀韌體版本，回 `!01 <版本>`；golden 取第 5 字元起 | `adam6024.cpp:2793`、`:2804` | p.136-137（範例 `!01 1.01`） |
| `$01A<cc><rr>\r` | 設 AI 通道範圍 | `adam6024.cpp:147` | Ed2 手冊**沒有**這條（新韌體指令，無法在手冊確認） |
| `$01B<cc>\r` | 讀 AI 通道範圍，回應第 4 字元起兩個 hex | `adam6024.cpp:172-190` | 同上，手冊沒有 |
| `$01C<cc><rr>\r` | 設 AO 通道範圍 | `adam6024.cpp:200` | 同上，手冊沒有 |
| `$01C<cc>\r` | 讀 AO 通道範圍（golden 讀、設用同一個字首 `C`） | `adam6024.cpp:225` | 同上，手冊沒有 |
| `%01GETMBTCPCN\r` | 讀目前 Modbus/TCP 連線數，回 `!` 後第 3 字元是個位數 | `adam6024.cpp:2819`、`:2829` | 手冊沒有 |

⚠ `ClearAllConnection`（912 `adam6024.cpp:2844-2863`）送的**也是** `%01GETMBTCPCN`，只看回應是不是 `!` 開頭——名字叫 Clear，**實際沒有清任何連線**。真正會釋放連線的是模組的 Host Idle 逾時（手冊 p.79）。照 golden，不要「修」。

range 代碼（golden 自己的 enum，912 `adam6024.cpp:57-71`）：AI `7`=4~20 mA、`8`=±10 V、`13`=0~20 mA；AO `0`=0~20 mA、`1`=4~20 mA、`2`=0~10 V。

## 2. 手臂機台怎麼用這顆模組（golden 912）

| 用途 | 通道 | golden 912 出處 |
|---|---|---|
| 主 EP 設定值（Contact Force，下壓力道） | AO1（reg 12） | `ADAM_DirectWriteData(code, 0)` 預設 `iAdd=1`，`adam6024.h:23`、`adam6024.cpp:1994` |
| Die Force EP（`INSTALL_DOUBLE_EP=1`） | AO0（reg 11） | `ADAM_DirectWriteData(code, 0, 0)`：`main.cpp:6519`（Start）、`:21729`（Timer2） |
| Two EP（`EP_Install=5`）Arm1／Arm2 | AO0／AO1 | `adam6024.cpp:2010-2022`（`iAdd` 10→reg 11、11→reg 12，其他兩個都寫） |
| 主 EP 回授電壓 | AI5 | `ADAM_ReadPA(double*, int iCH=5)`，`adam6024.h:8` |
| Die Force EP 回授 | AI2 | `adam6024.cpp:519-524`、`:2722` |
| 露點計 4~20 mA | AI3 | `adam6024.cpp:475-485`（非三溫機：`wHex[3]/4095.9375+4` = mA） |
| HT-1032 三溫露點 | AI2～AI4 | `adam6024.cpp:302-305`、`:382-387`；`TempCtrl\TriTemp.cpp:134`、`:161` |
| Two EP 回授 | AI0／AI1 | `adam6024.cpp:2698`、`:2704` |

golden 開機（`Open_ADAM_6024`，912 `adam6024.cpp:287-357`）會把 AI3 設成 4~20 mA、其他 AI 設成 ±10 V、兩個 AO 設成 4~20 mA（每個 `Num` 只做一次，`bADAM6420CheckRange[Num]`；`Num==2` 的 APAX 不做）。
⚠ golden 怪癖（照抄、不修）：
- AO 的讀回值是拿 `Adam6024_AI_mA_4To20`（=7）比（`:343`），不是 AO 的 `1`，所以讀回 1 時也會再設一次。
- `CHECK_EP_SETTING==0` 時 `GetAiInputRange`／`GetAoOutputRange` 寫的是 `i_byRange=0;`（改指標本身，`:164`、`:217`），呼叫端的 `rangeReadBack` 留在 0 ⇒ **反而每個通道都會送「設定範圍」**。

### 2.1 三個 IP（912 `adam6024.cpp:42`、`:400-409`）

| `Address[Num]` | IP | 什麼時候開 | 是什麼 |
|---|---|---|---|
| `[0]` | `172.16.8.110` | 一律 | 主 ADAM-6024（EP） |
| `[1]` | `172.16.8.111` | `USE_CKD_FCM_CleanAir` | 第二顆 ADAM（CKD FCM 吹氣流量；`asendic_Loader.cpp:215/219/598/641/833` 寫 `ADAM_DirectWriteData(...,1)`） |
| `[2]` | `172.16.8.112` | `INSTALL_DOUBLE_EP` 是 2 或 3 | APAX（獨立 EP／Multi EP），也走 ADAMTCP 的 Modbus/TCP |

## 3. EP 安裝型態

### 3.1 `EP_Install`（[System] `EP_Install`，golden 912 `main.cpp:1772-1786` 讀；設定畫面 `HandlerSys.dfm` 的 `ElectronPressure` 選項，`HandlerSys.cpp:591` 寫回）

| 值 | 畫面文字（`HandlerSys.dfm`） | golden 的處理 |
|---|---|---|
| 0 | Un Install | 不開模組；`Open_ADAM_6024` 回 false（`adam6024.cpp:391-394`） |
| 1 | Install (Analog) | ADAM-6024，AO 0..4095（`:1127`、`:1986-2004`） |
| 2 | Install (Digital) | 10 個 DO `SW[SwEP_D0+i]` 輸出二進位（`WriteDigital`，`:2041-2050`），滿刻度 1022（`:1131-1134`） |
| **3** | Install (Digital and Encoding) | ADAM-6024 ＋ **回授檢查**（`ADAM_ReturnValueCheck` 只在 3/5 跑，`:2683`；`main.cpp:22354`、`csystem.cpp:10998`） |
| 4 | PISO ET-7226 | ICP DAS ET-7226：`TClientSocket` 連 `IP:10001`（`adam6024.cpp:27`、`:270-273`），`TfAdam6024::WriteAO` 自己組 Modbus/TCP 功能碼 16 封包（`:2080-2108`）；不讀回授 |
| 5 | Two EP | 兩組 EP 在同一顆 ADAM 的 AO0／AO1（`:1887-1902`、`:2005-2028`），回授 AI0／AI1 |

第一次開機沒有這個鍵時，golden 跳 `"Machine have install Electrons-Press(EP)?"` 問 Yes/No 再寫檔（`main.cpp:1772-1779`）。

**HT9050**：`EP_Install=3`，ADAM-6024 在 `172.16.8.110`（`HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260929.md:44`，第 5 節第 6 條）。
Frank 的 HT9050 共用模擬組 `machines\HT9050\sim_9378\Gerneral.ini`（repo 相對路徑；commit `8edf4994`，**模擬組，不是機台正本**）：
`EP_Install=3`(:2)、`EP_MAXKPA=899`(:34)、`EP_MAXA=5.013`(:35)、`USE_CKD_FCM_CleanAir=0`(:58)、`EP_MINA_FeedBack=0.908`(:78)、`EP_MINMPA=0.005`(:83)、`INSTALL_DOUBLE_EP=1`(:106)、`DewPoint_Hardware_Install=0`(:113)、`Individual_EP_COUNT=16`(:142)、`CHECK_EP_SETTING=1`(:147)。
HT9050 只有 Z1 會上下（同一份 RULINGS 第 6 節）。

### 3.2 `INSTALL_DOUBLE_EP`（[System]，golden 912 `database.cpp:1106`；常數 `cmydef.h:4163-4166`；畫面 `rgDoubleEPControl`）

| 值 | 常數 | 意思 |
|---|---|---|
| 0 | `DOUBLE_EP_NONE` | 沒有第二組 |
| 1 | `DOUBLE_EP_NORMAL` | Die Force，同一顆 ADAM 的 AO0 |
| 2 | `DOUBLE_EP_INDIVIAL` | 獨立 EP（每 site 一路），APAX @`172.16.8.112` |
| 3 | `DOUBLE_EP_MULTI` | Multi EP（Max Qual Site），APAX-5070 ＋ 2×APAX-5028，**只有** `SW[SwMultiEp]` 真的 ON 時才走 APAX（`adam6024.cpp:96-120`） |

其他相關鍵：`CHECK_EP_SETTING`（預設 1，`database.cpp:1107`）、`Individual_EP_COUNT` → `iIndEPCnt` 4/8/16（`database.cpp:790-803`）、`DewPoint_Hardware_Install` 0/1/2（`database.cpp:678`）、`USE_CKD_FCM_CleanAir`（`database.cpp:609`）、`WEIGHT_CALIBRATION`（`database.cpp:571`）、
EP 校正值 `EP_MAXKPA`(預設 499)／`EP_MAXA`(5.013)／`EP_MINMPA`(0.001)／`EP_MINA_FeedBack`(0.908)、Dual：`EPDual_MAXKPA`(899)／`EPDual_MAXAFB`(4.905)／`EPDual_MINMPA`(0.001)／`EPDual_MinAFB`(0.968)（`ContactForce.cpp:985-1006`）。

### 3.3 APAX（獨立 EP／Multi EP）

- 寫出：`APAX_WriteData`（912 `adam6024.cpp:2305-2673`）**用 ADAMTCP 的 Modbus/TCP** 寫 `Address[2]`：S0 起始 1、8 個 word（`:2577`／Multi `:2416`），S1 起始 33（`:2646`／Multi `:2439`）；值是 12-bit 碼 ×16（`TransformFuntion` 先算好放在 `iAPAXEPValue[]`／`iAPAXDualEPValue[]`，`:1409-1419`、`:1697-1700`、`:1740-1744`）。
- 失敗重試 10 次，`iRet==0 || iRet==817` 算成功（`:2578`；817 的意義在 ADAMTCP.h 查不到，照 golden）；失敗會 `NewRecordProcess` 記 `"APAX SEND DATA FAIL %d, %s"` 並 `Close_ADAM_6024(); Open_ADAM_6024();`（`:2594-2597`）。
- Multi EP 的 16 個邏輯通道排法寫在 `:2317-2323`（Arm1：Dual1, Dual2, Site1..4, Dual3, Dual4；Arm2 同上）。
- 回讀：`TFormHS::ReadMultiEP`（912 `HS_Function.cpp:1148`）`ADAMTCP_ReadReg("172.16.8.112", 4, 33, 24, buffer)`（`:1166`），換算 `MultiTransferKG`（`:1176`）——這段是京元需求（`:1180` 註解），客戶分支照 S25 閘住。
- `Open_APAX`（`:2219-2267`）用 `ADSMOD.dll` 的 `MOD_*`（`#include "ADSMOD.h"`，`:18`；golden 連結 `Public\ADSMOD_BCB.lib`，`HT9045.bpr:159`）——**golden 912 全樹沒有任何呼叫者**（死碼）。

## 4. ADAMTCP.dll（廠商 DLL）

- 檔案：`D:\HT9045\EXE\ADAMTCP.dll`（437,760 bytes，2023-02-18）。`C:\MinGW\bin\objdump.exe -p` 量（**只讀檔，從不載入**，20261002）：`pei-i386`（PE32，只有 32 位元行程載得進）、71 個匯出、名稱**沒有裝飾**（`ADAMTCP_WriteReg` 等；只有 `_ADAM5KTCP_SendReceive6KTCPCmd@20`、`_ADAMTCP_Read6KSinglePulseDelayWidth@32` 兩個帶 `@`）、只匯入 `WS2_32` / `KERNEL32` / `USER32`。WS2_32 用序號匯入，含 115（WSAStartup）與 116（WSACleanup）。
- 呼叫慣例：廠商頭檔每個進入點都是 `EXPORTS int CALLBACK ...`（`EXPORTS` 定義在 912 `ADAMTCP.h:1-5`，進入點在 `:60-88`、`:154-265`）；Win32 的 `CALLBACK` ＝ `__stdcall`。
- golden 連結的是 `ADAMTCPbc.lib`（Borland OMF 匯入庫，912 `HT9045.bpr:155`）⇒ BCB 的 exe **靜態匯入** ADAMTCP.dll。V906 不行這樣做（RULINGS_20260929 第 5 節第 6 條＝A：執行時 `LoadLibrary`）。

### 4.1 golden 912 實際呼叫的 11 個進入點

| 函式（`ADAMTCP.h` 行） | golden 912 呼叫處 |
|---|---|
| `ADAMTCP_Open` (:154) | `adam6024.cpp:278`（只在 `Num==0`） |
| `ADAMTCP_Close` (:155) | `:426`、`:2996` |
| `ADAMTCP_Connect(ip, 502, 2000, 2000, 2000)` (:156-158) | `:359`、`:2991` |
| `ADAMTCP_Disconnect` (:159) | `:425`、`:2990` |
| `ADAMTCP_WriteReg` (:163) | `:1876`、`:1994`、`:2012-2021`、`:2416`、`:2439`、`:2577`、`:2646` |
| `ADAMTCP_ReadReg` (:162) | `HS_Function.cpp:1166`（Multi EP 回讀） |
| `ADAMTCP_Read6KAI(ip, 6017, 1, wGain, wHex, fValue)` (:188-189) | `:463` |
| `ADAMTCP_UDPOpen` (:180) | `:290`、`:2771`、`:2796`、`:2823`、`:2851`、`:2875` |
| `ADAMTCP_UDPClose` (:181) | `:355`、`:2783`、`:2808`、`:2838`、`:2861`、`:2882` |
| `ADAMTCP_SendReceive6KUDPCmd` (:183) | `:149`、`:174`、`:202`、`:227`、`:2773`、`:2798`、`:2825`、`:2853` |
| `ADAMTCP_GetHostIdleTime` (:264) | `:2877`（`GetModuleHostIdleTime`，沒有呼叫者） |

（筆電 75756cab 的 `Public/AdamTcpShim.h` 列的是前 10 個，沒有 `ADAMTCP_ReadReg`——它只在客戶分支 `ReadMultiEP` 用到。）

⚠ `ADAMTCP_Read6KAI` 的 `wModule` golden 傳的是 **`6017`**（不是 6024），`wIDAddr=1`，`wGain[]` 全設 `ADAMTCP_BI_10V`（`:379-387`）。研華範例 `E:\HT9045W_相關料件技術文件\ADAM\ADAM-6024.doc`（簡體中文實驗講義，範例程式是 ADAM-6017 讀 AI）用的正是 `DEFAULT_PORT 502`、三個 2000 ms 逾時、`ADAMTCP_Read6KAI(ip, 6017, 1, wGain, wHex, fValue)`——golden 的參數和那份範例一致。照 golden，不要改成 6024。

### 4.2 回傳碼（`ADAMTCP.h:126-143`；`E:\HT9045W_相關料件技術文件\ADAM\ADAM\ADAMTCP_SendReceive6KUDPCmd_Error Code.txt` 同一份）

| 值 | 名稱 | 值 | 名稱 |
|---|---|---|---|
| 0 | NoError | -9 | ReadStreamDataFailure |
| -1 | StartupFailure | -10 | InvalidIP |
| -2 | SocketFailure | -11 | ThisIPNotConnected |
| -3 | UdpSocketFailure | -12 | AlarmInfoEmpty |
| -4 | SetTimeoutFailure | -13 | NotSupportModule |
| -5 | SendFailure | -14 | ExceedDONo |
| -6 | ReceiveFailure | -15 | InvalidRange |
| -7 | ExceedMaxFailure | -100 | EventError |
| -8 | CreateWsaEventFailure | | |

golden 顯示錯誤字串用 `fAdam6024->ADAMErrorMessage[-iRet]`（表在 ctor，912 `adam6024.cpp:124-139`，16 格，字串前綴寫成 `ADAM5KTCP_`）。⚠ -100 會讀到第 100 格（陣列只有 16 格，`adam6024.h:87`）＝ golden 的越界，移植時要記成偏離或照抄並註明。

### 4.3 逾時與阻塞（golden 912）

- TCP：`iConnectionTimeout=iSendTimeout=iReceiveTimeout=2000` ms（`adam6024.cpp:36-38`）；`ADAMTCP_Connect` 在模組沒回應時會卡到逾時（筆電 75756cab 實測說法：一個沒反應的模組每次 Open 約 3 秒）。
- UDP：`Open_ADAM_6024` 用 2000/2000（`:290`），`GetModuleName` 等用 1000/1000（`:2771` 起）。每個 UDP 查詢都自己 `UDPOpen`／`UDPClose`。
- `ADAM_ReadVoltage` 讀失敗就 `Close_ADAM_6024(); Open_ADAM_6024();`（`:465-471`）；`APAX_WriteData` 寫失敗也一樣（`:2596-2597`）。⇒ 模組斷線時，每個讀寫點都會付一次重連的逾時。
- `Close_ADAM_6024` 呼叫 `ADAMTCP_Disconnect(); ADAMTCP_Close();`（`:425-426`）。筆電 3c348627 指出 `ADAMTCP_Close` 會無條件 `WSACleanup`——wb_serve 是單一行程，這會把它所有 socket 一起清掉。本 skill 只確認了 DLL 有匯入 WSACleanup（序號 116），**沒有反組譯確認是無條件呼叫**。

## 5. 相關 DO／DI（golden 名稱）

- `SW[SwEpArm1]`／`SW[SwEpArm2]`／`SW[SwIndEpArm1]`／`SW[SwIndEpArm2]`：哪支臂的 EP 氣路打開（`EpSwitch`，912 `adam6024.cpp:3053-3103`）。
- `SW[SwMultiEp]`：Multi EP 共用／獨立氣路切換閥（`main.cpp:21699-21714` 在 Timer2 裡自動切）。
- `Sen[SnEPDieForce]`：Die Force 氣壓感測，關閉時 `WAR0329`（`csystem.cpp:2754-2767`）。
- `SW[SwEP_D0..D9]`：`EP_Install=2` 的 10-bit 數位輸出（`adam6024.cpp:2047`）。
- HT9050 的 IO 表 `machines\HT9050\IO_Table.csv`（repo 相對路徑）有 `SwEpArm1/2`（:442-443）、`SwIndEpArm1/2`（:839-840）、`SnEPDieForce`（:843）；照表頭（:1）第 10 欄 `Enable` 讀，五列裡只有 `SwIndEpArm1` 是 1。哪些點真的有接線，本 skill **沒有上機確認**。

<!-- preserved-content:end -->
