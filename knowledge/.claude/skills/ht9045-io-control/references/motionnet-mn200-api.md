# MotionNet API 參考（ICP-DAS / 泓格 PISO-MN200）

舊引用路徑保留；[讀取整理後文件](../../hpi-io-control/references/io/references/motionnet-mn200-api.md)。

## 目錄

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/01.md#目錄)

## 系統限制

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/02.md#系統限制)

### 定址容量計算

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/02.md#定址容量計算)

## 系統架構

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/03.md#系統架構)

### Card ID 與通訊線編號對應

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/03.md#card-id-與通訊線編號對應)

## 初始化 API

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/04.md#初始化-api)

### 開啟與關閉

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/04.md#開啟與關閉)

### 取得板卡資訊

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/04.md#取得板卡資訊)

### 通訊線控制

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/04.md#通訊線控制)

### 通訊速度常數

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/04.md#通訊速度常數)

### 初始化流程

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/04.md#初始化流程)

## DIO 操作 API

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/05.md#dio-操作-api)

### 標準 DI/DO（含通訊狀態檢查）

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/05.md#標準-dido含通訊狀態檢查)

### 進階 DI/DO（高效能，無通訊檢查）

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/05.md#進階-dido高效能無通訊檢查)

### Port 編號對應

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/05.md#port-編號對應)

## 運動控制 API

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/06.md#運動控制-api)

### 基本運動

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/06.md#基本運動)

### 原點復歸

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/06.md#原點復歸)

### 群組運動

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/06.md#群組運動)

### 速度參數結構 (SPEED_PAR)

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/06.md#速度參數結構-speed_par)

### 運動方向常數

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/06.md#運動方向常數)

### 停止模式常數

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/06.md#停止模式常數)

### 固定脈波運動模式

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/06.md#固定脈波運動模式)

## 診斷 API

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/07.md#診斷-api)

### 通訊線狀態位元

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/07.md#通訊線狀態位元)

## 錯誤碼

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/08.md#錯誤碼)

### 通用錯誤

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/08.md#通用錯誤)

### 通訊錯誤

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/08.md#通訊錯誤)

### IO 錯誤

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/08.md#io-錯誤)

### 運動錯誤

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/08.md#運動錯誤)

### 速度參數錯誤

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/08.md#速度參數錯誤)

## BCB6 使用方式

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/09.md#bcb6-使用方式)

### 標頭檔

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/09.md#標頭檔)

### 連結庫

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/09.md#連結庫)

### 初始化範例

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/09.md#初始化範例)

### DIO 操作範例

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/09.md#dio-操作範例)

## 與 SYN-TEK 版本差異

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/10.md#與-syn-tek-版本差異)

## 與 TLaneIO 整合

[讀取此節](../../hpi-io-control/references/io/references/motionnet-mn200-api/11.md#與-tlaneio-整合)
