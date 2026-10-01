# W36-1 / W58: the SIM build starts every launch with the FTP / network options off

- Status: St02 batch `v906/st02-w58-simnet`, its own MR (20261001). The 6 claim lines in the laptop's files (`cprod.cpp` :171 / :3217 / :3263 / :3323, `tools/wb_serve.cpp:4052`, `CMakeLists.txt:3321`, see below) are carried in this batch: the laptop approved them (TO_STEVEN s4 20261001 09:2x, "Your claimed lines ... stay yours"; St02-M 14:2x: St02 applies them). They were re-checked on main `7ec1841e` by line number with neighbour lines: all 6 at the claimed numbers, the OLD text unchanged, same-line edits (the three files keep their line counts).
- Tags: `AI(W906-W58) 20260930 (St02-E)`, on top of the 0928 prototype `AI(W906-SIM-W36-1) 20260928 (St02-E helper)`.
- Steven's answers (decisions-decided.md:2779-2812, 20260929 08:1x; W58-4 at 11:06):
  - **Q1 = 要**: ticked + saved in SIM = written to config.ini, so a later SHIP launch follows it. Every SIM launch still starts with the keys off (W36-1).
  - **Q2 = 要**: table C counts too. The C keys that really reach the network are in; those without a network effect are left out, with the reason (see the key table).
  - **Q3 = 關 + enable**: a customer-forced or greyed key is masked too, and its control is enabled.
  - **Q4 = 關, W58-4**: TCP 7016 / 7017 stay in SIM mode unless `W906_SIM_TCP_SERVERS=1` (TesterComm/Handler/TesterCommWiring.cpp, MR !12).
  - **Q5 = 算**: a save path on a network share is not written in SIM unless `W906_SIM_NET_PATHS=1`. This batch does the ELA side; the Handler writers are a later claim.
- Compiled in both configurations (SIM `build`, SHIP `build_ship`), with and without the claim lines. Nothing was run on STEVEN-NB3 (F-Secure: build only).

## How it works

| Where | Phase | What |
|---|---|---|
| `tools/wb_serve.cpp:4052`, before `FileRW_IniConfig_Boot` | install | SIM: `W906_SimNetHook` (cprod.cpp:171) = the mask, and `W906_ElaSetBoolOverride` = the ELA override. SHIP: `W906_SimNetInstall` is empty; both stay NULL. |
| `cprod.cpp:3217`, end of `ReadLastSetIni` (after `SetCustomerLimitationForConfig`) | 0 | The first phase 0 of a launch resolves the table. Every masked key: field + proxy = this run's value (off unless released this run). Nothing is written. |
| `cprod.cpp:3263`, start of `SaveLastSetIni` | 1 | The operator's choice is the proxy for an HTEditList key, or the field for a CheckConfigurationBeforeSave key. **Ticked**: released; field + proxy = the tick, and the golden save writes it (Q1). **Unticked after a release this run**: 0, written (the operator's choice). **Armed**: the file text is snapshotted and field + proxy = the file's own value, so the mask never reaches the file. **Forced** (Q3): this run only, because HTEditList never saves it. |
| `cprod.cpp:3323`, end of `SaveLastSetIni` | 2 | Armed keys: the snapshot is written back if the save changed it (a key / section the save added is removed again). Then field + proxy = this run's value. |
| `ElaHub.cpp:77` / `:79`, `ElaSchedule.cpp:273` | ELA | `IniBoolOverride`: armed (or not resolved yet) = false, released = true, anything else = the file value. It applies only to a file named `config.ini`. |
| `ElaSchedule.cpp` `SimNetKindBlocked`, `ElaReports.cpp` `SimNetSkip` | Q5 | SIM: a `KIND_NETDRIVE` job is off, timed and manual, so it is never run or retried. The O06 / N10 / VTEST / N34 writers skip a network-share path; that also covers the Hub's `EL_VTEST_MTBF_SUM` / `EL_UPLOAD_CHIPADV_LOTEND`. |

Resolution happens at the first phase 0 of a launch, in `HandlerEnv::Resolve`.

- **An HTEditList key** resolves to the FIRST `elConfig` entry of (section, key): a TCheckBox bound to a bool, or a TRadioGroup bound to an int.
  - Q3: a forced (`bReadFromFile == false`) or greyed (`bEnable == false`) entry is masked too. It gets `bEnable = true` and `SourceControl->Enabled = true`.
  - The page reads both live: `FileRW/_EditList.cpp:101` / `:104` / `:135`, and the save's `ELEditable` drop rule, `FileRW/IniConfig.cpp:273-292`.
  - `SourceControl->Enabled` is set at once, because this read's `InitialDataToEdit` (cprod.cpp:3064) ran before phase 0.
  - A hidden entry that is off is not masked: nothing to turn off, and golden's control stays as it is. That includes the registration's "this customer has no such function" fallback (`bNoShow, bDisable, bFixedValue, 0`).
  - A hidden entry that is on is masked **and shown** in SIM (`bVisible` + `SourceControl->Visible`), so it can be released (E2 W58 m1, St02-M's decision).
    - The one such row is VTEST N10-3, `bNoShow, bDisable, bFixedValue, 1` (FileRW/IniConfig.gen.inc:6324). **SIM-only deviation**: golden hides that control.
  - The decision is `simnet::Q3Decide` (SimNetMask.h), so the ctest covers it (section 2b) with the real registrations of KYEC_LEE N10-3 / N10-1 and VTEST N10-3.
- **A CheckConfigurationBeforeSave key** is masked only where golden reads it from the file for this customer, as in the prototype. The RMS section is `RMS` for CC_SCC / CC_SCK, else `Server` (cprod.cpp:2196).
- **Once per launch holds.** `IC_ChangeCBListProperty` re-sets entry properties at every `ReadLastSetIni`, but only for A09 / D42 / D44 / F06 / F11 / F14 / F14-1 / F17 / I04 / I06 / P16 / P24 / F26, and none of them is in the table.
- The boot console prints `SimNet W58 (SIM): phase 0: N network key(s) OFF for this run [codes] ...`. That line is the list for the machine it runs on.

## The key table (58 rows, `SimNet/SimNetMask.cpp` kRows)

- **Jimmy RULINGS_20261001 #3** (20261001): [N07-1] Enable SECS GEM and [N07-2] Enable RCMD START (host start) are **not** masked. The laptop runs SIM SECS / host-start tests, so in SIM these two read their config.ini value as in SHIP. Everything else stays masked (60 → 58 rows).
  - Steven's W58 table B did include SECS GEM, so St02-M asked Steven to confirm. **Steven 20261001 13:4x: W63 = A** -- SIM keeps [N07-1] and [N07-2] out of the mask; the other 58 stay masked.

- **A. FTP / net-drive (39)**:
  - The prototype's 37 rows (N06, N06-1, N10-1/2/3/9/11/12, N40-1, N12, N14-3/12/13/19, N17-1, N21-1, N22 ×2, N23, N35, N22-1, N23-1, N23-3, N25-2..5, N26-1, N27-1, N30-1, N31-1, N32-1, N33, N35-1, O06, A32, A55).
  - W58 adds **N14-8** `[Handler_OEE] bN14_8_ULSetup` and **N14-9** `bN14_9_ULQtyReport`. Both are FTP uploads (ProductionInfo.cpp:1554-1582 / :3380-3383) that neither table had.
- **B. network links (5)**: A81, N05, N05-1, N25-1, N24. (N07-1 is out, Jimmy RULINGS_20261001 #3.)
- **C. "in doubt", W58 Q2 (14)**:
  - The SECS sub-options (N07-2 `Enable RCMD START` is out, Jimmy RULINGS_20261001 #3): N07-3 `SECS GEM OneCycle`, N07-3-2 `SECS GEM Alarm`, N07-4 `[Specific] N07_SecsLotCheck`, N07-5 `Enable Employee ID Cheak`, N07-6 `bN07_6EnableUploadOSRecipe`, N07-6-1 `bN07_6CompressedFile`, N07-7 `bN07_7SendRecipeAsBinary`.
  - The rest: N13 `[ARMS] bN13_EnableARMSFunction`, N09-1 `[Automation] bN09_LotCountAutoFunc`, N14-4 `N14_HandlerOEEAutoLoadMOFile`, N14-10 `N14_AutoDownloadSetupFileByMO`, N14-22 `bN14_21_ConfigUpdateFromServerExport`, N15-1 `[ESD_Control] N15_ESDControlUserLevelByTxt`, N41-1 `bN41_1_HandlerDataBackUpToDiskUseFunction` (never registered: USE_BU5_Function is 0).
- **C left out, no network effect**:
  - A75 is an OP permission guard (uLotInfo.cpp:1328-1334).
  - N28 and N08-1 are local logs.
  - N09-2 only picks a message or a log line.
  - N09-4 has no "off": 0 = FTP, 1 = Net Drive.
  - N14-11 has no consumer.
  - N14-21: turning it off opens golden's auto-teach motion path (main.cpp:25164).
  - N14-23 is a file-format choice.
  - N15-2 is the local ESD machine.
  - N16 is fixed at 0 for every customer.
  - Enable Check File / Check Setup File are local lot-start checks.
  - B11 and N17-3 are local by default; a network path falls under Q5.
  - N14-18 is the temperature offset itself; its download belongs to N14-4.
- **Not in the table**: table D, and the sub-options of a masked parent (N26-2, N32-2..4, N33-1, the host / path fields).

## Files written (SKILL §4)

- **`config.ini`**:
  - Path: `AuthPath + "config.ini"`. AuthPath = `W906_AUTH_PATH`, else golden `D:\HT9045\config\` (common.cpp:139-145 / :168).
  - When: phase 2 only, in the SIM build only.
  - What: only the armed keys whose text the golden save changed, written back to the snapshot text (`WriteRawValue`, vclcompat's in-place rules), or removed when the save added them (`RemoveKey`).
  - How (20261001): **atomically**. The whole new text goes to `config.ini.simnet.tmp` in the same folder, which then replaces config.ini in one `MoveFileExA(MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)`. A crash leaves the old or the new file, never half; the temp file is removed on a failure. ctest SimNet_Write.
  - The released keys' ticks are written by the golden `SaveLastSetIni` itself (Q1), not by the mask.
- Nothing else. Q5 only suppresses writes.

## The `W906_SIM_NET_KEYS` offer: dropped (20261001)

- While W58 phase 1 was on HOLD, St02 offered an escape, `W906_SIM_NET_KEYS=1`, to turn the whole mask off for the laptop's SIM SECS / host-start tests.
- Jimmy RULINGS_20261001 #3 took the laptop's proposal instead: N07-1 / N07-2 stay out of the mask, the rest is masked. So the escape was removed (simnet::MaskWanted, its InstallHooks check and its boot line are gone). There is no switch; the mask is always on in SIM.

## ctest (compiled, not run here)

- **`SimNet_Mask`**:
  - 0: containment first.
  - 1: the 58-row table (C in / out; N07-1 / N07-2 not rows, Jimmy RULINGS_20261001 #3).
  - 2: raw ini helpers.
  - 2b: `Q3Decide` on the real registrations. KYEC_LEE N10-3 / N10-1 (greyed, forced on): masked, forced. VTEST N10-3 (hidden, forced on): masked and shown. The "no such function" fallback: not masked. The real THTEdit / page part stays a Human review item.
  - 3: launch; the armed keys and a customer-forced key are off, nothing is written.
  - 4: the ELA while armed.
  - 5: a save with nothing released leaves the file byte-identical.
  - 6: released = on and written (N25-5 / N06-1 / N31). Forced N25-4 is on this run only and never written. Unticked + saved = off, with 0 in the file.
  - 7: a new launch is off again although the file says on; a save keeps the file's on.
  - 8: not ready.
  - 9: the SIM gate.
  - 10: the end.
- **`SimNet_Write`** (20261001, both configurations): WriteRawValue gives the in-place writer's bytes (a value, a missing key, a missing section, LF lines, a missing file), no temp file is left, a stale temp file is overwritten; RemoveKey is atomic and writes nothing for a missing key.
- **`SimNet_Mask` section 4** (Jimmy RULINGS_20261001 #3): in SIM the ELA reads N07-1 Enable SECS GEM and N07-2 Enable RCMD START at their config.ini value (1); section 5: after a save, N07-1 still reads its file value while A81 stays off.
- **`ELA_Schedule`**:
  - 4: `SimBlocksNetPath` (UNC / DRIVE_REMOTE blocked in SIM, not a local path, not with the env var, never in SHIP).
  - 7: an N17 job to `\\srv\RMS` in SIM is off, a manual run is refused, and it is on with `simNetPaths`.

## Side effects / limits

- **Read time follows the file.** Phase 0 runs after `ProcessLastSetIni_*` (`cprod.cpp:3201-3216`), so what those readers do while reading still follows the file (TSMC's FTP DataPath switch, SPIL's RMS XCOPY).
- **SPIL WAR16132** (cprod.cpp:3307-3312) warns when the N07-2 proxy is unticked while `bRCMDStart` is still true.
  - After a release, phase 1 now sets only the proxy of an unticked HTEditList row, and the field keeps the released value until phase 2 (E2 W58 n1). So golden's warning still fires.
  - For an armed key, both hold the file value in phase 1, as before.
- **Runtime writers bypass the mask until the next reread** (E2 W58 m2):
  - A tester SVID query while SECS is off force-enables SECS (TesterComm/Handler/HandlerGpibMsg.cpp:503 = golden 906 main.cpp:15679). Since Jimmy RULINGS_20261001 #3 N07-1 is not masked, so this one no longer interacts with the mask.
  - SIGURD's `SGFTP_ON` sets A32 (Command.cpp:12475).
  - Both act as a host-initiated release for this run. The slot stays armed, so the next ReadLastSetIni (page open, save) turns the key off again. No code change: golden behaviour, and in SIM both need a real tester / GPIB command.
- **Q1 and Q5 together** (E2 W58 m3): ticking a network-drive key (O06 `bAlarmStatistAutoSaveNetDrive`, or N10's Net Drive method) releases it, but the ELA job still does not write to a network share without `W906_SIM_NET_PATHS=1`. To verify such a job, do both.
- **Q5 counts mapped drive letters** (E2 W58 m4): Steven's option A said "starts with `\\`", and his reason (「有些電腦沒有打開對應的連結」) covers a mapped `Z:` too. So the rule is IsRemotePath (UNC, `//`, DRIVE_REMOTE). This is an interpretation, listed in the Human review.
- Between phases 1 and 2 an armed field briefly holds the file value. The only thing in that window is `SendCommand_EventLog(EL_UPDATE_PARAMETER)`, and the ELA reads through the override, so it stays off (E2 n2).
- `RemoveKey` rewrites the whole file, now atomically like `WriteRawValue` (20261001; was a direct `fopen("wb")`, E2 n3). It's used only when a save added a key that wasn't in the file.
- N41-1 and N40-1 are inert rows: never registered, since `USE_BU5_Function` is 0 (E2 n4).
- **A hidden key that is on** is masked and shown in SIM, so it can be released (m1 above; the one such row is VTEST N10-3).
- **Sub-options on the page** may show disabled until their masked parent is released and saved (then a second save).
- **SCK**: releasing N05 needs golden's RMS password. ERMS (N05-1) can be released only on the `bShowLotInfo` branch.
- **ASE-CL** registers N23 / N35 in `cbConfig_byRecipe`, not `elConfig`, so they are not masked.
- **Q5, Handler side: not guarded yet.** This is a later claim (the laptop's files):
  - cObserver.cpp O06 folders, fTesterTCP.cpp B05, uPAT_Function.cpp B14, SCK_ART_Remainder.cpp N09 / N17-1, fRPDefault.cpp N14-20, uHGemHT9045.cpp N07-6, cMyDB.cpp O19, cprod.cpp N10.
  - N17-1's default `asN17LotSummaryPath` is itself a `\\` share. Once the Handler side exists, N17-1 stays blocked in SIM even when released, unless `W906_SIM_NET_PATHS=1`.
- **Not masked**: the TSV listen socket and the OLP link follow the customer code only, like 7016 / 7017.
