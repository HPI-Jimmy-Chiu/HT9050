> 保存來源：`.claude/skills/ht9045-adam6024/references/golden-code-map.md`，main `84233b648`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# golden adam6024 程式地圖（912 為主，附 906 行號）

> 本檔是 `../SKILL.md` §3 的細節。
> - 912 檔：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\adam6024.cpp`（3104 行）、`adam6024.h`（92 行）、`adam6024.dfm`、`ADAMTCP.h`
> - 906 檔：`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\adam6024.cpp`（3102 行）
> - 兩棵都是 Big5，用 cp950 讀。函式範圍是用括號收支掃出來的（20261002）。

## 1. 49 個函式本體

（任務說明寫 48 個；實際掃到 49 個本體，含 TfAdam6024 的 7 個成員與 3 個 APAX 回呼。另有一個只有宣告、沒有本體的 `WORD TransFuntion(double)`，`adam6024.h:90`。）

| # | 函式 | 912 行 | 906 行 | 一句話 |
|---|---|---|---|---|
| 1 | `static ShowDoubleEPConnectGuide(IP, Num, Detail)` | 73-93 | 73-93 | 第二塊 EP 板（`Num==2`）連不上時，依 `INSTALL_DOUBLE_EP` 2/3 給現場人員一次性的說明訊息＋`NewRecordProcess`。**906 有本體但沒有呼叫者**，912 才在 `Open_ADAM_6024` 呼叫 |
| 2 | `IsMultiEPPressureRouteActive()` | 96-105 | 96-105 | `INSTALL_DOUBLE_EP==3`、`bIndEPSLK`、`SW[SwMultiEp]` 啟用且 ON ⇒ Multi EP（APAX）氣路生效 |
| 3 | `IsIndependentEPPressureRouteActive()` | 108-120 | 108-120 | `bIndEPSLK` 且（模式 2，或模式 3 且 #2 為真）⇒ 走 APAX 而不是 ADAM AO |
| 4 | `TfAdam6024::TfAdam6024(Owner)` | 122-140 | 122-140 | 建 `ADAMErrorMessage[0..15]` 錯誤字串表 |
| 5 | `SetAiInputRange(ip, ch, range)` | 142-158 | 142-158 | UDP `$01A<cc><rr>` 設 AI 範圍 |
| 6 | `GetAiInputRange(ip, ch, *range)` | 160-193 | 160-193 | UDP `$01B<cc>` 讀 AI 範圍；`CHECK_EP_SETTING==0` 直接回 true（不改輸出值） |
| 7 | `SetAoOutputRange(ip, ch, range)` | 195-211 | 195-211 | UDP `$01C<cc><rr>` 設 AO 範圍 |
| 8 | `GetAoOutputRange(ip, ch, *range)` | 213-246 | 213-246 | UDP `$01C<cc>` 讀 AO 範圍；同 #6 的開關 |
| 9 | `Open_ADAM_6024(IP, Num)` | 248-398 | 248-394 | 單一模組開機：狀態檢查 → 韌體新舊 → (`EP_Install==4` 開 ET-7226 socket) / `ADAMTCP_Open`(Num 0) → AI/AO 範圍檢查（一次）→ `ADAMTCP_Connect` → `wGain` → `bADAM6420Install=true` |
| 10 | `Open_ADAM_6024()` | 400-409 | 396-405 | 依設定開 `.110`、`.111`（CKD FCM）、`.112`（APAX），三個都成功才回 true |
| 11 | `Close_ADAM_6024()` | 411-431 | 407-427 | `bADAM6420Install` 時：ET-7226 關 socket，否則 `ADAMTCP_Disconnect`＋`ADAMTCP_Close` |
| 12 | `ADAM_ReadVoltage(Num, iCH=5)` | 433-505 | 429-501 | 防重入；`ADAMTCP_Read6KAI(Address[Num], 6017, 1, ...)` 讀全部 AI；失敗就 Close＋Open 回 0；AI3 轉 mA（露點） |
| 13 | `ADAM_ReadPA(*dValue, iCH=5)` | 507-526 | 503-522 | AI 電壓 → kPa（`EP_MINA_FeedBack`..`EP_MAXA` 線性對到 `EP_MINMPA*1000`..`EP_MAXKPA`；iCH 2 用 Dual 參數）；校正值是 0 或相等時直接回電壓（截成 int） |
| 14 | `AdamOutputToPA(iAdamOutput)` | 528-541 | 524-537 | 輸出碼 → kPa：`(EP_MAXKPA-EP_MINMPA*1000)/(fMaxUnit+1)*碼` |
| 15 | `ADAM_Rang(v)` | 543-546 | 539-542 | 設 `iADAMRange`（警報容差，來源 `[D26] iD26EPEncoderRange`） |
| 16 | `ADAM_Alarm(iArm=2)` | 549-605 | 545-601 | 讀回 kPa 對 `AdamOutputToPA(iWritePA)` ± `iADAMRange`（JCET/KYEC_LEE 可用百分比）；超出回 true＋`MNetLog`；同時更新 `iReadAdamEP`、`iAdamOutValue` |
| 17 | `ADAM_Alarm_Kg(iAdd)` | 607-668 | 603-664 | 以公斤比對 Contact 畫面設定值，容差分級 0.25/0.5/1/2 kg。**golden 912 沒有呼叫者** |
| 18 | `ADAM_DualAlarm(iType)` | 670-712 | 666-708 | Dual EP（AI2）讀回對 `edDoubleForce`，固定值或百分比（`iD26_3DualEPEncoderRange`） |
| 19 | `KpaTransferKG(int kPa, bDualForce=false)` | 714-972 | 710-968 | kPa → kg：`kPa*10.197 * (D²π/4 * loadRate) / 1000`；loadRate 依缸徑、NS kit、高溫 offset、Dual／動態缸徑表（`fContactForce->SLKClass` 等）選 |
| 20 | `MultiTransferKG(double kPa, bDualForce=false)` | 978-1041 | 973-1036（`int` 參數） | Multi EP 每 site 的簡化版 kPa → kg（V896 移植）；912 把參數改成 `double`（Eastsun 20260710） |
| 21 | `TransformFuntion(kg, bDualForce=false, bSoft=false, iArm=0)` | 1043-1812 | 1038-1796 | **kg → EP 輸出碼**（0..4095）：依測試模式除 site 數與 compliance unit、扣 contact offset、選 loadRate、算 MPa、線性換成碼並夾限；同時填 `iAPAXEPValue[]`／`iAPAXDualEPValue[]`（碼×16）。只寫 Contact 畫面三個欄位，**不寫 IO** |
| 22 | `ADAM_WriteVoltage(double kg)` | 1814-1931 | 1798-1915 | **主 EP 寫出入口**：ContactForce 畫面開著就不寫；`v<0` 寫 0；否則 `TransformFuntion` → `ADAM_DirectWriteData(code,0)`（或 APAX、或 Two EP、或數位）；記 `iWritePA`、`iAdamOutValue` |
| 23 | `ADAM_WriteMaxData(bFullForce)` | 1933-1950 | 1917-1934 | Auto Height 滿壓：缸徑 ≤2.5 寫 820/455，否則 4095/2275（依 `EP_MAXKPA<=500`）；false 寫 0 |
| 24 | `ADAM_DirectWriteData(WORD data, Num, iAdd=1)` | 1952-2037 | 1936-2021 | **直接寫碼**：ET-7226 → `WriteAO`；APAX 氣路 → `APAX_WriteData`；`EP_Install` 1/3 → `ADAMTCP_WriteReg(Address[Num],1,11+iAdd,1)`（1250..1252 改 1253）；5 → reg 11/12；2 → `WriteDigital`；失敗清 `bConnectStatus[Num]` |
| 25 | `WriteDigital(double v)` | 2041-2050 | 2025-2034 | `EP_Install=2`：10 個 DO 輸出二進位 |
| 26 | `TfAdam6024::ClientSocket1Connect` | 2052-2062 | 2036-2046 | ET-7226 socket 連上：清 `cmdBuf[8]` |
| 27 | `TfAdam6024::ClientSocket1Error` | 2064-2078 | 2048-2062 | ET-7226 socket 錯誤：`ErrorCode=0; Abort();` 關 socket |
| 28 | `TfAdam6024::WriteAO(AO1..AO4)` | 2080-2108 | 2064-2092 | 自組 Modbus/TCP 功能碼 16 封包（21 bytes）送 ET-7226 |
| 29 | `TfAdam6024::OpenSocket(IP, Port)` | 2110-2115 | 2094-2099 | 開 `ClientSocket1` |
| 30 | `TfAdam6024::CloseSocket()` | 2117-2127 | 2101-2111 | 關 `ClientSocket1` |
| 31 | `ADAM_ReadAIValue(iNum, iChannel, *mA, *degree)` | 2129-2204 | 2113-2188 | 露點計：讀 AI（mA）並依 `DewPoint_Hardware_Install` 1(-60~+60)／2(-80~+20) 轉溫度；未安裝回 9999；SOFT_SIMULTE 固定 5.87654 |
| 32 | `TfAdam6024::FormDestroy` | 2206-2216 | 2190-2200 | 只記 `LogSoftwareOffTime` |
| 33 | `Open_APAX(char IP[])` | 2219-2267 | 2203-2251 | `ADSMOD.dll` 的 `MOD_Initialize`/`MOD_AddTcpClientConnect`/`MOD_StartTcpClient`。**golden 沒有呼叫者** |
| 34 | `ConnectTcpServerCompletedEventHandler` | 2269-2291 | 2253-2275 | APAX 連線結果 → `bAPAXConnectFileAlarm`、記錄 `"APAX Connect to '%s' result = %d"` |
| 35 | `DisconnectTcpServerCompletedEventHandler` | 2293-2298 | 2277-2282 | 記錄 `"APAX Disconnect from ..."` |
| 36 | `ClientWriteReg_1_8_Handler` | 2300-2303 | 2284-2287 | `bApaxWriteFinish=true` |
| 37 | `APAX_WriteData(bDir, wdata, iArm=0)` | 2305-2673 | 2289-2671 | `INSTALL_DOUBLE_EP>=2`：組 16 通道（`bDir` true＝全部寫同一個 `wdata*16`，false＝寫 `iAPAXEPValue[]`），`ADAMTCP_WriteReg(Address[2],1,1,8)`／`(…,33,8)`，重試 10 次 |
| 38 | `ADAM_ReturnValueCheck(bHome=false)` | 2675-2759 | 2673-2757 | `EP_Install` 3/5：回授電壓不在 0.8～5.2 V 累計 >100 次（或 HOME 時立刻）報 `WAR16322`（主 EP）／`WAR16323`（Dual）；HOME 時 `fAllMotorHome=false`；功能都關時每 60 次讀一次讓 EP 不睡著 |
| 39 | `GetModuleName(Num)` | 2762-2785 | 2760-2783 | UDP `$01M` |
| 40 | `GetFirmwareName(Num)` | 2788-2810 | 2786-2808 | UDP `$01F` |
| 41 | `GetModuleConnectionCount(Num)` | 2814-2840 | 2812-2838 | UDP `%01GETMBTCPCN`，回個位數；格式錯回 -1 |
| 42 | `ClearAllConnection(Num)` | 2844-2863 | 2842-2861 | **名不符實**：送的也是 `%01GETMBTCPCN`，回 `!` 就算成功 |
| 43 | `GetModuleHostIdleTime(Num)` | 2867-2884 | 2865-2882 | `ADAMTCP_GetHostIdleTime`；**沒有呼叫者**；`adam6024.h:41` 宣告成 `()`，本體是 `(int Num)` |
| 44 | `SetModuleHostIdleTime(int)` | 2888-2905 | 2886-2903 | 本體全註解，固定回 false；沒有呼叫者 |
| 45 | `fCheckModuleName_ADAM6024(Num)` | 2909-2925 | 2907-2923 | 名稱 == `"16024-D"`（含位址尾碼 `1`，golden 寫法）才 true；SOFT_SIMULTE 回 true |
| 46 | `fCheckModuleFWISNew_ADAM6024(Num)` | 2929-2955 | 2927-2953 | 韌體字串前 4 字 `"6.01"` 且末兩位數 ≥ 21（例 `6.01 B21`）⇒ 新韌體 |
| 47 | `fCheckConnectStatus_ADAM6024(Num)` | 2958-3016 | 2956-3014 | 新韌體才做：名稱檢查 →「清連線」→ 連線數 ≥8 報錯 → `ADAMTCP_Disconnect` → `ADAMTCP_Connect`；舊韌體直接回 true |
| 48 | `EPSwitchOnOff(iArm)` | 3018-3051 | 3016-3049 | 0 全關／1 Arm1／2 Arm2／3 全開，考慮 `bArm1PickPlaceArm2Test` 與 `[D30]` 單臂模式 |
| 49 | `EpSwitch(Arm1, Arm2)` | 3053-3103 | 3051-3101 | 開關 `SwEpArm1/2`、`SwIndEpArm1/2`；IO 畫面開著時要該 Index 有 IC 才改 |

## 2. 檔案層級的全域（912 `adam6024.cpp`）

| 名稱 | 行 | 用途 |
|---|---|---|
| `DEFAULT_PORT 502`、`DEFAULT_ET7226_PORT 10001` | :26-27 | Modbus/TCP 埠 |
| `fAdam6024` | :32 | 表單實例（`HT9045.cpp:78`、`:239` CreateForm） |
| `NetID=0x01` | :34 | ET-7226 站號 |
| `iConnectionTimeout/iSendTimeout/iReceiveTimeout=2000` | :36-38 | ADAMTCP 逾時 |
| `bADAM6420Install`、`bOpen` | :40 | 模組已開（名字是 6420，實際是 6024） |
| `Address[3]` | :42 | `.110`/`.111`/`.112` |
| `cAddress[100]` | :43 | APAX IP（只有 `Open_APAX` 用） |
| `fValue[16]`、`wGain[10]`、`wHex[16]` | :44-45 | `Read6KAI` 緩衝 |
| `bCanReadData`、`iWritePA`、`iADAMRange`、`dADAMRange_Kg` | :46-49 | 寫出碼、容差 |
| `ulClientHandle` | :52 | APAX handle |
| `bConnectStatus[3]` | :53 | 每顆模組連線狀態 |
| `bADAM6420CheckRange[4]` | :54 | 範圍檢查做過了沒 |
| `bDoubleEPConnectGuideShown` | :55 | #1 只顯示一次 |
| `iAdamOutValue` | :548 | 最近一次寫出換成的 kPa |
| `Digital_10Bit` | :2040 | 10 |

外部全域（`cmydef`）：`EP_Install`（`cmydef.cpp:3059`）、`iIndEPCnt=16`（`cmydef.cpp:5104`）、`iAPAXEPValue[16]`（`cmydef.cpp:5063`）、`bAPAXConnectFileAlarm`（`cmydef.cpp:5060`）、`bADAM6024FWIsNew[3]`（`cmydef.cpp:5545`）、`iReadAdamEP`（`cmydef.h:4319`）、`iAPAXDualEPValue[16]`（`cmydef.h:4922`）、`dfComplianceUnit`（`cmydef.h:4965`）。

## 3. 資料流

### 3.1 寫出（kg → 碼 → 模組）

```
DeviceForm.fAireForce / dPress / TestIF.fAutoClean_AireForce / DoubleForce（kg）
  └─ ADAM_WriteVoltage(kg)                         adam6024.cpp:1814
       ├─ fContactForce->fShow → 不寫（:1822-1825）
       ├─ TransformFuntion(kg)  → code 0..4095      :1856（Two EP：:1860-1861 兩支臂各算）
       │    └─ 同時填 iAPAXEPValue[] = code*16（獨立／Multi EP）
       ├─ IsIndependentEPPressureRouteActive() → APAX_WriteData(false,0) ＋ ADAM AO1 寫 0（:1867-1878）
       └─ 否則 ADAM_DirectWriteData(code, 0)          :1885
            └─ ADAMTCP_WriteReg(Address[0], 1, 12, 1, &code)   :1994（AO1 = 40012）
  記 iWritePA = code、iAdamOutValue = AdamOutputToPA(code)  :1912-1913

Die Force：TransformFuntion(DoubleForce, true) → ADAM_DirectWriteData(code, 0, 0) → reg 11（AO0）
          main.cpp:6511-6519（Start）、:21716-21731（Timer2）
```

TransformFuntion 核心（非 `WEIGHT_CALIBRATION`）：
`fMaxMPA=EP_MAXKPA/1000`、`fMinMPA=EP_MINMPA`（:1124-1125）；kg 依 `TestIF.iTestMode` 除以 site 數 × compliance unit（:1194-1330，加重 `bUseAddWeight` 每顆 +1.25 kg）；
扣 contact offset、選 loadRate（:1359-1665）；`F = kg / (D²π/4 · loadRate)`、`MPa = F/10.197`（:1793-1797）；
`code = (int)(fMaxUnit/(fMaxMPA-fMinMPA)) * (MPa-fMinMPA)`（:1807；⚠ `(int)` 只截斜率，golden 寫法）；最後 `CheckRange(code, 0, fMaxUnit)`（:1809-1811）。
`WEIGHT_CALIBRATION` 時改用 `IniConfig.iContactForceMap` 的 16 kg／64 kg 兩點線性（:1138-1178）。`bSoft` 參數全檔沒用到。

### 3.2 讀回與警報（AI → kPa → kg → 警報）

```
ADAM_ReadVoltage(0, 5)   → AI5 電壓（:433）
  └─ ADAM_ReadPA          → kPa（:507-526）
       ├─ KpaTransferKG(kPa) → kg（:714-972；畫面、EP log、ReadEPData）
       ├─ ADAM_Alarm()       → kPa 對 AdamOutputToPA(iWritePA) ± iADAMRange → atester WAR1605
       └─ ADAM_ReturnValueCheck → 電壓窗 0.8～5.2 V → WAR16322 / WAR16323
```

## 4. golden 912 的呼叫者（adam6024.cpp 以外）

所有路徑都在 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` 底下；906 對照在 `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\`。

| 呼叫者 | 912 行 | 906 行 | 做什麼 |
|---|---|---|---|
| `TfMain::FormShow` | `main.cpp:9890` | `main.cpp:9457` | `Open_ADAM_6024()` 開機連線 |
| `TfMain::Start` | `main.cpp:6500-6521` | `main.cpp:6233-6254` | Double EP（`INSTALL_DOUBLE_EP` 1/3）：`TransformFuntion(DoubleForce,true)`（KYEC 預設值分支 :6505-6512）→ Contact 畫面沒開時 `ADAM_DirectWriteData(code,0,0)` |
| **`TfMain::Timer2Timer`「set EP force」段** | **`main.cpp:21677-21803`** | **`main.cpp:21058-21184`**（0618 樹 20980-21106） | **量產設定值寫手**（Timer2 是 1 秒計時器，`main.dfm` Timer2 沒寫 Interval ＝ VCL 預設 1000 ms，見 `main.dfm:17311-17316`）：Contact／IO 畫面開著或正在 IndexEveryTimeCheckEP 時**不寫**，只把 `fAirForce=iDieForce=-1`（離開後第一拍必重寫，:21677-21684）；否則 `fAireForce = dPress (+ 伺服器 offset × DutCount)`（:21688-21697）；`SwMultiEp` 自動切（:21702-21714）；Die Force 碼有變或每 10 拍重寫（:21716-21740）；主 EP 在 `fAirForce!=fAireForce`／每 10 拍／`bOpenChambo` 時重寫（:21742）——**加熱門開且臂上沒有真 IC 時寫 0**（:21745-21750）、**AutoClean 中寫 AutoClean 力道（不低於 `iEP_Min_KG`）**（:21753-21758）、其餘寫 `fAireForce`；RTC 驗證／測試中加壓時讓位；SPIL＋CKD FCM 時寫 `.111` |
| `TfMain::Timer2Timer` | `main.cpp:22354-22355` | `main.cpp:21724-21725` | `EP_Install` 3/5 時 `ADAM_ReturnValueCheck()` |
| `TfMain::FormClose` | `main.cpp:11947-11952`、`:12194` | `main.cpp:11462-…`、`:11677` | 關程式：`ADAM_WriteVoltage(0)`、`ADAM_DirectWriteData(0,0)`、APAX 歸零；最後 `Close_ADAM_6024()` |
| `TfMain::ReadEPData` | `main.cpp:33335-33380` | — | ASE/海思用的 EP 字串：`ADAM_ReadPA`＋`KpaTransferKG`＋（[D26]）`ADAM_Alarm` |
| GPIB 分派 | `main.cpp:16829-16831` | — | `MSG_CMD_GetTestArmEP` → `WriteHandlerTestArmEP()` |
| `TfMain::WriteHandlerTestArmEP` | `Command.cpp:5294-5300` | `Command.cpp:5293-5299` | 回 Tester `"%d PA\r"`（`ADAM_ReadPA`）；指令定義 `MessageDef.h:98`（72，`"GetTestArmEP"`）。`MSG_CMD_Force`／`MSG_CMD_ContactForce`（`Command.cpp:1541`、`:3855`）只回設定字串，不碰 ADAM |
| `ProcessMotorHome` | `uhome.cpp:1921-1928` | — | HOME 時 `Close_ADAM_6024(); Open_ADAM_6024()`；REALLY 模式開不起來 ⇒ `"Adam Connect Error and Stop Home"` 並中止 HOME |
| `DoHomeProcess` | `csystem.cpp:10998-10999` | — | HOME 完成後 `ADAM_ReturnValueCheck(true)` |
| `CheckSafeDoorIsClosed` | `csystem.cpp:2754-2767` | — | `SnEPDieForce` 關 ⇒ `WAR0329`（Multi EP 氣路時略過） |
| `DoIndexAutoClean_Arm1PickArm2Test` | `AutoClean\AutoClean.cpp:7231`、`:7772` | — | `ADAM_WriteVoltage(TestIF.fAutoClean_AireForce)` |
| `DoIndexAutoClean` | `AutoClean\AutoClean.cpp:8498`、`:9324`、`:10121` | — | 同上 |
| `GetTesterResult` | `atester.cpp:1660`、`:1673`、`:1918` | — | 測試中加壓（`dIndexAddPressEP_Kg`）與還原 |
| `DoTestHeadMotor` | `atester.cpp:6240` | — | `EPSwitchOnOff(eEPSwBoth)` |
| `IndexEveryTimeCheckEP` | `atester.cpp:9331-9437` | — | 每次下壓檢查 EP：打滿壓（4095／2275）→ `ADAM_Alarm` → 沒事就寫回 `dPress`，否則 `WAR1605` |
| `CheckAndRecodrEP(iArm)` | `atester.cpp:9439-` | — | `[D26] bD26EnableEPLog` 時寫 EP log CSV；`[D24]`/`[D26]` 開且運轉中有警報 ⇒ `WAR1605` |
| `aTester_Front.cpp` | `:2240-2241`、`:4613`、`:4919`、`:6888-6891`、`:6962-6965`、`:7156` | — | EP 開關＋寫出 |
| `aTester_Rear.cpp` | `:2243`、`:4780`、`:5083`、`:7146-7149`、`:7219-7222`、`:7406` | — | 同上（Arm2） |
| `atester_32Site.cpp` | `:2326`、`:2331` | — | 32 site 充氣 |
| `asendic_Loader.cpp` | `:215`、`:219`、`:598`、`:641`、`:833` | — | CKD FCM（`Num=1`，`.111`）吹氣流量 |
| `Tfiosetview` | `iosetview.cpp:205-224`（Timer1 顯示 kPa）、`:293-294`（FormClose 重連）、`:511`/`:520`（FormShow 歸零）、`:1929-1954`、`:4093`（滑桿直接寫碼）、`:4235`（500 kPa 按鈕） | — | IO 畫面 |
| `TfContactForce` | `ContactForce.cpp:924`（FormClose）、`:963`（btSaveClick）、`:1381`（滑桿直接寫碼） | — | EP 校正畫面 |
| `cContact.cpp` | 53 處 `ADAM_WriteVoltage`、14 處 `EPSwitchOnOff`、12 處 `ADAM_Alarm`、11 處 `ADAM_WriteMaxData`（Auto Height）、`:14284-14293` 的 TransformFuntion／APAX | — | Contact 畫面、Contact Test、Auto Height |
| `TFormHS` | `HS_Function.cpp:133-160`（主畫面 `lbEPenconder` 顯示 `"EP: 輸出 / 讀回"`）、`:901-`（RecordEPLog_HS）、`:1148-`（ReadMultiEP）、`:4548-`（CheckEPRange） | — | 顯示與 log |
| `cprod.cpp` | `:2838` | — | `ADAM_Rang(IniConfig.iD26EPEncoderRange)` |
| `cSetUp.cpp` | `:3649` | — | 存設定後寫 EP |
| `TempCtrl\TriTemp.cpp` | `:134`、`:161` | — | HT-1032 露點 `ADAM_ReadAIValue` |

<!-- preserved-content:end -->
