# Shuttle 客戶功能（S2 樣板列）

只列本次手工核對的 V906 殘料重試分支，不是全樹客戶清單。V912 未比對，沒有列可據此稱為 912-only。

| 客戶碼 | 函式／Task | 開關（INSTALL_／USE_／FUNC_CC_） | 行為差異一句話 | 來源（906／912） | 相關機台 |
|---|---|---|---|---|---|
| CC_ASE_KaohSiung、CC_ASE_KaohSiung_K12、CC_SCS、CC_AMKOR_Japan、CC_ASE_SG | Do_Auto_SHT1／Do_Auto_SHT2：500 | CUSTOMER_CODE；另有 thread／AccessLevel 條件 | 此客戶列表只給 K_RETRY，不能套一般高權限分支的 K_SKIP | V906 acarry.cpp 已核對；V912 未核對 | 非 Type_HT9050 的 SHT1/SHT2 分派，內部機型仍要核對 |
| CC_ASE_M | Do_Auto_SHT1：500 | CUSTOMER_CODE | SHT1 retry-only 列表含 ASE_M；SHT2 同段沒有，不可對稱推定 | V906 acarry.cpp 已核對；V912 未核對 | 同上；僅本列 SHT1 已核對 |

來源：[acarry.cpp](../../../../HT9011UC_Cpp_V3.33.906.0/acarry.cpp)::Do_Auto_SHT1／Do_Auto_SHT2 case 500 的 ShowErrorMessage 與 thread 分支。
HT9050 InSH／OutSH 是另外兩個入口，本表不自動適用；全樹掃描與其餘客戶列留給 S8。
