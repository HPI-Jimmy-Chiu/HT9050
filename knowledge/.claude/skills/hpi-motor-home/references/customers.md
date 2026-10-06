# Home 配置與客戶條件

本批沒有直接核對到需要加入的CUSTOMER_CODE條件，不虛構「全部客戶相同」結論。原V899欣銓案例仍保留原日期，不轉成當前客戶功能規則。

| 客戶碼 | 函式／Task | 開關（INSTALL_/USE_/FUNC_CC_） | 行為差異一句話 | 來源（906/912,912-only） | 相關機台 |
|---|---|---|---|---|---|
| 未核對到客戶碼分支；機型／drive條件 | MotorHome case20／40 | W906_HT9050_FULLHOME_ZSAFE | main預設OFF，條件開啟時才接額外ZSafe流程，不是客戶白名單 | 當前V906已核對；912未核對 | HT9050 arm Z |

其他HomeClass Visible／順序、LOAD_Z／LOAD_Y、picker等配置依 [原共用流程](flow/generic/original-entry.md) 核對當前執行版本。
