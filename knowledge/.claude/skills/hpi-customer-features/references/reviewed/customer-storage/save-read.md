# 存檔標記、寫入與再讀

來源：[HSys.gen.inc](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/FileRW/HSys.gen.inc)的 `HS_SaveSystemSet`／`HS_LoaderSystemSet`／`HS_FormShow`／`HS_ExitBtnClick`。前段SaveFlow在[HSys守衛](../customer-web-save/hsys.md)，舊版本／密碼caller差異在[存檔caller](../customer-persistence.md)。

## 標記比寫入早

HS_SaveSystemSet先標記SaveSystemSet，ELAsk答1後立即標記 `SaveSystemSet:write`，之後才開始多個WriteIniDataGeneral呼叫；客戶碼段更在後面。此標記證明已進入同意寫入的流程，沒有逐鍵成功回傳，也沒有證明客戶碼已落盤。

客戶碼段以 `atoi(EL<TEdit>("THandlerSystem","edtCustomerCode")->Text.c_str())`更新全域CUSTOMER_CODE，再以WriteIniDataGeneral寫[System] CUSTOMER_CODE。它讀proxy文字，不讀rgCustomerList.ItemIndex；全域值的更新也不能證明INI寫入成功。這次沒有呼叫任何一項存檔敘述。

## 顯示讀入與活動值

HS_DfmState先有edtCustomerCode.Text="0"；HS_FormShow再呼叫HS_LoaderSystemSet，後者將 `CheckAndReadIniDataGeneral("System","CUSTOMER_CODE",0)`的值給proxy Text，而非直接拿全域CUSTOMER_CODE顯示。該CheckAndRead的缺鍵寫預設行為已在[開機reader](../customer-boot/index.md)記錄，不能當成純讀方法。

SaveFlow看見SaveSystemSet:write後才執行HS_ExitBtnClick與HSTakeSnapshot；HS_ExitBtnClick含HSys.ReadGeneralIni與HS_SaveSafeDoorSet。ReadGeneralIni的編譯條件與活動客戶碼處理沿用[開機查證](../customer-boot/index.md)，不能從proxy文字或磁碟鍵推定最後consumer看到的CUSTOMER_CODE。這次只是順序／直接caller查證，沒有完成所有內層副作用、再次賦值、磁碟回讀或交易原子性分析。

HT9050與其他機台共用這套追蹤方法；機型／客戶／runtime、權限與實際讀取者必須分流。原人工差異列及846個未決保持，不從共用函式名稱推定機台實際設定相同。
