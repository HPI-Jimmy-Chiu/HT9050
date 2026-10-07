# 客戶與計數口徑

本批沒有重新追 V906／V912 的 UPH 客戶分支；既有 `CC_FOREHOPE_NINGBO`、`VTEST`、`IniConfig.bP11RecordUPH` 敘述保留在
[Per-Tray 原稿](legacy/references/uph-existing-realtime-method.md)，不能將一般寫檔／顯示旗標誤分類為客戶專屬功能。

後續核對使用六欄：

| 客戶碼 | 函式／Task | 開關（INSTALL_／USE_／FUNC_CC_） | 行為差異一句話 | 來源（906／912、912-only） | 相關機台 |
|---|---|---|---|---|---|

由 [hpi-customer-features](../../hpi-customer-features/SKILL.md) 找候選，
再追 `CalculateUPH`、計數 writer、site／良率、輸出 consumers；全樹索引命中不等於語意已核對。
