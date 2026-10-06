> 保存來源：`.claude/skills/ht9045-outarm-flow/references/DoOutArm_FlowChart.md`，main `372b91908`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# DoOutArm() FlowChart

> Source: `aoutarm.cpp` Line 1117-1135  
> Project: HT9011UC_Code_V3.33.897.0_20260306

---

## Summary

`DoOutArm()` 是 OutArm（輸出臂）的最上層入口函式。它負責資料前處理後直接呼叫 `DoOutArm_9045()` 進入 Dispatch。與 `DoInArm()` 不同，此函式不包含複雜的 Guard Checks，Guard 邏輯全在 `DoOutArm_9045()` 中執行。

## Process Flow

```
DoOutArm() Start
     │
     ├─ FRCarryKit.SetHasNullIcToNullIc()    ← 清理 Shuttle 1 上的 NULL IC 標記
     ├─ BRCarryKit.SetHasNullIcToNullIc()    ← 清理 Shuttle 2 上的 NULL IC 標記
     │
     ├─ [Loop] for all Shuttle Rows × Cols:
     │     FRCarryKit.Item[i][j] == HAS_IC || HAS_HOT_IC
     │       → 轉換為 (TEST_PASS + iTestBinCount)    ← 模擬未分類的測試結果
     │     BRCarryKit.Item[i][j] == HAS_IC || HAS_HOT_IC
     │       → 轉換為 (TEST_PASS + iTestBinCount)
     │
     ├─ SetFixTrayMiddleDtata()              ← 設定 Fix Tray 中間資料
     │
     └─ DoOutArm_9045()                      ← 進入 Dispatch（Guard + 分派）
           return
```

## Key Points

| 項目 | 說明 |
|---|---|
| **FRCarryKit** | Shuttle 1 (Front) 上的 IC 資料 |
| **BRCarryKit** | Shuttle 2 (Back/Rear) 上的 IC 資料 |
| **SetHasNullIcToNullIc** | 將 `HAS_NULL_IC` 狀態的 IC 標記為 `NULL_IC`（空的） |
| **HAS_IC → TEST_PASS+iTestBinCount** | 確保 Shuttle 上未被正確設定 BIN 的 IC 有一個預設 BIN 碼 |
| **SetFixTrayMiddleDtata** | 設定 Fix Tray AutoForm 的中間偏移資料 |

## 與 DoInArm() 入口比較

| 項目 | DoInArm() | DoOutArm() |
|---|---|---|
| Guard Checks | 多重檢查（InitIndex / HangUp / F16 / QA / CCD / IndexJam） | 無（Guard 在 DoOutArm_9045 中） |
| 資料前處理 | 無 | 有（Shuttle IC 狀態轉換） |
| 複雜度 | ~150 行 | ~20 行 |

<!-- preserved-content:end -->
