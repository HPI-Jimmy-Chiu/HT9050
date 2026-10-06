# Tray 客戶條件

按原文來源記錄，不由名稱推論906／912共有。

| 客戶碼 | 函式／Task | 開關（INSTALL_/USE_/FUNC_CC_） | 行為差異一句話 | 來源（906/912,912-only） | 相關機台 |
|---|---|---|---|---|---|
| CC_SCC | DoAutoColor／DoAutoEmpty Receive | CUSTOMER_CODE、bIsCatchingFromBuffer | 原案例退盤進行中不可Init，完整交棒完成後才清帳；修正版只暫緩尚未開始的退盤 | [CatchTray 原文](flow/catch-tray/original-entry.md)，含V908.16案例與V912.1修正，當前906未核對 | 原HT9045WA案例 |
| CC_SIGURD_ChungXing／CC_SIGURD_PeiXing／CC_UTAC_TW | DoSortingBinTray | FUNC_CC_*、bP27、bSortingBinTraywhenCleanOut | 原版啟用CleanOut IC整盤 | [Sorting 原文](sorting/original-entry.md)，V899，906／912未核對 | 原文件HT9045系列 |
| CC_SPIL_HsinChu | DoSortingBinTray | FUNC_CC_SPIL_HsinChu、bSortingBinTrayWhenTrayFeed | 原版啟用TrayFeed模式，CleanOut條件被mark | [Sorting 原文](sorting/original-entry.md)，V899，906／912未核對 | 原文件HT9045系列 |

其他條件依各原文章節追函式／Task，HT9050尚未核對的客戶功能不列為共用已啟用。
