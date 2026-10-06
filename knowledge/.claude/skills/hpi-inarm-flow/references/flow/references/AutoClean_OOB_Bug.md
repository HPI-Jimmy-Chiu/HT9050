> 保存來源：`.claude/skills/ht9045-inarm-flow/references/AutoClean_OOB_Bug.md`，main `372b91908`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Auto Clean — bInArmSuckActive Array OOB Bug 知識庫

> 此檔案從 ht9045-inarm-flow\SKILL.md §12 中分離出來，為獨立 reference。
> 對應 AutoClean.cpp 中 InArm 吸嘴在 Auto Clean 期間的控制邏輯。

---

## 1. 案例摘要（P260428-ATC-H9-01）

| 項目 | 內容 |
|------|------|
| **客戶** | AMKOR Shanghai DLC915 |
| **版本** | V3.33.889（回歸自 JerryYang 20250326 修改） |
| **發現日期** | 2026-04-28 |
| **修正版本** | V3.33.904.0 |
| **修正者** | Steven 20260504 |

---

## 2. 症狀

`bUse8Picker==false` 機台在 Auto Clean Pick-from-Kit 時，A 排（row 0: ACEG）Z 軸吸嘴下降撞擊 Shuttle sensor。B 排（row 1: BDFH）行為正常。

---

## 3. 相關全域變數

| 變數 | 型別 | 說明 |
|------|------|------|
| `bInArmSuckActive[2][4]` | `bool[MAX_ARM_Row][MAX_ARM_Col]` | 標記各吸嘴是否在本次下降動作中需啟動 |
| `bUse8Picker` | `bool` | 是否使用 8-picker（A+B 兩排）；`Type_HT9045_12Site=400` 不在 true 條件內 |
| `InArmSuck.iMotRow` | `int` | 吸嘴排數（`bUse8Picker=false` → 1 排，`true` → 2 排） |
| `InArmSuck.iMotCol` | `int` | 每排吸嘴數 |

---

## 4. 記憶體佈局

```
bool bInArmSuckActive[2][4] 連續配置：
  [0][0] [0][1] [0][2] [0][3] | [1][0] [1][1] [1][2] [1][3]
   A-row  A-row  A-row  A-row     B-row  B-row  B-row  B-row
   ZA     ZC     ZE     ZG        ZB     ZD     ZF     ZH

[1][-1]  等同  [0][3]  (A-row ZG 吸嘴)
```

---

## 5. Bug 根本原因（AutoClean.cpp — `SearchCleanKitUpDown()`）

```cpp
// AutoClean.cpp line ~1965 (V3.33.904 BETA)
// bUse8Picker==false 時 iRealRow=1（B 排）
for(int i=0; i<InArmSuck.iMaxCol; i++)
{
    iSuckCol = GetAutoCleanPickStep(i);
    if(iSuckCol == -1)      // site 被關閉時返回 -1
    {
        // ★ BUG: 空區塊，JerryYang 20250326 移除了 continue
    }
    // ...
    bInArmSuckActive[iRealRow][iSuckCol] = bFlag[iSuckCol];
    // → iSuckCol=-1: bInArmSuckActive[1][-1] = bFlag[-1]
    // → 等同 bInArmSuckActive[0][3] = stack 垃圾值
    // → A 排 ZG 被錯誤標記為需下降
}
```

**觸發條件**：`GetAutoCleanPickStep(i)` 返回 -1（有 site 關閉或 Auto Site Map 關站）。

**回歸版本**：`JerryYang 20250326` 修改（將 `continue` 改為空 `{}`），V3.28A 早於此修改故正常。

---

## 6. 修正（V3.33.904.0）

| # | 位置 | 修正內容 |
|---|------|---------|
| 1 | `SearchCleanKitUpDown()` line ~1967 | `if(iSuckCol==-1) { continue; }` — 補回 continue |
| 2 | `MoveInOutArmZToKitPickPlace()` line ~1057 | 防禦性保護：`bUse8Picker==false && bE43AutoCleanUseHotplate==false` 時強制清除 `bInArmSuckActive[0][j]` |

```cpp
// 修正 1
if(iSuckCol==-1)
{
    continue;    // ★ Steven 20260504 : fix array OOB
}

// 修正 2（MoveInOutArmZToKitPickPlace 入口）
if(bUse8Picker==false && IniConfig.bE43AutoCleanUseHotplate==false)
{
    for(int j=0; j<InArmSuck.iMotCol; j++)
    {
        if(bInArmSuckActive[0][j])
        {
            MyDBIProcess("AutoClean",
                AnsiString().sprintf("MoveInOutArmZToKitPickPlace: bInArmSuckActive[0][%d]=true force false", j),
                "Protection");
            bInArmSuckActive[0][j]=false;
        }
    }
}
```

---

## 7. Auto Clean 核心函式速查

| 函式 | 檔案 | 說明 |
|------|------|------|
| `SearchCleanKitUpDown(iRow, iSht)` | `AutoClean.cpp ~1764` | 設定 `bInArmSuckActive`；`bUse8Picker=false` 時 `iRealRow=1` |
| `MoveInOutArmZToKitPickPlace(Pick, bReset, iSht, iShtRow)` | `AutoClean.cpp ~1025` | 依 `bInArmSuckActive[i][j]` 命令 Z 軸下降 |
| `MoveInArmXYPickCleanKit(iPick, iShtRow, iSht)` | `AutoClean.cpp ~1996` | XY 定位（座標為 B 排設定） |
| `DoAutoCleanPickfromCleanKit(iSht, Restart)` | `AutoClean.cpp ~2565` | 主狀態機 |
| `PickFromCleanKit(iRowKit)` | `AutoClean.cpp ~2458` | `bUse8Picker=false` → `iSuckRow=1`（B 排）|
| `GetInarmSuckRow(iShtRowKit, &isuckRow, &ikitStep)` | `AutoClean.cpp ~9058` | Shuttle row → sucker row 映射 |
| `GetAutoCleanPickStep(i)` | `AutoClean.cpp` | 返回 -1 表示該 site 關閉（需 continue 跳過）|

---

## 8. bUse8Picker 判斷規則

`bUse8Picker=true` 的條件（需全部符合）：
- `USE_PICKER_COUNT == ep8Picker`
- `iInArmType` 為 2-row 配置（2x4、2x5、2x6、2x8 系列）
- 機型 **不是** `Type_HT9045_12Site`（=400）

`Type_HT9045_12Site` 機台（含 AMKOR Shanghai DLC915）→ `bUse8Picker=false`：A 排 Z 軸在 Auto Clean 期間**不應動作**。

---

## 9. EventLog 診斷特徵

A 排 OOB 發生時的 EventLog 樣式（每輪 Auto Clean 固定出現）：
```
ZE  pos -1085 home sensor on   ← A 排 E 吸嘴
ZC  pos -504  home sensor on   ← A 排 C 吸嘴
Auto Clean Finish
```

若看到 A 排吸嘴（ZA/ZC/ZE/ZG）在 Auto Clean 期間觸發 home sensor alarm → 優先懷疑此 OOB bug。

<!-- preserved-content:end -->
