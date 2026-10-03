---
name: ht9045-adam6024
description: HT9045 / HT9050 的 ADAM-6024（研華 12 通道萬用 I/O 模組）與 EP 電氣比例閥（下壓力道 Contact Force／Die Force 的設定值與回授）知識庫：硬體規格與 Modbus 位址、ASCII／UDP 指令、EP_Install 各型態（3＝ADAM-6024 含回授、4＝PISO ET-7226、5＝Two EP…）、APAX 獨立／Multi EP、廠商 ADAMTCP.dll（匯出、回傳碼、逾時）、golden 912 adam6024.cpp 49 個函式地圖與呼叫者、公斤→TransformFuntion→輸出碼→ADAM_DirectWriteData／APAX_WriteData 的寫出流程、AI→ADAM_ReadPA→KpaTransferKG→警報的讀回流程、906 與 912 差異、V906 移植現況（20261003：St02 照 912 翻的 ADAM 碼已在 main（MR !114，`9e46491f`），RULINGS_20261002 第 20c 條保留 912；EP 寫出由 `W906_ADAM_EP_LIVE` 關著；機台 WinLibs g++ 16.2 字面值精度的 A3 修正是 MR !135 `2428cf0a`，還沒進 main）、連線／壓力排錯與警報意義、碰這塊的規則。Use when：問 ADAM-6024 或 EP 氣壓、EP 設定值寫不出去、EP 回授不準、WAR1605／WAR16322／WAR16323／WAR0329、Connect Fail! Please Check ADAM IP!、Adam Connect Error and Stop Home、ADAM 連線數滿 8 條、韌體 6.01 B21、要移植或審查 adam6024、ADAMTCP.dll 執行時載入、atester_shims 的 ADAM 替身、Timer2 的 EP 寫出、HT9050 的 172.16.8.110。關鍵字：ADAM-6024, ADAM6024, adam6024.cpp, ADAMTCP, ADAMTCP.dll, ADAMTCP_WriteReg, ADAMTCP_Read6KAI, ADAMTCP_SendReceive6KUDPCmd, ADAMTCP_Connect, EP, 電氣比例閥, 電子調壓閥, EP_Install, INSTALL_DOUBLE_EP, CHECK_EP_SETTING, EP_MAXKPA, EP_MINA_FeedBack, TransformFuntion, KpaTransferKG, MultiTransferKG, AdamOutputToPA, ADAM_ReadPA, ADAM_ReadVoltage, ADAM_WriteVoltage, ADAM_DirectWriteData, ADAM_WriteMaxData, ADAM_Alarm, ADAM_DualAlarm, ADAM_ReturnValueCheck, ADAM_ReadAIValue, Open_ADAM_6024, Close_ADAM_6024, fCheckConnectStatus_ADAM6024, ClearAllConnection, APAX, APAX_WriteData, APAX-5070, ADSMOD, ET-7226, PISO DA, EPSwitchOnOff, EpSwitch, SwEpArm1, SwMultiEp, SnEPDieForce, 露點計, DewPoint, 172.16.8.110, 172.16.8.111, 172.16.8.112, Modbus/TCP 502, iWritePA, iAdamOutValue, iReadAdamEP, ST02-ADAM, AdamTcpShim, Adam6024_St02.h, 75756cab, 3c348627, GATE FW3-WA, W906-HOME-C2-GPIBADAM, 第 20c 條, #20c, W906_ADAM_EP_LIVE, W906_AdamEpLive, MR !114, 9e46491f, WinLibs, g++ 16.2.0, excess precision, FLT_EVAL_METHOD, A3, P18, MR !135, 2428cf0a, Adam6024_Pressure, FP_ORACLE_FINDINGS。
---

# HT9045 ADAM-6024／EP 知識庫

> ⛔ **20261003 現況，先讀這段**
> - **第 20c 條：ADAM-6024 照 912 保留**（`HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20261002.md` 第 23 條第 2 項＝C，使用者 1002 22:4x）。理由：912 的 `MultiTransferKG` 小數修正（Eastsun 20260710「修正沒有小數點」；906 的參數是 `int`，0.5 kPa 會被截成 0）是**真的 bug 修正**。所以這是第 20 條「只做 906」的例外，跟第 20a 條（溫控）、第 20b 條（HANA）同類；St02 **不用**照 906 重翻（第 20 條 1002 18:0x 那句「退回照 906 重翻」由這一項取代）。
> - St02 的移植（卡 **ST02-ADAM**，helper H1～H4）**已在 main**：MR !114 `v906/st02-adam6024`，Steven 1002 18:51 合進 main（`9e46491f`），檔案見 §5.3。
> - **EP 寫出仍然沒有上線**：總開關 `W906_ADAM_EP_LIVE` 在 `HT9011UC_Cpp_V3.33.906.0\MachineType.h:1809` 是註解掉的（預設關）；關著時不載 `ADAMTCP.dll`、寫出入口的回答跟舊替身一樣、Timer2／開機／關程式／HOME 的 EP 動作都直接返回（`Adam6024Integrate_St02.cpp:36-41`）。第 20c 條寫明「總開關仍關，要開時另外通知、EastSun 在旁」。不要把本 skill 讀成「EP 已上線」。
> - **機台的編譯器跟 BCB6 判斷不同**：HT9050 機台用 WinLibs g++ 16.2.0 建置，`Adam6024_Pressure` 在機台紅（4q／5e／5f／9t）；St02 的修正是 **MR !135**（`2428cf0a`，還沒進 main），**合了以後要在機台的 WinLibs 線上重跑 Adam6024_Pressure**。細節 §5.4。

## 0. 路徑與記號

| 記號 | 絕對路徑 | 說明 |
|---|---|---|
| golden 912（主來源；RULINGS_20261002 第 20c 條的例外） | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` | Big5，用 cp950 讀。`adam6024.cpp` 3104 行、`adam6024.h` 92 行、`ADAMTCP.h` 266 行 |
| golden 906 | `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\` | 本機用 0625_Steven 樹；0618 樹的行號在 :28427 之前 +78 才對得上 |
| V906 移植樹 | repo `HT9011UC_Cpp_V3.33.906.0\` | UTF-8。本 skill 的行號查於 origin/main `36f09560`（worktree `D:\AI_TempFile\st02-s18\HT9011UC_Cpp_V3.33.906.0\`）；main checkout `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` 的行號之後可能移動。20261003 加的段落（開頭的現況、§4 的 ⚠ 補註、§5.1 的補註、§5.3、§5.4）量於 origin/main `577c41c5`（含 MR !114）與 MR tip `2428cf0a` |
| 手冊 | `E:\HT9045W_相關料件技術文件\ADAM\ADAM-6000_Series_Manual_Ed2.pdf` | `p.N` = PDF 第 N 頁（印刷頁碼 N-10） |
| 廠商 DLL | `D:\HT9045\EXE\ADAMTCP.dll` | 只准 `objdump -p` 看，**永遠不載入** |

引用寫法：golden 的引用只寫 `檔名:行` 時，前面省略的是上表 **golden 912 根目錄**（例：`main.cpp:21677` ＝ `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:21677`）；906 一律明寫「906」；移植樹一律寫 `HT9011UC_Cpp_V3.33.906.0\…` 或標明「移植樹」。

細節分三份：
- `references/hardware-and-dll.md`：模組規格、Modbus 位址、ASCII 指令、EP_Install／INSTALL_DOUBLE_EP、APAX、ADAMTCP.dll 匯出與回傳碼、逾時。
- `references/golden-code-map.md`：49 個函式（912／906 行號＋一句話）、全域、資料流、全部 golden 呼叫者。
- `references/troubleshooting.md`：連不上、壓力不對、警報代碼、golden 留下的紀錄。

## 1. 硬體是什麼、機台拿它做什麼

**ADAM-6024**（研華）：6 AI（16-bit，±10 V／0~20 mA／4~20 mA）、2 AO（12-bit，0~10 V／0~20 mA／4~20 mA）、2 DI、2 DO；10/100 Ethernet，Modbus/TCP（埠 502）＋UDP（ASCII 指令）；10~30 VDC（手冊 p.41-43）。出廠 IP `10.0.0.1`（p.77）；一顆最多 8 條 TCP 連線，Host Idle 逾時會收回閒置連線（p.79）；韌體用 Utility 的 Firmware 頁更新，但 6024 不支援更新網頁檔（.html/.jar）（p.80）。Modbus：AI0～5 = 40001～40006，**AO0 = 40011、AO1 = 40012**（p.240）。

機台用法（golden 912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\adam6024.cpp`）：

| 用途 | 通道 | 出處 |
|---|---|---|
| **主 EP 設定值**（Contact Force，下壓力道） | AO1（`ADAMTCP_WriteReg(ip,1,11+iAdd,…)`，預設 `iAdd=1` → reg 12） | `adam6024.h:23`、`adam6024.cpp:1994` |
| Die Force EP（`INSTALL_DOUBLE_EP=1`） | AO0（`iAdd=0` → reg 11） | `main.cpp:6519`、`main.cpp:21729` |
| 主 EP 回授（1～5 V） | AI5（`ADAM_ReadPA` 預設 `iCH=5`） | `adam6024.h:8`、`adam6024.cpp:507-526` |
| Die Force 回授 | AI2 | `adam6024.cpp:519-524` |
| 露點計（4~20 mA） | AI3 | `adam6024.cpp:475-485`、`:2129-2204` |

開機時 golden 把 AI3 設 4~20 mA、其他 AI 設 ±10 V、兩個 AO 設 **4~20 mA**（`adam6024.cpp:287-357`）。三個 IP：`Address[0]=172.16.8.110`（EP）、`[1]=172.16.8.111`（CKD FCM 吹氣，`USE_CKD_FCM_CleanAir`）、`[2]=172.16.8.112`（APAX，`INSTALL_DOUBLE_EP` 2/3）（`adam6024.cpp:42`、`:400-409`）。

**`EP_Install`**（[System]；選項文字在 golden `HandlerSys.dfm` 的 `ElectronPressure`）：0 未安裝／1 Analog（ADAM，0..4095）／2 Digital（10 個 DO）／**3 Digital and Encoding＝ADAM-6024＋回授檢查**／**4 PISO ET-7226**（`TClientSocket` 埠 10001，自組 Modbus 封包）／5 Two EP（同一顆 ADAM 的 AO0/AO1 各一支臂）。細節 `references/hardware-and-dll.md` §3。
**APAX**：`INSTALL_DOUBLE_EP` 2（獨立 EP）／3（Multi EP，APAX-5070＋2×APAX-5028）時，每個 site 的碼（×16）經 ADAMTCP 的 Modbus/TCP 寫到 `172.16.8.112`（`APAX_WriteData`，`adam6024.cpp:2305-2673`）。`Open_APAX`（ADSMOD.dll）在 golden 912 沒有呼叫者。

**HT9050**：`EP_Install=3`，ADAM-6024 @`172.16.8.110`（`HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260929.md:44`）。Frank 的共用模擬組 `machines\HT9050\sim_9378\Gerneral.ini` 是 `EP_MAXKPA=899`、`EP_MAXA=5.013`、`EP_MINMPA=0.005`、`EP_MINA_FeedBack=0.908`、`INSTALL_DOUBLE_EP=1`、`CHECK_EP_SETTING=1`（模擬組，不是機台正本）。HT9050 只有 Z1 會下壓（同一份 RULINGS 第 6 節）。

## 2. 廠商 DLL：ADAMTCP.dll

- `D:\HT9045\EXE\ADAMTCP.dll`：PE32 i386（**只有 32 位元行程載得進**）、71 個匯出、名稱不裝飾、`__stdcall`（`ADAMTCP.h:154-265` 的 `EXPORTS … CALLBACK`）、只匯入 WS2_32/KERNEL32/USER32（含 WSAStartup／WSACleanup）。20261002 用 `C:\MinGW\bin\objdump.exe -p` 量，沒有載入。
- golden 連 `ADAMTCPbc.lib`（Borland 匯入庫，`HT9045.bpr:155`）＝靜態匯入。V906 照 **RULINGS_20260929 第 5 節第 6 條＝A**：執行時 `LoadLibrary("ADAMTCP.dll")`（exe 資料夾，再 `D:\HT9045\EXE`），沒有 DLL 也編得過，DLL 不在時走 golden 自己的錯誤路徑（`ADAMTCP_Open Fail!`，`adam6024.cpp:278-284`）。前例 `HT9011UC_Cpp_V3.33.906.0\Public\HTKeyProShim.cpp`。
- golden 912 只用 11 個進入點：`Open/Close/Connect/Disconnect/WriteReg/ReadReg/Read6KAI/UDPOpen/UDPClose/SendReceive6KUDPCmd/GetHostIdleTime`（`ReadReg` 只在客戶分支 `HS_Function.cpp:1166`）。
- 連線：`ADAMTCP_Connect(ip, 502, 2000, 2000, 2000)`（`adam6024.cpp:26`、`:36-38`、`:359`）；讀 AI 用 `ADAMTCP_Read6KAI(ip, 6017, 1, …)`（`:463`，**6017 是 golden 原文**，和研華範例 `E:\HT9045W_相關料件技術文件\ADAM\ADAM-6024.doc` 相同，不要改）。
- 回傳碼 0、-1～-15、-100（`ADAMTCP.h:126-143`；同 `E:\HT9045W_相關料件技術文件\ADAM\ADAM\ADAMTCP_SendReceive6KUDPCmd_Error Code.txt`）。golden 用 `ADAMErrorMessage[-iRet]` 顯示（表 16 格，`adam6024.cpp:124-139`；-100 會越界）。
- 筆電 3c348627 指出 `ADAMTCP_Close` 會無條件 `WSACleanup`，會把 wb_serve 整個行程的 socket 清掉。本 skill 只確認 DLL 有匯入 WSACleanup，沒有反組譯驗證。

## 3. golden 程式地圖（912）

49 個函式本體逐一列在 `references/golden-code-map.md` §1。最重要的幾支（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\adam6024.cpp`）：

| 函式 | 912 行 | 角色 |
|---|---|---|
| `Open_ADAM_6024(IP,Num)` / `()` | 248-398 / 400-409 | 連線（狀態檢查、韌體新舊、範圍設定、`ADAMTCP_Connect`） |
| `Close_ADAM_6024` | 411-431 | `ADAMTCP_Disconnect`＋`ADAMTCP_Close` |
| `ADAM_ReadVoltage` | 433-505 | 讀 AI；失敗整個重連 |
| `ADAM_ReadPA` | 507-526 | V → kPa |
| `KpaTransferKG` / `MultiTransferKG` | 714-972 / 978-1041 | kPa → kg |
| `TransformFuntion` | 1043-1812 | **kg → 輸出碼**（純換算，不寫 IO） |
| `ADAM_WriteVoltage` | 1814-1931 | **主 EP 寫出入口**（kg） |
| `ADAM_DirectWriteData` | 1952-2037 | 直接寫碼（ADAM／APAX／ET-7226／數位） |
| `APAX_WriteData` | 2305-2673 | 獨立／Multi EP 寫出 |
| `ADAM_Alarm` / `ADAM_DualAlarm` | 549-605 / 670-712 | 讀回對寫出的容差警報 |
| `ADAM_ReturnValueCheck` | 2675-2759 | 回授電壓 0.8～5.2 V → WAR16322／16323 |
| `fCheckConnectStatus_ADAM6024` | 2958-3016 | 新韌體的名稱／連線數檢查與重連 |
| `EPSwitchOnOff` / `EpSwitch` | 3018-3051 / 3053-3103 | 各臂 EP 氣路 DO |

**寫出**：kg（`DeviceForm.fAireForce`／`dPress`／`TestIF.fAutoClean_AireForce`／`DoubleForce`）→ `ADAM_WriteVoltage` → `TransformFuntion` → 碼 0..4095 → `ADAM_DirectWriteData(碼,0)` → `ADAMTCP_WriteReg(172.16.8.110, 1, 12, 1)`；獨立／Multi EP 時改 `APAX_WriteData`。記 `iWritePA`（碼）、`iAdamOutValue`（kPa）。
**讀回**：`ADAM_ReadVoltage(0,5)` → `ADAM_ReadPA` → kPa → `KpaTransferKG` → kg（畫面、EP log）；`ADAM_Alarm` 比 kPa → `WAR1605`；`ADAM_ReturnValueCheck` 比電壓 → `WAR16322`／`WAR16323`。

**golden 呼叫者**（全表 `references/golden-code-map.md` §4；路徑都在 golden 912 根目錄下）：
- `main.cpp:9890` `TfMain::FormShow` → `Open_ADAM_6024()`。
- `main.cpp:6500-6521` `TfMain::Start` 的 Double EP 段（906 `main.cpp:6233-6254`）。
- **`main.cpp:21677-21803` `TfMain::Timer2Timer` 的「set EP force」段＝量產設定值寫手**（906 `main.cpp:21058-21184`；筆電 3c348627 引的 20980-21106 是 0618 樹）：每秒一拍；Contact／IO 畫面或 EP 檢查中不寫；加熱門開且臂上沒有真 IC 寫 0；AutoClean 中寫 AutoClean 力道；平常寫 `fAireForce`；值變或每 10 拍重寫；Die Force 同理。`main.cpp:22354-22355` 同一個 Timer2 跑 `ADAM_ReturnValueCheck()`。
- `main.cpp:11947-11952`、`:12194` `TfMain::FormClose`：EP 歸零、APAX 歸零、`Close_ADAM_6024()`。
- `uhome.cpp:1921-1928` `ProcessMotorHome`：HOME 時重連；REALLY 模式連不上 ⇒ `"Adam Connect Error and Stop Home"`，HOME 中止。`csystem.cpp:10998-10999` HOME 後 `ADAM_ReturnValueCheck(true)`。
- `AutoClean\AutoClean.cpp:7231`、`:7772`、`:8498`、`:9324`、`:10121`：`ADAM_WriteVoltage(TestIF.fAutoClean_AireForce)`。
- `Command.cpp:5294-5300` `WriteHandlerTestArmEP`（GPIB `MSG_CMD_GetTestArmEP`，`MessageDef.h:98`；分派 `main.cpp:16829-16831`）：回 `"%d PA\r"`。
- `asendic_Loader.cpp:215`、`:219`、`:598`、`:641`、`:833`：CKD FCM（`.111`）。
- 另有 atester（`IndexEveryTimeCheckEP` `:9331-9437`、`CheckAndRecodrEP` `:9439-`）、aTester_Front／Rear、cContact（53 處 `ADAM_WriteVoltage`）、iosetview、ContactForce、HS_Function、TriTemp。

## 4. 906 與 912 的差異（adam6024 只差 82 行 diff）

| 項目 | 912 | 906 |
|---|---|---|
| `Open_ADAM_6024`：`Address[Num]=IP` 移到狀態檢查**之前**，失敗時呼叫 `ShowDoubleEPConnectGuide` | `adam6024.cpp:263-268`、`:368` | `adam6024.cpp:263-266`（`Address[Num]=IP` 在狀態檢查與韌體檢查之後）；`ShowDoubleEPConnectGuide`（:73-93）有本體但**沒有呼叫者** |
| `MultiTransferKG` 參數 `int` → `double`（Eastsun 20260710，「修正沒有小數點」） | `adam6024.cpp:978`、`adam6024.h:27` | `adam6024.cpp:973`、`adam6024.h:26` |
| `TransformFuntion` 的 F15 防呆：`DieForceOneByOneSLKClass`／`SLKIndClass` 8 通道迴圈先檢查 `i*8+j` 沒超過筆數；`k<8` 才寫 `iAPAXEPValue[k]`／`iAPAXDualEPValue[k]` | `adam6024.cpp:1396-1398`、`:1412-1418`、`:1678-1679`、`:1700` | 沒有 |
| `TransformFuntion` 的 F4 防呆：16 通道迴圈 `i*16+j>=iSLKIndSize` 就 break | `adam6024.cpp:1671`、`:1711-1712` | 沒有 |
| `APAX_WriteData` 刪掉兩段已註解的 `iIndEPCnt==8` 舊碼 | — | `adam6024.cpp:2481-2487`、`:2531-2537`（註解，行為相同） |

⚠ 移植樹的 `TransformFuntion`（筆電的 `HT9011UC_Cpp_V3.33.906.0\adam6024.cpp`）是 **906 本體**：上面的 F15／F4 防呆都沒有（8 通道迴圈 :446-468、:726-754，16 通道迴圈 :762-797）。HT9050 若 `INSTALL_DOUBLE_EP=1`，16 通道迴圈照樣會跑（:724-800 的區塊對所有非 3 的模式都執行）。要補走擁有者（筆電）的 claim，不能直接改。
→ **20261002 MR !114 已補**（同一行、標 `[912]`，筆電的檔、走認領）：origin/main `577c41c5` 的 `adam6024.cpp:449`／`:464`／`:467`（F15，8 通道 DieForce）、`:725`／`:731`／`:753`（F4 筆數＋F15）、`:763`（F4，16 通道）。照第 20c 條保留。

## 5. V906 移植現況（20261002；20261003 補 §5.3 的合併狀態與 §5.4）

### 5.1 main（origin/main `36f09560`）上有什麼

> ⛔ 這張表是 **MR !114 合進 main 之前**（`36f09560`）的樣子。!114（`9e46491f`）之後：五個替身已退場（`atester_shims.h:265` 改 include `Adam6024_St02.h`、`:266-269` 與 `atester_shims.cpp:326-330` 是 RETIRED 註解），Timer2「set EP force」寫手、FormClose 歸零／斷線、HOME 重連互鎖、WriteHandlerTestArmEP、HOME 後回授檢查、G24 都接上了（`Adam6024Integrate_St02.cpp`；呼叫端 `tools\wb_serve.cpp:4598`、`FileRW\MainClose.cpp:789`／`:1068`、`uhome.cpp:1507-1510`、`Command.cpp:2941`、`csystem.cpp:6369`、`:21213`）——**但全部在 `W906_ADAM_EP_LIVE`（關）後面**，所以機台上的行為仍跟這張表一樣：EP 不寫、不讀回授、`WAR1605` 不跳。逐列以 `Adam6024Integrate_St02.cpp` 檔頭為準。

| 項目 | 位置（repo `HT9011UC_Cpp_V3.33.906.0\`） | 狀態 |
|---|---|---|
| `TransformFuntion` | `adam6024.cpp:65-877`、宣告 `adam6024.h:32-33` | 已翻（筆電 f53def1a 20260919，906 :1038-1796；缸徑比對容差 c08fb121）。Contact 畫面三欄的顯示被 `GATE (W906-P2B-CONTACTDISPLAY)` 閘住（`adam6024.cpp:852-871`，缺 `KpaTransferKG` 與 `fContact` 元件），**回傳值不受影響** |
| `EPSwitchOnOff` / `EpSwitch` | `adam6024.cpp:879-`（本體 `:904`、`:939`），宣告 `atester_shims.h:281-282` | 已翻（筆電 df23bc65 20260928；d81b9fcc 改用 `W906_FormShowing`） |
| 替身 `ADAM_DirectWriteData(int,int,int=1)`、`void ADAM_WriteVoltage(double)`、`bool ADAM_Alarm()`、`bool ADAM_Alarm(int)`、`ADAM_Rang` | 宣告 `atester_shims.h:253-269`，本體 `atester_shims.cpp:325-330` | **空殼**：寫出不做事，`ADAM_Alarm` 永遠 false（⇒ `WAR1605` 永遠不跳）。簽章和 golden 不同（golden `WORD` 參數、`bool` 回傳、只有 `ADAM_Alarm(int iArm=2)`） |
| Start 的 Die Force 段 | `WebStart.cpp:3762-3783` | 照 golden 906 :6233-6254 翻好；`TransformFuntion` 真的算，但 `ADAM_DirectWriteData` 是替身 ⇒ 寫不出去 |
| `WriteHandlerTestArmEP` | `Command.cpp:2934-2943` | **GATE(FW3-WA)**：`ADAM_ReadPA` 在 `#if 0` 裡，回 Tester 的字串是空的 |
| HOME 重連與「連不上就擋 HOME」 | `uhome.cpp:1489-1515` | `GATE (W906-HOME-C2-GPIBADAM)`：Close/Open 閘住，互鎖寫成 `if(false && …)` ⇒ **少一個互鎖** |
| HOME 後回授檢查 | `csystem.cpp:7499` | `W906G4_ADAM_ReturnValueCheck(true)` no-op seam |
| `WAR0329` 的 Multi EP 例外 | `csystem.cpp:21195-21225` | GATE G24（多報方向） |
| 關程式 EP 歸零 | `FileRW\MainClose.cpp:788-793`、`:1068` | 記成 stub／missing，不呼叫 |
| 露點 `ADAM_ReadAIValue` | `TempCtrl\TriTemp.cpp:518` | `W7TT_ADAM_ReadAIValue` 離線回 false |
| **Timer2「set EP force」量產寫手**（golden 912 `main.cpp:21677-21803`） | — | **沒有翻**（20261002 全樹 grep `fAirForce=`、`bOpenChambo` 0 筆） |
| `ADAMTCP.dll` 執行時載入 | — | main 上沒有（見 5.2） |

### 5.2 筆電的第一次嘗試與撤回（都在 main 的歷史裡）

- **75756cab**（jimmychiu 20260929 11:14，RULINGS_20260929 第 5 節第 5～7 條）：加了 `Public/AdamTcpShim.{h,cpp}`（執行時載入 ADAMTCP.dll，10 個進入點、全有或全無、`W906_ADAMTCP_DLL` 環境變數與 `AdamTcp_InstallApiForTest` 假表當 ctest 接縫）、把 Open/Close、WriteVoltage／DirectWriteData／WriteMaxData／WriteDigital、狀態檢查鏈、ReturnValueCheck 翻進 `adam6024.cpp`、退掉替身、wb_serve 開機 Open、uhome 互鎖、`tests/test_adam_ep_write.cpp`。
- **3c348627**（jimmychiu 20260929 13:49）**撤回 EP／ADAM 部分**（審查 B1/B2 阻擋、M1/M2 重大）：
  1. EP 寫出會在**沒有 golden Timer2 量產寫手**的情況下上線 ⇒ AutoClean 之後每次下壓都留在 AutoClean 力道；加熱門開也不會歸零；
  2. 沒有 FormClose 歸零 ⇒ 關程式後壓頭仍有壓；
  3. `ADAMTCP_Close` 無條件 `WSACleanup` 會重置 wb_serve 所有 socket（HOME 可能卡在 ShowMyMessage）。
  重做方向（commit 訊息原文）：把 golden Timer2 段做成 1 秒 pump、加離開歸零、Close 加參考計數、把 `ADAM_ReturnValueCheck` 放進 pump。補丁留在筆電 scratchpad `impl/adam/`。
- 裁決仍是 **RULINGS_20260929 第 5 節第 6 條＝A**（執行時 LoadLibrary）。

### 5.3 St02 的移植（MR !114，**20261002 18:51 已合進 main `9e46491f`**；第 20c 條保留 912）

卡 ST02-ADAM（Steven 1002 約 10:5x：「Adam6024的code還沒寫, 派人進行移植 並且通知Jimmy」）。來源＝golden 912，912 才有的行標 `[912]` 並附 906 行。1002 18:0x 第 20 條曾要求「退回照 906 重翻」，1002 22:4x 第 23 條第 2 項改成 **C：保留**（＝第 20c 條，理由見開頭的現況）。總開關 `W906_ADAM_EP_LIVE`（`MachineType.h:1809`，預設關）要打開時另外通知、EastSun 在旁。ctest：`ADAM6024_Comm`（H1）、`Adam6024_Pressure`（H2）、`Adam6024_Apax`（H3）、`Adam6024_Flow`（H4）（`tests\CMakeLists.txt:6952-6990`）。各檔一個擁有者：

| helper | 檔案（repo `HT9011UC_Cpp_V3.33.906.0\`） | 內容 |
|---|---|---|
| H1 通訊層 | `Public\AdamTcp_St02.h/.cpp`、**`Adam6024_St02.h`（共用標頭）**、`Adam6024Comm_St02.cpp`、`tests\test_adam6024_comm.cpp` | ADAMTCP.dll 執行時載入＋測試假接縫；golden 912 `adam6024.h` 全部宣告（`TransformFuntion` 留在 `adam6024.h`、EP 開關留在 `atester_shims.h`）＋`TfAdam6024` 門面 |
| H2 壓力／警報 | `Adam6024Pressure_St02.cpp`、`tests\test_adam6024_pressure.cpp` | Route 判斷、AdamOutputToPA、Alarm 系列、KpaTransferKG、MultiTransferKG、ReturnValueCheck |
| H3 APAX | `Adam6024Apax_St02.cpp`、`tests\test_adam6024_apax.cpp`（另見 `Public\ApaxShim_St02.h/.cpp`，ADSMOD.dll 執行時載入） | Open_APAX、三個回呼、APAX_WriteData |
| H4 整合 | `Adam6024Integrate_St02.cpp`（需要時）、`tests\test_adam6024_flow.cpp`、claim 清單 | 退替身、接呼叫端的 claim 行 |
| H5 | 本 skill | — |

20261002 約 11:15 在 worktree 看到的未追蹤檔：`Adam6024_St02.h`、`Adam6024Apax_St02.cpp`、`Public\ApaxShim_St02.h`、`Public\ApaxShim_St02.cpp`；其餘還沒出現。內容以 St02-E 整合後的版本為準。`Adam6024_St02.h` 檔頭列了它和 `atester_shims.h:265-269` 替身的五個衝突（同一個 TU 不能同時 include 兩者，直到 H4 的 claim 落地）。——H4 的 claim 已隨 !114 落地（§5.1 的補註）；上面這段是 1002 中午的過程紀錄。

### 5.4 機台編譯器 WinLibs g++ 16.2.0 的字面值精度（A3，MR !135，**還沒進 main**）

- **現象**：HT9050 機台 1003 回報 `Adam6024_Pressure` 紅（4q／5e／5f／9t；EP 開關關著、只有測試）。St02-M 1003 04:42 查到根因：機台預設用 **WinLibs g++ 16.2.0 i686** 建置，它把 `5.6`、`40.2` 這類小數字面值用 80 位元算（`FLT_EVAL_METHOD` 2，C++ 預設 `-fexcess-precision=standard`），所以 `double == 5.6`／`== 40.2` 在機台是 false，BCB6 與基準線 g++ 6.3.0 是 true；`5.2 > 5.2` 這種邊界在機台也會成立（`HT9011UC_Cpp_V3.33.906.0\docs\FP_ORACLE_FINDINGS.md` §1／§3）。不只是測試：鋼徑 5.6 時增壓換算會掉到「其他」那一列（St02 量到 76.85 變 78.36，NIGHT_REPORT §0 第 77 項）。
- **改法**（MR !135 `v906/st02-adam-fp-a3`，tip `2428cf0a`＝`bf04c1d1` 兩組態完整建置 0 錯誤＋`2428cf0a` 一行註解；**只改 St02 自己的 `Adam6024Pressure_St02.cpp`**，6 行同一行改寫、整檔 906 行不變、測試不動）：
  - `:282`、`:305`（`KpaTransferKG`）golden `else if(fDiameter==5.6)` → A3 容差 `fDiameter - 5.6 < 1e-6 && fDiameter - 5.6 > -1e-6`；
  - `:535`（`MultiTransferKG`）golden `if(fDiameter==40.2)` → 同樣的 A3 容差；
  - `:822`（`ADAM_ReturnValueCheck`）golden `if((dReadVoltage<0.8 || dReadVoltage>5.2)` → 字面值轉 `(double)`：這是嚴格範圍，轉型會丟掉多餘精度＝BCB6 的 double 比法（`2428cf0a` 的 [W906] 註解說明為什麼不用容差）；
  - 檔頭 D2 說明 `:87-88` 更正（`6.0`／`4.0`／`0` 在二進位是精確值，照留 golden 的 `==`）。
  - 依據：使用者的 P18（20260923）／A3（20260924）裁決（commit 與 MR 寫成「RULINGS P18／A3」），記在 `FP_ORACLE_FINDINGS.md` §7／§8，同一種改法。
- **驗證**：g++ 6.3.0（筆電、St01 的建置）上行為不變，St01 代跑 `Adam6024_Pressure` 應該照舊綠；**真正的驗證要在機台的 WinLibs 線上重跑 `Adam6024_Pressure`**（St02 這台沒有 WinLibs）——筆電 1003 04:5x 說收進下一批後會在機台通知請 EastSun 重跑；人工審核 A31（`docs\handoff\ST02_HUMAN_REVIEW_20260930.md`，`v906/steven-handoff` 分支）。
- **整棵樹的解法**（加 `-fexcess-precision=fast` 或 `-std=gnu++17`）是工具鏈決定，在 NIGHT_REPORT §0 第 77 項等 Jimmy（筆電建議 A＝加，動手前先用 objdump 證明 g++ 6.3.0 基準線的機器碼不變）；他回之前維持一處一處改。

## 6. 排錯速查（細節 `references/troubleshooting.md`）

- **V906 上 EP 沒動** ⇒ 不是故障，是 §5 還沒上線（碼在 main，但 `W906_ADAM_EP_LIVE` 關著）。
- **`Adam6024_Pressure` 只在機台紅** ⇒ 先看是不是 WinLibs g++ 16.2 的字面值精度（§5.4）；MR !135 合了以後在機台重跑。
- **連不上**：IP（新模組出廠 `10.0.0.1`）、網段、線、電源；新韌體（`6.01 B21` 以上）多了名稱與連線數檢查——連線數 ≥ 8 ⇒ `Adam Clear Fail, Please Check Network Cable or IP address`（`adam6024.cpp:2985`），`ClearAllConnection` 其實不會清（`:2844-2863`），要等 Host Idle 逾時或重開模組；`Connect Fail! Please Check ADAM IP!` 要連續失敗超過內部計數 100 才跳（`:361-371`），之前照樣回 true。
- **壓力不對**：先看主畫面 `"EP: 輸出 / 讀回"`（`HS_Function.cpp:133-156`）分清寫錯或讀錯；校正值 `EP_MAXA`／`EP_MINA_FeedBack` 有 0 或相等 ⇒ 讀回變成電壓整數（`adam6024.cpp:512-515`）；V906 的力量表沒載＝「silent-30.0」缺陷（`HT9011UC_Cpp_V3.33.906.0\ContactForce.h:320-324`）；缸徑不在表裡用第一筆；Dual Force 只有 `INSTALL_DOUBLE_EP` 1/3 才寫。
- **警報**：`WAR1605` 讀回 kPa 超出容差（漏氣、入氣不足、校正）；`WAR16322`／`WAR16323` 回授電壓不在 0.8～5.2 V（AI 跳線、範圍）；`WAR0329` Die Force 感測關。
- **紀錄**：`D:\HT9045_Log\MNetLog`（`AdamOutValue=…, ReadAdamValue=…, Range=…`）、`D:\HT9045_Log\EP\<yyyymm>\<日期>.csv`（`[D26]` EP log）、Process 紀錄的 `APAX … FAIL`。

## 7. 碰這塊的規則

1. **STEVEN-NB3 只編譯不執行**：F-Secure 擋新 exe；不跑 exe／ctest；推之前兩組態編譯（sim quick＋ship build_ship）。執行驗證請 St01 代跑。
2. **測試絕不碰真模組**：不載入 `ADAMTCP.dll`／`ADSMOD.dll`、不開往 `172.16.8.x` 的 socket；用假接縫（75756cab 的 `AdamTcp_InstallApiForTest`／`W906_ADAMTCP_DLL` 設成不存在的路徑是前例）；ctest 先過 `tests\st02_test_containment.h` 的圍堵檢查。
3. **EP 寫出不准單獨上線**：替身退場的那一天，必須同時有 golden Timer2「set EP force」段（912 `main.cpp:21677-21803`）、FormClose 歸零（`main.cpp:11947-11952`）、`ADAMTCP_Close` 的 WSACleanup 對策、pump 裡的 `ADAM_ReturnValueCheck`——這就是 3c348627 撤回的理由，不要重犯。
4. **exe 不准靜態匯入** `ADAMTCP.dll`／`ADSMOD.dll`：建置後 `objdump -p` 每個 exe，不能出現 `DLL Name: ADAMTCP.dll`／`ADSMOD.dll`。
5. **照 golden**（RULINGS_20261001 #0；ADAM-6024 的 golden 是 912＝RULINGS_20261002 第 20c 條，其他模組照第 20 條只做 906）：每個函式標 golden 912 行（附 906 行）、偏離標 `// [W906]`；小數字面值的 `==`／嚴格範圍要想到機台的 WinLibs 線（§5.4，A3 寫法）；golden 怪癖照抄（`Read6KAI` 的 6017、名稱 `"16024-D"`、`ClearAllConnection` 名不符實、斜率 `(int)` 截斷、1250～1252 改 1253、連線失敗計數 90/50/100、`ADAMErrorMessage[-iRet]`）；客戶分支（JCET、KYEC_LEE、ASE、GIGAS、SPIL…）照 S25 閘住走 else。
6. **檔案擁有者**：`adam6024.cpp/.h`、`atester_shims.h/.cpp` 是**筆電（jimmychiu）的檔**；`WebStart.cpp`、`uhome.cpp`、`Command.cpp`、`AutoClean\AutoClean.cpp`、`asendic_Loader.cpp` 的提交也以筆電為主，但有其他人改過；`FileRW\MainClose.cpp`、`csystem.cpp` 多人改過（75756cab 說 MainClose 是 St01 的檔）——動手前用 git log 與 AI 標記確認，並查 `TO_STEVEN.md` §1 與 `TO_KEVIN.md` §1。改別人的檔一律寫 claim（file:line、擁有者、舊行、新行，盡量同一行、新碼放在 `//` 前面），St02 的新檔各有一個 helper 擁有者（§5.3）。
7. **新的全域符號只放自己的檔**：先查和替身（`atester_shims.h:265-269`）撞不撞；golden 的檔案層全域名字很泛（`Address`、`fValue`、`wGain`…），`Adam6024_St02.h` 用 `ADAM6024_ST02_INTERNAL` 把它們藏起來。
8. **不寫密碼**：模組的 Utility 密碼在手冊 p.80 與韌體 SOP，本 skill 不抄值。

## 8. 相關 skill

- `ht9045-contact-force`：Contact Force 計算、SLK 缸徑表、ContactInfo.ini、`TfContactForce` 開機建表。
- `ht9045-io-control`（`references/exit-shutdown.md`）：關程式時的 EP 歸零項目。
- `ht9045-alarm-dismissal`：WAR 類警報怎麼解除。
- `ht9050-hw`：HT9050 硬體總表。
