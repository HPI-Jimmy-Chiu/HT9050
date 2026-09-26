<!-- AI(W906-ELA-P0) 20260926：Event Log Analyzer 轉換 P0 帳本（St02）。內容由唯讀盤點產生（原始碼 D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code，
     SVN r891＝Rev891.0），之後各階段在本檔末尾追加。計畫：docs/ELA_20260926_WEB_CONVERSION_PLAN.md；skill：ht9045-eventlog-analyzer。
     §8 的決策題已抄進 skill ht9050-construction references/progress-st02.md「待使用者裁決」#15～#23。 -->

# ELA P0 inventory: Event Log Analyzer (TfrmELA) -> V906

- Date: 2026-09-26. Plan phase P0 ("定基準版本；Analyzer.cpp 每個函式分類 core／report／ui／comm；函式 100% 分類").
- Plan: `D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-web-conversion-plan.md`. Skill: `...\ht9045-eventlog-analyzer\SKILL.md`.
- Source read: `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code` (cp950, decoded to UTF-8 copies for reading only; nothing in the repo was changed).
- Writer side read: V906 `HT9011UC_Cpp_V3.33.906.0\LogObjects.cpp`, `cMyDB.cpp` (golden bodies in `#if 0`), `Public\MyStringList.cpp`, `vclcompat\TStringList.cpp`, `cObserver.cpp`, `cprod.cpp`, `Interface\InterfaceSYS.cpp`; golden V912 `note.cpp`, `main.cpp`, `HS_Function.cpp`, `uLotInfo.cpp`, `cConfiguration.cpp`, `cprod.cpp`.
- Sample data looked at: `D:\HT9045_Log\EventLogTxt` (707 csv, 2023-03 .. 2026-05, several customer machines), `D:\HT9045_Log\Production_Log`.
- Method: brace-matching extractor over comment/string-stripped source, every function start line checked against its signature line; each function classified by hand after reading its body. Coverage: **100 %** (88/88 in Analyzer.cpp; 330/330 across all 13 .cpp + XLSfile.pas).

Category rules used

| cat | meaning |
|---|---|
| core | pure analysis: file discovery, CSV parsing, counting, summaries, Jam level, MTBA/MTBF rules, Top/Filter data sets, Production_Log counts. "mixed ui" in notes = the body also writes widgets; the port keeps the data path and drops the widget writes. |
| report | writes report files or customer formats (summary .txt, Jam Rate, Jam_Summary, ChipAdvanced csv, XLS). |
| ui | VCL widgets only: grids, charts, dialogs, editors, filter check boxes. |
| comm | WM_COPYDATA / EL_* dispatch, the analysis thread, timers that scan or schedule, FTP, config reload for uploads. |
| util | string / file / ini / version helpers. |

---

## 1. Baseline

### 1.1 Candidates found

| # | where | date | version string | content |
|---|---|---|---|---|
| A | `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code` (SVN working copy, root `D:\HT9045_SVN_TempFile`, repo `file://backsrv/RD/軟體備份區/邏輯機台/SourceCode/SVN`) | files 2025-11-25 (r891), text updated 2026-03-30 | WC revision 904; `Analyzer.cpp` Last Changed Rev **891** (Steven, 2025-11-25 08:18); `.bpr` `MajorVer=20 MinorVer=25 Release=891 Build=0`, `FileVersion=20.25.891.0` | full source; `svn status` = no modified versioned file (only unversioned `*.bak`, `*.~*`) |
| B | SVN repository HEAD (checked with `svn info -r HEAD`, `svn status -u`) | HEAD = r912 | `EventlogAnalyzer` Last Changed Rev 891 | no commit to EventlogAnalyzer after r891 (log: r865, r872/873, r875, r891) |
| C | `\\backsrv\RD\軟體備份區\邏輯機台\SourceCode\EventlogAnalyzer\EventlogAnalyzer_Rev891.0_20251125.7z` | 2025-11-25 | name = Rev891.0 | password-protected 7z, contents not listable; older `Old\` = Rev812, 813.0, 819.0, 842.0, 865.0, 873.0, 875.0 |
| D | `D:\HT9045_Updater_NSIS\EventlogAnalyzer\EventlogAnalyzer.exe` (and the identical `... - 複製` copy) | 2025-11-25 08:17 | FileVersion **20.25.891.0**, MD5 74AADFD3... | the shipped binary, built 1 min before the r891 commit |
| E | `D:\HT9045_SVN_TempFile - 複製\EventlogAnalyzer\Code` | 2026-03-30 | same | `Analyzer.cpp` MD5 identical to A |
| F | `E:\實驗用工具\EventlogAnalyzer\EventlogAnalyzer.exe` (+ `.7z`, `Code\`) | 2021-04-09 | FileVersion 1.0.684.0 | old |
| G | `D:\EventlogAnalyzer\` | - | - | only `.github\agents\EventlogAnalyzer.agent.md` and empty `backup\` (the agent note names `EventlogAnalyzer_Rev891.0_20251125/`) |
| H | `*.bak`, `*.~*` in A | 2025-04 .. 2025-07 | `.~bpr` Release=873 | older than r891: `Analyzer.cpp.bak` has no VTEST/MTBF (0 hits vs 20), `uChipAdvancedFunc.cpp.bak` differs only in `frmELA->logRecords` vs member `logRecords` (2 lines) |

SVN log text of r891: `EventlogAnalyzer_Rev891.0` / `//RogerYang 20251104 : 偉測MTBF文件生成`; changed `Analyzer.cpp/.dfm/.h`, `Common.cpp/.h`, `EventlogAnalyzer.bpr/.res`.

### 1.2 Recommendation

**Baseline = SVN r891 = `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code` as it is (FileVersion 20.25.891.0).** "SVN Code 0330" and "Rev891.0" are the same version: 0330 is only the checkout/update date of the working copy; the source content is r891 and the repository has nothing newer (HEAD r912 touches other projects). The matching binary for the P1 oracle is `D:\HT9045_Updater_NSIS\EventlogAnalyzer\EventlogAnalyzer.exe` (20.25.891.0). The `.bak`/`.~` files are older and are ignored.

Optional check (needs a human): open the password-protected `EventlogAnalyzer_Rev891.0_20251125.7z` and diff it against A; expected identical.

---

## 2. Architecture as it really runs (facts that change the plan)

- `TAnalysisProcessThread::Execute` (:208-215) only calls `Synchronize(AnalysisThreadProcess)` every 500 ms. **Every EL_* job (full queries, file writes, FTP) runs on the VCL main thread.** While the non-pumping part of a job runs (CSV parse loop, report write), a Handler `SendMessage(WM_COPYDATA)` (golden InterfaceSYS.cpp:479) waits, so the Handler's calling thread (golden callers: `TfMain::Timer2Timer` at 00:00, form buttons, lot end) can freeze for the length of a 31-day query.
- `LoadFavorite` (:1644) calls `Application->ProcessMessages()` for every file and folder, so during a query WM_COPYDATA, Timer2 and user clicks can re-enter and start a second query on the same shared state (maps, lists, date pickers).
- The date pickers are the parameter bus: `O06SaveSummaryData`, `N10SaveSummaryData`, `DoVTestSaveSummary`, `ChipAdvancedFunc::AnalysisLog`, `SaveJamCodeFor7Days`, `SaveJamSummaryFor30Days` set `dtpStart*/dtpEnd*` and then call `btnQuery->Click()` or `GetEventLogTextToVec()`. After any EL_* job the screen shows that job's range.
- Config is held in widgets: `ReadConfig` copies config.ini into check boxes / edits; uploads read `edN10UserName->Text`, `edtN25_2_Host->Text`, `edN34->Text`, etc. The "Upload Setting" tab has no save handler (read-only mirror).
- OEE: three `TImage` canvases (`ImageOeeAna`, `ImageOeeChart`, `ImageDayOee`) drawn with `random()` data in FormShow, tabs hidden. **There is no TeeChart** (the .dfm has 0 TChart objects; the plan text "OEE 三組 TeeChart" is wrong). There is no real OEE computation in Analyzer.cpp; the only "OEE" number is ChipAdvanced's `(input-fail)/input`.
- `sgProdLog` / `tsProdLog` (Production_Log grid) is hidden and never filled; `slAllData` is filled and never read.


## 3. Totals per category

Line counts are function bodies from the signature line to the closing brace (comments inside included; file-level declarations, tables and includes not counted).

| category | Analyzer.cpp funcs | Analyzer.cpp lines | all units funcs | all units lines |
|---|---:|---:|---:|---:|
| core | 22 | 1465 | 58 | 1955 |
| report | 9 | 199 | 45 | 775 |
| ui | 44 | 990 | 48 | 1078 |
| comm | 9 | 328 | 46 | 1210 |
| util | 4 | 80 | 133 | 2501 |
| **total** | **88** | **3062** | **330** | **7519** |

"all units" = 13 .cpp + XLSfile.pas (13 Pascal routines). Without XLSfile.pas: 317 functions / 7302 lines.


Per file:

| file | funcs | lines | core | report | ui | comm | util | unused/dead |
|---|---:|---:|---|---|---|---|---|---:|
| Analyzer.cpp | 88 | 3062 | 22/1465 | 9/199 | 44/990 | 9/328 | 4/80 | 10 |
| Common.cpp | 49 | 948 | 14/241 | 0/0 | 3/58 | 0/0 | 32/649 | 3 |
| uAnalysisEventLogText.cpp | 5 | 26 | 5/26 | 0/0 | 0/0 | 0/0 | 0/0 | 0 |
| uAnalysisProductData.cpp | 5 | 41 | 5/41 | 0/0 | 0/0 | 0/0 | 0/0 | 5 |
| uAnalysisProductionLog.cpp | 12 | 182 | 12/182 | 0/0 | 0/0 | 0/0 | 0/0 | 5 |
| uChipAdvancedFunc.cpp | 16 | 231 | 0/0 | 16/231 | 0/0 | 0/0 | 0/0 | 0 |
| uChipMosZHUBEI_Func.cpp | 13 | 392 | 0/0 | 6/124 | 0/0 | 7/268 | 0/0 | 0 |
| TfFTP.cpp | 30 | 614 | 0/0 | 0/0 | 0/0 | 30/614 | 0/0 | 8 |
| FileInfo.cpp | 40 | 491 | 0/0 | 0/0 | 0/0 | 0/0 | 40/491 | 36 |
| HTMD5.cpp | 26 | 491 | 0/0 | 0/0 | 0/0 | 0/0 | 26/491 | 26 |
| MyStringList.cpp | 31 | 790 | 0/0 | 0/0 | 0/0 | 0/0 | 31/790 | 21 |
| SgdToXLS.cpp | 1 | 4 | 0/0 | 1/4 | 0/0 | 0/0 | 0/0 | 0 |
| EventlogAnalyzer.cpp | 1 | 30 | 0/0 | 0/0 | 1/30 | 0/0 | 0/0 | 0 |
| XLSfile.pas | 13 | 217 | 0/0 | 13/217 | 0/0 | 0/0 | 0/0 | 1 |

Cells are functions/lines.


By suggested target (all units):

| target | funcs | BCB lines |
|---|---:|---:|
| drop | 139 | 2402 |
| ElaCore | 36 | 1638 |
| ElaReports | 54 | 956 |
| ElaHub | 16 | 604 |
| web | 25 | 589 |
| V906 | 10 | 442 |
| KYECFTP | 22 | 354 |
| ElaProduction | 14 | 280 |
| vclcompat | 14 | 254 |

---

## 4. EL_* command set and dispatch

### 4.1 Wire format

`Common.h:48-66` `struct M_V { Byte bModeType; Byte bCommandType; Byte bCommand; unsigned iDataSize; Byte bData[256]; unsigned iMessageSize; Byte bMessage[2048]; char cSendData[3000]; }`.
Handler (golden `Interface/InterfaceSYS.cpp:479`, V906 `:552`): `FindWindow("TfrmELA","Event Log Analyzer")`, `bModeType=TYPE_HANDLER_EventLog (0x04)`, `bCommandType=CommandType_EventLog (0x09)`, `bCommand=CMD`, `iDataSize=len(Data)`, `bData[0]=atoi(Data)` (always "1"), `SendMessage(hwnd, WM_COPYDATA, WM_EventAnalysis=6, &cds)` with `dwData=6`, `cbData=sizeof(M_V)`.
Analyzer `OnMyCopyMsg` (Analyzer.cpp:487-558): aliases `pVM` onto `lpData` (`&pVM->bModeType=(Byte*)P->lpData;`), accepts only `wParam==WM_EventAnalysis` and `bModeType==0x04`, switches on `bCommand`. `bCommandType`, `iDataSize`, `bData` are ignored. No reply, no ack (SendMessage returns 0).

### 4.2 Commands (`Common.h:31-41 enum EventLog_COMMAND`)

| value | command | golden V912 sender (trigger) | V906 sender | analyzer handler | what it does | files |
|---:|---|---|---|---|---|---|
| 0 | `EL_UPDATE_PARAMETER` | `cprod.cpp:3140` (after config.ini save), `main.cpp:25374` (CC_SCK monitor defaults); analyzer `Timer1Timer` raises it itself at startup (:479-482) | `cprod.cpp:3315` (only V906 caller) | `TfrmELA::ReadConfig` (:3090) | re-read config.ini / Gerneral.ini into widgets. **No re-query.** | reads + writes defaults into `D:\HT9045\config\config.ini` |
| 1 | `EL_UPLOAD_JAMWEEK` | `main.cpp:22066` (`TfMain::Timer2Timer`, 00:00:01-04, `bN25_3_EnableULJamLog`, `#ifndef SOFT_SIMULTE`), `cConfiguration.cpp:7862` (N25-3 Manual button) | none | `ChipMosZHUBEI_Func::UploadJamCode` (uChipMosZHUBEI_Func.cpp:328) | once a day (HadUpload.txt): `Jam Rate.txt` = every JAM-containing record of now-8 00:00 .. now-1 23:59:59, FTP N25 to `<sN25_3_JamLogFTPPath>/<hostname>/` + `BackUp/Jam Rate_yyyymmdd_hhnn.txt` | `D:\HT9045_Log\JamWeek\Y\M\D\` (no zero pad) |
| 2 | `EL_UPLOAD_CHIPADV_LOTEND` | `uLotInfo.cpp:2332` (CC_CYUEAN 868 && `bN34_GenerateOEEAlarmRpt`, lot end) | none | `ChipAdvancedFunc::AnalysisLog` (uChipAdvancedFunc.cpp:22) | last "Lot Start, Lot ID:" within 8 days -> range lot start..now -> full query -> metrics / "OEE" / alarm-ratio csv | appends `<sN34_OEEAlarmRptPath>\ClipAdv_<LotID>_<y><m><d>.csv` |
| 3 | `EL_UPLOAD_SUMMARY` | `main.cpp:22071` (00:00, `bN25_4_EnableUpload`), `cConfiguration.cpp:7867` | none | `ChipMosZHUBEI_Func::UploadSummaryCount` (:352) | once a day: `Jam_Summary.csv` over now-31 .. now-1, intervals split at Recovery containing `CLEAN_OUT`, device count from Production_Log, FTP N25 (`sN25_4_UploadPath`) + BackUp | `D:\HT9045_Log\SummaryCount\Y\M\D\` |
| 4 | `EL_UPLOAD_EVENTLOG` | `main.cpp:22076` (00:00, `bN25_5_EnableUpload`), `cConfiguration.cpp:7872` | none | `ChipMosZHUBEI_Func::UploadEventLog` (:372) | once a day: copy yesterday's `EventLogTxt\yyyy\mm\EventLogTxt_yyyymmdd.csv` to `EventLog.txt`, FTP N25 (`sN25_5_UploadPath`) + BackUp | `D:\HT9045_Log\UploadEventLog\Y\M\D\` |
| 5 | `EL_UPLOAD_BYFILE_N10` | **no live sender**: `HS_Function.cpp:436-440` is commented out | none | `ChipMosZHUBEI_Func::UploadByFile_N10` (:392) | each row of `UploadFile.csv` (src dir, dst dir, src file, dst file) -> FTP N10 (`cN10Ftp*`); file never truncated | reads `D:\HT9045_Log\UploadFile\UploadFile.csv`; FTP_Log |
| 6 | `EL_VTEST_MTBF_SUM` | `HS_Function.cpp:272` (`bVTESTFunction && bO19_AutoRecordReportByEveryWeek`, weekday==`iO19_WeekPeriod`, 00:00-00:05; can fire repeatedly in that window) | none | `TfrmELA::DoVTestSaveSummary` (Analyzer.cpp:3240) | range now-7 00:00 .. now-1 23:59:59, full query, `SaveSummary(asO19_SavePath,"",bDetail=false)` | `D:\MTBF_Summary\<HandlerID>-SummaryData_<today>.txt` (default path) |

Not an EL command but also scheduled inside the analyzer: `Timer2Timer` (50 s) -> O06-4 / N10-3 summary saves (see ledger :3010).

### 4.3 How they are dispatched

1. `OnMyCopyMsg` (VCL main thread) only sets `MyAnalysisThread->bEL_<cmd>=true` (:499-554). Repeated sends before the job runs coalesce into one run.
2. `TAnalysisProcessThread::Execute` (:208-215): `do { Synchronize(AnalysisThreadProcess); SleepEx(500,true); } while(!Terminated);` -> the job body runs on the main thread.
3. `AnalysisThreadProcess` (:154-196): `else if` chain, first raised flag wins, flag cleared, handler run to completion. Priority: UPDATE_PARAMETER > UPLOAD_JAMWEEK > UPLOAD_SUMMARY > UPLOAD_CHIPADV_LOTEND > UPLOAD_EVENTLOG > UPLOAD_BYFILE_N10 > VTEST_MTBF_SUM. One job per >= 500 ms.
4. `ClearAllActive` (:198-206) omits `bEL_UPLOAD_BYFILE_N10` (harmless in BCB because TObject memory is zero-filled; must be initialised in C++17).
5. Handlers reach the query through the form: set `dtp*` pickers -> `btnQuery->Click()` (`SetStringGrid` -> `GetEventLogText` -> `ListProductionLog`) or `GetEventLogTextToVec()`; results are read back from `MySummary`, grids and memo.
6. Start-up: `FormShow` -> `Timer1` one-shot -> first query for today 00:00..now + `bEL_UPDATE_PARAMETER`; `Timer2` enabled.

---

## 5. EventLogTxt CSV as the analyzer reads it (`TfrmELA::GetEventLogText`, Analyzer.cpp:782-1181)

| step | rule | ref |
|---|---|---|
| file set | recursive scan of every `*.csv` under `labPath` (default `D:\HT9045_Log\EventLogTxt`), hidden entries skipped, into `lstEventLog` | `LoadFavorite` :1644-1678, `ImportProductionLod` :1680-1712, called from `SetStringGrid` :775 on every query |
| day loop | `for d in 0..iDate-1`, `iDate = trunc(end)-trunc(start)+1` | :804, :3235-3238 |
| file match | path contains `FN1="yyyymmdd.csv"` AND `FN2="EventLogTxt_"`; CC 915/919 (VTEST, Gerneral.ini `[System] CUSTOMER_CODE`): `FN1="yyyymmdd"`, `FN2="_EventLogTxt_"`. Case-sensitive substring. **Every** matching file is read (no de-dup). | :807-821, :3139 |
| read | `TStringList::LoadFromFile` (bytes as ANSI = cp950 on the machines, no conversion; CR/LF split) ; line 0 skipped as header | :829-830 |
| split | `tsRow->CommaText = line` (BCB6: delimiters are ',' **and unquoted blanks**; quotes honoured, `""` escape; leading blanks skipped) | :833 |
| width | `Count < 9` -> skip (Recipe optional) | :834 |
| fields | `eEventLog` 0 Date, 1 Time, 2 UnitName, 3 AlarmCode, 4 Recovery, 5 StopedTime, 6 Duplicate, 7 Message, 8 ErrorPart, 9 Recipe | Analyzer.h:112-125 |
| date/time | Date `YYYY/MM/DD` or `YYYY-MM-DD` (len 10); Time `HH:NN:SS` or `HH:NN:SS.mmm`; else skip | :837-847, Common.cpp:630-690, 715-800 |
| range | `dtStart <= t <= dtEnd` (inclusive) | :850 |
| duplicate | Duplicate field contains "1" -> skip row entirely | :852 |
| stop event | AlarmCode **contains** JAM, WAR or MES and `Count > 5`; `iMS = atoi(StopedTime)` (**seconds**) -> total/day/hour stop count+time; row to `slStopList` | :857-881 |
| alarm (MTBA) | `GetJemIncludeMTBA(UnitName, AlarmCode)` -> alarm count/time total/day/hour, `slAlarmList` | :882-899, :2812-2836 |
| failure (MTBF) | `GetJemIncludeMTBF(UnitName, AlarmCode)` -> fail count/time, `slFailList` | :901-918, :2838-2882 |
| JAM | AlarmCode **starts with** JAM -> `iJAMCount`, `slJamList`, `mapJamSummary` and `mapAlarmSummary` keyed by AlarmCode (count, stop time; first Unit/Message kept) | :927-953 |
| WAR | starts with WAR and UnitName does not contain "Motion" -> `iWARCount`, `slWarList`, `mapWarSummary`, `mapAlarmSummary` | :954-981 |
| MES | starts with MES -> `iMESCount` only | :982-985 |
| by unit | `mapUnitName[UnitName]` (keys = the first 30 of the 31 `cbJamArea` items "01 Input Arm" .. "30 Fix Tray 6", Analyzer.dfm:1946-1977; "31 Cylinder" has no check box, so its rows never reach By Filter, and the ctor logs "Need to add check box of UnitName") -> `ListByUnitName[i]` | :990-995, :265-278 |
| by function | UnitName != "Process": Message contains RTC -> RTC; Unit=="20 ESD System" or msg ESD / "Ion fan" -> ESD; Unit=="15 Temp. Controller" or msg Temperature -> Temp; "2D" -> 2DID; "OCR" -> OCR; "clean" -> AutoClean; Unit=="07 Tester I/F" -> Yield (first match wins) | :1000-1036 |
| ChangeLog | Unit=="ChangeLog" && ErrorPart=="ParameterLog" -> label block; never matches in practice; loop bound `sizeof(strPara)`=84 over 21 items (UB) | :1039-1162 |
| JAM0000.dat | `D:\HT9045\Error\English\JAM0000.dat`, section = UnitName, keys `<code>` (level), `<code> IncludeMTBA`, `<code> IncludeMTBF`; missing keys are **written** with the default | Common.cpp:124-139 |

`GetEventLogTextToVec` (:1189-1267) uses the same file/row/range/duplicate rules without the stop filter, has no CC915/919 variant and does not copy Recipe.
Production_Log (`ListProductionLog` :2316-2466): files whose path contains `yyyymmdd.csv` under `D:\HT9045_Log\Production_Log`; blanks replaced by `_` before CommaText when the line has no quote; `eLoadTime` (field 5) parsed; range exclusive `start < t < end`; contact count/test time only when `eOrderTest` (13) changes; `eTestTime` (27) split at '.'.
Encoding: the analyzer never converts; all comparisons are ASCII, so cp950 vs UTF-8 only matters for displayed Message text.

### 5.1 Writer side (Handler) and mismatches

| writer | row layout | where |
|---|---|---|
| header, normal | `Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe` | V906 `LogObjects.cpp:74-76` (golden TfMain ctor) |
| header, SPIL (`IniConfig.bSPILFunction`) | `UnitName, AlarmCode, OccurDateTime, Recovery, StopedTime, Duplicate, Message, ErrPart` | `LogObjects.cpp:68-70` |
| `MyDBIProcess` (3-arg) | `AddTextWithDateTime(CommaText[table,"\t","\t","\t","\t",S1,S2])` -> `YYYY-MM-DD, HH:NN:SS.mmm, table,"\t","\t","\t","\t",S1,S2` = 9 fields | golden body `cMyDB.cpp:1003-1069` (`#if 0`; live copy SECSGEM/uHGemEquipment.cpp:3475) |
| `MyDBIProcessNew` | `[table, AlarmCode, "\t","\t","\t", S1, S2]` = 9 fields (MES/WAR codes under Unit "Process"/"Motion") | `cMyDB.cpp:930-993` (`#if 0`; stand-in canary_support.cpp:453) |
| TotalLoader / TimeData / UPH / Production | 9 fields, Unit = TimeDataTotalLoader / TimeData / UPH / Production | `cMyDB.cpp:330, 378, 430, 658` (active in V906) |
| **alarm row with stop time** | `[YYYY-MM-DD, HH:NN:SS.mmm, sJamArea, sJamCode, Recovery, PassTime, iDuplicateError, Message, MotorAlarmNo, GetLastOpenFN()]` via CommaText, inserted at the alarm's line with `MyInsertToFile` = 10 fields | golden V912 `note.cpp:1122-1173` |
| SPIL rows | `AddTextWithLineNo(CommaText[Unit, AlarmCode, "YYYY-MM-DD HH:NN:SS", Recovery, StopedTime, Duplicate, Message, ErrPart])` | same functions, SPIL branch |
| file name | `<MachineType>_<SocketHandlerID>_EventLogTxt` (N10 daily), `EventLogTxt_<SocketHandlerID>` (O15), else `EventLogTxt`; SaveType Day / 12 Hour (N10), Hour..12 Hour / Month (O15) | golden `cprod.cpp` ~2521-2566; V906 same block `#if 0 TODO(GA1-B2)` |
| name patterns | day `<n>_yyyymmdd.csv` (+`_RT`,`_FT`,`_TesterOffline`), hour `<n>_yyyymmdd HH.csv`, 12 h `<n>_yyyymmddHH00.csv`, month `<n>_yyyymm.csv`; copy `AllEventLog\<n>_yyyymmdd.csv`; `ByLotID\...\_ByLotEventLog.csv` | V906 `Public/MyStringList.cpp:731-890, 586-592, 378-380` |

Mismatch flags

| # | issue | effect |
|---|---|---|
| M1 | **SPIL files are not analysed at all**: field 0 is UnitName, `IsValidDateString` fails, every row skipped. The file has no sequence column (the "No." column is only added by the cObserver viewer grid; skill §4 "第一欄為序號" is inaccurate). | SPIL machines get empty summaries from the golden analyzer |
| M2 | **Tokenizer**: BCB6 CommaText also splits on unquoted blanks; V906 `vclcompat` CommaText splits on commas only (`vclcompat/TStringList.cpp:188-226`); V912 cObserver uses a quote-aware comma-only `ParseEventLogLine` (V906 `cObserver.cpp:1351-1393`), the Rev891 analyzer does not. Real files contain unquoted `,16 System,` rows (local sample: 2025-09 154, 2025-11 24, 2025-12 10 rows with an unquoted numbered UnitName) that BCB splits into two or more fields, shifting AlarmCode to "System" -> not counted. | ElaCore on vclcompat CommaText will differ from the BCB oracle on those rows (decision Q1) |
| M3 | vclcompat `parseDelimited` stops (`break`) when a closing quote is not immediately followed by ','; BCB continues with the next token | rare field loss in V906 only |
| M4 | quoted TAB placeholder `"\t"`: BCB keeps TAB; V912 viewer splitter Trim()s to "" | display only |
| M5 | file match needs `yyyymmdd.csv`: hour / 12 h / month / `_RT` / `_FT` / `_TesterOffline` files are silently skipped (VTEST 915/919 accepts hour names, never month names) | silent zero counts when O15/N10 change the save period |
| M6 | `AllEventLog\<n>_yyyymmdd.csv` copy is read in addition to `yyyy\mm\<n>_yyyymmdd.csv` (sample 2025-08-25) | double counting |
| M7 | V906 writer gaps: no alarm row with StopedTime/Recovery/Recipe (V906 `ShowErrorMessage` is the sim stand-in `canary_support.cpp:92`), and file name / SaveType configuration gated | V906-written logs give zero JAM/WAR stop time; P1 fixtures must be BCB-written files; the writer gap belongs to the cMyDB / note work |
| M8 | encoding: BCB rows cp950, V906 `TStringList::SaveToFile` writes the UTF-8 bytes of V906 literals (`vclcompat/TStringList.cpp:262`) | reader must detect encoding for Message display; counts unaffected |
| M9 | `MyDBIProcessNew` rows (Process/Motion + MES/WAR code, StopedTime "\t") count as stop events with 0 s; MES from Process counts as MES; WAR from Motion is excluded from WAR count only; lookups create `[Process]`/`[Motion]` sections in JAM0000.dat (sample: ~4 500 Motion rows with WAR codes, mostly WAR2207/2208 "Do process motor home") | golden behaviour, keep and document |
| M10 | date written `YYYY-MM-DD`, analyzer accepts `/` and `-` | ok |


---

## 6. Suggested split (consistent with plan §3)

Analyzer.cpp alone by target: ElaCore 20 funcs / 1,342 lines (11 of them "mixed ui", 1,259 lines, of which the widget writes are dropped), ElaProduction 1 / 151, ElaReports 11 / 239, ElaHub 8 / 310, web 24 / 563, drop 24 / 457. All-unit totals by target are in §3.

| V906 module | takes (BCB functions) | notes |
|---|---|---|
| `EventLogAnalysis/ElaCore.h/.cpp` | types `_MySummaryStruct`, `_MyJamSummary`, `LogRecord`, `eEventLog`, the unit names (31 `cbJamArea` items, 30 with a filter box) and 7 function buckets; file discovery (`LoadFavorite`, event-log half of `ImportProductionLod`, FN1/FN2 + CC915/919 rule); row parse (one `ParseEventLogLine` shared with cObserver, `IsValidDateString`, `IsValidTimeString`, `ParseDate`, `ParseTime`, `ParseDateTime`); aggregation (`GetEventLogText` without :1039-1162, both `GetEventLogTextToVec`, bucket part of `SetStringGrid`); KPI (`UpdateSgProduction` formulas, `SetTotalSummary` numbers, `GetUPH`, `GetMUBA`, `GetMUBF`, `GetMTBA`, `GetMTBF`, `GetStartEndGap`, `ConvertSecToTime`, `ConvertDTToTime`); Top / drill-down / filter (`UpdateSgTop5Filter`, `UpdateSgTop5`, `UpdateSgByFilter` + the group mappings from the check-box handlers :2897-2965); Jam rules (`GetJamLevel`, `GetJemIncludeMTBA`, `GetJemIncludeMTBF` with a JAM0000.dat cache); `RunQuery(start,end)` = `btnQueryClick` order | range is a parameter (no pickers); no TStringGrid; ~1,650 BCB lines in |
| `ElaProduction.cpp` | `ListProductionLog` (input / contact / test time by day and hour), production half of `ImportProductionLod`, `uAnalysisProductionLog` (device count used by Jam_Summary), `uAnalysisEventLogText` (default name/folder), `getColumn` | `uAnalysisProductData` is a stub -> drop |
| `ElaReports.cpp` | `SetTotalSummary` text layout, `SaveSummary`, `btnSaveSummaryClick` (export API), `O06SaveSummaryData`, `N10SaveSummaryData`, `DoVTestSaveSummary`, all of `uChipAdvancedFunc`, `uChipMosZHUBEI_Func::SaveJamCodeFor7Days / SaveJamSummaryFor30Days x2 / SaveEventLog`, `IsValidFileName`, `FixFolderPath`, `WriteDataToFile`, `MyForceDirectories`; XLS (`SGDToXLS`, XLSfile.pas, the four `btnSave*Click`) -> P7 or CSV | report bytes must match BCB (except time stamps) -> keep `ConvertSecToTime` formats, LF-only `WriteDataToFile`, append semantics |
| `ElaHub.h/.cpp` | `OnMyCopyMsg` switch -> `ElaHub::Post(cmd)`; `TAnalysisProcessThread` + `AnalysisThreadProcess` (priority + coalescing) -> ElaThread inbox; `Timer1Timer` (start-up query + UPDATE_PARAMETER) and `Timer2Timer` (O06 / N10 schedule); `ReadConfig` -> typed settings from V906 `IniConfig` (already loaded) instead of widgets; `UploadJamCode`, `UploadSummaryCount`, `UploadEventLog`, `UploadByFile_N10`, `N10_UploadDataToFTP`, `N25_UploadJamDataToFTP`, `N25_UploadSummaryCountToFTP` on top of `KYECFTP`; analyzer `MyDBIProcess` -> ela log (FTP_Log through V906 `Public/MyStringList`) | `SendCommand_EventLog` body -> `ElaHub::Post`; `FindWindow`, `M_V`, `WM_EventAnalysis` retire |
| `ElaApi.cpp` + `web-overlay/eventlog.html` | range pickers, folder import, filter groups (section / unit / function), Top list type + row drill-down, grids (headers from `InitStringGrid`), summary memo, disk space, optional Jam Code Setting editor (`ChangeJamMessage`, `SaveJamLevel`, `cbJam*Change`, `GetJameCodeOfAxis`) | OEE charts are new work (nothing to port) |
| drop | OEE canvas functions (11), empty handlers, `CompareSummary`, `CompareJamSummary`, `AnalysisEventLogNormal`, `ProcessHMountConnect`, `VerInfo`, `HTMD5`, unused `FileInfo` / `TfFTP` / `MyStringList` parts, `uAnalysisProductData`, `WinMain`, `FormCreate/FormClose/AppException` | |

---

## 7. Golden behaviour that matters for the P1 oracle and the port

| # | behaviour | ref | port advice |
|---|---|---|---|
| G1 | `_MySummaryStruct::InitData` does not reset `iFailCount` -> `MySummary.iFailCount` (Fail Count, total MTBF, ChipAdvanced "OEE") accumulates over every query of one exe session, **including the automatic start-up query** | Analyzer.cpp:111-126 | oracle: run the BCB exe with a log root whose "today" is empty, one query per start; port: reset (record as deviation) |
| G2 | `ListByUnitName[]` / `ListByFunction[]` are never cleared -> "By Filter" rows accumulate over queries | :994-1034, :1995-2059 | same |
| G3 | O06-4 without the time-period option saves the whole-day summary **every minute** (`SystemNN % 1 == 0`) | :3014-3040 | decide keep / fix |
| G4 | "Top 5" lists every code (sorted count desc, code asc) | :2061-2203 | keep; web can cut |
| G5 | UPH(day)=input/24, UPH(hour)=input, total UPH=input/(days*24); MTBF/MTBA use the calendar period (<= 1 event -> whole period; input 0 -> 0) | :1787-1947, Common.cpp:518-576 | keep |
| G6 | range inclusive for EventLog, exclusive for Production_Log; first Production_Log header counted then `iInputCount--` | :850, :2385, :2360-2367, :2456 | keep |
| G7 | JAM/WAR/MES test differs: stop filter = contains, JAM/WAR/MES counters = prefix, ChipMos Jam Rate = contains | :857-985, uChipMosZHUBEI_Func.cpp:42 | keep |
| G8 | ChangeLog/ParameterLog block never matches and has an out-of-bounds loop | :1039-1162 | drop |
| G9 | crashes on empty data: `FindStartLotFromRecVec` (`size_t i=size-1`, :216), `SaveJamSummaryFor30Days` (`tIntervalRecs[size-1]`) | uChipAdvancedFunc.cpp:216, uChipMosZHUBEI_Func.cpp:95 | guard (deviation) |
| G10 | analyzer writes Handler files: default keys into JAM0000.dat for every new (unit, code) and into config.ini on every ReadConfig; the Jam editor rewrites JAM0000.dat and Error\<lang>\<code>.dat | Common.cpp:72-183, :2756-2797 | decision Q3 |
| G11 | function filter check boxes are wired to the wrong buckets (Temperature box shows ESD rows, 2D shows Temp, OCR shows 2DID, ESD shows OCR) | :257-263 | decide keep / fix |
| G12 | ChipAdvanced file name `%d%d%d` (no zero pad), report appended; anomaly loop skips record 0 | uChipAdvancedFunc.cpp:238, :54 | keep for byte equality |
| G13 | Jam_Summary device count reads `Production_Log\yyyymm\<gethostname()>_yyyymmdd.csv`; local sample prefix is the handler ID (e.g. `PJLD1014_`), so the count is 0 unless the Windows host name equals it; day loop can skip the last day | uChipMosZHUBEI_Func.cpp:112, uAnalysisProductionLog.cpp:62-79 | keep, flag to user |
| G14 | `UploadByFile_N10` never truncates UploadFile.csv (re-uploads every row every time) | uChipMosZHUBEI_Func.cpp:401 | moot while no sender |
| G15 | TfFTP `ChangeDirectories` creates `sCurr` instead of the next folder | TfFTP.cpp:120 | KYECFTP replaces it |
| G16 | modal "Drive D < 10 GB" MessageBox on the main thread stops all EL_* processing until clicked | :3188-3220 | web banner |

### 7.1 Corrections to the plan text (ela-web-conversion-plan.md)

1. §1.1 "OEE 三組 TeeChart": there is no TeeChart; three `TImage` canvases with `random()` demo data, tabs hidden, no OEE calculation. OEE on the web is new work.
2. §1.1 table: `EL_UPDATE_PARAMETER` only re-reads settings (no re-summary); `EL_UPLOAD_BYFILE_N10` has no live sender in golden V912 (commented out at `HS_Function.cpp:436-440`).
3. §1.1 "Timer1（掃目錄、觸發 EL_UPDATE_PARAMETER）": Timer1 is a one-shot start-up init; the directory scan runs on every query (`SetStringGrid` -> `ImportProductionLod(2)`); Timer2 (50 s) runs the O06/N10 summary saves (files only, no FTP).
4. §1.1 "uAnalysisProductData 生產資料": stub, body commented out.
5. Skill §4 "SPIL 格式…第一欄為序號": the SPIL file starts with UnitName and has OccurDateTime in field 2; the analyzer does not read SPIL files.
6. §2 "執行緒": golden already has a "thread", but it only marshals to the main thread; ElaThread must really own the work and serialise it (golden allows re-entry through `ProcessMessages`).
7. Line count: Analyzer.cpp is 3,263 lines (`wc -l`).

---

## 8. Questions that need a human decision

1. **Parser (M2)**: ElaCore follows BCB6 CommaText exactly (blank-splitting; loses the unquoted `16 System` rows, matches the BCB oracle) or uses the V912 quote-aware comma-only `ParseEventLogLine` already used by cObserver (one parser for both pages; deliberate deviation from Rev891)?
2. **SPIL (M1)**: keep golden "not analysed", or add a SPIL field mapping (new behaviour)?
3. **Write-backs (G10)**: may the V906 analyzer write defaults into JAM0000.dat / config.ini like golden, or must ElaCore be read-only on Handler files?
4. **Known golden bugs (G1, G2, G3, G11)**: keep for oracle equality or fix and list as deviations? (Suggested: fix G1/G2, oracle taken from a fresh exe per query.)
5. **File selection (M5, M6)**: faithful (skip hour/month files, double-count AllEventLog) or de-duplicate and accept the other SaveType names?
6. **Jam Code Setting editor**: port to eventlog.html, or leave Jam level / IncludeMTBA / IncludeMTBF editing to the Handler's own screens?
7. **Scheduling**: keep O06/N10 (analyzer Timer2) and the N25/VTEST triggers (Handler `Timer2Timer`, `HS_Function`) as they are, with ElaHub only receiving `Post(cmd)`, or move all schedules into ElaHub?
8. **Customer reports** (plan §5-4): senders exist for CC_CYUEAN 868 (ChipAdvanced), N25 flags (ChipMos Zhubei), CC_VTEST 915/919 (MTBF). Which ones ship?
9. **Oracle data (M7)**: V906 does not write alarm rows yet; which BCB-written machine/days (the local sample mixes many machines) are the P1 fixture, and is the 20.25.891.0 exe from `D:\HT9045_Updater_NSIS\EventlogAnalyzer` the agreed oracle binary?
10. Optional: someone with the password opens `EventlogAnalyzer_Rev891.0_20251125.7z` to confirm it equals SVN r891.


## 9. Function ledger

Columns: lines = start-end (count). cat = core/report/ui/comm/util. target = suggested V906 home (web = eventlog.html, drop = not ported). deps = VCL / platform dependencies. io = files read/written.


### 9.1 Analyzer.cpp (88 functions)

| # | function | lines | cat | target | purpose | deps | io | notes |
|---:|---|---|---|---|---|---|---|---|
| 1 | `_MySummaryStruct::InitData` | 111-126 (16) | core | ElaCore | Reset per-period summary counters (_MySummaryStruct). | - | - | BUG: iFailCount is NOT reset -> MySummary.iFailCount accumulates across queries until restart |
| 2 | `_MyJamSummary::InitData` | 128-135 (8) | core | ElaCore | Reset per-code aggregate (_MyJamSummary). | - | - |  |
| 3 | `CompareSummary` | 137-140 (4) | core | drop | Sort predicate by iAlarmCount desc. | - | - | unused |
| 4 | `CompareJamSummary` | 143-146 (4) | core | drop | Sort predicate by iCount desc. | - | - | unused (struct cmp at :31-39 is the one used) |
| 5 | `TAnalysisProcessThread::TAnalysisProcessThread` | 148-152 (5) | comm | ElaHub | TAnalysisProcessThread ctor; ClearAllActive(). | TThread | - |  |
| 6 | `TAnalysisProcessThread::AnalysisThreadProcess` | 154-196 (43) | comm | ElaHub | EL_* dispatcher: tests 7 bEL_* flags in a fixed else-if chain, runs ONE handler per tick. | TThread/Synchronize | - | order UPDATE_PARAMETER>JAMWEEK>SUMMARY>CHIPADV_LOTEND>EVENTLOG>BYFILE_N10>VTEST_MTBF_SUM |
| 7 | `TAnalysisProcessThread::ClearAllActive` | 198-206 (9) | comm | ElaHub | Clear command flags. | - | - | BUG: misses bEL_UPLOAD_BYFILE_N10 |
| 8 | `TAnalysisProcessThread::Execute` | 208-215 (8) | comm | ElaHub | Thread loop: Synchronize(AnalysisThreadProcess); SleepEx(500). | TThread::Synchronize | - | all analysis/FTP actually runs on the VCL main thread |
| 9 | `TfrmELA::TfrmELA` | 217-301 (85) | ui | web/drop | Form ctor: checkbox->unit/function arrays, mapUnitName from cbJamArea captions, allocate lists, slFTPLog, JAM0000.dat path, create suspended thread. | TCheckBox[],TStringList,TMyStringList,TMemo | creates FTP_Log under D:\HT9045_Log\UploadFile (by day); FileNameJam000=D:\HT9045\Error\English\JAM0000.dat | cbByFuncFilter wiring swapped (efTemp=cb2DID, efESD=cbTemperature, ef2DID=cbOCR, efOCR=cbESD); mapUnitName key list is core data; cbJamArea has 31 items vs eUnitNameTotal 30 ('31 Cylinder' unmapped) |
| 10 | `TfrmELA::AppException` | 303-307 (5) | ui | drop | Application->OnException -> mmoException. | TMemo | - |  |
| 11 | `TfrmELA::FormClose` | 309-347 (39) | ui | drop | Free lists, terminate thread. | TStringList | - | UB: delete[] on member arrays ListByUnitName/ListByFunction |
| 12 | `TfrmELA::FormCreate` | 349-354 (6) | ui | drop | Init flags, hook OnException. | TApplication | - |  |
| 13 | `TfrmELA::FormShow` | 356-441 (86) | ui | web/drop | Startup: status-bar version, default range today 00:00..now, area checks, draws 3 OEE TImage demos with random() then hides tsOee/tsOeeByDay/tsOeeOneDay/tsProdLog, Machine ID, hostname, DiskCheck, Resume thread. | TStatusBar,TDateTimePicker,TImage,TTabSheet,Winsock | reads D:\HT9045\system\Gerneral.ini [Version] Machine ID | OEE = demo data only |
| 14 | `TfrmELA::Timer1Timer` | 443-485 (43) | comm | ElaHub | Timer1 one-shot init (about 1 s after show): set dir lists, ImportProductionLod(2), btnQuery->Click(), Jam editor init, rgFilter, ReadConfig, raise bEL_UPDATE_PARAMETER, enable Timer2. | TTimer,TDirectoryListBox,TComboBox | scans D:\HT9045_Log\EventLogTxt and D:\HT9045_Log\Production_Log | first full query runs at startup |
| 15 | `TfrmELA::OnMyCopyMsg` | 487-558 (72) | comm | ElaHub | WM_COPYDATA handler: wParam==WM_EventAnalysis(6) && M_V.bModeType==0x04 -> switch(bCommand) sets bEL_* flag. | VCL_MESSAGE_HANDLER,COPYDATASTRUCT | - | bCommandType(0x09) and bData[0] ignored; `&pVM->bModeType=` hack aliases pVM onto lpData |
| 16 | `TfrmELA::ProcessHMountConnect` | 560-577 (18) | comm | drop | FindWindow("TfMain", HT9045/HT9046/...) -> HMountWnd/bFind. | Win32 FindWindow | - | result never used (dead) |
| 17 | `TfrmELA::InitStringGrid` | 579-644 (66) | ui | web | Grid headers/column widths (sgTop5Filter, sgTop5, sgTop5Alarm, sgByArea, sgAlarm, sgFail, sgByDay, sgByHour). | TStringGrid | - | column names = JSON field names |
| 18 | `TfrmELA::SetStringGrid` | 646-780 (135) | core | ElaCore | Query prep: read range from pickers, clear grids, build by-day (YYYY/MM/DD) and by-hour (YYYY/MM/DD HH:00:00) buckets, reset maps/MySummary, ImportProductionLod(2) re-scan, progress max. | TDateTimePicker,TStringGrid,TProgressBar | re-scans both log trees each query | mixed ui; bucket keys are grid text |
| 19 | `TfrmELA::GetEventLogText` | 782-1181 (400) | core | ElaCore | Multi-day EventLogTxt aggregation (stop/alarm/fail counts+time by total/day/hour, JAM/WAR/MES counts, per-code maps, by-unit and by-function row lists); then refreshes Top/Fail/Alarm/Filter grids. | TStringList.CommaText,TListBox(lstEventLog),TProgressBar,TStatusBar,20xTLabel,TComboBox | reads EventLogTxt *.csv; JAM0000.dat per stop row (write-back) | mixed ui; :1039-1162 ChangeLog/ParameterLog label block is ui, effectively dead, and has an OOB loop (j<sizeof(strPara)=84 for 21 items) |
| 20 | `TfrmELA::GetEventLogTextToVec` | 1183-1187 (5) | core | ElaCore | GetEventLogTextToVec(): range from pickers -> overload. | TDateTimePicker | - |  |
| 21 | `TfrmELA::GetEventLogTextToVec` | 1189-1267 (79) | core | ElaCore | Same file/row filters as GetEventLogText, emits std::vector<LogRecord> (all in-range non-duplicate rows). | TStringList.CommaText,TListBox,TProgressBar,TStatusBar | reads EventLogTxt *.csv | no CC915/919 name variant; Recipe not copied; clears slStopList.. (side effect) |
| 22 | `TfrmELA::AnalysisEventLogNormal` | 1269-1272 (4) | core | drop | Empty stub. | - | - | dead |
| 23 | `TfrmELA::dtpStartTimeChange` | 1274-1277 (4) | ui | web | Start time picker -> start date time. | TDateTimePicker | - |  |
| 24 | `TfrmELA::SetOeeAnaBG` | 1279-1308 (30) | ui | drop | OEE-by-day TImage legend/background. | TImage.Canvas | - | OEE tabs hidden; not TeeChart |
| 25 | `TfrmELA::SetOeeAnaDay` | 1310-1411 (102) | ui | drop | Draw one day's Running/Alarm/Pause bar. | TImage.Canvas | - | demo data |
| 26 | `TfrmELA::SetOeeAnaTitle` | 1413-1424 (12) | ui | drop | OEE title. | TImage.Canvas | - |  |
| 27 | `TfrmELA::SetOeeChartBG` | 1426-1450 (25) | ui | drop | OEE chart legend. | TImage.Canvas | - |  |
| 28 | `TfrmELA::SetOeeChartDay` | 1452-1513 (62) | ui | drop | Draw 1440-minute status strip from iStatus[0][]. | TImage.Canvas | - | demo data (random) |
| 29 | `TfrmELA::SetOeeChartTitle` | 1515-1526 (12) | ui | drop | OEE chart title. | TImage.Canvas | - |  |
| 30 | `TfrmELA::SetOeeChartHourLine` | 1528-1549 (22) | ui | drop | OEE chart hour grid. | TImage.Canvas | - |  |
| 31 | `TfrmELA::SetDayOeeTitle` | 1551-1562 (12) | ui | drop | Day-OEE title. | TImage.Canvas | - |  |
| 32 | `TfrmELA::SetDayOeeBG` | 1564-1593 (30) | ui | drop | Day-OEE background/legend. | TImage.Canvas | - |  |
| 33 | `TfrmELA::SetDayOeeDay` | 1595-1598 (4) | ui | drop | Empty. | - | - | dead |
| 34 | `TfrmELA::SetDayOeeHourLine` | 1600-1632 (33) | ui | drop | Day-OEE hour grid. | TImage.Canvas | - |  |
| 35 | `TfrmELA::dirlstEventLogChange` | 1634-1637 (4) | ui | drop | Show event-log dir. | TDirectoryListBox,TLabel | - |  |
| 36 | `TfrmELA::dirlstProdLogChange` | 1639-1642 (4) | ui | drop | Show production-log dir. | TDirectoryListBox,TLabel | - |  |
| 37 | `TfrmELA::LoadFavorite` | 1644-1678 (35) | util | ElaCore | Recursive FindFirstFile scan collecting every *.csv (skips hidden, . and ..). | Win32 FindFirstFile, Application->ProcessMessages | walks whole EventLogTxt / Production_Log tree | ProcessMessages = re-entrancy (WM_COPYDATA/Timer2 can run mid-query) |
| 38 | `TfrmELA::ImportProductionLod` | 1680-1745 (66) | core | ElaCore | Build input file sets: lstEventLog <- all *.csv under labPath; lstProdFile <- all *.csv under lblProdPath (iMethod 0/1/2). | TListBox,TLabel,TDirectoryListBox | D:\HT9045_Log\EventLogTxt, D:\HT9045_Log\Production_Log | mixed ui; includes AllEventLog/ByLotID/SGJamCount subtrees |
| 39 | `TfrmELA::btnImportClick` | 1747-1756 (10) | ui | web | Pick event-log folder -> ImportProductionLod(0). | SelectDirectory,TPageControl | - |  |
| 40 | `TfrmELA::btnImportProdClick` | 1758-1767 (10) | ui | web | Pick production-log folder -> ImportProductionLod(1). | SelectDirectory | - |  |
| 41 | `TfrmELA::dtpEndTimeChange` | 1769-1772 (4) | ui | web | End time picker -> end date time. | TDateTimePicker | - |  |
| 42 | `TfrmELA::sgTop5FilterMouseDown` | 1774-1785 (12) | ui | web | Top list row click -> UpdateSgTop5(row). | TStringGrid.MouseToCell | - |  |
| 43 | `TfrmELA::UpdateSgProduction` | 1787-1947 (161) | core | ElaCore | Per-day / per-hour KPI rows: UPH, contact count, total/avg test time, stop/alarm/fail time, MUBA, MTBA, MTBF. | TStringGrid(sgByDay,sgByHour) | - | mixed ui; MTBF(day)=24h/failCount (<=1 fail -> 24h; input==0 -> 0), UPH(day)=input/24, UPH(hour)=input |
| 44 | `TfrmELA::UpdateSgFailAndAlarm` | 1949-1993 (45) | ui | web | Copy slAlarmList/slFailList rows to sgAlarm/sgFail. | TStringGrid | - |  |
| 45 | `TfrmELA::UpdateSgByFilter` | 1995-2059 (65) | core | ElaCore | Filter view: rgFilter 0/1 -> ListByUnitName[] of checked units; 2 -> ListByFunction[] of checked functions -> sgByArea + count. | TStringGrid,TCheckBox,TRadioGroup,TLabel | - | mixed ui; ListByUnitName/ListByFunction never cleared between queries (BUG: rows accumulate) |
| 46 | `TfrmELA::UpdateSgTop5Filter` | 2061-2203 (143) | core | ElaCore | Top list: JAM / WAR / JAM+WAR maps -> vector sorted by count desc, code asc; ALL codes listed (not 5); raw rows to sgTop5Alarm. | TStringGrid,TComboBox(cbTopAlarmFilter) | - | mixed ui |
| 47 | `TfrmELA::UpdateSgTop5` | 2205-2236 (32) | core | ElaCore | Drill-down: rows of sgTop5Alarm whose AlarmCode starts with the selected code -> sgTop5 (Date,Time,Unit,Message). | TStringGrid | - | mixed ui; reads the grid as data store |
| 48 | `TfrmELA::sgTop5FilterDrawCell` | 2238-2243 (6) | ui | drop | Empty draw handler. | TStringGrid | - | dead |
| 49 | `TfrmELA::btnSaveAlarmClick` | 2245-2252 (8) | report | ElaReports(P7) | Save sgAlarm as XLS. | TSaveDialog,SGDToXLS | default C:\AlarmList.xls |  |
| 50 | `TfrmELA::btnSaveFailClick` | 2254-2261 (8) | report | ElaReports(P7) | Save sgFail as XLS. | TSaveDialog,SGDToXLS | default C:\FailureList.xls |  |
| 51 | `TfrmELA::btnTop5AlarmClick` | 2263-2278 (16) | report | ElaReports(P7) | Save Top list + raw rows + drill-down as 3 XLS. | TSaveDialog,SGDToXLS | Top5Alarm.xls, AlarmList.xls, AlarmList1.xls |  |
| 52 | `TfrmELA::btnSaveByFilterClick` | 2280-2297 (18) | report | ElaReports(P7) | Save filter view as XLS. | TSaveDialog,SGDToXLS | AlarmListByArea/ByYield/ByFunction.xls |  |
| 53 | `TfrmELA::sgAlarmMouseDown` | 2299-2306 (8) | ui | drop | Empty (commented). | - | - | dead |
| 54 | `TfrmELA::sgAlarmDrawCell` | 2308-2314 (7) | ui | drop | Empty (commented). | - | - | dead |
| 55 | `TfrmELA::ListProductionLog` | 2316-2466 (151) | core | ElaProduction | Production_Log aggregation: per in-range row input count by day/hour; on a new OrderTest (touch-down) add contact count + test time; then UpdateSgProduction + SetTotalSummary. | TStringList.CommaText,TListBox(lstProdFile),TComboBox,TProgressBar,TStatusBar,TMemo | reads Production_Log *.csv | mixed ui; header row of first file counted then -1; range exclusive (> <) unlike EventLog (>= <=); slAllData filled but never used |
| 56 | `TfrmELA::SetTotalSummary` | 2468-2555 (88) | core | ElaCore | Totals block text (Handler ID, input, period, MUBA, MTBA, MTBF, test time, UPH, MES/WAR/JAM, alarm, fail, stop, running time) -> mmoSummary/mmoSummary1. | TMemo | - | mixed ui+report: this text IS the SaveSummary body |
| 57 | `TfrmELA::btnSaveSummaryClick` | 2557-2570 (14) | report | ElaReports | Manual Save Summary via dialog. | TSaveDialog | default D:\RMS\yyyy-mm-dd |  |
| 58 | `IsValidFileName` | 2572-2601 (30) | util | ElaReports | Validate file name; ForceDirectories on missing dir. | - | creates folders |  |
| 59 | `FixFolderPath` | 2603-2612 (10) | util | ElaReports | Ensure trailing backslash. | - | - |  |
| 60 | `TfrmELA::SaveSummary` | 2614-2688 (75) | report | ElaReports | Write <Path><HandlerID>-SummaryData_<yyyy-mm-dd\|Name>.txt = mmoSummary1 (+ sgByHour rows + Top/raw grids when bDetail && chkIncludeAlarmList). | TMemo,TStringGrid,TCheckBox | writes summary .txt | appends to mmoSummary1 in place (a second call without re-query duplicates the detail) |
| 61 | `TfrmELA::cbJamAreaChange` | 2690-2698 (9) | ui | web | Jam area change -> reload code list. | TComboBox | reads D:\HT9045\Error\AlarmCodeList.txt |  |
| 62 | `TfrmELA::cbJamCodeChange` | 2700-2705 (6) | ui | web | Jam code change. | TComboBox | - |  |
| 63 | `TfrmELA::cbJamLangChange` | 2707-2710 (4) | ui | web | Jam language change. | TComboBox | - |  |
| 64 | `TfrmELA::ChangeJamMessage` | 2712-2754 (43) | ui | web | Jam Code Setting editor: save previous, load <code>.dat text, show level/IncludeMTBA/IncludeMTBF. | TRichEdit,TRadioGroup,TCheckBox | reads D:\HT9045\Error\{English,Chinese,Korea,Singapore}\<code>.dat |  |
| 65 | `TfrmELA::SaveJamLevel` | 2756-2797 (42) | ui | web | Persist Jam level/IncludeMTBA/IncludeMTBF and edited text. | TRichEdit,TRadioGroup,TCheckBox | writes JAM0000.dat and <code>.dat | writes Handler-owned files |
| 66 | `TfrmELA::GetJamLevel` | 2799-2810 (12) | core | ElaCore | Jam level from JAM0000.dat [area] <code> (default 0). | TIniFile | reads/writes JAM0000.dat | write-back default |
| 67 | `TfrmELA::GetJemIncludeMTBA` | 2812-2836 (25) | core | ElaCore | IncludeMTBA rule: [area] '<code> IncludeMTBA'; default true for JAM codes in areas 01..05, else false. | TIniFile | reads/writes JAM0000.dat | called per stop row; writes default -> creates sections |
| 68 | `TfrmELA::GetJemIncludeMTBF` | 2838-2882 (45) | core | ElaCore | IncludeMTBF rule: [area] '<code> IncludeMTBF'; default true for area '24 Motor' + 24 hard-coded codes, else false. | TIniFile | reads/writes JAM0000.dat | called per stop row; write-back |
| 69 | `TfrmELA::btnQueryClick` | 2884-2895 (12) | core | ElaCore | Query pipeline entry: SetStringGrid -> GetEventLogText -> ListProductionLog (every report path calls btnQuery->Click()). | TStringGrid,TProgressBar,TStatusBar | - | mixed ui |
| 70 | `TfrmELA::chkInputAreaMouseUp` | 2897-2903 (7) | ui | web | 'Input Area' group = Input arm + Input shuttle. | TCheckBox | - | group mapping is filter semantics |
| 71 | `TfrmELA::cbInputArmMouseUp` | 2905-2914 (10) | ui | web | Any unit box -> clear group boxes, refilter. | TCheckBox | - |  |
| 72 | `TfrmELA::chkOutPutAreaMouseUp` | 2916-2922 (7) | ui | web | 'Output Area' = Output arm + Output shuttle. | TCheckBox | - |  |
| 73 | `TfrmELA::chkIndexAreaMouseUp` | 2924-2930 (7) | ui | web | 'Index Area' = Index unit + Tester I/F. | TCheckBox | - |  |
| 74 | `TfrmELA::cbLoadUnloadMouseUp` | 2932-2951 (20) | ui | web | 'Loader/Unloader' = 15 tray units. | TCheckBox | - |  |
| 75 | `TfrmELA::chkSelectAllMouseUp` | 2953-2965 (13) | ui | web | Select all units. | TCheckBox | - |  |
| 76 | `TfrmELA::rgFilterClick` | 2967-2992 (26) | ui | web | Filter mode 0 section / 1 unit / 2 function; show group box. | TRadioGroup,TGroupBox,TPanel | - |  |
| 77 | `TfrmELA::cbRTCMouseUp` | 2994-2998 (5) | ui | web | Function box click -> refilter. | TCheckBox | - |  |
| 78 | `TfrmELA::cbTopAlarmFilterChange` | 3000-3008 (9) | ui | web | Top list type change -> rebuild. | TComboBox,TStringGrid | - |  |
| 79 | `TfrmELA::Timer2Timer` | 3010-3088 (79) | comm | ElaHub | Timer2 (50 s): minute gate; O06-4 production (every 1/10/30 min) / N10-3 (00:00 \| 08:00+20:00 \| hourly) -> range today 00:00..23:59:59 -> O06SaveSummaryData + N10SaveSummaryData; ProcessHMountConnect. | TTimer,TDateTimePicker,TCheckBox,TRadioGroup | - | iCheckInterval=1 means EVERY minute although the comment says hourly |
| 80 | `TfrmELA::ReadConfig` | 3090-3140 (51) | comm | ElaHub | EL_UPDATE_PARAMETER target: config.ini [Event Log]/[FTPUpLoad]/[ChipMos Function]/[N34 Function] + Gerneral.ini -> widgets (widgets are the config store). | TCheckBox,TEdit,TComboBox,TRadioGroup | reads/writes D:\HT9045\config\config.ini, reads Gerneral.ini | write-back of missing defaults into the Handler's config.ini; sCustCode feeds the core file-name rule |
| 81 | `TfrmELA::O06SaveSummaryData` | 3142-3152 (11) | report | ElaReports | O06 auto summary: if O06-4 -> btnQuery -> SaveSummary(AutoSaveProductionPath). | TCheckBox,TEdit,TButton.Click | writes <AutoSaveProductionPath>\<ID>-SummaryData_<date>.txt |  |
| 82 | `TfrmELA::N10SaveSummaryData` | 3154-3180 (27) | report | ElaReports | N10 summary: method 1 net drive -> SaveSummary(sN10UploadDrivePath); else SaveSummary(D:\HT9045_Log\EventLogSummary\yyyymm\). | TRadioGroup,TCheckBox,TEdit | writes summary .txt | no FTP here despite the 'FTP' branch |
| 83 | `TfrmELA::btEventLogClick` | 3182-3185 (4) | ui | drop | Empty. | - | - | dead |
| 84 | `TfrmELA::DiskCheck` | 3188-3220 (33) | ui | web | D: disk space labels; modal MessageBox when < 10 GB. | TLabel,MessageBox | - | modal box blocks the UI thread (and thus EL_* processing) |
| 85 | `TfrmELA::GetDateTime` | 3222-3226 (5) | util | ElaCore | DateOf(date)+TimeOf(time). | - | - |  |
| 86 | `TfrmELA::GetSysDateTimeRange` | 3228-3233 (6) | core | ElaCore | Range from pickers; days = trunc(end)-trunc(start)+1. | TDateTimePicker | - | mixed ui |
| 87 | `TfrmELA::GetTotalDays` | 3235-3238 (4) | core | ElaCore | Day count. | - | - |  |
| 88 | `TfrmELA::DoVTestSaveSummary` | 3240-3261 (22) | report | ElaReports | EL_VTEST_MTBF_SUM: range now-7 00:00..now-1 23:59:59, btnQuery, SaveSummary(asO19_SavePath,'',bDetail=false). | TDateTimePicker,TButton.Click | writes D:\MTBF_Summary\<ID>-SummaryData_<today>.txt (default) |  |

### 9.2 Common.cpp (49 functions)

| # | function | lines | cat | target | purpose | deps | io | notes |
|---:|---|---|---|---|---|---|---|---|
| 1 | `MyDBIProcess` | 23-48 (26) | util | ElaHub(log) | Analyzer-local logger: CommaText of 7 args -> mmoException + slFTPLog (FTP_Log csv). | TStringList,TMemo,TMyStringList | writes D:\HT9045_Log\UploadFile\yyyy\mm\FTP_Log_yyyymmdd.csv | NOT the Handler's MyDBIProcess (same name, different job) |
| 2 | `OpenIniFile` | 50-61 (12) | util | vclcompat/IniFile | Open/cache one TIniFile. | TIniFile | - |  |
| 3 | `CloseIniFile` | 63-70 (8) | util | vclcompat/IniFile | UpdateFile + delete cached TIniFile. | TIniFile | - | pointer not reset to NULL |
| 4 | `CheckAndReadIniData` | 72-88 (17) | util | vclcompat/IniFile | Read double; write default '%0.4f' when key missing. | TIniFile | write-back |  |
| 5 | `CheckAndReadIniData` | 90-105 (16) | util | vclcompat/IniFile | Read unsigned long (no write-back). | TIniFile | - |  |
| 6 | `CheckAndReadIniData` | 107-122 (16) | util | vclcompat/IniFile | Read int; write default when missing. | TIniFile | write-back |  |
| 7 | `CheckAndReadIniData` | 124-139 (16) | util | vclcompat/IniFile | Read bool; write default when missing. | TIniFile | write-back | used by GetJemIncludeMTBA/MTBF per stop row |
| 8 | `CheckAndReadIniData` | 141-166 (26) | util | vclcompat/IniFile | Read string; write default when missing or empty. | TIniFile | write-back |  |
| 9 | `CheckAndReadIniData` | 168-183 (16) | util | vclcompat/IniFile | Read TDateTime; write default when missing. | TIniFile | write-back |  |
| 10 | `WriteIniData` | 185-205 (21) | util | vclcompat/IniFile | Write bool. | TIniFile | write |  |
| 11 | `WriteIniData` | 207-228 (22) | util | vclcompat/IniFile | Write int. | TIniFile | write |  |
| 12 | `WriteIniData` | 230-251 (22) | util | vclcompat/IniFile | Write double. | TIniFile | write |  |
| 13 | `WriteIniData` | 253-273 (21) | util | vclcompat/IniFile | Write unsigned long. | TIniFile | write |  |
| 14 | `WriteIniData` | 275-295 (21) | util | vclcompat/IniFile | Write string. | TIniFile | write |  |
| 15 | `WriteIniData` | 297-316 (20) | util | vclcompat/IniFile | Write TDateTime. | TIniFile | write |  |
| 16 | `GetJameCodeOfAxis` | 318-343 (26) | ui | web (reuse cMyDB GetJameCodeOfAxis) | AlarmCodeList.txt -> codes whose chars 4-5 == axis -> TComboBox. | TComboBox | reads D:\HT9045\Error\AlarmCodeList.txt | same function exists in V906 cMyDB.cpp:1925 |
| 17 | `CrnGetMaxLenOfStringGridCol` | 345-358 (14) | ui | drop | Max text width of a grid column. | TStringGrid.Canvas | - | unused |
| 18 | `sgAddjust` | 360-377 (18) | ui | drop | Auto-size grid columns. | TStringGrid | - | unused |
| 19 | `ConvertSecToTime` | 379-405 (27) | util | ElaCore | Seconds -> '[N days ]HH:MM:SS[.mmm]' (report/grid format). | - | - | format is part of report byte-equality |
| 20 | `ConvertDTToTime` | 407-435 (29) | util | ElaCore | TDateTime span -> '[N days ]HH:NN:SS[.mmm]'. | - | - |  |
| 21 | `ConvertDTToStrDateTime` | 437-440 (4) | util | ElaCore | TDateTime -> 'YYYY/MM/DD HH:NN:SS'. | - | - |  |
| 22 | `getColumn` | 442-467 (26) | util | ElaProduction | Quote-aware single CSV column extract (comma-only). | - | - | used for Production_Log device count |
| 23 | `MyForceDirectories` | 469-509 (41) | util | ElaReports | ForceDirectories with logging; 1 ok / -1 fail. | - | creates folders |  |
| 24 | `GetStartEndGap` | 511-516 (6) | core | ElaCore | end-start. | - | - |  |
| 25 | `GetUPH` | 518-528 (11) | core | ElaCore | UPH = input/(days*24) (input==0 -> 0). | - | - | days==0 -> divide by zero (only <0 guarded) |
| 26 | `GetMUBA` | 530-540 (11) | core | ElaCore | MUBA = input/alarmCount (0 alarms -> input). | - | - |  |
| 27 | `GetMUBF` | 542-552 (11) | core | ElaCore | MUBF = input/failCount. | - | - | ChipAdvanced only |
| 28 | `GetMTBA` | 554-564 (11) | core | ElaCore | MTBA = gap/alarmCount (0 -> gap; input==0 -> 0). | - | - |  |
| 29 | `GetMTBF` | 566-576 (11) | core | ElaCore | MTBF = gap/failCount (0 -> gap; input==0 -> 0). | - | - |  |
| 30 | `GetTotalTestTime` | 578-582 (5) | core | drop | Total test time as TDateTime. | - | - | unused |
| 31 | `WriteDataToFile` | 584-608 (25) | util | ElaReports | Append (or overwrite) one line + '\n' with CreateFile. | Win32 CreateFile | writes | LF only (not CRLF) |
| 32 | `WriteDataToFile` | 610-613 (4) | util | ElaReports | AnsiString overload. | - | - |  |
| 33 | `MySleep` | 615-618 (4) | util | drop | Sleep wrapper. | - | - |  |
| 34 | `IsDigitString` | 620-628 (9) | core | ElaCore | All chars are digits. | - | - |  |
| 35 | `IsValidDateString` | 630-655 (26) | core | ElaCore | Date rule: length 10, 'YYYY/MM/DD' or 'YYYY-MM-DD', month 1-12, day 1-31. | - | - | CSV row gate |
| 36 | `IsValidTimeString` | 657-690 (34) | core | ElaCore | Time rule: 'HH:NN:SS' (8) or 'HH:NN:SS.mmm' (12). | - | - | CSV row gate |
| 37 | `ParseDateTime` | 692-713 (22) | core | ElaCore | Parse 'date time' (>=19 chars). | - | - | Production_Log times |
| 38 | `ParseDateTime` | 715-737 (23) | core | ElaCore | Parse date + time fields -> TDateTime (0 on failure). | - | - |  |
| 39 | `ParseDate` | 739-780 (42) | core | ElaCore | Parse Y/M/D with '/' or '-'. | EncodeDate | - |  |
| 40 | `ParseTime` | 782-800 (19) | core | ElaCore | Parse HH:NN:SS[.mmm]. | EncodeTime | - |  |
| 41 | `VerInfo::VerInfo` | 802-805 (4) | util | drop | VerInfo ctor. | Win32 version API | - | V906 has its own version source |
| 42 | `VerInfo::m_ClearData` | 807-822 (16) | util | drop | Clear version fields. | - | - |  |
| 43 | `VerInfo::m_SetFileName` | 824-828 (5) | util | drop | Set file name and load version info. | - | - |  |
| 44 | `VerInfo::m_strGetFixedFileVersion` | 830-837 (8) | util | drop | Fixed file version string. | - | - |  |
| 45 | `VerInfo::m_strGetFixedProductVersion` | 839-846 (8) | util | drop | Fixed product version string. | - | - |  |
| 46 | `VerInfo::GetAppVersion` | 848-875 (28) | util | drop | GetFileVersionInfo -> major/minor/build/rev. | Win32 version API | reads own exe |  |
| 47 | `VerInfo::GetSVNRev` | 877-885 (9) | util | drop | 'Rev' from own exe version. | - | - |  |
| 48 | `VerInfo::GetFileVersion` | 887-895 (9) | util | drop | 'x.y.z.w' from own exe (status bar). | - | - |  |
| 49 | `VerInfo::m_GetVerInfo` | 897-1018 (122) | util | drop | Read StringFileInfo keys. | Win32 VerQueryValue | - |  |

### 9.3 uAnalysisEventLogText.cpp (5 functions)

| # | function | lines | cat | target | purpose | deps | io | notes |
|---:|---|---|---|---|---|---|---|---|
| 1 | `AnalysisEventLogText::AnalysisEventLogText` | 12-15 (4) | core | ElaProduction | ctor. | - | - |  |
| 2 | `AnalysisEventLogText::~AnalysisEventLogText` | 17-20 (4) | core | ElaProduction | dtor. | - | - |  |
| 3 | `AnalysisEventLogText::Clear` | 22-25 (4) | core | ElaProduction | Clear (empty). | - | - |  |
| 4 | `AnalysisEventLogText::GetEventLogName` | 27-33 (7) | core | ElaProduction | 'EventLogTxt_yyyymmdd.csv' for a date. | - | - | hard-coded default name (ignores O15/N10 name variants) |
| 5 | `AnalysisEventLogText::GetEventLogFolder` | 35-41 (7) | core | ElaProduction | 'D:\HT9045_Log\EventLogTxt\yyyy\mm' for a date. | - | - |  |

### 9.4 uAnalysisProductData.cpp (5 functions)

| # | function | lines | cat | target | purpose | deps | io | notes |
|---:|---|---|---|---|---|---|---|---|
| 1 | `AnalysisProductData::AnalysisProductData` | 10-13 (4) | core | drop | ctor. | - | - | unit unused |
| 2 | `AnalysisProductData::~AnalysisProductData` | 15-18 (4) | core | drop | dtor. | - | - | unit unused |
| 3 | `AnalysisProductData::Clear` | 20-28 (9) | core | drop | Clear. | - | - | unit unused |
| 4 | `AnalysisProductData::GetProductDatasToVec` | 30-34 (5) | core | drop | Range from pickers -> overload. | frmELA pickers | - | unit unused |
| 5 | `AnalysisProductData::GetProductDatasToVec` | 36-54 (19) | core | drop | Loop over days building ProductData names; body commented out. | - | D:\HT9045_Log\ProductData (never read) | stub |

### 9.5 uAnalysisProductionLog.cpp (12 functions)

| # | function | lines | cat | target | purpose | deps | io | notes |
|---:|---|---|---|---|---|---|---|---|
| 1 | `AnalysisProductionLog::AnalysisProductionLog` | 16-19 (4) | core | ElaProduction | ctor. | - | - |  |
| 2 | `AnalysisProductionLog::~AnalysisProductionLog` | 21-24 (4) | core | ElaProduction | dtor. | - | - |  |
| 3 | `AnalysisProductionLog::Clear` | 26-34 (9) | core | ElaProduction | Clear. | - | - |  |
| 4 | `AnalysisProductionLog::GetProductInfoTextToVec` | 36-40 (5) | core | drop | Range from pickers -> overload. | frmELA pickers | - | unused |
| 5 | `AnalysisProductionLog::GetProductInfoTextToVec` | 42-60 (19) | core | drop | Read records of <title>_<yyyymmdd>.csv per day. | - | reads Production_Log\yyyymm\ | unused |
| 6 | `AnalysisProductionLog::GetProInfoDevice` | 62-79 (18) | core | ElaProduction | Sum in-range device rows over the day files. | - | reads D:\HT9045_Log\Production_Log\yyyymm\<title>_yyyymmdd.csv | day loop steps from start time-of-day: misses the last day when end time-of-day < start time-of-day |
| 7 | `AnalysisProductionLog::readCSV` | 81-99 (19) | core | drop | Read CSV lines (skip header). | std::ifstream | - | unused path |
| 8 | `AnalysisProductionLog::readCSVAndGetDeviceCount` | 101-121 (21) | core | ElaProduction | Count in-range rows of one file. | std::ifstream | reads one Production_Log csv |  |
| 9 | `AnalysisProductionLog::AnalysisRowData` | 123-161 (39) | core | drop | CommaText row -> TProductionRecord if LoadTime in range. | TStringList.CommaText | - | unused |
| 10 | `AnalysisProductionLog::AnalysisRowDataOnlyTime` | 163-185 (23) | core | drop | Count rows by LoadTime (vector). | - | - | unused |
| 11 | `AnalysisProductionLog::AnalysisRowDataOnlyTime` | 187-202 (16) | core | ElaProduction | 1 if the row's eLoadTime column is in range. | - | - |  |
| 12 | `AnalysisProductionLog::GetDeviceCount` | 204-208 (5) | core | ElaProduction | Device count for a range. | - | - | caller sets sFileTitle=gethostname() -> 0 when the Production_Log prefix is not the Windows host name |

### 9.6 uChipAdvancedFunc.cpp (16 functions)

| # | function | lines | cat | target | purpose | deps | io | notes |
|---:|---|---|---|---|---|---|---|---|
| 1 | `ChipAdvancedFunc::ChipAdvancedFunc` | 12-15 (4) | report | ElaReports | ctor. | - | - |  |
| 2 | `ChipAdvancedFunc::~ChipAdvancedFunc` | 17-20 (4) | report | ElaReports | dtor. | - | - |  |
| 3 | `ChipAdvancedFunc::AnalysisLog` | 22-72 (51) | report | ElaReports | EL_UPLOAD_CHIPADV_LOTEND: last 'Lot Start, Lot ID:' within 8 days -> range lot start..now -> full query -> anomaly stats -> report CSV. | frmELA pickers, SetStringGrid/GetEventLogText/ListProductionLog | reads EventLogTxt; writes ClipAdv_<LotID>_<y><m><d>.csv | anomaly loop starts at i=1 (skips first record); uses accumulated MySummary.iFailCount |
| 4 | `ChipAdvancedFunc::HadAnomaly` | 74-87 (14) | report | ElaReports | Aggregate count/time per alarm code. | - | - |  |
| 5 | `ChipAdvancedFunc::GetEventLogReport` | 89-98 (10) | report | ElaReports | Report = metrics + OEE block + alarm block. | - | - |  |
| 6 | `ChipAdvancedFunc::GetPerformanceMetrics` | 100-124 (25) | report | ElaReports | MTBA/MUBA/MTBF/MUBF/UPH/qty lines. | - | - |  |
| 7 | `ChipAdvancedFunc::GenerateOEEReport` | 126-152 (27) | report | ElaReports | Start/stop/analysis/production/alarm/downtime lines; 'OEE'=(input-fail)/input. | - | - | 'OEE' is really a pass ratio |
| 8 | `ChipAdvancedFunc::GenerateAlarmReport` | 154-175 (22) | report | ElaReports | Per-code count/time ratios. | - | - |  |
| 9 | `ChipAdvancedFunc::GetPerformanceData` | 177-180 (4) | report | ElaReports | 'a,b,c\n' line. | - | - |  |
| 10 | `ChipAdvancedFunc::GetAnomalyStatsData` | 182-191 (10) | report | ElaReports | 5-column line. | - | - |  |
| 11 | `ChipAdvancedFunc::GetHourMinSecStr` | 193-201 (9) | report | ElaReports | TDateTime -> total-hours 'HH:MM:SS.ss'. | - | - |  |
| 12 | `ChipAdvancedFunc::GetDateTimeStr` | 203-206 (4) | report | ElaReports | 'yyyy/mm/dd hh:nn:ss'. | - | - |  |
| 13 | `ChipAdvancedFunc::GetDaybySec` | 208-211 (4) | report | ElaReports | seconds -> days. | - | - |  |
| 14 | `ChipAdvancedFunc::FindStartLotFromRecVec` | 213-230 (18) | report | ElaReports | Search records backwards for 'Lot Start, Lot ID:'; parse Lot/OP/RunMode. | - | - | size_t i=size-1 underflows when no records -> crash |
| 15 | `ChipAdvancedFunc::SaveReport` | 232-241 (10) | report | ElaReports | Append report to N34 path. | FileInfo | writes <sN34_OEEAlarmRptPath>\ClipAdv_<LotID>_<y><m><d>.csv (append, no zero pad) |  |
| 16 | `ChipAdvancedFunc::ParseField` | 243-257 (15) | report | ElaReports | Value after 'Field:' up to ','. | - | - |  |

### 9.7 uChipMosZHUBEI_Func.cpp (13 functions)

| # | function | lines | cat | target | purpose | deps | io | notes |
|---:|---|---|---|---|---|---|---|---|
| 1 | `ChipMosZHUBEI_Func::ChipMosZHUBEI_Func` | 15-18 (4) | report | ElaReports | ctor. | - | - |  |
| 2 | `ChipMosZHUBEI_Func::~ChipMosZHUBEI_Func` | 20-23 (4) | report | ElaReports | dtor. | - | - |  |
| 3 | `ChipMosZHUBEI_Func::SaveJamCodeFor7Days` | 25-56 (32) | report | ElaReports | 'Jam Rate.txt': header + every record whose AlarmCode contains JAM, range now-8 00:00 .. now-1 23:59:59. | frmELA pickers | writes <folder>\Jam Rate.txt (append) | 8 days although named 7 |
| 4 | `ChipMosZHUBEI_Func::SaveJamSummaryFor30Days` | 58-120 (63) | report | ElaReports | 'Jam_Summary.csv': split records at Recovery containing CLEAN_OUT; per interval end time, device count (Production_Log), JAM count. | - | writes <folder>\Jam_Summary.csv (append); reads Production_Log | tIntervalRecs[size-1] on empty data -> crash |
| 5 | `ChipMosZHUBEI_Func::SaveJamSummaryFor30Days` | 122-132 (11) | report | ElaReports | Range now-31 00:00 .. now-1 23:59:59 -> overload. | frmELA pickers | - |  |
| 6 | `ChipMosZHUBEI_Func::SaveEventLog` | 134-143 (10) | report | ElaReports | Copy yesterday's EventLogTxt_yyyymmdd.csv to <folder>\EventLog.txt. | CopyFile | reads EventLogTxt; writes EventLog.txt | hard-coded default file name |
| 7 | `ChipMosZHUBEI_Func::N10_UploadDataToFTP` | 145-182 (38) | comm | ElaHub/KYECFTP | N10 FTP upload of one file (cN10Ftp* credentials). | TfFTP | FTP_Log | result only logged |
| 8 | `ChipMosZHUBEI_Func::N25_UploadJamDataToFTP` | 184-255 (72) | comm | ElaHub/KYECFTP | N25 FTP: <path>/<hostname>/<file> and BackUp/<name>_yyyymmdd_hhnn.<ext>; logs to HadUpload.txt. | TfFTP | reads local file; appends HadUpload.txt | also used for EventLog upload |
| 9 | `ChipMosZHUBEI_Func::N25_UploadSummaryCountToFTP` | 257-326 (70) | comm | ElaHub/KYECFTP | N25 FTP for Jam_Summary (same layout). | TfFTP | - |  |
| 10 | `ChipMosZHUBEI_Func::UploadJamCode` | 328-350 (23) | comm | ElaHub | EL_UPLOAD_JAMWEEK: D:\HT9045_Log\JamWeek\Y\M\D; skip if HadUpload.txt; SaveJamCodeFor7Days; N25 upload; mark. | - | JamWeek folder | once per day guard |
| 11 | `ChipMosZHUBEI_Func::UploadSummaryCount` | 352-370 (19) | comm | ElaHub | EL_UPLOAD_SUMMARY: D:\HT9045_Log\SummaryCount\Y\M\D; Jam_Summary; N25 upload; mark. | - | SummaryCount folder |  |
| 12 | `ChipMosZHUBEI_Func::UploadEventLog` | 372-390 (19) | comm | ElaHub | EL_UPLOAD_EVENTLOG: D:\HT9045_Log\UploadEventLog\Y\M\D; copy yesterday's log; N25 upload; mark. | - | UploadEventLog folder |  |
| 13 | `ChipMosZHUBEI_Func::UploadByFile_N10` | 392-418 (27) | comm | ElaHub | EL_UPLOAD_BYFILE_N10: each row of UploadFile.csv (src dir, dst dir, src file, dst file) -> N10 upload. | TStringList.CommaText | reads D:\HT9045_Log\UploadFile\UploadFile.csv | file never truncated here (DeleteFile commented) -> re-uploads all rows each time |

### 9.8 TfFTP.cpp (30 functions)

| # | function | lines | cat | target | purpose | deps | io | notes |
|---:|---|---|---|---|---|---|---|---|
| 1 | `TfFTP::TfFTP` | 13-41 (29) | comm | KYECFTP (replace) | Create TNMFTP, bind events (SOFT_SIMULTE hard-codes test credentials). | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 2 | `TfFTP::~TfFTP` | 43-51 (9) | comm | KYECFTP (replace) | Close + free lists. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 3 | `TfFTP::Connect` | 53-81 (29) | comm | KYECFTP (replace) | Connect (TimeOut 5000, poll Connected 100x5 ms). | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 4 | `TfFTP::ChangeDirectories` | 83-142 (60) | comm | KYECFTP (replace) | Multi-level cd with mkdir (N10). | TNMFTP (FastNet) | FTP_Log via MyDBIProcess | BUG: MakeDirectory(sCurr) instead of the next folder; loop starts at segment 1 |
| 5 | `TfFTP::ChangeDir` | 144-171 (28) | comm | KYECFTP (replace) | cd; on failure mkdir (single level). | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 6 | `TfFTP::CheckFTPFilePath` | 173-182 (10) | comm | drop | Normalise FTP path. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess | unused |
| 7 | `TfFTP::CheckLocalFilePath` | 184-193 (10) | comm | KYECFTP (replace) | Normalise local path. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 8 | `TfFTP::Rename` | 195-228 (34) | comm | drop | Remote rename. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess | unused |
| 9 | `TfFTP::GetFileList` | 230-233 (4) | comm | drop | List (stub). | TNMFTP (FastNet) | FTP_Log via MyDBIProcess | unused |
| 10 | `TfFTP::DownloadFilterFile` | 235-283 (49) | comm | drop | Download files matching a filter. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess | unused |
| 11 | `TfFTP::DownloadFilterFile_Get1stFileName` | 285-333 (49) | comm | drop | Download first match. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess | unused |
| 12 | `TfFTP::DownloadFilterFile_GetLastFileName` | 335-385 (51) | comm | drop | Download last match. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess | unused |
| 13 | `TfFTP::Upload` | 387-390 (4) | comm | KYECFTP (replace) | Upload (same / renamed target). | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 14 | `TfFTP::Upload` | 392-423 (32) | comm | KYECFTP (replace) | Upload (same / renamed target). | TNMFTP (FastNet) | FTP_Log via MyDBIProcess | guard `=="NULL" && ==""` is always false |
| 15 | `TfFTP::Delete` | 425-455 (31) | comm | drop | Remote delete by filter. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess | unused |
| 16 | `TfFTP::GetAllFolder` | 457-488 (32) | comm | drop | List remote folders. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess | unused |
| 17 | `TfFTP::Close` | 490-512 (23) | comm | KYECFTP (replace) | Disconnect. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 18 | `TfFTP::RemoveAllTrailingSlashes` | 514-522 (9) | comm | KYECFTP (replace) | Trim trailing '/'. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 19 | `TfFTP::NMFTP2ListItem` | 524-527 (4) | comm | KYECFTP (replace) | NMFTP event handler -> MyDBIProcess log line. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 20 | `TfFTP::NMFTP2Success` | 529-551 (23) | comm | KYECFTP (replace) | NMFTP event handler -> MyDBIProcess log line. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 21 | `TfFTP::NMFTP2AuthenticationFailed` | 553-557 (5) | comm | KYECFTP (replace) | NMFTP event handler -> MyDBIProcess log line. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 22 | `TfFTP::NMFTP2TransactionStop` | 559-564 (6) | comm | KYECFTP (replace) | NMFTP event handler -> MyDBIProcess log line. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 23 | `TfFTP::NMFTP2TransactionStart` | 566-571 (6) | comm | KYECFTP (replace) | NMFTP event handler -> MyDBIProcess log line. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 24 | `TfFTP::NMFTP2ConnectionFailed` | 573-578 (6) | comm | KYECFTP (replace) | NMFTP event handler -> MyDBIProcess log line. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 25 | `TfFTP::NMFTP2Failure` | 580-602 (23) | comm | KYECFTP (replace) | NMFTP event handler -> MyDBIProcess log line. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 26 | `TfFTP::NMFTP2UnSupportedFunction` | 604-625 (22) | comm | KYECFTP (replace) | NMFTP event handler -> MyDBIProcess log line. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 27 | `TfFTP::NMFTP2Error` | 627-633 (7) | comm | KYECFTP (replace) | NMFTP event handler -> MyDBIProcess log line. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 28 | `TfFTP::NMFTP2Status` | 635-641 (7) | comm | KYECFTP (replace) | NMFTP event handler -> MyDBIProcess log line. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 29 | `TfFTP::NMFTP2Connect` | 643-648 (6) | comm | KYECFTP (replace) | NMFTP event handler -> MyDBIProcess log line. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |
| 30 | `TfFTP::NMFTP2Disconnect` | 650-655 (6) | comm | KYECFTP (replace) | NMFTP event handler -> MyDBIProcess log line. | TNMFTP (FastNet) | FTP_Log via MyDBIProcess |  |

### 9.9 FileInfo.cpp (40 functions)

| # | function | lines | cat | target | purpose | deps | io | notes |
|---:|---|---|---|---|---|---|---|---|
| 1 | `ParameterInfo::ParameterInfo` | 12-16 (5) | util | drop | Bind a parameter pointer to a file key. | - | - | unused |
| 2 | `ParameterInfo::~ParameterInfo` | 18-20 (3) | util | drop | dtor. | - | - | unused |
| 3 | `FileInfo::FileInfo` | 23-26 (4) | util | drop | ctor. | - | - | unused |
| 4 | `FileInfo::~FileInfo` | 28-36 (9) | util | ElaReports/ElaProduction (std::filesystem) | Free parameter map. | - | - | used |
| 5 | `FileInfo::ReadFile` | 38-41 (4) | util | drop | Read key/value file into bound parameters. | - | - | unused |
| 6 | `FileInfo::ReadFile` | 43-65 (23) | util | drop | Read key/value file into bound parameters. | - | - | unused |
| 7 | `FileInfo::WriteFile` | 67-82 (16) | util | drop | Write bound parameters. | - | - | unused |
| 8 | `FileInfo::IsFilePathExist` | 84-89 (6) | util | drop | File exists. | - | - | unused |
| 9 | `FileInfo::DecodeASCII` | 91-96 (6) | util | drop | Decode obfuscated text. | - | - | unused |
| 10 | `FileInfo::DecodeASCII` | 98-116 (19) | util | drop | Decode obfuscated text. | - | - | unused |
| 11 | `FileInfo::DecodeReadMap2Parameter` | 118-134 (17) | util | drop | Split a line into bound parameters. | - | - | unused |
| 12 | `FileInfo::SaveAsTxtFile` | 136-142 (7) | util | drop | Write text file. | - | - | unused |
| 13 | `FileInfo::GetNameAndExtension` | 144-153 (10) | util | ElaReports/ElaProduction (std::filesystem) | Split file name / extension. | - | - | used (uChipMosZHUBEI) |
| 14 | `FileInfo::GetAllFileNamesInFolder` | 155-182 (28) | util | drop | List files in a folder. | - | - | unused |
| 15 | `FileInfo::DeleteFolderContents` | 184-215 (32) | util | drop | Delete folder contents. | - | - | unused |
| 16 | `FileInfo::SplitPath` | 217-242 (26) | util | drop | Split path/name/ext. | - | - | unused |
| 17 | `FileInfo::DirectoryExist` | 244-248 (5) | util | drop | Directory exists. | - | - | unused |
| 18 | `FileInfo::RemoveAllTrailingBackslashes` | 250-258 (9) | util | drop | Trim trailing backslashes. | - | - | unused |
| 19 | `FileInfo::PathCombin` | 260-282 (23) | util | ElaReports/ElaProduction (std::filesystem) | Join path + file (FTP '/' or local '\\'). | - | - | used: picks '/' when the path contains '/', else '\\' |
| 20 | `FileInfo::EnsureDirectoriesExist` | 284-312 (29) | util | ElaReports/ElaProduction (std::filesystem) | Create each path level. | - | - | used (ChipAdvanced, ChipMos) |
| 21 | `cDatabaseMin::cDatabaseMin` | 315-318 (4) | util | drop | Minimal DB base class. | - | - | unused |
| 22 | `cDatabaseMin::~cDatabaseMin` | 320-323 (4) | util | drop | Minimal DB base class. | - | - | unused |
| 23 | `cDatabaseMin::LoadFile` | 325-328 (4) | util | drop | Minimal DB base class. | - | - | unused |
| 24 | `cDatabaseMin::SaveFile` | 330-333 (4) | util | drop | Minimal DB base class. | - | - | unused |
| 25 | `cDBTStringGrid::cDBTStringGrid` | 335-339 (5) | util | drop | TStringGrid table wrapper. | - | - | unused |
| 26 | `cDBTStringGrid::~cDBTStringGrid` | 341-344 (4) | util | drop | TStringGrid table wrapper. | - | - | unused |
| 27 | `cDBTStringGrid::Clear` | 346-362 (17) | util | drop | TStringGrid table wrapper. | - | - | unused |
| 28 | `cDBTStringGrid::SetRowCol` | 364-373 (10) | util | drop | TStringGrid table wrapper. | - | - | unused |
| 29 | `cDBTStringGrid::GetRowCol` | 375-382 (8) | util | drop | TStringGrid table wrapper. | - | - | unused |
| 30 | `cDBTStringGrid::SetColTitle` | 384-390 (7) | util | drop | TStringGrid table wrapper. | - | - | unused |
| 31 | `cDBTStringGrid::GetColTitle` | 392-400 (9) | util | drop | TStringGrid table wrapper. | - | - | unused |
| 32 | `cDBTStringGrid::SetRowTitle` | 402-408 (7) | util | drop | TStringGrid table wrapper. | - | - | unused |
| 33 | `cDBTStringGrid::GetRowTitle` | 410-424 (15) | util | drop | TStringGrid table wrapper. | - | - | unused |
| 34 | `cDBTStringGrid::SetCell` | 426-450 (25) | util | drop | TStringGrid table wrapper. | - | - | unused |
| 35 | `cDBTStringGrid::GetCell` | 452-466 (15) | util | drop | TStringGrid table wrapper. | - | - | unused |
| 36 | `cDBTStringGrid::AddRow` | 468-474 (7) | util | drop | TStringGrid table wrapper. | - | - | unused |
| 37 | `cDBTStringGrid::AddCol` | 476-482 (7) | util | drop | TStringGrid table wrapper. | - | - | unused |
| 38 | `cDBTStringGrid::CloneTo` | 484-498 (15) | util | drop | TStringGrid table wrapper. | - | - | unused |
| 39 | `cDBTStringGrid::ClearRow` | 500-520 (21) | util | drop | TStringGrid table wrapper. | - | - | unused |
| 40 | `cDBTStringGrid::HideRow` | 522-543 (22) | util | drop | TStringGrid table wrapper. | - | - | unused |

### 9.10 HTMD5.cpp (26 functions)

| # | function | lines | cat | target | purpose | deps | io | notes |
|---:|---|---|---|---|---|---|---|---|
| 1 | `MD5::F` | 78-81 (4) | util | drop | MD5 implementation (F). | - | - | unused |
| 2 | `MD5::G` | 83-86 (4) | util | drop | MD5 implementation (G). | - | - | unused |
| 3 | `MD5::H` | 88-91 (4) | util | drop | MD5 implementation (H). | - | - | unused |
| 4 | `MD5::I` | 93-96 (4) | util | drop | MD5 implementation (I). | - | - | unused |
| 5 | `MD5::rotate_left` | 100-103 (4) | util | drop | MD5 implementation (rotate_left). | - | - | unused |
| 6 | `MD5::FF` | 108-111 (4) | util | drop | MD5 implementation (FF). | - | - | unused |
| 7 | `MD5::GG` | 113-116 (4) | util | drop | MD5 implementation (GG). | - | - | unused |
| 8 | `MD5::HH` | 118-121 (4) | util | drop | MD5 implementation (HH). | - | - | unused |
| 9 | `MD5::II` | 123-126 (4) | util | drop | MD5 implementation (II). | - | - | unused |
| 10 | `MD5::MD5` | 130-133 (4) | util | drop | MD5 implementation (MD5). | - | - | unused |
| 11 | `MD5::MD5` | 137-142 (6) | util | drop | MD5 implementation (MD5). | - | - | unused |
| 12 | `MD5::init` | 144-155 (12) | util | drop | MD5 implementation (init). | - | - | unused |
| 13 | `MD5::decode` | 159-168 (10) | util | drop | MD5 implementation (decode). | - | - | unused |
| 14 | `MD5::encode` | 173-182 (10) | util | drop | MD5 implementation (encode). | - | - | unused |
| 15 | `MD5::transform` | 186-270 (85) | util | drop | MD5 implementation (transform). | - | - | unused |
| 16 | `MD5::update` | 275-312 (38) | util | drop | MD5 implementation (update). | - | - | unused |
| 17 | `MD5::update` | 316-322 (7) | util | drop | MD5 implementation (update). | - | - | unused |
| 18 | `MD5::finalize` | 327-361 (35) | util | drop | MD5 implementation (finalize). | - | - | unused |
| 19 | `MD5::hexdigest` | 365-376 (12) | util | drop | MD5 implementation (hexdigest). | - | - | unused |
| 20 | `md5` | 383-387 (5) | util | drop | MD5 of a string. | - | - | unused |
| 21 | `md5_File` | 391-426 (36) | util | drop | MD5 of a file. | - | - | unused |
| 22 | `md5_Folder` | 428-480 (53) | util | drop | MD5 of a folder. | - | - | unused |
| 23 | `CheckFilenameExtension` | 484-499 (16) | util | drop | Extension match. | - | - | unused |
| 24 | `SearchFile` | 503-543 (41) | util | drop | Find files (one level). | - | - | unused |
| 25 | `SearchFileAll` | 545-591 (47) | util | drop | Find files recursively. | - | - | unused |
| 26 | `SearchFolder` | 593-630 (38) | util | drop | List folders. | - | - | unused |

### 9.11 MyStringList.cpp (31 functions)

| # | function | lines | cat | target | purpose | deps | io | notes |
|---:|---|---|---|---|---|---|---|---|
| 1 | `TMyStringList::TMyStringList` | 18-30 (13) | util | drop | ctor (path, file name, header). | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 2 | `TMyStringList::TMyStringList` | 32-50 (19) | util | V906 Public/MyStringList (reuse) | ctor (path, file name, header). | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | used (slFTPLog ctor) |
| 3 | `TMyStringList::~TMyStringList` | 52-65 (14) | util | V906 Public/MyStringList (reuse) | Flush and free. | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | used |
| 4 | `TMyStringList::SetPath` | 67-70 (4) | util | drop | Property setter (Path). | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 5 | `TMyStringList::SetFileName` | 72-75 (4) | util | drop | Property setter (FileName). | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 6 | `TMyStringList::SetFirstRow` | 77-80 (4) | util | drop | Property setter (FirstRow). | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 7 | `TMyStringList::SetMaxLineCount` | 82-85 (4) | util | V906 Public/MyStringList (reuse) | Property setter (MaxLineCount). | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | used (MaxLineCount=1) |
| 8 | `TMyStringList::SetSaveType` | 87-90 (4) | util | V906 Public/MyStringList (reuse) | Property setter (SaveType). | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | used (TByDay) |
| 9 | `TMyStringList::SetAutoSave` | 92-95 (4) | util | drop | Property setter (AutoSave). | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 10 | `TMyStringList::SetSaveSameFolder` | 97-100 (4) | util | drop | Property setter (SaveSameFolder). | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 11 | `TMyStringList::SetSaveByLotID` | 102-105 (4) | util | drop | Property setter (SaveByLotID). | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 12 | `TMyStringList::SetSaveFixedFile` | 107-110 (4) | util | drop | Property setter (SaveFixedFile). | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 13 | `TMyStringList::SetLotData` | 112-116 (5) | util | drop | Property setter (LotData). | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 14 | `TMyStringList::AddText` | 118-132 (15) | util | drop | Add raw line. | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 15 | `TMyStringList::AddTextWithLineNo` | 134-139 (6) | util | drop | Add line as-is (SPIL writer uses it). | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 16 | `TMyStringList::AddTextForUpload` | 141-156 (16) | util | V906 Public/MyStringList (reuse) | Add N10 upload log line. | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | used (N10 upload log line) |
| 17 | `TMyStringList::AddTextWithDateTime` | 158-173 (16) | util | drop | Add 'YYYY-MM-DD, HH:NN:SS.mmm, ' + text. | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 18 | `TMyStringList::AddTextWithDateTime` | 175-193 (19) | util | V906 Public/MyStringList (reuse) | Add 'YYYY-MM-DD, HH:NN:SS.mmm, ' + text. | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | used (MyDBIProcess) |
| 19 | `TMyStringList::AddTextWithDateTime2` | 195-211 (17) | util | drop | Add 'YYYY/MM/DD HH:NN:SS,' + text. | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 20 | `TMyStringList::AddTextWithDateTime3` | 213-228 (16) | util | drop | Add date/time/ms + 2 texts. | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 21 | `TMyStringList::GetTimeInfo` | 230-236 (7) | util | V906 Public/MyStringList (reuse) | Snapshot Now(). | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | used |
| 22 | `TMyStringList::GetYesterdayInfo` | 238-243 (6) | util | drop | Snapshot yesterday. | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 23 | `TMyStringList::MySaveToFile` | 245-248 (4) | util | V906 Public/MyStringList (reuse) | Flush. | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | used |
| 24 | `TMyStringList::MySaveToFileShareMode` | 250-431 (182) | util | V906 Public/MyStringList (reuse) | Append buffered lines with share mode (+ AllEventLog/ByLot copies). | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | used |
| 25 | `TMyStringList::MySaveFileByFileNameAndType` | 433-470 (38) | util | drop | Save to explicit name/type. | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 26 | `TMyStringList::MySaveFileByFileName` | 472-498 (27) | util | drop | Save to explicit name. | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 27 | `TMyStringList::GetFileName` | 500-672 (173) | util | V906 Public/MyStringList (reuse) | Build path/name per SaveType (by day/hour/.../month). | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | used |
| 28 | `TMyStringList::GetLastLine` | 674-702 (29) | util | drop | Line count of current file. | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 29 | `TMyStringList::SaveTryCatchLog` | 704-716 (13) | util | drop | Log a save failure. | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 30 | `TMyStringList::MyInsertToFile` | 718-773 (56) | util | drop | Insert a line at an index (alarm row writer). | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |
| 31 | `TMyStringList::MySaveSGJamCountToFile` | 775-837 (63) | util | drop | SG jam-count file. | TStringList | D:\HT9045_Log\UploadFile\...\FTP_Log_*.csv | unused |

### 9.12 SgdToXLS.cpp (1 functions)

| # | function | lines | cat | target | purpose | deps | io | notes |
|---:|---|---|---|---|---|---|---|---|
| 1 | `SGDToXLS` | 11-14 (4) | report | ElaReports(P7) | TStringGrid -> BIFF XLS (XLSfile.pas StringGridToXLS). | TStringGrid, Delphi unit | writes .xls |  |

### 9.13 EventlogAnalyzer.cpp (1 functions)

| # | function | lines | cat | target | purpose | deps | io | notes |
|---:|---|---|---|---|---|---|---|---|
| 1 | `WinMain` | 8-37 (30) | ui | drop | WinMain: single-instance mutex 'MyMutexAnalyzer', create TfrmELA. | VCL Application | - |  |

### 9.14 XLSfile.pas (13 functions)

| # | function | lines | cat | target | purpose | deps | io | notes |
|---:|---|---|---|---|---|---|---|---|
| 1 | `DataSetToXLS` | 56-89 (34) | report | ElaReports(P7) or CSV | TDataSet -> XLS. | Delphi TFileStream | writes .xls | unused |
| 2 | `StringGridToXLS` | 91-111 (21) | report | ElaReports(P7) or CSV | TStringGrid -> BIFF XLS (row cap). | Delphi TFileStream | writes .xls | used by SGDToXLS |
| 3 | `TXLSWriter.create` | 115-125 (11) | report | ElaReports(P7) or CSV | Open file stream, write BOF. | Delphi TFileStream | writes .xls |  |
| 4 | `TXLSWriter.destroy` | 127-132 (6) | report | ElaReports(P7) or CSV | Write EOF, close. | Delphi TFileStream | writes .xls |  |
| 5 | `TXLSWriter.WriteBOF` | 134-141 (8) | report | ElaReports(P7) or CSV | BIFF BOF record. | Delphi TFileStream | writes .xls |  |
| 6 | `TXLSWriter.WriteDimension` | 143-151 (9) | report | ElaReports(P7) or CSV | BIFF DIMENSIONS record. | Delphi TFileStream | writes .xls |  |
| 7 | `TXLSWriter.CellDouble` | 153-164 (12) | report | ElaReports(P7) or CSV | NUMBER cell. | Delphi TFileStream | writes .xls |  |
| 8 | `TXLSWriter.CellWord` | 166-176 (11) | report | ElaReports(P7) or CSV | INTEGER cell. | Delphi TFileStream | writes .xls |  |
| 9 | `TXLSWriter.CellStr` | 178-192 (15) | report | ElaReports(P7) or CSV | LABEL cell. | Delphi TFileStream | writes .xls |  |
| 10 | `SetCellAtribut` | 194-261 (68) | report | ElaReports(P7) or CSV | Cell attribute bytes. | Delphi TFileStream | writes .xls |  |
| 11 | `TXLSWriter.WriteWord` | 263-266 (4) | report | ElaReports(P7) or CSV | Write a word. | Delphi TFileStream | writes .xls |  |
| 12 | `TXLSWriter.WriteEOF` | 268-272 (5) | report | ElaReports(P7) or CSV | BIFF EOF record. | Delphi TFileStream | writes .xls |  |
| 13 | `TXLSWriter.WriteField` | 274-286 (13) | report | ElaReports(P7) or CSV | Write a TField cell. | Delphi TFileStream | writes .xls | DataSet path only |

## P1a ElaCore（20260927，St02）

- 新檔：`EventLogAnalysis/ElaCore.h/.cpp`，函式庫 `ht9045_ela`（標準 C++＋vclcompat 的 TIniFile），還沒連進 wb_serve（/api/ela 路由照 github-59 先 HOLD）。
- 翻了什麼（golden Rev891.0）：
  - `TfrmELA::GetEventLogText` 的多日統計（Analyzer.cpp:782-1181）：stop／MTBA／MTBF／JAM／WAR／MES 計數與秒數、by day／by hour、Jam／War／Alarm 彙總、By-Area（30 個 unit）與 By-Function（RTC／Temp／ESD／2DID／OCR／AutoClean／Yield）清單、檔名規則（含 VTEST 915／919）。
  - `SetStringGrid` 不含表格的部分（:646-780）：by-day 從起始日 0 點、by-hour 從起始日期＋時間，`do … while(cur<dtEnd)`；`GetSysDateTimeRange`／`GetTotalDays`。
  - `GetJamLevel`／`GetJemIncludeMTBA`／`GetJemIncludeMTBF`（:2799-2882）與 Common.cpp `CheckAndReadIniData`（沒有的 key 寫回預設）。
  - BCB6 RTL：`TStrings::SetCommaText`（SetDelimitedText＋AnsiExtractQuotedStr，逐 index 照 Delphi）、`GetCommaText`、`LoadFromFile` 的換行規則；Common.cpp 的日期驗證與解析（:620-800）。
  - TDateTime 用 double（1899-12-30 起算的天數），格式化照機台 zh-TW 的 `/` 日期分隔。
- **暫照 golden（A），待使用者確認**（progress-st02 #15～#19），都有 `ela::Options` 開關：
  - #15 `bcbCommaText=true`：未加引號的空白也切欄（`16 System` 那種行會切壞、不計）；false＝只切逗號（B）。
  - #16 SPIL 不分析（golden 形狀，不用開關）。
  - #17 `jamWriteBack=true`：JAM0000.dat 沒有的 key 寫回預設；路徑 `jamIniPath` 可注入，ctest 一律在 %TEMP%。
  - #18 `keepGoldenLeaks=true`：`InitData` 不清 iFailCount、By-Area／By-Function 清單不清；false 兩個都修。
  - #19 檔名照 golden 子字串比對，不去重。
- 不翻：ChangeLog／ParameterLog 的 label 區塊（:1039-1162，只有畫面 label，實際上不會命中）、ProgressBar／StatusBar。
- 已知不確定：清單尾端多一個逗號時補一個空欄，是 Delphi 7 RTL 的行為；BCB6 是否相同要用參考 exe 對一次（#23 的 oracle）。
- ctest `ELA_Core`（`tests/test_ela_core.cpp`）：CommaText 各情況、日期、SetRange 的 key、兩天真形狀的檔（重複、超出範圍、2/30、未加引號的 16 System、Motion 的 WAR、誘餌檔名）、JAM0000.dat 寫回與唯讀、golden 洩漏與修正、#15 B。全部在 `%TEMP%\ht9045_ela_core`。
  CommaText 的測試向量先用 Python 照同一套 index 邏輯鏡像跑過一次（全過）。
- 下一步 P1b：見下一節。
- ⚠ 未編譯（這台沒有 MinGW）。

## P1b 表格、摘要、Production_Log、檔案掃描（20260927，St02）

- 新檔 `EventLogAnalysis/ElaTables.cpp`（同一個 `ht9045_ela`）。表格用「golden 格子裡的字」表示：第 0 列＝golden 表格第 1 列（表頭不存）；"No Record!!" 放在第 1 列第 1 欄，跟 golden `Cells[1][1]` 一樣。
- 翻了什麼：
  - `UpdateSgTop5Filter`（:2061-2204）、`UpdateSgTop5`（:2205-2233）、`UpdateSgFailAndAlarm`（:1949-1993）、`UpdateSgByFilter`（:1995-2059）；
  - `UpdateSgProduction`（:1787-1947，sgByDay／sgByHour 14 欄）、`SetTotalSummary`（:2468-2566，mmoSummary 20 行）；
  - `ListProductionLog`（:2316-2466）：Production_Log → input／touch-down／test time，結尾照 golden 呼叫 UpdateSgProduction＋SetTotalSummary；
  - `LoadFavorite`（:1644-1678，遞迴 *.csv，略過隱藏檔）；
  - Common.cpp：`ConvertSecToTime`／`ConvertDTToTime`／`GetUPH`／`GetMUBA`／`GetMUBF`／`GetMTBA`／`GetMTBF`；
  - SysUtils `StrToDateTime`（zh-TW：y/M/d、`/`、`:`、`.`）給 golden 隱含的 AnsiString→TDateTime：日期＋時間或只有時間；`.7` 是 7 ms（ScanNumber 把數字當整數）。不支援兩位數年份、只有 M/D、上午／下午（Handler 不會寫）。
- 照留的 golden 行為（記錄，要改就是 #18 的 B）：
  - by-hour 的測試時間用 `dtTestTime × OneHourMS`（應該是 OneDayMS），數值偏小；
  - By Filter 剛好一筆時，"No Record!!" 會蓋掉那一筆的第 1 欄（Time）；
  - `UpdateSgTop5Filter` 不清它沒重寫的列：沒重新查詢就切 JAM／WAR／全部，後面的列會留著上一個篩選的資料；
  - `UpdateSgTop5` 用前綴比對 AlarmCode（`Pos(code)==1`），較長的碼也會被算進去；
  - 功能篩選的勾選框對應是交叉的（ctor :257-263：cb2DID 管 efTemp、cbTemperature 管 efESD、cbOCR 管 ef2DID、cbESD 管 efOCR）——`UpdateSgByFilter` 收的是依清單索引的布林陣列，頁面要照 golden 交叉或改正；
  - golden 表格在 Top5 最後多一列空白（RowCount 從 2 開始再逐筆 ++），這裡只存有資料的列；
  - LoadTime 轉不過去時 golden 例外直接離開 ListProductionLog（表格與摘要不更新）；測試時間轉不過去在 try 內，記 `":" + 訊息`（golden `e.HelpContext + ":"` 是 int 加 char* 的指標運算，HelpContext 0 時就是 ":"）。MyDBIProcess 那一行沒搬（ElaCore 不寫 DB／log）。
- 需要呼叫端做的事（ElaHub／頁面）：golden `GetEventLogText` 結尾會用畫面當下的狀態呼叫 `UpdateSgTop5Filter()`／`UpdateSgFailAndAlarm()`／`UpdateSgTop5(1)`／`UpdateSgByFilter()`，ElaCore 不知道畫面狀態，所以由呼叫端傳參數呼叫；查詢順序照 golden `btnQueryClick`：SetRange（SetStringGrid）→ GetEventLogText → ListProductionLog。
- ctest `ELA_Core` 第 8 節：時間格式、StrToDateTime、LoadFavorite、各表格（含上面的 golden 行為）、Production_Log → sgByDay／sgByHour（含 by-hour 的 bug）、mmoSummary。浮點相關的期望值在測試裡用跟 golden 相同的算式算，不寫死。
- ⚠ 未編譯。

## P1 修正：日期拆解與 x87 精度（20260927，St02；筆電建 9b4b33dc：ELA_Core 58 過／4 失敗）

- ① by-hour 少一筆（04/02 00:30 的 JAM 在 by-day 有、by-hour 沒有）：
  - 原因：我的 `DecodeDateTime` 先 floor 日期、再把時間部分四捨五入。SetRange 從 08:00 加 16 次 1/24 之後，46113.99999999… 被拆成 04/01＋86,400,000 ms＝「2026/04/01 24:00:00」。
    真正的「2026/04/02 00:00:00」那個 key 從來沒建，find() 找不到就漏掉（筆電量到 byHourKeys[16] 就是那個字串）。
  - golden：Delphi 6 `DateTimeToTimeStamp` 是 `FISTP(DateTime × MSecsPerDay)`，先把整個值四捨五入到毫秒、再 div／mod 拆日期與時間，進位會進到日期。
  - 修法：新增 `ela::DateTimeToMs`（x87 的 long double 乘法＋llrint，四捨六入五成雙），`DecodeDateTime` 改成拆它；所有格式化都經過它。
  - Python 鏡像（每步都是 double，跟 golden 的 TDateTime 變數一樣）重現了筆電量到的「24:00:00」，改法後得到正確的 key。
- ② StrToDateTime 三個 `==` 比較失敗：
  - 原因是 32 位元 x87：測試裡 `EncodeDate(...)+EncodeTime(...)` 留在 80 位元暫存器，拿去跟已經存回記憶體（double）的結果比。golden 每個 TDateTime 結果都存成 double。
  - 修法：`EncodeDate`／`EncodeTime`／`ParseDateTime` 的結果與 SetRange 每一步累加都強制經過記憶體成為 double（golden 的變數本來就在記憶體）；
    測試改用 golden 自己的拆法比較（`DateTimeToMs` 整數毫秒），不是放寬誤差。sgByDay／sgByHour 測試時間的期望值改用 analyzer 存好的輸入算。
- 新增回歸檢查：byHourKeys[16]＝「2026/04/02 00:00:00」；差 1 µs 到午夜的值會進位到隔天。
- ⚠ 仍未在 St02 編譯；請筆電／St01 重建。

## P2／P3 ElaHub：EL_* 指令、worker、查詢快照（20260927，St02）

- 新檔 `EventLogAnalysis/ElaHub.h/.cpp`（`ht9045_ela`，WebBridge/Sync.h 的 WbMutex／WbThread；MinGW 6.3 沒有 std::thread）。
- golden 形狀：
  - Handler 的 `SendCommand_EventLog` 送 WM_COPYDATA，`OnMyCopyMsg` 只把 `bEL_<cmd>` 舉起來（重複送會合併成一次）；
  - `TAnalysisProcessThread` 每 ≥500 ms 跑一個舉起的工作，順序 UPDATE_PARAMETER > UPLOAD_JAMWEEK > UPLOAD_SUMMARY > UPLOAD_CHIPADV_LOTEND > UPLOAD_EVENTLOG > UPLOAD_BYFILE_N10 > VTEST_MTBF_SUM。
  - golden 的這個「執行緒」其實 Synchronize 回 UI 執行緒做事；這裡真的在 worker 上做（計畫：分析永遠不在機台 tick 上跑）。
- 對應：
  - `Hub::Post(cmd)`＝OnMyCopyMsg；`Hub::RunOnce()`＝一次 AnalysisThreadProcess；`Start()`／`Stop()`＝worker（RunOnce＋SleepEx(500)）。
  - `Hub::PostQuery()`＝頁面的 Query 按鈕（golden btnQueryClick）：沒有 EL 工作時才跑；只留一個待跑的查詢，新的蓋舊的。
    查詢順序 SetRange → GetEventLogText → 四個 UpdateSg*（用請求裡的畫面狀態）→ ListProductionLog；兩個檔案清單照 golden `ImportProductionLod(2)` 用 LoadFavorite 重掃。
  - 一個 Hub 一個 Analyzer，跨查詢存活（跟 golden 只有一個 TfrmELA 一樣），所以 #18 的 golden 洩漏在 web 版也一樣出現。
  - `SnapshotJson()`：最後一次查詢的摘要與各表格（每個表最多 5,000 列，超過標 Truncated）、狀態、工作歷史、設定（**兩個 FTP 密碼不放**）。
    文字轉 UTF-8：BCB 寫的 log 是 cp950，V906 寫的是 UTF-8；合法 UTF-8 原樣，否則 cp950 → UTF-8。
- 已翻：`EL_UPDATE_PARAMETER` → `ReadConfig`（:3090-3141）讀進 `ElaConfig`（golden 元件名）。
  - `CheckAndReadIniData` 會把缺的 key 寫回 config.ini（字串版值是空的也會補），跟 #17 同一件事，用同一個 `jamWriteBack` 開關；路徑 `HubPaths` 可注入。
  - golden 讀的 `sCustCode`（Gerneral.ini [System] CUSTOMER_CODE）會傳給 Analyzer，影響 VTEST 915／919 的檔名規則。
- HOLD：其他六個工作（ChipMos N25 FTP、ChipAdvanced、N10、VTEST MTBF）照 golden 順序接受，但只在歷史記「not ported (HOLD)」——計畫 §5（客戶報表範圍、FTP 用 KYECFTP）是 Jimmy 的決定（progress-st02 #22）。
- 還沒接：`/api/ela` 路由、eventlog.html、InterfaceSYS `SendCommand_EventLog` 改呼叫 `Hub::Post`——都是共用檔，要先給 github-59 確切行號。所以現在除了 ctest 沒有人用 Hub。
- ctest `ELA_Hub`（`tests/test_ela_hub.cpp`，全部在 `%TEMP%\ht9045_ela_hub`）：閒置、golden 順序與合併、ReadConfig 的值與 38 個寫回、HOLD 工作的紀錄、查詢快照（含跳脫字元）、worker 執行緒、cp950→UTF-8、唯讀模式。
- ⚠ 未編譯。

## P3 服務入口與 P4 網頁（20260927，St02；新檔先推，共用檔的接線等 github-59 核對行號）

- 新檔 `EventLogAnalysis/ElaService.cpp`（`ht9045_ela`）：唯一的正式 Hub 與 wb_serve 入口。
  - `W906_ElaStart()`：照 golden 開機（FormShow :418-424＋Timer1Timer :443-485）——sHandlerID 取 Gerneral.ini [Version] Machine ID（檔案不在就 "HT-90xx"）；
    排一次「今天 00:00～現在」的查詢（cbTopAlarmFilter 0、rgFilter 0、全部勾選），舉起 `EL_UPDATE_PARAMETER`，啟動 worker。
    與 golden 的差別：Hub 先跑舉起的工作，所以 ReadConfig（sCustCode）在第一次查詢之前；golden 在 VTEST（915／919）機台的第一次查詢還用非 VTEST 的檔名。
  - **⚠ #17 ELA-3，暫照 golden（A）：開機時 ReadConfig 會把缺的 key 寫進真的 `D:\HT9045\config\config.ini`**（golden CheckAndReadIniData），
    Gerneral.ini 沒有 Machine ID 也會寫。`Options::jamWriteBack=false` 可以關掉。
  - `W906_ElaHttp()`：`GET /api/ela`（快照）、`POST /api/ela/query?from=&to=&top=&filter=&area=&func=&row=`（排查詢；跟其他 POST 一樣看 `--allow-cmd`，因為查詢可能寫 JAM0000.dat 的預設值）。
    沒有 Hub（`HT9045_ELA=0` 或還沒啟動）回 503。
  - `W906_ElaPost(cmd)`：給 Handler 的 `SendCommand_EventLog` 用（經 InterfaceSYS.cpp 的 hook）。沒有 Hub＝golden「找不到分析器視窗」，什麼都不做。
  - `W906_ElaStop()`：停 worker、刪 Hub。
- 新檔 `web/page/eventlog.html`：Summary／By Day／By Hour／Top5／Alarm／Fail／By Filter／Log 分頁。
  - 查詢送 `POST /api/ela/query`，每秒讀 `GET /api/ela`，seq 變了才重畫。
  - Top5 下半部在頁面上照 golden UpdateSgTop5 算（AlarmCode 前綴比對、取 Date／Time／UnitName／Message），點列不用重查。
  - 功能篩選勾選框照 golden 交叉對應（#18 A）；沒有 HOLD 工作的按鈕；FTP 密碼不顯示。
  - 沒動 `Data.Observer.html`（tsEventLogTxt 下架是 §5-1，Jimmy 決定），也沒加選單入口（另外認領）。
- 共用檔的接線（寫這節時還沒改；已接，見下一節）：wb_serve.cpp :439（宣告）、:2867（路由）、:4165（開機）、:5967（關機）；
  CMakeLists.txt :3420（wb_serve 連 `ht9045_ela`）；Interface/InterfaceSYS.cpp :552 `SendCommand_EventLog` 本體改呼叫 hook。
- ⚠ 未編譯；網頁 `node --check` 通過，沒在瀏覽器看過。

## P3 接線（共用檔；20260927，St02；github-59 GO，FROM_STEVEN §1 `7901a82f`）

- `tools/wb_serve.cpp`（行數不變，都接在既有行上）：
  - :439 全域宣告 `W906_ElaHttp`（接在 `W906_TesterCommHttp` 後面；:2867 在 `ht9045::formjson::` 的匿名 namespace 裡，宣告不能放那裡）。
  - :2867 路由：H1b（`/api/testercomm`）後面、`return false;` 前面呼叫 `::W906_ElaHttp`；POST 看 `g_tcAllowCmd`（`--allow-cmd`）。
  - :4165 開機：`W906_CreateLogObjects()`／`MyDBUpdateDB()` 之後 `W906_ElaStart()`，再裝 `W906_ElaPostHook = &W906_ElaPost`。
  - :5967 關機：`W906_DestroyLogObjects()` 之後先清 hook，再 `W906_ElaStop()`（join worker）。
- `CMakeLists.txt` :3421：wb_serve 連 `ht9045_ela`（跟 testercomm 一樣排在 RESCAN 群組前面；機台庫不回頭參照它）。
- `Interface/InterfaceSYS.cpp`（+4 行）：:552-553 新增 `void (*W906_ElaPostHook)(int) = 0;`；`SendCommand_EventLog` 在 `bUseMDB==false` 區塊最前面呼叫 hook。
  - **golden 的 FindWindow／WM_COPYDATA 那段保持原樣、照常執行**（github-59 裁示）：機台上可能還裝著舊的 EventlogAnalyzer.exe，
    它做 ElaHub 沒翻的六個工作（FTP／客戶報表，HOLD）；把那段關掉等於默默停掉它們。exe 要不要退場是 Jimmy 的決定。
  - 兩邊並存時同一個指令 Hub 和 exe 各收一次；兩邊都會跑 ReadConfig，補寫的預設值一樣。
  - 為什麼用 hook：InterfaceSYS.cpp 在 `ht9045_sm`（機台庫），ctest 連它時沒有 `ht9045_ela`。只有 wb_serve 開機會裝；其他執行檔 hook 是 NULL，行為跟接線前一樣。
- **⚠ #17 ELA-3（暫照 golden，A）：wb_serve 開機後 worker 的第一件工作就是 ReadConfig——把缺的 key 寫進真的 `D:\HT9045\config\config.ini`；
  `D:\HT9045\system\Gerneral.ini` 沒有 Machine ID 也會寫；開機排的「今天」查詢也照 golden 把缺的 unit／碼寫進 `D:\HT9045\Error\English\JAM0000.dat`。**
  `HT9045_ELA=0` 整個關掉（沒有 Hub，`/api/ela` 回 503，`SendCommand_EventLog` 只走外部 exe 那條）。沒有 ctest 會啟動 wb_serve（`tests/CMakeLists.txt` 查過）。
- V906 目前唯一會送指令的是 `cprod.cpp:3315`（存設定後 `EL_UPDATE_PARAMETER`）。
- 選單入口（eventlog.html）HOLD：先把 nav 檔與行號給 github-59，它查過擁有者再說。
- ⚠ 未編譯（本機沒有編譯器）；等筆電／St01 建置；沒在 SIM 跑過。

## P4 輪詢減量（20260927，St02；只動自己的檔）

- `GET /api/ela?since=<seq>`：seq 等於目前的 seq 時只回 `{"seq":N,"busy":b,"unchanged":true}`，不搬整份快照（每個表最多 5,000 列）。
  seq 在每個工作（EL_* 或查詢）做完都 +1，所以 seq 沒變＝history／config／result 都沒變，只有 busy 可能不同（工作開始時）。
- `eventlog.html` 第一次不帶 since 拿全份，之後都帶 `since=<上次的 seq>`；`unchanged` 時只更新狀態燈。
- 為什麼現在做：選單入口若照一般視窗登記，iframe 開機就載入，隱藏時也每秒輪詢；沒有這個，每秒都在搬幾 MB 的 JSON。
- ⚠ 未編譯；頁面 `node --check` 通過。

## P4 視窗登記（20260927，St02；github-59 GO，FROM_STEVEN §1 `9dcf86c0`）

- `web/background.html` WINDOWS 表在 :510 aoainfo 與 :511 styleguide 之間插兩行（:511 `eventlog`、:512 `testercomm`），`form:null`、hidden＋lazy。
  - golden 的分析器（TfrmELA）和 GPIB 橋接（H9046_32GPIB）都是另一支 exe 的視窗，Handler 沒有選單項，所以只給工作列按鈕；main.html 不加選單。
  - lazy：沿用 pci1203 的機制（:482、openWin :751），開了才載入頁面，也才開始每秒輪詢 `/api/ela`（輪詢本身已經減量，見上一節）。
  - owner 查過（github-59）：TO_STEVEN §1 與 St01 都沒登記這個檔；Q42 可能在 :484 MODAL_POLICY 加 observer，在插入點上方，不受影響。
- 待裁決 #31（G8）：V906 要不要照 golden 啟動外部 EventlogAnalyzer.exe（WakeupEventLogSaver）。建議 A 維持關閉。

## P3 ctest ELA_Service 與路徑 seam（20260927，St02；只動自己的檔，github-59 同意）

- `EventLogAnalysis/ElaService.h`（新）：`ela::ServeHttp(Hub*, const QueryRequest& base, ...)`＝原本 `W906_ElaHttp` 的本體，Hub 由呼叫端給；
  `W906_ElaHttp` 用正式的 Hub 呼叫它（先看路徑前綴，別的請求不建任何東西）。`base` 帶頁面不送的 eventLogDir／prodLogDir／handlerId。
- `ElaHub.cpp`：`QueryRequest` 與 `HubPaths` 的預設值改走 Handler 已有的 seam（沒設或空＝golden 字面值，行為不變）：
  `W906_EVENTLOG_ROOT`（cObserver.cpp :1304，Handler 寫、檢視頁讀的同一個 EventLogTxt）、`W906_PRODLOG_ROOT`（common.cpp :253）、
  `W906_GENERAL_INI_PATH`（common.cpp :155）。config.ini 沒有 seam，ctest 注入 HubPaths。
- ctest `ELA_Service`（`tests/test_ela_service.cpp`）：
  - 0 先把三個 seam 與 D5 那一對（W906_HT9045LOG_ROOT／W906_SAVEEVENTLOG_ROOT）用 CRT `_putenv` 指到自己的 `%TEMP%\ht9045_ela_service_<tick>`；也驗沒設時是 golden 字面值；
  - 1 非 /api/ela 路徑不接；2 沒有 Hub 回 503，`HT9045_ELA=0` 時 `W906_ElaStart` 不建 Hub（有先確認 getenv 讀得到才呼叫）；
  - 3 GET 快照、`since` 相同回短答、不同回全份；4 403／400／404，都不排查詢；
  - 5 URL 編碼的日期進 SetRange、handlerId 來自 base、`area=1` 只剩 01 Input Arm（byAreaCount 1）、空的 area＝全部（新 Hub，因為 #18 A 同一個 Hub 會累積）；
  - 6 真的 `D:\HT9045\config\config.ini`、`D:\HT9045\Error\English\JAM0000.dat`、`D:\HT9045\system\Gerneral.ini`、`D:\HT9045_Log\EventLogTxt` 的存在／大小／修改時間前後一樣；
  - 全綠才刪沙盒，有失敗就留著看。
- `tests/CMakeLists.txt`（我的區段，ELA_Hub 後面）：CMake 層的第二道是 `cmake_language(DEFER CALL set_property ... APPEND ENVIRONMENT)`——
  下面的 MACHINE-DATA CONTAINMENT 區塊會蓋掉當時已存在的每個測試的 ENVIRONMENT，立刻設會被蓋掉。
  不放 `_ht9045_env_extra`：那個清單會加到每一個測試，`W906_GENERAL_INI_PATH` 放進去會把別的測試的 Gerneral.ini 也移走。
- ⚠ 未編譯（本機沒有編譯器）。
