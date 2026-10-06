---
name: ht9045-bin-display
description: HT9045 / HT9050 的 Bin Display（每個出料盤位置一顆的外接數字顯示器，NUMBER_PANEL）知識庫：NUMBER_PANEL_TYPE 0～4（1／2＝DIO 脈衝面板、3＝三色七段 Modbus-ASCII、4＝TFT 20 位元組封包）、TComm 9600 8N1＋DTR／RTS、[NUMBER_PANEL]／[NUMBER_PANEL2] COM_PORT、Magazine 顯示器（MAGAZINE_BIN_DISP_TYPE：HT-A18／HT-BT008／TFT）、config.ini 的 C14／G16／P66、golden 912 BinDisplay\MyBinDisp.cpp 的 TDataModule3／TMyBinDispCtrl／TMyBinDispHT9046 函式地圖與 Timer1Timer（200 ms）狀態機、盤狀態→DoShowBinDigital→WriteTargetBin→封包→ack 的資料流、fShowBinSelect「Bin Display Status」分頁（tsUnloadMap、grpBinDisp）的 ChangeBinDispStatus 顏色與字（L／E／C、X、黑紅閃、藍）、全部呼叫者、906 與 912 差異（**基準＝906**，RULINGS_20261002 第 20 條；912 才有的 TFT 修正等不移植）、V906 移植現況（20261003：St02 的 C14 照 golden 906 做完——MR !127 C++ bring-up `1616a034`、MR !131 網頁分頁 `9723b74e`，在筆電第 49／50 批、還沒進 main；main 上仍是 TMyBinDispOffline、面板全灰 X、網頁是佔位）、golden 怪癖（type 3 也送 14 幀 Magazine-TFT 恢復封包）、golden 缺陷、排錯、碰這塊的規則。Use when：問 Bin 顯示器／數字顯示器／七段顯示器／TFT 顯示器、面板顯示 X 或 0 或顏色不對、「Bin display got error!!」、「Please check bin display. It have communication error!」、COM 開不起來或接錯埠、要移植或審查 MyBinDisp／ChangeBinDispStatus／DoShowBinDigital、要做 tsUnloadMap 網頁、HT9050 的 COM1 BIN／COM2 Multi Bin。關鍵字：Bin Display, BinDisp, BinDisp2, Bin 顯示器, 數字顯示器, 七段顯示器, 三色七段, NUMBER_PANEL, NUMBER_PANEL_TYPE, NUMBER_PANEL_DELAY, NUMBER_PANEL2, COM_PORT, TMyBinDispCtrl, TMyBinDispHT9046, TMyBinDispOffline, TDataModule3, DataModule3, TComm, SPComm, Spcomm, BinDisCtrl, HSys.BinDisCtrl, InstallColorBinDisplay, SystemModularInitial, Timer1Timer, iBinDispCtrlTask, DoStartGetStatus, DoStartSetColor, DoStartSetBin, DoOnce, DoOnceTFT, DoCycle, DoCycleTFT, WriteTargetBin, WriteTargetCount, ProcessStopStart, InitialTask, GetColorNow, GetBinNow, GerErrNow, UnitHasInstall, GetRunStatus, ChangeBinDispStatus, DoShowBinDigital, ShowBinDigital, InitShowBinDigital, bUpdateBinDigital, ShowBinSel, tsUnloadMap, Bin Display Status, grpBinDisp, UnLoadPanel, sbRunStatus, eBinDispName, eBinDispTotal, MAX_BIN_UNIT, iAddArrayTFT, command_TFT_Input, command_TFT_Font, A_Create_LCR, A_Create_LRC, GetCOMPortStatus, SwLoaderBin, MAGAZINE_BIN_DISP_TYPE, eMagBinDispType, HT-A18, HT-BT008, AUTO_EMPTY_COLOR, AUTO3_IS_MAGAZINE, bC14SaveBinDisplayLog, BinDisplayLog, bG16BinDispNeedAlarm, bP66AutoChangingFlashWarn, FlashPro, ClearAutoChangingWarn, bBinDispAlarm, TFT, Modbus-ASCII, LRC, RS-485, COM1 BIN, Multi Bin, BinDispTester, ST02-C14, W906-FW-BINDISP, GATE D1-D11, BinDispBringUp_St02, WebBinDispStatus_St02, W906_BinDispShownScope_St02, binsel.disp, ht9045_bindisp_status.js, C14_BinDisp, C14_BinDispPane, C14_BinDispPanePage, MagazineWriteBinFont_TFT, Magazine-TFT 恢復封包, MR !127, MR !131。
---

# ht9045-bin-display 相容入口

同主題已整合到 [hpi-bin-display](../hpi-bin-display/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-bin-display/references/source/original-entry.md)

## 0. 路徑與記號

[讀取此節](../hpi-bin-display/references/source/original-entry.md#0-路徑與記號)

## 1. 硬體（細節 `references/hardware-and-protocol.md`）

[讀取此節](../hpi-bin-display/references/source/original-entry.md#1-硬體細節-referenceshardware-and-protocolmd)

## 2. golden 程式地圖（細節 `references/golden-code-map.md`）

[讀取此節](../hpi-bin-display/references/source/original-entry.md#2-golden-程式地圖細節-referencesgolden-code-mapmd)

## 3. 906 與 912 的差異（基準＝906；下表「912 才有」的都**不移植**，RULINGS_20261002 第 20 條）

[讀取此節](../hpi-bin-display/references/source/original-entry.md#3-906-與-912-的差異基準906下表912-才有的都不移植rulings_20261002-第-20-條)

## 4. V906 移植現況（main＝20261002 量、20261003 重量相同；C14 的 MR 見 §4.1；細節 `references/port-status.md`）

[讀取此節](../hpi-bin-display/references/source/original-entry.md#4-v906-移植現況main20261002-量20261003-重量相同c14-的-mr-見-41細節-referencesport-statusmd)

### 4.1 St02 的 C14（照 golden 906_0625，20261003；**兩張 MR 都還沒進 main**）

[讀取此節](../hpi-bin-display/references/source/original-entry.md#41-st02-的-c14照-golden-906_062520261003兩張-mr-都還沒進-main)

## 5. 排錯速查（細節 `references/troubleshooting.md`）

[讀取此節](../hpi-bin-display/references/source/original-entry.md#5-排錯速查細節-referencestroubleshootingmd)

## 6. 碰這塊的規則

[讀取此節](../hpi-bin-display/references/source/original-entry.md#6-碰這塊的規則)

## 7. 相關 skill

[讀取此節](../hpi-bin-display/references/source/original-entry.md#7-相關-skill)
