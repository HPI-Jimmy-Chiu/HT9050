---
name: ht9045-contact-pick-interlock
description: HT9045 Contact 模式「取料資料流 + 模式互鎖」深度知識庫。當使用者詢問 Contact test 切不回、切不換模式、Please finish ONE CYCLE、機台有IC不能切換模式、DoZ1PickFromShuttle、DoZ2PickFromShuttle、case 3050 掉料防護、bContactTestICDropGuard、IndexHasIC、ShuttleHasIC、TestSocketHasIC、TestHeadHasIC、FTestSuck/BTestSuck/FLCarryKit/BLCarryKit 殘留、SetItemData vs MoveSuckData、iFTestBackItem/iBTestBackItem 還原、Contact 掉料中止、單臂/雙臂部分上料誤判掉料、HAS_IC 守護門檻、客戶碼隔離掉料防護等相關問題時，應先載入此技能。關鍵字：Contact 切不回, Please finish ONE CYCLE, 機台有IC不能切換模式, DoZ1PickFromShuttle, DoZ2PickFromShuttle, case 3050, bContactTestICDropGuard, IndexHasIC, ShuttleHasIC, TestSocketHasIC, FTestSuck, BTestSuck, FLCarryKit, BLCarryKit, SetItemData, MoveSuckData, iFTestBackItem, iBTestBackItem, HAS_IC, NULL_IC, 掉料防護, 部分上料, 客戶碼隔離, CC_GIGAS。
---

# HT9045 Contact 取料資料流 + 模式互鎖

> 本 Skill 補足 `ht9045-contact-force`（純力量數學）缺少的一塊：
> Contact 模式下「IC 資料在哪些容器之間搬移、誰負責清空、沒清乾淨為什麼鎖死模式」。
> 來源版本：`HT9045/HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422`（V899）。

## 適用場景

- Contact test 掉料/吸取異常後，**切不回 Contact test**、模式 widget 變灰不能切
- 出現 `Please finish ONE CYCLE before Contact Test!`（請先完成 ONE CYCLE 再執行 Contact test）
- `DoZ1PickFromShuttle` / `DoZ2PickFromShuttle` 取料狀態機（case 3000→3050→3100→700）
- 掉料防護 `bContactTestICDropGuard`、單臂/雙臂部分上料誤判掉料
- 追 IC 資料殘留（FTestSuck / BTestSuck / TestSocket / 4 個 CarryKit）
- 客戶碼隔離（只改某客戶行為）的雙層閘門寫法

## 核心檔案

| 檔案 | 內容 |
|------|------|
| `cContact.cpp` | `DoZ1/Z2PickFromShuttle` 取料狀態機、`DoTestContactFunction` 排程、`timerContactTimer` 模式鎖、`FormClose` 退出還原 |
| `csystem.cpp` | `IndexHasIC` / `ShuttleHasIC` / `TestHeadHasIC` / `TestSocketHasIC` 等互鎖判斷函式（#815-895） |
| `MyKitSuck.cpp` | `TMyKitSuck::SetItemData` / `MoveSuckData` / `UseSiteHasIC`（IC 資料容器） |

---

## 1. IC 資料容器模型（Contact 情境）

Contact 模式下 IC 的「邏輯位置」由數個 `TMyKitSuck` 物件的 `Item[i][j]` 記錄，
值為 `NULL_IC`（空）/ `HAS_IC`（有 IC）/ `HAS_NULL_IC`（空殼）等。

| 容器 | 物理位置 | 角色 |
|------|----------|------|
| `FLCarryKit` | Input Shuttle Front（入料飛梭前） | InArm 放上來的待測 IC |
| `BLCarryKit` | Input Shuttle Rear（入料飛梭後） | 同上（Arm2 側） |
| `FRCarryKit` | Output Shuttle Front（出料飛梭前） | 測完待 OutArm 取走 |
| `BRCarryKit` | Output Shuttle Rear（出料飛梭後） | 同上 |
| `FTestSuck` | Front Test Head（Z1 測試頭吸嘴） | Z1 從飛梭吸起的 IC |
| `BTestSuck` | Rear Test Head（Z2 測試頭吸嘴） | Z2 從飛梭吸起的 IC |
| `TestSocket` | Socket（測試座） | 下壓進 Socket 的 IC |

### SetItemData vs MoveSuckData（關鍵差異）

| 操作 | 行為 | 來源容器是否被清空 |
|------|------|--------------------|
| `FTestSuck.SetItemData(i,j,HAS_IC)` | 只把 FTestSuck 標成 HAS_IC | ? 不動來源，**飛梭 FLCarryKit 仍保留 HAS_IC** |
| `FTestSuck.MoveSuckData(FLCarryKit,i,j)` | 把 FLCarryKit 的料「搬」到 FTestSuck | ? 來源 FLCarryKit 被清空 |

> 這是踩雷重點：case 3050 在「無 2DID / 非 DeviceMapCheck」分支走的是
> `SetItemData(HAS_IC)`，**不會搬空飛梭**。所以即使測試頭資料被清，
> 飛梭 FLCarryKit/BLCarryKit 仍是 HAS_IC → `ShuttleHasIC()` 仍 true。

---

## 2. 模式互鎖鏈（為什麼切不回 Contact）

### 兩個互鎖點（FACT, cContact.cpp）

| 互鎖點 | 位置 | 行為 |
|--------|------|------|
| 啟動 Contact test 擋關 | `DoTestContactFunction()` case 1（#10954-10960） | 若有殘留 IC → `ShowMyMessage("Please finish ONE CYCLE before Contact Test!")` 並 return，**不讓進** |
| 模式切換 widget 鎖 | `timerContactTimer()`（#20156，JerryYang 20251118「機台有IC不能切換模式」） | 若有殘留 IC → `rgHandlerMode->Enabled=false; rbModeNormal->Enabled=false`，**畫面變灰** |

兩者判斷條件完全相同：
```cpp
IndexHasIC() || InArmSuck.HasIC() || OutArmSuck.HasIC() || ShuttleHasIC()
```

### 互鎖看的資料來源（FACT, csystem.cpp #815-895）

```
IndexHasIC()      = TestHeadHasIC() || TestSocketHasIC()
  TestHeadHasIC() = FrontTestHeadHasIC()(FTestSuck) || RearTestHeadHasIC()(BTestSuck)
  TestSocketHasIC()= TestSocket.UseSiteHasIC()

ShuttleHasIC()    = InputShuttleHasIC() || OutputShuttleHasIC()
  InputShuttleHasIC()  = FLCarryKit.UseSiteHasIC() || BLCarryKit.UseSiteHasIC()
  OutputShuttleHasIC() = FRCarryKit.UseSiteHasIC() || BRCarryKit.UseSiteHasIC()
```

> 注意：`InputShuttleRearHasIC()` / `OutputShuttleRearHasIC()` 在
> `bIndexArm2SupplyLight` / `bForEgisTecTest` / `bD58UseArm1PickPlaceArm2Test`
> 為 true 時直接 return false（Arm2 不算殘留）。

### 「想切模式被擋」反查表（從現象 → 查哪個容器）

切不回 Contact / widget 變灰時，依序檢查這 7 個容器的 `Item[i][j]` 是否 `!=NULL_IC`：

| 順位 | 容器 | 退出時誰會清 |
|------|------|--------------|
| 1 | `FTestSuck` | ? FormClose #1783-1790 還原（iFTestBackItem） |
| 2 | `BTestSuck` | ? FormClose #1783-1790 還原（iBTestBackItem） |
| 3 | `FLCarryKit` | ? **無人清**（最常見殘留鎖死源） |
| 4 | `BLCarryKit` | ? **無人清** |
| 5 | `TestSocket` | ? **無人清** |
| 6 | `FRCarryKit` | ? 無人清 |
| 7 | `BRCarryKit` | ? 無人清 |

---

## 3. DoZ1/Z2PickFromShuttle 取料狀態機（重點 case）

`DoZ1PickFromShuttle()`（#2417）/ `DoZ2PickFromShuttle()`（#3465），
`int &Task = Z_Height_Task`，回 `bool`（true=完成）。Contact test 走的主路徑：

```
1     → Z 軸回安全位 + 寫 EP 壓力
100   → 吸嘴上有 IC 自檢（CheckIndexArmStatus）
150   → Shuttle 右移 + Y 軸到測試位
200   → 真空 On + 判斷模式（Auto Height→300 / Contact test→3000）
3000  → Z 下降到飛梭取料高度，逐 site 開真空吸取
3050  → 吸取後：寫 IC 資料（SetItemData/MoveSuckData）+【掉料防護】
3100  → Z 上升 + CheckIndexAllSuckICFallDown 掉料二次檢查
700   → Z 回安全位，return true
```

### case 3050 寫資料邏輯（FACT）

逐 site 對 `FTestSuck`（Z2 為 `BTestSuck`）：
1. REALLY 模式且真空 sensor OFF（沒吸到）→ `Suck.Off()`（防沾黏），**不寫 HAS_IC**
2. 有吸到 →
   - DeviceMapCheck 或關 2DID：`if(FLCarryKit.Item!=NULL_IC) MoveSuckData` 否則 `SetItemData(HAS_IC)`
   - 一般：`SetItemData(i,j,HAS_IC)`（**不搬空飛梭**）

> 推論[INFER 基於 §1]：`HAS_IC` 只在「REALLY + 真空 sensor ON」時被 SetItemData，
> 所以 `FTestSuck.Item==HAS_IC` 等價於「該 site 實際吸取成功」。

---

## 4. 掉料防護 bContactTestICDropGuard（CASE-20260608-001 全智）

### 開關與豁免

- 外層總開關：`CosFunction.bContactTestICDropGuard`（false 時整段不執行，非客戶不受影響）
- 適用模式：`CONTACT_TEST` / `AUTO_CONTACT_TEST` / `CONTACT_AUTO_GET_HEIGHT`
- 客戶豁免旗標：`bGigasContactTestPartialPick = (CUSTOMER_CODE==CC_GIGAS && 上述模式)`

### 守護門檻（V899 現行邏輯）

```cpp
if(bGigasContactTestPartialPick)
    bNeedGuard = (FTestSuck.Item[i][j]==HAS_IC);          // 全智：只守實際吸到料的 site
else
    bNeedGuard = (LastSet.bUseTestSocket[0][i+iNN][j]==true); // 非全智：依 recipe site 開關（原行為）

if(bNeedGuard && FTestSuck.Suck[i][j].GetStatus()==false)
    bDropDetected = true;                                 // 該守的 site 真空掉了 → 掉料
```

- 全智允許「整臂本輪不上料」（部分手動上料）時跳過防護不報錯
- 已移除「整臂無守護 site 即強制報掉料」的致命 fallback

### 掉料中止前的還原（問題 b 修正）

掉料確認中止前，比照 FormClose 把本臂測試頭暫存資料還原回進站前：
```cpp
FTestSuck.SetItemData(ci,cj,iFTestBackItem[ci][cj]);   // Z2 用 BTestSuck / iBTestBackItem
```

> ?? **已知限制（重要）**：此還原只清 `FTestSuck/BTestSuck`（TestHead），
> **未清 TestSocket 與 4 個 CarryKit**。若殘留鎖死源是飛梭 `FLCarryKit/BLCarryKit`
> 或 `TestSocket`（§2 反查表 3~7），切不回 Contact 的問題仍可能存在。
> 補清飛梭/Socket 前必須先現場確認那是「掉料假殘留資料」還是「真有實體散料」——
> 真有散料卻把資料清成 NULL_IC，下一刀可能壓壞料。這正是原 code 只敢清 TestHead 的原因。

---

## 5. 客戶碼隔離雙層閘門範式（可重用）

修改只想影響單一客戶、不汙染其他客戶時，用雙層閘門：

```cpp
// 外層：CosFunction 開關（INI 控制，預設 false → 完全不改變既有行為）
if(CosFunction.bContactTestICDropGuard==true && <適用模式>)
{
    // 內層：客戶碼鎖定
    bool bGigasXxx = (CUSTOMER_CODE==CC_GIGAS && <適用模式>);
    if(bGigasXxx) { /* 只有全智走新邏輯 */ }
    else          { /* 其他客戶維持原行為 */ }
}
```

註解規範（每段變更）：`//AI(ht9045-v899) YYYYMMDD: 動作+原因(case-id)`，BIG5、pre-C++11。

---

## 6. 排查 SOP：Contact 掉料後切不回 Contact test

1. 重現「掉料中止 → 切不回 / widget 變灰」狀態，**不要關畫面**（關了 FormClose 會清掉 §2 表第 1-2 項，反而看不到真兇）。
2. 依 §2 反查表，逐一檢視 `FLCarryKit / BLCarryKit / TestSocket / FRCarryKit / BRCarryKit` 的 `Item[i][j]` 是否 `!=NULL_IC`。
3. 找到 `!=NULL_IC` 的容器 = 真正的鎖死源。
4. 確認那是「掉料後的假殘留資料」還是「真有實體 IC」。
5. 假殘留 → 在掉料中止路徑補還原該容器（綁客戶碼 + 防壓料判斷）；真有料 → 屬機構/上料問題，不可清資料。
