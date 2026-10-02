# 記憶體安全 Bug 案例

---

### F01 LastSet 陣列越界
- **問題編號**：通用（多版本曾發生）
- **影響版本**：V3.33.898 及更早
- **症狀**：機台隨機異常行為、數值污染、不明 crash
- **根因**：`LAST_GENERAL_SET` 結構中的陣列（如 `BinCT[]`, `iBinData32[]`, `bUseTestSocket[]`）新增成員時未同步更新 MAX 常數或迴圈邊界
- **修法**：
  1. 使用 `sizeof(arr)/sizeof(arr[0])` 取代硬編碼常數
  2. 新增成員時強制搜尋所有使用點
- **預防**：
  - pre-release-check P7 已包含 LastSet 陣列越界稽核
  - 參考 skill: `ht9045-array-audit`（原 `ht9045-lastset-array-audit`，已通用化）
- **高風險陣列一覽**：
  | 陣列 | 定義大小 | 風險說明 |
  |------|---------|---------|
  | `BinCT[MAX_BIN_NUM]` | 513 | BIN 號超範圍 |
  | `iBinData32[MAX_BIN_32SITE]` | 32 | Site 數超界 |
  | `bUseTestSocket[MAX_SITE]` | 依平台 | Site mapping 超界 |
  | `iSiteBinCTForAlways[MAX_SITE][MAX_BIN_NUM]` | 二維 | 雙維度都可能超界 |

---

### F02 除以零崩潰
- **問題編號**：通用
- **影響版本**：多版本
- **症狀**：程式 crash 或 floating point exception
- **根因**：除法運算前未檢查分母是否為零，常見於：
  - Yield 百分比計算
  - 平均值計算
  - Pitch 距離計算
- **修法**：
  ```cpp
  // 方式 1：SafeDiv 巨集
  #define SafeDiv(a, b) ((b) != 0 ? (a) / (b) : 0)
  
  // 方式 2：直接檢查
  if (divisor != 0)
      result = dividend / divisor;
  ```
- **預防**：
  - pre-release-check P6 已包含除法安全掃描
  - 新增除法時必須考慮分母為零的場景
