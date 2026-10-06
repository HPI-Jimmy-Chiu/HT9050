# 目前客戶名稱查表

唯讀基準main `f57d93f15`，HT9011UC_Cpp_V3.33.906.0/FileRW/HSys.cpp::FileRW_HSys_CustomerName以rgCustomerList文字前三碼比CUSTOMER_CODE，不取ItemIndex。

W906_HSys906Row讓807／808略過V912行，找不到時維持HonPrec；898改用906的AMD_SUZHOU字串。QLE／Infineon／CARSEM分支仍按CC與旗標處理，其餘取第5字至半形空格。這只處理名字查找，HandlerSys UI list仍是原V912產生清單；兩張表不能合稱「已同步名稱」。原895／970全形空格的特殊結果也保留原文件，不在本批改碼。

MachineType.h中CC_PTI定義957，這只證明該版本的符號數值；「某台HT9050使用957」還需對當台snapshot／CUSTOMER_CODE與裁決，不由機型推定。報告端代理商／語言以原表為準，Factory欄位與RunInfo.Factory名稱caller也分開。

新增customer時，golden Big5資料、移植樹UTF-8的產生檔與測試釘點需按版本處理；原workflow保留，但本批未新增客戶／執行generator／跑CustomerName測試。核對上述函式是靜態來源驗證。
