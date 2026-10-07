> 保存來源：`.claude/skills/ht9045-state-record-analysis/SKILL.md`，main `811d95068`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../common.md)。

<!-- preserved-content:start -->

# State Record Hang Analysis

## When to Use
- Investigating whether a machine thread has hung up.
- Determining whether `MainProc()` stopped entering normally.
- Reading a state record folder that contains `Task_ListWithTime.csv` and `Task_ListWithTime2.csv`.
- Comparing the last task activity time against the state record save time.
- Interpreting `MainProcMonitor` after the heartbeat logging change.

## Inputs
- State record folder path.
- Optional project path when code confirmation is needed.
- Optional note about whether the machine was in a normal run, paused state, alarm flow, or manual capture.

## Primary Files
- `Task_ListWithTime.csv`
- `Task_ListWithTime2.csv`
- `EventLogTxt_YYYYMMDD.csv`
- `MainForm.bmp` — screenshot of the machine UI at capture time, **shows the running software version** in the title bar / version label
- `\HT9045\` subfolder — the customer's actual working files at capture time, including `system\Gerneral.ini`, `system\ATC.ini`, `IniData\Data\<Recipe>\Temperature.Data` etc. **Treat as primary source of truth** for reproducing the issue (config flags, recipe values, ATC settings)
- `Ver.txt` (if present) — plain-text software version string
- Folder/file write timestamps when needed for cross-checking

## Pre-Analysis Checklist (do this BEFORE deep code-diving)

Before chasing a code path, always verify the following from the state record. These steps have repeatedly avoided wasted hours.

1. **Identify the customer's exact software version.**
   - First check `Ver.txt` if present.
   - Otherwise open `MainForm.bmp` — the title bar/version label shows the running version (e.g. `V3.21.895.2`).
   - Cross-check with `\HT9045\system\Gerneral.ini → [Version] Ver=`.
2. **Compare against the latest dev version.** If the customer is **not** on the latest, run a SVN diff for the suspect file/function before assuming it is a new bug:
   ```powershell
   svn cat "file:///U:/SourceCode/SVN/HT9011UC_Code_V3.20/<file>.cpp" -r <customer_rev> | Select-String "<keyword>"
   svn cat "file:///U:/SourceCode/SVN/HT9011UC_Code_V3.20/<file>.cpp" -r HEAD       | Select-String "<keyword>"
   svn log  "file:///U:/SourceCode/SVN/HT9011UC_Code_V3.20/<file>.cpp" -r <customer_rev>:HEAD --search "<keyword>"
   ```
   The reported issue may already be fixed in a newer revision — in which case the resolution is simply to ship a newer release, not to write new code.
3. **Read the runtime config from `\HT9045\` subfolder** before tracing code branches. Example items that change which code path executes:
   - `system\Gerneral.ini` → `CUSTOMER_CODE`, `Model`, `Serial No`, `USE_16_HEATER`, `[ATC] USE_ATC_MODE`, `ATC_SYSTEM_USEHEAT`, `ATC_SYSTEM_IP/PORT`
   - `system\ATC.ini` → `[System] iATC_MODE_TYPE`
   - `IniData\Data\<Recipe>\Temperature.Data` → `Multi Zone Enable`, `Zone Temp N Use`, temperature setpoints
   - `setup.inf` → currently loaded recipe name
   These values determine which `if(...)` branch the customer's machine actually executes; analyzing the wrong branch is a common waste of time.
4. **Only after** version + config are confirmed, proceed with `Task_ListWithTime*.csv` and `MainProcMonitor` analysis below.

## Core Procedure
1. Read `Task_ListWithTime.csv` and locate the `MainProcMonitor` line.
2. Extract these fields if present:
   - `Alive`
   - `CallCount`
   - `LastEnter`
   - `SilentSec`
   - `SaveTime`
3. Compute or describe the gap between `LastEnter` and `SaveTime`.
4. Read the tail of `Task_ListWithTime2.csv` and identify the latest normal task timestamps.
5. Compare three times:
   - Last normal task change time
   - `MainProc()` last enter time
   - State record save time
6. Decide whether the evidence points to:
   - normal or expected quiet time
   - suspicious delay or blockage
   - likely hang or thread stop
7. If needed, cross-check `EventLogTxt_YYYYMMDD.csv` for nearby machine actions, alarms, start/stop, or user-triggered operations.
8. If the case looks abnormal and the related code path is under review, inspect the run thread flow, especially `Synchronize(ThreadProcess)`, `ThreadProcess()`, and `MainProc()`.

## Interpretation Rules
- `Alive=N` alone is not enough to conclude the thread has hung.
- If `SilentSec` is only slightly above the threshold, it can still be a normal snapshot taken during a short quiet gap.
- `CallCount` is more useful across multiple captures. A single state record cannot prove whether it kept increasing afterward.
- If `Task_ListWithTime2.csv` stops much earlier than `LastEnter`, then `MainProc()` may still have been entering while no task state changed.
- If `LastEnter` is close to `SaveTime`, the main loop was still entering near the capture time.
- If `LastEnter` is far earlier than `SaveTime`, and task updates also stopped around that earlier time, hang suspicion is much stronger.
- If the state record was manually triggered, treat `SaveTime` as the capture/write time, not the fault start time.

## Recommended Classification
### Normal or likely normal
- `LastEnter` is near `SaveTime`.
- `SilentSec` is small or only slightly above threshold.
- Task updates are recent or the machine could reasonably be in a quiet state.

### Suspicious
- Task updates stopped noticeably earlier than `SaveTime`.
- `LastEnter` is newer than the last task update, but not very close to `SaveTime`.
- `Alive=N`, but the gap is still short enough that a temporary block or quiet wait is possible.

### Likely hang or loop stop
- `LastEnter` is much earlier than `SaveTime`.
- Task updates also stopped around the same earlier time.
- Repeated captures show the same frozen pattern.
- Code path suggests possible deadlock, especially around `Synchronize(ThreadProcess)` or GUI-thread dependency.

## Known Good Example
A normal-case reference from this workspace:
- `LastEnter=2026/04/02 17:39:27.318`
- `SaveTime=2026/04/02 17:39:32.806`
- `SilentSec=5.621`
- `Alive=N`
- Latest normal task updates were around `17:39:16.125`

Interpretation:
- `MainProc()` was still entering until `17:39:27.318`.
- The state record was written at `17:39:32.806`.
- `Alive=N` happened because the silence exceeded the 5-second threshold at capture time.
- This case should not be concluded as a hard thread hang based on `Alive=N` alone.

## Known Hang Example
A hang-like reference from this workspace:
- `LastEnter=2026/04/02 18:12:43.406`
- `SaveTime=2026/04/02 18:15:17.380`
- `SilentSec=154.070`
- `Alive=N`
- Latest normal task updates were also around `18:12:43.406`

Interpretation:
- `MainProc()` stopped entering at `18:12:43.406`.
- Normal task activity also stopped at essentially the same time.
- The state record was written about 154 seconds later.
- This pattern is strong evidence of a hung thread or a stopped main loop.

## Output Format
When reporting results, always include:
1. `MainProc()` last enter time.
2. State record save time.
3. Gap between them.
4. Latest normal task timestamp from `Task_ListWithTime2.csv`.
5. A plain-language conclusion:
   - normal
   - suspicious
   - likely hung
6. Confidence note and why.

## Response Template
Use this structure:

- `MainProc` last enter: `...`
- State record save time: `...`
- Gap: `... sec`
- Last normal task activity: `...`
- Interpretation: `normal | suspicious | likely hung`
- Reason: `...`

## Code-Level Follow-up
If the evidence suggests a hang, inspect these areas in the related project:
- run thread loop calling `Synchronize(ThreadProcess)`
- `ThreadProcess()` to `MainProc()` flow
- exception paths around `MainProc()`
- any GUI-thread dependency that can block synchronized execution
- sleep/wait loops that can mask a stalled state

## Workspace Notes
- In this workspace, `.cpp`, `.h`, and `.dfm` files with Chinese content are Big5 encoded.
- If analysis turns into code modification, use a Big5-aware edit path instead of a normal UTF-8 patch flow.

## Lessons Learned (from past investigations)

### LL-1: Always confirm the customer's running version FIRST
Spending hours tracing a code branch in the latest dev version is wasted if the customer is on an older release that simply lacks the fix. Quick checks:
- `MainForm.bmp` in the state record root shows the version label visible on the machine UI.
- `\HT9045\system\Gerneral.ini → [Version] Ver=` and (if present) `Ver.txt`.
- If the customer version is older than HEAD, do an SVN diff for the relevant file/function before any deep analysis.

**Case (2026-04-21, ATK / TeraTech Korea, ATC MultiZone "not working"):**
Customer was on `V3.21.895.2` (SVN r895). The MultiZone-bUse[]-expansion fix was added in r896 (`else if(TestIF.iTestMode==SingleSite && Temperature.bMultiZoneEnable)` branch in `main.cpp`). One SVN command resolved the case — no code change needed:
```
svn cat "file:///U:/SourceCode/SVN/HT9011UC_Code_V3.20/main.cpp" -r 895 | grep "iTestMode==SingleSite && Temperature.bMultiZoneEnable"  → empty
svn cat "file:///U:/SourceCode/SVN/HT9011UC_Code_V3.20/main.cpp" -r 896 | grep "iTestMode==SingleSite && Temperature.bMultiZoneEnable"  → found
```

### LL-2: Read INI / Recipe BEFORE tracing code branches
The `\HT9045\` subfolder under each state record contains the **actual** runtime configuration. Reading these files determines which `if(...)` branch the customer's machine actually executes. Examples that flip code paths:
- `[ATC] USE_ATC_MODE` → selects `eATC60 / eNewATCSystem / eATCHonPrecType / eWinWay` etc.
- `ATC_SYSTEM_USEHEAT` → selects channel count / heater mapping.
- `USE_16_HEATER` → selects `eHeaterType` enum (4 / 16 / 32 site, EJ1N, KT4H, DTME08).
- `Temperature.Data → Multi Zone Enable / Zone Temp N Use` → enables multi-zone bUse[] paths.
- `setup.inf` → identifies which recipe is loaded.

Without these, code analysis often targets the wrong branch.

### LL-3: When customer reports "feature X not working"
Order of investigation:
1. Version (LL-1) → maybe already fixed.
2. Runtime config (LL-2) → maybe a flag is off / set to a value that bypasses the feature.
3. Hang / thread analysis (the rest of this skill) → only when 1 & 2 don't explain it.

## Reference Files
- [references/analysis-sop.md](references/analysis-sop.md) — Analysis SOP
- [references/troubleshooting.md](references/troubleshooting.md) — Troubleshooting quick reference
- [references/case-ocr-barcode-hang-1150.md](references/case-ocr-barcode-hang-1150.md) — Case: OCR Barcode state 1150 hang (FMSH 884)

<!-- preserved-content:end -->
