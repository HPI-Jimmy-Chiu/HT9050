# HT9045 自動測高（Auto Height）／接觸測試（Contact Test）動作流程（golden 906 0618）

> 寫給 Steven（20261005）。只讀整理，沒有改程式。
> - 07:2x Steven 要求「把 HT9045 執行 auto height / contact test 的動作流程寫到 skill/reference」。
> - **08:1x 依 Steven 更正（「你的流程有點問題」）改成 Steven 的 7 步順序。**
> - **08:2x 依 Steven「使用 function / task 做參照，不要使用程式碼的行號」，全文只寫函式名＋Task／case，不寫行號。**
>
> HT9050 今天走到哪、卡在哪，另一份：`D:\HT9045\.claude\skills\ht9045-index-flow\references\autoheight-contact-test-ht9050-current.md`。
>
> **程式碼的樹**（找函式用）：
> - **golden 906 0618**＝`D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\`（cp950）。沒寫檔名的函式都在 `cContact.cpp`（`TfContact::`）。
> - **V912**＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（cp950）。
>
> 寫法：`DoTestContactFunction` 的狀態變數是 `CarlibrationTask`，寫成「DoTestContactFunction case N」。子狀態機（`DoZ1PickFromShuttle`、`Do_Z1_AutoGetHeight`…）的狀態變數各自不同，寫成「函式名 Task N」。
>
> 單位：Index Z 的位置是 Galil 計數，畫面欄位（`edContactHeight1` 等）用 `ConvertTouMType()`／`Get0_01MMType()` 換成 mm（0.01 mm 一格）。往下＝負。

## 0. Steven 的 7 步（本文的骨架）

| 步 | Steven 的說法 | golden 906 0618 在哪裡做 |
|---|---|---|
| 1 | In／Out Arm 讓開 | DoTestContactFunction case 100（Z 回安全）、case 200（In Arm XY、Out Arm XY 讓位；**同時 In Shuttle 1/2 移到 `InSHT[n].iLeft`＝放料等待位**） |
| 2 | In Shuttle 檢查浮料（置偏）→ 掃 2D ID（有開才掃）→ 往右進 Index | 浮料：DoTestContactFunction case 219（`CheckShuttleSensor_9045()`）／case 230（latch 版）。2D：`DoZ1PickFromShuttle` Task 130-143。往右：`DoZ1PickFromShuttle` Task 150（`MInShuttle1` → `InSHT[0].iRight`） |
| 3 | Index 從 In Shuttle 吸料 | DoTestContactFunction case 300 → `DoZ1PickFromShuttle` Task 200-700；Arm2 在 case 330 → `DoZ2PickFromShuttle`（**golden 是兩臂都先吸完料，才開始量高度**） |
| 4 | Index 1 到 socket 區量高度 | DoTestContactFunction case 340 → 400 → `Do_Z1_AutoGetHeight` |
| 5 | Index 2 到 socket 區量高度 | DoTestContactFunction case 799 → 800 → `Do_Z2_AutoGetHeight` |
| 6 | Index 把 IC 放回 In Shuttle | DoTestContactFunction case 805 → 1100 →（RTC learning 950/1000）→ 900 → `DoZPlaceToShuttle` |
| 7 | In Shuttle 回左邊 | DoTestContactFunction case 1700（`MInShuttle1/2` → `InSHT[n].iLeft`）→ case 1800 結束 |

**golden 的程式順序跟 7 步不一樣的地方（照實寫，沒有重排）：**

- 步 1 的 DoTestContactFunction case 200 已經把 In Shuttle 移到 `iLeft`（左＝等待位，操作員在這裡放 IC）。所以步 2 的「往右」是從 iLeft 出發。
- 步 2 的浮料檢查在 DoTestContactFunction（case 219），2D 掃描與往右卻在下一層的 `DoZ1PickFromShuttle`（Task 130-150），也就是步 3 那支函式的前半段。
- 步 4 開頭 `Do_Z1_AutoGetHeight` Task 1 有一個分岔：`USE_OUT_SHT_MOT==1 || MachineTypeChoice==Type_HT502` 時走 Task 150/151，**把 In Shuttle 1 移回 `InSHT[0].iLeft`、Out Shuttle 1 移到 `OutSHT[0].iRight`**，再下壓 socket；否則走 Task 100（動 Index Y，飛梭不動，停在 iRight）。一般 HT9045（`USE_OUT_SHT_MOT=0`）是後者。`Do_Z2_AutoGetHeight` Task 1 一律走 Task 100，沒有 150/151。
- 步 6 的 `DoZPlaceToShuttle`、`DoArm1/2PlaceToShuttle` **都不移動飛梭**，靠飛梭還停在 `iRight`。golden 在走 Task 150/151 的機型上，放回前沒有人把 In Shuttle 移回 iRight。這是 golden 現狀，本文只記錄、不判斷對錯。
- 步 7 的 DoTestContactFunction case 1700 先把 Index Y1/Y2 移到換 kit 位，再把 In Shuttle 1/2 移回 `iLeft`。Out Shuttle 的移動在 golden 是註解掉的（case 200 與 case 1700 都是）。

模式（`iContactMode`，`cContact.cpp` 檔頭常數）：

| 模式 | 值 | 單選鈕 | 主要差別 |
|---|---|---|---|
| `CONTACT_NORMAL` | 0 | rbModeNormal | DoTestContactFunction case 1 拒絕「Must Exit Contact Form」 |
| `CONTACT_AUTO_GET_HEIGHT` | 1 | rbAutoHeight | 步 4/5 用扭力找 socket 高度 |
| `CONTACT_MANUAL_GET_HEIGHT` | 2 | rbManualHeight | 步 4/5 手動（面板鍵 jog，或伺服 OFF 手推） |
| `CONTACT_TEST` | 3 | rbContactTest | 步 4/5 壓到存好的高度、等 T.Start 測試 |
| `AUTO_CONTACT_TEST` | 4 | rbAutoContactTest | case 340 → 1500 `Do_AutoContactTest` |
| `STEP_CONTACT_TEST` | 5 | rbStepContactTest | 步 1-2 改走逐步入料（case 80/85），步 7 改走逐步出料（case 2000/2100） |
| `CONTACT_LoadCell_AUTO_GET_HEIGHT` | 8 | rbLoadCellAutoHigh | case 2195 → 2160/2170 `Do_LoadCellAutoHigh(0/1)` |
| `CONTACT_DEVICE_MAP_CHECK` | 9 | rbDeviceMapping | case 1 → 50/55 `DoDeviceMapCheck` |
| `CONTACT_DEVICE_LOOP_TEST` | 10 | rbDeviceLoopTest | case 200 → 2300 |
| `K_TEMP_INDEX_MOVE` | 11 | rbKTempIndexMove | case 200 → 2400 |
| （V912 才有）`VISUAL_DETECTION_TEST` | 12 | rbVisualDetectionTest | V912 DoTestContactFunction case 60-65 |

## 1. 入口：按鈕 → START → DoTestContactFunction case 1

| 按鈕 | golden 906 0618 函式 | 做什麼 |
|---|---|---|
| 模式單選 | `rbModeNormalClick` → `SetContactMode()` | 只把單選鈕翻成全域 `iContactMode`，並處理畫面顯示：`cbOneTouchAutoContactHight` 只給 KYEC_LEE／KYEC_XILINX／MAXIM_THAILAND，`chkDailyCorrelation` 只在模式 5 可勾。KYEC_CHEN＋A16 另外叫 `ChangeContactMode()` |
| START（btnStart） | `btnStartClick` | `fMain->BtnStartClick(fMain)`，再把 `edTorue0/1` 設 "10" |
| T.Start／T.Step | `btnTStartClick`／`btnTStepClick` | 設 `bSetupStart`／`bSetupStep`，流程裡的 `WaitManualStartKey()`／`WaitManualStepKey()` 等的就是它 |
| One Cycle | `spbOneCycleClick` → `OneCycleProcess` | `bContinueContact` 反相（模式 <3 一律 false）。模式 1/3 時 case 1100 會回 400 一直重來 |
| 存檔 | `spbSaveClick` → `SaveSetupFile` | 高度按這裡才進工單（§8） |
| Exit | `sbtExitClick` | `CarlibrationTask!=1` 不准離開 |

START 之後，golden `csystem.cpp` 的 `MainProc` 在 Contact 頁開著、`iHome==0`、`iContactMode!=CONTACT_NORMAL` 時呼叫 `fContact->DoTestContactFunction()`，不跑生產流程。

**DoTestContactFunction case 1（初始化）的關卡**，任一項不過就 return（多數同時 `SystemStart=false`）：

1. 函式開頭：`SoftStop || SystemStart==false` 不做。
2. `SetContactMode()`。
3. 模式是 `CONTACT_NORMAL` →「Must Exit Contact Form」。
4. Index、In Arm、Out Arm、Shuttle 上有 IC →「Please finish ONE CYCLE before Contact Test!」。
5. 設速度：Device Map 用 `SetMotorSpeed()`，其他模式 `SetAllMotorSpeed(10)`。
6. `fAllMotorHome==false` →「Must home first」。
7. `SW[SwTesterAirCooling].On()`，**`ADAM_WriteMaxData(true)`（EP 以最大值充氣）**。
8. 分派：Device Map → case 50；RTC 且 `bFullTestBeforeContactHeight` → case 70；Step → case 77/80；其他 → case 90（RecordProcess 記「Start Auto Contact Height Calibration.」或「Start Contact Test.」）。

## 2. 步 1：In／Out Arm 讓開

| DoTestContactFunction | 動作 | 馬達／IO |
|---|---|---|
| case 90 | 訊息「EP 充飽氣、Z 軸回安全位置」 | — |
| case 100 | 速度上限：240／260／360／400 kg 機 `iSpeedZ`≤150，其他≤60；`iSpeed` 30～60。In Arm Z 回安全、Out Arm 回安全、Index Z1/Z2 一起回 `Prod.TestZ1_Safe`。提示「請置放 Device 至 Shuttle 上，並按 Step」 | `MoveInArmZToPlateSafe`、`MoveOutArmToAutoSafe`、`MTestZ1.Gali_Two_ZAxis_Move` |
| case 190 | `IsTrayArmAtEmptyOrColor()` 不在 → `fAllMotorHome=false`＋「Tray Arm is not safe position」 | — |
| case 200 | 移動四項：In Arm X → `XInArm_Tray_Pick[0][2]`；In Arm Y → `YInArm_Tray_Pick[0][2]-10000`；Out Arm XY → `MoveOutArmXY_ToFix_Tray_Full`；**In Shuttle 1/2 → `InSHT[n].iLeft`（等待位）**。Out Shuttle 的移動在這裡是註解掉的 | MInArmX/Y、MOutArm、MInShuttle1/2 |
| case 205-212 | SFC auto tune（`SHT_FLOATING_CHK==1` 且勾 `cbSFCAutoTune`）／Tray map（選配） | — |
| case 218-2195 | `DoIndecxCHECkFunction`（Index check）；Load Cell 模式 → 2160；In Shuttle latch teach → 2196-2198 | — |

## 3. 步 2：浮料（置偏）檢查 → 2D ID → 往右進 Index

### 3.1 浮料檢查：DoTestContactFunction case 219／230

- **case 219**：`CheckShuttleSensor_9045()`（`ainarm9045.cpp`）。依 Test Mode 叫 `CheckShuttleSensor_9045_1x1/1x2/2x2/1x4/2x3/…`，最後都到 `CheckShuttleSensorStatus()`。
  - 檢查的是 In Shuttle 上的進出／對射感測器 `SnInPutSHT1Sn`／`SnInPutSHT2Sn`（`CROSS_SENSOR_INSTALL` 時每格 4 顆，否則 2 顆）。
  - 用途：料凸起（浮料／疊料）或沒擺好時報警（In Shuttle floating error），並等這一項過了才往下。
  - **只在 `LastSet.iRealDummy==REALLY` 檢查**，Dummy 直接過。
- **case 230**：`In_Shuttle_Auto_Latch==eInSHAutoLtc` 且 REALLY 時改走這裡，用 `CheckInShuttleSensor_Latch_Contact()`（latch 版，兩顆 sensor 判疊料、飛料）。
- 通過之後：Hot 且 `CosFunction.bContactTestWaitSoakTime` → case 220 倒數 Soak time（按 Step 可跳過）→ case 290。
- **另一種「置偏」**：`SHT_FLOATING_CHK`（CCD 看 IC 置偏）。在 Contact 流程裡只有 case 205-210 的 auto tune 用到，平常這一步不跑。

### 3.2 選臂：DoTestContactFunction case 290

- 單 shuttle 且選 Shuttle 2：`EPSwitchOnOff(eEPSwArm2)`，跳 case 330。
- 其他情況 → case 300（ASE 高雄／ASE_CL 勾 In/Out Arm Z 教導時先走 295-297）。

### 3.3 2D ID 與往右：`DoZ1PickFromShuttle`（Z2 版 `DoZ2PickFromShuttle` 對稱）

| Task | 動作 |
|---|---|
| 1 | Z1/Z2 → `TestZ1_Safe`；模式 1 且沒開 `bD12UseDeviceFormPressDoShtHeight` → `ADAM_WriteMaxData(true)`，否則 `ADAM_WriteVoltage(DeviceForm.dPress)` |
| 100-120 | 開吸嘴、等 1 s；EP_Install=3＋D24 時檢查 EP 漏氣（WAR1605）；`CheckIndexArmStatus(0)`：吸嘴上不該有 IC，有就 WAR0320（K_RETRY），回 Task 100 重試。之後關吸嘴、等 1 s |
| **130**（2D 開關） | 條件 `BAR_CODE_INSTALL!=ebctUninstall && TestIF_File.bEnableBarCode && chkDailyCorrelation 沒勾`：<br>– 有開 Bottom 2D（`BOTTOM_2DID && bEnableBottom2D`）→ 不在這裡掃，只把開的 site 標 HAS_IC → Task 150；<br>– 否則畫面 `chk2DID` 有勾 → Task 140 掃；沒勾 → Task 150（Contact 可跳過 2D）。<br>條件不成立 → Task 150 |
| 140-143（2D 掃描） | 把開的 site 標 HAS_IC。勾 `cb2DMatrix` 時先 `fBarCode->DoBarcodeCCDAutoTeach`（自動學 2D 矩陣）。然後 `InitialBarcodeScanInShuttle1()`、`CleanBarcodeError(1)`。掃描函式依設定三選一：<br>– CCD 模式（`ebctUseCCDMode`）：`DoBarcodeCCDInShuttle_1()`，Sub Job／Pin Inspection 時改用 `DoBarcodeScanInShuttle_1(true)`；<br>– Trigger 模式（`b2DTriggerMode`）：`DoBarcodeTriggerInShuttle_1()`；<br>– 其他：`DoBarcodeScanInShuttle_1(true)`。<br>單 shuttle 選 2 時 Shuttle 1 跳過 |
| **150**（往右） | **`MInShuttle1` → `InSHT[0].iRight`**（到 Index 下方）；兩 shuttle 模式時 `MInShuttle2` 也到 iRight。**同時 Index Y1 → `TestY1_Front`、Y2 → `TestY2_Middle`**（Arm1 對準 Shuttle 1）。`bF21InOutArmZMotorPrivate` 時先 `DoInOutARM_SHT_MoveSafe` |

## 4. 步 3：Index 從 In Shuttle 吸料

`DoZ1PickFromShuttle`（DoTestContactFunction case 300），接著 case 320 →（兩臂時）case 330 `DoZ2PickFromShuttle` → case 335 → case 340。

| Task | 動作 | 扭力／等待／錯誤 |
|---|---|---|
| 200 | 開吸嘴。模式 1 且勾 `chkShuttle`（順便量 Shuttle 取料高度）→ Task 300；模式 3/9 → Task 250（延遲 1～3 s）；其他 → Task 3000 | — |
| 300 | — | **扭力上限 `COM2->iWriteAndCheckMotorTorque(0, iContactKG)`**（`CONTECT_SHUTTLE_KG`）；失敗「Motor torque set error」 |
| 400 | 吸嘴在高處就已經 ON →「sucker sensor initial on error」 | — |
| 500-560 | 量 Shuttle 高度：每次往下 300（AMKOR_China／QUALCOMM 100），每步讀一次扭力 | `iTorque>=iContactKG` 連 5 次 → Task 570。**Task 560 等 `edTorue0` 沒有逾時**。下降前 Index Y1 不在 Front ±10 →「Index Y1 Motor Position error」；encoder 低於 `Tech.iTestZ1ShutlePick-1000` →「Index arm Z1 to shuttle height error!」 |
| 570 | 停住；`edPickUp1=(Pos+PICK_UP_OFFSET−Tech.iTestZ1ShutlePick)/100`（ASE 高雄再減 0.2 mm） | 結果只寫在畫面欄位 |
| 600-650 | 扭力 80% 再下去吸，Z 回原點 | — |
| 3000-3100 | 一般取料：Z1 → `Tech.iTestZ1ShutlePick + edPickUp1 + Offset.iIndexArmPickUp[0]`，開吸 | 掉料：「Z1 Contact/Auto Height pick fail or IC drop, stop pressing」 |
| 660-680 | 沒吸到料 | JAM0301（K_RETRY） |
| 700/800 | Z 回 `TestZ1_Safe`，扭力回 300% | — |

DoTestContactFunction 吸料後的處理：

- **case 320／335**：只有 GIGAS／JCET 會用 `DoSocketSensorCheckRemainIC()` 查 socket 殘料。case 320 另外做 `EPSwitchOnOff(eEPSwBoth)`。
- **case 340**：關掉沒用到的吸嘴。開了 `bD11NoIcSkipAutoHeight` 時，哪一臂沒吸到料，那一臂就不量（直接到 case 805）。之後分派：
  - 模式 4 → case 1500；
  - 模式 3＋32 site → case 500；
  - 其他 → case 400。

## 5. 步 4：Index 1 到 socket 量高度——`Do_Z1_AutoGetHeight`（DoTestContactFunction case 400）

### 5.1 Task 1 → 去 socket 上方（各模式共用）

| Task | 動作 |
|---|---|
| 1 | 清 `dHeight`；查吸嘴掉料（JAM0303，K_SKIP）；通知 ATC 目前是哪一臂；查 SLK clamp 汽缸 I/O、socket clamp sensor；`EPSwitchOnOff(eEPSwBoth)`；Z1/Z2 → `TestZ1_Safe`。**分岔：`USE_OUT_SHT_MOT==1 \|\| Type_HT502` → Task 150，否則 → Task 100** |
| 100 | **Index Y1 → `TestY1_Middle`（socket 上方）、Y2 → `TestY2_Rear`**（一般 HT9045 走這裡，飛梭不動） |
| 150/151 | **In Shuttle 1 → `InSHT[0].iLeft`、Out Shuttle 1 → `OutSHT[0].iRight`**（兩支飛梭離開 socket 下方）；已經在位就直接到 Task 110 |
| 110 | 移動途中掉料 → Task 2200 |
| 200 | `fHome->InitDoTestZHome()` |
| 520 | 依模式分派：<br>– 模式 2（手動）→ Task 530 或 950；<br>– 模式 3/5/9 → `ADAM_WriteVoltage(DeviceForm.dPress)`（EP 設成工單壓力）→ Task 2900（D24 EP 檢查時先 Task 2800）；<br>– **模式 1** → `ADAM_WriteMaxData(true)`（D17=2 用最小力／2；D17=3 量兩次比對）→ Task 530（AMKOR_China／QUALCOMM 85 kg 機先等 3 s，Task 525） |

### 5.2 自動測高（模式 1）

| Task | 馬達 | 扭力 | 停止／出口 |
|---|---|---|---|
| 530 | Z1 → `Tech.iTestZDown-5000`（待命高度） | `kg=GetAutoHeightMaxKGTorque()`：<br>– 預設 40；<br>– 開 `bD14_AutoHeightUseSetTorque` 時用 `iD14`；<br>– 缸徑 <25 mm：10／15／22；<br>– 240/260 kg 機：15～30；<br>– 400/360/500/640/800 kg 機：12～20 | — |
| 536 | — | **寫扭力上限**：國際牌 `COM2->iWriteAndCheckMotorTorque(0, kg)`；三菱 `kgTranToMitsubishikg(kg)`（kg/3，夾在 15～30）。`USE_IO_CHANGE_TOQUE` 時先關 `SwIndexChangeToque1/2` | 寫失敗（回 2）→「Index Z1 Motor torque set error」、`fAllMotorHome=false`、`CarlibrationTask=1`、return false |
| 550 | — | `COM2->InitReadTorueTask()`、`chkReadTorque1=true`、清 `edTorue0` | 送 GPIB `MSG_CMD_Arm1Down` |
| **555** | — | **等 `edTorue0` 有值，golden 沒有逾時**（SIM 直接填 30）。讀 encoder 寫進 `edContactHeight1`；國際牌取絕對值 | 判斷 **`TorqueData>=kg` 或 `edContactHeight1<=fIndexDownPos`**：要**連 10 次**成立才算；不成立 → Task 560 再往下一步 |
| 555 第一次成立 | `ST` | 記 `iRecordZ1Pos1`（EP 充飽時的高度）；低於 `fIndexDownPos` 就用 `fIndexDownPos`，並警告「Over Z1 contact high」；**EP 漏氣檢查**：EP_Install 3/5＋D26 時 `ADAM_Rang`＋`ADAM_Alarm()`，否則看 `SnEPAlarm`；然後 **`ADAM_WriteVoltage(0)`＋`MySleep(3000)`（EP 洩氣）** | → Task 700 |
| 560 | 每拍往下一步：國際牌 `Gali_MotMoveSkipEncoder(Pos-iStepSpeed)`（`iStepSpeed`=300；D17=3 第二次 100）；三菱在扭力 >3 後改走 1/10 步 | | 下降前查 Index Y1 在 `TestY1_Middle` ±10（HT502 不查），否則「Index Y1 Motor Position error」；吸嘴掉料 → Task 564/565（上升 1000、JAM0303） |
| 700 | 目標＝encoder＋3000 | 接觸高度 `iRecordZ1Pos`＝`iRecordZ1Pos1-100`：<br>– EP 漏氣時改 −20；<br>– D17=1 用硬體高度，不減；<br>– 缸徑≤25 mm 時 +100；<br>– D17=3 記兩次高度比對，差太大報 WAR0377（Task 7110-7125） | |
| 710 | Z1 上升到目標；`edContactHeight1` 顯示；**EP 重新充到 `DeviceForm.dPress`**（先 0 再 dPress，中間隔 1 s） | | |
| 7150 | — | 扭力上限 **120%**；寫之前先查吸嘴（O-ring 有沒有掉） | |
| 7160 | Z1 → `iRecordZ1Pos` | 開始讀扭力 | `UpdateContactRelative()` |
| 7170 | — | 扭力 **≥125 連 5 次** →「Z1 Motor Auto Height error, check EP Value!」（EP 力撐不住，馬達自己在推）→ Z1 回原點 → Task 905 | 等扭力**沒有逾時** |
| 720/730 | Z1 → `iRecordZ1Pos`（停在接觸高度） | 扭力 **300%**（＝不限） | 送 `MSG_CMD_Arm1Down`，`iWhichArmDown=1` |
| 800 | 停在 socket 上 | | **等操作員**：<br>– T.Start → `DoSetupTest(0)` 測一次（Task 810）；<br>– **T.Step → 結束**（Z1 回原點、`RunTestProgram(false)`、`MSG_CMD_ContactTestAbort`）→ Task 905；<br>– D58 一丟一測 → Task 8005-8007 放 IC |
| 900-902 | 量到後上升：＋500×GearRatio → 再＋1000×GearRatio → `Tech.iTestZDown` | 扭力 kg+40（kg>50 時用 90）→ 99 → 300 | 手動模式 `Pos>SafeTestZContactHeight` →「test height too high」 |
| 905/930 | `Gali_SingalHome()`（Z1 回原點） | | |
| 2100/2200 | Z1 → 0；**升上來才查掉料**（JAM0303）；D58 查 socket sensor（WAR0323） | | `bContinueContact` → Task=1 再來一次；否則 **return true** |

`fIndexDownPos`（`TfContact::SetIndexDownPos`）是**下降的最後一道底線**，到了就當成接觸高度並跳警告：

| 條件 | 值 |
|---|---|
| 使用者自訂 `dUserDefMaxContactHeight` | 優先用它 |
| `Type_HT9046_LS` | −148.0 |
| 有 ATC | −146.0 |
| KYEC 等客戶 | −140.0 |
| 其他 HT9045 | −135.0 |

### 5.3 Contact Test（模式 3）

| Task | 動作 |
|---|---|
| 2800 | （EP_Install=3＋D24）等 2 s、`ADAM_Alarm()` → WAR1605（K_RETRY／K_SKIP） |
| 2900 | 在接觸高度+750 以上時查吸嘴掉料；80／40.2 mm kit 補高度。<br>– **Direct Test**：Z1 → `edContactHeight1`（+`edContactOffsetArm1`）→ Task 3000；<br>– **Drop Test**：Z1 → contact＋Drop offset → Task 2910 放開吸嘴、Task 2920 等 `TestZ_Drop_Wait` → Task 2930 |
| 2930 | Z1 → contact 高度；SoftEP 模式要到 Pos+1500 以內才 `EPSwitchOnOff(eEPSwBoth)`；SLK 分離機構 → Task 2952-2956（clamp 異常 JAM0373／0375） |
| 3000 | 送 GPIB `MSG_CMD_Arm1Down`／ATC TJ；`iGPIBIndexStatus=Z1Down_Z2Up`；Side Push → Task 3002 |
| 3010 | 等操作員：<br>– T.Start → `DoSetupTest(0)`（Task 3100，可回 3000 再測）；<br>– T.Step → Task 3110 結束。<br>KYEC_LEE／HONPREC_QC 有 Index jog 面板 |
| 3110 | 清測試狀態、`MSG_CMD_ContactTestAbort`。分岔：DropPlaceShift → 3150、SLK clamp → 3180、Side Push → 6200、其他 → **3200** |
| 3200-3300 | 重開吸嘴、等 2 s、Z1 上升 200 → Task 2100（收尾同 §5.2） |

模式 3 下壓時**沒有扭力停止條件**：它是「壓到存好的高度」，力量由 EP（`DeviceForm.dPress`）決定。

### 5.4 手動測高（模式 2）

- **`bD10ManualHeightComptibleWithNS=1`**：
  - Task 530：到待命高度，扭力上限 kg；
  - Task 560：按住面板 `SnFMotorDown` 且扭力 <kg → `Gali_JogNSetup` 往下 jog（速度從 100 遞增到 5000）；`SnBMotorDown` → 往上 jog；放開 → `ST`；
  - T.Start 測試，T.Step 結束。
- **`bD10=0`**：
  - Task 950/960：扭力上限 `Prod.iMaxPreasure`；
  - Task 1000：按住鍵 → `Gali_Command("MOY")`（**伺服 OFF，用手推**）；放開 → `SHY`；
  - Step → Task 2000 讀 encoder。

### 5.5 Load Cell（模式 8）：`Do_LoadCellAutoHigh(iIndex)`（DoTestContactFunction case 2160 → 2170）

流程同 §5.2，差別：

- Y 到 `Teach.iLoadCellY1/Y2`，Z 待命高度 `Teach.iLoadCellZ1Down-100`；
- 下限 `fIndexDownPos_LoadCell=-100`；`iStepSpeed=10`（細步）；
- 結果寫 `edLoadCellHeight1/2`。

**golden 怪處（照翻）：**

- Task 122／300／330 不論 iIndex 都寫 `iWriteAndCheckMotorTorque(0, …)`，等於 Arm2 也寫到 Z1 的扭力上限；
- Task 130 沒有 break，直接掉進 150；
- SIM 分支把 Z1 欄位填給 Z2。

## 6. 步 5：Index 2 到 socket 量高度——`Do_Z2_AutoGetHeight`（DoTestContactFunction case 799 → 800）

- **進入條件**：兩臂都在用，或單 shuttle 選 2（case 335 直接到 799）。case 799 只有 JCET 會用 `DoSocketSensorCheckRemainIC()` 查一次。
- **跟 Z1 逐 Task 對稱**，差別：
  - Task 1 一律走 Task 100，沒有 150/151；
  - Task 100：Y1 → `TestY1_Front`、Y2 → `TestY2_Middle`（Arm2 對準 socket）；
  - 扭力用 `iWriteAndCheckMotorTorque(1, …)`，讀 `edTorue1`／`chkReadTorque2`；
  - Task 555 一樣**沒有逾時**。
- **D58 一丟一測**：Arm2 量完，若 Arm1 上還有真的 IC → case 802 `DoZ1PickFromSocket`。
- **case 805**：
  - 模式 1 時 `bAutoHighFinish=true`；
  - 單 shuttle 時把沒用的那一臂高度強制寫成 −50.0、取料／放料高度抄另一臂；
  - 2D 排序模式兩臂都寫 −50.0。

## 7. 步 6：Index 把 IC 放回 In Shuttle——`DoZPlaceToShuttle`（DoTestContactFunction case 900）

DoTestContactFunction 的路徑：

- **case 1100**：`bContinueContact` 且模式 1/3 → 回 case 400 再量一次。
- 否則，RTC 有開且勾 `cbRTCModel` → case 950（`DoRTCAutoTuning`）或 case 1000（ROI learning，再到 1200/1300 auto verify）→ case 900。
- 都沒有 → 直接 case 900。

| 函式 | Task | 動作 |
|---|---|---|
| `DoZPlaceToShuttle` | 1 | 查兩臂掉料（JAM0303／JAM0304） |
| | 100 | Z1/Z2 回安全 |
| | 110 | `USE_INDEX_ARM_AXES==IndexArm_3_Axis` → Task 1000（一臂一臂放），否則 Task 120 |
| | 120-160 | Y1 → Front、Y2 → Rear；高度差太大先提示「please make sure and save data first」 |
| | 200-350 | 兩臂同時 Z → `edReleaseHeight＋Tech.iTestZ1ShutlePick`，關吸嘴，等 2 s（氣壓不足時分兩次放，Task 600-800） |
| | 400 | Z 回安全，return true |
| `DoArm1PlaceToShuttle` | 100-600 | Y1 Front／Y2 Rear → 高度差 >10 mm 提示存檔、把 `Prod.TestZ1_Pick` 改成新值 → Z1 下去、關吸嘴 → Task 400 等 `hContactDeley` → Z 回安全。**golden 906 在 Task 300 沒有先啟動 `hContactDeley`**；V912 的 `DoArm1PlaceToShuttle` Task 300 補了 `SetSecAndOn(2)` |
| `DoArm2PlaceToShuttle` | 100-600 | 3 軸機只動 Y1 到 Middle；其他同 Arm1（這支有啟動 2 s 計時器） |

**飛梭**：放回這一步**沒有移動飛梭**，靠的是步 2 往右之後，飛梭一直停在 `iRight`。走 `Do_Z1_AutoGetHeight` Task 150/151 的機型例外，見 §0。

## 8. 步 7：In Shuttle 回左邊——DoTestContactFunction case 1700 → 1800

| case | 動作 |
|---|---|
| 1700 | `MTestY1.GalilTwoY_Move(Prod.TestY1_Front_EndWaitPos, Prod.TestY2_Rear)`（Index 到換 kit 位）→ **`MInShuttle1` → `InSHT[0].iLeft`** → **`MInShuttle2` → `InSHT[1].iLeft`**（依序）。Out Shuttle 的移動在 golden 是註解掉的。Step 模式 → 1900/2000（逐步出料），其他 → 1800 |
| 1800 | 收尾：<br>– 關 `SwTesterAirCooling`，`fMain->Pause`；<br>– 解鎖畫面選項，`InitialTestHeadMotorTask()`；<br>– 非模式 3：把高度備份到 `edContactBackUp1/2`、`edOrgPick1/2`；<br>– **[D63]**：見 §9；<br>– Task=1 |

## 9. 存了什麼

| 什麼時候 | 寫到哪 |
|---|---|
| 量的過程 | 只寫畫面欄位：`edContactHeight1/2`（socket 接觸高度）、`edPickUp1/2`（Shuttle 取料高度）、`edReleaseHeight`、`edLoadCellHeight1/2`。`DoArm1PlaceToShuttle`／`DoZPlaceToShuttle` 會改 `Prod.TestZ1_Pick`／`TestZ2_Pick` |
| DoTestContactFunction case 1800 | `edContactBackUp1/2`、`edOrgPick1/2`（下次開頁顯示上次值） |
| DoTestContactFunction case 1800 [D63] | `bD63CheckIndexZHomeToZPhaseDistanceRange` 且有修正時，**`WriteIniData(asGeneralPath,"IndexDriver","Index_Z1_Home_Position"／"Index_Z2_Home_Position", …)`**，寫的是共用的 `D:\HT9045\system\Gerneral.ini`；並在 `asIndexZphasePath` 記一行 |
| 按存檔 | `TfContact::SaveSetupFile` → 工單 `[Test Arm1]`／`[Test Arm2]` 的 `Contact／Pick Up／Place／ContactBackUp／ShuttlePickBackUp`；`CosFunction.bContactHeightSaveToContactIni` 時另寫 `D:\HT9045\system\Contact.ini`（JSCC） |
| — | `D:\HT9045\system\ContactInfo.ini` **不是**測高結果，是 SLK 缸徑清單（`TfContact` 建構子讀） |

沒按存檔就離開：`FormClose` 會重讀檔，丟掉改動。

## 10. 扭力：怎麼設、怎麼讀（國際牌，COM2）

全在 golden 906 0618 `rs232.cpp` 的 `TCOM2`：

| 項目 | 函式 | 內容 |
|---|---|---|
| 寫上限 | `iWriteAndCheckMotorTorque(MotorIndex, Torque)` | Task 2 選軸（`chkReadTorque1/2`）→ Task 10 `WriteIndexTorqueSetting` → Task 100 讀回 → Task 200 比對。國際牌讀回值不等於設定值時重試 5 次，再失敗回 2。三菱：馬達有 alarm 直接回 2，上限夾在 100 |
| 國際牌參數 | `WriteIndexTorqueSetting_Pana` | A5：命令 0x17、參數 0x0D（Pr0.13 第一扭力上限）；A4：參數 0x5E。`SwReadTorue` relay 切 Z1（Off）／Z2（On）——**兩台驅動器共用一條 COM2** |
| 讀扭力 | `ReadTorque_Panasonic` | `chkReadTorque1/2` 有勾才讀。交握：ENQ → EOT → `{0x00, 位址, 0x52, 0xAE-位址}` → ACK+ENQ → EOT。**0x52＝命令 2／模式 5（讀目前扭力），額定扭力＝2000** |
| 解碼 | `ReadTorque_Panasonic` 收資料段 | `k=str[4]<<8|str[3]`；`k<0` 當 0；**`Torque=k/20.0`＝額定 %**；用 `%5.2f` 寫進 `fMain->edTorue0` |
| 讀失敗 | `ReadTorque_Panasonic` 的逾時段 | `Torque=-9999`、重開 COM；連續 >10 次「Rs232 Read Index Z1 Torque error!!」，**流程照樣等** |
| 自製通訊卡 | `ReadTorque_HPCard` | `TorqueUseHPComCard` 時改走這支 |

所以 golden 的「kg」在國際牌機其實是**額定扭力的 %**（40＝40%）。手冊細節：`D:\HT9045\.claude\skills\ht9045-motor-control\references\panasonic-rs232\`。

## 11. 等待與逾時

| 等什麼 | 有沒有逾時 |
|---|---|
| `Do_Z1/Z2_AutoGetHeight` Task 555 等扭力 | **沒有** |
| `Do_Z1/Z2_AutoGetHeight` Task 7170 等扭力 | **沒有** |
| `DoZ1/Z2PickFromShuttle` Task 560 等扭力 | **沒有** |
| 寫扭力 `iWriteAndCheckMotorTorque` | 有：每段 `iTorqueCommMaxTime` 秒、重試 5 次 → 回 2 →「torque set error」＋要重新歸零 |
| 讀扭力 RS-232 交握 | 每段 `iPanasonicWT` 拍；超過 10 次失敗只跳訊息，**流程照樣等** |
| 操作員 T.Start／T.Step | 沒有（本來就是等人） |
| 浮料檢查 DoTestContactFunction case 219 | 沒有（等到 sensor 過才往下） |

## 12. V912 跟 golden 906 0618 的差別（只列跟本流程有關的）

| V912 位置 | 差什麼 | 性質 |
|---|---|---|
| `Do_Z1/Z2_AutoGetHeight` Task 800 的 Step 判斷、DoTestContactFunction case 2195 與 case 1800 | `CosFunction.bAutoHeightSkipManualStepKey` 且模式 1：不等 T.Step，直接結束，最後跳「Index arm Auto height calibration is finished!!」（ASE-CL） | 功能 |
| `SetContactMode`、DoTestContactFunction case 1 與 case 60-65 | 新模式 12 `VISUAL_DETECTION_TEST`（TFAMD） | 功能 |
| DoTestContactFunction case 212 與 case 2060 | Tray Arm 等待位改用 `GetTrayArmWaitPos()` | 功能 |
| `DoArm1PlaceToShuttle` Task 300 | 補 `hContactDeley.SetSecAndOn(2)` | **修正** |
| `Do_ROILearning`、`Do_AutoContactTest` | 下降中的掉料檢查改看「往下的那一臂」 | 修正（不在測高本體） |
| `FormShow` 等 | ASE_CL 把 Contact 高度欄位反灰 | 功能 |
| `TfContact::CalcDeviceForce` | 統一力量公式 | 重構 |

以下函式的本體 V912 沒有其他差異：

- `Do_Z1/Z2_AutoGetHeight`
- `DoZ1/Z2PickFromShuttle`
- `Do_LoadCellAutoHigh`
- `DoZPlaceToShuttle`
- `CheckIndexArmStatus`
- `CheckShuttleSensor_9045`

`rs232.cpp` 的扭力讀寫段 V912 也沒改。

## 13. 三句話

1. In／Out Arm 讓開、In Shuttle 停在左邊等待位（操作員放 IC），按 Step 後先檢查 In Shuttle 浮料，2D 有開就掃，然後 In Shuttle 往右進 Index，Index 兩臂依序吸料。
2. Index 1、再 Index 2 到 socket 量高度：EP 充飽、扭力上限設 kg（國際牌＝額定 %），Z 每拍往下一步讀一次 COM2 扭力，連 10 次 ≥kg（或到 `fIndexDownPos`）就記高度、EP 洩氣；然後用 120% 驗 EP、300% 壓到接觸高度，等 T.Start／T.Step。
3. Index 把 IC 放回 In Shuttle，In Shuttle 回左邊；高度按存檔才進工單，golden 等扭力的地方都沒有逾時，[D63] 結束時會回寫共用的 Gerneral.ini。
