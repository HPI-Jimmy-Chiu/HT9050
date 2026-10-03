# St02 翻譯的 golden 本體：912 ↔ 906 稽核（唯讀，20260927）

> St02（Steven02，STEVEN-NB3）。github-59 20260927 要求（NB2 R92／R90）：規定是對照 golden 906，St02 的翻譯多半引 912（這台讀得到的那一棵）。
> **只盤點，不改程式**；有差的地方是給 Jimmy／Steven 的問題：V906 要 912 的行為還是 906 的。

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
＝以 906 為底、補上 912 的新增，**目標是 912**（`TESTERCOMM_PORT_LEDGER.md:182` 第 5 點；P2b atester 四段同一原則，`:210`）。

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
