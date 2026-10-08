# 版本、保存與未完成項

[AnsiString.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.cpp)；[AnsiString.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.h)。完整選定函式、模板與歷史裁決正文保存在 [manifest](source-manifest.json)。

- 本單元只靜態核V906 vclcompat AnsiString selected數值／printf／串接body，39原文去重；function存在不代表caller或runtime執行。
- HT9050與其他Handler同題共同shim由此入口分流；實際機型、客戶、V912／V899與runtime設定依 [機型樹](../../../../../../machines/index.md) 判別。來源沒有MachineTypeChoice或Type_HT9050分支，不能據此宣稱所有機台行為相同。
- 新引用golden依Steven最新裁決讀ht9045_913最新main；main與V3.33.913.0 tag refs曾不同，main為準。本單元沒有新913 RTL對照；3段歷史裁決、0618 header metadata與相容入口保存，不改寫成913驗證。舊裁決引用行號只在manifest原文，活文件用function／變數定位。
- 已完成assignInt、Trim、AnsiString(int)、str與bytes單元不重算；39摘錄包含7cpp、29inline及3歷史comment，沒有新增完整header census或canonical主題。
- 未閉合：完整ToInt／ToDouble／sprintf callers、printf格式與型別、unsigned long隱式多載、locale／errno／range／ABI、allocation與第二次vsnprintf錯誤、BCB／913實際差異及UPH／S8實機。正文宣稱mirror BCB6 exactly仍是歷史目的，不當本輪驗證結論。
- !348字串三單元29檔屬前批依賴；新工作分支包含它以保留引用，只有本單元新檔計入新批。前批未整合時不再送重複範圍MR；獨立整理、ordinary checkpoint照常進行。

回 [入口](index.md)、[bytes界線](../bytes/limits.md)、[caller樹](../../index.md)。
