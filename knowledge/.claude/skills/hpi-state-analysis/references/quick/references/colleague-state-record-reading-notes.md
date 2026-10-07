# StateRecord 實戰判讀補充

按需要選取以下章節，原文依順序保留。

- [StateRecord 實戰判讀補充](colleague-state-record-reading-notes/00.md)
- [1. Input Arm 正常等待 Shuttle 回左，不應直接列為主嫌](colleague-state-record-reading-notes/01.md)
- [2. Task_ListWithTime 底部持料區塊可直接讀出當下資料狀態](colleague-state-record-reading-notes/02.md)
- [3. 實戰案例總結](colleague-state-record-reading-notes/03.md)
- [4. 維護原則](colleague-state-record-reading-notes/04.md)
- [5. 如何判斷 Shuttle 在左邊還是右邊](colleague-state-record-reading-notes/05.md)
- [6. SaveTaskList() 與 Task_ListWithTime.csv 欄位完整對應表](colleague-state-record-reading-notes/06.md)
- [7. Level 2 關鍵決策變數記錄（DecisionVariables.csv）](colleague-state-record-reading-notes/07.md)

# StateRecord 實戰判讀補充

[讀取此節](colleague-state-record-reading-notes/00.md#staterecord-實戰判讀補充)

## 1. Input Arm 正常等待 Shuttle 回左，不應直接列為主嫌

[讀取此節](colleague-state-record-reading-notes/01.md#1-input-arm-正常等待-shuttle-回左不應直接列為主嫌)

### 何時可以先排除 InArm

[讀取此節](colleague-state-record-reading-notes/01.md#何時可以先排除-inarm)

### 實務判準

[讀取此節](colleague-state-record-reading-notes/01.md#實務判準)

## 2. Task_ListWithTime 底部持料區塊可直接讀出當下資料狀態

[讀取此節](colleague-state-record-reading-notes/02.md#2-task_listwithtime-底部持料區塊可直接讀出當下資料狀態)

### 可直接使用的欄位

[讀取此節](colleague-state-record-reading-notes/02.md#可直接使用的欄位)

### 常用狀態碼

[讀取此節](colleague-state-record-reading-notes/02.md#常用狀態碼)

### 例子

[讀取此節](colleague-state-record-reading-notes/02.md#例子)

### 用法建議

[讀取此節](colleague-state-record-reading-notes/02.md#用法建議)

### 進階推論：從持料快照推論取料位置判斷錯誤

[讀取此節](colleague-state-record-reading-notes/02.md#進階推論從持料快照推論取料位置判斷錯誤)

#### 症狀組合

[讀取此節](colleague-state-record-reading-notes/02.md#症狀組合)

#### 推論邏輯

[讀取此節](colleague-state-record-reading-notes/02.md#推論邏輯)

#### 定位步驟

[讀取此節](colleague-state-record-reading-notes/02.md#定位步驟)

#### 實戰案例（2026-04-17）

[讀取此節](colleague-state-record-reading-notes/02.md#實戰案例2026-04-17)

#### 推論技巧總結

[讀取此節](colleague-state-record-reading-notes/02.md#推論技巧總結)

## 3. 實戰案例總結

[讀取此節](colleague-state-record-reading-notes/03.md#3-實戰案例總結)

## 4. 維護原則

[讀取此節](colleague-state-record-reading-notes/04.md#4-維護原則)

## 5. 如何判斷 Shuttle 在左邊還是右邊

[讀取此節](colleague-state-record-reading-notes/05.md#5-如何判斷-shuttle-在左邊還是右邊)

### 程式裡的正式判斷函式

[讀取此節](colleague-state-record-reading-notes/05.md#程式裡的正式判斷函式)

### 判斷邏輯不是只看馬達目前位置

[讀取此節](colleague-state-record-reading-notes/05.md#判斷邏輯不是只看馬達目前位置)

#### Left 判斷

[讀取此節](colleague-state-record-reading-notes/05.md#left-判斷)

#### Right 判斷

[讀取此節](colleague-state-record-reading-notes/05.md#right-判斷)

### 對 StateRecord / Motor.xls 的實戰讀法

[讀取此節](colleague-state-record-reading-notes/05.md#對-staterecord--motorxls-的實戰讀法)

### 本案中的實務用法

[讀取此節](colleague-state-record-reading-notes/05.md#本案中的實務用法)

### 分析時的建議順序

[讀取此節](colleague-state-record-reading-notes/05.md#分析時的建議順序)

## 6. SaveTaskList() 與 Task_ListWithTime.csv 欄位完整對應表

[讀取此節](colleague-state-record-reading-notes/06.md#6-savetasklist-與-task_listwithtimecsv-欄位完整對應表)

### 程式位置與呼叫時機

[讀取此節](colleague-state-record-reading-notes/06.md#程式位置與呼叫時機)

### 完整欄位記錄順序

[讀取此節](colleague-state-record-reading-notes/06.md#完整欄位記錄順序)

#### 區塊 1：Task 歷史時序（共 59 個 Task）

[讀取此節](colleague-state-record-reading-notes/06.md#區塊-1task-歷史時序共-59-個-task)

#### 區塊 2：主流程控制旗標（22 個）

[讀取此節](colleague-state-record-reading-notes/06.md#區塊-2主流程控制旗標22-個)

#### 區塊 3：吸取與測試需求旗標（12 個）

[讀取此節](colleague-state-record-reading-notes/06.md#區塊-3吸取與測試需求旗標12-個)

#### 區塊 4：Auto Clean 流程控制旗標（6 個）

[讀取此節](colleague-state-record-reading-notes/06.md#區塊-4auto-clean-流程控制旗標6-個)

#### 區塊 5：InArm 狀態指示（4 個）

[讀取此節](colleague-state-record-reading-notes/06.md#區塊-5inarm-狀態指示4-個)

#### 區塊 6：Clean 流程檢查旗標（3 個）

[讀取此節](colleague-state-record-reading-notes/06.md#區塊-6clean-流程檢查旗標3-個)

#### 區塊 7：馬達狀態快照（單一行或多行，依據啟用馬達數）

[讀取此節](colleague-state-record-reading-notes/06.md#區塊-7馬達狀態快照單一行或多行依據啟用馬達數)

## 7. Level 2 關鍵決策變數記錄（DecisionVariables.csv）

[讀取此節](colleague-state-record-reading-notes/07.md#7-level-2-關鍵決策變數記錄decisionvariablescsv)

### 檔案說明

[讀取此節](colleague-state-record-reading-notes/07.md#檔案說明)

### 為何需要 Level 2 記錄

[讀取此節](colleague-state-record-reading-notes/07.md#為何需要-level-2-記錄)

### 階層式記錄架構

[讀取此節](colleague-state-record-reading-notes/07.md#階層式記錄架構)

### DecisionVariables.csv 記錄的變數類型

[讀取此節](colleague-state-record-reading-notes/07.md#decisionvariablescsv-記錄的變數類型)

#### 1. 流程模式類

[讀取此節](colleague-state-record-reading-notes/07.md#1-流程模式類)

#### 2. Shuttle / Kit 選擇類

[讀取此節](colleague-state-record-reading-notes/07.md#2-shuttle--kit-選擇類)

#### 3. 測試模式與狀態

[讀取此節](colleague-state-record-reading-notes/07.md#3-測試模式與狀態)

#### 4. Config 關鍵旗標

[讀取此節](colleague-state-record-reading-notes/07.md#4-config-關鍵旗標)

#### 5. 其他關鍵資訊

[讀取此節](colleague-state-record-reading-notes/07.md#5-其他關鍵資訊)

#### 6. 座標 Scale 參數

[讀取此節](colleague-state-record-reading-notes/07.md#6-座標-scale-參數)

### 檔案格式範例

[讀取此節](colleague-state-record-reading-notes/07.md#檔案格式範例)

# HT9045 StateRecord - Decision Variables (Level 2)

[讀取此節](colleague-state-record-reading-notes/07.md#ht9045-staterecord---decision-variables-level-2)

# Generated: 2026-04-17 14:23:05

[讀取此節](colleague-state-record-reading-notes/07.md#generated-2026-04-17-142305)

# === Flow Mode Variables ===

[讀取此節](colleague-state-record-reading-notes/07.md#-flow-mode-variables-)

# === Shuttle / Kit Selection ===

[讀取此節](colleague-state-record-reading-notes/07.md#-shuttle--kit-selection-)

# === Test Mode and Status ===

[讀取此節](colleague-state-record-reading-notes/07.md#-test-mode-and-status-)

### 實戰使用範例

[讀取此節](colleague-state-record-reading-notes/07.md#實戰使用範例)

### 變數值查表

[讀取此節](colleague-state-record-reading-notes/07.md#變數值查表)

#### iCloseSiteModeFor2x8

[讀取此節](colleague-state-record-reading-notes/07.md#iclosesitemodefor2x8)

#### iInArmType

[讀取此節](colleague-state-record-reading-notes/07.md#iinarmtype)

#### iOutArmiWhichKit

[讀取此節](colleague-state-record-reading-notes/07.md#ioutarmiwhichkit)

#### iRunStartMode

[讀取此節](colleague-state-record-reading-notes/07.md#irunstartmode)

### 維護原則

[讀取此節](colleague-state-record-reading-notes/07.md#維護原則)

### 何時使用 DecisionVariables.csv

[讀取此節](colleague-state-record-reading-notes/07.md#何時使用-decisionvariablescsv)

### 限制與注意事項

[讀取此節](colleague-state-record-reading-notes/07.md#限制與注意事項)

#### 區塊 8：六個持料快照矩陣

[讀取此節](colleague-state-record-reading-notes/07.md#區塊-8六個持料快照矩陣)

#### 區塊 9：Clean Kit 時間紀錄

[讀取此節](colleague-state-record-reading-notes/07.md#區塊-9clean-kit-時間紀錄)

### 實戰讀法範例

[讀取此節](colleague-state-record-reading-notes/07.md#實戰讀法範例)

#### 例 1：判斷 InArm 是否選定了 Shuttle1 作為下一個目標

[讀取此節](colleague-state-record-reading-notes/07.md#例-1判斷-inarm-是否選定了-shuttle1-作為下一個目標)

#### 例 2：判斷 OutArm 當時是否有料

[讀取此節](colleague-state-record-reading-notes/07.md#例-2判斷-outarm-當時是否有料)

#### 例 3：判斷 Front Right CarryKit 是否還有測後廢品留下

[讀取此節](colleague-state-record-reading-notes/07.md#例-3判斷-front-right-carrykit-是否還有測後廢品留下)

#### 例 4：判斷 Clean 流程是否在執行中

[讀取此節](colleague-state-record-reading-notes/07.md#例-4判斷-clean-流程是否在執行中)

#### 例 5：判斷 InArm 是從 Loader 還是從 CarryKit 準備取下一批

[讀取此節](colleague-state-record-reading-notes/07.md#例-5判斷-inarm-是從-loader-還是從-carrykit-準備取下一批)

### 常見問題排查流程

[讀取此節](colleague-state-record-reading-notes/07.md#常見問題排查流程)
