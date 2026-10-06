# InArm客戶條件索引

| 客戶碼 | 函式／Task | 開關（INSTALL_/USE_/FUNC_CC_） | 行為差異一句話 | 來源（906/912,912-only） | 相關機台 |
|---|---|---|---|---|---|
| CC_ASE_KaohSiung | DoInArm_9045／CheckInArmDestroyActive | ArmSpeed_File[InArm].bDevicConfirm；IniConfig.bD44CheckIndexICDestroy | ASE路徑先看回吸檢測開關與iIndexTakeDeviceChk2／iArmTask等條件，其他客戶走一般檢查 | V906 dispatcher已核對；912本批未核對 | 依配置，不是9050專屬 |
| CC_TSMC_TAINAN | SetInArmUseSuckToHasTrySuckIC | USE_PICKER_COUNT、Prod.iSiteMap | 多吸嘴TrySuck標記另看iSiteMap；單Picker在前段直接標(0,0)，不能跨分支套用 | V906 ainarm2.cpp已核對；912本批未核對 | 依Picker／Kit配置 |

ASM、SiteMode、Pitch與一般IniConfig開關不是自動等同客戶功能；本表僅列當次已核對的函式，其他條件看完整原文與來源碼，不宣稱全量盤點。
