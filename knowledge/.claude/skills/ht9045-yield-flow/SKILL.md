---
name: ht9045-yield-flow
description: HT9045 Yield Monitoring / Yield Alarm 流程知識庫。當使用者詢問 CalculateSiteYield、CheckBySiteYieldAlarm、CheckByPickerYieldAlarm、CheckBySiteByArmYieldAlarm、CheckLowYieldAlarm、CheckLowYieldAlarmByTotal、CheckIntervalLowYieldAlarmBySite、CheckIntervalLowYieldAlarmByTotal、CheckLowYieldAlarmSpecial、DoLowYieldAlarm、WAR0701/0702/0703/0705/0721/0722/0724/0725/0726/0727/07357/07358、AutoSiteOff、ClearYieldCount、Smart Auto Clean、SECS/GEM 清計數等問題時，應先載入此技能以理解 HT9045 良率警報完整流程。關鍵字：Yield Monitoring, Yield Alarm, Low Yield, DoLowYieldAlarm, WAR0701, WAR0702, WAR0703, WAR0705, WAR0721, WAR0722, WAR0724, WAR0725, WAR0726, WAR0727, WAR07357, WAR07358, AutoSiteOff, ClearYieldCount。
---

<!-- AI(W906-BA-SKILL) 20260915：這支 skill 來自網頁同事 20260915 交付的
     D:\HT9050\HT9045_skills_20260915\skills\ht9045-yield-flow，內容未改。
     ⚠ 資料通道以使用者 20260915 裁決為準：wb_serve HTTP 直讀 .Data +
       recipe.doc.put 寫。本 skill 若描述 JSON-only 或 C# Simulator 通道，
       那是同事那邊的舊架構，不是本專案的正式路徑。 -->

# HT9045 Yield Flow Knowledge

## 適用場景

當使用者詢問以下主題時，載入此技能：
- Yield 監控流程與 9 個核心函式的關係
- 各類低良率/差異警報如何觸發與分流
- `DoLowYieldAlarm()` 的統一報警行為（OneCycle/Retry/Smart Auto Clean）
- `WAR07xx` 警報碼來源、分群與觸發點
- AutoSiteOff、`ClearYieldCount()`、SECS/GEM 清計數對 Yield 判斷的影響
- By-bin/Category yield alarm（`WAR07357`、`WAR07358`）

## 專案資訊

- **專案**: HT9045 IC Test Handler
- **原始碼根路徑（可自定義）**: `${HT9045_SOURCE_ROOT}`
- **預設值（範例）**: `d:\HT9045\HT9011UC_Code_V3.33.897.0_20260306\`
- **語言**: C++ (Borland C++ Builder 6, VCL framework, AnsiString)
## 參考文件

- [YieldMonitoring_Functions_Explanation.md](references/YieldMonitoring_Functions_Explanation.md) — Yield 監控函式完整說明
- **警報碼來源**: `d:\HT9045\Error\AlarmCodeList.txt`

## 核心函式（uYieldMonitoring.cpp）

1. `CalculateSiteYield()`
2. `CheckBySiteYieldAlarm()`
3. `CheckByPickerYieldAlarm()`
4. `CheckBySiteByArmYieldAlarm()`
5. `CheckLowYieldAlarm()`
6. `CheckLowYieldAlarmByTotal()`
7. `CheckIntervalLowYieldAlarmBySite()`
8. `CheckIntervalLowYieldAlarmByTotal()`
9. `CheckLowYieldAlarmSpecial()`

## 警報碼對照（重點）

- `WAR0701`: Low Yield Alarm
- `WAR0702`: Site Yield Different over setting
- `WAR0703`: Arm Site Yield Different over setting
- `WAR0705`: Low Yield Alarm (By Total)
- `WAR0721`: Low Yield Alarm (By Site, Interval)
- `WAR0722`: Previous yield difference Warning (Interval)
- `WAR0724`: Low Yield Special1
- `WAR0725`: Low Yield Special2
- `WAR0726`: Arm 1 site yield difference over setting
- `WAR0727`: Arm 2 site yield difference over setting
- `WAR07357`: Category yield over limit
- `WAR07358`: Category count over failure

## 主要跨檔案入口

- `uYieldMonitoring.cpp`: 9 個核心判斷函式
- `atester_ProcessCount.cpp`: `DoLowYieldAlarm()` 統一報警入口
- `cShowBinSelect.cpp`: Category/By-bin yield 觸發 (`WAR07357/07358`)
- `cSortCT.cpp`: Unloader 端的 yield 相關警報顯示
- `csystem.cpp`: special low-yield 分支觸發 (`WAR0724`)
- `SECSGEM/uHGemHT9045.cpp`: `CLEAN_AUTO_SORT_COUNT` 觸發 `ClearYieldCount()`
- `uLotInfo.cpp`: FT_Yield 參數一致性檢查與寫回
- `cprod.h`: low-yield / by-total / interval / auto-site-off / special 旗標與門檻資料模型

## 警報碼導向流程（摘要）

```text
CalculateSiteYield
  -> 差異比較群 (WAR0702/0703/0726/0727)
  -> 低良率群 (WAR0701/0705)
  -> 區間群 (WAR0721/0722)
  -> 特規群 (WAR0724/0725)
  -> 分類群 (WAR07357/07358)
  -> DoLowYieldAlarm
      -> Smart Auto Clean? -> 清潔
      -> OneCycle/Retry 分流
      -> ClearYieldCount / AutoSiteOff / 記錄
```

## 回答指引

回答 Yield 問題時建議順序：
1. 先判斷使用者問的是哪一群警報碼（差異/低良率/區間/特規/分類）。
2. 回到對應核心函式說明 trigger 條件與計算基礎（count/limit/window）。
3. 再補 `DoLowYieldAlarm()` 的動作策略（OneCycle、Retry、Smart Auto Clean）。
4. 若涉及「為何突然不報警或延後報警」，務必檢查 `ClearYieldCount()` 與 Host/SECSGEM 清計數路徑。
5. 若涉及現場權限或處置規範，補充 `cSecurity.cpp` 的 Jam Level 分類影響。
