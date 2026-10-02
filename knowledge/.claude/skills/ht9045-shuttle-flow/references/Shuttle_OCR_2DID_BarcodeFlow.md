# Shuttle OCR 2DID Barcode Flow (BarCode_Sh1/Sh2)

> Source: `BarCode\BarCode_Sh1.cpp`, `BarCode\BarCode_Sh2.cpp`, `BarCode\BarCode.cpp`
> Related: `acarry.cpp` case 2400/2500（2DID scan trigger points）
> Updated: 2026-04-23

---

## 1. Shuttle OCR 2DID 呼叫路徑

```text
Do_Auto_SHT1() case 2400                              [acarry.cpp]
  ├─ ebctUseCCDMode → DoBarcodeCCDInShuttle_1()        [BarCode_Sh1.cpp line 83]
  ├─ b2DTriggerMode → DoBarcodeTriggerInShuttle_1()    [BarCode_Sh1.cpp line 2385]
  └─ default        → DoBarcodeScanInShuttle_1()       [BarCode_Sh1.cpp line 2956]
                          └─ Barcode_StartScan_In()    [BarCode.cpp line 2029]

Do_Auto_SHT1() case 2500
  └─ DoBarcodeScanOutShuttle_1()                       [BarCode_Sh1.cpp line 3921]
```

Shuttle 2 同理，改用 `_2()` 版本。

---

## 2. DoBarcodeScanInShuttle_1() 狀態機

> Task variable: `iInitialBarcodeInShuttle1Task`

### State 流程

```text
1 → 1000 → 1100 → 1120 → 1150 → 1200 → (loop or finish)
                             │
                             ├─ timeout → 1160 → retry 1100 or alarm 1170
                             ├─ HandShake timeout → 1180 (alarm)
                             └─ exposure error → 1180 (alarm)
```

### 各 State 說明

| State | 說明 | 關鍵邏輯 |
|-------|------|----------|
| 1 | 初始化，清計數 | `iNowCheckStep = iShtCol-1` 遞減 |
| 1000 | 計算需讀碼數 | `iNeedBarcodeCount` 累加 |
| 1100 | 清格 + 設定延遲 | 清 UI cell，設 `BarcodePosDelay` |
| 1120 | 位置到位等待 | `BarcodeDelay[iSht]` 設 timeout |
| 1150 | **送掃碼 + 等回傳** | 呼叫 `Barcode_StartScan_In()`，等 `bBarcodeNum` 全 true |
| 1160 | Retry offset move | 若 `bRetryOffsetMove`，先退再進 |
| 1170 | Save Fail Image | 若 `bSaveFailImage`，存圖 → 1175 → 1180 |
| 1180 | 告警 / 結果處理 | JAM0460 / WAR0471 / 重複碼判斷 |
| 1200 | 完成一步，遞減 | `iNowCheckStep--`，loop 回 1100 |

---

## 3. State 1150 三種退出路徑

```text
state 1150
  ├─ bBarcodeNum 全 true      → state 1200 (正常)
  ├─ HandShake timeout 觸發    → state 1180 (告警)
  └─ BarcodeDelay timeout 觸發 → retry 或 alarm
```

### HandShake timeout 條件（修復後）

```cpp
if((BAR_CODE_INSTALL==ebctEtherNetCCD || BAR_CODE_INSTALL==ebcUseOCR) &&
   CosFunction.bUseHandShakeCommunication &&
   TestIF_File.bUseHandShakeCommunication==true)
```

**重要**: 2026-04-20 修復前，此條件不包含 `ebcUseOCR`，
導致 OCR 模式下 HandShake timeout 不生效，可能造成 state 1150 hang。

---

## 4. Barcode_StartScan_In() Socket 發送邏輯

```text
Barcode_StartScan_In(BarCodeIndex, ...)
  └─ if(bBarcodeStartDelay == true)
       ├─ Socket Active → SendText(strTrigCMD)
       └─ Socket NOT Active → Log warning + try Open() reconnect  ← (2026-04-20 新增)
  └─ if(bBarcodeDataSaveReady || Simulate) → return true
  └─ else → return false (等待回傳)
```

---

## 5. CCD Socket 對應表

| Socket Component | Tag | BarCodeIndex | OCR Log 名稱 | Shuttle |
|-----------------|-----|-------------|-------------|---------|
| ClientSocket_Shuttle1_A | 0 | 0 | CCD1 | Shuttle 1 |
| ClientSocket_Shuttle1_B | 1 | 1 | CCD2 | Shuttle 1 |
| ClientSocket_Shuttle2_A | 2 | 2 | CCD3 | Shuttle 2 |
| ClientSocket_Shuttle2_B | 3 | 3 | CCD4 | Shuttle 2 |

### OCR 模式 iOCRMap 反轉

Shuttle 1: `iOCRMap = {1, 0, 3, 2}`
- iBarCode1_1 (Row A, index 0) → 透過 CCD index 1 (Shuttle1_B) 讀碼
- iBarCode1_2 (Row B, index 1) → 透過 CCD index 0 (Shuttle1_A) 讀碼

非 OCR 模式: `iOCRMap = {0, 1, 2, 3}` (identity)

---

## 6. 已知問題與修復紀錄

| 日期 | 問題 | 修復 | 影響檔案 |
|------|------|------|----------|
| 2026-04-20 | OCR 模式 state 1150 無 HandShake timeout | 加 `ebcUseOCR` 到條件 | BarCode_Sh1.cpp, BarCode_Sh2.cpp |
| 2026-04-20 | Socket 斷線 silent skip 無 log/重連 | else 分支加 log + Open() | BarCode.cpp |
| 2026-04-21 | Contact Test / Auto Height 未清 `bCheckCodeError`，導致 JAM0460 (empty site) | `CleanBarcodeError(*)` 加在所有 `InitialBarcodeScanInShuttle*()` 後（cContact.cpp 共 6 處）| cContact.cpp |
| 2026-04-23 | **case 1200 OCR mode 重複碼排除條件用硬碼 row index 0/1，OCR 模式下 row 已被 iOCRMap 反轉，導致誤判 JAM0460** | 把 `(i!=0)` 和 `(i!=1)` 改為 `(i!=iOCRMap[iBarCode1_1])` 和 `(i!=iOCRMap[iBarCode1_2])` | BarCode_Sh1.cpp |

---

## 7. InitialBarcodeScanInShuttle*() 與 CleanBarcodeError() 關係

### 呼叫點一覽

`InitialBarcodeScanInShuttle1()` 和 `InitialBarcodeScanInShuttle2()` 可在以下位置被呼叫：

| 位置 | 檔案 | 功能 | CleanBarcodeError 後補？ |
|------|------|------|--------------------------|
| acarry.cpp (主流程 2DID) | acarry.cpp | 正常生產 Shuttle 掃碼 | ❌ 不需要（case 1180 alarm retry 已呼叫）|
| cContact.cpp — Contact Test | cContact.cpp | case 142 / case 142 | ✅ 已補（20260421）|
| cContact.cpp — Auto Height | cContact.cpp | case 2100 / case 9500 | ✅ 已補（20260421）|

### 關鍵差異：主流程 vs Contact/AutoHeight

在 **主流程（acarry.cpp）**：
- case 1180（alarm handler）先呼叫 `CleanBarcodeError(1/2)` 再進入 `InitialBarcodeScanInShuttle*()`
- 因此 flag 已被清除，`InitialBarcodeScanInShuttle*` 進入後不會帶入殘留 error

在 **Contact/AutoHeight（cContact.cpp）**：
- 直接呼叫 `InitialBarcodeScanInShuttle*()` 而 **未先清除** `bCheckCodeError`
- 若上一次 lot 發生過 WAR0467 / JAM0460，`bCheckCodeError[iBarCode1_1]` 殘留 `true`
- `DoBarcodeScanInShuttle_1()` Task=1200 → 進入重複碼判斷 → 因 `bCheckCodeByLot==false` 跳過 WAR0467 → `sErrorPart=""` → **JAM0460 with empty site label**

### Fix C（2026-04-21, V3.33.903.1）

在所有 Contact/AutoHeight 的 `InitialBarcodeScanInShuttle*()` 呼叫後立即加入 `CleanBarcodeError(*)`:

```cpp
// cContact.cpp — Contact Test Sht1 case 142
InitialBarcodeScanInShuttle1();
CleanBarcodeError(1); //Steven 20260421 : Clean error flags to prevent false alarm in Contact/AutoHeight flow

// cContact.cpp — Contact Test Sht2 case 142
InitialBarcodeScanInShuttle2();
CleanBarcodeError(2); //Steven 20260421 ...

// cContact.cpp — Auto Height case 2100 Sht1 branch
InitialBarcodeScanInShuttle1();
CleanBarcodeError(1); //Steven 20260421 ...

// cContact.cpp — Auto Height case 2100 Sht2 branch
InitialBarcodeScanInShuttle2();
CleanBarcodeError(2); //Steven 20260421 ...

// cContact.cpp — Auto Height case 9500 (end of flow) Sht1
InitialBarcodeScanInShuttle1();
CleanBarcodeError(1); //Steven 20260421 ...

// cContact.cpp — Auto Height case 9500 Sht2
InitialBarcodeScanInShuttle2();
CleanBarcodeError(2); //Steven 20260421 ...
```

### JAM0460 診斷：empty site label vs 正常告警

| JAM0460 型態 | Log 特徵 | 根本原因 |
|-------------|----------|----------|
| **Empty site label**（如 `,, 2DID error`）| `sErrorPart=""` | `bCheckCodeError` 殘留，Task=1200 重複碼誤判 |
| 正常 site label（如 `Bb 2DID error`）| `sErrorPart="Bb"` | OCR 實際讀碼失敗、真正的重複碼 |

**診斷口訣**：JAM0460 有 empty site label（兩個逗號 `,,`）= 非 OCR 硬體問題 = 重複碼旗標殘留問題

> ⚠️ Fix C（2026-04-21）解決了「Contact/AutoHeight 後殘留 bCheckCodeError」導致的 JAM0460，
> 但並未解決 OCR 模式本身在 case 1200 的 row index 錯誤問題（Fix D）。
> 若客戶端 Fix C 後仍復現 JAM0460 empty site，需檢查 Fix D。

---

*Reference for: ht9045-shuttle-flow skill*

<!-- AI(skill-merge) 20260501 (RogerYang): 由 Steven 同步 Section 8 (Fix D, 2026-04-23) -->

## 8. case 1200 OCR Row Mapping Bug（Fix D, 2026-04-23）

### 問題描述

case 1200 在比對重複碼時，排除「自己的格子」的條件使用了硬碼 row index：

```cpp
// 修正前（WRONG）— BarCode_Sh1.cpp
if(FLCarryKit.cDeviceInf[0][iNowCheckStep]==asString && (i!=0 || j!=iNowCheckStep))
    bCheckCodeError[iOCRMap[iBarCode1_1]]=true;

if(FLCarryKit.cDeviceInf[1][iNowCheckStep]==asString && (i!=1 || j!=iNowCheckStep))
    bCheckCodeError[iOCRMap[iBarCode1_2]]=true;
```

在 **OCR 模式**，`iOCRMap={1,0,3,2}`：
- `cDeviceInf[0]` 的掃碼資料來自 CCD index 1（Shuttle1_B），存在 `mtBarcodeInSh` **row 1**
- `cDeviceInf[1]` 的掃碼資料來自 CCD index 0（Shuttle1_A），存在 `mtBarcodeInSh` **row 0**

因此硬碼 `i!=0` 無法排除 cDeviceInf[0] 對應的 row 1，反而把 row 0（另一顆 CCD 的資料）當作「自己的格子」排除，導致**正確的掃碼結果被互相誤判為重複碼**，觸發 JAM0460 empty site。

### Root Cause 確認方式（DBG-F log）

2026-04-23 透過在 case 1200 加入 debug log（DBG-F）後，客戶端復現，log 100% 確認：

```
DBG-F dup HIT row1, i=0, j=1, iNowCheckStep=1,
  asString="N5B12J0F-DA03071", cDeviceInf[1][1]="N5B12J0F-DA03071"
  -> set bCheckCodeError[0]=true        ← 誤判：CCD0 的資料不應觸發 bCheckCodeError

DBG-F dup HIT row0, i=1, j=1, iNowCheckStep=1,
  asString="N5B12J0F-DA03047", cDeviceInf[0][1]="N5B12J0F-DA03047"
  -> set bCheckCodeError[1]=true        ← 誤判：CCD1 的資料不應觸發 bCheckCodeError
```

mtBarcodeInSh（物理 CCD 順序）vs cDeviceInf（邏輯 iOCRMap 反轉後）：

| | mtBarcodeInSh row 0 | mtBarcodeInSh row 1 |
|---|---|---|
| 物理 CCD | CCD0 (Shuttle1_A) 資料 | CCD1 (Shuttle1_B) 資料 |
| cDeviceInf 對應 | **row 1**（iOCRMap[iBarCode1_2]=0）| **row 0**（iOCRMap[iBarCode1_1]=1）|

### 修正方式（BarCode_Sh1.cpp, Steven 20260423）

```cpp
// 修正後（CORRECT）
if(FLCarryKit.cDeviceInf[0][iNowCheckStep]==asString &&
   (i!=iOCRMap[iBarCode1_1] || j!=iNowCheckStep))  //Steven 20260423 : use iOCRMap[iBarCode1_1] as row ref
    bCheckCodeError[iOCRMap[iBarCode1_1]]=true;

if(FLCarryKit.cDeviceInf[1][iNowCheckStep]==asString &&
   (i!=iOCRMap[iBarCode1_2] || j!=iNowCheckStep))  //Steven 20260423 : use iOCRMap[iBarCode1_2] as row ref
    bCheckCodeError[iOCRMap[iBarCode1_2]]=true;
```

**非 OCR 模式相容性驗證**：
- 非 OCR：`iOCRMap={0,1,2,3}` → `iOCRMap[0]=0`, `iOCRMap[1]=1` → 條件與原碼完全相同，無破壞

### BarCode_Sh2.cpp 是否有同樣問題？

**No**。Sh2（`DoBarcodeScanInShuttle_2()`）的 OCR 分支：
```cpp
else if(BAR_CODE_INSTALL==ebcUseOCR)
{
    iBarCodeRowA = iBarCode2_1;  // = 2，無 swap
    iBarCodeRowB = iBarCode2_2;  // = 3
}
```
Sh2 的 `iOCRMap` 已被注解掉（`// int iOCRMap[]={1, 0, 3, 2};`），case 1200 直接用 `iBarCodeRowA/B` 且 row index 與 mtBarcodeInSh 一致，**不需修正**。

### JAM0460 診斷更新（加入 Fix D 場景）

| JAM0460 型態 | Log 特徵 | 根本原因 | Fix |
|-------------|----------|----------|-----|
| Empty site label（`,,`）+ 第一個 Lot | 首次啟動就發生 | Sh1 case 1200 OCR row mapping 錯誤 | **Fix D** |
| Empty site label（`,,`）+ Lot 切換後 | 上一 Lot 發生過 WAR/JAM | bCheckCodeError 殘留（Contact/AH） | Fix C |
| 正常 site label（如 `Bb 2DID error`） | sErrorPart 有值 | OCR 實際讀碼失敗 / 真正重複碼 | 查 OCR 硬體 |

---

*Reference for: ht9045-shuttle-flow skill*
