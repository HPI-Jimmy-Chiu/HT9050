# Galil DMC API 參考

按需要選取以下章節，原文依順序保留。

- [Galil DMC API 參考](galil-api/00.md)
- [系統配置](galil-api/01.md)
- [DMC API 函式](galil-api/02.md)
- [Galil 命令字串](galil-api/03.md)
- [TS 開關狀態位元](galil-api/04.md)
- [錯誤碼](galil-api/05.md)
- [TMyMotor Galil 方法實作](galil-api/06.md)
- [雙 Z 軸協調實作](galil-api/07.md)
- [常用命令組合範例](galil-api/08.md)
- [標頭檔](galil-api/09.md)
- [詳細命令參考（官方手冊）](galil-api/10.md)
- [Operand 使用摘要](galil-api/11.md)
- [資源連結](galil-api/12.md)

# Galil DMC API 參考

[讀取此節](galil-api/00.md#galil-dmc-api-參考)

## 系統配置

[讀取此節](galil-api/01.md#系統配置)

### 全域變數

[讀取此節](galil-api/01.md#全域變數)

### 速度參數

[讀取此節](galil-api/01.md#速度參數)

### 軸名稱對應

[讀取此節](galil-api/01.md#軸名稱對應)

## DMC API 函式

[讀取此節](galil-api/02.md#dmc-api-函式)

### 卡片管理

[讀取此節](galil-api/02.md#卡片管理)

### 開啟卡片

[讀取此節](galil-api/02.md#開啟卡片)

## Galil 命令字串

[讀取此節](galil-api/03.md#galil-命令字串)

### 基本格式

[讀取此節](galil-api/03.md#基本格式)

### 運動命令

[讀取此節](galil-api/03.md#運動命令)

### 速度與加減速

[讀取此節](galil-api/03.md#速度與加減速)

### JOG 運動

[讀取此節](galil-api/03.md#jog-運動)

### 位置讀取

[讀取此節](galil-api/03.md#位置讀取)

### 狀態讀取

[讀取此節](galil-api/03.md#狀態讀取)

### 伺服控制

[讀取此節](galil-api/03.md#伺服控制)

### 極限與保護

[讀取此節](galil-api/03.md#極限與保護)

### 線性補間

[讀取此節](galil-api/03.md#線性補間)

## TS 開關狀態位元

[讀取此節](galil-api/04.md#ts-開關狀態位元)

### 範例

[讀取此節](galil-api/04.md#範例)

## 錯誤碼

[讀取此節](galil-api/05.md#錯誤碼)

### DMC 函式錯誤碼

[讀取此節](galil-api/05.md#dmc-函式錯誤碼)

### TC 錯誤碼（Galil 控制器）

[讀取此節](galil-api/05.md#tc-錯誤碼galil-控制器)

## TMyMotor Galil 方法實作

[讀取此節](galil-api/06.md#tmymotor-galil-方法實作)

### Gali_Command

[讀取此節](galil-api/06.md#gali_command)

### Gali_MotMove

[讀取此節](galil-api/06.md#gali_motmove)

### Gali_MotHome

[讀取此節](galil-api/06.md#gali_mothome)

### Gali_MotHomeFindZ

[讀取此節](galil-api/06.md#gali_mothomefindz)

## 雙 Z 軸協調實作

[讀取此節](galil-api/07.md#雙-z-軸協調實作)

### Z1UpZ2Down

[讀取此節](galil-api/07.md#z1upz2down)

### 四軸同動命令範例

[讀取此節](galil-api/07.md#四軸同動命令範例)

## 常用命令組合範例

[讀取此節](galil-api/08.md#常用命令組合範例)

### 單軸絕對移動

[讀取此節](galil-api/08.md#單軸絕對移動)

### 等待運動完成

[讀取此節](galil-api/08.md#等待運動完成)

### JOG 點動

[讀取此節](galil-api/08.md#jog-點動)

### 伺服 ON/OFF

[讀取此節](galil-api/08.md#伺服-onoff)

### 位置重置

[讀取此節](galil-api/08.md#位置重置)

## 標頭檔

[讀取此節](galil-api/09.md#標頭檔)

### 連結庫

[讀取此節](galil-api/09.md#連結庫)

## 詳細命令參考（官方手冊）

[讀取此節](galil-api/10.md#詳細命令參考官方手冊)

### 命令分類速查

[讀取此節](galil-api/10.md#命令分類速查)

### AC（Acceleration）加速度

[讀取此節](galil-api/10.md#acacceleration加速度)

### BG（Begin）開始運動

[讀取此節](galil-api/10.md#bgbegin開始運動)

### BL（Backward Limit）反向軟體極限

[讀取此節](galil-api/10.md#blbackward-limit反向軟體極限)

### DC（Deceleration）減速度

[讀取此節](galil-api/10.md#dcdeceleration減速度)

### DE（Define Encoder）定義編碼器位置

[讀取此節](galil-api/10.md#dedefine-encoder定義編碼器位置)

### DP（Define Position）定義位置

[讀取此節](galil-api/10.md#dpdefine-position定義位置)

### FI（Find Index）搜尋 Index

[讀取此節](galil-api/10.md#fifind-index搜尋-index)

### FL（Forward Limit）正向軟體極限

[讀取此節](galil-api/10.md#flforward-limit正向軟體極限)

### HM（Home）回原點

[讀取此節](galil-api/10.md#hmhome回原點)

### JG（Jog）點動

[讀取此節](galil-api/10.md#jgjog點動)

### LI（Linear Interpolation Distance）線性補間距離

[讀取此節](galil-api/10.md#lilinear-interpolation-distance線性補間距離)

### LM（Linear Interpolation Mode）線性補間模式

[讀取此節](galil-api/10.md#lmlinear-interpolation-mode線性補間模式)

### MO（Motor Off）馬達斷電

[讀取此節](galil-api/10.md#momotor-off馬達斷電)

### PA（Position Absolute）絕對位置移動

[讀取此節](galil-api/10.md#paposition-absolute絕對位置移動)

### PR（Position Relative）相對位置移動

[讀取此節](galil-api/10.md#prposition-relative相對位置移動)

### SC（Stop Code）停止碼

[讀取此節](galil-api/10.md#scstop-code停止碼)

### SH（Servo Here）伺服啟用

[讀取此節](galil-api/10.md#shservo-here伺服啟用)

### SP（Speed）速度

[讀取此節](galil-api/10.md#spspeed速度)

### ST（Stop）停止

[讀取此節](galil-api/10.md#ststop停止)

### TC（Tell Error Code）錯誤碼查詢

[讀取此節](galil-api/10.md#tctell-error-code錯誤碼查詢)

### TD（Tell Dual Encoder）查詢輔助編碼器

[讀取此節](galil-api/10.md#tdtell-dual-encoder查詢輔助編碼器)

### TP（Tell Position）查詢位置

[讀取此節](galil-api/10.md#tptell-position查詢位置)

### TS（Tell Switches）查詢開關狀態

[讀取此節](galil-api/10.md#tstell-switches查詢開關狀態)

### 向量運動命令

[讀取此節](galil-api/10.md#向量運動命令)

#### VA（Vector Acceleration）

[讀取此節](galil-api/10.md#vavector-acceleration)

#### VD（Vector Deceleration）

[讀取此節](galil-api/10.md#vdvector-deceleration)

#### VS（Vector Speed）

[讀取此節](galil-api/10.md#vsvector-speed)

## Operand 使用摘要

[讀取此節](galil-api/11.md#operand-使用摘要)

## 資源連結

[讀取此節](galil-api/12.md#資源連結)
