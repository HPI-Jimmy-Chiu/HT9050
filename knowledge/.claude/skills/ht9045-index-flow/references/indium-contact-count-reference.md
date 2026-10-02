# 銦片計數（Indium LifeTime Count）完整 Reference

> 跨客戶銦片接觸壽命計數功能的完整參數使用關係。  
> 相關 Config 開關說明請參考 `ht9045-config` SKILL §6（LifeTimeCount）。

---

## 1. 旗標啟用條件

| 客戶 | 啟用位置 | 條件 | 設定的旗標 |
|------|----------|------|-----------|
| **VTEST** | `cprod.cpp` L3709 | `bVTESTFunction && ATC_SYSTEM > eATCUninstall` | `bUseHeadContactCount=true` |
| **SCC** | `CosFunction.cpp` L989 | `CC_SCC` 專屬區塊 | `bUseHeadContactCount=true` |
| **KYEC 海思** | `cprod.cpp` L3709 | `bHiSiliconFunction` | `bUseHeadContactCount=true` |
| **AMD_M** | 同上 | `bHiSiliconFunction` | `bUseHeadContactCount=true` |

> `bHeadContactCountByRecipe` 已為 dead flag（2026/05/15 全部註解），不再有作用。

---

## 2. 相關變數

| 變數 | 說明 |
|------|------|
| `IniConfig.bLifeTimeCount[3]` | O12/O13/O14 開關（Config 頁面勾選） |
| `IniConfig.ContactSet[x][i][j]` | SPEC 上限值：x=條件組(0~2)、i=Arm(0/1)、j=Head(0~15) |
| `IniConfig.HeadContactCount[x][i][j]` | 實際接觸計數 |
| `IniConfig.HeadContactCountHistory[x][i][j]` | 歷史累計 |
| `TestIF_File.iContactAlarmCount[4]` | UI 顯示用 SPEC 單值（Page1~4 各一） |
| `CosFunction.bUseHeadContactCount` | 功能總開關（由客戶函式設定） |
| `IniConfig.bD05_1SaveSocketCntByHandler` | D05 旗標：true=SocketCount.ini / false=Recipe |

---

## 2.5 實體架構與索引語意（權威定義）

> **2026/07/28 由 RogerYang 提供機構事實確認，此節為判讀 `ContactSet` / `HeadContactCount` 索引的唯一依據。**

### 2.5.1 機構配置

**銦片裝在測試頭上，測試頭裝在 Arm 上；一個 head 一片銦片。**

以 2×4 site 機台為例：

```
Arm1 ── SLK 模組(2×4) ── 8 個測試頭 ── 8 片銦片
Arm2 ── SLK 模組(2×4) ── 8 個測試頭 ── 8 片銦片
                                      ──────────
                                      合計 16 片銦片

Socket 測試座 2×4 = 8 個測試區域（固定，不隨 Arm 走）
Arm1 / Arm2 交換下壓到同一組 8 個 socket
```

**推論**：
- 銦片數量 = `Arm 數 × 每臂 head 數` = 2 × (iShtRow × iShtCol)
- Socket 數量 = `iShtRow × iShtCol`
- **兩者是不同的實體、要分開計數**，程式中也確實是兩組獨立陣列：

| 陣列 | 維度 | 對應實體 | 2×4 時的數量 |
|------|------|----------|:-----------:|
| `IniConfig.HeadContactCount[3][2][16]` | `[條件組][Arm][head]` | **銦片**（在 Arm 上） | **16** |
| `IniConfig.SocketContactCount[4][8]` | `[row][col]` | **Socket 測試座**（固定） | **8** |

### 2.5.2 索引語意 —— dim2 是 Arm，不是 row

```cpp
IniConfig.ContactSet[x][iArm][iPos];          // x=條件組(0~2)
IniConfig.HeadContactCount[x][iArm][iPos];    // iArm=0/1 → Arm1/Arm2
                                              // iPos = j*2+i → 單臂內 head 線性位置
```

`iPos` 的計算與 `bUseTestSocket[arm][row][col]` 的 (row, col) 對應：

```cpp
iPos = j*2 + i;     // i = FTestSuck.iShtRow 方向, j = FTestSuck.iShtCol 方向
```

> ⚠️ **注意 `iPos` 在 `iShtRow==1` 時不連續**（只會產生 0,2,4,6…）。
> 任何要「只掃實際存在的 head」的迴圈，**必須用與計數端相同的雙層迴圈**重算 `iPos`，
> 不可用 `iPos < iShtRow*iShtCol` 之類的範圍判斷。

### 2.5.3 三端一致性檢查（2026/07/28 稽核結果）

| 角色 | 位置 | 實際索引 | 對嗎 |
|------|------|----------|:----:|
| **計數端**（全客戶，Ifor 20160516） | `atester_ProcessCount.cpp` `ProcessHeadContactCount()` | `[iA][Index=Arm][j*2+i]` | ✅ |
| **檢查端 — 芯云**（RogerYang 20260626） | `csystem.cpp` `CheckContactOver()` | `[x][iArm][iPos]`（iPos 固定 0~7） | ✅ 語意對 |
| **檢查端 — 非芯云** | `csystem.cpp` `CheckContactOver()` | `[x][i][j]`，i<iShtRow、j<iShtCol | ❌ |
| **UI 顯示 — 芯云** | `cStartCondition.cpp` FormShow | `[tag][iArm][iPos]`，grid row `1+(iArm*8+iPos)` | ✅ |
| **UI 顯示 — 非芯云** | `cStartCondition.cpp` FormShow | `[tag][i][j]`，grid row `1+(i*8+j)`，標籤 `Arm{i+1} Head{j+1}` | ❌ |

**非芯云的缺陷（自 2016 年存在，跨所有 `bUseHeadContactCount` 客戶）：**

以 2×4 為例 —— 計數端寫 `dim3 = 0~7`，非芯云檢查端 / UI 只看 `dim3 = 0~3`：

- **每支 Arm 有一半的銦片（head 5~8）只累加、從不檢查壽命，也不顯示在畫面上**
- UI 標籤把 `i`（實體 row）標成「Arm」，是**誤標**
- grid 只用到 8 列（row 1-4、9-12），row 5-8、13-16 永遠空白

芯云在 20260626 已把 UI + 檢查端對齊到計數端，用滿 16 列。**芯云是對的，非芯云是錯的。**

### 2.5.4 UI 容量上限

`sgHeadCondition1/2/3` 的 `RowCount = 17`（`cStartCondition.dfm`）→ **16 列資料 = 2 Arm × 8 head**。

| 每臂 head 數 | 芯云 grid row `1+(iArm*8+iPos)` | 結果 |
|:---:|---|---|
| ≤ 4（2×2、1×4…） | Arm1 用 1~4、Arm2 用 9~12 | ✅ |
| **8**（2×4） | Arm1 用 1~8、Arm2 用 9~16 | ✅ 剛好用滿 |
| **> 8**（2×8…） | Arm1 用 1~16、Arm2 用 9~24 | ❌ **列重疊 + 超出 RowCount → VCL "Grid index out of range" 例外** |

> `1+(iArm*8+iPos)` 的 `*8` 硬編碼隱含「每臂最多 8 head」。
> **UI 天花板 = 16 片銦片**；若機型每臂超過 8 head，需同步加大 `RowCount` 與改用動態列距，
> 且回讀迴圈（固定 `i<2, j<8`）也要一起改。

### 2.5.5 全客戶對齊到 by-arm 模型的評估

**不需要資料遷移** —— `HeadContactCount` 只由計數端寫入，值本來就存在正確的 arm-oriented 槽位；
非芯云只是「用錯索引去讀、且只讀一半」。對齊後：

- 既有門檻值會出現在 `Arm1 Head1~4 / Arm2 Head1~4`
- 另外 8 格顯示 0（=不檢查），需**客戶補設定**
- ⚠️ 那半數 slot 可能藏著多年累積值（若客戶從未開過該頁按 Save）→ **上版時請客戶在該頁按一次 Clear 歸零重算**，否則一填門檻就立刻報警

需要同步修改的 4 處：

| # | 檔案 | 內容 |
|---|------|------|
| 1 | `cStartCondition.cpp` FormShow 非芯云分支 | 顯示改 `[arm][iPos]`、grid row 改 `1+(iArm*8+iPos)` |
| 2 | `cStartCondition.cpp` `ReadWriteStartCondition` | 回填 / 回讀同步改 |
| 3 | `csystem.cpp` `CheckContactOver()` 非芯云分支 | 改 `[arm][iPos]`，迴圈與 `ProcessHeadContactCount` 同界 |
| 4 | `cprod.cpp` `ProcessLastSetIni` / `ProcessLastSetIni_Count` | key 迴圈統一 |

改完後 **`CUSTOMER_CODE==CC_XINYUN` 的 4 處分支即可全部刪除**，回到單一模型。

---

## 3. SPEC 上限的兩條存取路徑

### 3.1 路徑 A — ProcessLastSetIni_Count（多值，每 Head 獨立）

讀取時機：`ReadLastSetIni()` 啟動載入  
寫入時機：`SaveLastSetIni()` / `WriteLastDataFile()`

| 條件 | 檔案 | Section | Key 格式 | 預設值 |
|------|------|---------|----------|--------|
| VTEST (`bVTESTFunction && bUseHeadContactCount`) | Recipe `HandlerCondition.Data` | `[O_Count]` | `O_14ContactSet{i}_{j}` | 6000 |
| SCC (D05=true) | `config.ini` | `[O_Count]` | `O_14ContactSet{i}_{j}` | 6000 |
| 其他 (D05=false) | `config.ini` | `[O_Count]` | `O_14ContactSet{i}_{j}` | 6000 |

x=1/2 的資料（O_15/O_16）一律存在 `config.ini`，不跟隨 Recipe。

> **注意**：此路徑不受 D05 影響。VTEST 判斷條件只看 `bVTESTFunction`，不看 D05。

### 3.2 路徑 B — ReadWriteStartCondition（單值，UI 設定用）

讀取時機：Start Condition 畫面 FormShow / ReadFile / FormClose  
寫入時機：sbHeadCondition1SaveClick

此函式用同一個 `szDir` 處理多種資料，但 **銦片 SPEC 與非銦片資料使用不同路徑變數**：

```
szDir          ← D05 決定（SocketCount.ini 或 Recipe），給非銦片資料用
szDirIndium    ← VTEST 時強制 Recipe，其他客戶等於 szDir，給銦片 SPEC 用
```

#### 銦片 SPEC（用 szDirIndium）

| 條件 | 檔案 | Key |
|------|------|-----|
| **VTEST**（不論 D05） | Recipe `HandlerCondition.Data` | `HeadContactSet[1~4]` |
| SCC（D05=true） | `SocketCount.ini` | `HeadContactSet[1~4]` |
| 其他（D05=false） | Recipe `HandlerCondition.Data` | `HeadContactSet[1~4]` |

#### 非銦片資料（用 szDir，跟 D05）

| 資料 | Key | D05=true | D05=false |
|------|-----|----------|-----------|
| 治具編號 | `Kit No 1/2/3` | SocketCount.ini | Recipe |
| Contact Warning | `HandContactWarningSet[0/1]` | SocketCount.ini | Recipe |
| 吸嘴計數 | `InArmSuckCnt`/`OutArmSuckCnt` | SocketCount.ini | Recipe |
| 吸嘴 SPEC | `O_20InOutArmLifeCntSet` | SocketCount.ini | Recipe |

> **設計原則**：VTEST 銦片一律跟 Recipe（by recipe 需求），D05 只影響非銦片資料。  
> 這確保 `ProcessLastSetIni_Count`（路徑 A）與 `ReadWriteStartCondition`（路徑 B）的銦片 SPEC 一定指向同一個檔案。

### 3.3 D05 與銦片路徑的關係（重要）

`cStartCondition.cpp` 中有 4 處 D05 判斷：

| # | 位置 | 函式 | 資料類型 | 銦片用 szDirIndium？ |
|---|------|------|---------|:-------------------:|
| 1 | L563 | FormClose | `HeadContactSet[1~4]` | ✅ 是 |
| 2 | L626 | sbSaveClick | `HandContactWarningSet` / `LastSet.ContactSet` | ❌ 用 szDir |
| 3 | L783 | ReadWriteStartCondition | 混合（銦片 + Kit No + Warning + 吸嘴） | ✅ 銦片用 szDirIndium，其餘用 szDir |
| 4 | L1160 | WritePickerCount | `InArmSuckCnt` / `OutArmSuckCnt` | ❌ 用 szDir |

> **為什麼不能單純讓 ProcessLastSetIni_Count 跟 D05？**  
> 因為 VTEST 的需求是銦片 by recipe。如果跟 D05，D05=true 時銦片就不跟 Recipe 了。  
> **為什麼不能單純在 ReadWriteStartCondition 用 VTEST 覆蓋整個 szDir？**  
> 因為同函式中還有非銦片資料（Kit No、Warning、吸嘴計數），這些應該跟 D05 走。

---

## 4. editContactCountAlarm UI 權限

`cStartCondition.cpp` FormShow：

```
進入條件（L317）：
  CC_KYEC_LEE || bHiSiliconFunction || bUseHeadContactCount

Enabled 門檻（L382）：
  if(CC_SCC || bVTESTFunction)
      Level >= iDefEngineerLevel (1)    ← 工程師可改
  else
      Level >= iDefHonPrecLevel (3)     ← 僅弘測可改
```

| 條件 | Visible | Enabled 門檻 |
|------|---------|--------------|
| `CC_KYEC_LEE` | ✅ | Level 3（弘測） |
| `bHiSiliconFunction`（KYEC 海思/AMD_M） | ✅ | Level 3（弘測） |
| `CC_SCC` | ✅ | Level 1（工程師） |
| `bVTESTFunction` | ✅ | Level 1（工程師） |
| 其他有 `bUseHeadContactCount` 的客戶 | ✅ | Level 3（弘測） |

---

## 5. 跑料中計數累加與告警

| 函式 | 位置 | 邏輯 |
|------|------|------|
| `ProcessHeadContactCount()` | `atester_ProcessCount.cpp` | 每次下壓對 `HeadContactCount[iA][Arm][j*2+i]` 累加（**by-arm，全客戶一致**，見 §2.5） |
| `CheckContactOver()` | `csystem.cpp` | 比較 `HeadContactCount` vs `ContactSet`，超限觸發告警。**芯云走 `[x][iArm][iPos]`（正確）、非芯云走 `[x][i][j]`（只檢查一半 head，見 §2.5.3）** |
| `WriteLastDataFile()` | `cprod.cpp` L1984 | 跑料中定期寫入 `HeadContactCount` 至 Recipe/config.ini |
| `SaveLastSetIni()` | `cprod.cpp` | 呼叫 `ProcessLastSetIni_Count(false)` 存檔 |

### 告警代碼

| Alarm Code | 說明 | 處理 |
|------------|------|------|
| WAR07460 | VTEST 銦片 x=0 接觸次數超過 SPEC | 自動歸零 + `SaveLastSetIni()` |
| WAR07461~07465 | 其他客戶 x=0~2 各 Arm 超過 SPEC | 不自動歸零 |

---

## 6. Lot Start 互動（VTEST）

Lot Start 時彈出「Change KIT」對話框（`uLotInfo.cpp`）：
- **YES**：保留計數繼續累加
- **NO**：歸零 `HeadContactCount[0][][]` 並呼叫 `SaveLastSetIni()`

---

## 7. 客戶別差異總表

| 客戶 | O12 | O13/O14 | 銦片儲存位置 | 非銦片資料位置 | SPEC UI 權限 |
|------|-----|---------|-------------|--------------|-------------|
| **VTEST** | `elConfig_byRecipe`（藍色） | 隱藏 | Recipe（**不受 D05 影響**） | 跟 D05 | 工程師 (Level 1) |
| **SCC** | 不進 InitConfigEdtList 區塊 | — | D05=true → `SocketCount.ini`；D05=false → Recipe | 跟 D05 | 工程師 (Level 1) |
| **KYEC 海思** | `elConfig` 強制開 | 顯示可編輯 | `config.ini` | 跟 D05 | 弘測 (Level 3) |
| **AMD_M** | `elConfig` 強制開 | 顯示可編輯 | `config.ini` | 跟 D05 | 弘測 (Level 3) |
| **其他** | `elConfig` 可編輯 | 顯示可編輯 | `config.ini` | 跟 D05 | 弘測 (Level 3) |

---

## 8. 修改歷史（2026/05 起）

| 日期 | 修改 | 檔案 | 說明 |
|------|------|------|------|
| 05/07 | 新增 VTEST 啟用 `bUseHeadContactCount` | `cprod.cpp` L3709 | VTEST 具備 ATC 時啟用銦片計數 |
| 05/08 | VTEST x=0 計數跟隨 Recipe | `cprod.cpp` L1986, L2470 | `ProcessLastSetIni_Count` / `WriteLastDataFile` |
| 05/15 | 註解 dead flag `bHeadContactCountByRecipe` | `CosFunction.cpp` ×3, `cprod.cpp` ×1 | 未被任何邏輯讀取 |
| 05/15 | editContactCountAlarm 進入條件加 `bUseHeadContactCount` | `cStartCondition.cpp` L317 | VTEST 可看到 UI |
| 05/15 | SCC/VTEST 工程師等級可修改 SPEC | `cStartCondition.cpp` L382 | `CC_SCC \|\| bVTESTFunction` → Level 1 |
| 05/17 | VTEST 銦片 SPEC 不受 D05 影響 | `cStartCondition.cpp` L563, L783, L833 | 新增 `szDirIndium` 變數，VTEST 強制走 Recipe；非銦片資料維持 `szDir` 跟 D05 |
| 06/26 | 芯云 by-arm 對齊（UI + 檢查端 + 存檔） | `cStartCondition.cpp`、`csystem.cpp`、`cprod.cpp` | `CUSTOMER_CODE==CC_XINYUN` 四處分支，改用 `[arm][iPos]`、grid row `1+(iArm*8+iPos)`，用滿 16 列 |
| 06/30 前後 | 芯云啟用 `bUseHeadContactCount` | `cprod.cpp` `SPILFunction()` 內 `CC_XINYUN` 區塊 | 905.15 尚無、906.5 已有。**此旗標一開，Contact alarm 語意從 per-site 翻轉成單一全域值**，是 20260728 客訴「sbHeadCondition1Save 設定失效」的觸發點 |
| 07/28 | 確立機構事實與索引語意 | （文件）§2.5 | 銦片在 Arm 上、一 head 一片；2×4 → 16 片銦片 vs 8 個 socket。確認 by-arm 為唯一正確模型 |

---

## 9. 已知風險與防護

### 9.1 D05 與 VTEST 銦片路徑分裂（已修復 05/17）

**問題**：`ProcessLastSetIni_Count`（跑料端）只看 `bVTESTFunction` 路由到 Recipe，  
但 `ReadWriteStartCondition`（UI 端）只看 D05 路由。如果 VTEST 機台 D05=true，  
UI 端寫到 `SocketCount.ini`，跑料端讀 Recipe → SPEC 改了沒用。

**修復**：在 UI 端引入 `szDirIndium` 變數，VTEST 強制走 Recipe，非銦片資料維持 `szDir`。

**防護原則**：
- VTEST 銦片路徑：一律 Recipe，不看 D05
- D05 影響範圍：Kit No、Contact Warning、吸嘴計數等非銦片資料
- 新增銦片相關讀寫時，必須使用 `szDirIndium` 而非 `szDir`

### 9.2 兩套索引模型並存（**未修復**，2026/07/28 確認）

**問題**：同一個 `ContactSet[3][2][16]` 陣列，計數端用 `[Arm][head]`、非芯云 UI/檢查端用 `[row][col]`。
詳見 §2.5.3。後果是**非芯云客戶有一半銦片從不檢查壽命**。

**影響客戶**：所有 `bUseHeadContactCount==true` 者 —— KYEC_LEE、KYEC_STM、JSCC_OS、SCC、
SJ_Semiconductor、VTEST、海思（`bHiSiliconFunction`）。芯云已於 20260626 對齊，不受影響。

**修復方向**：全部收斂到計數端的 by-arm 模型（§2.5.5），不可反向改計數端 —— 改它會讓所有客戶既有累積值錯位。

### 9.3 `bUseHeadContactCount` 會翻轉 Contact alarm 的設定語意（陷阱）

替某客戶打開 `CosFunction.bUseHeadContactCount` 時，除了開啟銦片計數，**還會同時翻轉
Contact alarm setting 的行為**：

| | `bUseHeadContactCount=false` | `=true` |
|---|---|---|
| `ReadWriteStartCondition()` 存檔分支 | 不進入 | 進入 → **grid Cells[1] 被 `iContactAlarmCount` 全域值覆蓋** |
| 使用者在 grid 逐 site 設的值 | 保留 | **被吃掉** |

**FormShow 的顯示判斷式（只看 `CC_KYEC_LEE`）與存檔判斷式（`CC_KYEC_LEE || bHiSiliconFunction || bUseHeadContactCount`）不一致**，
造成「畫面顯示 per-site、存檔寫全域值」→ 客戶感受為「設定存不進去」。

> 2026/06/30 前後替芯云開啟此旗標，即引發 20260728 客訴。
> **未來替任何客戶開啟 `bUseHeadContactCount` 前，務必同步檢查 `cStartCondition.cpp`
> FormShow 與 `ReadWriteStartCondition` 兩處判斷式是否對稱。**

### 9.4 grid 空白列會被回讀成 0（保留槽遭清零）

`ReadWriteStartCondition()` 的回讀迴圈固定掃 16 列，但 FormShow 只填 `iShtRow×iShtCol` 對應的列。
未填列 `atoi("")==0` → `ContactSet` / `HeadContactCount` 的保留槽**每次按 Save 都被清零**，
並由 `ProcessLastSetIni_Count` 寫回 ini → 永久遺失。

> ⚠️ **加空白 guard 前必讀**：`CheckContactOver()` 芯云分支（`csystem.cpp`）的 `for(iPos=0; iPos<8)`
> 註解明寫「iPos 4-7 ContactSet=0 會被 >0 擋掉」—— 它**依賴**這個清零行為。
> 若只加 guard 保留 `ContactSet[4..7]` 而不同步修 `CheckContactOver` 的迴圈邊界，
> 換設定後會對**不存在的 site** 跳「Arm1 Head5 Contact Over Count」擋機，
> 且該列在畫面上是空白、操作員無從清除。兩者必須同批修改。
