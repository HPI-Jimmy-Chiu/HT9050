---
name: ht9045-adam6024
description: HT9045 / HT9050 的 ADAM-6024（研華 12 通道萬用 I/O 模組）與 EP 電氣比例閥（下壓力道 Contact Force／Die Force 的設定值與回授）知識庫：硬體規格與 Modbus 位址、ASCII／UDP 指令、EP_Install 各型態（3＝ADAM-6024 含回授、4＝PISO ET-7226、5＝Two EP…）、APAX 獨立／Multi EP、廠商 ADAMTCP.dll（匯出、回傳碼、逾時）、golden 912 adam6024.cpp 49 個函式地圖與呼叫者、公斤→TransformFuntion→輸出碼→ADAM_DirectWriteData／APAX_WriteData 的寫出流程、AI→ADAM_ReadPA→KpaTransferKG→警報的讀回流程、906 與 912 差異、V906 移植現況（20261003：St02 照 912 翻的 ADAM 碼已在 main（MR !114，`9e46491f`），RULINGS_20261002 第 20c 條保留 912；EP 寫出由 `W906_ADAM_EP_LIVE` 關著；機台 WinLibs g++ 16.2 字面值精度的 A3 修正是 MR !135 `2428cf0a`，還沒進 main）、連線／壓力排錯與警報意義、碰這塊的規則。Use when：問 ADAM-6024 或 EP 氣壓、EP 設定值寫不出去、EP 回授不準、WAR1605／WAR16322／WAR16323／WAR0329、Connect Fail! Please Check ADAM IP!、Adam Connect Error and Stop Home、ADAM 連線數滿 8 條、韌體 6.01 B21、要移植或審查 adam6024、ADAMTCP.dll 執行時載入、atester_shims 的 ADAM 替身、Timer2 的 EP 寫出、HT9050 的 172.16.8.110。關鍵字：ADAM-6024, ADAM6024, adam6024.cpp, ADAMTCP, ADAMTCP.dll, ADAMTCP_WriteReg, ADAMTCP_Read6KAI, ADAMTCP_SendReceive6KUDPCmd, ADAMTCP_Connect, EP, 電氣比例閥, 電子調壓閥, EP_Install, INSTALL_DOUBLE_EP, CHECK_EP_SETTING, EP_MAXKPA, EP_MINA_FeedBack, TransformFuntion, KpaTransferKG, MultiTransferKG, AdamOutputToPA, ADAM_ReadPA, ADAM_ReadVoltage, ADAM_WriteVoltage, ADAM_DirectWriteData, ADAM_WriteMaxData, ADAM_Alarm, ADAM_DualAlarm, ADAM_ReturnValueCheck, ADAM_ReadAIValue, Open_ADAM_6024, Close_ADAM_6024, fCheckConnectStatus_ADAM6024, ClearAllConnection, APAX, APAX_WriteData, APAX-5070, ADSMOD, ET-7226, PISO DA, EPSwitchOnOff, EpSwitch, SwEpArm1, SwMultiEp, SnEPDieForce, 露點計, DewPoint, 172.16.8.110, 172.16.8.111, 172.16.8.112, Modbus/TCP 502, iWritePA, iAdamOutValue, iReadAdamEP, ST02-ADAM, AdamTcpShim, Adam6024_St02.h, 75756cab, 3c348627, GATE FW3-WA, W906-HOME-C2-GPIBADAM, 第 20c 條, #20c, W906_ADAM_EP_LIVE, W906_AdamEpLive, MR !114, 9e46491f, WinLibs, g++ 16.2.0, excess precision, FLT_EVAL_METHOD, A3, P18, MR !135, 2428cf0a, Adam6024_Pressure, FP_ORACLE_FINDINGS。
---

# ht9045-adam6024 相容入口

同主題已整合到 [hpi-ep-pressure](../hpi-ep-pressure/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-ep-pressure/references/adam/original-entry.md)

## 0. 路徑與記號

[讀取此節](../hpi-ep-pressure/references/adam/original-entry.md#0-路徑與記號)

## 1. 硬體是什麼、機台拿它做什麼

[讀取此節](../hpi-ep-pressure/references/adam/original-entry.md#1-硬體是什麼機台拿它做什麼)

## 2. 廠商 DLL：ADAMTCP.dll

[讀取此節](../hpi-ep-pressure/references/adam/original-entry.md#2-廠商-dlladamtcpdll)

## 3. golden 程式地圖（912）

[讀取此節](../hpi-ep-pressure/references/adam/original-entry.md#3-golden-程式地圖912)

## 4. 906 與 912 的差異（adam6024 只差 82 行 diff）

[讀取此節](../hpi-ep-pressure/references/adam/original-entry.md#4-906-與-912-的差異adam6024-只差-82-行-diff)

## 5. V906 移植現況（20261002；20261003 補 §5.3 的合併狀態與 §5.4）

[讀取此節](../hpi-ep-pressure/references/adam/original-entry.md#5-v906-移植現況2026100220261003-補-53-的合併狀態與-54)

### 5.1 main（origin/main `36f09560`）上有什麼

[讀取此節](../hpi-ep-pressure/references/adam/original-entry.md#51-mainoriginmain-36f09560上有什麼)

### 5.2 筆電的第一次嘗試與撤回（都在 main 的歷史裡）

[讀取此節](../hpi-ep-pressure/references/adam/original-entry.md#52-筆電的第一次嘗試與撤回都在-main-的歷史裡)

### 5.3 St02 的移植（MR !114，**20261002 18:51 已合進 main `9e46491f`**；第 20c 條保留 912）

[讀取此節](../hpi-ep-pressure/references/adam/original-entry.md#53-st02-的移植mr-11420261002-1851-已合進-main-9e46491f第-20c-條保留-912)

### 5.4 機台編譯器 WinLibs g++ 16.2.0 的字面值精度（A3，MR !135，**還沒進 main**）

[讀取此節](../hpi-ep-pressure/references/adam/original-entry.md#54-機台編譯器-winlibs-g-1620-的字面值精度a3mr-135還沒進-main)

## 6. 排錯速查（細節 `references/troubleshooting.md`）

[讀取此節](../hpi-ep-pressure/references/adam/original-entry.md#6-排錯速查細節-referencestroubleshootingmd)

## 7. 碰這塊的規則

[讀取此節](../hpi-ep-pressure/references/adam/original-entry.md#7-碰這塊的規則)

## 8. 相關 skill

[讀取此節](../hpi-ep-pressure/references/adam/original-entry.md#8-相關-skill)
