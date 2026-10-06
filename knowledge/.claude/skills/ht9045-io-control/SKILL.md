---
name: ht9045-io-control
description: HT9045 Handler IO 控制層模式。適用於氣缸、感測器、開關、真空吸取器或底層 IO 操作。涵蓋 TMyCylinder、TMySensor、TMySwitch、TMySucker、TMyKitSuck、TLaneIO 及 myio 模組介面。用於 IO 修改、IO 問題除錯、新增氣缸/感測器/開關定義或理解 IO 控制流程。亦涵蓋 IO 資料庫設定：IO_Table.csv / Sensor_xxxx.DB 欄位定義、ISABase 對應值（eMotionNet 0 / eISABase 1 / ePCI1735U 2 / ePCI1203 3 / ePLCbase 4）、**IO 表 Enable 技巧**（以 Enable=0 停用訊號取代改程式加廠牌判斷、保留欄位供日後擴充、ISABase=4 會強制覆寫 Enable 的陷阱、全欄留空的防護鏈、停用前須查有無「檢查 Enable 就擋機」的防呆函式）、ePLCbase 的 Port 欄 16 進位解析與 InType 強制為 1。亦涵蓋 **IO 畫面（fiosetview）的 GUI thread 阻塞風險**：ShowModal 無 Close 鈕（僅 CC_Greatek 有）、50ms Timer1 直接做阻塞式 Modbus/TCP 讀 ADAM-6024（讀失敗就地整套重連、2000ms x3 逾時、失敗訊息走 MNetLog 不是 EventLog 故卡住卻零紀錄）、FormClose 丟例外會讓 VCL CloseModal 把 ModalResult 歸 0 導致視窗關不掉（「IO 畫面卡住退不出」）—— 2026-09-15 客戶錄影已證實本案屬後者（分頁可切、標題列無「沒有回應」、LED 持續重繪＝GUI thread 與 timer 均正常），且 AppException 的 process 級去重 static 會讓重複例外完全不進 EventLog。觸發關鍵字：IO 畫面卡住, IO 畫面退不出, fiosetview, Tfiosetview, FormClose, ShowModal, CloseModal, ADAM6024, ADAMTCP_Read6KAI, Open_ADAM_6024, EP_Install, labPA, IO 表, IO_Table.csv, Sensor DB, ISABase, ePLCbase, Enable=0, 停用感測器, 保留擴充, HexStrToInt, bPLCIO, 安全 PLC IO 設定。 另含 Exit（關閉程式）時的停機／關站（V906 移植樹 W906_Main_CloseProgramOp、ShutdownSequence、1203 輸出清零與讀回、未停清單、S121／S163／Q44）→ references/exit-shutdown.md。Use when：按 Exit 關不掉輸出、關站停機、SIM 建置接真卡、1203 頁打開的 DO 沒關、未停、必須先停下才能關閉、Ctrl-C 停機。關鍵字：Exit, 關閉程式, 關站, 停機, FormClose, sbCloseProgramClick, W906_Main_CloseProgramOp, ShutdownSequence, W906_ProdCloseShutdown, W906_ServeQuitDue, W906_ConsoleCtrl, Program Close, notStopped, 未停, SwHeaterRelay, SwMotorRelay, Pci1203RouteCanWriteBit, kCmdDoSetByte, Acm_DaqDoSetByteEx, 清零, S121, S163, R38, R39, R40, Q39, Q44。
---

# ht9045-io-control 相容入口

同主題已整合到 [hpi-io-control](../hpi-io-control/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-io-control/references/io/original-entry.md)

## 相關技能與代理

[讀取此節](../hpi-io-control/references/io/original-entry/01.md#相關技能與代理)

## 快速參考

[讀取此節](../hpi-io-control/references/io/original-entry/02.md#快速參考)

## IO 基底類型 (`ISABase`)

[讀取此節](../hpi-io-control/references/io/original-entry/03.md#io-基底類型-isabase)

## IO 表 Enable 技巧（停用與擴充保留）

[讀取此節](../hpi-io-control/references/io/original-entry/04.md#io-表-enable-技巧停用與擴充保留)

### 正確填法：整列欄位全部留空，只填 `Enable=0`

[讀取此節](../hpi-io-control/references/io/original-entry/04.md#正確填法整列欄位全部留空只填-enable0)

### ⚠️ 陷阱：`ISABase=ePLCbase(4)` 會強制覆寫 `Enable=1`

[讀取此節](../hpi-io-control/references/io/original-entry/04.md#-陷阱isabaseeplcbase4-會強制覆寫-enable1)

### 為什麼「全部留空」安全（防護鏈）

[讀取此節](../hpi-io-control/references/io/original-entry/04.md#為什麼全部留空安全防護鏈)

### 停用後的行為

[讀取此節](../hpi-io-control/references/io/original-entry/04.md#停用後的行為)

### 日後啟用（擴充保留）

[讀取此節](../hpi-io-control/references/io/original-entry/04.md#日後啟用擴充保留)

### 停用前必查：有沒有「檢查 Enable 就擋機」的程式

[讀取此節](../hpi-io-control/references/io/original-entry/04.md#停用前必查有沒有檢查-enable-就擋機的程式)

### 檢查清單

[讀取此節](../hpi-io-control/references/io/original-entry/04.md#檢查清單)

## MotionNet 架構說明

[讀取此節](../hpi-io-control/references/io/original-entry/05.md#motionnet-架構說明)

### 系統限制（共通）

[讀取此節](../hpi-io-control/references/io/original-entry/05.md#系統限制共通)

### 定址格式

[讀取此節](../hpi-io-control/references/io/original-entry/05.md#定址格式)

### 常用 DIO Slave 模組

[讀取此節](../hpi-io-control/references/io/original-entry/05.md#常用-dio-slave-模組)

### 注意事項

[讀取此節](../hpi-io-control/references/io/original-entry/05.md#注意事項)

## EtherCAT PCI-1203 架構說明

[讀取此節](../hpi-io-control/references/io/original-entry/06.md#ethercat-pci-1203-架構說明)

### 系統限制

[讀取此節](../hpi-io-control/references/io/original-entry/06.md#系統限制)

### 初始化流程

[讀取此節](../hpi-io-control/references/io/original-entry/06.md#初始化流程)

### 與 TLaneIO 整合

[讀取此節](../hpi-io-control/references/io/original-entry/06.md#與-tlaneio-整合)

### HT9050：1203 的 IO 點、急停、安全門（20261005，ST01-E）

[讀取此節](../hpi-io-control/references/io/original-entry/06.md#ht90501203-的-io-點急停安全門20261005st01-e)

## 核心使用模式

[讀取此節](../hpi-io-control/references/io/original-entry/07.md#核心使用模式)

### 氣缸 Push/Pop

[讀取此節](../hpi-io-control/references/io/original-entry/07.md#氣缸-pushpop)

### 感測器讀取

[讀取此節](../hpi-io-control/references/io/original-entry/07.md#感測器讀取)

### 開關輸出

[讀取此節](../hpi-io-control/references/io/original-entry/07.md#開關輸出)

### 真空吸取控制

[讀取此節](../hpi-io-control/references/io/original-entry/07.md#真空吸取控制)

### 底層 IO (TLaneIO)

[讀取此節](../hpi-io-control/references/io/original-entry/07.md#底層-io-tlaneio)

### 傳統 TTL IO (myio.cpp)

[讀取此節](../hpi-io-control/references/io/original-entry/07.md#傳統-ttl-io-myiocpp)

## 安全門檢查

[讀取此節](../hpi-io-control/references/io/original-entry/08.md#安全門檢查)

## Kit IC 追蹤

[讀取此節](../hpi-io-control/references/io/original-entry/09.md#kit-ic-追蹤)

## Switch-Case 任務流程

[讀取此節](../hpi-io-control/references/io/original-entry/10.md#switch-case-任務流程)

### TMyCylinder::Push() / Pop() 任務流程

[讀取此節](../hpi-io-control/references/io/original-entry/10.md#tmycylinderpush--pop-任務流程)

### TMySucker::Suck() 任務流程

[讀取此節](../hpi-io-control/references/io/original-entry/10.md#tmysuckersuck-任務流程)

### TMySucker::Destroy() 任務流程

[讀取此節](../hpi-io-control/references/io/original-entry/10.md#tmysuckerdestroy-任務流程)

### TLaneIO::GetIOErrStr() 錯誤碼對照

[讀取此節](../hpi-io-control/references/io/original-entry/10.md#tlaneiogetioerrstr-錯誤碼對照)

## ⚠ IO 畫面（`fiosetview`）的 GUI thread 阻塞風險

[讀取此節](../hpi-io-control/references/io/original-entry/11.md#-io-畫面fiosetview的-gui-thread-阻塞風險)

### 1. 它是 modal，而且多數客戶沒有 Close 按鈕

[讀取此節](../hpi-io-control/references/io/original-entry/11.md#1-它是-modal而且多數客戶沒有-close-按鈕)

### 2. 50ms Timer1 直接做阻塞式 Modbus/TCP（最容易卡死的一條）

[讀取此節](../hpi-io-control/references/io/original-entry/11.md#2-50ms-timer1-直接做阻塞式-modbustcp最容易卡死的一條)

### 3. `FormClose` 丟例外 → modal 視窗永遠關不掉

[讀取此節](../hpi-io-control/references/io/original-entry/11.md#3-formclose-丟例外--modal-視窗永遠關不掉)

### ★ 影片實證（2026-09-15 偉測 HHT-25）：卡的是「關不掉」，不是「卡住」

[讀取此節](../hpi-io-control/references/io/original-entry/11.md#-影片實證2026-09-15-偉測-hht-25卡的是關不掉不是卡住)

#### 為什麼 EventLog 看不到那個例外（重要陷阱）

[讀取此節](../hpi-io-control/references/io/original-entry/11.md#為什麼-eventlog-看不到那個例外重要陷阱)

#### 定位根因要拿的東西

[讀取此節](../hpi-io-control/references/io/original-entry/11.md#定位根因要拿的東西)

### 判讀提示

[讀取此節](../hpi-io-control/references/io/original-entry/11.md#判讀提示)

## 詳細參考

[讀取此節](../hpi-io-control/references/io/original-entry/12.md#詳細參考)

## 工具腳本

[讀取此節](../hpi-io-control/references/io/original-entry/13.md#工具腳本)

## 合併補充：repo 既有參考（20261001）

[讀取此節](../hpi-io-control/references/io/original-entry/14.md#合併補充repo-既有參考20261001)
