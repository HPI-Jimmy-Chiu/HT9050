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
