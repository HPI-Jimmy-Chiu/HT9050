---
name: ht9045-array-audit
description: 稽核 HT9045/HT9011UC 全部結構陣列的越界（buffer overflow）風險。適用於檢查陣列存取、新增陣列成員、或上線前陣列邊界檢查。涵蓋兩大領域：(1) LastSet（LAST_GENERAL_SET）持久化結構陣列；(2) cprod.h 內 Prod/DeviceForm/TrayForm/Temperature/TestIF/TestMode/BinSelect 等全域結構陣列。提供邊界常數表、索引來源追蹤法、稽核檢查清單與歷史缺陷案例。觸發關鍵字：array overflow、out-of-bounds、buffer overrun、array index、boundary check、陣列越界、LastSet、cprod.h、Prod、iSocketSensor、XStart、iDutOnOff、MAX_ARM_Row、MAX_AUTO_TRAY、eTrayCount。
---

<!-- AI(W906-BA-SKILL) 20260915：這支 skill 來自網頁同事 20260915 交付的
     D:\HT9050\HT9045_skills_20260915\skills\ht9045-array-audit，內容未改。
     ⚠ 資料通道以使用者 20260915 裁決為準：wb_serve HTTP 直讀 .Data +
       recipe.doc.put 寫。本 skill 若描述 JSON-only 或 C# Simulator 通道，
       那是同事那邊的舊架構，不是本專案的正式路徑。 -->

# HT9045 陣列邊界稽核技能

## 目的

在 HT9045/HT9011UC 程式碼中偵測並防止結構成員的陣列越界存取（buffer overflow / out-of-bounds）。
涵蓋持久化結構（LastSet）與執行期全域結構（cprod.h 的 Prod 等）兩大領域。

## 使用時機

- 新增或修改程式碼，且以**變數索引**存取任何結構陣列時
- 在 `LastSet.h` 或 `cprod.h` 新增陣列成員時
- 上線前程式碼審查（`pre-release-check` 流程）
- 追查不明資料毀損或 crash 問題時

## 領域與參考

| 領域 | 內容 | 參考 |
|------|------|------|
| **LastSet 持久化結構** | `LAST_GENERAL_SET` 陣列邊界表、binary 版面相容規則、歷史缺陷 | [references/lastset-array-audit.md](references/lastset-array-audit.md)；源碼對照 [references/source-map.md](references/source-map.md)；完整超集見全域 `pre-release-check` skill P7 |
| **cprod.h 全域結構** | Prod/DeviceForm/TrayForm/TestIF 等陣列稽核結果（V3.33.905.8） | [references/cprod-array-audit.md](references/cprod-array-audit.md) |

## 核心邊界常數（MachineType.h / cmydef.h）

| 常數 | 值 | | 常數 | 值 |
|------|----|----|------|----|
| `MAX_ARM_Row` | 2 | | `MAX_AUTO_TRAY` | 6 |
| `MAX_ARM_Col` | 4 | | `MAX_FIX_TRAY` | 6 |
| `MAX_Index_Row` | 2 | | `MAX_TRACK` | 9 |
| `MAX_Index_Col` | 8 | | `eTrayCount` | 33 |
| `MAX_SOCKET_ROW` | 4 | | `TEST_MAX_BIN` | 256 |
| `MAX_SOCKET_COL` | 8 | | `iSnSocketCnt` | 24 |

## 稽核檢查清單（通用）

1. **識別索引變數**：索引是常數還是變數？
2. **追蹤範圍**：索引可能取到哪些值？沿賦值鏈往上追（迴圈邊界、enum 來源、tester/bin/tray 回傳值、滑鼠/grid 事件、INI/DLL/外部回傳）。
3. **檢查保護條件**：陣列存取前是否已有邊界檢查？
4. **留意 off-by-one**：迴圈邊界附近的 `j+1`、`i+2`、`X-1`；**亦須比對「迴圈硬編碼上限 N vs 被索引陣列宣告大小 M」**，N>M 即越界（本類型易漏，見 §F 歷史缺陷）。
5. **留意邏輯錯誤**：保護條件中的 `||` 與 `&&` 方向是否正確。
6. **檢查負索引**：來自外部來源（tester、DLL、滑鼠事件、INI）的值可能為 `-1`。
7. **檢查間接索引**：`array1[array2[x]]`，兩層都要做邊界檢查。
8. **維度不對稱**：Arm 維 `[2][4]` 小於 Socket 維 `[4][8]`，新增程式碼若以 socket row/col 直接索引 arm 維陣列將越界——日後高風險點。
9. **block-memory size 比對**：`memcpy / memmove / memset / ZeroMemory / strcpy / strncpy / strcat / sprintf` 的 size 引數必須 ≤ 目標緩衝宣告大小。常見陷阱：(a) 以來源長度（`Str.Length()`）為 `strncpy` 的 N，未改為 `sizeof(dst)-1`；(b) `strncpy` 複製恰填滿時未補結尾 `\0`；(c) 跨結構 `memcpy` 兩端型別/大小不一致（如 Prod`[2][4]` vs Tech`[2][8]`）。詳見 [references/cprod-array-audit.md §G](references/cprod-array-audit.md)。

### 修正時的語意保持原則（關鍵）

**新增邊界保護時，不得改變原始程式碼的語意。**

- **只加保護，不改邏輯**：新增 `if(idx>=0 && idx<SIZE)` guard 或調整迴圈邊界上限，不得改動被保護的運算式本身。
- **保持原始行為一致**：正常路徑執行結果必須與修改前完全相同。
- **禁止「順便」重構**：不得更動變數命名、合併/拆分迴圈、移動程式碼行或添加無關功能。
- **註解標示**：修改行旁加上 `//Steven YYYYMMDD : add boundary guard` 行內註解。
- **LastSet 例外**：`LAST_GENERAL_SET` 為 binary 持久化，**不得**變更既有欄位大小/順序，新欄位只能追加在結構最後方。

## 快速稽核 grep 模式

```powershell
$base = "d:\HT9045\HT9011UC_Code_Vx.xx.xxx.x_YYYYMMDD"

# LastSet 變數索引存取
Get-ChildItem $base -Filter *.cpp | Select-String -Pattern "LastSet\.\w+\[[^0-9`\"']"

# cprod.h 全域結構變數索引存取（範例）
Get-ChildItem $base -Filter *.cpp | Select-String -Pattern "Prod\.\w+\[[^0-9`\"']"
Get-ChildItem $base -Filter *.cpp | Select-String -Pattern "SThreadPara\.iSocketSensor\["
```

## 稽核狀態

| 領域 | 最近稽核版本 | 結果 |
|------|--------------|------|
| LastSet | V3.33.898（2026-03-16） | 5 項缺陷已修；`lSCKARTBinCT` `[20]→[256]` |
| cprod.h 宣告/設定值 | V3.33.905.8（2026-06-16） | 0 Critical / 0 High / 2 Low |
| 迴圈邊界 vs 陣列宣告 | V3.33.905.8（2026-06-16） | 2 已修（F-1 iAutoCassette、F-2 ZBasePickerAlignmentPos）；1 Low 待確認（F-3 iInSHSen9） |
| block-memory size | V3.33.905.8（2026-06-16） | 0 Critical / 0 High / 3 Medium 待修（G-1 Command.cpp、G-2 cShowBinSelect.cpp、G-3 BarCode.cpp）；2 Low |
