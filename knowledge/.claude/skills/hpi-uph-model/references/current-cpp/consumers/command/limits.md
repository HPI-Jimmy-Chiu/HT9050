# 版本／機型與未查範圍

同一[HT9050](../../../machines/ht9050/index.md)與[其他Handler](../../../machines/ht9045.md)Skill保留共用與差異。所選四getter／wrapper body沒有MachineTypeChoice或CUSTOMER_CODE gate；完整dispatcher、GetAll、客戶開關與作用中機型仍待查，不能據此推所有機型或客戶都走同指令。

V912保留Big5／BCB原始byte、V906保留UTF-8 pin；本次只有文件，未改W-140原始碼、執行指令、編譯或測平台。四完整文字讀完不代表producer／callback／transport全語意已驗。

grid物件生命週期／鎖、所有caller（含取address／callback）、SendMSG_CMD與訊息碼／transport、GetAll整份欄位與回覆、WebBridge實際頁面、DB／SECS／CSV保存、HT9050計數／容量／site／校正及S8仍待補。沒有C++／build、程式、API／LIVE、網頁、IO、機台或runtime操作。

回[Command入口](index.md)與[來源](../../../resources.md)。

[轉送helper局部](transport/index.md)已補七body與兩宣告；它不閉合所有callback／init caller、SendToEngine／接收端或送達結果。
