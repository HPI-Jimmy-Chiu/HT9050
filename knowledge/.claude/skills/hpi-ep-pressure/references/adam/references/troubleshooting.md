> 保存來源：`.claude/skills/ht9045-adam6024/references/troubleshooting.md`，main `84233b648`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# ADAM-6024／EP 排錯（細節）

> 本檔是 `../SKILL.md` §6 的細節。golden 912 = `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`。
> V906 移植樹 = repo `HT9011UC_Cpp_V3.33.906.0\`（行號查於 origin/main `36f09560`，20261002）。
> 先記住：**20261002 的 V906 main 根本不會連 ADAM-6024**（寫出是空殼、讀回被閘住），所以 V906 上「EP 沒動」不是排錯題，是還沒移植（見 `../SKILL.md` §5）。下面的流程是給 golden（BCB）機台，以及 St02 的移植合併之後用的。
> 20261003：St02 的移植已合進 main（MR !114 `9e46491f`，第 20c 條保留 912），但總開關 `W906_ADAM_EP_LIVE` 關著（`MachineType.h:1809`）⇒ 機台上仍然不連、不寫、不讀；§3 最後「移植樹現況」那條講的替身也已退場，關著時的回答跟替身一樣。機台上只有 `Adam6024_Pressure` 紅的話，先看 `../SKILL.md` §5.4（WinLibs 字面值精度，MR !135）。

## 1. 連不上

### 1.1 golden 會跳出的訊息（照出現順序）

| 訊息 | 912 出處 | 代表什麼 | 先查 |
|---|---|---|---|
| `ADAM Read ModuleName Fail, Please Check Lan Cable` | `adam6024.cpp:2970` | 新韌體模組，`$01M` 回的名稱不是 `16024-D` | 網路線、IP、那個 IP 是不是真的 6024 |
| `Clear ADAM Connection Fail!` | `:2978` | `%01GETMBTCPCN` 沒有 `!` 回應 | 同上；UDP 被擋 |
| `Adam Clear Fail, Please Check Network Cable or IP address` | `:2985` | 模組回報 TCP 連線數 ≥ 8（手冊上限 8，`E:\HT9045W_相關料件技術文件\ADAM\ADAM-6000_Series_Manual_Ed2.pdf` p.79） | 有別的程式（Utility、瀏覽器、另一支 HT9045）佔著連線；等 Host Idle 逾時或重開模組電源 |
| `ADAMTCP Connect Fail..., Error Code:%d` | `:2994` | 新韌體路徑的重連失敗 | 錯誤碼對 `ADAMTCP.h:126-143` |
| MyDBI `Motion` / `ADAMTCP_Open Fail!` / `ADAM5KTCP_...` | `:282` | DLL 初始化失敗（V906 的執行時載入：DLL 不在 = StartupFailure -1） | `ADAMTCP.dll` 在不在、行程是不是 32 位元 |
| `Failed to Get/Set ADAM AI/AO Range! IP=..., Channel=..., ...` | `:297`、`:312`、`:325`、`:338`、`:348` | UDP 範圍查詢或設定失敗 | UDP、韌體太舊不支援 `$01A/B/C`（手冊 Ed2 沒有這幾條） |
| `Connect Fail! Please Check ADAM IP!`（第二行 IP、第三行錯誤碼字串） | `:369` | `ADAMTCP_Connect` 連續失敗、內部計數超過 100 | IP、網段、線、電源 |
| `Double EP board connect failed. ...`（912 才有） | `:85`、`:89`（`ShowDoubleEPConnectGuide`） | 第二塊 EP 板（`.112`）連不上；模式 3 要 Multi EP APAX 板 | `INSTALL_DOUBLE_EP` 選對沒有（只需要左右獨立就設 2） |
| `Adam Connect Error and Stop Home` | `uhome.cpp:1926` | REALLY 模式下 HOME 時 `Open_ADAM_6024()` 回 false，**HOME 被擋** | 以上全部 |

### 1.2 golden 連線邏輯的幾個陷阱

- **連線失敗不一定回 false**：`Open_ADAM_6024(IP,Num)` 的失敗計數 `static int iCount=90`，失敗一次 +1，**超過 100 才**跳訊息並回 false；沒超過就照樣往下設 `bADAM6420Install=true`、回 true（912 `adam6024.cpp:253`、`:361-389`）。成功一次會把計數設成 50（`:375`）。⇒ 開機後要連續失敗約 11 次才看得到訊息；成功過之後要再失敗 51 次。
- **舊韌體跳過所有檢查**：`fCheckConnectStatus_ADAM6024` 只有 `bADAM6024FWIsNew[Num]==true`（韌體 `6.01` 且 B≥21，`:2929-2955`）才做名稱／連線數／重連；舊韌體直接回 true（`:3007-3011`）。注意 912 的 `Open_ADAM_6024` 先呼叫狀態檢查、**之後**才判斷韌體新舊（`:264-269`），所以第一次開機那一輪永遠走「舊韌體」分支。
- **讀失敗就整個重連**：`ADAM_ReadVoltage` 讀不到就 `Close_ADAM_6024(); Open_ADAM_6024();` 回 0（`:465-471`）；模組掛掉時每次讀都要付 2 秒級的逾時，畫面會頓。
- **`ClearAllConnection` 不會清連線**（送的是查詢指令，`:2849`）。
- 新韌體路徑重連前呼叫的 `ADAMTCP_Disconnect()` **沒有 IP 參數**（`ADAMTCP.h:159`；只斷一顆的是 `ADAMTCP_ModuleDisconnect(szIP)`，`:160`），之後只重連 `Address[Num]`（`adam6024.cpp:2990-2991`）。同時開著 `.110` 與 `.112` 時要留意這一點（是否會斷掉另一顆，本 skill 沒有在機台上確認）。
- `GetModuleHostIdleTime` 有本體但沒人叫；`SetModuleHostIdleTime` 本體全註解（`:2888-2905`）。Host Idle 要用 ADAM.NET Utility 的 Network 頁設（手冊 p.79）。
- 模組出廠 IP 是 `10.0.0.1`（手冊 p.77）；換新模組要先改成 `172.16.8.110`（HT9050 的 RULINGS_20260929 第 5 節第 6 條）。

## 2. 壓力不對

### 2.1 先分清楚是「寫錯」還是「讀錯」

- 寫出值：`iWritePA`（碼）與 `iAdamOutValue = AdamOutputToPA(iWritePA)`（kPa）。主畫面 `[D26] bD26EnableEncodeShow` 時 `lbEPenconder` 顯示 `"EP: <輸出 kPa> / <讀回 kPa>"`（912 `HS_Function.cpp:133-156`）。
- 讀回值：`ADAM_ReadPA` 用 `EP_MINA_FeedBack`..`EP_MAXA`（V）對到 `EP_MINMPA*1000`..`EP_MAXKPA`（kPa）（912 `adam6024.cpp:518`）。**校正值有一個是 0 或兩個相等時，直接回電壓（被截成 int）**（`:512-515`）——讀回看起來是 0～5 的小數字，就是這個。
- Contact 畫面三個欄位 `edSetKg`／`edTransfer`／`edAirKPA` 是 `TransformFuntion` 自己寫的（`:1801-1806`；Die Force 不寫）。

### 2.2 表沒載（V906 的「silent-30.0」缺陷）

- 移植樹的四張力量表（SLK／SLKInd／DieForce／DieForceOneByOne）開場是空的、`bLoaded==false`，必須先 `LoadContactForceTables()`（`HT9011UC_Cpp_V3.33.906.0\ContactForce.h:320-324` 的警告；移植樹註解常寫成 `:319`）。wb_serve 開機有載（`HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4095`），開機 log 印 `ContactForce tables loaded: %d entries (SLK=%u Ind=%u DieForce=%u DieForce1by1=%u)`（`:4096-4100`）。
- 新的呼叫端（ctest、另一個入口）沒走那條 bring-up，`TransformFuntion` 會拿空表算：`bUseDynamicKitDiameter` 路徑的 `SLKClass[iTag]` 在空表上是越界（golden 也一樣，`adam6024.cpp:1481-1482` 只把 iTag 夾回 0；golden 靠表單 ctor 保證載過）。
- 移植樹 `TransformFuntion` 是 906 本體，**少了 912 的 F4／F15 防呆**（見 `../SKILL.md` §4）：`SLKIndClass` 筆數少於 `iSlkTypeIndTokens*16` 時 16 通道迴圈會越界（移植樹 `adam6024.cpp:762` 起；912 在 `:1711-1712` 先 break）。

### 2.3 缸徑表與換算

- 缸徑比對是 `double ==`（golden），移植樹 :478、:504、:508、:514 改成 1e-6 容差（P18-A3 裁決，`HT9011UC_Cpp_V3.33.906.0\adam6024.cpp:478` 的註解）。缸徑不在表裡時 iTag=0 ＝ 用第一筆。
- `DeviceForm_File.dKitDiameter==0` 時當 3.0（`:787-788`、`:1375-1376`）。
- KYEC 特殊缸徑 2.8→3.0、5.8→6.0（`:844-854`、`:1452-1462`）是客戶分支。
- 測試模式決定除數（`:1220-1330`）：1x2 關一個 site 且 `[D27] bD27UseSingleSite85kg` 時只除 1；2x8 用 `fContact->dDutCount`；加重 `bUseAddWeight` 每顆 +1.25 kg。
- 斜率 `(int)(fMaxUnit/(fMaxMPA-fMinMPA))` 被截成整數（`:1807`），碼比理論值略小，照 golden。
- `ADAM_DirectWriteData` 把 1250、1251、1252 改寫成 1253（「EP 不會動」，`:1990-1991`）。

### 2.4 Dual Force（Die Force）

- 只有 `INSTALL_DOUBLE_EP` 是 1 或 3 時 Start 與 Timer2 才寫（912 `main.cpp:6501`、`:21716`）；寫 AO0（reg 11）。
- `INSTALL_DOUBLE_EP==0` 卻要求 Dual 時 golden 跳 `無安裝dual force, 請確認硬體選項`（`adam6024.cpp:821`、`:1386`）或 `No dual force installed, please check hardware option`（`:987`）。
- Dual 讀回是 AI2，參數 `EPDual_*`（`:519-524`）；警報 `ADAM_DualAlarm`（`:670-712`）、`WAR16323`（`:2737`）。

## 3. 警報代碼

| 代碼 | 文字（`D:\HT9045\Error\AlarmCodeList.txt`） | golden 912 觸發點 | 意思／檢查（`D:\HT9045\Error\English\` 與 `Chinese\` 的 .dat） |
|---|---|---|---|
| `WAR1605` | The air of electronic air regulator (EP) is not enough!（:868） | `atester.cpp:9390`、`:9413`（`IndexEveryTimeCheckEP`）、`:9577`（`CheckAndRecodrEP`） | 讀回 kPa 超出寫出 ± `iD26EPEncoderRange`。查：入氣是否 > 500 kPa、測試頭 O-Ring、氣囊膜片、EP 最大／最小壓力對應電壓校正（`Chinese\WAR1605.dat`） |
| `WAR16322` | EP Controller Return Value Error.（:1009） | `adam6024.cpp:2736` | 主 EP 回授電壓不在 0.8～5.2 V 超過 100 次（約 100 秒，Timer2 每秒一次）或 HOME 時立刻；查 ADAM AI 跳線（電壓／電流）與 AI 範圍設定（`English\WAR16322.dat`） |
| `WAR16323` | DOUBLE EP Controller Return Value Error.（:1010） | `adam6024.cpp:2738` | 同上，Dual EP（AI2） |
| `WAR0329` | EP die force is not enough alarm!（:117） | `csystem.cpp:2754-2767` | `Sen[SnEPDieForce]` 關（ATC active cooling 時），Multi EP 氣路時略過 |

- `ADAM_ReturnValueCheck` 只在 `EP_Install` 3/5 跑；`CUSTOMER_CODE==CC_GIGAS` 不檢查（`adam6024.cpp:2728-2729`，客戶分支）。HOME 中觸發會 `fAllMotorHome=false`（`:2740-2741`）。
- 四個 [D] 開關都關時，每 60 次呼叫讀一次 EP，避免 EP 睡著（`:2748-2756`）。
- 移植樹現況：`ADAM_Alarm()` 替身固定回 false ⇒ `WAR1605` 永遠不會跳（`HT9011UC_Cpp_V3.33.906.0\atester_shims.h:256` 的註解）；`ADAM_ReturnValueCheck(true)` 在 `csystem.cpp:7499` 是 no-op seam；`WAR0329` 的 Multi EP 例外被閘（`csystem.cpp:21195-21225` GATE G24，方向是多報不是漏報）。

## 4. golden 留下的紀錄

| 紀錄 | 內容 | golden 912 出處 |
|---|---|---|
| MNet log（`D:\HT9045_Log\MNetLog`，`main.cpp:1565`） | `AdamOutValue=%f, ReadAdamValue=%d, Range=%d`（`ADAM_Alarm`）、`..., Range=%f`（Kg）、`..., Range=%d%%`（Dual） | `adam6024.cpp:576`、`:586`、`:597`、`:663`、`:696`、`:706`；`MNetLog` 本體 `Motor\myMN200motor.cpp:2151-2160` |
| EP log CSV `D:\HT9045_Log\EP\<yyyymm>\<yyyy-mm-dd>.csv` | 欄位 `Time,Index,Setting,Kpa,Kg,Alarm`；`[D26] bD26EnableEPLog` 時 | `atester.cpp:9443-9472`（表頭）、`:9563`（每筆）；路徑 `common.cpp:49` |
| Process 紀錄（`NewRecordProcess`） | `APAX Connect to '%s' result = %d`、`APAX Disconnect from ...`、`APAX SEND DATA FAIL %d, %s`、`APAX MEP3 ARM1 S0 / ARM2 S1 SEND DATA FAIL ...`、Double EP 說明訊息 | `adam6024.cpp:2274-2276`、`:2296-2297`、`:2594-2595`、`:2428-2429`、`:2451-2452`、`:91` |
| MyDBI | `Motion` / `ADAMTCP_Open Fail!`；`Exception` / `ADAM_ReadVoltage`、`ADAM_WriteVoltage`、`ADAM_ReadAIValue`、`TfAdam6024::CloseSocket` | `:282`、`:495`、`:1847`、`:1921`、`:2193`、`:2125` |
| 軟體時間 | `TfMain, Open_ADAM_6024`、`TfMain, Close_ADAM_6024()`、`TfAdam6024, FormDestroy` | `main.cpp:9990`、`:12196`、`adam6024.cpp:2215` |

<!-- preserved-content:end -->
