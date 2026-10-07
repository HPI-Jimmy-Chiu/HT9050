# 20261007 最新來源盤點與既有查證分界

本輪已整合 main `ea5b206117c4bd3f5ede135a4ef8ead179f8ceb9`；[摘要與來源池](versions/main-check-ea5b20611.json) 保留新 scanner／Git blob、詞法候選差異與人工池變更。舊掃描、body manifest、13人工列與裁決正文都保留各自來源日期。

## 全樹候選與局部證據

唯讀 scanner 掃描 V906 C++ 2027檔／V912 717檔，共2744檔；較前一份2db盤點多6檔。仍有6382詞法候選；忽略blob後相對原341cea與前一份2db的候選增0／減0。原6382ID、846未決與13人工列保持；檔案或候選數相同不能證明機台行為相同。

reader／binary storage／text-book 的本批來源池 blob 與所選 pin相同。最新其他來源變更逐池記錄：原人工池 csystem／wb_serve／CMakeLists，boot池database／wb_serve，owner／reauth／web-save池wb_serve。這些舊manifest仍明確釘舊版，沒有把它們覆寫或稱作所有最新body已重審。

## 推前看到的相關更新

| 版本／function定位 | 本輪差異核對 | 適用界線 |
| --- | --- | --- |
| [V906 database.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/ea5b206117c4bd3f5ede135a4ef8ead179f8ceb9/HT9011UC_Cpp_V3.33.906.0/database.cpp)::SYSTEM_MODULAR::ReadGeneralIni；Enable_PLCSafety_IO／iSafePlcModel | 較boot原pin新增SafePlcModel讀取；從2db後此檔沒有再變 | 仍是PLC型號設定，非客戶碼；原客戶碼局部列不升格為現場組態或全部callee已驗 |
| [V906 csystem.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/ea5b206117c4bd3f5ede135a4ef8ead179f8ceb9/HT9011UC_Cpp_V3.33.906.0/csystem.cpp)::DoAllProcess／DoReceiveAllToBottom_9050／DoSystem／CheckSafeDoorIsClosed | 原人工池後新增SOFT_SIMULTE下HT9050 TestHead分派、TrayZ helper呼叫、PLC失聯處理及歷史註解修正 | diff定位，沒有核對全部新callee／硬體或安全；sim、機型、PLC設定與CUSTOMER_CODE分開 |
| [V906 wb_serve.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/ea5b206117c4bd3f5ede135a4ef8ead179f8ceb9/HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp)::DialogMailboxPostAlarm／main／W906_Dispatch1203Ex／W906_Pci1203ModuleCheckTick／W906_Pci1203ReopenTick | blob已變，差異含alarm附加文字及1203重新開卡host；selected owner／reauth／web-save仍保留de7來源 | 完整diff／callee／dispatch／guard競態尚待續查，不把old owner結論套作整個最新host或開卡安全背書 |

同一主題整合HT9050與其他Handler，分開機型分派、客戶、硬體型號與runtime。UPH三列局部客戶差異見 [UPH客戶表](../../hpi-uph-model/references/customers.md)，沒有加入S8人工13列或產生新的候選ID。

本次只做Git來源、詞法掃描與文件保存／引用驗證，未建置、執行API／開卡／PLC／機台或修改runtime。全語意、全部caller／consumer／落盤、實際機型／客戶及原846未決續查；本頁不為未來版本自動背書。
