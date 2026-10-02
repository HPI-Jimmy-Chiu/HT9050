# IO 類別參考

HT9045 IO 控制類別的詳細成員與配置參考。

## 目錄

1. [TMyCylinder](#tmycylinder)
2. [TMySensor](#tmysensor)
3. [TMySwitch](#tmyswitch)
4. [TMySucker](#tmysucker)
5. [TMyKitSuck](#tmykitsuck)
6. [TLaneIO](#tlaineio)
7. [myio 函式](#myio-函式)

---

## TMyCylinder

**標頭檔**: `mycylin.h`  
**原始檔**: `mycylin.cpp`  
**全域陣列**: `Cylinder[MaxCylinderItem]` (MaxCylinderItem = 295)

### 成員

| 成員 | 型別 | 說明 |
|------|------|------|
| `Enable` | bool | 氣缸啟用 |
| `Status` | bool | 目前狀態 |
| `bCylinderOn` | bool | ON 狀態旗標 |
| `CylinderName` | AnsiString | 顯示名稱 |
| `AlarmEnable` | bool | 警報檢查啟用 |
| `OnAlarmCode` | int | Push 失敗時的警報碼 |
| `OffAlarmCode` | int | Pop 失敗時的警報碼 |
| `OnAlarmTime` | int | Push 逾時（毫秒） |
| `OffAlarmTime` | int | Pop 逾時（毫秒） |
| `OnDelayTime` | int | Push 後延遲（毫秒） |
| `OffDelayTime` | int | Pop 後延遲（毫秒） |

### 輸出配置

| 成員 | 型別 | 說明 |
|------|------|------|
| `OutRing` | int | MotionNet Ring |
| `OutIP` | int | MotionNet IP |
| `OutPort` | int | Port 編號 |
| `OutBit` | int | Bit 編號 |
| `OutType` | int | TYPE_A=0, TYPE_B=1 |
| `OutISABase` | int | IO 基底類型 |

### On 感測器配置

| 成員 | 型別 | 說明 |
|------|------|------|
| `OnSenEnable` | bool | On 感測器啟用 |
| `OnSenRing` | int | MotionNet Ring |
| `OnSenIP` | int | MotionNet IP |
| `OnSenPort` | int | Port 編號 |
| `OnSenBit` | int | Bit 編號 |
| `OnSenType` | int | TYPE_A=0（高電位有效）, TYPE_B=1（低電位有效） |
| `OnSenISABase` | int | IO 基底類型 |
| `OnSensorName` | AnsiString | 感測器顯示名稱 |

### Off 感測器配置

與 On 感測器相同模式，使用 `OffSen*` 前綴。

### 方法

| 方法 | 回傳 | 說明 |
|------|------|------|
| `Push()` | bool | 伸出氣缸，含逾時/警報檢查 |
| `Pop()` | bool | 縮回氣缸，含逾時/警報檢查 |
| `On()` | void | 直接輸出 ON（不等待/無警報） |
| `Off()` | void | 直接輸出 OFF（不等待/無警報） |
| `OnSensor()` | bool | 檢查是否完全伸出 |
| `OffSensor()` | bool | 檢查是否完全縮回 |
| `OnStatus()` | bool | 原始 On 感測器狀態 |
| `OffStatus()` | bool | 原始 Off 感測器狀態 |
| `Reset()` | bool | 重置任務狀態 |
| `GetOutBit()` | bool | 目前輸出狀態 |

### Push() / Pop() 詳細行為與靜默條件

`Push()` 和 `Pop()` 內部有複雜的 Task 狀態機（Task=1/2/50/100/101），等 sensor 並做超時 alarm。
**多種情境下 alarm 會完全靜默不發出，使用前必須了解**：

| 條件 | 機制 | 影響 |
|------|------|------|
| `bHandlerPause==true` | Task=50 內每次都把 timer 重設為 OnAlarmTime/OffAlarmTime | 永不 timeout，**完全靜默** |
| `SystemStart==false` | 跳過 `OnTryTask++/OffTryTask++` 區塊 | timer 過期但不 alarm |
| `Try Task<2`（首次過期） | 只把 Task 設為 2 重試，不發 alarm | 短暫靜默（~2倍 alarm time） |
| HAlarm dedup | `HAlarm::Set` 內 `if(GetStat(iCode)) return;` | 第 2 次以後不寫 EventLog |
| 上層 On()/Off() 重置 Task | `OnTask=1`/`OffTask=1` 直接覆寫 | TryTask 歸零，重新計數 |
| `OnAlarmTime==0` / `OffAlarmTime==0` | 走無 timer 路徑，直接 TryTask++ | 仍受 SystemStart 守衛 |

→ **凡是會卡在 sensor 等待的設計，必須考慮：sensor 真的會亮嗎？** 若 sensor 物理上不會亮（如後勾氣缸的 CAuto1SideFixer-On），改用 `Off()`/`On()` 純命令 + `TQPF_Timer` 自管延遲。

### cylinder.DB AlarmCode 字串被忽略 ★

**注意**：`cinitial.cpp:4668` 雖然解析 cylinder.DB 的 `OnAlarmCode` / `OffAlarmCode` 字串欄位，
**但隨後立即被 `Cylinder[i].OnAlarmCode = 31000+i; Cylinder[i].OffAlarmCode = 31000+i;` 覆寫**。

- 所有 cylinder 的 `OnAlarmCode == OffAlarmCode == 31000+i`（i = cylinder index）
- 無法區分 Push 失敗 vs Pop 失敗（同一 code）
- cylinder.DB 寫的 alarm code 字串只有「documentation」作用，runtime 不使用
- 查 alarm 時：實際 code = 31000 + cylinder index（如 `C_Auto1Side_Fixer = 2` → Err31002）

---

## 後勾氣缸（C_AutoSide_Fixer）特殊邏輯 ★

`C_AutoSide_Fixer[0..3]` 是 unloader auto tray 的**後勾固定氣缸**，其 sensor 安裝方式與一般氣缸**相反**：

### 物理結構

| 氣缸位置 | 物理含義 | 是否常見 |
|---------|---------|---------|
| **Off**（縮回/讓開） | 後勾鉤住 tray 邊緣，**固定 tray** | ★ 生產正常狀態 |
| **On**（推到底） | 後勾完全縮回，tray 可自由出入 | 換 tray 時瞬間 |

### Sensor 安裝

| Cylinder slot | Sensor 名稱 | 物理含義 | 何時會亮 |
|--------------|------------|---------|---------|
| OnSensor slot | **空**（OnSenEnable=false）| — | 永不（無 sensor）|
| **OffSensor slot** | `CAuto1SideFixer-On` | 「氣缸推到底（On 位置）」 | **只有沒 tray 時才會亮** |

→ **命名反直覺**：sensor 叫 `*-On` 但放在 OffSensor 槽

### 設計意圖（推論）
- 一般氣缸 OffSensor = 「縮回到位」感測器
- 後勾氣缸縮回 = 鉤住 tray 邊緣 → **被 tray 擋住，sensor 裝不上**
- 開發者改裝 sensor 到「推到底」位置（On 物理位置）
- 命名為 `*-On`，但放 OffSensor 槽
- 程式呼叫 `Pop()` 等 OffSensor → 等於等「推到底 sensor 亮」
- **正確使用情境**：tray 已搬走後，呼叫 Pop() 驗證氣缸真的回到 On 位置 → sensor 亮 → 確認沒卡住

### 危險使用情境
- **生產中（有 tray）呼叫 `Pop()`** → sensor 永遠不亮 → 永等不到 → 配合 `bHandlerPause` 反覆觸發 → 完全靜默 hang
- 範例：`csystem.cpp:6053 AutoTrayReCheck()` case 10 的 `Pop()` 呼叫
- 範例：`aoutarm9045.cpp:3592 VerifyTrayStatus()` 的 `flag3 = Cylinder[C_AutoSide_Fixer[i]].OffSensor()` 也會被 ServoOff 後的氣缸位置異常觸發

### 安全替代方案
| 場景 | 建議用法 |
|------|---------|
| 純粹想搖動 fixer 重新就位 | `Off()` → 短延遲（0.2s）→ `On()` |
| 需確認 fixer 真的縮回 | 加裝其他 sensor（如 `SnAutoFixCyPush`）或檢查 tray 偏移量 |
| 想 timeout 警報 | 用 `TQPF_Timer` 自管，避開 `Pop()` 的 `bHandlerPause` 重設邏輯 |

### 相關替代 sensor
- `SnAutoFixCyPush[0..3]`：後勾氣缸推進 sensor（用作 `Pop()` 等不到時的軟體替代驗證）

### 對應 alarm code
- `C_Auto1Side_Fixer` = cylinder index 2 → 實際 alarm code = **Err31002**（不是 cylinder.DB 寫的字串）

---

## TMySensor

**標頭檔**: `mysensor.h`  
**原始檔**: `mysensor.cpp`  
**全域陣列**: `Sen[MAX_SENSOR_ITEM]`

### 成員

| 成員 | 型別 | 說明 |
|------|------|------|
| `Name` | AnsiString | 感測器名稱 |
| `Enable` | bool | 感測器啟用 |
| `Ring` | int | MotionNet Ring |
| `IP` | int | MotionNet IP |
| `Port` | int | Port 編號 |
| `Bit` | int | Bit 編號 |
| `Type` | int | 0=低電位有效, 1=高電位有效 |
| `ISABase` | int | IO 基底類型 |
| `State` | int | 上次讀取狀態 |
| `Using` | AnsiString | 使用說明 |

### 方法

| 方法 | 回傳 | 說明 |
|------|------|------|
| `Status()` | bool | 讀取感測器（依 Type 判斷） |
| `IsOn()` | bool | 同 Status() |
| `IsOff()` | bool | Status() 的反向值 |

---

## TMySwitch

**標頭檔**: `myswitch.h`  
**原始檔**: `myswitch.cpp`  
**全域陣列**: `SW[MAX_SWITCH_ITEM]` (MAX_SWITCH_ITEM = 370)

### 成員

| 成員 | 型別 | 說明 |
|------|------|------|
| `Name` | AnsiString | 開關名稱 |
| `Enable` | bool | 開關啟用 |
| `Ring` | int | MotionNet Ring |
| `IP` | int | MotionNet IP |
| `Port` | int | Port 編號 |
| `Bit` | int | Bit 編號 |
| `Type` | int | 0=低電位有效, 1=高電位有效 |
| `ISABase` | int | IO 基底類型 |
| `OutValue` | bool | 目前輸出狀態 |
| `Using` | AnsiString | 使用說明 |

### 方法

| 方法 | 回傳 | 說明 |
|------|------|------|
| `On()` | void | 輸出 ON |
| `Off()` | void | 輸出 OFF |
| `Status()` | bool | 目前狀態（依 Type 判斷） |
| `OnOff(bool)` | void | 依參數設定輸出 |

---

## TMySucker

**標頭檔**: `mykitsuck.h`  
**原始檔**: `mykitsuck.cpp`  
內嵌於 `TMyKitSuck::Suck[row][col]`

### 成員

| 成員 | 型別 | 說明 |
|------|------|------|
| `Enable` | bool | 吸嘴啟用 |
| `SuckerName` | AnsiString | 顯示名稱 |
| `AlarmEnable` | bool | 警報檢查啟用 |
| `OnAlarmTime` | int | 吸取逾時（毫秒） |
| `OffAlarmTime` | int | 破壞逾時（毫秒） |
| `OnDelayTime` | int | 吸取後延遲（毫秒） |
| `OffDelayTime` | int | 破壞後延遲（毫秒） |

### 真空 ON 配置 (OnPort)

| 成員 | 型別 | 說明 |
|------|------|------|
| `OnEnable` | bool | 真空輸出啟用 |
| `OnRing` | int | MotionNet Ring |
| `OnIP` | int | MotionNet IP |
| `OnPort` | int | Port 編號 |
| `OnBit` | int | Bit 編號 |
| `OnType` | int | TYPE_A/TYPE_B |
| `OnISABase` | int | IO 基底類型 |
| `OnPortName` | AnsiString | 顯示名稱 |

### 真空破壞配置 (OffPort)

與 On 相同模式，使用 `Off*` 前綴。

### 感測器配置

| 成員 | 型別 | 說明 |
|------|------|------|
| `SenRing` | int | MotionNet Ring |
| `SenIP` | int | MotionNet IP |
| `SenPort` | int | Port 編號 |
| `SenBit` | int | Bit 編號 |
| `SenType` | int | 高電位/低電位有效 |
| `SenISABase` | int | IO 基底類型 |
| `SensorName` | AnsiString | 顯示名稱 |

### 方法

| 方法 | 回傳 | 說明 |
|------|------|------|
| `Suck()` | bool | 真空 ON，含逾時/警報 |
| `Destroy()` | bool | 破壞 ON，含逾時/警報 |
| `On()` | void | 快速真空 ON |
| `Off()` | void | 快速真空 OFF |
| `OnSuck()` | void | 僅真空輸出 ON |
| `OffSuck()` | bool | 僅真空輸出 OFF |
| `OnDestroy()` | void | 僅破壞輸出 ON |
| `OffDestroy()` | void | 僅破壞輸出 OFF |
| `Normal()` | void | 全部輸出 OFF |
| `Sensor()` | bool | 讀取真空感測器 |
| `GetStatus()` | bool | 目前真空狀態 |

---

## TMyKitSuck

**標頭檔**: `mykitsuck.h`  
**原始檔**: `mykitsuck.cpp`  
**全域實例**: `InArmSuck`, `FTestSuck`, `BTestSuck`, `OutArmSuck` 等

### 常數

```cpp
#define _MAX_SUCK_ROW_ITEM 4
#define _MAX_SUCK_COL_ITEM 8
```

### 成員

| 成員 | 型別 | 說明 |
|------|------|------|
| `iMaxRow` | int | 使用列數 |
| `iMaxCol` | int | 使用欄數 |
| `Item[row][col]` | int | IC 狀態矩陣 |
| `Suck[row][col]` | TMySucker | 吸嘴陣列 |
| `PordRec[row][col]` | TMyProductionRecord | 生產紀錄 |
| `pLed[row][col]` | TALed* | UI LED 綁定 |

### 主要方法

| 方法 | 回傳 | 說明 |
|------|------|------|
| `NoIC()` | bool | 所有位置皆空 |
| `HasIC()` | bool | 任一位置有 IC |
| `UseSiteNoIC()` | bool | 使用中站點皆空 |
| `UseSiteHasIC()` | bool | 任一使用中站點有 IC |
| `SetItemData(row, col, data)` | void | 設定 IC 狀態 |
| `SetAllToNullIC()` | void | 清除所有位置 |
| `CountRealIC()` | int | 計算非空位置數量 |

---

## TLaneIO

**標頭檔**: `MyLaneIo.h`  
**原始檔**: `MyLaneIo.cpp`  
**全域實例**: `MyLaneIO`

### 方法

| 方法 | 回傳 | 說明 |
|------|------|------|
| `IOBitOn(Ring, IP, Port, Bit, ISABase, Alias)` | void | 設定輸出位元 |
| `IOBitOff(Ring, IP, Port, Bit, ISABase, Alias)` | void | 清除輸出位元 |
| `IOByteOut(Ring, IP, Port, Byte, ISABase)` | bool | 寫入完整 byte |
| `IOOutBitStatus(Ring, IP, Port, Bit, ISABase, Alias)` | bool | 讀取輸出狀態 |
| `IOInputBit(Ring, IP, Port, Bit, ISABase, Alias)` | bool | 讀取輸入位元 |
| `IOInputByte(Ring, IP, Port, ISABase)` | byte | 讀取完整 byte |
| `BackUpOutputData()` | void | 儲存目前狀態 |
| `RestoreOutputData()` | void | 還原已存狀態 |
| `CheckPortRangeErr(DO_Type, ISABase, Ring, IP, Port, Bit)` | int | 驗證位址 |

---

## myio 函式

**標頭檔**: `myio.h`  
**原始檔**: `myio.cpp`

傳統 TTL IO 函式，用於 ISA/PCI 卡（當 `TTL_CARD_TYPE > 0` 時跳過）。

| 函式 | 回傳 | 說明 |
|------|------|------|
| `IOBitOn(port, bit)` | void | 設定輸出位元 |
| `IOBitOff(port, bit)` | void | 清除輸出位元 |
| `IOByteOut(port, byte)` | void | 寫入完整 byte |
| `IOOutBitStatus(port, bit)` | bool | 讀取輸出狀態 |
| `IOOutByteStatus(port)` | byte | 讀取輸出 byte |
| `IOInputBit(port, bit)` | bool | 讀取輸入位元 |
| `IOInputByte(port)` | byte | 讀取輸入 byte |
| `BackUpOutputData()` | void | 儲存目前狀態 |
| `RestoreOutputData()` | void | 還原已存狀態 |

---

## 原始檔清單

| 檔案 | 用途 |
|------|------|
| `mycylin.cpp` | 氣缸實作 |
| `mysensor.cpp` | 感測器實作 |
| `myswitch.cpp` | 開關實作 |
| `mykitsuck.cpp` | 吸嘴/Kit 實作 |
| `MyLaneIo.cpp` | MotionNet/PCI IO 封裝 |
| `myio.cpp` | 傳統 TTL IO |

---

## MotionNet API 參考

詳見 [motionnet-api.md](motionnet-api.md)，包含完整初始化、DIO、診斷、看門狗 API 與系統限制說明。
| `_mnet_set_ring_quality_param(...)` | 設定通訊品質監控參數 |

### 錯誤碼

| 常數 | 說明 |
|------|------|
| `ERR_NoError` | 成功 |
| `ERR_Invalid_RingNo` | Ring 編號無效（確認 Ring 是否啟用） |
| `ERR_Invalid_Slave` | Slave IP 無效 |
| `ERR_InvalidDeviceType` | 設備類型無效 |
| `ERR_Not_Initialized` | 硬體未初始化 |
| `ERR_Not_Release` | 資源未釋放 |
| `ERR_FailToOpenFile` | 無法開啟設定檔 |
| `ERR_Invalid_Event` | 事件物件無效 |

### DIO Slave 模組類型碼

| 常數 | 值 | 配置 |
|------|-----|------|
| `G9002_Q32` | 0xB0 | 32 OUT |
| `G9002_I8Q24` | 0xB1 | 8 IN / 24 OUT |
| `G9002_I16Q16` | 0xB2 | 16 IN / 16 OUT |
| `G9002_I24Q8` | 0xB3 | 24 IN / 8 OUT |
| `G9002_I32` | 0xB4 | 32 IN |
| `G9102_I16` | 0xC0 | 16 IN |
| `G9102_I14Q2` | 0xC1 | 14 IN / 2 OUT |
| `G9102_I12Q4` | 0xC2 | 12 IN / 4 OUT |
| `G9102_I8Q8` | 0xC4 | 8 IN / 8 OUT |
| `G9102_Q16` | 0xC7 | 16 OUT |
