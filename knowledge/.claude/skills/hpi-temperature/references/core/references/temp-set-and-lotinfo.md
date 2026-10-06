> 保存來源：`.claude/skills/ht9045-temperature/references/temp-set-and-lotinfo.md`，main `84233b648`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# golden V912 的 Temp_Set 與 LotInfo 溫度項目

> G＝golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`，P＝移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`。
> 只寫 `:N` 的是 `G\uTemp_Set.cpp` 的行號，其他檔另外寫檔名。來源：20261001 唯讀查證；ST01-E2 抽查過 `[Mode] Mode`、
> `" Tj Avg Times"`、兩個 ATC.ini、`bATCTemperatureSet`、`iChamberBoostTime`。

## 1. Temp_Set（G\uTemp_Set.cpp，7085 行）

### 1.1 綁到哪些結構
- `SYSTEM_TEMPERATURE Temperature` 定義在 `G\cprod.h:1391-1661`（不在 MachineType.h）。逐通道陣列：`fTempOffSet[19][tcTotalCount]`（`cprod.h:1404`）、
  `fIndividualTemp[]`（`:1448`）。列索引 LowBase..SHigBase 在 `uTemp_Set.cpp:48-64`。
- 其他目標：`IniConfig.dSingleTempLimit[]`、`dATCAmbientTemperature`、`dHeatGunTempATC`、`LastSet.iTemperature`、`ATCInterfaceForm->iCheckSameTempTime`。

### 1.2 檔案與鍵
1. recipe `<DataPath><recipe>\Temperature.Data`；save-by-machine 與 A57_2 都開時改放 `IniData\SaveByMachine\`（`:1996-2007`）。
   - 讀 `ReadTempFile`（`:1986-3182`）、寫 `SaveSetupFile`（`:4606-5172`）。**沒有 `WriteTempFile` 這個函式**。
   - 段：`[Mode] [Ambient] [Time] [Index] [User OffSet] [IndividualTempSetting]／[Cal Temp]`（ATC6.0）`[Init Temp OffSet] [Kit *] [Boost Function]
     [LB Temp Function] [ChamberBoostMode] [DUT Setting] [InitialMode] [Sigma] [ATC]`（約 100 鍵）`[Cooling] [TriTempSet]`。
   - `[Mode] Mode`：0 Hot、1 Ambient、3 AmbientHot；其他值（2）維持目前模式，FTP 情形讀 WorkTempMode（`:2047-2071`）。
   - 設定溫度＝`edWorkTemp` → `[Mode] Temperature`，存 1 位小數（`:4635`）。
   - `[ATC] Temperature Set` 是 bool（`Temperature.bATCTemperatureSet`，`:2555` 讀、`:4771` 寫），不是目標溫度。
   - `[ChamberBoostMode] iChamberBoostTime` 範圍 1～30（`:2462`），單位是**分鐘**（LotInfo 的計時器乘 60，見 §2）。
2. recipe `Tester.Data [InitialMode]`：初始延遲，`:5103-5152` 寫。
3. 校正檔 `DefineTemp\Temperature*.Data`：用哪一個看加熱模式、60 mm pitch、ATC 冷熱、或 recipe 的 `DefineTemperature.Data`。
   內容：Points、Low／Mid／High／SHigh／AmbientHot base、逐 CH offset。選檔 `:2909-3032`（讀）、`:4402-4497`（寫）；值在 `spbSaveClick` 裡 `:4501-4537` 寫。
4. `D:\HT9045\config\config.ini [SingleTempLimit] CH%02d`，0～10：`G\cprod.cpp:2669-2685`，經 `SaveLastSetIni` 存（`:4547-4552`）。
5. `D:\HT9045\Config\ATC.ini [Setup] iCheckSameTempTime`：讀 `:2875`、寫 `:5053`（最小 20，`:5047`）。ATC 本身的設定在 `D:\HT9045\system\ATC.ini`（SKILL.md §3）。

### 1.3 檢查
- 鍵盤輸入範圍（`ShowQwertyKey` 自己會把 min／max 對調，`G\myQwertyKeyBoard.cpp:263-269`）：
  - 設定值／base：`MaxTempSetting`（`:5692`；135，依限制與加熱模式 150／155／175／200；ATC 常溫 30 或 19）、`MinTempSetting`（`:5765`；20；ATC 25 或 -5）。
  - offset：`InputLimit.iTempHigh／Low`（`:4174`）；InPC ±20（ASE-K ±60，`:5453`）；ATC 常溫 25..30 或 -5..19（`:5478`）；Chiller 依 ATC 型（`:5524`）；Arm offset ±2＋權限 17（`:5503`）。
- 讀檔時的夾值：SP2 20..135、InPC ±30、Chiller 5..40／-20..30／-40..30／-55..30（`:2562-2580`）、TJ 範圍、ChamberBoost 1..30 分；
  常溫溫度不在 25..40 就設 0（`:2099-2108`）；工作溫度夾在上面的 max／min（`:2172-2177`）。
- 存檔按鈕依序擋：A02 操作員（`:4240`）→ SECS `CheckTempSettingChange`（`:4258`）→ 逐通道 `FormHS->CheckTempOffset`（`G\HS_Function.cpp:4078`；
  base＋offset＋設定值，4 點，`:4296-4323`）→ ATC3.3 暖機鎖（`:4329`）→ FFC 開關時間順序（`:4343-4376`）。
- 權限：`LevelSet.AccessLevel[44, 50-57, 106, 124]`（`:904-992`）；ATC On 要 `fSecurity` 158（`:450`）；生產中關鍵參數鎖住（`:1076-1085`）。

### 1.4 事件與效果
- `spbSaveClick :4238` → `SaveSetupFile` → 寫 DefineTemp → 單點限制 → `BackupSetupFile` → `bNeedInitialTestDelay=true` → `ReadTempFile(true)`。
- `sbtExitClick :5175`：依 `rbATCActiveOn` 設 `bStartATCRun`、更新 LotInfo 的 `pl_ATC_Online`、按 ATC2.0 OnLine／OffLine、設 `bSetTempChange=true`。
- 主畫面尾段 `sbTempOffsetClick`（`G\main.cpp:28345-28401`）：更新 `LastSet.iTemperature`、呼叫 `SaveTestMode` 與 `ReadTempFile`、設 `fHeaterOK=false` 與 `iThermoTask=1`，再 `SetWorkParameter`。
- 其他：`rgIndexHeatModeClick :4232` → `ReadTempFile(false)`；`sbSafeTestATCClick :5623` → `SendATCSelfTest(2)`（`:5576`；1046 手動／1044 自動）；Defrost `:6844`／`:6885`；
  `rbATC*ActiveOnClick :5407-5451`。
- `ReadTempFile` 結尾（`:3150-3180`）呼叫 `fLotInfo->SetATCFormVisible` 與 `COM2->ATCInitialTask`。
- 遠端 offset：`SaveRemoteTempOffset :5934`、`ReadRemoteTempOffset :6169`（GPIB `SETTESTOFFSET_`，見 `ht9045-st02-workflow` 的 w9 計畫）。

## 2. LotInfo 的溫度項目（G\uLotInfo.cpp）

- **edTemp**：`CutTempToEdit :5336`（SCC／SCK，夾 25..140）。
  - `btDownloadClick` 的檢查（`:2860-2970`）拿 edTemp（"ROO"＝常溫）比對模式、`fWorkTemperBase`、`fAmbientTemp`；不符就跳訊息並設 `bHasDownloadFile=false`。
  - 從 `IniConfig.sProductTemp` 還原（`:14830`）。
- **顯示**：`ShowATCThermo :5574` 由 `G\cTemperFrom.cpp:1689` 每拍呼叫，選設定溫度與範圍後再呼叫 `ShowATC70Thermo :5631`、`ShowATC20Thermo :5972` 或 `ShowNewATCThermo :6462`；
  `ShowATCTempPanel :14836`、`SetATCFormVisible :10119`。
- **ATC 命令泵 `NetATCTimeTimer :8725`**：
  - ATC 關：斷線並設 `bSetTempChange`（`:8812-8846`）；`bSetTempChange` 為真時整串重送（`:8872-8888`）。
  - Task 6 `ChangeRecipe`（1001，`:9114`／`:9120`）；Task 7 `SetAllTemp`／`SetMultiZoneTemp`（1002，`:9164-9176`），加 KYEC LotStart／LotEnd 重送（1050／1051，`:9186-9203`）；
    Task 8 `SetATCOffset`（1003，`:9222`／`:9226`）、`SetATCFFCOffset`（1063，`:9244`）；Task 11 Chiller（1039，`:9358`）；Task 13／14／16 TJ（1076／1077／1100，`:9399`／`:9422`／`:9454`）；Task 17 PF（1104，`:9469`）。
- **`SetATCOffset :9586`**：offset＝InPC＋TestTime／Initial／ConsFail／QA offset。（`ht9045-atc` 寫的 "L9006" 是 V899 的行號。）
- `pl_ATC_OnlineClick :10420`（SystemStart 時擋；security 114）。
- Chamber boost：`tmrChamberBoostTimer :11911`（`iChamberBoostTime*60`，MES15401）；Start／Stop `:11986`／`:11994`。
- `btnClearTemperatureClick :14121`、`AutoTempOfsByFTP :14185`。
- **Lot start／end**：`SetLotStart :1614` 只設 KYEC 溫度紀錄旗標與 `asATCEvenLotID`（`:1798-1803`）。真正送 ATC LotStart 的是 `CheckCanRunStart_HS`
  （`G\HS_Function.cpp:2918-2956`），只有 KYEC 的 ATC3.1／3.2 或 ATC2.0，並一起檢查自我測試狀態。LotEnd：`G\csystem.cpp:11103`／`:11109`；自動自我測試：`G\csystem.cpp:18822`／`:18837`。
- `ht9045-atc` 寫的 `InitSiteToATC` 實際叫 `InitialAddrToATC`（`:6437`）。

## 3. 執行時誰用這些值

- 溫控迴圈：`HeaterThreadProcess`（`G\uHeaterThread.cpp:56`）→ `DoThermo`（`G\bthermo.cpp:1165`）→ `DoThermoReal :1232`。
  設定值由 `GetFactSetTemp :310`／`ConvertTempOffset :75` 算：base、n 點內插、Kit、User、Init／TestOverTime、Boost offset（在 `:1398-1427` 用）。
  寫入控制器：EJ1N `:3501`、DTME08 `:4879`、ATC6.0 `:3361`。
- `CheckHeater`（`G\uHeaterThread.cpp:136`）用這些範圍設 `fHeaterOK`：逐通道溫度（`:536`）、多區（`:565`）、TJ 範圍（`:571`）、ready 範圍（`:613-631`）、
  LB 告警（`:650`）、單點限制（`:681-689`）、guardband（`:782-808`）。開始生產的閘是 `CheckHeaterOK :112`。
- 浸泡：`Prod.iHotTime`（`G\cinitial.cpp:7076-7083`）；Jam 浸泡 `G\aTester_Front.cpp:1692`。

## 4. V906 現況（摘要；完整表在 SKILL.md §2）

- Temp_Set：C 路已接（`P\FileRW\Temperature.cpp`＋`Temperature.gen.inc` 跑 golden 存檔，加上 `MainTempOffsetTail` 與 `W906_ReadATCIni`）；
  網頁 `D:\HT9045\web\page\Setup.Temp_Set.html`＋`ht9045_temp_set_c.js`；json-bridge `write-inventory.md:36` 標 ✅（`bc935659`）；R97～R99（`4e74e8b4`）加了事件；R130：Arm offset 不存。
  `P\uTemp_Set.cpp` 裡 `SendATCSelfTest`（S5）、`sbtExitClick` 的 ATC 部分、Defrost 擋著；S3 打開（H-109）。
- LotInfo（`P\forms\fLotInfo.cpp`）：只有顯示（`W906_ShowATCThermoDisplay :6282`、`CutTempToEdit :2225`、`SetATCFormVisible :1468`、`ShowATCTempPanel :1568`）；
  沒移植（`P\forms\fLotInfo.h:1091-1128` 列為會動機台）：`NetATCTimeTimer`、`SetATCOffset`、`ShowATC*Thermo`、`SetATCFFCOffset`、ATC 上線切換、Chamber boost、ClearTemperature、`AutoTempOfsByFTP`。
  網頁 `Data.LotInfo.html` 有 ATC 面板。

<!-- preserved-content:end -->
