# Q6 回覆：vclcompat `Spcomm::TComm` 與 golden SPComm 的串列埠語意差異（Index Z 扭力 r28torq 決策用）

> **產出方式**：NB2 輔助 session 的子代理（workflow `nb2-requests-q6-q10-batch1`，同批 5 個 agent，全程只讀），主迴圈存檔前抽驗承重說法：
> * ✔ vclcompat/Comm.cpp:109 ReadIntervalTimeout(0)、:209-214 0→MAXDWORD 且 total 皆 0（屬實 ⇒ 讀取執行緒空轉）
> * ✔ Comm.cpp:263-265 開埠失敗靜默轉 SIM（屬實）
> * ✔ D:\HT9045\elec\Component\Spcomm.pas:330 FReadIntervalTimeout := 100、:542 PostThreadMessage 寫入執行緒（屬實）


> NB2 · 2026-09-25 · 基底 `D:\HT9045\_wt_assist` HEAD `6b942f15`（含 origin/main `0d3f40e9`；`vclcompat/` 與 origin/main 無差異）
> 記號：**【量】**＝讀程式碼或跑唯讀腳本得到；**【文】**＝Win32 官方文件寫的行為，本機沒量；**【推】**＝從程式碼推論，沒實測。
> 路徑簡寫：`port:`＝`HT9011UC_Cpp_V3.33.906.0/`；`golden:`＝golden 906 UTF-8 鏡像 `.nb2_scratch/golden_utf8/`（檔名同 Big5 原樹）；`SPComm.pas:`＝`D:\HT9045\elec\Component\Spcomm.pas`（Big5，行號與我轉的 UTF-8 副本相同）。

## 0. 五分鐘版

1. golden 用的 SPComm 元件**原始碼找得到**：`D:\HT9045\elec\Component\Spcomm.pas`（1,944 行）＋`spcomm.hpp`。golden 906 的 `HT9045.bpr:194` 把這個目錄放進 include path，`golden:rs232.h:6` include 它。所以下面 golden 欄是**讀原始碼**，不是文件預設值。
2. main 上**沒有任何一個**使用端會在執行期開真的 COM 埠【量＋推】。三個問題現在都是潛在的；r28torq 的 `TCOM2::Comm1` 會是第一個踩到的。
3. 三題答案：(1) 會空轉【量＋文】；(2) 會卡死 tick，有兩種卡法，第二種更容易踩【量＋文】；(3) 對，而且沒套的不只流控／DTR／RTS【量】。
4. 新電腦的修法（SPComm 預設＋寫入逾時＋閒置 Sleep(1)）**解得了 CPU 與寫入卡死，解不了「一包資料怎麼切」和「誰來處理收到的資料」**。Panasonic／三菱的扭力程式碼剛好依賴這兩件。
5. 禁止的組合：把 `ReadIntervalTimeout` 照 SPComm 改回 100，但 handle 仍是同步的 ⇒ tick 會卡在寫扭力那一行，永久停住。
6. 建議照翻 SPComm.pas（選項 B）；時間不夠先做 C；不要只做 A。要 Jimmy 裁決（§6）。

## 1. golden 的 SPComm 是哪一份

| 證據 | 檔:行 | 摘錄 |
|---|---|---|
| golden 906 include path | `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\HT9045.bpr:194` | `-ID:\HT9045\elec\Component` |
| golden 用它 | `golden:rs232.h:6` | `#include "SPComm.hpp"` |
| 元件原始碼 | `SPComm.pas:1`、`:149` | `unit SPComm;`、`TComm = class( TComponent )` |
| hpp 與 pas 一致 | `elec/Component/spcomm.hpp:247-271`、`:291` | 每個 published 屬性都 `nodefault`；`INPUTBUFFERSIZE = 0x4000`（pas `:289` 為 16384） |
| 移植樹自己也認它 | `port:vclcompat/Comm.h:8` | `D:/HT9045/elec/Component/spcomm.hpp:154-277` |

- `nodefault` ⇒ Delphi 把每個屬性值都寫進 .dfm。【量】40 個 golden TComm 物件都列出 CTS／DSR／DTR／RTS／RIT／4 個 total 逾時（腳本逐個讀，0 個缺）。所以 dfm 就是完整設定。
- 沒驗：編進 `DCLUSR60` 套件的二進位是不是就是這份 .pas（只驗了 hpp 宣告相符）。

## 2. (a) 差異表：golden SPComm vs 移植樹 `Spcomm::TComm`

| # | 項目 | golden SPComm | 移植樹 vclcompat | 後果 |
|---|---|---|---|---|
| 1 | 開檔模式 | overlapped：`FILE_FLAG_OVERLAPPED`（`SPComm.pas:384`） | 同步：`0, // synchronous I/O (blocking reader)`（`port:vclcompat/Comm.cpp:240`） | 同步 handle 上讀寫會排隊【文】（Q2 卡法 ii） |
| 2 | 讀取執行緒 | `TReadThread`：overlapped ReadFile＋`WaitForMultipleObjects(3,…,INFINITE)` 等「關閉／錯誤／資料」（`SPComm.pas:1203-1204`）；優先權 `tpHighest`（`:446`） | `ReaderProc_`：同步 `::ReadFile(im->hFile, buf, …, 0)` 迴圈（`Comm.cpp:71-89`、`:75`）；預設優先權（`:254-257`） | — |
| 3 | ReadIntervalTimeout | 建構子 `FReadIntervalTimeout := 100`（`SPComm.pas:330`）；Comm1 dfm `ReadIntervalTimeout = 100`（`golden:rs232.dfm:61`） | 建構子 `ReadIntervalTimeout(0)`（`Comm.cpp:109`）；套用時 0 換成 `MAXDWORD`（`:209-210`） | 空轉（Q1）；切包方式不同 |
| 4 | ReadTotalTimeout* | 成員，dfm 可設（`SPComm.pas:822-823`） | 沒有成員，固定 0（`Comm.cpp:211-212`） | 4 個 golden 物件設了非 0（附表） |
| 5 | WriteTotalTimeout* | 成員（`SPComm.pas:824-825`） | 沒有成員，固定 0（`Comm.cpp:213-214`） | 寫入沒上限（Q2） |
| 6 | 寫入 | `WriteCommData` 只複製一份（`LocalAlloc`，`:540`）再 `PostThreadMessage` 給寫入執行緒（`:542`），立刻回 True；寫入執行緒做 overlapped WriteFile＋`WaitForMultipleObjects(2,…,INFINITE)`（`:1880-1881`） | 呼叫端執行緒直接 `::WriteFile(pImpl_->hFile, …, 0)`（`Comm.cpp:326-328`） | golden 呼叫端永不卡；移植樹卡在 tick（Q2） |
| 7 | DCB | 從 `dcb.Flags := 1`（只開 fBinary，`SPComm.pas:749`）重建：CTS（`:756-757`）、DSR（`:759-760`）、DTR（`:762-765`）、RTS（`:785-790`）、XonLim／XoffLim（`:792-793`）、XonChar／XoffChar／ErrorChar（`:799-802`）；沒列到的位元（含 fAbortOnError、fNull）一律 0 | `GetCommState` 取驅動現值（`Comm.cpp:167`），只改 Baud／ByteSize／Parity／fParity／StopBits／fOutX／fInX／fBinary（`:170-200`） | 流控／DTR／RTS 等沿用「上一個人留下的值」（Q3） |
| 8 | 誰處理收到的資料 | 讀取執行緒 `PostMessage(hComm32Window, PWM_GOTCOMMDATA…)`（`:1635-1636`）到建構子開的隱藏視窗（`AllocateHWnd`，`:337`）；`OnReceiveData` 在**主執行緒**的 `CommWndProc` 跑（`:714-727`）。golden `MainProc` 也在主執行緒（`golden:uruncontrol.cpp:49` `Synchronize(ThreadProcess)` → `:38` `MainProc()`）⇒ 兩者**不會同時跑** | 讀取執行緒直接 `im->self->OnReceiveData(...)`（`Comm.cpp:79-83`） | 收資料的處理函式與 tick 同時跑 ⇒ 資料競爭【推】 |
| 9 | 交付緩衝 | 16384 bytes（`:289`）；另配 len+1 並補 `#0`（`:1378`、`:1389` `lpszPostedBytes[dwSizeofBuffer] := #0;`） | 1024 bytes 區域陣列（`Comm.cpp:70`），**不補 NUL** | golden 有處理函式把資料當 C 字串（§4 Q1） |
| 10 | 開啟前處理 | 檢查 `FILE_TYPE_CHAR`（`:391`）、`SetupComm(…,4096,4096)`（`:397`）、`PurgeComm` 清 TX/RX（`:409-410`） | 都沒有 | 開埠前殘留的位元組會變成第一包【推】 |
| 11 | 開埠失敗 | 丟 `ECommsError`（`:387-388`） | 靜默改 SIM：`pImpl_->bSim  = true;`（`Comm.cpp:263-265`），之後 `WriteCommData` 回 true | 見 §6 D2 |
| 12 | 重複 StartComm | 丟 `'This serial port already opened'`（`:376-377`） | 靜默 return（`Comm.cpp:224-225`） | 小 |
| 13 | StopComm | `SetEvent(hCloseEvent)`＋`PurgeComm(PURGE_RXABORT…)` 取消懸置讀取，最多等 10 s（`:625-631`） | 設旗標、等讀取執行緒 2 s（`Comm.cpp:283-285`），再 `CloseHandle`；註解假設「閒置時 ReadFile 馬上回 0 bytes」（`:278-282`） | 只有 MAXDWORD 時成立；RIT≠0 時旗標看不到【推】 |
| 14 | 錯誤／斷線事件 | `SetCommMask(EV_ERR or EV_RLSD or EV_RING)`（`:1183`）＋`OnReceiveError`／`OnRequestHangup` | 沒有；ReadFile 失敗就結束讀取執行緒（`Comm.cpp:85-86`） | Comm1 的兩個事件只寫 `bCom1Error`，golden 全樹沒人讀（`golden:cmydef.cpp:5224` 定義；`rs232.cpp:1145/4737/4742` 只寫）⇒ 對扭力無影響【量】 |
| 15 | 讀到 0 bytes | 讀取執行緒**結束**（`HandleReadData` 只處理 ≠0，`:1374`，回 False ⇒ `EndReadThread`）——golden 怪癖，只在 total≠0 時會發生 | 視為正常、繼續（`Comm.cpp:88`） | Comm1 的 total 全是 0，不受影響【推】 |

## 3. (b) 移植樹**每一個** TComm 使用端

量法：`git grep -n "\bTComm\b"`，以及所有 `StartComm/StopComm/WriteCommData/OnReceiveData/SetSimMode` 呼叫點，逐個查外層 `#if`。再用 **stale** 的 `build_nb2`（2026-09-25 09:07～09:08 建，commit 較舊）跑 `nm`：只有 4 個 TU 引用 TComm 方法——`ATC_WinWay.cpp.obj`、`cpublic.cpp.obj`、`LaserSensor.cpp.obj`、`MyBinDisp.cpp.obj`。這 4 支與 `vclcompat/Comm.*` 之後都沒有新 commit（`git log`）；之後唯一的 CMakeLists 變更 `0ca03ee6` 只加 `cDIOStatus.cpp`，它沒有 TComm。

dfm 欄順序：CTS 流控／DSR 流控／DTR／RTS／RIT／ReadTotal 乘數／ReadTotal 常數／WriteTotal 乘數／WriteTotal 常數。

| # | 使用端（port 檔:行） | 活／閘 | 開哪個埠／裝置 | golden 物件（dfm 檔:行） | dfm 值 |
|---|---|---|---|---|---|
| 1 | `OmronLaser/LaserSensor.cpp:264-267` `new Spcomm::TComm(0)`×4；`StartComm` 在 `btConnectClick`（`:880`）的 `:900/:921/:944/:966`；`WriteCommData` 17 處（`:1103`～`:1855`） | **編進、不開埠**【量＋推】：物件在靜態初始化時建（`:207` `TfLaserSensor *fLaserSensor = new TfLaserSensor();`）；建構子的 `if(USE_LASER_DISTANCE) btConnect->Click();`（`:399-400`）那時變數還是 0（`port:cmydef.cpp:3213`；ini 在 `port:database.cpp:1367` 才讀）；另兩個入口 `Timer1Timer`（`:1075`）、`TimerInArmTimer`（`:1627`，內含 `btConnectClick(0)` `:1718/:1779`）全樹 0 個呼叫者 | `HSys.asLASER_COM[0..3]`，38400，Even（1/2/InArm）／None（OutArm）；Omron 雷射測距 | `golden:OmronLaser/LaserSensor.dfm:1315/1352/1383/1420` | False/False/Enable/Enable/100/0/0/0/0（TxContinueOnXoff True） |
| 2 | `ATC/ATC_WinWay.cpp:94` 建構子收 `TComm*`；`StartComm :120`、`StopComm :135`、`WriteCommData :159/:212` | **編進、實例從不建立**【量】：`forms/fWinway.cpp:47` `arrATC_Site[i] = 0;`，`new ATC_WinWay(...)` 在 `#if 0`（`:50-61`）；呼叫端 `bthermo.cpp:4609-4659`（`#if 0` `:4570-4671`）、`forms/fLotInfo.cpp:4273-4302`（`#if 0` `:4269-4310`） | WinWay ATC 溫控（Modbus RTU 03/06，8 bytes）；golden 從設定讀埠名（`golden:ATC/WinWaySetting.cpp:124`） | `golden:ATC/WinWaySetting.dfm:322/353/384/415` | False/False/Enable/Enable/100/0/0/0/0（TxContinueOnXoff False） |
| 3 | `BinDisplay/MyBinDisp.h:356-357` `Spcomm::TComm *CommBin/*CommBin2`；`MyBinDisp.cpp:145-146` 設 NULL；`WriteCommData` 23 處（`:754`～`:1920`）、`StopComm` 10 處（`:2341`～`:3200`） | **編進、死**【量】：`port:database.cpp:225` 建的是 `TMyBinDispOffline`；`Timer1Timer` 是空樁（`MyBinDisp.cpp:332-334`）；CommBin 從未指派非 NULL | 彩色 Bin 顯示器；golden 在 Timer1Timer 設埠名（`golden:BinDisplay/MyBinDisp.cpp:362/382/401/421`），執行期改 `ReadIntervalTimeout=50`、XON/XOFF 關（`:347-349`） | `golden:BinDisplay/MyBinDisp.dfm:8/38` | False/False/Enable/Enable/100/0/0/0/0（Inx_XonXoffFlow True，執行期改 False） |
| 4 | `cpublic.cpp:157` `g_pDTKComm = nullptr;`；`WriteCommData :365/:380`（前面有 `if (g_pDTKComm)`） | **編進、產品裡是 no-op**【量】：只有測試指派（`tests/test_cpublic_foundation.cpp:91/:122`） | 台達 DTK4848；golden 走 `COM2->Comm2`，埠＝`HSys.sTempComPort`（`golden:rs232.cpp:266`） | `golden:rs232.dfm:8` Comm2 | False/False/Enable/Enable/100/0/0/0/0（TxContinueOnXoff True） |
| 5 | `cpublic.cpp:336/347/600/620/637/647` `COM2->Comm2->WriteCommData` | **閘**：`#if 0`（`:292-349`、`:581-649`） | UT100／KT4H 溫控 | 同 #4 | 同 #4 |
| 6 | `bthermo.cpp:2822/2871/2896/2935/3026/3085/3218/3267/3355` `COM2->Comm2->Stop/StartComm` | **閘**：每處自己的 `#if 0`（例 `:2821-2823`、`:2895-2897`）；此處 COM2 是 `TCOM2Shim`（`atester_shims.h:412`） | 溫控 Comm2 | 同 #4 | 同 #4 |
| 7 | `DynamicTemp.cpp:125/226-227/463/475-476/499` `COM2->TempComm6` | **閘**：`#if 0 // GATE (C1)`（`:124/:225/:462/:474/:498`） | 溫度 IC；golden `HSys.sTempDynamicComPort`（`golden:rs232.cpp:564`） | `golden:rs232.dfm:103` TempComm6（115200） | False/False/Enable/Enable/100/0/0/0/0 |
| 8 | `atester.cpp:8857` `COM2->TempComm6->WriteCommData` | **閘**：`#if 0`（`:8832-8900`）；本 TU 的 `#define COM2 (&W7T1_com2_ext)`（`:5618`），`iWriteAndCheckMotorTorque(...){ return 1; }`（`:5605`）＝r28torq 要換掉的替身 | 同 #7 | 同 #7 | 同 #7 |
| 9 | `forms/fAirCon.cpp:163/174-184/210` `CommAirCon` | **閘**：`#if 0` GATE A-1（`:150-165`）／A-2（`:167-197`）／A-11（`:199-219`）；`CommAirCon` 沒宣告（`forms/fAirCon.h:244`） | 冷氣機；golden `cAirCon.cpp:98` | `golden:cAirCon.dfm:293` | False/False/Enable/Enable/100/0/0/0/0（TxContinueOnXoff True） |
| 10 | `forms/fGroundMan.cpp:367` `comGM->StopComm()` | **閘**：`#if 0`（`:364-371`，在 SOFT_SIMULTE 的 `#else` 裡） | 接地監視；golden `GroundMan/GroundMan.cpp:286` | `golden:GroundMan/GroundMan.dfm:1845` | 同 #9 |
| 11 | `forms/fLotInfo.cpp:4295` `WinwayCOM->StopComm()` | **閘**：`#if 0`（`:4269-4310`） | WinWay ATC | 見 #2 | 見 #2 |
| 12 | `tests/test_keypro_tcomm.cpp:113/185`、`tests/test_cpublic_foundation.cpp:88/119` | 測試，全部 `SetSimMode(true)`（`:124/:186`；`:89/:120`） | 不碰真埠 | — | — |
| 加 | **r28torq（新電腦，未 commit，本機讀不到）**：`TCOM2::Comm1` | 會是**第一個真埠使用端**。要解的閘：`csystem.cpp:29974-29976`（`COM2->ReadTorque()`）、`:29988-29990`（`ReadWriterParameter()`），兩處都是 `#if 0`（`SAFETY-GATE(W906-T6-COM2TORQUE)`），對應 golden `MainProc`（`golden:csystem.cpp:16711`）的 `:16848/:16860`；以及 `atester.cpp:5618` 的替身 | 埠＝`HSys.sTorqueComPort`（`golden:database.cpp:519` `[IndexDriver] COM_PORT`，預設 COM1）；Panasonic 9600 8N1／三菱 9600 8E1／HP 卡 115200（`golden:rs232.cpp:238-255`） | `golden:rs232.dfm:39` Comm1 | `:43` False／`:44` False／`:45` DtrEnable／`:52` RtsEnable／`:61` 100／`:62-65` 0,0,0,0；另掛 `OnReceiveError`／`OnRequestHangup`（`:67-68`） |

- 附帶（非 TComm）：`KYECFTP/MiniFtpEngine.cpp:245/247/358` 的 `SetSimMode` 是 socket 的，不算。
- 結論【量＋推】：main 上 **0 個**使用端在執行期開真埠。

### 附表：golden 有、移植樹還沒有使用端的 25 個 TComm（給修法設計用）
- 40/40 個 golden 物件都是 CTS=False、DSR=False、DTR=Enable、RTS=Enable【量：腳本 `.nb2_scratch/agent_tmp/q6/dfm_tcomm.py`】。
- golden 執行期只有兩處改這類屬性，都是 RIT／XON-XOFF，不是流控【量：golden 全樹 grep】：`golden:Motor/TrayStepMotor.cpp:71` `ReadIntervalTimeout=1`、`golden:BinDisplay/MyBinDisp.cpp:347-349` `=50`＋XON/XOFF 關。
- 和 SPComm 建構子預設不同的只有：

| 物件（golden dfm 檔:行） | 不同處 |
|---|---|
| `rs232.dfm:327` cmVisionLight | XON/XOFF 開；ReadTotal 100/1000；WriteTotal 100/1000 |
| `MR/RFID.dfm:1375/1406` RFID_1/2 | XON/XOFF 開；ReadTotal 10/1000；WriteTotal 10/1000 |
| `uLotInfo.dfm:14547` RFID_Reader | XON/XOFF 開；RIT 700；ReadTotal 10/1000；WriteTotal 10/1000 |
| `fAOI.dfm:4050/4080` AOIComm/TopAOIComm | XON/XOFF 開；RIT 10 |
| `rs232.dfm:296` PadComm、`TempCtrl/MyTempture.dfm:7` Tempture | XON/XOFF 開 |
| 其餘 17 個（Comm3、Comm4、cmATC1-4、ACTCom、Barcode_1-4、CommOmron、comTrayStepMotor、CommOcr×3、commRFID） | 只有 TxContinueOnXoff／埠名／鮑率不同 |

## 4. (c) 三個問題

### Q1 `ReaderProc_` 在 RIT=MAXDWORD、total=0 時會不會空轉？ → **會**
- 【量】建構子 `ReadIntervalTimeout(0)`（`Comm.cpp:109`）；移植樹沒有任何使用端改它（`git grep` 的 `->ReadIntervalTimeout`／`.ReadIntervalTimeout` 只在 Comm.* 內）。
- 【量】套用時 `ReadIntervalTimeout ? ReadIntervalTimeout : MAXDWORD`（`:209-210`），兩個 ReadTotal 都 0（`:211-212`）。
- 【文】COMMTIMEOUTS 文件：RIT=MAXDWORD 且兩個 ReadTotal=0 ⇒ ReadFile 立刻回傳已收到的位元組，沒有也立刻回。
- 【量】0 bytes 時什麼都不等就再讀：`// ok && nRead==0 (interval timeout): loop and re-check the stop flag.`（`:88`）。
- ⇒ 每個開成真埠的 TComm，一條執行緒不停呼叫 ReadFile。「吃滿一顆核心（多為核心態時間）」是【推】；本機沒 COM 埠也不准執行，沒量。
- 【量】作者以為這是「間隔逾時」（註解 `:206-208`、`:278-282`），但 MAXDWORD 其實是「完全不等」。
- 空轉之外的副作用【推】：golden 是「停 100 ms 沒新位元組才交一包」（`SPComm.pas:330`＋`golden:rs232.dfm:61`）；移植樹變成「驅動緩衝裡剛好有多少就交多少」，一包怎麼切**由硬體決定**（主機板 UART 的 FIFO 逾時、USB 轉接器的 latency timer、PCIe 多埠卡各不同）。扭力碼有 3 處依賴「一個回覆＝一包」：

| golden 檔:行 | 依賴什麼 | 被拆包時 |
|---|---|---|
| `rs232.cpp:1801` `if(BufferLength==7 \|\|`（A5 是 9） | Panasonic 扭力值要一包 7／9 bytes | 不解析 ⇒ 讀回值不更新 ⇒ `iWriteAndCheckMotorTorque` 重試 5 次後 `return 2;`（`:1961`） |
| `rs232.cpp:938`、`:1596` `if(ptreot==ACK[0] && ptrenq==ENQ[0])`；兩值取自同一包的 `data[0]`／`data[1]`（`:1831-1832`） | ACK 與 ENQ 要在同一包 | 條件不成立 ⇒ `Task=1` 重來，10 s 後逾時（`:1406` `SetSecAndOn(10)`） |
| `rs232.cpp:1827` `::sprintf(cReciveMitsubishi_Data, data);`，目的地 `char cReciveMitsubishi_Data[32]`（`:55`） | 三菱回覆要一包，**且要 NUL 結尾** | 移植樹不補 NUL（`Comm.cpp:70`、`:81-83`）⇒ sprintf 讀過界，可能寫爆 32 bytes 全域【推】 |

- HP 卡路徑不怕拆包：`TimerHPCardTimer` 自己把各包接起來（`golden:rs232.cpp:4330-4345`）。但它的 `_byte_datas` 會被讀取執行緒（`:1775` `push_back`）與計時器同時碰【推】。
- 以上都是「失敗→報警」，不是「假成功」：驗證步驟比對讀回值（`golden:rs232.cpp:1947-1990`）。

### Q2 `WriteCommData` 同步 WriteFile、WriteTotal=0，流控卡住時會不會卡死單執行緒 tick？ → **會（兩種卡法）**
- 【量】寫入在呼叫端執行緒：`::WriteFile(pImpl_->hFile, pDataToWrite, …, &nWritten, 0)`（`Comm.cpp:326-327`）；WriteTotal 兩個都 0（`:213-214`）。
- 【文】WriteTotal 兩個都 0 ⇒ 寫入不使用總逾時。
- 【量】wb_serve 只有一條執行緒推機台（`tools/wb_serve.cpp:204-205`），tick 500 ms（`:2588`）；看門狗 5 s 後只印訊息、不救（`:218`、`:226`）。
- **卡法 i（流控）**：DCB 的 CTS／DSR 握手沿用驅動現值（Q3）。若現值是開的、線上又沒有 CTS／DSR（3 線 RS232 常見）⇒ WriteFile 不回 ⇒ MainProc 停、所有網頁命令 no ack【推】。XOFF 同理，但 Comm1 的 XON/XOFF 是關的（`golden:rs232.dfm:48-49`）。
- **卡法 ii（同步 handle 排隊）**：【文】沒有 `FILE_FLAG_OVERLAPPED` 的 handle，I/O 會序列化；一條執行緒卡在 ReadFile 時，另一條的 WriteFile 會被擋住（MSDN CreateFile 文件；MSDN 技術文章 Serial Communications）。現在 RIT=MAXDWORD，ReadFile 立刻回，所以只是搶鎖、通常很快【推】。**但只要有人把 RIT 設成非 0（包括「照 SPComm 預設 100」），ReadFile 會等到第一個位元組才回**【文】⇒ tick 的 WriteFile 排在它後面 ⇒ 扭力驅動器一問一答、不會先開口 ⇒ **永久卡死**【推】。golden 另有兩處照翻就會踩到：`TrayStepMotor.cpp:71`（RIT=1）、`MyBinDisp.cpp:347`（RIT=50）。
- 沒流控、沒排隊時：只卡傳輸時間，9600 baud 約 1 ms/byte（20 bytes ≈ 21 ms）【推】，相對 500 ms tick 可忽略。
- golden 對照【量】：`WriteCommData` 只是丟給寫入執行緒（`SPComm.pas:542`），呼叫端永不卡；寫入執行緒卡住時主執行緒照跑，扭力狀態機靠計時器逾時，報 `"Torque Comm Timeout. …"`（`golden:rs232.cpp:1942/:2000`）。golden 呼叫端也不看回傳（`golden:rs232.cpp:769` `Comm1->WriteCommData(str, len);`）。
- 附帶【推】：RIT≠0 時 StopComm 的 2 s 等待（`Comm.cpp:285`）每次讓 tick 停 2 s；golden 扭力逾時會 `StopComm(); StartComm();`（`golden:rs232.cpp:897-898` 等約 10 處）。`StartComm` 把停止旗標清回 0（`Comm.cpp:251`），舊讀取執行緒若還卡在 ReadFile，會變成兩條執行緒讀同一個埠。`CloseHandle` 在另一條執行緒卡在同步 ReadFile 時會不會也等，沒查證。

### Q3 `ApplyCommState_` 沒套 dfm 的流控／DTR／RTS？ → **對，而且不只這三項**
- 【量】成員只有 9 個（`Comm.h:140-148`）；檔頭明寫不做：`Read/WriteTotalTimeout*,`（`:39`）、`Dtr/RtsControl`（`:40`），理由是「906 從沒用過」。這個前提漏了 .dfm：40 個物件全都寫了這些值（§1）。
- 【量】`ApplyCommState_` 從 `GetCommState` 現值出發（`Comm.cpp:167`）。沒動的欄位：fOutxCtsFlow、fOutxDsrFlow、fDtrControl、fRtsControl、fDsrSensitivity、fTXContinueOnXoff、fErrorChar、fNull、fAbortOnError、XonLim、XoffLim、XonChar、XoffChar、ErrorChar。golden 從 `dcb.Flags := 1` 開始（`SPComm.pas:749`），沒列到的一律 0。
- 【推】所以移植樹行為取決於「上一個開這個埠的程式留下什麼」或驅動預設：
  - 沿用 CTS/DSR 握手開 ⇒ Q2 卡法 i。
  - 沿用 DTR/RTS 關 ⇒ 靠 DTR/RTS 供電或判斷連線的裝置不回話。
  - 沿用 `fAbortOnError=1` ⇒ 一次同位／框架錯誤後 ReadFile 失敗 ⇒ 讀取執行緒結束（`Comm.cpp:85-86`），埠變聾、沒有訊息。三菱是 Even 同位（`golden:rs232.cpp:243`），比較容易碰到。
  - 沿用 `fNull=1` ⇒ 0x00 被丟 ⇒ Panasonic 二進位封包損毀（例 `golden:rs232.cpp:1378` `{0x02, 0x00, 0x07, 0x00, 0x0D, 0x00}`）。
- 【推】剛開機、沒人改過設定的機台，驅動預設多半是「流控關、DTR/RTS 開」，那時與 golden 相同。差異只在別的程式（例如驅動器設定工具）動過設定後才冒出來——最難查的那種。

## 5. (d) 建議修法與對每個使用端的影響

原案拆成 3 件，加上原案沒涵蓋的 3 件：

| 件 | 做法 | 解什麼 | 注意 |
|---|---|---|---|
| ① DCB 照 SPComm | 補成員（Outx_CtsFlow、Outx_DsrFlow、DtrControl、RtsControl、DsrSensitivity、TxContinueOnXoff、XonLimit…ReplacedChar），預設＝SPComm 建構子（`SPComm.pas:309-334`）；`ApplyCommState_` 照 `_SetCommState`（`:729-805`）從 `Flags=1` 重建 | Q3；也拿掉 Q2 卡法 i 的來源 | 沒有 dfm 載入器，每個使用端要自己抄 dfm 值。Comm1 的 SPComm 預設與 dfm 只差 TxContinueOnXoff（dfm True），XON/XOFF 關時無作用 |
| ② WriteTotalTimeout | 補 WriteTotal 兩個成員；真埠用例如「常數 100 ms＋乘數 2 ms/byte」（數字是建議，golden 是 0） | Q2 剩下的無限等待 | 偏離 golden（golden 不在呼叫端寫，所以不需要）。逾時後 `WriteCommData` 回 false（`Comm.cpp:328`），golden 呼叫端不看 ⇒ 走原本逾時／報警路徑【推】 |
| ③ 閒置 Sleep(1) | 0 bytes 時 `Sleep(1)` | Q1 的 CPU | **不**恢復 golden 切包。wb_serve 沒呼叫 `timeBeginPeriod`（`tools/wb_serve.cpp:4045`；`uruncontrol.cpp:175-177` 那處也在 `#if 0`），Sleep(1) 實際可能約 15.6 ms【推】 |
| ④ 補 NUL（原案沒有） | 交付緩衝配 len+1 補 0，照 `SPComm.pas:1378-1389` | 三菱 sprintf 讀過界 | 很小 |
| ⑤ 事件交付執行緒（原案沒有） | 讀取執行緒只把資料排進佇列；tick 執行緒在 MainProc 前取出並呼叫 `OnReceiveData`＝golden「PostMessage 到主執行緒」的等價物 | 跨執行緒競爭（`bReceive`、`ptreot`、`asReceiveTorue`、`_byte_datas`、TEdit 文字） | `SimInjectReceive` 要維持同步觸發，否則 `tests/test_keypro_tcomm.cpp:168/:178` 的斷言要改 |
| ⑥ 切包（原案沒有） | B：overlapped＋RIT 照 dfm；C：讀取執行緒自己「停 RIT 毫秒沒新位元組才交一包」 | Q1 副作用（Panasonic／三菱判斷） | 不可用「同步 handle＋RIT=100」來做（Q2 卡法 ii） |

- B 的可行性【量】：本機 PATH 上的 MinGW `C:\MinGW\include\winbase.h` 有 `PurgeComm`（`:2234`）、`GetOverlappedResult`（`:1784`）、`CancelIo`（`:1350`），沒有 `CancelIoEx`。golden 本來就用 PurgeComm 取消讀取，不需要 CancelIoEx（Comm.cpp:281-282 擔心的那件）。

### 每個使用端的影響

| 使用端 | 現在 | A＝①②③＋④ | B＝照翻 SPComm.pas（含 ⑤） | C＝①～⑥（同步 handle＋自己組包） |
|---|---|---|---|---|
| #1 LaserSensor ×4 | 不開埠 | 執行期不變 | 執行期不變；建構子不可開執行緒／事件（它在靜態初始化時跑，`LaserSensor.cpp:207`） | 執行期不變 |
| #2 ATC_WinWay ×4 | 不建實例 | 不變 | 不變 | 不變 |
| #3 MyBinDisp ×2 | NULL | 不變 | 不變 | 不變 |
| #4 cpublic `g_pDTKComm` | nullptr | 不變 | 不變 | 不變 |
| #5～#11（`#if 0`） | 不編譯 | 不變；日後解閘直接受益 | 同左 | 同左 |
| #12 測試（SIM） | SIM | 不變：SIM 路徑不經 `ApplyCommState_`／`ReaderProc_`（`Comm.cpp:316-322`、`:138-152`） | SIM 路徑保持原樣就不變 | 同 B |
| r28torq Comm1：CPU | 空轉 | 修好 | 修好 | 修好 |
| r28torq：DCB | 沿用現值 | ＝golden | ＝golden | ＝golden |
| r28torq：寫入卡 tick | 可能無限 | 最多約 100 ms＋2 ms/byte | **永不卡**（寫入執行緒） | 最多約 100 ms＋2 ms/byte |
| r28torq：Panasonic 切包 | 看硬體 | 看硬體＋Sleep 解析度 | ＝golden（停 100 ms 才交） | 約等於 golden（±Sleep 解析度） |
| r28torq：三菱 NUL | 讀過界 | 修好 | 修好 | 修好 |
| r28torq：收資料在哪條執行緒 | 讀取執行緒（競爭） | 讀取執行緒（競爭） | tick 執行緒＝golden | tick 執行緒 |
| r28torq：StopComm/StartComm 重試 | 快 | 快 | 快（SetEvent＋PurgeComm，`SPComm.pas:625-628`） | 快 |
| 工作量（估計） | — | 約 40 行 | 約 350～450 行 C++（對應 `SPComm.pas:1105-1932`） | 約 120 行 |

- 驗證（三案都一樣）：要在機台上、扭力驅動器接好時驗。筆電可先用虛擬 COM 對（例 com0com）驗 CPU、切包、StopComm 延遲、寫入逾時。

## 6. 待 Jimmy

### D1：串列埠元件要補到什麼程度（決定 r28torq 會不會卡 tick）
- **白話**：我們的串列埠元件，讀法、寫法、誰處理收到的資料，三樣都跟 BCB6 不同。扭力是第一個真的用它的功能。要決定補多少。
- **舉例**：Panasonic 驅動器回「ACK＋ENQ」兩個位元組。BCB6 等 0.1 秒沒新資料才一起交出，`ptreot==ACK && ptrenq==ENQ` 成立。我們這邊若硬體把它拆成兩次交，判斷不成立，扭力寫入一直重試，最後報「扭力異常」。另一例：若有人照 SPComm 把 `ReadIntervalTimeout` 改回 100、但沒改成 overlapped，按 START 後 tick 可能卡在寫扭力那行，整台停、網頁全部 no ack。
- **選項**：
  - **A**：原案最小修（DCB 照 dfm＋寫入逾時＋閒置 Sleep(1)＋補 NUL）。約 40 行。殘留：切包看硬體；收資料仍與 tick 同時跑。
  - **B**：照翻 `D:\HT9045\elec\Component\Spcomm.pas`（overlapped、讀寫各一條執行緒、關閉事件；RIT／total 照每個 dfm；收到的資料由 tick 執行緒交付）。約 350～450 行。§2 的差異全部消失，寫入永不卡 tick。
  - **C**：折衷（保留同步 handle＋MAXDWORD，讀取執行緒自己「停 RIT 毫秒才交一包」＋補 NUL＋Sleep(1)；寫入逾時；DCB 照 dfm；tick 執行緒交付）。約 120 行。殘留：寫入仍在 tick 上同步（有上限）；組包時間有 ±Sleep 解析度誤差。
  - **禁止**：不論選哪個，都不可以「同步 handle＋RIT≠MAXDWORD」。
- **建議**：**B**。理由：忠於 golden；元件原始碼就在，golden 906 就是編它（`HT9045.bpr:194`）；一次解掉 15 項差異，未來 25 個 golden 物件只要照 dfm 抄值。時間不夠（T＝9/29 09:00）先做 **C**，不要只做 A。

### D2：真機組態下開埠失敗，要不要照 golden 報錯
- **白話**：現在 COM 埠打不開時，元件會假裝開成功、改成模擬，寫入回 true（`Comm.cpp:263-265`）。
- **舉例**：機台上扭力埠被別的程式佔住。BCB6 丟例外，`RS232Init` 的 catch 跳「Index Torque : COM1 port error」（`golden:rs232.cpp:257-261`）。我們這邊不跳，之後才出現「Torque Comm Timeout」，會往錯的方向查。
- **選項**：**A** 非 SOFT_SIMULTE 組態照 golden 丟例外（翻過來的 try/catch 會顯示 golden 的訊息）；**B** 維持靜默轉 SIM；**C** 不丟例外，但 `IsOpen()` 回 false、`WriteCommData` 回 false、寫 log。
- **建議**：**A**。與常設裁決「模擬／真機只看建置期 SOFT_SIMULTE」一致（`D:\HT9045\_wt_assist\CLAUDE.md:289`）；`port:vclcompat/ClientSocket.h:174-184` 已對安全 IO 做了同樣的決定。

## 7. 沒能驗證的事
1. 沒有任何執行期量測：本機沒 COM 硬體，任務規則不准建置／執行。空轉幅度、寫入卡多久、實際怎麼切包、`CloseHandle` 會不會等，全是【文】＋【推】。移植樹 `docs/DEVLOG.md:444` 也寫「TComm real \\.\COMx 路徑已編未測(無硬體)」。
2. r28torq 內容讀不到（在新電腦）：不知道它怎麼接 Comm1、有沒有改 RIT、是否解了 `csystem.cpp:29974/29988` 的閘、收資料在哪條執行緒。
3. `Spcomm.pas` 與編進 `DCLUSR60` 的二進位是否同一版：只比了 hpp 宣告。
4. HT9050 的扭力埠是哪種硬體（主機板 COM／PCIe 多埠卡／USB）、驅動器型號（`INDEX_DRIVER_TYPE`、`USE_HP_COM_CARD`）：`machines/HT9050/` 沒有 Gerneral.ini。切包會不會真的被拆，取決於這個。
5. 機台上那個埠目前的 DCB：可在機台上、wb_serve 啟動**前**跑唯讀的 `mode COMx`（換成實際埠號），看 CTS handshaking／DSR handshaking／DTR circuit／RTS circuit。
6. nm 用的是 stale 的 `build_nb2`（09:07～09:08 建）；已用 `git log` 確認 4 支相關 TU 與 Comm.* 之後沒改，但沒重建驗證。
7. 範圍外、順手記：golden 有用「tick 數」算逾時的地方（`golden:rs232.cpp:762` `iPanasonicWT=500`，golden tick 約 1 ms，`golden:uruncontrol.cpp:45` `timeBeginPeriod(1)`）；移植樹 tick 500 ms，放大 500 倍（約 250 s），實際會先被 10 s 計時器截斷（`golden:rs232.cpp:1406`）。沒細查。

## 8. 檔案（絕對路徑）
- D:\HT9045\_wt_assist\HT9011UC_Cpp_V3.33.906.0\vclcompat\Comm.h
- D:\HT9045\_wt_assist\HT9011UC_Cpp_V3.33.906.0\vclcompat\Comm.cpp
- D:\HT9045\elec\Component\Spcomm.pas（Big5）
- D:\HT9045\elec\Component\spcomm.hpp
- D:\HT9045\_wt_assist\.nb2_scratch\agent_tmp\q6\Spcomm_utf8.pas（我轉的 UTF-8 副本，0 個 U+FFFD）
- D:\HT9045\_wt_assist\.nb2_scratch\agent_tmp\q6\dfm_tcomm.py（掃 40 個 dfm TComm 物件的腳本；`python dfm_tcomm.py full` 或 `compact`）
- D:\HT9045\_wt_assist\.nb2_scratch\golden_utf8\rs232.dfm、rs232.cpp、uruncontrol.cpp、csystem.cpp
