# OutArm — 取料段（Shuttle Pick）詳細參考

舊引用路徑保留；[讀取整理後文件](../../hpi-outarm-flow/references/flow/references/outarm-pick.md)。

## 1. 呼叫階層總覽

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-pick.md#1-呼叫階層總覽)

## 1.1 OutArm 吸嘴結構與 X 軸硬體設定

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-pick.md#11-outarm-吸嘴結構與-x-軸硬體設定)

## 2. DoOutArm() — 入口

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-pick.md#2-dooutarm--入口)

## 3. DoOutArm_9045() — Dispatch

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-pick.md#3-dooutarm_9045--dispatch)

### Pre-Dispatch Guard Checks

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-pick.md#pre-dispatch-guard-checks)

### iInArmType Dispatch Table

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-pick.md#iinarmtype-dispatch-table)

## 4. 主狀態機 — DoOutArm_9045_2x8_8()

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-pick.md#4-主狀態機--dooutarm_9045_2x8_8)

### Shuttle 1 Pick Flow

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-pick.md#shuttle-1-pick-flow)

### Shuttle 2 Pick Flow

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-pick.md#shuttle-2-pick-flow)

### State Descriptions（取料相關）

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-pick.md#state-descriptions取料相關)

## 5. DoPickFromShuttle_9045_2x8_8() — Shuttle 取料

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-pick.md#5-dopickfromshuttle_9045_2x8_8--shuttle-取料)

### 關鍵 Case

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-pick.md#關鍵-case)

### 吸取邏輯（case iOUTARM_SUCK）

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-pick.md#吸取邏輯case-ioutarm_suck)

### Error 處理（case 2000）

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-pick.md#error-處理case-2000)
