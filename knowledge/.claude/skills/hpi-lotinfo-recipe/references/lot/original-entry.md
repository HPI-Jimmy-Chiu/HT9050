> 保存來源：`.claude/skills/ht9045-lotinfo-flow/SKILL.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../common.md)。

<!-- preserved-content:start -->

# ht9045-lotinfo-flow — LotInfo 批次管理流程知識庫

## 適用範圍

本 Skill 適用於以下查詢：
- `TfLotInfo`、`uLotInfo.cpp`、Lot Start / Lot End 流程
- `SetLotStart`、`SetLotID`、`SetLotInfo`、`sbSECSLotStartClick`、`sbSECSLotEndClick`
- Recipe 下載 / 上傳（`DownloadFromServer`、`UploadToServer`）
- 批次 ID 持久化、SECS/GEM 事件、ATC 溫控、FTP 自動化
- OLP 遠端 `PP_DL_REQUEST`、`LotInfo_REQUEST` 指令流

---

## 1. 模組概述

| 項目 | 值 |
|------|-----|
| 主類別 | `TfLotInfo` |
| 主檔案 | `uLotInfo.cpp` (V899: ~11,886 行) |
| 表單檔 | `uLotInfo.dfm` |
| V899 路徑 | `d:\HT9045\HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422\uLotInfo.cpp` |
| 持久化路徑 | `AuthPath + "config.ini"` → `[Lot Info]` |

### 主要職能

```
TfLotInfo 職能樹
├── Lot 生命週期管理（Start / End）
├── Config 持久化（LotID、OP ID、Run Mode 等）
├── Recipe 下載（FTP / RMS / ERMS / SPIL）
├── Recipe 上傳（Upload → 打包 zip → 複製到 Server）
├── SECS/GEM 整合（DoLotStart / DoLotEnd / SwitchSetupFile）
├── ATC 溫控顯示（Hontech / New ATC / ATC7.0 / WinWay）
├── Barcode / OCR 整合
├── ESD 監控（18 Probes / 36 Decay Points）
├── Yield Monitor（SIGURD / TERAPOWER variant）
└── FTP Automation（自動上傳生產紀錄）
```

---

## 2. 關鍵函式速查

| 函式 | V899 行號 | 說明 |
|------|-----------|------|
| `SetLotStart(sFunc, bReadFromFile)` | L1438 | Lot Start 核心邏輯 |
| `SetLotID(ID, bReadFromFile)` | ~L6025 附近 | 設定 Lot ID → config.ini |
| `SetLotInfo(...)` | ~L1246 | 設定 OP ID、Run Mode 等 |
| `SetLotInfo_CYUEAN(iLotStatus, bReadFromFile)` | ~L1273 | CyuEan 專用 CSV log |
| `SetStartTime()` | ~L1363 | 設定批次開始時間 |
| `SetEndTime()` | ~L1380 | 設定批次結束時間 |
| `sbSECSLotStartClick` | ~L6025 | UI 手動 Lot Start 按鈕 |
| `sbSECSLotEndClick` | L876 | UI 手動 Lot End 按鈕 |
| `DownloadFromServer(sDLFileName, bFromFTP)` | ~L3423 | Recipe 下載（主流程） |
| `DownloadFromServer_TSMC(...)` | — | TSMC 專用下載 |
| `DownloadFromERMS(...)` | — | ERMS 下載 |
| `UploadToServer()` | — | Recipe 上傳 |
| `DoBackupSetupFile(DataPath, sDLFileName)` | ~L2832 | 下載前備份本機參數 |
| `DoOverWriteSetupFile(DataPath, sDLFileName)` | ~L3066 | 下載後還原本機參數 |
| `btDownloadClick` | — | UI 手動下載按鈕 |
| `btUploadClick` | — | UI 手動上傳按鈕 |
| `ShowATCThermo()` | — | ATC 溫度顯示（Timer 驅動） |
| `Timer1Timer` | ~L4362 | RTC 檔案切換定時器 |
| `Timer2Timer` | — | 分頁顯示更新定時器 |
| `RefreshYieldMonitor()` | ~L10974 | Yield Monitor 刷新 |
| `CheckingCheckList / GenerateCheckList` | ~L10123 | CheckList 驗證 |

---

## 3. Lot Start 流程

### 3.1 呼叫來源

```
sbSECSLotStartClick  ← 手動按鈕
auto9045.cpp::SetLotInfo()  ← OLP LotInfo_REQUEST 指令
Command.cpp (L9405, L14190)  ← Command 指令
BarcodeXML.cpp        ← 條碼觸發
SCK_ART.cpp           ← ART 流程
HANA_ART.cpp          ← HANA ART 流程
```

### 3.2 SetLotStart 狀態機（V899 L1438）

```
SetLotStart(sFunc, bReadFromFile)
│
├─ bReadFromFile=true（還原模式）
│   ├─ SetLotID("", true)     → 從 config.ini 讀取 Lot ID
│   ├─ ReadWriteLotInfo(true)  → 讀取 OP ID、Run Mode 等
│   └─ slEventLog->SetLotData(LotNo, StartTime, LogName)
│
└─ bReadFromFile=false（新 Lot 啟動模式）
    ├─ SetLotID(edtSysLotID->Text)  → 寫入 LotID + LotStartTime
    ├─ 設定 lbledtStarTime（若為空）
    ├─ ArmDataLot[i]->ClearALLCT()            ← V899 新增（By Lot Summary）
    ├─ slEventLog->SetLotData(LotNo, StartTime)
    ├─ RecordProcess("Lot Start, Lot ID:..., OP ID:..., Run Mode:...")
    ├─ ReadWriteLotInfo(false)                 → 寫入所有 Lot Info 到 config.ini
    ├─ SetTesterStartTimeByB03()               ← PTI ART 用
    ├─ CosFunction.bRunModeFollowLotInfo → 切換 FT/RT/EQC Run Mode
    ├─ VTEST 特殊處理（CheckVTENGmode、清 ART 計數）
    ├─ EventReport(SECS_EVENT.DoLotStart)   ← 非 TSMC 且 RunInfo.bLotStart==false
    ├─ SetLotComponents(false)              ← 鎖定/開放 UI 元件
    ├─ TCP_IP_MODE → 送 LOTNUMBER/LOTSTART/OPERATORID 給 Tester
    └─ 客製 log（OEE、SLT Summary、2DID sorting 等）
```

### 3.3 Lot Start 前置條件（sbSECSLotStartClick）

1. 特殊字元驗證：Lot ID、OP ID 不可含 `\/:*?"<>|`
2. 2DID Sorting：若啟用 `bSortingBy2DList`，需先下載 sorting list（FTP 或 Net Drive）
3. 其他客戶驗證（ASE-CL、KYEC、VTEST 等各有自訂邏輯）
4. **V906 移植樹（C++，20261002 St01 E-020 LI-1）**：整支照翻成 `bool W906_LotInfo_SECSLotStart(std::string* whyNot, std::vector<std::string>* skipped)`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\LotInfo_SECSLotStart.cpp`，golden V912 `uLotInfo.cpp:7501-8583`；906 樹的行號是 7382-8416）。wb_serve 的 `lot.start` 那一臂（筆電的）先設 Lot ID／OP ID 再呼叫它；true＝跑到 `SetLotStart`（是否真的開批仍看 `RunInfo.bLotStart`）、false＝golden 提早 return，whyNot＝golden 訊息原文。13 個 GATE（W906-E020-LI1-1..13：DoPassword、TfBarCode 的 2D 清單／白名單成員、LEADYO 換工作檔、RunModeRW、MES 下載、OEE 會拒絕；rgSort2DID、FTP、CSV 比對重置、PANTHER、VTEST 換 KIT、AMD 只略過並記 log）列在檔頭。特殊字元另擋 `!`（golden 字元類別有、訊息沒寫）；SCC 5~30 字只在出貨組態（`#ifndef SOFT_SIMULTE`）。ctest `E020_LotInfoSECSLotStart`。

---

## 4. Lot End 流程

### 4.1 sbSECSLotEndClick（L876）

```
sbSECSLotEndClick
│
├─ 前置檢查
│   ├─ SystemStart==true → 提示「正在運作，無法結束批次」
│   ├─ CheckCanChangeRealDummy()==false → 拒絕
│   └─ iTestHeadMotorTask==1 → 拒絕（Index 正在下壓中）
│
├─ SECS/GEM 事件
│   ├─ 2DID Sorting → EventReport(DoVisualSortLotEnd)
│   └─ 其他 → EventReport(DoLotEnd)
│
├─ FTP Upload（PTI 特定）
│   └─ 上傳 JamAlarmLog 檔案
│
├─ SaveJamRateByLot()    ← 儲存卡料率
├─ SetEndTime()           ← 清除 UI Lot ID + 記錄結束時間
└─ SetLotComponents(true) ← 開放 UI 輸入
```

---

## 5. Config 持久化架構

**所有 Lot Info 均持久化到：`AuthPath + "config.ini"` → `[Lot Info]`**

| INI Key | 對應欄位 | 寫入時機 |
|---------|---------|---------|
| `Lot ID` | `edtSysLotID->Text` | `SetLotID(..., false)` |
| `LotStartTime` | `RunInfo.LotStartTime` | `SetLotID(..., false)` |
| `OP ID` | `edtSysOperatorID->Text` | `SetLotInfo(..., false)` |
| `Run Mode` | `cbRunMode->Text` | `SetLotInfo(..., false)` |
| `Customer Lot ID` | `edCustomerLotId->Text` | `SetLotInfo(..., false)` |
| `Station` | `coStation->Text` | `SetLotInfo(..., false)` |
| `Station Number` | `edStationNum->Text` | `SetLotInfo(..., false)` |
| `Customer Lot ID` | `RunInfo.LotStartTime`（時間戳） | `SetLotID`（初始化為時間） |

**程式碼關鍵賦值：**
```cpp
RunInfo.LotNo = edtSysLotID->Text;  // 在 SetLotID 中設定
```

---

## 6. Recipe 下載流程概要

詳細流程見 [references/recipe-download.md](references/recipe-download.md)

### 觸發方式
| 來源 | 函式 | 特點 |
|------|------|------|
| UI 手動 | `btDownloadClick` | 需 Device Name + 溫度驗證 |
| OLP 遠端 | `PP_DL_REQUEST` in automation.cpp | Server 遠端觸發 |
| SECS/GEM | `fFTPClient->bControlBySECSGEM` | 無彈窗 |
| GPIB | `fFTPClient->bControlByGPIB` | 無彈窗 |

### 下載引擎
| 類型 | 函式 | 條件 |
|------|------|------|
| 標準 RMS | `DownloadFromServer()` | `IniConfig.bEnableRms` |
| ERMS | `DownloadFromERMS()` | `IniConfig.bEnableErms` |
| TSMC 專用 | `DownloadFromServer_TSMC()` | `CUSTOMER_CODE==CC_TSMC_TAINAN` |
| SPIL | BAT script | `IniConfig.bSPILFunction` |

---

## 7. SECS/GEM 事件對應

| 事件 | 觸發函式 | 條件 |
|------|---------|------|
| `DoLotStart` | `SetLotStart(false)` | 非 TSMC + `bLotStart==false` |
| `DoLotEnd` | `sbSECSLotEndClick` | 非 2DID sorting |
| `DoVisualSortLotStart` | `SetLotStart(false)` | 2DID sorting 模式 |
| `DoVisualSortLotEnd` | `sbSECSLotEndClick` | 2DID sorting 模式 |
| `SwitchSetupFile` | `btSaveSetupFileClick` | Recipe 切換後 |

---

## 8. ATC 溫控顯示架構

| ATC 類型 | 偵測條件 | 溫度取得 |
|---------|---------|---------|
| Hontech ATC 7.0 | `Temperature.bATC70Active && ATC_SYSTEM==eATCHontechType` | `fATC7NowTemp[i]` |
| Hontech ATC 2.0 | `ATC_SYSTEM==eATCHontechType` | `GetATCSiteNowTemperature(i)` |
| New ATC | `ATC_SYSTEM==eNewATCSystem` | `ATC_InterfaceForm->dTC[i]` |
| WinWay | `ATC_SYSTEM==eWinWay` | `fWinway->arrATC_Site[i]->GetPT_NoCommand()` |

> ⛔ 20261002 更正（ST01-E2 對 golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uLotInfo.cpp`）：上表的 `eATCHontechType` 不對，程式是 **`eATCHonPrecType`**（`:5608`、`:5613`；`eATCHontechType` 在 uLotInfo.cpp 0 筆）。分派在 `ShowATCThermo`（`:5574-5629`），由 `TfTemperFrom::Timer1Timer` 每拍呼叫（`cTemperFrom.cpp:1689`）。補充：ATC 7.0 另有 TSD 版（`fATC7NowTSDTemp`），值為 0 時顯示 999.9（`:5727-5739`）；ATC 2.0 第二欄是 `GetATCSiteNowTemperature_Ref`；New ATC 顯示 `dTC/10`，第二欄 `bShowTJTemp` 時是 `dTJ/10`、否則 `dTC2/10`（`:6559-6622`）；WinWay 那列是在 Timer2Timer（`:7195`），不在 ShowATCThermo。告警另有 WAR15300（`:5756`、`:6675`）、WAR15306（`:6706`）、WAR15309（`:6050`）、WAR15243（`:5627`）。溫度視窗（TfTemperFrom）那邊 ATC 怎麼顯示：`D:\HT9045\.claude\skills\ht9045-temperature\references\main-screen-display.md` §5。

**告警分層（[Network] 設定）：**
- `[L11-4]` 絕對溫度上限 (`WAR15304`)
- `[L11-5]` 參考感測器差異
- `[L11-6]` OutsideAlarm 超出設定範圍連續計時 (`WAR15301`)
- `[L11-7]` MaxSurge 瞬間突波 (`WAR15302`)
- `[L11-8]` CompareAlarm 兩點差異 (`WAR15303`)

---

## 9. CyuEan 特殊 CSV Log

```cpp
// 路徑：D:\HT9045_Log\LotInfo\YYYY\YYYYMM_LotInfo.csv
// 欄位（14 欄）：
// StartTime, EndTime, TesterOsVer, TesterID, Operator,
// Customer, TestProg, DeviceName, LotNo, SubLotNo,
// ModeCode, TestCode, TestBinNo, MachineID
```
- `iLotStatus=1` → Lot Start，記錄 StartTime
- `iLotStatus=2` → Lot End，輸出 CSV 行 + 清空欄位

---

## 9b. V906 移植樹：RTC 換檔、Change File、palSecsGem 暗門（20261002，St01，todo E-020 LI-6／LI-11／LI-13）

golden＝906 `D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\uLotInfo.cpp`（cp950，不在 git；Jimmy RULINGS_20261002 第 20 條）。移植樹＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0`。下表與下面的行號是 V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uLotInfo.cpp`（只拿來對照）；906 對照（20261003 E-030，AI(W906-E030-CITE)，兩棵內容相同）：Timer1Timer :5255-5342、RTCChangeFile :5344-5364、enum :5246-5252、WaitRtcDeleteDelay :5253、10 秒逾時 :5323、Timer1 dfm :14496-14501；btChangeFileClick :10213-10237（dfm :4517-4524；FormShow :545-546）；palSecsGemMouseDown :10351-10425（dfm :314-323 同號）；呼叫者 cSetUp.cpp :2824／:2832、uhome.cpp :1823、main.cpp :10576、cContact.cpp :14565；BarCode.cpp :6119-6138／:6195-6211。

| 項目 | golden | 移植樹 |
|------|--------|--------|
| RTC 換檔狀態機 | `Timer1Timer` :5366-5453、`RTCChangeFile(bool bNeedDelete=true)` :5455-5475、`enum eWaitRtcDeleteTask` :5357-5363、`TQPF_Timer WaitRtcDeleteDelay` :5364；Timer1 dfm :14621-14626（Enabled=False、Interval=100） | `LotInfo_E020.cpp`（ht9045_sm）；宣告在 `forms/fLotInfo.h:2333`（同一行） |
| Change File | `btChangeFileClick` :10394-10418（dfm :4641-4648；可見度 FormShow :572） | `W906_E020_btChangeFileClick`（`LotInfo_E020.cpp`）；網頁 `act.lotInfo.changeFile` |
| palSecsGem 6 下暗門 | `palSecsGemMouseDown` :10532-10606（dfm palSecsGem :314-323） | 本體＝jimmychiu 的 `forms/fLotInfo.cpp:4794`（照 golden）；網頁 `act.lotInfo.palSecsGemMouseDown` |

- **RTC 換檔的呼叫者**（V912）：`cSetUp.cpp:2846`／`:2854`（讀配方；移植樹 `cSetUp.cpp:1084`／`:1094`，20261002 解閘）、`uhome.cpp:1946`（歸零；移植樹 `uhome.cpp:1554`，解閘，等待點 `:1956` 讀 `bRTCChangeFileFinish`）、`main.cpp:11017`（開機，移植樹沒有那段 FormShow）、`cContact.cpp:14673`（ROI 學習，fContact 沒移植）。
- 呼叫條件都是 `REAL_TIME_CCD && !COM2->bCCDDummyRum`；模擬組態 `bCCDDummyRum` 永遠 true（`cSetUp.cpp:1055-1056`），所以只有出貨組態＋Gerneral.ini `REAL_TIME_CCD=1`＋配方開 CCD buffer 才會進來。
- **COM2 的 RTC 視覺那一半沒有移植**（`atester_shims.h:373-462` 的 `TCOM2Shim` 只有扭力那半）：Timer1Timer 裡每一個送給視覺的敘述（@FILE、@SITE、DELETE、MODEL、InspEnd）都閘住（`GATE (W906-E020-LI6-1)`～`-6`），每一筆印一行 `[E020-LI6] GATE ... skipped` 並記在 `W906_E020_RtcSkips()`（最後 200 筆）；狀態機與旗標照 golden 跑。
  結果＝golden 在 RTC 斷線時的行為：DELETE 的回覆永遠等不到，`wrdWaitReply` 靠 golden 自己的 10 秒逾時（:5434）結束，最後 `bSendRealCCDSendStart=true`、`bRTCChangeFileFinish=true`、Timer1 關掉。
- **Timer1 還沒有人敲**（todo X-1／E-025，Jimmy 的 timer 表）：`W906_TfLotInfo_Timer1Timer()` 是給將來 timer 卡用的 OnTimer（每 100 ms 呼叫；照 VCL，只有 `Timer1->Enabled` 時才跑）。接上之前，`RTCChangeFile` 只把 Timer1 打開，歸零在 `uhome.cpp:1956` 照舊等（每 10 秒「RTC Change File TimeOut」）——跟解閘前一樣。
  ⚠ 接上 timer 卡之後：RTC 機台歸零會完成，但視覺端沒收到 @FILE／@SITE／MODEL（human-review A，要上機看）。
- **Change File（BarCode 分頁）**：分支照 golden 選；三個分支都還沒移植，C++ 回 `guard:"gated"`＋`whyNot:"GATE (W906-E020-LI11-n): ..."`，什麼都不做：
  - `-1` `BAR_CODE_INSTALL==ebctInShtIntel`：golden 按 fBarCode 的 change-file 斷線／連線鈕（V912 `BarCode/BarCode.cpp:6141-6160`，ClientSocket_BarcodeChangeFile）——移植樹 fBarCode 沒有這兩顆鈕。
  - `-2` `ebctEtherNetCCD`、`-3` `ebctUseCCDMode`＋`CosFunction.b2DUseSubJobFunction`＋`TestIF_File.b2DUseSubJob`：golden `bBarcodeConnect=true`＋`InitialBarcodeScanChangeFile()`＋`TimerBarcodeChangeFile` → `DoBarcodeChangeFile`（`BarCode.cpp:6217-~6700`）——整條換檔鏈沒移植（`cStateRecord.cpp:1938` GATE、`WebRecipeChange.cpp:537` 缺口同一件事）；**不單獨設 `bBarcodeConnect`**。
  - golden 不走任何分支（`bEnableBarCode` 關、或沒有對應的 BAR_CODE_INSTALL）＝ executed、什麼都不做（照 golden）。
  - 按鈕看不看得到：C++ 先照 FormShow :571-572 重算（`tab-hidden`／`button-hidden`）；頁面另外照 tag `lot.barcode.changeFile.visible` 顯示。
  - BarCode 的筆記放這裡：`ht9045-barcode-flow`／`ht9045-clearcount-flow` 兩支是 RogerYang 的、已在 main 撤掉（RULINGS_20261002 第 13 條），不要重建。
- **palSecsGem 暗門**：左左右右左左（golden `static int iStep`）；運轉中、等級低於 HonPrec、`CC_KYEC_LEE` 時 golden 直接 return。第 6 下：Lot ID 空的 ⇒ `SetLotID("0123456789")`（寫 `AuthPath config.ini [Lot Info]` 的 Lot ID、LotStartTime、Customer Lot ID，改 `RunInfo.LotNo`），Operator ID 空的 ⇒ "12345"。golden 沒有任何提示。
  網頁每一下都送（排隊、不丟、不套冷卻），帶 `seq`（第幾下），不然伺服器 WebCmdGuard（同指令＋同 value 400 ms 內 ⇒ busy:）會把「左、左」併成一下；按在面板裡的按鈕上不算；面板右鍵不跳瀏覽器選單。C++ 先查 `tsLotID` 分頁與 `palSecsGem` 看不看得到（`tab-hidden`／`panel-hidden`）。
  主畫面的 Lot Start 框（筆電的 `ht9045_lotstart.js`）不會預填這兩個值（交給筆電）。
- 網頁路由：`JsonBridge/ChanAction.cpp:348`（同一行）→ `ht9045::sjson::W906_LotInfoE020Act`；act.* 照例要操作權杖、過 WebCmdGuard、不持 FormLock（E-021 先例）。頁面 `D:\HT9045\web\page\ht9045_lotinfo_e020.js`（`Data.LotInfo.html` :496 載入、:302 拿掉 disabled、:92 加 `data-e020`）。
- ctest：`E020_LotInfoRTC`、`E020_LotInfoActs`（`tests/test_e020_lotinfo_e020.cpp`；測試一定要 `W906_AUTH_PATH` 指到沙盒，否則 exit 2）、`E020_LotInfoPage`（node）。對照組：`W906_E020_SRC_ROOT`／`W906_E020_PAGE_DIR` 指到改之前的檔要變紅。

---

## 10. 已知問題 / 注意事項

| 問題 | 說明 |
|------|------|
| `SetLotStart` 不在 HT9046LS 同名位置 | V899 定義在 L1438；HT9046LS 早期版本從 `auto9045.cpp` 呼叫 |
| `iTestHeadMotorTask` 鎖定 | Lot End 時若 Index 正在下壓，`sbSECSLotEndClick` 會拒絕 |
| Recipe 下載後 Contact Height 被覆蓋 | 由 `Security_new.def [Network] Contact High` (預設 false=保留本地) 控制 |
| `bLotStart==false` 才觸發 DoLotStart | 避免重複 Lot Start 時重送 SECS 事件 |
| `ONECYCLE_REQUEST` 不在 Release Build | 見 AutoStart SKILL（`#ifdef DEBUG_DUTONOFF`） |

<!-- preserved-content:end -->
