# 同題版本、機型與客戶界線

| 分流 | 本層局部證據 | 下一步與未查範圍 |
| --- | --- | --- |
| V906共同 | 十二完整定義，preinit／socket建立、五hook callback、非ON_LINE有效介面與Start分支 | PublishSettings／GpibAux、Hub選擇／restart、事件與socket API、thread與例外還原尚未閉合 |
| HT9045／HT9050／其他Handler | 本層所選十二body無MachineTypeChoice或Model分支，使用共同LastSet／TestIF／IniConfig | 不代表全樹無差異；HT9050 FindBridgeWindow與完整上層caller／配置、site與容量沿 [機型樹](../../../../../machines/index.md) 查證 |
| 客戶與設定 | D1權限／I40、D3 I27、D6 OffLineBin、D7 SiteMapping／VTEST等旗標可影響選讀callback；所選body無CUSTOMER_CODE分支 | 不把歷史TSMC／ASM註解當正式客戶部署矩陣；依實際客戶版本與旗標核對 |
| BCB6／V912 | 來源內保留golden／V912註解；同題 [封包版本](../../../versions.md) 的歷史差異仍在 | 本單元沒有重新讀V912對應完整初始化／切換body，不把V906結果套給V912 |
| NI／Sim與SOFT_SIMULTE | D2的IC拒絕macro分支、有效TestType、driver injection三者分開 | ABI／capacity／NI DLL／硬體與實機UPH校正、全producer／consumer及退出契約仍待驗證 |

本次只執行文件與git／hash／引用查核，沒有configure、build、ctest、driver／機台程式或runtime設定變更。測試檔內斷言與來源註解均不等於本次執行通過。

回 [本層索引](index.md)、[前置](preinit.md)、[選擇](selection.md)、[factory界線](../limits.md)。
