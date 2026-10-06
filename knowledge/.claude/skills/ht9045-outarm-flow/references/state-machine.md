# OutArm 狀態機與放料細節

舊引用路徑保留；[讀取整理後文件](../../hpi-outarm-flow/references/flow/references/state-machine.md)。

## 主狀態機 — DoOutArm_9045_2x8_8() (代表所有 XxY_Z 版本)

[讀取此節](../../hpi-outarm-flow/references/flow/references/state-machine.md#主狀態機--dooutarm_9045_2x8_8-代表所有-xxy_z-版本)

### Pre-Switch Guard

[讀取此節](../../hpi-outarm-flow/references/flow/references/state-machine.md#pre-switch-guard)

### 主流程

[讀取此節](../../hpi-outarm-flow/references/flow/references/state-machine.md#主流程)

### Shuttle 1 Pick Flow (case 1000 ~ 1235)

[讀取此節](../../hpi-outarm-flow/references/flow/references/state-machine.md#shuttle-1-pick-flow-case-1000--1235)

### Shuttle 2 Pick Flow (case 2000 ~ 2235)

[讀取此節](../../hpi-outarm-flow/references/flow/references/state-machine.md#shuttle-2-pick-flow-case-2000--2235)

### Place to Tray Flow (case 3000 ~ 3500)

[讀取此節](../../hpi-outarm-flow/references/flow/references/state-machine.md#place-to-tray-flow-case-3000--3500)

### Special Cases

[讀取此節](../../hpi-outarm-flow/references/flow/references/state-machine.md#special-cases)

### State Descriptions

[讀取此節](../../hpi-outarm-flow/references/flow/references/state-machine.md#state-descriptions)

## DoPickFromShuttle_9045_2x8_8() — Shuttle 取料

[讀取此節](../../hpi-outarm-flow/references/flow/references/state-machine.md#dopickfromshuttle_9045_2x8_8--shuttle-取料)

### 關鍵 Case

[讀取此節](../../hpi-outarm-flow/references/flow/references/state-machine.md#關鍵-case)

### Error 處理（case 2000）

[讀取此節](../../hpi-outarm-flow/references/flow/references/state-machine.md#error-處理case-2000)

## DoOutArmAdditionalFunction() — 附加功能

[讀取此節](../../hpi-outarm-flow/references/flow/references/state-machine.md#dooutarmadditionalfunction--附加功能)

## DoOutArmPlaceToAuto_9045() — 放料到 Tray 控制

[讀取此節](../../hpi-outarm-flow/references/flow/references/state-machine.md#dooutarmplacetoauto_9045--放料到-tray-控制)

## DoOutArmPlaceToAuto() — 實際放料 Destroy

[讀取此節](../../hpi-outarm-flow/references/flow/references/state-machine.md#dooutarmplacetoauto--實際放料-destroy)

## DoOutArmAfterPlaceToAuto() — 放料後處理

[讀取此節](../../hpi-outarm-flow/references/flow/references/state-machine.md#dooutarmafterplacetoauto--放料後處理)

### 回傳值對照

[讀取此節](../../hpi-outarm-flow/references/flow/references/state-machine.md#回傳值對照)

## CheckOutArmCleanOut() — Clean Out 判斷

[讀取此節](../../hpi-outarm-flow/references/flow/references/state-machine.md#checkoutarmcleanout--clean-out-判斷)
