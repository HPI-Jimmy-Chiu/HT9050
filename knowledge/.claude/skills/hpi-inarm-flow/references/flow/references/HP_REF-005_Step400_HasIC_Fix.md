> 保存來源：`.claude/skills/ht9045-inarm-flow/references/HP_REF-005_Step400_HasIC_Fix.md`，main `372b91908`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# REF-005：Step 400 HasIC() 統一修正（3x5 HP Col2 IC 遺失）

| 項目 | 內容 |
|------|------|
| **日期** | 2026-04-02 |
| **版本** | V3.33.900.0 |
| **客戶** | TESNA（843）via TeraTech |
| **機台配置** | 3x5 HP（XDiv=3, YDiv=5），DualSite2x1（e9045_1x2_2_14）|

---

## 症狀

模擬 log（`D:\HT9045_StateRecord\2026-04-02 15_16_42\`）無任何 alarm 或 error，
但 HP 資料確認 **Col2 只有 Row0 有 IC（Aa 放料），Row1 以下全空**（Ad 的 IC 遺失）。

- EventLog：無異常紀錄
- HP 資料（xlrd 讀取）：HP2 Col3 只有 Row0（Count=5, Row=11=Aa），Row1+ 全空

## 根因分析

### Step 400 使用 `GetPlaceToHotPlateSuckCol(j)` 迴圈檢查殘留 IC

原始程式碼（`ainarm9045_1x2_2_14.cpp` Step 400）：

```cpp
case 400:
    if(MoveInArmZToPlateSafe(Task))
    {
        for(int i=0; i<1; i++)
        {
            for(int j=0; j<iStepHP; j++)
            {
                j2=GetPlaceToHotPlateSuckCol(j);
                if(TestIF.iSiteMap[0][j]!=0 &&
                   InArmSuck.Item[i][j2]!=NULL_IC)
                {
                    Task=1;
                    return false;
                }
            }
        }
        Task=500;
    }
    break;
```

### Bug 機制

在 3x5 HP（XDiv=3）Col2 情境下：

1. `GetPlaceToHotPlateSuckCol(j)` 對 Col2 的 j=0 和 j=1 **回傳相同的 j2**：
   - 偶數 Row（`iPlacePlateY[0]%2==0`）→ j2=0（Aa），不論 j=0 或 j=1
   - 奇數 Row（`iPlacePlateY[0]%2==1`）→ j2=3（Ad），不論 j=0 或 j=1

2. 因此 Step 400 的迴圈只能偵測到**主放料吸嘴**是否已放完，
   無法偵測**另一吸嘴**是否仍有 IC

### 時序重建

```
15:16:37.899  InArmPlaceToHotPlateTask  step 1    → SearchPlateToPlace()
15:16:37.901  step 100  → XY 移動
15:16:38.077  step 200
15:16:38.081  step 350  → j=0: Aa 放 Col2/Row0 ✅; j=1: j2=0(Aa)=NULL_IC → skip
15:16:39.115  step 400  → j=0: j2=0(Aa)=NULL → pass; j=1: j2=0(Aa)=NULL → pass
                         → Task=500!（Ad 仍 HAS_IC 但未被檢查到！）
15:16:39.125  step 500 → 501 → return true（IC 永久遺失）
```

---

## 修正方案

### 改用 `InArmSuck.HasIC()` 取代 `GetPlaceToHotPlateSuckCol` 迴圈

```cpp
case 400:
    if(MoveInArmZToPlateSafe(Task))
    {
        if(InArmSuck.HasIC())                                           //Steven 20260402 : Fixed Step400 use HasIC() instead of GetPlaceToHotPlateSuckCol loop
        {
            Task=1;
            return false;
        }

        Task=500;
    }
    break;
```

### 選擇 `HasIC()` 的理由

1. **直接解決 Bug** — 不再依賴 `GetPlaceToHotPlateSuckCol(j)` 回傳值
2. **不需要 `iSiteMap` 判斷** — 關站吸嘴在 pick 階段就不會被填入 IC，`HasIC()` 不會誤判；
   Step 501 的 `SetInArm_Unuse_SuckToNullICForHP()` 負責最終清理
3. **與其他模式一致** — 2x4_8、2x3_6、2x5_8、2x6_8、2x8_8 等模式的 Step 400
   均使用 `HasIC()` 判斷

---

## 修改檔案清單

| # | 檔案 | 原始 Pattern | 修改 |
|---|------|-------------|------|
| 1 | `ainarm9045_1x2_2_14.cpp` (line 531) | `iSiteMap[0][j]` + `GetPlaceToHotPlateSuckCol(j)` | → `HasIC()` |
| 2 | `ainarm9045_1x2_2.cpp` (line 576) | `iSiteMap[0][j]` + `GetPlaceToHotPlateSuckCol(j)` | → `HasIC()` |
| 3 | `ainarm9045_1x2_4_Hot.cpp` (line 823) | `iSiteMap[0][j]` + `GetPlaceToHotPlateSuckCol(j)` | → `HasIC()` |
| 4 | `ainarm9045_2x1_2.cpp` (line 620) | `iSiteMap[j][0]` + `GetPlaceToHotPlateSuckCol(j)`  | → `HasIC()` |

---

## 全模式 Step 400 調查結果

| 模式 | Step 400 Pattern | 是否有 Bug |
|------|-----------------|-----------|
| `1x2_2_14` | ~~SiteMap+Loop~~ → **HasIC()** | ✅ 已修正 |
| `1x2_2` | ~~SiteMap+Loop~~ → **HasIC()** | ✅ 已修正 |
| `1x2_4_Hot` | ~~SiteMap+Loop~~ → **HasIC()** | ✅ 已修正 |
| `2x1_2` | ~~SiteMap+Loop~~ → **HasIC()** | ✅ 已修正 |
| `2x4_8` | HasIC() | ✅ 原始正確 |
| `2x3_6` | HasIC() | ✅ 原始正確 |
| `2x5_8` | HasIC() | ✅ 原始正確 |
| `2x6_8` | HasIC() | ✅ 原始正確 |
| `2x8_8` | HasIC() | ✅ 原始正確 |
| `2x8_32` | HasIC() | ✅ 原始正確 |
| `2x2_8_Hot` | HasIC() | ✅ 原始正確 |
| `1x3_4` | HasIC() | ✅ 原始正確 |
| `1x4_4` | HasIC() | ✅ 原始正確 |
| `1x4_4_Back` | HasIC() | ✅ 原始正確 |
| `1x4_8_Hot` | HasIC() | ✅ 原始正確 |
| `All_1Pick` | HasIC() | ✅ 原始正確 |
| `1x1_1` | 特殊（Prod.fInArmSuck4x8）| ✅ 不同邏輯 |
| `2x2_4` | GetShuttleCol(i,j) | ⚠️ 不同函式，無此 Bug |
| `2x2_4_12` | GetShuttleCol(i,j) | ⚠️ 不同函式，無此 Bug |
| `2x2_4_14` | GetShuttleCol(i,j) | ⚠️ 不同函式，無此 Bug |
| `2x3_6_14` | GetShuttleCol(i,j+iPickKit32) | ⚠️ 不同函式，無此 Bug |
| `2x4_4` | GetShuttleCol(i,j+iPickKit32) | ⚠️ 不同函式，無此 Bug |
| `1x3_2_14` | GetShuttleCol(i,j+iPickKit32) | ⚠️ 不同函式，無此 Bug |
| `1x4_2` | GetShuttleCol(i,j+iPickKit32) | ⚠️ 不同函式，無此 Bug |
| `2x4_16` | 無 DoPlaceToHotPlate 函式 | N/A |

---

## 修正工具

- 修改腳本：`D:\HT9045\AI_Temp\fix_step400_hasic.py`
- 使用 Python bytes 模式（Big5 編碼保護）
- 每個檔案精確匹配 1 次，避免誤改

---

## 關聯項目

- [REF-001](HP_Knowledgebase/29.md#ref-001iforplacehpx3step-race-condition3x5-hp-1x2-mode) — `SearchPlacePlateXItem3_1x2Suck` race condition
- [REF-002](HP_Knowledgebase/29.md#ref-002getplacetohotplatecol-race-condition3x5-hp-1x2-mode) — `GetPlaceToHotPlateCol` race condition
- [REF-003](HP_Knowledgebase/29.md#ref-003新增-debug-logproposal-a--b) — Proposal A + B debug log
- [REF-004](HP_Knowledgebase/29.md#ref-004同類條件風險提示) — 同類條件風險提示

<!-- preserved-content:end -->
