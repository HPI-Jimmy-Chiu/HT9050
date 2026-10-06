# HT9045 CatchTray Flow Knowledge

按需要選取以下章節，原文依順序保留。

- [HT9045 CatchTray Flow Knowledge](original-entry/00.md)
- [適用場景](original-entry/01.md)
- [專案資訊](original-entry/02.md)
- [參考文件](original-entry/03.md)
- [關鍵原始檔](original-entry/04.md)
- [1. 呼叫階層總覽](original-entry/05.md)
- [2. DoCatchTray() 主流程](original-entry/06.md)
- [3. 下一層函式 Case 摘要](original-entry/07.md)
- [4. 子函式 switch（下一層內）](original-entry/08.md)
- [使用指引](original-entry/09.md)
- [Local Preserved Notes (from previous local agent)](original-entry/10.md)
- [5. TrayX 座標排列（物理位置）](original-entry/11.md)
- [6. CatchTray 補盤狀態判斷（State Record 診斷用）](original-entry/12.md)
- [7. Auto Tray（出料盤）「收料 vs 退盤」：DoAutoReceiveBinTray ≠ DoReceiveAutoTray](original-entry/13.md)
- [8. DoLoad auto-clean-out 觸發條件 ＋「收工後補料」的坑（JSCC 死雞 code-confirmed 根因）](original-entry/14.md)
- [9. 分盤機構（緩衝站 Tray Separate：兩段舉升 + Selector 分離爪）](original-entry/15.md)
- [10. 「Loader 空盤放哪一軌 / Auto 空盤從哪一軌來」的三個判斷點（P04 陷阱）⚠](original-entry/16.md)
- [11. Color / Empty 軌「退盤序列」不可中途 Init ⚠（JSCC `CC_SCC` 專屬防護的回歸）](original-entry/17.md)
- [合併補充：repo 既有參考（20261001）](original-entry/18.md)

# HT9045 CatchTray Flow Knowledge

[讀取此節](original-entry/00.md#ht9045-catchtray-flow-knowledge)

## 適用場景

[讀取此節](original-entry/01.md#適用場景)

## 專案資訊

[讀取此節](original-entry/02.md#專案資訊)

## 參考文件

[讀取此節](original-entry/03.md#參考文件)

## 關鍵原始檔

[讀取此節](original-entry/04.md#關鍵原始檔)

## 1. 呼叫階層總覽

[讀取此節](original-entry/05.md#1-呼叫階層總覽)

## 2. DoCatchTray() 主流程

[讀取此節](original-entry/06.md#2-docatchtray-主流程)

### Case 清單

[讀取此節](original-entry/06.md#case-清單)

### 主流程段落

[讀取此節](original-entry/06.md#主流程段落)

## 3. 下一層函式 Case 摘要

[讀取此節](original-entry/07.md#3-下一層函式-case-摘要)

### 3.1 DoCatchFromLoader

[讀取此節](original-entry/07.md#31-docatchfromloader)

### 3.2 CatchNewTrayFromBuffer

[讀取此節](original-entry/07.md#32-catchnewtrayfrombuffer)

### 3.3 DoPlaceTrayToAuto

[讀取此節](original-entry/07.md#33-doplacetraytoauto)

### 3.4 DoPlaceToBuffer

[讀取此節](original-entry/07.md#34-doplacetobuffer)

### 3.5 DoSlapTray

[讀取此節](original-entry/07.md#35-doslaptray)

### 3.6 DoCatchUnderTray

[讀取此節](original-entry/07.md#36-docatchundertray)

### 3.7 DoSupportUnderTray

[讀取此節](original-entry/07.md#37-dosupportundertray)

### 3.8 DoPlaceBufferTray

[讀取此節](original-entry/07.md#38-doplacebuffertray)

## 4. 子函式 switch（下一層內）

[讀取此節](original-entry/08.md#4-子函式-switch下一層內)

### 4.1 C_CatchTray_Fix_Puch

[讀取此節](original-entry/08.md#41-c_catchtray_fix_puch)

### 4.2 C_CatchTray_Fix_Pop

[讀取此節](original-entry/08.md#42-c_catchtray_fix_pop)

### 4.3 DoLoadCarRotArmReadRFID

[讀取此節](original-entry/08.md#43-doloadcarrotarmreadrfid)

## 使用指引

[讀取此節](original-entry/09.md#使用指引)

## Local Preserved Notes (from previous local agent)

[讀取此節](original-entry/10.md#local-preserved-notes-from-previous-local-agent)

## 5. TrayX 座標排列（物理位置）

[讀取此節](original-entry/11.md#5-trayx-座標排列物理位置)

### 驗證依據

[讀取此節](original-entry/11.md#驗證依據)

### 全域變數對應

[讀取此節](original-entry/11.md#全域變數對應)

### 常見易錯提醒

[讀取此節](original-entry/11.md#常見易錯提醒)

## 6. CatchTray 補盤狀態判斷（State Record 診斷用）

[讀取此節](original-entry/12.md#6-catchtray-補盤狀態判斷state-record-診斷用)

### 6.1 關鍵指標

[讀取此節](original-entry/12.md#61-關鍵指標)

### 6.2 CatchTray 空閒循環（Idle Loop）特徵

[讀取此節](original-entry/12.md#62-catchtray-空閒循環idle-loop特徵)

### 6.3 OutArm case 3010 中的 CatchTray 忙碌判斷

[讀取此節](original-entry/12.md#63-outarm-case-3010-中的-catchtray-忙碌判斷)

#### 6.3.1 TrayArm↔OutArm 干涉區在哪，以及唯一真正的撞機失效模式

[讀取此節](original-entry/12.md#631-trayarmoutarm-干涉區在哪以及唯一真正的撞機失效模式)

#### 6.3.2 ⚠ 尚未修的對稱缺口：TrayArm 側只看命令位置

[讀取此節](original-entry/12.md#632--尚未修的對稱缺口trayarm-側只看命令位置)

### 6.4 如何從 State Record 快速判斷

[讀取此節](original-entry/12.md#64-如何從-state-record-快速判斷)

## 7. Auto Tray（出料盤）「收料 vs 退盤」：DoAutoReceiveBinTray ≠ DoReceiveAutoTray

[讀取此節](original-entry/13.md#7-auto-tray出料盤收料-vs-退盤doautoreceivebintray--doreceiveautotray)

### 7.1 功能對照

[讀取此節](original-entry/13.md#71-功能對照)

### 7.2 DoReceiveAutoTray 退盤狀態序列（csystem.cpp）

[讀取此節](original-entry/13.md#72-doreceiveautotray-退盤狀態序列csystemcpp)

### 7.3 JAM1101/JAM1201/JAM1301 兩種來源（同碼、不同條件）

[讀取此節](original-entry/13.md#73-jam1101jam1201jam1301-兩種來源同碼不同條件)

### 7.4 JSCC clean-out 死雞關聯（見記憶 [[jscc-cleanout-deadchicken-judgeempty]]）

[讀取此節](original-entry/13.md#74-jscc-clean-out-死雞關聯見記憶-jscc-cleanout-deadchicken-judgeempty)

### 7.5 呼叫來源 ＋「兩支共用 `iMMAuto_Car`」（為何 JAM 出自生產函式、根卻在退盤）

[讀取此節](original-entry/13.md#75-呼叫來源-兩支共用-immauto_car為何-jam-出自生產函式根卻在退盤)

## 8. DoLoad auto-clean-out 觸發條件 ＋「收工後補料」的坑（JSCC 死雞 code-confirmed 根因）

[讀取此節](original-entry/14.md#8-doload-auto-clean-out-觸發條件-收工後補料的坑jscc-死雞-code-confirmed-根因)

### 8.1 DoLoad auto-clean-out 只在「no any tray」才觸發（含 Loader Car）

[讀取此節](original-entry/14.md#81-doload-auto-clean-out-只在no-any-tray才觸發含-loader-car)

### 8.2 SupplyNewIC_From_LoaderCar = loader 常態補盤

[讀取此節](original-entry/14.md#82-supplynewic_from_loadercar--loader-常態補盤)

### 8.3 JSCC「clean-out 死雞」根因（翻案版，取代先前「判空 desync」推測）

[讀取此節](original-entry/14.md#83-jsccclean-out-死雞根因翻案版取代先前判空-desync推測)

### 8.4 防範方向

[讀取此節](original-entry/14.md#84-防範方向)

## 9. 分盤機構（緩衝站 Tray Separate：兩段舉升 + Selector 分離爪）

[讀取此節](original-entry/15.md#9-分盤機構緩衝站-tray-separate兩段舉升--selector-分離爪)

### 9.1 硬體兩件套

[讀取此節](original-entry/15.md#91-硬體兩件套)

### 9.2 舉升三高度（汽缸 or 馬達雙版本）

[讀取此節](original-entry/15.md#92-舉升三高度汽缸-or-馬達雙版本)

### 9.3 分盤序列（以 Loader 入料側為例，asendic_Loader.cpp）

[讀取此節](original-entry/15.md#93-分盤序列以-loader-入料側為例asendic_loadercpp)

### 9.4 Auto 側略有不同

[讀取此節](original-entry/15.md#94-auto-側略有不同)

### 9.5 常見陷阱

[讀取此節](original-entry/15.md#95-常見陷阱)

## 10. 「Loader 空盤放哪一軌 / Auto 空盤從哪一軌來」的三個判斷點（P04 陷阱）⚠

[讀取此節](original-entry/16.md#10-loader-空盤放哪一軌--auto-空盤從哪一軌來的三個判斷點p04-陷阱)

### 10.1 路由來源

[讀取此節](original-entry/16.md#101-路由來源)

### 10.2 三個判斷點（P04 只有兩處認得）

[讀取此節](original-entry/16.md#102-三個判斷點p04-只有兩處認得)

### 10.3 連帶：Auto 也取不到盤

[讀取此節](original-entry/16.md#103-連帶auto-也取不到盤)

### 10.4 自洽 vs 必死組合

[讀取此節](original-entry/16.md#104-自洽-vs-必死組合)

## 11. Color / Empty 軌「退盤序列」不可中途 Init ⚠（JSCC `CC_SCC` 專屬防護的回歸）

[讀取此節](original-entry/17.md#11-color--empty-軌退盤序列不可中途-init-jscc-cc_scc-專屬防護的回歸)

### 11.1 退盤（把 Color/Empty 待用新盤收回料疊）是**四段交棒**，帳只在最後一段清

[讀取此節](original-entry/17.md#111-退盤把-colorempty-待用新盤收回料疊是四段交棒帳只在最後一段清)

### 11.2 一被打斷就是永久壞掉（四方互鎖，零 alarm）

[讀取此節](original-entry/17.md#112-一被打斷就是永久壞掉四方互鎖零-alarm)

### 11.3 所有 `InitAutoXxxReceiveTask()` 呼叫點的保護（為何只有 JSCC 中招）

[讀取此節](original-entry/17.md#113-所有-initautoxxxreceivetask-呼叫點的保護為何只有-jscc-中招)

### 11.4 觸發情境：Clean Out 之後 Auto1 的第一次換盤（race，非必然）

[讀取此節](original-entry/17.md#114-觸發情境clean-out-之後-auto1-的第一次換盤race非必然)

### 11.5 現場 30 秒辨識

[讀取此節](original-entry/17.md#115-現場-30-秒辨識)

### 11.6 修正（V3.33.912.1_20260908_RogerYang_AI，2026-09-14）

[讀取此節](original-entry/17.md#116-修正v3339121_20260908_rogeryang_ai2026-09-14)

## 合併補充：repo 既有參考（20261001）

[讀取此節](original-entry/18.md#合併補充repo-既有參考20261001)
