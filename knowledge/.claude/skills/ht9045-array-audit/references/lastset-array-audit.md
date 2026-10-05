# LastSet 陣列邊界稽核（LAST_GENERAL_SET）

> 本文件由原 `ht9045-lastset-array-audit/SKILL.md` 內容降為 reference（2026-06-16）。
> **完整超集版本**（含結構定義位置、Load/Save 函數、高風險存取位置、版本異動、語意保持原則）
> 見 skill：`D:\HT9045\.claude\skills\pre-release-check\references\lastset-array-audit.md`（P7）。
> HT9045 工作區源碼對照另見同目錄 [source-map.md](source-map.md)。

## 目的

在 HT9045 程式碼中偵測並防止 `LAST_GENERAL_SET`（LastSet）結構成員的陣列越界存取。此結構以 binary blob 方式持久化到磁碟，任何記憶體破壞都可能無聲地污染已儲存狀態。

## Binary 版面相容規則（關鍵）

`LAST_GENERAL_SET` 以 raw binary 格式儲存/載入，因此必須遵守：

- **不得**變更結構內任何既有欄位的位元長度/大小。
- **不得**調整既有成員順序、刪除既有成員或改變既有成員大小。
- 若需新增欄位，**只能追加在結構最後方**。

違反此規則會造成 binary 版面不相容，並可能導致持久化資料損毀。

## 使用時機

- 新增或修改程式碼，且以**變數索引**存取任何 LastSet 陣列時
- 在 `LastSet.h` 新增陣列成員時
- 上線前程式碼審查（`pre-release-check` 流程）
- 追查不明資料毀損或 crash 問題時

## 主要陣列與邊界

| 陣列 | 宣告 | 安全索引範圍 |
|-------|------------|------------------|
| `BinCT` | `unsigned int [4][256]` | `[0-3][0-255]` |
| `BinCT_ART` | `unsigned int [4][256]` | `[0-3][0-255]` |
| `BinCT_PTI` | `long [4][256]` | `[0-3][0-255]` |
| `iBinData32` | `int [4][260]` | `[0-3][0-259]` |
| `iBinData32_ART` | `int [4][260]` | `[0-3][0-259]` |
| `iBinCTForAlways` | `int [4][20]` | `[0-3][0-19]` |
| `iSiteBinCTForAlways` | `unsigned int [4][8][256]` | `[0-3][0-7][0-255]` |
| `iSiteTotalCTForAlways` | `unsigned int [4][8]` | `[0-3][0-7]` |
| `lSCKARTBinCT` | `long [256]` | `[0-255]` (was `[20]` before V3.33.898) |
| `bUseTestSocket` | `bool [2][4][8]` | `[0-1][0-3][0-7]` |
| `bUseTestSocketEE` | `bool [2][4][8]` | `[0-1][0-3][0-7]` |
| `bAutoSiteOffSocket` | `bool [2][4][8]` | `[0-1][0-3][0-7]` |
| `iCloseSiteByLowYield` | `int [2][4][8]` | `[0-1][0-3][0-7]` |
| `iSocketContactCount` | `int [4][8]` | `[0-3][0-7]` |
| `OEEBinCT` | `long [260]` | `[0-259]` |
| `OEETrayCT` | `long [256]` | `[0-255]` |
| `SoftSpeed` | `int [200]` | `[0-199]` |
| `HeadPass` | `long [160]` | `[0-159]` |
| `dTempHistroy` | `double [100][60]` | `[0-99][0-59]` |
| `TrayCount` | `int [10]` | `[0-9]` |
| `iJamCount` | `int [3]` | `[0-2]` |
| `SendCT` | `long [4]` | `[0-3]` |

## 常見索引來源與範圍

| 變數 | 典型來源 | 範圍 |
|----------|---------------|-------|
| `OutArmSuck.iWhichAuto[i][j]` | Tray 指派 | `eAuto1(0)` ~ `eMag14(32)`，即 `0-32` |
| `OutArmSuck.iBinData[i][j]` | Tester 回傳值 | 可能為 `-1` 或 `0` ~ `TEST_MAX_BIN(256)` |
| `iFixRightHalf` | 設定初始化 | `eFix6(11)` 或 `eFix12(17)` |
| `iTestBinCount` | 執行期設定 | `0` ~ `TEST_MAX_BIN(256)` |
| `FTestSuck.iShtRow` | Socket 設定 | `1` ~ `MAX_SOCKET_ROW(4)` |
| `FTestSuck.iShtCol` | Socket 設定 | `1` ~ `MAX_SOCKET_COL(8)` |
| `eTrayCount` | 列舉常數 | `33` |
| `X`（來自滑鼠事件） | Grid cell 索引 | `0` ~ `MAX_SOCKET_COL-1` |

## 稽核檢查清單

在檢查存取 LastSet 陣列的程式碼時：

1. **識別索引變數**：索引是常數還是變數？
2. **追蹤範圍**：索引可能取到哪些值？請沿著賦值鏈往上追。
3. **檢查保護條件**：陣列存取前是否已有邊界檢查？
4. **留意 off-by-one**：例如迴圈邊界附近的 `j+1`、`i+2`、`X-1`。
5. **留意邏輯錯誤**：保護條件中的 `||` 與 `&&` 是否正確。
6. **檢查負索引**：來自外部來源（tester、DLL、滑鼠事件）的值可能為 `-1`。
7. **檢查間接索引**：例如 `array1[array2[x]]`，兩層都要做邊界檢查。

## 歷史缺陷案例（V3.33.898 稽核，2026-03-16）

| 問題 | 檔案 | 修正 |
|-----|------|-----|
| `iSiteBinCTForAlways` 的保護條件 `\|\|` 應為 `&&` | `atester_ProcessCount.cpp` L1629 | 改為 `&&` |
| `lSCKARTBinCT[20]` 被 `iWhichAuto`（最高到 32）寫爆 | `aoutarm9045.cpp` L2743 | 擴大為 `[256]` |
| `j==7` 時仍存取 `bUseTestSocket[0][0][j+1]` | `uYieldMonitoring.cpp` L3563 | 新增 `j+1<8` 保護 |
| `X==0` 時仍存取 `bUseTestSocketEE[Z][Y][X-1]` | `main.cpp` L28038 | 新增 `X!=0` 保護 |
| `iBinData32` 對 `iBinData` 缺少上界檢查 | `aoutarm9045.cpp` L2711-2736 | 新增 `>iTestBinCount` 檢查 |

## 快速稽核的 grep 模式

```powershell
# 找出所有使用變數索引的 LastSet 陣列存取
Get-ChildItem -Recurse -Include *.cpp -Exclude .svn | Select-String -Pattern "LastSet\.\w+\[[^0-9\]]"

# 找出特定高風險模式
Get-ChildItem -Recurse -Include *.cpp -Exclude .svn | Select-String -Pattern "LastSet\.iBinCTForAlways\["
Get-ChildItem -Recurse -Include *.cpp -Exclude .svn | Select-String -Pattern "LastSet\.lSCKARTBinCT\["
Get-ChildItem -Recurse -Include *.cpp -Exclude .svn | Select-String -Pattern "LastSet\.bUseTestSocket\w*\["
Get-ChildItem -Recurse -Include *.cpp -Exclude .svn | Select-String -Pattern "LastSet\.iSiteBinCTForAlways\["
Get-ChildItem -Recurse -Include *.cpp -Exclude .svn | Select-String -Pattern "LastSet\.iBinData32\["
```
