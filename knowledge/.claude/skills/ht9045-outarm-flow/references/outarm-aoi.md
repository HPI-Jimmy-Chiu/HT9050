# OutArm — AOI（自動光學檢測）詳細參考

> 主 Skill 見 [ht9045-outarm-flow/SKILL.md](../SKILL.md)。
> Source: `fAOI.cpp`, `fAOI.h`, `aoutarm9045.cpp`

## 1. AOI 在 OutArm 流程中的位置

AOI 位於 OutArm 的 **「Pick from Shuttle 之後、Place to Auto/Fix 之前」**。
由 `DoOutArmAdditionalFunction()` 統一調度（與 Rotator、Fix AI CCD 並列）。

```
OutArm 從 Shuttle 取料完成 (case 3000)
  → MoveOutArmToAutoSafe()
  → CheekNeedToDoOutArmAdditionalFunction()     ← 檢查是否需要附加功能
    → bDoAOI = true（若 AOI 啟用）
  → case 7000: DoOutArmAdditionalFunction()
    → case 20000: DoAOIFunction()                ← AOI 核心分派
      → AOI 完成後回 case 100，繼續判斷其他附加功能
  → case 3010: SearchTrayToPlace_9045()          ← 根據 AOI 結果決定放到哪個 Tray
  → case 3310: DoOutArmPlaceToAuto_9045()        ← 實際放料
```

### 觸發條件

`PreSetOutAdditionalFlag()`（`aoutarm9045.cpp`）於 Pick 完成後設定：

```cpp
OutArmSuck.bAlreadyAOI = (tAOISetup.bEnabledAOI ||
    (USE_Scanner_AOI_Inspection==(int)eBtnAOI_BottomInstall && ScannerAOIIF.iEnableScannerMode!=0) ||
    (USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall && FrmAOI->ttbInsp->iEnable==1)
) ? false : true;
```

`CheekNeedToDoOutArmAdditionalFunction()`（`aoutarm9045.cpp`）中檢查：

```cpp
if(OutArmSuck.bAlreadyAOI==false) {
    if(tAOISetup.bEnabledAOI ||
       (USE_Scanner_AOI_Inspection==(int)eBtnAOI_Uninstall && ScannerAOIIF.iEnableScannerMode!=0) ||
       (USE_Top_Scanner_AOI_Inspection==(int)eBtnAOI_BottomInstall && ScannerAOIIF.iEnableTopScannerMode!=0) ||
       FrmAOI->RunTopBottomInspect()==true)
    {
        bDoAOI=true;
    }
}
```

## 2. 四種 AOI 模式

| # | 模式名稱 | 啟用條件（Gerneral.ini + AOI.Data） | 通訊方式 | DoAOIFunction 內 case |
|---|---------|-----------------------------------|---------|----------------------|
| 1 | **Vitrox BGA View** | `USE_AOI_Inspection==1` + `tAOISetup.tBGAView.bEnabled` | Vitrox SDK（IO 觸發） | case 1000 |
| 2 | **Vitrox PAD View** | `USE_AOI_Inspection==1` + `tAOISetup.tPADView.bEnabled` | 同上 | case 2000 |
| 3 | **Scanner AOI（Bottom）** | `USE_Scanner_AOI_Inspection==1` + `ScannerAOIIF.iEnableScannerMode!=0` | RS-232 串口 | case 6000 |
| 4 | **Top & Bottom Inspect** | `USE_Scanner_AOI_Inspection==2` + `ttbInsp->iEnable==1` | TCP/IP 網路 | case 7000 |

另有 **Top Scanner AOI**（`USE_Top_Scanner_AOI_Inspection==1`，case 6500）及 **Fix AI CCD**（不在 DoAOIFunction 內，而是 DoOutArmAdditionalFunction case 30000）。

### Gerneral.ini 硬體開關

| INI Section/Key | 值 | 說明 | 讀取位置 |
|-----------------|---|------|---------|
| `[System] AOI` | 0/1 | Vitrox BGA/PAD/Top View AOI | `database.cpp` |
| `[System] Scanner_AOI` | 0=未安裝, 1=Bottom, 2=Top&Bottom | Scanner AOI 類型（enum `eBtnAOI_*`） | `database.cpp` |
| `[System] Top_Scanner_AOI` | 0/1 | Top Scanner AOI（TFAMD 專用） | `database.cpp` |

### AOI 模式 enum

```cpp
// MachineType.h
enum {
    eBtnAOI_Uninstall        = 0,  // 未安裝
    eBtnAOI_BottomInstall    = 1,  // Bottom（Scanner AOI）
    eBtnAOI_TopBottomInstall = 2,  // Top & Bottom Inspect
    eBtnAOI_Total            = 3
};
```

## 3. DoAOIFunction() — 核心分派

> Source: `fAOI.cpp` L~2707 | Task: `iAOITask`

```
case 1: 判斷啟用哪種 AOI
  ├─ tBGAView.bEnabled            → case 1000: InitBGAViewFunction() → DoBGAViewFunction()
  ├─ tPADView.bEnabled            → case 2000: InitDoPADViewFunction() → DoPADViewFunction()
  ├─ Scanner_AOI==eBtnAOI_Bottom  → case 6000: InitialScanAOITask() → DoScanAOIFunction()
  ├─ Top_Scanner_AOI              → case 6500: InitialTopScanAOITask() → DoTopScanAOIFunction()
  ├─ TopBottomInspect             → case 7000: DoTopBtmInspFunc(true/false)
  └─ 都沒啟用                      → case 4000: bResult=true

case 4000: 完成
  → bPickSH1Flag=false, bPickSH2Flag=false
  → return true
```

**注意**：BGA View 完成後會接著檢查 PAD View 是否也啟用（串接），Scanner AOI 完成後接著 Top Scanner AOI。

## 4. Scanner AOI — 底部掃描（最常見模式）

### 4.1 DoScanAOIFunction()

> Source: `fAOI.cpp` L~2089 | Task: `iScanAOITask`

逐顆遍歷 `OutArmSuck.Item[i][j]`，對每顆非 NULL_IC 且尚未 AOI 的 IC 執行檢測：

```
case 1:   bRunAOI=false
case 100: 搜尋下一顆需要 AOI 的 IC（含抽檢邏輯）
  → iScanAOIIntervalCounter++ 判斷是否到達抽檢間隔
  → 找到 → case 1000
  → 全部完成 → return true
case 1000: DoMoveXY_ScannerAOI(iRow, iCol)  ← OutArm 移到 Scanner 位置
  → Dummy IC 跳過不檢
  → case 2000
case 2000: DoScanAOIFunction_Inspection(iRow, iCol)  ← 實際檢測
case 3000: MoveOutArmToAutoSafe()
case 4000: 檢查是否還有未檢測的 IC → 有:回 case 100 → 無:告警判斷
```

### 4.2 DoScanAOIFunction_Inspection() — 單顆檢測

> Source: `fAOI.cpp` L~1806 | Task: `iScannerAOIInspectionTask`

```
case 1:    StartDelay 延遲等待
case 1000: 延遲完成
case 2000: TriggerAOISystem(true, true)    ← RS232 發送觸發命令
           → LGA Mode: 多點拍攝循環（case 2100 → case 3700 → 回 2000）
           → 一般 Mode: 等回覆（case 2500）
case 2500: 等待 bTriggerAOI==false（AOI 回覆）或 Timeout
case 3000: 解析結果 iAOIResult[0] != 1 → bAOI_Fail_Unit=true
case 3500: Timeout → WAR0883（Retry/Skip）
case 4000: ★ Fail 時修改 OutArmSuck.iWhichAuto = GetAOIFailBin() ← 改分到 Fix
           ★ Pass 時 bAOIPassFail = true（維持原 bin）
case 5000: 連續 Fail 告警統計
           → DoContinuousFailBySiteScanAOI()
           → DoContinuousFailByArmScanAOI()
           → return true
```

### 4.3 RS232 通訊協定

**觸發命令** `TriggerAOISystem(bSite1, bSite2, iNum)`：
- 格式：`{Length}{bSite1}{bSite2}BDAOI{Checksum}`
- 範例：`"\x07" + "11BDAOI" + checksum_byte`
- 設定 `bTriggerAOI=true`，透過 `AOIComm` (SPComm) 發送

**接收結果** `GetScannerResult(sData)`：
- 解析 `"site X 1"` 格式，取 `site` 後第 6 字元
- `iAOIResult[0] == 1` → Pass
- `iAOIResult[0] != 1` → Fail

**RS232 參數**（AOI.Data `[RS232]` section）：
- `Device` — COM Port
- `Baud Rate` — 預設 57600
- `Byte Size` — 預設 8
- `Stop Bit` — 預設 1
- `Parity` — 預設 None

### 4.4 抽檢邏輯（Interval Counter）

- `ScannerAOIIF.iIntervalCounter`（AOI.Data `iIntervalCounter`）
- **SingleSite**：每 `iIntervalCounter+1` 顆檢一次
- **DualSite（50% 抽檢）**：計數 1,3,6,8 時檢測（SHT mode 0 + interval==1 特殊邏輯）
- **其他模式**：每 `iIntervalCounter+1` 顆且 Site 不重複檢

### 4.5 OutArm 移動到 Scanner 位置

`DoMoveXY_ScannerAOI(iRow, iCol)`（`fAOI.cpp` L~1685）：
- X 位置：`Prod.iScannerAOI_X` + Pitch 修正
- Y 位置：`Prod.iScannerAOI_Y` + Row 修正
- Z 位置：`Prod.OutArm_ScannerAOI_Place[iRow][iCol]`
- LGA Mode 大顆 IC：多點拍攝掃描（X/Y 分割矩陣）
- 使用 `OutArmContinuousMove_9045()` 執行連續移動

## 5. Top & Bottom Inspect（TCP/IP 模式）

> Source: `fAOI.cpp` L~5506 (TTopBottomInspect::DoTopBtmInspFunc) | Jimmychiu 20240322

### 5.1 啟用條件

```cpp
bool TFrmAOI::RunTopBottomInspect() {
    return USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall && ttbInsp->iEnable==1;
}
```

### 5.2 流程概要

```
DoTopBtmInspFunc(true)   ← 初始化
DoTopBtmInspFunc(false)  ← 狀態機推進

case 1:   CommClose, 設定 IC Size, 準備馬達
case 100: DoMoveXY2SafePos + DoMoveFixedSeatAndCCD2Ready
case 200: DoCommuncation()  ← TCP 連線
          → SetSuckActive(eAOI_SCANAOI)
case 500: GetNeedActSuck → DoOutarm2AOI_PickPlaceIC（放到 AOI 固定座）
          → Bottom mode: DoMoveFixedSeatXYandClamp
          → Top&Bottom mode: 取放 IC
case 1900: DoMoveZ_TopView(Prod.iScannerAOI_Z)
case 2000: DoTopBtmInspFunc_Inspection(true/false)  ← 實際拍照檢測
           → Bottom: 完成後直接結束
           → Top&Bottom: Bottom 拍完 → 旋轉 180° → Top 再拍
case 3000: DoMoveFixedSeatXY2RotatePos → 取回 IC
case 3500: MoveOutArmToAutoSafe → 回 case 200 找下一顆
case 9900-9999: 完成
```

### 5.3 檢測子流程 DoTopBtmInspFunc_Inspection()

```
case 1:    DoAutoRun
case 50:   DoCamaChange (切換相機)
case 100:  DoCamaDown + DoClamp
case 140:  DoResetFlow → DoTestLighting → 拍照
case 200:  DoPhotosCenterpos (<=65mm) 或 DoPhotos4pos (>65mm)
case 250:  DoInquireResult → 取得檢測結果
case 303:  DoRecordResult → 記錄結果
           → Bottom mode → 完成
           → Top&Bottom mode → case 350-600（旋轉 180° 拍 Top 面）
```

### 5.4 Retry 機制

- 外層 `DoTopBtmInspFunc` retry 3 次（Eastsun 20260304）
- 內層 `DoTopBtmInspFunc_Inspection` retry 3 次
- AOI Photos Timeout retry 2 次
- AOI Disconnect Error retry 2 次 → WAR09114

### 5.5 專用馬達

| 馬達 | 用途 |
|------|------|
| `MTopAOIArmX` | AOI 固定座 X 軸 |
| `MTopAOIArmY` | AOI 固定座 Y 軸 |
| `MTopAOICCDZ` | CCD Z 軸 |
| `MTopAOIArmR` | 旋轉軸（0°/180°切換 Top/Bottom） |

## 6. Vitrox BGA/PAD View（傳統 AOI）

### 6.1 DoBGAViewFunction()

> Source: `fAOI.cpp` L~1523 | Task: `iBGAViewTask`

逐顆檢測：
```
case 100:  找未檢測 IC（bVitroxBGAViewUse 篩選）
case 1000: DoMoveXY_BGAView → 移到 BGA 檢查位置
           → Manual Step 暫停（if 權限允許）
case 2000: DoBGAViewFunction_Inspection → 實際檢測
case 3000: MoveOutArmToAutoSafe
case 4000: 檢查是否還有 → 有:回 case 100 → 無:告警 → 完成
```

### 6.2 DoPADViewFunction()

與 BGA View 結構相似，使用 `iPADViewTask` / `DoPADViewFunction_Inspection()`。

### 6.3 Vitrox Fail Bin 分配

```cpp
// BGA 或 PAD View fail 時：
if(tAOISetup.tTesterFailBin.bEnabled) {
    // 測試 Fail 的 IC → tBGAPADView.iTestFailFailBin
    // 測試 Pass 的 IC → tBGAPADView.iTestPassFailBin
} else {
    // 統一用 tPADView.iFailBin 或 tBGAView.iFailBin
}
OutArmSuck.iWhichAuto[i][j] = iBin;
```

特殊選項 `tAOISetup.tAOINoSort.bEnabled`：AOI Fail 時不改分 Bin（維持原測試結果）。

## 7. AOI Fail → Bin 分配邏輯

### 7.1 GetAOIFailBin(iRow, iCol)

> Source: `fAOI.cpp` L~2849

```
iAOIFailBinType == 0（舊規則）:
  → iBin = ScannerIfError + 4

iAOIFailBinType == 1（新規則 — RogerYang 20251120）:
  → IC 測試 Pass：iBin = ScannerIfErrorAndTestPass + 4
  → IC 測試 Fail：iBin = ScannerIfError + 4
```

### 7.2 Bin 編號對照

| iBin | Tray |
|------|------|
| 0 | Auto1 |
| 1 | Auto2 |
| 2 | Auto3 |
| 3 | Fix1 |
| 4 | Fix2 |
| 5 | Fix3 |
| 6 | Fix4 |
| 7 | Fix5 |
| 8 | Fix6 |

### 7.3 Fix 上下層映射

```cpp
ScannerIfError==2 → iBin=5(Fix3) or 7(Fix5)  // 視 bTrayUpDownSet[eFix2]
ScannerIfError==3 → iBin=6(Fix4) or 8(Fix6)  // 視 bTrayUpDownSet[eFix3]
```

### 7.4 修改出料 Tray 的方式

AOI Fail 時核心操作：
```cpp
OutArmSuck.iWhichAuto[iRow][iCol] = iBin;  // 將 IC 分配到 Fail Tray
bAOIPassFail[0][iRow][iCol] = false;        // 標記為 AOI Fail
```

後續 `SearchTrayToPlace_9045()` 會讀取 `OutArmSuck.iWhichAuto` 決定放到哪個 Tray。

## 8. AOI.Data 工作檔設定

> 路徑：`{DataPath}{RecipeName}\AOI.Data`（Recipe 層級，隨工作檔切換）

### [SETTING] section — Scanner AOI 相關

| Key | 預設值 | 說明 |
|-----|-------|------|
| `iEnableScannerMode` | 0 | Scanner AOI On/Off |
| `iIntervalCounter` | 0 | 抽檢間隔（0=每顆檢） |
| `iRetryCounter` | 0 | Fail 重試次數 |
| `iAOIFailBinType` | 0 | 0=舊規則, 1=依測試結果分 bin |
| `iScanAOIFialBin` | 0 | AOI Fail → 放到 Fix 幾（0=Fix1, 1=Fix2...） |
| `iAOIFialAndTestPass` | 0 | 新規則：AOI Fail 但測試 Pass 時的 bin |
| `ScannerReadTimeout` | 100 | 等 AOI 回覆 Timeout（ms） |
| `StartDelayTimeScanAOIView` | 10 | 觸發前延遲（ms） |
| `TimeOutScanAOIView` | 3 | Timeout（秒） |
| `iBallDamageType` | 0 | 錫球損傷判定類型 |
| `bEnabledScanAOIUseLGAMode` | false | LGA 大顆 IC 多點拍攝 |

### [SETTING] section — Vitrox 相關

| Key | 預設值 | 說明 |
|-----|-------|------|
| `EnabledTopView` | false | Top View 開關 |
| `EnabledPADView` | false | PAD View 開關 |
| `EnabledBGAView` | false | BGA View 開關 |
| `FailBinTopView / PADView / BGAView` | 15 | Fail Bin 編號 |
| `TimeOutTopView / PADView / BGAView` | 3 | Timeout（秒） |
| `EnabledTesterFailBin` | false | 依測試 Pass/Fail 分 bin |
| `EnabledAOINoSort` | false | AOI Fail 不改 bin |
| `EnabledContinueAlarm` | false | 連續 Fail 告警 |

### [SETTING] section — 連續 Fail 告警

| Key | 預設值 | 說明 |
|-----|-------|------|
| `bEnabledScanAOIBySiteAlarm` | false | By Site 告警開關 |
| `bEnabledScanAOIByArmAlarm` | false | By Arm 告警開關 |
| `iScanAOIAlarmCountBySite` | 3 | Site 連續 Fail 閾值 |
| `iScanAOIAlarmCountByArm` | 3 | Arm 連續 Fail 閾值 |
| `bEnabledScanAOIUnUseFailBin` | false | AOI Fail 不丟 Fail Bin（TF-AMD 用） |

### [SETTING] section — Ball Damage 計數器

| Key | 預設值 | 說明 |
|-----|-------|------|
| `bBDTotalFunction` | false | 累計總數告警 |
| `bBDTotalContiFunction` | false | 累計連續告警 |
| `bBDSiteFunction` | false | 各 Site 告警 |
| `bBDSiteContiFunction` | false | 各 Site 連續告警 |
| `bDBAlramAutoResetCount` | false | 告警後自動歸零 |

### [RS232] section — Scanner AOI 串口

| Key | 預設值 | 說明 |
|-----|-------|------|
| `Device` | "" | COM Port |
| `Baud Rate` | "57600" | 鮑率 |
| `Byte Size` | "8" | 資料位元 |
| `Stop Bit` | "1" | 停止位元 |
| `Parity` | "None" | 校驗 |

### [DutOnOff_BGAView] / [DutOnOff_PADView] sections

- 依 Site 開關 Vitrox 檢測（`Dut A-1` / `Dut A-2` 等）

## 9. 連續 Fail 告警機制

### 9.1 告警類型

| 告警 | 啟用設定 | 閾值 | 函式 |
|------|---------|------|------|
| By Site 連續 | `bEnabledScanAOIBySiteAlarm` | `iScanAOIAlarmCountBySite` | `DoContinuousFailBySiteScanAOI()` |
| By Arm 連續 | `bEnabledScanAOIByArmAlarm` | `iScanAOIAlarmCountByArm` | `DoContinuousFailByArmScanAOI()` |
| Ball Damage 累計 | `bBDTotalFunction` 等 4 種 | `iBDTotalCounter` 等 | `CheckBallDamageCounter()` |

### 9.2 Arm Side 判定

```cpp
if(bPickSH1Flag==true && bPickSH2Flag==false)   → ArmSide=0（SH1）
if(bPickSH1Flag==false && bPickSH2Flag==true)    → ArmSide=1（SH2）
```

## 10. 相關全域變數

| 變數 | 型別 | 用途 |
|------|------|------|
| `tAOISetup` | `TAOISetup` | Vitrox AOI 設定（`cprod.h`/`cprod.cpp`） |
| `ScannerAOIIF` | `SYSTEM_SCANNER_AOI_IF` | Scanner AOI 設定（`cprod.h`/`cprod.cpp`） |
| `iAOIResult[2]` | `int[]` | AOI 回傳結果（1=Pass） |
| `bTriggerAOI` | `bool` | AOI 觸發等待旗標 |
| `bRunAOI` | `bool` | 正在執行 AOI |
| `bAOI_Fail_Unit` | `bool` | 當前 IC AOI Fail |
| `iAOI_Fail_Count` | `int` | Fail 累計計數 |
| `bAOIPassFail[0][][]` | `bool[][]` | 每顆 IC 的 AOI Pass/Fail 結果 |
| `OutArmSuck.iAOIStation[][]` | `int[][]` | AOI 檢測站標記（eAOI_Top/eAOI_BGA/eAOI_SCANAOI） |
| `OutArmSuck.iWhichAuto[][]` | `int[][]` | 出料目標 Tray 編號 |
| `bPickSH1Flag / bPickSH2Flag` | `bool` | 標記 IC 來自 SH1 或 SH2 |
| `USE_AOI_Inspection` | `int` | Gerneral.ini `[System] AOI` |
| `USE_Scanner_AOI_Inspection` | `int` | Gerneral.ini `[System] Scanner_AOI` |
| `USE_Top_Scanner_AOI_Inspection` | `int` | Gerneral.ini `[System] Top_Scanner_AOI` |

## 11. AOI 相關 Alarm Code

| WAR Code | 說明 |
|----------|------|
| `WAR0883` | Scanner AOI Timeout |
| `WAR0887` | Top Scanner AOI Timeout |
| `WAR09114` | Top & Bottom Inspect Disconnect Error |

## 12. 相關函式索引

| 函式 | 檔案 | 說明 |
|------|------|------|
| `DoAOIFunction()` | `fAOI.cpp` L~2707 | AOI 主分派 |
| `InitAOIFunction()` | `fAOI.cpp` L~2693 | 初始化 Task |
| `DoScanAOIFunction()` | `fAOI.cpp` L~2089 | Scanner AOI 逐顆處理 |
| `DoScanAOIFunction_Inspection()` | `fAOI.cpp` L~1806 | Scanner AOI 單顆檢測 |
| `DoTopScanAOIFunction()` | `fAOI.cpp` L~2483 | Top Scanner AOI |
| `DoTopScanAOIFunction_Inspection()` | `fAOI.cpp` L~2000 | Top Scanner 單顆檢測 |
| `DoBGAViewFunction()` | `fAOI.cpp` L~1523 | Vitrox BGA View |
| `DoPADViewFunction()` | `fAOI.cpp` L~915 | Vitrox PAD View |
| `DoTopViewFunction()` | `fAOI.cpp` L~490 | Vitrox Top View |
| `DoMoveXY_ScannerAOI()` | `fAOI.cpp` L~1685 | 移動 OutArm 到 Scanner 位置 |
| `TriggerAOISystem()` | `fAOI.cpp` L~4067 | RS232 發送觸發命令 |
| `GetScannerResult()` | `fAOI.cpp` L~3938 | 解析 AOI 回覆 |
| `GetAOIFailBin()` | `fAOI.cpp` L~2849 | Fail Bin 映射 |
| `PreSetOutAdditionalFlag()` | `aoutarm9045.cpp` L~2314 | 設定 bAlreadyAOI |
| `CheekNeedToDoOutArmAdditionalFunction()` | `aoutarm9045.cpp` L~2138 | 檢查 AOI 是否需要執行 |
| `DoOutArmAdditionalFunction()` | `aoutarm9045.cpp` L~2241 | 附加功能（含 AOI）調度 |
| `fAOI_ReadFile()` | `fAOI.cpp` L~3140 | 載入 AOI.Data |
| `TFrmAOI::RunTopBottomInspect()` | `fAOI.cpp` L~4381 | TopBottom 啟用判定 |
| `TTopBottomInspect::DoTopBtmInspFunc()` | `fAOI.cpp` L~5506 | TopBottom 主流程 |
| `TTopBottomInspect::DoTopBtmInspFunc_Inspection()` | `fAOI.cpp` L~5718 | TopBottom 檢測子流程 |
| `bCheckAOIFailBinUse()` | `fAOI.cpp` L~7999 | AOI Fail Bin 即時良率檢查 |
