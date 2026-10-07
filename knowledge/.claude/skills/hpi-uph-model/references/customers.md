# 客戶與計數口徑

首次入口交付未追 V906／V912 的 UPH 客戶分支；下方另記後續指定 body 查證。既有 `CC_FOREHOPE_NINGBO`、`VTEST`、`IniConfig.bP11RecordUPH` 敘述保留在
[Per-Tray 原稿](legacy/references/uph-existing-realtime-method.md)，不能將一般寫檔／顯示旗標誤分類為客戶專屬功能。

後續核對使用六欄：

| 客戶碼 | 函式／Task | 開關（INSTALL_／USE_／FUNC_CC_） | 行為差異一句話 | 來源（906／912、912-only） | 相關機台 |
|---|---|---|---|---|---|

由 [hpi-customer-features](../../hpi-customer-features/SKILL.md) 找候選，
再追 `CalculateUPH`、計數 writer、site／良率、輸出 consumers；全樹索引命中不等於語意已核對。

## 20261007 指定 body 的局部客戶差異

來源為 [目前C++輸出分流](current-cpp/outputs.md) 的兩版CalculateUPH指定body，pin8f213da4f。僅caller／guard靜態證據，不驗輸出成功或現場機型。

| 客戶碼 | 函式／Task | 開關（INSTALL_／USE_／FUNC_CC_） | 行為差異一句話 | 來源（906／912、912-only） | 相關機台 |
| --- | --- | --- | --- | --- | --- |
| CC_KYEC_LEE | CalculateUPH | IniConfig.bP11RecordUPH（一般閘；此處無INSTALL_／USE_／FUNC_CC_） | P11下呼叫LotRecordUPH，傳grid時間與iUPH；完整writer未查 | 906／912指定body已查 | Handler；作用中caller／機型待查 |
| CC_FOREHOPE_NINGBO | CalculateUPH | IniConfig.bP11RecordUPH（一般閘） | 保存iLoaderCount以iTrayCount名傳入、site用GetSiteCount(false)，不由名稱認定實際盤數 | 906／912指定body已查 | Handler；容量／site／分派待查 |
| CC_ASE_KaohSiung | CalculateUPH | IniConfig.bG10ShowImmediateUPH（顯示閘） | 關閉ImmediateUPH時StatusBar寫入在V912活body；V906文字寫入仍在#if 0 | 906／912指定body差異已查 | Handler；完整UI／現場顯示待查 |

VTEST是一般設定，不新增客戶碼；其V912專用writer受P11與前兩客戶順位限制。這三列只在UPH主題增加局部證據，不覆寫S8的13人工列／6382候選／846未決。
