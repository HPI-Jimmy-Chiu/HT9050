# ADAM-6024 硬體、EP 安裝型態、APAX、ADAMTCP.dll（細節）

舊引用路徑保留；[讀取整理後文件](../../hpi-ep-pressure/references/adam/references/hardware-and-dll.md)。

## 1. ADAM-6024 本體（研華 12 通道萬用 I/O 模組）

[讀取此節](../../hpi-ep-pressure/references/adam/references/hardware-and-dll.md#1-adam-6024-本體研華-12-通道萬用-io-模組)

### 1.1 Modbus 位址（手冊 p.240，B.2.4 ADAM-6024）

[讀取此節](../../hpi-ep-pressure/references/adam/references/hardware-and-dll.md#11-modbus-位址手冊-p240b24-adam-6024)

### 1.2 ASCII 指令（UDP，經 `ADAMTCP_SendReceive6KUDPCmd`）

[讀取此節](../../hpi-ep-pressure/references/adam/references/hardware-and-dll.md#12-ascii-指令udp經-adamtcp_sendreceive6kudpcmd)

## 2. 手臂機台怎麼用這顆模組（golden 912）

[讀取此節](../../hpi-ep-pressure/references/adam/references/hardware-and-dll.md#2-手臂機台怎麼用這顆模組golden-912)

### 2.1 三個 IP（912 `adam6024.cpp:42`、`:400-409`）

[讀取此節](../../hpi-ep-pressure/references/adam/references/hardware-and-dll.md#21-三個-ip912-adam6024cpp42400-409)

## 3. EP 安裝型態

[讀取此節](../../hpi-ep-pressure/references/adam/references/hardware-and-dll.md#3-ep-安裝型態)

### 3.1 `EP_Install`（[System] `EP_Install`，golden 912 `main.cpp:1772-1786` 讀；設定畫面 `HandlerSys.dfm` 的 `ElectronPressure` 選項，`HandlerSys.cpp:591` 寫回）

[讀取此節](../../hpi-ep-pressure/references/adam/references/hardware-and-dll.md#31-ep_installsystem-ep_installgolden-912-maincpp1772-1786-讀設定畫面-handlersysdfm-的-electronpressure-選項handlersyscpp591-寫回)

### 3.2 `INSTALL_DOUBLE_EP`（[System]，golden 912 `database.cpp:1106`；常數 `cmydef.h:4163-4166`；畫面 `rgDoubleEPControl`）

[讀取此節](../../hpi-ep-pressure/references/adam/references/hardware-and-dll.md#32-install_double_epsystemgolden-912-databasecpp1106常數-cmydefh4163-4166畫面-rgdoubleepcontrol)

### 3.3 APAX（獨立 EP／Multi EP）

[讀取此節](../../hpi-ep-pressure/references/adam/references/hardware-and-dll.md#33-apax獨立-epmulti-ep)

## 4. ADAMTCP.dll（廠商 DLL）

[讀取此節](../../hpi-ep-pressure/references/adam/references/hardware-and-dll.md#4-adamtcpdll廠商-dll)

### 4.1 golden 912 實際呼叫的 11 個進入點

[讀取此節](../../hpi-ep-pressure/references/adam/references/hardware-and-dll.md#41-golden-912-實際呼叫的-11-個進入點)

### 4.2 回傳碼（`ADAMTCP.h:126-143`；`E:\HT9045W_相關料件技術文件\ADAM\ADAM\ADAMTCP_SendReceive6KUDPCmd_Error Code.txt` 同一份）

[讀取此節](../../hpi-ep-pressure/references/adam/references/hardware-and-dll.md#42-回傳碼adamtcph126-143eht9045w_相關料件技術文件adamadamadamtcp_sendreceive6kudpcmd_error-codetxt-同一份)

### 4.3 逾時與阻塞（golden 912）

[讀取此節](../../hpi-ep-pressure/references/adam/references/hardware-and-dll.md#43-逾時與阻塞golden-912)

## 5. 相關 DO／DI（golden 名稱）

[讀取此節](../../hpi-ep-pressure/references/adam/references/hardware-and-dll.md#5-相關-dodigolden-名稱)
