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
| file match | path contains `FN1="yyyymmdd.csv"` AND `FN2="EventLogTxt_"`; CC 915/919 (VTEST, Gerneral.ini `[System] CUSTOMER_CODE`): `FN1="yyyymmdd"`, `FN2="_EventLogTxt_"`. Case-sensitive substring. **Every** matching file is read (no de-dup). **W19 B (V906 default): by the time each name covers, once, rows de-duplicated -- see the end** | :807-821, :3139 |
| read | `TStringList::LoadFromFile` (bytes as ANSI = cp950 on the machines, no conversion; CR/LF split) ; line 0 skipped as header | :829-830 |
| split | `tsRow->CommaText = line` (BCB6: delimiters are ',' **and unquoted blanks**; quotes honoured, `""` escape; leading blanks skipped). **W15 B (V906 default): `ela::SplitEventLogCsv`, see the W15／W18／W19 section at the end** | :833 |
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
| JAM0000.dat | `D:\HT9045\Error\English\JAM0000.dat`, section = UnitName, keys `<code>` (level), `<code> IncludeMTBA`, `<code> IncludeMTBF`; missing keys are **written** with the default. AI(W906-SEC-S128) 20260927: the check-then-write runs under the named mutex `Local\HT9045_JAM0000_dat` shared with the security.jam page (ElaCore.cpp JamFileLock; JamIniMerge.h on v906/steven-st02-on-cbridge); not acquired in 5 s -> read without write-back that time | Common.cpp:124-139 |

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
| M2 | **Tokenizer**: BCB6 CommaText also splits on unquoted blanks; V906 `vclcompat` CommaText splits on commas only (`vclcompat/TStringList.cpp:188-226`); V912 cObserver uses a quote-aware comma-only `ParseEventLogLine` (V906 `cObserver.cpp:1351-1393`), the Rev891 analyzer does not. Real files contain unquoted `,16 System,` rows (local sample: 2025-09 154, 2025-11 24, 2025-12 10 rows with an unquoted numbered UnitName) that BCB splits into two or more fields, shifting AlarmCode to "System" -> not counted. | ElaCore on vclcompat CommaText will differ from the BCB oracle on those rows (decision Q1) -> **W15 B (Steven 20260927)**: ElaCore uses the viewer's splitter (`EventLogCsv.h`) |
| M3 | vclcompat `parseDelimited` stops (`break`) when a closing quote is not immediately followed by ','; BCB continues with the next token | rare field loss in V906 only |
| M4 | quoted TAB placeholder `"\t"`: BCB keeps TAB; ~~V912 viewer splitter Trim()s to ""~~ **corrected 20260927 (W15): the V912 viewer keeps the TAB too** -- text inside quotes is not trimmed | display only |
| M5 | file match needs `yyyymmdd.csv`: hour / 12 h / month / `_RT` / `_FT` / `_TesterOffline` files are silently skipped (VTEST 915/919 accepts hour names, never month names) | silent zero counts when O15/N10 change the save period -> **W19 B**: every save period is read |
| M6 | `AllEventLog\<n>_yyyymmdd.csv` copy is read in addition to `yyyy\mm\<n>_yyyymmdd.csv` (sample 2025-08-25) | double counting -> **W19 B**: a row counted from another file is skipped (`dupRowsSkipped`) |
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
| G1 | `_MySummaryStruct::InitData` does not reset `iFailCount` -> `MySummary.iFailCount` (Fail Count, total MTBF, ChipAdvanced "OEE") accumulates over every query of one exe session, **including the automatic start-up query** | Analyzer.cpp:111-126 | oracle: run the BCB exe with a log root whose "today" is empty, one query per start; port: reset (record as deviation) -> **W18 B: reset** |
| G2 | `ListByUnitName[]` / `ListByFunction[]` are never cleared -> "By Filter" rows accumulate over queries | :994-1034, :1995-2059 | same -> **W18 B: cleared per query** |
| G3 | O06-4 without the time-period option saves the whole-day summary **every minute** (`SystemNN % 1 == 0`) | :3014-3040 | W18 B: once an hour; now **★W43 = C (Steven 0928)**: without [O06-8] no timed save, with it every 10 / 30 min (R5 "★W43") |
| G4 | "Top 5" lists every code (sorted count desc, code asc) | :2061-2203 | keep; web can cut |
| G5 | UPH(day)=input/24, UPH(hour)=input, total UPH=input/(days*24); MTBF/MTBA use the calendar period (<= 1 event -> whole period; input 0 -> 0) | :1787-1947, Common.cpp:518-576 | keep |
| G6 | range inclusive for EventLog, exclusive for Production_Log; first Production_Log header counted then `iInputCount--` | :850, :2385, :2360-2367, :2456 | keep |
| G7 | JAM/WAR/MES test differs: stop filter = contains, JAM/WAR/MES counters = prefix, ChipMos Jam Rate = contains | :857-985, uChipMosZHUBEI_Func.cpp:42 | keep |
| G8 | ChangeLog/ParameterLog block never matches and has an out-of-bounds loop | :1039-1162 | drop |
| G9 | crashes on empty data: `FindStartLotFromRecVec` (`size_t i=size-1`, :216), `SaveJamSummaryFor30Days` (`tIntervalRecs[size-1]`) | uChipAdvancedFunc.cpp:216, uChipMosZHUBEI_Func.cpp:95 | guard (deviation) |
| G10 | analyzer writes Handler files: default keys into JAM0000.dat for every new (unit, code) and into config.ini on every ReadConfig; the Jam editor rewrites JAM0000.dat and Error\<lang>\<code>.dat | Common.cpp:72-183, :2756-2797 | decision Q3 |
| G11 | function filter check boxes are wired to the wrong buckets (Temperature box shows ESD rows, 2D shows Temp, OCR shows 2DID, ESD shows OCR) | :257-263 | decide keep / fix -> **W18 B: fixed in eventlog.html** |
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

1. **[decided 20260927: B = W15]** **Parser (M2)**: ElaCore follows BCB6 CommaText exactly (blank-splitting; loses the unquoted `16 System` rows, matches the BCB oracle) or uses the V912 quote-aware comma-only `ParseEventLogLine` already used by cObserver (one parser for both pages; deliberate deviation from Rev891)?
2. **SPIL (M1)**: keep golden "not analysed", or add a SPIL field mapping (new behaviour)?
3. **Write-backs (G10)**: may the V906 analyzer write defaults into JAM0000.dat / config.ini like golden, or must ElaCore be read-only on Handler files?
4. **[decided 20260927: B = W18]** **Known golden bugs (G1, G2, G3, G11)**: keep for oracle equality or fix and list as deviations? (Suggested: fix G1/G2, oracle taken from a fresh exe per query.)
5. **[decided 20260927: B = W19]** **File selection (M5, M6)**: faithful (skip hour/month files, double-count AllEventLog) or de-duplicate and accept the other SaveType names?
6. **[decided: W20 merge (Steven 20260927); ★W45 ruled Steven 0928 item 18 -- 18a A / 18b B / 18c A, others 照 golden, 未明確裁決; section "★W45（原 W20）Steven 0928 裁決後"]** **Jam Code Setting editor**: port to eventlog.html, or leave Jam level / IncludeMTBA / IncludeMTBF editing to the Handler's own screens?
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
- **暫照 golden（A），待使用者確認**（progress-st02 #15～#19），都有 `ela::Options` 開關（→ 20260927 已裁決：#15 B、#16 A、#17 A、#18 B、#19 B；程式見檔尾「W15／W18／W19＝B」）：
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
- 照留的 golden 行為（記錄，要改就是 #18 的 B）（→ W18＝B 20260927：除了「:」＋訊息與 Top5 尾端空白列，其餘都改成偏離，`keepGoldenBugs=true` 才照 golden；見檔尾）：
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
  - `W906_ElaHttp()`：`GET /api/ela`（快照）、`POST /api/ela/query?from=&to=&top=&filter=&area=&func=&row=`（排查詢；`allowCmd` 參數是 false 才回 403。⚠ 20260927 更正：wb_serve 傳的 `g_tcAllowCmd` 恆為 true，見檔尾「allowCmd 更正」）。
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
  - :2867 路由：H1b（`/api/testercomm`）後面、`return false;` 前面呼叫 `::W906_ElaHttp`；POST 看 `g_tcAllowCmd`（＝`allowCmd`，自 ZEROARG 起恆為 true，見檔尾「allowCmd 更正」）。
  - :4165 開機：`W906_CreateLogObjects()`／`MyDBUpdateDB()` 之後 `W906_ElaStart()`，再裝 `W906_ElaPostHook = &W906_ElaPost`。
  - :5967 關機：`W906_DestroyLogObjects()` 之後先清 hook，再 `W906_ElaStop()`（join worker）。
- `CMakeLists.txt` :3421：wb_serve 連 `ht9045_ela`（跟 testercomm 一樣排在 RESCAN 群組前面；機台庫不回頭參照它）。
- `Interface/InterfaceSYS.cpp`（+4 行）：:552-553 新增 `void (*W906_ElaPostHook)(int) = 0;`；`SendCommand_EventLog` 在 `bUseMDB==false` 區塊最前面呼叫 hook。
  - **golden 的 FindWindow／WM_COPYDATA 那段保持原樣、照常執行**（github-59 裁示）：機台上可能還裝著舊的 EventlogAnalyzer.exe，
    它做 ElaHub 沒翻的六個工作（FTP／客戶報表，HOLD）；把那段關掉等於默默停掉它們。#31＝A（使用者 20260927）：V906 不啟動舊 exe，這段只給手動開的 exe。
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
- ~~待裁決 #31（G8）~~ → **已裁決 A（使用者 20260927）**：V906 不照 golden 啟動外部 EventlogAnalyzer.exe（WakeupEventLogSaver）。

## P3 ctest ELA_Service 與路徑 seam（20260927，St02；只動自己的檔，github-59 同意）

- `EventLogAnalysis/ElaService.h`（新）：`ela::ServeHttp(Hub*, const QueryRequest& base, ...)`＝原本 `W906_ElaHttp` 的本體，Hub 由呼叫端給；
  `W906_ElaHttp` 用正式的 Hub 呼叫它（先看路徑前綴，別的請求不建任何東西）。`base` 帶頁面不送的 eventLogDir／prodLogDir／handlerId。
- `ElaHub.cpp`：`QueryRequest` 與 `HubPaths` 的預設值改走 Handler 已有的 seam（沒設或空＝golden 字面值，行為不變）：
  `W906_EVENTLOG_ROOT`（cObserver.cpp :1304，Handler 寫、檢視頁讀的同一個 EventLogTxt）、`W906_PRODLOG_ROOT`（common.cpp :253）、
  `W906_GENERAL_INI_PATH`（common.cpp :155）。config.ini 沒有 seam，ctest 注入 HubPaths。（20260928 更新：config.ini 已接 Handler 的 `W906_AUTH_PATH`＝`<W906_AUTH_PATH>config.ini`，`ElaHub.h` `ElaConfigIniPath`；沒設＝golden 字面值）
- ctest `ELA_Service`（`tests/test_ela_service.cpp`）：
  - 0 先把三個 seam 與 D5 那一對（W906_HT9045LOG_ROOT／W906_SAVEEVENTLOG_ROOT）用 CRT `_putenv` 指到自己的 `%TEMP%\ht9045_ela_service_<tick>`；也驗沒設時是 golden 字面值；
  - 1 非 /api/ela 路徑不接；2 沒有 Hub 回 503，`HT9045_ELA=0` 時 `W906_ElaStart` 不建 Hub（有先確認 getenv 讀得到才呼叫）；
  - 3 GET 快照、`since` 相同回短答、不同回全份；4 403（測試自己傳 allowCmd=false）／400／404，都不排查詢；
  - 5 URL 編碼的日期進 SetRange、handlerId 來自 base、`area=1` 只剩 01 Input Arm（byAreaCount 1）、空的 area＝全部（新 Hub，因為 #18 A 同一個 Hub 會累積）；
  - 6 真的 `D:\HT9045\config\config.ini`、`D:\HT9045\Error\English\JAM0000.dat`、`D:\HT9045\system\Gerneral.ini`、`D:\HT9045_Log\EventLogTxt` 的存在／大小／修改時間前後一樣；
  - 全綠才刪沙盒，有失敗就留著看。
- `tests/CMakeLists.txt`（我的區段，ELA_Hub 後面）：CMake 層的第二道是 `cmake_language(DEFER CALL set_property ... APPEND ENVIRONMENT)`——
  下面的 MACHINE-DATA CONTAINMENT 區塊會蓋掉當時已存在的每個測試的 ENVIRONMENT，立刻設會被蓋掉。
  不放 `_ht9045_env_extra`：那個清單會加到每一個測試，`W906_GENERAL_INI_PATH` 放進去會把別的測試的 Gerneral.ini 也移走。
- ⚠ 未編譯（本機沒有編譯器）。

## allowCmd 更正與 SIM 驗收（20260927，St02；St01 04:35 指出，github-59 轉達）

- **更正**：之前寫的「`POST /api/ela/query` 沒帶 `--allow-cmd` 回 403」是錯的（3e4ce1de 的交件列、本帳本 P3 接線節；commit 訊息不改寫）。
  - wb_serve 自 ZEROARG 起 wb_serve.cpp:3670 `bool allowCmd = true;`（AI(W906-ZEROARG) 20260918） 是預設，**沒有任何參數會把它關掉**：
    `--allow-cmd`（:3694）只是再設一次 true，說明文字寫「accepted; already the default」（:3711），:4549 的註解也寫「恆為 true」。
  - :4351 `server.SetReadOnly(!allowCmd);  g_tcAllowCmd = allowCmd;` ⇒ `W906_ElaHttp` 收到的 `allowCmd` 在 wb_serve 永遠是 true，
    `ServeHttp` 的 403 分支在 wb_serve 走不到；只有呼叫端自己傳 false（ctest `ELA_Service`）才看得到 403。
  - 同一件事也適用 `/api/testercomm` 的 POST（TesterCommWiring.cpp:207，H2）。wb_serve.cpp :2836／:4351 那兩行註解
    「follows --allow-cmd」「only with --allow-cmd」是 H1a／H2 寫的過期說法，行本身是共用檔，要改另外認領。
- **SIM 驗收時實際會看到的**：
  - 開機印 `[ELA] Event Log Analyzer hub started ...`；`GET /api/ela` 回 200 JSON。
  - `POST /api/ela/query` 沒帶 from／to（或日期不合法）回 **400**；帶了回 **202**，接著 worker **真的跑查詢**——
    照 golden（#17 A）會把缺的 unit／碼寫進真的 `D:\HT9045\Error\English\JAM0000.dat`。
  - 開機那一次 ReadConfig 也照 golden 寫真的 `D:\HT9045\config\config.ini`（缺的 key）與 `Gerneral.ini`（沒有 Machine ID 時）。
  - 要一個完全不寫的 SIM：環境變數 `HT9045_ELA=0`（沒有 Hub，`/api/ela` 回 503）。
- ~~誰來跑、在哪跑是待裁決 #33~~ → **已解決**：筆電 20260927 05:2x 在自己的機台跑了（備份→驗證→還原；TO_STEVEN §4，main `231fffa6`），不用裁決。
  - ✅ 開機印 `[ELA] Event Log Analyzer hub started ...`。
  - ✅ `GET /api/ela` 三次都 200；history＝`EL_UPDATE_PARAMETER: ReadConfig done` ＋ 開機查詢「今天..今天：0 個 event log 檔」。
  - ✅ `HT9045_ELA=0` → 503。
  - ✅ ELA 開與關兩次跑完，被碰到的真檔**逐位元組相同**：ELA 沒有多寫任何東西——那台的 config.ini 本來就有全部的 key，
    那天沒有 EventLog 所以沒有建立 JAM0000.dat。
  - ⚠ **還沒驗**：`POST /api/ela/query`（有真 EventLog 的那一天）與它照 golden 對 JAM0000.dat 的寫回；
    config.ini 缺 key 時的寫回也還沒在真機檔上看過（ctest `ELA_Hub` 在沙盒驗過 38 個）。
- 建置：St01 用兩組態建 `0d4811dc`，7／7 綠、真檔 0 差異；筆電全量 gate（到 `3ce47956`）ELA_Core 65／65、ELA_Hub 15／15、ELA_Service 30／30。
- `eventlog.html`（E-005）還要瀏覽器看過並跟 BCB6 分析器畫面比對（St01），狀態維持 IMPLEMENTED。

## #31 G8 裁決：V906 不啟動舊的 EventlogAnalyzer.exe（使用者 20260927，選 A；FROM_STEVEN 5dea6828）

- `TesterComm/Handler/HandlerBridgeCtl.cpp:259` 的 G8（golden 912 main.cpp:18447-18490，WakeupEventLogSaver 殺掉／重開 exe）**永久 `#if 0`**，
  註解從 TODO 改成 CLOSED；`WakeupEventLogSaver` 不翻。
- V906 的分析器只有行程內的 **ElaHub**（`EventLogAnalysis/`，wb_serve 開機啟動）。
- `Interface/InterfaceSYS.cpp` `SendCommand_EventLog` 的 golden FindWindow／WM_COPYDATA 照送：只給**手動開的**舊 exe（它仍能做 V906 沒翻的六個報表／上傳工作）。
- 那六個工作（`EL_UPLOAD_JAMWEEK`、`EL_UPLOAD_CHIPADV_LOTEND`、`EL_UPLOAD_SUMMARY`、`EL_UPLOAD_EVENTLOG`、`EL_UPLOAD_BYFILE_N10`、`EL_VTEST_MTBF_SUM`）仍等 #22（計畫 §5，Jimmy）。

## W15／W18／W19＝B（20260927，St02-E；Steven 裁決，計畫 `D:\HT9045\.claude\skills\ht9045-st02-workflow\references\research-ela-w15-w18-w19.md`）

分支 `v906/steven-ela-wip`：`1042d4cc`（W15）、`2ceca61c`（W18）、`82196ca3`（W19）。兩組態只編譯（St02 不執行）；ctest 待 St01。

- **開關**（`ela::Options`，`EventLogAnalysis/ElaCore.h`）：預設＝裁決；每個都能切回 golden，給 #23 參考 exe 對照（oracle）用。
  - `bcbCommaText`（#15）：false＝`ela::SplitEventLogCsv`；true＝BCB6 CommaText。
  - `keepGoldenBugs`（#18，原名 `keepGoldenLeaks`）：false＝修掉；true＝golden 的行為。
  - `goldenFileRule`（#19，新）：false＝依檔名涵蓋的時間選檔＋跨檔列去重；true＝golden 每天的檔名子字串比對。
  - `goldenFileRuleVtest`（**★W38＝B，Steven 0928**，20260928 新）：true（預設）＝VTEST 915／919 機台一律照 golden 的檔名規則；false＝只看 `goldenFileRule`。見下「W19 偏離」VTEST 那一項。
  - `jamWriteBack`（#17＝A）不變。正式的 Hub（`ElaService.cpp` `W906_ElaStart`）用預設＝裁決。
- **W15 偏離（切欄）**
  - 新檔 `EventLogAnalysis/EventLogCsv.h`（header-only）的 `ela::SplitEventLogCsv`＝V906 `cObserver.cpp:1351-1385` 那兩個 helper
    （golden 912 `cObserver.cpp:3812-3846`）：引號外的逗號切、每個 `"` 切換；欄位 Trim 後**開頭是引號**才去掉頭尾引號並把 `""` 變 `"`；
    引號內的字原樣（`"\t"` 仍是 TAB、`" 08:45:33.712"` 保留空白）；沒關的引號到行尾；結尾逗號多一個空欄；逐 byte（cp950 安全）。
  - golden Rev891 `Analyzer.cpp:833`（與 906_0625_Steven `cObserver.cpp:3848`／`:3892`／`:3927`）用 BCB6 CommaText，未加引號的空白也切，
    `,16 System,` 那種行會切壞不算。樣本 (a) 2025-09-10：golden 123 stop 列／0 s／MES 115 → 131／309 s／123。
  - 原本的 `CommaOnlyText` 拿掉：它留下 `, Process` 的前導空白，`!= "Process"` 與 mapUnitName 查表都會壞。
  - 為什麼 header-only：cObserver.cpp 在 `ht9045_sm`，sm 從不連 `ht9045_ela`（`ht9045_ela` 只連 vclcompat＋nmftp）。
  - **cObserver.cpp 已改**（共用檔，認領 FROM_STEVEN §1 `0c55fd18` 核准；`a9b93d61`，行數不變 7779→7779，就是交給 St02-M 的那份 diff 25／25 行）：
    `:110` 空行改 `#include "EventLogAnalysis/EventLogCsv.h"`、`:1351-1385` 兩個 helper 改呼叫 `ela::SplitEventLogCsv`；
    `ParseEventLogLine`（`:1387-1393`）的 SPIL 分支照 golden（W16＝A），三個呼叫處（:1444／:1488／:1523）不動。
    ctest `ELA_Core` 第 9 節：W15 以前的兩個本體（`namespace viewer`，逐字）當參考，現在的兩個本體（`namespace viewer_now`，逐字編進測試）
    與 `SplitEventLogCsv` 在每種形狀、每一行樣本上都要跟參考一樣；而且測試讀 `cObserver.cpp` 原始碼確認 `viewer_now` 就是 `:1369-1385` 的字
    （`W906_ELA_SRC_DIR`，唯讀），檔案改了測試就會失敗。
  - **M4 更正**：上面 §5.1 M4「V912 viewer splitter Trim()s to ""」寫錯了——引號內的字不 trim，V912 檢視頁也保留 TAB。
- **W18 偏離**（`keepGoldenBugs=false`；行號 Rev891 `Analyzer.cpp`）
  1. `iFailCount` 每次查詢歸零（golden `InitData` :111-126 漏了；G1）。Hub 一個 Analyzer 用整個生命週期，所以開機那次也不再往上加。
  2. By-Unit／By-Function 清單每次查詢清掉（G2）。
  3. by-hour 的測試時間用 `dtTestTime × OneDayMS`（golden :1899 用 OneHourMS，小 24 倍）；每小時 MTBF／MTBA 的期間仍是 OneHourMS。
  4. By Filter 只有表格真的空才寫 "No Record!!"（golden :2050 `RowCount==2` 會蓋掉唯一一筆的時間）。
  5. Top5Filter 每次重建、只標彙總列、重算列數（golden 沒重查就換篩選會留舊列，而且每個事件列都寫 "Top n"）。
  6. Top5 明細 AlarmCode 完全相同才算（golden :2218 前綴比對，WAR1520 會吃到 WAR15206）；`web/page/eventlog.html` 同步。
  7. `ListProductionLog`：打不開／空的檔、壞行（寫到一半的最後一行）跳過，每個檔記一行 exceptions，表格與摘要照樣更新、回 true（golden 例外直接離開）。
  8. `web/page/eventlog.html` 功能篩選框各管自己的清單（golden ctor :257-263 交叉了四個；G11），框名照 `Analyzer.dfm:1161-1210`。
  - 留著（兩種模式都一樣）：":"＋訊息的例外文字、Top5 尾端空白列（不存）、"31 Cylinder" 沒有勾選框；G5／G7／G12／G13（規則或客戶檔名）。
  - G3（O06 每分鐘存）與 G9（空資料當掉）屬 ElaReports（R2），移植時修。
- **W19 偏離（選檔＋去重）**
  - `ela::EventLogFileSpan`：路徑仍要含 golden 的 `EventLogTxt_`；檔名結尾＋`.csv`（大小寫不拘）決定涵蓋時間：
    日檔（含 `_RT`／`_FT`／`_TesterOffline`）[D, D+1)；`D HH`（含 `_RT`／`_FT`）[D+HH, D+1)；
    `D HHNNSS`（行數上限）[同一個 base 名前一個檔的時間, 下一個檔的時間)，第一個往前開放、最後一個往後開放；
    `DHH00` [D+HH, D+HH+12 h)（2000 那個檔跨到隔天 08:00）；月／年檔＝整月／整年。跟查詢區間重疊就讀；每列仍照 golden 精確比對區間。
    ByLot／JamStat／SGJamCount RawData／固定檔名／分鐘檔（.TXT）都不會對上。
  - 每次查詢每個檔只讀一次：主資料夾先、AllEventLog 後，各依涵蓋起點排序（golden 每天×每檔：月檔被跳過、AllEventLog 當天讀兩次）。
  - 列去重：會計入的列用 `BcbGetCommaText(row)` 當 key，同一個 key 已經從**另一個**檔計過就跳過（`dupRowsSkipped`；快照 `dupRowsSkipped`、
    工作歷史、eventlog.html 的 Log 分頁都看得到）；同一個檔內的重複照算（golden；Duplicate 欄才是操作員重複的過濾）。
  - **VTEST：★W38＝B，Steven 0928（客戶指定功能固定格式）**（`bf7cb8e3`，AI(W906-ELA-W38) 20260928，St02-E helper）。Steven：「大部份客戶指定功能，都是固定格式，不可以亂更動」。
    VTEST 機台（Gerneral.ini `[System] CUSTOMER_CODE` 字串＝"915"／"919"，golden 原樣比對、不去空白）**預設照 golden 的固定檔名**：
    `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:807-811`（`<yyyymmdd>`＋`_EventLogTxt_`，每天一輪）——一般的 `EventLogTxt_yyyymmdd.csv` 不讀、不跨檔去重。
    開關 `ela::Options::goldenFileRuleVtest`（預設 true）；`Analyzer::UsesGoldenFileRule()`＝`goldenFileRule || (goldenFileRuleVtest && custCode 是 "915"／"919")`；false＝回到 W19 B（VTEST 也讀一般日檔＋去重）。其他客戶碼預設仍 W19 B。
    範圍：`GetEventLogText`（每週 MTBF 報表 `DoVTestSaveSummary`、頁面查詢、Save Summary 都走它）與 `GetEventLogTextToVec`（golden 那個函式沒有 VTEST 分支＝一般檔名；呼叫端只有 851／868 的報表，VTEST 機台不會跑）。
    ctest `ELA_Core` 第 10 節（預設含 `goldenFileRuleVtest`）、第 11 節 VTEST（golden／預設／`goldenFileRuleVtest=false` × 919／915／000，加 `SetCustCode("919")`；預設在 919／915 由 2 檔 30 s 改成 1 檔 20 s）。
    （原本：預設模式在 VTEST 機台也讀一般日檔＝待 Steven 確認。）
    ⚠ 未上機。上機要看：偉測（915／919）機台 O10 與 O19 開，每週 `<asO19_SavePath>\<Machine ID>-SummaryData_<日期>.txt` 的數字只來自檔名含 `_EventLogTxt_`＋那天日期的檔
    （跟舊分析器同一週的報表逐項一樣）；eventlog.html 在那台查詢時 Log 分頁 `dupRowsSkipped`＝0、讀的檔只有那些。
  - 樣本：(b) 2025-08-25＋AllEventLog 複本 golden 80 stop 列 → 40、`dupRowsSkipped` 40；(c) 12 h 2024-07-01～02 golden 0 → 278 列／510 s（JAM 37），
    其中 07-02 00:00～07:59 的 116 列／441 s 在 07-01 那個 2000 檔；小時檔 2023-09-01 golden 0 → 66；月檔 2025-11-06～07 golden 0 → 11（只讀一次）。
  - 行數上限檔（`HTSaveType` TByMaxLineCount，檔名 `Public/MyStringList.cpp:789`／AllEventLog `:589`）的檔名只有一個時間、沒有結束。
    St02-M 20260927 定（W19＝B 內的設計細節，不另裁決）：不用固定兩天；同一個 base 名的檔是連續的，最後一個開放到查詢結束。
    查 writer 時發現：Handler 是在**寫出緩衝時**才決定檔名（`AddTextWithDateTime` `:431-436` → `MySaveToFile` → `GetFileName`），
    所以檔名時間是那批列的**結束**、列都在它之前；St02-M 的說法是把它當開始。`ela::SelectEventLogFiles` 兩種都涵蓋：
    每個檔＝[同 base 名前一個檔的時間, 下一個檔的時間)，第一個往前開放、最後一個往後開放（多讀的檔各列仍逐列比對區間，不會多算）。
    ctest 第 11 節：同 base 三個檔（04-20 08:00／04-21 12:00／04-24 07:00），列照 Handler 的寫法放在檔名時間之前，另加一列在最後一個檔名之後；
    04-21 00:00～11:00 讀前兩個、算到第二個檔 04-21 10:00 那列；04-27 只讀最後一個、它的 04-27 那列有算。
    （Handler 的 EventLogTxt 實際上不用這個存檔方式：`cprod.cpp:2538-2568` 只設 Hour～12 Hour／Day／Month。）
- **ctest `ELA_Core`**（`tests/test_ela_core.cpp`）
  - 第 3～8 節明確設 golden 的三個開關（oracle 回歸，原斷言都沒改）；第 8 節多釘兩個 golden 行為（Top5 前綴比對、Production_Log 壞行例外離開）。
  - 第 9 節 W15：真實的列形狀、V906 檢視頁的兩個 helper 逐字抄在 vclcompat 上當參考（每種形狀、每一行樣本都要一樣）、
    `BcbGetCommaText`→`BcbCommaText` 來回、樣本 (a) 兩種切法。
  - 第 10 節：第 3～8 節的檔用預設（裁決）跑，斷言計畫 §4 的翻轉值。
  - 第 11 節 W19：每種檔名的涵蓋時間與不是 event log 的檔、同檔重複照算／跨檔跳過、樣本 (b)(c)、小時檔、月檔，每個查詢也用 golden 模式跑。
  - Fixture：`tests/fixtures/ela/EventLogTxt/` 是從 `D:\HT9045_Log\EventLogTxt`（唯讀）**複製**的 7 個檔；248 KB 的 2000 檔裁成第 0～201 行＋第 2480 行到尾
    （20:00 起、跨午夜、07:30～07:50）。測試先複製到 `%TEMP%\ht9045_ela_core_fx\EventLogTxt\<真實的 yyyy\mm\dd>` 再讀；AllEventLog 複本在測試裡去 CR 產生
    （同 `Public/MyStringList.cpp:599`），ByLot／JamStat／RawData 誘餌也在測試裡產生。CMake：`W906_ELA_FIXTURE_DIR`（`tests/CMakeLists.txt` ELA 區段）。
  - 期望值先用 Python 鏡像（同一套切法、選檔、去重）算過，重現計畫 §4 的數字；來回測試 3,144 行 0 失敗。
  - 沒改斷言：`test_ela_hub.cpp`（`""A""` 那列兩種切法一樣）、`test_ela_service.cpp`（只改 :241 過期註解）、`test_observer_core.cpp`（ObserverCore）。
- ⚠ 未上機。上機要看：有 `16 System` 未加引號行的那天，分析頁的計數；按小時／12 小時／月存檔的機台（以前是 0）；有 AllEventLog 的機台不重算；
  By Function 各框；Top5 明細；第二次查詢 Fail Count 不再變大；By Hour 的 Total Test Time（會比 BCB exe 大 24 倍，這是修正）。

## R1／R2：ElaFileUtil、GetEventLogTextToVec、ElaReports（20260927，St02-E；計畫 `D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md` §4，#22 D-a～D-f 已裁決）

分支 `v906/steven-ela-wip`：`99dc54ba`（R1）、`1b6bf39e`（R2）。兩組態只編譯（St02 不執行）；ctest `ELA_Reports` 待 St01。行號＝Rev891（`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code`）。

- **R1 ElaCore**（`EventLogAnalysis/ElaCore.*`）
  - `LogRecord`（Common.h:92-104）、`GetEventLogTextToVec` 兩個形式（Analyzer.cpp:1183-1267）、`GetStartEndGap`（Common.cpp:511-516）。
  - 跟 `GetEventLogText` **同一個 `ela::Options`**：W15 切法（`Split`）、W19 選檔（`SelectEventLogFiles`）＋跨檔去重（`vecDupRowsSkipped`，key＝`BcbGetCommaText(row)`，同檔內重複照收）。
    `goldenFileRule=true`＝golden 每天迴圈 `<yyyymmdd>.csv`＋`EventLogTxt_`；**這個函式 golden 沒有 VTEST 915／919 分支**（`GetEventLogText` :807-811 才有），照翻。
  - 照 golden：清六個 `sl*List`（:1198-1203），不動 summary／map；`LogRecord.recipe` 不填（:1244-1254）；第一形式只設 `dtStart／dtEnd／iDate`（GetSysDateTimeRange；`RangeFromPickers` 跟 `SetRange` 共用）。
  - **D-d**：呼叫端在產報表前重掃資料夾（`LoadFavorite`）再傳清單；R2 的報表都走 `BtnQuery`（golden btnQuery 的 `ImportProductionLod(2)` 本來就每次重掃）。
- **R1 ElaFileUtil**（新檔 `EventLogAnalysis/ElaFileUtil.*`）
  - Common.cpp `WriteDataToFile`（:584-613，OPEN_ALWAYS＋移到尾巴或 CREATE_ALWAYS，資料＋LF）、`MyForceDirectories`（:469-509）、`MyDBIProcess`（:23-48）；
    FileInfo.cpp `GetNameAndExtension`（:144-153）、`PathCombin`（:260-282）、`EnsureDirectoriesExist`（:284-312）；Analyzer.cpp `IsValidFileName`（:2572-2601）、`FixFolderPath`（:2603-2612）；
    以及它們呼叫的 Delphi 6 SysUtils：`LastDelimiter`（MBCS：cp950 尾碼 0x5C 不算 `\`）、`ExtractFilePath／Name`、`DirectoryExists`、`ForceDirectories`、`TStrings.SaveToFile`（`SaveBytesToFile`）。
  - **FTP_Log 不在這裡**：golden `MyDBIProcess` 也寫 `slFTPLog`（`D:\HT9045_Log\UploadFile\FTP_Log`）；這裡經 `SetDbiProcessHook` 交給 ElaFtp（R4，另一個 helper），沒裝 hook＝只進 mmoException。
  - 跟 golden 不同的地方（都不影響寫出的內容）：`WriteDataToFile` 回 bool（golden void）；`EnsureDirectoriesExist("")` 什麼都不做（golden `sPath[0]` 丟 ERangeError）；
    `IsValidFileName` 遇到 golden `ForceDirectories` 會丟例外的名字（相對路徑、沒有上層）回 false（golden 是例外離開，一樣不存檔）。
  - golden 事實：`MyForceDirectories` 的兩個 catch 只有在 `ForceDirectories` 丟例外時才到（相對路徑沒有 `\`，例 "abc"）；一般建不出來的資料夾是**靜默** -1。
  - 接縫：`Ht9045LogRoot()`＝`W906_HT9045LOG_ROOT`（Handler 同一個接縫，common.cpp :240／:425；沒設＝golden `D:\HT9045_Log`）。
- **R2 ElaReports**（新檔 `EventLogAnalysis/ElaReports.*`）
  - `SaveSummary`（:2614-2688）：檔名 `FixFolderPath(Path)`＋`sHandlerID`＋`-SummaryData_`＋（Name 或 `Now()` 的 yyyy-mm-dd）＋`.txt`；內容＝mmoSummary1（SetTotalSummary 的 20 行），
    bDetail 再加 `""`＋sgByHour（表頭 DateTime…MTBF＋每小時一列，14 格各加 `,`），chkIncludeAlarmList 再加 `//===`、sgTop5Filter（RowCount 列、6 格、表頭第 0 格空）、`""`、`//===`、sgTop5Alarm（9 格）、`""`。
    `IsValidFileName` 不過就不存（golden）；建檔＝`TFileStream fmCreate`（覆寫）。
  - `O06SaveSummaryData`（:3142-3152）、`N10SaveSummaryData`（:3154-3180）、`DoVTestSaveSummary`（:3240-3261）：各跑 `BtnQuery`（ElaHub，golden btnQueryClick，頁面查詢共用）再 `SaveSummary`。
  - **R5 呼叫點**：`Timer2Due(c, HH, NN, keepGoldenBugs)`＝golden Timer2 的 bNeedUpload；`Timer2Run`＝:3071-3083（今天 00:00:00～23:59:59，O06 再 N10）。50 秒、分鐘鎖、錯開、開機補跑、O10 閘都歸 R5。
  - `AutoJobsEnabled(c)`＝**D-a** O10 閘（O10＝O06-1＝config.ini `[Event Log] EnableAutoSaveEventLog`＝`ElaConfig::cbO06`，906_0625_Steven cprod.cpp:2394）。
  - ⚠ **D-a 的實際值（St02-E 20260927 23:5x，R5 helper 查證）**：兩棵樹的 `InitialCosFunction` 先把 `bEventLogAutoSaveFunction` 對**每個客戶**都設成 true（golden `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\CosFunction.cpp` 第 3894 行、V906 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CosFunction.cpp` 第 4111 行，`CustomerFunctionSelect` 先呼叫它：golden cprod.cpp 第 3654 行、V906 第 3901 行），各客戶的函式只會再設 true，沒有任何地方設 false。所以**目前每個客戶的有效 O10＝`EnableAutoSaveEventLog`**（golden cprod.cpp 第 2379～2394 行在讀設定時把 `bO10UseEventLogSaver` 蓋掉）。本機 `D:\HT9045\config\config.ini` 第 3 行 `EnableAutoSaveEventLog=0`、第 41 行 `bO10UseEventLogSaver=1` ⇒ **實際是關**。**不要去「修」config.ini 存的 `bO10UseEventLogSaver=1`**——它本來就會被蓋掉。程式照 golden 走 `ela::EffectiveO10`（wb_serve 裝的 getter 讀 `IniConfig.bO10UseEventLogSaver`，已被 V906 cprod.cpp 第 2519 行覆寫）。
  - **HOLD 解除**：`ElaHub` 的 `EL_VTEST_MTBF_SUM` → `DoVTestSaveSummary`，O10 關就記「skipped」不跑（D-a）。（原本「那次查詢變成快照」已改：見下面「R2 追加」，工作結果另存 job record，不換頁面快照。）
    `Hub::PostQuery` 同時記成頁面狀態（資料夾、篩選、sHandlerID、includeAlarmList），EL_* 工作用它查。其他五個工作仍 HOLD（R3／R4），訊息不變。
  - `QueryRequest.includeAlarmList`（新，預設 false＝dfm 的 chkIncludeAlarmList 不勾，golden 從不讀 ini）。`Hub::SetClock`：ctest 的假時鐘。
- **R2 偏離**
  1. **D-e：報表檔是 UTF-8**（golden：TMemo 的 ANSI 文字＝cp950）。逐行 `ToUtf8`（合法 UTF-8 不動、其他當 cp950 轉），不加 BOM，行尾 CRLF（golden TMemo 每行＋#13#10）。檔名仍是 ANSI bytes（CreateFileA）。
     ⚠ 讀這些報表的人吃不吃 UTF-8：還沒查（計畫 D-e 的提醒）。R2 的三個報表不走 FTP，但**不一定只在本機**：O06 的 `AutoSaveProductionPath` 可以指到網路磁碟
       （config 另有 O06 網路磁碟設定 `bAlarmStatistAutoSaveNetDrive`／`asAlarmLocalDirectory`＝Z:，Rev891 Analyzer.cpp:3097、:3106-3107，分析器只讀不用），
       N10-3 的 `iN10UploadMethod`＝1 就是網路磁碟 `sN10UploadDrivePath`。
  2. **W18 B（G3）**：O06-4 沒勾時間週期時 golden 每分鐘存一次全天摘要（`SystemNN % 1`），註解寫「預設一小時一次」→ `Timer2Due` 整點一次（`keepGoldenBugs`＝golden 每分鐘）。
     → **★W43＝C（Steven 0928）取代**：沒勾 [O06-8]＝`Timer2Due` 一天 1440 分鐘都不觸發 O06-4（見 R5「★W43」）。
  3. **W18 B**：golden `SaveSummary` 把明細**原地**加在 mmoSummary1，沒重查再存一次會重複一份明細；這裡每次從查詢的 mmoSummary 開始（`keepGoldenBugs`＝golden）。
  4. W21／§3.3「每次重算、不附加」：`SaveSummary` 本來就 fmCreate 覆寫——跟 golden 一樣，不是偏離。
- **R2 照翻的 golden 事實**
  - O06／N10：`MyForceDirectories` 在功能開關**之前**（O06-4／N10-3 關著也會建資料夾；Timer2 觸發時）。
  - `TRadioGroup.ItemIndex` 會夾住：`rgN10_3_1` 只有 3 項（Analyzer.dfm:2765-2768），Handler 的 `iN10UploadProductMethod`＝3（指定時間）在分析器＝**每小時**；
    `rgN10_4` 只有 2 項，≥2＝網路磁碟。（計畫 §1.1 N10 列原寫「方法 3 沒有分支」，已更正。）`TComboBox` 不合法的 `TimePeriodSaveLog`＝-1＝30 分鐘。
  - N10 的 "FTP" 分支只存 `D:\HT9045_Log\EventLogSummary\yyyymm\`、不上傳；yyyymm 是 SystemYY／MM，O06 的 SaveSummary 會先改它（跨月才看得出來）。
  - golden 模式下 btnQuery 在 Production_Log 丟例外 → 不存檔；golden 的例外還讓 Timer2（`bFirstIn` 停在 false）或分析執行緒從此停掉——V906 不重現，只記。
  - DoVTestSaveSummary 沒有 MyForceDirectories：資料夾由 `IsValidFileName` 建。
- **ctest `ELA_Reports`**（`tests/test_ela_reports.cpp`，沙盒 `%TEMP%\ht9045_ela_reports_<tick>`；根目錄在 `D:\HT9045`、`D:\HT9045_Log`、`D:\RMS`、`D:\MTBF_Summary`、`D:\EventlogAnalyzer` 底下就拒跑；自己設 `W906_HT9045LOG_ROOT`；綠燈才刪沙盒）
  1. ElaFileUtil 每個函式（含 cp950 尾碼 0x5C、MyDBIProcess hook）；2. GetEventLogTextToVec：2025-09-10 W15 B 131 筆 JAM／WAR／MES＝GetEventLogText 的 131 stop、golden 123；AllEventLog 複本預設不加、golden 加倍；
  3. SaveSummary：46 行、表頭、24 小時、Total Input Qty 265＝各小時加總、MES 123、UPH 11、Top5 兩個表、再存一次位元組相同、golden 模式重複明細、`|` 被拒、cp950 Handler ID 寫成 UTF-8；
  4. VTEST：Now()-7～Now()-1（7 天）、20 行；5. Timer2Due 表、Timer2Run 兩個檔＝第 3 節的報表、N10-4 夾住、關著也建資料夾；6. Hub：EL_VTEST_MTBF_SUM 寫檔＋job record（含 7 天區間）、頁面快照不動、O10 關＝skipped、其他仍 HOLD；
  7. **oracle**：`tests/fixtures/ela/oracle/<ID>-SummaryData_r2detail.txt`／`_r2alarm.txt`（BCB exe 產生，步驟在同資料夾 `README.txt`），cp950→UTF-8 後逐位元組比（golden 模式；Fail Count 的 G1 位移從 oracle 算）。**沒有 oracle＝SKIP，不是 FAIL**。
  - Fixture：`tests/fixtures/ela/Production_Log/202509/PMLD1019_20250910.csv`（從 `D:\HT9045_Log\Production_Log\202509\` **複製**，唯讀）＋既有的 EventLogTxt 樣本。
- **V906 還沒有 `EL_VTEST_MTBF_SUM` 的送出端**：golden HS_Function.cpp:257-272 在 V906 是 `forms/fHS.h:691` `TimerAutoBackupTimer`（仍 GATE，不是 St02 的檔）；
  計畫 W21 把這個每週觸發搬進 ElaSchedule（R5），不翻 TimerAutoBackupTimer。R5 之前只有 ctest（或外部 Post）會舉起它；O06／N10 也要等 R5。
- ⚠ 未上機。上機要看（R5 接好或手動送之後）：VTEST（915／919）機台 O10 開，`asO19_SavePath`（預設 `D:\MTBF_Summary`）有 `<Machine ID>-SummaryData_<今天>.txt`（20 行，UTF-8），內容區間是前 7 天；
  O10 關時不產生；eventlog.html 仍顯示使用者自己最後一次查詢（工作的 7 天區間在 job record，R6 顯示）。

## R4 ElaFtp（WinINet）＋N25-3／N10 BYFILE（20260927，St02-E；分支 `v906/steven-elaftp-wip`，兩組態只編譯，ctest 待 St01）

設計 `D:\HT9045\.claude\skills\ht9045-st02-workflow\references\research-wininet-elaftp.md`；計畫 `D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md` §3／§4 R4；
Steven 20260927：FTP 傳輸＝Windows WinINet（`IElaFtp` 後面，ctest 用腳本化假物件）。

- **新檔**
  - `EventLogAnalysis/ElaFtp.h`／`.cpp`：`IElaFtp`（Connect／Close／Abort／ChangeDir／MakeDir／Put／Size／List）、`FtpEndpoint`／`FtpStatus`／`FtpEntry`；
    null 傳輸 `NewNullFtp`（Hub 預設）、只寫 log 的傳輸 `NewLogOnlyFtp`；純函式（回覆碼、`213 n`、`MaskSecrets`、golden `PathCombin`＝`FtpPathCombin`、BackUp 檔名、FNV-1a、錯開、
    退避 `BackoffSeconds`／`UploadRetry`）；`FtpLog`（FTP_Log）；`EnsureDirOneLevel`（golden ChangeDir）、`EnsureDirPath`（golden ChangeDirectories）、`UploadAndVerify`；
    `FtpSessionLock`（一次一個傳輸）；`FtpAbortSlot`（Hub::Stop 取消）。
  - `EventLogAnalysis/ElaFtpWinInet.cpp`：全樹唯一 include `<wininet.h>` 的檔，`NewWinInetFtp()`（InternetOpenA DIRECT、CONNECT_TIMEOUT＝5 s、CONNECT_RETRIES＝1、
    控制／資料 timeout＝30 s、InternetConnectA FTP、FtpSetCurrentDirectoryA／FtpCreateDirectoryA／FtpPutFileA binary、`TYPE I`＋`SIZE` 用 FtpCommandA＋InternetGetLastResponseInfoA、
    LIST 用 FtpFindFirstFileA(NULL, RELOAD|NO_CACHE_WRITE)、看門狗 WbThread 逾時關 hOpen、Abort 任何執行緒）。
  - `EventLogAnalysis/ElaChipMos.h`／`.cpp`：`SaveJamCodeFor7Days`、`N25_UploadJamDataToFTP`、`UploadJamCode`（Rev891 `uChipMosZHUBEI_Func.cpp:25-56`／`:184-255`／`:328-350`）、
    `N10_UploadDataToFTP`、`UploadByFile_N10`（`:145-182`／`:392-418`）、`Hub::RunUploadJob`。
  - tests：`tests/ElaFtpFake.h`（腳本化假傳輸，header-only）、`tests/test_ela_ftp.cpp`（ctest `ELA_Ftp`）、`tests/ela_ftp_wininet_link.cpp`（只編譯，不進 ctest）。
- **改的檔（都是 St02 的；行號＝合併 gpib-widget `bf8ecd42`〔R1／R2〕之後）**：`EventLogAnalysis/ElaHub.h`（include、`SetFtpFactory`／`GetFtpFactory`／`JamWeekRetry`、`RunUploadJob` 宣告、
  三個成員 :166-168）、`EventLogAnalysis/ElaHub.cpp` :261-264（JAMWEEK／BYFILE 分派，R2 的 VTEST 分派 :265-268 緊接在後）、:473（Start → slot Reset）、:481（Stop 先 Abort）；
  `EventLogAnalysis/ElaService.cpp` :162-166（`W906_ElaStart` 裝傳輸＝全樹唯一裝 WinINet 的地方）、:167（裝 MyDBIProcess → FTP_Log 的 hook）、:188（Stop 後清 hook）、:176-178（開機訊息，R2＋R4）；
  `tests/test_ela_hub.cpp` :9／:81（JAMWEEK 現在被設定閘擋下，不再記 HOLD）。
- **跟 R1／R2 合併時（20260927）**：時鐘統一用 R2 的 `ela::Clock`（`Hub::SetClock(const Clock*)`，`ElaReports.h`）；R4 原本的 `double (*)()` 時鐘與同名 `SetClock` 拿掉（同名會撞）。
  上傳工作的 O10 閘改用 R2 的 `AutoJobsEnabled(c)`（同一個 `cbO06`）；EventLogTxt 資料夾改用 R2 的頁面狀態 `base_.eventLogDir`（golden labPath；預設同 `W906_EVENTLOG_ROOT`）。
  合併後 HOLD 只剩三個工作（CHIPADV_LOTEND／SUMMARY／EVENTLOG）；R2 那段寫的「其他五個」是合併前的數字。
- **CMake**：`CMakeLists.txt` :1370-1372 三個新檔（ht9045_ela 清單尾端，各一行）；:1375 `ht9045_ela` 改連 `$<$<PLATFORM_ID:Windows>:wininet>`，**不再連 `ht9045_nmftp`**；
  :1376 三個新檔開 `-Wall -Wextra`（sim 編譯 0 警告）。**`ht9045_nmftp` 庫保留**（R0 的建議：KYECFTP 的 `ht9045_kyecftp` PUBLIC 連它，TNMFTP 只在一個 archive；
  ELA 從來沒用過 `Nmftp::`）。`CMakeLists.txt:3084-3085` 的 R0 註解還寫 ht9045_ela 會用它——已過期；這次只動 ELA 區段，留給下一個動 CMake 的人（行數不變改一行）。
  `tests/CMakeLists.txt` :3421-3435（接在 ELA_Hub 之後）。
- **傳輸怎麼選**：Hub 預設＝null（ctest 裡建的 Hub 連不到網路）；只有 `W906_ElaStart` 裝：`W906_ELA_FTP_WININET`（`ElaFtp.h` :41-46）＝1 → WinINet，0 → 只寫 log。
  sim 組態的 wb_serve 連 WinINet 都不連結（objdump 20260927：sim `wb_serve.exe` 沒有 WININET.DLL；`test_ela_ftp.exe`／`test_ela_service.exe` 只有 KERNEL32＋msvcrt；只有 `ela_ftp_wininet_link.exe` 有 WININET.DLL）。
- **連不連（#22 D-f，照 golden；沒有環境變數開關）**：`N25_3GateReason`＝O10（＝O06-1 `EnableAutoSaveEventLog`，906_0625_Steven `cprod.cpp:2394`；D-a）、CUSTOMER_CODE 851
  （golden 只在 `CC_ChipMos_ZHUBEI` 才從檔案讀 `bN25_3_EnableULJamLog`：906_0625_Steven `cConfiguration.cpp:3729`／`:3743`）、`bN25_3_EnableULJamLog`（config.ini [ChipMos Function]，
  唯讀、不寫回；分析器從沒讀過它）、N25-2 主機非空、N25-3 路徑非空（golden `:188`）。閘沒開＝不建傳輸、不建資料夾。
  N10 BYFILE：O10、`iN10UploadMethod`＝0（FTP）、N10 主機非空（golden 分析器不閘；golden 也沒有送出端，906_0625_Steven `HS_Function.cpp:432-436` 註解掉）。
- **V906 現在沒有人送 `EL_UPLOAD_JAMWEEK`／`EL_UPLOAD_BYFILE_N10`**：golden 送出端 906_0625_Steven `main.cpp:21442-21447` 在 `#ifndef SOFT_SIMULTE`（:21425-21459）裡、沒翻，排程是 R5。
  所以 R4 合進去不會自己連線；`SendCommand_EventLog` 給外部 exe 的 WM_COPYDATA 照舊。
- **偏離（R4）**
  1. 驗證：STOR 成功＋SIZE＝本機位元組數（500／502／504 → 用 LIST 的大小；本機非空但列出 0 位元組＝失敗；列表沒大小＝「只驗檔名」並寫進 FTP_Log）＋重新 LIST 看得到檔名，
     三個都成立才算（golden「沒丟例外」＝成功，Rev891 `TfFTP.cpp:412-422`）。
  2. `HadUpload.txt` 只有主檔**和** BackUp 都驗證成功才寫，內容照 golden 三行（`:227` "Upload path:"、`:237` "Upload backup path:"、`:348` "HadUpload"）；golden `:227` 在主檔 STOR 後就寫進去
     （BackUp 失敗也擋掉當天所有重試），`:235-243` 只看 BackUp 的結果。`:237` 印的是主檔資料夾（golden bug）→ 這裡寫 BackUp 資料夾（W18 B）。
  3. `Jam Rate.txt` 每次重建（先刪再寫），不再每次附加表頭＋資料（plan §3-3）；UTF-8（D-e；BCB 寫的 EventLogTxt 是 cp950 → `ToUtf8`）；LF 照 golden（WriteDataToFile 寫 "\n"）。
  4. 記錄＝R1 的 `Analyzer::GetEventLogTextToVec`（ElaCore，Rev891 `Analyzer.cpp:1183-1267`；合併後改用，原本檔內的 `ReadRecords` 刪掉——兩者規則相同）：產報表時重掃資料夾（D-d，`LoadFavorite`），
     Hub 的 W15 切法與 W19 選檔＋跨檔去重（`goldenFileRule`／`bcbCommaText` 切回 golden）。跑在**這個工作自己的** Analyzer 上：golden 會把頁面的日期挑選器改成 Now-8～Now-1、清掉 sl*List，這裡不動頁面的 Analyzer。
  5. `ChangeDir`（golden `TfFTP.cpp:144-171`）：CWD → 失敗就 MKD → **再 CWD**（golden 不再 CWD，但 `Upload` `:402` 會再 ChangeDir 一次，順序相同）。
  6. `ChangeDirectories`（N10，golden `:83-142`）：從根逐層 CWD，失敗就 MKD 那一層＋CWD。golden `:120` MKD 父層 `sCurr`、每層先 NLST、用 CommaText（空白也切）、
     從 CurrentDir 相對起算、`i=1` 起跳（相對路徑的第一段被略過）——都修掉（W18 B）。
  7. golden 的 SOFT_SIMULTE 覆寫（`TfFTP.cpp:15-23`、`uChipMosZHUBEI_Func.cpp:206-210` 帳密＋127.0.0.1、`:342-344` 路徑）**不搬**；模擬組態原本改用只寫 log 的傳輸（★W36 A），**★W36＝C（Steven 20260928）後跟出貨組態一樣用 WinINet**（見下）。
     那些帳密字面值不出現在 V906。
  8. 本機根目錄走 `W906_HT9045LOG_ROOT` seam（golden 字面 `D:\HT9045_Log`，`:332`／`:396`；沒設＝同值）；建資料夾與本機路徑串接也認 '/'（seam 可能是正斜線；golden 路徑結果相同）。
  9. 主機名用 `GetComputerNameExA(ComputerNameDnsHostname)`（golden `Analyzer.cpp:426-434` `gethostname`，同一個名字；不讓 ht9045_ela 為此連 ws2_32）。
  10. FTP_Log（golden slFTPLog，`Analyzer.cpp:220-224`）：`<log root>\UploadFile\<yyyy>\<mm>\FTP_Log_<yyyymmdd>.csv`、首列 `Date, Time, Action, S2, S3, S4, S5, S6`、CRLF、
      `yyyy-mm-dd, hh:nn:ss.zzz, <CommaText>`（`MyStringList.cpp:141-193`、`:506-642`）；每行立刻寫（golden MaxLineCount=1 緩衝）；golden 把所有 MyDBIProcess 都寫進去（`Common.cpp:39`），
      這裡上傳的每一步寫一行，另外 `W906_ElaStart` 把 R1 的 `SetDbiProcessHook` 接到 `ela::FtpLogDbiLine`，所以 ElaFileUtil 的 `MyDBIProcess` 行也進 FTP_Log（golden 同）；
      上傳的行都遮帳密（hook 的行不含帳密）。**FTP_Log 歸 R4（`ElaFtp.cpp` `ela::FtpLog`）**；R2 不寫 FTP_Log。
  11. 一次一個傳輸：Hub worker 本來就一次一個工作（`runMu_`），另加行程內 `FtpSessionLock`。
  12. `Hub::Stop` 先 `FtpAbortSlot::Abort()`（WinINet 關 hOpen），關機不等 30 秒的傳輸；`Hub::Start` 重新開放。
  13. 退避：`BackoffSeconds(n)`＝min(60·2^(n-1), 1800) s ×(1＋0.2u)，u 由 FNV-1a-32(Machine ID＋"#n") 決定；`OnUploadFailed` 6 次重試後放棄，或下一次表定時間（N25：隔天 00:00:01）先到就放棄。
      R4 只算並記進工作歷史（"retry due …"），**真正排重試是 R5**。
  14. N10 BYFILE 的列用 Hub 的 W15 切法（golden TStringList CommaText 空白也切，`:404`）；`UploadFile.csv` 照 golden 不刪（`:401` 註解掉 → 每次重傳每一列）。
- **待 Steven（原本兩個一行開關；★W36 已裁決）**
  - **★W36＝C（Steven 20260928）：模擬組態跟出貨組態一樣上傳**（AI(W906-ELA-W36) 20260928，St02-E helper，分支 `v906/steven-w36-w46`；兩組態只編譯、未上機）。
    原本的 A（R4 暫定）：`ElaFtp.h` `#define W906_ELA_SIM_FTP_LOG_ONLY 1`，沒定義 `W906_NO_SOFT_SIMULTE` 時 `W906_ElaStart` 裝只寫 log 的傳輸，sim 的 wb_serve 連 WinINet 都不連結。
    改法：`W906_ELA_SIM_FTP_LOG_ONLY`／`W906_ELA_FTP_WININET` 兩個巨集拿掉，`ElaFtp.h` 留歷史註解；`W906_ElaStart` 兩個組態都裝 `NewWinInetFtp`；`BuildFtpTransport()` 恆為 WinINet；
    `GET /api/ela/schedule` 的 `build` 改看 `W906_NO_SOFT_SIMULTE`、`ftp` 看傳輸（兩組態都是 "WinINet"）；eventlog.html 確認框的上傳說明改看 `ftp`、不再看 `build`。
    連結：`ht9045_ela` 本來就 PUBLIC 連 wininet（`CMakeLists.txt:1378`，不分組態），CMake 不用改；sim 的 wb_serve.exe 現在匯入 WININET.dll（objdump -p 看過）。`NewLogOnlyFtp` 保留（ELA_Ftp 第 12 節用；沒有組態再裝它）。
    **排程沒變**：模擬組態仍不自動跑 N25-3／N25-4／N25-5（golden main.cpp:21425 `#ifndef SOFT_SIMULTE`）；手動「立即執行」（`POST /api/ela/job`）現在會真的連線。
    ctest 仍不連線：只有 `W906_ElaStart` 裝 WinINet，唯一呼叫它的 ctest（ELA_Service 第 2 節）先設 `HT9045_ELA=0`、不建 Hub；其他 Hub 是預設的 null 傳輸（ELA_Ftp 第 0 節）或注入假傳輸／只寫 log 的傳輸；
    test_ela_ftp.exe 仍不匯入 WININET.dll（第 0 節照舊成立），test_ela_service.exe／test_ela_schedule.exe 現在匯入（出貨組態本來就是）但沒有路徑會呼叫。（B「SIM 用 WinINet 但主機改 127.0.0.1」沒做。）
  - **Passive 預設（★W46，待 St02-M 選來源）**：`EventLogAnalysis/ElaFtp.h:56` `const bool kElaFtpPassive = false;`（active／PORT）。golden TfFTP 從不設 Passive（Rev891 `grep -i passive *.cpp *.h *.dfm`＝0 筆，20260927），
    用 FastNet TNMFTP 元件的預設；本機沒有 NMFtp.hpp，預設值查不到；暫定照任務讀成 active。Jimmy 的 `KYECFTP/MiniFtpEngine.h:52-63` 推測元件預設是 true；
    Handler 自己的 N10 路徑用 `bN10FtpPassive`（906_0625_Steven `HS_Function.cpp:2134`），分析器忽略它。N25-3 與 N10 BYFILE 共用這一個開關（research 原建議 N25-3＝passive、N10 讀 bN10FtpPassive）。
- **ctest `ELA_Ftp`**（`tests/test_ela_ftp.cpp`；全部在 `%TEMP%\ht9045_ela_ftp_<tick>`，綠燈就刪；先 `_putenv` `W906_HT9045LOG_ROOT`／`W906_EVENTLOG_ROOT`）：
  0 網路防護（開頭與結尾 `GetModuleHandleA("wininet.dll")==NULL`）、seam、Hub 預設 null；1 純函式；2 假時鐘退避（6 次、放棄、時段先到、成功歸零）；
  3 N25-3 成功（整串 CWD／MKD／STOR／SIZE／LIST 逐行比對、Jam Rate.txt 位元組、HadUpload.txt 三行、同一天再跑不建傳輸）；4 SIZE 不符 → 重試成功、報表重建；
  5 主檔／BackUp STOR 550；6 SIZE 502 → LIST 大小／0 位元組／只驗檔名；7 LIST 沒有、本機沒檔；8 登入失敗（帳密遮蔽、只有 CONNECT＋CLOSE）、逾時可重試；
  9 N10 逐層建資料夾；10 N10 BYFILE；11 D-f 閘五種＋Hub 端（沒 bN25_3 → skipped；有 → 跑在假傳輸上，假時鐘）；12 組態的傳輸（★W36＝C：兩組態都是 WinINet；原本 sim＝log-only）、log-only 全流程、null；
  13 取消槽；14 FTP_Log 路徑、表頭、沒有帳密、`FtpLogDbiLine`（hook 的目標）寫一行。帳密在執行時產生，檔案裡沒有密碼字面值。Hub 的假時鐘是 R2 的 `ela::Clock`。
- **只編譯**：`ela_ftp_wininet_link`（tests，無 add_test）：兩組態都要連得過 WinINet；main 只取 factory 位址，不連線。
- **還沒做**：R5 排程（半夜觸發、重試、開機補跑、錯開）；`/api/ela/job` 手動觸發；N25-4／N25-5（EL_UPLOAD_SUMMARY／EL_UPLOAD_EVENTLOG 仍 HOLD）；UploadProdLog（W13，檔案複製不是 FTP）；
  eventlog.html 的上傳狀態（R6）。ElaChipMos.cpp 裡還有幾個跟 R1 ElaFileUtil 重複的小工具（EnsureDirectoriesExist、WriteDataToFile 形狀的 AppendLine、本機路徑串接）——
  在匿名 namespace、不撞符號，之後可換成 ElaFileUtil 的版本（行為要先比對：R4 的也認 '/'）。
- ⚠ 未上機。上機要看：(1) 851 機台 N25-3 與 O10 開時一次上傳（R5 之後半夜，或手動 Post）：伺服器上 `<path>/<hostname>/Jam Rate.txt` 與 `BackUp/Jam Rate_yyyymmdd_hhnn.txt`、
  本機 `D:\HT9045_Log\JamWeek\y\m\d\HadUpload.txt` 三行、`D:\HT9045_Log\UploadFile\yyyy\mm\FTP_Log_yyyymmdd.csv`；(2) 伺服器吃不吃 active 模式（`kElaFtpPassive`）、有沒有 SIZE（沒有就走 LIST 大小）；
  (3) 拔網路線時約 6 秒內失敗、關 wb_serve 不卡 30 秒；(4) 客戶讀不讀 UTF-8 的 Jam Rate.txt（D-e，St02-M 要跟 Steven 確認）；
  (5) ★W36＝C：模擬組態的 wb_serve 對 Steven 團隊自己架的 FTP 伺服器，用 eventlog.html「立即執行」跑一次 N25-3（或 N10 BYFILE），看 FTP_Log、伺服器上的檔與 HadUpload.txt。

## R2 追加：工作結果跟頁面快照分開；報表編碼只在一個函式（20260927，St02-E；St02-E 對 R2 問題 2、4 的回覆）

分支 `v906/steven-ela-wip`：`673cf249`。兩組態只編譯。
- **偏離**：背景工作（EL_VTEST_MTBF_SUM、R3 的 EL_UPLOAD_CHIPADV_LOTEND、R4 的上傳工作、R5 之後的排程工作）**不再換掉** eventlog.html 看的快照；
  快照只在使用者自己的查詢時變。每個 EL_* 工作留一筆 `ela::JobRecord`（時間、指令、有沒有跑、有沒有寫檔、檔名、那次查詢的區間、說明），
  `Hub::Jobs()`／SnapshotJson `"jobs":[...]`（最多 50 筆，R6 顯示）。golden 的視窗在工作後會顯示工作的區間（日期挑選器就是參數匯流排，§1.2）。
  R4 的上傳工作（`RunUploadJob`，ElaChipMos.cpp）的 record＝它留下的最後一行歷史（`Hub::RecordUploadJob`，merge `9f9cb4ad`）。
- **D-e 只在一個函式**：`ela::ReportEncode(line)`（ElaReports.cpp）決定報表的位元組（現在＝`ToUtf8`）；SaveSummary（`SummaryBytes`）與 R3 ClipAdv 都走它。
  之後若裁決改回 cp950 或加開關，只改這一行。
- ctest：`ELA_Reports` 第 6 節（工作前後 `"result"` 為 null／頁面自己的查詢才出現；job record 的區間）、`ELA_Hub` 第 2 節（每個工作一筆 record）。

## R3：N34 ChipAdvanced（OEE／警報報表，CC_CYUEAN 868）（20260927，St02-E；計畫 §4 R3）

分支 `v906/steven-ela-wip`：`929ced5a`；跟 gpib-widget（含 R4）合併 `9f9cb4ad`。兩組態只編譯（St02 不執行）；ctest `ELA_Reports` 第 8 節待 St01。行號＝Rev891 `uChipAdvancedFunc.cpp`。
- **移植**（`EventLogAnalysis/ElaReports.*` 檔尾）：`ChipAdvancedFunc`——`AnalysisLog`（:22-72）、`HadAnomaly`（:74-87）、`GetEventLogReport`（:89-98）、
  `GetPerformanceMetrics`（:100-124）、`GenerateOEEReport`（:126-152）、`GenerateAlarmReport`（:154-175）、`FindStartLotFromRecVec`（:213-230）、`SaveReport`（:232-241）；
  無狀態的 `ChipAdvParseField`（:243-257）、`ChipAdvHourMinSecStr`（:193-201）、`ChipAdvDateTimeStr`、`ChipAdvDaybySec`、兩個組字串函式。
  流程：最近 8 天的列（`GetEventLogTextToVec`，先重掃資料夾＝D-d）→ 從尾巴找**最後一個** "Lot Start, Lot ID:" → 從那一列到 Now() 跑 `BtnQuery` → 再取一次列 →
  各碼次數／停機秒數 → `<sN34_OEEAlarmRptPath>\ClipAdv_<LotID>_<y><m><d>.csv`（`%d%d%d` 不補零，golden）。
- **HOLD 解除**：`EL_UPLOAD_CHIPADV_LOTEND` → `Hub::RunChipAdv`（O10 閘 D-a、job record、不換頁面快照）。分析器端 golden 不檢查 N34 開關（`AnalysisThreadProcess` :173-178）；
  Handler 在 lot end 檢查 `bN34_GenerateOEEAlarmRpt` 才送（906_0625_Steven uLotInfo.cpp:2327-2334）。
  **V906 的送出端是 St01 的 `forms/fLotInfo.cpp` 第 7082～7095 行，仍是 `#if 0`**（GATE W906-PROD-S117-S25-CYUEAN；St02 沒改）——St01 打開之前只有 ctest 會舉起這個工作。
- **偏離**
  1. **D-c**：8 天內沒有任何列時 golden `for(size_t i=iTsize-1; i>0; i--)` 會繞回最大值、讀到向量外（當掉）；這裡直接不出報表（記「D-c」）。
  2. **W21／§3.3＋D-b**：報表每次重寫（`WriteDataToFile(..., true)`）；golden 附加（同一批同一天第二次 lot end 會接在後面）。
  3. **D-e**：位元組走 `ReportEncode`；行尾照 golden 是 LF，最後多一個 LF（WriteDataToFile 自己加的）。
  4. **W18 B（ParseField）**：golden 在欄位名後面**固定跳一個字元**（`pos+fieldLen+1`，原意是 "Lot ID: X" 的空白），但 Handler 寫的是 "Lot ID:%s"（沒有空白，906_0625_Steven uLotInfo.cpp:1609）
     ⇒ golden 的 Lot ID／OP ID／Run Mode 都少第一個字（"Lot ID:AFYD09N370-D001" → "FYD09N370-D001"；一個字的 Lot ID 變空字串 → 檔名 "ClipAdv_(null)_..."）。
     這裡只有後面真的是空白才跳；`keepGoldenBugs`＝golden。
     **★W47＝A，Steven 0928**（批號少一個字＝修正，取完整批號）：程式本來就照 A 做，沒改（`keepGoldenBugs` 仍可切回 golden）。
- **照翻的 golden 事實**：找 Lot Start 時 8 天清單的第 0 筆從不看；計數從第 1 筆開始（第二次清單的第 0 筆就是 Lot Start 那列本身）；
  Lot ID 空字串時檔名印 "(null)"（Borland `%s` 遇到空 AnsiString＝NULL；Handler 自己的 log 也有 "(null)==>EQC"）；OEE＝(input − iFailCount)／input；
  `iDate`＝(dtEnd − dtStart + 1) 截成整數；`GetHourMinSecStr` 的秒是 `%02.2f`（小於 10 秒不補零，例 "36:00:7.25"）。
  `sN34_OEEAlarmRptPath` 空字串時不寫（golden 會在目前目錄建 "(null)" 資料夾；ReadConfig 不會讓它是空的）。
- **沒驗證**：golden 的 "%0.2f%"／"%.2f%" 最後是一個單獨的 `%`（C 語言未定義，Borland RTL 怎麼印沒查）；這裡照原意印出 `%`。要 BCB exe 實跑才能定。
- **ctest `ELA_Reports` 第 8 節**：helper（時間格式、ParseField 兩種模式）；自己寫的一批（每個數字手算：報表逐位元組、檔名、第二次重寫不附加）、golden 模式（ClipAdv_OT123_…）、
  D-c（沒有列：不當、不出）、golden 的第 0 筆、W23 樣本（15 個碼、39 次、336 秒，Python 鏡像算的）、Hub 工作（O10 開／關）。
  Fixture：`tests/fixtures/ela/ChipAdv/EventLogTxt/2025/03/HT-9045HA_E3200-0390_EventLogTxt_20250330.csv`＝`D:\HT9045_Log\EventLogTxt\2025\03\` 那個檔的**複本**，
  只留表頭＋第 3900～4050 行（最後一個 Lot Start 23:27:28 前後）。
- ⚠ 未上機。上機要看（CC_CYUEAN 868、O10 與 N34 開、St01 打開送出端之後）：lot end 後 `sN34_OEEAlarmRptPath`（預設 `D:\HT9045_Log\Product_Loader\OEEAlarmRpt`）
  有 `ClipAdv_<LotID>_<y><m><d>.csv`，區間是最後一個 Lot Start 到 lot end；Lot ID 第一個字還在；同一批同一天第二次 lot end 會取代舊檔。

## ★W45（原 W20）第一步：JAM0000.dat 共用規則 JamRules.h、W906_JAM0000_PATH（20260927，St02-E；Steven 還沒裁決 ★W45）

> 20260928：Steven 已裁決（第 18 項），開關改成 B、第二框已做——見下一節「★W45（原 W20）Steven 0928 裁決後」。本節是當時的紀錄。

分支 `v906/steven-ela-wip`：`143445be`。兩組態只編譯（St02 不執行）；ctest `Jam_Rules`（新）待 St01。
調查：`D:\HT9045\.claude\skills\ht9045-st02-workflow\references\research-w20-jamcode-editor.md` §5。只做**不需要裁決**的部分；★W45-1～4（原 W20-1～10）都還沒答。

- **新檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JamRules.h`**（header-only，放在樹根、跟 `JamIniMerge.h` 同一層＝調查 §5.1 的位置；St02 的）：
  - `Jam0000Path()`：沒設 `W906_JAM0000_PATH`（或空字串）＝golden 字面值 `D:\HT9045\Error\English\JAM0000.dat`（906_0625_Steven `cSecurity.cpp:216`；Rev891 `Analyzer.cpp:296`）。
  - `IsMtbfAlias(933／967)`（906_0625_Steven `MachineType.h:300`／`:337`；`cSecurity.cpp:1335-1336`、`:1138-1139`），int 與字串（`Options::custCode`）兩種。
  - 鍵名：`LevelKey`、`IncludeMtbaKey`、`IncludeMtbfKey`、`HandlerIncludeMtbaKey(code, alias)`（Handler 的「Include MTBA」框：933／967＝IncludeMTBF，其他＝IncludeMTBA；`cSecurity.cpp:1335-1353` 讀、`:1138-1146` 寫）。
  - 缺鍵預設：`DefaultJamLevel()`＝0（Rev891 `Analyzer.cpp:2806`；Handler 讀字串 "0" 再套客戶鉗制 `cSecurity.cpp:1211-1293`，鉗制不在這裡）；
    `DefaultIncludeMtba`＝兩邊同一條（Rev891 `Analyzer.cpp:2820-2832`＝906_0625_Steven `cSecurity.cpp:1342-1354`）；`AnalyzerMtbfListed`＝分析器的 24 碼＋`24 Motor`（Rev891 `Analyzer.cpp:2846-2871`）。
  - **★W45 的一行開關**（原 W20-3）：`static const IncludeMtbfRule kIncludeMtbfRule = kIncludeMtbfGoldenPerSide;`——A＝兩邊各照 golden（**預設**，等 Steven），B＝兩邊都照 Handler（933／967 缺鍵＝true），C＝兩邊都照分析器。
    `DefaultIncludeMtbfAnalyzer(area, code, alias, rule)`（ELA 用）、`DefaultIncludeMtbaHandler(area, code, alias, rule)`（Handler 將來用）。
  - 沒放：調查 §5.1 的 `SecondBoxKey`（第二個勾選框綁哪個鍵）——那要等 ★W45 的編輯器形狀（原 W20-5）裁決。
- **ElaCore**：`JamConfig::GetJamLevel`／`GetJemIncludeMTBA`／`GetJemIncludeMTBF`（`EventLogAnalysis/ElaCore.cpp` 第 606～646 行附近）的鍵名與缺鍵預設改從 `JamRules.h` 取；
  `JamConfig::SetCustCode`（新），`Analyzer::SetCustCode` 一起傳（ReadConfig 的 `sCustCode`）。**開關＝A 時行為完全不變**：ctest `Jam_Rules` 第 3 節把三個 getter 跟它們 W45 以前的本體
  （逐字抄在測試裡）在 33 個 section × 12 個碼 × 4 個客戶碼（"", 933, 967, 868）上比答案，並比寫出來的 JAM0000.dat 逐位元組相同。
- **接縫**：`EventLogAnalysis/ElaService.cpp` `W906_ElaStart` 設 `o.jamIniPath = jamrules::Jam0000Path()`（W906_JAM0000_PATH，沒設＝golden 字面值）。
  `tests/test_ela_service.cpp` 把它指到自己的沙盒；`Jam_Rules` 第 1 節測沒設／有設。
- **偏離**：無（開關＝A）。開關改 B 時 ELA 對 933／967 的缺鍵 IncludeMTBF 改寫 true（偏離分析器 golden）；改 C 時要改 Handler 端（下面的呼叫點）。
- **將來的呼叫點（沒改；等 ★W45 裁決與 ST01-E 確認擁有者）**，都在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`：
  - `cSecurity.cpp` 第 272 行（建構子）與第 2085 行（`W906_SecurityJamBoot`）的 `FileNameJam000="D:\\HT9045\\Error\\English\\JAM0000.dat"` → `jamrules::Jam0000Path()`（同一行換，行數不變）。
  - `cSecurity.cpp` 第 1283／1287 行（`SaveJamLevel` 寫 Include MTBA 框）→ `jamrules::HandlerIncludeMtbaKey(JamCode.c_str(), jamrules::IsMtbfAlias(CUSTOMER_CODE))`。
  - `cSecurity.cpp` 第 1474～1504 行 `TfSecurity::GetJemIncludeMTBA`（別名第 1483 行、其他第 1494／1498 行）→ 鍵＝`HandlerIncludeMtbaKey`、預設＝`jamrules::DefaultIncludeMtbaHandler(area, code, alias)`。
  - `WebSecurityJam.cpp` 第 472～500 行 `SnapGetJemIncludeMTBA`（背景匯出的複本；別名第 480 行）→ 同上，兩處一起改（檔頭第 278 行「golden 改了 getter 這裡要跟著改」）。
  - 第二個勾選框（原 W20-5＝A）：`WebSecurityJam.cpp` 第 82～96 行的 `kBox`、`D:\HT9045\web\page\ht9045_wire_statussecurity.js` 的 `BOXES`、`D:\HT9045\web\page\Status.Security.html` 第 56 行——用 `jamrules::IncludeMtbfKey`／`IncludeMtbaKey` 與 `DefaultIncludeMtbfAnalyzer`／`DefaultIncludeMtba` 當預設。
- **ctest `Jam_Rules`**（`tests/test_jam_rules.cpp`，`%TEMP%\ht9045_jam_rules_<tick>`，沙盒或接縫的路徑在 `D:\HT9045` 底下就拒跑；綠燈才刪）：
  1. 接縫；2. 規則表（兩個 golden 當 oracle）、別名、鍵名、A／B／C 的預設、開關目前＝A；3. ELA JamConfig＝W45 以前的本體（答案＋位元組）；
  4. 一個檔兩個讀者兩種順序（Handler 規則的讀者在測試裡用 TIniFile＋JamRules.h 組，ELA 用真的 JamConfig）：A 時 933 的檔案內容看誰先讀（把 golden 的衝突釘住，調查 §4.2 例 1）；B、C 時兩種順序位元組相同。
  CMake：`tests/CMakeLists.txt` St02 的 Security_JamMerge 區段後面一個新區段（`-Wall -Wextra`）。
- ⚠ 未上機（沒有行為改變要看）。

## ★W45（原 W20）Steven 0928 裁決後：一個 Jam Code 編輯器（20260928，St02-E helper）

**W45：18a／18b／18c＝Steven 0928（第 18 項）；其他照 golden，未明確裁決。**
分支 `v906/steven-w45-jam`（從 `v906/steven-gpib-widget` `233c5b95` 開）：程式＋ctest `6f1f8024`，文件是下一個 commit。兩組態只編譯（St02 不執行），0 錯；ctest `Jam_Rules`（擴充）待 St01。
調查：`D:\HT9045\.claude\skills\ht9045-st02-workflow\references\research-w20-jamcode-editor.md`（§6 加了「狀態」欄）。
golden：Handler＝906_0625_Steven（`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cSecurity.cpp`／`cSecurity.dfm`），分析器＝Rev891（`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp`／`Analyzer.dfm`）。
原則（Steven 0928）：客戶指定的功能照 golden 的格式；兩個 golden 不一樣時，Handler 擁有的資料（JAM0000.dat）照 Handler。

| 題 | 狀態 | V906 做了什麼 |
|---|---|---|
| W20-1 編輯器放哪 | **18a＝A**（Steven 0928） | 一個編輯器＝Status.Security 的 Jam Code 分頁（`data-t="10"`，WS `security.jam`）。`D:\HT9045\web\page\eventlog.html` 工具列加「Jam Code Setting」鈕（golden 分頁的名字，`Analyzer.dfm:1840`）：送 background.html 現成的 `{open:'security'}`（Main.html sbPassword 同一條路，守衛照走），等 security 視窗顯示、頁面載好再點 Jam Code 分頁；頁面單獨開（不在 background）就開新分頁 `Status.Security.html`。這頁自己不改任何東西。 |
| W20-2 誰能改 | 未明確裁決，照 golden | 兩個 golden 不同（Handler＝[29]＋Supervisor／HonPrec，分析器沒有密碼）→ Handler 的：`WebSecurityJam.cpp` `JamTabAllowed`（golden FormShow :301-380）不變，第二框在同一個閘後面。 |
| W20-3 IncludeMTBF 缺鍵預設 | **18b＝B**（Steven 0928） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JamRules.h` 一行開關 `kIncludeMtbfRule`＝B。ELA `JamConfig::GetJemIncludeMTBF`：933／967 缺鍵＝true（＝906_0625_Steven `cSecurity.cpp:1338`）——**只有 ELA 偏離** Rev891 `Analyzer.cpp:2846-2878`；`ela::Options::goldenJamMtbfDefault=true`＝golden（#23 對照用，預設 false）。Handler getter 不動（B 之下 Handler 的預設就是 golden 自己的）。客戶碼在 Hub ReadConfig 設（`Analyzer::SetCustCode`），開機查詢之前就跑（ElaHub.cpp `RunOnce`：EL_* 旗標先於排著的查詢）。 |
| W20-4 缺鍵寫回 | 未明確裁決，照 golden | 讀到缺鍵就寫預設（兩個 golden 相同，#17＝A）：不變。第二框也走 golden `CheckAndReadIniData`。 |
| W20-5 分析器多的那一格 | **18c＝A**（Steven 0928） | 每個鍵一個框：框 1＝golden `cbIncludeMTBA`（不動）；框 2＝分析器的 `cbIncludeMTBF`（`Analyzer.dfm:2029-2042`，讀 `Analyzer.cpp:2753`、存 `:2772`），綁「另一個鍵」——一般客戶 `<碼> IncludeMTBF`「Include MTBF」（golden 字樣）、預設＝分析器清單；933／967 `<碼> IncludeMTBA`「Include MTBA (Analyzer)」、預設＝JAM＋01～05（`:2820-2832`）。所以 18b 之後**每個鍵只有一個缺鍵預設**，誰先讀都一樣（ctest 第 5 節逐格驗）。 |
| W20-6 等級 | 未明確裁決，照 golden | 兩個 golden 不同 → Handler 的（4／5 格＋客戶鉗制）；分析器那組 4 格原始值不加。 |
| W20-7 訊息說明可不可改 | 未明確裁決；⚠ 待定 | 兩個 golden 都能改；V906 網頁在 W45 以前就唯讀（St01 的既有偏離，`WebSecurityJam.cpp:43`、JS :124）。W45 **沒動**：「照舊」＝唯讀、「照 golden」＝可改（要處理 Big5／cp949 韓文／RTF），交 St02-E／Steven 定。 |
| W20-8 CSV 欄 | 未明確裁決，照 golden | 匯入／匯出不加 MTBA／MTBF 欄（golden `spbImportClick`／`spbExportClick`）；第二框不進 CSV，匯入後重讀。 |
| W20-9 KYEC FTP 的鎖 | 未明確裁決，照 golden | 不動（golden 沒有鎖）；調查建議的 B（筆電包鎖）沒做、沒認領。 |
| W20-10 分析器產生的 section | 未明確裁決，照 golden | 編輯器只列 31 區（兩個 golden 都一樣）。 |

- **新檔（St02）** `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSecurityJamW45.h`（header-only，wb_serve 來源清單不變）：`jamw45::Load(pv)`／`Apply(k, it)`／`Save()`／`SaveThenLoad(pv)`／`WriteBox(w, pv)`；
  I/O 用 golden `common.cpp` 的 `CheckAndReadIniData`／`WriteIniData`（bool，同一個 INIFile 單例，值變了也寫設定變更紀錄），客戶碼每次 Load 讀 `CUSTOMER_CODE`；全部在頁面已經拿著的 JAM0000.dat 鎖裡（open :794、select／save／import :990）。
  它 include `JamIniMerge.h`，所以 `WebSecurityJam.cpp` 第 68 行換成 include 它，什麼都不少。
- **改的檔（St02）**：`JamRules.h`（開關＝B；`kSecondBoxId`／`SecondBoxKey`／`SecondBoxCaption`／`DefaultSecondBox`；`jamrules::SecondBox`＝虛擬勾選，Io 用樣板，ctest 用 TIniFile）、
  `EventLogAnalysis/ElaCore.h`／`.cpp`（`goldenJamMtbfDefault`）、`D:\HT9045\web\page\eventlog.html`（鈕）、`tests/test_jam_rules.cpp`。
- **第二框的存取順序（照兩個 golden）**：open／select／save 先讓 golden `ChangeJamMessage(false)` 讀 Handler 的鍵，再 Load 框 2（分析器 `ChangeJamMessage` 最後一行）；
  `ApplyValues` 收 `cbIncludeMTBF`（＝操作員點框）；save＝golden `SaveJamLevel` 之後 Save 框 2（分析器 `SaveJamLevel` 最後一行）；
  select 換區／碼／語言＝golden `cbJam*Change`（它先存 from 的 Handler 鍵、再顯示 to）之後 `SaveThenLoad`（框 2 存 from、讀 to）；都一樣＝不存（golden 選同一項不觸發 OnChange）。
  區或碼是空的就不讀不寫（`Analyzer.cpp:2759-2766`＝Handler `:1065`／`:1125`）。不看 FileExists（Handler 擁有的資料：Handler 自己的 getter／SaveJamLevel 剛寫過同一個檔）。
- **認領（都不是 St02 的檔；一行換一行、行數不變、程式碼都在 `//` 前面；逐行 OLD→NEW 在 St02-E 的交接回報）**：
  - `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSecurityJam.cpp`（St01 寫；手冊說已交出，但最後的 commit 沒標 St02 → 當別人的檔）：第 68、160、210、835、995、1004、1005、1013、1018、1023、1040 行。
  - `D:\HT9045\web\page\ht9045_wire_statussecurity.js`（St01）：第 130 行（`BOXES` 加 `cbIncludeMTBF`）、第 216 行（標籤文字用 C++ 回的 `caption`）。
  - `D:\HT9045\web\page\Status.Security.html`（St01）第 56 行：`cbIncludeMTBA` 的 label 後面插一個 `<label class="ckb" id="cbIncludeMTBF">`，left 244／top 280／277×17＝`cbContAlarmNotUpload`（244／304）上面那一列空位。
    分析器放在 MTBA 下一格（`Analyzer.dfm:2029-2033`），但 Handler 那一欄從 160 起每 24 格都有元件、304 會壓到 `spbImport`（535／317）。位置是頁面擁有者可以改的。
  - 選做（security.jam 端到端測試用）：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp`（St01）第 2085 行 `W906_SecurityJamBoot` 的 `FileNameJam000` 字面值改讀 `W906_JAM0000_PATH`（同一行）。golden `SaveJamLevel` 另外會寫 `D:\HT9045\Error\<語言>\<碼>.dat`，端到端仍要照 #33「備份→驗證→還原」。
  - 沒認領：`KYECFTP\*`（W20-9 照 golden）、`cSecurity.cpp` 的 getter（18b：Handler 不動）、`tests/CMakeLists.txt`（`Jam_Rules` 已註冊，不用改）。
  - **20260928 14:2x 已套上**（ST01-M 13:5x 同意，經 St02-M；St02-E 在 gpib-widget 自己一個 commit；OLD 行先對過 main 1818cfa4 與 review6；cSecurity.cpp:2085 選做那行沒套）。ctest：`Jam_Rules|ELA_|Security_JamMerge`（ST01-M 代跑）。
  - 認領套上之前：第二框不出現（C++ 不送 `boxes.cbIncludeMTBF`；JS 找不到元件就跳過），其餘不變；18b 與 eventlog.html 的鈕照樣有效。套了 JS 沒套 HTML（或反過來）也一樣安全。
- **偏離**：18b（ELA 對 933／967 缺 IncludeMTBF 寫 true；Rev891 寫清單值）；第二框 933／967 的標籤「Include MTBA (Analyzer)」（golden 沒有這個字樣；調查 §5.1 選項 A）；第二框的位置；eventlog.html 沒有自己的 Jam 分頁（18a）。
- **ctest `Jam_Rules`**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_jam_rules.cpp`；只寫 `%TEMP%\ht9045_jam_rules_<tick>`，沙盒或接縫在 `D:\HT9045` 底下就拒跑；綠燈才刪；第 6 節等鎖 5 秒一次）：
  1. 接縫；2. 規則表（兩個 golden 當 oracle）、A／B／C、**開關＝B**、B 不改 Handler 的預設；
  3. ELA `JamConfig` 對 W45 以前的本體：golden 模式＝完全相同（答案＋位元組），裁決＝只差 933／967 缺 IncludeMTBF＝true（2 模式 × 4 客戶 × 33 section × 12 碼 × 3 getter）；18b 真的生效（933 寫 1、868 寫 0）；
  4. 一個檔兩個讀者兩種順序：golden 模式 933 看誰先讀（衝突釘住）；**裁決（真的 ELA JamConfig）兩種順序位元組相同**；C 也相同；
  5. 第二框：鍵／標籤／預設；每格「兩框綁兩個不同的鍵」與「每個鍵一個預設」（4 客戶 × 33 × 12），golden 模式只剩 933／967 的框 1 不一致；
     `jamrules::SecondBox` 在檔上：Load 補寫、第二次 Load 不寫、別的元件名不收、Apply／Save、ELA 讀得到（868 的 MTBF、933 的 MTBA）、JSON（對的紀錄／過期紀錄）、換紀錄、空碼不讀不寫；
  6. ELA 933、另一條執行緒拿著鎖：回 true 但不寫；鎖放掉後寫 1（調查 §5.4 第 3 項）。
  沒有 ctest 能直接呼叫 `W906_SecurityJamOp`（在 wb_serve 的來源裡）：頁面那一段要 ST01-M 的 SIM 實跑（上面選做的接縫＋#33）。
- ⚠ **未上機**。上機要看：
  1. 一般客戶（非 933／967）：Security → Jam Code 分頁左邊多一個「Include MTBF」；勾一個碼、換碼或按 Exit → JAM0000.dat 該區有 `<碼> IncludeMTBF=1`；ELA 查詢那個碼的停機算進 MTBF（Fail）。
  2. 933（ASE_CL）：第二框標「Include MTBA (Analyzer)」、寫 `<碼> IncludeMTBA`；原本的「Include MTBA」寫 `<碼> IncludeMTBF`；新的 JAM0000.dat 上第一次 ELA 查詢寫的 IncludeMTBF 是 1（不是 0）。
  3. eventlog.html「Jam Code Setting」→ 開出 Password and Security、停在 Jam Code 分頁；等級不夠時頁面說 not-authorized。
  4. 改第二框後設定變更紀錄有一筆（common.cpp `WriteIniData`）。

## R5 ElaSchedule：依時間的 ELA 工作（20260927，St02-E helper；計畫 `D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md` §2／§3／§4 R5／§6）

分支 `v906/steven-elasched-wip`：`155e27dd`（程式＋ctest）、`08beb509`（合併 gpib-widget＝R1／R2）、`a6bc5a26`（`SchedSystemClock` 改名、`iN10UploadMethod` 夾值）、`4be12ec8`（合併 gpib-widget 57d66925＝R2 追加／R3／R4，並去掉重複的 `Fnv1a32`）、
`f08cea52`（接 R2／R4 本體的轉接 `ScheduleUseHubJobs`，見下）、O10 單一來源（見偏離 8）。兩組態只編譯（St02 不執行），0 錯、ElaSchedule.cpp 與測試開 `-Wall -Wextra` 0 警告；ctest `ELA_Schedule` 待 St01。
`ElaService.cpp`／`ElaHub.cpp` 沒改（接線在合併時由 St02-E 加，見下）。

- 新檔：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaSchedule.h`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaSchedule.cpp`（`ht9045_ela`；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CMakeLists.txt:1370`，`ElaTables.cpp` 下一行）、
  `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_ela_schedule.cpp`（ctest `ELA_Schedule`；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt` 在 `ELA_Reports` 區塊後、空一行）。
- **工作表**（`ela::JobId`；golden 行號：分析器 `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp`，Handler `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\`）：

| 工作 | golden 觸發 | V906 時段 | 閘（D-a／D-f／客戶碼） | 種類與錯開 W | 本體 |
|---|---|---|---|---|---|
| `JOB_O06_4` | Timer2 :3026-3040 | 沒勾 [O06-8]＝**不自動存**（**★W43＝C，Steven 0928**；W18 B 原本每 60 分，golden 每 1 分）；有勾＝10／30 分 | O10、`EnableAutoSaveProductiont`、[O06-8] `EnanleTimePeriodSaveLog`（自動才看，手動不看） | 本機不錯開；`[O06-6] Use Net Drive`（`bAlarmStatistAutoSaveNetDrive`，:3097）或遠端路徑＝300 s，上限半個週期 | R2 `O06SaveSummaryData` |
| `JOB_N10_3` | Timer2 :3042-3068 | 方法 0＝00:00、1＝08:00＋20:00、2＝每小時、3（指定時間）＝**每天 Handler 設的那一分鐘一次；Handler 關著錯過＝開機後補發一次**（★W44＝B＋**★W44-2＝補發**，Steven 0928，見下；原本照 golden 分析器夾成每小時） | O10、`bN10_DailyUploadProdData` | 上傳方法是網路磁碟（`rgN10_4` 夾值後＝1）且 `sN10UploadDrivePath` 遠端＝300 s，否則本機 | R2 `N10SaveSummaryData` |
| `JOB_N25_3` | `main.cpp:21442-21447`（在 `#ifndef SOFT_SIMULTE` :21425 內） | 每天 00:00 | 851、O10、`bN25_3_EnableULJamLog`、N25-2 主機非空；SIM 組態不自動 | FTP 900 s | R4 `UploadJamCode` |
| `JOB_O19_VTEST` | `HS_Function.cpp:257-272`（`CheckClockTrigger(60)` :4312-4328） | 每週 `iO19_WeekPeriod`（0＝週日）00:00 | 915／919、O10、`bO19_AutoRecordReportByEveryWeek`、星期 0～6 | 本機不錯開；`asO19_SavePath` 遠端 300 s | R2 `DoVTestSaveSummary` |
| `JOB_N17_PRODLOG`（W13） | `HS_Function.cpp:204-207` → `Command.cpp:12401-12447`（V906 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp:3608-3654`） | 每天 01:00 | `bN17UploadProdLog`（**不看 O10**：golden 是 Handler 自己的工作） | 本機不錯開；`asN17ProductionLogPath` 遠端 300 s | 本檔 `RunN17UploadProdLog` |

- **API**（`ElaSchedule.h`）：`IClock`／`SchedSystemClock`（時間＝本地秒，1899-12-30 起＝TDateTime×86400；**不叫 SystemClock**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaReports.h:45` 已有 R2 的 `ela::SystemClock`，同名兩個類別＝同一個 vtable 符號）；
  `JobFn`＝`std::function<JobStatus(JobContext&)>`；`JobContext` 帶 `slot`／`slotDateTime`（**資料範圍用它**，不用實際執行的時間）、`attempt`、`catchUp`、`manual`、`cfg`、`slotFlags`、`detail`（一行，不含密碼）。
  回傳 `JOB_VERIFIED`／`JOB_NOTHING_TO_DO`（記成完成）、`JOB_RETRY`（退避）、`JOB_FAILED`（不重試）、`JOB_NO_BODY`（R2／R4 還沒接：記一次 log、不記完成）。
  類別 `Scheduler`（ctest 用）；整個行程一份：`ScheduleInstall`／`ScheduleUninstall`／`ScheduleTick`／`ScheduleSetJob`／`ScheduleRunNow`／`ScheduleReloadConfig`／`ScheduleLog`／`ScheduleStatusJson`。
  每個工作自己的紀錄 `JobState::results`（最近 20 行，`StatusJson` 的 `jobs[].results`，給 R6）；**不進**頁面的查詢快照。
  純函式：`Fnv1a32`、`StaggerKey`、`StaggerOffsetSec`、`BackoffBaseSec`／`BackoffDelaySec`、`IsRemotePath`、`O06PeriodMin`、`N10EffectiveMethod`、`ResolveKind`、`StaggerWindowSec`、`JobEnabled`、
  `CustomerCodeAllows`（給 R3／R4：JAMWEEK／SUMMARY／EVENTLOG＝851、CHIPADV_LOTEND＝868、VTEST_MTBF_SUM＝915／919）、`JobCalendar`、`N17FileName`、`CopyFileAndVerify`。
- **設定**（`ReadScheduleConfig`，**唯讀**，從不寫鍵）：config.ini `[Event Log]`／`[FTPUpLoad]`／`[ChipMos Function]`／`[Production_Log]`，Gerneral.ini `[System] CUSTOMER_CODE`／`SPIL_FOR_QLE`、`[Version] Model`／`Machine ID`；
  每 60 秒重讀（`ScheduleReloadConfig` 下一次 Tick 就重讀）。分析器擁有的鍵用分析器的預設（`bN10_DailyUploadProdData` 缺鍵＝true），Handler 擁有的觸發用 Handler 的預設（`iO19_WeekPeriod` 缺鍵＝0）。
  radio group 的值照 VCL 夾（`TCustomRadioGroup::SetItemIndex`）：`rgN10_3_1` 三項（`Analyzer.dfm:2765-2768`，3 以上＝2）、`rgN10_4` 兩項（`Analyzer.dfm:2732-2734`，2 以上＝1），跟 R2 的 `RadioIndex`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaReports.cpp:40-45`）一樣。
- **錯開**：offset＝FNV-1a-32(Machine ID) mod W；Machine ID 空的或 "HT-90xx" 就用主機名。例：PJLD1014 → FTP 607 s、網路磁碟 7 s。
- **退避**：第 n 次等 min(60·2^(n-1), 1800) s ×(0.8＋0.4u)，u＝fmix32(FNV-1a-32(Machine ID＋"#"＋n))／0xFFFFFFFF（FNV-1a 最後一個 byte 幾乎不動高位，不加 fmix32 的話每個 n 的 u 幾乎一樣）；
  最多**重試 6 次（共 7 次嘗試）**，或下一個表定時段先到就放棄（新時段重新開始）。PJLD1014：54／127／198／520／997／2120 s。
- **記錄（不需裁決，St02-E 20260927）**：第一次開機不補跑＝A（沒有紀錄＝不知道有錯過，從現在開始記），這是我們對 D-b 的讀法；
  退避＝**重試 6 次＝共 7 次嘗試**（等 1／2／4／8／16／30 分），計畫「6 次後放棄」的意思。
- **開機補跑（D-b）**：狀態檔 `<log root>\UploadFile\ElaScheduleState.ini`（`[工作名]`、`done=`＝最後驗證成功的時段、`armed=`＝V906 第一次看到這個工作開著的時間；先寫 .tmp 再換名）。
  開機時最近一個時段晚於 done／armed ⇒ 補跑**一次**（開機＋offset），只補最近那一個；驗證成功才更新 done，失敗照退避。
  N10-3 方法 3（指定時間）也照 D-b 補：**★W44-2＝補發（Steven 0928）**（原本是例外、不補跑——★W44＝B 時的讀法，golden 也沒有補跑），見下。
  沒有任何紀錄（第一次）⇒ 不補跑、寫 armed（不知道有沒有錯過）。執行中才打開的工作不補已經過去的時段。
- **FTP_Log**：`ScheduleLog`（沒裝排程時是 no-op）寫 `<log root>\UploadFile\yyyy\mm\FTP_Log_yyyymmdd.csv`（golden TMyStringList 的表頭 `Date, Time, Action, S2, S3, S4, S5, S6`，
  行＝`yyyy-mm-dd, hh:nn:ss.000, Schedule, <工作>, <事件>, <時段>, <說明>`，CRLF；`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:220-224`、`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\MyStringList.cpp:526`／`:642`）。
  log root＝`W906_HT9045LOG_ROOT`（as9045LogPath 接縫，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp:240`），沒設＝golden `D:\HT9045_Log`。沒有帳密（`ScheduleConfig` 不帶）。
  傳輸層的 FTP_Log 是 R4 的（R1 的 `MyDBIProcess`／`SetDbiProcessHook`）；兩者都在 hub worker 上，一次一個；合併時可用 `ScheduleSetup::logSink` 改走同一個寫檔。
- **偏離**（W18 B／W21／D-b 之下的細節）：
  1. **每個工作自己決定**（自己的開關、週期、錯開）：golden 用一個 `bNeedUpload`（:3071-3083），O06 或 N10 任一個時段到了兩個存檔都跑——W18 B 拆開（St02-E 20260927）；`keepGoldenBugs=true` 的 golden 模式保留互相觸發。
  2. O06-4 沒勾 [O06-8]＝**不自動存**（**★W43＝C，Steven 0928**，見下；golden 每分鐘＝G3，`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:3014` 註解寫「預設一小時一次」；W18 B 原本改成每小時）；R2 的 `Timer2Due` 也一樣。`keepGoldenBugs=true` 回到每分鐘。
  3. W21／D-b：錯開、退避、驗證成功才算、開機補跑（golden 全都沒有，計畫 §5）。
  4. 時間點：golden 50 秒計時器＋分鐘鎖，開機那一分（`CurrentNN=1`）不存；Handler 觸發在 00:00:01～04（N25-3 最多送 4 次、分析器合併）、00:00:04（VTEST）、01:00:04（N17）。V906 在表定整點＋offset，一個時段一次。
  5. VTEST：golden 邊緣觸發，開機晚於那天 00:00:03 當週就沒了；V906 由開機補跑補上。
  6. N17：golden 在 Handler 的 `TimerAutoBackupTimer`（`InitialOK==false` 或 ATC 有警報時整段跳過），失敗彈 `ShowMyMessage`、成功 `RecordProcess`；
     V906 在 ElaHub（`HT9045_ELA=0` 時不跑），寫 FTP_Log 與歷史。目標資料夾不在＝重試（golden 彈窗一次）；目標已在且同大小＝成功（重試時）；
     不同大小＝不覆寫、不重試（golden 也是失敗）；同一個時段我們自己上一次寫壞的可以覆寫。
  7. N17 的 SPIL 檔名：`bSPILFunction` 不在任何 ini（CosFunction 客戶表）；N17 只有 SPIL 或 CC_QUALCOMM 999 能開（`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cConfiguration.cpp:3576-3590`，其他固定 0），所以「N17 開而且不是 999」當成 SPIL。
  8. **O10 只有一個來源**（St02-E 20260927 更正；原本 R2 `AutoJobsEnabled` 與 R5 都只看 `EnableAutoSaveEventLog`＝`ElaConfig::cbO06`）：`ela::EffectiveO10`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaReports.h`／`.cpp`）。
     golden `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cprod.cpp:2379-2394`：Configuration 頁先讀存起來的 `bO10UseEventLogSaver`（`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cConfiguration.cpp:3978-3980`，`ReadLastSetIni` :2966），
     接著 `ProcessLastSetIni_EventLog`（:3019）在 `if(IniConfig.bEventLogAutoSaveFunction)` 裡把它改成 `bO06_EventLogAutoSave`；V906 同一行在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cprod.cpp:2519`。
     ⇒ 有效 O10＝`bEventLogAutoSaveFunction ? EnableAutoSaveEventLog : 存起來的 bO10UseEventLogSaver`。
     - **Handler 的即時值優先**：wb_serve 在靜態初始化時裝 `ela::SetEffectiveO10Getter`（回傳 `IniConfig.bO10UseEventLogSaver`，已經過 :2519 的覆寫），**每次判斷都呼叫**、不快取（Configuration 頁執行中可以改 O10）。
       安裝處 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_TesterIF.cpp:293`（同一行接在 `g_tifGpibAuxHook` 後面，行數不變）：FileRW 只編進 wb_serve，wb_serve 連 `ht9045_ela`；
       不選 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Handler\TesterCommWiring.cpp`：它在 `ht9045_testercomm_handler`，`test_testercomm_handler`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt:3343-3347`，呼叫 `W906_TesterCommInit`）連這個庫但不連 `ht9045_ela`，會缺符號。
     - **沒裝（ctest、別的程式）**：公式 `EffectiveO10FromIni(functionFlag, …)`，旗標由 `SetO10FunctionFlagFallback` 注入，預設 true。**不抄客戶表。**
     - 查到的事實：兩棵樹都是 `InitialCosFunction` 先把 `bEventLogAutoSaveFunction` 設成 true 給**每一個**客戶（`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\CosFunction.cpp:3894`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CosFunction.cpp:4111`；
       `CustomerFunctionSelect` 先呼叫它，`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cprod.cpp:3654`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cprod.cpp:3901`），之後各客戶函式只會再設 true（V906 :380／:407／:451／:540／:619／:655／:1004／:1124／:1233／:1367／:1398／:1598／
       :2437／:2791／:2822／:2957／:3194／:3326／:3520／:3645／:3683；:2185／:2622／:2725 是註解），**兩棵樹都沒有地方設 false**。所以現在所有客戶的 O10 都等於 `EnableAutoSaveEventLog`；
       本機 `D:\HT9045\config\config.ini:3` `EnableAutoSaveEventLog=0`、`:41` `bO10UseEventLogSaver=1` ⇒ golden 的有效 O10 是**關**（勾選框存的 1 在讀設定時被蓋掉）。
     - 用到它的：R2 `DoVTestSaveSummary`（`Hub::RunVTest`）、R3 `Hub::RunChipAdv`、R4 `ElaChipMos` 兩個上傳（`AutoJobsEnabled`）、R5 `JobEnabled`（每次 Tick）與 `RunN25OnHub`。
       `ElaConfig::o10Stored`（`Hub::ReadConfig` 唯讀讀 `bO10UseEventLogSaver`，golden 分析器不讀這個鍵所以不寫回）；`ScheduleConfig::o06_1`／`o10Key`。
  9. 手動（`ScheduleRunNow`）：不看 O10 與時間（D-a：網頁的操作不受影響），看功能開關與客戶碼；不記成完成、不重試。
  10. SIM 組態：N25-3 不自動（golden `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\main.cpp:21425`），手動可以；VTEST／N17／O06／N10 golden 沒有 SIM 分支。
  11. O06-4 的網路磁碟錯開：300 s（St02-E 20260927，計畫原本寫 min(週期/2, 120 s)），但不超過半個週期（golden 每分鐘模式＝30 s），否則每次都排到下一個時段之後、被下一個時段取代。
- **★W44＝B，Steven 0928**（N10-3 方法 3「指定時間」＝照 Handler 設的時間每天上傳一次；原本是加掛點）：`59a7dcc7`，AI(W906-ELA-W44) 20260928，St02-E helper。
  - **golden**：Handler `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\HS_Function.cpp:488-509`（`TFormHS::TimerAutoBackupTimer`）：`bN10_DailyUploadProdData && iN10UploadProductMethod==3` 時，
    `DecodeTime(IniConfig.dN10_3_1_SpecifiedTime)` 的時、分等於現在的時、分，而且 `bN10_3_1_Flag==false` ⇒ `UpDataToServer_KYEC(5)`、旗標設 true；分鐘不一樣就清旗標（`:505-508`）。
    ⇒ 那一分鐘裡只跑一次；**沒有補跑**（那一分鐘 Handler 沒開＝那天就沒有）；旗標只在記憶體（`HS_Function.cpp:59`、`HS_Function.h:82`）。
    分析器 `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:3042-3068` 只有 0／1／2，方法 3 被 `rgN10_3_1` 夾成每小時（`:3112` 讀 `iN10UploadProductMethod`，不讀時間）。
  - **設定鍵**（查到的事實）：
    - `[FTPUpLoad] bN10_DailyUploadProdData`＝`IniConfig.bN10_DailyUploadProdData`（golden `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cConfiguration.cpp:3305`／`:3369`；分析器 `Analyzer.cpp:3111`）。
    - `[FTPUpLoad] iN10UploadProductMethod`＝`IniConfig.iN10UploadProductMethod`（golden `cConfiguration.cpp:3306` 固定值那組、`:3346` 讀檔那組；分析器 `:3112`）。
    - 時間＝`IniConfig.dN10_3_1_SpecifiedTime`（`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\Config.h:1030`，double＝TDateTime）：**golden 從來不讀也不存**。
      畫面的 `dtpN10_3_1_SpecifiedTime`（`cConfiguration.dfm:15395`、`cConfiguration.h:2160`；設計時的值 40162.5441927662＝13:03:38）沒有任何 `elConfig->Add`；
      整棵樹只有 `HS_Function.cpp:492` 讀這個欄位；`IniConfig` 是全域變數（`cprod.cpp:78` `HT9045_CONFIG IniConfig;`，沒有建構子）⇒ 永遠 0.0＝**00:00**。912、899 兩棵樹一樣。
    - V906：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Config.h:1031` 有欄位；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc:6272`／`:6312` 只綁 `iN10UploadProductMethod`；
      頁面 `D:\HT9045\web\page\Setup.Configuration.html:57`、`D:\HT9045\web\page\Config.Configuration.html:58` 只顯示 dfm 的固定值、沒接；
      ELA 已讀的：`ElaSchedule.cpp` `ReadScheduleConfig` 的 `bN10_DailyUploadProdData`／`iN10UploadProductMethod`，`ElaHub.cpp:421`（`rgN10_3_1`）。
  - **做法**：`ReadScheduleConfig` 多讀 `[FTPUpLoad] dN10_3_1_SpecifiedTime`（唯讀；這是 Handler 的 HTEditList 會給這個欄位的名字：鄰居的 section、欄位名當 key；**★W44-1＝A（Steven 0928）**：接 picker 的方式已定——golden HTEditList 的 TDateTimePicker 支以 ECDouble 存 `[FTPUpLoad] dN10_3_1_SpecifiedTime`＝FloatToStr（例 0.3125＝07:30），ELA 讀法不變；C-route 產生器那一行由 St01 加（tools/editlist/IniConfig.py 的 replace＋重產 IniConfig.gen.inc）；存檔送 EL_UPDATE_PARAMETER 後 ELA 立刻重排（ScheduleReloadConfig）。
    FloatToStr＝15 位有效數字（golden `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\Public\HTEditList.cpp:925`、V906 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Public\HTEditList.cpp:979-988`），不是 %0.4f：00:11＝0.00763888888888889 讀回 00:11（%0.4f 的 0.0076 會變 00:10）——ctest 第 19 節的 00:11 那一筆鎖這個格式；
    重排＝`ElaHub.cpp` `Hub::RunOnce` 的 EL_UPDATE_PARAMETER 在 `ReadConfig();` 同一行加 `ScheduleReloadConfig();`（沒有排程時什麼都不做），`d133b136`，AI(W906-ELA-W44-1) 20260928，St02-E helper），
    `TIniFile::ReadFloat`，只取一天裡的時、分（golden DecodeTime）→ `ScheduleConfig::n10SpecifiedMinute`；**缺鍵（現在所有機台）＝0.0＝00:00，跟 golden Handler 實際的行為一樣**。
    `AddN10Rules`：方法 3 → 每天那一分鐘一次（`N10SpecifiedTimeRule`）；手動組的 ScheduleConfig 留 -1＝分析器的每小時。
  - **不重複**：一個時段只跑一次；驗證成功記在 `ElaScheduleState.ini` 的 `done=`，同一天重開機（在那一分鐘內或之後）不再跑。
  - **★W44-2＝補發（Steven 0928）**（原本這裡寫「錯過的時間不補跑＝Steven 的裁決」；Steven 0928「W44-2 要補發」推翻）：開機時那天的時間已經過了一分鐘以上＝開機後補發**一次**（D-b），FTP_Log 記 `boot catch-up`（`catch-up for yyyy/mm/dd hh:mm`），隔天照常；見下「★W44-2」。
    開機時還在那一分鐘內＝照 golden（下一個計時器 tick 就對上）跑一次，不算補跑。
  - **改時間**：`ReadConfigNow` 看到方法或時間改了（新舊有一個是指定時間）就讓 N10-3 重新定位：新的時間今天已經過了＝今天不跑、明天才跑；還沒到＝今天到了就跑；
    剛好在那一分鐘內＝馬上跑。golden 也是拿「現在」跟「目前的設定」比（`HS_Function.cpp:494-497`）。
  - 資料範圍照原本：那個時段那一天的 00:00:00～23:59:59（golden Timer2 :3073-3079）——00:00 那一次存的是剛開始的那一天（方法 0 也一樣，golden 也是）。錯開與退避照 R5（網路磁碟 300 s）。
  - 沒動：R2 `ElaReports.cpp` 的 `Timer2Due`（只有 ctest 用，排程不走它）方法 3 仍是每小時——ElaReports 現在 W42 在改，要改再說；Handler 自己的 `UpDataToServer_KYEC(5)`（`forms/fHS.*`，GATE，不是 St02 的檔）沒翻。
  - ctest `ELA_Schedule`：第 5 節（方法 3 的行事曆）、第 14 節（缺鍵＝0）、**第 19 節**（假時鐘：之前不跑、07:30 一次、當天不再、隔天再一次；同一天重開機；錯過的開機補發一次（★W44-2，原本「錯過不補」）；開機在那一分鐘內；執行中改時間四種；讀鍵五種（含 ★W44-1 FloatToStr 的 00:11）＋缺鍵＝每天 00:00）。
  - ⚠ 未上機。上機要看：CC_CYUEAN（868）機台 `iN10UploadProductMethod=3`、O10 開：沒有 `dN10_3_1_SpecifiedTime` 鍵時每天 00:00（＋錯開）只存一次 N10 摘要（`sN10UploadDrivePath`，或 `D:\HT9045_Log\EventLogSummary\yyyymm\`）；
    FTP_Log 的 N10-3 一天一行 verified；重開機不重跑；Handler 關著錯過的那天開機後補發一次（FTP_Log `boot catch-up`，★W44-2）。畫面上的時間要真的生效，要有人把 `dtpN10_3_1_SpecifiedTime` 接到 `IniConfig`／config.ini（Handler 的檔，不是 St02 的）。
  註：R2 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaReports.h:111-114` 的註解寫方法 3「never」，但它的程式（`ElaReports.cpp:293` `RadioIndex`）也是夾成每小時——只有註解過期（★W43 那次已改正註解）。
- **★W43＝C，Steven 0928**（「W43 就按照你的建議做吧」；之前問「沒勾不是就不做嗎?」「這功能是不是跟「Auto save event log」有衝突?」；St01 09:1x 轉達）：`b11a7a20`，AI(W906-ELA-W43) 20260928，St02-E helper。
  分析器跟 Handler 走：[O06-8]「Update Production Record every 10 / 30 minutes」**沒勾＝不自動存 SummaryData**；勾了＝每 10／30 分。取代 W18 B 的「沒勾＝每小時」（G3 那列、R2 偏離 2、上面偏離 2 已改寫）。
  - **設定鍵**（查到的事實，golden＝906_0625_Steven `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven`）：
    - 畫面：`cConfiguration.dfm:13713-13737`，勾選框 `chkO06TimePeriod`（字「[O06-8] Update Production Record every ___ minutes」）＋下拉 `cbO06TimePeriod`（10／30）。
    - `cConfiguration.cpp:3950`：`chkO06TimePeriod` → `IniConfig.bO06SaveLogTimePeriod`＝`[Event Log] EnanleTimePeriodSaveLog`（ECBool，預設 0）；
      `:3960`：`cbO06TimePeriod` → `IniConfig.iO06SaveLogTimePeriod`＝`[Event Log] TimePeriodSaveLog`（ECInteger，0＝10 分、1＝30 分，JerryYang 20151026）；功能旗標 `bEventLogAutoSaveFunction` 關時 `:3966` 固定 0、不顯示。
      欄位 `Config.h:1347-1348`。V906：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cConfiguration.cpp:2977`／`:2987`／`:2993`、`FileRW\IniConfig.gen.inc:6919`／`:6929`／`:6935`、`Config.h:1348-1349`。
    - 分析器讀同兩個鍵：Rev891 `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:3098-3099`；V906 `ElaHub.cpp:408-409`（頁面設定）、`ElaSchedule.cpp` `ReadScheduleConfig`（排程，唯讀）。
  - **Handler 自己怎麼用**（golden）：`main.cpp:31052-31055` `TfMain::TimerESDTimer`（`main.dfm:17313-17317` 沒設 Interval＝TTimer 預設 1000 ms）只在勾了時數秒，
    到 `iO06SaveLogTimePeriod*1200+600` 秒（600＝10 分、1800＝30 分）寫一行 Production record（`ProductionLog(str, true)`，`<asProductionLogPath>\<ID>_yyyymmdd.logs`）；
    沒勾時 `cpublic.cpp:598` 的 `ProductionLog` 直接 return（整個 Production record 都不記），`main.cpp:9411` 開機讀當天 .logs 也只在勾了時。⇒ 沒勾＝Handler 沒有週期紀錄；分析器照這個。
  - **做法**：`ElaSchedule.cpp` `O06PeriodMin`：沒勾＝0（沒有時段，`AddO06Rules` 不加規則、錯開窗 0）；`JobEnabled` 的 O06-4 **自動**執行多一道閘，原因
    `O06-8 off ([Event Log] EnanleTimePeriodSaveLog): no timed SummaryData save (W43 = C, as the Handler)`（Auto Jobs 分頁看得到；英文，★W42-a）；
    `JobEnabled` 多一個尾參數 `keepGoldenBugs`（預設 false，Tick 傳 `ScheduleSetup::keepGoldenBugs`）；R2 `ElaReports.cpp` `Timer2Due` 同規則（`iCheckInterval` 0＝不觸發）。
    勾了＝照 golden 分析器每 10／30 分、對齊整點（`SystemNN % 10／30`；Handler 自己是開機起算的秒數，兩者本來就不同步）；不合法的 `TimePeriodSaveLog`＝30 分（分析器 TComboBox；Handler 是 `idx*1200+600` 秒）。
    `keepGoldenBugs=true`＝golden 分析器每分鐘。**手動「立即執行」不受 [O06-8] 影響**（它不是定時存檔；St02-M 0928 同意）；`EnableAutoSaveProductiont` 關時手動也照舊擋。
  - **手動**（Auto Jobs 分頁「立即執行」、`POST /api/ela/job?id=O06-4`）照常可以跑：不是週期存檔；O06-4 本身（`EnableAutoSaveProductiont`）關著照樣擋。
  - ctest：`ELA_Reports` 第 5 節（沒勾＝一天 1440 分鐘都不觸發）；`ELA_Schedule` 第 3 節（閘＋原因、手動可以、golden 模式開）、第 4 節（週期 0、沒有錯開窗）、第 5 節（沒有時段；10 分＝10:30 之後 10:40），
    第 10／13／17／18 節改成勾 [O06-8]（17 節 30 分：10:00 跑、O10 關的 10:30／11:00 跳過、11:30 跑；18 節 `next`＝00:30），**第 20 節**（假時鐘一整天：沒勾 0 次、10 分 144 次、30 分 48 次；執行中勾上＝下一個時段起跑、已過的不補；再取消＝停；手動照跑）。
    另：第 0 節接縫沒生效就 exit 2、什麼都不跑（containment first，同 `tests/test_tcp_cmd_server.cpp`，`b12ab375`）。
  - ⚠ 未上機。上機要看：O10 開、O06-4 勾、[O06-8] 沒勾：`AutoSaveProductionPath`（預設 `D:\RMS`）不再每小時出現新的 `<ID>-SummaryData_<日期>.txt`（只有手動）；Auto Jobs 分頁 O06-4＝off、原因 O06-8；
    勾 [O06-8] 10 分：每 10 分（整點對齊）一次，FTP_Log 一行 verified；Configuration 頁面改了之後 60 秒內（或 EL_UPDATE_PARAMETER）照新值。
- **★W44-2＝補發，Steven 0928**（「W44-2 要補發」，St01 09:1x 轉達；推翻上面 ★W44 B 的「錯過不補跑」，也跟 golden 不同——golden 沒有補跑）：`50c7d02e`，AI(W906-ELA-W44B) 20260928，St02-E helper。
  - **規則**＝D-b，不另訂：開機時（第一個 Tick）指定時間最近一次已經過了一分鐘以上、而且晚於 `done=`／`armed=` ⇒ 開機＋offset 補發**一次**；資料範圍＝那一次時間的那一天（00:00:00～23:59:59，同其他工作）。
    - 只補**最近那一次**，不補好幾天（D-b 原本就是這樣）：例 10-02 做過、10-03 與 10-04 關機、10-05 06:00 開機 ⇒ 06:00 補 10-04 那一次，10-03 不補，07:30 照常跑 10-05 的。
    - 開機時今天的時間還沒到 ⇒ 補的是昨天那一次（昨天的資料），今天的時間到了再照常跑——兩個不同的時段，不算同一天重跑。
    - 開機時還在那一分鐘內＝照 golden 的下一個 tick 跑，不算補發（同 ★W44 B）。
    - 一天一次：驗證成功才寫 `done=`，同一天再開機不重跑；補發失敗照退避（1／2／4…分），下一次指定時間先到就放棄。
    - 沒有任何紀錄（第一次）⇒ 不補、寫 `armed=`（D-b 第一次的規則）。**只在開機**：執行中把時間改到已經過去、或執行中才打開＝不跑（★W44 B 的重新定位不變）。
  - **FTP_Log**：`N10-3, boot catch-up, <那次時間>, catch-up for yyyy/mm/dd hh:mm: the specified time passed while the Handler was off (W44-2); last verified …; runs at …`，
    跑完再一行 `N10-3, attempt 1 (boot catch-up): verified, <那次時間>, …`；不再記 `missed`。
  - 程式：`ElaSchedule.cpp` `Scheduler::Tick` 方法 3 的分支（原本記 `missed` 的那段），`AddN10Rules` 與 `ElaSchedule.h` 檔頭的註解。
  - ctest `ELA_Schedule` 第 19 節：錯過那天＝開機（08:00）補一次、FTP_Log 兩行、沒有 `missed`、隔天 07:30 照常；停兩次只補最近那次（06:00 補 10-04、10-03 不補、07:30 照常、再開機不重跑）；沒紀錄不補（armed）。
  - ⚠ 未上機。上機要看：CC_CYUEAN（868）機台方法 3、O10 開，指定時間前關機、過了時間再開：開機後（本機立刻、網路磁碟最多 300 s）存一次那天的 N10 摘要，FTP_Log 有 `boot catch-up`＋`catch-up for …`；同一天再開機不重跑。
  - 還沒定（Steven 未答）：要不要請 Jimmy 在 BCB 912 補同樣的東西——不是 St02 的檔。
- **R2／R4 本體的轉接**（`ElaSchedule.h:292-305`，`ElaSchedule.cpp` 檔尾；**不用改 ElaHub／ElaService**）：`RunO06OnHub`／`RunN10OnHub`／`RunVTestOnHub`（R2 的
  `O06SaveSummaryData`／`N10SaveSummaryData`／`DoVTestSaveSummary`）與 `RunN25OnHub`（R4 的 `UploadJamCode`），`ScheduleUseHubJobs(hub, opt)` 一次登記四個。
  只用 Hub 的公開介面 `Hub::Config()`（EL_UPDATE_PARAMETER 讀的設定）與 `Hub::GetFtpFactory()`；每次用**自己的 Analyzer**（工作不換頁面快照，R2 追加的規則）、
  **固定在時段的 Clock**（資料範圍、報表日期、N25-3 BackUp 檔名都從時段來）、頁面的預設資料夾與篩選（golden Timer2 用的是視窗當時的篩選——偏離，影響 Top5／By Filter 那幾段）。
  結果：R2 的檔案在、不是空的＝`JOB_VERIFIED`；開關開著但資料夾／檔案失敗＝`JOB_RETRY`；R4 `N25Result::ok()`＝`JOB_VERIFIED`、`HadUpload.txt` 已在或只寫 log 的傳輸＝`JOB_NOTHING_TO_DO`、
  可重試＝`JOB_RETRY`、閘擋下＝`JOB_FAILED`。手動的 N25-3 傳 `o10=true` 給 R4（手動不看 O10）。
  N25-3 沒有 `FtpAbortSlot`（Hub 的是私有的）：`Hub::Stop` 會等傳輸自己的逾時（連線 5 s、傳輸 30 s），不會中途取消。
  工作結果在 `ScheduleStatusJson()` 的 `jobs[].results`，**不在** `Hub::Jobs()`（R6 兩個都要顯示，或接線時再加）。
- **跟 R4 重疊的地方（接線時二選一）**：R4 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaFtp.h:171-187` 也有 `StaggerOffsetSeconds`、`BackoffSeconds`（±20% 用 `Fnv1a32(id#n) % 20001`）、
  `kMaxUploadRetries`、`UploadRetry`／`OnUploadFailed`；`Hub::RunUploadJob`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaChipMos.cpp:395-397`）只算出下次時間、寫「R5 schedules it」，
  不會自己重試，所以現在不會重複重試。建議由 R5 退避（排程的工作），R4 的 `n25Retry_` 只當紀錄或拿掉。
  `Fnv1a32` 兩邊原本各有一份（同名同參數、回傳型別不同＝同一個符號），合併時拿掉 R5 的、用 R4 的（演算法相同）。
- **接線（合併時由 St02-E 加；四行，這批 commit 沒動 ElaService.cpp／ElaHub.cpp）**：
  - `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaService.cpp:167`（`ela::SetDbiProcessHook(&ela::FtpLogDbiLine);`）之後、`:173` `if (!g_hub->Start())` 之前：
    `ela::ScheduleInstall(ela::ScheduleSetup());` 與 `ela::ScheduleUseHubJobs(g_hub, o);`
  - `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaHub.cpp:566`（`Hub::Entry` 迴圈的 `h->RunOnce();`）之後：`ela::ScheduleTick();`
  - `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaService.cpp:186`（`W906_ElaStop` 的 `h->Stop();`）之後、`:187` `delete h;` 之前：`ela::ScheduleUninstall();`（本體拿著 hub 指標）
  - 可選：`EL_UPDATE_PARAMETER`（`Hub::RunOnce`）加 `ela::ScheduleReloadConfig()`（不然 60 秒內重讀）；`/api/ela` 的狀態接 `ela::ScheduleStatusJson()`（R6）。
- **ctest `ELA_Schedule`**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_ela_schedule.cpp`）：假時鐘、不 sleep；只寫 `%TEMP%\ht9045_ela_sched_<tick>`（在 `D:\HT9045*` 底下就拒跑；`_putenv` 三個接縫指進去）。
  第 0～15 節（第 19 節＝★W44／★W44-2、第 20 節＝★W43，見各段）：接縫、時間、FNV／錯開／退避數值、閘、種類與 W（含 `[O06-6]`）、行事曆（含方法 3 夾值與 ★W44 的每天指定時間）、O06／N10（golden 每分鐘＋互相觸發 vs W18 B）、N25-3 在 00:10:07、SIM 不跑、VTEST 每週一次（915／919）、
  N17 真的複製＋資料夾不在先失敗再成功、CopyFileAndVerify 各情況、O10 關、開機補跑（停機兩天／三週／沒紀錄／上次失敗）、退避時間與放棄、下一時段截斷、工作自己的紀錄、手動、沒有本體、FTP_Log 檔、
  整個行程的入口、ReadScheduleConfig 唯讀、第 16 節轉接（%%TEMP%% 的 Hub 上跑 R2 的 O06：時段那天的摘要檔在、不是空的＝verified；五個工作都有本體）、真的 config.ini／Gerneral.ini／狀態檔沒被碰。期望值先用 Python 鏡像（同一套演算法）算過。
- ⚠ 未上機。上機要看：開機時 FTP_Log 每個工作一行 on／off（原因）；N17 在 SPIL 機台 01:00 的檔名與複製；851 出貨組態 N25-3 在 00:00＋offset 只跑一次；
  重開機後 `ElaScheduleState.ini` 的內容與補跑；網路磁碟斷線時的重試間隔；O10 跟 Configuration 頁面改的即時值走（改了之後下一個時段就照新的值；偏離 8）。

## R6：eventlog.html 的自動工作分頁＋OEE 圖的設計（20260928，St02-E helper；計畫 `D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md` §4 R6）

分支 `v906/steven-elasched-wip` `353c0c6e`。兩組態只編譯（St02 不執行），0 錯；ctest `ELA_Schedule` 第 18 節待 St01；頁面 `node --check` 過。

- **頁面** `D:\HT9045\web\page\eventlog.html`：新分頁「Auto Jobs」（在 By Filter 與 Log 之間）；原本的查詢分頁、查詢列、版面與樣式都沒動。工作結果只在這個分頁，**不換掉使用者自己的查詢畫面**（R2 追加的規則）。
  - 排程表（R5）：每個工作一列——狀態（開／關＋關的原因，例如 O10、開關、客戶碼、SIM 規則）、種類與錯開、上次執行時間（手動有標）、結果
    （verified／nothing to do／failed (retry)／failed (not retried)／manual refused／no job body，顏色分）、說明、下次執行、嘗試次數（n／7）、最後完成的時段、「立即執行」鈕。
    點一列看那個工作自己的紀錄（`results`，最近 20 行）；下面是全部的排程歷史。
  - EL_* 工作紀錄（Hub，新的在上）：快照的 `jobs`（`ela::JobRecord`：時間、指令、有沒有跑、有沒有寫檔、檔案、查詢區間、說明）。
  - 更新：快照照舊每秒輪詢（`?since`）；排程狀態 `GET /api/ela/schedule` 在這個分頁每 2 秒、其他時候每 10 秒。
- **路由**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaService.cpp` `ela::ServeHttp`，St02 的檔；wb_serve 已經把 `/api/ela` 底下全部交給 `W906_ElaHttp`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:2868`，**不用改**）：
  - `GET /api/ela/schedule` → `{installed, build (sim／ship), ftp (log-only／WinINet), o10, o10Source, schedule}`；`schedule`＝`ela::ScheduleStatusJson()`（沒安裝＝null）。
    **不放進快照**：排程會在 Hub 的 seq 不動時改變，放進快照的話 `?since` 會把它藏起來。
  - `POST /api/ela/job?id=<O06-4｜N10-3｜N25-3｜O19-VTEST｜N17-UploadProdLog>` → `ScheduleRunNow`：202（下一個 tick 手動跑一次）、400 id 錯、403 唯讀伺服器、503 排程沒安裝。
- **排程狀態多的欄位**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaSchedule.h` `Scheduler::StatusJson`）：最上層 `now`、`maxRetries`；每個工作 `next`（等重試時＝重試時間，否則＝下一個時段＋offset，關著＝"-"）、
  `retries`、`lastAt`／`lastStatus`／`lastDetail`／`lastManual`（手動被擋也記：`manual refused`＋原因）。
- **手動執行安全嗎**：是——(1) 伺服器照 `JobEnabled(automatic=false)`：不看 O10 與時間（D-a），但工作的開關與客戶碼照看；(2) 上傳一律用 WinINet（★W36＝C，Steven 20260928；R6 當時模擬組態是只寫 log 的傳輸），
  仍要過 D-f 的開關與帳密閘；(3) 頁面**每次都先問**（上傳工作寫明會用 WinINet 連 FTP 主機——看伺服器回的 `ftp`，不再看 `build`；其他寫會寫報表／複製檔案）；(4) 沒有本體的工作按鈕是灰的；(5) 頁面不顯示帳密，伺服器也不送（`ScheduleConfig` 沒有帳密、R4 的 detail 已遮蔽）。
- **OEE 圖：只提設計，不寫程式**。golden 沒有真的 OEE：`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:385-412` 三張 `TImage` 圖用 `random()` 的假資料，分頁還藏起來（`:413-416`）：
  「每日稼動率」（當月每天 pass／fail／idle 三條，`SetOeeAnaDay` :1310）、「每日稼動狀態」（每天 1440 分鐘的狀態條，`SetOeeChartDay` :1452）、「當日稼動狀態」（今天 24 小時，`SetDayOeeDay` :1595）。建議：
  1. 第一步（只改頁面）：用現有快照的 `byDay`（Input Qty、Total Test Time、Total Stop Time、Alarm Time）在頁面畫當月每天「測試／停機／其他」三段的百分比長條（一天＝24 h），SVG 自己畫、不用函式庫。
  2. 第二步（C++，ElaCore）：每分鐘的狀態（有 Production_Log 測試紀錄＝運轉；EventLogTxt 計入停機的報警 Time～Time＋StopedTime＝停機；其他＝閒置），每天一串 1440 個字元放快照，頁面畫狀態條（每日／當日兩張）。
  3. 真的 OEE（可用率×效率×良率）要 Steven 定三件事：計畫時間（24 h 還是班別）、理想 UPH 從哪裡來（新設定？配方？）、良率怎麼算（pass／input？哪些 bin 算 pass）。
     golden ChipAdvanced 的「OEE」其實只是 (input−fail)/input（R3，uChipAdvancedFunc.cpp）。
  → **★W48（Steven 0928＝A）已做**：取代上面的第 1 步（改成 C++ 算、快照帶 `oee`），見下一節「★W48 OEE (A)」；第 2、3 步不做。
- **ctest `ELA_Schedule` 第 18 節**：N25-3 第一次失敗後 `pending`、`next`＝重試時間 00:11:01、`lastStatus` failed (retry)；重試成功後 `next`＝隔天 00:10:07；O06-4（[O06-8] 30 分，★W43）的 `next`＝下一個 30 分時段；
  851 機台 VTEST 關著、原因寫 915、`next`＝"-"；`GET /api/ela/schedule` 的 installed／o10／build／ftp（兩組態各自）／沒有密碼；`POST /api/ela/job` 403／400／202（下一個 tick 手動跑）／503；其他子路徑 404。
- ⚠ 未上機。上機要看：Auto Jobs 分頁的每一列跟 FTP_Log 一致；按「立即執行」先跳確認；851 出貨組態手動 N25-3 真的連出去（模擬組態只寫 log）；開著這個分頁時查詢頁不會被換掉。

## ★W48 OEE (A)：eventlog.html 的 OEE 分頁——每天測試／停機／閒置的百分比（20260928，St02-E helper；**Steven 0928＝A**）

分支 `v906/steven-w48-oee`（從 gpib-widget `8c3bb48e` 開；本機，St02-E 審查後推）。兩組態只編譯（St02 不執行），0 錯；ctest `ELA_Oee`（新）與 `ELA_Hub` 第 3 節新的一條待 St01；頁面 inline script `node --check` 過。
行號：分析器＝Rev891（`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code`），Handler＝906_0625_Steven（`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven`）。依 St02-E helper 06:4x 的設計筆記（`W48_OEE_DESIGN_NOTE_20260928.md`）「Proposed implementation」1～6。

- **新功能，不是移植**：golden 沒有真的 OEE。`Analyzer.cpp:385-416` 的 FormShow 用 `random()`（srand :359）把三張 TImage 各畫一次，然後三個分頁都藏起來（:413-415）。
  第一張「YYYY-MM 每日稼動率」＝`SetOeeAnaDay`（:1310 起）：每天一條 100 % 橫條，由左到右 Idle｜Alarm｜Running，段內 `%0.2f%` 字，Running＝100－alarm－idle；圖例 Running／Alarm／Pause（:1305-1307）；
  顏色 :1286-1288（BGR）；資料是 `SetOeeAnaDay(Date,i,random(100),random(100),random(100))`（:391）。ChipAdvanced 的「OEE」只是 (input－fail)/input（`uChipAdvancedFunc.cpp:140-150`）。
  裁決 A＝把第一張圖換成真的每日數字；第二、三張（每天 1440 分鐘的狀態條、當日狀態）不做。
- **三項規則都是 St02 預設，待 Steven 確認**（St02-M 07:3x；程式註解與這裡都標 "St02 default, pending Steven"）——
  **已被 Steven 0928 09:3x 的裁決取代（W48-1／W48-2＝B／W48-3＝B），見下一節**；本節的數字與 ctest 表是第一版的：
  - **Q1＝B　測試**＝Production_Log 的觸壓時間軸：Order of testing（第 13 欄）換了就是一次觸壓（golden Contact Count 的規則），時間 t(k)＝SOT（第 14 欄 `yyyymmdd_hhmmss`），沒有就 Arm Time（第 12 欄），再沒有就 In Time（第 5 欄）；
    依時間排序，相鄰兩次相隔 ≤ G＝300 秒（`ela::kOeeRunGapSec`，`OeeOptions::gapSec` 可改）就算中間在跑，每次觸壓再加它自己的 Test Time（第 27 欄）——所以間隔前最後一次觸壓會加上它的 Test Time；
    取聯集（兩支手臂／多個 site 不重複算），**再扣掉停機**（告警框開著時照樣有觸壓，算停機）。
  - **Q2＝A　閒置**＝區間－測試－停機；不分關機（關機也算閒置）。
  - **Q3＝A　停機**＝GetEventLogText 留下的全部 JAM／WAR／MES 告警列（golden Total Stop Time 那些列，MES 告警框也算），每列 [告警時間, 告警時間＋StopedTime)，取聯集、跨午夜切開。
- **怎麼算**（全文在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaOee.h` 檔頭）：每一天 d，區間＝那一天 ∩ [From, To 那一秒的結尾) ∩ [.., 查詢當時)；
  停機＝停機聯集 ∩ 區間；測試＝(觸壓聯集－停機聯集) ∩ 區間；閒置＝區間－測試－停機；測試 %／停機 % 四捨五入到 0.01，閒置 %＝100－兩者。
  那一天沒有觸壓、沒有停機列、測試與停機都是 0 ⇒ **無資料**（灰色）。
- **V906 的程式**：
  - 新檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaOee.h`／`ElaOee.cpp`（ht9045_ela，-Wall -Wextra）：`ela::ComputeOee(const Analyzer&, double now, const OeeOptions&)` 只讀這次查詢已經留在 Analyzer 裡的
    `slStopList`／`slAllData`／`byDayKeys`／`dtStart`／`dtEnd`，**不再讀一次檔**；ElaCore 維持只有 golden 的東西。`ela::OeeJson` 產生快照的 `"oee"`。
  - `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaHub.cpp` `PublishSnapshot`（只有 `RunQuery` 在 `BtnQuery` 之後呼叫它）多一個 `"oee"`，時間用報表時鐘 `clock_->Now()`（`Hub::SetClock`）。
    **只有使用者自己的查詢**會更新；EL_* 工作照舊不換快照。**沒有新路由**：`GET /api/ela` 本來就帶快照。
  - `CMakeLists.txt`：`ElaOee.cpp` 加在 ht9045_ela 來源清單 `ElaCore.cpp` 那一行、加進 -Wall -Wextra 那行（兩行都是同一行加，行數不變）。
  - 頁面 `D:\HT9045\web\page\eventlog.html`：By Hour 後面新分頁「OEE」——inline SVG「每日稼動率（測試／停機／閒置）」每天一條 100 % 橫條（由左到右 測試｜停機｜閒置），段 ≥ 5 % 才寫百分比，
    段與段之間 2 px 白縫，每一列滑過去有提示（日期、三段的時:分:秒與 %、區間、觸壓次數、停機列數）；圖例 測試／停機／閒置／無資料；
    顏色＝golden 的 Running #57D1C9、Alarm #FF8080，Idle #FFFBCB 在白底看不到所以加深成 #E3CF73，無資料 #C9CFD3；圖下一行規則說明（G 從快照讀）與「V906 還不寫這些 log」的小字；
    再下面是表格（日期、區間、測試、停機、閒置、三個 %、觸壓次數、停機列、備註）；「本月」鈕＝From 本月 1 日 00:00:00、To 現在，然後按 Query（golden 圖的月份，FormShow :368）。
    其他分頁、查詢列、樣式都沒動（只多一個分頁、一個 pane、一段 script）。
- **快照 `"oee"` 的樣子**（ctest 的查詢）：
  `{"rule":{"test":"touchdownTimeline","q1":"B","gapSec":300,"down":"stopRowsUnion","q3":"A","idle":"spanMinusTestDown","q2":"A","pending":"St02 default, pending Steven","toSecondIncluded":true,"now":"2026/05/05 12:00:00","touchdowns":2434,"stopRows":6},`
  `"days":[{"date":"2026/05/01","spanSec":86400,"testSec":64800,"downSec":3600,"idleSec":18000,"testPct":75.00,"downPct":4.17,"idlePct":20.83,"touchdowns":2160,"stopRows":2,"noData":false}, …]}`
  （秒最多三位小數；% 兩位小數）。
- **自己決定的地方**（不是三項裁決；待 Steven 一起看）：
  1. **To 那一秒算進去**：golden 查詢的判斷是 `dtCurrent <= dtEnd`，所以 To＝23:59:59 是一整天（86400 秒，不是 86399）。
  2. **停機是聯集、跨午夜切開**，跟 By Day 的 Total Stop Time（整筆加在告警那天、重疊的會重複加）刻意不同；By Day 那一欄照舊是 golden 的算法（ctest 有釘：05/02、05/03 都還是 1200 秒）。
  3. 觸壓次數不管 Test Time 讀不讀得出來（golden 讀不出來就不算 contact）。
  4. 兩個 % 都進位使總和超過 100 時，從停機扣（閒置 0）。
  5. 無資料的日子三個時間與三個 % 都是 0（不宣稱閒置）；查詢當時以後的日子區間 0、也是無資料（頁面寫「還沒到」）。
  6. StopedTime 是負數時當 0（golden 會把負數加進 Total Stop Time）。
  7. 橫條順序改成 測試｜停機｜閒置（golden Idle｜Alarm｜Running），讓每天的測試段從同一個左邊開始比。
- **V906 自己還寫不出這些資料**：帶 StopedTime 的告警列（`SaveErrEventLog` 還閘著，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fNote.h:95`）、Production_Log（`SaveRecord` 不在範圍，
  `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Public\MyProductionRecord.cpp:15-19`）、每小時的 TimeData（`RecordTimeData` 沒有人呼叫）。所以 **V906 寫的 log 那幾天會是「無資料」**；BCB6 機台寫的 log 才畫得出來（頁面圖下有這行小字）。
- **ctest `ELA_Oee`**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_ela_oee.cpp`；log 由測試自己產生、只在 `%TEMP%\ht9045_ela_oee`，拒絕 D:\HT9045*／D:\RMS，全過才刪）：
  查詢 2026/05/01 00:00:00～2026/05/05 23:59:59（走 `BtnQuery`），查詢當時＝05/05 12:00:00；每次觸壓兩列（site 1／2，同一個 Order of testing）。

  | 日 | 內容 | 區間 | 測試 | 停機 | 閒置 | 測試 % | 停機 % | 閒置 % |
  |---|---|---|---|---|---|---|---|---|
  | 05/01 | Steven 的例子：04:00:00～09:59:30、11:00:00～22:59:30 每 30 秒觸壓（Test Time 30 秒）；JAM 10:00 停 3600 秒；MES2110 04:00 停 0 秒；WAR Duplicate=1（不算） | 86400 | 64800 | 3600 | 18000 | 75.00 | 4.17 | 20.83 |
  | 05/02 | 觸壓 08:00:00、08:05:00（隔 300 秒＝在跑）、08:10:01（隔 301 秒＝閒置），Test Time 2 秒；JAM 23:50 停 1200 秒（跨午夜 600＋600） | 86400 | 304 | 600 | 85496 | 0.35 | 0.69 | 98.96 |
  | 05/03 | 前一天過來的 600 秒；JAM 10:00 600 秒＋JAM 10:05 600 秒（聯集 900，不是 1200）；09:50～10:20 每 60 秒觸壓（Test Time 2 秒），停機蓋過測試 | 86400 | 902 | 1500 | 83998 | 1.04 | 1.74 | 97.22 |
  | 05/04 | 什麼都沒有 ⇒ 無資料 | 86400 | 0 | 0 | 0 | 0.00 | 0.00 | 0.00 |
  | 05/05 | 08:00:00～09:59:30 每 30 秒觸壓（Test Time 30 秒）；JAM 11:50 停 1200 秒，切在查詢當時 12:00 | 43200 | 7200 | 600 | 35400 | 16.67 | 1.39 | 81.94 |

  另外：觸壓 2434 次（不是列數 4868）＝golden Contact Count；`ParseSotStamp`；By Day Total Stop Time 05/02／05/03 仍是 golden 的 1200／1200；G＝301 秒時 05/02 測試＝603 秒；
  查詢當時在範圍之前＝每天區間 0、無資料；只查 05/01（To 23:59:59）＝區間 86400、閒置 18000；JSON 第 1 天／第 4 天的全文與 rule。
- **ctest `ELA_Hub`** 第 3 節多一條：使用者查詢的快照有 `"oee"`，2026/04/01 區間 86400、測試 0、停機 50 秒（兩列告警 30＋20 秒，沒有 Production_Log）。
- ⚠ **未上機**。上機要看：
  1. 拿一台 BCB6 機台的一天（EventLogTxt＋Production_Log）查詢，OEE 的測試時間跟 Handler 自己的稼動時間比：`labProductTime`／`labRunningTime`（`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cObserver.cpp:757-759`），
     或 `D:\HT9045_Log\TimeData\<年>\TimeData_<年>.csv` 的 ProductionTime／JamTime；停機跟 By Day 的 Total Stop Time 差多少（重疊與跨午夜）。
  2. G＝300 秒合不合實際的換盤／換料間隔。
  3. 待上機確認：告警框開著但機台照跑（NonStop 的 WAR）時算停機；第一次觸壓前的 soak 算閒置；From 之前就開始的告警看不到；MES 告警框（例 MES1640 One cycle finish）算停機（Q3）。
  4. 「本月」查一整個月要多久、快照多大（Production_Log 一個月可能上百萬列；By Day 本來就要讀）。
  5. V906 寫的 log 那幾天顯示「無資料」。

## ★W48 三項裁決（Steven 0928 09:3x）：W48-1／W48-2＝B／W48-3＝B（20260928，St02-E helper；取代上一節的 Q1／Q2／Q3）

分支 `v906/steven-w48-rework`（從 gpib-widget `959c2236` 開；本機，St02-E 審查後推）。兩組態只編譯（St02 不執行），0 錯；ctest `ELA_Oee`（改寫）、`ELA_TimeData`（新）、`ELA_Hub` 第 3 節待 St01；頁面 inline script `node --check` 過。
註解標記 `AI(W906-ELA-W48B) 20260928 (St02-E helper)`。行號：Handler golden＝906_0625_Steven（`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven`，Big5），分析器＝Rev891（`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code`），GPIB 橋＝V906 的逐行翻譯（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Gpib`，golden GPIB 905 `D:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525\Main.cpp`）。
Steven 原話在 `docs/handoff/FROM_STEVEN.md`（交接分支）09:3x 那一列。

### W48-1 測試時間：用 Production_Log 第 27 欄（golden 就是這樣量的）

- **調查結果：golden 的 Test Time（第 27 欄 `hh:nn:ss.zzz`）＝ Steven 的定義**（GPIB 送 0x41 到收到 BIN ON＋ECHO OK），只多了 Handler 與橋程式之間的訊息延遲（兩端各一次，毫秒到數十毫秒；沒量過）：
  - 第 27 欄＝`RunInfo.TestTime`（`cObserver.cpp:2024`，`:2033` `AddTestTime`）＝ TimeInfoGrid 最新一列（`RecordTimeInfo` `:1894-1931`）＝ `RecordEndTestTime`（`:2150-2161`）－`RecordStartTestTime`（`:2136-2147`）。
  - 起點：`atester.cpp:1481-1482` 在 `fMain->RunTestProgram`（`:1530`）之前蓋章；RunTestProgram 叫橋程式舉 SRQ 0x41（V906 `TesterComm\Gpib\GpibTestGpib.cpp:400-402`）。SOT（第 14 欄）是同一個函式蓋的（`cObserver.cpp:2146` `sBufferSOT`）。
  - 終點：測試工作看到 `bEcho`（`atester.cpp:1645`）之後，`aTester_Front.cpp:2925-2926`／`aTester_Rear.cpp:2894`／`atester_32Site.cpp:2783` 呼叫 `RecordEndTestTime`；`bEcho` 由橋程式送來的結果設（`main.cpp:17122`），
    而 GPIB 橋**收到 ECHOOK 才送結果**（`GpibTestGpib.cpp:1422-1449` return 1 → `GpibCore.cpp:1079-1116` `SendCaptureFinish`）；BINON 不回 ECHO 的設定是收到 BINON 就送。RS232／TTL 走同一段（`atester.cpp:1443-1452` 進 case 55；直接 DIO 的 TTL 是 `:1965-1966`），所以也一樣。
  - golden 的小毛病（照舊，沒改）：Double contact（SRQ 0x42／0x43／0xC1 重測）每次接觸都重蓋起點、終點只蓋一次 ⇒ 第 27 欄只有最後一次；只用分／秒／毫秒相減（超過 60 分鐘的測試會錯）；iShuttleMode 1 只有選到的那支手臂會更新（`cObserver.cpp:2163-2169`）；沒有真的 IC 就不蓋章（`atester.cpp:1481`）。
- **ELA 的改法**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaOee.cpp`）：測試＝每次觸壓 [起點, 起點＋Test Time) 的聯集（單一測試頭不重疊＝加總），扣掉停機；起點＝SOT，沒有就 EOT（第 29 欄）－Test Time，再沒有就 Arm Time、In Time。
  **Q1＝B 的「相隔 ≤ 300 秒算在跑」拿掉了**（`kOeeRunGapSec`／`OeeOptions::gapSec` 刪除），觸壓之間的時間現在是閒置。讀不到 Test Time 的觸壓照算次數、不加時間（`noTestTime`）。
- **V906 自己還寫不出第 27 欄**：`RecordStartTestTime` 是活的（`atester.cpp:1596`、`:2119`），`RecordTimeInfo`＋`AddTestTime` 也翻了（`cObserver_TimeInfo.cpp:244`，St01 S116），但
  (1) 三支測試工作呼叫的 `RecordEndTestTime` 都是回 1 的替身，終點從來沒蓋；(2) Production_Log 的寫檔（`TMyProductionRecord::SaveRecord`，golden `Public\MyProductionRecord.cpp:660-1384`）不在範圍（`Public\MyProductionRecord.cpp:15-19`）。
  **認領清單（不是 St02 的檔，同一行、行數不變；請 St01-M 轉給擁有者）**——V906 的 `RecordEndTestTime`（`cObserver.cpp:3111`）回的是 `SendTestResultToHttp()`，而那支在 V906 也是回 1 的替身，所以 `ret2` 照舊是 1：
  1. `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\aTester_Front.cpp:3263`
     OLD `static int  W7Ck4_RecordEndTestTime(int /*iWhich*/) { return 1; }               // golden (Sam 20201231)`
     NEW `static int  W7Ck4_RecordEndTestTime(int iWhich) { extern int RecordEndTestTime(int); return RecordEndTestTime(iWhich); }   // golden (Sam 20201231); AI(W906-ELA-W48B): the real stamp, cObserver.cpp:3111`
  2. `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\aTester_Rear.cpp:3147`
     OLD `static int  W7Bk4_RecordEndTestTime(int /*iWhich*/) { return 1; }               // golden (Sam 20201231)`
     NEW `static int  W7Bk4_RecordEndTestTime(int iWhich) { extern int RecordEndTestTime(int); return RecordEndTestTime(iWhich); }   // golden (Sam 20201231); AI(W906-ELA-W48B): the real stamp, cObserver.cpp:3111`
  3. `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\atester_32Site.cpp:393`（`:396` 的 `#define RecordEndTestTime` 在它後面，所以這一行叫得到真的那支）
     OLD `static int  W5_32S_RecordEndTestTime(int /*iWhich*/){ return 1; } // golden (Sam 20201231) -- offline: OK(1), matches golden's own DummyMode==false "no XML retry needed" default`
     NEW `static int  W5_32S_RecordEndTestTime(int iWhich){ extern int RecordEndTestTime(int); return RecordEndTestTime(iWhich); } // golden (Sam 20201231); AI(W906-ELA-W48B): the real stamp, cObserver.cpp:3111 (returns SendTestResultToHttp's offline 1)`
  4. Production_Log 的寫檔（SaveRecord 那一半）＝一整段移植，不是一行，交給 `Public\MyProductionRecord.cpp` 的擁有者排。
  ⚠ 1～3 套上前要先確認：連到 aTester_Front／Rear／32Site 的 ctest 也連得到 `cObserver.cpp`（沒有的話會 undefined reference）；RecordTimeInfo 會開始更新觀察頁的時間表（golden 行為）。
- **備案（只設計，沒做）**：若 Steven 要的是橋程式自己的訊號、而不是 Handler 的兩個章，可以在 St02 的橋程式裡蓋：起點 `GpibTestGpib.cpp:401-402`（`ibrsv(noncontroller, 0x41)`），終點 `GpibCore.cpp:1080`（`ret==1`，ECHOOK 之後）；
  RS232／TTL 起點是送出 SOT（例 `TesterComm\Rs232\Rs232HandlerMsg.cpp:621-622` `@WSOTS…`）、終點是結果送回 Handler 的地方；每次觸壓寫一列新的 `<HT9045_Log>\TestTime\` 檔（引擎執行緒，自己的鎖），ELA 再讀它。
  因為第 27 欄已經就是這個時間，**建議不做**（兩個來源會重複算，而且 BCB 機台的舊 log 只有第 27 欄）。
- **A8（審查意見，St02 自己裁；`AI(W906-ELA-A8) 20260928 (St02-E helper)`）超豐的欄位順序不一樣**：golden `SaveRecord` 在 CUSTOMER_CODE＝CC_Greatek（956，`MachineType.h:325`）時改走 `SaveDataForGreatek`（906_0625_Steven `Public\MyProductionRecord.cpp:703-708` → `:851-887`，`iSortData` `:858`，表頭 `asDataTitleGreatek` `:46-88`）⇒
  Test Time 在**第 37 欄**（第 27 欄是 Out Arm Shuttle Pick `:73`），Order of testing 19、SOT 20、EOT 39、Arm Time 18、In Time 6。其他客戶都是預設順序（`:710-772`，表頭 `asDataTitle` `:92-162`）：KYEC_LEE／Greatek／HANA 只把 X／Y 加 1（`:255`／`:291`／`:394`），LEADYO 另存一份同格式（`:1038`），SJ／O24 只改檔名（`:1120-1142`）；912_0908_Jimmy 相同。
  `ElaOee` 的 `ProdLogColumns`：六欄先照 slAllData[0] 表頭的 golden 名字找（兩種表頭都有這六個名字），沒有名字才看 CUSTOMER_CODE（剛好 "956"＝超豐，其他＝預設）；預設的結果完全沒變（快照 `w481` 仍是 `col27`，超豐是 `col37`）。ctest `ELA_Oee` 第 8 節（預期值另用獨立 Python 模型對過）。
  ⚠ **上游缺口（沒改，要裁）**：golden ListProductionLog（Rev891 `Analyzer.cpp:2377-2385`＝`ElaTables.cpp:531-543`）不分客戶用第 5 欄當 In Time 篩列；超豐的第 5 欄是 In Y（小整數，被當成 1899 年的時刻）⇒ **超豐的列一列都進不了 slAllData**，OEE 的測試段與 By Day 的投入／接觸／測試時間在超豐機台上都是 0（golden 分析器本來就這樣）。要不要讓 ListProductionLog 也認超豐的順序＝偏離 golden 分析器，待決定。
  另更正上面第 (2) 點：V906 的 SaveRecord 已移植（`Public\MyProductionRecord.cpp:1185`，超豐分支 `:1232`，`aoutarm9045.cpp:5058` 呼叫），三支 RecordEndTestTime 也已接真的（`4f6f4cbf`）。

### W48-2＝B 閒置 vs 關機：用 golden 每小時的 TimeData

- **golden**：`HS_Function.cpp:233-238`（`TFormHS::TimerAutoBackupTimer`，1000 ms 的 VCL timer，建構子 `:56` 打開；`:106-111` 的關卡：InitialOK、重入、ATC 告警）整點 `CheckClockTrigger(60)`（`:4312-4328`）→
  `RecordTimeData(2)`（N10 上傳方法 1）或 `RecordTimeData(3)`（`cMyDB.cpp:339-498`；VTEST 直接 return `:345-346`）：寫一列 `D:\HT9045_Log\TimeData\<yyyy>\TimeData_<yyyy>.csv`（`main.cpp:1539-1543`，TByYear），
  欄位 `Date, Time, StartTime, HomeTime, ContactTestTime, PauseTime, ProductionTime, JamTime, PowerOnTime, UnloadingCount, JamCount, MUBA, MTBA`（`:380-392`，都是「上一列以來」的秒數），一列 HANDLER LOG，然後把 [2]／[3] 計數、BinCT[1]、iJamCount[2] 歸零。
  PowerOn 計數在 `main.cpp:8166-8177`（UpdateRecordScreen）。分析器 Rev891 沒有讀 TimeData。
- **V906 原本就有的**：`RecordTimeData` 已經移植（`cMyDB.cpp:479`，P1；golden 的 Summary Report 那段 `:393-476` 仍是 `#if 0` GA1-B4）；`slTimeData` 開機時建（`LogObjects.cpp:136-140`，走 as9045LogPath＝`W906_HT9045LOG_ROOT` 接縫）；
  PowerOn 等計數 St01 S113 已接上（`FileRW\MainRecord.cpp:141-152`，wb_serve 主迴圈每 500 ms，`tools\wb_serve.cpp:5953`）。**cMyDB 不用改**。缺的只有「整點呼叫它」的那一個 timer（V906 沒有 TimerAutoBackupTimer，`forms\fHS.h` 註明沒移植，TFormHS 也拿不到）。
- **呼叫端＝St02 自己的 tick（不用認領）**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Handler\TesterCommWiring.cpp`
  `W906_TimeDataHourTick()`（1000 ms 節流＝golden timer 的預設 Interval；InitialOK 關卡；重入旗標；整點判斷照 golden `:4312-4328` 自己一份 static）→ `RecordTimeData(2 或 3)`。
  掛在 `W906_TesterCommTick()` 與 `W906_TesterCommPoll()` 的第一行（在 HT9045_TESTERCOMM=0 的退出判斷之前）：Tick 跟 UpdateRecordScreen 同一條 wb_serve 主執行緒（golden 兩個都是主執行緒的 VCL timer，所以歸零不會跟累加搶），Poll 讓它在告警框等待迴圈裡也照跑（golden 的 timer 在 ShowModal 期間照跳）。
  **不用 ELA hub 的時鐘**：hub 是另一條執行緒，會跟主迴圈搶 `LastSet.SystemAccSecond`，而且 ht9045_ela 不連 Handler 的程式。
  沒做的 golden 關卡：ATC 告警（`HasAlarmMsg`）——V906 的全域 `ATC_InterfaceForm` 是只有一個欄位的替身（`acarry_shims.h:115`），翻好的 `TATC_InterfaceForm::HasAlarmMsg` 沒有物件可以叫。
  ⚠ **只能有一個呼叫端**：將來誰移植 TimerAutoBackupTimer，`:233-238` 那段就不要搬（或把這個 tick 拿掉），不然每小時兩列。
- **ELA 讀 TimeData**（`ElaOee.h`／`.cpp`）：`ReadTimeData(資料夾, 查詢起, 查詢迄)` 讀查詢年份前後各一年的 `<資料夾>\<yyyy>\TimeData_<yyyy>.csv` 與 2021/06/23 以前的平放檔名 `<資料夾>\TimeData_<yyyy>.csv`（唯讀；排序、完全相同的列去掉）；
  資料夾＝`HubPaths::timeDataDir`＝`DefaultTimeDataDir()`＝`W906_HT9045LOG_ROOT`（沒設＝golden `D:\HT9045_Log`）＋`\TimeData`。`Hub::PublishSnapshot` 每次使用者查詢讀一次。
  - 第 k 列（時間 at(k)、PowerOnTime P(k)）：at(k)－at(k-1)－P(k) ≤ 60 秒（`kTimeDataSlackSec`；Handler 寫整數秒、觸發有抖動）⇒ 這一段全都開著；否則開著的是 [at(k)－P(k), at(k))——**量是對的，位置不知道**，所以放在尾端（重開機後第一列也帶著關機前的時間）。
  - 第一列前面沒有列可比：最多算它結束的那一小時（at－3600－60 秒）——V906 第一列會帶著 S113 上線以來從沒歸零的累計（見下面風險）。
  - 最後一列之後到「查詢當時」：3600＋60 秒以內＝開著（分析器就在 Handler 裡跑，這一小時還沒記）；超過＝沒有 TimeData（算閒置，不算關機）。
  - 關機＝TimeData 涵蓋的時間－開著的時間－任何測試－任何告警列（有測試或告警框就一定開著）。**沒有 TimeData 涵蓋的時間不算關機、留在閒置**，另外報 `uncoveredSec` ⇒ 沒有 TimeData 時結果跟改之前一樣（沒有關機段）。
  - 閒置＝區間－測試－停機－關機。另外每天報 `onSec`（TimeData 的開著時間）、`startSec`（TimeData 的 StartTime＝SystemStart 秒數，平均攤在那一列的開著時段上；資訊用）。
- ~~待 Steven（W48-2 的邊界）~~ **已裁：★W61＝B**（見下面「★W61＝B」）：「按了 Start 但沒在測試」算生產，不算閒置。
- **頁面**（`D:\HT9045\web\page\eventlog.html` OEE 分頁）：第四段「關機」灰色 `#8C939B`（畫在最後；無資料仍是淺灰 `#C9CFD3`），圖例多「關機」，提示多關機、軟體開著（TimeData）、沒 TimeData 的時間；
  表格多「關機」「關機 %」「開著（TimeData）」「SystemStart（TimeData）」「沒 TimeData」；規則說明三段都改成 Steven 的裁決；圖下小字改成「V906 從這一版起每小時寫 TimeData…」。

### W48-3＝B 停機只算「計入 MTBA」的告警

- `ElaOee`：停機＝`slAlarmList`（`JamConfig::GetJemIncludeMTBA` 算進去的那些列＝golden 的 Alarm Time，Rev891 `Analyzer.cpp:883-899`）的 [t, t＋StopedTime) 聯集；其他留下的告警列（WAR、MES1640 One cycle finish、沒勾的 JAM）＝閒置，但仍證明軟體開著（不會算關機）。每天與總數多 `alarmRows`。
- ⚠ **「沒設過的代碼預設算」不完全對**：ELA 的 `GetJemIncludeMTBA` 沒有 key 時用 golden 的預設（`Analyzer.cpp:2812-2835`：代碼有 "JAM" 而且在 01～05 區＝算，其他＝不算），**不是一律算**；
  沒有 JAM0000.dat 時全部不算（`:2818` FileExists）。W45-2／18b（Steven 0928）改的只是 933／967 的「IncludeMTBF」預設（`JamRules.h`，`v906/steven-w45-jam`）。
  所以 ST01-M 舉的例子反過來：MES1640（16 System 區）沒設過＝本來就不算停機、是閒置；客戶**勾了**它才會變成停機。933／967 的機台上，這裡讀的 key 是合併畫面的第二格「Include MTBA (Analyzer)」（W45 18c），Handler 自己那格寫的是 IncludeMTBF。
  如果 Steven 真的要「沒設過一律算」，要另外裁（會跟 golden 的 MTBA 統計、Handler 畫面的預設不一樣；建議不要）。

### ★W61＝B：SystemStart 全部算生產（20260929，St02-E helper；Steven 裁決，St02-E 轉達）

Steven：「沒測試,沒systemstart的, 就是idle」、「system start都算在生產中, 只是內部會再細分成 test time, contact test, off-line, home 之類的」。註解標記 `AI(W906-ELA-W61) 20260929 (St02-E helper)`（`ElaOee.h` 的 W61 段有全部 golden 行號）。
最上層改成**生產／停機／閒置／關機**（加起來 100.00 %）；生產＝SystemStart 的時間，再分測試／contact test／home／其他。每天：
空檔 free＝區間－測試－停機－關機（W48 的閒置）；start＝TimeData 的 StartTime（每列平均攤到它開著的時段，跟 `startSec` 同一套）－計入停機的馬達告警；
**nonTest＝min(max(start－測試, 0), free)**（測試只會在 SystemStart 裡，所以 start－測試＝有 SystemStart 沒在測；上限 free：測試／停機／關機優先，五段不會超過區間）；
**生產＝測試＋nonTest，閒置＝free－nonTest**；nonTest 再依序分 contact test（ContactTestTime）、home（HomeTime－馬達告警），剩下的＝「其他」（換盤、soak、手臂走位）。
golden 依據（906_0625_Steven）：UpdateRecordScreen `main.cpp:8181-8201` 在 SystemStart 時把每一跳加到 StartTime，並且只加到 HomeTime（`!fAllMotorHome`）／ContactTestTime（`fContact->fShow`）／ProductionTime 其中一個；RecordTimeData `cMyDB.cpp:380-387` 寫成 TimeData 第 2／3／4／6 欄。
- **停機跟 SystemStart 會不會重疊**：StartTime（`:8181`）跟 JamTime（`fNote->fShow` `:8226-8230`）是兩個獨立的 if。一般告警 ShowErrorMessage（`note.cpp:532`）在 `:801` 先把 SystemStart 關掉才 ShowModal（`:991`）⇒ **不重疊，所以停機不從 start 扣**。
  **例外＝馬達告警**（`WAR24%03d`＋類型，`note.cpp:4291-4295`／`:1077`；golden 自己也用 "WAR24" 認，`cSecurity.cpp:1271`）：ShowMotorErrorMessage（`:1052`）只清 SoftStop／SoftStart、`fAllMotorHome=false`（`:1054-1059`）就 ShowModal（`:1133`），呼叫端回來才關 SystemStart（`csystem.cpp:4064-4067`），Timer1 在 modal 迴圈裡照跳（MainProc 走 TRunControl 的 Synchronize，`uruncontrol.cpp:30-44`）⇒ 那段同時記成 StartTime＋HomeTime＋JamTime。
  所以**計入停機（slAlarmList）的 WAR24 列**從 start 與 home 扣掉，不重算（預設 MTBA 只算 JAM，WAR24 要客戶勾了才會是停機）。
- **off-line：golden 沒有訊號** ⇒ 不另外分，算在「其他」（不捏造）：TimeData 沒有這一欄、UpdateRecordScreen 不看 `LastSet.iTester`；off-line 的觸壓跟 online 一樣寫 Production_Log（`atester.cpp:1424-1425` → `:1481-1482` RecordStartTestTime、`:1562-1568` 等 Prod.iTesterDummyTime），
  沒有欄位標模式（第 37 欄「Test Mode」只有 AddTestModeRecord `MyProductionRecord.cpp:482-485` 會寫，沒有人呼叫）；EventLog 只把 off-line 時跳的告警標 Duplicate=2（`note.cpp:805-806`），標的是列不是時間。所以 off-line 的觸壓照樣算「測試」。
- 取整：生產／停機／關機四捨五入到 0.01 %，閒置＝100－三者（超過時依序從關機、停機、生產扣）；生產裡測試／contact test／home 四捨五入，其他＝生產 %－三者（負的依序從 home、contact test、測試扣）。
  **沒有 TimeData＝start 是 0 ⇒ 生產＝測試、閒置＝W48 的閒置、測試 % 不變**。
- JSON 每天多 `prodSec`／`contactSec`／`homeSec`／`prodRestSec` 與對應的 `…Pct`（接在 `offPct` 後面）；W48 的鍵都留著、順序不變（`testSec`／`testPct` 現在是生產的一部分；`idleSec`／`idlePct` 改成不含 SystemStart；`startSec` 仍是 TimeData 讀到的原始秒數）。
  rule 的 `idle` 值改成 `spanMinusProdDownOff`，多 `prod`＝`timeDataStartTime`、`w61`＝`B`、`contact`、`home`、`offline`＝`noGoldenSignal`、`motorDownOutOfStart`＝`WAR24`。
- 頁面：長條＝生產（測試 `#57D1C9`／contact test `#9C8CD9`／home `#A9CBE8`／其他 `#5B8DB8`，底下一條深綠線 `#1F6F6A` 標出整段生產）｜停機｜閒置｜關機；圖例、提示、表格跟著改；舊的快照（沒有 prodPct）照舊畫成生產＝測試。
- ctest `ELA_Oee`：主資料 05/01～05/04 數字不變（05/01 的 StartTime 3604 秒比測試少 ⇒ 生產＝測試）；05/05 的 07:00:04 列加 HomeTime 600、08:00:04 列加 ContactTestTime 900 ⇒ 生產 8700（20.14 %）、閒置 13796→12296 秒（31.93→28.46 %）；
  第 9 節（05/09）：SystemStart 5000／測試 2000 ⇒ 生產 5000、閒置少 3000；分段 contact 900＋home 600＋其他 1500；WAR24 400 秒從 start／home 扣、JAM 400 秒不扣；上限（整天 SystemStart＋JAM 3600 ⇒ 閒置 0）；取整借位。預期值用獨立 Python 模型算。

### 快照 `"oee"` 現在的樣子（ctest 的查詢；W61 之後）

`{"rule":{"test":"touchdownTestTime","w481":"col27","down":"mtbaAlarmsUnion","w483":"B","off":"timeDataPowerOn","w482":"B","idle":"spanMinusProdDownOff","ruled":"Steven 0928 09:3x","prod":"timeDataStartTime","w61":"B","contact":"timeDataContactTestTime","home":"timeDataHomeTime","offline":"noGoldenSignal","motorDownOutOfStart":"WAR24","toSecondIncluded":true,"now":"2026/05/05 12:00:00","touchdowns":2434,"noTestTime":0,"stopRows":11,"alarmRows":6,"timeDataRows":71,"timeDataSlackSec":60,"timeDataDir":"…"},`
`"days":[…,{"date":"2026/05/05","spanSec":43200,"testSec":7200,"downSec":600,"idleSec":12296,"offSec":21604,"testPct":16.67,"downPct":1.39,"idlePct":28.46,"offPct":50.01,"prodSec":8700,"contactSec":900,"homeSec":600,"prodRestSec":0,"prodPct":20.14,"contactPct":2.08,"homePct":1.39,"prodRestPct":0.00,"onSec":21596,"startSec":8700,"uncoveredSec":0,"touchdowns":240,"stopRows":1,"alarmRows":1,"noData":false}]}`
（W48 的鍵照 W48 的順序排在前面——`tests\test_ela_hub.cpp:146` 比對 `"spanSec":86400,"testSec":0,"downSec":30,` 這一串；W61 的鍵接在 `offPct` 後面。）

### ctest

- **`ELA_Oee`**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_ela_oee.cpp`，改寫）：先做「containment first」——`W906_HT9045LOG_ROOT` 不是 ctest 的 machine_log_scratch 就 exit 2；log 全部由測試產生、只在 `%TEMP%\ht9045_ela_oee`（沙盒內另有 JAM0000.dat：JAM0302 不勾、MES1641 勾），全過才刪。
  查詢 05/01～05/05 23:59:59、查詢當時 05/05 12:00；TimeData 71 列（05/02 12:00:04 關機、21:00:04 那列帶 5400 秒；05/04 00:00:04～05/05 06:00:04 關機）。

  | 日 | 區間 | 測試 | 停機 | 閒置 | 關機 | 測試 % | 停機 % | 閒置 % | 關機 % |
  |---|---|---|---|---|---|---|---|---|---|
  | 05/01 Steven 的例子 | 86400 | 64800 | 3600 | 18000 | 0 | 75.00 | 4.17 | 20.83 | 0.00 |
  | 05/02 三次觸壓各 2 秒；JAM0105 在關機時段內（證明開著） | 86400 | 6 | 720 | 58794 | 26880 | 0.01 | 0.83 | 68.05 | 31.11 |
  | 05/03 聯集 900；W48-3 五列（只有勾了的 MES1641 算停機） | 86400 | 32 | 1700 | 84668 | 0 | 0.04 | 1.97 | 97.99 | 0.00 |
  | 05/04 軟體沒開（改之前是「無資料」） | 86400 | 0 | 0 | 4 | 86396 | 0.00 | 0.00 | 0.00 | 100.00 |
  | 05/05 切在 12:00；06:00:04 以前關機（★W61：閒置改成 12296／28.46，生產 8700／20.14） | 43200 | 7200 | 600 | 13796 | 21604 | 16.67 | 1.39 | 31.93 | 50.01 |

  另外：`ParseTimeDataLine`（Handler 的格式、SIGURD 沒空白的格式、負數＝0、表頭／錯日期／錯時間／非數字／欄數不夠都拒）；`ReadTimeData`（兩種檔名、重複列與壞列去掉、排序）；`onSec`／`startSec`／`uncoveredSec`；
  沒有 TimeData＝沒有關機段、05/04 回到無資料；最後一列之後超過 3660 秒＝沒有 TimeData（算閒置）；第一列帶 5 天的 PowerOn 只算一小時；By Day 的 Total Stop／Alarm Time 仍是 golden 的 1320／1320、2700／1400；查詢當時在範圍之前＝無資料；只查一天；JSON 全文；第 7 節 W48-1 沒有 SOT 時的起點。
  （這些預期值另外用一支獨立的 Python 模型對過，不是跑建出來的程式。）
- **`ELA_TimeData`**（新，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_ela_timedata.cpp`，連 Handler 的函式庫，RESCAN）：**containment first**（b12ab375 的規則）：四個 cMyDB log 根目錄與 ctest 的轉向變數不是 machine_log_scratch 就 exit 2（任何 Handler 程式碼之前）；
  然後把 as9045LogPath／asSaveEventLogPath 指到 `%TEMP%\ht9045_ela_timedata_<tick>`（在 D:\HT9045* 底下就拒），自己建 `fMain->slTimeData`（golden 的名字、表頭、TByYear），全過才刪沙盒。
  1. 整點判斷：00:00～00:02 上膛、≥ 00:04 觸發一次、錯過那 3 秒＝那一小時沒有列；2. InitialOK false：不讀時鐘、不上膛、不寫、不歸零；3. 方法 0 → RecordTimeData(3)：golden 表頭＋一列 `11,18,17,12,14,15,3600,12,3,4`、[2]／[3]／BinCT[1]／iJamCount[2] 歸零、[0] 不動、HANDLER LOG 寫在沙盒；
  4. 方法 1 → RecordTimeData(2)；5. VTEST：不寫、不歸零；6. `ela::ReadTimeData` 讀回 Handler 寫的兩列（PowerOnTime／StartTime）。
- **`ELA_Hub`** 第 3 節：停機 30 秒（W48-3：WAR2401 不算，原本 50）、規則名 `touchdownTestTime`；`HubPaths::timeDataDir` 釘在沙盒（不存在＝沒有關機段）。

### 風險與上機要看

- 未上機。上機要看：
  1. 裝上後第一個整點（hh:00:04）`D:\HT9045_Log\TimeData\<年>\TimeData_<年>.csv` 多一列、HANDLER LOG 多一列；**第一列的 PowerOn／StartTime／MUBA／MTBA 是 S113 上線以來累計的（以前沒有人每小時歸零），不是一小時的量**——ELA 把第一列最多算一小時。之後每小時一列、PowerOn ≈ 3600。
  2. 關掉 wb_serve 一段時間再開：下一個整點那一列的 PowerOn＝關機前＋開機後，OEE 圖在那段中間出現灰色的關機（位置放在尾端）。
  3. 開著告警框跨過整點：還是有一列（Poll 也會叫）。
  4. 拿一台 BCB6 機台的 TimeData＋EventLogTxt＋Production_Log 查一天：測試＝第 27 欄的加總（跟觀察頁 TimeInfoGrid 的測試時間比），關機跟實際關機時間比。
  5. 第 27 欄跟真的 0x41～ECHOOK（GPIB log 的 `0100 SRQ:0x41`／`0600 LISTEN:ECHOOK` 時間）差多少毫秒。
  6. ★W61：同一天 TimeData 的 StartTime 加總 ≈ OEE「生產」（除非被停機／關機蓋掉），HomeTime／ContactTestTime ≈「home」／「contact test」；按 Start 後 soak、換盤的時間要落在「其他」、不在閒置；
     跳一個有勾 Include MTBA 的 WAR24 馬達告警，看 TimeData 那一小時的 StartTime／HomeTime 有沒有把告警框的時間也算進去（有＝golden 行為，ELA 已扣掉）。
- 開發機跑 wb_serve 也會每小時寫 `D:\HT9045_Log\TimeData` 與 HANDLER LOG、把每小時計數歸零（下一次 WriteLastDataFile 會存進 lastdata.dat）——跟 S113 已經在做的累加同一類的寫入，golden 行為。
- golden 的 Summary Report 那段（`cMyDB.cpp:393-476`，iDataType 3 每小時也會跑）：前半 `:393-431`（`asProduct_LoaderPath` 的每小時產量檔、`LastSet.iLoaderCount` 歸零）**A4（20260928）已解開**，見 `docs/CMYDB_PORT_LEDGER.md`「A4」。
- **已知缺口（O19）**：golden `cMyDB.cpp:433-476` 的 08:00 日／週／月 Production Summary Report（`IniConfig.asO19_SavePath\<yyyy>\<HostName>_<mmdd>….csv`）V906 **還不會寫**（`cMyDB.cpp:575-630` 仍 `#if 0`：DoProduction_Summary_Report 是空殼、沒有 DayOfWeek、沒有 fConfiguration->edtN04_Host）。
- ATC 告警時 golden 整個 timer 不跑（那一小時可能沒有列），V906 照跑（見上）。
- **A5（20260928，St02-E helper；St02-E2 審查 A5）每小時那列 HANDLER LOG 的兩個舊 gate**，細節與認領清單在 `docs/CMYDB_PORT_LEDGER.md`「A5」：
  - (a) TesterID 欄（golden 906_0625_Steven `cMyDB.cpp:1741`／SaveEventTracker `:1648`）**已解開**：`forms\fLotInfo.h:2204` 早就有 `edtASECL_TesterID`，gate 過期。V906 `cMyDB.cpp:2097`／`:1989`。那一欄從 `" "` 變成 ASE-CL Tester ID 輸入框的字（沒設＝`""`，golden 同）；`ELA_TimeData` 第 3 步多驗這一欄。
  - (b) **已知缺口**：換日時 golden `cMyDB.cpp:1793-1798` 呼叫 `TfLotInfo::UploadEventLogFile`（`uLotInfo.cpp:10829-10838`：把前一天的 `HANDLER LOG_*.csv` **本機複製**到 `D:\HT9045_Log\ASECL\`，不是網路上傳）。V906 沒有這支（`forms\fLotInfo.h:1053` EXIT REGISTER A），它是 St01 的檔 ⇒ `cMyDB.cpp:2167-2174` 維持 `#if 0`，等 St01 照認領清單補上宣告與本體。**20260929 A5 (b) 已補**（認領同意後 St02-E 做：宣告 `forms\fLotInfo.h:2216`、本體 `forms\fLotInfo.cpp` 檔尾，gate 改成 `//#if 0`），見 CMYDB 帳本 A5。跟 N25-5（前一天 EventLogTxt 經 FTP）是不同的檔，不能代替。
  - 上機要看：① 有設 ASE-CL Tester ID（N22 開、`config.ini` `[Lot Info] TESTER_ID` 有值）時，整點那列 `HANDLER LOG_<HandlerID>_yyyy_mm_dd.csv` 第 3 欄是那個值、沒設時是空的（以前是一個空白）；② 跨日後 `D:\HT9045_Log\ASECL\` 要出現前一天的 `HANDLER LOG_<HandlerID>_<昨天>.csv`（與 `asSaveEventLogPath` 那一份相同；A5 (b) 20260929 起）。

## P7a：分析器的手動 Save Summary（20260928，St02-E helper；**Steven 0928 第十題＝A：資料夾固定 D:\RMS\、不能選**）

分支 `v906/steven-elasched-wip` `8e8fc312`。兩組態只編譯（St02 不執行），0 錯；ctest `ELA_Service` 第 7 節待 St01；頁面 `node --check` 過。

- golden `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:2557-2570` `btnSaveSummaryClick`：`SaveDialog1->FileName = "D:\\RMS\\" + yyyy-mm-dd`（:2563），確定後
  `SaveSummary(ExtractFilePath, ExtractFileName)`（:2614-2688，R2 已移植＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaReports.cpp` `SaveSummary`）＝`<資料夾><sHandlerID>-SummaryData_<檔名>.txt`，內容是畫面上的表格。
- V906：`D:\HT9045\web\page\eventlog.html` 查詢列多了 Name（預設今天 yyyy-mm-dd＝golden 對話框的預設）與「Save Summary」鈕，結果（寫到哪、或錯誤）顯示在旁邊；
  `POST /api/ela/summary?name=`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaService.cpp`）排進 `Hub::PostSaveSummary`，由 **hub worker** 在 `RunOnce` 跑（排在等待中的查詢之後：先 Query 再 Save 會存那次查詢），
  結果是一筆 `JobRecord`（cmd `SaveSummary`），**頁面的查詢快照不換**。
- 資料夾：`HubPaths::summaryDir`＝golden `D:\RMS\`；`W906_RMS_ROOT` 設了就用它（Handler 的 D:\RMS 接縫，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp:263`）。
- **偏離**：
  1. 頁面只送檔名、不能選資料夾（golden 對話框可以選任何路徑）；檔名只收 A-Z a-z 0-9 空白 - _ . ( )，最多 100，不可有 ".."（不讓網路請求寫到別的地方）。
  2. VTEST／ChipAdv 的報表工作也在同一個 `analyzer_` 上查詢；它們跑過之後按 Save Summary，會先把頁面的查詢**再跑一次**（重新讀檔，之後寫進去的列會算進來），紀錄上會寫。golden 只有一組表格，工作跑完畫面就是工作的區間。
  3. 還沒查詢就按：不寫檔，記一筆「no query yet」。
- ctest `ELA_Service` 第 7 節：接縫（沒設＝`D:\RMS\`）、403／400（".."、反斜線、資料夾）／202、ServeHttp 裡什麼都不跑、沒查詢的紀錄、查詢後寫出 `<沙盒>\RMS\H1-SummaryData_Day1.txt` 而且頁面結果不變、
  VTEST 工作（測試用 getter 打開 O10、`asO19_SavePath` 在沙盒）之後先重跑頁面的查詢。
- ⚠ 未上機。上機要看：按 Save Summary 寫出 `D:\RMS\<Machine ID>-SummaryData_<檔名>.txt`，內容是畫面上那次查詢；Summary 分頁仍顯示使用者的查詢；帶資料夾的檔名被拒。
- **R6／P7a 的 POST 路由沒有登入等級檢查**（St02-E 審查 20260928，St02-M 要求記下）：`POST /api/ela/job`、`/api/ela/summary`、`/api/ela/query`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaService.cpp` `ela::ServeHttp`）不看 AccessLevel；唯一的 403 是 allowCmd=false，而 wb_serve 傳的 allowCmd 自 ZEROARG 起永遠是 true（`ElaService.cpp:144-145` 的註解），所以實際上什麼都不擋。
  現在可以接受，理由：(1) wb_serve 的網頁伺服器兩個組態都只綁 127.0.0.1——預設 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.cpp:381`、空的時候也是 127.0.0.1（:625-626），`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4305` 刻意不改；(2) golden 分析器視窗的這些鈕也沒有密碼：`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp` 裡沒有 AccessLevel／登入檢查（有 Password 的只有 FTP 帳密欄位 :3116／:3123）。
  注意：Origin 檢查（`WebBridgeServer.cpp:1089-1128`，允許的來源在 :1105-1110）只在 WebSocket 升級時做，這幾個 HTTP POST **沒有** Origin 檢查——機台瀏覽器若開了別的網頁，那個網頁可以盲送 POST 到 127.0.0.1:8045（拿不到回應，但工作會排進去）。
  **如果哪天網頁伺服器改綁區網介面，這三條路由要先加等級檢查**（還有上面這個 Origin 缺口）。

## P7 XLS 匯出：唯讀盤點（20260928，St02-E helper）＋做法 D（.xlsx）

- **★ 第五題＝D（Steven 0928）**（Steven 原話「你能做成xlsx就太好了」）。St02-E helper 20260928，分支 `v906/steven-xls-d`（本機，St02-E 審查後推）；只改頁面：沒有伺服器程式、沒有新路由、沒有外部程式庫或 CDN。取代做法 C 的 CSV（`e40f110a`）。
  - `D:\HT9045\web\page\ht9045_ela_xlsx.js`（新檔，`window.HtXlsx`，`eventlog.html` 用 `<script src>` 載入）：頁面自己組真的 .xlsx。
    - 容器＝手寫的**只儲存（不壓縮）ZIP**，照 PKWARE APPNOTE.TXT：4.3.7 本地檔頭（0x04034b50）、4.3.12 中央目錄（0x02014b50）、4.3.16 目錄結尾（0x06054b50）；壓縮方法 0（4.4.5）；CRC-32 多項式 0xEDB88320、初值 0xFFFFFFFF、結果取補數（4.4.7）；
      固定 DOS 日期 1980-01-01 00:00（4.4.6，同樣的資料出同樣的位元組）；沒有 data descriptor、extra、ZIP64、註解；檔名都是 ASCII（非 ASCII 才設 bit 11）。
    - 內容＝最小的 SpreadsheetML（ECMA-376 Part 1 §12.3／§18，Part 2 OPC）：`[Content_Types].xml`、`_rels/.rels`、`xl/workbook.xml`、`xl/_rels/workbook.xml.rels`、`xl/styles.xml`、`xl/worksheets/sheetN.xml`；沒有 sharedStrings、沒有 docProps。
  - **每格都是文字**（同 golden BIFF2 的 LABEL，`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\XLSfile.pas:178-192`）：`t="inlineStr"`（`<is><t xml:space="preserve">`）＋內建「文字」格式 numFmtId 49（"@"），Excel 不轉型（"0316" 還是 "0316"）；
    工作表的 `ignoredErrors numberStoredAsText` 關掉「數值儲存為文字」的綠色三角（golden 的檔會有）。第一列是表頭（粗體，golden 第 0 列）；欄寬照每欄最長的字（中文算 2，+2，4～60）；UTF-8；
    XML 跳脫（& < >）＋ST_Xstring 的 `_xHHHH_`（XML 不能放的控制字元、CR；資料裡原本的 "_xHHHH_" 的底線寫成 `_x005F_`）；超過 32,767 字的格截在 32,767（Excel 的上限）。
  - 檔名＝golden 預設名改 .xlsx，各一個工作表、表名＝檔名：`AlarmList.xlsx`（`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:2247`）、`FailureList.xlsx`（:2256）、
    `AlarmListByArea.xlsx`／`AlarmListByYield.xlsx`／`AlarmListByFunction.xlsx`（:2285／:2290／:2292，看 Filter 下拉與 By Yield 框）。
  - **Top5＝一個檔三個工作表**：檔名 `Top5Alarm.xlsx`——golden 的對話框預設就是 `C:\Top5Alarm.xls`（:2266，存 sgTop5Filter），另外兩個 `AlarmList.xls`（:2272，sgTop5Alarm）與 `AlarmList1.xls`（:2275，sgTop5）是寫在它旁邊；
    工作表照 golden 的順序與檔名：`Top5Alarm`、`AlarmList`、`AlarmList1`。
  - 按鈕字＝golden 的 `Save`（`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.dfm:658`／`:821`／`:1285`／`:1321`），旁邊寫檔名；結果（或錯誤）寫在查詢列。每個表格最多 5,000 列（快照上限 `ElaHub.cpp` `kMaxRows`），被截的表名寫在按鈕旁邊。
  - 偏離：存到瀏覽器的下載資料夾（golden 對話框預設 `C:\`）；Top5Alarm 只有看得到的列（golden 寫 RowCount 列，含尾端空白列）；AlarmList1 四欄（golden 的格線表多一個空欄）；
    golden 的 Top5 是三個檔，這裡是一個檔三個工作表（Steven 指定）；表頭粗體、欄寬、中文用 UTF-8（golden 是 cp950、沒有 CODEPAGE）。
  - 檢查：`node --check`（`ht9045_ela_xlsx.js` 與 `eventlog.html` 的內嵌 script）。沒有執行 node、沒有開瀏覽器。
  - ⚠ 未上機。**上機要看**：
    1. 每個鈕下載的 .xlsx 在 Excel 2007 以上**雙擊直接開，沒有「修復」或任何警告**（LibreOffice 也要開得起來）；
    2. 中文正確；
    3. "0316" 這類代碼仍是文字（前導 0 在，沒有變成數字或日期）；
    4. `Top5Alarm.xlsx` 有三個工作表 `Top5Alarm`／`AlarmList`／`AlarmList1`。

- **做法 C（已被 D 取代）**（St02-E helper 20260928，`v906/steven-elasched-wip` `e40f110a`；只改頁面，沒有伺服器程式、沒有新路由）。以下是當時的紀錄：
  - `D:\HT9045\web\page\eventlog.html` 的 Alarm／Fail／Top5／By Filter 分頁各一個「Save CSV」：用頁面已經有的表格資料組 CSV，Blob＋下載連結由瀏覽器下載；Top5 一次三個檔（間隔 400 ms），跟 golden 一樣。
  - 檔名＝golden 預設名、副檔名改 .csv：`AlarmList`（`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:2247`）、`FailureList`（:2256）、`Top5Alarm`（:2266）＋`AlarmList`（:2272）＋`AlarmList1`（:2275）、
    `AlarmListByArea`／`AlarmListByYield`／`AlarmListByFunction`（:2285／:2290／:2292，看 Filter 下拉與 By Yield 框，同 golden 的 rgFilter／chkByYield）。
  - 格式：UTF-8＋BOM、CRLF、RFC 4180（有逗號、引號、CR、LF 的欄位加引號，引號變兩個）、每格都是文字、第一列是 golden 的表頭（第 0 列）。
  - 偏離：存到瀏覽器的下載資料夾（golden 對話框預設 `C:\`）；Top5Alarm 只有看得到的列（golden 寫 RowCount 列，含尾端空白列），AlarmList1 四欄（golden 的格線表多一個空欄）；
    每個表格最多 5,000 列（快照的上限，被截的檔名會在按鈕旁邊寫出來）。
  - 檢查：`node --check`；用 node 跑 `csvText`（引號、BOM、CRLF）。上機要看：每個鈕下載 golden 名字的 .csv，Excel 開起來中文正確；Top5 三個檔（瀏覽器可能先問一次能不能下載多個檔）。

- **四個按鈕、六個表格**（`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp`）：`btnSaveAlarmClick` :2245-2252 → `sgAlarm`（預設 `C:\AlarmList.xls`）；`btnSaveFailClick` :2254-2261 → `sgFail`（`C:\FailureList.xls`）；
  `btnTop5AlarmClick` :2263-2278 → `sgTop5Filter`（`C:\Top5Alarm.xls`）＋同資料夾 `sgTop5Alarm`（`AlarmList.xls`）＋`sgTop5`（`AlarmList1.xls`），一次三個檔；
  `btnSaveByFilterClick` :2280-2297 → `sgByArea`（`AlarmListByArea.xls`／`AlarmListByYield.xls`／`AlarmListByFunction.xls`，看 rgFilter 與 chkByYield）。
  SaveSummary 裡的 ByDay／ByHour XLS 是註解掉的（:2624-2628、:2634-2638）。都用 `TSaveDialog` 讓使用者選路徑。
- **寫法**：`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\SgdToXLS.cpp:11-14` 只是轉呼叫 `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\XLSfile.pas:91-111` `StringGridToXLS`（ktop 的 Yudi Wibisono XLSFILE 元件，dllee 修改）：
  - 記錄：`BOF` opcode `0x0809`（`BOF_BIFF5`）但內容是 BIFF2 的 6 位元組（版本 0、`DOCTYPE_XLS`＝0x0010、0；:134-141）→ `DIMENSIONS` `0x0000`（BIFF2，8 位元組：rwMic 0、rwMac **65535**、colMic 0、colMac `max(100, ColCount+1)`；:143-151，註解的欄名寫反但值對）→
    每格一筆 `LABEL` `0x0004`（BIFF2：row、col、3 位元組屬性全 0、1 位元組長度、字元；:178-192）→ `EOF` `0x000A`（:268-272）。
  - **只有一個工作表、沒有表名、沒有字型／格式／XF／CODEPAGE**；每一格都是文字（數字也是，Excel 會標綠色三角）；列上限 65535（:99-100）。
  - 編碼：`AnsiString` 原樣寫出（cp950），沒有 CODEPAGE 記錄，Excel 用系統字碼頁解讀。
  - golden 的坑：字串長度存在 1 個 byte（`slen:=length(avalue)`，:184），超過 255 byte 的字會變成「長度 mod 256」被截掉；檔案已存在時用 `fmOpenWrite` 開、**不截斷**（:118-121），舊檔比較長時 EOF 後面留著舊資料（Excel 讀到 EOF 就停，所以看不出來）。
- **可行的做法**（Excel 怎麼開）：
  1. 小的 BIFF2 寫入器，照 golden 的位元組（約 60 行 C++，可以用 Python 鏡像比位元組）：Excel 當成舊版 .xls 開，沒有「格式與副檔名不符」的警告；但 Excel 2 格式在新版 Office 可能被「檔案封鎖設定」擋或用保護檢視開（要上機看）。
     中文要寫成 cp950（偏離 D-e 的 UTF-8；BIFF2 沒有 Unicode）；超過 255 byte 的字截在 255（W18 B 修掉 mod 256）；檔案先截斷。
  2. XML 試算表（SpreadsheetML 2003，副檔名 .xml）：UTF-8、可以有型別、Top5 三個表格可以放一個檔三個工作表；Excel 直接開（.xml 用 mso-application 指示）。若硬取名 .xls，Excel 每次都跳「格式與副檔名不符」。
  3. CSV 取名 .xls：Excel 跳「格式與副檔名不符」後用文字開；UTF-8 沒 BOM 會亂碼。**不建議**。
  4. 真的 .csv（UTF-8＋BOM）：Excel 直接開，中文正確；golden 的格本來就全是文字，沒損失；可以只在頁面做（快照裡已經有這些表格，但每個表格最多 5,000 列，`ElaHub.cpp` `kMaxRows`）。
  - 存到哪：golden 存在分析器那台電腦（對話框預設 `C:\`）；網頁版最自然是瀏覽器下載（`<a download>`，不用改 wb_serve 的標頭）。

## W22：N25-4 Jam_Summary.csv／N25-5 前一天 EventLog.txt（分析器端其他的上傳）（20260928，St02-E helper；計畫 `D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md` §1.2）

分支 `v906/steven-ela-wip`：`7b69cdc1`（在 gpib-widget `c66f1eb1` 之上；之後合併 gpib-widget `78be9552`（P7a／P7）再兩組態重編）。兩組態只編譯（St02 不執行），0 錯；ctest `ELA_Ftp` 第 15～17 節、`ELA_Schedule` 的 W22 檢查（第 3／4／5／7／14／16 節）、`ELA_Reports` 第 6 節待 St01。
行號：分析器＝Rev891 `uChipMosZHUBEI_Func.cpp`（另註檔名的除外），Handler＝906_0625_Steven。

- **golden 怎麼做**：Handler `TfMain::Timer2Timer` 00:00:01～04：`bN25_4_EnableUpload` 開 → `EL_UPLOAD_SUMMARY`（main.cpp:21449-21452）、`bN25_5_EnableUpload` 開 → `EL_UPLOAD_EVENTLOG`（:21454-21457），都在 `#ifndef SOFT_SIMULTE`（:21425）裡；
  開關與路徑 `sN25_4_UploadPath`／`sN25_5_UploadPath`（預設 `/Summary/naslfs2/Handler/`）只在 CC_ChipMos_ZHUBEI 851 才載入（cConfiguration.cpp:3729、:3745-3748）。分析器照佇列順序 JAMWEEK＞SUMMARY＞…＞EVENTLOG 跑（Analyzer.cpp:161-183）。
  - N25-4 `UploadSummaryCount`（:352-370）：本機 `D:\HT9045_Log\SummaryCount\<y>\<m>\<d>`，有 `HadUpload.txt` 就不做；`SaveJamSummaryFor30Days`（:122-132 → :58-120）取 Now-31 00:00:00 ～ Now-1 23:59:59.059 的列，
    Recovery 含 "CLEAN_OUT" 的列切一段（AlarmCode 含 "JAM" 的列只算 JAM，else-if），每段一列 `Time,Summary,JamCount`＝段尾時間、那段的 Production_Log 顆數、JAM 筆數。
    顆數＝`AnalysisProductionLog::GetDeviceCount`（uAnalysisProductionLog.cpp:62-79／:101-121／:172-191）：`D:\HT9045_Log\Production_Log\yyyymm\<hostname>_yyyymmdd.csv` 第 5 欄 In Time 落在段內的列。
    `N25_UploadSummaryCountToFTP`（:257-326，跟 N25-3 一樣的 base／hostname／BackUp、N25-2 帳號）成功才寫 "HadUpload"（沒有 Upload path 兩行）。
  - N25-5 `UploadEventLog`（:372-390）：本機 `D:\HT9045_Log\UploadEventLog\<y>\<m>\<d>`；`SaveEventLog`（:134-143）把昨天的 `D:\HT9045_Log\EventLogTxt\yyyy\mm\EventLogTxt_yyyymmdd.csv`（uAnalysisEventLogText.cpp:27-41）
    `CopyFile` 成 `EventLog.txt`，用 N25-3 的 `N25_UploadJamDataToFTP` 上傳（兩行 Upload path＋"HadUpload"）。
- **V906**（`EventLogAnalysis/ElaChipMos.*` 的 N25-4／N25-5 區塊；`Hub::RunUploadJob` 兩個新分支；`ElaHub.cpp` RunOnce 把 SUMMARY／EVENTLOG 送進去——**沒有 HOLD 的 EL_* 工作了**）：
  - 同一套傳輸與驗證：`IElaFtp`、STOR＋SIZE＋LIST（`UploadAndVerify`）、FTP_Log、一次一個 session。N25-3 的主檔＋BackUp 上傳抽成 `UploadMainAndBackup`（每個工作自己的 FTP_Log 文字），N25-3 的行為與文字不變（`ELA_Ftp` 第 3～14 節照舊）。
  - 排程（R5）：工作表接在最後加 `JOB_N25_4`（"N25-4"）、`JOB_N25_5`（"N25-5"）（狀態檔用名字當 section，舊檔照讀）；閘＝N25-3 那一套（851、有效 O10＝D-a、各自的開關＝D-f、N25-2 主機、SIM 組態不自動）；FTP、W＝900 s、每天 00:00。
    三個 FTP 工作同一個 Machine-ID offset、同一個時段，Tick 依 id 順序跑＝golden 的 JAMWEEK＞SUMMARY＞EVENTLOG。本體 `RunN25_4OnHub`／`RunN25_5OnHub`（`ScheduleUseHubJobs` 註冊，六個＋N17 自己的）：
    資料範圍、日期資料夾、BackUp 檔名都用時段；驗證成功＝verified；沒東西可送（30 天沒有列／昨天沒有檔）＝nothing to do（這個時段算完成，不重試）；閘沒開＝failed（不重試）；其他＝retry（退避）。
  - 手動：`POST /api/ela/job?id=N25-4`／`N25-5`；Auto Jobs 分頁照排程送來的工作列表，自動多兩列（頁面只改第 23 行的說明）。
- **偏離**（`keepGoldenBugs`＝golden 只影響第 6 項）
  1. **D-f**：開關（config.ini `[ChipMos Function]`，唯讀）、851、O10、N25-2 主機、路徑都有才建傳輸；golden 分析器不看開關（Handler 開著才送）。
  2. **驗證＋HadUpload**：主檔與 BackUp 都驗證才寫；golden 由 BackUp 的結果單獨決定（:307-314／:235-243）。N25-5 的 "Upload backup path:" 寫 BackUp 資料夾（golden :237 印主資料夾，同 N25-3）。N25-4 只寫 "HadUpload"（照 golden :368）。
  3. **沒東西就不連線**：本機檔不在就不連（golden 先連線再看檔，:296／:223；N25-5 昨天沒有檔時 golden 的 `CopyFile` 靜靜失敗、照樣連線）。
     FTP_Log 多寫 "N25-4 Jam_Summary upload"／"N25-5 EventLog upload"、"File not exist N25-5"、"No event log N25-4"（golden 這兩個工作自己不寫）。
  4. **重算**：`Jam_Summary.csv` 每次先刪再寫（golden `WriteDataToFile` 附加：同一天重試會重複表頭與每一列）；每行經 `ReportEncode`（D-e；內容本來就是 ASCII）；LF 照 golden。
  5. **D-c**：30 天內沒有任何列時 golden 讀空向量的 `tIntervalRecs[size-1]`（:95，G9，當掉）；這裡不寫檔、不上傳，記 nothing to do（跟 R3 N34 同一個做法）。
  6. **W18 B（顆數）**：golden `for(dtCurrent=dtstart; dtCurrent<=dtend; dtCurrent+=1)`（uAnalysisProductionLog.cpp:69）從段頭的時刻一天一天跳，段尾的時刻比段頭早（被 CLEAN_OUT 切在白天的段）就少讀最後一天的檔；
     這裡讀段頭那天到段尾那天的每個日檔（列本身照 golden 用時間比）。`keepGoldenBugs`＝golden。
  7. **N25-5 的複本要驗證**（大小跟來源一樣）；複本照 golden 是位元組複製（cp950，不是報表，D-e 不適用）。
  8. golden 的 SOFT_SIMULTE 帳號覆寫（:279-283）不搬；模擬組態原本走只寫 log 的傳輸（★W36 A），★W36＝C（Steven 20260928）後跟出貨組態一樣用 WinINet。
- **照翻的 golden 事實**：段的兩端都含（剛好在 CLEAN_OUT 那一秒的顆會算進前後兩段）；AlarmCode 含 "JAM" 就算 JAM（那列的 Recovery 也是 CLEAN_OUT 時不切段）；
  顆數檔名用主機名（gethostname，Analyzer.cpp:429-432，不是 Machine ID）；Production_Log 用 `std::ifstream`＋`getline` 讀（第一行當表頭；CRLF 的 CR 去掉）；
  `getColumn`（Common.cpp:442-467）引號只切換、不留下；`ParseDateTime(AnsiString)`（Common.cpp:692-713）不到 19 字＝0、時間只取 8 個字。
- **Steven 0928 第十一題＝A（照 golden）**：N25-5 只找 golden 的一般檔名 `EventLogTxt_yyyymmdd.csv`；用 O15（`EventLogTxt_<ID>_yyyymmdd.csv`）或 N10（`<Model>_<ID>_EventLogTxt_yyyymmdd.csv`）檔名、或不是每天一個檔的機台，昨天的檔找不到＝不上傳（golden 也是）。
- **另外修的**：`ELA_Reports` 第 6 節原本檢查 "EL_UPLOAD_JAMWEEK: not ported (HOLD"——R4 合併（`9f9cb4ad`）之後 JAMWEEK 已經會跑，這個檢查一直會失敗；改成三個上傳工作在 915 機台被客戶碼閘擋下。
- **ctest**：`ELA_Ftp` 第 15 節（N25-4：GetColumn／ParseDateTimeText；顆數 W18 B 3 對 golden 2；Jam_Summary.csv 逐位元組；遠端步驟；HadUpload 只有標記；同一天第二次不連；golden 模式；D-c；五個閘；BackUp 驗證失敗→重試→重算只有一個表頭）、
  第 16 節（N25-5：位元組複本、遠端步驟、HadUpload 兩行＋標記、昨天沒有檔／只有 O15 檔名＝不連且 FTP_Log 有記、閘、FTP_Log 沒有帳密）、
  第 17 節（Hub：開關關＝skipped、開＝兩個都驗證、SUMMARY 一定在 EVENTLOG 前；排程本體 verified／同一時段再跑 nothing to do／沒有檔 nothing to do／開關關 failed／沒有 hub no body）。
  `ELA_Ftp` 另把 `W906_PRODLOG_ROOT` 指進沙盒。`ELA_Schedule`：閘、SIM、種類與 900 s、每天 00:00、三個 FTP 工作同一時段依序（00:10:07，N25-3→N25-4→N25-5）、讀兩個鍵、七個本體。
- ⚠ 未上機。上機要看（851、O10 開、`bN25_4_EnableUpload`／`bN25_5_EnableUpload` 開、出貨組態）：00:00＋offset 緊接 N25-3 之後，FTP 上有 `<path>/<hostname>/Jam_Summary.csv`、`EventLog.txt` 與 BackUp 的兩份；
  本機 `D:\HT9045_Log\SummaryCount\…`、`D:\HT9045_Log\UploadEventLog\…` 有 HadUpload.txt；Jam_Summary.csv 的 Summary 跟 Production_Log（`<電腦名>_yyyymmdd.csv`）對得上；FTP_Log 每一步都有、沒有帳密；
  Handler 00:00 剛換檔時昨天的 EventLogTxt 能不能複製（CopyFile 的分享模式）。
