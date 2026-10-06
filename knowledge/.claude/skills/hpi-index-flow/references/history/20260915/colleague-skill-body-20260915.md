# repo 版 SKILL.md 正文（合併前原樣保留）

> 原始文件拆成 12 個順序片段；依下面導覽讀取需要的段落。機型與版本來源見 [共同與差異](../../common.md)。

- [repo 版 SKILL.md 正文（合併前原樣保留）](colleague-skill-body-20260915/00.md)
- [適用場景](colleague-skill-body-20260915/01.md)
- [專案資訊](colleague-skill-body-20260915/02.md)
- [參考文件](colleague-skill-body-20260915/03.md)
- [關鍵原始檔](colleague-skill-body-20260915/04.md)
- [1. 呼叫階層總覽](colleague-skill-body-20260915/05.md)
- [2. DoTestHeadMotor() Main Flow (atester.cpp)](colleague-skill-body-20260915/06.md)
- [3. Next Layer: DoTestY() (atester.cpp)](colleague-skill-body-20260915/07.md)
- [4. Child State Machines](colleague-skill-body-20260915/08.md)
- [5. Other Direct Child State Machines](colleague-skill-body-20260915/09.md)
- [6. End-to-End Runtime Flow](colleague-skill-body-20260915/10.md)
- [7. Notes](colleague-skill-body-20260915/11.md)

# repo 版 SKILL.md 正文（合併前原樣保留）

[讀取此節](colleague-skill-body-20260915/00.md#repo-版-skillmd-正文合併前原樣保留)

# HT9045 Index (Test Head Motor) Flow Knowledge

[讀取此節](colleague-skill-body-20260915/00.md#ht9045-index-test-head-motor-flow-knowledge)

## 適用場景

[讀取此節](colleague-skill-body-20260915/01.md#適用場景)

## 專案資訊

[讀取此節](colleague-skill-body-20260915/02.md#專案資訊)

## 參考文件

[讀取此節](colleague-skill-body-20260915/03.md#參考文件)

## 關鍵原始檔

[讀取此節](colleague-skill-body-20260915/04.md#關鍵原始檔)

## 1. 呼叫階層總覽

[讀取此節](colleague-skill-body-20260915/05.md#1-呼叫階層總覽)

## 2. DoTestHeadMotor() Main Flow (atester.cpp)

[讀取此節](colleague-skill-body-20260915/06.md#2-dotestheadmotor-main-flow-atestercpp)

### Pre-switch guards

[讀取此節](colleague-skill-body-20260915/06.md#pre-switch-guards)

### Top-level phase map

[讀取此節](colleague-skill-body-20260915/06.md#top-level-phase-map)

### Key state groups

[讀取此節](colleague-skill-body-20260915/06.md#key-state-groups)

#### A. Index head init & vacuum self-check

[讀取此節](colleague-skill-body-20260915/06.md#a-index-head-init--vacuum-self-check)

#### B. F16 shuttle broken sensor check (optional)

[讀取此節](colleague-skill-body-20260915/06.md#b-f16-shuttle-broken-sensor-check-optional)

#### C. CCD startup check path

[讀取此節](colleague-skill-body-20260915/06.md#c-ccd-startup-check-path)

#### D. Core index pre-test check (torque + vacuum + socket)

[讀取此節](colleague-skill-body-20260915/06.md#d-core-index-pre-test-check-torque--vacuum--socket)

#### E. AutoSiteMap trigger and test execution

[讀取此節](colleague-skill-body-20260915/06.md#e-autositemap-trigger-and-test-execution)

#### F. Move into run posture and enter test cycle

[讀取此節](colleague-skill-body-20260915/06.md#f-move-into-run-posture-and-enter-test-cycle)

#### G. PlaceToShuttleFirst cleanup route

[讀取此節](colleague-skill-body-20260915/06.md#g-placetoshuttlefirst-cleanup-route)

#### H. RTC FullView / ROI learning route

[讀取此節](colleague-skill-body-20260915/06.md#h-rtc-fullview--roi-learning-route)

## 3. Next Layer: DoTestY() (atester.cpp)

[讀取此節](colleague-skill-body-20260915/07.md#3-next-layer-dotesty-atestercpp)

### State machine overview

[讀取此節](colleague-skill-body-20260915/07.md#state-machine-overview)

### Case summary

[讀取此節](colleague-skill-body-20260915/07.md#case-summary)

## 4. Child State Machines

[讀取此節](colleague-skill-body-20260915/08.md#4-child-state-machines)

### 4.1 DoTestYFront() (`aTester_Front.cpp`)

[讀取此節](colleague-skill-body-20260915/08.md#41-dotestyfront-atester_frontcpp)

### 4.2 DoTestYRear() (`aTester_Rear.cpp`)

[讀取此節](colleague-skill-body-20260915/08.md#42-dotestyrear-atester_rearcpp)

### 4.3 DoTestY_TwoArm32Site() (`atester_32Site.cpp`)

[讀取此節](colleague-skill-body-20260915/08.md#43-dotesty_twoarm32site-atester_32sitecpp)

## 5. Other Direct Child State Machines

[讀取此節](colleague-skill-body-20260915/09.md#5-other-direct-child-state-machines)

### 5.1 DoFrontTestDestroyIC(false) (`aTester_Front.cpp`)

[讀取此節](colleague-skill-body-20260915/09.md#51-dofronttestdestroyicfalse-atester_frontcpp)

### 5.2 DoRearTestDestroyIC(false) (`aTester_Rear.cpp`)

[讀取此節](colleague-skill-body-20260915/09.md#52-doreartestdestroyicfalse-atester_rearcpp)

### 5.3 DoFTestSuckTestIC() (`aTester_Front.cpp`)

[讀取此節](colleague-skill-body-20260915/09.md#53-doftestsucktestic-atester_frontcpp)

### 5.4 DoBTestSuckTestIC() (`aTester_Rear.cpp`)

[讀取此節](colleague-skill-body-20260915/09.md#54-dobtestsucktestic-atester_rearcpp)

### 5.5 DoTestSuckTestIC_TwoArm32Site() (`atester_32Site.cpp`)

[讀取此節](colleague-skill-body-20260915/09.md#55-dotestsucktestic_twoarm32site-atester_32sitecpp)

### 5.6 IndexEveryTimeCheckEP() (`atester.cpp`)

[讀取此節](colleague-skill-body-20260915/09.md#56-indexeverytimecheckep-atestercpp)

## 6. End-to-End Runtime Flow

[讀取此節](colleague-skill-body-20260915/10.md#6-end-to-end-runtime-flow)

## 7. Notes

[讀取此節](colleague-skill-body-20260915/11.md#7-notes)
