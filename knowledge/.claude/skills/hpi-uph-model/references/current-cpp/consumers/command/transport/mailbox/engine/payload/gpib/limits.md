# GPIB 與 Handler 差異界線

本段只描述 V906 GPIB局部。所選 MyGPIBWrite依 `LastSet.iTesterMode==InterfaceType_Delta_Castle`與其他LastSet欄位分支；不是按HT9050機種判斷，也未查證某客戶當前值。RS232、V912 writer與所有dispatcher未讀完，不能套用GPIB的重試或bool口徑。

HT9050與[其他Handler](../../../../../../../../machines/ht9045.md)仍使用同一[Skill機型路由](../../../../../../../../machines/index.md)，依版本／介面／客戶分流。完整UPH producer／counter／保存、容量／site／校正未閉合；新增一個dispatcher片段與一個完整writer body不增加已完成機型數。

所選來源註解的歷史行號、golden相容或buffer設計主張原樣保存；活文用function／變數，未執行程式、機台或runtime。回[payload界線](../limits.md)與[engine界線](../../limits.md)。
