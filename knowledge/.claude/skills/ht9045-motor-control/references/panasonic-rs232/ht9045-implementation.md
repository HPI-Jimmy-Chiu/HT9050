# HT9045 的國際牌 RS232 實作（golden 0618 → V912 → V906 移植）

> 行號沒特別註明樹的，一律是 **golden 0618**：`D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\`。
> 協定規則見 `protocol.md`，指令內容見 `command-list.md`。

## 1. 樹、檔案、怎麼讀

| 樹 | 路徑 | 編碼 | 這份用到的檔 |
|---|---|---|---|
| golden 0618 | `D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\` | Big5（cp950）、CRLF | rs232.cpp（4786 行）、rs232.h、rs232.dfm、atester.cpp、cContact.cpp、csystem.cpp、uhome.cpp、cinitial.cpp、database.cpp、cmydef.*、main.cpp／.dfm、HandlerSys.*、Command.cpp、uruncontrol.cpp |
| golden V912 | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` | Big5 | rs232.cpp（4803 行） |
| V906 移植 review6 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`（分支 `v906/steven-cbridge-review6`，1004 時 HEAD `84419e85`） | UTF-8 | rs232.cpp（2496 行）、atester.cpp、csystem.cpp、cinitial.cpp、uhome.cpp、tests/test_rs232_torque.cpp |
| V906 main | `git show origin/main:HT9011UC_Cpp_V3.33.906.0/<檔>`（1004 17:44 為 `2a8bbb24`） | UTF-8 | rs232.cpp:358（E-038）、IndexZTorque1203.*、EtherCAT/Pci1203TorqueRead.cpp |

- Big5 檔**不可以直接改**，也不要用編輯器開了存檔。要讀就用 Python 解碼成 UTF-8 副本（`C:\Users\steven\AppData\Local\Programs\Python\Python314\python.exe`；PATH 上的 `python` 是 Store 空殼）：`open(src,'rb').read().decode('cp950').replace('\r\n','\n')`，行數不變。
- 純 ASCII 的識別字可以直接對 Big5 檔 `grep -a -n`，行號一樣。

## 2. 設定與開埠

| 項目 | 位置 | 內容 |
|---|---|---|
| 驅動器種類 | rs232.cpp:103-111（`TCOM2` 建構子） | 讀 `Gerneral.ini [IndexDriver] INDEX_DRIVER_TYPE`（預設 `Panasonic_DRIVER`＝0）與 `USE_HP_COM_CARD`（預設 false）；讀到 2（A5）就改成 `INDEX_DRIVER_TYPE=0`＋`iPanasonicDriverType=Panasonic_DRIVER_A5` |
| 代碼 | cmydef.h:80-82 | `Panasonic_DRIVER 0`（A4）、`Mitsubishi_DRIVER 1`、`Panasonic_DRIVER_A5 2`；全域預設 cmydef.cpp:3460-3463 |
| COM 埠 | database.cpp:519、:544-547 | `HSys.sTorqueComPort`＝`[IndexDriver] COM_PORT`，缺鍵預設 `"COM1"`；不是 `COM` 開頭就補上 |
| System 畫面 | HandlerSys.cpp:207、:267-268、:706、:786-787；HandlerSys.dfm:2557-2576 | `cbComIndex`、`rgIndexMotorType`（'Panasonic A4'／'Mitsubishi'／'Panasonic A5'）、`chkUseHPComCard` |
| 其他 Index 鍵 | database.cpp:645、:1081、:1085 | `CLEAN_AIR`、`USE_IO_CHANGE_TOQUE`（另一種用 IO 切扭力的機構，下壓前都會 `SwIndexChangeToque1／2.Off()`）、`USE_ReadIndex_TOQUE`（aTester_Front／Rear 用） |
| 建立 | HT9045.cpp:174 | `Application->CreateForm(__classid(TCOM2), &COM2)` |
| 開埠 | `RS232Init` rs232.cpp:149-262；由 cinitial.cpp:5840 呼叫 | :155-161 `GetCOMPortStatus` 探測，失敗跳「Index Torque : COMx port error」；:236-262 設 `\\.\COMx`、Parity None（三菱 Even）、9600（HP 卡 115200）、8 bit、1 stop、`StartComm()` |
| 元件設計期值 | rs232.dfm:39-71 `object Comm1: TComm` | BaudRate 9600、無流控、`DtrControl=DtrEnable`、`RtsControl=RtsEnable`、**`ReadIntervalTimeout=100`**、`OnReceiveData=Comm1ReceiveData`、`OnReceiveError`／`OnRequestHangup`（只設 `bCom1Error`，rs232.cpp:4735-4743；只有 HP 卡路徑讀它） |
| 關埠 | rs232.cpp:3711-3714 `DataModuleDestroy` | `Comm1->StopComm()` |
| 選軸繼電器 | `SW[SwReadTorue]`（cmydef.cpp:1947＝68；`D:\HT9045\system\IO_Table.csv:23`） | Off＝Z1、On＝Z2；模擬組態 `Enable=false`（cinitial.cpp:1517），HOME 用它判斷要不要做扭力設定（uhome.cpp:2491-2501） |

- 一個 COM 埠、兩台驅動器、一個繼電器：讀扭力時 axis 欄送 0（Z1）或 1（Z2）（:909）；參數讀寫 axis 一律送 0（封包第 2 個位元組固定 `0x00`，:1378、:1393、:1426、:1445；:1546 的 `Address=0` 沒被用到）。
- 驅動器端的 Pr5.31（A5）／RSW(ID)（A4）實際設多少：程式未見，要上機看。

## 3. 節拍

- `MainProc()`（csystem.cpp:16711）每輪（`InitialOK` 之後，:16724-16725 是唯一提早 return）呼叫 `COM2->ReadTorque()`（:16848）與 `COM2->ReadWriterParameter()`（:16860）；同一輪稍後才跑 `DoTestHeadMotor()`（:17597、:18844）。
- `MainProc` 由 `TRunControl::Execute`（uruncontrol.cpp:42-68）用 `Synchronize` 在主執行緒跑，每輪之間 `MySleepEx(1)`（運轉中三輪睡兩輪）。所以 golden 的「拍數逾時」換成時間要乘上 MainProc 一輪的實際長度——**程式沒有固定週期**。
- 暫停分支裡每輪呼叫 `COM2->ResetPanasonicTime()`（csystem.cpp:18935）：參數讀寫進行中時把 10 秒逾時改成 5 秒重算（rs232.cpp:1504-1510）。

## 4. 函式地圖（rs232.cpp）

| 函式 | 行 | 角色 |
|---|---|---|
| `TorqueSend(str,len)` | :765-777 | `Comm1->WriteCommData`，把送出的位元組寫進 Torque log（`fMain->AddTorqueLog`，main.cpp:32586），`bReceive=false` |
| `ReadIndexTorqueSetting(Index)` | :779-787 | 分派：HP 卡／Panasonic（A4＋A5）／三菱 |
| `WriteIndexTorqueSetting(Index, AnsiString)` | :789-797 | 同上；Panasonic 用 `atoi` |
| `InitReadTorueTask()` | :803-807 | Panasonic 時 `autoTask=1` |
| `ReadTorque()` | :809-817 | 分派到 `ReadTorque_Panasonic` |
| `ReadTorque_Panasonic()` | :820-1000 | 讀扭力狀態機（§5） |
| `ReadIndexTorqueSetting_Pana(Index)` | :1368-1411 | 組讀 Pr0.13／Pr5E 封包，`fPanasonicParameterRW=true`、10 s 逾時、`bRWPanasonicParameterFlag=true`、`iPanasonicTask=1` |
| `WriteIndexTorqueSetting_Pana(Index, Data)` | :1414-1465 | 組寫封包，同上但 flag＝false（寫） |
| `ResetPanasonicTime()` | :1504-1510 | 暫停時把參數讀寫逾時重設 5 s |
| `ReadWriterParameter()` | :1513-1521 | 分派到 `ReadWriterParameter_Panasonic` |
| `ReadWriterParameter_Panasonic()` | :1523-1681 | 參數讀寫狀態機（§7） |
| `Comm1ReceiveData(...)` | :1750-1833 | 收資料、解碼（§6） |
| `InitWriteAndCheckMotorTorqueTask()` | :1838-1845 | `iWriteAndCheckMotorTorqueTask=1`、`edtReadZ1／Z2="0"`、兩個勾清掉 |
| `iWriteAndCheckMotorTorque(MotorIndex, Torque)` | :1847-2009 | 寫上限＋讀回比對；回 0 進行中、1 成功、2 失敗（§8） |
| `GetReadTorueTask()` | :3763-3766 | 回 `autoTask` |
| 三菱 | :2114-3233 | 另一套協定，不在本 skill |
| HP 通訊卡 | :1002-1364、:1467-1502、:1683-1748、:4296-4700 | 鴻勁自製卡，ASCII `:@00A…`＋LRC＋`@#`、115200；卡回的錯誤碼表有「EOT／Time out／CMD／RS485／LRC／NACK」（:4498），推定卡內部才跟驅動器跑國際牌協定（程式未見卡的韌體） |

## 5. 讀扭力狀態機 `ReadTorque_Panasonic`（:820-1000）

**武裝**：呼叫端把 `fMain->chkReadTorque1`（Z1）或 `chkReadTorque2`（Z2）打勾，清空 `edTorue0／1`，呼叫 `COM2->InitReadTorueTask()`。

| 步驟 | 行 | 做什麼 |
|---|---|---|
| 模擬組態 | :822-825 | 直接 return（值由呼叫端自己填，例：`ShowMainScreenPresure` 填 "10.0"，cinitial.cpp:13851-13853；Auto Height 填 30，cContact.cpp:5769-5770） |
| 參數讀寫中 | :838-839 | `fPanasonicParameterRW` 為真就 return（整台暫停讀扭力） |
| 選軸 | :841-853 | 勾 1 → `iReadTorueIndex=0`、繼電器 Off；勾 2 → 1、On（每拍都設） |
| 沒武裝 | :854-859 | 兩個勾都沒勾 → `Task=1`、`iAlarmCT=0`、return |
| 失敗太多 | :861-871 | `iAlarmCT>10` → `ShowMyMessage("Rs232 Read Index Z1/Z2 Torque error!! maybe Relay or Com Port fail, please check .")`，`Task=1`、計數歸 0，**不解除武裝**，繼續重試 |
| case 1 | :875-878 | 設 100 ms 延遲，`Task=3`（直接落入 case 3） |
| case 3 | :879-885 | 延遲到 → `Address=iReadTorueIndex`，`Task=5` |
| case 5 | :886-891 | 送 ENQ，`ct=0`，`Task=10` |
| case 10 | :892-922 | 等 EOT：`ct>iPanasonicWT`（500 拍）→ `Task=1`、`StopComm／StartComm`、`Torque[Address]=-9999`、`iAlarmCT++`；收到且 `data[0]==EOT` → 送 `00 Address 52 (AE−Address)`，`Task=20`；收到別的 → `Task=1` |
| case 20 | :923-951 | 等 `data[0]==ACK && data[1]==ENQ`（同一包）→ 送 EOT，`Task=30`；逾時同上；別的 → `Task=1` |
| case 30 | :952-990 | 等 `data[0]==0x03`（回覆的 N）→ 送 ACK，把 `asReceiveTorue` 寫進 `edTorue0`＋`fContact->PnlTorue0`（或 1），`Task=40`；逾時同上；別的 → `Task=1` |
| case 40 | :991-996 | `Task=999`、兩個勾清掉、`iAlarmCT=0` |

- `ct` 每拍加 1，**不是毫秒**；`iPanasonicWT=500`（:762，註解「Sam 20200507 : 400>500」）。
- 收到「別的東西」（NAK、錯的 N…）一律回 case 1 從 ENQ 重來，不送 NAK、不計 `iAlarmCT`。只有逾時才計數；**10 次以上**逾時才跳訊息。
- **999 只活一拍**：case 40 把兩個勾清掉，下一次 `ReadTorque()` 走 :854-858 把 Task 拉回 1。`GetReadTorueTask()==999` 只有在同一輪 `MainProc` 裡、`ReadTorque()` 之後的程式（例如 `DoTestHeadMotor`）看得到。

## 6. 收資料 `Comm1ReceiveData`（:1750-1833）

| 行 | 做什麼 |
|---|---|
| :1753-1754 | `InitialOK==false` 就丟掉 |
| :1763-1776 | HP 卡：只把位元組推進 `_byte_datas`（由 `TimerHPCardTimer` 解析），不設 `bReceive` |
| :1779-1785 | 一包 >2000 位元組：記 log 後丟掉 |
| :1787-1797 | 每包都記一行 Torque log（ASCII＋hex） |
| :1799-1814 | Panasonic：`BufferLength==7`（或 A5 時 9）→ `k=(short)(str[4]<<8 | str[3])`；`str[1]<6` 時：`k<0` 當 0、`Torque[iReadTorueIndex]=k/20.0`、`asReceiveTorue=sprintf("%5.2f", Torque[str[1]])` |
| :1816-1822 | 參數讀取中（`fPanasonicParameterRW && bRWPanasonicParameterFlag`）：`edtReadZ1／Z2 = k`（整數） |
| :1825-1828 | 三菱：`sprintf(cReciveMitsubishi_Data, data)`（把收到的資料當格式字串） |
| :1829 | `bReceive=true` |
| :1831-1832 | `ptreot=data[0]`、`ptrenq=data[1]`（就算只收到 1 個位元組也讀 `data[1]`） |

- 判斷是不是扭力**只看長度**：A4 讀參數的回覆也是 7 個位元組，也會跑 :1804-1814，把參數值 /20 寫進 `Torque[iReadTorueIndex]` 與 `asReceiveTorue`（不會進 `edTorue0`，因為那時 `ReadTorque_Panasonic` 在 :838 停著）。
- 不驗檢查碼、不看錯誤碼位元組、不看第 3 個位元組是哪個指令。

## 7. 參數讀寫狀態機 `ReadWriterParameter_Panasonic`（:1523-1681）

封包由 :1368-1465 先組好放在 `Panasonicdatatrq[]`、長度 `iPanasonicNum`（A5 讀 6、A5 寫 10、A4 讀 5、A4 寫 7）。

| 步驟 | 行 | 做什麼 |
|---|---|---|
| 沒事 | :1531-1532 | `fPanasonicParameterRW==false` 就 return |
| 總逾時 | :1536-1541 | `hPanasonicParameterTimeOut`（設定時 10 s；暫停時 `ResetPanasonicTime` 改 5 s）到 → `fPanasonicParameterRW=false`、`bPanasonicCommErr=true`（這個旗標**沒有人讀**）、return |
| case 1 | :1544-1548 | `hPDelay.Set0_1SecAndOn(rwCommandDelay)`（`rwCommandDelay=5`，:119 ⇒ 0.5 s），落入 case 5 |
| case 5 | :1549-1557 | 延遲到 → 送 ENQ，`Task=10` |
| case 10 | :1558-1583 | 等 EOT → 送整個封包，`Task=20`；500 拍逾時 → `Task=1`＋重開埠 |
| case 20 | :1584-1609 | 等 ACK＋ENQ 同一包 → 送 EOT，`Task=30` |
| case 30 | :1610-1669 | A5：`iPanasonicNum==6`（讀）要 `data[0]==0x05`、`==10`（寫）要 `0x01`；A4：`==5` 要 `0x03`、`==7` 要 `0x01` → 送 ACK，`Task=40`；`iPanasonicNum==8` 的分支（:1636-1641）**沒有任何組包函式會設 8**，是死碼 |
| case 40 | :1670-1679 | 再等 `MOTOR_MIN_WAIT`（10 拍，:761）→ `fPanasonicParameterRW=false`、`Task=1` |

- 讀回的值在 `Comm1ReceiveData` :1816-1822 寫進 `edtReadZ1／edtReadZ2`。
- 寫入的值寫進驅動器 RAM；沒有寫 EEPROM、沒有取執行權（見 `protocol.md` §9）。

## 8. 寫上限＋讀回 `iWriteAndCheckMotorTorque`（:1847-2009）

| case | 行 | 做什麼 |
|---|---|---|
| 模擬組態 | :1851-1858 | 回 1 |
| 三菱前置 | :1866-1876 | 有警報回 2；上限最多 100 |
| 1 | :1880-1885 | 計數歸 0 |
| 2 | :1886-1904 | `edtReadZ1／Z2="0"`；**把目標軸的 `chkReadTorque` 打勾**（:1890-1899，順便武裝了一次讀扭力）；20 ms |
| 5 | :1905-1910 | 等 20 ms |
| 10 | :1911-1921 | `WriteIndexTorqueSetting(MotorIndex, Torque)`；計時 `iTorqueCommMaxTime`（12 s，cmydef.cpp:3467） |
| 100 | :1922-1946 | 寫完（`fPanasonicParameterRW==false`）→ `ReadIndexTorqueSetting`、`Task=200`；12 s 還沒完 → 回 case 2，第二次起跳「Torque Comm Timeout. Please check Torque Comm and try home or to restart!」 |
| 200 | :1947-2004 | 讀完 → Panasonic 比 `edtReadZx->Text != AnsiString(Torque)`：不同 → 重試，第 5 次回 **2**；相同 → 回 **1**（三菱比 ±1）；12 s 沒讀完 → 回 case 2＋同樣的 Timeout 訊息 |
| 其他 | :2006 | 回 0（進行中） |

- 線斷掉時：寫入 10 s 總逾時 → 讀取 10 s 總逾時 → 讀回值還是 "0" ≠ 設定值 → 重試；5 輪後回 2（推算約 100 s 以上；呼叫端才跳「Motor torque set error」）。
- 呼叫端回 2 的反應：atester.cpp:6358-6364「Motor torque set error」＋`fAllMotorHome=false`；uhome.cpp:2525-2533「Motor MTestZ1 torque set error!!」＋停止 HOME。

## 9. 呼叫者

### 9.1 量產 Index 下壓 `DoTestHeadMotor`（atester.cpp:5562）

| case | 行 | 做什麼 |
|---|---|---|
| 120 | :6335-6339 | `InitWriteAndCheckMotorTorqueTask()`、`lbArm0Torque="1:Writing"` |
| 12000 | :6340-6365 | `iWriteAndCheckMotorTorque(0, Prod.iMaxPreasure)`；2 → 「Motor torque set error」 |
| 12100／12101 | :6366-6427 | Z1 下到測試高度；`ReadTorqueDelay` 10 s（:6424）；KYEC Xilinx 浮動 Shuttle 走 12102 多等 `dD01ReadTorqueDelayTime` |
| 12110 | :6436-6583 | 1 s 內第一次：勾 1、清 `edTorue0`、`InitReadTorueTask()`、return（:6439-6450）；`edTorue0==""` → 10 s 後**且** `GetReadTorueTask()==999` 才報「The test head 1 Motor torque read error」（:6452-6464），否則一直等；有值 → `ShowMainScreenPresure(0)`；socket sensor 檢查；`TorqueData=abs(atoi(edTorue0))`（:6536-6537）比 `iMaxPreasure`（或 `DeviceForm.iIndexTorqueMax`）：超過連 10 次 → 12111 抬起＋WAR0321（KYEC Xilinx 比 `dD01ReadTorque+dZ1Torue`，:6544-6555） |
| 12200 | :6616-6638 | `iWriteAndCheckMotorTorque(0, 300)` 改回 300，成功後兩個勾清掉 |
| Z2：14000／14101／14110／14200 | :6971-6995、:6999-7057、:7066-7230、:7265-7286 | 同 Z1；14110 另外在 `bControlTorque` 時比 Z1／Z2 差值 `iIndexTorqueCmp`（:7211-7221） |

### 9.2 其他

| 位置 | 做什麼 |
|---|---|
| atester.cpp:849 `GetTesterResult`，:883-905 | `TestIF_File.bEnableReadAndCheckTorque` 時在測試開始武裝一次讀扭力 |
| atester.cpp:9413 `CheckAndRecodrTorque`、:9465 `NewCheckAndRecodeTorque` | 扭力寫 CSV log（`bD26EnableEPLog`）／VTEST |
| uhome.cpp:2491-2568（HOME case 310／320／322） | `SwReadTorue.Enable` 時 Z1、Z2 各寫 300 並讀回 |
| cContact.cpp:2487 `DoZ1PickFromShuttle`（:2828 寫 `iContactKG`、:2994-3026 讀、:3212／:3232 寫 80、:3860 寫 300）；:3880 `DoZ2PickFromShuttle` | Contact 頁從 Shuttle 取料 |
| cContact.cpp:5389 `Do_Z1_AutoGetHeight`（:5704-5756 寫 `kg`、:5757-5764 武裝、:5765-5790 等值＋`TorqueData>=kg` 或到下限 `fIndexDownPos` 連 10 次）；:8174 `Do_Z2_AutoGetHeight`；:17197 `Do_LoadCellAutoHigh`；:10513 `Do_ContactTest_32Site` | 自動測高等（詳見 `ht9045-motor-control/references/index-torque-autoheight.md` §1） |
| atester_32Site.cpp:1831／:1846、aTester_Front.cpp:7311、aTester_Rear.cpp:7584 | 其他機種直接 `ReadIndexTorqueSetting` |
| main.cpp:21927-21953 | 主畫面 Torque 頁 `btnSetZ1／Z2`（運轉中不理；寫完 `fAllMotorHome=false`）、`btnReadZ1／Z2` |
| cinitial.cpp:13816-13875 `ShowMainScreenPresure` | `lbArm0Torque="1: <edTorue0> %"`（可加高度）；TSMC／Korea／SPIL 以外 `asArmForce1=<值>+"T"` |
| Command.cpp:1510-1532 `TfMain::WriteArmForce` | GPIB 回報壓力字串 `asArmForce1／2` |

## 10. UI 欄位

| 欄位 | 位置 | 內容 |
|---|---|---|
| `chkReadTorque1／2` | main.dfm:15104、:15120（`pcCommView` → `TabSheet4` 'Torque' → `pal_RS232`） | 讀扭力的武裝旗標（不是給人按的開關，程式一直改它） |
| `edTorue0／1` | main.dfm:15136、:15152 | 讀到的扭力 %（`"%5.2f"`），空字串＝還沒讀到 |
| `edtReadZ1／Z2` | main.dfm:15168、:15185 | 讀回的扭力上限（整數 %） |
| `edtSetZ1／Z2`、`btnSet／ReadZ1／Z2` | main.dfm:15232-15294 | 手動寫／讀上限 |
| `ListBox14` | main.dfm:15353（'Comm Data' 頁） | Torque log，超過 300 行清空（main.cpp:32586 起） |
| `lbArm0Torque／lbArm1Torque` | main.dfm:4176 | 主畫面「1: 12.35 %」 |
| `PnlTorue0／1` | cContact.dfm:18469、:18482 | Contact 頁的扭力欄 |

## 11. V912 差異

- rs232.cpp 的國際牌部分（:762-2009、:3763）**與 0618 完全相同**；V912 只改了溫控（`IsNoHeaterMachine`、`IsValEqual_HeaterInsOpt`）、RTC 指令、視覺光源開埠保護。行號：0618 :1–3762 的國際牌段在 V912 +6（例：`datatrq` 0618:833 → 912:839；解碼 0618:1804 → 912:1810），`GetReadTorueTask` 0618:3763 → 912:3778。
- 呼叫端：V912 atester.cpp `DoTestHeadMotor` 在 :5593；12110 的 999 判斷在 :6500、14110 在 :7238；邏輯與 0618 相同（逐行比過）。`MainProc` 的兩個呼叫在 V912 csystem.cpp:17660、:17672。

## 12. V906 移植現況

### 12.1 review6（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\rs232.cpp`）

檔頭 :1-77 把對照與差異寫得很完整，改之前先讀。重點：

| 項目 | 行 | 說明 |
|---|---|---|
| 物件 | :161-179 | `COM2` 是 `TCOM2Shim`（atester_shims.h），`Comm1` 用 vclcompat `TComm`，填 rs232.dfm 的設計期值；**刻意不設** `ReadIntervalTimeout`（檔頭 (2)：同步 handle 會讓 MainProc 的寫卡在讀取執行緒後面） |
| 分包語意 | :144-154、:181-208 | 讀取執行緒只排隊（`W906_Comm1QueueRx`），`ReadTorque()`／`ReadWriterParameter()` 開頭呼叫 `W906_PumpComm1()`，安靜滿 100 ms 才整包交給 `Comm1ReceiveData`；補一個 NUL 讓 `data[1]` 不越界（:206） |
| 建構子本體 | :212-236 `W906_CreateFormBoot` | golden :95-125 的扭力段；由 wb_serve 在 golden CreateForm 的位置呼叫（tools/wb_serve.cpp:4052）；會在 Gerneral.ini 缺鍵時回寫預設值（golden 同樣，檔頭 :75-76） |
| 開埠 | :257-302 | 扭力段照翻；`IO_CARD_TYPE==PCI1203_IO`（HT9050）不探測、不開（:263-269） |
| 照翻 | :305-925 | `TorqueSend`、分派、`ReadTorque_Panasonic`（:371-552）、組包（:557-656）、`ReadWriterParameter_Panasonic`（:681-843）、`Comm1ReceiveData`（:844-925）——跟 golden 逐行比只有：`fContact`→`fContactForm`（檔頭 (6)）、HP 卡分支閘掉（(3)）、`slTorqueLog` 那行閘掉（(7)）、三菱 `sprintf` 改有界複製（(4)）、全域改 static（(5)） |
| 寫上限＋讀回 | :936-1098 | 照翻；:950 同一行：`MOT[MTestZ1].CardType=="PCI1203"` → `W906_Ht9050TorqueLimit` |
| HT9050 上限 | :2255-2291 | 值＝%×10，經 `W906_Pci1203TorqueLimitHook`（EtherCAT/Pci1203TorqueHook.cpp:79 安裝）寫 60E0h／60E1h 並讀回；沒掛鉤回 2；Z2 在 HT9050 沒啟用視為成功；5 次對不上回 2 |
| 呼叫端 | csystem.cpp:30399、:30412（MainProc :30231）；cinitial.cpp:16988；atester.cpp:6811（12000）、:6907（12110，999 判斷 :6927）、:7087（12200）、:7537（14110，:7558）；uhome.cpp:2482、:2526 | |
| 測試 | tests/test_rs232_torque.cpp | 假 A5 伺服器：[A] 讀寫封包逐位元組比對、[F] ACK 與 ENQ 分兩次到仍當一包、[B] `iWriteAndCheckMotorTorque(0,300)` 完整跑完與 5 次失敗、[C] 讀扭力得 `"15.00"`、999、勾被清掉、[H*] HT9050 掛鉤 |

### 12.2 main 的 E-038 Phase A（`e6cec741`，已在 origin/main，不在 review6）

- main rs232.cpp:358：`ReadTorque()` 開頭同一行先問 `W906_Ht9050TorqueRead()`：-1（模擬或不是 1203）→ 照 golden 走 RS232；1＝寫了一個值 → `autoTask=999`（＝golden case 40）；2＝沒武裝 → `autoTask=1`（＝golden :854-858）；0＝等。
- 值來自 1203 監看器的 6077h，換成國際牌比例（%）、負值當 0、`"%5.2f"` 寫 `edTorue0`＋`PnlTorue0`（main IndexZTorque1203.cpp:91 起、:117-118）。
- `Gerneral.ini [IndexDriver] HT9050_INDEXZ_TORQUE_CONFIRMED`（缺鍵＝0＝不給值）、`HT9050_INDEXZ_TORQUE_BASELINE`（main IndexZTorque1203.cpp:40-41），只讀不回寫。
- 細節、正負號、測試：`ht9045-motor-control/references/index-torque-autoheight.md` §6。

### 12.3 E-044（`9432ff6e`，分支 `v906/st01e-e044`；1004 17:44 時不在 origin/main）

- 在 HT9050（`MOT[MTestZ1].CardType=="PCI1203"`）的 12110／14110 加 5 秒等待逾時：解除武裝、警報停機（暫定 WAR0361／WAR0362，K 碼照 Index 慣例）、`fAllMotorHome=false; Task=1`（e044 分支 atester.cpp:6908、:7538 同一行插入；本體 Ht9050TorqueWait.cpp／.h；測試 tests/test_e044_torque_wait.cpp）。not golden。
- 起因就是 §5 的「999 只活一拍」：golden 12110 讀不到值時永遠等。

## 13. 已知陷阱

1. **0x52 的意義**：command 2／mode 5＝Read out of present torque output（A5II p.427），不是「命令 5／模式 2」；單位是「額定扭力＝2000」，所以 `k/20`＝額定扭力的 %。手冊同頁說正負號跟扭力命令方向走。
2. **負值變 0**（:1809-1810）：往上的反作用力或負向命令全部讀成 0.00；呼叫端的 `abs()`（atester.cpp:6537、cContact.cpp:3018、:5786-5787）沒有作用。
3. **atoi 截斷**：`edTorue0` 是 `"12.35"`，呼叫端 `atoi` 成 12 才比門檻（atester.cpp:6536、cContact.cpp:5775、:3015）。
4. **`Torque[]` 只有 5 格**（cmydef.cpp:2809），:1807 卻允許 `str[1]` 到 5，:1812 用回覆的 axis 去讀 `Torque[str[1]]`——回覆 axis 是 5 就越界；回覆 axis 跟 `iReadTorueIndex` 不同時，`edTorue` 顯示的是另一格的舊值。
5. **只看長度判斷扭力**：A4 讀參數的回覆也被當扭力解碼（§6）；要加新指令得先改解碼。
6. **999 只活一拍**（§5）；12110 讀不到值不會報錯（HT9050 的 E-044 就是補這個）。
7. **逾時以拍數計**（`iPanasonicWT=500`），每次逾時都重開 COM 埠（:897-898 等）；10 次以上才跳訊息，訊息後繼續重試。
8. **ACK＋ENQ 要同一包**：靠 `ReadIntervalTimeout=100`；換通訊元件（例如移植樹的 vclcompat）一定要補「安靜 100 ms 才交付」，否則 case 20 永遠對不上。golden 只收到 1 個位元組時也讀 `data[1]`（:1832）。
9. **不驗檢查碼、不看錯誤碼、不送 NAK**；手冊規定錯了要回 NAK（A5II p.416），golden 改成自己從 ENQ 重來。
10. **寫入只在 RAM、沒取執行權**：驅動器斷電後上限回到 EEPROM 值（A5II p.433）；靠 HOME（uhome.cpp:2510／:2543）與每次下壓前重寫。手冊建議寫參數前取執行權（A5II p.425、p.419），golden 沒做。
11. **參數讀寫 axis 永遠 0、讀扭力 Z2 送 1**：兩台驅動器的位址設定沒有程式依據（程式未見）；上機要確認 Z2 驅動器對 axis 0 與 1 都會回。
12. **兩個狀態機共用 `bReceive`／`ptreot`／`ptrenq`**：參數讀寫期間讀扭力暫停在原地，回來時可能等到逾時才重來；`iWriteAndCheckMotorTorque` case 2 還會順手武裝一次讀扭力（:1890-1899）。
13. **死碼與沒人讀的旗標**：`iPanasonicNum==8`（:1636-1641）、`bPanasonicCommErr`（:1461、:1539）、`bCom1Error`（只有 HP 卡讀）。
14. **模擬組態**：`ReadTorque_Panasonic` 直接 return、`iWriteAndCheckMotorTorque` 回 1、`SwReadTorue.Enable=false`；呼叫端自己塞假值（"10.0"、30）——模擬跑得過不代表通訊對。
15. **Gerneral.ini 回寫**：`CheckAndReadIniDataGeneral` 缺鍵會把預設值寫回檔案；`D:\HT9045\system\Gerneral.ini:435` 的 `INDEX_DRIVER_TYPE` 前面有空白＋TAB（`   \tINDEX_DRIVER_TYPE=2`），本次沒有驗證 TIniFile 是否照讀——讀不到會被當成 A4（0）並回寫。

## 14. 還沒答案的

- 驅動器端 Pr5.31／RSW(ID) 實際值、繼電器 `SwReadTorue` 的實體接線（切 TXD／RXD 還是整條線）：程式未見，要上機。
- 哪些出貨機用 A4／A5／三菱／HP 卡：要收各機台 Gerneral.ini。
- 手冊 T2 單位矛盾（`protocol.md` §6）。
- 不取執行權直接寫參數，A4／A5 手冊都沒說會被拒；實務上讀回有過。
- HP 通訊卡內部是不是用本協定跟驅動器講：程式只看得到卡回的錯誤碼表（:4498），卡的韌體不在這裡。
