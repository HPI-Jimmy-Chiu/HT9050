---
name: ht9045-index-flow
description: HT9045 IC Test Handler Index Arm（下壓模組）流程知識庫。當使用者詢問 Index Arm、Test Head、DoTestHeadMotor、DoTestY、DoTestYFront、DoTestYRear、Index 測試流程、Socket 下壓、RTC FullView、AutoSiteMap、PlaceToShuttleFirst、IndexEveryTimeCheckEP、32-site 兩臂測試、Front/Rear 測試切換、Index Torque、Socket Sensor、EP 充氣壓力、Index Cycle Time / Test Time / Index Time 三欄定義與記錄機制、Index Cycle Time 包含哪些時間、cycle time 變長排查、[D70] Index Cycle Time Record 記錄檔、INDEXCYCLETIME? 查詢、Test Information → Test Time 頁欄位說明、右側無標題欄（InArm 放料到 Shuttle 的循環時間 / InArm pick 到下一次 pick）、UPH 後台記錄檔案與統計口徑、[P11] Record UPH 的 CSV 格式等相關問題時，應先載入此技能以理解 Index 完整處理流程。關鍵字：DoTestHeadMotor, DoTestY, DoTestYFront, DoTestYRear, DoTestY_TwoArm32Site, IndexEveryTimeCheckEP, DoFrontTestDestroyIC, DoRearTestDestroyIC, FTestSuck, BTestSuck, MTestY, MTestZ, Index, TestHead, Index Cycle Time, IndexCycleTime, Test Time, Index Time, RecordStartTestTime, RecordEndTestTime, RecordTimeInfo, ShowIndexTime, tIndexTimer, GetIndexTime_flag, TimeInfoGrid, Time Info, tsTestTime, sgTimeData, Motion Part, DropContactTimer, Drop Contact 三段, bD70IndexCycleTimeRecord, RecordIndexCycle, IndexCycleTimeRecord, INDEXCYCLETIME?, MSG_CMD_IndexCycleTime, RunInfo.IndexCycleTime, bEnableIndexCycleTimeMonitoring, bIndexTimeSet, SOT, EOT, cycle time 變長, TimeInfoGrid_InArm, RecordInArmTime, fRecordInArmTime, tRecordInArmTimer, InArm 循環時間, InArm pick to pick, 無標題欄位, UPH, UPH 記錄, UPH log, UPH csv, CalculateUPH, CaculateUPH, MyDBIUPH, RecordUPH, LotRecordUPH, RecordLotUPH_For_FOREHOPE_NINGBO, bP11RecordUPH, P11 Record UPH, as9045UPH, HT9045_Log\UPH, EventLogTxt, bRecordUPH, iUPH_LoaderCount, tUPH_PauseTime, RunInfo.iUPH, RunInfo.iAvgUPH, iNetUPH, iGrossUPH, Avg UPH, Net UPH, Gross UPH, UPH_StringGrid, Tab_UPH, bShowUPH, bG10ShowImmediateUPH, CountMTBF, MTBA, MUBA, UPHRecordEnd。另含 **Auto Clean 後 RTC 不回應 @ARM2+ 導致 WAR0335「RTC arm 1 error!」** 完整案例：WAR0335, RTC arm 1 error, RTC Arm1 Error, SnRealTimeCCDIndexArm, @ARM2+, rtArmIndex2, iRealCCDSendArmCT, OpenRTCComPortAgain, WAR0335 auto retry com port, bFullTestBeforeAutoClean, Full View before Auto Clean, DoFullViewCheck, DoReleaseAndInspEnd, DoReleaseAndInspEndByFlag, InitRealTimeCCDPara, bSendRealCCDSendStart, rtInspStart, @START+, @RELEASE+, @END+, bIndexCheckCanTurnOff, iD71IndexCheckOnOffMode, bAfterAutoCleanNoIndexCheck, iD69IndexCheckModeForAutoClean, 升版後才發生, 降版就正常, SCK RTC, RTC 1.0。另含**下壓後 hangup / 跨臂 Z 安全位互鎖卡死**完整案例：DoInterFaceErrorStep, TESTZ1UP, TESTZ2UP, iDoInterFaceErrorStepTask, Prod.TestZ1_Safe, Prod.TestZ2_Safe, 跨臂互鎖, 兩臂互相等待, Front Rear 互鎖, 下壓後hangup, 下壓後卡死, Index下壓hangup, PAUSE後卡死, PAUSE中斷座標移動, bATCHasAlarmBinNeedToError, RearTestSuckTestICTask, FrontTestSuckICTask, iBTestSuckTestICTask, iFTestSuckTestICTask, GetTesterResult, bEcho, MTestZ1, MTestZ2, Galil VS0 急停, 綠燈亮不動作。
---

# ht9045-index-flow 相容入口

> 同主題已整合到 [hpi-index-flow](../hpi-index-flow/SKILL.md)，先由共同流程與機型差異選路。原觸發詞與完整正文保留。

- [原版詳細內容](../hpi-index-flow/references/history/ht9045-index-flow/index.md)

## 適用場景

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/01.md#適用場景)

## 專案資訊

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/02.md#專案資訊)

## 參考文件

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/03.md#參考文件)

## 關鍵原始檔

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/04.md#關鍵原始檔)

## 1. 呼叫階層總覽

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/05.md#1-呼叫階層總覽)

## 2. DoTestHeadMotor() Main Flow (atester.cpp)

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/06.md#2-dotestheadmotor-main-flow-atestercpp)

### Pre-switch guards

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/06.md#pre-switch-guards)

### Top-level phase map

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/06.md#top-level-phase-map)

### Key state groups

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/06.md#key-state-groups)

#### A. Index head init & vacuum self-check

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/06.md#a-index-head-init--vacuum-self-check)

#### B. F16 shuttle broken sensor check (optional)

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/06.md#b-f16-shuttle-broken-sensor-check-optional)

#### C. CCD startup check path

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/06.md#c-ccd-startup-check-path)

#### D. Core index pre-test check (torque + vacuum + socket)

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/06.md#d-core-index-pre-test-check-torque--vacuum--socket)

#### E. AutoSiteMap trigger and test execution

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/06.md#e-autositemap-trigger-and-test-execution)

#### F. Move into run posture and enter test cycle

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/06.md#f-move-into-run-posture-and-enter-test-cycle)

#### G. PlaceToShuttleFirst cleanup route

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/06.md#g-placetoshuttlefirst-cleanup-route)

#### H. RTC FullView / ROI learning route

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/06.md#h-rtc-fullview--roi-learning-route)

## 3. Next Layer: DoTestY() (atester.cpp)

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/07.md#3-next-layer-dotesty-atestercpp)

### State machine overview

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/07.md#state-machine-overview)

### Case summary

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/07.md#case-summary)

## 4. Child State Machines

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/08.md#4-child-state-machines)

### 4.1 DoTestYFront() (`aTester_Front.cpp`)

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/08.md#41-dotestyfront-atester_frontcpp)

### 4.2 DoTestYRear() (`aTester_Rear.cpp`)

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/08.md#42-dotestyrear-atester_rearcpp)

### 4.3 DoTestY_TwoArm32Site() (`atester_32Site.cpp`)

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/08.md#43-dotesty_twoarm32site-atester_32sitecpp)

## 5. Other Direct Child State Machines

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/09.md#5-other-direct-child-state-machines)

### 5.1 DoFrontTestDestroyIC(false) (`aTester_Front.cpp`)

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/09.md#51-dofronttestdestroyicfalse-atester_frontcpp)

### 5.2 DoRearTestDestroyIC(false) (`aTester_Rear.cpp`)

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/09.md#52-doreartestdestroyicfalse-atester_rearcpp)

### 5.3 DoFTestSuckTestIC() (`aTester_Front.cpp`)

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/09.md#53-doftestsucktestic-atester_frontcpp)

### 5.4 DoBTestSuckTestIC() (`aTester_Rear.cpp`)

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/09.md#54-dobtestsucktestic-atester_rearcpp)

### 5.5 DoTestSuckTestIC_TwoArm32Site() (`atester_32Site.cpp`)

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/09.md#55-dotestsucktestic_twoarm32site-atester_32sitecpp)

### 5.6 IndexEveryTimeCheckEP() (`atester.cpp`)

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/09.md#56-indexeverytimecheckep-atestercpp)

## 6. End-to-End Runtime Flow

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/10.md#6-end-to-end-runtime-flow)

## 7. Notes

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/11.md#7-notes)

## 8. Index Cycle Time / Test Time 記錄機制（摘要）

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/12.md#8-index-cycle-time--test-time-記錄機制摘要)

### 三欄定義（Observer「Time Info」頁，勿混用）

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/12.md#三欄定義observertime-info頁勿混用)

### 打點位置

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/12.md#打點位置)

### Index Cycle Time 包含哪些

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/12.md#index-cycle-time-包含哪些)

### 常見判讀陷阱

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/12.md#常見判讀陷阱)

## 9. UPH 記錄機制（摘要）

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/13.md#9-uph-記錄機制摘要)

### 統計口徑（與 cycle time 完全不同，不可互推）

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/13.md#統計口徑與-cycle-time-完全不同不可互推)

### 後台記錄位置

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/13.md#後台記錄位置)

### `[P11]` 三種 CSV 格式（依 `CUSTOMER_CODE` 分流，`ainarm9045.cpp:5535`）

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/13.md#p11-三種-csv-格式依-customer_code-分流ainarm9045cpp5535)

## 合併補充：repo 既有參考（20261001）

[讀取此節](../hpi-index-flow/references/history/ht9045-index-flow/index/14.md#合併補充repo-既有參考20261001)
