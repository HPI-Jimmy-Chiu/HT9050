# 本批整合main 02ed3e735（20261006）

自己的Skill工作樹已git pull origin main，保留五個本機題。與前次b8ea3a511相比，這次main增加night-loop／交付文件、HT9050 17:59快照與construction待答／todo紀錄；本批核對的wb_serve、WebLogin、WebPageTable、WebWindowRegistry、LogObjects、cMyDB、ht9045_link以及CLAUDE／write-boundary-policy blob均相同。因此本Skill前次靜態查證結果仍有同一來源，沒有宣稱重跑上機驗收。

## E-043第2版仍分清WIP與main

[待答原文](../../../ht9050-construction/references/decisions-pending.md)新增Q-R7、Q-R4'、Q-R6、Q-R8、Q-R10；[todo原紀錄](../../../ht9050-construction/references/todo.md)說v906/st01-e043c2是WIP、仍待整合／gate／SHIP／MR。待答文字的「照預設出貨」不能直接讀成這批main已包含所有新增driver／guard。門、class BusState、Refresh與Alarm的實際狀態需看相應branch／caller；本批不改這些程式、不替Steven或EastSun回答、不把WIP測試當main或實機完成。

## 機台快照

本次鏡像Gerneral.ini新增SafePlcModel=1，GPIB general.ini改LastFile記錄路徑，其他lastdata等檔按機台鏡像歷史保留。這是機台→GitLab的資料更新，沒有安裝到本機system，也未由這個鍵推定PLC／UI／互鎖功能已接線。來源與身份仍讀 [snapshot來源](../../../../../machines/HT9050/snapshot/SNAPSHOT_SOURCE.md)。
