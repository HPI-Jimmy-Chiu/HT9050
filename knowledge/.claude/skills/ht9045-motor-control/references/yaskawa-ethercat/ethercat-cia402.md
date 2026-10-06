# EtherCAT 與 CiA402：SGDXS 這一端的行為

舊引用路徑保留；[讀取整理後文件](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402.md)。

## 1. 通訊規格與身分

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/01.md#1-通訊規格與身分)

## 2. ESM（EtherCAT State Machine）

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/02.md#2-esmethercat-state-machine)

## 3. 同步模式（Free-run／DC）

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/03.md#3-同步模式free-rundc)

## 4. PDO

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/04.md#4-pdo)

## 5. CiA402 狀態機（6040h／6041h）

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/05.md#5-cia402-狀態機6040h6041h)

### 5.1 狀態轉移（依 p.600 圖判讀＋圖註）

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/05.md#51-狀態轉移依-p600-圖判讀圖註)

### 5.2 Controlword 6040h（UINT、RW、PDO、預設 0、不存 EEPROM，p.665）

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/05.md#52-controlword-6040huintrwpdo預設-0不存-eepromp665)

### 5.3 Statusword 6041h（UINT、RO、PDO，p.667-670）

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/05.md#53-statusword-6041huintropdop667-670)

### 5.4 停止選項碼（p.670-672；全部 INT、RW、不能 PDO、存 EEPROM）

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/05.md#54-停止選項碼p670-672全部-intrw不能-pdo存-eeprom)

### 5.5 伺服 ON 前後的時序（手冊）

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/05.md#55-伺服-on-前後的時序手冊)

## 6. 運轉模式 6060h／6061h

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/06.md#6-運轉模式-6060h6061h)

## 7. 回原點（Homing）

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/07.md#7-回原點homing)

### 7.1 相關物件（p.613、p.676-677）

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/07.md#71-相關物件p613p676-677)

### 7.2 支援的方法（p.613-615、p.676）

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/07.md#72-支援的方法p613-615p676)

### 7.3 method 24／28 的三種起點（依 p.615 圖判讀）

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/07.md#73-method-2428-的三種起點依-p615-圖判讀)

### 7.4 回原點時的 6041h（p.669）

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/07.md#74-回原點時的-6041hp669)

### 7.5 回原點中遇超程（p.204）

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/07.md#75-回原點中遇超程p204)

### 7.6 原點偏移與軟體極限

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/07.md#76-原點偏移與軟體極限)

## 8. Touch probe（60B8h～60BDh，p.622-623、p.689-691）

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/08.md#8-touch-probe60b8h60bdhp622-623p689-691)

## 9. 數位 I/O（60FDh／60FEh，p.692-693）

[讀取此節](../../../hpi-motor-control/references/control/references/yaskawa-ethercat/ethercat-cia402/09.md#9-數位-io60fdh60fehp692-693)
