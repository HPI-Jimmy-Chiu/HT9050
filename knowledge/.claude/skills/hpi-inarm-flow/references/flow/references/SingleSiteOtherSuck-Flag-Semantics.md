> 保存來源：`.claude/skills/ht9045-inarm-flow/references/SingleSiteOtherSuck-Flag-Semantics.md`，main `372b91908`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Flag 語意（Single Site Other Suck）+ AutoClean 一致性修正

> 來源：Jerry Yang 2026-04-13 更新；整合日期：2026-04-15

## Part A — 兩個旗標的語意差異

| 旗標 | 影響模組 | 說明 |
|------|---------|------|
| `Prod.bSingleUseOtherSuck` | InArm + OutArm | 單 Site 使用其他吸嘴的**通用**旗標，可同時影響 InArm 與 OutArm 的吸嘴分支 |
| `Prod.bSingleInArmUseOtherSuck` | **僅 InArm** | 僅限 InArm 的吸嘴分支切換（InArm 使用 C 吸嘴情境），設計上不應影響 OutArm 的 Shuttle X/Y 對位補償分支 |

### Cross-Module 注意事項

- 若發現 OutArm 在 1x1 + InArm C 吸嘴設定下需額外手補 X offset（如約 +5mm），優先檢查 OutArm 是否誤用 `Prod.bSingleInArmUseOtherSuck`。
- OutArm 端旗標邊界規則詳見 [outarm-flow/references/SingleSiteOtherSuck-Flag-Boundary.md](../../../../ht9045-outarm-flow/references/SingleSiteOtherSuck-Flag-Boundary.md)

---

## Part B — §12 AutoClean 單站吸嘴一致性（2026-04-13）

### 問題摘要

- 現場回報：Single site 跑 Auto Clean 時，InArm 會使用後排（D 吸嘴），期望改為前排 C 吸嘴。

### 修正重點

- 檔案：`AutoClean/AutoClean.cpp`
- 目的：補齊 AutoClean 對 `bSingleInArmUseOtherSuck` 的判斷一致性。

### 修改點 1：`GetAutoCleanPickStep(int iCol)`

- 原判斷：`Prod.bSingleUseOtherSuck`
- 新判斷：`Prod.bSingleUseOtherSuck || Prod.bSingleInArmUseOtherSuck`

### 修改點 2：`PickFromCleanKit(...)` 的 `K_SKIP` 分支

- 1x1 單站吸嘴欄位判斷
- 原判斷：`iInArmType==e9045_1x1_1 && Prod.bSingleUseOtherSuck`
- 新判斷：`iInArmType==e9045_1x1_1 && (Prod.bSingleUseOtherSuck || Prod.bSingleInArmUseOtherSuck)`

### 維護提醒

- 與 Single site 吸嘴切換相關的邏輯，建議統一以 `Prod.bSingleUseOtherSuck || Prod.bSingleInArmUseOtherSuck` 為條件，避免只判其中一個旗標。
- 原始碼檔（`.cpp/.h/.dfm`）維持 Big5（CP950）編碼，避免 UTF-8 轉碼造成中文註解亂碼。

<!-- preserved-content:end -->
