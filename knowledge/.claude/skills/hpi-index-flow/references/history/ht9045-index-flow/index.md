# HT9045 Index (Test Head Motor) Flow Knowledge

> 原始文件拆成 15 個順序片段；依下面導覽讀取需要的段落。機型與版本來源見 [共同與差異](../../common.md)。

- [HT9045 Index (Test Head Motor) Flow Knowledge](index/00.md)
- [適用場景](index/01.md)
- [專案資訊](index/02.md)
- [參考文件](index/03.md)
- [關鍵原始檔](index/04.md)
- [1. 呼叫階層總覽](index/05.md)
- [2. DoTestHeadMotor() Main Flow (atester.cpp)](index/06.md)
- [3. Next Layer: DoTestY() (atester.cpp)](index/07.md)
- [4. Child State Machines](index/08.md)
- [5. Other Direct Child State Machines](index/09.md)
- [6. End-to-End Runtime Flow](index/10.md)
- [7. Notes](index/11.md)
- [8. Index Cycle Time / Test Time 記錄機制（摘要）](index/12.md)
- [9. UPH 記錄機制（摘要）](index/13.md)
- [合併補充：repo 既有參考（20261001）](index/14.md)

# HT9045 Index (Test Head Motor) Flow Knowledge

[讀取此節](index/00.md#ht9045-index-test-head-motor-flow-knowledge)

## 適用場景

[讀取此節](index/01.md#適用場景)

## 專案資訊

[讀取此節](index/02.md#專案資訊)

## 參考文件

[讀取此節](index/03.md#參考文件)

## 關鍵原始檔

[讀取此節](index/04.md#關鍵原始檔)

## 1. 呼叫階層總覽

[讀取此節](index/05.md#1-呼叫階層總覽)

## 2. DoTestHeadMotor() Main Flow (atester.cpp)

[讀取此節](index/06.md#2-dotestheadmotor-main-flow-atestercpp)

### Pre-switch guards

[讀取此節](index/06.md#pre-switch-guards)

### Top-level phase map

[讀取此節](index/06.md#top-level-phase-map)

### Key state groups

[讀取此節](index/06.md#key-state-groups)

#### A. Index head init & vacuum self-check

[讀取此節](index/06.md#a-index-head-init--vacuum-self-check)

#### B. F16 shuttle broken sensor check (optional)

[讀取此節](index/06.md#b-f16-shuttle-broken-sensor-check-optional)

#### C. CCD startup check path

[讀取此節](index/06.md#c-ccd-startup-check-path)

#### D. Core index pre-test check (torque + vacuum + socket)

[讀取此節](index/06.md#d-core-index-pre-test-check-torque--vacuum--socket)

#### E. AutoSiteMap trigger and test execution

[讀取此節](index/06.md#e-autositemap-trigger-and-test-execution)

#### F. Move into run posture and enter test cycle

[讀取此節](index/06.md#f-move-into-run-posture-and-enter-test-cycle)

#### G. PlaceToShuttleFirst cleanup route

[讀取此節](index/06.md#g-placetoshuttlefirst-cleanup-route)

#### H. RTC FullView / ROI learning route

[讀取此節](index/06.md#h-rtc-fullview--roi-learning-route)

## 3. Next Layer: DoTestY() (atester.cpp)

[讀取此節](index/07.md#3-next-layer-dotesty-atestercpp)

### State machine overview

[讀取此節](index/07.md#state-machine-overview)

### Case summary

[讀取此節](index/07.md#case-summary)

## 4. Child State Machines

[讀取此節](index/08.md#4-child-state-machines)

### 4.1 DoTestYFront() (`aTester_Front.cpp`)

[讀取此節](index/08.md#41-dotestyfront-atester_frontcpp)

### 4.2 DoTestYRear() (`aTester_Rear.cpp`)

[讀取此節](index/08.md#42-dotestyrear-atester_rearcpp)

### 4.3 DoTestY_TwoArm32Site() (`atester_32Site.cpp`)

[讀取此節](index/08.md#43-dotesty_twoarm32site-atester_32sitecpp)

## 5. Other Direct Child State Machines

[讀取此節](index/09.md#5-other-direct-child-state-machines)

### 5.1 DoFrontTestDestroyIC(false) (`aTester_Front.cpp`)

[讀取此節](index/09.md#51-dofronttestdestroyicfalse-atester_frontcpp)

### 5.2 DoRearTestDestroyIC(false) (`aTester_Rear.cpp`)

[讀取此節](index/09.md#52-doreartestdestroyicfalse-atester_rearcpp)

### 5.3 DoFTestSuckTestIC() (`aTester_Front.cpp`)

[讀取此節](index/09.md#53-doftestsucktestic-atester_frontcpp)

### 5.4 DoBTestSuckTestIC() (`aTester_Rear.cpp`)

[讀取此節](index/09.md#54-dobtestsucktestic-atester_rearcpp)

### 5.5 DoTestSuckTestIC_TwoArm32Site() (`atester_32Site.cpp`)

[讀取此節](index/09.md#55-dotestsucktestic_twoarm32site-atester_32sitecpp)

### 5.6 IndexEveryTimeCheckEP() (`atester.cpp`)

[讀取此節](index/09.md#56-indexeverytimecheckep-atestercpp)

## 6. End-to-End Runtime Flow

[讀取此節](index/10.md#6-end-to-end-runtime-flow)

## 7. Notes

[讀取此節](index/11.md#7-notes)

## 8. Index Cycle Time / Test Time 記錄機制（摘要）

[讀取此節](index/12.md#8-index-cycle-time--test-time-記錄機制摘要)

### 三欄定義（Observer「Time Info」頁，勿混用）

[讀取此節](index/12.md#三欄定義observertime-info頁勿混用)

### 打點位置

[讀取此節](index/12.md#打點位置)

### Index Cycle Time 包含哪些

[讀取此節](index/12.md#index-cycle-time-包含哪些)

### 常見判讀陷阱

[讀取此節](index/12.md#常見判讀陷阱)

## 9. UPH 記錄機制（摘要）

[讀取此節](index/13.md#9-uph-記錄機制摘要)

### 統計口徑（與 cycle time 完全不同，不可互推）

[讀取此節](index/13.md#統計口徑與-cycle-time-完全不同不可互推)

### 後台記錄位置

[讀取此節](index/13.md#後台記錄位置)

### `[P11]` 三種 CSV 格式（依 `CUSTOMER_CODE` 分流，`ainarm9045.cpp:5535`）

[讀取此節](index/13.md#p11-三種-csv-格式依-customer_code-分流ainarm9045cpp5535)

## 合併補充：repo 既有參考（20261001）

[讀取此節](index/14.md#合併補充repo-既有參考20261001)
