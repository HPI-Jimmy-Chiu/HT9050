---
name: ht9045-heater-control
description: >
  HT9045 溫控器廠牌與溫控迴圈知識庫：整台一個廠牌 HEATER_CTRL_TYPE（Heater Type）與 golden V912 逐通道廠牌
  （71 個 [TempCtrl] HeaterInsOpt_<通道>，0 TC401、1 Panasonic KT4H、2 Omron E5DC、3 No Heater、4 DTK4848；
  23 個有下拉、48 個沒有；-9999＝跟著 HEATER_CTRL_TYPE）怎麼讀、怎麼存、存完記憶體留 -9999 不重讀；
  站號寫死（KT4H／DTK4848／E5DC＝序號＋1，TC401＝(序號÷4)+1 台、通道 序號%4）、全部共用一個溫控 COM 埠 9600 8N1；
  廠牌分派沒有 else（已安裝通道是 No Heater／-9999 時溫控迴圈卡在那一通道，V912 既有缺陷）；EJ1N／DTME08 的 Index 區
  不走溫控 COM 埠；V906 移植樹現況（頁面照 912、底層只看 TC401HeaterControl、wb_serve 沒有跑溫控迴圈）與 Steven 的裁決
  （RULINGS_20260926 第 26 條、S136＝Q14 B、S137＝Q15、S154／S166＝Q34 方案 D：St01 的讀寫檔＋頁面＋ctest 已做，
  新鍵 HeaterInsMode／HeaterInsIndexOpt／HeaterInsOtherOpt／HeaterInsAddr_、開頁不寫檔、缺鍵＝3，底層歸 Jimmy）。
  20261002 E-029／Q71～Q76（St01）：廠牌 7 個（＋5 Omron EJ1N、6 Delta DTM，同一個 HeaterInsOpt_ 鍵；BCB V912 認不得＝Steven 接受的風險）、
  71 個通道全列可選、Index 區 EJ1N／DTM 與 [System] USE_16_HEATER 雙向連動（一次存檔兩個鍵都寫）、全機相同時 EJ1N／DTM 只在 Index 下拉
  （其他位置 golden 5 個）、Head1～4⇄Ax／Bx（看 rgHeater）與 Socket⇄DUT1～4（看 rgUse4DUT）互斥即時顯示、EJ1N 台號＋CH／DTM 站＋CH
  （新鍵 HeaterInsCh_）、D-8a 分三條匯流排、HEATER_CTRL_TYPE 永遠不寫 5／6 → §9。
  Use when：Heater 分頁、HandlerSys 溫控器廠牌、逐通道廠牌、混廠牌、Index 溫控器、站號、溫度顯示 999、溫控不動作、
  溫控迴圈卡住、Gerneral.ini [TempCtrl] 被寫 -9999、HeaterInsOpt 缺鍵、Q34、Q15、Q14、方案 D、要改 bthermo 的廠牌判斷。
  關鍵字：heater, 溫控, 溫控器, 廠牌, HEATER_CTRL_TYPE, TC401HeaterControl, HeaterInsOpt, HeaterInsOpt_Read,
  g_tHeaterInsInfo, THeaterInsInfo, EN_HEATER_SHEET, IsValEqual_HeaterInsOpt, IsNoHeaterMachine, IsAllSame_HeaterInsOpt,
  GetCtrlItemVisProp, rgHeaterType, rgHeaterTypeClick, rgHeater, USE_16_HEATER, eht16HeaterEJ1N, eht32HeaterKT4H, DTME08,
  TC401, KT4H, E5DC, DTK4848, NoHeater, INVALID_INT_VAL_NEG, -9999, DoThermo, DoThermoReal, bthermo, bUT150Install,
  UN150Read, g_iHeaterTypeIdx_SendCmd, Comm2ReceiveData, UT100WordWriteNoSucm, TMC401WriteTemp, E5DCWriteTemp,
  DTK4848WordWriteNoSucm, g_pDTKComm, COM_PORT, COM_PORT_OMRON, HeaterThread, StageThermo, HSys_Heater.h,
  ht9045_hsys_heater_c.js, grpHeater, HeaterInsMode, HeaterInsAddr, Q34_HEATER_MIX_PLAN, HeaterInsIndexOpt, HeaterInsOtherOpt,
  W906_HeaterStationIdx, W906_HeaterMixReadFile, W906_HeaterMixSave, HSys_HeaterMix, test_hsys_heater_mix, D-8, 站號重複,
  E-029, E029, Q71, Q72, Q73, Q74, Q75, Q76, EJ1N=5, DTM=6, Delta DTM, Omron EJ1N, HeaterInsCh_, W906_HeaterInsUnitCh, W906_HeaterInsBus,
  W906_HeaterInsU16For, W906_HeaterInsU16Area, HmLinkIndex, HmActive, rgUse4DUT, SocketBasedAdd4Temp, 互斥, 兩個變數, E029_HeaterPages, 台號, 內部站號。
  71 通道全表、通訊框格式、讀存檔逐行 → references/golden-v912-channels.md；移植樹逐檔現況、底層待改清單、
  方案 D 與 St02 平行方案對照、方案 D 實作（§5）→ references/port-status-and-plans.md；
  溫度總入口（V906 溫度現況一張表、溫度迴圈內部、Temp_Set、溫度檔案）→ D:\HT9045\.claude\skills\ht9045-temperature\SKILL.md；
  各廠牌溫控器手冊（通訊參數、暫存器、面板設定、手冊對程式的疑點）→ D:\HT9045\.claude\skills\ht9045-temperature\references\controllers\index.md
---

# ht9045-heater-control 相容入口

同主題已整合到 [hpi-temperature](../hpi-temperature/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-temperature/references/controllers/configuration/original-entry.md)

## 1. 三個設定，別混

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#1-三個設定別混)

## 2. golden V912 的逐通道廠牌（事實）

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#2-golden-v912-的逐通道廠牌事實)

## 3. golden V912 的溫控迴圈（事實）

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#3-golden-v912-的溫控迴圈事實)

### 3.1 逐通道分派，沒有 else

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#31-逐通道分派沒有-else)

### 3.2 換通道與接收解析

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#32-換通道與接收解析)

### 3.3 站號寫死、共用一條匯流排

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#33-站號寫死共用一條匯流排)

### 3.4 EJ1N／DTME08 的 Index 區不走溫控 COM 埠

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#34-ej1ndtme08-的-index-區不走溫控-com-埠)

## 4. V906 移植樹現況（20260927，HEAD `89ccb4cc`）

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#4-v906-移植樹現況20260927head-89ccb4cc)

## 5. Steven 的裁決

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#5-steven-的裁決)

## 6. 方案 D（只是方案，沒有實作）

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#6-方案-d只是方案沒有實作)

## 7. 實例：Steven01 這台（只讀，不改）

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#7-實例steven01-這台只讀不改)

## 8. 規則

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#8-規則)

## 9. E-029：EJ1N／DTM、71 個通道、Index ⇄ USE_16_HEATER（20261002，St01）

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#9-e-029ej1ndtm71-個通道index--use_16_heater20261002st01)

### 9.1 廠牌與檔案

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#91-廠牌與檔案)

### 9.2 Index 區 ⇄ USE_16_HEATER（Q72，兩個變數一起存）

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#92-index-區--use_16_heaterq72兩個變數一起存)

### 9.3 下拉與互斥（Q73～Q76，1002 17:1x）

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#93-下拉與互斥q73q761002-171x)

### 9.4 D-8a 分匯流排

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#94-d-8a-分匯流排)

### 9.5 連線設定（唯讀顯示）

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#95-連線設定唯讀顯示)

### 9.6 給 Jimmy（底層）

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#96-給-jimmy底層)

### 9.7 驗證

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#97-驗證)

## 相關 skill

[讀取此節](../hpi-temperature/references/controllers/configuration/original-entry.md#相關-skill)
