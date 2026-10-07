# V906 C++與Web分析

ElaCore／ElaTables／ElaHub／ElaService、報表／排程／上傳與OEE的原移植紀錄、分支／日期、已編譯與待ctest／上機界線見[原正文](../ela/original-entry.md)。[Web轉換計畫](../ela/references/ela-web-conversion-plan.md)是原時間點計畫，後續實作不由該頁的「未動工」推論。

本次main2db局部核對W906_ElaStart、Hub::Entry、SendCommand_EventLog及W44排程函式；Source存在／呼叫陳述與完整binary／現場鏈分開，見[來源查證](../runtime/source-review.md)。沒有重新跑gate、ctest、HTTP或FTP，也未查全wb_serve啟動／停止掛鉤與所有客戶寫檔caller。

V906為UTF-8／C++移植；HT9050與其他Handler共用主題但版本、Model、CUSTOMER_CODE、SOFT_SIMULTE組態與runtime需按實際資料核對。模擬組態不能直接推論不連FTP，原W36決定與實作來源分開記錄。
