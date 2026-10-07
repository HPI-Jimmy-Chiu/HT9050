# 輸出與客戶分流

來源：[V906 ainarm9045.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/8f213da4f7c211ecdf23e6f88493c1c918f52996/HT9011UC_Cpp_V3.33.906.0/ainarm9045.cpp)、[V912 ainarm9045.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/8f213da4f7c211ecdf23e6f88493c1c918f52996/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/ainarm9045.cpp)，定位CalculateUPH的IniConfig.bP11RecordUPH／bVTESTFunction／bEnable_SECS_GEM／CUSTOMER_CODE。

| 路徑 | V906指定body | V912指定body |
| --- | --- | --- |
| MyDBIUPH | 算出iUPH後呼叫；DB結果未驗 | 同一位置；DB結果未驗 |
| P11＋KYEC_LEE | 呼叫LotRecordUPH，傳grid時間與iUPH | 同一caller表達式；完整writer未查 |
| P11＋FOREHOPE_NINGBO | 保存iLoaderCount作iTrayCount、GetSiteCount(false)作iSiteCount，傳專用writer | 同一caller口徑；site helper／writer未查 |
| P11＋VTEST＋前兩客戶未命中 | 沒有RecordLotUPH_For_VTEST分支，落剩餘else | 呼叫專用writer，傳原elapsed、count及iShtRow*iShtCol |
| P11其他else | bCloseExcelflag／finishflag在#if 0 | 修改兩flag；不等於CSV保存成功 |
| SECS | 填tsUPH、啟用時呼叫UPHRecordEnd；consumer未查 | 同位置，不由呼叫推定送達 |
| StatusBar | ASE_KaohSiung／ImmediateUPH條件仍有，文字寫入在#if 0 | ASE關閉ImmediateUPH時清一欄並寫UPH；其他客戶直接寫，完整UI未查 |

V912的VTEST專用輸出受P11與前兩客戶else-if順位限制；一般VTEST顯示三欄不等於一定走writer。V906同名UI／旗標存在也不能說與V912輸出相同。

[客戶表](../customers.md) 補三個真正CUSTOMER_CODE分支；VTEST／P11一般設定不另造客戶碼。未讀現場旗標、寫檔、DB或SECS API，沒有驗證實際保存／送達。
