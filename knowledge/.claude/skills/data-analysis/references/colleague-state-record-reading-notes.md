# StateRecord 實戰判讀補充

本文件整理在實際 Hangup 分析中，容易誤判、但可直接用 StateRecord 排除的兩個重點。

## 1. Input Arm 正常等待 Shuttle 回左，不應直接列為主嫌

### 何時可以先排除 InArm

若同時滿足以下條件，可先把 Input Arm 自主嫌名單排除：

- `InArmSuck` 顯示 InArm 手上仍有 IC 資料。
- 現場馬達位置或 StateRecord 證據顯示 In Shuttle 仍停在右側，尚未回左。
- InArm 的流程語意本來就是「等 Shuttle 回左才能把手上 IC 放回 Shuttle」。

這種情況下，InArm 停住通常是正常等待，不是根因。

### 實務判準

- InArm 若是在等 Shuttle 回左，這是結果，不是原因。
- 除非有直接證據證明 InArm 狀態機不跳轉、命令未發、或馬達失敗，否則不要把 InArm 當第一嫌疑。
- 下一步應優先查 Shuttle 為何未回左，或更上游地查是誰讓 Shuttle 仍必須停在右側。

## 2. Task_ListWithTime 底部持料區塊可直接讀出當下資料狀態

### 可直接使用的欄位

`Task_ListWithTime.csv` 底部常見以下區塊：

- `InArmSuck`
- `OutArmSuck`
- `FRCarryKit`
- `BRCarryKit`
- `FTestSuck`
- `BTestSuck`

這些不是單純 sensor on/off，而是各物件內部 `Item[][]` 的資料快照。

### 常用狀態碼

以下狀態碼已在 V3.33.878 分支對到常數定義：

- `0 = NULL_IC`
- `2 = HAS_IC`
- `5 = HAS_NULL_IC`
- `1001 = TEST_PASS`

因此：

- `1002` = `TEST_PASS + 1`
- `1004` = `TEST_PASS + 3`

這些 `100x` 值代表測後 bin 資料，不是普通未測 IC。

### 例子

若 StateRecord 顯示：

```text
InArmSuck
5,5,5,2
5,2,5,5
```

可直接解讀為：

- InArm 兩排四顆吸嘴都不是空的。
- 其中大部分位置是 `HAS_NULL_IC`。
- 少數位置是 `HAS_IC`。

這表示 InArm 的確拿著料，只是不同吸嘴上拿的是不同型態的資料。

若同時看到：

```text
OutArmSuck
0,0,0,0
0,0,0,0
```

則表示 OutArm 當下是空手。

若同時看到：

```text
FRCarryKit
0,0,0,0,0,0,1002,0
0,0,0,0,0,0,0,0
```

則表示 Front Right carry 區還有測後 bin 資料留在某個 site。

### 用法建議

- 先讀這六組快照，再決定哪個模組該列入嫌疑。
- 若某模組手上資料與其等待條件一致，先排除它。
- 若 carry 區有資料、但對應吸嘴區為空，優先查「誰負責搬運卻沒有搬走」。

### 進階推論：從持料快照推論取料位置判斷錯誤

當發現以下組合時，高度懷疑**取料位置判斷邏輯錯誤**：

#### 症狀組合

1. **OutArmSuck 全 0**（完全沒拿到料）
2. **FRCarryKit / BRCarryKit 有料且位置明確**
   - 例如：`Item[0][6] = 1002`（只有 RowA Column 6 有測後資料）
   - 或：`Item[1][4] = HAS_IC`（只有 RowB Column 4 有 IC）
3. **PickFromShuttle 子任務完成**（回到 state 1）
4. **OutArmTask 循環執行取料流程**（50 → 100 → 1200 → 50）

#### 推論邏輯

```
OutArm 執行了取料動作（PickFromShuttle 完成）
    ↓
但 OutArmSuck 全 0（沒拿到料）
    ↓
而 Shuttle 上確實有料（FRCarryKit 有資料）
    ↓
==> OutArm 移動到了錯誤的位置
    ↓
==> 取料位置判斷函數回傳值錯誤
```

#### 定位步驟

**Step 1：確認實際 IC 位置**
```
從持料快照找出唯一有料的位置：
FRCarryKit:
0, 0, 0, 0, 0, 0, 1002, 0,  ← RowA: Item[0][6] 有料
0, 0, 0, 0, 0, 0, 0, 0,     ← RowB: 全空

==> IC 在 RowA, Column 6
```

**Step 2：找出位置判斷函數**

根據測試模式找對應函數：
- 2x8 模式：`GetNowShuttleMode_2x8_8()`
- 2x4 模式：`GetNowShuttleMode_2x4_X()`
- 其他模式：類似命名規則

**Step 3：比對預期 vs 實際**
```
查看函數回傳值格式（以 2x8 為例）：
- 5XYZW 格式
- 51000 = Row A, Column 組 1（右側 4,6）
- 51001 = Row B, Column 組 1（右側 4,6）

預期：IC 在 RowA → 應回傳 51000
實際：函數回傳 51001（RowB）
==> 判斷邏輯錯誤
```

**Step 4：檢查程式碼常見 Bug 模式**

1. **重複檢查同一位置**（Copy-Paste Bug）：
   ```cpp
   if(Item[0][4]>=HAS_IC || Item[0][4]>=HAS_IC)  // ❌ 重複
   ```

2. **遺漏關鍵位置**：
   ```cpp
   // 只檢查 Column 4，忘記檢查 Column 6
   if(Item[0][4]>=HAS_IC)  // ❌ 遺漏 Item[0][6]
   ```

3. **if-else 最後的 else 無條件假設**：
   ```cpp
   if(條件A) return X;
   else if(條件B) return Y;
   else return Z;  // ❌ 無檢查直接假設
   ```

#### 實戰案例（2026-04-17）

**StateRecord 資料**：
```
iCloseSiteModeFor2x8 = 5 (e2x8CloseEven)
FRCarryKit: Item[0][6] = 1002（只有 RowA Column 6 有料）
OutArmSuck: 全 0
```

**問題定位**：
```cpp
// aoutarm9045_2x8_8.cpp 第 194 行
else if(ptrOutSHT->Item[0][4]>=HAS_IC || ptrOutSHT->Item[0][4]>=HAS_IC)
    return 51000;  // ❌ 重複檢查 Item[0][4]，應為 Item[0][6]
else
    return 51001;  // ❌ 錯誤：實際 IC 在 RowA 但回傳 RowB
```

**修正方向**：
```cpp
else if(ptrOutSHT->Item[0][4]>=HAS_IC || ptrOutSHT->Item[0][6]>=HAS_IC)
    return 51000;  // ✅ 正確檢查 Column 4 和 6
else if(ptrOutSHT->Item[1][4]>=HAS_IC || ptrOutSHT->Item[1][6]>=HAS_IC)
    return 51001;  // ✅ RowB 也要檢查
else
{
    EventLog("WAR: No IC found in both rows!");
    return 51000;  // 預設或報 WAR
}
```

#### 推論技巧總結

當遇到「OutArm 空轉但 Shuttle 有料」時：

1. ✅ **先看持料快照** → 確認實際 IC 位置（哪個 Row、哪個 Column）
2. ✅ **識別位置判斷函數** → 根據測試模式找對應函數
3. ✅ **比對預期 vs 實際** → 函數應該回傳什麼 vs 實際回傳什麼
4. ✅ **檢查 Bug 模式** → 重複條件、遺漏位置、無條件假設
5. ✅ **驗證修正** → 修正後測試各種 IC 位置組合

這種推論方式可應用於所有「位置判斷邏輯錯誤」導致的空轉問題。

## 3. 實戰案例總結

在 2026-03-27 的 Hangup 個案中，這套讀法幫助快速排除兩種常見誤判：

- 誤判 InArm 卡死
- 誤判 Shuttle 單純無法左移

正確做法是：

1. 先從 `InArmSuck` 證明 InArm 手上有料，且其等待 Shuttle 回左屬正常語意。
2. 再從 `FRCarryKit` / `BRCarryKit` 與 `OutArmSuck` 的對照，判定後續應優先查 OutArm 是否未成功把 Shuttle 上的料搬走。

## 4. 維護原則

## 5. 如何判斷 Shuttle 在左邊還是右邊

### 程式裡的正式判斷函式

Input Shuttle 左右位的正式判斷函式在 `csystem.cpp`：

- `InSHT1InLF()`
- `InSHT1InRT()`
- `InSHT2InLF()`
- `InSHT2InRT()`
- `InShtInRT(iSht)`

### 判斷邏輯不是只看馬達目前位置

程式實際上是用「移動方向旗標 + 馬達到位 + teach position」組合判斷。

#### Left 判斷

以 `InSHT1InLF()` / `InSHT2InLF()` 為例：

1. 先看 `b1ShuttleMoveToLeft` / `b2ShuttleMoveToLeft` 是否為 true。
2. 再看馬達 `iInposLed` 是否成立，確認目前到位、不是還在移動中。
3. 最後才判斷位置是否等於 `Prod.InSHT[i].iLeft`，或在 offset mode 下是否落在左側 teach point 附近。

所以：

- 不是「位置看起來接近左邊」就算在左邊。
- 若左移旗標沒被設起來，`InSHTxInLF()` 會直接回 false。

#### Right 判斷

以 `InSHT1InRT()` / `InSHT2InRT()` 為例：

1. 先看馬達 `iInposLed` 是否成立。
2. 再判斷位置是否等於 `Prod.InSHT[i].iRight`。
3. 若有 offset 修正，則接受「距離右側 teach point 小於約 200 pulse」的範圍判定。

### 對 StateRecord / Motor.xls 的實戰讀法

若沒有直接跑程式函式，分析 StateRecord 或現場資料時可用下面的實戰判準：

1. 先看 `MInShuttle1` / `MInShuttle2` 的 Current 與 Target。
2. 若 Current 與 Target 都接近 `Prod.InSHT[i].iLeft`，可判定 Shuttle 在左側。
3. 若 Current 與 Target 都接近 `Prod.InSHT[i].iRight`，可判定 Shuttle 在右側。
4. 若 Current 與 Target 都不在左/右 teach point附近，表示 Shuttle 可能在中間、偏移補正位置，或仍在移動過程中。

### 本案中的實務用法

在 2026-03-27 個案中，現場觀察是：

- `MInShuttle1` / `MInShuttle2` 的 Current 與 Target 都停在右側數值。

因此可直接得出：

- Shuttle 當下不在左邊。
- InArm 等 Shuttle 回左屬正常等待語意。
- 後續應追查為什麼流程邏輯仍讓 Shuttle 留在右側，而不是先懷疑 InArm。

### 分析時的建議順序

判 Shuttle 左右位時，建議順序如下：

1. 原始碼語意：優先以 `InSHTxInLF()` / `InSHTxInRT()` 的判斷規則理解系統定義。
2. 現場 / StateRecord：以馬達 Current / Target 對照 `iLeft` / `iRight` teach point 做快速判讀。
3. 若兩者不一致，再追 offset flag、到位燈、或移動方向旗標是否未成立。

若未來在其他分支驗證到更多狀態碼，請補充：

- 狀態碼數值
- 對應常數名稱
- 是否屬於未測 IC、Null IC、Clean Pad、測後 bin
- 哪些模組會直接以 `HasIC()` 或 `HasRealIC()` 判讀它

## 6. SaveTaskList() 與 Task_ListWithTime.csv 欄位完整對應表

### 程式位置與呼叫時機

**源碼位置**：
- 函式定義：`main.cpp` TfMain::SaveTaskList()  
  - 版本 V3.33.825.0：第 5578 行起
  - 版本 V3.33.878.0：預估類似位置

**呼叫時機**：
- StateRecord 收集完成時
- 調用來自：DoExecution() 收集 StateRecord 事件時

**輸出位置**：
- StateRecord 資料夾下：`Task_ListWithTime.csv`

### 完整欄位記錄順序

SaveTaskList() **按以下順序**記錄到 CSV，共分七大區塊：

#### 區塊 1：Task 歷史時序（共 59 個 Task）

每個 Task 記錄 10 個時間戳 slot：

```
Task1,<時刻>,<時刻>,...,<時刻>
Task2,<時刻>,<時刻>,...,<時刻>
...
Task60,<時刻>,<時刻>,...,<時刻>
```

- 每行表示某個 Task 的執行時間歷史。
- 若該時刻位置無紀錄則留空。
- 最多保留最近 10 次執行的時刻（由 `QueueTaskList[iCount]` 與 `MAX_Q_10=10` 控制）。

#### 區塊 2：主流程控制旗標（22 個）

記錄當次 StateRecord 收集時刻的流程旗標狀態：

| 欄位名 | 含義 | 何時 true |
|--------|------|----------|
| `bPickFromLoader` | 正在從 Loader 取料 | InArm 執行 PickFromLoader Task 時 |
| `bPlaceToHotplate` | 正在放到 Hotplate | InArm 執行 PlaceToHotplate Task 時 |
| `bPickFromHotplate` | 正在從 Hotplate 取料 | InArm/OutArm 執行 PickFromHotplate Task 時 |
| `bDestoryOnSht` | 正在 Shuttle 上吹氣破壞（可能是舊用法） | 特定摧毀流程時 |
| `bPlaceToShuttle2Step` | 使用二段放 Shuttle 流程 | InArm 放 IC 到 Shuttle 時 |
| `bWaitPreciserFinish` | 等待精準定位完成 | 馬達精準模式下 |
| `bRecIndexDropAlarm1` | Index Arm Front 掉料警報記錄 | Front 測後掉料時 |
| `bRecIndexDropAlarm2` | Index Arm Rear 掉料警報記錄 | Rear 測後掉料時 |

#### 區塊 3：吸取與測試需求旗標（12 個）

記錄當次收集時刻的測試臂狀態需求：

| 欄位名 | 含義 | 用途 |
|--------|------|------|
| `fRearNeedSuckIC` | Rear 臂需要吸取 | Rear Index 準備測試 |
| `fFrontNeedSuck` | Front 臂需要吸取（舊名） | Front Index 準備 |
| `fFrontNeedDestroy` | Front 臂需要破壞吹氣 | Front 測後清理 |
| `fFrontNeedSuckIC` | Front 臂需要吸取 IC | Front Index 準備測試 |
| `fRearNeedSuck` | Rear 臂需要吸取（舊名） | Rear Index 準備 |
| `fRearNeedDestroy` | Rear 臂需要破壞（摧毀） | Rear 測後清理 |
| `f32SiteNeedDestroy` | 32-Site 模式需破壞 | 32-Site 配置下測後清理 |
| `f32SiteNeedSuck` | 32-Site 模式需吸取 | 32-Site 配置下準備測試 |
| `fRearNeedTest` | Rear 臂準備測試 | Rear 測試完整邏輯 |
| `fFrontNeedTest` | Front 臂準備測試 | Front 測試完整邏輯 |
| `fTwoArmNeedTest` | 雙臂同時測試 | 32-Site 模式下同時測 |

#### 區塊 4：Auto Clean 流程控制旗標（6 個）

記錄自動清洗流程的鎖定狀態：

| 欄位名 | 含義 |
|--------|------|
| `bLockPlaceToShuttleByAutoClean` | 清洗中禁止放料到 Shuttle |
| `bLockPickFromShuttleByAutoClean` | 清洗中禁止從 Shuttle 取料 |
| `bPlaceToCleanKit` | 正在放料到清洗 Kit |
| `bPickFromKitByAutoClean` | 清洗機制正在從 Kit 取料 |
| `bPlaceToShuttleByAutoClean` | 清洗機制正在放料到 Shuttle |
| `bPickFromShuttleByAutoClean` | 清洗機制正在從 Shuttle 取料 |

#### 區塊 5：InArm 狀態指示（4 個）

記錄 InArm 當次決策時刻的選擇狀態：

| 欄位名 | 含義 | 取值範圍 |
|--------|------|---------|
| `InArmSuck.iWhichSht` | InArm 下一次將放到哪個 Shuttle | 1 或 2（對應 InSHT1 或 InSHT2） |
| `InArmSuck.iWhichKit` | InArm 下一次目的地是哪個 CarryKit | FRCarryKit / BRCarryKit / 其他 |
| `InArmSuck.iWhichShtPickFor32` | 32-Site 模式下 InArm 預定從哪個 Shuttle 取 | 1 或 2 |
| `InArmSuck.iWhichKitPickFor32` | 32-Site 模式下 InArm 預定從哪個 Kit 取 | FRCarryKit / BRCarryKit / 其他 |

#### 區塊 6：Clean 流程檢查旗標（3 個）

記錄清洗流程檢查狀態：

| 欄位名 | 含義 |
|--------|------|
| `bCheckShuttle1Flag` | 清洗中是否檢查 Shuttle1 |
| `bCheckShuttle2Flag` | 清洗中是否檢查 Shuttle2 |
| `NeedWaitTrayArm` | 是否需要等待 Tray Arm(CatchTray) 放盤完成 |

#### 區塊 7：馬達狀態快照（單一行或多行，依據啟用馬達數）

記錄所有啟用狀態的馬達當下狀態：

```
MOT[MotorAlias], fCanMove=<true/false>, fCanMoveR=<true/false>, fCanMoveM=<true/false>, fCanMoveL=<true/false>, CMD=<位置值>, Encoder=<編碼器值>, Speed=<速度值>
```

- `fCanMove` 能否移動
- `fCanMoveR` 能否向右移動
- `fCanMoveM` 能否中間移動（某些馬達）
- `fCanMoveL` 能否向左移動

## 7. Level 2 關鍵決策變數記錄（DecisionVariables.csv）

### 檔案說明

從 **V3.33.902.0** 版本開始，StateRecord 資料夾內會新增 **`DecisionVariables.csv`** 檔案。

這是 **Level 2 決策變數記錄**，與 `Task_ListWithTime.csv` 分離，專門記錄「影響流程分支判斷的關鍵變數」。

### 為何需要 Level 2 記錄

在分析 StateRecord 時，常遇到以下情境：

- **問題**：看到 OutArmTask=1200，但不知道 `iCloseSiteModeFor2x8` 執行時是什麼值（30002 STM 模式？還是 40002 SPIL 模式？）
- **影響**：無法確認 OutArm 使用哪組映射表（Y=0~3 還是 Y=4~7）
- **時間成本**：需要 30+ 分鐘反覆查原始碼、假設驗證

**解決方案**：記錄關鍵決策變數到獨立檔案，分析時直接查表。

### 階層式記錄架構

| 層級 | 內容 | 檔案 | 用途 |
|------|------|------|------|
| Level 1 | Task 歷史 + 旗標 + 持料快照 | Task_ListWithTime.csv | 流程狀態 |
| Level 2 | 關鍵決策變數 | DecisionVariables.csv | 分支判斷 |

### DecisionVariables.csv 記錄的變數類型

約 20+ 個變數，分為 6 大類：

#### 1. 流程模式類
- `iInArmType` - InArm 機構類型（如 26 = e9045_2x8_8）
- `iCloseSiteModeFor2x8` - 2x8 吸嘴模式（30002=STM, 40002=SPIL）
- `InArmSuck.iModeX/iXStep/iYStep` - InArm 吸取配置
- `OutArmSuck` 相關配置
- `bSingleInArm` - 單臂模式旗標

#### 2. Shuttle / Kit 選擇類
- `InArmSuck.iWhichShuttle/iWhichKit` - InArm 目標選擇
- `iInArmiWhichKit` - InArm Kit 選擇
- `OutArmSuck.iWhichSht/iWhichKit` - OutArm 目標選擇
- `iOutArmiWhichKit` - OutArm 左右側切換（0=左側 Y=0~3, 1=右側 Y=4~7）

#### 3. 測試模式與狀態
- `iRunStartMode` - 運行模式（0=連續, 1=Online, 2=Offline）
- `LastSet.iTester` - Tester 型號
- `LastSet.iRealDummy` - Dummy/Real 模式
- `bUse32SiteMode` - 32-Site 模式旗標

#### 4. Config 關鍵旗標
- `IniConfig.bD30EnableSiteModeSelect` - Site Mode 選擇功能
- `IniConfig.bSingleInArmFunction` - 單臂功能啟用
- `IniConfig.bNewResetFunction` - 新版 Reset 功能

#### 5. 其他關鍵資訊
- `TestIF_File.iShuttleMode` - Shuttle 運作模式
- `TestIF_File.iShuttle_Sel` - Shuttle 選擇
- `CUSTOMER_CODE` - 客戶代碼

#### 6. 座標 Scale 參數
- `OutArmSuck.iXStep/iYStep` - OutArm X/Y 步距
- `Prod.Tray_X_Pitch[0]/Tray_Y_Pitch[0]` - Tray 間距

### 檔案格式範例

```csv
# HT9045 StateRecord - Decision Variables (Level 2)
# Generated: 2026-04-17 14:23:05

# === Flow Mode Variables ===
iInArmType, 26
iCloseSiteModeFor2x8, 30002
iOutArmiWhichKit, 0
InArmSuck.iModeX, 123

# === Shuttle / Kit Selection ===
InArmSuck.iWhichShuttle, 1
InArmSuck.iWhichKit, 0

# === Test Mode and Status ===
iRunStartMode, 0
LastSet.iTester, 1
CUSTOMER_CODE, 1032
```

### 實戰使用範例

**情境**：分析 OutArm 2x8 取料問題，OutArmTask 停在 case 1200

**原始流程**（無 Level 2）：
1. 查看 Task_ListWithTime.csv，發現 OutArmTask=1200
2. 查原始碼 `aoutarm9045_2x8_8.cpp` case 1200
3. 發現需要知道 `iCloseSiteModeFor2x8` 變數值
4. **無法得知該變數實際值**
5. 猜測假設（30002 或 40002）
6. 每種假設檢查一次
7. ⏱ **耗時 30+ 分鐘**

**新流程**（有 Level 2）：
1. 查看 Task_ListWithTime.csv，發現 OutArmTask=1200
2. **查看 DecisionVariables.csv**
3. **直接得知 `iCloseSiteModeFor2x8=30002`（STM 模式）**
4. 確認 Y=0~3, 左側吸取
5. 定位問題
6. ⏱ **耗時 5 分鐘**

**效率提升**：6 倍（30min → 5min）

### 變數值查表

詳細的 enum 值對應表請參考 **[decision-variables-registry.md](decision-variables-registry.md)**。

**常用查表範例**：

#### iCloseSiteModeFor2x8
- `30002` = STM 模式（Y=0~3）
- `40002` = SPIL 模式（Y=4~7）

#### iInArmType
- `26` = e9045_2x8_8（2x8 模式，8 個吸嘴）

#### iOutArmiWhichKit
- `0` = 左側吸取（Y=0~3）
- `1` = 右側吸取（Y=4~7）

#### iRunStartMode
- `0` = 連續開機模式
- `1` = Online 模式
- `2` = Offline 模式

### 維護原則

**程式碼即文件**：
- 唯一真相來源：`main.cpp` 的 `SaveDecisionVariables()` 函數
- 查詢變數清單：看最近一次產生的 `DecisionVariables.csv` 或看 `main.cpp` 原始碼
- 補充變數流程：發現需要新變數 → 修改 main.cpp → 編譯測試 → 完成

### 何時使用 DecisionVariables.csv

分析 StateRecord 時，若遇到以下情境，應優先查看 DecisionVariables.csv：

- ✅ 需要確認 InArm/OutArm 使用哪種模式（2x8 / 1x1 / STM / SPIL）
- ✅ 需要確認左右側切換狀態（iOutArmiWhichKit）
- ✅ 需要確認運行模式（連續/Online/Offline）
- ✅ 需要確認客戶代碼或特殊功能旗標
- ✅ 無法從 Task_ListWithTime.csv 推測執行路徑時

### 限制與注意事項

1. **版本限制**：V3.33.902.0 以前的版本不會產生 DecisionVariables.csv
2. **僅記錄決策變數**：不記錄中間計算結果或衍生變數
3. **快照時間點**：與 Task_ListWithTime.csv 同一時間點，代表當時「最後執行狀態」
4. **變數數量**：約 20+ 個，未來可按需擴充

---

**實施日期**：2026-04-17  
**適用版本**：V3.33.902.0+  
**技術報告**：`D:\00_Weekly Report\2026\04\20260417\RD5軟體_TechReport_StateRecord_Level2_20260417.md`
- `CMD` 命令位置（pulses）
- `Encoder` 編碼器反饋（pulses）
- `Speed` 當前速度設定

#### 區塊 8：六個持料快照矩陣

依序記錄當次 StateRecord 收集時刻的持有 IC 資訊矩陣：

```
InArmSuck
<row1: col0, col1, col2, col3...>
<row2: col0, col1, col2, col3...>
...

OutArmSuck
<row1: col0, col1, col2, col3...>
<row2: col0, col1, col2, col3...>
...

FRCarryKit
<row1: col0, col1, col2, col3, col4, col5, col6, col7>
<row2: col0, col1, col2, col3, col4, col5, col6, col7>

BRCarryKit
<row1: col0, col1, col2, col3, col4, col5, col6, col7>
<row2: col0, col1, col2, col3, col4, col5, col6, col7>

FTestSuck
<row1: col0, col1, col2, col3, col4, col5, col6, col7>
<row2: col0, col1, col2, col3, col4, col5, col6, col7>

BTestSuck
<row1: col0, col1, col2, col3, col4, col5, col6, col7>
<row2: col0, col1, col2, col3, col4, col5, col6, col7>
```

各矩陣大小：
- `InArmSuck`：2 行 × 4 列（2 排吸嘴，各 4 分割位）
- `OutArmSuck`：2 行 × 4 列
- `FRCarryKit`：2 行 × 8 列（Front Right，2 排各 8 site）
- `BRCarryKit`：2 行 × 8 列（Back Right）
- `FTestSuck`：2 行 × 8 列（Front Test Socket，8 site × 2 層或 2 arm × 8 site）
- `BTestSuck`：2 行 × 8 列（Back Test Socket）

#### 區塊 9：Clean Kit 時間紀錄

記錄自動清洗 Kit 各 Site 最後一次清洗時間：

```
CleanKitTime
<Y座標0: Site_00_CleanTime, Site_01_CleanTime, ..., Site_0N_CleanTime>
<Y座標1: Site_10_CleanTime, Site_11_CleanTime, ..., Site_1N_CleanTime>
...
```

- 每個 Site 記錄「年月日-時分秒」或 `na`（未清）。

### 實戰讀法範例

#### 例 1：判斷 InArm 是否選定了 Shuttle1 作為下一個目標

查看 CSV 中的 `InArmSuck.iWhichSht` 行：

```
InArmSuck.iWhichSht, 1
```

值為 `1` 表示 InArm 已決定下一步放到 Shuttle1。

#### 例 2：判斷 OutArm 當時是否有料

查看 CSV 中的 `OutArmSuck` 矩陣區塊：

```
OutArmSuck
0,0,0,0
0,0,0,0
```

全 0 表示 OutArm 手上是空的，沒有 IC。

#### 例 3：判斷 Front Right CarryKit 是否還有測後廢品留下

查看 CSV 中的 `FRCarryKit` 矩陣：

```
FRCarryKit
0,0,1002,0,0,0,1002,0
0,0,0,0,0,0,0,0
```

第一行第 3 位和第 7 位顯示 `1002`（測後 bin 資訊），表示還有廢品卡在那裡。

#### 例 4：判斷 Clean 流程是否在執行中

查看 CSV 中的以下旗標：

```
bLockPlaceToShuttleByAutoClean, true
bLockPickFromShuttleByAutoClean, true
bPlaceToCleanKit, true
```

若這些都是 `true`，表示自動清洗流程正在執行，主流程被鎖定。

#### 例 5：判斷 InArm 是從 Loader 還是從 CarryKit 準備取下一批

查看 CSV 中的 `bPickFromLoader`：

```
bPickFromLoader, true
```

若為 `true`，表示 InArm 當時決定從 Loader 取料；若 `false`，可能是準備從 CarryKit 或 Shuttle 取。

### 常見問題排查流程

1. **機台停住，InArm 似乎沒有動作**
   - 先看 `bPickFromLoader`、`bPlaceToHotplate`、`bPickFromHotplate` 是否有真的執行
   - 再看 `InArmSuck` 矩陣是否確實有料
   - 若旗標都是 true 但馬達 Encoder 沒動，才確認是馬達故障

2. **測試區料沒清掉，一直卡在那邊**
   - 查看 CleanKitTime 是否有更新
   - 查看 `f32SiteNeedDestroy` / `fFrontNeedDestroy` / `fRearNeedDestroy` 是否真的為 true
   - 查看 `FTestSuck` / `BTestSuck` 是否仍有資料

3. **Shuttle 的料一直沒被 OutArm 拿走**
   - 查看 `OutArmSuck` 是否為空（如果空，OutArm 沒有成功 pick）
   - 查看 `FRCarryKit` / `BRCarryKit` 是否有對應的料（如果有，代表曾經 pick 過，但可能沒有完成放置）
   - 對比 `bPickFromShuttleByAutoClean` 狀態判斷是否被 Auto Clean 鎖定

4. **32-Site 模式 InArm 選擇異常**
   - 查看 `InArmSuck.iWhichShtPickFor32` 與 `InArmSuck.iWhichKitPickFor32` 是否符合預期配置
   - 查看 `f32SiteNeedSuck` / `f32SiteNeedDestroy` 是否正確觸發