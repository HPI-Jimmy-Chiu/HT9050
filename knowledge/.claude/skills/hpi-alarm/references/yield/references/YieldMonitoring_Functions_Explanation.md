> 保存來源：`.claude/skills/ht9045-yield-flow/references/YieldMonitoring_Functions_Explanation.md`，main `7deec0f76`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Yield Monitoring Functions 說明

> Source: `uYieldMonitoring.cpp`
> Verified lines:
> - `CalculateSiteYield()` at line 3503
> - `CheckBySiteYieldAlarm()` at line 3647
> - `CheckByPickerYieldAlarm()` at line 3871
> - `CheckBySiteByArmYieldAlarm()` at line 4106
> - `CheckLowYieldAlarm()` at line 4360
> - `CheckLowYieldAlarmByTotal()` at line 4786
> - `CheckIntervalLowYieldAlarmBySite()` at line 5249
> - `CheckIntervalLowYieldAlarmByTotal()` at line 5386
> - `CheckLowYieldAlarmSpecial()` at line 5462

---

## 1. CalculateSiteYield()

- **用途**: 計算每個 site（含前後 arm）的良率資料，提供後續所有 low-yield 檢查使用。
- **重點邏輯**:
  - 依測試模式（例如 NN_1Row/NN_2Row、Shuttle 模式）取對應 site 的 pass/total。
  - 產生 site 層級與 picker 層級的 yield 緩存資料（供 BySite/ByPicker 類檢查使用）。
- **是否直接報警**: 否。
- **副作用**: 更新 yield 相關陣列/計數快取，作為後續 alarm 判斷基礎。

## 2. CheckBySiteYieldAlarm()

- **用途**: 檢查同 site 在不同 arm 間的 yield 差異是否過大。
- **重點邏輯**:
  - 以 `CalculateSiteYield()` 產生的資料比對同座標 site 的 arm 差異。
  - 差異超過設定閾值且達到檢查計數條件後，觸發 low-yield 行為。
- **典型警報**: `WAR0703` - `Arm Site Yield Different over setting!`
- **副作用**:
  - 標記異常 site。
  - 若啟用 AutoSiteOff，可能關閉低良率 site。
  - 觸發後會清除/重置部分 yield 累計。

## 3. CheckByPickerYieldAlarm()

- **用途**: 檢查同一組測試中，各 picker（吸嘴）間 yield 差異。
- **重點邏輯**:
  - 找出 picker 最大 yield。
  - 其他 picker 若低於 `max - 閾值` 則判定異常。
- **典型警報**: 
  - `WAR0726` - `Arm 1 site yield difference over setting!`
  - `WAR0727` - `Arm 2 site yield difference over setting!`
- **副作用**:
  - 異常 picker/site 標示。
  - 可搭配 AutoSiteOff 做自動關站。

## 4. CheckBySiteByArmYieldAlarm()

- **用途**: 以「全體 site 中最高 yield」為基準，檢查其他 site 是否明顯偏低。
- **重點邏輯**:
  - 第一輪找 `max site yield`。
  - 第二輪比較 `max - each_site` 是否超過閾值。
- **典型警報**: `WAR0702` - `Site Yield Different over setting!`
- **副作用**:
  - 可標記或關閉低良率 site。
  - 觸發後重置對應計數。

## 5. CheckLowYieldAlarm()

- **用途**: 基礎 low-yield 檢查（整體或 socket 依設定），判斷是否低於最低 yield 門檻。
- **重點邏輯**:
  - 依 pass bin 與總測試數計算 yield。
  - 達到最低檢查 count 後，若 yield 低於限制則報警。
- **典型警報**: `WAR0701` - `Low Yield Alarm!`
- **副作用**:
  - 觸發後清除 yield 累計（`ClearYieldCount` 類行為）。
  - 依客戶邏輯可能保留或清除部分歷史。

## 6. CheckLowYieldAlarmByTotal()

- **用途**: 以 total bin 全域角度檢查 low-yield（不聚焦單一 site）。
- **重點邏輯**:
  - 以全 bin/pass 統計計算總體 yield。
  - 達到總量門檻且低於閾值即觸發。
- **典型警報**: `WAR0705` - `Low Yield Alarm!(By Total)`
- **副作用**:
  - 更新 total yield UI 欄位與統計計數。
  - 觸發後進入 low-yield alarm 流程。

## 7. CheckIntervalLowYieldAlarmBySite()

- **用途**: 用「區間窗口（最近 N 顆）」檢查每個 site 的 low-yield。
- **重點邏輯**:
  - 由 `bIntervalYieldIsPass` 這類窗口資料計算每 site 區間 yield。
  - 與 `dIntervalLowYieldLimitBySite` 比較。
- **典型警報**: `WAR0721` - `Low Yield Alarm!(By Site)`
- **副作用**:
  - 更新 interval site yield 顯示。
  - 觸發時顯示/記錄該區間異常。

## 8. CheckIntervalLowYieldAlarmByTotal()

- **用途**: 用「區間窗口（最近 N 顆）」做 total 層級 low-yield 檢查。
- **重點邏輯**:
  - 由 `bYieldTotalBinIsPass` 類窗口資料計算區間 total yield。
  - 與 `dIntervalLowYieldLimitByTotal` 比較。
- **典型警報**: `WAR0722` - `Previous yield difference Warning!`
- **副作用**:
  - 更新 interval total 顯示。
  - 觸發後清空區間緩衝以進入下一輪觀察。

## 9. CheckLowYieldAlarmSpecial()

- **用途**: 客戶特規（PTI）兩段式 low-yield 檢查。
- **重點邏輯**:
  - 第一階段達到門檻且低於限制時，先觸發清機/清料流程。
  - 第二階段若仍低於限制，再觸發正式低良率警報。
- **典型警報**: `WAR0725` - `Low Yield Special2`（第二階段）
- **副作用**:
  - 可能觸發 `CleanOut`。
  - 清理 PTI 專用統計資料並重置旗標。

---

## 10. 專案中其他 Yield Alarm 相關區塊

以下為 9 個核心函式之外，實際會影響 yield alarm 行為的程式區塊：

1. `DoLowYieldAlarm()` 統一報警入口
   - 檔案: `atester_ProcessCount.cpp:256`
   - 說明:
     - 幾乎所有良率相關 alarm 最後都會進入此函式。
     - 在這裡決定要 `K_RETRY`、`K_ONECYCLE`，或先進行 Smart Auto Clean。
     - 會依客戶別/功能旗標改變 alarm 行為（例如是否強制 one-cycle）。

2. By-bin/分類異常也會走 yield alarm 流程
   - 檔案: `cShowBinSelect.cpp:1856`
   - 檔案: `cShowBinSelect.cpp:1939`
   - 說明:
     - 在分類良率或失敗數超限時，會呼叫 `DoLowYieldAlarm()`。
     - 使用警報碼:
       - `WAR07357` = `Category yield over limit`
       - `WAR07358` = `Category count over failure`

3. SortCT 的 Unloader 顯示警告路徑
   - 檔案: `cSortCT.cpp:1688`
   - 檔案: `cSortCT.cpp:1705`
   - 檔案: `cSortCT.cpp:1731`
   - 檔案: `cSortCT.cpp:1739`
   - 說明:
     - 在 Tray/Unloader 場景下，會把低良率與相關差異警告轉成 Unloader 訊息顯示。
     - 包含 `WAR0701`、`WAR0702`、`WAR0721`、`WAR0722` 等。

4. Special low-yield 第二段警報觸發點（系統流程）
   - 檔案: `csystem.cpp:14581`
   - 說明:
     - 在特定流程分支（Special low-yield）中，會直接觸發 `DoLowYieldAlarm("WAR0724", "")`。
     - `WAR0724` 對應訊息為 `Low Yield Special1`。

5. SECS/GEM 遠端清計數會重置 yield alarm 計數
   - 檔案: `uHGemHT9045.cpp:1771`
   - 檔案: `uHGemHT9045.cpp:1807`
   - 說明:
     - 收到 `CLEAN_AUTO_SORT_COUNT` 後，在特定條件會呼叫 `fYieldMonitoring->ClearYieldCount()`。
     - 代表 Host 端操作會直接影響後續 low-yield 判斷窗口。

6. Lot CheckList 對 yield 參數做一致性檢查與寫回
   - 檔案: `uLotInfo.cpp:11551`
   - 檔案: `uLotInfo.cpp:11578`
   - 檔案: `uLotInfo.cpp:12138`
   - 說明:
     - 讀寫 `FT_Yield` 區段（`Yield Func`、`Preset`、`Low Yield`、`Variance`）。
     - 若 lot/checklist 不一致，會觸發設定差異警示流程。

7. Yield alarm code 在安全等級/Jam 設定中的歸類
   - 檔案: `cSecurity.cpp:1110`
   - 說明:
     - `WAR0701`、`WAR0702`、`WAR0703`、`WAR0705` 被納入 Jam/權限等級設定邏輯。
     - 這會影響現場操作層面（誰可處理、需不需要特定權限）而非 yield 計算本體。

8. Yield 參數資料模型（全域旗標/門檻）
   - 檔案: `cprod.h:516`
   - 檔案: `cprod.h:795`
   - 檔案: `cprod.h:1629`
   - 說明:
     - 定義 low-yield、by-total、interval、auto-site-off、by-picker、special 等所有核心旗標與門檻值。
     - `uYieldMonitoring.cpp` 與其他流程檔實際都是讀寫這些欄位來完成判斷。

---

## 函式關係（建議理解順序）

1. `CalculateSiteYield()`
2. 差異比較型：
   - `CheckBySiteYieldAlarm()`
   - `CheckByPickerYieldAlarm()`
   - `CheckBySiteByArmYieldAlarm()`
3. 基礎低良率：
   - `CheckLowYieldAlarm()`
   - `CheckLowYieldAlarmByTotal()`
4. 區間低良率：
   - `CheckIntervalLowYieldAlarmBySite()`
   - `CheckIntervalLowYieldAlarmByTotal()`
5. 客戶特規：
   - `CheckLowYieldAlarmSpecial()`

> 實務上可視為：先算 yield，再從「site/picker差異」、「total」、「interval」、「special」多層交叉監控。

---
<!-- preserved-content:end -->
