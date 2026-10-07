# Handler 共用路徑與版本差異

本子樹在同一[HT9050](../../../../../../machines/ht9050/index.md)／[其他 Handler](../../../../../../machines/ht9045.md) UPH Skill。V906 所選 hub／thread 函式沒有 MachineTypeChoice／CUSTOMER_CODE gate；這不表示所有機台、客戶或作用中介面都會走它。V912 先前所選[WM_COPYDATA 出口](../../flow.md)保持分開，不套用本子樹的 Running／mailbox 生命週期。

已查七完整 body 的本地控制流，沒有證明所有 engine factory、SelectTestType／StopEngine／IsUp／Send caller、鎖與 OS 等待契約、並行／exception／shutdown安全。依[payload](payload/index.md)及[GPIB](payload/gpib/index.md)繼續往下核對返回值。

完整 UPH producer／counter／grid、DB／CSV／SECS保存、每機型容量／site／校正、客戶分派與 S8 仍待查。原文、metadata、裁決及前層來源紀錄保留；靜態查證不稱實機驗證。

回[啟停流程](lifecycle.md)、[mailbox界線](../limits.md)與[來源](../../../../../../resources.md)。
