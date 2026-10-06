> 保存來源：`.claude/skills/ht9045-motor-control/references/panasonic-rs232/overview.md`，main `eb7f47e7f`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../common.md)。

<!-- preserved-content:start -->
> 原為獨立 skill `ht9045-panasonic-rs232`；Steven 20261005 08:0x 要求併入 `ht9045-motor-control` 當參照（本檔＝原 SKILL.md 正文，其餘參照在同資料夾）。

# ht9045-panasonic-rs232：Index Z 國際牌伺服驅動器的 RS232

## 這是什麼

HT9045 的 Index 有兩支 Z 軸（Z1＝`MTestZ1`、Z2＝`MTestZ2`），移動走 Galil，**扭力**卻是 PC 直接用一個 COM 埠跟國際牌驅動器講：

1. **讀目前扭力**：一次讀一軸，結果寫進 `fMain->edTorue0／edTorue1`（`"%5.2f"`，單位＝額定扭力的 %），Index 下壓判斷、Auto Height、GPIB 回報壓力都讀這兩格。
2. **寫／讀第 1 扭力限制**（A5 Pr0.13、A4 Pr5E，單位 %）：下壓前把上限改成產品設定值，壓完改回 300；寫完一定讀回比對（`iWriteAndCheckMotorTorque`）。

兩台驅動器共用一個 COM 埠，靠輸出 `SW[SwReadTorue]` 切換（Off＝Z1、On＝Z2；golden 0618 rs232.cpp:841-852）——警報字串說「可能是 Relay 或線材脫落」（:867），所以推定是繼電器切 RS232 線（實體接線圖程式未見）。

不在這份範圍：三菱驅動器路徑（`INDEX_DRIVER_TYPE=1`，另一套 STX／ETX 協定，rs232.cpp:2114-3233）、鴻勁自製通訊卡（`USE_HP_COM_CARD=1`，115200、ASCII `:@00A…` 加 LRC，rs232.cpp:1002-1364、:4296-4700）。只在對照時提到。

## 哪些機台會走這條路

- 每台看 `Gerneral.ini [IndexDriver]`：`INDEX_DRIVER_TYPE` 0＝Panasonic A4、1＝Mitsubishi、2＝Panasonic A5（cmydef.h:80-82；System 畫面 `rgIndexMotorType`，HandlerSys.dfm:2557-2576）；`COM_PORT`（缺鍵預設 COM1，database.cpp:519）；`USE_HP_COM_CARD`。A5 進來後會被改寫成 `INDEX_DRIVER_TYPE=Panasonic_DRIVER`＋`iPanasonicDriverType=Panasonic_DRIVER_A5`（rs232.cpp:103-111），**之後程式裡所有 `INDEX_DRIVER_TYPE==Panasonic_DRIVER` 都同時涵蓋 A4 與 A5**。
- repo 內的設定：`D:\HT9045\system\Gerneral.ini:434-440`＝A5、COM11（另兩份 config-seed／HT9050 sim 相同）。
- 哪些出貨機用 A4／A5／三菱／HP 卡：程式未見，要看各機台的 Gerneral.ini。
- 會用到扭力值的客製：KYEC Xilinx 浮動 Shuttle 門檻（atester.cpp:6382-6388、:6544-6555）、TSMC 台南／Korea／SPIL 的 GPIB 壓力回報（cinitial.cpp:13867-13873、Command.cpp:1510-1532）、VTEST 扭力 log（atester.cpp:6579-6580）。
- **HT9050 沒有國際牌驅動器**：Index Z1 是安川 Σ-X 掛在 PCI-1203（EtherCAT），見下面「HT9050」。

## 協定一張表（細節與頁碼在 protocol.md）

| 項目 | 內容 | 出處 |
|---|---|---|
| 實體層 | A5：X2 腳 3 TXD、4 RXD、1 GND（RS232）；5–8 為 RS485 兩對 | A5II p.101、p.414 |
| 參數 | 8 位元、無同位、1 stop；A5 2400–115200（Pr5.29，預設 2＝9600）；A4 2400–57600（Pr0C，預設 2＝9600） | A5II p.414、p.276；A4中 p.282、p.110 |
| 交握碼 | ENQ 05h、EOT 04h、ACK 06h、NAK 15h；RS485 在 ENQ／EOT 前加 `80h｜模組 ID` | A5II p.415 |
| 一次往返 | 主機 ENQ → 驅動器 EOT → 主機送區塊 → 驅動器 ACK（或 NAK）→ 驅動器 ENQ → 主機 EOT → 驅動器送回覆區塊 → 主機 ACK | A5II p.416 |
| 區塊 | `N, axis, (mode<<4)｜command, 參數 N 個位元組, checksum`；全長 N+4 | A5II p.417、p.416；第 3 位元組的高低半位元由 p.419 範例 `71h`＝command 1／mode 7 確認 |
| 檢查碼 | 從第一個位元組加到最後一個參數，取 2 補數；收方把整塊（含檢查碼）加總＝0 才回 ACK | A5II p.417、p.416 |
| 逾時／重試 | T1 字元間、T2 協定逾時、RTY 重試次數；重試一律從 ENQ 重來 | A5II p.417、p.416 |

## HT9045 實際送出的東西（golden 0618 rs232.cpp）

| 用途 | 位元組（16 進位） | command／mode | 手冊名稱 | 行號 |
|---|---|---|---|---|
| 交握 | `05`（ENQ）、`04`（EOT）、`06`（ACK）；**從不送 NAK** | — | — | :830-832、:887、:940、:969；:1527-1529、:1552、:1598、:1626-1658 |
| 讀扭力 Z1／Z2 | `00 00 52 AE`／`00 01 52 AD` | 2／5 | Read out of present torque output（A5II p.427；A4中 p.294） | :833、:909-911 |
| A5 讀 Pr0.13 | `02 00 07 00 0D EA` | 7／0 | Individual read out of parameter（A5II p.433） | :1375-1389 |
| A5 寫 Pr0.13 | `06 00 17 00 0D L H 00 00 sum`（300 → `…2C 01 00 00 A9`） | 7／1 | Individual writing of parameter（A5II p.433） | :1423-1441 |
| A4 讀 Pr5E | `01 00 08 5E 99` | 8／0 | 個別讀參數（A4中 p.300） | :1390-1404 |
| A4 寫 Pr5E | `03 00 18 5E L H sum`（300 → `…2C 01 5A`） | 8／1 | 個別寫參數（A4中 p.300） | :1442-1457 |

- 扭力回覆 `03 axis 52 L H err sum`（7 位元組）→ `k=(short)(H<<8|L)`、`k<0` 當 0、`Torque=k/20.0`（:1801-1813）。手冊：**單位＝額定扭力 2000**、16 位元、負號＝負方向（A5II p.427；A4中 p.294「= 2000」）⇒ `k/20`＝額定扭力的 %。
- 寫進去的上限值（%）：HOME 與每次壓完寫 300（uhome.cpp:2510／:2543、atester.cpp:6623）；下壓前寫 `Prod.iMaxPreasure`（atester.cpp:6350）；Auto Height 寫 `kg`（cContact.cpp:5713）、120、90、99、80 等。手冊 Pr0.13 範圍 0–500 %（A5II p.234），多數組合上限 300、預設 300（A5II p.134）。
- **沒有送的**：取得執行權（1／7）、寫 EEPROM（A5 7／2、A4 8／4）、讀狀態／警報／清警報、INIT。所以寫入的上限只在 RAM（A5II p.433「temporarily」），驅動器斷電後回到 EEPROM 值。

## 程式地圖

| 樹 | 路徑 | 狀態 |
|---|---|---|
| golden 0618（基準） | `D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\rs232.cpp／.h／.dfm`（Big5，要用 cp950 解碼讀） | 全部行號以這裡為準 |
| golden V912 | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\rs232.cpp` | 國際牌那一段**跟 0618 一字不差**，行號 +6（`GetReadTorueTask` +15）；12110／14110 等待邏輯也相同 |
| V906 移植 review6 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\rs232.cpp`（UTF-8，`TCOM2Shim`） | 國際牌函式逐句照翻；SPComm 分包語意補在 `W906_PumpComm1`；HP 卡閘著；HT9050 不開扭力埠、扭力上限改走 1203 |
| V906 main | `origin/main`（2a8bbb24）同檔 | 多了 E-038 Phase A：`rs232.cpp:358` 同一行把 HT9050 的讀扭力導到 1203 6077h |

核心函式（golden 0618 rs232.cpp）：`RS232Init` :149-262（扭力段 :155-161、:236-262）／`TorqueSend` :765／`ReadTorque` :809 → `ReadTorque_Panasonic` :820-1000／`ReadIndexTorqueSetting_Pana` :1368／`WriteIndexTorqueSetting_Pana` :1414／`ReadWriterParameter_Panasonic` :1523-1681／`Comm1ReceiveData` :1750-1833／`iWriteAndCheckMotorTorque` :1847-2009／`GetReadTorueTask` :3763。節拍：`MainProc`（csystem.cpp:16711）每輪呼叫 `COM2->ReadTorque()`（:16848）與 `COM2->ReadWriterParameter()`（:16860）。

呼叫者：`DoTestHeadMotor`（atester.cpp:5562；Z1 case 120／12000／12100-12110／12200，Z2 14110 等）、HOME（uhome.cpp:2491-2568）、`DoZ1／Z2PickFromShuttle`、`Do_Z1／Z2_AutoGetHeight`、`Do_LoadCellAutoHigh`、`Do_ContactTest_32Site`（cContact.cpp）、主畫面 Torque 頁的 Set／Read 按鈕（main.cpp:21927-21953）。完整表在 ht9045-implementation.md §9。

## HT9050

- 沒有 RS232 扭力驅動器：移植樹 `RS232Init` 在 `IO_CARD_TYPE==PCI1203_IO` 時不探測、不開扭力埠（review6 rs232.cpp:263-269）。
- 扭力**上限**：`iWriteAndCheckMotorTorque` 在 `MOT[MTestZ1].CardType=="PCI1203"` 時改呼叫 `W906_Ht9050TorqueLimit`（review6 rs232.cpp:950、:2255-2291），值＝golden %×10 寫 60E0h／60E1h 並讀回；沒有掛鉤就回 2，不假成功。
- 扭力**讀值**：E-038 Phase A（`e6cec741`，已在 origin/main）在 `ReadTorque` 開頭（main rs232.cpp:358）讀 6077h、換成國際牌的 % 比例寫 `edTorue0`，`[IndexDriver] HT9050_INDEXZ_TORQUE_CONFIRMED` 沒設 1 前不給值。
- 等待逾時：golden 12110 只有在讀完（999）後文字又被清掉才報錯，讀不到就永遠等；E-044（`9432ff6e`，分支 `v906/st01e-e044`，1004 17:4x 時還不在 origin/main）在 HT9050 的 12110／14110 加 5 秒逾時→警報＋停機（not golden）。
- 換算、正負號、裁決 Q87：讀 `D:\HT9045\.claude\skills\ht9045-motor-control\references\index-torque-autoheight.md`（那份 §1 把 0x52 寫成「命令 5／模式 2」，origin/main 版 §6.7 的 Q1 還在問單位——本 skill 的答案：command 2／mode 5、手冊單位「額定扭力＝2000」⇒ k/20＝%，見 command-list.md）。

## 讀哪份 reference

| 想知道 | 讀 |
|---|---|
| 接線、鮑率、交握順序、區塊、檢查碼怎麼算（含手算範例）、逾時參數、A4 與 A5 差在哪、MCB 通訊板 | protocol.md |
| 手冊所有指令（command／mode、送什麼回什麼、單位、頁碼），哪幾個 HT9045 有用，加新指令（讀警報、狀態、寫 EEPROM）怎麼組 | command-list.md |
| golden 每個函式、每個 case、計時、UI 欄位、呼叫者、V912 差異、移植樹對照、HT9050、已知陷阱 | ht9045-implementation.md |

## 改這塊之前一定要知道的五件事

1. **999 只活一拍**：`case 40` 設 999 並清掉兩個勾（:991-995），下一次 `ReadTorque()` 看到兩個勾都沒勾就把 Task 拉回 1（:854-858）。12110 的錯誤判斷要「文字是空的＋剛好 999」才成立；讀不到值時 12110 不會報錯，只會一直等（rs232 自己每 10 次以上失敗跳一次訊息）。
2. **逾時用 MainProc 拍數算**，不是毫秒：`iPanasonicWT=500`（:762）；實際秒數跟 MainProc 週期走（uruncontrol.cpp:42-68 每輪 `Synchronize`＋`MySleepEx(1)`）。每次逾時都 `StopComm／StartComm` 重開埠。
3. **ACK＋ENQ 要在同一次 OnReceiveData 收到**（:938 看 `data[0]`、`data[1]`），靠 rs232.dfm:61 `ReadIntervalTimeout=100`；移植樹用 100 ms 安靜佇列模擬（review6 rs232.cpp:31-42、:190-208），改通訊層時別拆掉。
4. 收資料**不驗檢查碼、不看錯誤碼位元組**；任何意外都回到 ENQ 重來（手冊也規定重試從 ENQ 開始，A5II p.416）。
5. 負扭力被丟成 0（:1809），呼叫端再 `atoi` 成整數比門檻（atester.cpp:6536、cContact.cpp:5775）；呼叫端的 `abs()` 實際上沒作用。

## 相關 skill

- `ht9045-motor-control`（`references/index-torque-autoheight.md`：Auto Height、安川 6077h 換算、E-038／E-044 細節）
- `ht9045-motor-control/references/yaskawa-ethercat/`：HT9050 實際在用的安川 Σ-X SGDXS EtherCAT 驅動器（6077h、2704h、警報與重置）；HT9050 的 Index Z1 扭力改走這條。
- `ht9045-motor-control/references/panasonic-ethercat-a6bn/`：Panasonic MINAS A6BN（線性馬達龍門型）EtherCAT 規格；不是 RS232 這條，目前沒有機台在用。
- `rs232-standard-interface`（Handler↔Tester 的 RS232 指令，不是這條）
- `rs232-ttl-communication`（TTL 通訊板，不是這條）
- `ht9050-hw`（HT9050 馬達表）、`ht9045-motor-home`（HOME 流程裡的 case 310-322）

<!-- preserved-content:end -->
