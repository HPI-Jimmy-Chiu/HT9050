# `cUnitConvert.h`：`_File` → 執行中 的轉換層（全部）

舊引用路徑保留；[讀取整理後文件](../../hpi-web-hmi/references/bridge/references/unit-convert-layer.md)。

## 一、它在做什麼

[讀取此節](../../hpi-web-hmi/references/bridge/references/unit-convert-layer.md#一它在做什麼)

## 二、七個函式逐一

[讀取此節](../../hpi-web-hmi/references/bridge/references/unit-convert-layer.md#二七個函式逐一)

### `DoTestIFConvert()`（`:27-60`）`TestIF_File` → `TestIF`

[讀取此節](../../hpi-web-hmi/references/bridge/references/unit-convert-layer.md#dotestifconvert27-60testif_file--testif)

### `DoDeviceConvert()`（`:62-75`）`DeviceForm_File` → `DeviceForm`

[讀取此節](../../hpi-web-hmi/references/bridge/references/unit-convert-layer.md#dodeviceconvert62-75deviceform_file--deviceform)

### `DoHotPlateConvert()`（`:77-91`）`HotPlateForm_File` → `HotPlateForm`

[讀取此節](../../hpi-web-hmi/references/bridge/references/unit-convert-layer.md#dohotplateconvert77-91hotplateform_file--hotplateform)

### `DoArmOffsetConvert()`（`:93-201`）`*ArmOffSet_File[i]` → `*ArmOffSet[i]`、`Offset_File` → `Offset`

[讀取此節](../../hpi-web-hmi/references/bridge/references/unit-convert-layer.md#doarmoffsetconvert93-201armoffset_filei--armoffsetioffset_file--offset)

### `DoArmSpeedConvert()`（`:203-216`）`ArmSpeed_File[i]` → `ArmSpeed[i]`、`SHSpeed_File` → `SHSpeed`、`MGSpeed_File` → `MGSpeed`

[讀取此節](../../hpi-web-hmi/references/bridge/references/unit-convert-layer.md#doarmspeedconvert203-216armspeed_filei--armspeedishspeed_file--shspeedmgspeed_file--mgspeed)

### `DoDefFormConvert()`（`:218-252`）`UserDefForm_File[i]` → `UserDefForm[i]`（`i<4`）

[讀取此節](../../hpi-web-hmi/references/bridge/references/unit-convert-layer.md#dodefformconvert218-252userdefform_filei--userdefformii4)

### `DoStructUnitConvert()`（`:254-271`）總指揮

[讀取此節](../../hpi-web-hmi/references/bridge/references/unit-convert-layer.md#dostructunitconvert254-271總指揮)

## 三、golden 什麼時候叫它（V912，24 個呼叫點）

[讀取此節](../../hpi-web-hmi/references/bridge/references/unit-convert-layer.md#三golden-什麼時候叫它v91224-個呼叫點)

## 四、移植樹現況（20260923）

[讀取此節](../../hpi-web-hmi/references/bridge/references/unit-convert-layer.md#四移植樹現況20260923)

## 五、對橋接層的意義

[讀取此節](../../hpi-web-hmi/references/bridge/references/unit-convert-layer.md#五對橋接層的意義)
