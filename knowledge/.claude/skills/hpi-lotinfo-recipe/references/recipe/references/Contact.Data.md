> 保存來源：`.claude/skills/ht9045-recipe/references/Contact.Data.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Contact.Data — Index Arm 接觸點位置設定

**路徑：** `D:\HT9045\IniData\Data\[工作檔名]\Contact.Data`
**模組：** [cContact.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cContact.cpp)
**結構體：** `DeviceForm_File`（TfContact 表單類別成員）
**讀：** `TfContact::ReadFile()` ｜ **寫：** `TfContact::SaveFile()`

---

## Section 總覽

| Section | 用途 |
|---------|-----|
| `[Test Arm1]` | 第一測試臂 Z 軸位置 |
| `[Test Arm2]` | 第二測試臂（雙臂機台） |
| `[Wait Time]` | 動作等待時間 |
| `[Mode]` | 接觸模式與功能設定 |
| `[IndexDriver]` | Shuttle KG 設定 |
| `[Torque Control]` | 接觸力/扭矩計算 |
| `[Height Calibration]` | 高度校正測試動作設定 |

---

## `[Test Arm1]` / `[Test Arm2]` — Z 軸位置

| Key | 型態 | 說明 |
|-----|------|------|
| Pick Up | float | 從 Shuttle 吸取位置 Z (mm) |
| Contact | float | 接觸（下壓）位置 Z (mm)；負值=向下壓 |
| ContactBackUp | float | 接觸後退避 Z (mm) |
| Drop | float | Drop 模式放下量 (mm) |
| Drop_DropContactMode | float | Drop Contact 模式專用的放下量 (mm) |
| Place | float | Shuttle 放置位置 Z (mm) |
| ShuttlePickBackUp | float | 從 Shuttle 吸取後退避 Z (mm) |
| Up | float | 上升後目標 Z (mm) |
| LoadCellZ1 | float | LoadCell 校正位置 Z1 (mm)（Arm1） |
| LoadCellZ2 | float | LoadCell 校正位置 Z2 (mm)（Arm2） |

---

## `[Wait Time]` — 動作等待

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| Drop Wait | float | — | 放鬆（Drop Release）後等待時間 (秒) |
| Drop Speed | float | — | Drop 動作速度 (%) |
| Up Wait | float | — | 上升後等待時間 (秒) |
| Up Speed | int | — | 上升速度 (%) |
| Side Push Wait Time | float | — | 側推等待時間 (秒) |

---

## `[Mode]` — 接觸模式與功能

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| Contact | int | — | 接觸模式（見下方對照表） |
| Vacuum | int | 0 | 0=測試中保持真空, 1=測試時關閉真空 |
| Head Device Mode | int | — | 測試頭裝置模式 |
| AutoKSHTOfs | float | 2.0 | 自動 Shuttle KSH 補正量 (mm) |
| Dummy Contact | int | 0 | 1=使用 Dummy 接觸（無實際 IC） |
| Kit Diameter | float | — | Kit 直徑 (mm) |
| KitDiameterMode | int | 999 | Kit 直徑模式；999=不使用 |
| Die Force Kit Diameter | float | 2.0 | Die Force 依 Kit 直徑計算基準 (mm) |
| Suck Shuttle Device After Tested | int | 0 | 1=測試後持續吸取 Shuttle Device |
| Suck Shuttle Device Wait On Shuttle | int | 0 | 等待 Shuttle Device 停止後才吸取 |
| iSocketInitialICCheckPosition | int | 0 | Socket Initial IC 檢測位置模式 |
| fSocketInitialICCheckPositionOffset | float | 0.0 | Socket Initial IC 檢測位置補正 (mm) |
| dDropByPassDetect | float | 0.0 | Drop By-Pass 感測器閾值 |
| Shuttle Waiting Out Site Chamber | int | 0 | Shuttle 等待 Out Side Chamber 模式 |
| Air Purge Before Pick From Shuttle | int | 0 | 1=從 Shuttle 吸取前先吹氣 |
| Air Purge Before Pick Shuttle Time | int | 1 | 吹氣持續時間 (秒) |
| Air Purge Before Pick Shuttle Interval | int | 1 | 吹氣間隔 (秒) |
| Air Purge Before Pick Shuttle OffSet | int | 1 | 吹氣補正量 |
| Tester Side Push | int | 0 | 0=不使用, 1=使用 Tester 側推 |
| Tester Side Push Mode | int | 0 | 側推模式 |
| IndexUpSpeed | int | 0 | Index 上升速度覆寫值（0=不覆寫） |

### Contact Mode 對照

| 值 | 模式名稱 | 說明 |
|----|---------|------|
| 0 | DirectContact | 直接接觸 |
| 1 | Drop | 放鬆後接觸 |
| 2 | DirectDiffSpeed | 直接接觸（雙速） |
| 3 | TMove | TMove 模式 |
| 4 | TMoveDrop | TMove + Drop |
| 5 | DirectSoftEP | 軟 EP 直接接觸 |
| 6 | DropSoftEP | 軟 EP Drop |
| 7 | DropPlaceShiftContact | Drop Place Shift |
| 8 | DropDiffSpeed | Drop 雙速 |
| 9 | TMoveSlowContact | TMove 慢速接觸 |
| 10 | TMoveDropSlowContact | TMove Drop 慢速 |

---

## `[IndexDriver]` — Shuttle 驅動

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| CONTECT_SHUTTLE_KG | int | 10 | Index 接觸 Shuttle 的力量上限 (Kg) |

---

## `[Torque Control]` — 接觸力計算

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| Pin Number | int | 0 | 測試 Socket 的接觸 Pin 數 |
| X Dimension | int | 0 | X 方向 Die 尺寸 |
| Y Dimension | int | 0 | Y 方向 Die 尺寸 |
| Force Per Pin G | float | 30.0 | 每 Pin 接觸力量 (g) |
| Force Per Pin N | float | — | 每 Pin 接觸力量 (N)，由 G 換算 |
| Force Per Pin Kg | float | — | 每 Pin 接觸力量 (Kg)，由 G 換算 |
| Die Force Per Pin | float | — | Die 接觸總力 |
| Die Force Per Pin Kg | float | — | Die 接觸總力 (Kg) |
| iIndexTorqueMax | int | 120 | Index Torque 上限值 |
| iIndexTorqueCmp | int | 30 | Index Torque 補正值 |
| Torque | float | — | 目前 Torque 值（唯讀/參考） |
| bUseAddWeight | bool | false | 1=加掛重物（使用附加重量模式） |
| Double Force | float | 3.0 | 雙倍力量係數 |
| Pin of Die | int | 10 | Die 上的 Pin 數 |
| dZ1Torue | float | 0.0 | Z1 測試扭矩值 |
| dZ2Torue | float | 0.0 | Z2 測試扭矩值 |
| Enable Use Universal Kit | int | 0 | 1=使用通用 Kit 模式 |
| Original Force Per Pin Kg | float | 0 | 原始每 Pin 力量 (Kg)（備份值） |
| Original Pin Number | int | 0 | 原始 Pin 數（備份值） |

---

## `[Height Calibration]` — 高度校正動作

| Key | 型態 | 預設 | 說明 |
|-----|------|------|------|
| Test Contact Count | string | "1" | 每次高度校正的接觸次數 |
| Test Sec | string | "1" | 每次接觸停留時間 (秒) |
| Auto Contact Test Count | string | "4" | 自動高度校正的測試總次數 |

---

## 範例

```ini
[Test Arm1]
   Pick Up=23.79
   Contact=-100.70
   ContactBackUp=-100.10
   Drop=2.00
   Drop_DropContactMode=2.00
   Place=25.79
   ShuttlePickBackUp=23.79
   Up=23.79
   LoadCellZ1=0.00
[Test Arm2]
   Pick Up=23.79
   Contact=-100.70
   ContactBackUp=-100.10
   Drop=2.00
   Drop_DropContactMode=2.00
   Place=25.79
   ShuttlePickBackUp=23.79
   Up=23.79
   LoadCellZ2=0.00
[Wait Time]
   Drop Wait=0.10
   Drop Speed=30.00
   Up Wait=0.10
   Up Speed=30
   Side Push Wait Time=0.00
[Mode]
   Contact=2
   Vacuum=0
   Head Device Mode=0
   Kit Diameter=6.0000
   KitDiameterMode=999
   AutoKSHTOfs=2.0
   Dummy Contact=0
   Suck Shuttle Device After Tested=0
   iSocketInitialICCheckPosition=0
   Tester Side Push=0
   Tester Side Push Mode=0
   IndexUpSpeed=0
   Air Purge Before Pick From Shuttle=0
[IndexDriver]
   CONTECT_SHUTTLE_KG=10
[Torque Control]
   Pin Number=72
   Force Per Pin G=30.0
   iIndexTorqueMax=120
   iIndexTorqueCmp=30
   bUseAddWeight=0
[Height Calibration]
   Test Contact Count=1
   Test Sec=1
   Auto Contact Test Count=4
```

---

## 注意事項

- `Contact` 值對應 `enum eContactMode`（定義於 [MachineType.h](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\MachineType.h)）
- `Contact=-100.70`：負值表示向下壓 100.70mm
- 雙臂機台需同時設定 `[Test Arm1]` 與 `[Test Arm2]`
- `[Torque Control]` 由 Torque Calculation 功能使用，需搭配 IniConfig 啟用
- `LoadCellZ1/Z2` 用於 LoadCell 感測器校正位置

---

## 關聯程式碼

- 讀寫實作：[cContact.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cContact.cpp) — 搜尋 `ReadFile` 定位
- 表單定義：[cContact.h](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cContact.h)
- Contact Mode 列舉：[MachineType.h](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\MachineType.h) — `enum eContactMode`
- Index 流程：ht9045-index-flow Skill
<!-- preserved-content:end -->
