# 各種軸卡／IO 卡的開卡流程與錯誤確認對照（HT9045／HT9050）

按需要選取以下章節，原文依順序保留。

- [各種軸卡／IO 卡的開卡流程與錯誤確認對照（HT9045／HT9050）](card-init-and-health/00.md)
- [0. 架構先講一次](card-init-and-health/01.md)
- [1. 怎麼判斷一個軸／一個 IO 點用哪一種技術](card-init-and-health/02.md)
- [2. 開機時誰先開、誰後開](card-init-and-health/03.md)
- [3. 對照總表（一種技術一列）](card-init-and-health/04.md)
- [4. 各技術細節](card-init-and-health/05.md)
- [5. 共通：馬達警報碼怎麼讀、IO 錯誤記在哪裡](card-init-and-health/06.md)
- [6. 馬達或 IO 出問題時先問的問題](card-init-and-health/07.md)
- [7. 缺口（golden 與移植樹不同、還沒移植、沒有文件）](card-init-and-health/08.md)
- [8. 建議放的位置與指標行](card-init-and-health/09.md)

# 各種軸卡／IO 卡的開卡流程與錯誤確認對照（HT9045／HT9050）

[讀取此節](card-init-and-health/00.md#各種軸卡io-卡的開卡流程與錯誤確認對照ht9045ht9050)

## 0. 架構先講一次

[讀取此節](card-init-and-health/01.md#0-架構先講一次)

## 1. 怎麼判斷一個軸／一個 IO 點用哪一種技術

[讀取此節](card-init-and-health/02.md#1-怎麼判斷一個軸一個-io-點用哪一種技術)

### 1.1 先看 `D:\HT9045\system\Gerneral.ini` 的卡別鍵

[讀取此節](card-init-and-health/02.md#11-先看-dht9045systemgerneralini-的卡別鍵)

### 1.2 馬達：讀哪一份表、哪一欄決定類別（golden `cinitial.cpp InitialMotorParameter`）

[讀取此節](card-init-and-health/02.md#12-馬達讀哪一份表哪一欄決定類別golden-cinitialcpp-initialmotorparameter)

### 1.3 IO：`IO_Table.csv` 的 `ISABase` 欄（golden `MachineType.h enum eIOType`）

[讀取此節](card-init-and-health/02.md#13-ioio_tablecsv-的-isabase-欄golden-machinetypeh-enum-eiotype)

### 1.4 每種技術一列範例

[讀取此節](card-init-and-health/02.md#14-每種技術一列範例)

## 2. 開機時誰先開、誰後開

[讀取此節](card-init-and-health/03.md#2-開機時誰先開誰後開)

### 2.1 golden 906（`main.cpp TfMain::FormShow`）

[讀取此節](card-init-and-health/03.md#21-golden-906maincpp-tfmainformshow)

### 2.2 移植樹（`tools\wb_serve.cpp` 的 `main`）

[讀取此節](card-init-and-health/03.md#22-移植樹toolswb_servecpp-的-main)

## 3. 對照總表（一種技術一列）

[讀取此節](card-init-and-health/04.md#3-對照總表一種技術一列)

## 4. 各技術細節

[讀取此節](card-init-and-health/05.md#4-各技術細節)

### 4.1 PCIE-1203 EtherCAT 馬達（`TMyEtherCatMotor`）

[讀取此節](card-init-and-health/05.md#41-pcie-1203-ethercat-馬達tmyethercatmotor)

### 4.2 PCIE-1203 IO（ISABase 3）

[讀取此節](card-init-and-health/05.md#42-pcie-1203-ioisabase-3)

### 4.3 MotionNet 先達 PCI-L112／L122（`OpenPCI132Card` 的先達段）

[讀取此節](card-init-and-health/05.md#43-motionnet-先達-pci-l112l122openpci132card-的先達段)

### 4.4 MotionNet 泓格 PISO-MN200（`OpenPCI132Card` 的 PISO 段＋`TMyMN200Motor`）

[讀取此節](card-init-and-health/05.md#44-motionnet-泓格-piso-mn200openpci132card-的-piso-段tmymn200motor)

### 4.5 先達 motion slave（`TMySYNTEKMotor`＋`Hontech_M4`）

[讀取此節](card-init-and-health/05.md#45-先達-motion-slavetmysyntekmotorhontech_m4)

### 4.6 Galil DMC（Index 四軸）

[讀取此節](card-init-and-health/05.md#46-galil-dmcindex-四軸)

### 4.7 CONTEC SMC（`TMySMCMotor`）

[讀取此節](card-init-and-health/05.md#47-contec-smctmysmcmotor)

### 4.8 和椿 MC88X1（`HTMC88X1Motor`）

[讀取此節](card-init-and-health/05.md#48-和椿-mc88x1htmc88x1motor)

### 4.9 Tray 步進馬達（`TdmTrayMotor`，RS-232）

[讀取此節](card-init-and-health/05.md#49-tray-步進馬達tdmtraymotorrs-232)

### 4.10 其他 IO

[讀取此節](card-init-and-health/05.md#410-其他-io)

## 5. 共通：馬達警報碼怎麼讀、IO 錯誤記在哪裡

[讀取此節](card-init-and-health/06.md#5-共通馬達警報碼怎麼讀io-錯誤記在哪裡)

## 6. 馬達或 IO 出問題時先問的問題

[讀取此節](card-init-and-health/07.md#6-馬達或-io-出問題時先問的問題)

## 7. 缺口（golden 與移植樹不同、還沒移植、沒有文件）

[讀取此節](card-init-and-health/08.md#7-缺口golden-與移植樹不同還沒移植沒有文件)

### 7.1 golden 與移植樹不同

[讀取此節](card-init-and-health/08.md#71-golden-與移植樹不同)

### 7.2 還沒移植（移植樹是離線樁或閘住）

[讀取此節](card-init-and-health/08.md#72-還沒移植移植樹是離線樁或閘住)

### 7.3 golden 本身的疑似缺陷（照翻；都沒有上機驗證）

[讀取此節](card-init-and-health/08.md#73-golden-本身的疑似缺陷照翻都沒有上機驗證)

### 7.4 沒有文件，或文件有錯

[讀取此節](card-init-and-health/08.md#74-沒有文件或文件有錯)

## 8. 建議放的位置與指標行

[讀取此節](card-init-and-health/09.md#8-建議放的位置與指標行)
