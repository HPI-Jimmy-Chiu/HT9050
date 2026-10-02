# Census 129 (e) - golden objects, registrations, timers and periodic work that the V906 port never creates or never runs

Generated 2026-10-01 06:22 from `census129_e.tsv` by `tools/mk_md_final.py`.  READ-ONLY census: no repo file was edited; no build, no ctest, no wb_serve was run.

## 0. Trees, machine facts, how to read the TSV

| item | value |
|---|---|
| port measured | `D:\HT9045\.claude\worktrees\b9\HT9011UC_Cpp_V3.33.906.0` = GitLab main (f528311a when the re-base started, 14b278a4 at the final validation, 2026-10-01 06:07; the two newer commits touch only vclcompat/Controls.h, tests and a webprobe script). Rows were first written on mach0930 (12fe15e4) and all 360 re-validated on main. |
| golden | `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618` (BCB6, cp950, untracked; searched with `git -C <tree> grep --no-index -a`; line numbers taken with a Python cp950 reader, never `iconv | sed`). V912 golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy` used as cross-reference only. |
| machine tables | `machines\HT9050\IO_Table.csv` (1,124 lines: 969 named rows, 155 blank) and `Mot_Table.csv` (48 motors); identical on both port trees |
| machine options | this laptop's `D:\HT9045\system\Gerneral.ini` and `config\config.ini` (read only): EP_Install=3, NUMBER_PANEL_TYPE=3, CUSTOMER_CODE=957 (=CC_PTI, Powertech), INDEX_SUCKER_TYPE=1, USE_16_HEATER=0 (=eht4Heater), REAL_TIME_CCD=0, USE_ATC_MODE=5 (eNonChamber), Tri_Temp_Machine=0, BAR_CODE_INSTALL=0, INSTALL_OCR=0, USE_AUTO_ALIGNMENT=0, USE_ROTATE_KIT=0, USE_PICKER_COUNT=1, USE_TRAY_MAPPING=1 (TRAY_MAPPING_GRAB=0), AGVModal=0, [SECS GEM] Enable SECS GEM=0, N14_HandlerOEEUseFunction=0, LoaderUnload_StepMotor=0. **This is the laptop's ini, not necessarily the real HT9050 machine's** - a row marked "inert here" may be live on the machine. |
| SOFT_SIMULTE | a golden block inside `#ifndef SOFT_SIMULTE` is live only in a SHIP build (`-DW906_NO_SOFT_SIMULTE=ON`); rows say which polarity matters. |

TSV columns: `id, kind, name, golden_file_line, port_status_with_citation, what_is_dead_because_of_it, action_class, confidence, notes, provenance`.  `provenance` answers "hand-read or tool-derived" per row (see section 1.2).  Status words in column 5: ABSENT (no port code), STUB (empty / constant body), GATED (`#if 0` / TODO / option never true), NO_CALLER / DEF_NO_CALL (translated, nothing calls it), PARTIAL (part of it is driven by a W906 slice), ALIVE (checked, not a gap).  Confidence = probability that the row is a REAL dead feature whose golden dependencies exist in the port (lower when a dependency is missing; "UNDECIDED" in the text when it cannot be decided).  The machine-table and customer annotations (`MACHINE_IO[...]`, `CUSTOMER_SPECIFIC`) in `notes` were added mechanically to every non-IO row.

## 1. Counts

360 rows in `census129_e.tsv`: 202 with confidence >= 70 (35 if the 202 IO_Table rows are left out), 48 with 40-69, 110 with < 40.  8 rows are CHECKED_ALIVE (verified not gaps, kept so nobody re-checks them).

| action_class (all rows) | rows |
|---|---:|
| STATE | 194 |
| IO_OUT | 92 |
| COMM | 34 |
| FILE_WRITE | 15 |
| DISPLAY | 13 |
| MOTION | 12 |

Without the 202 IO_Table rows: STATE 55, COMM 34, IO_OUT 29, FILE_WRITE 15, DISPLAY 13, MOTION 12.

| row family (id prefix) | rows |
|---|---:|
| E-IO-* | 202 |
| E-T1-* | 31 |
| E-TM-* | 18 |
| E-BOOT-* | 15 |
| E-T2-* | 13 |
| E-FT2-* | 12 |
| E-FT1-* | 10 |
| E-T3-* | 10 |
| E-CG-* | 7 |
| E-RT-* | 7 |
| E-TH-* | 7 |
| E-REG-* | 6 |
| E-SMC-* | 6 |
| E-SUCK-* | 6 |
| E-FTF-* | 5 |
| E-OK-* | 3 |
| E-FT-* | 1 |
| E-MOT-* | 1 |

| kind | rows |
|---|---:|
| IO_ROW_NO_OBJECT | 203 |
| TIMER_SEGMENT | 61 |
| TIMER | 29 |
| BOOT_GROUP | 27 |
| OTHER | 10 |
| CHECKED_ALIVE | 8 |
| THREAD | 7 |
| TIMER_RUNTIME | 7 |
| TIMER_FAMILY | 5 |
| CALLEE_GATED | 1 |
| TIMER_PARTIAL | 1 |
| TIMER_WHOLE | 1 |

### 1.1 Validation

Every row was checked mechanically against the port tree (`tools/validate.py`: each `path:line` token in column 5 must exist and be within the file, a quoted snippet directly after it must occur within +-6 lines, and `IDENTS0=name:max` absence claims are re-counted with `git grep -w`): rows 360 cites 413 ok 413 quotes 269 ok 269 flagged 0 (validate.py on the b9 worktree, HEAD 14b278a4, 2026-10-01 06:21).  The golden-side citations (column 4) are not machine-checked; they were read with `effview.py` / `gl.py` when the row was written.

### 1.2 Provenance (hand-read vs tool-derived)

| provenance (first words) | rows |
|---|---:|
| tool-derived | 207 |
| hand-read | 96 |
| hand-read by sub-agent A | 31 |
| tool-derived name-level callee census, every U/M item then hand-read | 15 |
| tool-derived counts + hand-read verdicts | 11 |

Sub-agent A (TfMain::Timer1Timer, 31 rows) was dispatched before the no-sub-agent order and its rows were reviewed and machine-validated (87/87 cites, 72/72 quotes).  Sub-agent D (boot) died on the API rate limit without output and the planned E1 sub-agents never ran; both segments were done by hand afterwards (E-BOOT-*, E-FT*-*).  Of the 105 non-TfMain timers about 40 handler bodies were read (fully or by their head); the other 65 are covered by 5 tool-derived family rows (E-FTF-*, confidence 8) and the display-only row E-FT-DISP (confidence 5) and must be read before anyone acts on them.

## 2. Top 15 for this machine table (ordered by relevance to `machines/HT9050`, then action class, then confidence)

Raw confidence alone would put E-SMC-001 (rotate kit, 88), E-TM-001 (ScanKey, 90) and E-CG-005 (hot-plate dispatcher stub, 80) in the list, but on this laptop's configuration they are inert (USE_ROTATE_KIT=0; FK panel keys Enable=0; no hot-plate rows), so they are ranked by relevance instead.  Rows 6, 7 and 13 are new in this final pass.

| # | id | what | golden | port status | what is dead / effect |
|--:|---|---|---|---|---|
| 1 | E-T2-001 | SwBigFan output never driven (FAN ON/OFF button) (IO_OUT 95) | main.cpp:20968 Timer2Timer `SW[SwBigFan].OnOff(LastSet.bBigFan)` | ABSENT: FileRW/MainClick.cpp:1407/:1419 only flip LastSet.bBigFan | the fan never switches; SwBigFan is Enable=1 in the machine table (IO_Table.csv:20) |
| 2 | E-T1-012 | SwSafeDoorLock follows SystemStart (INBOX 65) (IO_OUT 85) | main.cpp:3051-3064 Timer1Timer | ABSENT: only boot error paths write it (cinitial.cpp:8130 ...) | door-lock solenoid never energised while running, never released after stop; SwSafeDoorLock Enable=1 |
| 3 | E-T2-008 + E-TH-001 + E-RT-007 | heater chain: bUT150Install never filled, THeaterThread never created, UT150 / DT4848 / KT4H drivers not run (IO_OUT 85) | main.cpp:20743 HotplateHeatMode, :20266 IndexHeatMode, :18613 Index16Heater; uHeaterThread.cpp:363/:436 | ABSENT + documented (JsonBridge/StageThermo.cpp:51-58); empty in SHIP builds | no channel read / compare / alarm / soak in a ship build; every consumer of bUT150Install (about 80 sites) sees no heater |
| 4 | E-T3-001 | eight fYieldMonitoring->Check*YieldAlarm calls have no caller (STATE 92) | main.cpp Timer3Timer; uYieldMonitoring.cpp:349 ... :2031 | NO_CALLER x8 (bodies translated, declared in forms/fYieldMonitoring.h:286-299) | by-site / by-arm / low / total / interval / picker yield alarms never fire (WAR0721 / WAR0722, auto-clean start, CleanOut) |
| 5 | E-TM-002 | tESDError queue: 11 producers, no consumer (COMM 85) | main.cpp:30861-30945 TimerESDTimer drain | ABSENT consumer: TesterComm/Handler/HandlerGpibMsg.cpp:1235 `tESDError->Add("MES0731");` + 10 more producers | every tester-side ESD / GPIB alarm (MES0731 stop-from-tester, WAR07326/7/8, ...) never reaches the operator |
| 6 | E-FT2-001 | OLP host link (CC_PTI / MTI / Greatek) never instantiated: TfAutomation::tmrOLPTimer, OLP server 6671, ProcessBuffer, reports  [NEW in this pass] (COMM 65) | Automation/automation.cpp:435-644 | TRANSLATED BUT NOT WIRED: Automation/automation.cpp:2795 `Nothing in this tree calls AutomationEngine() yet`; fAutomation is the no-op TfAutomationShim (atester_shims.cpp:367) | no host listener, no ALARM / PRODUCTION / TEST_RESULT reports, no host commands, PTI start-mode handshake skipped; this laptop is CC_PTI |
| 7 | E-FT2-011 | RUN_INFO::SaveJamRateByDay body is #if 0: the By-Day jam file is not written at midnight rollover  [NEW, live data loss] (FILE_WRITE 70) | cprod.cpp:995-1110; call sites cprod.cpp:970, HS_Function.cpp:187, main.cpp:11435 | BODY GATED: cprod.cpp:1072 `#if 0 // TODO(GA1-B2)`; only FileRW/MainClose.cpp:219 (exit path) writes | the first JAM after midnight calls SaveJamRateByDay() + InitialDailyData(): yesterday's counts are discarded, no DailyJamRate.txt |
| 8 | E-TM-004 / -005 / -008 / -014 | periodic file writes with no caller: SaveSiteYield("By Interval"), production record [O06-8] + TimerRecordLoaderDate, TemperatureStorageLog(0), RecordJamRateByTime [O11] (FILE_WRITE 85) | main.cpp TimerESDTimer / Timer8Timer / TimerTemperatureStorageMinuteTimer | NO_CALLER (periodic drivers absent) | the option-enabled periodic yield / production / temperature / jam-rate files are never produced mid-session |
| 9 | E-BOOT-005 | RunInfo.Factory = HandlerSystem->GetCustomerName() not executed at boot (COMM 80) | main.cpp:10615 `RunInfo.Factory=HandlerSystem->GetCustomerName();` | ABSENT: HandlerSys.cpp:821 (only declared), cObserver.cpp:7467 reads it | SECS SV 1005 "Factory" and the Observer factory label stay empty |
| 10 | E-T3-003 + E-RT-001 + E-SMC-004 | bin-count display panel: DoShowBinDigital / ChangeBinDispStatus have no caller, HSys.BinDisCtrl never created (NUMBER_PANEL_TYPE=3 here) (COMM 60) | main.cpp:25237-25241 Timer3Timer | NO_CALLER: cShowBinSelect.cpp:2405 / :2253 defined, never called | physical bin-count panels are neither installed nor refreshed; the bin-display error strip never updates |
| 11 | E-T2-003 + E-BOOT-003 | EP regulator / ADAM-6024: write and read-back layer is a no-op (EP_Install=3 here); dependency (ADAM Modbus/TCP driver) missing (IO_OUT 40) | adam6024.cpp:1798-1915; main.cpp:20980-21104 | STUB: atester_shims.cpp:326-327 empty bodies | die-force / contact-air pressure never written, chamber-door zero-pressure safety (main.cpp:21048-21053) never acts |
| 12 | E-T1-015 | Index-sucker pause pump (INDEX_SUCKER_TYPE=1 here): suck / destroy sequences not driven to completion on PAUSE or while a box is open (IO_OUT 65) | main.cpp:3098-3114 Timer1Timer | ABSENT: of the Timer1 slices only CheckIndexAllSuckICFallDown runs in the modal tick (tools/wb_serve.cpp:7630) | vacuum outputs stay half-done, bIndexCheckNoStopVaccum stays true |
| 13 | E-FT1-001 | work golden runs inside alarm / message boxes (TfNote, TMyMessageBox, TfAGV timers) that the port wait loop does not  [NEW] (STATE 50) | note.cpp:3143-3525; mymessbox.cpp:538-757; AGV.cpp:1290-1307 | PARTIAL BY DESIGN: tools/wb_serve.cpp:7620 W906_ModalWaitTick covers about half | tray scan, safe-door logging, digital-status pass, heater supervision, PLC-safety check and E84 handshake pause while any alarm box is open |
| 14 | E-BOOT-002 | ServoOnAllMOT() empty stub: no servo is switched on by the boot (MOTION 45) | main.cpp:10165 -> Motor/mymotor.cpp:4609-4621 | STUB: Motor/mymotor.cpp:2564 `void ServoOnAllMOT()                   {}` | after a cold start every servo is free until the operator sends Motor Power |
| 15 | E-TM-011 | Timer6 start-check absent (SECS RCMD START timeout WAR16110, ChipMOS FTP auto-start, GM-Test parameter check) (STATE 70) | main.cpp:31299-31548 | ABSENT and KNOWN: WebStart.cpp:3684 `//MARKED(W906-ST-S3-B1)` | those three start triggers can never reach SoftStart=true; customer / option specific |

### 2.1 Live defects found while tracing (not only missing features)

* **E-FT2-011**: data loss at the midnight rollover (see top list, rank 7)
* **E-T1-030**: LIVE HANG, ChipMOS-style option only: bNeedManualCheckEmptyTray is never cleared when SnEmptyTrayIsLock1 opens (CosFunction.bOpenDoorCheckLoaderAfterTrayEnd); SnEmptyTrayIsLock1 is Enable=0 on this machine table
* **E-T3-002**: latent: iInitialSoakTimer / iSoakTimer never recomputed, the HandlerSoak status bit stays 1 after the first soak; masked in ship builds because fHeaterOK is only set true in the SOFT_SIMULTE branch (uHeaterThread.cpp:508)
* **E-FT2-005 / E-T2-003**: no value shown or written for the EP regulator (EP_Install=3): the operator sees an empty EP label and the regulator is never commanded

### 2.2 Suggested order of work (non-binding; every item that arms an output or a link needs your decision first)

1. **File writes and display values, no machine motion** (low risk, can be done and verified on the laptop): E-FT2-011 (un-gate SaveJamRateByDay or call the wb_serve copy from the rollover), E-BOOT-005 (one assignment), E-TM-004 / -005 / -008 / -014 (periodic drivers; each behind its option flag), E-T3-003 (bin-display refresh), E-FT2-002 (00:00 jobs of the HS housekeeping timer).
2. **Outputs with Enable=1 in the machine table**: E-T2-001 (SwBigFan, one line in Timer2), E-T1-012 (SwSafeDoorLock: interlock design, INBOX 65), the heater chain (E-T2-008 / E-TH-001: large, needs the UT150 / DT4848 / KT4H drivers and a heater-thread ruling).
3. **Decisions, not code**: OLP wiring for PTI / MTI / Greatek (E-FT2-001: arms host START / HOME / PAUSE), SECS engine (E-FT2-003: planned for wb_serve), ADAM-6024 driver for the EP regulator (E-T2-003: dependency missing), servo-on at boot (E-BOOT-002), work inside alarm boxes (E-FT1-001).
4. **Verify before fixing**: any driver fix makes its whole closure live at once (E-CG-006: 596 functions / 13,676 statements); each needs a verification pass over what it will start running.

## 3. What was found, by brief item

**Item 1, IO objects.**  Constants and registrations are identical to golden (E-OK-001); the machine table is ahead of the code: 202 IO_Table rows have no object at all, 167 of them Enable=1 (E-IO-*, one row each; golden 906 and V912 do not know those names either, so this is a table/code version skew, not a port loss).  155 objects that golden references live are never referenced by the port; only 3 are wired and enabled in the machine table (SwBigFan -> E-T2-001, SwCCDZBreaker [documented gate], SwIonBarPower [golden-dead]) (E-CG-001).  All 147 sucker rows are Enable=0 and the sucker binder is alive (E-SUCK-*; the INBOX 132 premise was wrong on main).

**Item 2, motor aliases.**  274 golden `SetAlias` entries vs the port vs the 48 Mot_Table rows: one intentional rename (MInShutte1/2 -> MInShuttle1/2, E-MOT-001); nothing missing.  The related hole is the modular-machine motor-id tables (E-SMC-001, `MInRotate`/`MOutRotate`, inert with USE_ROTATE_KIT=0) and the boot servo-on stub (E-BOOT-002).

**Item 3, periodic work.**  All 14 TfMain timers were read statement by statement (E-T1-*, E-T2-*, E-T3-*, E-TM-*); the ten worker threads and 8 code-created timers have rows (E-TH-*, E-RT-*); the 105 other form timers are E-FT1-* (motion / output / tool pages), E-FT2-* (state, communication, file write), the tool-derived family rows E-FTF-* and E-FT-DISP (section 1.2).  The class-scoped census says 94 of the 119 handler bodies are not defined in the port and 25 are defined but never called, and none is driven by a real TTimer; a number of them are nevertheless covered under other names (W906 slices), which is why the CHECKED_ALIVE rows exist (E-FT2-004 TfTesterTCP, E-FT1-004 TfMotorTest, E-T3-004 A01 auto-logout).

**Item 4, boot registrations.**  The TfMain ctor / FormCreate / FormShow were analysed against the port boot (E-BOOT-000..014): 194 distinct callees, 121 reachable in the port, 25 defined but unreachable, 48 not defined; every U/M item was then checked by hand, because the FileRW layer re-implements form-level boot reads under other names (E-BOOT-004, E-BOOT-014 record what is ported).  Real gaps: ServoOnAllMOT stub (E-BOOT-002), Open_ADAM_6024 (E-BOOT-003), RunInfo.Factory (E-BOOT-005), directory creation (E-BOOT-008), InitCommonString (E-SMC-003).  The ctor/FormCreate/FormShow bodies of the other 132 forms were NOT read (E-CG-002).

## 4. Method, exact commands and denominators

All scripts live in `census129\tools\` (Python 3.13, `PYTHONIOENCODING=utf-8:replace`); every script was written to a file first (the Bash tool halves backslashes in inline Python). Whole-tree searches used `git -C <tree> grep -n -a -E ... -- '*.cpp' '*.h' '*.inc'` (port) and `git -C <golden> grep --no-index -a ...` (golden, untracked), never `grep -r`. Golden line numbers come from a Python cp950 reader (`gl.py`, `effview.py`), not from `iconv | sed` (shifted by 1 line in one case). Absence claims carry the command and the time in the row.

### 4.1 IO objects (brief item 1)

| step | command / script | result |
|---|---|---|
| constants | `s1_consts.py`: every `const int Sw*/Sn*` of golden 906, V912 and the port `cmydef.cpp` | golden 906: 1,070; V912: 1,072; port: 1,072; all 1,070 golden constants exist in the port with identical values; the +2 are `SwCassetteEmptyMotBreaker` (369) and `SwCassetteAuto3MotBreaker` (370) (cpp 0039) |
| registrations | `s1_regs.py`: `SW[..].Name=`, `Sen[..].Name=`, `Cylinder[..].CylinderName=` | golden 331 SW / 790 Sen / 260 Cyl; port 333 / 790 / 260; names identical (E-OK-001) |
| IO_Table | `s1_iotable.py` over `machines/HT9050/IO_Table.csv` | 1,124 lines = 969 named + 155 blank; Sensor 380 (104 Enable=1), Switch 143 (28), Cylinder 128 (64) + `_On` 92 (38) + `_Off` 79 (25), Sucker 49 + On 49 + Off 49 (all Enable=0) |
| unbindable rows | `s1_iomatch2.py`: name-only match of every alias against the 333 + 790 + 260 registered names (and the derived `<Cyl>_On/_Off` base names) | 202 rows have no object (Cylinder 56, `_On` 38, `_Off` 25, Sensor 76, Switch 7), **167 of them Enable=1** (of 259 enabled named rows); none of the 202 has a registered base name; golden 906 and V912 do not know them either (the table is ahead of the code; per `docs/IO_LIST_20260928.md` the flows exist only in the HT9050 tree V910_HT9050, which is not on this machine) -> rows E-IO-001..202 (one row each, grouped by family in the notes) |
| objects used by nobody | `s5_objuse.py`: live references (comments, strings, extern/const lines, compile-disabled regions removed) of 1,666 objects in both trees | referenced in golden: Sn 487, Sw 227, Cylinder 227; in both: 425 / 157 / 204; **155 referenced only by golden** (Sn 62, Sw 70, Cylinder 23); wired+enabled in the machine table: 3 -> E-CG-001 |
| suckers / cylinders | `s9_iopage_*.py`, `mech_manual_rows.py` | all sucker names exist; every sucker row is Enable=0; binder alive (E-SUCK-*) |

### 4.2 Motor aliases (item 2)

`s2_motors.py`: golden `MOT[..].SetAlias(idx,"name")` 274 entries vs the port and vs the 48 rows of `Mot_Table.csv`. One difference: the port spells `MInShuttle1/2` where golden has `MInShutte1/2` (E-MOT-001, intentional rename). No Mot_Table alias lacks a port alias and no port alias lacks a golden one.

### 4.3 Periodic work (item 3)

| set | how it was built | count |
|---|---|---|
| dfm TTimers | `s3_timers.py` over every golden `.dfm` (`object X: TTimer`, Interval, Enabled, OnTimer handler) | 123 timers in 60 form classes (46 design-time Enabled=True, 77 False, 4 without a handler); 14 belong to TfMain |
| code-created timers | `git grep --no-index -n -a -E "new\s+TTimer|SetTimer\(|timeSetEvent|CreateWaitableTimer"` (2026-10-01 01:08) | 8 (BinDisp, Renesas server, CCLink, DNM100UD, EtherCAT, OCR, PowerSaving, MyTempture) -> E-RT-001..007 |
| threads | `git grep ... "new T\w*Thread|CreateThread|->Resume\(\)|Synchronize\("` | 10 worker bodies: THeaterThread, TRunControl (MainProc), HThreadCtrl shuttle, ScanBtn, PadRS232, PLCIO, Omron x2, ATC/Monitor socket watchers; `TShuttleThread` is never instantiated in golden either -> E-TH-001..007 |
| port status per timer | `s3_portmap2.py`: class-scoped `Class::Handler(` definition + non-test caller | 94 NO_DEF, 25 DEF_NO_CALL (of the 119 with a handler body), 0 driven by a real TTimer |
| effect class per handler | `s3_fingerprint.py`: pattern scan of the comment-stripped, SOFT_SIMULTE-undefined body | used to order work, not as evidence |
| TfMain timers (14) | read statement by statement with `effview.py` (Timer1 by sub-agent A, reviewed) | E-T1-*, E-T2-*, E-T3-*, E-TM-* |
| other form timers (105) | by hand after the E1 sub-agents never ran: `s18_timer_callees.py` (per-timer callee status R/U/M against the port call graph, objects touched, alarm codes) for all 105, then ~40 bodies read with `effview.py` and their port counterpart located with `pgrep.py` / `git grep`; rows written by `mech_ft.py` (23 rows) and `mech_fallback.py` (5 tool-derived family rows); coverage of all 105 checked by name | E-FT1-*, E-FT2-*, E-FTF-*, E-FT-DISP |
| port tick chain | `tools/wb_serve.cpp` main loop (<= 50 ms pass; 500 ms `pumpBeat` -> `PumpTick()` = GetTimeInfo, W906_FlushFlagTick, W906_HeaterSimTick [SIM only], MainProc, W906_CounterRefreshTick, then the W906_*Tick slices) | there is no VCL message loop and no working TTimer: each form header defines an inert stand-in TTimer, `git grep "OnTimer\s*\("` in non-test source = comments only |
| reachability | `s14_reach.py` (port) + `s15_golden_periodic.py` (golden): name-level call graphs, roots = timers / threads / boot vs `main()` | 1,582 golden-named port functions unreachable from `main()`; 596 of them are run by a golden timer/thread/boot root (13,676 port statements) -> E-CG-006 |
| no-caller functions | `s6_callgraph.py`, `s10_callers.py`, `s10_appendix.py` | 425 (E-CG-000, appendix `census129_e_nocaller.tsv`) |

### 4.4 Boot registrations (item 4)

TfMain constructor `main.cpp:1340-2265` (926 lines, 531 statements), FormCreate `:26686-26706` (9), FormShow `:9141-11365` (2,225 lines, 1,084 statements) read by hand against the port boot (`tools/wb_serve.cpp` boot sequence, `FileRW/MainBoot.cpp`, `forms/fMain.cpp` ctor, `cinitial.cpp`; sub-agent D died on the rate limit; tools `s16_boot_callees.py`, `s16_m_evidence.py`, `s17_dirs.py`) -> E-BOOT-*. Earlier rows: SYSTEM_MODULAR ctor (E-SMC-*), InitialHandler gates (E-REG-*). The other 132 forms' ctor/FormCreate/FormShow bodies (13,898 golden statements vs 5,508 in the port) were NOT read (E-CG-002, appendix `census129_e_formboot.tsv`).

### 4.5 Cross-checks that found nothing (recorded so they are not repeated)

* Config option registrations: golden `cConfiguration.cpp` and the port have the same 1,537 `elConfig->Add` entries / 993 distinct `[section] key` pairs (`s12_config_keys.py`) -> E-OK-003.
* SECS/GEM SV/EC tables: golden-only ids are all documented gates (E-CG-004); the golden `SECSGEM/SECSGEM.cpp` (133 SV + 91 EC) is not in `HT9045.bpr` and is commented out of every includer.
* Alarm-code catalog: 157 codes used in golden code are not in `AlarmCodeCatalog.cpp`, but the catalog is generated from the MDB Updater (`AlarmCodeCatalog.cpp:2-4`), so this is not evidence of a gap.
* WM_COPYDATA receiver `TfMain::OnMyCopyMsg` (1,829 lines): GPIB branch ported (HandlerGpibMsg.cpp, gates G1..G23), the other five branches have no receiver (E-CG-003).
* Comm components: 104 non-visual components / 261 event handlers in golden dfm files; 51 handler names are defined in the port (`census129_e_comm.tsv`); the timers that poll them are in the E-FT1-* / E-FT2-* / E-FTF-* rows.


## 5. CHECKED-ALIVE list (verified not gaps)

* **E-FT1-004** - TfMotorTest::Timer1Timer (5 ms, fShow-gated): motor-test page LED / loop-move / home state machine
* **E-T3-004** - Timer3Timer: [A01] auto switch to Operator level after idle (iOperatorModeCount, bAutoOpenConfigA01) -- was absent in the first pass, now PORTED on main (St01 D-015)
* **E-BOOT-004** - fCleaning->LoadAutoCleanData() + SearchCleanNum() at boot -- PORTED through the FileRW layer (name-level census showed them as missing; the validator's hit count caught it)
* **E-BOOT-014** - Boot statements that ARE in the port (verified): SystemInitialOK=true (INBOX 127, now on main), bin count / jam-rate / version stamp / limit and critical-parameter authority reads, lot lists, ResetLotInfo, Alarm object, tray assignment
* **E-OK-001** - IO object tables: all 1,070 golden Sw/Sn/C_ constants and all 331 + 790 + 260 name registrations exist in the port with identical values (+2 machine-added switches)
* **E-OK-002** - Driven and alive (verified, not gaps): tower lights, SECS RunStatus event, tester-bridge sync, vision connect, state-record image pump, test seconds, run info, ATC socket thread
* **E-OK-003** - Configuration option registrations: golden cConfiguration.cpp and the port register the same 1,537 elConfig->Add entries (993 distinct [section] key pairs)
* **E-FT2-004** - TfTesterTCP::TimerTCPIPConnectTimer + TimerProcessTCPDataTimer (1000 ms / 1 ms): built-in TCP/IP tester channel

## 6. Corrections to earlier beliefs (found during this census)

* **INBOX 132 premise ("the sucker binder in `cinitial.cpp:493-878` is `#if 0`")** is stale on main: the IO_Table half of InitSucker is live since 20260924 (only the BDE/DB else-half `#if 0` at cinitial.cpp:656-877 is gated); all 147 sucker rows are Enable=0 on the machine table, so the IO page grays them as golden does (E-SUCK-001, E-SUCK-002).
* **[A01] auto-logout (Timer3)** became ported on main while the census ran (`FileRW/Main_A01AutoLogout.cpp:299 W906_A01AutoLogoutTick`, called from `tools/wb_serve.cpp:5953` and from the modal wait) - E-T3-004 is CHECKED_ALIVE.
* **AutoClean boot read** (`fCleaning->LoadAutoCleanData()`, SearchCleanNum) looked missing in the name-level boot census but is ported through the FileRW layer (`FileRW/TestIF_File_Cleaning.cpp:390`) - E-BOOT-004.  Lesson: a golden callee missing by name is NOT proof of a missing boot step; 33 forms have a `FileRW_<Form>_Boot` and 11 `W906_FRWBoot_*` functions re-implement FormShow statements under other names.
* **ProcessSingleMotorHome is real** since 20260927 (`acatchtray_shims.h:424 AI(W906-SMHOME)`), but two web strings (`WebMotorAccess.cpp:1390` and `:2675`) and census129_cd row C2-142 still call it a stub - stale text, not a missing function.
* **TfTesterTCP timers are alive** (`TesterComm/Tcp/TcpPump.h`, `W906_TcpPumpTick`) although the class-scoped census lists them NO_DEF (E-FT2-004).
* **`fAutomation` is not the OLP engine**: it is the no-op `TfAutomationShim`; the translated `TfAutomation` engine is never created (E-FT2-001).  The earlier assumption that "Automation is translated" (census129_ab A4 rows list its TU-local stand-ins) does not mean the OLP link runs.

## 7. Limitations and what was not done

* **Name-level call graphs** (`s6`, `s14`, `s15`) resolve calls by identifier only (no overloads, virtuals or function pointers other than tables listed as roots); reachability and NO_CALLER counts are candidate lists, each used row was re-checked with `git grep -w`.
* **Machine facts come from this laptop's ini files**, not from the real HT9050 machine; several rows depend on options (SECS, OEE, ATC, CCD, barcode, OLP customer list) whose machine values are unknown (marked "inert here" / "not evaluated").
* **About 65 of the 105 non-TfMain timer bodies were not read** (family rows E-FTF-*, E-FT-DISP: confidence 5-8, tool-derived).  The 132 non-TfMain forms' ctor / FormCreate / FormShow bodies were not read (E-CG-002; appendix `census129_e_formboot.tsv`).
* **Golden 906 is the reference**, not V912 (the golden tree differs from V912 in places; V912 was consulted for the machine-specific rows only).  The tree `V910_HT9050` that holds the HT9050 flows behind the 202 IO rows is not on this machine, so those rows cannot be traced to code.
* **No runtime measurement**: nothing was built, run or tested; the statements are static.  Confidence is a judgement, not a measurement.
* The sub-agent rule changed mid-way (max 5 agents, then none): agent A's output was reviewed and validated; agent D and the E1 agents were replaced by hand work, so the coverage of E1 is lower than planned (above).

## 8. Files (all in the scratchpad `census129` directory)

```
census129_e.tsv                 the census (360 rows, 10 columns)
census129_e.md                  this file
census129_e_qa.tsv              the same rows with a mechanical check column (validate.py --fix)
census129_e_timers.tsv          all 123 golden dfm timers: design-time settings, body size, class-scoped port status, covering rows (4 have no OnTimer handler)
census129_e_nocaller.tsv        425 port functions with golden call-sites and zero live callers (E-CG-000)
census129_e_unreachable.tsv     596 functions unreachable from main() that golden drives from a timer / thread / boot root (E-CG-006)
census129_e_comm.tsv            104 non-visual components / 261 event handlers of the golden dfm files (51 handler names defined in the port)
census129_e_formboot.tsv        133 forms: golden vs port ctor / FormCreate / FormShow statement counts (E-CG-002)
tools\                          every script that produced the above (assemble.py, validate.py, mech_*.py, s*_*.py, effview.py, pgrep.py ...)
```
