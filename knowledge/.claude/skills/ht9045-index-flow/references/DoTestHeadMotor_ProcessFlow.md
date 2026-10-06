# DoTestHeadMotor Process Flow

> 內容已分層整理，舊路徑保留供既有引用使用。

[讀取整理後文件](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md)

## 1. Call Hierarchy

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#1-call-hierarchy)

## 2. DoTestHeadMotor() Main Flow (atester.cpp)

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#2-dotestheadmotor-main-flow-atestercpp)

## Pre-switch guards

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#pre-switch-guards)

## Top-level phase map

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#top-level-phase-map)

## Key state groups

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#key-state-groups)

### A. Index head init & vacuum self-check

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#a-index-head-init--vacuum-self-check)

### B. F16 shuttle broken sensor check (optional)

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#b-f16-shuttle-broken-sensor-check-optional)

### C. CCD startup check path

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#c-ccd-startup-check-path)

### D. Core index pre-test check (torque + vacuum + socket)

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#d-core-index-pre-test-check-torque--vacuum--socket)

### E. AutoSiteMap trigger and test execution

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#e-autositemap-trigger-and-test-execution)

### F. Move into run posture and enter test cycle

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#f-move-into-run-posture-and-enter-test-cycle)

### G. PlaceToShuttleFirst cleanup route

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#g-placetoshuttlefirst-cleanup-route)

### H. RTC FullView / ROI learning route

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#h-rtc-fullview--roi-learning-route)

## 3. Next Layer: DoTestY() (atester.cpp)

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#3-next-layer-dotesty-atestercpp)

## State machine overview

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#state-machine-overview)

## Case summary

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#case-summary)

## 4. Child State Machines (called by DoTestY / DoTestHeadMotor)

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#4-child-state-machines-called-by-dotesty--dotestheadmotor)

## 4.1 DoTestYFront() (`aTester_Front.cpp`)

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#41-dotestyfront-atester_frontcpp)

## 4.2 DoTestYRear() (`aTester_Rear.cpp`)

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#42-dotestyrear-atester_rearcpp)

## 4.3 DoTestY_TwoArm32Site() (`atester_32Site.cpp`)

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#43-dotesty_twoarm32site-atester_32sitecpp)

### 4.3.1 Socket IC Check 鏈（case 81→97）與 [D41] 總開關（2026-07 查證）

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#431-socket-ic-check-鏈case-8197與-d41-總開關2026-07-查證)

## 5. Other Direct Child State Machines (from DoTestHeadMotor)

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#5-other-direct-child-state-machines-from-dotestheadmotor)

## 5.1 DoFrontTestDestroyIC(false) (`aTester_Front.cpp`)

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#51-dofronttestdestroyicfalse-atester_frontcpp)

## 5.2 DoRearTestDestroyIC(false) (`aTester_Rear.cpp`)

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#52-doreartestdestroyicfalse-atester_rearcpp)

## 5.3 DoFTestSuckTestIC() (`aTester_Front.cpp`)

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#53-doftestsucktestic-atester_frontcpp)

## 5.4 DoBTestSuckTestIC() (`aTester_Rear.cpp`)

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#54-dobtestsucktestic-atester_rearcpp)

## 5.5 DoTestSuckTestIC_TwoArm32Site() (`atester_32Site.cpp`)

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#55-dotestsucktestic_twoarm32site-atester_32sitecpp)

## 5.6 IndexEveryTimeCheckEP() (`atester.cpp`)

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#56-indexeverytimecheckep-atestercpp)

## 6. End-to-End Runtime Flow (condensed)

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#6-end-to-end-runtime-flow-condensed)

## 7. Notes

[讀取此節](../../hpi-index-flow/references/flow/DoTestHeadMotor_ProcessFlow.md#7-notes)
