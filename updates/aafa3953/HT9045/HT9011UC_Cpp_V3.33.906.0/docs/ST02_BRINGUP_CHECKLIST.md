# St02 上機驗證清單（V906 C++ 移植：St02 已推的工作）

| 項目 | 內容 |
|---|---|
| 版本 | 分支 `v906/steven-gpib-widget`，遠端 head `db863df4`（20260928 14:45；MR !3 https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/merge_requests/3）。這一棵已含 main `1818cfa4`：St01 review6 到 `0bca1318` 的批次（Q9／Q24／observer.get 權杖、Speed／TS-7 伺服器半邊 `70aa17e8`、Configuration `64ade3b7`、關頁 API `5b73d905`）都在裡面。**上機只要這一個 exe**（例外見 0.2） |
| 誰寫的 | St02-E helper（STEVEN-NB3），20260928 07:12 初版；14:5x 依 0928 全部推送更新（分支 `v906/steven-bringup-0928`）；St02-E 審過才推 |
| 狀態 | **尚未上機驗證**。STEVEN-NB3 只編譯、不執行（每次推送前兩組態 0 errors）；ctest 由 St01 代跑。下面所有「預期」都是從程式碼、帳本、計畫與 commit 訊息抄來的，沒有一項在真機看過 |
| 0928 下午變動 | 原第四部分的 4-1～4-6 都已接上（搬到安全段與第一段）；新增 S-1～S-3、1-10～1-13、2-13、2-14；3-3／3-4 改寫（★W36＝C：模擬版也真的連 FTP）；風險第 2、8 項已解 |
| 移植樹／網頁 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`／`D:\HT9045\web\` |
| golden | 906＝`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\`（寫作 906_0625_Steven）；912＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`；舊分析器 Rev891＝`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\`；RS232 golden＝`D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410\` |
| 行號 | V906 程式碼、網頁與帳本的行號一律以 `db863df4` 為準（0928 下午逐一重新核對；`ELA_PORT_LEDGER.md` 第 875 行以後的行號跟初版不同） |

---

## 0. 怎麼用這份清單

### 0.1 項目的狀態與寫法

- **已推・會運作**：在 gpib-widget `db863df4` 上，編出來就會跑。照安全段、第一～三段驗。
- **已推・不會運作（等認領）**：本體已推，但呼叫它的那一行還在等別人的檔認領，現在上機什麼都看不到。放在第四部分（現在只剩 4-7）。
- **未推**：只在 St02 本機分支、等 Steven 裁決。放在第五部分，**不是步驟**。
- 每一項有「預期」（該看到什麼）與「還原」（把機台放回原狀的設定或檔；沒有東西要還原就寫「不用」）；0928 下午新增或改寫的項目另有「失敗時」（要留哪個 log、哪個畫面）。其他項目失敗時一律先留：wb_serve 主控台最後 30 行、當天 `D:\HT9045_Log\SaveEventLog\HANDLER LOG_<HandlerID>_yyyy_mm_dd.csv`、畫面截圖。
- 「待 Steven 第 N 題」指交接分支 `v906/steven-handoff` 的 `docs/handoff/ST02_QUESTIONS_20260928.md`；「0927 第 N 題」指同資料夾的 `ST02_QUESTIONS_20260927.md`；★W36～★W62 是 20260927 起的待裁決編號（白話版在 `D:\docs\ChangeLog\CHANGES_20260927_Steven02.md` §11 與 `D:\docs\ChangeLog\CHANGES_20260928_Steven02.md` §11、§11h）。

### 0.2 用哪個 build

| 名稱 | 怎麼建 | exe 位置 | 跟本清單有關的差別 |
|---|---|---|---|
| 模擬版（sim） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\build.bat`（PowerShell 用全路徑呼叫） | `D:\HT9045\Obj\V906\build\wb_serve.exe` | ELA 的 FTP **跟出貨版一樣用 WinINet 真的連線**（★W36＝C，`EventLogAnalysis\ElaFtp.h` 第 35～44 行），但 N25-3／4／5 不在 00:00 自動跑（照 golden `#ifndef SOFT_SIMULTE`，`EventLogAnalysis\ElaSchedule.cpp` 第 497～498、509～511 行），手動「立即執行」會真的連；Tester 切換的 D2 不擋（`TesterComm\Handler\HandlerTesterConnect.cpp` 第 61 行）；開機不跑 InitDIOStstus（B2，`cDIOStatus.cpp` 第 98～100 行） |
| 出貨版（ship） | 先設 `V906_BUILD_DIR=build_ship`、`V906_CMAKE_ARGS=-DW906_NO_SOFT_SIMULTE=ON` 再跑同一支 build.bat | `D:\HT9045\Obj\V906\build_ship\wb_serve.exe` | ELA 的 FTP 用 WinINet；N25 半夜自動跑；D2 會擋；開機跑 `InitDIOStstus(false)` |

- 在 worktree 裡建時，exe 在 `<worktree>\Obj\V906\build\`（或 `build_ship\`），規則在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\build.bat` 第 68～78 行。
- **一棵樹就夠**：gpib-widget `db863df4` 含 Q9／Q24／observer.get 權杖（`64e2c048`、`7f0c24b1`、`94f16127`）與 St01 的伺服器半邊（`70aa17e8`、`64ade3b7`、`5b73d905`）（`git merge-base --is-ancestor` 查過，20260928 14:5x）。
- **唯一例外**：1-9 最後一步（Speed 滑桿放開送 `change` 事件）要 St01 分支 `v906/steven-cbridge-review6` 的伺服器（`7e1785dc`、`29a13bdb`，還沒進 main）加 `--root <gpib-widget 那份的 web 資料夾>`；St01 的 TS-1 佇列 `ht9045_temp_set_ts1.js` 也只在那棵。
- **不要**把任何 V906 的 exe 複製進 `D:\HT9045\EXE\`（那是 BCB6 量產建置的輸出）。

### 0.3 啟動方式與環境變數

- wb_serve **不帶參數**啟動；網頁在 `http://127.0.0.1:8045/?src=ws`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 第 39 行）。
- `--dry` **已退場**：帶了會印一段說明並以 2 結束（同檔第 3683～3684 行）。保護方式是「先備份 → 驗證 → 沒問題才刪備份」，不是旗標。
- 會用到的環境變數（都只在啟動 wb_serve 的那個主控台設）：
  - `HT9045_ELA=0`：整個 Event Log 分析器不啟動（`/api/ela` 回 503，開機不會寫 config.ini／JAM0000.dat）。
  - `HT9045_TCPCMD_SIM=1`：7016／7017 兩個 TCP 伺服器留在模擬，不開真的 socket。
  - `HT9045_TESTERCOMM=0`：Tester 通訊整個不裝（7016／7017 物件仍會建，按 Start 不會當掉）。⚠ **P8 B4 之後**，這樣開的 wb_serve 在「GPIB／RS232 配方＋On-Line」「TCP/IP 配方＋Off-Line」「TTL 配方＋`TTL_CARD_TYPE` 2 或 3」按 START 會照 golden 被擋（跳 `GPIB no execute!!`／`RS232 no execute!!`／`TTL RS232 no execute!!`，`WebStart.cpp` 第 1722～1757 行）。要按 START 就不要設它，或把 GPIB／RS232 配方切到 Off-Line（見 S-2）。D1～D7 的規則不受它影響（`TesterComm\Handler\TesterCommWiring.cpp` 第 97 行在退出判斷之前）。
  - 上機驗證時**不要**設任何 `W906_*` 測試縫（例如 `W906_INIDATA_ROOT` 設了 wb_serve 會拒絕啟動）；唯一例外是 2-10 可以先用 `W906_PWBOOK_PATH` 指到複本練習。

### 0.4 總表

| 編號 | 項目（白話） | 代號 | 狀態 | 段 |
|---|---|---|---|---|
| S-1 | ini 檔物件不再「刪了還拿來用」（看不到變化才對） | common.cpp INI-UAF | 已推・會運作（0928 新） | 安全 |
| S-2 | START 擋「找不到橋接／版本錯」；GPIB／RS232／TTL 第一次連線 | P8 B1～B5 | 已推・會運作（0928 新） | 安全 |
| S-3 | Tester 鈕切 On／Off-Line 的規則（權限、有 IC 不准切、I27、2D_SORT、ASM、強制回 Operator） | GB P2d D1／D2／D3／D4／D6／D7 | 已推・會運作（原 4-1、4-2） | 安全 |
| 1-1 | 開機、Event Log 分析器啟動 | ELA P3 | 已推・會運作 | 一 |
| 1-2 | 各種 log 真的寫出來（事件、HANDLER LOG、加熱器） | P4／W7 | 已推・會運作 | 一 |
| 1-3 | 沒拿控制權的瀏覽器也看得到三頁資料 | Q2 | 已推・會運作（在 main） | 一 |
| 1-4 | Observer 的 Yield 分頁按鈕要控制權 | observer.get 權杖 | 已推・會運作（已進 main） | 一 |
| 1-5 | Observer 的事件記錄檢視頁欄位不變 | W15（cObserver） | 已推・會運作 | 一 |
| 1-6 | 事件記錄頁（eventlog.html）與自動工作分頁 | ELA R5／R6、O10 | 已推・會運作 | 一 |
| 1-7 | Tester 介面設定頁的新畫面（不存檔） | Q41 A 段 TI-1／TI-3／W6 | 已推・會運作 | 一 |
| 1-8 | 其他設定頁的畫面連動（不存檔） | Q41 B 段 | 已推・會運作 | 一 |
| 1-9 | 速度設定頁（不存檔）＋滑桿放開送事件 | Q41 SP-1～5；Speed form.event | 已推・會運作（滑桿事件那一步要 St01 分支） | 一 |
| 1-10 | Motion View 畫主畫面托盤 | S118 | 已推・會運作（原 4-3） | 一 |
| 1-11 | 溫度設定頁：基準點點數、Boost 下限、Airstream 夾值（不存檔） | TS-7、★W39 TS-8、Q52 TS-9 | 已推・會運作（原 4-5） | 一 |
| 1-12 | Configuration 頁 D46 上下鍵與勾選框走伺服器（不存檔） | CC-E2／CC-E7 | 已推・會運作（原 4-6） | 一 |
| 1-13 | Tester 介面設定頁「不存就關」的尾段 | Q41 (d) | 已推・會運作（原 4-4） | 一 |
| 2-1 | 各設定頁存檔後重讀一致 | Q41 A／B 段 | 已推・會運作 | 二 |
| 2-2 | Tester 遠端改溫度 offset，寫進配方 | W9 | 已推・會運作（在 main） | 二 |
| 2-3 | Auto Retest 的 Tester 廠牌跟配方走 | W1／W1b | 已推・會運作（在 main） | 二 |
| 2-4 | 事件記錄頁的查詢（計數規則改了） | ELA W15／W18／W19、S128 | 已推・會運作 | 二 |
| 2-5 | 手動 Save Summary | ELA P7a | 已推・會運作 | 二 |
| 2-6 | 四個「Save」按鈕存成 Excel（.xlsx） | ELA P7 做法 D（Steven 0928 第五題＝D） | 已推・會運作 | 二 |
| 2-7 | 自動存報表開關與定時報表（O06-4 跟 [O06-8]；N10-3 每天指定時間） | O10、R2、R5、★W43＝C、★W44＝B／W44-2 | 已推・會運作 | 二 |
| 2-8 | SPIL／999 每天複製產品 log | W13（N17） | 已推・會運作 | 二 |
| 2-9 | 功能類 log（O06 生產 log、Greatek 歷史檔、2D／ASM／QtyData） | P4-D2／P4-D4／W7 | 已推・會運作 | 二 |
| 2-10 | login.dat 帳號新增／刪除／修改 | Q9 | 已推・會運作（已進 main） | 二 |
| 2-11 | 權限等級存檔照 golden 寫 login.dat | Q24 | 已推・會運作（已進 main） | 二 |
| 2-12 | Jam Code 匯出與 JAM0000.dat 補鍵 | S127／S128 | 已推・會運作（在 main） | 二 |
| 2-13 | Jam Code 第二個勾選框（Include MTBF／Include MTBA (Analyzer)） | ★W45 | 已推・會運作（0928 新） | 二 |
| 2-14 | 每小時一列 TimeData＋OEE 分頁的灰色「關機」 | ★W48 | 已推・會運作（0928 新） | 二 |
| 3-1 | 非 Greatek／TeraPower／TeraProbe 按 Start 不會當掉 | W10 修正 `c102083d`＋`b2a7349c` | 已推・會運作 | 三 |
| 3-2 | TCP 指令伺服器 7016／結果伺服器 7017（14 項） | W10 | 已推・會運作（只在 956／967／804） | 三 |
| 3-3 | ELA 的 FTP 手動上傳（兩種組態都真的連線） | ELA R4／W22、★W36＝C | 已推・會運作（改寫） | 三 |
| 3-4 | ELA 的 FTP 半夜自動上傳（出貨版，要 Steven 同意） | ELA R4／R5／W22 | 已推・會運作 | 三 |
| 3-5 | SECS 遠端改溫度 offset（選做） | W9 G34 | 已推・會運作（在 main） | 三 |
| 4-7 | 崇越（868）批結束的 OEE／警報報表 | ELA R3（N34） | 已推・不會運作 | 四 |
| A-1 | ESD 程式開著時 IonBar 上電序列 | ESD G5 | 已推・會運作（在 main） | 附錄 A |
| A-2 | 每次測試開始記一行 socket ID | W11 | 已推・會運作（在 main） | 附錄 A |
| A-3 | Castle Tester 的 READTEMP／READDAQ 回覆字串 | GB P1 | 已推・會運作 | 附錄 A |

---

## 備份段：開機前總備份（每一次開 wb_serve 之前都做）

**wb_serve 一開機就會寫檔**，所以「第一段唯讀」只是指不主動改設定、不按存檔；開機本身會寫下面這些，必須先備份。

### B.1 開機本身會寫的檔（照 golden）

| 檔 | 誰寫、什麼時候 | 出處 |
|---|---|---|
| `D:\HT9045\system\Gerneral.ini` | LoadMachineConfig 補缺的鍵；ELA 開機讀設定時沒有 `[Version] Machine ID` 也會寫 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 第 53～58 行；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ELA_PORT_LEDGER.md` 第 791～793 行 |
| `D:\HT9045\config\config.ini` | ELA 開機的 EL_UPDATE_PARAMETER 把缺的鍵寫回（#17＝A，照 golden CheckAndReadIniData） | 同帳本第 763～764、791 行 |
| `D:\HT9045\Error\English\JAM0000.dat` | ELA 開機排的「今天」查詢把缺的 unit／碼寫回 | 同帳本第 792 行 |
| `D:\HT9045\Error\AlarmCodeList.txt`、`D:\HT9045\Error\English\JAM31nnn.dat`、`D:\HT9045_Log\MDB_UpdateLog\` | MyDBUpdateDB（P3）第一次開機有新碼時整份重存、補缺的 dat；AlarmCodeList.txt 不在時先寫一份 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\CMYDB_PORT_LEDGER.md` 第 110～114、191 行 |
| 作用中配方的 `Tester.Data`（`[AutoRetest] iTesterType=1`） | W1：`CosFunction.bUseSCKART` 開、配方沒有這個鍵時，開機或換配方就寫回 1 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\TESTERCOMM_PORT_LEDGER.md` 第 331～334 行 |
| `D:\HT9045_Log\UploadFile\ElaScheduleState.ini` | ELA 排程第一次看到某個工作開著時寫 `armed=` | ELA 帳本第 1251～1254 行（R5「開機補跑」） |
| `D:\HT9045_Log\TimeData\<年>\TimeData_<年>.csv`＋HANDLER LOG 一行 | ★W48：開著就每到整點寫一列（2-14），不用按任何東西；VTEST 915／919 不寫 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Handler\TesterCommWiring.cpp` 第 138、146、196～227 行；`cMyDB.cpp` 第 485～486、649～652 行 |
| `D:\GPIB9045\system\general.ini`、`D:\RS232Standard\System\Setup.ini` 等 | Tester 橋接一起來就讀、缺鍵寫回（S-2） | `D:\HT9045\.claude\skills\ht9045-gpib-bridge\references\gb-p8-bringup-plan.md` §2.5 |
| `D:\HT9045_Log\` 底下各 log 資料夾 | 正常的 log（1-2） | CMYDB 帳本第 161～162 行 |

### B.2 備份步驟

1. 關掉所有 wb_serve、HT9045.exe（BCB6 與 V906 都關）。
2. 用現成的工具備份機台參數與作用中配方：
   `python D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\realfile_guard.py snap <標籤>`
   - 涵蓋：`D:\HT9045\system\` 的 teach.ini、tech.dat、Gerneral.ini、Mot_Table.csv、IO_Table.csv、ContactInfo.ini、ArmByLot0～2（含 _backup）、lastdata.dat、machinerecord.dat、machinerecordRealCCD.dat、MachineLife.ini；`D:\HT9045\config\config.ini`；`D:\HT9045\CurrentSetupData.txt`；`D:\HT9045\SetUp.inf`（清單寫成小寫 setup.inf，Windows 上是同一個檔）；以及 SetUp.inf 第一行指的配方資料夾 `D:\HT9045\IniData\Data\<配方名>\` 裡的每一個檔（同檔第 39～110 行）。
   - 備份放在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\_realfile_guard\`。
3. realfile_guard **沒有涵蓋**、要手動另外複製的（建議放 `D:\HT9045_Backup_<日期>\`）：
   - `D:\HT9045\system\login.dat`、`D:\HT9045\system\levelset.dat`（2-10、2-11）；
   - 整個 `D:\HT9045\Error\`（含 AlarmCodeList.txt、`English\` 的 JAM0000.dat、JAM31nnn.dat；2-13 存 Jam 設定時 golden 也寫 `D:\HT9045\Error\<語言>\<碼>.dat`，ELA 帳本第 1199 行）；
   - S-2 要接 Tester 時：P8 計畫 §2.5 那張表的檔（`D:\GPIB9045\system\general.ini`、`GpibString.dat`、`D:\RS232Standard\System\Setup.ini`）；
   - `D:\HT9045\system\lastdata_backup.dat`、`D:\HT9045\system\lastdata_backup2.dat`（lastdata 的備份檔，3-2 的 720／700／702 前一起留）；
   - `D:\HT9045_Log\UploadFile\ElaScheduleState.ini`（有的話）。
4. 記下 log 資料夾的現況（不用複製，之後比對新增了什麼）：`D:\HT9045_Log\EventLogTxt\`、`D:\HT9045_Log\SaveEventLog\`、`D:\HT9045_Log\ASE log\`、`D:\HT9045_Log\Heater_On_Off_LOG\`、`D:\HT9045_Log\UploadFile\`、`D:\HT9045_Log\TCPIP_Log\`、`D:\HT9045_Log\TimeData\`、`D:\GPIBLOG\`、`D:\RS232Log\`、`D:\RMS\`、`D:\MTBF_Summary\`。

### B.3 驗完之後

- `python D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\realfile_guard.py check <標籤>`：逐檔印出差異（same／只多了幾個鍵／CHANGED＋diff）。把輸出存下來跟本清單的預期比。
- 要還原：`... realfile_guard.py restore <標籤>`，再把 B.2 第 3 點手動複製的檔放回去。
- 確認沒問題才 `... realfile_guard.py drop <標籤>` 刪備份。

---

## 安全段：開機後、動任何頁面之前（S-1～S-3）

> 備份段做完才開始。S-1 不用 Tester；S-2 第 1 步不用 Tester，之後要接真的 Tester；S-3 的 D2 要出貨版才看得到。

### S-1 ini 檔物件不再「刪了還拿來用」（common.cpp；看不到變化才對）

- 狀態：已推・會運作（`4d3468a3`，筆電同意的認領）。`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp` 第 587、622、1600 行：CloseIniFile／CloseIniFileMem／CloseGeneralIniFile 刪掉物件後把指標清成 nullptr。golden（906_0625_Steven common.cpp:336-343、:358-365、:1414-1421）不清，V906 原本照抄；每一次 ReadIniData／WriteIniData 都經過這三個函式。
- 以前壞掉的樣子：St01 09:0x 代跑 ctest TesterComm_TcpCmdServer 時 SIGSEGV，堆疊 `TIniFile::ValueExists ← CloseGeneralIniFile ← ReadTechData`（commit `b12ab375` 訊息）；OpenIniFile 遇到同一個檔名還會重用已刪掉的物件（同檔第 561～572 行）。wb_serve 有同樣的風險，可能的樣子（推論，沒在 wb_serve 看過）：存檔或 7016 的存檔指令之後 wb_serve 自己消失，或 ini 寫入沒進檔。
- 步驟：
  1. 做 2-1 時，Configuration、Setup.TesterIF、Setup.Speed 各連續存檔兩次；956／967／804 機台做 3-2 第 10 項時一起看。
  2. 整段上機期間注意 wb_serve 有沒有自己關掉。
- 預期：**看不到任何變化**：不當掉；存的值重開頁一致；`realfile_guard.py check` 只列出改的那幾個鍵。
- 失敗時：留主控台最後 30 行；Windows「事件檢視器 → Windows 記錄 → 應用程式」裡 wb_serve.exe 的 Application Error（例外代碼、錯誤模組、時間）；記下最後按的是哪一頁哪個鈕。
- 還原：不用（程式修正，沒有設定）；存過的檔照 B.3 restore。

### S-2 START 擋「找不到橋接／版本錯」；GPIB／RS232／TTL 第一次連線（P8 B1～B5）

- 狀態：已推・會運作（`35d17d44`，筆電五項都同意）。完整的上機步驟（GPIB 查詢 → 一個 SOT／EOT → 32 站 BINON；RS232 同順序；TTL `WINIT` → `WSOTS`；每步的預期、失敗看哪裡、怎麼停）在 `D:\HT9045\.claude\skills\ht9045-gpib-bridge\references\gb-p8-bringup-plan.md`，**照它做，這裡不重抄**。
  - ⚠ 那份計畫 §1 的 B1～B5 與 §8 第 5 點寫的是還沒補的樣子（寫於 `0c4b8e25`）；B1～B5 已在 `35d17d44` 補上。**還沒解的是 B6（網頁的 On／Off-Line 入口，見 S-3）與 B7（`POST /api/testercomm/...` 沒有權限閘）**。
- 補上的五項，上機在哪裡看：

| # | 改了什麼 | 程式（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`） | 在計畫哪一節看 | 預期 |
|---|---|---|---|---|
| B1 | `TfMain::GetTTLState` 又會把 `MSG_CMD_State_TTL`（48 碼 TTL 參數）送給橋接 | `Command.cpp` 第 11094～11097、11260～11262 行；`TesterComm\TesterWndSeat.h` | §7.2 | TTL 板收到 `WINIT` |
| B2 | 開機的 `InitDIOStstus(false)` 解閘（只在出貨版） | `cDIOStatus.cpp` 第 97～101 行（開機呼叫在 `tools\wb_serve.cpp` 第 3596 行） | §7.1～7.2 | `WINIT` 的 Bin 模式、SOT 邏輯、SOT 寬度是 DIO 檔的值、不是全 0；開機時 TTL 輸出線的狀態跟 BCB6 一樣 |
| B3 | `TTL_CARD_TYPE==2` 時 `TTLRS232VerCheck=7071601`（golden） | `database.cpp` 第 1224～1226 行 | §7.2 | 一塊板、韌體 07071601 以上不再跳 `TTL RS232 Version Error!` |
| B4 | 找不到橋接或版本錯時 START 照 golden 拒絕（golden `bFind`） | `WebStart.cpp` 第 1721～1773 行 | 本項步驟 1 | 見下 |
| B5 | 測試逾時後按 Skip／Retry 又會送 `@WCSOT` | `csystem.cpp` 第 28765～28795 行 | §7.4 | RS232 log 有 `@WCSOT00000000`（板子帶站號時 `@00WCSOT00000000`；兩塊板是 `@00…` 與 `@01…` 兩行） |

- 步驟：
  1. **先驗 B4（不用接 Tester）**：GPIB 配方、On-Line（切法見 S-3），設 `HT9045_TESTERCOMM=0` 開 wb_serve（橋接不會起來＝golden 的「找不到」），按 START。RS232 配方再做一次。
  2. 關掉 wb_serve、清掉 `HT9045_TESTERCOMM`，照 P8 計畫 §3 起逐節做；TTL 機台做 §7（B1、B2、B3、B5 都在這一節）。
- 預期：
  - 步驟 1：跳 `GPIB no execute!! Please Check GPIB !!`（RS232 配方：`RS232 no execute!! Please Check RS232 !!`；TTL 配方＋`TTL_CARD_TYPE` 2／3：`TTL RS232 no execute!! Please Check TTL RS232 !!`），機台不動（`WebStart.cpp` 第 1728～1736、1752～1756 行）。版本錯的訊息是 `GPIB Version Error!! … Must be V<n>.00`、`TTL RS232 Version Error!! … Must be 7071601`（第 1739～1747、1759～1763 行）。
  - 步驟 2：照 P8 計畫各節的「預期」。
- 失敗時：照 P8 計畫 §2.6 留 log：GPIB＝先在 testercomm.html 的 GPIB 分頁按 Save Log，再拿 `D:\GPIBLOG\Log\YYYY_MM\`；RS232／TTL＝`D:\RS232Log\LOG\YYYY\MM\RS232_Log_YYYYMMDD HH.log`（TTL 先勾 Show Log）；Handler＝當天 HANDLER LOG 與警報代碼。步驟 1 沒被擋時截圖配方的 `[Mode] Tester Type` 與主畫面的 On／Off-Line。
- 還原：關 wb_serve；放回 P8 計畫 §2.5 備份的 general.ini／Setup.ini／配方；`HT9045_TESTERCOMM` 用完清掉。
- 注意：B7 還沒解，上機期間只讓在場的人開 testercomm 頁（Manual Start／Manual Test 一按就打到設備，P8 計畫 §8 第 4 點）。

### S-3 Tester 鈕切 On／Off-Line 的規則（GB P2d D1／D2／D3／D4／D6／D7；原 4-1、4-2）

- 狀態：已推・會運作。呼叫端 `c4700fd6` 改 Jimmy 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp` `TfMain::ChangeTesterConnect`（宣告第 1056 行；D1 第 1113 行、D2 第 1115 行、D3 第 1134 行、D4 第 1148 行、D6 第 1191 行、D7 第 1261 行；golden 原文留在各行下面的 `#if 0` 對照）。本體：D1／D2／D3／D6／D7＝`TesterComm\Handler\HandlerTesterConnect.cpp`，由 `TesterComm\Handler\TesterCommWiring.cpp` 第 97 行 `W906_TesterConnectRulesInstall()` 裝上（在 `HT9045_TESTERCOMM=0` 的退出之前）；D4＝`WebLogin.cpp` 第 531～563 行 `W906_WebLoginForceOperator`（第 565 行開機時裝上）。照 golden 906_0625_Steven main.cpp:12064-12257。
- 怎麼按 Tester 鈕：**現在沒有網頁按鈕會送這個動作**（P8 計畫 §1 B6：`D:\HT9045\web\page\main-control.js` 第 139～141 行送的是舊的 `request('TfMain::imgTesterClick', …)`，而且沒有任何頁面載入這支 JS）。上機時在任一個有載入 `ht9045_recipe_client.js` 的頁（例 `Status.Security.html`）按 F12 → Console，貼這一行（下面「按一次」都是指送它一次）：

```
HT9045Recipe.rawCmd('control.acquire').catch(function(){}).then(function(){ return HT9045Recipe.rawCmd('act.main.testerConnect', { value: '{}' }); }).then(function(r){ console.log(r); }, function(e){ console.log(e); })
```

  回覆裡有 `before`／`after`（ON_LINE／OFF_LINE／2D_SORT）、`modeChanged`、`changeTesterConnectReturn`（1＝被規則擋下），或 `guard`（`system-running`、`not-authorized`…）（`JsonBridge\actions\MainTesterConnect.cpp` 第 163～201 行）。
- 前置條件：機台停止；`realfile_guard.py snap tc`（`bLastSetInSetUpFile` 開時切換會寫配方 `TestMode.Data`，`forms\fMain.cpp` 第 1230～1234 行；D7 會改 Run Start Mode）；另存 `D:\HT9045\system\login.dat`、`levelset.dat`（照說不會動）；用 Engineer 或 Supervisor 登入。D2 要**出貨版**；D3 要 I27（`bI27_ManualSortMode`）開；D6 要 Test IF 的 `bSortingBy2DIDList`；D7 要 I21 ASM＋`bUseAutoSiteMapping`＋`bDutOnOffNeedASM`。
- 步驟與預期（MES 碼寫進 EventLogTxt 與 HANDLER LOG，見 1-2）：

| # | 做什麼 | 預期 |
|---|---|---|
| 1（D4） | 非 I27 機台，Off-Line 時按一次 | 變 On-Line（MES2157 `Change to On_Line`、MES2147）；權限降回 Operator：5 級權限的非 KYEC 客戶顯示 Open，其他顯示 Operator 並多一筆 MES2140 `======== Operator login ========`；登入鈕變回 Login；溫度欄位鎖住；主控台一行 `[WebLogin] D4 Off-Line -> On-Line: forced back to …`（`WebLogin.cpp` 第 561 行） |
| 2 | On-Line 時再按一次 | 變 Off-Line（MES2155 `Change to Off_Line`、MES2146）；不做 D4 |
| 3（D2，出貨版） | Off-Line、機台裡留一顆 IC 或托盤，按一次；Clean out 之後再按 | 第一次跳 MES1646，訊息列出哪個位置有 IC（`ChangeTesterConnect :…`），不切換，回覆 `changeTesterConnectReturn` 是 1；Clean out 後可以切。模擬版照 golden 不擋 |
| 4（D2，SECS） | 有 IC 時由 SECS 主機切一次（`SECSGEM\uHGemHT9045.cpp` 第 3431 行那條路） | 靜靜拒絕，沒有訊息框 |
| 5（D3，I27 機台） | Off-Line 按一次；再按一次 | 第一次＝MANUAL MODE（MES2156、MES2145 `XXXX  Tester MANUAL MODE  XXXX`），`after` 仍是 OFF_LINE、**不做 D4**；第二次＝On-Line＋D4 |
| 6（D6） | 配方開 bSortingBy2DIDList，On-Line 時按一次；再按一次 | 變 2D_SORT（MES2155 `Change to 2D_SORT`）；再按變 Off-Line |
| 7（D7） | ASM 三個開關開，Off-Line → On-Line | 多 site：主畫面 Run Start Mode 變 Auto Site Map（VTEST 只在 bAutoSiteMappingOpenSite 開時）；單 site：ContinuStart 或 ContinuRetest，事件多一行 `Silent run mode change by iTester On/Off Line : …` |
| 8（D1） | 權限不夠的帳號按一次 | 回 `guard: not-authorized`（按鈕先查 `fSecurity->Insufficient(8)`，D1 在這條路沒有另外的差別）；SECS 遠端切照樣通過 D1 |

- 失敗時：留 Console 的回覆、主控台最後 30 行、當天 HANDLER LOG（找 MES2155／2156／2157／2145～2147／2140／1646）、主畫面登入名稱與 Run Start Mode 的截圖；D2 沒擋時記下組態（sim／ship）和哪個位置有 IC。
- 還原：再按到原本的模式；重新登入；Run Start Mode 在主畫面改回；`realfile_guard.py restore tc`。
- 依賴：Steven 0928 第十四題 D4＝A（全部客戶）、第十六題 D1＋D2＝A；I27（0928 第四題）：D3 已接上，現在照 golden；B6（網頁入口）待定。

---

## 第一段：唯讀檢查（開頁、看值、看 log 有沒有出來）

> 這一段不按存檔、不改設定。開機會寫的檔已在備份段備份。

### 1-1 開機、Event Log 分析器啟動（ELA P3）

- 狀態：已推・會運作（gpib-widget；開機接線 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 第 4166 行，本體 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaService.cpp` `W906_ElaStart`）。
- 前置條件：模擬版或出貨版都可；任何客戶碼；備份段完成；沒設 `HT9045_ELA`。
- 步驟：
  1. 在 PowerShell 啟動 `D:\HT9045\Obj\V906\build\wb_serve.exe`（出貨版換 `build_ship`），看主控台。
  2. 瀏覽器開 `http://127.0.0.1:8045/api/ela`。
  3. 關掉 wb_serve，設 `HT9045_ELA=0` 再開一次，再開同一個網址。
- 預期結果：
  - 主控台一行 `[ELA] Event Log Analyzer hub started (EventlogAnalyzer Rev891.0 port; VTEST MTBF summary on; N25-3/4/5 / N10 BYFILE uploads: WinINet transport; timed jobs: ElaSchedule, gated by the auto-save switch)`；**兩種組態都寫 WinINet**（★W36＝C，`ElaService.cpp` 第 278～280 行）。
  - `/api/ela` 回 200 的 JSON；工作歷史有 `EL_UPDATE_PARAMETER: ReadConfig done`（筆電 20260927 在它的機台看過，ELA 帳本第 849～850 行）。
  - `HT9045_ELA=0` 時 `/api/ela` 回 503。
  - realfile_guard check：config.ini、JAM0000.dat 預期只多出缺的鍵／列（realfile_guard 會分出「只多了幾個鍵」與「CHANGED」；出現 CHANGED 要回報）。
- 如何還原：備份段 B.3 的 restore。
- 依賴：#17 ELA-3＝A（照 golden 寫回）；#31 G8＝A（V906 不啟動舊的 EventlogAnalyzer.exe）。不需要認領。

### 1-2 各種 log 真的寫出來（P4 cMyDB 真本體＋W7 log 物件）

- 狀態：已推・會運作（gpib-widget `06f8ef0a`、`b4712e5f`；還沒進 main）。P4 之前這些呼叫大多是替身，不寫檔；**現在約 947 個呼叫點都真的寫檔**。
- 前置條件：模擬版或出貨版；任何客戶碼；同一台不要同時開第二個 wb_serve 或 HT9045.exe。
- 步驟：
  1. 開機後照平常操作按 START、PAUSE、ALARM RESET 各一次。
  2. 讓一個會跳 MESxxxx 的事件發生（一般操作即可）；如果方便，讓一個 SECS 送出失敗或回答一個 MyMessageBox。
  3. 加熱器 relay 切換一次（開／關）；最後正常關掉 wb_serve。
  4. 一批事件多的批次時，看 tick 有沒有明顯變慢。
- 預期結果（CMYDB 帳本第 183 行、`06f8ef0a`／`b4712e5f` commit 訊息）：
  - 每按一個鍵，`D:\HT9045_Log\EventLogTxt\` 當天檔、`D:\HT9045_Log\SaveEventLog\HANDLER LOG_<HandlerID>_yyyy_mm_dd.csv` 各多一行。
  - MESxxxx 那一類（NewRecordProcess）另外在 `D:\HT9045_Log\ASE log\yyyy\mm\dd\<HandlerID>@..._EventTracker.csv` 多一行。
  - SECS 送出失敗／MyMessageBox 的回答多一行 "Exception"／"Message"。
  - `D:\HT9045_Log\Heater_On_Off_LOG` 每次 relay 切換一行 "Heater On/Off, by ..."，關機一行 "Heater Off, by Close"；關機不當掉，之後不再呼叫 HeaterLog。
  - 其他 golden log 資料夾只在寫它的功能跑時才出現（開機不會冒出一堆空檔）。
  - 每一筆都是同步寫檔（golden 也是）；tick 沒有明顯變慢。確切的時間差帳本沒寫預期值，上機時記下實際值。
- 如何還原：新增的 log 列不用還原。
- 依賴：Steven 20260927 P4 D1＝A／D2＝B／D3＝A／D4＝B、W7＝A；筆電同意共用檔清單。⚠ 見「風險」一節第 1 項（State Record 背景執行緒）；第 2 項（32-site 少一筆 WAR2206）已解，32-site 機台順便看。

### 1-3 沒拿控制權的瀏覽器也看得到三頁資料（Q2）

- 狀態：已推・會運作（`9d790ff2`，在 main；兩棵樹都有）。
- 前置條件：任一組態；兩個瀏覽器（或兩個分頁）連同一個 wb_serve，只有一個拿控制權（權杖）。
- 步驟：在沒拿權杖的那個打開 Contact CT、Counter Clear、Observer（`D:\HT9045\web\page\Data.Observer.html`）三頁看資料，再試按存檔。
- 預期結果：三頁都看得到資料；存檔被拒（commit `9d790ff2` 的上機要看）。拒絕時的確切字串帳本沒寫，上機時記下實際值。
- 如何還原：不用（沒有寫檔）。
- 依賴：S124＝B（Q2）。observer.get 在 WS 層以名稱免權杖（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.cpp` 第 1448 行），Yield 分頁那四個會改記憶體的按鈕由 wb_serve 逐個再查權杖（1-4，現在也在 gpib-widget）。

### 1-4 Observer 的 Yield 分頁按鈕要控制權（observer.get 權杖）

- 狀態：已推・會運作（`94f16127`，經 St01 `ed365c68` 與 main `8b3a07af` 進到 gpib-widget）。
- 前置條件：gpib-widget 的 wb_serve；兩個瀏覽器同時開 Data.Observer；只有一個拿權杖。
- 步驟：
  1. 沒拿權杖的那個看 Yield 分頁的資料。
  2. 在它的 Yield 分頁按 Site／Max／Min 設定（yieldSite／yieldMax／yieldMin）。**Yield Clear（yieldClear）會清掉 bin 歷史（記憶體），生產中不要按。**
- 預期結果：沒權杖的也讀得到 Yield 分頁；它按 Yield 分頁的按鈕時，St01 的頁面會先拿權杖（acquire → get → release）再做（commit `94f16127`）。如果頁面沒拿到權杖就送，伺服器回 not-operator、什麼都不改（`tools\wb_serve.cpp` 第 5261 行）。
- 如何還原：不寫檔；Yield 設定按錯了就在頁面改回原值。
- 依賴：S124＝B（ST01-M 16:25 認領 FROM_STEVEN §1 `240630bb`）。

### 1-5 Observer 的事件記錄檢視頁欄位不變（W15，cObserver 共用切欄）

- 狀態：已推・會運作（`a9b93d61`；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp` 第 110、1351～1385 行，行數不變）。
- 前置條件：任一組態；`D:\HT9045_Log\EventLogTxt\` 裡找一個有「引號包住的 TAB」、XCOPY 列、沒加引號的 `16 System` 列的檔。
- 步驟：在 Data.Observer 的事件記錄分頁（tsEventLogTxt）開這個檔；同一個檔用 BCB6（V899 或 V912）的 Observer 也開一次。
- 預期結果：每一欄跟以前一樣（commit `a9b93d61`：「the columns must be what they were」）。
- 如何還原：不用。
- 依賴：W15＝B（Steven 20260927）；cObserver.cpp 已認領（FROM_STEVEN §1 `0c55fd18`）。

### 1-6 事件記錄頁（eventlog.html）與自動工作分頁（ELA R5／R6、O10 單一來源）

- 狀態：已推・會運作（`353c0c6e`、`4dc55b42`、`102ed625`）。
- 前置條件：任一組態、任一客戶碼；1-1 已看過 ELA 啟動。
- 步驟：
  1. 從工作列的 eventlog 按鈕開事件記錄頁（`D:\HT9045\web\background.html` WINDOWS 表 id `eventlog`，lazy，開了才載入），或直接開 `http://127.0.0.1:8045/page/eventlog.html`（網頁根目錄是 `D:\HT9045\web`）。
  2. 看各分頁（Summary／By Day／By Hour／Top5／Alarm／Fail／By Filter／Auto Jobs／Log）；**先不要按 Query、Save Summary、立即執行**。
  3. 開 `http://127.0.0.1:8045/api/ela/schedule`。
  4. 開今天的 `D:\HT9045_Log\UploadFile\yyyy\mm\FTP_Log_yyyymmdd.csv`。
- 預期結果：
  - Auto Jobs 分頁每個工作一列（O06-4、N10-3、N25-3、O19-VTEST、N17-UploadProdLog、N25-4、N25-5）：開或關＋關的原因、種類與錯開秒數、下次時間、嘗試次數 n／7；結果依顏色分（ELA 帳本 R6 一節第 1380～1387 行）。O06-4 沒勾 [O06-8] 時原因是 `O06-8 off ([Event Log] EnanleTimePeriodSaveLog): …`（★W43，`ElaSchedule.cpp` 第 483～484 行）。
  - `/api/ela/schedule` 有 `installed`、`build`（sim／ship）、`ftp`（兩種組態都是 WinINet，★W36＝C）、`o10`、`o10Source`；**不含任何帳號密碼**（同帳本第 1257、1387 行）。
  - FTP_Log 首列 `Date, Time, Action, S2, S3, S4, S5, S6`；開機時每個工作一行，格式 `yyyy-mm-dd, hh:nn:ss.000, Schedule, <工作>, <事件>, <時段>, <說明>`：開著的寫 `on`＋`<種類>; stagger N s of W s`，關著的寫 `off`＋原因；O10 關時原因是 `O10 off (the Handler's IniConfig.bO10UseEventLogSaver)`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaSchedule.cpp` 第 1076～1078 行、`ElaReports.cpp` 第 64～69 行）。Auto Jobs 的每一列要跟 FTP_Log 對得上。
  - 機台的 `D:\HT9045\config\config.ini` `[Event Log] EnableAutoSaveEventLog=0` 時 O10 是關的，所有定時工作都該顯示 off（STEVEN-NB3 的 config.ini 第 3 行就是這樣，ELA 帳本第 1280 行）。
- 如何還原：不用。
- 依賴：#22 D-a（O10 閘）、D-b（開機補跑）；R5、R6 不需要認領。**不要去「修」config.ini 存的 `bO10UseEventLogSaver=1`**：golden 讀設定時本來就用 `EnableAutoSaveEventLog` 把它蓋掉（ELA 帳本第 1270～1282 行）。

### 1-7 Tester 介面設定頁的新畫面，不存檔（Q41 A 段 TI-1／TI-3／W6）

- 狀態：已推・會運作（`e6e90401`）。
- 前置條件：任一組態；開 `D:\HT9045\web\page\Setup.TesterIF.html`；權限夠改 Tester 介面。
- 步驟：
  1. TI-1：在 DIO 種類下拉（cbDIOType）換不同選項。
  2. TI-3：按 Tester TCP 鈕（btTesterTCPShow）。
  3. W6：配方是 GPIB 模式時看 RS232 分頁；Bit Length、Stop Bit、Parity 的新選項（5 Bits／6 Bits、1.5 Bits、Mark／Space）。
  4. 這一段**不按存檔**，離開前取消。
- 預期結果：
  - TI-1：下方清單（lstTTL）顯示那個 DIO 的摘要預覽，8／10 bit 時有 golden 的警告；預覽在開頁時算，不改任何執行中的設定（commit `e6e90401`）。
  - TI-3：開出 Tester Comm 視窗（`D:\HT9045\web\page\testercomm.html`）並停在 TCP/IP 分頁。
  - W6：GPIB 模式也看得到 RS232 分頁，只有 pnlRS232 那四個欄位可改，其餘灰掉並有說明；新選項看得到。
- 如何還原：不用（沒存檔）。
- 依賴：Q41 認領 FROM_STEVEN §1（handoff `240630bb`）；W6（Q5）＝A（Steven 20260927）。W6 在 GPIB 模式放寬了 golden（golden 在 GPIB 模式把 RS232 分頁藏起來），是刻意偏離。

### 1-8 其他設定頁的畫面連動，不存檔（Q41 B 段）

- 狀態：已推・會運作（只有 SetUp 動到 C++，其他都只改頁面 JS）。
- 前置條件：任一組態；權限夠改那一頁；每一頁改完**不存檔**，用取消或重開頁面放棄。
- 步驟與預期結果（每列取自各 commit 的上機要看；頁面都在 `D:\HT9045\web\page\`）：

| 頁面 | commit | 怎麼試 | 預期 |
|---|---|---|---|
| Setup.SetUp.html | `aaaa46d6` | 切 One Side；勾 SLK 夾具；勾 Arm1 Pick Arm2 Test；換 Y Pitch Offset 模式；勾 Preciser | Shuttle1 被勾、吸嘴模式回 0 並鎖住；勾 SLK 才能選 separability test；Shuttle 模式群組消失；NN＝10 且鎖住、換回來是檔案值；像 Off-center kit 一樣換圖 |
| Config.Configuration.html（現在由伺服器算，見 1-12） | `f1f8a5cd` | 勾 E30／E31／E32、E39、D36；D21／D47／F05；O06 的 Set All；Search Function 打兩個字再清空 | 對應面板出現；出現 E39_1；D33 取消、D35 勾上；子欄位跟著勾；O06 整列 On；符合的勾選框搬到搜尋頁、清空後回原位 |
| Setup.TrayForm.html | `2dccb3e8` | X 數量打 1；X 數量是 1 時點 X Pitch；Type2 頁 Copy From | X Pitch 變 0、格子圖一欄；不開小鍵盤、變 0；只列 Type1／Type3，Copy 後九欄抄過來 |
| Setup.Ld_ULd.html | `60610b61` | 按 Default Value；權限不夠時再按 | 8 欄變出廠值；按不下去 |
| Setup.HotPlate.html | `c1147af4` | X 數量打 1（或 X 已是 1 時在 Y 數量按一下） | X Pitch 變 0 |
| Setup.BarCode.html | `2211570f`、`7cc75fd6` | Lot Verification 填 Start／End 與樣本字串按 Get、Test；按 2DID Offset（Offset 視窗開著或沒開各一次） | 切出 Lot ID、回填 2D Start／End；Offset 視窗打開、切到 Index offset、選到第一組 |
| Setup.Contact.html | `15457abf` | 按 Contact Force／Offset／Barcode／Temp Set／Temp Offset／AT／TCPIP；Contact 全螢幕時再按 | 開對應視窗；TCPIP 開 Tester Comm 的 TCP/IP 分頁；全螢幕時開出來的視窗能不能操作，帳本沒寫預期，上機記下 |
| Setup.Cleaning.html | `6a0b9e74` | 片數旁的上下鍵；2x6 模式；權限不夠 | 每次跳 site 數的倍數、奇數倍多跳一格、不超過 Min／Max；2x6 看不到；按不動 |
| Setup.Temp_Set.html（TS-7～TS-9 見 1-11） | `05c1a153` | Clear All；Same as Arm1；切到 ATC 頁再切走；Tj Map 選第 3 種 | 基準點清空；FFC 10 組抄到 Arm2；基準點欄消失再出現；第 3 種才出現表格 |
| Setup.YieldMonitoring.html | `d7193041` | 勾／取消 Least Retest Times；重開頁 | FT ART 選項消失／出現；開頁照配方值顯示 |

- 如何還原：不用（沒存檔）；存檔的檢查在 2-1。
- 依賴：Q41（S158）認領 FROM_STEVEN §1（handoff `240630bb`）；ST01-E 16:50 同意 Speed 只改頁面 JS。CL-5 不做。

### 1-9 速度設定頁，不存檔（Q41 SP-1～5；Speed form.event；滑桿事件）

- 狀態：已推・會運作。頁面 `D:\HT9045\web\page\ht9045_speed_c.js`（St02 的檔）看伺服器 editlist.get 的 "events" 決定怎麼做（同檔第 167～176 行）：
  - gpib-widget `db863df4` 的 wb_serve：含 St01 `70aa17e8`，"events" 列齊 13 個按鈕（`FileRW\ArmSpeed_File.gen.inc` 第 2946～2958 行）⇒ 九個勾選框、Select All、Set To Default、±10 都送 form.event，由伺服器照 golden 算（`18e73f47`）；滑桿位置放在每個事件的 "state" 裡（同 JS 第 251～254 行）。
  - St01 分支 `v906/steven-cbridge-review6` 的 wb_serve（多了 `7e1785dc` 滑桿事件列、`29a13bdb`「有自己事件的元件不從 state 改」）：放開滑桿時另外送 `{"control":"tbAllSpeed","event":"change","position":<數值>}`（`tbAccSpeed`、`tbEPControl` 同樣），伺服器跑 golden 的 tb*Change（`0daefb95`；同 JS 第 39～43、271、334～336 行）。
  - 沒列出事件的舊伺服器：照 `a2baa2c4` 在頁面本地算，行為不變。
- 前置條件：開 `D:\HT9045\web\page\Setup.Speed.html`；F12 開著 Network → WS 看訊息。要試滑桿事件：用 St01 分支編的 wb_serve 加 `--root <gpib-widget 那份的 web 資料夾>` 啟動。
- 步驟：
  1. 勾任一軸；按 Select All；按 Set To Default；拖 EP 滑桿。**不存檔**。
  2. **滑桿＋±10**：把「全部速度」（tbAllSpeed）拖到 57、放開；按 +10；再拖到 57、按 -10。
- 預期：
  - 勾任一軸之後「全部速度」滑桿和 ±10 才可按；±10 要等伺服器回應後才能按；Select All 九個全勾；Set To Default 全部回出廠值；EP 數字跟著變（ChangeLog 0928 §11b）。
  - 步驟 2：**從拖到的位置起算**：57 → +10 → 60（golden 先加 10 再捨去個位，同 JS 第 137～141 行）；57 → -10 → 47；勾選的軸與「全部速度」欄跟著變。若結果是從拖之前的位置算的，就是伺服器用了舊的位置（`0daefb95` 要修的就是這個）。
  - 用 St01 分支的伺服器時，WS 裡放開滑桿那一刻多一則 form.event，內容有 `"event":"change"` 與 `"position":57`；gpib-widget 的伺服器不會有這一則（位置在 ±10 那則的 "state" 裡）。
- 失敗時：存 WS 訊息（DevTools 的 Network → WS → 右鍵 Save all as HAR）、瀏覽器 Console（找 `[Speed/Q41]` 開頭的行）、頁面截圖；記下用的是哪一個 wb_serve。
- 還原：不用（沒存檔）；離開前取消或重開頁面放棄。
- 依賴：ST01-E 16:50 同意（只改頁面 JS）；滑桿事件是 St01 `7e1785dc`（還沒進 main）。存檔在 2-1（Speed 的 SaveFlow 已在這棵，`FileRW\ArmSpeed_File.cpp` 第 63 行）。

### 1-10 Motion View 畫主畫面托盤（S118；原 4-3）

- 狀態：已推・會運作。本體 `74080bb3`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\ChanMvTrays.h`／`.cpp`）；接線 `b243f15e`（筆電同意的認領）：`tools\wb_serve.cpp` 第 439 行宣告、第 2894 行 `PublishExtraTags` 裡 `n += ::W906_StageMotionViewTrays(snap);`，`D:\HT9045\web\page\Main.MotionView.html` 第 2970 行載入 St02 的 `D:\HT9045\web\page\ht9045_mv_trays.js`。
  - 主畫面 34 個托盤各一個 tag `motionView.trays.<名稱>`（{name, xItem, yItem, ver, cells}），另有 `motionView.trays.ver`、`motionView.trays.keys`（`JsonBridge\ChanMvTrays.cpp` 第 48～52 行）；全部送，頁面依機種決定畫哪些（★W54＝A，0928 第二題）。
- 前置條件：任一組態；有配方；從工作列開 Motion View（`D:\HT9045\web\background.html` 第 502 行 `motionview`）；F12 → Console。
- 步驟：
  1. Console 依序打：`HT9045Tags.get('motionView.trays.ver')`、`HT9045Tags.get('motionView.trays.keys')`、`JSON.parse(HT9045Tags.get('motionView.trays.loader'))`、`HT9045MvTrays.lastVer()`。
  2. 在 Loader 盤上取放一顆 IC（或讓機台跑一小段），再打一次。
  3. 停著不動一分鐘，看 WS 訊息。
- 預期（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\S118_MOTIONVIEW_TRAY_PRODUCER.md` 第 5 節）：keys 是 34 個名稱的陣列；loader 的 `xItem`／`yItem` 等於配方 Loader 盤的 X／Y 數；Motion View 的托盤畫的是實際格子（不是靜態版面）；取放後那一格變、`ver` 加 1、`lastVer()` 跟著變；不動時不再送 trays 的更新。
- 失敗時：Console 四個值的截圖；完全沒有 `motionView.trays.*`＝wb_serve 太舊，記下 exe 的建置時間；有 tag 但畫面不變＝頁面那半，存 Console 的錯誤訊息。
- 還原：不用（只讀記憶體，不寫檔）。
- 備註：`S118_MOTIONVIEW_TRAY_PRODUCER.md` 第 3 節還寫「還沒接上」，已過期（`b243f15e` 接上了）。

### 1-11 溫度設定頁：基準點點數、Boost 下限、Airstream 夾值，不存檔（TS-7、★W39 TS-8、Q52 TS-9；原 4-5）

- 狀態：已推・會運作。三段都在 St02 的 `D:\HT9045\web\page\ht9045_temp_set_c.js`，由 `D:\HT9045\web\page\Setup.Temp_Set.html` 第 348 行載入（`5e16f04f`）：
  - **TS-7**：1／2／3／5／6 點與 System／Kit 基準點每按一次送 form.event，伺服器照 golden rb1PointClick／rgBasePointClick＋UpDateEdit 算（St01 `70aa17e8`，事件表 `FileRW\Temperature.gen.inc` 第 6413～6421 行，已在這棵）。**暫時**：St01 的 TS-1／TS-2 還有事件在等回覆時，TS-7 先等（同 JS 第 255 行，`db863df4`）；St01 的 `ht9045_temp_set_ts1.js` 只在 St01 分支，所以在 gpib-widget 上這一行不起作用。
  - **TS-8**（★W39＝A，`959c2236`）：golden edtIdleTime_LongClick 的 Boost 下限規則照原樣（看起來寫反）：LB 下限 ≤ Boost 下限時，把 Boost 下限改成 LB 下限＋2（同 JS 第 302～313 行；golden 906_0625_Steven uTemp_Set.cpp:5736-5747）。只在 12 個 Idle Time／Boost Duration／Post Boost 框的小鍵盤關掉時跑（同 JS 第 281～283 行）；LB、Boost 下限自己的框不跑。
  - **TS-9**（Q52，`7f4e30a1`）：Airstream offset 四個框的小鍵盤關掉時照 golden 夾值：主畫面工作溫度＋Index Airstream 的值 ＜ -70 → 改成 -70－工作溫度；＞ 35 → 改成 0（同 JS 第 351～378 行；golden 的 Tag 都是 0，所以四個框檢查的都是 Index Airstream 那一格）。工作溫度是開頁時伺服器給的（`FileRW\TempSet_Ts9.cpp`，掛在 `FileRW\Temperature.cpp` 第 40、245 行）。
- 前置條件：任一組態；開 Setup.Temp_Set；等級 52 以上的帳號，另備一個 52 以下的；F12 → Console；**這一項不存檔**（存檔在 2-1）。
- 步驟：
  1. TS-7：切 1／2／3／5／6 點；切 System／Kit；先改一格再切點數；換等級 52 以下的帳號再按；快速連點。
  2. TS-8：LB 下限打 30.0、Boost 下限打 35.0；開任一個 Idle Time 框的小鍵盤，按 OK；再試一次按取消。
  3. TS-9：主畫面工作溫度 25 時開頁，Index Airstream（edt_SetIndexAirstreamTemp）打 15，再打 -20；主畫面工作溫度改 -60、重開頁，打 -20。
- 預期：
  - TS-7：原本那個選項取消勾選、各通道欄位出現或消失；先改的那格還在；等級 52 以下按不到；快速連點不出錯（ChangeLog 0928 §11b）。
  - TS-8：OK 或取消都一樣，Boost 下限變成 `32`。
  - TS-9：25＋15＝40 ＞ 35 → 框變 `0`；25－20＝5 → 不動；-60－20＝-80 ＜ -70 → 框變 `-10`；每次改值 Console 一行 `[Temp_Set/Q41] TS-9 …`。
- 失敗時：Console（`[Temp_Set/Q41]` 開頭的行）、WS 訊息（TS-7）、頁面截圖、當時主畫面的工作溫度。
- 還原：不用（沒存檔）；離開前取消或重開頁面。
- 依賴：Setup.Temp_Set.html:348 認領（ST01-E 同意）；★W39＝A（Steven 0928）；Q52 TS-9＝B（St01 20260927）；TS-7 的暫時等待之後要換成 St01 的共用佇列。

### 1-12 Configuration 頁 D46 上下鍵與勾選框走伺服器，不存檔（CC-E2／CC-E7；原 4-6）

- 狀態：已推・會運作。`D:\HT9045\web\page\ht9045_config_q41.js`（`a54b8fae`）在伺服器的 editlist.get 帶 eventTag 時改送 form.event；St01 `64ade3b7` 已在這棵（`FileRW\IniConfig.cpp` 第 458 行送 `"eventTag":"Config.Configuration"`），所以 1-8 表裡 Configuration 那一列現在也是伺服器算。
- 前置條件：開 `D:\HT9045\web\page\Config.Configuration.html`；F12 → Network → WS；**不存檔**。
- 步驟：按 D46 上下鍵；勾 E30／E31／E32、E39、D36、D21／D47／F05。
- 預期（`a54b8fae` 訊息）：D46 一格一格變、夾在 5～15；E30／E31／E32 面板出現且底下可改；勾 E39 出現 E39_1；勾 D36 → D33 取消、D35 勾上；子欄位跟著勾；每按一次 WS 裡一則 form.event。
- 失敗時：WS 訊息、Console（edD46 出現在存檔回應的 ignored 裡時會印 warn）、截圖。
- 還原：不用（沒存檔）；存檔重讀在 2-1。

### 1-13 Tester 介面設定頁「不存就關」的尾段（Q41 (d)；原 4-4）

- 狀態：已推・會運作。本體 `38971aab`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_TesterIF.cpp` 第 590 行起 `TIF_OnPageClosed`）；第 272 行用 St01 `5b73d905` 的 `W906_WindowEdgeRegister("FTestIF", …)` 登記（`7f4e30a1`）。
- 前置條件：開 Setup.TesterIF；一份 TTL 模式的配方；看 wb_serve 主控台。
- 步驟：改一個值不存就關、重開；TTL 模式開頁再關。「改值存檔後再關」會寫檔，放到 2-1 做 TesterIF 那一頁時一起看。
- 預期（TESTERCOMM 帳本第 687～688 行）：不存就關 → 重開看到的是檔案值；TTL 模式關頁 → GPIB 程式關掉、WakeupGPIB 再拉起；存檔後關頁 → 主控台只有一行 `[TesterIF] page closed: golden FormClose + sbTesterClick tail already ran at save (Steven S107-1) -- not run again`（同檔第 595 行），沒有第二次 CloseGpibProgram。關掉後約 0.5 秒才跑；按 F5 約 15 秒後才算關；兩個 HMI 分頁可能多算一次關頁。
- 失敗時：主控台最後 30 行（找 `[TesterIF]`）、HANDLER LOG 的 `GPIB Close - <來源>`、截圖。
- 還原：不用（只重讀，不寫檔）；TTL 配方的橋接會自己再拉起來。
- 依賴：Steven S107-1。

---

## 第二段：會寫檔的檢查（每一項先照「前置條件」備份）

### 2-1 各設定頁存檔後重讀一致（Q41 A／B 段）

- 狀態：已推・會運作。
- 前置條件：`realfile_guard.py snap q41`（涵蓋 config.ini 與作用中配方資料夾）；任一組態；機台停止中。
- 步驟：1-7、1-8、1-9、1-11、1-12 每一頁各挑一項改掉 → 存檔 → 關頁 → 重開，看值；每頁做完跑一次 `realfile_guard.py check q41`。另外：
  1. Tester 介面設定頁（TI-4／TI-5）：存檔後照 golden 跑關表單的尾段（重讀 TestIF、TTL 模式檢查並關 GPIB 程式、重載 DIO、換算、重載參數、換測試模式圖）。TTL 模式的配方存一次。
  2. RS232 格式：試存 5 bits＋2 stop、6／7／8 bits＋1.5 stop（GPIB 與 RS232 模式各一次）。
- 預期結果：
  - 每一頁重開後是剛存的值；check 只列出那一頁該改的鍵。
  - TI-4／5：TTL 模式存檔後 GPIB 程式被關掉、之後由 WakeupGPIB 拉起來。存檔尾段的 log 字串帳本沒寫，上機時記下實際值。
  - RS232 那幾種 Windows 不接受的組合：不寫檔（Q3(1)＝A，`TESTERCOMM_PORT_LEDGER.md` 第 316 行）。拒絕訊息帳本沒寫，上機時記下。
  - Speed 頁：St01 的 SaveFlow 已在這棵（`FileRW\ArmSpeed_File.cpp` 第 63 行，存檔鈕之後補關窗尾段），重開要是剛存的值。
  - 1-11 溫度設定頁、1-12 Configuration 頁也在這一步各存一次（TS-8／TS-9 改過的值要存進去）；每一頁連續存兩次，順便驗 S-1。
- 如何還原：`realfile_guard.py restore q41`。
- 依賴：同 1-7～1-9、1-11、1-12；TI-4／5 照 Steven S107-1「存檔後就跑」；Q41-W3G（GPIB 模式也擋格式，#37）暫照 A、待確認。

### 2-2 Tester 遠端改溫度 offset，寫進配方（W9）

- 狀態：已推・會運作（`014e9094`，在 main）。
- 前置條件：
  - 有 Tester 經 GPIB 送指令（`SETTESTOFFSET_…` 或 `DEVICETEMP…`）；
  - **先備份作用中配方的 `D:\HT9045\IniData\Data\<配方名>\Temperature.Data`**（realfile_guard snap 會一起備）；
  - 確認 `D:\HT9045\SetUp.inf` 第一行就是正在跑的配方，否則會寫進 `D:\HT9045\IniData\Data\Fail Open\`；
  - 任一組態、任一客戶碼（GPIB 機台）。
- 步驟（TESTERCOMM 帳本第 589～599 行，10 點）：
  1. 一個 site 送一次小的 `SETTESTOFFSET_0.5`。
  2. 看 log、看 Temperature.Data 的鍵與值、看溫度設定頁。
  3. 看加熱器 SV 有沒有真的跟著動。
  4. ATC 機台另外看 ATC 那邊。
  5. 問客戶 Tester 送的是增量還是絕對值、會不會重送。
- 預期結果：
  - log 一行「Remote Control Temp. Offset a to b」；非 ATC 寫 `[User OffSet] CH<n>`、ATC 寫 `[ATC] ATCTempOffset[<n>]`，值用 `%0.4f`，**是現值＋offset（會累加）**，只在上下限內才寫（上限 ±60）；溫度設定頁顯示重載後的值（912 的重載）。
  - SV 有沒有重送沒追到（記憶體進 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\bthermo.cpp` 第 282 行 ConvertTempOffset），上機記下。
  - ATC 機台：ATC 控制器不變（SetATCOffset 還閘著）。
  - Tester 永遠收到 "ECHO"，沒有失敗回報；超出範圍的 log 用 %d 印 double（golden 的怪處）。
  - `DEVICETEMP…` 也被當 offset：Tester 若送 ±60 內的絕對 Tj，會被寫進配方。
  - 寫哪一臂看收到指令當下的 IndexStatus。
  - 改動是永久的，會有一筆 ChangeLog；配方 MD5 檢查可能會報。
- 如何還原：`realfile_guard.py restore <標籤>`（或把備份的 Temperature.Data 放回），**然後重開 wb_serve**（記憶體裡已經是新值）。
- 依賴：Steven 20260927 W9＝A；912 重載＝要；SECS G34＝開（見 3-5）；912 的 iSiteToOfs 修正不做（記在帳本待辦）。⚠ 任何連著真 IniData 跑 wb_serve 的 PC，收到 GPIB `SETTESTOFFSET_` 就會改那台的配方。

### 2-3 Auto Retest 的 Tester 廠牌跟配方走（W1／W1b）

- 狀態：已推・會運作（`d78132eb`、`884b3035`，在 main）。
- 前置條件：SCK ART 機台（`CosFunction.bUseSCKART` 開、A10 開）；備份作用中配方的 `Tester.Data`（realfile_guard snap）；有 93K 或 Flex Tester。
- 步驟：
  1. 用一個沒有 `[AutoRetest] iTesterType` 鍵的配方開機。
  2. 93K Tester 送 SRQMASK；Flex Tester 送 LOTSTATUS?。
  3. W1b：93K 配方跑到 clean-out 結束；Flex 配方再跑一次。
- 預期結果：
  - 開機後配方 Tester.Data 多一行 `iTesterType=1`（偏離：golden 缺鍵是 0＝Flex；Steven W1）。
  - 送 SRQMASK／LOTSTATUS? 之後配方的值跟著變，INPUTQTY 的解析跟著換；會記一筆 change log。
  - W1b：93K 配方的 clean-out 結束不再進 Flex-ART 的批檢查；Flex 配方仍然會進。
- 如何還原：realfile_guard restore（把 Tester.Data 放回）。
- 依賴：Steven 20260927 W1＝(b)；W1b 筆電 TO_STEVEN §4 10:3x「在 W1 裡做」。

### 2-4 事件記錄頁的查詢（ELA W15／W18／W19；S128）

- 狀態：已推・會運作。
- 前置條件：備份 `D:\HT9045\Error\English\JAM0000.dat`（備份段 B.2 第 3 點）；任一組態。準備幾種日期：有沒加引號的 `16 System` 列的那一天；用小時／12 小時／月存檔的機台；有 AllEventLog（HTSaveSameFolder）的機台。
- 步驟：在事件記錄頁按 Query（`POST /api/ela/query`）查上面那幾天；看 By Filter → By Function 各框、Top5 的明細、By Hour 的 Total Test Time；同一個查詢再按一次。查詢跑的時候到 Status.Security 存一次 Jam 設定（S128）。
- 預期結果（ELA 帳本第 942～943 行、commit `1042d4cc`／`2ceca61c`／`82196ca3`／`5884cf6a`）：
  - `16 System` 那種列現在會算進去（以前被切壞、不算）。
  - 按小時／12 小時／月存檔的機台現在有數字（以前是 0）；有 AllEventLog 的機台不會算兩次，Log 分頁看得到 `dupRowsSkipped`；兩天的查詢跨一個月檔時，讀檔清單只列一次。
  - By Function：2D barcode 框只列 2D 的列、Temperature 框只列溫度的列（golden 的四個框對調已修）。
  - Top5 明細：AlarmCode 完全相同才算（例：WAR1520 不再吃到 WAR15206）。
  - 第二次查詢 Fail Count 不再變大。
  - By Hour 的 Total Test Time 會比 BCB 舊分析器大 24 倍（這是修正，不是錯）。
  - JAM0000.dat 只會多出缺的列（照 golden 寫回）。
  - S128：查詢跑的時候 Jam 存檔可能回一次 "busy"，重試成功；JAM0000.dat 只增加。
- 如何還原：把備份的 JAM0000.dat 放回。
- 依賴：Steven 20260927 W15＝B、W18＝B、W19＝B；#17＝A；S128＝B。VTEST（915／919）讀一般日檔是 ★W38，等 Steven（有 `goldenFileRule` 可切回）。

### 2-5 手動 Save Summary（ELA P7a）

- 狀態：已推・會運作（`8e8fc312`）。
- 前置條件：`D:\RMS\` 存在（不在就先建）；先記下 `D:\RMS\` 的檔案清單；任一組態。
- 步驟：
  1. 在事件記錄頁先 Query 一次。
  2. 查詢列的 Name 欄保留預設（今天 yyyy-mm-dd）或打一個名字，按 Save Summary。
  3. 名字打成帶資料夾的（例如含 `..` 或 `\`）再按一次。
  4. 沒查詢就按（重開頁面後直接按）。
- 預期結果（ELA 帳本 P7a 一節，第 1578～1594 行）：
  - 寫出 `D:\RMS\<Machine ID>-SummaryData_<名字>.txt`，內容是畫面上那次查詢；按鈕旁顯示寫到哪裡。
  - Summary 分頁仍顯示使用者自己的查詢。
  - 帶資料夾的名字被拒（400）；只收英數、空白與 - _ . ( )，最多 100 字。
  - 沒查詢就按：不寫檔，工作紀錄記 "no query yet"。
- 如何還原：刪掉新產生的那個 .txt。
- 依賴：Steven 0928 第十題＝A（資料夾固定 `D:\RMS\`，帳本 `8c3bb48e` 登記）。報表編碼現在是 UTF-8（★W42＝A；細項 W57～W62 未決，見第五部分）。

### 2-6 四個「Save」按鈕存成 Excel（.xlsx）（ELA P7 做法 D，Steven 0928 第五題＝D）

- 狀態：已推・會運作（`22d07050`，merge `233c5b95`；只改頁面：`D:\HT9045\web\page\eventlog.html`＋`D:\HT9045\web\page\ht9045_ela_xlsx.js`）。取代原本的「存成 CSV」（做法 C）。
- 前置條件：事件記錄頁已經查詢過、各表有資料；知道瀏覽器的下載資料夾；機台上有 Excel 2007 以上（或 LibreOffice）。
- 步驟：在 Alarm、Fail、Top5、By Filter 分頁各按一次「Save」；By Filter 換 Filter 選項與 By Yield 框各再按一次；每個檔用 Excel 直接雙擊開。
- 預期結果（ELA 帳本「P7 XLS 匯出＋做法 D」一節）：
  - 檔名（golden `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp` 第 2245～2297 行，副檔名改 .xlsx）：Alarm → `AlarmList.xlsx`；Fail → `FailureList.xlsx`；Top5 → **一個檔 `Top5Alarm.xlsx`、三個工作表** Top5Alarm／AlarmList／AlarmList1；By Filter → `AlarmListByArea.xlsx`／`AlarmListByYield.xlsx`／`AlarmListByFunction.xlsx`。
  - 雙擊開**不會**跳「修復」或「格式不符」的警告；中文正常；像 `0316` 這種代碼保持文字（前面的 0 還在，不會變成數字或日期）；第一列是粗體表頭。
  - 每個表最多 5,000 列，被截時按鈕旁會寫出來。
- 如何還原：刪掉下載的檔。
- 若 Excel 跳修復提示：先回報；頁面程式 `ht9045_ela_xlsx.js` 的 `ignoredErrors` 那一行可以拿掉再試（helper 註記）。
- 依賴：Steven 0928 第五題＝D（真的 .xlsx，已做）。

### 2-7 自動存報表開關與定時報表（O10、ELA R2、R5；★W43＝C、★W44＝B／W44-2）

- 狀態：已推・會運作（`102ed625`、`1b6bf39e`、`155e27dd`、`4dc55b42`；★W38／★W44 `75ca7b37`；★W43／W44-2 `2b037895`、`3a3af4fe`）。規則全文在 ELA 帳本 R5 一節：★W44 第 1286～1315 行、★W43 第 1316～1337 行、★W44-2 第 1338～1349 行。
- 前置條件：
  - realfile_guard snap（config.ini）；備份 `D:\HT9045_Log\UploadFile\ElaScheduleState.ini`；
  - 記下 `D:\RMS\`、`D:\MTBF_Summary\`、`D:\HT9045_Log\EventLogSummary\` 的現況；
  - O06-4 的存檔資料夾是 config.ini 的 `AutoSaveProductionPath`（帳本第 1336 行：預設 `D:\RMS`）；
  - N10-3 的「指定時間」：畫面上的 `dtpN10_3_1_SpecifiedTime` **還沒接**（★W44-1＝A 的接線還沒做：`FileRW\IniConfig.gen.inc` 沒有這個欄位），config.ini 沒有 `[FTPUpLoad] dN10_3_1_SpecifiedTime` 鍵＝**每天 00:00**，跟 golden Handler 的實際行為一樣（帳本第 1294～1303 行）；
  - VTEST（O19）只在客戶碼 915／919；CUSTOMER_CODE 在 `D:\HT9045\system\Gerneral.ini` `[System]`。
- 步驟：
  1. Configuration 頁把 O06-1（config.ini `[Event Log] EnableAutoSaveEventLog`，golden 讀設定時用它蓋掉 O10）關掉存檔，看 FTP_Log 與 Auto Jobs。
  2. 打開 O06-1 存檔，**不重開 wb_serve**，等下一個時段。
  3. **★W43**：打開 O06-4、[O06-8] **不勾**，等一個整點；再勾 [O06-8] 選 10 分，等 10 分；最後在 Auto Jobs 對 O06-4 按一次「立即執行」（[O06-8] 不勾時也試一次）。
  4. **★W44**：O10 開、`[FTPUpLoad] bN10_DailyUploadProdData` 開、`iN10UploadProductMethod=3`（帳本的例子是 868 機台）：過 00:00（＋錯開秒數）看一次；同一天重開 wb_serve。
  5. **★W44-2**：00:00 前關 wb_serve、過了 00:00 再開。
  6. 915／919 機台：打開 O19 每週報表，等那一週的 00:00（或看開機補跑）。
  7. 關 wb_serve、等過一個時段、再開，看 ElaScheduleState.ini 與補跑。
  8. O06-4 的資料夾如果是網路磁碟：拔掉共用、看重試間隔。
- 預期結果：
  - O10 關：定時工作都不跑，FTP_Log 有 `off … O10 off (the Handler's IniConfig.bO10UseEventLogSaver)`；打開後下一個時段就跑，不用重開（commit `102ed625`）。
  - ★W43＝C（O06-4 跟著 [O06-8]）：沒勾＝整點**不再**出現新的 `<ID>-SummaryData_<日期>.txt`，Auto Jobs 的 O06-4 顯示 off、原因 `O06-8 off ([Event Log] EnanleTimePeriodSaveLog): no timed SummaryData save (W43 = C, as the Handler)`（`ElaSchedule.cpp` 第 483～484 行）；勾 10 分＝每 10 分（整點對齊）一次，FTP_Log 一行 verified；Configuration 改了之後 60 秒內照新值；**手動「立即執行」不管 [O06-8] 都照跑**（`EnableAutoSaveProductiont` 關時照樣擋）。
  - ★W44＝B：N10-3 方法 3 每天只在那一分鐘跑一次（現在是 00:00）；FTP_Log 的 N10-3 一天一行 verified；同一天重開機不重跑。
  - ★W44-2：開機時那天的時間已過一分鐘以上＝開機後補發**一次**，FTP_Log 一行 `N10-3, boot catch-up, …, catch-up for yyyy/mm/dd hh:mm: the specified time passed while the Handler was off (W44-2); …`，接著 `N10-3, attempt 1 (boot catch-up): verified, …`；只補最近那一次，不補好幾天；第一次開機（沒有紀錄）不補、只寫 `armed=`。
  - VTEST：`D:\MTBF_Summary\`（`asO19_SavePath` 預設）有 `<Machine ID>-SummaryData_<今天>.txt`，20 行、UTF-8，內容區間是前 7 天；O10 關時不產生；事件記錄頁仍顯示使用者自己的查詢（工作的 7 天區間在 Auto Jobs 的工作紀錄）。
  - 重開機：`ElaScheduleState.ini` 有 `[工作名]`、`done=`、`armed=`；錯過的最近一個時段補跑**一次**（開機＋錯開秒數）；第一次開機沒有紀錄時不補跑、只寫 armed。
  - 網路磁碟斷線：退避 1／2／4／8／16／30 分左右（依 Machine ID 有 ±20% 亂數），最多 7 次或下一個時段先到就放棄（ELA 帳本第 1247～1250 行）。
  - N10 的 "FTP" 分支只存 `D:\HT9045_Log\EventLogSummary\yyyymm\`，不上傳。
- 失敗時：當天 `D:\HT9045_Log\UploadFile\yyyy\mm\FTP_Log_yyyymmdd.csv`、`ElaScheduleState.ini`、`/api/ela/schedule` 的輸出（瀏覽器直接開，存成檔）、Auto Jobs 分頁截圖、config.ini 的 `[Event Log]`／`[FTPUpLoad]` 兩段。
- 如何還原：realfile_guard restore（config.ini）；放回 ElaScheduleState.ini；刪新產生的報表檔。
- 依賴：#22 D-a～D-f（Steven 20260927）、W18＝B、W21；★W43＝C、★W44＝B、★W44-2＝補發、★W44-1＝A（接法定了，接線還沒做）；★W42 細項（W57～W62）待 Steven。

### 2-8 SPIL／999 每天複製產品 log（W13，N17）

- 狀態：已推・會運作（R5 的 `JOB_N17_PRODLOG`）。
- 前置條件：SPIL 機台或 CC_QUALCOMM 999；`bN17UploadProdLog` 開；目標資料夾 `asN17ProductionLogPath`（可能是網路路徑）；記下目標資料夾現況。**不看 O10**。
- 步驟：等 01:00（或在 Auto Jobs 按 N17-UploadProdLog 的「立即執行」，會先詢問）。
- 預期結果：產品 log 複製到目標資料夾，大小一樣才算成功；FTP_Log 有記錄。確切檔名帳本只寫「SPIL 檔名」（`bSPILFunction` 不在任何 ini，所以「N17 開而且不是 999」當成 SPIL），上機時記下實際值。目標資料夾不在＝重試；重試時目標已有同大小的檔＝成功；大小不同＝不覆寫、不重試。
- 如何還原：刪掉複製過去的檔。
- 依賴：W13（計畫：這是檔案複製，不是 FTP）；`HT9045_ELA=0` 時不跑（偏離：golden 是 Handler 自己的工作）。

### 2-9 功能類 log（P4-D2 ProductionLog、P4-D4 SaveMessageHistroy、W7）

- 狀態：已推・會運作（`ec1b034c`、`06f8ef0a`、`b4712e5f`）。
- 前置條件：記下 `D:\RMS\`、`D:\HT9045_log\ProductionInfo\`、`D:\HT9045_Log\2DMapping\`、`D:\HT9045_Log\ASM\`、`D:\HT9045_Log\QtyData\` 的現況；開 O06 要改設定 → realfile_guard snap。帳本沒寫 O06 是哪一個子開關，上機時記下。
- 步驟：
  1. O06 開、`D:\RMS\` 存在：跑一段、重開 wb_serve、再跑、關機。
  2. O06 開、`D:\RMS\` 不存在：跑一段。
  3. CC_Greatek（956）＋N14_1：跑一個 MO，讓 MESxxxx 事件發生。
  4. 2D sort 一批；Auto Site Map 跑一次；Clarn_Data（清料）一次。
- 預期結果（CMYDB 帳本第 183 行、`ec1b034c` 訊息）：
  - `D:\RMS\<SocketHandlerID>_yyyymmdd.logs` 重開 wb_serve 後**不被截短**（開機先載回當天檔），關機多一行 `--> Close`。
  - `D:\RMS\` 不存在時什麼都不寫、不報錯、不卡住（偏離：golden 丟 EFCreateError）。
  - Greatek＋N14_1：每筆 NewRecordProcess 在 `D:\HT9045_log\ProductionInfo\<MO>\<MO>_History.csv` 多一行 `<code>,yyyy/mm/dd,hh:nn:ss,<message>,0,0`，表頭 `AlarmCode,AlarmDate,AlarmTime,ErrMessage,AlarmType,SkipLevel`；資料夾名少一層（`_sOEE_DirectoryName` 恆為空，偏離）。
  - 2D sort 寫 `D:\HT9045_Log\2DMapping\*`；Auto Site Map 寫 `D:\HT9045_Log\ASM\ASMLog*`；清料寫 `D:\HT9045_Log\QtyData\*`。
- 如何還原：realfile_guard restore（設定）；新增的 log 不用還原。
- 依賴：Steven P4 D2＝B、D4＝B、W7＝A。

### 2-10 login.dat 帳號新增／刪除／修改（Q9）

- 狀態：已推・會運作（`64e2c048`，經 St01 `221cc069` 與 main `8b3a07af` 進到 gpib-widget）。
- 前置條件：
  - gpib-widget 的 wb_serve；
  - 客戶碼要是用 login.dat 當密碼本的（`CosFunction.bUseLoginDatToSetLevel`＝true，例如 JSCC_OS、SCC 943、SJ 791、JCET；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CosFunction.cpp` 第 1197、1308、1773、1807、1892、2324、2440、2547、2565、2842 行）；
  - 先把 `D:\HT9045\system\login.dat` 複製出來，記下 SHA256（`certutil -hashfile D:\HT9045\system\login.dat SHA256`）與大小（必須是 64004 bytes）；
  - 可先練習：設 `W906_PWBOOK_PATH` 指到 login.dat 的複本（St01 的 webprobe 也這樣做；設了測試縫時 Q24 的 SavePassword 會跳過）。
- 步驟（Q9 計畫 `D:\HT9045\.claude\skills\ht9045-st02-workflow\references\q9-q24-login-plan.md` 第 103～107 行、commit `64e2c048`）：
  1. 開機看 login 那一行。
  2. 在 Status.Security（`D:\HT9045\web\page\Status.Security.html`）的密碼功能新增一個測試帳號（New）。
  3. 用 `fc /b` 比對新的 login.dat 與備份。
  4. 用新帳號登入。
  5. 用 PW_Editor 905（原始碼 `D:\HT9045\Password_V1.00.905_20260525\`）看得到它；在 PW_Editor 存一次，登入仍然可以。
  6. Edit（改密碼）；再用錯的舊密碼 Edit 一次；Delete（要確認）；Delete 不確認一次。
  7. 在主控台輸出與 `D:\HT9045_Log\` 搜尋測試密碼。
  8. 用 BCB6 V899／V912 以 V906 寫的 login.dat 登入。
- 預期結果：
  - 開機：`login: mode=book (btLogin) pwPath=D:\HT9045\system\login.dat boot AccessLevel=…`（`WebLogin.cpp` 第 147～148 行）。
  - New → MES1673；`fc /b` 只在 ID 4+30·s、PW 30004+30·s、Level 60004+4·s（s＝槽號）不同。
  - Edit → MES1675；錯的舊密碼 → WAR1677，SHA256 不變；Delete → MES1674；不確認 → cancelled、不寫；重複帳號 → WAR1672；空白 → WAR1678；寫檔失敗 → WAR1682。
  - 寫之前自動備份 `D:\HT9045\system\login.dat.bak_<YYYYMMDD_HHMMSS>`，只留最新一份（備份＝A）。
  - 測試密碼不出現在主控台與 `D:\HT9045_Log\` 任何檔。
  - PW_Editor V1.00.648 只改得到前 30 個槽，而且有一個字（key byte＋1）顯示錯（St01 skill ht9045-login）。
- 如何還原：把最初複製出來的 login.dat 放回，確認 SHA256 跟最初一樣；刪掉 `.bak_*`。
- 依賴：S131 Q9＝B、備份＝A（Steven 20260927）。

### 2-11 權限等級存檔照 golden 寫 login.dat（Q24）

- 狀態：已推・會運作（`7f0c24b1`，經 main `8b3a07af` 進到 gpib-widget）。
- 前置條件：gpib-widget 的 wb_serve；備份 `D:\HT9045\system\login.dat` 與 `D:\HT9045\system\levelset.dat`，記下兩個檔的 SHA256；沒設 `W906_LOGINDAT_PATH`／`W906_PWBOOK_PATH`／`W906_LEVELSET_PATH`（設了就跳過）。
- 步驟：
  1. 在 Status.Security 做一次權限等級存檔（WS `system.levels.put`），記憶體跟檔案一樣的情況下。
  2. wb_serve 開著時用 PW_Editor 改一個帳號，再做一次權限等級存檔。
- 預期結果：
  - 第 1 步：login.dat 的 SHA256 不變（commit `7f0c24b1`）。
  - 第 2 步：golden 的 SavePassword 把記憶體寫回，PW_Editor 剛改的會被蓋掉（golden 行為）——記下實際情形。
  - levelset.dat 會不會變、怎麼變，由 St01 的權限存檔決定，帳本沒寫，上機時記下。
- 如何還原：放回備份的 login.dat、levelset.dat，重開 wb_serve。
- 依賴：S144 Q24＝B；ST01-M 放行（FROM_STEVEN §4 13:50）。⚠ 用密碼本的機台如果沒有 login.dat：選單那一支會建一個明碼槽的檔，Q24 會寫出一個全 0 的檔（照 golden）。

### 2-12 Jam Code 匯出與 JAM0000.dat 補鍵（S127／S128）

- 狀態：已推・會運作（`4fbaf7e9`、`5884cf6a`，在 main）。
- 前置條件：備份 `D:\HT9045\Error\English\JAM0000.dat`；找一台 JAM0000.dat 缺鍵的機台（不缺鍵的話看不到補鍵，只能驗「寫 0 筆」那一條）。
- 步驟：在 Status.Security 的 Jam 設定按匯出；再匯出一次；匯出時看進度條。
- 預期結果（commit `4fbaf7e9`）：
  - 旁邊出現 `*.s128.bak`；JAM0000.dat 只多出新的 `key=value` 列（跟備份 diff）。
  - 第二次匯出寫 0 筆、全部略過。
  - 進度條先走各 area、再走各 key。
- 如何還原：放回備份的 JAM0000.dat，刪 `*.s128.bak`。
- 依賴：Steven Q5／Q6＝B（S127／S128）；JAM0000.dat 跟 ELA 共用具名 mutex `Local\HT9045_JAM0000_dat`。

### 2-13 Jam Code 第二個勾選框（★W45：Include MTBF／Include MTBA (Analyzer)）

- 狀態：已推・會運作（`6f276498`；認領 `ba46ad9f`，ST01-M 同意）。Status.Security 的 Jam Code 分頁多一個分析器的勾選框 `cbIncludeMTBF`（`D:\HT9045\web\page\Status.Security.html` 第 56 行；`D:\HT9045\web\page\ht9045_wire_statussecurity.js` 第 130、216 行；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSecurityJam.cpp` 第 68、160、210、835、995、1004、1005、1013、1018、1023、1040 行；規則 `JamRules.h` 第 122～141 行、`WebSecurityJamW45.h`）：
  - 一般客戶：字樣「Include MTBF」，寫 JAM0000.dat 那一區的 `<碼> IncludeMTBF`；
  - 933（ASE_CL）／967（TeraPower）：字樣「Include MTBA (Analyzer)」，寫 `<碼> IncludeMTBA`（這兩家 Handler 原本的「Include MTBA」框寫的是 IncludeMTBF，golden）。
  - 事件記錄頁工具列的「Jam Code Setting」鈕（`D:\HT9045\web\page\eventlog.html` 第 77 行）開 Password and Security 並切到 Jam Code 分頁（18a）；事件記錄頁自己不改東西。
- 前置條件：備份整個 `D:\HT9045\Error\`（B.2 第 3 點）；權限夠開 Jam Code 分頁（golden：[29]＋Supervisor／HonPrec）；知道機台的客戶碼。⚠ **只點選一個碼就可能寫檔**：第二框照 golden `CheckAndReadIniData`，缺鍵時寫回預設（`WebSecurityJamW45.h` 第 23～25 行）。
- 步驟：
  1. 事件記錄頁按「Jam Code Setting」。
  2. 選一個區與碼，看第二框的字樣與勾選；把它勾上（或取消）；換一個碼；按 Exit。
  3. 用記事本開 `D:\HT9045\Error\English\JAM0000.dat` 找那個碼；看設定變更紀錄。
  4. 事件記錄頁 Query 一段有那個碼的日期。
  5. 換等級不夠的帳號再按「Jam Code Setting」。
- 預期（ELA 帳本第 1212～1216 行、ChangeLog 0928 §11o）：
  - 步驟 1：開出 Password and Security、停在 Jam Code 分頁，旁邊寫 `已切到 Password and Security 的 Jam Code 分頁`（eventlog.html 第 481 行）。
  - 一般客戶：第二框「Include MTBF」，勾了之後那一區有 `<碼> IncludeMTBF=1`；933／967：第二框「Include MTBA (Analyzer)」，寫 `<碼> IncludeMTBA`；原本的「Include MTBA」框照舊。
  - 改了第二框之後設定變更紀錄多一筆（common.cpp `WriteIniData`）。
  - ELA 查詢照第二框算那個碼（一般客戶算進 MTBF／Fail）；933／967 沒設過的碼 IncludeMTBF 預設算（18b）。
  - 等級不夠：頁面說 not-authorized。
- 失敗時：JAM0000.dat 改前改後兩份（拿去 diff）、Jam Code 分頁截圖（第二框的字樣）、瀏覽器 Console、HANDLER LOG 的設定變更紀錄。
- 還原：放回備份的 `D:\HT9045\Error\`（JAM0000.dat 與 `<語言>\<碼>.dat`），重開 wb_serve。
- 依賴：Steven 0928 第 18 題（18a／18b／18c）；其他照 golden（帳本第 1175～1183 行，未明確裁決）；匯出／匯入 CSV 不含這一欄（照 golden）。

### 2-14 每小時一列 TimeData＋OEE 分頁的灰色「關機」（★W48）

- 狀態：已推・會運作（merge `b4c11660`；Steven 0928 09:3x 三項裁決 W48-1／W48-2＝B／W48-3＝B）。
  - **新的機台行為**：wb_serve 開著就每到整點（約 hh:00:04）寫一列 `D:\HT9045_Log\TimeData\<年>\TimeData_<年>.csv`、HANDLER LOG 一行（代碼 220000000），並把每小時的計數歸零（golden HS_Function.cpp:233-238；V906 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Handler\TesterCommWiring.cpp` 第 138、146 行呼叫、第 196～227 行 `W906_TimeDataHourTick`；寫檔本體 `cMyDB.cpp` 第 479～656 行 RecordTimeData，HANDLER LOG 在第 652 行）。VTEST（`bVTESTFunction`）不寫（`cMyDB.cpp` 第 485～486 行）。欄位 `Date, Time, StartTime, HomeTime, ContactTestTime, PauseTime, ProductionTime, JamTime, PowerOnTime, UnloadingCount, JamCount, MUBA, MTBA`，都是上一列以來的秒數（ELA 帳本第 1509 行）。
  - **OEE 分頁**（`D:\HT9045\web\page\eventlog.html` 第 85 行；顏色第 338～339 行）：每天四段——測試／停機／閒置／關機。停機只算 JAM0000.dat 勾「計入 MTBA」的警報（W48-3＝B；沒設過的碼照 golden 預設：代碼含 JAM 且區 01～05 才算，所以 MES1640 沒勾是閒置）；測試＝每次觸壓 Production_Log 第 27 欄 Test Time（W48-1）；關機＝TimeData 算出軟體沒開的時間，灰色 `#8C939B`（W48-2＝B）；沒有 TimeData 的時間算閒置，表格另列「沒 TimeData」。
  - ⚠ **V906 還寫不出第 27 欄，也不寫帶 StopedTime 的警報列**（RecordEndTestTime 三行認領等筆電；Production_Log 寫檔與 SaveErrEventLog 沒移植：ELA 帳本第 1487～1499 行、`EventLogAnalysis\ElaOee.h` 第 82～83 行）。讀不到 Test Time 的觸壓只算次數、不加測試時間（`noTestTime`，`EventLogAnalysis\ElaOee.cpp` 第 431～434 行）。所以 **V906 自己跑的日子只分得出閒置與關機**；測試與停機要 BCB6 寫的 log 才有（頁面也這樣寫，eventlog.html 第 99 行）。
- 前置條件：任一組態；非 915／919；記下 `D:\HT9045_Log\TimeData\` 的現況（B.2 第 4 點）。
- 步驟：
  1. wb_serve 開著過一個整點（hh:00:04 之後），開當年的 TimeData_<年>.csv 與當天 HANDLER LOG。
  2. 再過一個整點，看第二列。
  3. 關 wb_serve 約 20 分鐘再開；過下一個整點後，在事件記錄頁 Query 今天，看 OEE 分頁。
  4. 同一台以前用 BCB6 生產過的日期 Query 一次，看四段與表格。
- 預期（ELA 帳本第 1568～1570 行）：
  - 第 1 步：TimeData 多一列、HANDLER LOG 多一行。**第一列的 PowerOn／StartTime／MUBA／MTBA 是 S113 上線以來累計的**（以前沒人每小時歸零），不是一小時的量；ELA 最多算它一小時。
  - 第 2 步：之後每列 PowerOnTime ≈ 3600。
  - 第 3 步：關掉的那段在圖上是灰色「關機」（放在那一小時的尾端：量對、位置不精確）；表格有「關機」「關機 %」「開著（TimeData）」「SystemStart（TimeData）」「沒 TimeData」。V906 自己跑的今天：測試是 0（`touchdowns` 0），停機多半也是 0（V906 的警報列沒有 StopedTime）。
  - 第 4 步：測試＝各觸壓 Test Time 的聯集；停機只有勾 MTBA 的碼；四個 % 加起來 100.00。
- 失敗時：當年的 TimeData_<年>.csv、當天 HANDLER LOG、`http://127.0.0.1:8045/api/ela` 的快照（存成檔，看 `"oee"`）、OEE 分頁截圖、wb_serve 開關的時間表。整點沒寫：記下當時是否在告警框等待中、開機是否已完成（InitialOK）。
- 還原：新增的 TimeData 列不用還原（golden 也寫）；計數每小時歸零是 golden 行為，不還原。
- 依賴：待 Steven：按了 Start 但沒在測（換盤、浸泡、歸零）算閒置還是運轉（現在算閒置，帳本第 1526～1527 行）；第 27 欄的三行認領（筆電）。

---

## 第三段：網路／FTP（最後做）

> ★W36＝C 之後**模擬版也會真的連 FTP**（手動「立即執行」時）：3-3 只對 Steven 的人自己架的 FTP 做。出貨版半夜自動上傳到客戶的 FTP（3-4）、讓區網主機連進 7016（3-2 第 8、9 項）、HTSET,333 真的啟動機台（3-2 第 6 項），都要 Steven 另外同意。

### 3-1 非 Greatek／TeraPower／TeraProbe 按 Start 不會當掉（W10 修正 `c102083d`）

- 狀態：已推・會運作（`c102083d`）。`c102083d` 以前，W10 的四個呼叫都寫在同一行 `//` 註解後面、沒編進去，兩個伺服器物件是空的；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp` 第 3483～3486 行（906_0625_Steven main.cpp:6042-6046）每次按 Start 都寫 `TCPCommandServer->Active`，非這三家的客戶會當掉。修正：`W906_CmdServersEnsure()`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Tcp\CmdServerPump.cpp`）先把兩個物件建好。
- 前置條件：客戶碼不是 956／967／804（例如開發機 868）；任一組態；照平常開機可以按 Start 的狀態。
- 步驟：
  1. `netstat -ano | findstr "7016 7017"`。
  2. 按 Start、Pause／Stop 各一次；再設 `HT9045_TESTERCOMM=0` 重開、再按 Start。
- 預期結果：netstat 沒有 7016／7017 LISTENING；按 Start 不當掉；`HT9045_TESTERCOMM=0` 時也不當掉（物件仍會建，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Handler\TesterCommWiring.cpp` 第 104 行）。⚠ `HT9045_TESTERCOMM=0` 時，GPIB／RS232 配方在 On-Line（或 TCP/IP 配方在 Off-Line）按 Start 會先被 P8 B4 擋下（`GPIB no execute!!`，S-2）——那是 golden 的拒絕，不是當掉；要驗「不當掉」就切 Off-Line 再按。
- 如何還原：不用。
- 依賴：W10＝B。`forms\fMain.cpp` 第 235 行（Jimmy 的檔）的 `W906_TcpServersCreate(this)` 已移到註解前面（`b2a7349c`，筆電同意）；Ensure 現在只是不做事的保險。

### 3-2 TCP 指令伺服器 7016／結果伺服器 7017（W10，14 項）

- 狀態：已推・會運作，**只在客戶碼 956（Greatek）／967（TeraPower）／804（TeraProbe）**（`CosFunction.bEnableHandlerResultServer`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CosFunction.cpp` 第 1645、1962、2010 行；預設 false）。
- 前置條件：
  - 只在客戶碼本來就是 956／967／804 的機台做；**不要為了測 7016 改客戶碼**（客戶碼會改很多行為），除非 Steven 同意；
  - 沒設 `HT9045_TCPCMD_SIM`、`HT9045_TESTERCOMM`；
  - realfile_guard snap（config.ini、lastdata.dat、作用中配方的 HandlerCondition.Data／Tester.Data／Contact.Data），並照備份段 B.2 第 3 點另存 lastdata_backup*.dat：S-a 的 16 個寫入會真的寫這些檔；
  - 先確認 Windows 防火牆：7016／7017 只開給 MES／AMR 主機（第 8 項）；
  - 測試用戶端先用同一台 PC 的 127.0.0.1（例如 PowerShell 的 System.Net.Sockets.TcpClient 或 TCP 測試工具），同一狀態的 BCB6 回覆要另外錄一份來比。
- 步驟與預期結果（TESTERCOMM 帳本第 650～663 行；log 在 `D:\HT9045_Log\TCPIP_Log\YYYY_MM_DD\YYYY_MM_DD_HH.txt`，每小時一個檔）：

| # | 做什麼 | 預期 |
|---|---|---|
| 1 | `netstat -ano \| findstr "7016 7017"` | 只有 wb_serve 在 LISTENING，而且只在 956／967／804 |
| 2 | 看 TCPIP_Log | 開機就有 `[7016] Server Listen`、`[7017] Server Listen`（golden FormShow），第一次 START 再一次（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp` 第 13529 行格式 `[%4d] Server Listen`） |
| 3 | 同一個狀態送 `HTGR,109`／`HTGR,205`／`HTGR,801`／`HTGR,802` | 回覆跟 BCB6 byte 相同（ASCII `HTSR,<id>,<欄位>,`，沒有 CR/LF）；告警框開著時 HTGR,801 回 Down（R3） |
| 4 | 先送 `HTGR,1` 停一秒再送 `01,`；送 `HTSET,322,` 再送 `5,`；一次送兩條 | 一個回覆；BinSel 頁是 bin 5（不是 bin 0）；兩個回覆照順序 |
| 5 | 送 3000 byte 沒有逗號的 `HTSET,403,…`；送 200 byte 的 403 文字 | log `#Overflow# N bytes dropped (limit 2048)`，程式沒掛，下一條照答；200 byte 那條跳對話框（BCB6 會溢位）。**403 會跳擋住畫面的對話框** |
| 6 | HTSET,333 在 HALT、**手握 E-STOP**；HTSET,334；手動教導中送 333；非 TeraPower 且沒開 bRemoteLotStart 送 333 | 跟網頁 START 一樣；334 會暫停；教導中 → NG；後者 → NG（golden）。**333 會真的啟動機台：出貨版要 Steven 同意、一人手握 E-STOP** |
| 7 | 先用 BCB6 或第二個 wb_serve 佔住 7016，再開 | `Socket Server Open Error!!`（排他綁定）；7017 那次不會開（golden 的 catch 在 7016 就跳出） |
| 8 | 防火牆 | 7016／7017 只開給 MES／AMR 主機。打開後區網任何主機都能啟動／暫停、開批、拿 Supervisor（不用密碼）、清計數、改運轉模式、開關 site、叫 AMR 補盤（golden 本來就這樣）。**要 Steven 同意才讓區網主機連** |
| 9 | HTSET,812／811 跟真的 AMR 走一次 | 帳本沒寫預期字串，上機時記下實際值。**要 Steven 同意** |
| 10 | 720／700／702 之前先 realfile_guard snap；S-a 的指令各送一次 | config.ini 與配方 HandlerCondition.Data／Tester.Data／Contact.Data 真的被寫（314／315／316／331／332／804 寫 config.ini、350 寫 Contact.Data、461～470 寫 HandlerCondition.Data、710／712／713 寫 Tester.Data）；check 的差異只在送的那幾個鍵 |
| 11 | 兩個 client 同時連；拔掉 7017 的 client 再按 START | 兩邊都收到回覆（golden 廣播）；下一次 START 重開 7016、斷掉所有 7016 連線（golden） |
| 12 | 送 `HTSET,702,abc,LOT,` | log `#Exception#`、不回覆、程式不掛（★W41＝A，Steven 0928：只記錄、不回覆） |
| 13 | client 連上但不讀資料，連送指令 | 7016 的回覆送不出去（非阻塞 SendBuf 回 -1，golden 也不看回傳值），tick 不會卡 |
| 14 | ★W40＝A（Steven 0928，`441beb53`）：先備份配方的 `Contact.Data`（SetUp.inf 指的那份）；機台停止時送 `HTSET,354,<G>,`，Contact 頁不要開著 | 回 `HTSR,354,OK,`；`[Torque Control]` 的 `Force Per Pin G`＝送的值、`Force Per Pin N`＝原本的值（不會用 G 重算，跟 golden 一樣）；`bFixNameOfForcePerPinG` 關時寫 `Force Per Pin`／`Force Per Pin Kg`；跟 BCB6 同一份配方比，逐 byte 相同——特別看 `Force Per Pin N` 的格式：V906 寫 `FormatFloat("0.0000", …)`（例 `0.2940`），要跟 BCB6 寫出來的一樣（BCB6 的 FormatFloat 四捨五入方式不同，只有第 5 位剛好是 5 時可能差 1）；運轉中回 `HTSR,354,Fail,`、檔案不變 |

- 另外：`HTSET,322`／`323` 的索引超出 0～255 → log `[ #Ignore#   ] HTSET,322 index N out of range 0..255`，回覆照 golden（`HTSR,322,DoubleContact_On,N,`）；HTSET,354 的兩個寫入已照 golden 打開（★W40＝A，第 14 項）；HTSET,701 回 `HTSR,701,,`；7017 會 listen 但不推 HTSR,501。
- 如何還原：關 wb_serve；`realfile_guard.py check` 看改了什麼、`restore` 還原；netstat 確認 7016／7017 已關。
- 依賴：Steven 20260927 W10＝B（R1 修、R3 要、S-a 要、R2 照 RS232 BCB 接命令、不回 NG）；★W40（354）＝A、★W41（702）＝A（Steven 0928）。安全說明見 `D:\HT9045\.claude\skills\ht9045-gpib-bridge\references\tcp-command-server-7016.md` 第 6 節。

### 3-3 ELA 的 FTP 手動上傳：兩種組態都真的連線（R4 N25-3、W22 N25-4／N25-5；★W36＝C）

- 狀態：已推・會運作（`1c361c13`）。兩種組態都裝 WinINet 傳輸（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaFtp.h` 第 35～44 行、第 65 行）；模擬版只是不在 00:00 自動跑（`ElaSchedule.cpp` 第 497～498、509～511 行），手動「立即執行」會真的連。原本「模擬版只寫 log」（★W36 A）已拿掉。
- 前置條件：
  - Steven 的人架好的 FTP 伺服器（Steven 0928：他們自己架來驗）；**不要**指到客戶的 FTP；
  - 這三個工作只給客戶碼 851（南茂竹北）：機台本來就是 851 才做；要在實驗機改 CUSTOMER_CODE 先 realfile_guard snap（Gerneral.ini）；
  - config.ini `[ChipMos Function]` 的 `bN25_3_EnableULJamLog`（N25-3）、`bN25_4_EnableUpload`（N25-4）、`bN25_5_EnableUpload`（N25-5）開；N25-2 主機 `sN25_2_FTPHost`（`ElaSchedule.cpp` 第 496 行）與各自的上傳路徑不是空的；
  - 手動執行不看 O10；
  - 記下 `D:\HT9045_Log\JamWeek\`、`D:\HT9045_Log\SummaryCount\`、`D:\HT9045_Log\UploadEventLog\` 的現況。
- 步驟：在事件記錄頁的 Auto Jobs 分頁，對 N25-3、N25-4、N25-5 各按一次「立即執行」；確認框寫的傳輸方式要是 WinINet 才按確定。
- 預期（ChangeLog 0928 §11i；ELA 帳本 R4 一節第 1083～1085 行）：
  - 手動被接受時 `POST /api/ela/job` 回 202，下一個 tick 跑。
  - FTP_Log（`D:\HT9045_Log\UploadFile\yyyy\mm\FTP_Log_yyyymmdd.csv`）有 `FTP` 那一行（主機、路徑，最後一欄 `wininet`，`EventLogAnalysis\ElaChipMos.cpp` 第 226 行），接著每一步；**沒有帳號密碼**；連不上時一行 `FTP Failure, Connection Failed`（第 233 行）。
  - FTP 伺服器上有 `<path>/<hostname>/Jam Rate.txt` 與 `BackUp/Jam Rate_yyyymmdd_hhnn.txt`；N25-4／N25-5 是 `Jam_Summary.csv`、`EventLog.txt` 與 BackUp 的兩份。
  - 本機 `D:\HT9045_Log\JamWeek\y\m\d\HadUpload.txt` 三行；SummaryCount／UploadEventLog 有 HadUpload.txt。
  - 模擬版過 00:00 不會自己跑（Auto Jobs 的原因寫 `SIM build: golden sends EL_UPLOAD_JAMWEEK only without SOFT_SIMULTE (main.cpp:21425)`）。
  - N10 BYFILE：V906 目前沒有觸發端（沒有人送 EL_UPLOAD_BYFILE_N10，排程表也沒有它），不用驗。
- 失敗時：當天 FTP_Log、Auto Jobs 分頁該列點開的工作紀錄、`/api/ela/schedule` 存檔、FTP 伺服器端的 log；記下伺服器吃主動還是被動模式。
- 還原：FTP 伺服器上的檔自己刪；本機刪新產生的 HadUpload.txt 與報表檔；realfile_guard restore（若改過客戶碼或 config.ini）。
- 依賴：#22 D-f、★W36＝C（Steven 0928）、★W46（主動／被動：預設照 BCB＝主動，`ElaFtp.h` 第 56 行 `kElaFtpPassive = false`；加客戶可選的分支還在本機）、Steven 0928 第十一題＝A（N25-5 只找 golden 的檔名，帳本 `8c3bb48e`）。

### 3-4 ELA 的 FTP 半夜自動上傳（出貨版；要 Steven 同意）

- 狀態：已推・會運作（出貨版用 WinINet）。
- 前置條件：**Steven 同意**；出貨版；客戶碼 851；O10 開；各開關開；客戶的 FTP 主機可連；備份同 3-3；先問客戶讀不讀 UTF-8 的 `Jam Rate.txt`（★W42）。
- 步驟：等 00:00＋錯開秒數；再做一次拔網路線；關 wb_serve 時正好在傳輸中各一次。
- 預期結果（ELA 帳本 R4 一節第 1083～1085 行、W22 一節第 1690～1692 行）：
  - 伺服器上 `<path>/<hostname>/Jam Rate.txt` 與 `BackUp/Jam Rate_yyyymmdd_hhnn.txt`；`<path>/<hostname>/Jam_Summary.csv`、`EventLog.txt` 與 BackUp 的兩份。
  - 本機 `D:\HT9045_Log\JamWeek\y\m\d\HadUpload.txt` 三行；`D:\HT9045_Log\SummaryCount\…`、`D:\HT9045_Log\UploadEventLog\…` 有 HadUpload.txt。
  - `D:\HT9045_Log\UploadFile\yyyy\mm\FTP_Log_yyyymmdd.csv` 每一步都有、沒有帳號密碼。
  - 自動時 N25-3 在 00:00＋offset 只跑一次，N25-4、N25-5 緊接在後。
  - 伺服器吃不吃主動模式（`ElaFtp.h` 第 56 行 `kElaFtpPassive = false`；★W46 Steven 0928：預設照 BCB＝主動，待上機確認，加客戶可選）、有沒有 SIZE（沒有就用 LIST 的大小）：上機記下。
  - 拔網路線約 6 秒內失敗；關 wb_serve 不會卡 30 秒。
  - Jam_Summary.csv 的 Summary 欄跟 `D:\HT9045_Log\Production_Log\yyyymm\<電腦名>_yyyymmdd.csv` 對得上；Handler 00:00 剛換檔時昨天的 EventLogTxt 能不能複製（CopyFile 的分享模式），上機記下。
- 如何還原：伺服器上的檔請客戶端刪；本機刪 HadUpload.txt 讓它能重傳（需要時）。
- 依賴：#22 D-f、FTP 傳輸＝WinINet（Steven 20260927）；★W36＝C、第十一題＝A（Steven 0928）；★W42 細項、★W46-1 待 Steven。

### 3-5 SECS 遠端改溫度 offset（W9 G34，選做）

- 狀態：已推・會運作（`014e9094`，在 main；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\SECSGEM\uHGemHT9045.cpp` 第 7096～7131 行）。
- 前置條件：SECS/GEM 連著主機；同 2-2 備份 Temperature.Data。
- 步驟：主機送 TEMP_OFFSET 遠端指令一次（小的值）。
- 預期結果：Save 成功就重讀並回 HCACK=0，否則 HCACK=1；檔案變化同 2-2（golden 906_0625_Steven SECSGEM/uHGemHT9045.cpp:3422-3431）。
- 如何還原：同 2-2。
- 依賴：W9＝A、G34＝開（github-46 決定）。

---

## 第四部分：已推但還不會運作（等認領）

> 本體已推，呼叫它的那一行還沒接上。**現在上機看不到這些行為**；下面寫的是接上之後怎麼驗。可以順便確認「現在沒有改變」。

原本的 4-1～4-6 在 0928 都已接上，搬走了：

| 原編號 | 內容 | 接上的 commit | 現在在 |
|---|---|---|---|
| 4-1、4-2 | Tester 切換的 D1～D7（含 D4 強制回 Operator） | `c4700fd6`（fMain.cpp 呼叫端）＋`TesterCommWiring.cpp` 第 97 行安裝 | S-3 |
| 4-3 | 主畫面托盤給 Motion View（S118） | `b243f15e` | 1-10 |
| 4-4 | Tester 介面設定頁「不存就關」 | `7f4e30a1`（`W906_WindowEdgeRegister`） | 1-13 |
| 4-5 | 溫度設定頁 TS-7 | `5e16f04f`＋main 的 `70aa17e8` | 1-11 |
| 4-6 | Configuration 頁走伺服器 | main 的 `64ade3b7` | 1-12 |

### 4-7 崇越（868）批結束的 OEE／警報報表（ELA R3，N34）

- 狀態：已推・不會運作。本體 `929ced5a`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaReports.cpp` 檔尾 ChipAdvancedFunc）；送出端是 St01 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp` 第 7082～7095 行，還是 `#if 0`（St02-M 已請 St01 解閘）。
- 前置條件（接上之後）：客戶碼 868（CC_CYUEAN）；O10 開、N34（`bN34_GenerateOEEAlarmRpt`）開；記下 `D:\HT9045_Log\Product_Loader\OEEAlarmRpt\` 現況。
- 步驟：跑一批到 lot end；同一批同一天再 lot end 一次。
- 預期結果（ELA 帳本 R3 一節第 1127～1128 行）：`sN34_OEEAlarmRptPath`（預設 `D:\HT9045_Log\Product_Loader\OEEAlarmRpt`）有 `ClipAdv_<LotID>_<y><m><d>.csv`，區間是最後一個 Lot Start 到 lot end；Lot ID 第一個字還在（golden 會少第一個字，已修）；第二次 lot end 取代舊檔。
- 如何還原：刪掉新產生的 csv。
- 依賴：St01 解閘 fLotInfo.cpp（第 7084 行 `#if 0 // GATE (W906-PROD-S117-S25-CYUEAN)`，0928 下午仍閘著）；★W42＝A（UTF-8；Excel 雙擊中文會亂碼，細項 W57～W62 待 Steven）；★W47＝A（批號第一個字照修正，Steven 0928）。

---

## 第五部分：未推、等裁決（只在 St02 本機分支；不是步驟）

| 代號 | 內容（白話） | 本機分支與 hash | 裁決後 |
|---|---|---|---|
| ★W42（報表編碼） | Steven 0928＝A：自動報表檔 UTF-8；報表裡的訊息只存英文，畫面依主畫面語言顯示。第 1～3 步做完，細項 W57～W62 待 Steven | `v906/steven-w42-encode` `9a4bfcce`（已合 gpib-widget `db863df4`，只在本機） | 裁決後兩組態編譯、merge-tree 再推；報表寫檔集中在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaReports.cpp` 第 161 行 |
| ★W46（FTP 主動／被動） | Steven 0928：預設照 BCB（主動，待上機確認），加客戶可選 | `v906/steven-w46-passive` `4efd91e1` | ★W46-1 待 Steven |
| ★W36-1 | 原型（commit 標題 `WIP W36-1 SimNetMask prototype`），**永遠不推** | `v906/steven-w36-1-proto` `1851e6f9` | 待 Steven |

- 已推、從這張表拿掉的：★W39（TS-8＝A，`959c2236`）與 Q52（TS-9＋TesterIF 關頁掛勾，`7f4e30a1`）→ 見 1-11、1-13。

---

## 風險（上機前讀）

1. **State Record 背景執行緒跟 tick 執行緒同時寫 log（P4 之後才有，Jimmy 的檔，未修）**
   - 位置：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cStateRecord.cpp` 第 1410～1420 行；背景工作 `RunStateRecordJob`（第 1420、1473、1479 行）呼叫 RecordProcess。
   - P4 之後 RecordProcess 是 golden 本體（`cMyDB.cpp` 第 1893～1902 行），會改全域變數 `ExString` 並寫 HANDLER LOG／EventLogTxt；主執行緒同時也在寫。第 1412～1413 行的註解自己寫著「golden 本體回來後要改成交給 tick 執行緒」，這個條件已經成立。
   - 上機會碰到的時機：每次做 State Record（報警時自動或手動）。可能的現象（推論，沒實測）：那段時間的 log 列錯亂或少列；最壞是 wb_serve 當掉（兩條執行緒同時改同一個字串）。上機時記下每次 State Record 的時間，當掉或 log 異常時一起回報。
   - 出處：分支 `v906/steven-handoff` 的 `docs/handoff/ST02_P4_STALE_COMMENTS_20260928.md` §1 第 1 項（`git show origin/v906/steven-handoff:docs/handoff/ST02_P4_STALE_COMMENTS_20260928.md`）；建議修法（擁有者決定）：背景工作把訊息收在 job 裡，tick 執行緒收工作時再寫。
2. **（已解，`95193fb2`）32-site 機台少一筆 Auto homing 紀錄**
   - `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\atester_32Site.cpp` 第 414 行的替身 `#define` 已註解掉（筆電同意的認領），第 2389 行 `MyDBIProcessNew("Motion", "WAR2206", "Auto homing", "0316")` 現在照 golden 寫。
   - 上機要看：32-site 機台自動回原點時，HANDLER LOG／EventLogTxt 有一筆 WAR2206 "Auto homing"。
3. **會失敗的網頁探針與過時的瀏覽器字串**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\s1_eventlog_probe.py` 第 278～279 行期待 stdout，P4 之後一定失敗；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\ChanAction.cpp` 第 243～248 行送到瀏覽器的診斷字串說 MyDBIProcess「都沒有落地」，已過時（出處同上 §1 第 3、4 項）。
4. **7016 打開後區網任何主機都能操作機台**（golden 本來就這樣）：啟動／暫停、開批、拿 Supervisor 不用密碼、改 config.ini 與配方。防火牆只開給 MES／AMR（3-2 第 8 項）。
5. **W9 會累加而且是永久的**：每一次 `SETTESTOFFSET_` 都加在現值上，之後用這個配方的每一批都帶著（2-2）。
6. **ELA 的 POST 沒有 Origin 檢查也沒有等級檢查**：`POST /api/ela/job`、`/api/ela/summary`、`/api/ela/query`。wb_serve 只聽 127.0.0.1，但機台瀏覽器裡開的別的網頁可以盲送 POST 排一個工作（ELA 帳本第 1595～1598 行）。上機時機台瀏覽器不要開不相干的網頁。
7. **Q24 會蓋掉 PW_Editor 在執行中做的修改**（golden 行為，2-11）。
8. **（已解）兩棵樹的問題**：gpib-widget `db863df4` 已含 St01 的 Q9／Q24／observer.get 權杖與伺服器半邊（0.2）。只剩 Speed 滑桿事件、TS-1 佇列在 St01 分支。
9. **`POST /api/testercomm/<key>?cmd=` 沒有權限閘**（P8 B7，未解）：Manual Start／Manual Test／TTL Manual 一按就打到真設備；上機期間只讓在場的人開 testercomm 頁（S-2）。
10. **wb_serve 每小時會自己寫 TimeData 並把每小時計數歸零**（★W48，golden 行為，2-14）：同一台同時開 BCB6 HT9045.exe 會變成兩個程式各寫一份，不要同時開。

---

## 附錄 A：其他 St02 已推、有「上機要看」的小項目

### A-1 ESD 程式開著時 IonBar 上電序列（ESD G5＝B）

- 狀態：已推・會運作（`20829c7a`，在 main）。
- 前置條件：真的 ESD 程式（**手動開**）＋HT IonBar 控制器；`iUseHTIonBarFunction` 開；任一組態。
- 步驟：開 wb_serve，等一陣子；再手動開 ESD 程式；看各 IonBar。
- 預期結果（TESTERCOMM 帳本第 562 行）：V906 **不會自己啟動** `D:\ESD_Program\EXE\ESD_Program.exe`；ESD 視窗出現後約 50 秒（每秒一格，450→500），警報感測器 on 的那幾支 IonBar 收到 `ESD_HT_IONBAR_ControllerN_PowerOn`、只收一次；感測器 off 的那支等它 on 才送。
- 如何還原：關 ESD 程式。
- 依賴：RULINGS_20260927 第 24 條（#32 G5＝B）。

### A-2 每次測試開始記一行 socket ID（W11）

- 狀態：已推・會運作（`9d790ff2`＋St01 `9b78d312` 裝本體，在 main）。
- 前置條件：任一組態；會跑測試的配方。
- 步驟：跑幾個 test start。
- 預期結果：每次 test start 多一行 socket ID 的 CSV（commit `9d790ff2`）。帳本沒寫檔案路徑與確切內容，上機時記下實際值。
- 如何還原：不用。
- 依賴：W11＝B。

### A-3 Castle Tester 的 READTEMP／READDAQ 回覆字串（GB P1）

- 狀態：已推・會運作（照 golden 保留的疑似 bug）。
- 前置條件：Delta Castle Tester 經 GPIB 連著。
- 步驟：Tester 送 READTEMP 與 READDAQ。
- 預期結果：golden 的格式字串尾巴多一個 `%`（無效的 printf 轉換）；V906 照翻。回給 Tester 的確切字串帳本沒寫，上機時記下實際值，並跟 BCB6 的 H9046_32GPIB.exe 比（TESTERCOMM 帳本第 95 行）。
- 如何還原：不用。
- 依賴：P1 記下但照 golden 保留，給 Jimmy 決定。

---

## 附錄 B：這份清單用的來源

- 帳本：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\TESTERCOMM_PORT_LEDGER.md`（W1、ESD G5、W9、W10、Q41 (d)）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ELA_PORT_LEDGER.md`（P3、W15／W18／W19、R1～R6、P7a、P7、W22）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\CMYDB_PORT_LEDGER.md`（P3、P4、W7）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\S118_MOTIONVIEW_TRAY_PRODUCER.md`。
- skill：`D:\HT9045\.claude\skills\ht9045-st02-workflow\SKILL.md` 與 `references\current-state.md`、`w9-remote-temp-offset-plan.md`、`w10-tcp-command-server-plan.md`、`q9-q24-login-plan.md`；`D:\HT9045\.claude\skills\ht9045-gpib-bridge\references\tcp-command-server-7016.md`、`references\gb-p8-bringup-plan.md`（S-2）；`D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\SKILL.md`；`D:\HT9045\.claude\skills\ht9050-construction\references\progress-st02.md`。
- ChangeLog：`D:\docs\ChangeLog\CHANGES_20260927_Steven02.md`、`D:\docs\ChangeLog\CHANGES_20260928_Steven02.md`（0928 下午的項目：§11k～§11p 與摘要第 22～28 列；11:13～11:31 那一批認領 `b2a7349c`、`c4700fd6`、`4d3468a3`、`95193fb2`、`b243f15e`、`2b037895` 在 ChangeLog 裡沒有獨立一節，依 commit 訊息與程式碼寫）。
- 交接分支 `v906/steven-handoff`：`docs/handoff/ST02_P4_STALE_COMMENTS_20260928.md`、`docs/handoff/ST02_FMAIN_CLAIM_20260928.md`、`docs/handoff/ST02_QUESTIONS_20260928.md`。
- commit 訊息（gpib-widget、St01 分支、St02 本機分支）：`git -C D:\HT9045 log --grep=上機要看 --grep="real machine" origin/v906/steven-gpib-widget`。
- 工具：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\realfile_guard.py`。
