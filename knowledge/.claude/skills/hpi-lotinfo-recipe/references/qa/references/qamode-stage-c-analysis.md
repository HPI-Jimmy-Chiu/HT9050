> 保存來源：`.claude/skills/ht9045-qamode/references/qamode-stage-c-analysis.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# QA Mode 階段 C「Fix 模式收尾」深入分析

> 本文件為 `ht9045-qamode` SKILL §5.4 的詳細補充。

## 階段 C 目的

把 `ArmSpeed[InArm].bVariModeFIX=true` → 讓最後 20 顆切到
Fix（close-pitch）模式 → `SearchLoadTrayUpDown_9045` 走 `Find_InArm_Single`
（每 cycle 只吸 **1 顆**）→ counter 連續 +1 → 必命中 Stage A。

## `ainarm9045_2x8_32.cpp` 為死碼

Grep `HT9045.bpr` 確認：**該檔未列入專案編譯**。`e9045_2x8_32`
實際 dispatch 至 `DoInArm_9045_2x8_8()`（ainarm9045.cpp:4135）：
```cpp
else if(iInArmType==e9045_2x8_32)
{
    DoInArm_9045_2x8_8();              // <-- 不是 _2x8_32！
}
```
→ 32-Site N Mode 與 16-Site (2x8_8) 共用同一條 InArm 流程。

## Find_InArm_Single 生效確認

`SearchLoadTrayUpDown_9045`（ainarm9045.cpp:6227）：
```cpp
AdjustInArmClosePitchCondition(bCanPick2ICAtOnceTime);  // bVariModeFIX=true ⇒ false
if(bCanPick2ICAtOnceTime)
    Find_InArm_PickerMaxUseCountOnTime(...);   // 多顆 batch
else
    Find_InArm_Single(iUseSuck, iRow, iCol);    // 1 顆
```

## ATK V3.21.904.1 失效的三個可能根因

1. **bVariModeFIX 被其他路徑覆寫**：csystem.cpp 有 7 處 restore
   （10466, 10916, 10967, 12004, 13088, 14920, 14951）
2. **Stage C 觸發時本輪 cycle 的 pick mode 已決定**
3. **多 Site mapping 關 site 後 effective batch ≠ 1**

→ 不論根因，已在 904.2 以 `>=` 修正防護。

<!-- preserved-content:end -->
