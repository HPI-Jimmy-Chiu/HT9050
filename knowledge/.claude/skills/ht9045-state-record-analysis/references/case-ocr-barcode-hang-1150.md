# Case Study: OCR Barcode State 1150 Hang (FMSH 884)

> 2026-04-20 | HT-9045HW PMLD1361 | V3.33.893.4 → V3.33.903.0

---

## 現象

- State Record: `BarcodeInShuttle1Task` 停在 **state 1150** 超過 34 秒
- State 轉移序列: `1100 → 1120 → 1150 (HANG)`
- 時間軸:
  - 15:12:17.256 → state 1100
  - 15:12:18.194 → state 1120
  - 15:12:18.244 → state 1150 (最後記錄)
  - 15:12:52 → State Record 寫入，仍停在 1150

## 根因分析

### State 1150 的作用

`DoBarcodeScanInShuttle_1()` 中的 state 1150:
1. 呼叫 `Barcode_StartScan_In()` 對每個 CCD 發送觸發指令
2. 等待 `bBarcodeNum[...]` 全部為 true（代表所有站回傳完成）
3. 有三種退出路徑:
   - `bBarcodeNum` 全 true → state 1200（正常完成）
   - HandShake timeout → state 1180（逾時警報）
   - `BarcodeDelay` timeout → retry 或 alarm

### Bug: OCR 模式缺少 HandShake timeout 保護

HandShake timeout 的條件為:
```cpp
else if(BAR_CODE_INSTALL==ebctEtherNetCCD &&   // ← 只判斷 EtherNetCCD
        CosFunction.bUseHandShakeCommunication &&
        TestIF_File.bUseHandShakeCommunication==true)
```

**OCR 模式** (`BAR_CODE_INSTALL==ebcUseOCR`) 不符合此條件，因此:
- HandShake timeout timer 不會啟動
- HandShake timeout 分支永遠不會觸發
- 只能依賴 `BarcodeDelay` 的一般性 timeout

### Bug: Socket 斷線無告警

`Barcode_StartScan_In()` 中:
```cpp
case 0:
  if(ClientSocket_Shuttle1_A->Active)
  {
      ClientSocket_Shuttle1_A->Socket->SendText(strTrigCMD+"\r");
  }
  break;  // 沒有 else — silent skip
```

Socket 斷線（`Active==false`）時:
1. 不發送觸發指令，不記錄 log
2. `bBarcodeDataSaveReady` 永遠不會被設為 true
3. `bBarcodeNum` 永遠為 false
4. 唯一的退出是 `BarcodeDelay` timeout → retry

### 加劇因素

`ClientSocket_Shuttle1_AError` 事件處理器:
- 只 Close() 不重連
- State Record 中顯示從 09:13:53 開始持續觸發

## 修復方案 (V3.33.903.0 patch)

### Fix A: OCR timeout 保護
在 `BarCode_Sh1.cpp` 和 `BarCode_Sh2.cpp` 的 `DoBarcodeScanInShuttle_1/2()` 中:
```cpp
// BEFORE:
if(BAR_CODE_INSTALL==ebctEtherNetCCD &&
// AFTER:
if((BAR_CODE_INSTALL==ebctEtherNetCCD || BAR_CODE_INSTALL==ebcUseOCR) &&
```
修改位置: state 1150 的 timer start（2 處/檔）+ timeout check（1 處/檔）

### Fix B: Socket 斷線告警/重連
在 `BarCode.cpp` 的 `Barcode_StartScan_In()` 中:
- `Active==false` 時加 `AddCCDCommunicationLog` 記錄
- 嘗試 `ClientSocket->Open()` 非阻塞重連

## 識別重點

當分析 State Record 看到 `BarcodeInShuttle1Task` 或 `BarcodeInShuttle2Task` 停在 **state 1150**:
1. 先確認 `BAR_CODE_INSTALL` 模式（OCR / EtherNetCCD / CCD）
2. 檢查 OCRLOG 的 EventLog 是否有 CCD 回應 ERROR
3. 檢查 State Record thread 是否有 `ClientSocket_*Error` 持續觸發
4. 若為 OCR 模式 + state 1150 hang → 此 case 的 Bug pattern

## CCD 對應表（OCR 模式）

| Socket | Tag | CCD Index | OCR Log 名稱 | Shuttle |
|--------|-----|-----------|-------------|---------|
| Shuttle1_A | 0 | 0 | CCD1 | Shuttle 1 |
| Shuttle1_B | 1 | 1 | CCD2 | Shuttle 1 |
| Shuttle2_A | 2 | 2 | CCD3 | Shuttle 2 |
| Shuttle2_B | 3 | 3 | CCD4 | Shuttle 2 |

OCR 模式 Shuttle 1 的 `iOCRMap={1,0,3,2}`:
- Row A → CCD1 (Shuttle1_B)
- Row B → CCD0 (Shuttle1_A)

---

*Reference for: ht9045-state-record-analysis skill*
