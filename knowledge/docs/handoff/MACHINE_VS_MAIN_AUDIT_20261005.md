> 1005 20:0x laptop: read-only audit by a laptop sub-agent (machine C++ chain tip `76a370f5` = the machine after packages 147 + 148, vs main `d07a03de`). Decisions on it: M1 / M5 / M6 -> laptop batch 71; M2 + M3 -> St02 card ST02-P2 (after ST02-P1); M4 -> NIGHT_REPORT s0 #119; U1 -> s0 #120.

# Machine tree (chain tip 76a370f5) vs main -- machine-only lines, classified (read-only audit, 2026-10-05 19:xx)

## 0. Scope, pins, method

| item | value |
|---|---|
| machine | chain tip `76a370f5` (cpp 0223 PKG-147/148, 10-05 18:29) |
| main | **`d07a03de`** = origin/main when the classification was run. origin/main moved during the audit (`d4f9b018` -> `d07a03de`); the only C++-subtree change in between is `tools/measure_tick.ps1` (Jerry `d8feab7d`, MR !214), handled below. |
| package-148 base | main `44b2b9ce` (what the machine received). Used as a third point: a machine-only line that IS in `44b2b9ce` but not in `d07a03de` = main changed it after the machine's last package ("main-ahead"). |
| subtree | `HT9011UC_Cpp_V3.33.906.0/`, excluding `docs/`, `*.md`, `.vscode/` |
| comparison | per file, multiset of lines, CR + trailing whitespace stripped, blank lines ignored; a line counts only when the machine has more copies than main (moved lines cancel). Positions = first matching `-` lines of `git diff --ignore-cr-at-eol -U0`. |
| complement | also listed lines both `44b2b9ce` and `d07a03de` have but the machine lacks (machine deletions): every such line is the other half of an edit below -- no pure-deletion machine change found (outside `tools/vscode-htdesigner/`, which the chain does not carry). |
| scripts (scratchpad) | `mvm_audit2.py` (3-point multiset), `mvm_classify.py` (class per line, totals), `mvm_linepair.py` (intra-line diffs), `mvm_deletions.py` (complement) |

**Totals: 8,691 machine-only non-blank lines in 45 files** (31 files that main also has, 14 files absent on main).

| class | lines | what |
|---|---|---|
| R retired on main | 6,508 | 11 files, each byte-identical to main's last blob before `f89be4ce` / `834fcc78` |
| D decided to stay on the machine | 1,691 | TEACH-KB (1,638), mkpkg-excluded build.bat + F5 probe (41), BRAKE-BOOT goldenPower (9), cStateRecord comments (2), flag19 / W-79 (1) |
| T machine-local temporary | 114 | HOMEPOS0 / TRAYWAIT / TRAYSAFE (65), TOKEN-OFF (19), BYPASS-WAR1603 (16), TEMP-DOORS (11), WAR1604 `&& false` (2), SOFT_SIMULTE comment-out (1) |
| S superseded / main-ahead | 65 | main has the same thing in another form (44) or changed the line after package 148 (21) |
| **M MISSED** | **277** | MT-ACCLIVE 9, TEACH-HOMEALL 90, SOFTKEY family 98, HOME-PERAXIS 41, EXIT-ORDER 28, GATE-DRIFT 11 |
| U uncertain (needs a ruling) | 36 | SOFT E-STOP (conflicts with user ruling P11-ESTOP) |

Before calling anything M, `git grep` on main `d07a03de` over `HT9011UC_Cpp_V3.33.906.0/docs/MACHINE_PATCHES_*.md` and `docs/handoff/MACH*_LEDGER*.md`: SOFTKEY 0, SOFTKEY-DIAG 0, SOFTKEY-NOTOKEN 0, SOFT E-STOP / SOFT-ESTOP / panel.estop / panel.key 0, HOME-PERAXIS 0, EXIT-ORDER 0, GATE-DRIFT / macro_order_gate 0, PKG140-MERGE 0; MT-ACCLIVE 2 and TEACH-HOMEALL 1 (both = the "漏收兩件" line, MACHINE_PATCHES_20261005.md:86).

## 1. Summary table

Line numbers are machine (`76a370f5`) lines. Doc lines are on main `d07a03de` unless another ref is named.

| file | machine-only lines | class | evidence |
|---|---|---|---|
| FileRW/TestIF_File.cpp (absent on main) | 4,802 | R | `f89be4ce` 2026-09-26 Steven "formbridge: retire the A-shape TestIF_File bridge"; machine blob == `f89be4ce^` blob; MACHINE_PATCHES_20261002.md:42 |
| tests/test_formbridge_testerif.cpp (absent) | 138 | R | `f89be4ce` (deleted); blob identical |
| tools/formbridge/TFTestIF.py, TfSetup.py, TfYieldMonitoring.py (absent at that path) | 71 + 384 + 397 | R | `f89be4ce` renamed them (R100) into `tools/formbridge/_retired/`; blobs identical |
| JsonBridge/gen/form_TfDIOFrom / TfHotPlate / TfSpeed / TfTrayAssignment .gen.cpp, form_registry.gen.cpp (absent) | 31 + 32 + 361 + 113 + 17 | R | `834fcc78` 2026-09-27 Steven "retire S12 type 1 (FormJson ... gen/form_*.gen.cpp)"; blobs == `834fcc78^` |
| tools/gen_formjson.py (absent) | 162 | R | `834fcc78`; blob identical |
| FileRW/TeachKb.cpp (absent) | 467 | D | TEACH-KB `5ce4b89e` (10-02) + HT9050-TEACH-KB-LOADERY `16ffcedd` / -ALL `d25041e1` (10-04): MACHINE_PATCHES_20261002.md:38 (held), MACHINE_PATCHES_20261005.md:16-17 「留在機台」 |
| tests/test_teach_kb.cpp (absent) | 1,152 | D | same; MACH0210_LEDGER_20261005.md:51 (kExp9050 「留」) |
| CMakeLists.txt L3439 | 1 | D | `TeachKb.cpp` + its comment in the wb_serve source line: MACHINE_PATCHES_20261002.md:28 |
| FileRW/Teach.cpp L54, L304 | 2 | D | TEACH-KB call `FileRW_Teach_KbExtraJson`: MACHINE_PATCHES_20261002.md:38 |
| tests/CMakeLists.txt L6813-6830, L7594, L7915 | 16 | D | `test_teach_kb` block + TeachKb.cpp links ("machine-only ... TEACH-KB, held for Jimmy on main"): MACHINE_PATCHES_20261002.md:24, :38 |
| tests/CMakeLists.txt L7849-7850 | 2 | U | stub for SOFT E-STOP, see §3 U1 |
| MachineType.h L64 | 1 | T | `// #define SOFT_SIMULTE`: MACH0210_LEDGER_20261005.md:30 「留」; MACHINE_PATCHES_20261005.md:41 |
| MachineType.h L1783-1792 | 10 | T | TOKEN-OFF `W906_WEB_TOKEN_ENFORCE 0`: MACHINE_PATCHES_20261002.md:39 |
| MachineType.h L1866-1880 | 15 | T | BYPASS-WAR1603 (`db2b99a9`, cpp 0158): TO_ES02.md:225 ④; MACHINE_PATCHES_20261005.md:86 |
| csystem.cpp L16782 | 1 | T | BYPASS-WAR1603 (`W906_War1603Armed()`) |
| csystem.cpp L16840, L16854 | 2 | T | WAR1604 negative-pressure checks `&& false`: MACHINE_PATCHES_20261005.md:86 「負壓 `&& false`」 -- note: added **silently inside PKG-133** `529f87d5` (10-03 17:37), no AI tag, not in the message (see §4 N2) |
| cinitial.cpp L3044-3050, L3079, L3088, L3225-3226 | 11 | T | TEMP-DOORS / 「JerryYang 20260926 安全門還沒好」: MACHINE_PATCHES_20261002.md:40 |
| cinitial.cpp L5818, L5855 | 2 | S | main-ahead: `6c81fdf4` TEACH-INDEXZ-OUTSHT (package 149, machine-authored dispatch) rewrote `Prod.TestZ1_Place`; machine has not applied 149 |
| cinitial.cpp L7941 | 1 | S | same code on main + `AI(W906-MACH0210)` comment: MACH0210_LEDGER_20261005.md:41 |
| WebBridge/WebBridgeServer.cpp L1390, L1401-1402, L1450-1451 | 5 | T | TOKEN-OFF: MACHINE_PATCHES_20261002.md:39 |
| WebBridge/WebBridgeServer.cpp L1449 | 1 | M | `panel.key` (+ `panel.estop`) token exemption -> M3 (estop half -> U1). Main's same line carries St02 !176 `act.observerSG.state`, which the machine never applied (MACHINE_PATCHES_20261005.md:19) |
| WebBridge/WebBridgeServer.h L104 | 1 | T | TOKEN-OFF `enforceControlToken` |
| tools/wb_serve.cpp L4323, L5287, L8367 | 3 | T | TOKEN-OFF (main :8366 comment "main has no machine patch 0018 TOKEN-OFF") |
| tools/wb_serve.cpp L5739 | 1 | M | `panel.key` (+ `panel.estop`) main-loop dispatch -> M3 |
| tools/wb_serve.cpp L848 | 1 | U | `panel.estop` inside the blocking-dialog wait -> U1 |
| tools/wb_serve.cpp L5982, L5991, L6030, L8866-8889, L9056-9058 | 28 | M | EXIT-ORDER -> M5 |
| Motor/mymotor.cpp L3616-3622, L3635-3639 | 12 | T | TRAYSAFE / -2 / -3 comments (code `if(p>iSafePos)` is golden on both): MACH0180_LEDGER_20261004.md:32, :34; MACHINE_PATCHES_20261005.md:14 |
| Motor/myGALILmotor.cpp L6488 | 1 | T | HOMEPOS0 (Index Z safe pos 0 after the Galil home): MACH0180_LEDGER_20261004.md:52 |
| uhome.cpp L708-712, L732-736, L4349-4352, L4516-4520 + 30 `W906_HomePos(...)` wrappers (L3562 ... L5182, incl. L4201) | 48 | T | HOMEPOS0 / HOMEPOS0-2: MACH0180_LEDGER_20261004.md:33, :35, :52; MACHINE_PATCHES_20261005.md:46, :86 |
| uhome.cpp L4058, L4314 | 2 | T | HOME-TRAYWAIT: MACH0180_LEDGER_20261004.md:42 |
| uhome.cpp L720-721 | 2 | T | TRAYSAFE-3 (`W906_HomeTrayArm` bCheckPos=false on HT9050): MACHINE_PATCHES_20261005.md:14 |
| uhome.cpp L4403-4423 | 18 | S | HOME-1530POS + HOME-AUDIT1005 -> main `W906_Home1530GiveUp` (`AI(W906-MACH0218)`): MACHINE_PATCHES_20261005.md:60; MACH0180_LEDGER_20261004.md:41 |
| uhome.cpp L2568-2589 | 21 | S | PKG140-MERGE (`af5f5f63`, 10-04 12:05): HOME-BRAKE + BRAKE-SERVOFIRST are both on main (MACH0180 `3e607395`, MACH0180_LEDGER_20261004.md:21) but in the opposite order -- see §4 N1 |
| uhome.cpp L4239 | 1 | D | flag19 (machine does not wait for MLoaderZ): open question W-79 -- MACHINE_PATCHES_20261005.md:86; WAITING_REPLIES.md:93 「沒回之前 main 維持等 flag19」 |
| WebMotorAccessLive.cpp L201, L220-228 | 9 | D | BRAKE-BOOT `goldenPower`: MACHINE_PATCHES_20261005.md:44 「機台 PKG-140 留下的 goldenPower（BRAKE-BOOT）不收」; main :201 `AI(W906-MACH0216)` |
| WebMotorAccessLive.cpp L823-829 | 7 | M | MT-ACCLIVE -> M1 |
| WebMotorAccessLive.cpp L22, L452 | 2 | M | TEACH-HOMEALL -> M2 |
| WebMotorAccess.cpp L641-642 | 2 | M | MT-ACCLIVE -> M1 |
| WebMotorAccess.cpp L96, L4603, L8859-8888 | 32 | M | TEACH-HOMEALL -> M2 |
| WebMotorAccess.cpp L1491-1497, L3477-3492, L3513 | 22 | M | HOME-PERAXIS -> M4 |
| WebMotorAccess.h L267 | 1 | M | TEACH-HOMEALL (`GoldenHomeAll`) -> M2 |
| WebMainScanKey.cpp L521-553 | 33 | M | TEACH-HOMEALL (`W906_PanelHomeKeyArm`) -> M2 |
| WebMainScanKey.cpp L47, L422-494, L572 | 75 | M | SOFTKEY + SOFTKEY-DIAG -> M3 |
| WebMainScanKey.cpp L496-519 | 24 | U | SOFT E-STOP -> U1 |
| ckernel.cpp L3162, L4117-4118 | 3 | M | SOFTKEY hook in `ScanPannelKey` -> M3 |
| tests/test_main_scankey.cpp L265-282 | 18 | M | test [13] soft keys (+ E-STOP checks) -> M3 |
| tests/test_main_scankey_stubs.cpp L27-30 | 4 | U | `W906_MotorAccessOnAlarm` counting stub for E-STOP -> U1 |
| tests/test_scankey_golden_machine_stub.cpp (absent) | 5 | U | no-op stub for E-STOP (`845a08a0`) -> U1 |
| tests/test_web_motor_access.cpp L279, L964-965, L1045, L1349-1366 | 22 | M | TEACH-HOMEALL -> M2 |
| tests/test_web_motor_access.cpp L1367-1382, L3528-3534 | 19 | M | HOME-PERAXIS -> M4 |
| tests/test_web_motor_access.cpp L2939-2963 | 10 | S | main-ahead: `dc65097a` (package 149) EXT row 7 recount |
| tests/test_machine_cylinders.cpp L115-118 | 4 | S | same CHECKs on main + MACH0210 comments: MACH0210_LEDGER_20261005.md:50 |
| tests/test_gear_teach_save.cpp L342 | 1 | S | main-ahead `dc65097a` |
| FileRW/Teach.gen.inc L6 | 1 | S | main-ahead `6c81fdf4` (273 -> 274 TECH_PARA) |
| WebTeachButtons.gen.inc L5 | 1 | S | main-ahead `6c81fdf4` (6 -> 7 EXT rows) |
| tools/webprobe/teach_kb_golden_selftest.cjs L227-228 | 2 | S | main-ahead `dc65097a` |
| forms/fDTME08.cpp L410 | 1 | S | main-ahead `13b23512` Ifor I-03b DTM channel map (package 149) |
| tools/measure_tick.ps1 L38-40 | 3 | S | main-ahead `d8feab7d` Jerry (MR !214 -> main `5595d133`, 10-05 19:04, after package 149) |
| build.bat | 14 | D | machine blob == main blob of `676ad380` (09-24), replaced on main by `99af9f82` OBJROOT (09-25); mkpkg EXCLUDE: MACHINE_PATCHES_20261002.md:41 |
| tools/webprobe/f5_contract_probe.cjs | 27 | D | machine blob == main blob of `010f363e` (09-24); same doc line |
| cStateRecord.cpp L1473-1474 | 2 | D | comment-only (code identical): MACHINE_PATCHES_20261002.md:43; MACHINE_PATCHES_20260930.md:8 |
| tools/macro_order_gate.ps1 L1, L47, L64-74 | 11 | M | GATE-DRIFT -> M6 |

## 2. MISSED (M)

All six were committed on the machine on 10-03 between 14:12 and 16:41 (GitHub cpp 0150 / 0153 / 0154 / 0155 / 0156 / 0157). The laptop parked the ScanKey chain on 10-03 17:3x ("先等", TO_ES02.md:225 ②), handed it to St02 (TO_STEVEN.md:513: stack "(7) soft keys 0153 / 0156 / 0157 + web 0096 / 0099-0101, (8) 0154 / 0155; 0158 never"), and St02's MR !165 carried only 0152 + steps 2-4. St02 wrote it down: "**不在這張**：軟體鍵 0153／0156／0157＋web 0096／0099-0101（要 MT-ACCLIVE；軟體 ALARM RESET 也要跑 ScanPannelKey 的每鍵副作用，例如 N07 消音）、0154 HOME-PERAXIS／軟體 E-STOP（要裁決）、0155 EXIT-ORDER、0158 BYPASS-WAR1603 永遠不合" (FROM_STEVEN.md:451 on `origin/v906/steven-handoff` `5811d399`; also St02 skill `current-state.md:207`). The prerequisite (MC01 pushes MT-ACCLIVE standalone and re-exports 0152-0157 on package 133, TO_ES02.md:225 ③ / :228) never arrived, and nobody picked the rest up. The patches are byte-for-byte on `origin/v906/mc01-scankey-patches` (`173cdb1a`) `docs/handoff/mc01_scankey/`.

### M1 -- MT-ACCLIVE (known miss, confirmed) -- 9 lines

* **Where**: `WebMotorAccessLive.cpp` L823-829 (`GoldenSetCell` rows 8 / 9: `M->SetAccDataBase(v); M->SetAcc(v);` and the Dec twin); `WebMotorAccess.cpp` L641-642 (`Send1203SpeedValues`: `if (families & (kFamJog | kFamPtp))` instead of `kFamJog` only).
* **Chain commit**: `d0ea8a263` 2026-10-03 14:12 "PKG-132 + MT-ACCLIVE: GitHub main a071554 (GitLab main db9f0430) on the machine -- ... plus the machine's Acc/Dec edit taking effect at once" (= cpp 0150; hidden in a PKG commit).
* **What it does**: golden `strngrdMotorSelectCell` rows 8/9 (uMotorTest :1214 / :1216) only store the DB value; the runtime dAcc/dDec that SetSpeed sends follow only at the next `SetADCRate(100)` (e.g. a HOME). EastSun 10-03 「我把加速度上調也沒用」: Acc saved as 400000, every jog / move still sent 40000 (measured 13:54). The fix makes the runtime value follow the edit immediately, and lets the PTP family (Move+ / Move- / Go) also raise the card's CFG_AxMaxVel / MaxAcc / MaxDec ceiling (InitMotor had written the old Acc as the ceiling), not only the jog family (JOG-MAXVEL).
* **Main anchors** (`d07a03de`): `WebMotorAccessLive.cpp:816-817`, `WebMotorAccess.cpp:641`. Deviation from golden (DB-only) is covered by RULINGS_20260930 #11; MACHINE_PATCHES_20261005.md:86 already names it 漏收.
* **Conflicts**: none known. It is also the textual context cpp 0153 needs to apply (TO_STEVEN.md:511).
* **Owner**: laptop (3 code lines + comments) -- do it first.

### M2 -- TEACH-HOMEALL (known miss, confirmed) -- 90 lines

* **Where**: `WebMainScanKey.cpp` L521-553 (`W906_PanelHomeKeyArm`); `WebMotorAccess.cpp` L96 (catalog row `teachHomeAll`), L4603 (dispatcher), L8859-8888 (`TeachHomeAllDispatch`); `WebMotorAccess.h` L267 (`GoldenHomeAll` default -1); `WebMotorAccessLive.cpp` L22 (declaration), L452 (override); `tests/test_web_motor_access.cpp` L279 (fake), L964-965 (67 commands / 50 actions), L1045 (45 live incl. teachHomeAll), L1349-1366 (behaviour checks). Web half: web 0096 (`HW.teach.html` btnHomeAll, `page/ht9045_teach_homeall_c.js`, `JSON/motor-access.json` / `.js`).
* **Chain commit**: `aeca37f99` 2026-10-03 15:59 "TEACH-HOMEALL + SOFTKEY: (1) Teach 'Home All' button (uteach btnHomeAll -> motor.access teachHomeAll): runs the panel HOME key's arm (golden TfMain::ScanKey main.cpp:2545-2563, ..." (cpp 0153, 7 C++ files -- the SOFTKEY half is M3).
* **What it does**: a Teach-page button that runs exactly golden's panel HOME-key arm (BtnHome enabled / not SystemStart -> G06 Z1 check -> CheckAutoOnlySetOneBin -> P28 CheckAuto1OnlyBin1 -> bHomeByStart=false -> `fMain->Home`), returning 0 disabled / 1-2 bin check refused / 3 Home refused / 4 armed. Accepted only from uteach btnHomeAll, refused while any Teach / Motor Test home job or jog runs; on "armed" the live backend closes the Teach page (golden MainProc pauses while fTeach is shown). EastSun needed it because the physical keys do not communicate (RS-232 pad `uPadInterface` not ported -- Ifor01 IP-1 / W-78, NIGHT_REPORT 1005 17:3x row).
* **Main**: no `btnHomeAll` / `teachHomeAll` anywhere (C++ or web). Everything it calls exists on main (`MskBtnHomeEnabled` / `MskCheckAutoOnlySetOneBin` / `MskCheckAuto1OnlyBin1` from 0152 in WebMainScanKey.cpp; `W906_FormProgramShowHook` csystem.cpp:30204).
* **Conflicts**: `WebMainScanKey.cpp` is St02's (!165); test counts on main are now 66 / 49 / 44 (-> 67 / 50 / 45); `HW.teach.html` was hand-patched in 5 places for package 149 (MACHINE_PATCHES_20261005.md:73) so web 0096 needs a manual apply; 0153 does not apply on main without M1 first.
* **Owner**: St02 (it is one patch with M3: cpp 0153 + web 0096), laptop gates. (Alternative split: laptop takes the WebMotorAccess* half, St02 the `W906_PanelHomeKeyArm` arm.)

### M3 -- SOFTKEY family: screen panel keys (+ DIAG, + NOTOKEN) -- 98 lines

* **Where**: `WebMainScanKey.cpp` L47 (`#include <string>`), L422-494 (queue `kSoftKeyLifeMs` 3000 / `kSoftKeyMax` 4, `MskSoftKeyPop`, `W906_SoftPanelKeyPush` for START / PAUSE / HOME / RESET / ONECYCLE / CLEANOUT / TRAYFEED / TRAYEND / ALARMRESET / RETRY / SKIP, DIAG gate report), L572 (`s_lastNoticeUp`); `ckernel.cpp` L3162 (`ScanPannelKey` returns a queued soft key after golden's SystemInitialOK / SafeLock / employee-ID refusals), L4117-4118 (`int (*W906_VirtualPanelKeyHook)() = 0;`, null = golden); `tools/wb_serve.cpp` L5739 (main-loop dispatch of `panel.key`, same line as `panel.estop`); `WebBridge/WebBridgeServer.cpp` L1449 (`panel.key` token-exempt); `tests/test_main_scankey.cpp` L265-282 (test [13]). Web: 0096 (`page/ht9045_main_softkeys.js`, `main.html`), 0099 SOFTKEY-NOTOKEN, 0100 SOFTKEY-DELEGATE, 0101 SOFTKEY-ZINDEX.
* **Chain commits**: `aeca37f99` 10-03 15:59 (cpp 0153, soft-key half); `4b1e62d6d` 10-03 16:35 "SOFTKEY-DIAG: panel.key's ack (logged in the oplog OK line) now lists every golden gate in front of the key readers ..." (cpp 0156); `f23150a67` 10-03 16:41 "SOFTKEY-NOTOKEN: panel.key token-exempt like panel.estop (a screen panel key is a press, as the physical key) ..." (cpp 0157). (`git log -S` per string: `W906_SoftPanelKeyPush` / `W906_VirtualPanelKeyHook` / wb_serve `panel.key` -> `aeca37f9`; `s_lastNoticeUp` -> `4b1e62d6`; `cmdName != "panel.key"` -> `f23150a6`; test [13] in test_main_scankey.cpp -> `95597df3`.)
* **What it does**: screen buttons that inject one physical front-panel key press into golden's own key readers (TfMain::ScanKey, the Home hook, the alarm / message waits) through `ScanPannelKey`, so golden still decides; a key not read within 3 s is dropped, at most 4 queued; the ack names the golden gate holding it (SystemInitialOK, employee ID, InitialOK, SECSGEM lock, alarm / notice / IO / message box open, running); token-exempt like a physical press.
* **Known gap to fix while porting** (St02, FROM_STEVEN.md:451 and TO_STEVEN.md:513 step 6): the hook returns before `ScanPannelKey`'s per-key side effects, so a soft ALARM RESET skips e.g. the N07 silence.
* **Conflicts**: `WebMainScanKey.cpp` + `ckernel.cpp` ScanPannelKey (St01 E-034 / St02 N07 lines nearby); `tools/wb_serve.cpp` main dispatch line (main :5738); `WebBridgeServer.cpp:1449` now also holds St02 !176 `act.observerSG.state` (St02 claimed that line, FROM_STEVEN.md:175 on `origin/v906/steven-handoff`).
* **Owner**: St02 (ScanKey owner). The physical-pad port (Ifor01 IP-1) does not make this redundant (screen alternative), but say so in the card.

### M4 -- HOME-PERAXIS -- 41 lines

* **Where**: `WebMotorAccess.cpp` L1491-1497 (Motor Test `DoHome` release: no more `EndSingleHome`, which cancelled every axis's home job without stopping those axes; only the Light Scale single-home wait drops, and only if it was this axis), L3477-3492 + L3513 (Teach btnHome release on a 1203 axis: `CancelJobsOnAxis` + `Stop1203` on that axis only instead of golden's `StopAllMotor`; STOP still stops everything; Galil / non-1203 unchanged); `tests/test_web_motor_access.cpp` L1367-1382 (two axes homing, release one -> one job left), L3528-3534 (no StopAllMotor). Web half: web 0097 (`HW.MotorTest.html` / `HW.teach.html`: the HOME button follows the selected axis's runtime `homeJob`, switching axes no longer sends a release for the previous axis).
* **Chain commit**: `95597df3c` 2026-10-03 16:12 "HOME-PERAXIS + SOFT E-STOP: (1) axes home independently (EastSun ruling 20260929, asked again 1003 'X stops when I home Y'): releasing HOME on a 1203 axis ends and stops only t[hat axis] ..." (cpp 0154, part 1).
* **What it does**: EastSun 1003 「我按X回HOME 並且在回HOME過程中再去按Y X回HOME動作會停止 不是應該各歸各地嗎?」 -- manual single-axis homes on different 1203 axes no longer cancel / stop each other.
* **Caveat**: St02 parked 0154 as 「要裁決」 (bundled with SOFT E-STOP). It deviates from golden (uteach :2194 StopAllMotor; uMotorTest :1166-1170 one shared bSingleHome). Basis to take it: the 0929 "axes independent" rule is already on main for Motor Test motion (MT-AXISLOCK cpp 0027, MACHINE_PATCHES_20260930.md:43, taken under RULINGS_20260930 #11); S26_FINDINGS_20261004.md:122 / :367 / :492 list it as machine-only. One-line OK from Jimmy before porting.
* **Conflicts**: none known in `WebMotorAccess.cpp` (main :1490-1493, :3473-3474, :3495); test file shared with M2 and the package-149 hunks (separate places). Web 0097 also carries soft E-STOP lines in `ht9045_main_softkeys.js` / `main.html` -- leave those with M3 / U1.
* **Owner**: laptop (C++ + tests + the Motor Test / Teach part of web 0097).

### M5 -- EXIT-ORDER -- 28 lines

* **Where**: `tools/wb_serve.cpp` L5982 (`W906_ExitWatchdogStart()` when the main loop breaks), L5991 (`W906_HmiShellQuit` removed from right after "Program Close"), L6030 (`W906_HmiShellQuit` + `W906_ExitDoneLog` just before `stopped`), L8866-8889 (watchdog thread + the two oplog helpers; L9056-9058 are their brace lines in the multiset).
* **Chain commit**: `908a07884` 2026-10-03 16:24 "EXIT-ORDER + GATE-DRIFT: (1) EastSun 1003 'Exit does not seem to close everything': the HMI window now closes LAST (W906_HmiShellQuit moved after server.Stop / TesterComm / log objects / Pci1203ControlDisable / ..." (cpp 0155).
* **What it does**: the HMI window used to vanish while the server, TesterComm, log objects and the 1203 card were still closing, so a hung shutdown was invisible and never ended. Now the window closes last, a 15 s watchdog writes stdout + an oplog `EXIT` line and calls `ExitProcess(3)` (stand-in for golden FormClose's 10 s taskkill, main.cpp:11936-11950), and the oplog records begin / complete / fired.
* **Main**: still closes the HMI at :5990, no watchdog. `FileRW/MainClose.cpp:1108-1109` lists golden's taskkill as "missing -- 刻意不做" because that bat would kill a BCB6 HT9045.exe on the same PC; EXIT-ORDER only ends its own process, so it does not contradict that.
* **Why missed**: TO_ES02.md:225 ② 「cpp 0155 EXIT-ORDER 依賴那一串，一起等」 (only textual context; the code is independent of ScanKey).
* **Conflicts**: `tools/wb_serve.cpp` is a shared hot file (HMI-SHELL 0930, NB2-1 !177 F5-CLOSE, St02 S-24 hooks in batch 70) -- re-check claims at port time.
* **Owner**: laptop.

### M6 -- GATE-DRIFT -- 11 lines

* **Where**: `tools/macro_order_gate.ps1` L47 ("The three known" -> "The known"), L64-74 (expected sites re-baselined `ainarm9045_2x8_32.cpp` 4365 / 4393 -> 4335 / 4363, `Automation/auto9045.h` 228 -> 209; two documented false positives `FileRW/TTLCfg.cpp:281`, `TesterComm/UiHome.cpp:74`), L1 (machine copy has no UTF-8 BOM).
* **Chain commit**: `908a07884` (same as M5, part 2).
* **What it does**: makes the macro-order hygiene gate pass again. Verified on main `d07a03de`: the sources sit exactly at 4335 / 4363 / 209 and both false-positive sites exist, so main's table (last touched `96f3bd0e`, 09-11) is stale.
* **Port note**: keep main's BOM (the file has ~363 non-ASCII bytes; PowerShell 5.1 on the ACP-950 laptop needs it); take only L47 + L64-74.
* **Conflicts**: none. Not wired into ctest (comments only), so low priority.
* **Owner**: laptop.

## 3. Uncertain

### U1 -- SOFT E-STOP (36 lines) -- needs Jimmy's ruling before anyone ports it

* **Where**: `WebMainScanKey.cpp` L496-519 (`W906_SoftEmergencyStop`: drops queued soft keys, `StopAllMotor(true)`, `W906_MotorAccessOnAlarm("SOFT-ESTOP")` stops every opened 1203 axis and every Teach / Motor Test job, `SoftStop = SoftStart = SystemStart = false`, aborts a running full HOME like the panel PAUSE); `tools/wb_serve.cpp` L848 (`panel.estop` served inside the blocking-dialog wait) plus the `panel.estop` half of L5739 and of `WebBridgeServer.cpp` L1449 (token-exempt); test support `tests/test_main_scankey_stubs.cpp` L27-30, `tests/test_scankey_golden_machine_stub.cpp` (5), `tests/CMakeLists.txt` L7849-7850, and the E-STOP checks in test [13]. Web: parts of 0096 / 0097 / 0100.
* **Chain commits**: `95597df3c` 2026-10-03 16:12 (cpp 0154, part 2); the two test-stub pieces came in `845a08a0b` 2026-10-04 22:13 "PKG-141..144: ..." (machine-only stub so St02's `test_scankey_golden` links).
* **Why not M**: user ruling **P11-ESTOP** (2026-09-23) 「不用，已經有實體控制」 -- the web does not do an emergency stop (INBOX_QUEUE.md:414; DUET3D_REFERENCE_ANALYSIS.md:1319-1321). EastSun asked for one on 10-03 (「按鈕裡面我也需要緊急停止按紐」) while the panel keys did not work; the code itself says it is a software stop, not the hardware E-stop loop. St02 also marked 0154 「要裁決」. Not recorded as "leave on machine" either, so neither D nor M.
* **If Jimmy says yes**: port with M3 (St02); if no: record "SOFT E-STOP stays machine-local" in the next MACHINE_PATCHES so it stops showing up.

## 4. Notes (residual differences and the hidden-in-PKG census)

* **N1 -- uhome.cpp case 302 brake order (classified S above).** Machine (PKG140-MERGE inside `af5f5f63` PKG-135..140, 10-04 12:05, message: "brake release once per HOME"): the five golden brake groups are released once (`if(s_W906HomeBrakeTicks==0)`) BEFORE the BRAKE-SERVOFIRST wait (`W906_BrakeMotionReadyHook`, 5 s bound). Main (MACH0180 `3e607395`, 10-04 21:xx; MACH0180_LEDGER_20261004.md:21): wait first (uhome.cpp:2605), group release after (:2606-2621). Main's ledger reasoning: the per-axis tick (!163) releases the brakes and 302 waits for it, the group release then mostly logs "yes". So main relies only on `BrakeAxisTick` to pass the wait; the machine releases the groups first. Same pieces, different order -- worth one look by the brake owner (NB2-1, !163), not a miss.
* **N2 -- machine changes hidden inside PKG-* / MERGE-* commits that are still in the tip** (all accounted for above): PKG-132 `d0ea8a26` MT-ACCLIVE (M1); **PKG-133 `529f87d5` WAR1604 negative-pressure `&& false`** (T -- but untagged and not in the message; RULINGS_20261005 #10 (RULINGS_20261005.md:112) wants `AI(W906-TEMP-<name>)` and the README "current bypasses" list: ask the machine to tag it); PKG-135..140 `af5f5f63` TRAYSAFE-3 (T), PKG140-MERGE (S / N1), BRAKE-BOOT union (D); PKG-141..144 `845a08a0` E-STOP test stub (U1) and !176 not applied on WebBridgeServer.cpp:1449; PKG-146 `ac3721c5` flag19 HOMEPOS0 wrapper (T); PKG-147/148 `76a370f5` kept its own uhome / WebMotorAccessLive / cinitial / csystem (flag19 D, HOMEPOS0 T, BRAKE-BOOT D). No other hidden machine change survives in the tip.
* **N3 -- main-ahead lines (21)**: package 149 (`6c81fdf4` TEACH-INDEXZ-OUTSHT, `dc65097a` test recount, `13b23512` Ifor I-03b) and Jerry `d8feab7d` (MR !214, after 149). They disappear when the machine applies 149 / 150.
* **N4 -- scope limits**: web tree not audited (the web chain stops at web 0084); the web halves of M2 / M3 / M4 / U1 are web 0096 / 0097 / 0099 / 0100 / 0101 and none is on main web (no `ht9045_main_softkeys.js`, `ht9045_teach_homeall_c.js`, `btnHomeAll`; `HW.MotorTest.html` has only MT-AXISLOCK's `lock.perAxis`). `tools/vscode-htdesigner/` and `.vscode/` (F5-EXTCON cpp 0127) excluded as instructed.
* **N5 -- suggested order for batch 71**: M1 (laptop) -> M5 + M6 (laptop) -> M4 after Jimmy's one-line OK (laptop) -> M2 + M3 as one St02 MR on top of M1 (St02, with the ALARM RESET side-effect fix) -> U1 only if Jimmy overrides P11-ESTOP.
