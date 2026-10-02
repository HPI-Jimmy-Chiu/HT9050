# OutArm — Bin 分類 / CleanOut / FAQ 詳細參考

> 摘自 §10–§13 Bin/CleanOut 流程。主 Skill 見 [ht9045-outarm-flow/SKILL.md](../SKILL.md)。

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
