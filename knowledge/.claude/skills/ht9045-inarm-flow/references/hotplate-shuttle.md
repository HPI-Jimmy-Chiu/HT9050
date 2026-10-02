# InArm HotPlate / Shuttle / 附加功能細節

## DoInArmAdditionalFunction() — 附加功能

> Source: `ainarm9045.cpp` line ~3182  
> Task: `iInArmAdditionalFunctionTask`

依序檢查並執行四種附加功能，每個完成後回到 case 100 檢查下一個：

```
case 1 → case 100 (check flags)
           ├─ Die Clean    → case 10000: DoInDieClean()
           ├─ Precisor     → case 20000: DoInArmDevicePosPrecise()
           ├─ Bottom 2DID  → case 30000: DoBottom2DIDScan()
           ├─ Rotator      → case 40000: Z safe → case 41000: DoInArmRotateKIT()
           └─ All done     → return true
```

---

## DoPlaceToHotPlate_9045() — HotPlate 放料

> Source: `ainarm9045_2x8_8.cpp` line ~1733  
> Task: `iInArmPlaceToHotPlateTask`

### 流程摘要

搜尋 HP 位置 → 移動 XY → Release Delay → Destroy 吹氣放料 → Z safe → 檢查剩餘 IC（分段放料）→ 驗證黏貨

### 關鍵 Case

| case | 動作 |
|---|---|
| 1 | SearchPlateToPlace() → case 100 |
| 100 | MoveInArmXYToHotPlatePlace → case 200；掉料 → case 110 |
| 200 | Offset check + Release delay → case 340/350 |
| 350 | Destroy 吹氣放料（loop 2 rows × N cols）→ case 400 |
| 400 | Z safe + HasIC? → YES: case 1 (分段), NO: case 500 |
| 500 | CheckInArmDestroyICFail（黏貨檢查）→ case 501 |
| 501 | AdjustShuttleWhichKitOrder + Laser check → return true |

---

## DoInArmPickFromHotPlate_9045() — HotPlate 取料

> Source: `ainarm_SearchPickPlate.cpp` line ~683  
> Task: `iInArmPickFromHotPlateTask`

### 流程摘要

搜尋 HP 位置 → 取得 Team 分組 → 移動 XY → 等待 Shuttle 就位 → Z 下降吸取 → HotplateDataConversion 資料轉換 → 異常 Retry/Skip/Home

### 關鍵 Case

| case | 動作 |
|---|---|
| 1 | SearchPlateToPick + SuckerMap → case 50 |
| 50 | Z safe → case 100 |
| 100 | GetHPFirstTeamMotUse / SuckUse（Team 分組）→ case 110 |
| 110 | MoveInArmXYPickHotPlate_9045 → case 150 |
| 150 | 等待 Shuttle 在 Load-Free 位置 → case 190 |
| 200 | Z 下降吸取（MoveInArmZToHotPlatePick）→ case 300 |
| 300 | HotplateDataConversion（資料轉換）→ 異常: case 320, OK: case 340 |
| 330 | JAM0109 alarm → K_SKIP / K_HOME / K_RETRY |
| 350 | DataForwardAndNextTeam → 還有: case 1, 全完成: case 400 |
| 400 | return true |

---

## DoInArmPlaceToShuttle_9045() — Shuttle 放料

> Source: `ainarm9045_2x8_8.cpp` line ~2039  
> Task: `iInArmPlaceToShuttleTask`

### 架構

兩個 Shuttle 邏輯對稱：
- **Shuttle 1**: case 900 ~ 1610
- **Shuttle 2**: case 1900 ~ 2610（結構相同，case 編號 +1000）

### 關鍵 Case（以 Shuttle 1 為例）

| case | 動作 |
|---|---|
| 1 | SetInArmUseSuckToHasNullIC、HasRealIC 檢查 → case 100 |
| 100 | 選擇 Shuttle 1 (case 900) 或 Shuttle 2 (case 1900) |
| 900 | 等待 InSHT1InLF → speed adjust → case 930/950 |
| 930 | MoveInArmXYToWaitTrayArm(ZAxisNotDown) → case 1000 |
| 950 | MoveInArmXYToWaitTrayArm(ZAxisDown) → case 1100 |
| 1000 | 檢查 carrier kit / bCanFreeShuttle → case 1050 or 900 |
| 1100 | MoveInArmZToShuttlePlace_9045 + Offset → case 1200 |
| 1200 | Destroy 吹氣（loop 2×4）→ 還有: case 1250, 全放完: case 1300 |
| 1300 | InitDoInArmCheckShtFloatTask → case 1400 |
| 1400 | DoInArmCheckShuttleFloating（浮起檢查: 搖搖敲敲）→ case 1500 |
| 1500 | AdjustShtOrderWhenPlaceToSht + Soak time → return true |
| 1600 | Y Pitch Home (bE57YPitchHome) → return true |
