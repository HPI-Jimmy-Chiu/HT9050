# golden V912 逐通道溫控器：71 通道全表、讀存檔逐行、通訊框

舊引用路徑保留；[讀取整理後文件](../../hpi-temperature/references/controllers/configuration/references/golden-v912-channels.md)。

## 1. 71 通道全表

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/golden-v912-channels.md#1-71-通道全表)

## 2. 讀、存、按鈕（golden `HandlerSys.cpp`）

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/golden-v912-channels.md#2-讀存按鈕golden-handlersyscpp)

## 3. 通訊框（golden `cpublic.cpp`，全部經 `COM2->Comm2` 送到 `[TempCtrl] COM_PORT`）

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/golden-v912-channels.md#3-通訊框golden-cpubliccpp全部經-com2-comm2-送到-tempctrl-com_port)

## 4. 溫控迴圈的 Task（golden `bthermo.cpp` `DoThermoReal`）

[讀取此節](../../hpi-temperature/references/controllers/configuration/references/golden-v912-channels.md#4-溫控迴圈的-taskgolden-bthermocpp-dothermoreal)
