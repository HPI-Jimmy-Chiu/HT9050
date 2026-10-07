# MotorProfiler 校正表規格

按需要選取以下章節，原文依順序保留。

- [MotorProfiler 校正表規格](uph-motor-time-table/00.md)
- [0. 架構總覽（三方混合：埋點 + uMotorTest 補位 + Excel）](uph-motor-time-table/01.md)
- [1. 校正策略（適用於 Phase 1B 補位掃描）](uph-motor-time-table/02.md)
- [2. CSV 檔案規格](uph-motor-time-table/03.md)
- [3. 程式架構](uph-motor-time-table/04.md)
- [4. 全軸自動歸零](uph-motor-time-table/05.md)
- [5. 移動區間：Teach 座標 + 手動覆蓋](uph-motor-time-table/06.md)
- [6. Speed 與 Acc 區間選擇](uph-motor-time-table/07.md)
- [7. 安全門中斷 → 重新初始化](uph-motor-time-table/08.md)
- [8. 操作介面元件（tsUPHProfiler Tab — 四個子分頁）](uph-motor-time-table/09.md)
- [9. Excel 端 UPH 計算器（不寫 code）](uph-motor-time-table/10.md)
- [10. 後續擴充（Phase 3+）](uph-motor-time-table/11.md)
- [11. Phase 1A 流程埋點實作](uph-motor-time-table/12.md)

# MotorProfiler 校正表規格

[讀取此節](uph-motor-time-table/00.md#motorprofiler-校正表規格)

## 0. 架構總覽（三方混合：埋點 + uMotorTest 補位 + Excel）

[讀取此節](uph-motor-time-table/01.md#0-架構總覽三方混合埋點--umotortest-補位--excel)

### 為何三方混合最強

[讀取此節](uph-motor-time-table/01.md#為何三方混合最強)

### 共用資料介面

[讀取此節](uph-motor-time-table/01.md#共用資料介面)

### 實作優先順序

[讀取此節](uph-motor-time-table/01.md#實作優先順序)

## 1. 校正策略（適用於 Phase 1B 補位掃描）

[讀取此節](uph-motor-time-table/02.md#1-校正策略適用於-phase-1b-補位掃描)

### 距離模式：Teach 座標導出（取代固定距離）

[讀取此節](uph-motor-time-table/02.md#距離模式teach-座標導出取代固定距離)

### InArm（使用 `InArmContinuousMove_9045` 完整移動原語）

[讀取此節](uph-motor-time-table/02.md#inarm使用-inarmcontinuousmove_9045-完整移動原語)

### Index（MTestY1/Y2 + MTestZ1/Z2 + MInShuttle1/2，**8 步交替 cycle**）

[讀取此節](uph-motor-time-table/02.md#indexmtesty1y2--mtestz1z2--minshuttle128-步交替-cycle)

#### 真實 cycle 序列（Arm1 / Arm2 交替）

[讀取此節](uph-motor-time-table/02.md#真實-cycle-序列arm1--arm2-交替)

#### 對應量測項

[讀取此節](uph-motor-time-table/02.md#對應量測項)

#### Z 高度來源（cContact.cpp DeviceForm_File）

[讀取此節](uph-motor-time-table/02.md#z-高度來源ccontactcpp-deviceform_file)

### 為何不能簡化成單軸 Z 量測

[讀取此節](uph-motor-time-table/02.md#為何不能簡化成單軸-z-量測)

### OutArm（使用 `OutArmContinuousMove_9045` 完整移動原語）

[讀取此節](uph-motor-time-table/02.md#outarm使用-outarmcontinuousmove_9045-完整移動原語)

### TrayArm / Loader/Unloader Z / Rotate

[讀取此節](uph-motor-time-table/02.md#trayarm--loaderunloader-z--rotate)

### Speed 維度

[讀取此節](uph-motor-time-table/02.md#speed-維度)

### Acc 維度（Phase 1 即支援）

[讀取此節](uph-motor-time-table/02.md#acc-維度phase-1-即支援)

### 重複次數

[讀取此節](uph-motor-time-table/02.md#重複次數)

### 總量（Speed-only 範例）

[讀取此節](uph-motor-time-table/02.md#總量speed-only-範例)

## 2. CSV 檔案規格

[讀取此節](uph-motor-time-table/03.md#2-csv-檔案規格)

### 主檔欄位

[讀取此節](uph-motor-time-table/03.md#主檔欄位)

### 檔名規則

[讀取此節](uph-motor-time-table/03.md#檔名規則)

### 過程 Log

[讀取此節](uph-motor-time-table/03.md#過程-log)

### 存放路徑

[讀取此節](uph-motor-time-table/03.md#存放路徑)

### 不寫入 MDB / Eventlog

[讀取此節](uph-motor-time-table/03.md#不寫入-mdb--eventlog)

## 3. 程式架構

[讀取此節](uph-motor-time-table/04.md#3-程式架構)

### 整合方式：新增 Tab 進 uMotorTest（不新增獨立 Form）

[讀取此節](uph-motor-time-table/04.md#整合方式新增-tab-進-umotortest不新增獨立-form)

### 修改範圍

[讀取此節](uph-motor-time-table/04.md#修改範圍)

### 不需要 `#define`

[讀取此節](uph-motor-time-table/04.md#不需要-define)

## 4. 全軸自動歸零

[讀取此節](uph-motor-time-table/05.md#4-全軸自動歸零)

### 歸零策略

[讀取此節](uph-motor-time-table/05.md#歸零策略)

### Task 狀態機嵌入

[讀取此節](uph-motor-time-table/05.md#task-狀態機嵌入)

## 5. 移動區間：Teach 座標 + 手動覆蓋

[讀取此節](uph-motor-time-table/06.md#5-移動區間teach-座標--手動覆蓋)

### 距離來源

[讀取此節](uph-motor-time-table/06.md#距離來源)

### Teach 帶入邏輯

[讀取此節](uph-motor-time-table/06.md#teach-帶入邏輯)

### 使用者可覆蓋

[讀取此節](uph-motor-time-table/06.md#使用者可覆蓋)

## 6. Speed 與 Acc 區間選擇

[讀取此節](uph-motor-time-table/07.md#6-speed-與-acc-區間選擇)

### Phase 1 即同時支援 Speed + Acc

[讀取此節](uph-motor-time-table/07.md#phase-1-即同時支援-speed--acc)

### Scan 迴圈結構

[讀取此節](uph-motor-time-table/07.md#scan-迴圈結構)

## 7. 安全門中斷 → 重新初始化

[讀取此節](uph-motor-time-table/08.md#7-安全門中斷--重新初始化)

### 每 Task step 入口檢查

[讀取此節](uph-motor-time-table/08.md#每-task-step-入口檢查)

### 中斷後流程

[讀取此節](uph-motor-time-table/08.md#中斷後流程)

### 為何必須重新歸零

[讀取此節](uph-motor-time-table/08.md#為何必須重新歸零)

### 狀態流程圖

[讀取此節](uph-motor-time-table/08.md#狀態流程圖)

## 8. 操作介面元件（tsUPHProfiler Tab — 四個子分頁）

[讀取此節](uph-motor-time-table/09.md#8-操作介面元件tsuphprofiler-tab--四個子分頁)

### 8.1 tabLogger — Phase 1A 控制面板

[讀取此節](uph-motor-time-table/09.md#81-tablogger--phase-1a-控制面板)

### 8.2 tabCoverage — Coverage Map 熱圖

[讀取此節](uph-motor-time-table/09.md#82-tabcoverage--coverage-map-熱圖)

### 8.3 tabPredict — UPH 即時試算

[讀取此節](uph-motor-time-table/09.md#83-tabpredict--uph-即時試算)

### 8.4 tabSupplement — Smart 補位掃描（沿用原 scan queue）

[讀取此節](uph-motor-time-table/09.md#84-tabsupplement--smart-補位掃描沿用原-scan-queue)

### 啟動按鈕邏輯

[讀取此節](uph-motor-time-table/09.md#啟動按鈕邏輯)

## 9. Excel 端 UPH 計算器（不寫 code）

[讀取此節](uph-motor-time-table/10.md#9-excel-端-uph-計算器不寫-code)

## 10. 後續擴充（Phase 3+）

[讀取此節](uph-motor-time-table/11.md#10-後續擴充phase-3)

## 11. Phase 1A 流程埋點實作

[讀取此節](uph-motor-time-table/12.md#11-phase-1a-流程埋點實作)

### 11.1 設計原則

[讀取此節](uph-motor-time-table/12.md#111-設計原則)

### 11.2 新增檔案

[讀取此節](uph-motor-time-table/12.md#112-新增檔案)

### 11.3 IniConfig 欄位（已完成）

[讀取此節](uph-motor-time-table/12.md#113-iniconfig-欄位已完成)

### 11.4 埋點位置清單

[讀取此節](uph-motor-time-table/12.md#114-埋點位置清單)

### 11.5 埋點範例（InArm）

[讀取此節](uph-motor-time-table/12.md#115-埋點範例inarm)

### 11.6 CSV append 與 buffer 策略

[讀取此節](uph-motor-time-table/12.md#116-csv-append-與-buffer-策略)

### 11.7 與 Phase 1B 的互動

[讀取此節](uph-motor-time-table/12.md#117-與-phase-1b-的互動)

### 11.8 風險評估

[讀取此節](uph-motor-time-table/12.md#118-風險評估)
