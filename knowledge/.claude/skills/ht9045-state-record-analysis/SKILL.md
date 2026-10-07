---
name: ht9045-state-record-analysis
description: 'Analyze machine hang up from state record folders. Use when checking Task_ListWithTime.csv, Task_ListWithTime2.csv, MainProcMonitor, LastEnter, SaveTime, SilentSec, CallCount, and thread/MainProc health in HT machine projects.'
argument-hint: 'Provide the state record folder path and, if known, the related project path.'
user-invocable: true
disable-model-invocation: false
---

# ht9045-state-record-analysis 相容入口

同主題已整合到 [hpi-state-analysis](../hpi-state-analysis/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-state-analysis/references/source/original-entry.md)

## When to Use

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#when-to-use)

## Inputs

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#inputs)

## Primary Files

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#primary-files)

## Pre-Analysis Checklist (do this BEFORE deep code-diving)

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#pre-analysis-checklist-do-this-before-deep-code-diving)

## Core Procedure

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#core-procedure)

## Interpretation Rules

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#interpretation-rules)

## Recommended Classification

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#recommended-classification)

### Normal or likely normal

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#normal-or-likely-normal)

### Suspicious

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#suspicious)

### Likely hang or loop stop

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#likely-hang-or-loop-stop)

## Known Good Example

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#known-good-example)

## Known Hang Example

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#known-hang-example)

## Output Format

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#output-format)

## Response Template

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#response-template)

## Code-Level Follow-up

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#code-level-follow-up)

## Workspace Notes

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#workspace-notes)

## Lessons Learned (from past investigations)

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#lessons-learned-from-past-investigations)

### LL-1: Always confirm the customer's running version FIRST

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#ll-1-always-confirm-the-customers-running-version-first)

### LL-2: Read INI / Recipe BEFORE tracing code branches

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#ll-2-read-ini--recipe-before-tracing-code-branches)

### LL-3: When customer reports "feature X not working"

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#ll-3-when-customer-reports-feature-x-not-working)

## Reference Files

[讀取此節](../hpi-state-analysis/references/source/original-entry.md#reference-files)

## C++ V906 補充（Jimmy 20261006：可自主維護分析所需記錄）

- C++ State Record 的 `Ver.txt` 可能是空白；先讀 `W906_ReadMe.txt` 的 `build`、`exe`、`exeTime`，再讀 `W906_Executable.txt` 的執行檔 SHA-256 與編譯時間。不可只靠版本字串把不同執行檔當成同版。
- `DecisionVariables.csv` 的 Tray Arm teaching 區記錄當次快照的機種、教導值、Prod 實際目標、位置快取，以及 `EffectiveTeachIni`。`W906_EffectiveTeach.ini` 是實際生效來源的複本；有 W906_TEACH_INI_PATH 覆寫時，不應改讀預設 system/teach.ini 當作實際來源。
- `TrayArm teach guard rejected` 事件記錄觸發當下的 target、safe、station、check、教導／Prod 值及位置快取。這是拒絕瞬間的證據；DecisionVariables 是稍後按 State Record 的快照，兩者要區分。記錄只讀快取，不為了記錄而操作馬達。
- NB2 20261006 19:04 案例：HOME 19:04:02.686 完成；19:04:03.938 自動搬盤 CatchTrayTask=10 的 6800 目標超過 6500 教導保護界線。不能稱為 HOME 再次失敗。MainProc LastEnter=19:04:29.333、SaveTime=19:04:29.554（差 0.221 秒，SilentSec=0.263）、Alive=Y；最後正常 task=19:04:03.911，屬警報等待，非執行緒掛死。
- ZIP 外仍有同名資料夾時，先檢查 ZIP CRC 與殘留檔內容。此案殘留 2 個含 U+00A6 的檔名，ANSI CP950 roundtrip 把它轉成 `|`，舊 Del_Tree 找不到檔案。已改為時間資料夾範圍內的 Unicode 清除，不跟隨 junction；清除失敗會保留資料並記錄 Win32 錯誤。不要把殘留資料夾一律解釋成壓縮失敗。
- 模擬教導副本用 `tools/prepare_ht9050_sim_teach.py` 產生，只調整 MTrayX Color=20000；此為合成流程資料，不可當成真機教導值或同步回機台。機台仍須現場核對 Empty/Color、取盤偏移與實際運動範圍。
