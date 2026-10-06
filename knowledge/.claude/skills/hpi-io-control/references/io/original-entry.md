# HT9045 IO 控制層

按需要選取以下章節，原文依順序保留。

- [HT9045 IO 控制層](original-entry/00.md)
- [相關技能與代理](original-entry/01.md)
- [快速參考](original-entry/02.md)
- [IO 基底類型 (`ISABase`)](original-entry/03.md)
- [IO 表 Enable 技巧（停用與擴充保留）](original-entry/04.md)
- [MotionNet 架構說明](original-entry/05.md)
- [EtherCAT PCI-1203 架構說明](original-entry/06.md)
- [核心使用模式](original-entry/07.md)
- [安全門檢查](original-entry/08.md)
- [Kit IC 追蹤](original-entry/09.md)
- [Switch-Case 任務流程](original-entry/10.md)
- [⚠ IO 畫面（`fiosetview`）的 GUI thread 阻塞風險](original-entry/11.md)
- [詳細參考](original-entry/12.md)
- [工具腳本](original-entry/13.md)
- [合併補充：repo 既有參考（20261001）](original-entry/14.md)

# HT9045 IO 控制層

[讀取此節](original-entry/00.md#ht9045-io-控制層)

## 相關技能與代理

[讀取此節](original-entry/01.md#相關技能與代理)

## 快速參考

[讀取此節](original-entry/02.md#快速參考)

## IO 基底類型 (`ISABase`)

[讀取此節](original-entry/03.md#io-基底類型-isabase)

## IO 表 Enable 技巧（停用與擴充保留）

[讀取此節](original-entry/04.md#io-表-enable-技巧停用與擴充保留)

### 正確填法：整列欄位全部留空，只填 `Enable=0`

[讀取此節](original-entry/04.md#正確填法整列欄位全部留空只填-enable0)

### ⚠️ 陷阱：`ISABase=ePLCbase(4)` 會強制覆寫 `Enable=1`

[讀取此節](original-entry/04.md#-陷阱isabaseeplcbase4-會強制覆寫-enable1)

### 為什麼「全部留空」安全（防護鏈）

[讀取此節](original-entry/04.md#為什麼全部留空安全防護鏈)

### 停用後的行為

[讀取此節](original-entry/04.md#停用後的行為)

### 日後啟用（擴充保留）

[讀取此節](original-entry/04.md#日後啟用擴充保留)

### 停用前必查：有沒有「檢查 Enable 就擋機」的程式

[讀取此節](original-entry/04.md#停用前必查有沒有檢查-enable-就擋機的程式)

### 檢查清單

[讀取此節](original-entry/04.md#檢查清單)

## MotionNet 架構說明

[讀取此節](original-entry/05.md#motionnet-架構說明)

### 系統限制（共通）

[讀取此節](original-entry/05.md#系統限制共通)

### 定址格式

[讀取此節](original-entry/05.md#定址格式)

### 常用 DIO Slave 模組

[讀取此節](original-entry/05.md#常用-dio-slave-模組)

### 注意事項

[讀取此節](original-entry/05.md#注意事項)

## EtherCAT PCI-1203 架構說明

[讀取此節](original-entry/06.md#ethercat-pci-1203-架構說明)

### 系統限制

[讀取此節](original-entry/06.md#系統限制)

### 初始化流程

[讀取此節](original-entry/06.md#初始化流程)

### 與 TLaneIO 整合

[讀取此節](original-entry/06.md#與-tlaneio-整合)

### HT9050：1203 的 IO 點、急停、安全門（20261005，ST01-E）

[讀取此節](original-entry/06.md#ht90501203-的-io-點急停安全門20261005st01-e)

## 核心使用模式

[讀取此節](original-entry/07.md#核心使用模式)

### 氣缸 Push/Pop

[讀取此節](original-entry/07.md#氣缸-pushpop)

### 感測器讀取

[讀取此節](original-entry/07.md#感測器讀取)

### 開關輸出

[讀取此節](original-entry/07.md#開關輸出)

### 真空吸取控制

[讀取此節](original-entry/07.md#真空吸取控制)

### 底層 IO (TLaneIO)

[讀取此節](original-entry/07.md#底層-io-tlaneio)

### 傳統 TTL IO (myio.cpp)

[讀取此節](original-entry/07.md#傳統-ttl-io-myiocpp)

## 安全門檢查

[讀取此節](original-entry/08.md#安全門檢查)

## Kit IC 追蹤

[讀取此節](original-entry/09.md#kit-ic-追蹤)

## Switch-Case 任務流程

[讀取此節](original-entry/10.md#switch-case-任務流程)

### TMyCylinder::Push() / Pop() 任務流程

[讀取此節](original-entry/10.md#tmycylinderpush--pop-任務流程)

### TMySucker::Suck() 任務流程

[讀取此節](original-entry/10.md#tmysuckersuck-任務流程)

### TMySucker::Destroy() 任務流程

[讀取此節](original-entry/10.md#tmysuckerdestroy-任務流程)

### TLaneIO::GetIOErrStr() 錯誤碼對照

[讀取此節](original-entry/10.md#tlaneiogetioerrstr-錯誤碼對照)

## ⚠ IO 畫面（`fiosetview`）的 GUI thread 阻塞風險

[讀取此節](original-entry/11.md#-io-畫面fiosetview的-gui-thread-阻塞風險)

### 1. 它是 modal，而且多數客戶沒有 Close 按鈕

[讀取此節](original-entry/11.md#1-它是-modal而且多數客戶沒有-close-按鈕)

### 2. 50ms Timer1 直接做阻塞式 Modbus/TCP（最容易卡死的一條）

[讀取此節](original-entry/11.md#2-50ms-timer1-直接做阻塞式-modbustcp最容易卡死的一條)

### 3. `FormClose` 丟例外 → modal 視窗永遠關不掉

[讀取此節](original-entry/11.md#3-formclose-丟例外--modal-視窗永遠關不掉)

### ★ 影片實證（2026-09-15 偉測 HHT-25）：卡的是「關不掉」，不是「卡住」

[讀取此節](original-entry/11.md#-影片實證2026-09-15-偉測-hht-25卡的是關不掉不是卡住)

#### 為什麼 EventLog 看不到那個例外（重要陷阱）

[讀取此節](original-entry/11.md#為什麼-eventlog-看不到那個例外重要陷阱)

#### 定位根因要拿的東西

[讀取此節](original-entry/11.md#定位根因要拿的東西)

### 判讀提示

[讀取此節](original-entry/11.md#判讀提示)

## 詳細參考

[讀取此節](original-entry/12.md#詳細參考)

## 工具腳本

[讀取此節](original-entry/13.md#工具腳本)

## 合併補充：repo 既有參考（20261001）

[讀取此節](original-entry/14.md#合併補充repo-既有參考20261001)
