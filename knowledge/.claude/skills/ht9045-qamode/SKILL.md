---
name: ht9045-qamode
description: >
  HT9045 QA Mode（品保抽測模式）知識庫。涵蓋核心三階段流程、Run Type 4 種行為、
  iQAModeBin Bin Routing、已知陷阱與修正歷程。
  UI 欄位詳細對照 → references/qamode-ui-fields.md
  InArm 變體/SiteMap/程式碼座標 → references/qamode-inarm-variants.md
  階段 C 深入分析 → references/qamode-stage-c-analysis.md
  觸發關鍵字：QA Mode, QA Sampling, iQAModeLoaderCT, iQAModeCount, iQAModeRunType,
  iQAModeBin, bQAModeQuickCleanOut, bQAModeFinishCleanOut, rsmQAMode,
  Check_QA_ModeCount, QABackupStatus, Untest Bin, QA 抽測, QA 停測.
applyTo: "**/QAMode.cpp, **/QAMode.h, **/ainarm2.cpp, **/ainarm9045*.cpp, **/csystem.cpp, **/cinitial.cpp, **/main.cpp"
---

<!-- AI(W906-BA-SKILL) 20260915：這支 skill 來自網頁同事 20260915 交付的
     D:\HT9050\HT9045_skills_20260915\skills\ht9045-qamode，內容未改。
     ⚠ 資料通道以使用者 20260915 裁決為準：wb_serve HTTP 直讀 .Data +
       recipe.doc.put 寫。本 skill 若描述 JSON-only 或 C# Simulator 通道，
       那是同事那邊的舊架構，不是本專案的正式路徑。 -->

# HT9045 QA Mode 知識庫

> **詳細參考檔**（僅在需要時載入）：
> - `references/qamode-ui-fields.md` — UI 欄位 / Tester.Data / 變數層級 / 客戶鎖死
> - `references/qamode-inarm-variants.md` — InArm 變體 / SiteMap / ART 互動 / 程式碼座標 / State Record 判讀
> - `references/qamode-stage-c-analysis.md` — 階段 C Fix 模式深入分析

## 1. 模式定位

QA Mode（品保抽測模式）：每個 Lot Start 時前 N 顆 IC 走正常測試流程，
做完後 Handler 自動切換到 Off_Line（或其他模式），剩餘 IC 直接送 Untest Bin，
用於產線品保抽測。**Run Start Mode = `rsmQAMode`** 時生效。

**關鍵全域變數**：`IniConfig.bQAMode`（全域開關）、`Prod.iQAModeCount`（量產用閾值）、
`Prod.iQAModeRunType`（做完後動作）、`Prod.iQAModeBin`（Untest Bin index）。

## 2. iQAModeRunType（做完後動作）

| 值 | UI 文字 | 行為 |
|----|---------|------|
| 0 | Clean out every devices without test automatically | 自動 CleanOut → Off_Line，殘餘 IC 不測試 |
| 1 | One cycle and alarm, then normal production | Maxim 版本（按 Unload 數計） |
| 2 | Clean out every devices as setted bin automatically | CleanOut，殘餘 IC 設為指定 Bin |
| 3 | Not counting Device. Need to end manually | 不計數，TrayEnd 重設 |

## 3. 核心三階段判斷（`Check_QA_ModeCount()` @ ainarm2.cpp:155）

```
階段 C（接近）→ 階段 A（到量）→ 階段 B（Off_Line）
```

| 階段 | 條件 | 動作 |
|------|------|------|
| **C** | `Count-20 < CT < Count` | `bVariModeFIX=true`（切單顆吸取） |
| **A** | `CT >= Count`（904.2 改為 `>=`） | `bQAModeQuickCleanOut=true`，啟動 One Cycle |
| **B** | `CT > Count && bQAModeQuickCleanOut` | `ModifyTester(IniConfig.iBackUpTesterMode)`，`bQAModeFinishCleanOut=true` |

> ⚠️ **V3.33.904.5 修正**：Stage B **必須**保持 `ModifyTester(OFF_LINE)` 以維持 QA 停測守衛（main.cpp:886/1267/16684）。
> Tester Mode 的還原改在 `SetRunStartMode()` 離開 QA Mode 時執行，此時 CleanOut 已結束、守衛已完成任務。
> V904.4 曾在此處改為 `ModifyTester(iBackUpTesterMode)` 但造成 regression（停測失效），已於 V904.5 撤銷。詳見 §6.3 & §6.4。

## 4. Bin Routing 機制（main.cpp:16677）

QA CleanOut 後殘餘 IC 的 Bin 決定邏輯：

```cpp
if(rsmQAMode &&
   (iQAModeRunType==0 || iQAModeRunType==2) &&   // 904.3: 擴展 RunType 0
   OFF_LINE && bQAModeFinishCleanOut)
{
    iBin = Prod.iQAModeBin;       // 強制 Untest Bin
}
else
{
    iBin = HGpib2Handler->Result; // 走 GPIB 結果
}
```

搭配 `iTestRunMode` 的對照表（`Binasgn.Data` vs `BinasgnOff-Line.Data`）
決定最終放料位置。

### iQAModeBin 生效條件

| Run Type | iQAModeBin 套用 | iTestRunMode | 放料對照表 |
|----------|-----------------|-------------|-----------|
| **0** | ✓（904.3+） | FT（保持） | Binasgn.Data |
| 1 | ✗ | — | — |
| **2** | ✓（原始設計） | FT（保持） | Binasgn.Data |
| 3 | ✗ | — | — |

### OffT Guard（main.cpp:889 & 1267）

```cpp
if(CosFunction.bOffLineBin && LastSet.iTester==OFF_LINE &&
   !(rsmQAMode && (RunType==0||RunType==2) && bQAModeFinishCleanOut))
{
    iTestRunMode=OffT;  // 只有非 QA CleanOut 時才切 OffT
}
```

## 5. 已知陷阱（DO NOT MISS）

### 5.1 多 Site 模式下 `==` 判斷會被跳過（已修正 904.2）
多 Site 模式下 counter 可能跳過精確值 → 階段 A 不觸發 → QA 停測失效。
**修法**：`==` → `>=` + 狀態守衛。

### 5.2 `iQAModeBin` 是 0-based ComboBox index
ATK 預設 `Untest Bin=1` 意指 Bin index 1（第 2 個 Bin）。

### 5.3 ART + QA 混用注意
`iQAModeLoaderCT` 每 Lot 歸零（Run Type 3 除外），
但 SCK ART 的 `iSCKARTInputCT` 跨 Lot 累計。

---

## 6. 修正歷程

### 6.1 V3.33.904.2 — QA Mode 32-Site Stop Fix（2026-05-11）

**問題**：32-Site `==` 被跳過，QA 停測失效。

| 檔案 | 修改 |
|------|------|
| `ainarm2.cpp:159` | `==` → `>=` + 狀態守衛 |
| `ainarm2.cpp:185` | 階段 C 範圍改動態 `iMaxRow*iMaxCol*2` |
| `ainarm9045.cpp:7080` | `bNeedOneCycle()` 同步 `>=` |

### 6.2 V3.33.904.3 — QA Mode CleanOut Bin Routing Fix（2026-05-14）

**問題**：QA Stop 成功觸發後，CleanOut 殘餘 IC 被誤送到 Fix2（雙重流程逃脫）。

**逃脫點 1**：`main.cpp:889/1267` — Off_Line 切 `iTestRunMode=OffT` → 查 BinasgnOff-Line.Data（全 Fix2）  
**逃脫點 2**：`main.cpp:16677` — `iQAModeBin` 只在 RunType==2 套用，RunType==0 走 GPIB SOFTBIN

| # | 檔案 | 修改 |
|---|------|------|
| A1 | `main.cpp:16677` | RunType 條件擴展：`==2` → `==0 \|\| ==2` |
| A2a | `main.cpp:889` | QA CleanOut 期間不切 OffT（guard） |
| A2b | `main.cpp:1267` | 同 A2a |

**修正後**：QA CleanOut 保持 FT bin table → `iQAModeBin`=1 → Binasgn.Data Bin1 → Auto1 ✓

---

### 6.3 V3.33.904.4 — QA Mode Tester Mode Restore Fix（2026-05-19）

**問題**（P260518-ATK-H9-02，ATK / Matthew Han）：  
QA Mode 完成後（Run Type 0），Tester 被切到 Off_Line，應維持 On_Line。

**根本原因**：  
- Lot Start 時已將原始 Tester Mode 備份到 `IniConfig.iBackUpTesterMode`  
- 但 Stage B 完成路徑（Run Type 0/2）寫死呼叫 `ModifyTester(OFF_LINE)`，未使用備份值  
- Run Type 1（Maxim）早已正確使用 `iBackUpTesterMode`，Run Type 0/2 遺漏

| # | 檔案 | 修改 |
|---|------|------|
| C1 | `ainarm2.cpp` | `ModifyTester(OFF_LINE)` → `ModifyTester(IniConfig.iBackUpTesterMode)`（主路徑） |
| C2 | `csystem.cpp` | 同上，位於 `DoCleanOutFinishCheck()` 的 backup path |

**修正後行為**：
- Lot Start = On_Line → QA finish → 還原 On_Line ✓  
- Lot Start = Off_Line → QA finish → 還原 Off_Line ✓（向下相容）  
- Run Type 1（Maxim）：無異動 ✓

> **注意**：C2（csystem.cpp）為防禦性路徑，條件 `bQAModeQuickCleanOut==true && bQAModeFinishCleanOut==false`，正常情況由 ainarm2 的 C1 先行，C2 僅在 C1 未觸發時才生效。

> ⚠️ **V904.4 已被 V904.5 撤銷**：C1/C2 的修改導致 `LastSet.iTester` 被提前還原為 `ON_LINE`，破壞 main.cpp:886/1267/16684 三處守衛，QA 停測完全失效。詳見 §6.4。

---

### 6.4 V3.33.904.5 — QA Mode V904.4 Regression Fix（2026-05-21）

**問題**（P260521-ATK-H9-01，ATK / Matthew Han）：  
V904.4 修正後 QA Mode 不在設定數量（125 ea）停止，實際測試 200+ 顆。

**Regression 根因**：  
V904.4 將 `ModifyTester(OFF_LINE)` 改為 `ModifyTester(IniConfig.iBackUpTesterMode)`，
當 backup 為 `ON_LINE` 時，`LastSet.iTester` 被立即還原為 `ON_LINE`。
但 `LastSet.iTester == OFF_LINE` 在 QA Mode 中扮演**雙重角色**：

1. **顯示狀態** — UI 上的 Online/Offline 指示
2. **內部停測旗標** — 三處守衛依賴此值：
   - `main.cpp:886`（OffT Guard 1）
   - `main.cpp:1267`（OffT Guard 2）
   - `main.cpp:16684`（Bin Routing — 將殘餘 IC 送 Untest Bin）

提前還原 → 三處守衛全部失效 → QA 停測被靜默跳過。

**V904.5 修正方案（3 步）**：

| # | 檔案 | 修改 |
|---|------|------|
| D1 | `ainarm2.cpp:174` | **REVERT** → 還原 `ModifyTester(OFF_LINE)`（6 行→2 行） |
| D2 | `csystem.cpp:14948` | **REVERT** → 還原 `ModifyTester(OFF_LINE)`（6 行→2 行） |
| D3 | `main.cpp:538` | **NEW** — 在 `SetRunStartMode()` 新增 20 行 hook |

**D3 hook 邏輯**：離開 QA Mode 時（`rsmQAMode → 其他模式`），且 CleanOut 已完全結束（`iCleanOut==0`），
才呼叫 `ModifyTester(IniConfig.iBackUpTesterMode)` 還原 Tester Mode。

```cpp
if(bQAModeFinishCleanOut==true &&
   LastSet.iRunStartMode==rsmQAMode &&
   Mode!=rsmQAMode && Mode!=rsmNull &&
   iCleanOut==0)
{
    IniConfig.bQAModeFirstIn = true;
    fMain->ModifyTester(IniConfig.iBackUpTesterMode);
    // MES log + restore InArm/AutoFeed
    bQAModeFinishCleanOut = false;
    bQAModeQuickCleanOut  = false;
}
```

**安全條件**：
- `bQAModeFinishCleanOut==true` → 確保 QA CleanOut 已啟動
- `iCleanOut==0` → 確保 CleanOut 已完全結束（保護 Bin Routing 守衛）
- `Mode!=rsmNull` → 避免無意義的 null 切換觸發還原

**修正後行為**：
- Lot Start = On_Line → QA 測 125 ea → 停測 ✓ → CleanOut ✓ → 切換模式時還原 On_Line ✓
- Lot Start = Off_Line → QA 測 125 ea → 停測 ✓ → CleanOut ✓ → 維持 Off_Line ✓
- Run Type 1/3：無異動 ✓

> **關鍵教訓**：`LastSet.iTester == OFF_LINE` 在 QA Mode CleanOut 期間是**不可觸碰的停測旗標**，
> 任何提前還原都會破壞三處守衛。Tester Mode 還原必須延遲到 `SetRunStartMode()` 的模式切換點。
