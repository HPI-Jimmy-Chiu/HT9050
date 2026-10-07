# 機型、版本與剩餘

同題整合[HT9050](../../../machines/ht9050/index.md)與[其他Handler](../../../machines/ht9045.md)，不另建技能鏡像。這次只查V906 `WebBridgeTags.cpp`的本地UPH consumer；所選三body沒有 `MachineTypeChoice` gate，但不能據此推每台HT9050／9045／9046皆走同caller或同表格。

V912計算／UI在[既有C++對照](../../versions.md)；此處沒有把V906 snapshot layer套到V912，更未比較V912全UI。HT9050 [Loader專屬dispatch](../../state/loader/dispatch/index.md)與此grid顯示之間的計數／計算鏈仍未閉合；HTML離線scheduler／animation不是這個UPH consumer。

已讀三完整函式文字、兩sentinel及一call區段，只查本地UPH條件／包裝；其他index／version語意、cust源頭、所有caller、stage helper／snapshot輸出、web頁實際呈現、grid有效生命週期／thread、DB／SECS／CSV、容量site與校正待查。沒有C++／build、API、LIVE、頁面執行、IO、機台或runtime變更。

只記source與body／片段hash，不把輸出tag存在、valid參數或字串轉換當送達／持久化／實機產能驗證。回[WebBridge入口](index.md)及[來源](../../../resources.md)。
