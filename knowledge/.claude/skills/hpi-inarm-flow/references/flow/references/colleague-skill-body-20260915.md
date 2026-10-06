# repo 版 SKILL.md 正文（合併前原樣保留）

按需要選取以下章節，原文依順序保留。

- [repo 版 SKILL.md 正文（合併前原樣保留）](colleague-skill-body-20260915/00.md)
- [適用場景](colleague-skill-body-20260915/01.md)
- [關鍵原始檔](colleague-skill-body-20260915/02.md)
- [參考文件](colleague-skill-body-20260915/03.md)
- [1. 呼叫階層](colleague-skill-body-20260915/04.md)
- [2. DoInArm() — 入口 Guard Checks (ainarm2.cpp ~1588)](colleague-skill-body-20260915/05.md)
- [3. DoInArm_9045() — Dispatch (ainarm9045.cpp ~3907)](colleague-skill-body-20260915/06.md)
- [4. 主狀態機 (DoInArm_9045_XxY_Z, Task = iArmTask)](colleague-skill-body-20260915/07.md)
- [5. DoInArmPickFromLoadStage_9045() (Task = iPickFromLoadStageTask)](colleague-skill-body-20260915/08.md)
- [6. DoInArmAdditionalFunction() (Task = iInArmAdditionalFunctionTask)](colleague-skill-body-20260915/09.md)
- [7. DoPlaceToHotPlate_9045() (Task = iInArmPlaceToHotPlateTask)](colleague-skill-body-20260915/10.md)
- [8. DoInArmPickFromHotPlate_9045() (Task = iInArmPickFromHotPlateTask)](colleague-skill-body-20260915/11.md)
- [9. DoInArmPlaceToShuttle_9045() (Task = iInArmPlaceToShuttleTask)](colleague-skill-body-20260915/12.md)
- [10. 吸嘴配置 (iInArmType)](colleague-skill-body-20260915/13.md)
- [10.5 InArm Pitch 模式 — `ArmSpeed[InArm].bVariModeFIX`](colleague-skill-body-20260915/14.md)
- [11. 常見問題快查](colleague-skill-body-20260915/15.md)

# repo 版 SKILL.md 正文（合併前原樣保留）

[讀取此節](colleague-skill-body-20260915/00.md#repo-版-skillmd-正文合併前原樣保留)

# HT9045 InArm Flow Knowledge

[讀取此節](colleague-skill-body-20260915/00.md#ht9045-inarm-flow-knowledge)

## 適用場景

[讀取此節](colleague-skill-body-20260915/01.md#適用場景)

## 關鍵原始檔

[讀取此節](colleague-skill-body-20260915/02.md#關鍵原始檔)

## 參考文件

[讀取此節](colleague-skill-body-20260915/03.md#參考文件)

## 1. 呼叫階層

[讀取此節](colleague-skill-body-20260915/04.md#1-呼叫階層)

## 2. DoInArm() — 入口 Guard Checks (ainarm2.cpp ~1588)

[讀取此節](colleague-skill-body-20260915/05.md#2-doinarm--入口-guard-checks-ainarm2cpp-1588)

## 3. DoInArm_9045() — Dispatch (ainarm9045.cpp ~3907)

[讀取此節](colleague-skill-body-20260915/06.md#3-doinarm_9045--dispatch-ainarm9045cpp-3907)

## 4. 主狀態機 (DoInArm_9045_XxY_Z, Task = iArmTask)

[讀取此節](colleague-skill-body-20260915/07.md#4-主狀態機-doinarm_9045_xxy_z-task--iarmtask)

### Ambient Mode (常溫: Loader → Shuttle)

[讀取此節](colleague-skill-body-20260915/07.md#ambient-mode-常溫-loader--shuttle)

### Hot Mode (高溫: Loader → HotPlate → Shuttle)

[讀取此節](colleague-skill-body-20260915/07.md#hot-mode-高溫-loader--hotplate--shuttle)

### State Descriptions

[讀取此節](colleague-skill-body-20260915/07.md#state-descriptions)

## 5. DoInArmPickFromLoadStage_9045() (Task = iPickFromLoadStageTask)

[讀取此節](colleague-skill-body-20260915/08.md#5-doinarmpickfromloadstage_9045-task--ipickfromloadstagetask)

## 6. DoInArmAdditionalFunction() (Task = iInArmAdditionalFunctionTask)

[讀取此節](colleague-skill-body-20260915/09.md#6-doinarmadditionalfunction-task--iinarmadditionalfunctiontask)

## 7. DoPlaceToHotPlate_9045() (Task = iInArmPlaceToHotPlateTask)

[讀取此節](colleague-skill-body-20260915/10.md#7-doplacetohotplate_9045-task--iinarmplacetohotplatetask)

## 8. DoInArmPickFromHotPlate_9045() (Task = iInArmPickFromHotPlateTask)

[讀取此節](colleague-skill-body-20260915/11.md#8-doinarmpickfromhotplate_9045-task--iinarmpickfromhotplatetask)

## 9. DoInArmPlaceToShuttle_9045() (Task = iInArmPlaceToShuttleTask)

[讀取此節](colleague-skill-body-20260915/12.md#9-doinarmplacetoshuttle_9045-task--iinarmplacetoshuttletask)

## 10. 吸嘴配置 (iInArmType)

[讀取此節](colleague-skill-body-20260915/13.md#10-吸嘴配置-iinarmtype)

### General.ini 吸嘴相關參數

[讀取此節](colleague-skill-body-20260915/13.md#generalini-吸嘴相關參數)

### TMyKitSuck 核心成員速查

[讀取此節](colleague-skill-body-20260915/13.md#tmykitsuck-核心成員速查)

### 軟體架構深入（HP 縮 Pitch、Teaching 推導、機型拓樸）

[讀取此節](colleague-skill-body-20260915/13.md#軟體架構深入hp-縮-pitchteaching-推導機型拓樸)

## 10.5 InArm Pitch 模式 — `ArmSpeed[InArm].bVariModeFIX`

[讀取此節](colleague-skill-body-20260915/14.md#105-inarm-pitch-模式--armspeedinarmbvarimodefix)

### 10.5.1 語意

[讀取此節](colleague-skill-body-20260915/14.md#1051-語意)

### 10.5.2 三層變數結構

[讀取此節](colleague-skill-body-20260915/14.md#1052-三層變數結構)

### 10.5.3 主要 Pitch 判斷點（讀取 `bVariModeFIX`）

[讀取此節](colleague-skill-body-20260915/14.md#1053-主要-pitch-判斷點讀取-bvarimodefix)

### 10.5.4 設值點與時機（寫入 `bVariModeFIX`）

[讀取此節](colleague-skill-body-20260915/14.md#1054-設值點與時機寫入-bvarimodefix)

### 10.5.5 SECS/GEM 整合

[讀取此節](colleague-skill-body-20260915/14.md#1055-secsgem-整合)

### 10.5.6 與多顆吸取 (iRowCT) 的影響表（已更正 2026-05-11）

[讀取此節](colleague-skill-body-20260915/14.md#1056-與多顆吸取-irowct-的影響表已更正-2026-05-11)

## 11. 常見問題快查

[讀取此節](colleague-skill-body-20260915/15.md#11-常見問題快查)
