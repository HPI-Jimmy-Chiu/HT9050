# D-005 查證與實作方案（St01 唯讀，20260927 15:4x）

> 交件給 ST01-E。這一趟**沒有改任何程式、沒有 git 寫入、沒有 build、沒有執行 wb_serve**；唯一寫的是本資料夾
> `C:\Users\steven\AppData\Local\Temp\claude\d---github\99781e6b-2c4e-4591-81e8-fc879566566c\scratchpad\d005\`。

## 0. 範圍、三棵樹、量法

- **題目**：todo D-005（`D:\HT9045\.claude\skills\ht9050-construction\references\todo.md:56`）——golden TfMain 裡還沒移植、
  會呼叫 `SaveTestMode`／`WriteLastDataFile` 的其餘入口。
- **golden V912（BCB6，cp950）**：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（主 repo 那份；HT9045_ref 20260926 已退場）。
  本文件 golden 行號**一律用這一份**。⚠ S65／S88 當時用的是 HT9045_ref，`main.cpp` 在 :4796 之後比這份**少 2 行**；
  D-005 原文的 `:24199`／`:24201`／`:12698`／`:28379-28381`／`:29805-29861` 是 HT9045_ref 行號，這份是 `:24201`／`:24203`／`:12700`／`:28381-28383`／`:29807-29871`。
- **移植樹（C++，UTF-8）**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`，分支 `v906/steven-cbridge-review6`，量的時候 HEAD＝`fca6248c`（開工時 `19b24d3e`，
  中途進來 1de5005b 合 main 等 8 顆，收工前重跑結果不變）。
- **網頁**：`D:\HT9045\web\page\`（main.html 等）。
- **交接快照**：`D:\HT9045_handoff\`（20260927 13:00，main=382cc42d）；另讀了 `origin/main`（56039beb，14:28）的 `docs/handoff/TO_STEVEN.md` 與 `docs/INBOX_QUEUE.md`、`docs/NIGHT_REPORT.md`。
- **量法（20260927 15:2x～15:4x；收工前 15:36 重跑移植樹那一半，結果相同）**：
  - golden：`scratchpad\d005\scan.py`（整棵 V912 以 cp950 讀，找 `\b(SaveTestMode|WriteLastDataFile|WriteLastDataFN)\b`，附所在函式、`#if` 堆疊、註解判斷）→ 結果 `scratchpad\d005\scan_out.txt`；
    `scan2.py ShowTestHeadComp`／`RunICModeChange`／`ChangeTempMode`（呼叫者）→ `sthc_callers.txt`；`ranges.py`（大括號配對算函式範圍）。
    UTF-8 副本在 `scratchpad\d005\g\`。
  - 移植樹：`git grep -n -E "\bSaveTestMode\s*\(\s*\)\s*;"`、`git grep -n -E "\bWriteLastDataFile\b"`、`WriteLastDataFN`、`ShowTestHeadComp`，
    `#if` 堆疊用 `scratchpad\frw_s88\ctx.py`；`ShowTestHeadComp` 呼叫點分類用 `scratchpad\d005\port_sthc.py` → `port_sthc2.txt`。
  - 「全樹沒有」的宣稱都只代表上面這些指令在 15:36 的結果；平行的同事隨時可能加進來。

## 1. 結論（先講重點）

1. **D-005 剩下的 5 件，沒有一件的主體是 St01 的**（照 S85「看區段」＋既有前例）：
   - `ShowTestHeadComp`／`ShowTestHeadComp1` → **Jimmy**。筆電已經自己排了：`docs/INBOX_QUEUE.md` 第 69、77 列、`docs/NIGHT_REPORT.md:135`「排在白天的大件：`ShowTestHeadComp1`（golden 980 行…START 也會呼叫）」。
   - `RunICModeChange`＋`imgRunModeClick`（主畫面 Run Mode 圖示）→ 建議 **Jimmy**（改 HotPlate 盤資料、InArm 取放索引、機台內有 IC 的互鎖）；見 Q 題 D5-Q2。
   - `Panel42Click`（主畫面溫度 On/Off 圖示 → `ChangeTempMode(10)`）→ 建議 **Jimmy**（`ChangeTempMode` 本體是 Jimmy 的 `MainTempMode.cpp`，會動加熱／ATC）；St01 可以代做薄薄一層入口，見 D5-Q3。
   - `DoTrayFeedProcess` 三處 → **Jimmy**（S79）。⚠ D-005 原文說「解閘時把 V912 新加的 :11362／:11882 帶進來」，**跟 RULINGS_20260926 第 26 條 Q2「底層照 906」衝突**，要 Jimmy 判斷算不算例外（見 D5-Q4）。
   - `MainTempOffsetTail` 擋住的溫控任務重啟 → **Jimmy**（加熱器；符號都在，解閘本身是 St01 檔裡 3 行，見 D5-Q5）。
   - `ChangeTesterConnect` 已由 **St02** 做完（done H-102，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp:1233` `SaveTestMode();`），D-005 可以拿掉這一項。
2. **影響最大的是 `ShowTestHeadComp`**：移植樹有 **37 個活的** `ShowTestHeadComp(...)` 呼叫點（另 4 個在 `#if 0`），現在全部打到
   `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp:247` 的空殼 `{}`。Jimmy 一裝本體，這 37 處（含 START `WebStart.cpp:1567`、HOME `forms\fMain.cpp:792`、
   換配方 `WebRecipeChange.cpp:346`／`:496`、St01 的溫度頁／Setup 頁／Configuration 頁存檔尾段）**同時開始**：寫 `<配方>\TestMode.Data`、寫 `system\lastdata.dat`、
   跑 `ChangeATCSiteUse`（會送 ATC 命令）、跑 `DoInArm_SuckerMap`。St01 要跟著更新自己幾個檔的「會寫哪些檔」清單（第 3 節 C 段）。
3. **順帶發現的現況落差**（不改行為，只是 golden 有、移植樹永遠是舊值）：SECS SVID 1250「Enabled Site Count」（`iEnabledSiteCount`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cmydef.cpp:5791`）
   在移植樹**沒有任何寫入點**，永遠是 0；SECS ECID 1530／1531（`fMain->tSiteOnOff[0/1]`）開機之後不會再更新。兩者 golden 都只在 `ShowTestHeadComp1`（:24173-24231）算。等 Jimmy 那一件。
4. **網頁**：主畫面 `D:\HT9045\web\page\main.html` 上 D-005 相關的三個點**都沒有送出任何東西**：Site 格（:449-481，點格切換刻意拿掉）、溫度圖示 🌡️（:176）、Run Mode 圖示 ▣（:190）。
   `D:\HT9045\web\page\main-control.js` 雖然有 `Panel42`／`imgRunMode` 的 click，但**沒有任何 html 載入它**（15:3x `grep -rl main-control` 只有它自己），而且它送的是舊 C# 模擬器那條路（`HTSimulatorBridge`）；main.html 也沒有 `id="Panel42"`／`id="imgRunMode"`。

## 2. 全部呼叫點對照表

分類：❶ 活的（會真的寫檔）／❷ 函式在但被 `#if 0`、GATE、空替身擋住／❸ 移植樹沒有這一段。
寫的檔：**TM**＝`D:\HT9045\IniData\Data\<配方>\TestMode.Data`（`SaveTestMode`，移植樹本體 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cprod.cpp:3504`；`InitialOK==false` 或 `CosFunction.bLastSetInSetUpFile==false` 時自己 return，所以開機那幾次實際不寫）；
**LD**＝`D:\HT9045\system\lastdata.dat`＋`lastdata_backup.dat`（`BackUp2=true` 時再加 `lastdata_backup2.dat`）＋`D:\HT9045\config\config.ini` 幾節（`WriteLastDataFile`，本體 `cprod.cpp:2014`；ctest 由 `W906_LastDataPath` 轉進沙盒）；
**SI**＝`D:\HT9045\SetUp.inf`（`WriteLastDataFN`，本體 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp:1524`，只寫最後開的配方名）。
golden 行號＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` 那份；移植樹行號＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` 那份。

### 2.1 TfMain（golden `main.cpp`＋`Command.cpp`；golden 全樹只有這兩個檔有 `TfMain::` 定義）

| # | golden 位置 | 所在函式（golden 範圍） | 觸發 | 寫 | 移植樹狀態 | 網頁 | 歸屬 | 建議 |
|---|---|---|---|---|---|---|---|---|
| T1-3 | main.cpp:9301／:9310／:9319 `SaveTestMode` | `TfMain::DoReadLastData`（:9264-9441） | 開機（FormShow）與換配方（ChangeSetUpFile）都跑，依 `iMachineTempMode` 0／1／3 | TM | ❶ `tools\wb_serve.cpp:3138`／`:3147`／`:3156`（`W906_DoReadLastData`） | 間接（開機／換配方） | St01（已接） | — |
| T4 | main.cpp:10047 `WriteLastDataFile()` | `TfMain::FormShow`（:9568-11849）:10043-10048 | 開機，`IniConfig.bShowLotInfo` | LD | ❶ `tools\wb_serve.cpp:4162` 同一行 → `FileRW\MainBoot.cpp:245`（`c317ca30`，S65） | — | St01（已接） | — |
| T5 | main.cpp:11861 `WriteLastDataFile(true)` | `TfMain::FormClose`（:11852-12478） | 關程式 | LD（含 backup2） | ❶ `FileRW\MainClose.cpp:1148`（Exit 鈕 step 1）、`:1284`（`--seconds` 關站）（`6905f8eb`，S95／S120-1） | Exit 鈕 `main.html:31` → `act.main.closeProgram` | St01（已接） | S65 表 #17 要從 ❸ 改成 ❶ |
| T6 | main.cpp:12130 `WriteLastDataFN(str1)` | `TfMain::FormClose` :12094-12146 | 關程式，只在 `CosFunction.bFTPFunction`（KYEC FTP）且目前配方名含 `_NET` | SI（並 XCOPY／RD 配方資料夾） | ❸ 刻意不做：`FileRW\MainClose.cpp:873` 關站清單標「不做：會 XCOPY／RD 配方資料夾」 | — | 客戶專屬（S25） | 維持不做 |
| T7 | main.cpp:12700 `SaveTestMode` | `TfMain::ChangeTesterConnect`（:12581-12778） | 主畫面 Tester 圖示、GPIB／TCP 遠端切 On/Off-Line | TM | ❶ `forms\fMain.cpp:1233`（St02 `979eac6b`，done H-102） | `act.main.testerConnect` | St02（已接） | D-005 刪掉這一項 |
| T8 | main.cpp:13642 `WriteLastDataFile()` | `TfMain::UpdateMainOperateMode`（:13326-13650） | 很多地方呼叫（換配方、溫度頁尾段、ChangeTesterConnect、RunICModeChange…） | LD | ❶ `forms\fMain_OperateMode.cpp:514`，經 `forms\fMain.cpp:507` 的 `W906_UpdateMainOperateModeHook`，wb_serve 開機裝（`tools\wb_serve.cpp:4065`）；筆電 OPMODE `7304dcef` | 間接 | Jimmy（已接） | S65 表 #18 要從 ❷ 改成 ❶；`FileRW\Temperature.cpp:122` 註解「移植樹門面計數 stub」過期 |
| T9 | main.cpp:15628 `WriteLastDataFile(false)` | `TfMain::Clarn_Data`（:15458-15648） | 清計數（各 Tag） | LD | ❶ `JsonBridge\actions\MainClarnData.cpp:315`（經 `forms\fMain.cpp:495` 的 `W906_ClarnDataBody`） | `act.main.clarnData`、`act.sortCT.clearCount` | 筆電／St01（已接） | — |
| T10 | main.cpp:15873 `WriteLastDataFile(false)` | `TfMain::SetLotState`（:15698-15972） | `hanaART->IsHanaArtAvailable() && iState==8`（HANA ART Lot End） | LD | ❷ `forms\fMain.cpp:553` 空殼 `{}` | — | St02（INBOX 第 77 列「St02 範圍」） | 不在 D-005，留給 St02 |
| T11 | main.cpp:16065 `WriteLastDataFile(false)` | `TfMain::ProcessARTMessage`（:15974-16475） | GPIB `MSG_CMD_SCKART_LOTRTCLEAR` | LD | ❶ `TesterComm\Handler\HandlerGpibMsg.cpp:306`（St02 GB-P2c，gate G3 解開） | — | St02（已接） | S65 表 #21 要從 ❸ 改成 ❶ |
| T12 | main.cpp:22552 `SaveTestMode` | `TfMain::ChangeTempMode`（:22457-22633） | 見 T13 與 GPIB SETTEMP 類 | TM | ❶ 本體 `MainTempMode.cpp:172`（Jimmy FW-CMD-E，照 906 翻）；**活的呼叫者只有 SIGURD GPIB**：`Command.cpp:11379`、`:11459`、`:11471`（`WriteSetTempStatus`／`SetTempByDLL` 那幾處在 `#if 0`：`Command.cpp:1931`、`:1952`、`:9630`） | — | Jimmy | 本體已在；缺的是 T13 |
| **T13** | （main.cpp:22437 → `ChangeTempMode(10, true, bRefreshFunction)`） | `TfMain::Panel42Click`（:22392-22449）＝ `imgTempOnOff`（dfm `palTemperatureOnOff`）的 OnClick | 主畫面溫度圖示：Hot↔Ambient 切換；守衛 `Insufficient(7)`，客戶分支 HiSilicon／KYEC_LEE（PE）、`Barcode_Reader(bcTemperature)`（KYEC）、CC_ASE_CL（確認框＋`DoPassword`）；之後 `AmkorSendMessage`、`ShowTestHeadComp(false)`、A09 時 `fTestCategory->ShowTestCategory(0/1)`、SECS `SwitchTemperature` | TM（經 T12）＋LD（經 ShowTestHeadComp1） | ❸ 移植樹沒有 `Panel42Click`（全樹 0 命中，15:36） | 🌡️ `main.html:176` 只是字，沒有 click | **Jimmy**（D5-Q3） | 見 3.E2 |
| **T14** | main.cpp:24201 `SaveTestMode`、:24203 `WriteLastDataFile()` | `TfMain::ShowTestHeadComp1`（:23253-24232，980 行），由 `ShowTestHeadComp`（:23238-23247）呼叫 | golden 54 個活的 `ShowTestHeadComp(...)` 呼叫者：START（:4895）、HOME（:7417）、開機（:10660、:11756）、溫度鈕（:22439）、換配方（:25497、:25769）、Temp. 鈕尾段（:28400）、Setup／Configuration 關窗（:28481、:28641）、Site 格點擊（:30274、:30553）、工程師 Site 鈕（:33790）、GPIB／TCP 的 site map（Command.cpp:7830-13535 共 6 處）、Clean Out（csystem.cpp:16161-16569）、低良率關 site、SCK_ART、QAMode、SECS S2F15、LotInfo 下載…（清單 `scratchpad\d005\sthc_callers.txt`） | TM＋LD | ❸ 函式不在；`forms\fMain.cpp:247` `void TfMain::ShowTestHeadComp(bool) {}`；移植樹 37 個活的呼叫點全打到空殼（`scratchpad\d005\port_sthc2.txt`） | Site 格 `main.html:449-481` 只讀（`site.arm*` tag），點格拿掉 | **Jimmy**（INBOX 69／77，NIGHT_REPORT:135 已排） | 見 3.C（St01 跟進）與 D5-Q1 |
| **T15** | main.cpp:29856 `SaveTestMode` | `TfMain::RunICModeChange`（:29807-29871）；呼叫者 `imgRunModeClick`（:29796-29805，`!SystemStart && Insufficient(11)`）、JCET 換配方 :25569 | 主畫面 Run Mode 圖示：Real → Dummy → Tray Only 循環 | TM＋LD（經 :29859 `UpdateMainOperateMode`） | ❸ `RunICModeChange`／`imgRunModeClick` 全樹 0 命中；JCET 那一處 `WebRecipeChange.cpp:541` 標 Cust | ▣ `main.html:190` 只是字（`palRunMode_1` 顯示 `runmode.value`） | **Jimmy**（D5-Q2） | 見 3.E1；golden 有一個「先清盤再擋」的疑點（D5-Q6） |
| T16-17 | main.cpp:25743／:25753 `SaveTestMode`、:25723 `WriteLastDataFN` | `TfMain::ChangeSetUpFile`（:25655-25817） | 換配方 | TM、SI | ❶ `WebRecipeChange.cpp:320`／`:330`、`:303` | 主畫面配方下拉 → `recipe.change` | St01（已接） | — |
| T18-19 | main.cpp:28363／:28373 `SaveTestMode` | `TfMain::sbTempOffsetClick`（:28347-28402）尾段 | 主畫面 Temp. 鈕 → 溫度視窗關掉之後 | TM | ❶ `FileRW\Temperature.cpp:84`／`:94`（`MainTempOffsetTail`，`295bc768`，S88／S107-1＝存檔後就跑） | 溫度頁 `editlist.save`（tag=Temperature） | St01（已接） | 溫控重啟那 3 行見 T20 |
| **T20** | main.cpp:28381-28383（`fHeaterOK=false; bHeatOKBellowError=false; iThermoTask=1;`） | 同上 | 同上，`LastSet.iTemperature` 是 Hot／AmbientHot 時 | —（不寫檔，重啟溫控任務把新設定值送到加熱器） | ❷ `FileRW\Temperature.cpp:103-107` `#if 0 // GATE(W906-FRW-S88-THERMO)`；三個符號都在（`bthermo.cpp:1261`、`cmydef.cpp:2762`、`:2765`） | 同上 | **Jimmy**（D5-Q5） | 見 3.E3 |
| T21 | Command.cpp:13762 `WriteLastDataFile(false)` | `TfMain::TCPCommandServerClientRead`（Command.cpp:12770-14309）HTSET,701 | TCP 遠端指令 | LD | ❷ `Command.cpp:17588` 在 `#if 0`（:17556，GATE W906-FW-CMD-C／A11、SAFETY S3） | — | **St02**（W10，7016／7017 TCP 指令伺服器全部歸 St02，TO_STEVEN §4 20260927 12:3x） | S65 表 #4 的歸屬從 Jimmy 改 St02 |

### 2.2 不在 TfMain 的（列出來對帳，都不是 D-005 本體）

| golden 位置 | 所在函式 | 寫 | 移植樹狀態 | 歸屬／備註 |
|---|---|---|---|---|
| cprod.cpp:1582 `SaveTestMode` | `LastSetUseTestSocketToTestModeDutOnOff`（ReadLastDataFile 呼叫） | TM | ❶ `cprod.cpp:1682` | — |
| cprod.cpp:1666、:1687 | `ReadLastDataFile` | LD | ❶ `cprod.cpp:1769`、`:1790` | — |
| cprod.cpp:2615 | `ProcessLastSetIni_Index` | LD | ❶ `cprod.cpp:2750` | — |
| cConfiguration.cpp:5519 | `TfConfiguration::edSoftSpeed0Change` | LD | ❸ | S140：Soft 速度**不做** |
| cConfiguration.cpp:5530 | `TfConfiguration::edOCRTrayLotChange` | LD | ❶ `FileRW\IniConfig.gen.inc:9492`（S140 補上、R60） | S65 表 #2 要從 ❸ 改成 ❶ |
| cConfiguration.cpp:7296 | `TfConfiguration::SaveConfiguration` | LD | ❶ `FileRW\IniConfig.gen.inc:8253` | — |
| cSortCT.cpp:1946 | `TfSortCT::pnlAuto1DblClick` | LD | ❶ `cSortCT.cpp:554` | — |
| cStartCondition.cpp:738、:769 | `TfStartCondition::sbSaveClick`／`spbExitClick` | LD | ❶ `FileRW\StartCondition.gen.inc:1432`、`:1460` | — |
| cTowerLight.cpp:79 | `TfTowerLight::RGB00Click` | LD | ❶ `forms\fTowerLight.cpp:256` | — |
| csystem.cpp:11849 `SaveTestMode` | `DoTrayFeedProcess`（QA mode 結束） | TM | ❷ `csystem.cpp:8338`，整支在 `csystem.cpp:7563` `#if 0`（GATE G4-3） | Jimmy（S79） |
| csystem.cpp:11362、:11882 `SaveTestMode` | `DoTrayFeedProcess`（V912 RogerYang 20260629「TrayEnd 區塊原無存檔，補上保持檔案一致」，同段 `LastSet.iTester=` 改成 `fMain->ModifyTester`） | TM | ❸ 906 底沒有這兩行 | Jimmy；D5-Q4 |
| csystem.cpp:12465 | `CheckContinusStartIsReady`（P65，V912 新增） | LD | ❸ | Jimmy（底層照 906） |
| csystem.cpp:15022、:15107 | `DoART_AfterCleanOut` | LD | ❷ `csystem.cpp:5771`、`:5856`，被 `csystem.cpp:4191-4192` 的 `#define WriteLastDataFile W7C2_WriteLastDataFile`（空替身 return true）吃掉 | Jimmy |
| cBuilder.cpp:545 `SaveTestMode` | `TfBuilder::bSaveAllFillOrFile` | TM | ❷ `forms\fBuilder.cpp:887`（`#if 0` GATE B-9） | golden 本身走不到（S88） |
| Automation\SCK_ART.cpp:437 | `TfSCKART::AccessFile` | LD | ❷ 空巨集 `Automation\SCK_ART_Remainder.cpp:590` | 客戶專屬 S80 |
| SECSGEM\uHGemHT9045.cpp:5269 | `HT9045Gem::S7F4_ProcessProgramAcknowledge` | LD | ❷ `SECSGEM\uHGemHT9045.cpp:1502`（`#if 0` :1050） | Jimmy（S65 當時的歸屬） |
| uLotInfo.cpp:12133 `WriteLastDataFN` | `TfLotInfo::ClearAllSetupFile`（FTP／RMS 下載、`bKeepOnly1SetupFile` 刪其他配方資料夾） | SI | ❸ 函式不在；呼叫者 `SECSGEM\uHGemHT9045.cpp:1880`、`:6108` 都在 `#if 0` | 客戶專屬（LotInfo 區段屬 St01，但 S25 不做） |

**總數**（golden 活的呼叫點；定義、宣告、`Automation\uRENESAS_Server.cpp:876` 註解掉的不算）：

| 函式 | golden | ❶ | ❷ | ❸ |
|---|---|---|---|---|
| `SaveTestMode` | 16 | 10 | 2（cBuilder、csystem :11849） | 4（ShowTestHeadComp1、RunICModeChange、csystem :11362／:11882） |
| `WriteLastDataFile` | 23 | 14 | 6（HTSET,701、SCK_ART、DoART_AfterCleanOut×2、S7F4、SetLotState） | 3（Soft 速度、P65、ShowTestHeadComp1） |
| `WriteLastDataFN` | 3 | 1 | 0 | 2（FormClose FTP、ClearAllSetupFile） |

另：移植樹多一個 golden 沒有的 `WriteLastDataFile`（`WebTowerLight.cpp:317`，決斷 D3，維持）。
`TesterComm\Gpib\*` 裡的 `WriteLastDataFile()` 是 GPIB 橋接程式自己的 `TSerialPoll::WriteLastDataFile`（`TesterComm\Gpib\GpibUi.cpp:1671`），不是 handler 這支，不算。

## 3. St01 能做的（實作步驟）

> 原則：D-005 的五件主體歸 Jimmy（St02 一件）；St01 能做的是**台帳更正、自己檔案的跟進、網頁**，以及（Steven／Jimmy 同意的話）兩個主畫面薄入口。
> `tools\wb_serve.cpp` 另一位同事正在改，**只讀**；要碰它的步驟標「等 Q40 交件」。（15:23 已有 `76058840`「S157 (Q40=A): WS form.event」，
> 但 wb_serve.cpp 最後一顆是 15:32 的 `9b78d312`；Q40 是否已整包交件請 ST01-E 跟那位同事確認。）

### A. 台帳（不動程式）

- **A1** 改寫 todo D-005 那一列（`D:\HT9045\.claude\skills\ht9050-construction\references\todo.md:56`）：
  - 行號改成主 repo V912（上表），註明 HT9045_ref 行號少 2。
  - 拿掉 `ChangeTesterConnect`（已 done H-102）。
  - 項目與歸屬：ShowTestHeadComp／1（Jimmy，INBOX 69／77）、RunICModeChange＋imgRunModeClick（Jimmy）、Panel42Click（Jimmy）、DoTrayFeedProcess×3（Jimmy S79，912 兩行待 D5-Q4）、溫控重啟（Jimmy，D5-Q5）；
    另加一句「HTSET,701（Command.cpp:13762）歸 St02 W10」。
  - 「下一步」寫：Jimmy 裝 `ShowTestHeadComp` 本體之後，St01 做 3.C。
- **A2** S65 的 WriteLastDataFile 對照表（`D:\docs\ChangeLog\CHANGES_20260926_Steven.md` §11.25 摘要＋ scratch `frw_s65\audit.md`）四列狀態過期：
  #2 OCR Tray Lot ❸→❶（`IniConfig.gen.inc:9492`）、#17 FormClose ❸→❶（`MainClose.cpp:1148`／`:1284`）、#18 UpdateMainOperateMode ❷→❶（`fMain_OperateMode.cpp:514`）、#21 ProcessARTMessage ❸→❶（`HandlerGpibMsg.cpp:306`）；
  #4 HTSET,701 歸屬 Jimmy→St02。建議寫在 `D:\HT9045\.claude\skills\ht9045-json-bridge\references\write-inventory.md` 的 WriteLastDataFile 那一列（ChangeLog 只增不改，另開一節寫更正）。
  ＝ 10＋2＝**14 個活的**（S65 當時 10 個）。

### B. St01 自己檔案的註解更正（行為不變，同一行改，行數不變）

- **B1** `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.cpp:122` 行尾註解「（移植樹門面計數 stub，forms/fMain.cpp；同 WebRecipeChange.cpp）」已過期
  → 改成「（筆電 OPMODE `7304dcef`：經 forms/fMain.cpp:507 hook 跑真本體 forms/fMain_OperateMode.cpp，wb_serve 開機裝；會寫 lastdata.dat）」。
  同檔 `:65-69`「⚠ 會寫的真實檔」補一行 `D:\HT9045\system\lastdata.dat（fMain->UpdateMainOperateMode → golden :13642）`——這是**現在就成立**的，不用等 Jimmy。
- **B2** 同樣檢查 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebRecipeChange.cpp`、`FileRW\MainClick.cpp`（S100-a 尾段）裡把 `UpdateMainOperateMode` 寫成 stub 的註解與 ack 的 writes 清單；
  `.gen.inc` 裡的註解要改 `tools\editlist\*.py` 再 `gen_editlist.py --only <頁>` 重產，不要手改。

### C. Jimmy 裝 `ShowTestHeadComp` 本體之後 St01 要跟的（觸發條件：Jimmy 推 main）

Jimmy 預計的裝法（同 OPMODE 前例，`forms\fMain.cpp:507`）：`forms\fMain.cpp:247` **同一行**換成
`void (*W906_ShowTestHeadCompHook)(TfMain*, bool) = 0; void TfMain::ShowTestHeadComp(bool bRefresh) { if (W906_ShowTestHeadCompHook != 0) W906_ShowTestHeadCompHook(this, bRefresh); }`，
本體放 `ht9045_sm` 的新檔（例 `forms\fMain_TestHeadComp.cpp`），wb_serve 開機裝（**等 Q40 交件**，或接在 `tools\wb_serve.cpp:4065` 筆電 OPMODE 那一行同一行後面）；沒裝的 ctest＝原本的空殼。
這是 Jimmy 的檔與區段，St01 不動。之後 St01：

- **C1** 更新 St01 幾個呼叫點的「會寫哪些檔」與 ack writes：`FileRW\Temperature.cpp:131`（MainTempOffsetTail）、`FileRW\MainClick.cpp:350`（sbSetupClick 尾段）、
  `FileRW\IniConfig.gen.inc:7725`（Configuration FormClose，改 `tools\editlist\IniConfig.py`）、`FileRW\TestIF_File_SetUp.gen.inc:4040`／`:4063`／`:4068`（Setup 存檔，改 `tools\editlist\TestIF_File_SetUp.py`）、
  `WebRecipeChange.cpp:346`／`:496`（換配方；`:346` 行尾「門面 stub」）——都要多列 `TestMode.Data`、`system\lastdata.dat`，以及「會送 ATC SiteUse 命令（ChangeATCSiteUse）」。
- **C2** 提醒 Jimmy 的 sysguard 預期變動：START、HOME、換配方、溫度頁／Setup 頁／Configuration 頁存檔，從此每次都會寫 `TestMode.Data` 與 `lastdata.dat`（golden 同）。
- **C3**（可選）St01 寫一支 ctest：裝上 hook 後 `fMain->ShowTestHeadComp(false)` 會把 `LastSet.bUseTestSocket` 同步進 `TestMode.iDutOnOff` 並寫進沙盒的 TestMode.Data／lastdata.dat
  （沙盒：`W906_LastDataPath`，`c265f087`；配方路徑要另外轉向）。新檔 `tests\test_d005_sitesave.cpp`＋`tests\CMakeLists.txt` **檔尾附加**。要不要由 St01 寫，交 Jimmy 決定（他的本體、他的測試習慣）。
- **C4** 執行緒：移植樹 37 個呼叫者裡有機台流程（`csystem.cpp` Clean Out、`ainarm2.cpp`、`WebStart.cpp`）、WS 分派（`WebRecipeChange.cpp`、FileRW 存檔，持 FormLock）、GPIB 指令（`Command.cpp`）。
  裝本體前要確認它們都在 wb_serve 主迴圈同一條執行緒（`WriteLastDataFile` 沒有鎖）——這是**待查**，本次沒有查證。

### D. 網頁（todo E-003「DUT on/off 點格」歸 St01；都要等 Jimmy 的 C++ 入口＋wb_serve 分派）

- **D1** Site 格（`D:\HT9045\web\page\main.html:449-481`）：Jimmy 翻 `mtDutOnOffMouseUp`（golden main.cpp:29932-30616，685 行；S59 已定「整支是底層流程＋外部設備 → Jimmy」，St01 只出了
  `FileRW\MainClick.cpp` 的 `W906_Main_DutOnOff_SaveATC7Channels`）並定 WS 指令名之後，St01 把點格接回：送 `{z,x,y}`、頁面第二道防連點（C++ 端 WebCmdGuard 中央擋）、
  C++ 端重新檢查 `SystemStart`／`Insufficient(10)`（不信任前端）。wb_serve 分派那一行 → **等 Q40 交件**。
- **D2** 🌡️（`main.html:176`）→ Panel42Click、▣（`main.html:190`）→ imgRunModeClick：同 D1 的做法，等 T13／T15 的 C++ 入口。
- **D3** `D:\HT9045\web\page\main-control.js` 是沒人載入的舊檔（送 C# 模擬器）。不刪；在 D2 做完時請 Jimmy 決定要不要退場（見 D5-Q7）。

### E. 如果 Steven／Jimmy 把 T13／T15／T20 交給 St01（D5-Q2、Q3、Q5 選 B 時才做）

- **E1 RunICModeChange＋imgRunModeClick**（golden :29796-29871，共 75 行）
  - 新函式放 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp` **檔尾附加**：`int W906_Main_RunICModeChange(bool bDefineMode, int iMode)`（逐行照 golden）＋
    `std::string W906_Main_RunModeOp(const std::string& payloadJson, bool* ok)`（＝imgRunModeClick：`SystemStart` return、`Insufficient(11)`、再呼叫前者；FormLock 包起來，同 `W906_Main_PEModelOp`）。
  - 相依全部已在（15:4x 查 `.h`）：`MOT[].InitEmptyTray`（`Motor\mymotor.h:348`）、`iPickPlate[]`／`iPickPlateX[]`／`iPlacePlateY[]`（`aHotPlateSubstrate.h:775-776`）、`HotPlateForm.iPlateSelect`、
    `fMain->CheckCanChangeRealDummy`、`HasICUnderMachine`、`HasAnyICInMachine`（`csystem.h:109`）、`ShowErrorMessage("MES1646"…)`、`SaveTestMode`、`fMain->UpdateMainOperateMode`（hook）、
    `fMain->LoadRunModePicture`（`forms\fMain.h:509`）、`NewRecordProcess`、`EventReport(SECS_EVENT.SwitchRunMode)`（`SECSGEM\SecsEventReport.h:55`）。
    ⚠ `aHotPlateSubstrate.h` 是陷阱 #3 那顆（177 個 TU 用的 TMyKitSuck）；MainClick.cpp 目前沒 include 它，**只宣告要用的 extern**（同檔 :70-79 的做法），不要把 `mykitsuck.h` 帶進來。
  - 會寫：TM（`TestMode.iRunMode`）、LD（經 UpdateMainOperateMode）；會改記憶體：HotPlate 1／2 盤資料、InArm 取放索引、`LastSet.iRealDummy`。
  - WS：`act.main.runMode`，wb_serve 分派同一行 `else if` → **等 Q40 交件**；頁面 ▣ 加 click（兩段式確認由 Steven 決定要不要，golden 沒有確認框）。
- **E2 Panel42Click**（golden :22392-22449）
  - `FileRW\MainClick.cpp` 檔尾附加 `W906_Main_Panel42ClickOp`：`Insufficient(7)` → 客戶分支（HiSilicon／KYEC_LEE、KYEC `Barcode_Reader(bcTemperature)`、CC_ASE_CL 確認框＋`DoPassword`）照翻但標 Cust（S25）
    → `fMain->ChangeTempMode(10, true, bRefreshFunction)`（`MainTempMode.cpp:65`，Jimmy 的本體，不改）→ `AmkorSendMessage` 照 `FileRW\Temperature.cpp` 的 GATE(W906-FRW-S88-AMKOR) 同樣擋
    → `fMain->ShowTestHeadComp(false)`（Jimmy 裝之前是空殼）→ `IniConfig.bA09_ByArmCloseSite` 時 `fTestCategory->ShowTestCategory(0)`／`(1)`（`forms\fTestCategory.h:475`）→ SECS `SwitchTemperature`。
  - 風險：這是**新的觸發入口**，本身不加新的機台碼，但按下去就是真的 Hot↔Ambient 切換（ChangeTempMode 會清加熱暫存、送 ATC、改 TestMode）；ChangeTempMode 自己擋 `SystemStart` 與機台內有 IC。
  - WS：`act.main.tempOnOff` → **等 Q40 交件**；頁面 🌡️ 加 click。
- **E3 溫控重啟解閘**（`FileRW\Temperature.cpp:102-107`）：刪 `:102` 的 `ELTodo`、`:103` `#if 0` 與 `:107` `#endif` 改成同位置的註解行（行數不變），
  3 行照 golden 跑。前提：RULINGS_20260927 第 16 條（golden 加熱執行緒照 golden 接上，筆電做）已落地，否則 `iThermoTask=1` 沒有人消費；機台驗證要機台端在場。

## 4. 要 Steven 或 Jimmy 決定的

> 題號由 ST01-E 在 `decisions-pending.md` 編（目前最後是 Q46、R79）；下面暫記 D5-Q1～Q7。格式照 decisions-pending：背景／選項／St01 建議。

### D5-Q1.（給 Jimmy）`ShowTestHeadComp1` 裡的寫檔那幾行要不要拆給 St01
- **背景**：golden :24173-24203 是「LastSet → TestMode.iDutOnOff 同步、`tSiteOnOff` 字串、`SaveTestMode`、`WriteLastDataFile`」，其餘 950 行是 site 開關規則與 mtDutOnOff 畫格子。S59 對 `mtDutOnOffMouseUp` 的前例是拆（St01 出 `W906_Main_DutOnOff_SaveATC7Channels`）。
- **選項**：A 不拆，Jimmy 整支照翻（寫檔兩行呼叫的都是既有本體），St01 做 3.C 跟進／B 拆，St01 出 `W906_Main_SiteSaveTail(bool bRefresh)`，Jimmy 在 :24173 的位置呼叫。
- **St01 建議**：A。拆出去只剩 27 行、而且夾在同一個迴圈裡，拆了反而破壞照翻的可讀性。

### D5-Q2.（給 Steven）`RunICModeChange`＋`imgRunModeClick`（主畫面 Run Mode 圖示）誰做
- **背景**：75 行，相依都在。內容是 Real／Dummy／Tray Only 模式切換：清 HotPlate 盤資料、改 InArm 取放索引、機台內有 IC 不給換（MES1646）、寫 TestMode.Data、跑 UpdateMainOperateMode。
- **選項**：A Jimmy（模式切換＋盤資料＝機台流程）／B St01（比照 PE 模式鈕 S58、Exit 鈕 S121 由 St01 整支做），做法見 3.E1。
- **St01 建議**：A；Jimmy 手上排滿（TO_STEVEN §4 12:3x「筆電這邊在追出料臂那條鏈」）的話可以 B，但動手前要 Jimmy 點頭。

### D5-Q3.（給 Steven）`Panel42Click`（主畫面溫度 On/Off 圖示）誰做
- **背景**：58 行，大部分是守衛與客戶分支，真正做事的 `ChangeTempMode` 已經在（Jimmy）。
- **選項**：A Jimmy／B St01 做薄入口（3.E2），`ChangeTempMode` 不動。
- **St01 建議**：B（不加新機台碼，只加入口；Jimmy 點頭後做），但網頁 🌡️ 要不要現在就開放按，請 Steven 決定——按下去就是真的切加熱模式。

### D5-Q4.（給 Jimmy）`DoTrayFeedProcess` 解閘時，V912 新加的兩行 `SaveTestMode`（csystem.cpp:11362、:11882）要不要帶進來
- **背景**：RULINGS_20260926 第 26 條 Q2「底層照 906、畫面照 912」；這兩行是 V912 RogerYang 20260629 的修正（TrayEnd 區塊沒存檔，配合同段 `fMain->ModifyTester` 取代 `LastSet.iTester=`），906 沒有。D-005 原文寫「帶進來」是 S88 當時的建議，跟第 26 條衝突。先例：RULINGS_20260927 第 28 條（銦片壽命留 912，記成第 26 條的例外）。
- **選項**：A 照 906（不帶）／B 當第 26 條的例外，連 ModifyTester 一起帶進來。
- **St01 建議**：B 要 Jimmy 判斷；St01 沒有立場，只把衝突點出來，D-005 原文改成「待 Jimmy 決定」。

### D5-Q5.（給 Jimmy）`MainTempOffsetTail` 的溫控重啟（golden :28381-28383）什麼時候解
- **背景**：St01 檔裡 3 行 `#if 0`（`FileRW\Temperature.cpp:103-107`），符號都在。golden 另一處同樣重啟在 `TfMain::SetTemp`（:24598-24688 的 :24680），移植樹 SetTemp 也還沒翻（INBOX 第 77 列「溫度與模式」）。
- **選項**：A 等 Jimmy 翻 SetTemp／加熱執行緒（第 16 條）時一起解，由 Jimmy 改 St01 這 3 行或通知 St01 改／B St01 現在就解。
- **St01 建議**：A（沒有消費者之前解了也沒效果；有消費者之後就是加熱器動作，要機台端在場）。

### D5-Q6.（給 Jimmy／Steven）golden `RunICModeChange` 先清盤再擋，疑似會丟 HotPlate 的 IC 資料
- **背景**：golden :29809-29835 在 `LastSet.iRealDummy!=REALLY` 時**先** `MOT[MMPlate1/2].InitEmptyTray()`、清取放索引，:29837-29843 **才**檢查機台內有 IC（MES1646 return）。也就是 Dummy／Tray Only 模式下機台裡有 IC 時按 Run Mode：被擋下來，但 HotPlate 的盤資料已經清掉。
- **選項**：A 照翻，程式註解「疑似順序寫反，待 Jimmy 在 BCB6 確認原意」（同 Q13＝A 的作法）／B 移植樹把檢查移到前面（偏離 golden）。
- **St01 建議**：A（CLAUDE.md「照翻並註解」；改行為要 Steven 決定）。誰翻 RunICModeChange 誰加註解。

### D5-Q7.（給 Jimmy）`D:\HT9045\web\page\main-control.js` 要不要退場
- **背景**：沒有任何 html 載入它；它把 Panel42／imgRunMode／imgTester／起動模式等主畫面事件送到舊 C# 模擬器（`HTSimulatorBridge`，JSON Simulator 在 CPP 模式不啟用）。TO_STEVEN RSMODE 那列也提到它（:146 起動模式下拉）。
- **選項**：A 留著不動／B D2 做完後刪掉或搬進封存。
- **St01 建議**：B，但等 D2 做完、確定沒有頁面要參考它的事件名再動；這不是 St01 登記的檔，要 Jimmy 同意。

## 5. 順帶發現（不在 D-005，列給 ST01-E 分派）

- `FormHS->SaveCloseOpenSiteEven`（golden HS_Function.cpp:3706-3744，site 開關 event log）移植樹**只有宣告**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fHS.h:715`「GATE (Cat A)」），唯一呼叫點 `uYieldMonitoring.cpp:2352` 在 GATE (Q3a)。
  golden `mtDutOnOffMouseUp` :30266 也呼叫它。屬「讀寫檔本體」→ 可能是 St01 的，跟 S93（`Save_SiteStatusLog`，等 S72 的 myLog）一起看。
- 主畫面「工程師 Site」鈕 `sbEngSiteClick`（golden :33782-33791，`AccessLevel<=1 || SystemStart` 就 return，之後 `ShowTestHeadComp(false)`）移植樹沒有；`sbEngSite` 不在 `forms\fMain.h`
  （`forms\fMain.cpp:785-790` GATE W906-HOME-W1-ENGSITE）。`bRefreshEng`／`bRefreshEng1`（golden main.cpp:212-213 的檔內全域）移植樹也沒有，Jimmy 翻 ShowTestHeadComp1 時會碰到。
- golden `ShowTestHeadComp` 開頭 `ChangeArmSiteView(false)`（golden :9523-9561）移植樹沒有；`DrawTestSitePanel`（:23027-23174）是畫面，歸網頁。
- 906 golden 不在這台（`D:\HT9045` 下沒有 `HT9011UC_Code_V3.33.906.0_*`），這次沒有比 906／912 的 `ShowTestHeadComp1` 差異；Jimmy 翻的時候用哪一版由 Jimmy 定（INBOX 第 77 列用的是 906 行號 :22530）。

## 6. 這次的中間檔（都在 scratchpad\d005\）

- `scan.py`／`scan_out.txt`：golden 三支函式全部出現點。
- `scan2.py`／`sthc_callers.txt`：golden `ShowTestHeadComp` 呼叫者。`[LINECMT]` 標記有誤判（行尾註解），以本文為準。
- `ranges.py`：golden 函式範圍。
- `port_sthc.py`／`port_sthc2.txt`：移植樹 `ShowTestHeadComp` 41 個呼叫點（37 活、4 在 `#if 0`；「活」只表示不在 `#if 0`，不代表上游一定有人呼叫）。
- `g\`：golden V912 用到的檔的 UTF-8 副本。
