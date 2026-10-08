# 版本、機型與剩餘查證

本節點屬V906 C++移植樹及同repo web來源；HT9050與其他Handler共用這些元件的事實，只到選定function／表列／HTTP key，不外推成每種機型、客戶及runtime都走相同路徑。

| 對象 | 本段證據／界線 |
| --- | --- |
| HT9050／其他V906 Handler | GPIB raw快照、channel副本、頁面TSerialPoll列及選定browser函式；完整初始化、profile窗口建立、hardware NI／Sim及customer選路未閉合 |
| TTL／RS232／TCPIP | HomeKeyOfTestType對0／2／3的映射保存；未因此驗證各engine的builder／命令／通信或HomeCompose |
| 客戶流程 | 前段StartBridgeProgram／CloseGpibProgram保留各customer與早退條件；本段不重新聲稱客戶全部一致或實機已成功 |
| V912／V899 golden | 保留既有 [機型分流](../../../../../machines/index.md)；本段未新做BCB6量產樹、golden UpdateLed／ClearAllFlag或客戶runtime比較 |
| wb_serve／browser實際運行 | 只查所選函式／安裝區段、whole-file byte hash及literal缺席；沒有build／fixture／API／UI／機台量測 |

仍待完整HTTP helper／路由與transport、hook生命週期及各HMI回報、PageTableTick與window registry互鎖、HomeCompose、string／TStrings／locale／ABI、cinitial等其他main來源差異、V912與客戶／機型矩陣及UPH／S8實機驗證。

原文與metadata保存、hash／引用／錨點通過只證明文件保存及選定靜態證據；歷史裁決註解、時序註解與fixture不能變成本輪runtime驗證。
回 [入口](index.md) 與 [前段版本界線](../20261008-online-pad/versions.md)。
