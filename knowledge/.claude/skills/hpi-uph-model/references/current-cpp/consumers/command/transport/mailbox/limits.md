# Handler 共同流程與版本界線

這段在同一[HT9050](../../../../../machines/ht9050/index.md)／[其他Handler](../../../../../machines/ht9045.md)UPH Skill下；V906採hub／SyncMailbox，V912先前所選出口是[WM_COPYDATA](../flow.md)，不能把mailbox timeout套給V912。所選七body沒有MachineTypeChoice／CUSTOMER_CODE gate，不表示所有機台或客戶都走相同作用中介面。

所選文字／宣告與原稿保存，舊golden／20260926設計註解作歷史來源；本批沒有重裁thread隔離、即時性、nested Send絕不死鎖或timeout安全性。只描述pin的本地分支，不宣稱完整並行語意已驗。

待查：engine side handler／OnHandlerMessage、thread Loop／Running生命週期、所有SetHandler／Reset／Send caller、啟動／切換／shutdown與鎖契約、request例外及OS等待失敗、payload結構／編碼、接收端和傳送結果、UPH producer／counter／grid／DB／CSV／SECS保存、每機型容量／site／校正，以及S8。

未執行C++／build、程式、指令、server／engine、API／LIVE、網頁、IO、Home、機台或runtime。回[本地流程](flow.md)、[上游界線](../limits.md)與[來源](../../../../../resources.md)。
