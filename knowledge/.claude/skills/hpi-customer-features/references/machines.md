# HT9050與其他Handler的分流

| 項目 | 共用方法 | HT9050／其他機型差異 |
|---|---|---|
| 客戶碼來源 | 同版本MachineType.h＋作用中CUSTOMER_CODE | 機型名不能決定客戶碼；當台值未讀取／未改動 |
| 執行入口 | 追caller、MachineTypeChoice／Type_*、Task與INSTALL_／USE_ | 先看所屬主題的機型reference；同名function／同Task不證明相同機構 |
| 客戶功能 | 人工核對完整CC條件和其他開關 | HT9050是否進該分支須查dispatch；其他Handler也不按名字推定 |
| runtime／snapshot | 區分Git快照、作用中檔案、來源時間與單向鏡像 | 本批只讀Git源碼，不套用快照、不修改system／config，不測試機台 |

[Shuttle人工樣板](../../hpi-shuttle-flow/references/customers.md)只核對非Type_HT9050的Do_Auto_SHT1／Do_Auto_SHT2 case 500。HT9050 InSH／OutSH是另外入口，不能套用該表；CC_ASE_M的SHT1／SHT2條件也不能對稱推定。

[Config機型樹](../../hpi-config/references/machines/index.md)與[各主題索引](topics/index.md)負責實際分流。未核對差異保持待補，不寫「同通用」；不把WIP、歷史裁決或機台端改動冒充main已實作。
