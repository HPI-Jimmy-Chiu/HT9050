# P7 — LastSet 陣列邊界稽核：完整參考

> 本文件由原 `ht9045-lastset-array-audit` skill 遷移而來（2026-04-16）。  
> 適用版本基準：HT9011UC_Code_V3.33.900.0_20260331

---

## 目的

在 HT9045 程式碼中偵測並防止 `LAST_GENERAL_SET`（LastSet）結構成員的陣列越界存取。  
此結構以 raw binary blob 方式持久化到磁碟（路徑：`D:\HT9045\IniData\LastSet.bin`），  
任何記憶體破壞都可能**無聲地污染**已儲存狀態。

---

## Binary 版面相容規則（關鍵）

`LAST_GENERAL_SET` 以 raw binary 格式儲存/載入，因此必須遵守：

- **不得**變更結構內任何既有欄位的位元長度/大小。
- **不得**調整既有成員順序、刪除既有成員或改變既有成員大小。
- 若需新增欄位，**只能追加在結構最後方**。

違反此規則會造成 binary 版面不相容，並可能導致持久化資料損毀。

---

## 結構定義位置

```
路徑：d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\LastSet.h
struct LAST_GENERAL_SET  結束行：~L1144
```

### Load / Save 函數

| 函數 | 檔案 | 行號 |
|------|------|------|
| `ReadLastSetIni()` | cprod.cpp | L2930 |
| `SaveLastSetIni()` | cprod.cpp | L3045 |
| `ReadLastSetIni()` 呼叫處 | cConfiguration.cpp | L111, L4413, L7478 |
| `SaveLastSetIni()` 呼叫處 | cConfiguration.cpp | L7076 |
| `SaveLastSetIni()` 呼叫處 | ContactForce.cpp | L784 |

---

## 完整陣列邊界表

| 陣列 | 宣告 | 安全索引範圍 |
|------|------|-------------|
| `BinCT` | `unsigned int [4][256]` | `[0-3][0-255]` |
| `BinCT_ART` | `unsigned int [4][256]` | `[0-3][0-255]` |
| `BinCT_PTI` | `long [4][256]` | `[0-3][0-255]` |
| `lBinCT_AutoBin` | `long [10][256]` | `[0-9][0-255]` |
| `iBinData32` | `int [4][260]` | `[0-3][0-259]` |
| `iBinData32_ART` | `int [4][260]` | `[0-3][0-259]` |
| `iBinCTForAlways` | `int [4][20]` | `[0-3][0-19]` |
| `iSiteBinCTForAlways` | `unsigned int [4][8][256]` | `[0-3][0-7][0-255]` |
| `iSiteTotalCTForAlways` | `unsigned int [4][8]` | `[0-3][0-7]` |
| `lSCKARTBinCT` | `long [256]` | `[0-255]`（V3.33.898 前為 `[20]`） |
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

---

## 高風險陣列存取位置（V3.33.900.0）

| 陣列 | 宣告（LastSet.h） | 主要存取檔案 |
|------|-----------------|-------------|
| `lSCKARTBinCT[256]` | `long [256]`（V3.33.898 前為 `[20]`） | aoutarm9045.cpp:L2786、asortarm.cpp:L3527、Command.cpp:L13714 |
| `iSiteBinCTForAlways[4][8][256]` | `uint [4][8][256]` | atester_ProcessCount.cpp:L1642 |
| `iBinCTForAlways[4][20]` | `int [4][20]` | aoutarm9045.cpp:L2734/2738、asortarm.cpp:L3476 |
| `iBinData32[4][260]` | `int [4][260]` | aoutarm9045.cpp:L2751/2753/2758 |
| `bUseTestSocket[2][4][8]` | `bool [2][4][8]` | adam6024.cpp:L1096/1097/1109 |

---

## 常見索引來源與範圍

| 變數 | 典型來源 | 範圍 |
|------|---------|------|
| `OutArmSuck.iWhichAuto[i][j]` | Tray 指派 | `eAuto1(0)` ~ `eMag14(32)`，即 `0-32` |
| `OutArmSuck.iBinData[i][j]` | Tester 回傳值 | 可能為 `-1` 或 `0` ~ `TEST_MAX_BIN(256)` |
| `iFixRightHalf` | 設定初始化 | `eFix6(11)` 或 `eFix12(17)` |
| `iTestBinCount` | 執行期設定 | `0` ~ `TEST_MAX_BIN(256)` |
| `FTestSuck.iShtRow` | Socket 設定 | `1` ~ `MAX_SOCKET_ROW(4)` |
| `FTestSuck.iShtCol` | Socket 設定 | `1` ~ `MAX_SOCKET_COL(8)` |
| `eTrayCount` | 列舉常數 | `33` |
| `X`（來自滑鼠事件） | Grid cell 索引 | `0` ~ `MAX_SOCKET_COL-1` |

---

## 稽核檢查清單

在檢查存取 LastSet 陣列的程式碼時：

1. **識別索引變數**：索引是常數還是變數？
2. **追蹤範圍**：索引可能取到哪些值？請沿著賦值鏈往上追。
3. **檢查保護條件**：陣列存取前是否已有邊界檢查？
4. **留意 off-by-one**：例如迴圈邊界附近的 `j+1`、`i+2`、`X-1`。
5. **留意邏輯錯誤**：保護條件中的 `||` 與 `&&` 是否正確。
6. **檢查負索引**：來自外部來源（tester、DLL、滑鼠事件）的值可能為 `-1`。
7. **檢查間接索引**：例如 `array1[array2[x]]`，兩層都要做邊界檢查。

### 修正時的語意保持原則（關鍵）

**新增邊界保護時，不得改變原始程式碼的語意。** 具體規則：

- **只加保護，不改邏輯**：新增 `if(idx>=0 && idx<SIZE)` guard，或改迴圈邊界上限，但不得改動被保護的運算式本身。
- **保持原始行為一致**：若原始碼在越界時恰好不會觸發（如 NN_2Row 模式 `iShtRow` 最大為 2，使 `i+2<=3` 不越界），加入防禦性 guard 後的正常路徑執行結果必須與修改前完全相同。
- **禁止「順便」重構**：修正越界時不得更動變數命名、不得合併/拆分迴圈、不得移動程式碼行、不得添加無關功能。
- **註解標示**：修改行旁加上 `//Steven YYYYMMDD : add boundary guard` 格式的行內註解，說明修改原因。

---

## 歷史缺陷案例（V3.33.898 稽核，2026-03-16）

| 問題 | 檔案 | 修正 |
|------|------|------|
| `iSiteBinCTForAlways` 的保護條件 `\|\|` 應為 `&&` | `atester_ProcessCount.cpp` L1629 | 改為 `&&` |
| `lSCKARTBinCT[20]` 被 `iWhichAuto`（最高到 32）寫爆 | `aoutarm9045.cpp` L2743 | 擴大為 `[256]` |
| `j==7` 時仍存取 `bUseTestSocket[0][0][j+1]` | `uYieldMonitoring.cpp` L3563 | 新增 `j+1<8` 保護 |
| `X==0` 時仍存取 `bUseTestSocketEE[Z][Y][X-1]` | `main.cpp` L28038 | 新增 `X!=0` 保護 |
| `iBinData32` 對 `iBinData` 缺少上界檢查 | `aoutarm9045.cpp` L2711-2736 | 新增 `>iTestBinCount` 檢查 |

---

## 版本異動紀錄

| 版本 | 異動 | 原因 |
|------|------|------|
| V3.33.898 | `lSCKARTBinCT` `[20]` → `[256]` | `iWhichAuto` 最高可達 32，遠超原始邊界 |
| V3.33.898 | `bUseTestSocket` `[4][8]` → `[2][4][8]` | 支援雙 Index 配置 |

---

## 快速稽核 grep 模式

```powershell
$base = "d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331"

# 全部 LastSet 變數索引存取
Get-ChildItem $base -Filter *.cpp | Select-String -Pattern "LastSet\.\w+\[[^0-9`\"']"

# 各高風險陣列
Get-ChildItem $base -Filter *.cpp | Select-String "LastSet\.lSCKARTBinCT\["
Get-ChildItem $base -Filter *.cpp | Select-String "LastSet\.iBinCTForAlways\["
Get-ChildItem $base -Filter *.cpp | Select-String "LastSet\.iSiteBinCTForAlways\["
Get-ChildItem $base -Filter *.cpp | Select-String "LastSet\.iBinData32\["
Get-ChildItem $base -Filter *.cpp | Select-String "LastSet\.bUseTestSocket\w*\["
```
