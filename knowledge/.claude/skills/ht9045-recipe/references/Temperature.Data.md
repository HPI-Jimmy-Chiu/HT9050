# Temperature.Data — 溫度模式與加熱設定

**路徑：** `D:\HT9045\IniData\Data\[工作檔名]\Temperature.Data`
**模組：** [uTemp_Set.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\uTemp_Set.cpp)
**結構體：** `SYSTEM_TEMPERATURE Temperature` (global)
**讀：** `TfTemp_Set::ReadTempFile(bool bUpdateAll)` ｜ **寫：** `TfTemp_Set::WriteTempFile()`

> **注意：** 程式模組位於 `uTemp_Set.cpp`，非 cTemperature.cpp。

> ⛔ 20261001 更正（St01 ST01-E2 對 golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp` 抽查）：本檔有幾處錯，以這裡與 `D:\HT9045\.claude\skills\ht9045-temperature\references\temp-set-and-lotinfo.md` 為準——
> - 連結指到 V900 樹；**沒有 `WriteTempFile`**，寫檔是 `SaveSetupFile`（`:4606-5172`）；結構 `SYSTEM_TEMPERATURE` 定義在 `cprod.h:1391-1661`（不在 MachineType.h）；`uATC_controller.cpp` 不存在。
> - `[Mode] Mode`：0 Hot、1 Ambient、**3 AmbientHot**；其他值（2）＝維持目前模式（FTP 情形讀 WorkTempMode）（`:2047-2071`）。下面「範例」的 Mode 值不要照抄。
> - `[Index] Heating Mode` 是 `eIndexHeatMode`（golden `MachineType.h:539`）：0 HeadOnly、1 ChamberOnly、2 HeadChamber、3 SocketChamber、4 HeadSocket、5 HeadChamberSocket（讀 `:2146`、預設 HeadOnly；寫 `:4765`），不是 0＝HeadOnly／1＝All。
> - `[ATC] Temperature Set` 是 **bool**（`Temperature.bATCTemperatureSet`，`:2555`／`:4771`），不是目標溫度。
> - `[ChamberBoostMode] iChamberBoostTime` 範圍 1～30（`:2462`），單位**分鐘**（LotInfo 計時器乘 60），不是秒。
> - 沒寫到的：校正檔 `DefineTemp\Temperature*.Data`、`Tester.Data [InitialMode]`、`config.ini [SingleTempLimit]`、`Config\ATC.ini [Setup] iCheckSameTempTime`、讀檔時的夾值、`" Tj Avg Times"` 寫檔鍵名多一個空白（`:4822`，讀的是 `:2799`）。

---

## Section 總覽

| Section | 用途 |
|---------|-----|
| `[Mode]` | 溫控模式與主要參數 |
| `[Ambient]` | 常溫模式控制 |
| `[Time]` | 浸泡/冷卻時間設定 |
| `[Index]` | Index 加熱模式與 Air Cooling |
| `[User OffSet]` | 使用者溫度補正 CH1..CHn |
| `[Kit Low/Mid. /High OffSet]` | Kit 各溫度偏移量 |
| `[Kit AmbientHotLowOffSet]` / `[Kit AmbientHotMidOffSet]` | 環溫 Hot 偏移量 |
| `[Init Temp OffSet]` | 初始溫度補正 |
| `[Cal Temp]` / `[IndividualTempSetting]` | 各 Channel 獨立溫度設定 |
| `[Boost Function]` | Boost 升溫功能 |
| `[LB Temp Function]` | LB（液態氮/低溫）功能 |
| `[ChamberBoostMode]` | Chamber 升溫功能 |
| `[DUT Setting]` | DUT 固定溫度設定 |
| `[InitialMode]` | 初測模式與 Air Stream 設定 |
| `[ATC]` | ATC（主動溫控）詳細設定 |
| `[Cooling]` | 冷卻模式 Chamber 溫度 |

---

## `[Mode]` — 溫控主要參數

| Key | 型態 | 預設 | 說明 | 對應變數 |
|-----|------|------|------|---------|
| Mode | int | 0 | 溫控模式（0=Hot, 1=Ambient, 2/3=特殊） | `Temperature.iMachineTempMode` |
| Temperature | float | 0.0 | 工作目標溫度 (°C) | `Temperature.fWorkTemperBase` |
| WorkTempMode | int | LastSet | 工作溫度模式（從 LastSet 讀取） | `LastSet.iTemperature` |
| UseIndividualTemp | bool | false | true=各 Channel 獨立溫度設定 | `Temperature.bUseIndividualTemp` |
| bUseInitialDelayAsSoakTime | bool | false | true=Initial Delay 當作 Soak Time | `Temperature.bUseInitialDelayAsSoakTime` |
| bTempAlarmBinNeedToError | bool | false | true=溫度警報 Bin 需觸發 Error | `Temperature.bTempAlarmBinNeedToError` |
| AmbCheck | bool | false | 啟用環境溫度監察 | `Temperature.bUseAbitCHK` |
| WaitDewPoint | bool | false | 啟用等待露點感測器功能 | `Temperature.bWaitDewPoint` |
| DewPointRange | float | 10.0 | 露點報警範圍 (°C)，ClampRange(1~30) | `Temperature.dDewPointRange` |
| DewPointAlarmInterval | int | 10 | 露點報警間隔 (秒) | `Temperature.iDewPointAlarmInterval` |
| bLBCoolingAirOn | bool | false | LB 冷卻時吹氣 | `Temperature.bLBCoolingAirOn` |
| dLBAirOnTemp | float | 55.0 | LB 吹氣觸發溫度 (°C) | `Temperature.dLBAirOnTemp` |
| iLBTempAlmInterval | int | 30 | LB 溫度報警間隔 (秒) | `Temperature.iLBTempAlmInterval` |
| Active_Heat_Gun | int | - | 使用熱槍（Hot Gun）加熱 | — |
| Use CDA Only | int | - | 只使用壓縮乾燥空氣 | — |
| iHotGunFLowLimit_H | int | - | 熱槍流量上限 | — |
| iHotGunFLowLimit_L | int | - | 熱槍流量下限 | — |
| Active_ATC_Heat_Gun | int | - | ATC 熱槍啟用 | — |
| bTempCalByRecipe | bool | - | 依 Recipe 做溫度校正 | — |

---

## `[Ambient]` — 常溫控制

| Key | 型態 | 預設 | 說明 | 對應變數 |
|-----|------|------|------|---------|
| Temperature | float | 25.0 | 環境溫度警戒上限 (°C) | `Temperature.fAbitTemp` |
| Check | int | - | 1=啟用環溫上限警報 | `Temperature.iAbitCHKStatus` |
| fAmbientHotGuartbent | float | 3.0 | 環溫防護帶寬度 (°C)，ClampRange(1~30) | `Temperature.fAmbientHotGuartbent` |
| bShuttleNoHeatUp | bool | false | Shuttle 不加熱（旁路） | `Temperature.bShuttleNoHeatUp` |
| bSLKNoHeatUp | bool | false | SLK 不加熱（旁路） | `Temperature.bSLKNoHeatUp` |
| bAmbUsingAFan | bool | true | 使用 A-Fan 散熱 | `Temperature.bAmbUsingAFan` |
| bAmbientGuardbandCheck | bool | true | 啟用環溫 Guardband 檢查 | `Temperature.bAmbientGuardbandCheck` |
| iAmbGuardband | int | 1 | 環溫 Guardband 容許範圍 | `Temperature.iAmbGuardband` |
| ByPassChamber | int | - | 旁路 Chamber | — |

---

## `[Time]` — 時間設定

| Key | 型態 | 預設 | 說明 | 對應變數 |
|-----|------|------|------|---------|
| Soak | float | 999.0 | 浸泡時間 (秒) | `Temperature.fSoakTime` |
| Jam Soak | float | 0.0 | Jam 後重新預熱時間 (秒) | `Temperature.fJamSoakTime` |
| H.Initial | float | 0.0 | 加熱初始等待時間 (秒) | `Temperature.fInitialWaitTime` |
| A.Initial | float | 0.0 | 常溫初始等待時間 (秒) | `Temperature.fAbitInitWaitTime` |
| Cool Time | float | 0.0 | 冷卻等待時間 (秒) | `Temperature.fAbitColdTime` |
| iInitialStart1Time | int | 0 | Initial Start 1 時間 (秒) | `Temperature.iInitialStart1Time` |
| iInitialStart2Time | int | 0 | Initial Start 2 時間 (秒) | `Temperature.iInitialStart2Time` |
| iIndexSoakTime | int | 0 | Index 下壓後額外浸泡 (秒) | `Temperature.iIndexSoakTime` |
| iOSTime | int | 0 | OS 時間 (秒) | `Temperature.iOSTime` |
| In Shuttle Soak Time Mode | int | 1 | 1=進 Shuttle 時開始計算浸泡 | `Temperature.iShuttleSoakTimeMode` |
| bZ2DownSocket | int | 0 | 1=Z2 下壓 Socket 後才計算浸泡時間 | `Temperature.bZ2DownSocket` |

---

## `[Index]` — Index 加熱 / Air Cooling

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| Heating Mode | int | HeadOnly | 0=HeadOnly, 1=All（全區加熱） |
| Arm1/Arm2 Offset | float | — | Arm1/Arm2 溫度偏移量 |
| Arm1/Arm2 No Fullsite Offset1~5 | float | — | 非全 Site 時各類偏移量 |
| ATC_HeatGunTemp | float | — | ATC 熱槍溫度設定 |
| dSocketAirCoolingOnTimer | float | — | Socket Air Cooling 开啟計時 |
| dSocketAirCoolingOffTimer | float | — | Socket Air Cooling 關閉計時 |

---

## `[User OffSet]` — 使用者溫度補正

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| CH1, CH2, ... CHn | float | 0.1 | 各溫控點補正值 (°C)，有 ClampRange 保護 |
| MaxLimit | int | 60 | 補正上限 (°C) |
| MinLimit | int | -60 | 補正下限 (°C) |

---

## Kit 偏移 Sections — Kit 各溫度補正

以下 section 均為 `CH1..CHn` 陣列（預設 0.0）：

| Section | 用途 | 對應 Index |
|---------|-----|------------|
| `[Kit Low OffSet]` | Kit 低溫模式補正 | `KitLowBase` |
| `[Kit Mid. OffSet]` | Kit 中溫模式補正 | `KitMidBase` |
| `[Kit High OffSet]` | Kit 高溫模式補正 | `KitHigBase` |
| `[Kit AmbientHotLowOffSet]` | 環溫 Hot Low 補正 | `KitAmbientHotLow` |
| `[Kit AmbientHotMidOffSet]` | 環溫 Hot Mid 補正 | `KitAmbientHotMid` |
| `[Init Temp OffSet]` | 初測溫度補正 | `InitTempOffset` |

---

## `[Cal Temp]` / `[IndividualTempSetting]` — 各 Channel 設定

需 `UseIndividualTemp=true` 才生效。  
`CH1..CHn`：各 Channel 獨立目標溫度 → `Temperature.fIndividualTemp[i]`

---

## `[Boost Function]` — Boost 升溫功能

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| bBoostFuncttion | bool | false | 啟用 Boost 功能（注意：原始 typo 為 Funct**t**ion） |
| iBoostFunctionMode | int | 0 | Boost 模式（0=標準） |
| dBoostIdleTime[0/1/2] | float | 15.0/5.0/— | Boost 閒置計時 (秒) |
| dBoostOffset[0/1/2] | float | — | Boost 升溫偏移量 (°C) |
| dBoostDuration[0/1/2] | float | — | Boost 持續時間 (秒) |
| dPostBoostDuration[0/1/2] | float | — | Boost 後補償時間 (秒) |

---

## `[LB Temp Function]` — LB 低溫功能

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| bLBTempFunction | bool | false | 啟用 LB（液態氮/Coldplate）溫控功能 |
| dBoostIdleTime[3/4/5] | float | — | LB Boost 閒置計時 (秒) |
| dBoostOffset[3/4/5] | float | — | LB 升溫偏移量 (°C) |
| dBoostDuration[3] | float | — | Boost 持續時間 (秒) |
| dPostBoostDuration[3] | float | — | Post-Boost 時間 (秒) |
| bEnableBoostOffset[3/5] | bool | — | 啟用對應 Boost Offset |
| dBoostTimeOut | float | 1200.0 | Boost 超時時間 (秒) |
| LB_Temp_High_Setting | float | — | LB 高溫警戒值 |
| LB_Temp_Low_Setting | float | — | LB 低溫警戒值 |
| bLBTempHighAlarm_Enable | bool | — | 啟用高溫警報 |
| bLBTempLowAlarm_Enable | bool | — | 啟用低溫警報 |
| Threshold | float | 0.0 | 溫度閾值 |

---

## `[ChamberBoostMode]` — Chamber 快速升溫

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| bEnableChamberBoost | bool | false | 啟用 Chamber Boost |
| iChamberBoostTime | int | 10 | Boost 時間 (秒) |
| iChamberBoostOffset | int | 10 | Boost 溫度偏移量 (°C) |

---

## `[DUT Setting]` — DUT 固定溫度

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| bUseFixTemp | bool | false | 啟用固定 DUT 溫度功能 |
| dFixedTemp | float | 40.0 | 固定溫度值 (°C) |
| bShowFixedTemp | bool | false | 在 UI 顯示固定溫度 |

---

## `[InitialMode]` — 初測模式（AirStream）

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| EnableTemperatureOffsetforInitial | bool | false | 初測時套用溫度補正 |
| iCintactCntForTempOffsetAtInitial | int | 1 | 初測補正啟動接觸計數 |
| iCintactDelayCntForInitTempOffset | int | 10 | 初測補正延遲計數 |
| iTempReadyRange | int | 0 | 溫度就緒容許範圍 (°C) |
| UsePIDControl | bool | false | 啟用 PID 控制 |
| EnableAirMachineSocket | bool | — | 啟用 Socket Air Machine |
| SetTempature2AirMachine | float | — | AirStream 目標溫度 |
| bEnableArm_1_Air / bEnableArm_2_Air | bool | — | Arm1/Arm2 Air 啟用 |
| bEnableSocket_Air | bool | — | Socket Air 啟用 |
| iAirVolumeLmt | int | — | Air 流量限制 |
| dSetIndexAirstreamTemp | float | — | Index AirStream 溫度 |
| SetAirstreamTemperatureRang_Index | float | — | Index AirStream 容許範圍 |
| dSetAirstreamTemperatureRang_Socket | float | — | Socket AirStream 容許範圍 |
| AirStreamIndex_Offset | float | — | Index AirStream 補正值 |
| AirStreamSocket_Offset | float | — | Socket AirStream 補正值 |
| OutShuttleDesoakTime | float | — | OutShuttle 冷卻等待時間 |
| UseOutShuttleDesoakTime | bool | — | 啟用 OutShuttle 冷卻功能 |
| EnableTesterDryAirControl | bool | — | Tester 乾燥空氣控制 |
| Defrost_Time_Too_Lower | float | — | 除霜時間不足判斷 |
| iATC_PID_Max_Offset_ / iATC_PID_Min_Offset_ | int | — | ATC PID 補正上下限 |

---

## `[ATC]` — 主動溫控（Active Thermal Control）

> ATC section 含大量 key（40+），僅列常用欄位，詳細請參考原始碼。

| Key | 型態 | 說明 |
|-----|------|------|
| Type Name | string | ATC 型號名稱 |
| Temperature Set | float | ATC 目標溫度 |
| Chiller Temp | float | Chiller 目標溫度 |
| Multi Zone Enable | bool | 多區域控制啟用 |
| Tj Mode | int | TJ 模式 |
| USE Tj Function | bool | 啟用 TJ 功能 |
| bPowerFollow_Enable | bool | 功率追蹤功能 |
| bEnableChamberBoost | bool | Chamber Boost 啟用 |
| ATC7 | bool | ATC7 模組啟用 |
| ATC7CH1~CH4Enabled | bool | ATC7 各 CH 啟用 |

---

## `[Cooling]` — 冷卻

| Key | 型態 | 說明 |
|-----|------|------|
| ChamberCoolTemp | float | Chamber 冷卻目標溫度 |

---

## 範例（加熱 85°C + Boost）

```ini
[Mode]
   Mode=2
   Temperature=85.0
   WorkTempMode=2
   UseIndividualTemp=0
   AmbCheck=1
   bUseInitialDelayAsSoakTime=0
   WaitDewPoint=0
[Ambient]
   Temperature=30.0
   Check=1
   fAmbientHotGuartbent=5.0
   bShuttleNoHeatUp=0
   bAmbUsingAFan=1
   bAmbientGuardbandCheck=1
   iAmbGuardband=3
[Time]
   Soak=90.0
   Jam Soak=30.0
   H.Initial=1.0
   A.Initial=0.0
   Cool Time=0.0
   iIndexSoakTime=0
   In Shuttle Soak Time Mode=1
[Index]
   Heating Mode=0
[User OffSet]
   CH1=0.0
   CH2=0.0
   CH3=0.0
   CH4=0.0
   MaxLimit=60
   MinLimit=-60
[Boost Function]
   bBoostFuncttion=1
   iBoostFunctionMode=0
   dBoostIdleTime[0]=15.0
   dBoostOffset[0]=10.0
   dBoostDuration[0]=30.0
   dPostBoostDuration[0]=10.0
```

---

## 注意事項

- `Mode=1` (Ambient)：`Information.txt` 寫出 `Temp=25`, `Soaktime=0`
- `Mode=2` (Heat)：等待溫度達到 `Temperature` 才開始計算浸泡時間
- `UseIndividualTemp=true`：需於 `[Cal Temp]` 或 `[IndividualTempSetting]` 設定各 CH 目標
- `[ATC]` section 由 `uATC_controller.cpp` 搭配讀取，此處定義優先權較高

---

## 關聯程式碼

- 讀寫實作：[uTemp_Set.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\uTemp_Set.cpp) — 搜尋 `ReadTempFile` 定位
- 表單定義：[uTemp_Set.h](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\uTemp_Set.h)
- 結構體：[MachineType.h](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\MachineType.h) — `SYSTEM_TEMPERATURE`
- ATC 控制：[uATC_controller.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\uATC_controller.cpp)