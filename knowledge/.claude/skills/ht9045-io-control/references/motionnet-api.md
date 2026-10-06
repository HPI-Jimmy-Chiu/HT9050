# MotionNet API 參考（SYN-TEK / 先達）

舊引用路徑保留；[讀取整理後文件](../../hpi-io-control/references/io/references/motionnet-api.md)。

## 目錄

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/01.md#目錄)

## 系統限制

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/02.md#系統限制)

### 定址容量計算

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/02.md#定址容量計算)

## 系統架構

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/03.md#系統架構)

### 通訊類型

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/03.md#通訊類型)

## 初始化 API

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/04.md#初始化-api)

### _m114g_initial

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/04.md#_m114g_initial)

### _l132_open / _l112_open

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/04.md#_l132_open--_l112_open)

### _m114g_open_mnet

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/04.md#_m114g_open_mnet)

### _mnet_start_ring

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/04.md#_mnet_start_ring)

### _mnet_stop_ring

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/04.md#_mnet_stop_ring)

### _mnet_reset_ring

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/04.md#_mnet_reset_ring)

### _mnet_close

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/04.md#_mnet_close)

### 初始化流程

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/04.md#初始化流程)

## DIO 操作 API

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/05.md#dio-操作-api)

### _mnet_io_output

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/05.md#_mnet_io_output)

### _mnet_io_input

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/05.md#_mnet_io_input)

## 診斷 API

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/06.md#診斷-api)

### _mnet_get_slave_type

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/06.md#_mnet_get_slave_type)

### _mnet_get_slave_info

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/06.md#_mnet_get_slave_info)

### _mnet_get_ring_active_table

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/06.md#_mnet_get_ring_active_table)

### _mnet_get_slave_error_table

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/06.md#_mnet_get_slave_error_table)

### _mnet_clear_slave_error_flag

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/06.md#_mnet_clear_slave_error_flag)

### _mnet_get_error_device

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/06.md#_mnet_get_error_device)

### _mnet_get_ring_status

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/06.md#_mnet_get_ring_status)

## 看門狗 API

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/07.md#看門狗-api)

### _mnet_enable_soft_watchdog

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/07.md#_mnet_enable_soft_watchdog)

### _mnet_disable_soft_watchdog

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/07.md#_mnet_disable_soft_watchdog)

### _mnet_watchdog_link

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/07.md#_mnet_watchdog_link)

### _mnet_get_com_status

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/07.md#_mnet_get_com_status)

### _mnet_set_ring_quality_param

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/07.md#_mnet_set_ring_quality_param)

### _mnet_clear_ring_error

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/07.md#_mnet_clear_ring_error)

## DIO Slave 模組類型

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/08.md#dio-slave-模組類型)

### 32-bit 模組（G9002 系列）

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/08.md#32-bit-模組g9002-系列)

### 16-bit 模組（G9102 系列）

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/08.md#16-bit-模組g9102-系列)

### 16-bit 模組（G9205 系列）

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/08.md#16-bit-模組g9205-系列)

### Motion Slave 模組

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/08.md#motion-slave-模組)

### Analog I/O 模組

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/08.md#analog-io-模組)

## 錯誤碼

[讀取此節](../../hpi-io-control/references/io/references/motionnet-api/09.md#錯誤碼)
