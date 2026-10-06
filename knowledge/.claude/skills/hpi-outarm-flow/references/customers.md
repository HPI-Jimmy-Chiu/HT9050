# OutArm客戶條件索引

| 客戶碼 | 函式／Task | 開關（INSTALL_/USE_/FUNC_CC_） | 行為差異一句話 | 來源（906/912,912-only） | 相關機台 |
|---|---|---|---|---|---|
| CC_ASE_KaohSiung | DoOutArm_9045／CheckOutArmDestroyActive | ArmSpeed_File[OutArm].bDevicConfirm | ASE依回吸檢測開關才呼叫Destroy檢查，其他客戶走一般檢查 | V906 dispatcher已核對；912本批未核對 | 依配置，非9050專屬 |
| CC_ASE_SG | MoveOutArmXY_ToFix_Tray_Full／bMoveY | bE90_OutArmFixFullExtraY；TrayArm互鎖另查 | 客戶分支用Tech.iOutArmAuto2X，Y依bMoveY取SafeY或SafeY-15000；其他客戶走一般分支 | V906該函式已核對；原版本詳完整入口，912本批未重驗 | 有Fix退讓需求的原配置，HT9050無Fix不套用 |

其他Bin、Magazine、AOI、客戶碼957與一般配置開關須另追source；未做客戶全量掃描。機型沒有某盤型，不等於該客戶碼一定沒有該功能。
