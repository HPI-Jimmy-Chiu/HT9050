> 保存來源：`.claude/skills/ht9045-outarm-flow/references/colleague-skill-body-20260915.md`，main `372b91908`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# repo 版 SKILL.md 正文（合併前原樣保留）

> 📥 20261001 與 RogerYang 版合併時，主 SKILL.md 以 RogerYang 版為底；repo 版中「同一內容不同寫法」的段落以 RogerYang 寫法為準，
> 為避免遺失任何字句，repo 版 SKILL.md 正文整份原樣保存在此。


# HT9045 OutArm Flow Knowledge

## 適用場景

當使用者詢問以下主題時，載入此技能：
- OutArm / Output Arm 的動作流程、狀態機、case 數值意義
- Shuttle 取料（Pick from Shuttle 1 / Shuttle 2）
- Unloader Tray 放料（Place to Auto Tray / Fix Tray）
- Bin 分類與 Tray 搜尋（SearchTrayToPlace, CheckBin）
- Magazine 相關流程（Magazine Tray / Buffer）
- 附加功能（Rotator / AOI / Fix AI CCD）
- 吸取異常處理（Retry / Skip / Home）
- IC 掉料（Fall Down）、Destroy 吹氣確認
- Clean Out 整盤（Auto Sorting BinTray）
- OutArm sucker 幾何配置（iInArmType）

## 專案資訊

- **專案**: HT9045 IC Test Handler
- **原始碼根路徑（可自定義）**: `${HT9045_SOURCE_ROOT}`
- **預設值（範例）**: `d:\HT9045\HT9011UC_Code_V3.33.897.0_20260306\`
- **語言**: C++ (Borland C++ Builder 6, VCL framework, AnsiString)
- **架構**: State Machine pattern — `switch(Task)` 搭配 `int &Task` 參考變數
- **呼叫方式**: 函式回傳 `bool`，`true` = 動作完成，`false` = 尚未完成（主迴圈反覆呼叫）
- **FlowChart 文件**: `${HT9045_SOURCE_ROOT}\..\FlowChart\`（或依你的目錄結構調整）

## 關鍵原始檔

| 檔案 | 主要函式 | Task 變數 |
|---|---|---|
| `aoutarm.cpp` | `DoOutArm()` — 最上層入口，資料前處理後呼叫 `DoOutArm_9045()` | （無 switch） |
| `aoutarm.cpp` | `CheckOutArmCleanOut()` — Clean Out 判斷（整盤 / Magazine / ATK AMR） | （無 switch） |
| `aoutarm.cpp` | `DoOutArmPlaceToAuto()` — 放料到 Auto/Fix Tray 的 Destroy 吹氣主流程 | （迴圈遍歷，無 switch） |
| `aoutarm.cpp` | `CheckBin()` — 依據 BIN 分配 IC 到對應 Tray | （無 switch） |
| `aoutarm.cpp` | `CheckOutSuckICFallDown()` — 掉料檢查 | （無 switch） |
| `aoutarm9045.cpp` | `DoOutArm_9045()` — Dispatch，依 `iInArmType` 分派到對應 sub-function | （Dispatch，無 switch） |
| `aoutarm9045.cpp` | `DoPickFromShuttle_9045()` — Shuttle 取料 Dispatch | （Dispatch，無 switch） |
| `aoutarm9045.cpp` | `DoOutArmPlaceToAuto_9045()` — 放料到 Auto Tray 主控制流程 | `iPlaceToAutoTask` |
| `aoutarm9045.cpp` | `DoOutArmAfterPlaceToAuto()` — 放料後的後處理（整盤 / Magazine / CleanOut） | `iDoOutArmAfterPlaceToAutoTask` |
| `aoutarm9045.cpp` | `DoOutArmAdditionalFunction()` — 附加功能（Rotator / AOI / Fix AI CCD） | `iOutArmAdditionalFunctionTask` |
| `aoutarm9045.cpp` | `SearchTrayToPlace_9045()` — 搜尋可放 IC 的 Tray | （無 switch） |
| `aoutarm9045_2x8_8.cpp` | `DoOutArm_9045_2x8_8()` — 2x8 模式主狀態機 | `OutArmTask` |
| `aoutarm9045_2x8_8.cpp` | `DoPickFromShuttle_9045_2x8_8()` — 2x8 模式 Shuttle 取料 | `iPickFromShuttle1Task` / `iPickFromShuttle2Task` |

> 其他 `aoutarm9045_*.cpp` 檔案（如 `_1x4_4.cpp`, `_2x2_4.cpp` 等）的狀態機結構與 `_2x8_8` 相同，僅 sucker 幾何處理不同。

---

## 1. 呼叫階層總覽

```
DoOutArm()                                 <- aoutarm.cpp (最上層入口)
  └─ DoOutArm_9045()                       <- aoutarm9045.cpp (Dispatch)
       └─ DoOutArm_9045_2x8_8()            <- aoutarm9045_2x8_8.cpp (主狀態機, 以 2x8 為例)
            ├─ case 1200/2200: DoPickFromShuttle_9045_2x8_8(iSht)
            │                    ├─ case 200: MoveOutArmToShuttleIncludeZ (XY/Z 定位)
            │                    ├─ case iOUTARM_SUCK: 逐一 Suck() 吸取 IC
            │                    └─ case 2000: Error Retry/Skip/Home
            ├─ case 7000: DoOutArmAdditionalFunction()
            │               ├─ DoOutArmRotateKIT()
            │               ├─ DoAOIFunction()
            │               └─ DoFix2AICCDFunction()
            ├─ case 3010: SearchTrayToPlace_9045() (搜尋放料 Tray)
            ├─ case 3310: DoOutArmPlaceToAuto_9045()
            │               └─ DoOutArmPlaceToAuto(iWhichAuto) (Destroy 吹氣放料)
            └─ case 3500: DoOutArmAfterPlaceToAuto()
                            ├─ case 1000: DoFix3FullTray() (Fix3 整盤)
                            ├─ case 5000: DoSortingBinTray() (Clean Out 整盤)
                            └─ case 11100: DoPickFromMagazineBuffer()
```

---

## 2. DoOutArm() — 入口

> Source: `aoutarm.cpp` line 1117-1135

入口函式非常簡潔，主要做資料前處理：

1. `FRCarryKit.SetHasNullIcToNullIc()` / `BRCarryKit.SetHasNullIcToNullIc()` — 清理 Shuttle 上的 NULL IC 標記
2. 將 Shuttle 上的 `HAS_IC` / `HAS_HOT_IC` 統一轉換為 `TEST_PASS + iTestBinCount`（模擬未分類的測試結果）
3. `SetFixTrayMiddleDtata()` — 設定 Fix Tray 中間資料
4. 呼叫 `DoOutArm_9045()` — 進入 Dispatch

> 與 InArm 不同：DoOutArm() 沒有複雜的 Guard Checks，Guard 邏輯全在 DoOutArm_9045() 中。

---

## 3. DoOutArm_9045() — Dispatch

> Source: `aoutarm9045.cpp` line ~403-620

### Pre-Dispatch Guard Checks

1. `TestingNeedStopAllMotor` → return
2. `bAlarmNeedServoOff && bMyServoOffOutArm` → return
3. `CheckOutArmDestroyActive()` → 確認 Destroy 吹氣完成，若未完成 → return
4. `QA Mode` — `Check_QA_ModeUnloadCount()`
5. `iPauseBackUp != -1 && Suck/Destroy 完成` → return
6. `bResetOutArmTask` → `InitOutArmTask()` → return
7. **Auto Alignment CCD** — 複雜的 Tray 補料確認與自動對位流程
   - `bAutoNeedTrayMustFinish` — 需要等 Tray 補好
   - `bRunOutArmAutoAlignment` — 正在對位中 → return

### iInArmType Dispatch Table

| iInArmType | Sub-function | Picker Config |
|---|---|---|
| `ep1Picker` | `DoOutArm_9045_All_1Picker()` | Single picker |
| `e9045_1x1_1` / `e9045_1x4_1_Ac` | `DoOutArm_9045_1x1_1()` | 1x1, 1 sucker |
| `e9045_1x2_2_14` / `e9045_1x2_2_13` | `DoOutArm_9045_1x2_2()` | 1x2, 2 suckers |
| `e9045_1x2_4_Hot` | `DoOutArm_9045_1x2_4()` | 1x2, 4 suckers |
| `e9045_1x3_2_14` | `DoOutArm_9045_1x3_2_14()` | 1x3, 2 suckers |
| `e9045_1x3_4` | `DoOutArm_9045_1x3_4()` | 1x3, 4 suckers |
| `e9045_1x4_2_14` | `DoOutArm_9045_1x4_2()` | 1x4, 2 suckers |
| `e9045_1x4_4_13` | `DoOutArm_9045_1x4_4S()` | 1x4, 4 suckers (S) |
| `e9045_1x4_4` | `DoOutArm_9045_1x4_4()` | 1x4, 4 suckers |
| `e9045_1x4_8_Hot` | `DoOutArm_9045_1x4_8()` | 1x4, 8 suckers |
| `e9045_2x1_2_13` | `DoOutArm_9045_2x1_2()` | 2x1, 2 suckers |
| `e9045_2x2_4_12/13/14` | `DoOutArm_9045_2x2_4()` | 2x2, 4 suckers |
| `e9045_2x2_8_Hot` | `DoOutArm_9045_2x2_8()` | 2x2, 8 suckers |
| `e9045_2x3_6_14` | `DoOutArm_9045_2x3_6_14()` | 2x3, 6 suckers |
| `e9045_2x3_6` | `DoOutArm_9045_2x3_6()` | 2x3, 6 suckers |
| `e9045_2x4_4_13/14` | `DoOutArm_9045_2x4_4()` | 2x4, 4 suckers |
| `e9045_2x4_8` | `DoOutArm_9045_2x4_8()` | 2x4, 8 suckers |
| `e9045_2x5_8` | `DoOutArm_9045_2x5_8()` | 2x5, 8 suckers |
| `e9045_2x6_8` | `DoOutArm_9045_2x6_8()` | 2x6, 8 suckers |
| `e9045_2x8_8` / `e9045_2x8_32` | `DoOutArm_9045_2x8_8()` | 2x8, 8/32 suckers |

> Dispatch 結構與 InArm 完全對稱，使用相同的 `iInArmType` 作為分派依據。

---

## 深層狀態機與放料細節

主狀態機 case 完整對照、Shuttle 取料、附加功能、放料流程、DoOutArmAfterPlaceToAuto 回傳值、Clean Out 判斷邏輯均在：

→ [references/state-machine.md](state-machine.md)

---

## 4. OutArm vs InArm 差異對照

| 項目 | InArm | OutArm |
|---|---|---|
| **功能** | Loader Tray → (HotPlate) → Shuttle | Shuttle → Unloader Tray |
| **取料來源** | Loader Tray / HotPlate | Shuttle 1 / Shuttle 2 |
| **放料目標** | HotPlate / Shuttle | Auto Tray / Fix Tray / Magazine |
| **附加功能** | Die Clean / Precisor / 2DID / Rotator | Rotator / AOI / Fix AI CCD |
| **HotPlate 相關** | 有（Place/Pick from HP） | 無 |
| **Bin 分類** | 無 | 有（CheckBin / SearchTrayToPlace） |
| **整盤功能** | 無 | 有（Fix3 Full Tray / P27 Sorting BinTray） |
| **Magazine** | 無 | 有（Magazine Tray / Buffer 取放） |
| **Production Log** | 無 | 有（AddUnloadRecord / SaveRecord） |
| **Lot Summary** | 無 | 有（LotSummary.AddCount / iBinData32） |
| **Task 變數** | `iArmTask` | `OutArmTask` |
| **入口 Guard** | 複雜（DoInArm 中有多重檢查） | 簡潔（DoOutArm 無 Guard，Guard 在 DoOutArm_9045） |

---

## 12. FAQ / 常見問題索引

| 問題 | 參考章節 |
|---|---|
| OutArm 的主流程是什麼？ | §4 主狀態機 |
| Shuttle 1 跟 Shuttle 2 怎麼選擇？ | §4 case 100（OldPos / 32Site 優先級） |
| OutArm 怎麼從 Shuttle 吸 IC？ | §5 DoPickFromShuttle |
| 吸取失敗怎麼處理？ | §5 case 2000 Error 處理 |
| IC 放到哪個 Tray？ | §7 case 10 SearchTrayToPlace_9045 / §8 DoOutArmPlaceToAuto |
| 放料時的 Destroy 吹氣如何工作？ | §8 DoOutArmPlaceToAuto (Destroy loop) |
| OutArm 有哪些附加功能？ | §6 Rotator / AOI / Fix AI CCD |
| Clean Out 時的整盤功能？ | §9 case 5000 / §10 CheckOutArmCleanOut |
| Magazine 怎麼運作？ | §4 case 10000-12000 / §9 case 11100 |
| DoOutArmAfterPlaceToAuto 回傳值代表什麼？ | §9 回傳值對照表 |
| OutArm 和 InArm 有什麼區別？ | §11 差異對照表 |
| STM/TW153 交錯模式怎麼影響 OutArm？ | §4 case 1200/2200 內的 `iCloseSiteModeFor2x8` 判斷 |
<!-- preserved-content:end -->
