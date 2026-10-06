# PISO-MN200 MotionNet API 參考

舊引用路徑保留；[讀取整理後文件](../../hpi-motor-control/references/control/references/mn200-api.md)。

## 系統架構

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/01.md#系統架構)

### 通訊線拓撲

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/01.md#通訊線拓撲)

### 全域變數（HT9045 使用）

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/01.md#全域變數ht9045-使用)

## 速度參數結構

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/02.md#速度參數結構)

### MaxSpeed 列舉

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/02.md#maxspeed-列舉)

### AccDec_Mode 模式

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/02.md#accdec_mode-模式)

## 系統初始化函式

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/03.md#系統初始化函式)

### mn_open_all

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/03.md#mn_open_all)

### mn_close_all

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/03.md#mn_close_all)

### mn_start_line

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/03.md#mn_start_line)

### mn_stop_line

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/03.md#mn_stop_line)

### mn_reset

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/03.md#mn_reset)

### mn_set_comm_speed

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/03.md#mn_set_comm_speed)

## 硬體配置函式

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/04.md#硬體配置函式)

### mn_set_motion_cfg

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/04.md#mn_set_motion_cfg)

### MotionConfig 配置項目

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/04.md#motionconfig-配置項目)

### 脈波輸出模式（PULSE_MODE）

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/04.md#脈波輸出模式pulse_mode)

### 編碼器模式（ENC_MODE）

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/04.md#編碼器模式enc_mode)

### mn_set_softlimit

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/04.md#mn_set_softlimit)

### mn_servo_on

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/04.md#mn_servo_on)

### mn_alarm_reset

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/04.md#mn_alarm_reset)

## 獨立運動函式

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/05.md#獨立運動函式)

### mn_fix_move

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/05.md#mn_fix_move)

### mn_velocity_move

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/05.md#mn_velocity_move)

### mn_stop_move

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/05.md#mn_stop_move)

### mn_change_v

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/05.md#mn_change_v)

### mn_change_p

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/05.md#mn_change_p)

## 原點搜尋函式

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/06.md#原點搜尋函式)

### mn_home_start

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/06.md#mn_home_start)

### mn_leave_home

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/06.md#mn_leave_home)

### mn_home_search

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/06.md#mn_home_search)

## 補間運動函式

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/07.md#補間運動函式)

### mn_line2_move

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/07.md#mn_line2_move)

### mn_line3_move

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/07.md#mn_line3_move)

### mn_linen_move

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/07.md#mn_linen_move)

### mn_arc2_move

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/07.md#mn_arc2_move)

## 群組運動函式

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/08.md#群組運動函式)

### mn_set_group

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/08.md#mn_set_group)

### mn_group_stop_move

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/08.md#mn_group_stop_move)

### mn_group_hold_move

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/08.md#mn_group_hold_move)

### mn_group_start_move

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/08.md#mn_group_start_move)

## 狀態函式

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/09.md#狀態函式)

### mn_motion_done

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/09.md#mn_motion_done)

### mn_get_cmdcounter

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/09.md#mn_get_cmdcounter)

### mn_get_enccounter

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/09.md#mn_get_enccounter)

### mn_set_cmdcounter

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/09.md#mn_set_cmdcounter)

### mn_set_enccounter

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/09.md#mn_set_enccounter)

### mn_get_speed

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/09.md#mn_get_speed)

### mn_get_mdio_status

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/09.md#mn_get_mdio_status)

### MOTION_IO 結構

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/09.md#motion_io-結構)

### mn_get_error_status

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/09.md#mn_get_error_status)

### 錯誤狀態位元

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/09.md#錯誤狀態位元)

## 六軸步進模組專用函式

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/10.md#六軸步進模組專用函式)

### mn_step6_set_home_check

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/10.md#mn_step6_set_home_check)

### mn_step6_set_micro_step

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/10.md#mn_step6_set_micro_step)

### mn_step6_set_current

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/10.md#mn_step6_set_current)

## 數位 I/O 函式

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/11.md#數位-io-函式)

### 並列 I/O（板卡端）

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/11.md#並列-io板卡端)

### 串列 I/O（Bit 操作）

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/11.md#串列-iobit-操作)

### 串列 I/O（Byte 操作）

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/11.md#串列-iobyte-操作)

### 串列 I/O（Word 操作）

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/11.md#串列-ioword-操作)

## 類比 I/O 函式

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/12.md#類比-io-函式)

### mn_set_ao

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/12.md#mn_set_ao)

### mn_get_ai

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/12.md#mn_get_ai)

### mn_get_ai_all

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/12.md#mn_get_ai_all)

## 錯誤碼參考

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/13.md#錯誤碼參考)

### 系統錯誤碼

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/13.md#系統錯誤碼)

### 速度參數錯誤

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/13.md#速度參數錯誤)

### 運動錯誤

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/13.md#運動錯誤)

### 插補錯誤

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/13.md#插補錯誤)

### 通訊錯誤

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/13.md#通訊錯誤)

## HT9045 使用範例

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/14.md#ht9045-使用範例)

### 初始化馬達

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/14.md#初始化馬達)

### 絕對位置移動

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/14.md#絕對位置移動)

### JOG 移動

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/14.md#jog-移動)

### 讀取狀態

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/14.md#讀取狀態)

## 參考資源

[讀取此節](../../hpi-motor-control/references/control/references/mn200-api/15.md#參考資源)
