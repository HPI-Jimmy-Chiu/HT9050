---
name: debug-knowledge-base
description: >-
  全域 Bug 修復知識庫。跨專案共享排錯經驗（HT9045 / GPIB9045 / RS232Standard / MDB Updater）。
  涵蓋：BCB6 編譯錯誤、機台流程異常（InArm/OutArm/Shuttle/Index）、GPIB 通訊故障、
  馬達定位異常、IO 控制問題、ATC 溫控故障、2DID/OCR 掃碼誤判、Yield 計數異常、
  QA Mode 邏輯錯誤、Auto Clean 碰撞、陣列越界、記憶體損毀、Big5 編碼亂碼、
  3-way merge 遺失內容、GetTickCount 歧義。
  觸發關鍵字：debug, bug fix, troubleshoot, 排錯, 除錯, 異常, error, JAM, WAR,
  Alarm, crash, hang, timeout, 當機, 碰撞, collision, 亂碼, garbled,
  ambiguity, E2015, E2141, Fatal Error, ibwrt fail, Wait TACS,
  Position error, 掉料, IC Fall Down, 黏貨, Sticky, Pick Error,
  array overflow, divide by zero, race condition, 時序競爭
---

# Bug 修復知識庫（Debug Knowledge Base）

跨專案共享的排錯經驗索引。每條記錄含：錯誤特徵 → 根因 → 修復方式 → 預防措施。

---

## 使用方式

1. 遇到錯誤時，先比對下方「錯誤分類索引」
2. 找到匹配項目後，查閱對應的 `references/` 詳細文件
3. 若為新 Bug，修復後依模板新增記錄

---

## 錯誤分類索引

### A. BCB6 編譯錯誤

| ID | 錯誤特徵 | 根因 | 修法 | 詳細 |
|----|---------|------|------|------|
| A01 | `E2015 Ambiguity between GetTickCount` | Indy header 引入 `Idglobal` namespace | 呼叫點加 `::GetTickCount()` | [bcb6-compile-errors.md](references/bcb6-compile-errors.md#a01) |
| A02 | `E2141 Declaration syntax error` | 3-way merge difflib 遺失內容，函式體插入錯位 | 改用「複製 target + 套用 source delta」策略 | [bcb6-compile-errors.md](references/bcb6-compile-errors.md#a02) |
| A03 | `Unresolved external` / `Undefined symbol` | 合併遺漏 .cpp 或 .h、mak 檔未同步 | 確認 bpr2mak 重產 .mak，比對來源 | [bcb6-compile-errors.md](references/bcb6-compile-errors.md#a03) |

### B. 機台流程異常

| ID | 錯誤特徵 | 根因 | 修法 | 詳細 |
|----|---------|------|------|------|
| B01 | Auto Clean InArm Z 軸碰撞 Shuttle Sensor | site 關閉時未跳過處理，記憶體越界 | 補 `bUse8Picker==false` 跳過邏輯 | [machine-flow-bugs.md](references/machine-flow-bugs.md#b01) |
| B02 | OutArm Picker 碰 Shuttle（Shaft 變形） | SHT Y 安全窗口僅 3mm，移動保護不足 | 擴大 ±70mm 安全窗口 + 互鎖強化 | [machine-flow-bugs.md](references/machine-flow-bugs.md#b02) |
| B03 | Index Position error Z1UpZ2Down1 | 保護判斷與 Galil 向量插補時序競爭 | 待修：需調整保護判斷時序 | [machine-flow-bugs.md](references/machine-flow-bugs.md#b03) |
| B04 | OutArm Z-Axis Offset Reset to 0 | `cOffSet.cpp ReadFile()` 在 Save 後誤歸零 | 修正 ReadFile 邏輯，保留 Z offset | [machine-flow-bugs.md](references/machine-flow-bugs.md#b04) |
| B05 | QA Mode 計數不準、停測失效 | 計數邏輯未考慮多盤情境 | 修正 Check_QA_ModeCount 邏輯 | [machine-flow-bugs.md](references/machine-flow-bugs.md#b05) |
| B06 | Auto Clean Count 不增加 | Clean out alarm 重置計數 | 分離 alarm 重置與 clean 計數 | [machine-flow-bugs.md](references/machine-flow-bugs.md#b06) |
| B08 | 1x2/2x2 大Pitch+4×8雙盤只放半盤→WAR0150掛機 | 換盤邏輯與 ix 同層，ix 與盤號鎖死同相位 | 換盤搬進 `if(ix>spacX)` 內，掃完全欄才換盤 | [machine-flow-bugs.md](references/machine-flow-bugs.md#b08) |
| B09 | 單臂模式+D63馬達尋相→Index Home 卡機(Y1停15864) | 被關臂 Middle 歸零但 Middle_Home 保留教導值，D63 尋相驅到教導值與 production 0 不一致 | 關掉的軸略過 D63 尋相（uhome.cpp case 3050/3250 判 `iShuttleMode==1 && TestY*_Middle==0`） | [machine-flow-bugs.md](references/machine-flow-bugs.md#b09) |

### C. GPIB / RS232 通訊故障

| ID | 錯誤特徵 | 根因 | 修法 | 詳細 |
|----|---------|------|------|------|
| C01 | `Wait TACS` → `ibwrt fail` | Tester 未及時切 Talker 角色給 Handler | 加大 iMyGpibWriteWaitMS / Retry | [comm-bugs.md](references/comm-bugs.md#c01) |
| C05 | 93K ART FT lot-end tester 等 `SRQ:0xC0` 不來、Handler ART Alarm（彈「請結批報表」）| `bAutoRetestGPIBmode` 於 lot 中變 false（與 `iTesterType==1` 不一致），`DoART_AfterCleanOut()` 走 else 分支漏送 `SetLotState(8)` | 以 `iTesterType` 單一真值重算 `bAutoRetestGPIBmode` / 分支 #3 守門 | [comm-bugs.md](references/comm-bugs.md#c05) |
| C06 | 新機 / 清記憶後 SECS/GEM·TCP ART 全程收不到 `SRQ:0xC0` | `DoInitialStart()` 空 if-branch 不送 `RunDummy` → `bDummyART` 沿用 GPIB 啟動 ini 未歸零 | empty branch 無條件送 `RunDummy`（D1）+ 既有 `HasICUnderMachine/HasAnyICInMachine` 開批防呆（D3）| [comm-bugs.md](references/comm-bugs.md#c06) |

### D. 2DID / OCR 掃碼異常

| ID | 錯誤特徵 | 根因 | 修法 | 詳細 |
|----|---------|------|------|------|
| D01 | JAM0460/0461 持續報警（OCR 模式） | Contact Test/Auto Height 前未清錯誤旗標 | 啟動掃碼前清除 bCheckCodeError | [barcode-bugs.md](references/barcode-bugs.md#d01) |
| D02 | Shuttle 1 重複碼誤判 JAM0460 | OCR row 反轉映射未套用於排除條件 | 改用映射後邏輯 Row 索引 | [barcode-bugs.md](references/barcode-bugs.md#d02) |

### E. 編碼與合併問題

| ID | 錯誤特徵 | 根因 | 修法 | 詳細 |
|----|---------|------|------|------|
| E01 | Big5 檔案中文變亂碼（`??閬?`） | replace_string_in_file 以 UTF-8 寫入 Big5 檔 | 用 binary 模式從來源複製 bytes | [encoding-merge-bugs.md](references/encoding-merge-bugs.md#e01) |
| E02 | 3-way merge 靜默遺失內容 | difflib 在 source≈base 時砍掉 target 新增 | 改用「複製 target + 套用 source delta」 | [encoding-merge-bugs.md](references/encoding-merge-bugs.md#e02) |

### F. 陣列越界 / 記憶體安全

| ID | 錯誤特徵 | 根因 | 修法 | 詳細 |
|----|---------|------|------|------|
| F01 | LastSet 陣列越界導致隨機異常 | 新增成員時未更新 MAX 常數或迴圈邊界 | 用 `sizeof(arr)/sizeof(arr[0])` | [memory-safety-bugs.md](references/memory-safety-bugs.md#f01) |
| F02 | 除以零崩潰 | 除法前未檢查分母 | `SafeDiv()` 或 `if(divisor!=0)` | [memory-safety-bugs.md](references/memory-safety-bugs.md#f02) |

---

## 排錯 SOP（通用）

```
Step 1: 確認 MachineType.h 的 SOFT_SIMULTE 是否關閉（release 必須 = 0）
Step 2: 清理 Obj 目錄（del /Q *.obj *.tds），全重建
Step 3: 檢查 BCB6 build log 的 error / warning 數量
Step 4: 比對上一個已知正常版本的差異（svn diff）
Step 5: 在 State Record 中查看 Task_ListWithTime.csv 確認卡住的 thread
Step 6: 查閱本知識庫的錯誤分類索引
Step 7: 若為新 Bug → 修復後新增記錄到 references/
```

---

## 新增 Bug 記錄模板

在 `references/` 對應檔案中，以下列格式新增：

```markdown
### [ID] 簡述
- **問題編號**：#P/R YYMMDD-CUSTOMER-MC-NN
- **影響版本**：Vx.xx.xxx.x
- **症狀**：（一句話描述現象）
- **根因**：（技術原因）
- **修法**：（程式碼層面的具體修改）
- **預防**：（避免再犯的措施）
- **案例**：（客戶、機台序號、日期）
- **提案文件**：（連結到 docs/customers/ 下的報告）
```

---

## 客戶問題索引

完整客戶問題追蹤表：[docs/customers/issue-index.md](../../../docs/customers/issue-index.md)
