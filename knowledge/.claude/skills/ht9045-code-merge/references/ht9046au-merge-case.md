# HT-9046AU 合併實戰案例（2026-03-23）

## 版本資訊

| 項目 | 值 |
|------|-----|
| 來源 | `HT9011UC_Code_V3.33.897.1_20260314_RogerYang_Add for HT-9046AU` |
| 目標 | `HT9011UC_Code_V3.33.899.0_20260323` |
| Base | `HT9011UC_Code_V3.33.899.0_20260323_backup_pre_9046AU` |
| SVN 共同基準 | Rev 897 |

## 最終合併結果

| 類別 | 數量 |
|------|------|
| 直接複製 | 177 |
| 新增檔案 | 4 |
| 無需變更 | 637 |
| 跳過 | 4 |
| 衝突 | 0 |
| 錯誤 | 0 |
| 掃描總檔數 | 818 |

## 新增檔案

- `Motor\myEthercatmotor.cpp`
- `Motor\myEthercatmotor.h`
- `asortarm.cpp`
- `asortarm.h`

## 關鍵實戰經驗

### 1. 有 backup 時，可用 backup 當 merge base 做快速分類

當目標版本有「合併前備份」且確認 target 初始狀態與 backup 一致時，可直接以 backup 作為 base：

- `source != base && target == base` → 直接複製
- `source != base && target != base` → 重疊，需人工或 3-way merge
- `base 不存在 && target 不存在` → 新增檔案

此方法可避免大量逐檔 `svn cat -r BASE`，速度更快，也較不受 SVN CLI/terminal 輸出問題影響。

### 2. BCB6 實際編譯錯誤：E2148 default argument redeclared

此次合併後，`Motor\myGALILmotor.h` 與 `Motor\mymotor.h` 對下列函式同時提供 default argument，造成 BCB6 E2148：

- `StopAllMotor(bool bIndexCanStop=true)`
- `GetGalilErrString(long RC, AnsiString FunctionName="")`

**修正方式**：保留一處 default argument，另一處只留函式宣告。

### 3. BCB6 實際 linker 錯誤：新 .cpp 未加入 .bpr

新增 `asortarm.cpp` 與 `Motor\myEthercatmotor.cpp` 後，若未同步更新 `HT9045.bpr`，會出現：

- `Unresolved external 'SetSortArmHome()'`
- `Unresolved external 'DoSortArm()'`
- `Unresolved external 'MoveSortArmXYToSortShtWait()'`
- `Unresolved external 'MoveSortArmToAutoSafe()'`
- `Unresolved external '_iSortArmPlaceOrder'`
- `Unresolved external 'TMyEtherCatMotor::~TMyEtherCatMotor()'`
- `Unresolved external '__fastcall TMyEtherCatMotor::TMyEtherCatMotor(int)'`

**修正方式**：
1. 在 `OBJFILES` 加入 `..\Obj\myEthercatmotor.obj ..\Obj\asortarm.obj`
2. 在 `FILE` 區段加入對應 `.cpp` 條目
3. 重新執行 `bpr2mak`
4. 再做 `make -f HT9045.mak -B`

### 4. `MAKE0000.@@@` 錯誤通常是工作目錄問題

若 linker 出現：

```text
Fatal: Unable to open file 'MAKE0000.@@@'
```

通常不是原始碼問題，而是：

- `make` 不在專案目錄執行
- 透過外層 shell/batch 重導時，工作目錄沒有正確切到 `.bpr/.mak` 所在目錄

### 5. Build 完成後需保留兩個資訊

- `EXIT_CODE`
- `Warning Wxxxx` 統計數量

此次結果：

| 項目 | 結果 |
|------|------|
| Exit Code | 0 |
| Warnings | 113 |
| EXE | `D:\HT9045\EXE\HT9045.exe` |

## Pre-release 檢查摘要

本次針對以下關鍵檔案掃描 P1/P6 模式，結果皆為 0 風險：

- `asortarm.cpp`
- `asortarm.h`
- `Motor\myEthercatmotor.cpp`
- `Motor\myEthercatmotor.h`
- `cinitial.cpp`
- `csystem.cpp`
- `uhome.cpp`
- `acarry.cpp`

## 建議何時引用本案例

適用於：

- 有 backup 可當 base 的同版號整合
- 新增機構模組 / 馬達模組 / 新 `.cpp` 檔案
- BCB6 linker 出現 unresolved external，但來源碼明明已存在
- `MAKE0000.@@@` 類型工作目錄錯誤排查

---

## 6. 直接複製造成 ChangeToFloatNonPcnt 保護回歸（後補）

### 問題

本次 177 個檔案為「直接複製」，因 Roger 的版本基於較早的 revision，當中的裸除法已被 Steven 在後續 commit 替換為 `ChangeToFloatNonPcnt` 安全保護。直接複製後，這些保護全數回歸為裸除法。

### 影響規模

- **243 處** `ChangeToFloatNonPcnt` 被還原為裸除法
- **47 個檔案** 受影響
- 主要涉及 Motor、AutoTeach、SECSGEM、SortingBinTray、cinitial、main 等核心模組

### 偵測方式

```powershell
cd "<TargetPath>"
svn diff . | Select-String "ChangeToFloatNonPcnt" -Context 2,2
```

若 diff 中出現 `-` 行有 `ChangeToFloatNonPcnt` 而 `+` 行為裸除法，即為此問題。

### 修復方式

使用自動還原腳本 `scripts/restore_changeToFloat.py`：
1. 解析 `svn diff` 的 unified diff 輸出
2. 匹配被移除的 `ChangeToFloatNonPcnt` 行與對應的裸除法行
3. 在工作複本中精確替換
4. 產生 `restore_changeToFloat.log` 供手動複核

本次結果：238 處自動還原 + 5 處手動修復 = 243 處全部修復。

### 教訓

**直接複製不等於無風險**。當 source revision < target revision 時，必須在 **Step 2 重疊偵測**階段執行跨版本比對（`svn cat -r <source_rev>`），將有 committed 差異的檔案改為 3-way merge 處理。前置比對是根本防線，而非事後補救。合併完成後需驗收 `ChangeToFloatNonPcnt` 基線計數（參見 Step 2 基線統計節）。
