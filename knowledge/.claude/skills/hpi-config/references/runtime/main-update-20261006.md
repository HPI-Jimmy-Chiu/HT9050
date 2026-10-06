# 設定層整合main 02ed3e735（20261006）

自己的Skill工作樹已pull最新main；此次HT9050 17:59機台鏡像中，D_HT9045_system/Gerneral.ini新增SafePlcModel=1，D_GPIB9045_system/general.ini只改LastFile的log路徑；新增的Gerneral.ini.bak_plcmodel是機台鏡像備份。不要把GPIB log路徑變更當Model變更，不由新鍵值推定PLC硬體／guard已生效；本批未讀寫runtime或裝這些檔。

前次核對的cprod.cpp、FileRW/HSys.cpp、wb_serve.cpp與fLotInfo.cpp來源blob相同，Config／Recipe／CustomerName／Cowner／LotInfo caller敘述仍依原基準。相關原文與metadata保持，不把現在快照值寫回舊歷史表。

[construction待答](../../../ht9050-construction/references/decisions-pending.md)和[todo](../../../ht9050-construction/references/todo.md)的E-043第2版仍標v906/st01-e043c2 WIP、gate／SHIP／MR未完成，不能把它等同main的新實作。實際機台身份與同步限制看 [snapshot來源](../../../../../machines/HT9050/snapshot/SNAPSHOT_SOURCE.md)及AGENTS；本批不更動snapshot、執行同步apply或現場設定。
