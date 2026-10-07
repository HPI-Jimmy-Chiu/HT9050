# CustomerFunctionSelect：入口與直接賦值差異

來源：[V912 cprod.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/36625f91b2df7fb56b85e832eb99349c67f86a34/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/cprod.cpp)、[V906 cprod.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/36625f91b2df7fb56b85e832eb99349c67f86a34/HT9011UC_Cpp_V3.33.906.0/cprod.cpp)。兩版此函式與ReadGeneralIni中的caller都沒有字面#if 0包覆；未評估所有實際建置巨集與整個consumer graph。

## 共用入口與設定層

CustomerFunctionSelect先呼叫InitialCosFunction，再按IniConfig.bKoreaFunction／bVTESTFunction／bSingaporeFunction／bSPILFunction／bMaximFunction／bSIGURDFunction等呼叫各功能函式，另受USE_AUTO_RETEST、REAL_TIME_CCD、ATC_SYSTEM、TestIF_File等設定影響。這些設定或安裝條件不是各自獨立的客戶碼功能；巢狀callee與最後consumer仍需查證。

ReadLastSetIni本身也在讀LastData與後續config項前呼叫CustomerFunctionSelect。這次只記實際順序，不說「全部config讀完才依客戶碼套用」；要判斷運行中的IniConfig／CosFunction值，須查後續讀取與寫者。

## 客戶碼局部列

[V912 MachineType.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/36625f91b2df7fb56b85e832eb99349c67f86a34/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/MachineType.h)及[V906 MachineType.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/36625f91b2df7fb56b85e832eb99349c67f86a34/HT9011UC_Cpp_V3.33.906.0/MachineType.h)均定義CC_XINYUN=781。

| 客戶碼 | 函式／Task | 開關（INSTALL_／USE_／FUNC_CC_） | 行為差異一句話 | 來源（906／912、912-only） | 相關機台 |
|---|---|---|---|---|---|
| CC_XINYUN／781 | CustomerFunctionSelect的SPILFunction後分支 | IniConfig.bSPILFunction==true | 兩版直接設bShowFormByInitPos=false；V912此分支另設6個IniConfig／CosFunction值，V906此分支沒有這6個賦值 | V912／V906函式此分支已核對；不能改稱6功能全樹912-only | 依版本caller與實際SPIL開關核對；HT9050與其他Handler同題 <!-- review-record:boot-xinyun-direct-assignments --> |

V912另直接設CosFunction.bATC32UseTJMode=false、IniConfig.bFTBin2RTBin=true、CosFunction.bUseHeadContactCount=true、CosFunction.bLoginASECL=true、CosFunction.b2DUseSubJobFunction=false、CosFunction.bFTPFunction=true。兩版分支之前都有SPILFunction呼叫；未核對它與其他寫者時，不能由此推論V906運行中的6值為相反或未支援。

## 其他版本差異，不併成客戶碼列

V912把CosFunction.bFullTrayAlarmAfterUnloadEnd設為fAGV->IsSPIL_AMR()或IsTFAMD_AMR()；V906此處只用IsSPIL_AMR()。這是直接賦值條件差異，Is*回傳與其他寫者未全量核對，不能推定當前機台AMR行為。

V912尾端會依ATC_SYSTEM等調整fTemp_Set的rgIndexHeatMode UI；V906同段字面#if 0，不能把保留的VCL設定區段當成活動Web UI。當前Web呈現另讀所屬WebHMI主題。

未追完所有InitialCosFunction／各客戶功能callee、SCK_ART等其他caller、機型dispatch、HTTP／owner或846個原未決候選。本頁不宣稱全量功能表完成，也沒有改他人已維護的主題客戶表。
