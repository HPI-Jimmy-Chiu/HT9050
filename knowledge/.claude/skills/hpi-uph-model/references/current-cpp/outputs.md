# 輸出與客戶分流

來源：[V906 ainarm9045.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/8f213da4f7c211ecdf23e6f88493c1c918f52996/HT9011UC_Cpp_V3.33.906.0/ainarm9045.cpp)、[V912 ainarm9045.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/8f213da4f7c211ecdf23e6f88493c1c918f52996/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/ainarm9045.cpp)，定位CalculateUPH的IniConfig.bP11RecordUPH／bVTESTFunction／bEnable_SECS_GEM／CUSTOMER_CODE。

| 路徑 | V906指定body | V912指定body |
| --- | --- | --- |
| MyDBIUPH | 算出iUPH後呼叫；DB結果未驗 | 同一位置；DB結果未驗 |
| P11＋KYEC_LEE | 呼叫LotRecordUPH，傳grid時間與iUPH；兩版writer本體相同 | 同一caller表達式；共用寫檔結果未驗 |
| P11＋FOREHOPE_NINGBO | 保存iLoaderCount作iTrayCount、GetSiteCount(false)作iSiteCount，呼叫的指定writer是空本體 | 同一caller口徑；writer有六欄每日檔，site helper／保存結果未驗 |
| P11＋VTEST＋前兩客戶未命中 | 沒有RecordLotUPH_For_VTEST分支，落剩餘else | 呼叫七欄每日檔writer，傳原elapsed、count及iShtRow*iShtCol；保存結果未驗 |
| P11其他else | bCloseExcelflag／finishflag在#if 0 | 修改兩flag；不等於CSV保存成功 |
| SECS | 填tsUPH、啟用時呼叫UPHRecordEnd；consumer未查 | 同位置，不由呼叫推定送達 |
| StatusBar | ASE_KaohSiung／ImmediateUPH條件仍有，文字寫入在#if 0 | ASE關閉ImmediateUPH時清一欄並寫UPH；其他客戶直接寫，完整UI未查 |

V912的VTEST專用輸出受P11與前兩客戶else-if順位限制；一般VTEST顯示三欄不等於一定走writer。V906同名UI／旗標存在也不能說與V912輸出相同。

[客戶表](../customers.md) 補三個真正CUSTOMER_CODE分支；VTEST／P11一般設定不另造客戶碼。未讀現場旗標、寫檔、DB或SECS API，沒有驗證實際保存／送達。

20261007 後續 [CSV writer 子樹](writers/index.md) 另釘 pin06fb64e54 的 writer／common／V912 FileInfo body；前一段 pin8f213da4f 的 caller 證據保留。兩pin選讀 ainarm blob 相同，空stub／換行／目錄失敗界線已補，完整輸出consumer與實際結果仍待查。
