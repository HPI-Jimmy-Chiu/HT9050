# Lot生命週期與配方下載

最新已整合main的 [WA-5／WA-9／WD-3更新](../runtime/main-update-20261006.md)先看；原關閘紀錄仍按其原日期解讀。

- [原完整主體](original-entry.md)：SetLotStart、SetLotID、SetLotInfo、LotStart／End來源、Config、ATC、FTP與客戶。
- [原生命週期詳解](references/lot-lifecycle.md)：bReadFromFile與新批、RunInfo、UI／事件。
- [原下載流程](references/recipe-download.md)：DownloadFromServer／ERMS／TSMC、DoBackupSetupFile／DoOverWriteSetupFile及Security_new.def條件。
- [Recipe資料樹](../recipe/index.md)：切換文件、欄位與寫者，下載存在不代表所有客戶或callback接通。

目前main `f57d93f15` 的HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp::lot.start dispatch直接呼叫fLotInfo->SetLotStart("wb_serve::lot.start")，先要求tag LotID及value Operator非空，再查RunInfo.bLotStart；它沒有呼叫W906_LotInfo_SECSLotStart。FileRW/LotInfo_SECSLotStart.cpp雖保留該wrapper及拒絕／略過／記錄型gate，原caller註記与LI1-8退場紀錄不能證明目前lot.start已走wrapper的全部檢查。其他caller仍需逐一核對，whyNot／skipped是wrapper脈絡；本次未開批或執行任何API。

Lot Start不是機械Start，參照 [AutoStart共用入口](../../../hpi-autostart-autoclean/SKILL.md)。ATC與Yield顯示分別路由 [溫控](../../../hpi-temperature/SKILL.md)、[Alarm／Yield](../../../hpi-alarm/SKILL.md)，不把原錯誤enum表當目前定義。
