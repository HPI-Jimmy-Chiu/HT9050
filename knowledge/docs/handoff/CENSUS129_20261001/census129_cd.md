# INBOX 129 census, categories (c) and (d)

(c) web actions with no real C++ effect, and web controls that do nothing.
(d) global scalars that are read but never written.

Author: ht9045-v906 sub-agent (READ-ONLY census). Finished 20261001 02:03.
Nothing in any repo was edited, built, run or opened in a browser. All output is in this scratchpad directory:

    C:\Users\JIMMYC~1\AppData\Local\Temp\claude\D--HT9045\9190c0f6-6ccd-4722-a0db-a8f189dce683\scratchpad\census129\

| file | content |
|---|---|
| census129_cd.tsv | 975 rows + header, 10 tab-separated columns (id, category, name, sender_or_reader_file_line, receiver_or_writer_file_line, classification, golden_ref, action_class, confidence, notes). UTF-8, LF, no control characters, every line has 10 columns (checked) |
| census129_cd.md | this file |
| census129_cd_stats.json | the counts used below |
| assemble_cd.py, gen_rows_c.py, gen_rows_d.py, rows_c_cmd.py | row builders (assemble.py in the same directory belongs to the (a)(b) census, not to this one) |
| c*.py, d*.py, x*.py | measurement scripts, listed in section 8 |

## 0. Trees and state

| tree | path | state |
|---|---|---|
| port (A tree) | D:\HT9045\.claude\worktrees\mach0930\HT9011UC_Cpp_V3.33.906.0 | worktree HEAD 12fe15e4 ("TO_STEVEN s4 21:0x ..."), `git status --short` empty; checked again at 20261001 01:53, after the last measurement, and unchanged |
| web | D:\HT9045\.claude\worktrees\mach0930\web | same worktree; 326 tracked .js/.html/.htm/.json/.cjs/.mjs files, 84 tracked `web/page/*.html` |
| golden 906 (cp950) | D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618 | 706 .cpp/.h files read as cp950 |
| V912 | D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy | 716 .cpp/.h files; used as a second golden for (d) |

Search tools: the Grep tool (ripgrep) and `git grep` only, never recursive grep. Python with backslashes was written to files with the Write tool first. Results were never piped into `head` when the question was "is X absent" (one early slip of that kind is listed in section 5).

## 1. Counts

### 1.1 Category (c): 541 rows generated, 8 duplicates and 23 twins dropped, 510 kept

C = web command inventory and page controls. "Coarse class" is the first token of the `classification` column; the text after the colon is the subclass.

| sub-census | what it lists | rows | REAL | STUB | UNHANDLED | GATED | UNDECIDED |
|---|---|---:|---:|---:|---:|---:|---:|
| C1 | every command a web file can send and every C++ receiver (WS arms, act.* names, HTTP POST routes, system file keys) | 76 | 59 | 5 | 11 | 0 | 1 |
| C2 | C++ replies/log strings that themselves say "not done / not ported / no-op" on a web-reachable path (136 `ELTodo` literals + 192 marker strings) | 169 | 0 | 165 | 0 | 4 | 0 |
| C3 | web page controls (DFM-driven, main screen, hand-mapped pages, DOM scan, menus, disabled inventory) | 265 | 16 | 0 | 200 | 24 | 25 |
| total C | | 510 | 75 | 170 | 211 | 28 | 26 |

Fine classes over all 510 kept (c) rows (the text after the colon in `classification`): STUB:PARTIAL 156, UNHANDLED:UNWIRED 83, UNHANDLED:TRANSLATED_NOT_WIRED 72, REAL 57, UNDECIDED 26, GATED:DISABLED_WITH_REASON 22, UNHANDLED:UNWIRED_PORT_CALLED 17, REAL:SHELL_EXIT 13, UNHANDLED:DEAD_MENU_ITEM 13, STUB 11, UNHANDLED:NOSENDER 11, UNHANDLED:INERT_PAGE 4, UNHANDLED:NO_WEB_CONTROL 4, UNHANDLED:RELEASE_TITLE 4, GATED:CONDITIONAL_INSTALL 3, REAL:INVENTORY 2, REAL:PROBE_ONLY 2, STUB:RULED_UNWIRED 2, and one row each of GATED:BUILD_HAVE_PCI1203, GATED:RULED_NOT_WIRED, GATED:UNWIRED_PORT_GATED, REAL:FAITHFUL_NOOP, STUB:STALE_TEXT, UNHANDLED (stream.resync), UNHANDLED:INERT_BUTTON (main.html Set), UNHANDLED:PAGE_REFUSES (IO page). Definitions: NOSENDER = the C++ receiver exists and nothing in the web sends it; UNWIRED = the page shows a control, no table row or script handles it and the golden handler is not in the port; TRANSLATED_NOT_WIRED = the golden handler is in the port but nothing calls it from the web; UNWIRED_PORT_CALLED = translated and called by other C++ but not from the web; INERT_PAGE = the whole page has no script for its action buttons; RELEASE_TITLE = a script selects by `title`, which release mode strips; PAGE_REFUSES = the page blocks a command the C++ would accept; DISABLED_WITH_REASON = the page greys the control and says why. Exact numbers: census129_cd_stats.json (`c_fine_total`).

Rows with confidence >= 50: C1 11, C2 0, C3 73. Rows with confidence >= 70: C1 6, C3 15.

By action class (kept rows): C1 MOTION 9, IO_OUT 7, STATE 29, COMM 3, FILE_WRITE 23, DISPLAY 5; C2 MOTION 11, IO_OUT 5, STATE 77, COMM 33, FILE_WRITE 30, DISPLAY 13; C3 MOTION 28, IO_OUT 8, STATE 110, COMM 29, FILE_WRITE 26, DISPLAY 64.

### 1.2 Category (d): 465 rows

| sub-census | rule | rows | conf >= 50 | conf >= 70 |
|---|---|---:|---:|---:|
| D1 | golden writer >= 1 and port writer = 0 (258), "port writers exist but only ever assign false while golden sets true" (99, a disjoint set: the port has writers), all port writers in uncalled functions (5), struct members (4), TfHome::fAbort (1) | 367 | 14 | 13 |
| D2 | declared in a port global header, written by no tree (golden 906, V912, port), with live port readers | 98 | 0 | 0 |

Denominators: 2,501 scalar/array globals declared in cprod.h, cpublic.h, cmydef.h/.cpp, csystem.h, ckernel.h and the MachineType/MachineDefine-adjacent headers (2,148 scalars + 353 arrays). Golden 906 has >= 1 live writer for 2,006 of them, V912 for 2,011; the port has >= 1 live writer for 1,759. Golden writer >= 1 and port writer = 0: 258 (58 with live port readers, 200 without). Written by no tree: 484 (98 with live port readers = D2).

## 2. Conventions

* id: C1-nnn, C2-nnn, C3-nnn, D1-nnn, D2-nnn; numbered by descending confidence inside each group. `category` is `c` or `d`.
* `sender_or_reader_file_line`: for (c) the web sender; for (d) the first live port readers with the line text.
* `receiver_or_writer_file_line`: for (c) the C++ receiver; for (d) the golden writer and the port status of the writer's function.
* `confidence` = confidence (0-100) that the row is a real blank round whose golden dependencies exist in the port, i.e. fixable with code that is already there or by plain translation. For classification REAL it is therefore low (1-30; the certainty of the REAL verdict itself is written into the notes as "[classification certainty N]"). For the ELTodo/marker rows it is 8-45 (35 baseline for ELTodo, 30 for marker strings, +10 for MOTION/IO_OUT, capped at 30 when the missing piece is the .xls writer and at 25 for customer-specific text: the port itself reports the missing effect, but often a vendor library or a customer path is what is missing). For ruled items (RESET family, siteClick, ctlButton[reset]) it stays high because they are blank on purpose; the classification says `RULED_UNWIRED`.
* `action_class`: MOTION, IO_OUT, FILE_WRITE, COMM, STATE or DISPLAY, by the regexes in c5e_handlers.py (golden handler body) or by hand for the manual rows.
* Customer/option gating lowers confidence. Options that are assigned only `=false` in both golden trees and have no INI/UI binding are treated as dead (see section 4.4).

## 3. Category (c): method, commands, denominators

### 3.1 C1: commands and receivers

1. Receivers. Every `wc.cmd == "..."` and `wc.cmd.compare/rfind(...)` arm in `tools/wb_serve.cpp` plus the `cmd ==` arms of `WebBridge/WebBridgeServer.cpp` (control.*): 69 literals (x07_cmd_crosscheck.py). Result: 0 arms are missing from the C1 rows (every arm name appears in a row's name, receiver, sender or notes). The dispatch chain is wb_serve.cpp:4677-5932; an unknown name falls into the else arm at :5920-5931 (`unknown cmd '<x>'`). act.* unknown names are refused at JsonBridge/ChanAction.cpp:351-355 (`unknown-action`).
2. act.* names: `x10_act_names.py` reads every quoted `act.*` literal in 280 tracked web .js/.html files (22 names) and in 1,483 tracked C++ files outside tests/docs/third_party (26 names). 22 of 22 web names exist in C++; 4 C++ names have no web sender: act.counterClear.exe, act.main.autoSkip, act.main.clarnData, act.main.peModel (all are C1 NOSENDER rows).
3. Web senders: `x08_send_literals.py` and `x09_cmd_context_literals.py` (quoted dotted literals on lines that mention cmd/send/wireIcon...; 70 distinct literals) found only two without an arm: `stream.resync` (ht9045_recipe_client.js:284, UNHANDLED) and `ui.reconnect` (web/js/pci1203.js:787, a page-local pseudo command, not a WS message).
4. HTTP POST is a second command surface: `x12_http_writes.py` over the same web files finds two pages that send mutating HTTP: `eventlog.html` (POST /api/ela/query|job|summary, receiver EventLogAnalysis/ElaService.cpp:194-247, all seven job ids have a body because `ScheduleUseHubJobs` is called at ElaService.cpp:269) and `testercomm.html` (POST /api/testercomm/<key>?cmd=, receiver TesterComm/Handler/TesterCommWiring.cpp:279; the per-verb grammar is documented in TesterComm/Gpib/GpibUiSnapshot.cpp:19-42 and was not enumerated control by control). `allowCmd` is always true (wb_serve.cpp:3671, :4352).
5. system file keys: `x15_sysfile_keys.py`: 27 of the 40 `SysFileEntry` keys (wb_serve.cpp:1172-1216) are never named as a quoted string in the 326 web files (UNDECIDED row).
6. motor.access: all 51 rows of web/JSON/motor-access.json map to one of the 39 C++ actions (WebMotorAccess.cpp:43-98; c4_motor_buttons.py).
7. The 73 manually read rows are in rows_c_cmd.py (+2 HTTP rows, +1 key row, +1 page-refusal row added later).

### 3.2 C2: what the C++ side itself reports as not done

`c8_eltodo.py` harvests the `ELTodo(...)` / `Todo` literals of FileRW/*.cpp|*.inc and the generated `.gen.inc` files: 136. `c1_stub_strings.py` harvests the marker strings (`no-op`, `stub`, `not ported`, `not wired`, `deferred`, `未移植`, `尚未接`, `還沒接`, `eventOnly`, ...) that are compiled into live code outside third_party/EtherCAT/wb_serve/WebBridgeTags/MainClose: 192. 177 rows were generated; 8 were dropped as duplicates of C1 rows (MainClick.cpp:1408, :1703, :1705, :1715, :1817, :1820, :1831 and WebTeachLeave.cpp:220) leaving 169. Four are guards that cannot fire in the shipped binary and are classified GATED: ChanAction.cpp:159 (wb_serve.cpp:4261 installs the body), MainStateRecord.cpp:132 (wb_serve.cpp:4300), WebSortCT.cpp:62 (cSortCT.cpp:486/491), WebMotorAccessLive.cpp:285 (needs HAVE_PCI1203). One reply text is stale (MainTesterConnect.cpp:216 still says ChangeTesterConnect is not translated; it is, Automation/auto9045.cpp:210).

### 3.3 C3: page controls

1. DFM-driven (44 of the 84 `web/page/*.html` have `<title>... .dfm / fXxx : TfXxx</title>` and map to a golden .dfm): `c5a_pages.py` (mapping), `c5b_wiring.py` (golden event rows x page ids x FileRW event tables x page JS closure), `c5e_handlers.py` (golden handler body and action class), `c6_handlers_port.py` (is the handler translated: LIVE, GATED or ABSENT, and does live code call it).
   * 4,006 golden event rows on those pages; 1,430 are button OnClick rows: 36 have a C++ event-table row, 229 are referenced by the page's scripts, 1,165 have neither (this includes controls hidden or disabled by the DFM parent chain, including `TabVisible=False`).
   * 932 of the 1,165 are visible and enabled by default. 544 of them are the Teach Set/Go family (SetButtonNNN/GoButtonNNN) that the page binds by title family from teach-access.json (one REAL row). Net 388 buttons, 157 distinct handlers: 70 LIVE in the port (55 of them have no caller at all: translated, nobody calls), 86 ABSENT (not translated), 1 GATED. The 388 buttons are grouped by (page, handler) into 171 rows, plus hand-written rows for the pages with their own notes (HW.ShuttleMove 19 buttons, HW.home, Status.LtcSensor, Setup.AGV, Alert.Password).
   * 758 distinct handlers over all 4,006 rows: LIVE 424, ABSENT 330, GATED 4.
   * 24 UNDECIDED rows aggregate the non-button events (checkbox, combo, radio, grid...) per page, because static analysis cannot tell generic engine load/save wiring from a missing handler.
2. Main screen, `c12_main_family.py`: golden main.dfm has 850 controls; 192 click-like events (OnClick, OnDblClick, OnMouseDown/Up, OnChange, OnKeyPress, OnSelectCell) on button-like classes; 91 are visible by the DFM default. The family pages are `main.html` + the 12 `Main.*.html` pages (+ Status.ShowMessage.html as a name source) (359 element names, 43 script files, 712 KB of JS, comments and strings separated by `strip_js`). 49 of the 91 have a script reference by name (quoted, `#id`, `[title=...]` or identifier/object key); 42 do not (30 absent from every family page, 12 present without a reference). Manual reading of the 42 gave the rows listed under "main screen" in 3.4. A script reference only proves the name is mentioned (cbRunStartMode is mentioned by a display mapping, not a sender), so this is an upper bound for "wired".
3. Pages the title mapper could not map, `c12b_unmapped.py` (hand-mapped dfm): Data.LotInfo (135 events; page ids are custom so names cannot decide, UNDECIDED row), Status.ShowBinSelect, Data.SortCT, Status.TemperFrom, Data.ContactCT, Data.TestCategory, HW.TrayEdit, Status.ShowMessage, Setup.BinSelNormal. Of the 40 unmapped pages, 13 are the main family (3.3.2), 9 are these, 2 are HTTP pages (eventlog, testercomm; 3.1.4) and 16 were not compared at all: Alert.MotionView, Alert.MotionView9050, Alert.MyMessageBox.NonStop, Alert.Note.NonStop, IDE.* (7 tooling pages), IoLive, ScreenShots, _partials/testcate_inner, shot/FrmRotate, shot/MainTabs.
4. DOM hygiene, `c16_dead_buttons.py`: 1,873 `<button>`/`<input type=button>` in 75 pages; 186 have no id, no onclick and no data-* hook. 180 are the Status.Security matrix labels (bound through the radios, `ht9045_wire_statussecurity.js:186`), 2+2 are P1/P2 tabs of the MotionView9050 pages, 1 is a disabled LotInfo button with a reason, 1 is the main.html `Set` button (a real finding).
5. Menus, `c11_menu.py`: 13 menu items in main.html palSetup/palConfig resolve to neither DFM_MAP nor SHOT_MAP (class `todo`): they draw but open nothing.
6. Disabled-with-reason inventory, `c17_disabled_scan.py`: 34 of 223 web .js/.html files disable or grey controls from script; 20 are named in rows; the other 14 lock controls by security level (`data-gb-dis`), by hardware option or while a request is in flight (13 read, plus the on-screen keyboard qwerty.js), which is not a missing feature.
7. `pagewire` PENDING dictionaries, `x13_pending_fields.py`: 6 files, 132 fields; YieldMonitoring (72), TesterIF (28) and Contact (12) are C-route bridged (GOLDEN_BRIDGE, ht9045_wire_engine.js:1038-1060) so their lists are stale; HotPlate (5 fields) and SCK_ART (15) are live and have rows.
8. Release-mode title stripping: theme.js:13-24 (default mode release) moves every `title` to `data-htitle` at DOMContentLoaded. Scripts that select `[title=...]` without a `data-htitle` fallback bind nothing. Four rows (Light/FAN buttons, Observer tabs, StartCondition life tabs, a legacy simulator helper). Not verified in a browser.

### 3.4 Findings added during this run (beyond the first pass)

* main screen `Set` (golden spbSet): main.html:205 is an inert `<button class="btn3d">` and C++ `TfMain::SetTemp` is a stub (forms/fMain.cpp:511). C3 row "main.html 'Set' button".
* spbChamberFan, sbSaveMMI, sbLaguage, palEQC, btnAutoClean (ShowBinSelect): no web control; golden handlers exist (main.cpp:28444, :34910, :8485, :33433; cShowBinSelect.cpp:2101). The Auto Clean start is SAFETY-QUEUED on purpose (forms/fShowBinSelect.h:130).
* pagewire HotPlate fields, Unloader Clip fields, Offset barcode prompt, AOA runtime labels: documented deferrals in page JS, one row each.
* page-side refusal of 152 IO outputs (the coordinator's INBOX 132 hint): C1 row `io.btnPanelClick [page-side refusal ...]`, see 3.5.

### 3.5 The page-side refusal (INBOX 132)

`web/page/ht9045_io_do.js:99` `whyNotWritable()` returns 'IO 表 Enable=0' when `pt.hw.enable !== 1`; `paint()` (:113-118) greys such buttons (`io-na`), `onClick` (:245-255) and the capture handler (:262-281) refuse before any message is sent. The C++ side would accept them (`W906_IO_PAGE_NO_GUARDS 1`, JsonBridge/IoBtnPanelClick.cpp:215). `c9_io_enable.py` on machines/HT9050/IO_Table.csv: 589 non-sensor rows; 152 output rows have ISABase=3 and Enable!=1: 144 sucker rows (FTestSuck/BTestSuck AA..BH with _On/_Off aliases = 96, InArmSuck/OutArmSuck = 48), C_Shuttle1Floodgate(Off), C_OutShuttle1Floodgate(Off), C_MobileCassetteSelect, C_OTD_FrontBack/LeftRight/Valve and two more with no address. The page also refuses when the read-back quality is not good (:253-255) or the provider is not `wb_serve:pci1203-monitor` (:245-247).

## 4. Category (d): method

1. `d1_decls.py` lists the globals declared in the port headers (2,501, see 1.2). `d2_scan.py` strips comments and strings column-preservingly, runs a small preprocessor (`#if 0`, known W906/WB/HAVE/INSTALL/SOFT macros) and classifies every occurrence as W (assignment, ++/--, compound), A (address-of), P (passed), R (read), D (declaration) or M (memset) in golden 906, V912 and the port. Buckets: core, wbserve, tests, tools, ui; only core and wbserve count as live port code. `d3_analyze.py` writes d_counts.json.
2. Candidates: golden (906 or V912) live writers >= 1 and port live writers = 0: 258. Address-taken or passed-by-reference golden variables with no assignment were checked separately: 5 exist, all pass-by-value (iMMgzTray, SortArmZIndex, iTempCode, bSpin, TRAY_MISS_CHECK_TIME), none is a missed write.
3. Polarity (`d10_polarity.py`, `d11_polarity_sites.py`): for booleans that golden sets true and the port only ever sets false: 99.
4. Gates: `d14_gate_heads.py` walks up from the golden writer line through the enclosing blocks and keeps MULTI-LINE condition heads; `d13_option_reach.py` checks every `IniConfig/CosFunction/TestIF_File/TestIF/Temperature/Prod/LastSet/Sim` member in those heads for any non-false assignment or INI/UI binding (`&Struct.member`, not `&&`) in both golden trees. Three options are dead (only `=false`, never bound): `CosFunction.bSecurityHave5Level`, `CosFunction.b2DCodeCheckByCoustomerLot`, `IniConfig.bFinishSuckAfterPause`. Rows whose golden writer sits under one of them are capped at 5 and tagged DEAD_OPTION_GATE.
5. `d15_default_writers.py`: candidates whose every live golden writer assigns the port's initialiser: 18 (tagged WRITES_INITIAL_VALUE_ONLY, capped at 3). `MOTIONNET_SPEED` is the same case through a macro (COMMSPEED_20M = 3).
6. `d12_deadwriters.py`: globals whose only live port writers are in functions nobody calls (5: pitch-check tasks, ATC7 socket read).
7. `d9_members.py`: runtime struct members (RunInfo, LastSet): 4 rows. `fHome->fAbort` is a TfHome member with its own row.
8. D2: declared, written by no tree, with live port readers: 98 (almost all constant tables and ini-loaded values; confidence 5-8).
9. Known item: `SystemInitialOK` (golden TfMain::FormShow main.cpp:9659, readers ckernel.cpp:2817, :3152, cprod.cpp:1803) is reported as D1 "SystemInitialOK (bool) [fix pending 364d3d1d]" (INBOX 127, WIP branch v906/jimmy-sysinit, not on main). The other 14 globals whose golden writer is inside TfMain::FormShow each have their own row: TempFuseLimitType, bBarcodeReader, iDefHonPrecLevel, iDefEngineerLevel, iDefSupervisorLevel, bRotateChange, XResolution, YResolution, fNowSystemHeightScale, fNowSystemWidthScale, iNowSystemHeight, iNowSystemWidth, iSourceSystemHeight, iSourceSystemWidth (d14_gates.json, heads containing TfMain::FormShow).

Most D1 rows cluster on a few untranslated golden functions: TfMain::FormShow, TfMain::Timer1Timer, TfMain::ScanKey, TfMain::Timer3Timer, DoAuto3Magazine (empty shim csystem_shims.cpp:147), SYSTEM_MODULAR ctor (database.cpp:139-156 is `#if 0`), TfLotInfo::LoaderAction, HS_Function, TCOM2 ATC over RS232, the AutoAlignment CCD flow, ShowErrorMessage side effects. The notes give the golden gate and the customer/option.

## 5. Corrections made while verifying (what the first pass got wrong)

| item | first pass | after re-reading |
|---|---|---|
| TempFuseLimitType | one reader (CheckHeater) | three live readers (cprod.cpp:3033-3035 clamps the socket range to about -36, forms/fHS.cpp:857 makes Setup.Temp_Set refuse every save with WAR15194, uHeaterThread.cpp:1294 switches the heater relay off after 41 s in ship builds); SIM builds return at uHeaterThread.cpp:505-511 |
| iPauseBackUp / bFinishSuckAfterPause | 60 | 5: the option is assigned only `=false` in golden 906, V912 and the port and has no INI/UI binding; the port's missing ScanKey branch changes nothing |
| bClearSrtart | 60 | 8: the assignments are translated but inside `#if 0 ... T17 user ruling ... direct-card branch not ported` (atester.cpp:2064-2317) |
| bCheckPCI_MN200StateRun | 72 | 60: golden sets it only for `IO_CARD_TYPE==0 \|\| MOTION_CARD_TYPE==0` (Syn-Tek) or MN200 IO cards; this machine's Gerneral.ini has MOTION_CARD_TYPE=0 so golden would set it here |
| bCheckLotError | 35 | 5 (dead option b2DCodeCheckByCoustomerLot) |
| MInRotate / MOutRotate | 55 | 30: reachable only with USE_ROTATE_KIT=1 (database.cpp:1372); the dev machine's system/Gerneral.ini:485 has 0 |
| iMagneticScalePos | 45 | 20: only under the Config option IniConfig.bA27EnableLightScale (Motor/mymotor.cpp:4092) |
| bMagCatchTrayfalg, bMagGetNewTrayflag, bChaneMagTrayflag, bMagazineGetNewTray | 40 / 35 | 25 / 20: AUTO3_IS_MAGAZINE option; dev machine ini:171 has 0 |
| MOTIONNET_SPEED | 30 | 3 (same value) |
| SECS PP_MUSIC / PP_SIGNALTOWER flags | 30 (generic) | 70: the gated arms G16/G17 (SECSGEM/uHGemHT9045.cpp:5897-5990) say the globals are "DEFINED NOWHERE"; they are defined since 20260924 (ckernel.cpp:1272-1278); the host gets HCACK=1 and the tower/buzzer readers (ckernel.cpp:1692-1765) never follow |
| HW.teach unwired rows | listed twice (DFM-derived and leftover) | 23 twins dropped |
| Exit buttons (13 rows) | 45 | 8-25: every such page has the `.exitbtn` scaffold (e.g. Setup.Speed.html:85-86); golden FormClose side effects not compared |
| `InstallClarnDataBody` line | wb_serve.cpp:4243 | :4261 |
| LtcSensor note | "this machine's Gerneral.ini" | "the dev machine's ini (ShuttleMove.cpp:18, Steven 20260926 S47)" |
| one absence check | `git grep ... \| head` hid the late hit in tools/wb_serve.cpp (alphabetical order) | re-run without head: wb_serve.cpp:4300 installs W906_InstallStateRecordBody |

## 6. Absence claims and the commands that measured them (20261001, port HEAD 12fe15e4)

| claim | command | result |
|---|---|---|
| no web sender for main.runStartMode | `git grep -n -i runStartMode -- web`; `git grep -n -e main-control.js -e simulator-bridge.js -- web` | only a display mapping (ht9045_wire_main.js:40) and main-control.js:145 (sends to the C# simulator bridge); no page loads main-control.js or simulator-bridge.js (0 hits) |
| no web sender for ttlcfg.op | `git grep -n -i ttlcfg -- web` | 2 hits, neither sends (ht9045_testerif_c_wire.js:165 comment, ht9045_wire_engine.js:1048 page map) |
| Set button has no binding | `git grep -n -w spbSet -- web` | IDE.ComponentMap.html:126 (doc), main-control.js:131 (orphan), main.html:200 (the row title) |
| no web control for spbChamberFan, sbSaveMMI, sbLaguage, palEQC | `git grep -n -w <name> -- web` | only IDE.ComponentMap.html and the orphan main-control.js |
| no web handler for sbAbortHome | `git grep -n -w sbAbortHome -- web`; read of HW.home.html:56-110 | the button (id sbAbortHome, plain btn3d) exists; the page's only script handles tabs, .exitbtn, .btnpanel and trays |
| sbAbortHomeClick is translated but has one caller | `git grep -n -w sbAbortHomeClick -- HT9011UC_Cpp_V3.33.906.0` (docs/tests/scratchpad excluded) | definition uhome.cpp:5010, caller csystem.cpp:32735 (MainProc stop arm) |
| ShuttleMoveClick not translated | `git grep -n -w ShuttleMoveClick -- HT9011UC_Cpp_V3.33.906.0` (excl. docs/tests/scratchpad/dfm2rc) | one comment, FileRW/ShuttleMove.cpp:27 |
| no writer of TempFuseLimitType | `git grep -n -w TempFuseLimitType -- HT9011UC_Cpp_V3.33.906.0 ':!HT9011UC_Cpp_V3.33.906.0/docs'`; Grep `TempFuseLimitType\s*=` over golden 906 and V912 | port: cmydef.cpp:5603-5604 only; golden 906 main.cpp:9539/9547/9551, V912 main.cpp:9972/9980/9984 |
| bFinishSuckAfterPause never true | Grep `bFinishSuckAfterPause` over golden 906 and V912 | Config.h, CosFunction.cpp (`=false`), main.cpp (readers) |
| C++ TfMain::SetTemp is a stub | read forms/fMain.cpp:511 and forms/fMain.h:590 | `return W906_SetTemp_Sim;` |
| MInRotate/MOutRotate writers gated | read database.cpp:139-156 | constructor body is `#if 0 // TODO(wave)` |

## 7. Limits

* Everything is static. No browser, no build, no ctest, no wb_serve. The release-title finding (Light/FAN) follows from reading theme.js:13-24 and the registration order of the DOMContentLoaded listeners (theme.js at main.html:529, ht9045_main_st01_ev.js at :640, `init` registered at ht9045_main_st01_ev.js:444); it has not been observed in a browser.
* Name matching between DFM controls and web elements fails for pages with custom ids (Data.LotInfo) and over-counts "wired" when a name is merely mentioned (display mappings). Both directions are noted in the rows.
* 16 web pages were not compared (list in 3.3.3). The per-verb receivers of /api/testercomm were not enumerated.
* (d) counts write sites by token classification, not by compiling; macros outside the known list are treated as live. Customer and option reachability is judged from assignments and bindings in the two golden trees only.
* Confidence is a ranking aid, not a measurement. Rows at 10 or below are listed for completeness.

## 8. Script index (all in the scratchpad directory)

| script | purpose |
|---|---|
| x01_dump_wccmd.py, x02_receivers.py, x05_arms.py, x07_cmd_crosscheck.py | wb_serve arm enumeration and cross-check |
| x03_show.py, x04_web_literals.py, x08_send_literals.py, x09_cmd_context_literals.py, x10_act_names.py, x12_http_writes.py, x14_check_cites.py, x15_sysfile_keys.py | web sender literals, act.* names, HTTP POST, line-citation check, system file keys |
| c1_stub_strings.py, c8_eltodo.py | C++ marker strings and ELTodo literals |
| c2_release_title.py, c3_pci1203_cmds.py, c4_motor_buttons.py, c9_io_enable.py, c11_menu.py | release-title scan, 1203 command allowlist, motor-access map, IO Enable census, menu items |
| c5a_pages.py, c5b_wiring.py, c5e_handlers.py, c6_handlers_port.py, c6b_join.py, c7_pagestat.py | DFM-driven page census |
| c12_main_family.py, c12b_unmapped.py, c13_main_visibility.py, c14_show_handlers.py, c15_dfm_chain.py | main screen and hand-mapped pages, runtime Visible writes, handler reader, parent chains |
| c16_dead_buttons.py, c17_disabled_scan.py, x11_web_markers.py, x13_pending_fields.py | DOM hygiene, disabled inventory, web not-wired markers, pagewire PENDING |
| d1_decls.py ... d12_deadwriters.py | declarations, token scan, counts, candidates, conditions, members, polarity, uncalled writers |
| d13_option_reach.py, d14_gate_heads.py, d15_default_writers.py | option reachability, multi-line gates, writers of the initial value |
| rows_c_cmd.py, gen_rows_c.py, gen_rows_d.py, assemble_cd.py | row construction and TSV assembly |
| x06_lines.py | prints a line range of a file in a given encoding |
