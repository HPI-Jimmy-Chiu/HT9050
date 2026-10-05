# Contact Force 計算流程、SLK 缸徑表與 V906／web 端實作（同事版）

> 📥 本檔是 repo 既有的 `ht9045-contact-force/SKILL.md`（同事整理，含 Steven 20260921～0927 的 web 端實作與移植樹註記），
> 20261001 與 RogerYang 版合併時，因兩份是**不同主題的文件**（不是新舊版），整份原樣移到這裡、由主 SKILL.md 連結。


# HT9045 Contact Force 計算流程

> **//Steven 20260921** — 這條計算鏈**已有 web 端實作**：
> `D:\HT9045\client\ht9045_contact_slk.js`（部署到 `web\page\`），
> 給 `Setup.Contact.html` 用。逐函式忠實翻譯，四處刻意的界線見本檔末節。
> 當日完整變更紀錄：`<入口網站 repo>\public\Docs\ChangeLog\Steven\CHANGES_20260921_Steven.md` §8 / §9

> **//Steven 團隊 20260926** — 移植樹（V906 C++）現況，給要改這條計算鏈的人：
> - 上面那支 web 端檔案現在的位置是 `D:\HT9045\web\page\ht9045_contact_slk.js`（`D:\HT9045\client\` 已不存在，
>   `web\page\` 是唯一權威；`Setup.Contact.html:134` 載入它）。
> - `Setup.Contact.html` 另走 C 路（`DeviceForm_File`，golden `TfContact`）：存檔前伺服器端照 golden 事件順序重算
>   衍生欄位（`CalculateTotalAirForce`／`GetMaxIndexForceLimit`／`GetMinForce`／`CountDieForceKg`，commit `a9636d9c`，
>   skill `ht9045-json-bridge` `references/generators.md` 十四）。
> - golden `TfContactForce`（`ContactForce.cpp`）的讀寫檔已翻（S57，commit `21d37f2b`）：`SLKClass` 容器在
>   `ContactForce.h` 的 `ContactForceTables()`、開機載入器 `ContactForceLoad.cpp` 的 `LoadContactForceTables()`（含
>   `dIndexZOffset`：`[0..1][0..14]` 讀 `Gerneral.ini [Test Arm]`、`[2][*]` 是固定階梯）；EP 四鍵與 HSys 兩頁都寫，
>   採「本頁沒改的欄位不把舊值蓋回」（S90，`f1ad780c`）。`Setup.ContactForce.html` 還是靜態頁、沒接 C 路——
>   建頁備忘見 skill `ht9045-json-bridge` 的 `references/pending-pages.md` 十五。

> **//Steven 團隊 20260927** — R15＝B（commit `725038a6`，移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp`）起，
> 移植樹**開機就跑** golden `TfContactForce` 建構子，會寫 `D:\HT9045\system\ContactInfo.ini`（`--dry` 擋不住）。
> 開機順序、兩份表（頁面面板 vs 機台用的 `ContactForceTables()`）、開機補哪些鍵（Steven01 是 64 段 128 把鍵）、
> golden 沒檔時把 `EP_*_1032` 寫成 0（R72）、過期註解、R15～R18 現況（R16 紀錄與程式不符）→
> [references/v906-boot-contactinfo.md](references/v906-boot-contactinfo.md)


## 核心檔案

| 檔案 | 說明 |
|------|------|
| `cContact.cpp` | Contact 模式主控（~19000 行），包含 Force 計算、Auto Height、Contact Test 狀態機 |
| `cContact.h` | TfContact 類別宣告 |
| `ContactForce.cpp` | SLK 類別初始化、Load Rate 讀寫、ContactInfo.ini 管理 |
| `ContactForce.h` | THTSLKClass / THTDieForceSLKClass / THTSLKIndClass 類別定義 |

## Contact Force 計算架構

### 計算入口
```
ShowArmAndDeviceForce()
  → CalculateTotalAirForce(dBallCount, dSingleGf)
    → GetMinForce(dKitDiameter, iTag)        // 最小力量保護
    → GetMaxIndexForceLimit()                 // 最大力量上限
```

### 關鍵公式

#### 1. 單顆 IC 所需力量
```
dDeviceGf = dBallCount × dSingleGf × 0.001    // (kg)
```
- `dBallCount`：Pin 數（球數）
- `dSingleGf`：每 Pin 力量 (gf)

#### 2. 總力量計算
```
dTotalForce = dDeviceGf × dDutCount
```
- `dDutCount` 由 `DutCount()` 依 TestMode 決定

#### 3. 每個 Compliance Unit 分攤力量
```
dNowKgPerHead = dTotalForce / (HeadCount × fComplianceUnit)
```
- `HeadCount` 依 TestMode 而定（見下表）
- `fComplianceUnit` 由 scrbSLK->Position 決定

#### 4. D28 最大力量限制（依缸徑計算）
```
dHeadMaxForce = ((dKitDiameter² × π) / 4) × coefficient × 0.0101972 × fComplianceUnit
```
- `coefficient` = 5.0（EP_MAXKPA ≤ 500）或 6.0（EP_MAXKPA > 500）
- 需開啟 `IniConfig.bD28MaxForceLimitByDiameter`

## SLK 缸徑（Kit Diameter）對照表

### 最小力量（Min Force per Compliance）

| 缸徑 (mm) | 預設 Min Force (kg) | D04 覆蓋參數 |
|-----------|---------------------|-------------|
| 20        | 0.5                 | `dD04MinForceByFile_20mm` |
| 28, 30    | 1.5                 | `dD04MinForceByFile_30mm` |
| 40        | 4.0                 | `dD04MinForceByFile_40mm` |
| 58, 60    | 8.0                 | `dD04MinForceByFile_60mm` |
| 80        | 15.0                | `dD04MinForceByFile_80mm` |
| 其他      | 讀 SLKClass.dContactOffset | `dD04MinForceByFile` |

> 若 `IniConfig.bD04MinForceByFile == true`，使用 ini 檔設定值（取較大者）。

### 最大力量（D28 MaxForce by Diameter）

以缸徑 30mm、EP_MAXKPA=499 為例：
```
dHeadMaxForce = ((30² × 3.14) / 4) × 5.0 × 0.0101972 × fComplianceUnit
             = (706.5 / 4) × 5.0 × 0.0101972 × fComplianceUnit
             = 176.625 × 5.0 × 0.0101972 × fComplianceUnit
             ≈ 9.01 × fComplianceUnit (kg)
```

### 各缸徑 D28 MaxForce 速查（fComplianceUnit=1.0）

| 缸徑 (mm) | EP≤500kPa (coeff=5) | EP>500kPa (coeff=6) |
|-----------|---------------------|---------------------|
| 20        | ≈ 4.00 kg           | ≈ 4.80 kg           |
| 28        | ≈ 7.85 kg           | ≈ 9.42 kg           |
| 30        | ≈ 9.01 kg           | ≈ 10.81 kg          |
| 40        | ≈ 16.02 kg          | ≈ 19.22 kg          |
| 58        | ≈ 33.67 kg          | ≈ 40.40 kg          |
| 60        | ≈ 36.02 kg          | ≈ 43.23 kg          |
| 80        | ≈ 64.04 kg          | ≈ 76.85 kg          |

> 此限制僅在 `IniConfig.bD28MaxForceLimitByDiameter == true` 時啟用。

### SLK 類別預設 MinForce / MaxForce（ContactForce.cpp 建構子）

| 條件 | dMinForce | dMaxForce 公式 |
|------|-----------|---------------|
| diameter ≤ 30 | 0.5 | π × (d/100)² × 500 |
| 30 < diameter < 40 | 1.0 | 同上 |
| 40 ≤ diameter < 50 | 2.0 | 同上 |
| 50 ≤ diameter < 60 | 4.0 | 同上 |
| diameter = 402 (40x2) | 4.0 | 同上 |
| diameter ≥ 60 | 8.0 | 同上 |

SLK MaxForce 公式：`dMaxForce = 3.14 × (diameter/100)² × 500`
- 30mm → `3.14 × 0.09 × 500 = 141.3 kgf`
- 40mm → `3.14 × 0.16 × 500 = 251.2 kgf`

## Compliance Unit（scrbSLK Position）

| Position | 說明 | fComplianceUnit |
|----------|------|----------------|
| 2 | 1 Device / 1 Compliance | 1.0 |
| 3 | 2 Device / 1 Compliance | 0.5 |
| 4 | 4 Device / 1 Compliance | 0.25 |
| 5 | 2 Device / 4 Compliance | 2.0 |
| 6 | 8 Device / 1 Compliance | 0.125 |

## Index Press Type 最大力量上限

| INDEX_PRESS_TYPE | Max Limit (kg) |
|-----------------|----------------|
| e85KG (default) | 85 |
| e120KG | 120 |
| e160KG | 160 |
| e240KG | 240 |
| e260KG | 260 |
| e360KG | 360 |
| e400KG | 400 |
| e500KG | 500 |
| e640KG | 640 |
| e800KG | 800 |

> 預設 85 kg 機台：40mm 缸徑+Single Site 最多 55 kg，60mm 以上最多 85 kg。

## DutCount 與 HeadCount（TestMode 對應）

| TestMode | dDutCount | HeadCount（CalculateTotalAirForce 除數） |
|----------|-----------|----------------------------------------|
| SingleSite | 1 | 1 |
| DualSite / 2X2N / 2x1 | 2 | 2 |
| TriSite1X3 / 2X3N | 3 | 3 |
| 1X4 / 2X2 / 1X4_8 / 2X4N | 4 | 4 |
| 2X3_6 | 6 | 6 |
| 2X4_8 | 8 (12 if bOctal_12Kit) | 8 (12) |
| 2X5_10 | 10 | 10 |
| 2X6_12 | 12 | 12 |
| 2X8_16 | 16 (8 if half-site) | 16 (8) |
| 4X4_16 | 8 | 8 |
| 4X8N_32 / 4X8M_32 | 16 | 16 |

## 客戶特殊邏輯

- **ASE_KaohSiung + 2X2**：缸徑 30mm 時 dMinForce 被覆蓋為 1 kg
- **KYEC_LEE**：預設 SLK Type 為 28,40,58,56（非標準 30,40,60,56）
- **D27 (bD27UseSingleSite85kg)**：DualSite 僅開一個 Site 時，可使用整個 85kg 上限

## 設定檔路徑

| 檔案 | 說明 |
|------|------|
| `D:\HT9045\system\ContactInfo.ini` | SLK Type 列表、Load Rate、Contact Offset |
| `D:\HT9045\system\Gerneral.ini` | EP_MAXKPA、EP_MAXA、EP_MINMPA |
| cConfiguration (D04) | bD04MinForceByFile、各徑 Min Force |
| cConfiguration (D27) | bD27UseSingleSite85kg |
| cConfiguration (D28) | bD28MaxForceLimitByDiameter |

## EP 壓力差異與比例閥異常警報

### EP 壓力差異警報（WAR1605）

#### 核心判斷函式

| 函式 | 檔案 | 說明 |
|------|------|------|
| `ADAM_Alarm(iArm)` | adam6024.cpp L491 | PA 回讀值 vs 設定值差異超過 `iADAMRange` |
| `ADAM_Alarm_Kg(iAdd)` | adam6024.cpp L548 | 以 Kg 為單位比較，Range 自動分級 |
| `ADAM_DualAlarm(iType)` | adam6024.cpp L612 | Dual EP 通道差異檢查 |

#### ADAM_Alarm_Kg Range 自動分級

| 力量範圍 | 容許偏差 |
|----------|---------|
| ≤ 5 kg | ± 0.25 kg |
| 6–10 kg | ± 0.5 kg |
| 11–60 kg | ± 1.0 kg |
| 61–120 kg | ± 2.0 kg |

#### 呼叫時機

| 時機 | 函式 | 檔案行號 |
|------|------|---------|
| Index 每次測試前 | `IndexEveryTimeCheckEP()` | atester.cpp L8887 |
| Index 測試中記錄 | `CheckAndRecodrEP(iArm)` | atester.cpp L8995 |
| Contact Z1 取料 | `DoZ1PickFromShuttle` case 110 | cContact.cpp L2504 |
| Contact Z2 取料 | `DoZ2PickFromShuttle` case 110 | cContact.cpp L3475 |
| Auto Height | `Do_Z1/Z2_AutoGetHeight` case 2800 | cContact.cpp L6178/L8752 |

#### EP Leakage 補償
Auto Height 完成時若 `ADAM_Alarm()` 回 true → `bEPLeakage=true` → 高度少補 20 pulse（≈0.2mm）

#### 開關設定

| Config | 變數 | 功能 |
|--------|------|------|
| D24 | `bD24EnableEPCheckFuntion` | EP Check 主開關 |
| — | `bIndexEveryTimeCheckEP` | Index 每次測試前檢查 |
| D26 | `bD26EnableEPEncoderRange` | EP Encoder Range 差異檢查 |
| D26 edD26 | `iD26EPEncoderRange` | 差異容許範圍（kPa），預設 100 |
| D26_1 | `bD26EnableEPLog` | EP Log 記錄 |
| D26_2 | `bD26EnableEncodeShow` | EP Value 顯示 |
| D26_3 | `bD26_3EnableDualEPEncoderRange` | Dual EP 差異檢查 |

### 比例閥電壓異常警報（WAR16322/WAR16323）

| 項目 | 內容 |
|------|------|
| 函式 | `ADAM_ReturnValueCheck(bHome)` |
| 檔案 | adam6024.cpp L2200 |
| 正常電壓範圍 | DC 1.0V ~ 5.0V |
| 異常判定 | 回讀電壓 < 0.8V 或 > 5.2V |
| 累計門檻 | 連續異常 100 次（≈100秒）觸發警報 |
| 警報代碼 | Arm1: WAR16322, Arm2: WAR16323 |
| Home 模式 | `bHome=true` 時立即觸發，並阻止回原點 |

### 警報代碼總覽

| 代碼 | 訊息 | 觸發源 | 操作 |
|------|------|--------|------|
| WAR1605 | 請檢查EP是否漏氣 | EP壓力差異超範圍 | Retry/Skip |
| WAR16322 | EP Controller Arm1 電壓異常 | 比例閥回讀電壓異常 | Retry |
| WAR16323 | EP Controller Arm2 電壓異常 | 比例閥回讀電壓異常 | Retry |

## Contact Mode 列表

| 常數 | 值 | 說明 |
|------|---|------|
| CONTACT_NORMAL | 0 | 一般模式 |
| CONTACT_AUTO_GET_HEIGHT | 1 | 自動取高 |
| CONTACT_MANUAL_GET_HEIGHT | 2 | 手動取高 |
| CONTACT_TEST | 3 | Contact Test |
| AUTO_CONTACT_TEST | 4 | Auto Contact Test |
| STEP_CONTACT_TEST | 5 | Step by Step Contact Test |
| CONTACT_IN_SHUTTLE_CHECK | 6 | InShuttle 檢查 |
| CONTACT_OUT_SHUTTLE_CHECK | 7 | OutShuttle 檢查 |
| CONTACT_LoadCell_AUTO_GET_HEIGHT | 8 | Load Cell 自動取高 |
| CONTACT_DEVICE_MAP_CHECK | 9 | Device Map Check |
| CONTACT_DEVICE_LOOP_TEST | 10 | Device Loop Test |
| K_TEMP_INDEX_MOVE | 11 | K Temperature 模式 |

## 狀態機函式

| 函式 | 說明 |
|------|------|
| `DoTestContactFunction()` | Contact Mode 主排程 |
| `Do_Z1_AutoGetHeight()` | Z1 自動取高（~2700 行） |
| `Do_Z2_AutoGetHeight()` | Z2 自動取高 |
| `Do_ContactTest_32Site()` | 32-site Contact Test |
| `DoZ1PickFromShuttle()` | Z1 從 Shuttle 取 IC |
| `DoZ2PickFromShuttle()` | Z2 從 Shuttle 取 IC |
| `DoZPlaceToShuttle()` | 放 IC 回 Shuttle |
| `Do_AutoContactTest()` | 自動 Contact Test |
| `DoStepContactLoadDevice()` | Step Contact 上料 |
| `DoStepContactUnloadDevice()` | Step Contact 下料 |
| `Do_ROILearning()` | RTC ROI 學習 |
| `DoFullViewCheck()` | FullView 檢查 |
| `Do_LoadCellAutoHigh()` | Load Cell 自動取高 |
| `DoDeviceMapCheck()` | Device Map 檢查 |
| `DoContactDeviceLoopTest()` | Device Loop Test |
| `DoContactKTemperatureTest()` | K Temperature Test |
| `DoRTCAutoTuning()` | RTC Auto Tuning |
| `Do2DIDMapCheck()` | 2DID Map Check |
| `DoIndecxCHECkFunction()` | Index 檢查功能 |


---

## web 端實作（20260921）

`Setup.Contact.html` 的 SLK 捲軸與整組 Contact Force 顯示已接上，
主體是手寫的 `client\ht9045_contact_slk.js`（**不是**產生器產物）。

### 逐函式對照

| golden `cContact.cpp` | 行數 | web |
|---|---|---|
| `scrbSLKChange()` | 2085..2092 | `scrbSLKChange()` |
| `DutCount()` | 1970..2083 | `dutCount()` |
| `ShowArmAndDeviceForce()` | 1907..1930 | `showArmAndDeviceForce()` |
| `CalculateTotalAirForce()` | 18765..18981 | `calculateTotalAirForce()` |
| `GetMaxIndexForceLimit()` | 18983..19045 | `getMaxIndexForceLimit()` |
| `GetMinForce()` | 19047..19112 | `getMinForce()` |
| `CalcDeviceForce()` | 22852..22858 | `calcDeviceForce()` |
| `edAirForceChange()` | 2279..2284 | `edAirForceChange()`（**不含** `ADAM_WriteVoltage`） |
| `edPinCountChange()` | 1932..1938 | 同名 |
| `edForcePerPinNChange()` | 1939..1954 | 同名 |
| `edDieForcePerPinGChange()` | 1955..1974 | 同名（同時是 `edForcePerPinG` 的 OnChange） |
| `CountDieForceKg(bool)` | 18461..18484 | `countDieForceKg()` |
| `SaveSetupFile()` 的三行 | 14381 / 14389..14415 / 14418 | `collectSaveEdits()` |

### 資料來源（web 沒有全域變數，都要自己讀）

| golden 全域 | web 來源 |
|---|---|
| `MachineTypeChoice` | `Gerneral.ini [Version] Model` |
| `CUSTOMER_CODE` / `INDEX_PRESS_TYPE` / `EP_MAXKPA` / `INSTALL_DOUBLE_EP` | `Gerneral.ini [System]` |
| `IniConfig.bD27UseSingleSite85kg` | `config.ini [Index] UseSingleSite85kg` |
| `IniConfig.bD28MaxForceLimitByDiameter` | `config.ini [Index]` 同名 |
| `IniConfig.bD04MinForceByFile` ＋ `dD04MinForceByFile*` | `config.ini [Contact Force]` |
| `fContactForce->SLKClass[]` | `ContactInfo.ini [SLK Type] Type/Visible` ＋ `[Diameter_<d>.000mm] ContactOffset(_NS)` |
| `TestIF_File.iTestMode` / `iSiteMap` / `bOctal_12Kit` / `bQualSite2X2Shift` / `bNS7000kit` / `bNSKitPress` | 配方 `HandlerCondition.Data [Configuration]` |
| `DeviceForm_File.iHeadDeviceCT` / `dKitDiameter` | 配方 `Contact.Data [Mode]` |
| `LastSet.bUseTestSocket[arm][row][col]` | 執行期 tag `site.arm{1,2}.s{1..16}`（`WebBridgeTags.cpp:789`） |
| `CosFunction.bForecePerPinKGf` | 客戶碼查表（`CosFunction.cpp` 預設 false；791 / 959 / 919 / 865 為 true） |

### 四處刻意的界線

1. **`ADAM_WriteVoltage()` 沒搬** —— 那是真的會讓 EP 出力的動作，留在 C++ 端。
2. **`iCloseSiteModeFor2x8` 一律當 `e2x8Standard(0)`** —— 它是
   `ainarm9045_2x8_8.cpp` 跑料途中才改的執行期全域，不在任何 ini 也沒有 tag。
3. **`LastSet.bUseTestSocket` 的 tag 接不上時一律當 `true`** ——
   猜 `false` 會讓畫面顯示一個比實際允許值大一倍的空壓。
4. **`IniConfig.iEP_Min_KG` 沒有回寫** —— 那是給 `adam6024` 用的執行期副作用。

另外 `EP_Install==5`（雙臂各一組 SLK）與 `CC_ASE_SG` 的 `"80_Hi"` 沒有進 `SLKClass`，
兩者都會讓 `SLKClass` 的長度與 `rgKitDiameter` 的選項對不齊。

### 一個容易踩的點：`rgKitDiameter` 的選項不是 dfm 那三顆

V910 的 `CosFunction.bUseDynamicKitDiameter` 在 `InitialCosFunction():4297` 給預設
`true`，**全樹沒有任何一處設回 false**，所以 golden 一定會用
`ContactInfo.ini [SLK Type] Type` 重建選項，dfm 裡的 `30/40/60` 永遠看不到。
這台開發機的 `Type` 是 `30,40,60,56,80`（五顆）。web 端照做了重建；
不重建的話配方選 80mm 時 `ItemIndex` 落空 → `iTag=-1` → 最小力量會用 30mm 的
1.5kg 而不是 80mm 的 15kg，而且**不會報錯**。

### 存檔的公分制

`[Mode] Kit Diameter` 存的是**公分**（`3.0` = 30mm），`"40x2"` 存 `40.2`。
golden `SaveSetupFile` 的前三個 `ItemIndex` 是**寫死**的 `0/1/2 → 3.0/4.0/6.0`
（假設頭三顆一定是 30/40/60），第四顆起才走 `atof(items[idx])/10.0`。
