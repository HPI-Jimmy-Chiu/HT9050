# lastset-array-audit 源碼參照

> 版本基準：HT9011UC_Code_V3.33.900.0_20260331

## 結構定義

```
路徑：d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\LastSet.h
struct LAST_GENERAL_SET  結束行：~L1144
持久化方式：raw binary（路徑 D:\HT9045\IniData\LastSet.bin）
```

## Load / Save 函數

| 函數 | 檔案 | 行號 |
|------|------|------|
| `ReadLastSetIni()` | cprod.cpp | L2930 |
| `SaveLastSetIni()` | cprod.cpp | L3045 |
| `ReadLastSetIni()` 呼叫處 | cConfiguration.cpp | L111, L4413, L7478 |
| `SaveLastSetIni()` 呼叫處 | cConfiguration.cpp | L7076 |
| `SaveLastSetIni()` 呼叫處 | ContactForce.cpp | L784 |

## 高風險陣列存取位置（V3.33.900.0）

| 陣列 | 宣告（LastSet.h） | 主要存取檔案 |
|------|-----------------|-------------|
| `lSCKARTBinCT[256]` | `long [256]`（V3.33.898 前為 `[20]`） | aoutarm9045.cpp:L2786、asortarm.cpp:L3527、Command.cpp:L13714 |
| `iSiteBinCTForAlways[4][8][256]` | `uint [4][8][256]` | atester_ProcessCount.cpp:L1642 |
| `iBinCTForAlways[4][20]` | `int [4][20]` | aoutarm9045.cpp:L2734/2738、asortarm.cpp:L3476 |
| `iBinData32[4][260]` | `int [4][260]` | aoutarm9045.cpp:L2751/2753/2758 |
| `bUseTestSocket[2][4][8]` | `bool [2][4][8]` | adam6024.cpp:L1096/1097/1109 |

## 稽核 grep 指令（在專案目錄直接執行）

```powershell
$base = "d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331"
# 全部 LastSet 變數索引存取
Get-ChildItem $base -Filter *.cpp | Select-String -Pattern "LastSet\.\w+\[[^0-9\`"']"
# 各高風險陣列
Get-ChildItem $base -Filter *.cpp | Select-String "LastSet\.lSCKARTBinCT\["
Get-ChildItem $base -Filter *.cpp | Select-String "LastSet\.iBinCTForAlways\["
Get-ChildItem $base -Filter *.cpp | Select-String "LastSet\.iSiteBinCTForAlways\["
Get-ChildItem $base -Filter *.cpp | Select-String "LastSet\.iBinData32\["
Get-ChildItem $base -Filter *.cpp | Select-String "LastSet\.bUseTestSocket\w*\["
```

## 版本異動紀錄

| 版本 | 異動 | 原因 |
|------|------|------|
| V3.33.898 | `lSCKARTBinCT` `[20]` → `[256]` | `iWhichAuto` 最高可達 32，遠超原始邊界 |
| V3.33.898 | `bUseTestSocket` `[4][8]` → `[2][4][8]` | 支援雙 Index 配置 |
