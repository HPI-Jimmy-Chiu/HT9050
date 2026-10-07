# CUSTOMER_CODE讀入：路徑、守衛與caller

來源：[V912 database.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/36625f91b2df7fb56b85e832eb99349c67f86a34/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/database.cpp)、[V906 database.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/36625f91b2df7fb56b85e832eb99349c67f86a34/HT9011UC_Cpp_V3.33.906.0/database.cpp)。定位SYSTEM_MODULAR::ReadGeneralIni、CUSTOMER_CODE、MachName、bHandlerModel；與機型／客戶runtime分開核對。

## 兩個來源不能混用

兩版先讀 `D:/GPIB9045/system/general.ini` 的[Version] Model做MachName守衛；CUSTOMER_CODE則用CheckAndReadIniDataGeneral讀INIFileGeneral的[System] CUSTOMER_CODE。[V912 common.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/36625f91b2df7fb56b85e832eb99349c67f86a34/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/common.cpp)與[V906 common.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/36625f91b2df7fb56b85e832eb99349c67f86a34/HT9011UC_Cpp_V3.33.906.0/common.cpp)的OpenGeneralIniFile用asGeneralPath建TIniFile，預設 `D:/HT9045/system/Gerneral.ini`；V906的InitCommonString及初值保留W906_GENERAL_INI_PATH seam。未讀現場檔或環境值，不能推定本機實際路徑。

int版CheckAndReadIniDataGeneral會先ValueExists：缺鍵就WriteInteger預設0，有鍵才ReadInteger。讀入可能補寫鍵，不能拿這條路徑做未備份的「只讀測試」，也不能將預設0說成每台機的實際客戶碼。

## HT9050與其他機型

V912此入口接受9045GPIB、9046GPIB、9046_32GPIB、9045GPIB_12Site、502GPIB、1032GPIB、7080GPIB；V906另接受9050GPIB並保存W906_GpibModel。非白名單設bHandlerModel=false後return，尚未讀此段客戶碼；這是ReadGeneralIni範圍差異，不能外推成全版本支援判定。

## ASE編譯條件與記憶體值

數值已核對兩版MachineType.h：CC_ASE_KaohSiung=936、CC_ASE_KaohSiung_K12=929、CC_HONPREC_QC=0。作用中的ASE_KaohSiung巨集另查建置，不能從客戶數值推定。

| 客戶碼 | 函式／Task | 開關（INSTALL_／USE_／FUNC_CC_） | 行為差異一句話 | 來源（906／912、912-only） | 相關機台 |
|---|---|---|---|---|---|
| CC_ASE_KaohSiung／936；例外比較0／929 | ReadGeneralIni | ASE_KaohSiung defined | 先向同鍵寫936，再讀與規整記憶體；後面的0／929比較不等於保留原磁碟值 | V912與V906此段已核對；實際巨集未查 | 通過各版Model守衛的Handler；V906含HT9050 <!-- review-record:boot-ase-build-on --> |
| CC_ASE_KaohSiung／936 | ReadGeneralIni | ASE_KaohSiung not defined | 讀到936時記憶體CUSTOMER_CODE轉0；此映射敘述沒有把已存在的936回寫為0 | V912與V906此段已核對；完整後續寫者未核對 | 通過各版Model守衛的Handler；V906含HT9050 <!-- review-record:boot-ase-build-off --> |

## 呼叫順序與結果界線

V912有HSys全域物件，SYSTEM_MODULAR建構子呼叫InitialMemory→InitCommonString→OpenGeneralIniFile→ReadGeneralIni；[V912 main.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/36625f91b2df7fb56b85e832eb99349c67f86a34/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/main.cpp)的TfMain建構子另讀同鍵與ASE規整，不能把所有reader併成一次。

V906在database.cpp／database.h保留的明確建構子／宣告是字面#if 0；活動HSys定義不代表它已跑讀入。[wb_serve.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/36625f91b2df7fb56b85e832eb99349c67f86a34/HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp)的main明確先InitialMemory／InitCommonString，再LoadMachineConfig→OpenGeneralIniFile→HSys.ReadGeneralIni；[CMakeLists.txt](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/36625f91b2df7fb56b85e832eb99349c67f86a34/HT9011UC_Cpp_V3.33.906.0/CMakeLists.txt)有database.cpp的ht9045_db、cprod.cpp的ht9045_globals，以及wb_serve的連結組。這是該target的source／caller關係，不宣稱所有執行檔都同路徑。

LoadMachineConfig只有INIFileGeneral==0分支return false；ReadGeneralIni是void，Model不符的return不會由這個呼叫直接變成LoadMachineConfig false。因此不能把函式成功回傳當成Model／客戶設定驗證成功。

ReadGeneralIni先呼叫CustomerFunctionSelect，再ReadLastSetIni；後者又呼叫一次CustomerFunctionSelect，再ReadLastDataFile及其他config項。讀[功能入口](feature-selection.md)並追最後consumer，不能將首次賦值當成最終狀態。存檔後的HS_ExitBtnClick／V912 ExitBtnClick也會再讀，詳見[存檔caller](../customer-persistence.md)。
