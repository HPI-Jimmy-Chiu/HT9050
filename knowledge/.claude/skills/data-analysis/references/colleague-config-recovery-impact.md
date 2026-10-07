# Config 旗標對錯誤恢復路徑的影響

舊引用路徑保留；[讀取整理後文件](../../hpi-state-analysis/references/quick/references/colleague-config-recovery-impact.md)。

## 概述

[讀取此節](../../hpi-state-analysis/references/quick/references/colleague-config-recovery-impact.md#概述)

## D 系列（Index 相關）

[讀取此節](../../hpi-state-analysis/references/quick/references/colleague-config-recovery-impact.md#d-系列index-相關)

### D42 — IndexPickICShuttlePause

[讀取此節](../../hpi-state-analysis/references/quick/references/colleague-config-recovery-impact.md#d42--indexpickicshuttlepause)

### D43 — IndexDropErrorCanRetryandSkip

[讀取此節](../../hpi-state-analysis/references/quick/references/colleague-config-recovery-impact.md#d43--indexdroperrorcanretryandskip)

### D44 — CheckIndexICDestroy

[讀取此節](../../hpi-state-analysis/references/quick/references/colleague-config-recovery-impact.md#d44--checkindexicdestroy)

### D50 — IndexPickErrSkipNeedCheckVac

[讀取此節](../../hpi-state-analysis/references/quick/references/colleague-config-recovery-impact.md#d50--indexpickerrskipneedcheckvac)

### D64 — IndexPickErrOnlySKIP

[讀取此節](../../hpi-state-analysis/references/quick/references/colleague-config-recovery-impact.md#d64--indexpickerronlyskip)

### D72 — NNModeMoveShtAfterContact

[讀取此節](../../hpi-state-analysis/references/quick/references/colleague-config-recovery-impact.md#d72--nnmodemoveshtaftercontact)

## F 系列（Shuttle 相關）

[讀取此節](../../hpi-state-analysis/references/quick/references/colleague-config-recovery-impact.md#f-系列shuttle-相關)

### F07 — OutShuttleSensorMode

[讀取此節](../../hpi-state-analysis/references/quick/references/colleague-config-recovery-impact.md#f07--outshuttlesensormode)

## 交互影響分析

[讀取此節](../../hpi-state-analysis/references/quick/references/colleague-config-recovery-impact.md#交互影響分析)

### D42 + JAM0302 SKIP → 死鎖風險

[讀取此節](../../hpi-state-analysis/references/quick/references/colleague-config-recovery-impact.md#d42--jam0302-skip--死鎖風險)

### D43 + D42 + JAM0302 SKIP

[讀取此節](../../hpi-state-analysis/references/quick/references/colleague-config-recovery-impact.md#d43--d42--jam0302-skip)

### bIndexPickErrOnlySKIP (D64) + D42

[讀取此節](../../hpi-state-analysis/references/quick/references/colleague-config-recovery-impact.md#bindexpickerronlyskip-d64--d42)

## 待補充

[讀取此節](../../hpi-state-analysis/references/quick/references/colleague-config-recovery-impact.md#待補充)
