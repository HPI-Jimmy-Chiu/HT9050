# 客戶碼、UI名稱與Factory來源（局部核對）

同題整合HT9050與其他Handler的共同查證方式。CUSTOMER_CODE與MachineTypeChoice是不同維度；實際機台客戶／runtime本批未讀取。符號、UI文字、Factory及報告地區／代理商分別查權威，避免數值相同就合併名字。

來源定義：[v912 MachineType.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/MachineType.h)、[v906-cpp MachineType.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/MachineType.h)；UI原表：[v912 HandlerSys.dfm](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/HandlerSys.dfm)、[v906-cpp HandlerSys.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/HandlerSys.cpp)的kCustomerListItems與[v906-cpp FileRW/HSys.gen.inc](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/FileRW/HSys.gen.inc)。

| 客戶碼 | 函式／定位 | 開關／條件 | 行為差異 | 來源版本 | 相關機台 |
|---|---|---|---|---|---|
| 807 | GetCustomerName／W906_HSys906Row／FileRW_HSys_CustomerName | CUSTOMER_CODE=807；名稱列表存在且有相符列 | V912定義CC_TFAMD_M，原列可取TFAMD_M；V906 MachineType.h未定義此符號，W906_HSys906Row也略過807；若沒有其他匹配便回HonPrec | V912對照V906 Cpp | HT9050與其他Handler都先核對作用中版本，不由UI殘留列判斷支援 |
| 808 | 同上 | CUSTOMER_CODE=808；名稱列表存在且有相符列 | V912定義CC_AMD_US，原列可取AMD_US；V906未定義此符號且名稱lookup略過808；無匹配回HonPrec | V912對照V906 Cpp | 同上；沒有符號定義不代表所有等效業務行為不存在 |
| 898 | MachineType.h／W906_HSys906Row／FileRW_HSys_CustomerName | CUSTOMER_CODE相符、proxy／列表存在 | V912為CC_TFAMD_SUZHOU；V906為CC_AMD_SUZHOU，lookup將generated原TFAMD_SUZHOU文字覆寫為AMD_SUZHOU後再取名 | V912對照V906 Cpp | 同一數值的版本名稱差異，不由機型或舊UI字串推定客戶 |
| 982 | MachineType.h／rgCustomerList／kCustomerListItems | 程式符號、列表文字與報告權威分別核對 | V912符號CC_AMD_SG，V906符號CC_AMD_M；兩版已讀UI列仍是AMD M／AMD Malaysia，不能據此自動改報告地區或合併別名 | V912對照V906 Cpp | HT9050與其他Handler共用權威分層；客戶現場身分另確認 |

<!-- review-record:customer-name-807 -->
<!-- review-record:customer-name-808 -->
<!-- review-record:customer-name-898 -->
<!-- review-record:customer-name-982 -->

## 名稱解析與caller

[v912 HandlerSys.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/HandlerSys.cpp)的THandlerSystem::GetCustomerName以每列前三字元atoi比CUSTOMER_CODE，預設HonPrec，匹配後取第5字元起到第一個半形空白之前。QLE／IFXTH／Carsem另有特名分支；不按ItemIndex取得客戶碼。main.cpp有RunInfo.Factory及labFactory.Caption的賦值caller，沒有把名稱反推成CUSTOMER_CODE。

[v906-cpp FileRW/HSys.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/FileRW/HSys.cpp)的FileRW_HSys_CustomerName先找THandlerSystem／rgCustomerList proxy，找不到回HonPrec；有列表時經W906_HSys906Row過濾／覆寫後再比CUSTOMER_CODE並取名稱。HSys.gen.inc保留807、808及898的V912列表文字，但lookup會過濾／覆寫，這與UI文字不是同一層。上述807／808結果須同時符合實際列表和無其他匹配條件。

[v906-cpp tools/wb_boot_factory.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/tools/wb_boot_factory.cpp)的W906_Boot_RunInfoFactory呼叫FileRW_HSys_CustomerName，寫RunInfo.Factory及labFactory.Caption；[v906-cpp tools/wb_serve.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp)有boot caller。[v906-cpp FileRW/_editlist_sources.cmake](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/FileRW/_editlist_sources.cmake)列FileRW/HSys.cpp，[v906-cpp CMakeLists.txt](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/CMakeLists.txt)合成W906_FILERW_SRC供wb_serve。[v906-cpp FileRW/_fallback.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/FileRW/_fallback.cpp)另有HonPrec fallback，不能將wb_serve查證套給所有binary。

## 報告資料權威

[資料權威](../../authority.md)及[客戶碼表指引](../../../../make-report-skill/references/customer-code-table/customer-code-table.instructions.md)才決定報告地區／代理商／語言；這份程式名稱核對不覆寫報告表，也不從名稱猜地區。未修改CUSTOMER_CODE、MachineType.h、UI／generator或runtime。
