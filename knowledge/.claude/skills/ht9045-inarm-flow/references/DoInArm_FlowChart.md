## Flow Summary

### 1. Initial Guard Checks (Early Return)
| Check | Condition | Action |
|-------|-----------|--------|
| Index Check | `bInitialStartIndexCheckDone == false` | return |
| HangUp | `iHPHangUpCount != 0` | Record, Show Error, Clear, return |
| F16 Sensor | `bF16CheckShuttleSensorBroken && bDoingF16` | Enable shuttle movement, return |

### 2. QA Mode Processing
- If QA Mode enabled and using `bQAModeUseUnloadCnt` -> skip
- Otherwise call `Check_QA_ModeCount()`, if true -> mark eligible suckers as `HAS_NULL_IC`

### 3. Auto Alignment CCD
- If `bLoaderNeedTrayMustFinish`:
  - Clean out with no IC -> clear flag, return
  - Has tray -> do alignment check, return
  - Clean out -> do alignment check, continue
  - Otherwise -> return
- If alignment running -> return
- If no CCD feature -> clear alignment flags

### 4. Index Jam / Main Arm Control
- **IndexJam enabled**:
  - **Safe position needed**: finish all pickers -> `InitInArmTask()` + `MoveInArm2XYToWait()`
  - **No safe position needed**: if shuttles not paused -> check D43 error or call `DoInArm_9045()`
- **IndexJam disabled**: check D43 error or call `DoInArm_9045()`

> Core execution always routes to **`DoInArm_9045()`** as the main operational function.
