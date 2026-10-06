# Fix3 滿盤氣缸 JAM1940 / JAM1941 誤報 —— `Fix3CylinderDelay` 的守門旗標在恢復瞬間被搶先清掉

舊引用路徑保留；[讀取整理後文件](../../hpi-outarm-flow/references/flow/references/fix3-cylinder-jam1940-false-alarm.md)。

## 1. 一秒判定

[讀取此節](../../hpi-outarm-flow/references/flow/references/fix3-cylinder-jam1940-false-alarm.md#1-一秒判定)

## 2. 案例：JSCC ILD502，2026-09-08，當天 7 筆全是誤報

[讀取此節](../../hpi-outarm-flow/references/flow/references/fix3-cylinder-jam1940-false-alarm.md#2-案例jscc-ild5022026-09-08當天-7-筆全是誤報)

## 3. 機制：補丁選對了旗標，但輸在 dispatch 順序

[讀取此節](../../hpi-outarm-flow/references/flow/references/fix3-cylinder-jam1940-false-alarm.md#3-機制補丁選對了旗標但輸在-dispatch-順序)

### 相關碼

[讀取此節](../../hpi-outarm-flow/references/flow/references/fix3-cylinder-jam1940-false-alarm.md#相關碼)

### 為什麼失效

[讀取此節](../../hpi-outarm-flow/references/flow/references/fix3-cylinder-jam1940-false-alarm.md#為什麼失效)

### `bHangTimePause` 的正確語意（別再誤解）

[讀取此節](../../hpi-outarm-flow/references/flow/references/fix3-cylinder-jam1940-false-alarm.md#bhangtimepause-的正確語意別再誤解)

## 4. 修法（都只動 `aoutarm9045.cpp`，不碰任何共用碼）

[讀取此節](../../hpi-outarm-flow/references/flow/references/fix3-cylinder-jam1940-false-alarm.md#4-修法都只動-aoutarm9045cpp不碰任何共用碼)

### 方案 B（建議）—— 自足式 gap-guard，不依賴任何外部旗標

[讀取此節](../../hpi-outarm-flow/references/flow/references/fix3-cylinder-jam1940-false-alarm.md#方案-b建議-自足式-gap-guard不依賴任何外部旗標)

### 方案 A（最小改動）—— 補讀 `bHandlerPause`

[讀取此節](../../hpi-outarm-flow/references/flow/references/fix3-cylinder-jam1940-false-alarm.md#方案-a最小改動-補讀-bhandlerpause)

### 方案 C（保守，不改判斷邏輯）—— 只延長門檻

[讀取此節](../../hpi-outarm-flow/references/flow/references/fix3-cylinder-jam1940-false-alarm.md#方案-c保守不改判斷邏輯-只延長門檻)

## 5. 這是一個家族缺陷：`TQPF_Timer` 缺 gap-guard

[讀取此節](../../hpi-outarm-flow/references/flow/references/fix3-cylinder-jam1940-false-alarm.md#5-這是一個家族缺陷tqpf_timer-缺-gap-guard)

## 6. 與 Pattern #25 的關係

[讀取此節](../../hpi-outarm-flow/references/flow/references/fix3-cylinder-jam1940-false-alarm.md#6-與-pattern-25-的關係)

## 7. 現場處置（未改版前）

[讀取此節](../../hpi-outarm-flow/references/flow/references/fix3-cylinder-jam1940-false-alarm.md#7-現場處置未改版前)
