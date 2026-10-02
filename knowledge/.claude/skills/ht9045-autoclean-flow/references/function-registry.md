# Auto Clean Function Registry

**檔案**: `AutoClean/AutoClean.cpp` (~9000 行)

## 初始化

| 函式 | 行號 | 說明 |
|------|------|------|
| InitialSet() | 79 | 載入 offset/speed 參數 |
| EnableAutoclean(bool Manual) | 130 | 手動/自動觸發入口 |
| InitialAutoCleanAllTask() | 677 | 初始化全部 3 個 Task 佇列 + 觸發 OneCycle |
| InitialAutoCleanTask() | 691 | 主 Task 初始化, FTP jam 準備 (SPIL) |
| InitialShuttleAutoCleanTask() | 708 | 重置 Shuttle1/2 task 計數器 |
| InitialIndexAutoCleanTask() | 715 | 重置 Index task 計數器 |

## 速度管理

| 函式 | 行號 | 說明 |
|------|------|------|
| CleanSetSpeed(bool bBackup) | 725 | 備份/恢復馬達速度 (InArm, Shuttle, Z) |

## 座標計算

| 函式 | 行號 | 說明 |
|------|------|------|
| GetAutoCleanPickCount() | 162 | Grid 計數 |
| GetAutoCleanPickStep() | 189 | 映射吸嘴 column → X 位置 |
| CalculateAutoCleanXPitch() | 226 | 計算 X 間距 |
| GetXPitchOfCleanKit_HP() | 261 | X pitch (HotPlate 模式) |
| GetXPitchOfCleanKit_Kit() | 327 | X pitch (CleanKit 模式) |
| GetXPitchOfCleanKit() | 536 | 分派器 |
| GetYPitchOfCleanKit() | 545 | 計算 Y 間距 |

## Clean Pad 追蹤

| 函式 | 行號 | 說明 |
|------|------|------|
| SetAutoCleanICCount(bool Work) | 571 | UI StringGrid 設定 + MOT trace |
| SetAutoCleanTrayPosition() | 658 | 定位 UI Tray 元件 |
| SearchCleanNum() | 1217 | 找最小 Clean 計數 |
| SearchiAutoCleanNum() | 1122 | 搜尋下一可用 Clean Pad |
| TrayHasCleanIC() | 2143 | 布林：Kit 是否有 HAS_CLEAN_IC |
| TrayHasCleanICCount() | 2165 | 計算 HAS_CLEAN_IC 數量 |
| CheckCleaningCount() | 2188 | 驗證計數 vs device count |
| SearchCleanKitRowCol() | 1643 | 尋找下一 Kit Row/Col (bool) |
| SearchCleanKitRowCol() (v2) | 1679 | 依 Shuttle 尋找 (void) |
| SearchCleanKitUpDown() | 1747 | 決定 Shuttle 方向 |
| GetInarmSuckRow() | 8993 | 映射 Kit row → InArm sucker row |

## 持久化

| 函式 | 行號 | 說明 |
|------|------|------|
| ReadWriteAutoCleanCount(bool bRead, bool bReset) | 1162 | INI 讀寫 Clean 計數 |
| AutoCleanWriteData() | 153 | 寫入 HandlerCondition.Data |
| SetAutoCleanStringGrid() | 1143 | 寫入 UI Grid 單格 |

## 資料管理

| 函式 | 行號 | 說明 |
|------|------|------|
| CleanOnlyHasNullInShuttle() | 55 | 過濾 Shuttle 只留 HAS_NULL_IC |
| RestoreCleanKitData() | 1425 | 從備份恢復 IC 狀態 |
| SetShuttleIcForSpecialMode() | 1266 | 特殊 IC 模式處理 |
| GetShuttleState(eWhichShuttle, bool) | 935 | 掃描 HAS_CLEAN_IC → 0-4 |

## 運動控制

| 函式 | 行號 | 說明 |
|------|------|------|
| MoveInArmXYPickCleanKit() | 1980 | XY 移至 Clean Kit |
| MoveInArmZToShuttlePlace() | 813 | Z 移至 Shuttle 放料高度 |
| MoveInOutArmZToKitPickPlace() | 1022 | Z 移至 Kit pick/place 高度 |
| MoveInArmZ_Shuttle_Pick() | 3119 | Z 移至 Shuttle pick 高度 |
| InOutArmSuckActiveSet() | 806 | 重置 Sucker active 旗標 |

## 核心 Pick/Place

| 函式 | 行號 | 說明 |
|------|------|------|
| PickFromCleanKit() | 2440 | 核心吸取 Clean Pad |
| PlaceToCleanKit() | 4096 | 核心放回 Clean Pad |
| DoPlaceToKitSwapData() | 2418 | IC 資料交換 (通用) |
| CheckInArmSuckFromCleanKitICFallDown() | 2301 | 吸取後掉料檢查 |
| CheckInSuckICFallDown() | 3283 | IC 掉落驗證 |

## Shuttle 協調

| 函式 | 行號 | 說明 |
|------|------|------|
| CleanPad_PlaceToShuttle() | 3369 | Shuttle 放料輔助 |
| CleanPad_PickFromShuttle() | 3728 | Shuttle 取料輔助 |
| DoPlaceToShuttle() | 3511 | 完整 Place-to-Shuttle 流程 |
| DoPickFromShuttle() | 3815 | 完整 Pick-from-Shuttle 流程 |
| CheckShuttleSensor_Clean() | 4312 | Shuttle 感測器檢查分派 |

## 狀態機（主要）

| 函式 | 行號 | Task 變數 |
|------|------|----------|
| **DoAutoCleanKit()** | 4367 | iDoAutoCleanTask |
| **DoAutoCleanPickfromCleanKit()** | 2549 | iAutoCleanPickFromCleanKitStageTask |
| **DoAutoCleanPlaceToCleanKit()** | 4178 | iAutoCleanPlaceToCleanKitTask |
| **DoShuttle1AutoClean()** | 5945 | iDoShuttle1AutoCleanTask |
| **DoShuttle2AutoClean()** | 6093 | iDoShuttle2AutoCleanTask |
| **DoIndexAutoClean()** | 7231 | iDoIndexAutoCleanTask |

## 狀態機（特殊模式）

| 函式 | 行號 | 說明 |
|------|------|------|
| DoIndexAutoClean_Arm1PickArm2Test() | 6348 | 雙臂模式 Index |
| DoShuttle1AutoClean_Arm1PickArm2Test() | 5835 | 雙臂模式 Shuttle1 |
| RunAutoCleanByArmPickArm2Test() | 1155 | 雙臂模式選擇器 |
| CheckAutoCleanCloseSite() | 4357 | Close-site 驗證 |
| Special_2X6_Tray_XItem7() | 8977 | 2x6 Tray 邊界處理 |

## 錯誤處理

| 函式 | 行號 | 說明 |
|------|------|------|
| DoSocketSensorAlarm() | 6318 | Socket 浮料報警處理 |
| DoInArmPineRelease() | 5704 | 卡料後 Servo On/Release |
| DoInArmMoveToWaitPosByAutoClean() | 6269 | 卡料恢復安全位置 |
| ResetAutoClean() | 5781 | 清除所有 Clean Pad IC |
| RecDebug() | 6307 | 追蹤 Kit 資料 |

## 全域 Task 變數

```cpp
int iDoAutoCleanTask = 1;                        // 主協調器
int iAutoCleanPickFromCleanKitStageTask = 1;     // Pick from Kit
int iAutoCleanPlaceToCleanKitTask = 1;           // Place to Kit
int iAutoCleanPlaceToShuttleTask = 1;            // Place to Shuttle
int iDoShuttle1AutoCleanTask = 1;                // Shuttle1 定位
int iDoShuttle2AutoCleanTask = 1;                // Shuttle2 定位
int iDoIndexAutoCleanTask = 1;                   // Index 接觸
```

## 全域完成旗標

```cpp
bool bInedxCleanFinish[2] = {false, false};      // [Arm1][Arm2]
bool b1ShuttleMoveToLeft, b1ShuttleMoveToRight;
bool b2ShuttleMoveToLeft, b2ShuttleMoveToRight;
```

## 全域錯誤恢復旗標

```cpp
bool bAutoCleanShuttle1MoveToLeft;
bool bAutoCleanShuttle1HasPickErr;
bool bAutoCleanShuttle2MoveToLeft;
bool bAutoCleanShuttle2HasPickErr;
```

## 全域流程控制

```cpp
bool bRunAutoClean;                   // AutoClean 執行中
bool bAutoCleaning;                   // 正在清潔
bool bPlaceToCleanKit;                // 放料階段
bool bIsAutoOneCycleAutoclean;        // OneCycle+AutoClean 複合
int iAutoClean_IndexContactCount;     // 累計 Index 接觸次數
```
