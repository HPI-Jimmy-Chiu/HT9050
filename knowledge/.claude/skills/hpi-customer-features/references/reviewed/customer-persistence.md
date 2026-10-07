# CUSTOMER_CODE的存檔caller與版本差異

這是S8後續局部靜態查證，來源釘住09d926ab8；下列HandlerSys、HSys與WebLogin檔在推前main ef41839d4及合併後程式來源均未改。原6筆讀寫候選保留，字面停用呼叫另核對；未呼叫存檔API、登入初始化或修改任何runtime。

## V912：表單存檔

[V912 HandlerSys.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/HandlerSys.cpp)的THandlerSystem::SaveBtnClick呼叫SaveSystemSet；後者先問是否儲存，非IDYES立即return。通過後，客戶碼段以 `atoi(edtCustomerCode->Text.c_str())` 寫CUSTOMER_CODE記憶體，並 `WriteIniDataGeneral("System", "CUSTOMER_CODE", CUSTOMER_CODE)`，再呼叫fMain->InitialSuperVisorPassword(CUSTOMER_CODE)。

這不是從rgCustomerList.ItemIndex取得數值。前面的UI輸入驗證與完整存檔副作用需各自查證；函式有WriteIniDataGeneral敘述不等於每次存檔成功，也不能當成新機型預設值。

## V906：PageDesc到存檔函式

[V906 FileRW/HSys.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/FileRW/HSys.cpp)實際include HSys.gen.inc。kPage把HSys／THandlerSystem的save callback設為SaveFlow，PageRegistrar登記；FileRW/_editlist_sources.cmake與CMake的收錄已在[客戶名稱caller](customer-names/index.md)核對，不能僅看到generated檔就推定caller。

SaveFlow先看HS_ModelReadError與W906_HeaterMixSaveCheck，任一擋下即return；然後HSApplyOverlapRule、HS_SaveBtnClick。[HSys.gen.inc](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/FileRW/HSys.gen.inc)的HS_SaveBtnClick呼叫HS_SaveSystemSet；ELAsk答1才標SaveSystemSet:write並繼續，其他答案return。CUSTOMER_CODE段以具名proxy的edtCustomerCode.Text經atoi更新全域值，並寫Gerneral.ini的[System] CUSTOMER_CODE。

HSys的callback登記與這段函式call graph已核對；HTTP權限、完整EditPage／Cowner契約、磁碟寫入結果與現場設定未作全量核對。SaveSystemSet:write是流程標記，不能取代各檔寫入成功證據。SaveFlow只有看見此標記才跑HS_ExitBtnClick的資料半段與HSTakeSnapshot，不能把答NO的畫面訊息當成已存檔。

## 呼叫保存與實際效果

V912存檔後有InitialSuperVisorPassword的活動呼叫；V906同一段以ELTodo記缺口，原呼叫在字面#if 0中。[WebLogin.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp)另有static InitialSuperVisorPassword與WebLogin_Boot的活動呼叫，並會受LastSet.szSupervisor覆寫；這只證明boot路徑，不能改稱存檔時也會重新初始化。此處只記caller差異，不複製密碼表內容，也不更動登入實作。

## HT9050與其他機台

以上CUSTOMER_CODE鍵與表單／proxy的查證方式共用；實際碼、機型、權限、runtime檔案與磁碟owner另按現場確認。同一數值在版本間的符號／顯示名稱仍讀[名稱差異](customer-names/index.md)，報告地區／代理商按[權威](../authority.md)，不能由此存檔流程推定客戶功能或機台配置。

CUSTOMER_CODE開機讀入／CustomerFunctionSelect入口已補[局部查證](customer-boot/index.md)；完整EditPage入口守衛、callee／consumer與磁碟效果仍待核對。本頁不宣稱全部reader／writer已盤點，也不消除原846個未決候選。

20261007補[Web接收／PageSave與HSys守衛](customer-web-save/index.md)：接收時owner、主迴圈／頁面守衛與callback已作局部靜態核對；HTTP全部入口、reauth／proxy／副作用與磁碟效果仍未全量驗證。原本頁09d926ab8的來源／日期與保存內容保持。

20261007續補[proxy／INI寫入局部查證](customer-storage/index.md)：核對edtCustomerCode真正父鏈、文字型別、標記先於寫入及V906 void包裝界線。原12列／6382候選／846未決與舊來源保持；仍未驗證實際磁碟成功、所有proxy寫者／最後consumer或完整交易。

20261007續補[reauth頁面分流／清除與ack局部查證](customer-reauth/index.md)：HSys不在PointOf對照表，沒帶答案仍需原存檔閘。原12列／候選／舊來源保持；完整登入、owner時序、所有結果寫者與磁碟效果仍未完成。

20261007續補[owner／queue局部時序](customer-owner/index.md)：核對接收owner、控制權變更、connId／ticket、queue／共用guard與所選存檔分支。完整HTTP／carry／callee／owner競態仍未完成；不由接收通過或ack推定持續控制權與磁碟成功。
