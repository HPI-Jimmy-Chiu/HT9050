> 保存來源：`.claude/skills/ht9045-outarm-flow/references/outarm-bin.md`，main `372b91908`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# OutArm — Bin 分類 / CleanOut / FAQ 詳細參考

> 摘自 §10–§13 Bin/CleanOut 流程。主 Skill 見 [ht9045-outarm-flow/SKILL.md](../original-entry.md)。

## 10. CheckOutArmCleanOut() — 停止/CleanOut 判斷

> Source: `aoutarm.cpp` L~215（在 `DoOutArmPlaceToAuto_9045()` case 300 進入放料前調用）

### 流程邏輯

```
CheckOutArmCleanOut(iWhichAuto)
  ├─ Fix3 FullTray              → return 5000  (P27 Fix3 整盤)
  ├─ Magazine 已滿              → return 11100 (Magazine overflow)
  ├─ ATK AMR Fix Tray 需換盤    → return 11100
  ├─ Auto Z Teach 需執行        → return 310   (Auto Z Teach)
  └─ 正常                       → return 0
```

### 回傳值對照

| 回傳值 | 意義 | 主狀態機處理 |
|---|---|---|
| 0 | 正常，可繼續放料 | 繼續 case 300 放料 |
| 310 | Auto Z Teach 需執行 | case 310 暫停等待 |
| 5000 | P27 Fix3 Full Tray — 整盤 | case 5000 DoSortingBinTray |
| 11100 | Magazine 滿 / ATK AMR Fix Tray | case 11100 Magazine 換盤 |

### 觸發條件細節

| 條件 | 相關設定 / 函數 |
|---|---|
| P27 Fix3 Full | `GetFix3TrayFull()` 回傳 true |
| Magazine overflow | `GetMagazineOverflow()` 或 `iMagZineCount >= iMagZineMax` |
| ATK AMR Fix Tray | `CC_TERATECHATK` 客戶代碼 && `iAMRFixTraySelect==1` |
| Auto Z Teach | `bNeedAutoZTeach==true` && `IsAutoZTeachTime()` |

## 11. OutArm vs InArm 差異對照表

| 項目 | InArm | OutArm |
|---|---|---|
| **主函數** | `DoInArm()` → `DoInArm_9045()` | `DoOutArm()` → `DoOutArm_9045()` |
| **取料來源** | Loader Tray | Shuttle |
| **放料目標** | HotPlate / Shuttle | Auto Tray / Fix Tray / Unloader |
| **附加功能** | Precisor / Rotator / Die Clean / Bottom 2DID | Rotator / AOI / Fix AI CCD |
| **SearchTray** | `SearchPlateToPlace_9045()` → HotPlate 空格 | `SearchTrayToPlace_9045()` → Auto Tray 空格 |
| **Destroy** | 放 HotPlate 後不 Destroy（吸著）| 放 Auto Tray 前 Destroy（吹氣放料）|
| **Bin 計數** | 無（入料不做 Bin）| `LastSet.BinCT[]`、`LastSet.iBinData32[]` |
| **CleanOut** | `CheckInArmCleanOut()` | `CheckOutArmCleanOut()` |
| **Pick Error retry** | 3 次後 `Skip/Home` | 3 次後 `Skip/Home` |
| **Vacuum Sensor JAM** | JAM0109 | JAM0217 |
| **ArmType** | `iInArmType` | `iOutArmType` |

## 12. SearchTrayToPlace() — 搜尋放料 Tray 位置

> Source: `aoutarm9045.cpp` L~2150

流程：
1. 遍歷 `iOutBinSelect[]`（依 BIN 設定找對應 Auto/Fix Tray slot）
2. 套用 `iBinBoxSelect` 確認 BinBox 啟用
3. 確認 `MOT[Motor].Tray.fHasTray && !FullIC` → 找到空格
4. 回傳 `iWhichAuto`（1=Auto1, 2=Auto2, 3=Auto3, 10=Fix1, 11=Fix2, 12=Fix3 P27）

特殊情況：
- `BinBox` 模式：固定放入 BinBox，不依 Bin 分類
- Fix3／P27：受 `CheckOutArmCleanOut()` 保護（滿後 return 5000）
- ATK AMR：透過 `iAMRFixTraySelect` 旋轉 Fix Tray 槽號

## 13. DoSortingBinTray() — Clean Out 整盤分類

> Source: `aoutarm9045.cpp` / `aoutarm.cpp` | 從 `case 5000` 進入

```
DoSortingBinTray()
  ├─ 鎖定所有 Auto/Fix Tray slot → 禁止其他放料
  ├─ 呼叫 CatchTray 換盤（若需要）
  ├─ 重置 SearchTrayToPlace 搜尋序列
  └─ return → 回主狀態機 case 100
```

主要用途：
- P27 Fix3 Tray Full 後強制整盤
- 多 Bin Tray Sort（依 Bin 分配到不同槽位）
- Clean Out 命令後清空所有 IC

## 14. CheckBin() — 無有效 bin 的料「靜默」導向 Error 盤

> Source: `aoutarm.cpp` `int CheckBin(int &ct, int iShuttle)`（V3.33.908.8 約 L602）
> 呼叫點（**每顆料一次**）：`aoutarm.cpp`（OutArm 主出料）、`asortarm.cpp`（SortArm）、`fAOI.cpp` ×3、`FixAICCD.cpp`

### 核心兩行

```cpp
ct=(ct>iTestBinCount-1)?Prod.iIfErrorT6:Prod.iT6CatData[ct];   // ← 無有效 bin → 改判到 Error 盤
                                                               //   (908.8 約 L673)
if(ct!=Prod.iIfErrorT6)                                        // ← 上一行剛把 ct 變成 iIfErrorT6，必然 false
{   ...  bShowMess=true;  ...  }                               //    整個報警區塊被跳過
...
if(bShowMess && bAlarm)  ShowErrorMessage("WAR07356", ...);     // ← 因此永遠不報警
```

**結論：「本來就沒有有效 bin」的料，一顆一顆安靜放進 Error 盤，不報警、不留紀錄。**
只有 Tray Feed / One-Cycle-Finish 時的計數結算（`MyDBIProductionData()` 的 `Fix3Count` 等欄位）才看得出來 —— 也就是**損失發生時完全無聲，事後才從數字發現**。

`WAR07356` 只在「bin 是**有效值**但對應盤別不合法」時才報（`ct<eAuto1`、超出 Fix 範圍、Error Bin 設到 BulkBox/Magazine/`tNotUse` 等）。

### 什麼情況 `ct > iTestBinCount-1`

| 來源 | `ct` 值 | 說明 |
|---|---|---|
| I/F Error 假 bin | `= iTestBinCount` | `SetAllRealIC2InterfaceBin()` / `Item=TEST_PASS+iTestBinCount` → **從未被測過** |
| Tester 逾時 | `999` | `atester.cpp` 逾時路徑 `iTesterBIN[i][j]=999` |
| 2DID 無碼不測 | `= iTestBinCount` | `iNoCodeDeviceToErr==2` / `bBarcodeErrNoTestAndShowH`（記 `"NonTestToRBin"`） |
| 關 site 有料不測 | `= iTestBinCount` | `[Site Yield Alarm] HP Close Site Do Not Test != 0`（記 `"NonTestToRBin"` / `"NonTestToSettedBin"`） |

`iTestBinCount` 由 Tester I/F 型別決定（`cTesterIF.cpp`）：RS232 32Bin→`iRs232MaxBinCount`、GPIB 256Bin→255、GPIB 16Bin→17、GPIB 32Bin→**33**、其他→16。

### 修正（V3.33.908.8 / 20260804）—— 只補軌跡，零行為變更

在轉換行**之前**插入連續計數 + 門檻式 log（`MyDBIProcess("Message", …)`）：

```
No valid bin to Error tray : bin=%d, Sht=%d, consecutive=%d pcs
```

- **連續 8 顆（= 一組吸嘴量，語意「一整組料都沒有測試結果」）才記一筆**，之後每 8 顆再記一筆
- 遇到有效 bin 就歸零 → **正常生產零星單顆（2DID 無碼、逾時丟 R Bin）完全不記**，不洗版
- 不改 `ct` / `bShowMess` / `bAlarm` / 回傳值 → 分盤與報警行為**位元級不變**

判讀：

| log 內容 | 判讀 |
|---|---|
| `bin=`（= `iTestBinCount`） | **I/F Error 假 bin ＝ 從未被測過** |
| `bin=999` | tester 逾時丟 R Bin |
| `consecutive=` 持續累加 | **整批料正在無聲進 ER，要立刻停機查** |
| `Sht=0/1` / `Sht=2` | OutArm 前/後臂 / SortArm |

**已知限制**：counter 由 6 個呼叫點共用，OutArm 與 AOI/FixAICCD 交錯呼叫時連續計數可能被打斷；log 行帶 `Sht=` 可分辨。

### 實例（偉測 HHT-79 20260802）

RT 投入 133 顆，因 [Pattern #17](../../../../ht9045-state-record-analysis/references/deadlock-patterns.md) 全部滯留熱盤未測（GPIB `START TEST` 全程 0 次）。09:44:30 按 HOME 後 InArm 恢復，09:46:27–09:57:17 由 5 次 ONE CYCLE + 最後的 Tray Feed 把料清出 —— **131 顆一顆一顆經 OutArm 靜默進 Fix3(ER)，全程零報警**。09:57:18 結算才顯示 `Load 133 / Unload 131 / Fix3Count 131 / Auto1~3 = 0`。
「判 Err」不是某一刻的動作，而是「一直沒有測試結果」這個既成狀態在 OutArm 分盤那一刻被兌現。

## FAQ / 常見問題索引

| 問題 | 對應章節 |
|---|---|
| OutArm 取料後就不放，一直等待 | §2 Shuttle 取料等 → [outarm-pick.md](outarm-pick.md) |
| DoOutArm_9045() 主狀態機 case 說明 | §4 主狀態機 → [outarm-pick.md](outarm-pick.md) |
| InArm 和 OutArm 流程差在哪 | §11 對照表（本頁）|
| Rotator 或 AOI 什麼時候觸發 | §6 AdditionalFunction → [outarm-place.md](outarm-place.md) |
| `DoOutArmPlaceToAuto_9045` 放不到 Tray | §7 PlaceToAuto_9045 → [outarm-place.md](outarm-place.md) |
| Destroy 吹氣後計數異常 | §8 PlaceToAuto → [outarm-place.md](outarm-place.md) |
| Fix3 整盤或 Magazine 滿盤觸發 | §10 CheckOutArmCleanOut（本頁）|
| DoOutArm() 函數入口參數 | §1 → [outarm-pick.md](outarm-pick.md) |
| 掉料偵測 / IC Fall Down OutArm 端 | §3 DoPickFromShuttle → [outarm-pick.md](outarm-pick.md) |
| 料進了 Error/ER 盤但完全沒報警、事後才從計數發現 | §14 CheckBin 靜默導向 Error 盤（本頁）|
| 整批料無聲進 ER、`Fix3Count` 暴增而 `Auto1~3=0` | §14（本頁）＋ [Pattern #17](../../../../ht9045-state-record-analysis/references/deadlock-patterns.md) |

<!-- preserved-content:end -->
