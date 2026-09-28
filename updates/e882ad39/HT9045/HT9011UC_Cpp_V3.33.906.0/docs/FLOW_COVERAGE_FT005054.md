# 動作流程功能清單：對照檔 FT005054 的每一個 task 步驟，C++ 有沒有（20260928）

> 使用者 0928 10:4x（RULINGS_20260928 第 2 條）：對照檔（`D:\HT9045\Staterecord\2025-12-11 17_47_57`，BCB6 在真機上的紀錄）當「功能清單」——
> 真機走過的每一個 task 步驟，C++ 都要有、而且是活的。這份就是量尺。
>
> **怎麼量的**：`Task_ListWithTime.csv`／`Task_ListWithTime2.csv` 裡 210 個有值的 task、463 組（task, 步驟值）；3 個唯讀 agent 逐一查移植樹那個 case 在不在、
> 是不是活的（沒被 `#if 0`／GATE 擋、不是空殼、從 tick 迴圈呼叫得到），每一條都附 golden 與移植樹的行號（workflow `wf_5a7678ce-487`，0928 10:49～11:40）。
> **0928 16:4x 重量**：FLOW-2／FLOW-3 推上去之後，HOME／初始化／手臂組與 Index／AutoClean 組在 `d5994b52` 上**每一步從頭重驗**（workflow `wf_d1e05cd7-ce7`，15:52～16:43，2 個唯讀 agent）；
> 托盤／Loader 組上次就全部是活的，沒有重跑（FLOW-2 動到的托盤替身只影響資料，不影響步驟）。
> 只有閒置值（通常是 1）的 task 算「活的」，條件是變數存在、有登錄、重設的地方存在。
>
> **狀態**：活的＝LIVE；被閘住＝GATED（case 在，但它的出口或裡面的動作被 `#if 0`／no-op 擋住）；空殼＝STUB；缺＝MISSING；走不到＝UNREACHABLE（程式是活的，但進入它的前一步被閘住）。
>
> ⚠ 這是**靜態**量測（讀程式），還沒實跑；實跑對照用 `tools/flowcmp`（S1 段）。
> ⚠ 對照檔那台是 HT9046_LS（`LoaderUnload_StepMotor=1`、`REAL_TIME_CCD=1`）；HT9050 機台設定（0928 裝進筆電）這兩項是 0，所以 `SetStepMotorTask` 與 RTC 那幾步在 9050 上本來就不會走。

## 總表

| 狀態 | 步驟數 |
|---|---|
| LIVE | 275 |
| GATED | 1 |
| STUB | 0 |
| MISSING | 16 |
| UNREACHABLE | 0 |
| UNKNOWN | 0 |

**一句話（0928 16:4x 重量）**：對照檔走過的步驟，除了 HT9050 本來就不會走的 `SetStepMotorTask`（12 步，`LoaderUnload_StepMotor=0`）、`AllPassVerifyTask`（RTC 專用，`REAL_TIME_CCD=0`）與 3 個只有閒置值、移植樹沒有變數的 task（AOI／Auto_TeachPitch／ConntectionOk），**全部是活的**；AutoClean 從頭到尾跑得完（FLOW-2 補上之後，被閘住 13→1、走不到 17→0）。唯一被閘住的一步是 `AutoCleanIndexTask=2300` 的 EP 壓力寫入（`ADAM_WriteVoltage` 空殼，HT9050 `EP_Install=3`）。

**步驟活著不等於動作到位（靜態量到、HT9050 要注意的）**：

| # | 事項 | 影響 | 處理 |
|---|---|---|---|
| 1 | **Index Z 在 HT9050 是 1203 軸，golden 的 Index 流程一律呼叫 `Gali_*`**（AutoClean 42 處、atester 77、Front 66、Rear 76、32Site 40、uhome 11、csystem 11），而 Galil 卡從來沒開（`Open_GaliCard` 沒有呼叫端） | 步驟照走，但 **Index Z 不會真的下壓**（走 golden「沒裝卡」的模擬分支） | 使用者 0928 17:0x：改成 1203 控制、不動動作流程 task ⇒ 在 `Gali_*` 這一層依卡別分流（第 27 條 A），筆電做 |
| 2 | EP 壓力寫入 `ADAM_WriteVoltage` 是空殼（`EP_Install=3`） | AutoClean 與測試時的下壓氣壓不會照配方設定 | FLOW-5 查相依中 |
| 3 | 警報後的「手臂伺服斷電保護」守衛是活的，但**沒有人把旗標設成 true**（golden 寫入點沒翻） | JAM 之後 golden 會擋手臂動作直到伺服恢復，移植樹不擋 | FLOW-5 查中 |
| 4 | `SearchCleanNum` 只翻了一半（少了 WAR16313「AutoClean 次數到達上限」警報） | 對照檔沒觸發，不影響這條路徑 | FLOW-5 補 |

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

1. SetStepMotorTask is MISSING for all 12 observed steps (1,100,1000,1100,2000,2100,3000,3100,4100,5100,6000,6100). Golden TdmTrayMotor (Motor/TrayStepMotor.cpp: DoTrayStepMotor :109-395, switch :134, StartSetSpeed :89-100, timer :102) is not ported: dmTrayMotor and TdmTrayMotor have 0 live hits. Four gates depend on it: the registration (cStateRecord.cpp:1787-1789), HomeStep case 1 StartSetSpeed (uhome.cpp:997-999, golden :1335), cSpeed.cpp:890+ GATE (S5) and WebRecipeChange.cpp:365. The reference had LoaderUnload_StepMotor=1 (Gerneral.ini:477). The local HT9050 D:/HT9045/system/Gerneral.ini:582 now reads 0, so confirm whether HT9050 needs it before porting.
2. Alarm servo-off protection is live but inert. The guards now run: DoInArm_9045 ainarm9045.cpp:906-911 and DoOutArm_9045 aoutarm9045.cpp:761-766. But no port code ever sets fNote->bMyServoOffInArm or bMyServoOffOutArm to true (0 live writers). The golden writers are untranslated: main.cpp:7168/7182/7201/7215 (ResetServoOff), mymessbox.cpp:846/1185 and note.cpp:2053/2074. Stale '#if 0 GATE G7' copies with the false reason 'TfNote carries bMyServoOffInArm only' (fNote.h:305 has the OutArm member) are still in: aoutarm.cpp:914 (MoveOutArmXY_ToFix_Tray_Full, the helper of OutArm case 300), :992, :1039 and :1100; RotateKit/aRotateKIT_Out.cpp:466/1318; asortarm.cpp:919; and aoutarm9045.cpp:2174. This is safety-relevant: after a JAM with bAlarmNeedServoOff (CosFunction.cpp:4135 sets it true), golden blocks arm motion until servo is restored.
3. HomeStep RTC CCD handshake is gated. Case 650 does not wait for the RTC home ack and does not raise WAR0338 (uhome.cpp:3193-3208, golden :3294-3307). Case 600 does not send rtHome (:3141-3146), and case 1 does not send TIMESYNC (:1023-1026). The reason still holds: TCOM2Shim (atester_shims.h:373-462) has no bRealTimeCom_ReceiveOK, rtHome, OpenRTCComPortAgain or SendCommToVision. The reference machine had REAL_TIME_CCD=1 (Gerneral.ini:16), so its step 650 waited on the RTC. The local HT9050 has 0.
4. The latch-sensor module (golden LtcSensor.cpp) is not ported. TfLtcSensor is an offline shim (acarry_shims.cpp:42-60): GetLtcSensor returns 0 and SetLtcSensor is empty. Consequences: the InitialICCheck latch clear in cases 2000/10000 is the no-op seam W7C1_FLTCSENSOR_CLEAR (csystem.cpp:6630-6631/6811-6812, golden :8538-8539/:8719-8720). AutoSHT1/2 case 120 latch reads (acarry.cpp:3923/5779, golden :3858/:5714) capture nothing. The steps still run; only Y-latch based detection is affected (reference iF07OutShuttleSensorMode=0, ENABLE_OUT_SHUTTLEY_LATCH=1).
5. Stale stand-ins in InitialStartTask case 1. GATE G-W5cI-2 (csystem.cpp:10425-10427) still skips ResetSCK_OEECount(), although the function is now live at csystem.cpp:28609 (golden csystem.cpp:23376; golden call :6137). The TU-local W7C1_THPPlaceLogSeam (csystem.cpp:2569-2571) turns HPPlaceLog.InitialPosition() at :10424 (golden :6136) into a no-op, although the real TMyHotPlatePlaceLog HPPlaceLog exists (ainarm_SearchPlacePlate.cpp:76, :599).
6. HomeStep peripheral sub-gates that matter on the reference configuration. ROTATESPEED: FrmRotate->SetIn/OutRotateSpeed(70,100) after home is gated at uhome.cpp:3693-3696 (golden :3774-3775; reference USE_ROTATE_KIT=1). GPIBADAM: Open/Close_ADAM_6024 is still absent, so golden's 'ADAM connect error -> refuse home' interlock is if(false&&...) at uhome.cpp:1505-1510. Two empty stubs are also called on home steps. TMyMotor::TrayArmInitial (Motor/mymotor.cpp:1927; golden Motor/mymotor.cpp:5656-5660 sets iTrayXTask=1) runs in case 300; it has no effect with TRAY_ARM_MODE=0, but an eUnderCoveyor ship build would never finish TrayArmMotorMove. SetAutoSkipCount (ainarm9045.cpp:3424; golden :2408) runs in case 1600.
7. The previous run's arm-variant identification was wrong and is corrected here. The reference machine has USE_PICKER_COUNT=4 = ep1Picker, so it ran DoInArm_9045_All_1Pick (port ainarm9045_All_1Pick.cpp:1892) and DoOutArm_9045_All_1Picker (aoutarm9045_All_1Picker.cpp:747). Both match golden. Golden 906 All_1Pick TryPick goes 350->360->400 (golden :1617/:1668). The reference never sampled 360, which fits either a one-tick state or a V3.33.880.3 version difference; the port equals golden 906.
8. Low priority, idle-only in the reference: iAOITask (golden fAOI.cpp:55), iAuto_TeachPitch (golden uteach.cpp:43) and iConntectionOkTask (golden BarCode/BarCode.cpp:42) have no port variable, so their rows stay empty in the port Task_ListWithTime.

<details><summary>agent 摘要（英文）</summary>

I re-measured all 88 rows at worktree HEAD d5994b52 and re-verified every non-idle step from scratch. For each state machine I extracted the case/transition table from both trees and diffed them, compared each observed case body statement by statement with #if 0 lines removed, listed the dead preprocessor regions, resolved each called function to its live port definition to catch stubs, and walked the callers up to a live driver. Registration: every row points at the same variable as golden TfMain::FormShow (main.cpp:9717-10036). The one deliberate exception is CarlibrationTask, which goes to fContactForm. Four registrations are still #if 0: SetStepMotorTask (cStateRecord.cpp:1787-1789), AOITask (:1920-1922), Auto_TeachPitch (:1890-1892) and ConntectionOkTask (:1935-1937). None of their variables exists live in the port. Result: 76 rows are IDLE_ONLY and 8 multi-step rows are ALL_LIVE. The 4 NONE_LIVE rows are SetStepMotorTask (12 steps MISSING) and the three absent idle-only variables. The ALL_LIVE rows are HomeStep (39 steps), InitialStartTask (7), InitialICCheckTask (10), InArmTask (2), InArmTryPickFromHotPlateTask (5), OutArmTask (4), AutoSHT1Task (4) and AutoSHT2Task (6). In every case table the port and golden have the same cases and the same literal Task= transitions, so no golden transition between observed steps is missing. Reachability: PumpTick (WebBridgeTags.cpp:632) calls MainProc (csystem.cpp:30196). From MainProc, DoHomeProcess (:31587, then :7359) reaches ProcessMotorHome, and CheckContinusStartIsReady (:32435, then :9522) reaches DoInitialStart. DoAllProcess (:32474) reaches W906_DoAllProcessLadder, then DoInitialICCheck (:1854, then :33313). DoAllProcess also calls DoInArm (:1887), Do_Auto_SHT1/2 (:1903/:1908) and DoOutArm (:1939). One correction to the previous run: the reference machine has USE_PICKER_COUNT=4 (Gerneral.ini:71), and 4 is ep1Picker (golden MachineType.h:1308). Golden's first ladder rung therefore sends the arms to DoInArm_9045_All_1Pick (ainarm9045.cpp:4535-4537) and DoOutArm_9045_All_1Picker (aoutarm9045.cpp:539-542), not to the 2x2_4 variants the previous run analysed. Both All_1Pick machines are identical to golden in the port (tables, and bodies at ratio 1.000); the 2x2_4_13/_14 variants are also identical. What changed since the previous run: (1) The DoInArm_9045 guard gates (Gali ST, bMyServoOffInArm, bEject), the DoOutArm_9045 bMyServoOffOutArm gate and the DoOutArm SetHasNullIcToNullIc gate (G10) are retired. (2) The previous run called OutArm LIVE even though its steps ran on stand-ins: MoveOutArmToAutoSafe_9045 and CheckOutArmInitState_9045 returned true, and CheckOutArmDestroyActive was an empty body. Those are now the golden bodies. InArm's CheckInArmDestroyActive was an empty stub and is now golden. (3) DoInitialICCheck case 1 is now verbatim golden, including ClearAutoSiteOffStatus. (4) DoInitialStart's yield gates G11 and G12 are lifted. Still present, and none of them blocks a step: the ResetSCK_OEECount gate and the HPPlaceLog seam in InitialStart case 1 are stale. The latch-sensor clear and read are no-ops because TfLtcSensor is an offline shim; this affects InitialICCheck 2000/10000 and AutoSHT1/2 case 120. HomeStep keeps gates on peripheral actions whose reasons still hold after re-measurement: the RTC CCD handshake, the ADAM-6024 reconnect interlock, rotate-kit speed and tray-stepper StartSetSpeed. MoveOutArmXY_ToFix_Tray_Full still has the stale GATE G7. The local HT9050 Gerneral.ini now reads LoaderUnload_StepMotor=0, REAL_TIME_CCD=0 and USE_ROTATE_KIT=0; the reference had 1/1/1. The g2 AutoClean question is outside this group. Scripts and intermediate tables are in C:/Users/JIMMYC~1/AppData/Local/Temp/claude/D--HT9045/cba1f203-f3aa-4295-ba14-963147c110e7/scratchpad/cov/rerun_g1_home_init_arms/.

</details>


## Index（下壓）／AutoClean（12 個 task 有走流程、22 個只有閒置值）

| task | 對照檔走過的步驟數 | 活的 | 被閘住 | 空殼 | 缺 | 走不到 | 狀態 |
|---|---|---|---|---|---|---|---|
| `TestHeadMotorTask` | 27 | 27 | 0 | 0 | 0 | 0 | ✅  |
| `AutoCleanIndexTask` | 25 | 24 | 1 | 0 | 0 | 0 | ⚠ 不通的步驟：2300:GATED |
| `AutoCleanTask` | 18 | 18 | 0 | 0 | 0 | 0 | ✅  |
| `AutoCleanPlaceToShuttleTask` | 10 | 10 | 0 | 0 | 0 | 0 | ✅  |
| `AutoCleanPickFromCleanKitStageTask` | 9 | 9 | 0 | 0 | 0 | 0 | ✅  |
| `AutoCleanPlaceToCleanKitTask` | 6 | 6 | 0 | 0 | 0 | 0 | ✅  |
| `AutoCleanPickFromShuttleTask` | 5 | 5 | 0 | 0 | 0 | 0 | ✅  |
| `AutoCleanShuttle1Task` | 4 | 4 | 0 | 0 | 0 | 0 | ✅  |
| `AutoCleanShuttle2Task` | 4 | 4 | 0 | 0 | 0 | 0 | ✅  |
| `IndexStatus` | 2 | 2 | 0 | 0 | 0 | 0 | ✅  |
| `AllPassVerifyTask` | 1 | 0 | 0 | 0 | 1 | 0 | ⛔ 不通的步驟：1:MISSING |
| `TestZTask` | 1 | 1 | 0 | 0 | 0 | 0 | ✅  |

**這一組最重要的缺口**（agent 原文，英文）：

1. Real-machine blocker for the index side of AutoClean and for TestHeadMotor: the Galil card is never opened. Golden TfMain::FormShow main.cpp:9660-9661 calls `if(INDEX_MOTION_CARD==0) Open_GaliCard();`, and the reference Gerneral.ini has INDEX_MOTION_CARD=0. The port has no caller of Open_GaliCard (def myGALILmotor.cpp:4090, decl myGALILmotor.h:76, only comments elsewhere), so bGali_CardInstall stays false (myGALILmotor.cpp:736). Every index move then takes golden's no-card simulated branch (Gali_MotMove :2055-2090, Gali_Two_ZAxis_Move :3900-3908) in DoIndexAutoClean 100..3800 and DoTestHeadMotor 9/15/100/12101/15000/15100/14101. The state machines still advance, but MTestY1/Y2/Z1/Z2 never physically move. Translating that one boot line into wb_serve's boot path fixes it; golden runs it after SystemInitialOK=true.
2. ADAM_WriteVoltage is still an empty stub (atester_shims.cpp:327; golden adam6024.cpp:1798-~1900). The reference machine has EP_Install=3, so golden DoIndexAutoClean case 2300 (golden AutoClean.cpp:8314-8317 = port :7446-7449) writes TestIF.fAutoClean_AireForce to the ADAM6024 EP regulator; the port drops it. The pad is pressed with whatever EP pressure was set before. The same stub is also hit at AutoClean.cpp:5672/6093/6645, atester.cpp:1766/1779/2050/9662/9685 and the aTester_Front/Rear EP writes.
3. SearchCleanNum (AutoClean.cpp:1662-1698, called in DoAutoCleanKit 2000 at :9070) is a partial translation of golden AutoClean.cpp:1245-1304. It drops the fShowBinSelect->ed_AutoCleanCount write (:1278), the pnlCleanCount font colour, and the WAR16313 'AutoClean count reached AlarmCount' alarm (:1282-1301). Its stated reason ('fShowBinSelect has no link-visible home') is stale: cShowBinSelect.cpp:160 is a real global, and 44f36926 already uses it at :3977/:4037. This did not happen on the reference (no WAR16313 in EventLogTxt_20251211), so it does not block the reference path.
4. AllPassVerifyTask is still MISSING. The port has no live iDoAllPassVerifyTask: golden atester.cpp:8684 is outside the verbatim #if 0 block at atester.cpp:9148-9278, and the live DoAllPassVerifyRTC stub at :9280 returns false. The registration at cStateRecord.cpp:1793-1795 is in #if 0 (golden main.cpp:9846). The reference machine has REAL_TIME_CCD=1, but the reference only showed the reset value 1.
5. RTC seam in atester.cpp: W7T1_TCOM2Ext hard-codes bCCDDummyRum=true (:6013) and makes SendCommToVision/OpenRTCComPortAgain no-ops (:6011-6012). Golden sets COM2->bCCDDummyRum from cSetUp.cpp:2797-2805 and main.cpp:12134. The RTC light-on in 600000 (:6283-6285) and the wait in 11 (:6443-6460) are #if 0. The reference went 11->9 in 1 ms, so this is not on the reference path. But this machine has REAL_TIME_CCD=1, so any recipe that turns the CCD buffer on would diverge.
6. Stale gate outside the SM path: SetAutoCleanICCount's `#if 0` around DoTestIFConvert (AutoClean.cpp:3716-3718) cites 'no real body', but DoTestIFConvert is live at cUnitConvert.cpp:370. SetAutoCleanICCount is live and called from csystem.cpp:11058/13770 and FileRW/TestIF_File_Cleaning.cpp:388.
7. Empty stubs that DoTestHeadMotor calls only on non-reference branches: CheckAndRecodrTorque (atester.cpp:9963; golden :9413-9463; called in 12110 at :7040 only if bVTESTFunction) and IndexCheckOneByOne (atester.cpp:10390; golden :9849-9939; called in 12300 at :7128 only if bUseOneByOneIndexCheck). Also TU-local no-ops RecordIndexPosition/EncoderTeachingMaxMinCount/TrigerIndexAxisHome (:5891-5896) and W7T1_CHECKVACINIT (:5918), used in unobserved cases (:7353, :8018, :8555-8564, :8672).

<details><summary>agent 摘要（英文）</summary>

Re-measured at port HEAD d5994b52 (worktree m0925). I re-derived every non-IDLE step from the current code and did not copy the old statuses. Scope: 34 rows. 10 have more than one observed step, 22 only ever showed their initial value, plus AllPassVerifyTask and TestZTask.

How I checked (scratch: .../scratchpad/cov/rerun_g2_index_autoclean/):
- For each state machine I listed its case labels and Task= writes in golden and port (sm.py/cmp.sh). They are IDENTICAL for all 8 AutoClean machines. In DoTestHeadMotor they differ only in case 5 (CCD) and in the gated RTC 11->1, and neither sits between observed steps.
- I diffed the function bodies with comments and whitespace removed (fdiff/allfn over all 65 golden AutoClean.cpp functions).
- I split each observed case into its own segment, listed every call inside it (seg.py), and checked each callee's live definition with defs.py and isdead.py.
- Golden transitions between observed steps that the port lacks: NONE. The resets 14101->1, 2000->1, 3900->1 and PlaceToShuttle 2400->1 come from outside the case in both trees (InitialTestHeadMotorTask, InitialAutoCleanTask, InitialIndexAutoCleanTask, InitPlaceToShuttleTask at AutoClean.cpp:9177).

g2 answer: YES in code. The reference Initial-Start AutoClean path can now run to the end in the port:
- It starts from MainProc csystem.cpp:32086-32104 (golden 18363-18381) and runs through the ladder arm csystem.cpp:33389-33499 (golden 9582-9695).
- AutoCleanTask 1->5->20->2100->(2200->2300->2400)x4->2500->2510->2515->2540->(2600->2800->3000->3050->3100->3200)x4->2000 now completes, together with PlaceToShuttle, PickFromShuttle, PickFromCleanKitStage, PlaceToCleanKit, Shuttle1/2 and AutoCleanIndexTask 1->100->2100..3900.
- What unblocked it:
  - All six GATE W7d-I1 if(false) sites are retired. AutoClean.cpp:4524/4598/4790/8720/8982/9385 are now real MoveInArmXYToShuttle_9045 calls (ainarm9045.cpp:3929 = golden :740-824). InArmOffSet is allocated at wb_serve.cpp:3502/:4012.
  - InArmZNeedDown_9045 is translated (ainarm9045.cpp:13661-13760 = golden :613-712). The only change is a NULL guard for ptrInSHT, which does not fire on this path because AutoClean.cpp:4738/4740 set ptrInSHT first. All 23 dispatch targets are live.
  - The GetShuttleCol ladder is translated (ainarm9045.cpp:13911, identical to golden :8369-8461).
  - CleanSetSpeed pushes speeds again (W7a-I4 retired, :919-987).
  - InitDoTestZHome (AutoClean.cpp:9641 = golden uhome.cpp:4891-4906) and ACSmartClearData (:9068) are restored.
- bRunAutoClean is now cleared in case 2000 (:9027), so the ladder falls through to DoTestHeadMotor (csystem.cpp:1921).
- In DoTestHeadMotor, case 21 EPSwitchOnOff is now real (adam6024.cpp:904/:939, 0 diffs) and the CheckIndexConnect preamble is live (atester.cpp:6098-6102). The preamble runs on this machine because USE_16_HEATER=5 (eht16HeaterDTME08).

Remaining gaps on the reference path (these come from the reference machine's own config files):
1. The Galil index card is never opened, so on a real machine the index never physically moves (top gap 1).
2. EP_Install=3, but ADAM_WriteVoltage is still an empty stub (atester_shims.cpp:327). So DoIndexAutoClean 2300 (AutoClean.cpp:7446-7449; golden :8314-8317 -> adam6024.cpp:1798) never sends the AutoClean air-force value to the pressure regulator. That is the only step I reclassified to GATED.
3. SearchCleanNum, called from case 2000 (:9070), still drops golden :1278 and the WAR16313 AlarmCount alarm (:1282-1301). This was not triggered on the reference (no WAR16313 in EventLogTxt_20251211).
4. AllPassVerifyTask is still not registered.

Status changes vs the previous run:
- AutoCleanTask: 20, 2400, 2540, 2800 and 2000 went GATED->LIVE, and 11 UNREACHABLE steps became LIVE.
- PlaceToShuttle 100/2000/2170 and PickFromShuttle 10 went GATED->LIVE; all their downstream steps went from UNREACHABLE to LIVE.
- AutoCleanIndexTask 1/2200/3000 went GATED->LIVE; 2300 went LIVE->GATED.
- TestHeadMotor 21 went GATED->LIVE.
- TestZTask went MISSING->LIVE: it is now registered at cStateRecord.cpp:1888 with its variable at forms/fHome.h:140. The reference only ever showed 0, but golden 906 writes 1 in DoIndexAutoClean case 1, so the port will show 1 there.

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

