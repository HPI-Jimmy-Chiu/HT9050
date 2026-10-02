# 2DID / OCR 掃碼異常案例

---

### D01 JAM0460/0461 持續報警（Contact Test / Auto Height 後）
- **問題編號**：待編號
- **影響版本**：V3.33.893.4
- **症狀**：OCR 4-Site (2X2 QUAD) 模式下 `JAM0460` / `JAM0461` 持續報警，無法正常生產
- **根因**：Contact Test 及 Auto Height 流程啟動 Shuttle 掃碼後，內部錯誤旗標 `bCheckCodeError` 未被清除，後續正常生產時殘留的旗標觸發誤判
- **修法**：在 Contact Test / Auto Height 啟動掃碼前，加入 `bCheckCodeError = false` 清除動作
- **預防**：任何啟動掃碼的非正常流程（Contact Test / Auto Height / Teaching）都必須先清 error flag
- **案例**：FMSH (PMLD1361), V3.33.893.4, 修復於 V3.33.903.1 (2026-04-21)
- **提案文件**：[docs/customers/鴻勁興業/884_FMSH/proposals/2026/20260420_Steven_HT9045HW_OCR4Site2DIDError_internal.md](../../../../docs/customers/鴻勁興業/884_FMSH/proposals/2026/20260420_Steven_HT9045HW_OCR4Site2DIDError_internal.md)

---

### D02 Shuttle 1 重複碼誤判 JAM0460（OCR Row 反轉）
- **問題編號**：接續 D01
- **影響版本**：V3.33.903.1（D01 修復後仍殘留）
- **症狀**：OCR 模式下 Shuttle 1 仍偶發 JAM0460
- **根因**：Shuttle 1 的兩支 CCD 實體順序與邏輯順序為反轉關係，但重複碼排除條件使用了硬編碼的物理 row 索引，未跟隨 OCR 模式的反轉映射（`iOCRMap`）
- **修法**：將 Shuttle 1 2DID 掃碼中的重複碼排除條件改為使用映射後的邏輯 Row 索引（147 處整合）
- **預防**：
  1. 涉及 Shuttle row 的判斷一律使用 `iOCRMap` 映射
  2. pre-release-check P8 已加入 OCR mapping 一致性稽核
- **案例**：FMSH (PMLD1361), 修復於 V3.33.903.2 (2026-04-23)
