# StateRecord Decision Variables Registry

## 文件說明

本文件記錄 HT9045 StateRecord **Level 2 關鍵決策變數清單**。

這些變數儲存於 `DecisionVariables.csv`，與 `Task_ListWithTime.csv` 分離，用於記錄「影響流程分支判斷的關鍵變數」。

**維護原則**：
- ✅ 程式碼 (`main.cpp` 的 `SaveDecisionVariables()`) 是唯一真相來源
- ✅ 本檔案用於「查表解讀」（例如 enum 值對應）
- ✅ 不強制同步，發現查表需求時才更新

---

## 快速查表：常用變數解讀

### iCloseSiteModeFor2x8（2x8 吸嘴模式）

| 數值 | 對應 Enum | 說明 |
|------|----------|------|
| 0 | 標準模式 | 未使用 Close Site |
| 10000 | 標準4列 | 上下4列 |
| 10002 | 標準8列 | 上下8列 |
| 20000 | 標準8列 | 上下8列 |
| 20002 | 標準4列 | 上下4列 |
| 30002 | `e2x8_STMMode` | STM 8 site Auto clean 專用 |
| 31000 | `e2x8_STMMode` (右側) | STM 右側子模式 |
| 31101 | `e2x8_STMMode` (其他) | STM 其他子模式 |
| 40002 | `e2x8_TW153Mode` | TW153TK 規格 |
| 50002 | `e2x8CloseEven` | 關閉偶數列 |
| 60002 | `e2x8CloseEven1By1` | 關偶數列逐一模式 |
| 70002 | `e2x8CloseOdd` | 關閉奇數列 |
| 80002 | `e2x8CloseOdd1By1` | 關奇數列逐一模式 |
| 90002 | `e2x8Run2x2_13` | 2x2 模式 13 |
| 100002 | `e2x8Run2x2_14` | 2x2 模式 14 |
| 110002 | `e2x8Run2x4Standard` | 2x4 標準 |
| 120002 | `e2x8Run2x4Step2` | 2x4 Step2 |

**實戰範例**：
```
若 iCloseSiteModeFor2x8=30002 + iOutArmiWhichKit=0
→ OutArm 使用 STM 模式，吸取左側（Y=0~3）
```

---

### iInArmType（InArm 機構類型）

| 數值 | 對應 Enum | 說明 |
|------|----------|------|
| 0 | `e9045_1x1_1` | 1x1 單吸嘴 |
| 8 | `e9045_1x4_4` | 1x4 四吸嘴 |
| 15 | `e9045_2x2_4_13` | 2x2 四吸嘴 (13版本) |
| 26 | `e9045_2x8_8` | 2x8 八吸嘴 |

**查法**：參考 `MachineType.h` 的 `enum eInArmType`

---

### iOutArmiWhichKit（OutArm 左右側切換）

| 數值 | 語義 | 吸取範圍（8-kit模式） |
|------|------|---------------------|
| 0 | 左側 | Y=0~3 |
| 1 | 右側 | Y=4~7 |

---

### iRunStartMode（運行模式）

| 數值 | 對應 Enum | 說明 |
|------|----------|------|
| 0 | `rsmContinuStart` | 連續開機 |
| 1 | `rsmInitialStart` | 初始開機 |
| 2 | `rsmContinuRetest` | 連續重測 |
| 3 | `rsmCInitialRetest` | 初始重測 |
| 11 | `rsmAutoRetest` | Auto Retest 模式 |

---

## 變數分類

### 🔹 1. 流程模式類
- `iInArmType` - InArm 機構類型
- `iCloseSiteModeFor2x8` - 2x8 Close Site 模式
- `InArmSuck.iModeX` / `iXStep` / `iYStep` - InArm 吸嘴配置
- `OutArmSuck.iModeX` / `iXStep` / `iYStep` - OutArm 吸嘴配置
- `bSingleInArm` - 單/雙 InArm 模式

### 🔹 2. Shuttle / Kit 選擇類
- `InArmSuck.iWhichShuttle` / `iWhichKit` - InArm 目標
- `iInArmiWhichKit` - InArm Kit 選擇
- `OutArmSuck.iWhichSht` / `iWhichKit` - OutArm 目標
- `iOutArmiWhichKit` - OutArm 左右側切換

### 🔹 3. 測試模式與狀態類
- `iRunStartMode` - 運行模式
- `LastSet.iTester` - Tester 狀態
- `LastSet.iRealDummy` - 真實/Dummy 模式
- `bUse32SiteMode` - 32 Site 模式

### 🔹 4. Config 關鍵旗標
- `IniConfig.bD30EnableSiteModeSelect` - Site Mode 切換
- `IniConfig.bSingleInArmFunction` - 單 InArm 功能
- `IniConfig.bNewResetFunction` - 新版 Reset
- `IniConfig.bSPILFunction` - SPIL 客戶模式

### 🔹 5. Shuttle / Index 關鍵狀態
- `TestIF_File.iShuttleMode` - Shuttle 測試模式
- `TestIF_File.iShuttle_Sel` - Shuttle 選擇

### 🔹 6. Customer Code
- `CUSTOMER_CODE` - 客戶代碼

---

## 使用範例

### 從 DecisionVariables.csv 讀取變數

```csv
# DecisionVariables.csv 範例
iInArmType, 26
iCloseSiteModeFor2x8, 30002
InArmSuck.iModeX, 2
OutArmSuck.iWhichSht, 0
iOutArmiWhichKit, 0
iRunStartMode, 0
CUSTOMER_CODE, 1032
```

### 查表解讀

1. `iInArmType=26` → 查表得知為 `e9045_2x8_8`（2x8 吸嘴）
2. `iCloseSiteModeFor2x8=30002` → 查表得知為 STM 模式
3. `iOutArmiWhichKit=0` → 左側吸取（Y=0~3）
4. `iRunStartMode=0` → 連續開機模式

---

## 擴充記錄

當發現新的「需要查表解讀」的變數時，在此補充：

| 日期 | 變數名稱 | 新增原因 | 查表需求 |
|------|---------|---------|---------|
| 2026-04-17 | `iCloseSiteModeFor2x8` | 分析 OutArm 2x8 取料問題時需要解讀模式值 | 需要對照 enum 值 |

---

## 維護記錄

| 版本 | 日期 | 修改內容 | 修改者 |
|------|------|---------|--------|
| v1.0 | 2026-04-17 | 初始版本建立 | JerryYang |

---

## 相關文件

- [state-record-reading-notes.md](state-record-reading-notes.md) Section 7 - DecisionVariables 使用說明
- [SKILL.md](../SKILL.md) - StateRecord 分析六步法
- **程式碼真相來源**：`main.cpp` 的 `SaveDecisionVariables()` 函數
