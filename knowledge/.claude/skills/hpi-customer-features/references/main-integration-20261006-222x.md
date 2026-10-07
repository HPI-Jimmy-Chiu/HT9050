# 22時main來源更新核對

已整合main `2db43115d065beaa81ddbc69d0bd34df17a9959b`，來源／scanner更新獨立於原候選與人工查證樹保存。[本次摘要](versions/main-check-2db43115d.json)釘住Git來源blob、scanner hash、候選差異及人工來源池變更。

## 候選與保存界線

唯讀掃描2,738個來源檔：V912 717、V906 C++ 2,021。原341cea3d7樹的6,382筆候選／ID、846筆未決、09d926ab8人工8列及36625f91b開機／功能入口3列完整保留；本次仍6,382筆，忽略blob後詞法定位／條件提示／guard候選增0、減0。較上次ef41839d4多1個來源檔，不是新增已驗證客戶功能。

原source09d926ab8人工池只見CMakeLists blob改變，其他人工ART／名稱原來源相同。36625f91b開機池只見V906 database.cpp blob改變；原manifest及三列釘住舊版，不覆寫來源日期。完整owner／callee／consumer／link與build gate仍待續查，數量相同不能當機台行為相同。

## HT9050與其他Handler、機型與客戶分流

| 最新來源定位 | 觀察 | 適用界線 |
|---|---|---|
| [V906 database.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/2db43115d065beaa81ddbc69d0bd34df17a9959b/HT9011UC_Cpp_V3.33.906.0/database.cpp)::SYSTEM_MODULAR::ReadGeneralIni；Enable_PLCSafety_IO、iSafePlcModel | 舊SafePlcIO旁新增CheckAndReadIniDataGeneral("System", "SafePlcModel", 0)；註解記0是golden map、1是Reer MOSAIC M1S COM | 是PLC型號設定，不是CUSTOMER_CODE分流；原缺鍵補寫行為仍要考慮，沒有讀或改本機runtime，沒有驗證實際PLC配置／通訊 |
| 同函式CUSTOMER_CODE、CustomerFunctionSelect／ReadLastSetIni順序 | 從36625f91b到本次該檔只有上述一行差異，原客戶碼讀寫／ASE條件／caller順序段未改 | 原三列是局部查證，不延伸成全部後續功能或各機台有效值都已查完 |
| [V906 CMakeLists.txt](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/2db43115d065beaa81ddbc69d0bd34df17a9959b/HT9011UC_Cpp_V3.33.906.0/CMakeLists.txt)的AutoRetest.cpp及FileRW清單 | AutoRetest.cpp直接列在CMake；FileRW/HSys.cpp在_editlist_sources.cmake，由include與W906_FILERW_SRC收錄；main新增PLC來源／測試等更新另留blob | 來源清單存在不等於本輪成品符號／全部runtime caller已核對；舊ART／名稱人工列來源日期保持 |

HT9050與其他Handler仍走同一客戶主題，PLC型號／裝配、客戶碼及模擬／出貨組態分開核對。W-44 Steven原裁決正文不變；前批機型／caller局部更新見[20時核對](main-integration-20261006-2022.md)，不得將當時結論提升為後續來源或現場通過。

只做Git來源與文件保存／引用驗證；未建置、跑API、呼叫ReadGeneralIni、執行機台或修改runtime。推送前仍須重新fetch／整合及查相關變更，本頁不是未來來源自動背書。
