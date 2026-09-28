# 動作流程功能清單：對照檔 FT005054 的每一個 task 步驟，C++ 有沒有（20260928）

> 使用者 0928 10:4x（RULINGS_20260928 第 2 條）：對照檔（`D:\HT9045\Staterecord\2025-12-11 17_47_57`，BCB6 在真機上的紀錄）當「功能清單」——
> 真機走過的每一個 task 步驟，C++ 都要有、而且是活的。這份就是量尺。
>
> **怎麼量的**：`Task_ListWithTime.csv`／`Task_ListWithTime2.csv` 裡 210 個有值的 task、463 組（task, 步驟值）；3 個唯讀 agent 逐一查移植樹那個 case 在不在、
> 是不是活的（沒被 `#if 0`／GATE 擋、不是空殼、從 tick 迴圈呼叫得到），每一條都附 golden 與移植樹的行號（workflow `wf_5a7678ce-487`，0928 10:49～11:40）。
> 只有閒置值（通常是 1）的 task 算「活的」，條件是變數存在、有登錄、重設的地方存在。
>
> **狀態**：活的＝LIVE；被閘住＝GATED（case 在，但它的出口或裡面的動作被 `#if 0`／no-op 擋住）；空殼＝STUB；缺＝MISSING；走不到＝UNREACHABLE（程式是活的，但進入它的前一步被閘住）。
>
> ⚠ 這是**靜態**量測（讀程式），還沒實跑；實跑對照用 `tools/flowcmp`（S1 段）。
> ⚠ 對照檔那台是 HT9046_LS（`LoaderUnload_StepMotor=1`、`REAL_TIME_CCD=1`）；HT9050 機台設定（0928 裝進筆電）這兩項是 0，所以 `SetStepMotorTask` 與 RTC 那幾步在 9050 上本來就不會走。

## 總表

| 狀態 | 步驟數 |
|---|---|
| LIVE | 245 |
| GATED | 13 |
| STUB | 0 |
| MISSING | 17 |
| UNREACHABLE | 17 |
| UNKNOWN | 0 |

**一句話**：托盤／Loader 組全部是活的；HOME／初始化／手臂組只缺 `SetStepMotorTask`（9050 不用）；**AutoClean 在 C++ 上跑不完**（6 個 `if(false)` 閘＋`InArmZNeedDown_9045` 空殼，卡在 `AutoCleanTask=2400`）——FLOW-2 正在補。

## HOME／開機初始化／入出料臂／Shuttle（12 個 task 有走流程、76 個只有閒置值）

| task | 對照檔走過的步驟數 | 活的 | 被閘住 | 空殼 | 缺 | 走不到 | 狀態 |
|---|---|---|---|---|---|---|---|
| `HomeStep` | 39 | 39 | 0 | 0 | 0 | 0 | ✅  |
| `SetStepMotorTask` | 12 | 0 | 0 | 0 | 12 | 0 | ⛔ 不通的步驟：1:MISSING 100:MISSING 1000:MISSING 1100:MISSING 2000:MISSING 2100:MISSING 3000:MISSING 3100:MISSING 4100:MISSING 5100:MISSING 6000:MISSING 6100:MISSING |
| `InitialICCheckTask` | 10 | 10 | 0 | 0 | 0 | 0 | ✅  |
| `InitialStartTask` | 7 | 7 | 0 | 0 | 0 | 0 | ✅  |
| `AutoSHT2Task` | 6 | 6 | 0 | 0 | 0 | 0 | ✅  |
| `InArmTryPickFromHotPlateTask` | 5 | 5 | 0 | 0 | 0 | 0 | ✅  |
| `OutArmTask` | 4 | 4 | 0 | 0 | 0 | 0 | ✅  |
| `AutoSHT1Task` | 4 | 4 | 0 | 0 | 0 | 0 | ✅  |
| `InArmTask` | 2 | 2 | 0 | 0 | 0 | 0 | ✅  |
| `AOITask` | 1 | 0 | 0 | 0 | 1 | 0 | ⛔ 不通的步驟：1:MISSING |
| `Auto_TeachPitch` | 1 | 0 | 0 | 0 | 1 | 0 | ⛔ 不通的步驟：1:MISSING |
| `ConntectionOkTask` | 1 | 0 | 0 | 0 | 1 | 0 | ⛔ 不通的步驟：1:MISSING |

**這一組最重要的缺口**（agent 原文，英文）：

1. SetStepMotorTask MISSING (all 12 observed steps 1,100,1000,1100,2000,2100,3000,3100,4100,5100,6000,6100). The golden TdmTrayMotor data module (Motor/TrayStepMotor.cpp: DoTrayStepMotor :109-395, switch :134, timer tmrTrayStepMotorTimer :102, StartSetSpeed :89, RS232Init :56) is not in the port; the only trace is dfm2rc layout metadata (tools/dfm2rc/layout_out/Motor/TrayStepMotor_layout.gen.cpp:12). Four gates depend on it: cStateRecord.cpp:1787-1789 (registration), uhome.cpp:997-999 (HOME case 1 StartSetSpeed, golden uhome.cpp:1335), cinitial.cpp:16999 (RS232Init) and cSpeed.cpp:890-977 (GATE S5 speed table). The reference Gerneral.ini:477 and the local HT9050 D:/HT9045/system/Gerneral.ini:585 both have LoaderUnload_StepMotor=1, so the loader-tray stepper speed table is never pushed over RS232 on HT9050. Fix: port TrayStepMotor.cpp, create a dmTrayMotor global, drive the timer from the tick loop, then remove the four gates.
2. The GATE G10 in DoOutArm (aoutarm.cpp:1800-1803) no longer has a valid reason. It skips FRCarryKit/BRCarryKit.SetHasNullIcToNullIc(), which golden runs on every OutArm tick (aoutarm.cpp:1197-1198). The gate says mykitsuck.cpp is unregistered and the class only has the inverse method. Both claims are now false: mykitsuck.cpp is registered (CMakeLists.txt:2003), aHotPlateSubstrate.h:97 includes mykitsuck.h, and TMyKitSuck::SetHasNullIcToNullIc is defined live at mykitsuck.cpp:574. Remove the gate.
3. Four guard early-returns in the arm dispatchers are still gated for dependencies that now exist. (1) ainarm9045.cpp:898-900: MOT[MTestZ1].Gali_Command("ST") (golden ainarm9045.cpp:4451); TMyMotor::Gali_Command is live at Motor/myGALILmotor.cpp:1385 and already used live at uhome.cpp:2127. (2) ainarm9045.cpp:906-911: fNote->bMyServoOffInArm (golden :4457); the member exists at forms/fNote.h:285. (3) ainarm9045.cpp:953-958: bEject half-clean return (golden :4502); bEject is defined live at acatchtray_shims.cpp:208 and used live in atester.cpp:6127 and acatchtray.cpp:6155. (4) aoutarm9045.cpp:761-766: fNote->bMyServoOffOutArm (golden aoutarm9045.cpp:413); the member exists at forms/fNote.h:305. Items 2 and 4 are the servo-off-after-alarm protections (bAlarmNeedServoOff), so they matter for safety.
4. DoInitialICCheck has two no-op seams on observed steps. (1) W906G4_FYIELD_CLEARAUTOSITEOFF (csystem.cpp:6333, used at :6504 in case 1; golden :8412) no longer has a valid reason: TfYieldMonitoring::ClearAutoSiteOffStatus is live at uYieldMonitoring.cpp:812 and already called at cContactCT.cpp:1333. (2) W7C1_FLTCSENSOR_CLEAR (csystem.cpp:2574, used at :6630-6631 in case 2000 and :6811-6812 in case 10000; golden :8538-8539 and :8719-8720) is justified in the code only as a missing include. The deeper problem is that TfLtcSensor itself is an offline shim: GetLtcSensor returns 0 (acarry_shims.cpp:42) and SetLtcSensor is empty. The latch-sensor module is not ported, which matters on Y-latch (LS) shuttles.
5. HomeStep sub-gates inside live steps (the step transitions themselves are live). Case 650 does not wait for the RTC CCD home ack (uhome.cpp:3193-3208), and case 600 does not send it (:3141-3146). The reason still holds: I searched the port headers for rtHome, bRealTimeCom_ReceiveOK, OpenRTCComPortAgain and SendCommToVision and found no live declarations. This only affects REAL_TIME_CCD machines: the reference has REAL_TIME_CCD=1, the local HT9050 Gerneral.ini:16 has 0. HomeLog is also gated (uhome.cpp:848, 869, 3901, 4337); only the declaration cpublic.h:40 is live, so no Home_Home.logs file is written.
6. AOITask, Auto_TeachPitch and ConntectionOkTask have no port variable (0 live hits each). Golden has them at fAOI.cpp:55, uteach.cpp:43 and BarCode/BarCode.cpp:42. Their registrations are gated at cStateRecord.cpp:1920-1922, 1890-1892 and 1935-1937, so these rows are empty in the port's Task_ListWithTime. They are idle-only in the reference, so this is low priority.

<details><summary>agent 摘要（英文）</summary>

I checked 88 rows. Registrations: the golden TfMain::FormShow list (main.cpp:9717-10036) and the port's W906_BootRegisterTaskList (cStateRecord.cpp:1603+) point every row at the same variable. The one exception is CarlibrationTask, which is deliberately registered to fContactForm (cStateRecord.cpp:1578-1580). Four rows are registered inside #if 0 in the port, and I confirmed each gate's reason still holds. Three of them (AOITask, Auto_TeachPitch, ConntectionOkTask) have no port variable at all; they are idle-only in the reference. The fourth is SetStepMotorTask, a real multi-step task: the whole golden TdmTrayMotor (Motor/TrayStepMotor.cpp) is missing from the port, so all 12 of its observed steps are MISSING. The reference machine has LoaderUnload_StepMotor=1, and so does the local D:/HT9045/system/Gerneral.ini:585, so HT9050 needs this task. The other 8 multi-step rows are fully live: HomeStep (39 steps), InitialStartTask, InitialICCheckTask, InArmTask, InArmTryPickFromHotPlateTask, OutArmTask, AutoSHT1Task and AutoSHT2Task. For each I compared the case table and every literal Task= transition between golden and port with a script; they match 1:1. A statement-level comparison of every observed case body (with #if 0 lines removed) gives ratio 1.000 for the InArm, TryPick, OutArm, SHT1/2, InitialStart cases 50-200 and ICCheck cases 2100-2400. The only differences are the documented gates and seams listed per step. Reachability is live throughout. MainProc (csystem.cpp:30196) reaches ProcessMotorHome through the iHome==1 arm (:31556), DoHomeProcess (:31587, :7359). It reaches DoInitialStart through CheckContinusStartIsReady (:32435, :9522). The rest goes through DoAllProcess (:32474), which calls DoInitialICCheck via W906_DoAllProcessLadder (:1854, :33313), plus DoInArm (:1887), Do_Auto_SHT1/2 (:1903/:1908) and DoOutArm (:1939). All of this sits downstream of SystemStart. The arm variant was chosen from the reference recipe (HandlerCondition.Data: Test Mode=Square 4-Site(2X2), Use Suck Mode=4), which gives iInArmType e9045_2x2_4_13 or _14 (golden ainarm9045.cpp:1858-1970) and OutArm DoOutArm_9045_2x2_4. I checked both InArm variants. No golden transition between two observed steps is missing from the port. Five guard gates on these paths no longer have a valid reason and should be removed: DoOutArm SetHasNullIcToNullIc (aoutarm.cpp:1800-1803); DoInArm_9045 Gali ST, bMyServoOffInArm and bEject (ainarm9045.cpp:898-900, 906-911, 953-958); and DoOutArm_9045 bMyServoOffOutArm (aoutarm9045.cpp:761-766). DoInitialICCheck also has two no-op seams: W906G4_FYIELD_CLEARAUTOSITEOFF, whose target now exists, and W7C1_FLTCSENSOR_CLEAR; the TfLtcSensor behind the latter is itself an offline shim. In the default SOFT_SIMULTE build, HomeStep 100/200/250/270/280 are skipped (case 20 jumps to 300) exactly as in golden. Those steps are live only in the ship build (-DW906_NO_SOFT_SIMULTE=ON), which is the build the reference machine corresponds to.

</details>


## Index（下壓）／AutoClean（12 個 task 有走流程、22 個只有閒置值）

| task | 對照檔走過的步驟數 | 活的 | 被閘住 | 空殼 | 缺 | 走不到 | 狀態 |
|---|---|---|---|---|---|---|---|
| `TestHeadMotorTask` | 27 | 26 | 1 | 0 | 0 | 0 | ⚠ 不通的步驟：21:GATED |
| `AutoCleanIndexTask` | 25 | 22 | 3 | 0 | 0 | 0 | ⚠ 不通的步驟：1:GATED 2200:GATED 3000:GATED |
| `AutoCleanTask` | 18 | 5 | 5 | 0 | 0 | 8 | ⚠ 不通的步驟：20:GATED 2400:GATED 2500:UNREACHABLE 2510:UNREACHABLE 2515:UNREACHABLE 2540:GATED 2600:UNREACHABLE 2800:GATED 3000:UNREACHABLE 3050:UNREACHABLE 3100:UNREACHABLE 3200:UNREACHABLE 2000:GATED |
| `AutoCleanPlaceToShuttleTask` | 10 | 1 | 3 | 0 | 0 | 6 | ⚠ 不通的步驟：100:GATED 2000:GATED 2100:UNREACHABLE 2150:UNREACHABLE 2160:UNREACHABLE 2170:GATED 2200:UNREACHABLE 2300:UNREACHABLE 2400:UNREACHABLE |
| `AutoCleanPickFromCleanKitStageTask` | 9 | 9 | 0 | 0 | 0 | 0 | ✅  |
| `AutoCleanPlaceToCleanKitTask` | 6 | 6 | 0 | 0 | 0 | 0 | ✅  |
| `AutoCleanPickFromShuttleTask` | 5 | 1 | 1 | 0 | 0 | 3 | ⚠ 不通的步驟：10:GATED 50:UNREACHABLE 200:UNREACHABLE 400:UNREACHABLE |
| `AutoCleanShuttle1Task` | 4 | 4 | 0 | 0 | 0 | 0 | ✅  |
| `AutoCleanShuttle2Task` | 4 | 4 | 0 | 0 | 0 | 0 | ✅  |
| `IndexStatus` | 2 | 2 | 0 | 0 | 0 | 0 | ✅  |
| `AllPassVerifyTask` | 1 | 0 | 0 | 0 | 1 | 0 | ⛔ 不通的步驟：1:MISSING |
| `TestZTask` | 1 | 0 | 0 | 0 | 1 | 0 | ⛔ 不通的步驟：0:MISSING |

**這一組最重要的缺口**（agent 原文，英文）：

1. Remove the six stale GATE (W7d-I1) 'if(false)' substitutions for MoveInArmXYToShuttle_9045 in AutoClean.cpp: :4524 (DoPlaceToShuttle 100->2000), :4598 (2170->2100), :4790 (DoPickFromShuttle 10->50), :8720 (DoAutoCleanKit 540->600), :8982 (1300->2600/2100) and :9385 (2540->2600). Golden: AutoClean.cpp:3583/3656/3939/4895/5156/5551. The gate's reason (InArmOffSet NULL) no longer holds: wb_serve allocates InArmOffSet at tools/wb_serve.cpp:3502 and :4010-4013, and the real body is at ainarm9045.cpp:3929. Without this, the port's AutoClean parks at AutoCleanTask=2400 / PlaceToShuttle=100.
2. Translate InArmZNeedDown_9045 (empty stub at ainarm9045.cpp:3436; golden ainarm9045.cpp:613-712). All 22 per-mode functions it dispatches to already exist live. Without it NeedDestroy/iNeedSuck are never set, so AutoClean place and pick move no Z and hand no pad over (PlaceToShuttle 2300 falls back to 100). Every InArm CheckXYPitch_* path in other groups also calls it.
3. Consequence to verify after both fixes: on the reference path (Initial-Start AutoClean, csystem.cpp:32086-32104) bRunAutoClean is never cleared today. The ladder (csystem.cpp:33389-33499) then returns every tick, so DoTestHeadMotor (csystem.cpp:1921) and AutoCleanIndexTask beyond 2300 never run.
4. EPSwitchOnOff is a no-op stub at atester_shims.cpp:333 and is used in DoTestHeadMotor case 21 (atester.cpp:6660). Golden adam6024.cpp:3016-3090 (EPSwitchOnOff and EpSwitch) drives the SW[SwEpArm1/2] and SW[SwIndEpArm1/2] outputs, which exist in the port.
5. GATE (W7a-I4) at AutoClean.cpp:919-987 disables the CleanSetSpeed motor-speed push (golden :770-815), so AutoClean speeds never reach the drives (AutoCleanTask 20 and 2000). The retirement condition has already been met: W5a-G was OPENED on 20260920 (cinitial.cpp:3833).
6. W906DIAC_InitDoTestZHome is a no-op stand-in at AutoClean.cpp:278, used in DoIndexAutoClean cases 1, 2200 and 3000 (:6575/:7441/:7814). Translate golden uhome.cpp:4891-4906 (MOT[MTestY1/Y2/Z1/Z2].MovFlag/bScanFlag/GaliSofDelayCount reset). Adding the TfHome::TestZTask member would also let the TestZTask row (cStateRecord.cpp:1887-1889) be registered.
7. The fContactCT->ACSmartClearData() call dropped from DoAutoCleanKit 2000 (AutoClean.cpp:9062-9068; golden :5239) can be restored: fContactCT exists at cContactCT.cpp:99, with the method at :1527.
8. The DoTestHeadMotor preamble check (golden atester.cpp:5609-5621, CheckIndexConnect/WAR0360 for USE_16_HEATER) is still under TODO(W7) at atester.cpp:6101, although CheckIndexConnect is live at csystem.cpp:21807.
9. CheckInArmDestroyActive is an empty stub at ainarm9045.cpp:3433 (golden ainarm9045.cpp:1169-1256). It is called at the AutoClean-branch entry in csystem.cpp:33396.
10. The AllPassVerifyTask row is not registered, because DoAllPassVerifyRTC is still in #if 0 (atester.cpp:9148-9278; the live stub at :9280 returns false). It is RTC-only, so low priority for 9050. ADAM_WriteVoltage is also a no-op (atester_shims.cpp:327) in DoIndexAutoClean 2300 (:7448); that matters only if EP_Install is set.

<details><summary>agent 摘要（英文）</summary>

Scope: 35 rows (10 multi-step, 25 idle). How I checked: for every state machine I compared the golden and port function bodies line by line with comments and whitespace removed, compared the list of 'case N:' labels and Task= writes (they match in all 9 AutoClean machines and in DoTestHeadMotor), checked whether each called function has a real body in the port, and walked callers up to MainProc. The call chain is live: MainProc (csystem.cpp:30196) -> DoAllProcess (csystem.cpp:1837) -> W906_DoAllProcessLadder (called at :1854, defined :32945) -> AutoClean branch (csystem.cpp:33389-33499, which matches golden csystem.cpp:9535-9695). The Initial-Start trigger is at csystem.cpp:32086-32104 (golden 18366/18381). DoTestHeadMotor is called at csystem.cpp:1921. The port registers every row to the same variable as golden (checked cStateRecord.cpp:1666-1888 against golden main.cpp:9724-9936). Two rows are not registered at all: AllPassVerifyTask and TestZTask.

How I assigned each step's status: a step's own defect comes first (GATED means the step's own exit transition, or a machine action inside that step, is replaced by if(false), #if 0 or a no-op stub). Next comes UNREACHABLE, meaning the step's code is live but its entry on the reference path runs through a gated step of the same state machine. Everything else is LIVE. When one state machine is blocked because a different one never finishes, I note it but do not change the step status.

Main finding: in the port, the reference AutoClean path cannot complete. Three things block it.
(1) GATE (W7d-I1) replaces the golden call MoveInArmXYToShuttle_9045 with 'false' at AutoClean.cpp:4524 (DoPlaceToShuttle case 100), 4598 (case 2170), 4790 (DoPickFromShuttle case 10), 8720 (DoAutoCleanKit case 540), 8982 (case 1300) and 9385 (case 2540). The gate's stated reason was that InArmOffSet[] is NULL. That no longer holds: wb_serve allocates it (tools/wb_serve.cpp:3502 calls EnsureArmOffsetObjects at cOffSet.cpp:166-172, and it is allocated again at :4010-4013). The real body is live at ainarm9045.cpp:3929 and other code already calls it, e.g. ainarm9045S_1x4_4.cpp:1003.
(2) InArmZNeedDown_9045 is an empty stub at ainarm9045.cpp:3436 (golden ainarm9045.cpp:613-712), even though all 22 per-mode functions it dispatches to have live bodies in the port. Without it the per-nozzle NeedDestroy / iNeedSuck flags are never set, so the Z moves and pad hand-over do nothing.
(3) The result: the port's AutoCleanTask parks at 2400 while AutoCleanPlaceToShuttleTask parks at 100. AutoCleanIndexTask then waits forever at 2300 because no pad ever reaches shuttle 2. bRunAutoClean is never cleared, so on this reference path (Initial-Start AutoClean) DoTestHeadMotor never starts.

Code that is intact: AutoCleanPickFromCleanKitStageTask, AutoCleanPlaceToCleanKitTask, AutoCleanShuttle1Task and AutoCleanShuttle2Task are faithful and live. DoIndexAutoClean and DoTestHeadMotor match golden except for the no-op stand-ins listed below. IndexStatus 0->2 is live (written at atester.cpp:6863). The case lists match exactly, so every golden transition between two observed steps is present in the port as code; it is missing only where a gate or stub sits on it.

Transitions the port cannot take:
- AutoCleanTask: 2400->2200 and 2400->2500 (DoPlaceToShuttle never returns true), 2540->2600 (if(false)), 2800->3000 (DoPickFromShuttle never returns true).
- AutoCleanPlaceToShuttleTask: 100->2000 and 2170->2100 (if(false)), and 2300->2400 (would still fall back to 100 because of the InArmZNeedDown stub).
- AutoCleanPickFromShuttleTask: 10->50.

Actions dropped inside steps that are otherwise live:
- CleanSetSpeed motor-speed push is under #if 0 GATE (W7a-I4) at AutoClean.cpp:919-987. Its reason is gone: W5a-G was opened on 20260920 (cinitial.cpp:3833).
- InitDoTestZHome is a no-op stand-in at AutoClean.cpp:278, used in DoIndexAutoClean cases 1, 2200 and 3000.
- EPSwitchOnOff is a no-op at atester_shims.cpp:333, used in DoTestHeadMotor case 21. Golden drives the SW[SwEpArm*] outputs.
- ACSmartClearData was dropped from DoAutoCleanKit case 2000; fContactCT exists at cContactCT.cpp:99.
- The CheckIndexConnect check in the DoTestHeadMotor preamble is gated at atester.cpp:6101 (TODO(W7)); CheckIndexConnect is live at csystem.cpp:21807.

TestHeadMotor 12110/12200 exist only in non-SOFT_SIMULTE (ship) builds. Golden has the same #ifdef; under SOFT_SIMULTE, 12101 goes straight to 12300.

Scratch notes: C:/Users/JIMMYC~1/AppData/Local/Temp/claude/D--HT9045/cba1f203-f3aa-4295-ba14-963147c110e7/scratchpad/cov/g2_index_autoclean/notes.md

</details>


## 托盤／Loader／升降／Auto／Bin（15 個 task 有走流程、73 個只有閒置值）

| task | 對照檔走過的步驟數 | 活的 | 被閘住 | 空殼 | 缺 | 走不到 | 狀態 |
|---|---|---|---|---|---|---|---|
| `SupplyNewIC_From_LoaderCar` | 16 | 16 | 0 | 0 | 0 | 0 | ✅  |
| `CatchTrayTask` | 8 | 8 | 0 | 0 | 0 | 0 | ✅  |
| `AutoColorTask` | 8 | 8 | 0 | 0 | 0 | 0 | ✅  |
| `LoadNewColorTrayToCarTask` | 8 | 8 | 0 | 0 | 0 | 0 | ✅  |
| `TrayArmCatchNewTrayFromBufferTask` | 7 | 7 | 0 | 0 | 0 | 0 | ✅  |
| `ColorTrayToRearTask` | 7 | 7 | 0 | 0 | 0 | 0 | ✅  |
| `LoadTask` | 6 | 6 | 0 | 0 | 0 | 0 | ✅  |
| `BinTrayTask[0]` | 6 | 6 | 0 | 0 | 0 | 0 | ✅  |
| `BinTrayTask[1]` | 4 | 4 | 0 | 0 | 0 | 0 | ✅  |
| `BinTrayTask[2]` | 4 | 4 | 0 | 0 | 0 | 0 | ✅  |
| `TrayZLoadTrayToWaitTask` | 3 | 3 | 0 | 0 | 0 | 0 | ✅  |
| `iLifterTask[0][0]` | 3 | 3 | 0 | 0 | 0 | 0 | ✅  |
| `iLifterTask[0][2]` | 3 | 3 | 0 | 0 | 0 | 0 | ✅  |
| `iLifterTask[1][5]` | 3 | 3 | 0 | 0 | 0 | 0 | ✅  |
| `LoadNewICTrayTask` | 2 | 2 | 0 | 0 | 0 | 0 | ✅  |

**這一組最重要的缺口**（agent 原文，英文）：

1. Stale Tech stand-in in asendic_Loader.cpp:295-301: W7L1L_Tech has iMLoaderYSurePos=0, and `#define Tech W7L1L_Tech` hides the real TECH Tech (LastSet.cpp:40 / LastSet.h:1158, loaded by forms/fTeachRegistry.cpp:701). Effect: in DoSupplyNewICTray case 150's wait branch (port :819, golden asendic_Loader.cpp:548) and in case 160 (port :877, golden :606), the call is TrayMoveIn(true,0,0) instead of using the taught Loader-Y position. When OCR+bTrayOCR or INSTALL_OCR_YMot is on, TrayMoveIn (asendic.cpp:1419-1425) drives MLoaderY toward 0. The reference sat in case 150 for 38 s (17:36:26.561 -> 17:37:04.468), so it ran this line repeatedly. Fix: delete the stand-in struct and the #define.
2. Stale continue-start stand-ins in asendic_Auto.cpp:399-400: `#define iAutoTrayData/bOldAutoHasTray W7L1A_*` points at file-local copies that are always false/zero. DoReceiveAutoTray (csystem.cpp:11483-11487) writes the real globals (csystem.cpp:9767 / acatchtray_shims.cpp:211), but BinTrayTask case 1 (port asendic_Auto.cpp:576-584 = golden asendic_Auto.cpp:214-222) reads the dummies. So on Continue Start / Continue Retest the Auto tray map is never restored and the tray is not re-marked fHasTray.
3. Stale output stand-ins in asendic_Auto.cpp:393-395 and :403-404: SaveUnloaderInfo, SaveProductionRecord, SaveTrayRecord, CheckAllAutoTrayEjectFinsh and bNeedEject are no-ops, but real bodies now exist (cinitial.cpp:5533, SortingBinTray/SortingBinTray.cpp:2699/:2788, csystem.cpp:25241, cmydef.cpp:6285). Effect: BinTrayTask case 850 (port :971/:981 = golden :609/:619) never writes the per-unloader-tray production log or the 10-tray summary; the eject roll-up is dead.
4. Stale stand-ins in asendic_Loader.cpp:272/276-277/280 (#defines :302/:306-308): GetColorSensorIsMapping always returns true (real: cprod.cpp:3106), so WAR09200 is never raised in DoLoadNewICTray case 1 (port :2385-2400) or DoTrayZLoadTrayToWait (:2242) when the MU-N color sensor is enabled. InitOCRFlow/CleanOCRData are no-ops (real: OCRInsp.cpp:777/:1929); MyDeCodeASCII is a table copy (real: EJ1N/TextProcess.cpp:155). Not stale: LogIndexMaxMinPos and GetBundleInfo, whose real bodies are still inside #if 0 (cpublic.cpp:1816, :2336).
5. Version difference in ColorTrayToRearTask (reference V3.33.880.3 vs golden 906): the reference went 450->460 in 2 ms and 460->500 in 200 ms. In golden 906 (and the port, which is identical), 460 is reached only through 455, and only when fAGV->IsATK_AMR() (golden asendic_Color.cpp:566-609 / port :682-727; IsATK_AMR = CC_AMKOR_Korea + NFC CID + A65, Automation/AGV_predicates.cpp:37-45). Otherwise the path is 450->480->500. A dynamic START comparison on a non-ATK 9050 will show 480 where the reference shows 460. This is not a port defect.
6. DoCatchTray has a case 245 that is not in golden 906 (port acatchtray.cpp:6750-6793, AI(W906-TIF912), from golden 912). It only runs when TestIF_File.bPreventDropfunction is set and InArm holds a real IC. It is the only place the port's tray/loader state machines depart from golden 906; it does not affect any observed step.
7. Runtime prerequisites (not verified by running): every chain here depends on SystemStart && fAllMotorHome && !SoftStop from the web START path, and on the golden early-return ladder W906_DoAllProcessLadder (csystem.cpp:1851-1858). The observed branches need these config values to match the reference: LOAD_Z_USE_MOTOR[0] and [2] (lifter 1000/1100 motor path, asendic.cpp:311-315 / :446-450), AUTO_EMPTY_COLOR!=0 (DoAutoColor, csystem.cpp:2012), TrayForm.LodareType!=0 with AutoFromEmptyColor set (CatchTray 400->600, acatchtray.cpp:6935-6939). Align them in the 9050 Gerneral.ini/IniData before using the reference as a dynamic check.

<details><summary>agent 摘要（英文）</summary>

All 88 rows are registered in the port to the same variable as in golden. I compared golden main.cpp:9720-10000 (TfMain::FormShow) against the port's cStateRecord.cpp:1669-1969 (W906_BootRegisterTaskList). The iLifterTask/iAutoTask rows use W906_GOLDEN_STRIDE7 (cStateRecord.cpp:1612), which copies golden's `extern int iLifterTask[3][7]` / `iAutoTask[3][7]` addressing (golden main.cpp:9136-9137). So row "iLifterTask[1][5]" is storage slot 12 = definition [1][2] = CylinderMiddle(C_Color_Up); its timing matches LoadNewColorTrayToCar case 200 (17:36:25.542 vs .543). The 15 multi-step rows are driven by 11 state machines. I compared each one's code body against golden with a normalised diff: 10 have zero diff (DoSupplyNewICTray, DoLoad, DoLoadNewICTray, DoTrayZLoadTrayToWait, DoAutoColor, DoLoadNewColorTrayToCar, DoColorTrayToRear, DoAutoReceiveBinTray, CylinderUp/CylinderMiddle, CatchNewTrayFromBuffer). DoCatchTray differs only by an added case 245 (a golden-912 Prevent-Drop feature, AI(W906-TIF912)). All function headers are at preprocessor top level, and no observed case body is inside #if 0 / GATE. Every observed step value has a case in both trees (port and golden case lines are cited per step). Every observed transition between steps has the matching golden `Task=` line in the port. The call chain is live throughout: PumpTick (WebBridgeTags.cpp:632) -> MainProc (csystem.cpp:30196) -> DoAllProcess at csystem.cpp:32474 (the definition at :1837; the copy at :695 is inside #if 0 :694) -> DoLoad :1871/:1875, DoCatchTray :1930, DoAutoReceiveBinTray :1994/:1998, DoAutoColor :2024/:2029. This matches golden csystem.cpp:18728 and 10059/10063/10116/10174/10178/10204/10209. The 73 IDLE_ONLY rows all have a port definition initialised to 1 (or initLifterTask/initAutoTask, now called at boot from cStateRecord.cpp:1608 as golden main.cpp:9159-9160 does). The reset to 1 that the reference shows at 17:47:30.133 on several rows at once (LoadTask, CatchTrayTask, BinTrayTask[*]) is golden InitAllProcessTask (csystem.cpp:5698-5736). The port copy (csystem.cpp:286) is live through CheckContinusStartIsReady -> DoInitialStart (csystem.cpp:9522/:11070). Result: all observed steps are LIVE; no row is GATED, STUB, MISSING or UNREACHABLE. The real gaps are in data, not steps. Several file-local #define stand-ins in asendic_Loader.cpp and asendic_Auto.cpp are now stale: the real symbols exist and are compiled, but these files still point at dummy copies (details in top_gaps). One version caveat: the reference machine ran V3.33.880.3 (reference HT9045/system/Gerneral.ini [Version] Ver=V3.33.880.3), not 906. Its ColorTrayToRear path 450->460->500 is not a direct path in golden 906, and the port copies golden 906 exactly. Reachability here was traced through the code, not run. The observed branches also depend on configuration: LOAD_Z_USE_MOTOR[0]/[2] for the lifter 1000/1100 motor path, AUTO_EMPTY_COLOR!=0 for DoAutoColor, TrayForm.LodareType/AutoFromEmptyColor for CatchTray 400->600, and INSTALL_OCR_YMot for the Tech stand-in. These values must be aligned to HT9050 in Gerneral.ini / IniData to reproduce the reference sequence. Notes: C:/Users/JIMMYC~1/AppData/Local/Temp/claude/D--HT9045/cba1f203-f3aa-4295-ba14-963147c110e7/scratchpad/cov/g3_tray_loader/notes.md

</details>

