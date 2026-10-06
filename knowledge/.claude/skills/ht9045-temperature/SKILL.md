---
name: ht9045-temperature
description: >
  HT9045 溫度這一塊的總入口（20261001，St01 ST01-E2 依 Steven「程式碼中有許多溫度相關的項目，有做成skill了嗎?」整理）：
  溫度分幾層、每層看哪個 skill；V906 移植樹的溫度現況一張表（加熱執行緒沒啟動、bthermo 20 個 #if 0、
  模擬版 HeaterSimTick 每拍跑 CheckHeater、出貨版 fHeaterOK 沒有人設、cTemperFrom 只翻一部分、EJ1N 沒移植、溫度 tag 多半 null）；
  溫度檔案與鍵（Temperature.Data、DefineTemp、config.ini SingleTempLimit、system\ATC.ini 與 Config\ATC.ini 的分工、
  DTME08_Control.ini）；golden V912 已知問題；移植樹缺的 V912 溫度改動；程式項目與 todo 編號（D-029～D-031、D-035、D-036、G-034、G-035、C-003）。
  Use when：溫度相關程式在哪、溫度為什麼不動、溫度顯示 --- 或 999、Temp_Set 頁、LotInfo 的溫度、加熱執行緒、DoThermo、
  CheckHeater、fHeaterOK、CheckHeaterOK、WAR15 溫度告警、ATC.ini 在哪個資料夾、Temperature.Data 欄位、EJ1N、DTME08、
  HeaterSimTick、溫度 tag、要找某個溫度事實記在哪個 skill。
  關鍵字：溫度, 溫控, temperature, heater, thermo, bthermo, uHeaterThread, HeaterThreadProcess, THeaterThread, cTemperFrom,
  ShowThermo, EJ1N, OmronEJ1N, DTME08, uDTME08Control, fDTME08, uTemp_Set, Temp_Set, uLotInfo, LotInfo, NetATCTimeTimer,
  SetATCOffset, ReadTempFile, SaveSetupFile, Temperature.Data, DefineTemp, SingleTempLimit, ATC.ini, iCheckSameTempTime,
  iATC_MODE_TYPE, AutoTempOfsByFTP, SetTempOfs, GetFactSetTemp, ConvertTempOffset, CheckHeater, CheckHeaterOK, fHeaterOK, fHeaterStableOK, iThermoTask,
  HeaterSimTick, W906_HeaterSimTick, UN150Read, bUT150Install, WAR15, MES2130, MES2131, temp.sv, temp.pv, StageThermo, E-029, EJ1N=5, DTM=6,
  Status.TemperFrom.html, Setup.Temp_Set.html, Tj Avg Times。
  移植樹溫度程式逐檔 → references/port-thermo-loop.md；golden Temp_Set／LotInfo 溫度項目 → references/temp-set-and-lotinfo.md；
  散在其他 skill 的溫度事實索引＋兩個 ATC skill 的重疊 → references/temperature-facts-index.md；
  溫控器手冊（20261001，一個控制器系列一份：KT4H、E5DC、EJ1N、DTK4848、DTB4824、DTM／DTME08、RKC SRZ；通訊參數、暫存器、面板設定、手冊對程式的疑點）
  → references/controllers/index.md。主畫面溫度怎麼顯示（各機型看得到哪些格子、畫面名稱、71 通道 → 各廠牌實體位址、ATC、HT9050、網頁現況，20261002）
  → references/main-screen-display.md。顯示關鍵字：TfTemperFrom, ShowThermo, ShowHotName, asTempCtrl, Index16Heater, bUT150Install, iAddrToATC, DOUN150ReadTemp, ShowATCThermo, 溫度畫面, 主畫面溫度。手冊關鍵字：CompoWay/F, Modbus ASCII, Modbus TCP, BCC, LRC, 站號, 面板設定, 出廠值, cmwt, Port A, DTM, DTMN08, DTB4824, RKC, SRZ, 溫控器手冊。
---

# ht9045-temperature 相容入口

同主題已整合到 [hpi-temperature](../hpi-temperature/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-temperature/references/core/original-entry.md)

## 1. 溫度分幾層、看哪裡

[讀取此節](../hpi-temperature/references/core/original-entry.md#1-溫度分幾層看哪裡)

## 2. V906 移植樹的溫度現況（20261001，HEAD `c13d34b4`）

[讀取此節](../hpi-temperature/references/core/original-entry.md#2-v906-移植樹的溫度現況20261001head-c13d34b4)

## 3. 溫度檔案與鍵

[讀取此節](../hpi-temperature/references/core/original-entry.md#3-溫度檔案與鍵)

## 4. golden V912 已知問題（照 BCB 保留，只通報 Jimmy）

[讀取此節](../hpi-temperature/references/core/original-entry.md#4-golden-v912-已知問題照-bcb-保留只通報-jimmy)

## 5. 移植樹缺的 V912 溫度改動（四份程式照 V906 翻）

[讀取此節](../hpi-temperature/references/core/original-entry.md#5-移植樹缺的-v912-溫度改動四份程式照-v906-翻)

## 6. 程式項目（ST01-M 已登記；這份 skill 不改程式）

[讀取此節](../hpi-temperature/references/core/original-entry.md#6-程式項目st01-m-已登記這份-skill-不改程式)

## 7. 規則

[讀取此節](../hpi-temperature/references/core/original-entry.md#7-規則)

## 相關 skill

[讀取此節](../hpi-temperature/references/core/original-entry.md#相關-skill)
