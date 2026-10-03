# St02 翻譯的 golden 本體：912 ↔ 906 稽核（唯讀，20260927）

> St02（Steven02，STEVEN-NB3）。github-59 20260927 要求（NB2 R92／R90）：規定是對照 golden 906，St02 的翻譯多半引 912（這台讀得到的那一棵）。
> **只盤點，不改程式**；有差的地方是給 Jimmy／Steven 的問題：V906 要 912 的行為還是 906 的。
> ⛔ **20261003：下面表格的「裁決」欄（「版本基準（目標 912）」）已被 `RULINGS_20261002.md` 第 20 條（只做 906）與第 23 條第 6 項（已在 main 的 912 內容逐件改回 906）取代；每一項現在的結果看下一節。**

## 1002-1003 現況（以這節為準）

依據：`RULINGS_20261002.md` 第 20 條（使用者 1002 18:0x「現在分工處理只能做906 C++專案」）；例外第 20a 條（溫控照 912）、第 20b 條（HANA 照 912）、第 20c 條（ADAM-6024 照 912 保留＝第 23 條第 2 項）；第 23 條第 6 項（清理範圍，Q-A 例外不動）；Steven 1003 05:4x 的常設規則（「912 比較好就留 912」，註解寫 906 檔:行與做法、912 修正的檔:行、「#20 exception (Steven 1003 standing rule)」；St01 轉述在 `v906/steven-handoff` 分支 `docs/handoff/FROM_STEVEN.md` §3 1003 05:36）——**常設規則跟第 20 條不同，在 NIGHT_REPORT §0 第 78 項等 Jimmy，他回之前不做新的 912 搬移**。

St02 的清理是四張 MR，都在筆電第 50 批、**還沒進 main `577c41c5`**：!130 `aba800e9`（`v906/st02-c912-1`）、!132 `b2ab72d1`（`-c912-2`，疊在 !130 上）、!134 `ef721c80`（`-c912-3`，疊在 !132 上）、!136 `02fe28f0`（`-c912-4`，疊在 !134 上）。人工審核編號在 `v906/steven-handoff` 分支 `docs/handoff/ST02_HUMAN_REVIEW_20260930.md`。

| # | 項目 | 結果 | MR（commit） | 人工審核 | 還在等 |
|---|---|---|---|---|---|
| 1① | ProcessARTMessage：Qorvo Tester Pause 等 MaxTestTime 才響 | **改回 906**：馬上響（906_0625 main.cpp:15806）；全域、G15／G22 拿掉 | !132（`660b1d32`） | B32 | Steven (a)：要不要依常設規則回到 912 |
| 1② | ProcessARTMessage：GPIB 遠端 START／STOP（204／205） | **拿掉**（906 與 `D:\GPIB9045` 都沒有）：沒有分支、不回覆、只留通訊紀錄；`W906_RemoteRunStart` 保留 | !130（`2298e664`） | B30 | — |
| 2 | OnMyCopyMsg `case WM_GPIB_Program` | :17023 RESUME 取消逾時跟 1① 一起**拿掉**；:17508 `ResetHandlerArmCache`（G23 閘著）與 :17545／:17555 Dynamic PID 收 ATC 3.6 屬溫控＝**第 20a 條，不動** | !132（`660b1d32`） | — | — |
| 3① | RunTestProgram：P65 ARM-QA 重測 iLotStatus 1（0x42） | !134 先改回 906（`286b8501`），!136 revert（`1dbcd9f3`）＝**保留 V912**（常設規則；Steven 0927 #35＝A），註解有 906 main.cpp:18417-18418 與 912 main.cpp:19037-19040 | !136 | C9（B33 作廢） | — |
| 3② | RunTestProgram：AMD 執行期 `i2DIDFormat==eAMD` | **改回 906**：只有 Delta Castle（HandlerBridgeCtl.cpp:797） | !134（`b172868f`） | B34 | Steven (b) |
| 4、5 | SendMSG_CMD ×2：同 3② | **改回 906**（HandlerBridgeCtl.cpp:487／:543） | !134（`b172868f`） | B34 | Steven (b) |
| 6 | ChangeTesterConnect：「Silent run mode change」紀錄 | **保留 V912**（常設規則；只是追蹤紀錄），!136 加雙註記 | !136（`02fe28f0`） | C10 | — |
| 7 | WakeupGPIB：HANA `PrepareHANARMSConnect`（G9） | **第 20b 條（HANA 照 912）**：C10 HANA（MR !126，筆電第 49 批）解閘 | !126 | — | — |
| 8 | Timer2Timer P2f：Multi2D `iStatus[1]／[2]` | **拿掉**（沒人讀） | !134（`ef721c80`） | — | — |
| 9 | GetTesterResult | ① Murata NonTestToRBin 紀錄**拿掉**；② Barcode-CSV（含 WAR04217）**拿掉**＝906 :1038-1058；③ HANA RMS A76（T08）＝第 20b 條，維持 fail-safe | !136（`d7ee1d8b`） | B35 | ①：Steven (c) |
| 10 | ProcessTesterTimeOut：Test Timeout Skip 時重設 P65 旗標 | 跟 3① 同一套：**保留 V912**（912 atester.cpp:3826-3835，移植樹 atester.cpp:4176） | !136（`1dbcd9f3`） | C9 | — |
| 11 | cMyDB `MyDBUpdateDB`：912 的 AlarmCode 目錄 | **還沒動**：R68-MYDB（NB2 R77 交 Jimmy）；St02-M 1003 06:14 排在給 Steven 的三題之後、先問再動 | — | — | R68-MYDB |
| 12、13 | ckernel ShowRunLed／ScanPannelKey：Pause 逾時才響 | **改回 906**（ckernel.cpp :1638／:3374／:3382＝906 :738／:2127／:2136） | !132（`660b1d32`） | B32 | Steven (a) |

不在上表、也在這次清理裡的 St02 項目（`v906/steven-handoff` 分支 `docs/handoff/ST02_912_AUDIT_20261002.md` §1.2）：
- H-013（MR !79，main `3184703d`）的 MachineStatus `IsSafePLCIOInstall()` ⇒ **改回 906** `Enable_PLCSafety_IO`（906_0625 Command.cpp:7406，!130 `d9fa9a09`，人工審核 A30）；`fSecsAlarm->Visible` 三個條件 ⇒ **改回 906**（!132 `0fb778a8`）。兩者執行結果都不變。
- AOI TopBottomInspect 的 with912 ⇒ **改回 906**（!136 `f3dc245c`）。
- Data.Observer 事件記錄拆欄（912 cObserver.cpp:4023 ParseEventLogLine 對 906 :3848 CommaText）⇒ **保留 V912**（常設規則，W70＝A，!136，人工審核 C11）。
- ADAM-6024（MR !114，main `9e46491f`）⇒ **第 20c 條保留 912**（912 的 MultiTransferKG 小數修正是真 bug 修正）；`W906_ADAM_EP_LIVE` 仍關。
- 下面「不同，但不是 St02 的翻譯」提到的 N07：St02 後來另做卡 ST02-C15（MR !137／!138），照本機的 **0625_Steven**（含 `SystemStart==true`）；912 拿掉 SystemStart 的差異是給 Steven 的 Q3，沒有翻。⛔ 1003 08:2x：NB2 R177／筆電重查真的 golden 0618 **沒有** N07（只有 0625 與 V912 有）⇒ 兩張 MR 先不收進第 50 批，NIGHT_REPORT §0 第 79 項等 Jimmy（MR 開著）。這也是本稿「0625 當 0618 的替身」限制的實例：0625 比 0618 多的不只是行號。

「還在等」的 (a)(b)(c) 經 ST01-M 問 Steven（客戶專屬：Qorvo、AMD、Murata）；程式現在都是 906，Steven 說 912 比較好才補回。

## 方法與限制

- 912＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy`；906＝**`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven`，只是 golden
  `906.0_20260618` 的替身**：0618 在這台只有加密 7z（不猜密碼），而 0625_Steven 從 main.cpp :28427 之前某處起比 0618 多 78 行，
  所以這裡的 **906 行號不能當 golden 引用**；本體比對當作近似（NB2 用 0618 量過的幾個跟這裡一致）。
- 比法：函式本體（第 0 欄的定義到第 0 欄的 `}`），去掉空白後逐行比；再去掉 `//`／單行 `/* */` 註解比一次，只有註解不同算「相同」。
  大函式只看 St02 用到的那一段（OnMyCopyMsg 的 `case WM_GPIB_Program`、TfMain 建構子的 log 物件、FormDestroy、FormShow 兩處、Timer2Timer 的 P2f 段）。
- 範圍：GB P2a–P6 Handler 端、SetTestTimeOutTimer／T16、P2b atester、atester_32Site 的計時器、cMyDB P0–P3、P2c ② ckernel、ELA 的 Handler 端送出、
  ESD G3、S119。不在範圍：GPIB／RS232 引擎（H9046_32GPIB、RS232Standard 902 是別的樹）、NUMCMP（RS232Standard 的 uSocketServerClient）、
  ELA 分析器本體（另一棵 SVN）、MDB Updater（AlarmCode 目錄）。

## 相同（St02 用到的）：56

main.cpp 15（ResetForESC、ProcessHVisionConnect、SendMSG_CMD_DeviceMapSRQ、SendMSG_TestMode、CloseGpibProgram、InitialBarCodeList、
imgTesterClick、spbClearRecordClick、meShuttle2DblClick、sbLaguageClick、SendMessageToGpibProg、WakeupESD，以及建構子 :1550-1724、
FormDestroy :12508-12550、FormShow :10655／:11147-11153 三段）；atester.cpp 4（ProcessTestResult、SetTestTimeOutTimer、
InitialIndexSocketCheckTask、DoIndexSocketCheck）；cMyDB.cpp 34／35；InterfaceSYS.cpp 3（SendCommand_EventLog、SendCommand_ESD、
_SendStructMessage_Send）。

## 不同，而且在 St02 的翻譯裡（13 處）

行號「912 / 906s」＝912 ／ 906_0625_Steven。「0618（NB2）」＝NB2 用真的 golden 906_20260618 量的結果（R95 工具 #32、R77／P2f 那一輪的工具 #27）；
NB2 只量得到「註解寫了 `golden <檔>:<行>`＋函式名」的引用，St02 的檔頭表格多半不是這個寫法，所以多數項目 NB2 **沒有量到**。
「裁決」欄已更正（20260927 對帳時補上漏看的）：TesterComm 的**使用者 20260926 14:3x 裁決**「912 是新版本，可以拿 906 的補充 912 的就好」
＝以 906 為底、補上 912 的新增，**目標是 912**（`TESTERCOMM_PORT_LEDGER.md:182` 第 5 點；P2b atester 四段同一原則，`:210`）。⛔ 20261003：這條 0926 的版本基準已被 RULINGS_20261002 第 20 條取代，下表「裁決」欄只留作紀錄，結果看上面「1002-1003 現況」。

| # | 函式（段） | 912 / 906s | 差在哪 | V906 | 0618（NB2） | 裁決 |
|---|---|---|---|---|---|---|
| 1 | main.cpp `ProcessARTMessage` | :15974-16475 / :15409-15858 | ① :16371-16381 Tester Pause 逾時才響（`bPauseAlarmDelayActive`／`hPauseAlarmDelay`，RogerYang 20260626）；② :16431-16472 GPIB 遠端 START／STOP（JerryYang 20260828） | ① 照 912 live；② 照 912 翻，強制回 SETTINGNG | **不同**（R95：912 多 51 句、906 多 0 句，就是這兩件）＝同本稿 | ① 20260926 ②；② 裁決 8＝B；兩件都在版本基準（目標 912）內 |
| 2 | main.cpp `OnMyCopyMsg` `case WM_GPIB_Program` | :16481-18313 / :15864-17692 | :17023 RESUME 取消逾時計時；:17508 `ResetHandlerArmCache()`；:17545／:17555 Dynamic PID 也收 ATC 3.6 | :17023 live（G22）；:17508 gate G23；3.6 照 912 live | 沒量到 | 版本基準（目標 912）；:17023 另有 ② |
| 3 | main.cpp `RunTestProgram` | :18971-19193 / :18351-18575 | ① P65 ARM QA 重測：912 `bP65QAING && bNeedTest` → `iLotStatus=1`（0x42，Ifor 20260422）；906s `bP65QAReTest` → `iLotStatus=2`（Ifor 20260407）；② AMD：912 執行期 `i2DIDFormat==eAMD`、906s 編譯期 `#ifdef AMD_Version` | ①② 照 912 live | 沒量到（0625_Steven 比 0618 晚、仍是舊寫法，推測 0618 也一樣——未驗） | 版本基準（目標 912）；① 是**取代** 906 的行為而不是新增，Tester 看得到 |
| 4 | main.cpp `SendMSG_CMD(int)` | :18734-18826 / :18108-18203 | 同 #3② | 照 912 live | 沒量到 | 版本基準 |
| 5 | main.cpp `SendMSG_CMD(int, AnsiString)` | :18828-18847 / :18205-18227 | 同 #3② | 照 912 live | 沒量到 | 版本基準 |
| 6 | main.cpp `ChangeTesterConnect` | :12581-12778 / :12064-12257 | 多兩行 `RecordProcess("Silent run mode change …")`（CASE-FOREHOPE_NINGBO-20260920） | 照 912 live | 沒量到；日期在 0618 之後，0618 不可能有 | 版本基準（P2d 翻 912 本體） |
| 7 | main.cpp `WakeupGPIB` | :18499-18596 / :17878-17970 | :18535-18539 HANA `PrepareHANARMSConnect()` | gate G9 | 沒量到本體（R93 只對行號：906 :17878） | —（沒翻） |
| 8 | main.cpp `Timer2Timer`（P2f `SyncBridgeSettings`） | :22186-22194 / — | 送 2DID 格式時多帶 Multi2D `iStatus[1]／[2]` | 照 912 live（HandlerBridgeCtl.cpp:1174） | **不同**（工具 #27 對 0618 :21391-21525：Multi2D 是 912 新增，並註明照版本基準「是對的」）＝同本稿 | 版本基準 |
| 9 | atester.cpp `GetTesterResult` | :849-2573 / :849-2553 | ① :1048 多記 NonTestToRBin；② :1052-1067 Barcode CSV 比對（Ifor 20260511）；③ :1395-1397 HANA RMS A76 | ① live；② gate T03；③ gate T08 | 沒量到 | P2b「906 為底補 912」（`:210`） |
| 10 | atester.cpp `ProcessTesterTimeOut` | :3614-3879 / :3594-3849 | :3826-3835 Test Timeout Skip 時重設 P65 旗標 | 照 912 live | 沒量到 | P2b 同上；跟 #3① 同一套 P65 |
| 11 | cMyDB.cpp `MyDBUpdateDB` | :1812-2067 / :1812-2046 | 912 的 AlarmCode 目錄（MES0103、WAR0354→WAR0357＋WAR0359、JAM1014…） | 照 912 | **不同**（R77 工具 #27：對 0618 少 2 句、多 24 句）＝同本稿 | **既有待裁決 R68-MYDB**（NB2 R77 已交 Jimmy：A atester 也改 WAR0357／B 照 RULINGS Q2「底層照 906」改回／C 維持） |
| 12 | ckernel.cpp `ShowRunLed` | :744-748 | Pause 逾時才響（同 #1①，RogerYang 20260626） | 照 912 live | 沒量到；日期在 0618 之後 | 20260926 ② |
| 13 | ckernel.cpp `ScanPannelKey` | :2144-2148／:2158-2162 | 同 #12 | 照 912 live | 沒量到；日期在 0618 之後 | 20260926 ② |

## 不同，但不是 St02 的翻譯（只列，不處理）

atester.cpp `DoInterFaceErrorStep`（WAR0354→WAR0357）、`DoTestHeadMotor`（Index IC 掉落檢查、RTC 逐一送）、`CheckSocketSensor`（`FOR_QLE`）；
atester_32Site.cpp `DoTestY_TwoArm32Site`、`DoTestSuckTestIC_TwoArm32Site`，另有兩支 912 才有的函式（St02 在這檔只改了 `hFTestTimeOutDelay` 的 extern，:337-338）；
ckernel.cpp `ScanSystemSensor`、`ShowRunLed` 其他段（`fSecsAlarm`、N07、`IsSafePLCIOInstall`）、`ShowRunLabel`；Timer2Timer 的 N07（912 拿掉 `SystemStart==true`）、`Off_lineDisplay`、`fAGV` 段。

## 對帳結論（20260927，NB2 R95 之後）

- NB2 R95 說「St02 的檔只引到 1 支不同（ProcessARTMessage）」，是因為工具 #31／#32 只認 `golden <檔>:<行>`＋函式名的引用；
  St02 的檔頭表格（例：`HandlerBridgeCtl.cpp` `:18965-19193 RunTestProgram`）不是這個寫法，NB2 R93 也註明了。
  NB2 **有量到**的三支（#1、#8、#11）跟本稿一致：對 0618 也是不同。其餘 10 支 NB2 沒量到，不能拿來縮小清單。
- 本稿原本寫「沒有裁決」的 7 項，對帳時發現**漏看了使用者 20260926 14:3x 的版本基準裁決**（TesterComm 以 906 為底補 912、目標 912；P2b 同）：
  - #2（ATC 3.6）、#3①②、#4、#5、#8、#9①、#10 ⇒ **都在這條裁決內**（V906 照 912 是被要求的），不是新問題。
  - #11 MyDBUpdateDB ⇒ **已經是 R68-MYDB**（NB2 R77 交給 Jimmy），不另開。
- ✅ **#3① decided A, Steven 20260927**：P65 ARM-QA 重測維持 912 行為（iLotStatus 1），算在 20260926 14:3x 裁決內，不改程式（progress-st02 #35）。
- 所以 #35 **沒有剩下新的題目**。唯一值得請 Jimmy 明講的是 #3①：P65 重測送給 Tester 的 iLotStatus（912＝1、906＝2）是**取代** 906 的行為、不是新增，
  而且 Tester 看得到——版本基準的字面是「補 912 的新增」，這一項是否也算在內，確認一句就好。
- 決定之前不改程式。
- 本稿的 906 行號是 0625_Steven 的；0618 的行號等 golden 讀得到（#34）或 NB2 的工具。

## RULINGS_20261003 第 1 條：保留 912 的地方（兩邊行號都註明；St02 自己判斷）

| 項目 | golden 0618（906 的做法） | V912（保留的做法） | 為什麼算修正／比較好 | 程式 |
|---|---|---|---|---|
| C16 SECS Use Die Force | uHGemHT9045_SV.cpp:387 SV 2022＝Use Die Force（BOOLEAN），同時 uHGemHT9045_EC.cpp:158 EC 2022＝The no of pins on die（INT_4）：同一個號碼兩樣東西 | uHGemHT9045_EC.cpp:158 EC 2022＝Use Die Force（BOOLEAN，可由主機改）、:161 EC 2025＝pin 數；沒有 SV 2022 | Steven 1003 Q83＝A；0618 的號碼重複是缺陷，主機查 2022 拿到哪一個看是問 SV 還是 EC | `SECSGEM/uHGemHT9045_EC.cpp:569／:571`、`uHGemHT9045_SV.cpp:790`（筆電的行，同一行）；SecsCatalogue 計數 772 → 771；skill 的 ec_table.csv 與 SVID-ECID 對照表 |
| W71 Qorvo Tester Pause 蜂鳴 | main.cpp:15806 收到 Tester Pause 就 `bTesterPauseMusic=true`（立刻響）；Alarm Reset 只清掉、不再響（ckernel.cpp:738／:2113／:2121，0618 原文核過） | main.cpp:16371-16381 先不響、等 Max Test Time（hPauseAlarmDelay）才響；Alarm Reset 時若還在 Pause 就重新計時（ckernel.cpp:744-748／:2144-2148／:2158-2162）；RESUME／Cancel 清掉（main.cpp:17023） | Steven 0927 W8＝A 選 912；RogerYang 20260626 的修正：測試機的正常 Pause 在 Max Test Time 內不該吵，逾時才叫人；iMaxTime≤0 時跟 906 一樣立刻響 | `HandlerGpibMsg.cpp`（St02）Pause／RESUME 兩段＋閘門登記 G15／G22；`ckernel.cpp:1638／:3374／:3382`、`cmydef.cpp`／`cmydef.h` 兩個全域、`forms/fLotInfo.cpp` Cancel 那一行＝!132 之前的原文（一字不差，讓 !137 之後合得乾淨） |
| W72 AMD 的 Site Map／ATC Type | main.cpp:18158-18168／:18210-18220／:18454-18464 `#ifdef AMD_Version`（906 建置沒定義：MachineType.h:49 註解掉、USERDEFINES 只有 `_DEBUG;_VER6;`）`#else` 只有 Delta Castle 才送 | main.cpp:18785／:18834／:19076 `if(TestIF.iGpibMode==InterfaceType_Delta_Castle \|\| TestIF_File.i2DIDFormat==eAMD)`（Ifor 20260717「不使用define方式處理，改用TEST IF 設定判斷」） | Steven 1003 W72＝A；把只能靠編譯旗標切的 AMD 行為改成看配方設定，AMD 2DID 格式的配方不用另編一版 | `TesterComm/Handler/HandlerBridgeCtl.cpp:487／:543／:797`（St02）；撤銷 b172868f（!134）；ctest St02_Keep912 [2] |
| W73 Murata 2DID NG 不測 | atester.cpp:1044-1048：跳過測試、`iBinData=iTestBinCount`，不寫生產紀錄 | atester.cpp:1048 再加 `TestSocket.PordRec[i][j].AddTestResultRecord(iTestBinCount, "NonTestToRBin");`（兩個參數：SBin＝這串字） | Steven 1003 W73＝A；這顆沒測的 IC 也記進生產紀錄、算進批次 E1 | `atester.cpp:1114`（St02 的行）＋檔頭 :813-:818／:898-:899；撤銷 d7ee1d8b（!136）的 Murata 那一行，Barcode CSV 那段仍不翻；ctest St02_Keep912 [3] |
