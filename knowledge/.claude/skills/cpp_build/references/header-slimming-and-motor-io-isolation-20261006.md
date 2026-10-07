# 熱門標頭瘦身 ＋ Motor／IO 模組化評估（2026-10-06，main `84233b648`）

舊引用路徑保留；[讀取整理後文件](../../hpi-build/references/cpp/references/header-slimming-and-motor-io-isolation-20261006.md)。

## 0. 結論

[讀取此節](../../hpi-build/references/cpp/references/header-slimming-and-motor-io-isolation-20261006.md#0-結論)

## A1. 熱門標頭現況〔實測〕

[讀取此節](../../hpi-build/references/cpp/references/header-slimming-and-motor-io-isolation-20261006.md#a1-熱門標頭現況實測)

## A2. mymotor.h 用到各個 include 的方式〔實測〕

[讀取此節](../../hpi-build/references/cpp/references/header-slimming-and-motor-io-isolation-20261006.md#a2-mymotorh-用到各個-include-的方式實測)

## A3. cmydef.h／cprod.h 內容分區〔實測〕

[讀取此節](../../hpi-build/references/cpp/references/header-slimming-and-motor-io-isolation-20261006.md#a3-cmydefhcprodh-內容分區實測)

## A4. 傳遞成本 g++ -E〔實測，旗標照 compile_commands，g++ 6.3〕

[讀取此節](../../hpi-build/references/cpp/references/header-slimming-and-motor-io-isolation-20261006.md#a4-傳遞成本-g--e實測旗標照-compile_commandsg-63)

## A5. 瘦身計畫（依「省下的重編量 ÷ 風險」排序；MachineType.h 一律不動）

[讀取此節](../../hpi-build/references/cpp/references/header-slimming-and-motor-io-isolation-20261006.md#a5-瘦身計畫依省下的重編量--風險排序machinetypeh-一律不動)

## B1. 現有的靜態庫〔實測〕

[讀取此節](../../hpi-build/references/cpp/references/header-slimming-and-motor-io-isolation-20261006.md#b1-現有的靜態庫實測)

## B2. 跨邊界的全域狀態〔實測，去註解〕

[讀取此節](../../hpi-build/references/cpp/references/header-slimming-and-motor-io-isolation-20261006.md#b2-跨邊界的全域狀態實測去註解)

## B3. 要做成 DLL 需要什麼

[讀取此節](../../hpi-build/references/cpp/references/header-slimming-and-motor-io-isolation-20261006.md#b3-要做成-dll-需要什麼)

## B4. 離「標頭隔離」還差多遠〔實測〕

[讀取此節](../../hpi-build/references/cpp/references/header-slimming-and-motor-io-isolation-20261006.md#b4-離標頭隔離還差多遠實測)

## B5. 建議（分階段；DLL 不做）

[讀取此節](../../hpi-build/references/cpp/references/header-slimming-and-motor-io-isolation-20261006.md#b5-建議分階段dll-不做)

## C. 下一步（給 Steven 決定後派工）

[讀取此節](../../hpi-build/references/cpp/references/header-slimming-and-motor-io-isolation-20261006.md#c-下一步給-steven-決定後派工)
