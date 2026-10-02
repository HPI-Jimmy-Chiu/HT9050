# P8 — 索引映射陣列旁路（Index-Map Bypass）詳細指南

## 背景

HT9045 BarCode 模組以「邏輯索引 → 物理索引」的映射陣列處理 OCR 模式下的 CCD row 反轉：

- **非 OCR 模式**：邏輯 row = 物理 row（identity）
- **OCR 模式**：Shuttle 1 的 CCD 實體顺序與邏輯索引反轉
  - 邏輯 `iBarCode1_1`（Row A）→ 物理 CCD index 1
  - 邏輯 `iBarCode1_2`（Row B）→ 物理 CCD index 0

任何在同一函式內**跳過映射、直接使用邏輯索引**存取 state array，都會在 OCR 模式下造成 row/col 交錯。

---

## 觸發掃描的 pattern

| Pattern | 說明 |
|---------|------|
| `iOCRMap` | 遺留模式，必查初始化與旁路 |
| `iBarCodeRowA` / `iBarCodeRowB` | 新模式，必查初始化方向與旁路 |

---

## 遇到 `iOCRMap` 時的檢查清單

1. **陣列填入確認**
   - OCR 模式反轉初始化：`iOCRMap[]={1,0,3,2}`
   - 非 OCR 模式 identity reset：`for(i=0;i<4;i++) iOCRMap[i]=i`
   - 兩者缺一即為 bug。

2. **同一函式內混用**
   - 同時出現 `arr[iOCRMap[X]]` 與 `arr[X]`
   - 迴圈內出現 `(i!=0 || ...)` 等裸數字 row 比對 → 必為 bug（見 Fix D）

3. **Copy-paste 不一致**
   - `if(arr[iOCRMap[B]])` 區塊內出現 `arr[iOCRMap[A]]=false` → 必為 bug（見 Fix C-1）

4. **重構建議**（`DoBarcodeScanInShuttle_*` 語意，OCR 反轉）
   ```cpp
   //Steven YYYYMMDD : refactor iOCRMap[] to RowA/RowB pattern
   int iBarCodeRowA = (BAR_CODE_INSTALL==ebcUseOCR) ? iBarCode1_2 : iBarCode1_1;
   int iBarCodeRowB = (BAR_CODE_INSTALL==ebcUseOCR) ? iBarCode1_1 : iBarCode1_2;
   ```
   全函式取代 `iOCRMap[iBarCode1_1]` → `iBarCodeRowA`、`iOCRMap[iBarCode1_2]` → `iBarCodeRowB`。

---

## 遇到 `iBarCodeRowA` / `iBarCodeRowB` 時的檢查清單

1. **初始化方向是否符合函式語意**

   | 函式類型 | OCR 模式行為 | 宣告寫法 |
   |---------|-------------|---------|
   | `DoBarcodeCCDInShuttle_*`（直接送 socket 指令） | **不反轉** | `iBarCodeRowA = iBarCode1_1; iBarCodeRowB = iBarCode1_2;` |
   | `DoBarcodeScanInShuttle_*`（索引 state array） | **OCR 反轉** | `(BAR_CODE_INSTALL==ebcUseOCR) ? iBarCode1_2 : iBarCode1_1` |

   反向使用即為語意錯誤，OCR 機台必當機。

2. **直接索引旁路**
   - 函式內是否有殘留 `arr[iBarCode1_1]` / `arr[iBarCode1_2]` / `arr[iOCRMap[...]]`
   - 應全部走 `iBarCodeRowA/B`

3. **Copy-paste 對換錯誤**
   - `if(arr[iBarCodeRowB])` 區塊內出現 `arr[iBarCodeRowA]=false`（A/B 在 if 條件與 body 對不齊）→ 必為 bug

4. **裸數字 row 比對**
   - `(i!=0 || ...)` / `(i==1 && ...)` 等與 RowA/B 等價的條件
   - 改為 `(i!=iBarCodeRowA || ...)`

---

## 歷史案例

### Fix C-1（HT9045 BarCode_Sh1.cpp, case 1180）
Copy-paste 錯誤：
```cpp
if(bCheckCodeError[iOCRMap[iBarCode1_2]])
{
    ...
    bCheckCodeError[iOCRMap[iBarCode1_1]]=false;   // ← BUG：應為 iBarCode1_2
}
```

### Fix D（HT9045 BarCode_Sh1.cpp, case 1200, 2026-04-23）
OCR 模式下 row 被 `iOCRMap` 反轉，裸數字條件導致兩 row 互判為重複碼：
```cpp
// Before (BUG):
if(BLCarryKit.cDeviceInf[0][step]==str && (i!=0 || j!=step))
    bCheckCodeError[iBarCodeRowA]=true;  // i==0 在 OCR 下實為 RowB
if(BLCarryKit.cDeviceInf[1][step]==str && (i!=1 || j!=step))
    bCheckCodeError[iBarCodeRowB]=true;

// After (FIX):
if(BLCarryKit.cDeviceInf[0][step]==str && (i!=iBarCodeRowA || j!=step))
    bCheckCodeError[iBarCodeRowA]=true;
if(BLCarryKit.cDeviceInf[1][step]==str && (i!=iBarCodeRowB || j!=step))
    bCheckCodeError[iBarCodeRowB]=true;
```
觸發現象：JAM0460 empty site，OCR 讀取實際正常。

### 重構（BarCode_Sh1.cpp `DoBarcodeScanInShuttle_1`, 2026-04-23）
147 處 `iOCRMap[iBarCode1_X]` 全面替換為 `iBarCodeRowA/B`，宣告改用三元運算子。
`iOCRMap[]` 陣列及 identity for-loop 已移除。