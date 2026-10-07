# 11 大時間項 → 程式碼來源對應

每一項列出：包含的子動作、相關 cpp 函式、計時量測點、Recipe / Config 欄位。

## (1) InArm 吸放料

- **子動作**：Vacuum Suck → Suck Check → Z Up → 移到目標 → Z Down → Vacuum OFF → Destroy
- **函式**：`ainarm9045_<mode>.cpp` 的 `DoInArm_9045()` 各 `iArmTask` 分支
- **既有 timer**：`MyInArmAtShuttleTimer`（ainarm9045.cpp）
- **影響速度**：M00 (MInArmX)、M01 (MInArmY)、M03~M10 (MInArmZA~ZH)
- **Recipe 欄位**：`ArmCondition.Data` 的 Suck Delay、Destroy Time

## (2) Tray Form 多次擺放

- **子動作**：同 row 內 X/Y pitch 連續放料（吸嘴數 < TrayX 時需多次擺放）
- **計算**：擺放次數 = ceil(TrayX / SuckCols)
- **函式**：`ainarm9045_<mode>.cpp` 的 X-Pitch 計算（CheckXYPitch、iXPitch60、iXPitchManual635...）
- **影響速度**：M00 (MInArmX)
- **參考**：`ht9045-sucker-architecture` skill

## (3) HotPlate Soak

- **子動作**：IC 放入 HotPlate → 等待 Soak 完成 → 取出
- **既有 timer**：`tInitSoakTimer.LatchCycleTimeSec`（aTester_Front.cpp）
- **Recipe 欄位**：`HotPlate.Data` 的 Soak Time、HotPlate 容量
- **平行性**：見 `uph-parallel-model.md` 規則 1

## (4) Rotate / Bottom 2D

- **子動作**：InArm 移到 Rotator → 旋轉 → 移回；或移到 Bottom 2D 站 → 拍照 → 移回
- **函式**：`RotateKit/`、`BarCode/`、`LoadCCD/`
- **影響速度**：M41 (MInRotate)、M02 (MInArmPitch)
- **Recipe 欄位**：`ArmCondition.Data` 的 Rotate Enable、Bottom 2D Enable

## (5) Shuttle 移動 + 2D

- **子動作**：Sht1/2 進出 + 2DID scan（含 Qorvo CONFIGURE 流程）
- **函式**：`uShuttleThread.cpp` 的 `Do_Auto_SHT1/2`、`DoBarcodeScanInShuttle`
- **影響速度**：M11 (MInShuttle1)、M12 (MInShuttle2)、M17/M18 (MOutShuttle1/2)
- **參考**：`ht9045-shuttle-flow` skill

## (6) Index 吸放

- **子動作**：Sht pick → Z up → 移到 Test Y → release → Z up → 移回 Sht
- **函式**：`aTester_Front.cpp::DoTestY` / `aTester_Rear.cpp::DoTestYRear`
- **既有 timer**：`tIndexTimer`、`AddIndexCycleTimeRecord`
- **影響速度**：M13/M16 (MTestY1/2)、M14/M15 (MTestZ1/2)
- **Recipe 欄位**：`ArmCondition.Data::IndexCycleTimeMonitoring`

## (7) Index Contact

- **子動作**：Z 下壓 → Drop Contact 1st → 補壓 → Drop Contact 2nd → Contact Sensor OK
- **既有 timer**：`DropContactTimer1`、`DropContactTimer2`、`DropContactTimer3`
- **既有計時點**：`fObserver->AddTimeData(18/19, ...)`（已寫入 TimeInfoGrid）
- **Recipe 欄位**：`Contact.Data` 的 Drop Wait Time、Contact Height

## (8) Start Delay + Test Time

- **子動作**：Drop OK → Initial Delay → Tester SOT → EOT
- **既有 timer**：`dwStartInitialCount`、`dwStartInitialCount1/2`
- **Recipe 欄位**：`Temperature.Data::iInitialStart1Time`、`Prod.iInitialDelay`
- **Tester 影響**：SCK/SPIL 的 `bIndexCycleTimeMonitor` 監控

## (9) OutArm 吸放 + Rotate + 4S

- **子動作**：Sht pick → 移到 Rotator/4S → 旋轉/檢查 → 移到 Tray → release
- **函式**：`aoutarm9045_<mode>.cpp` 的 `DoOutArm_9045()`
- **影響速度**：M19~M29 (MOutArmX/Y/Pitch/ZA~ZH)、M42 (MOutRotate)
- **Recipe 欄位**：`ArmCondition.Data` 的 OutArm Rotate、4S Check

## (10) Loader Tray 補入

- **子動作**：Tray Arm 移到 Loader → 取空盤 → 放空盤匣 → 取新盤 → 放 Loader
- **函式**：`acatchtray.cpp::DoCatchFromLoader`、`DoCatchNewTrayFromBuffer`
- **既有 timer**：`tCatchNewTrayTimer.LatchCycleTimeSec`
- **影響速度**：M30 (MTrayX)、M35 (MLoaderZ)、M36 (MEmptyZ)
- **參考**：`ht9045-catchtray-flow` skill

## (11) Unloader Tray 退出

- **子動作**：Tray Arm 移到 Auto1/2/3 → 取滿盤 → 放滿盤匣 → 取空盤 → 放 Unloader
- **函式**：`acatchtray.cpp::DoPlaceTrayToAuto`、`DoPlaceToBuffer`
- **影響速度**：M30 (MTrayX)、M38~M40 (MAuto1~3 Z)、M37 (MColorZ)

---

## 全域變數速查

| 變數 | 型別 | 來源 | 用途 |
|------|------|------|------|
| `MyInArmAtShuttleTimer` | TQPF_Timer | ainarm9045.cpp | InArm 到 Sht 等待 |
| `DropContactTimer1/2/3` | TQPF_Timer | aTester_Front/Rear | Drop Contact 三段 |
| `dwStartInitialCount` | TQPF_Timer | aTester_*.cpp | Initial Delay |
| `tInitSoakTimer` | TQPF_Timer | main / aTester | HotPlate Soak |
| `tCatchNewTrayTimer` | TQPF_Timer | acatchtray.cpp | Tray Arm 取盤 |
| `tIndexTimer` | TQPF_Timer | aTester | Index 整段 |
| `tOTDTimer` | TQPF_Timer | main.cpp | Tester Docking 等待 |

新增的 `UPH_MotorProfiler` **不重複建 timer**，全部沿用 `TQPF_Timer`。
