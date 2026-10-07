# 機型、來源更新與剩餘

同一[HT9050](../../../../machines/ht9050/index.md)與[其他Handler](../../../../machines/ht9045.md)UPH Skill共用這段指令鏈；V906的轉送表／hub與V912直接WM_COPYDATA分開。所選七body沒有MachineTypeChoice／CUSTOMER_CODE gate，不代表所有機型／客戶都呼叫它們或採同介面。

pin保留V906 UTF-8、V912 cp950。舊Frank／Steven／Ifor與golden註解只是來源歷史，本批只記所選語句／條件，沒有重裁歷史規則或實機查證。

main本輪變更落在 `ckernel.cpp`的 `ScanPannelKey`、測試／保護工具／文件及版控snapshot；既有kernel manifest不改，所選兩版ShowRunLabel／DoSystemMessage四body hash仍相同，見本manifest的kernel_refresh。整份V906 ckernel.cpp byte已變，不宣稱整檔未變；snapshot沒有套到runtime。

所有GetUPH dispatcher／callback／init caller、fTesterSide／hub初始化與並行生命週期、資料結構與字串長度／編碼、SendToEngine／視窗／接收端、保存與送達結果、完整UPH counter／grid、容量／site／校正與S8仍待補。沒有執行C++／build、指令、server／engine、API／LIVE、網頁、IO、Home、機台或runtime。

回[轉送入口](index.md)、[原取值界線](../limits.md)與[來源](../../../../resources.md)。

[mailbox局部](mailbox/index.md)已查SendToEngine與六個SyncMailbox body；全部engine／thread／caller／生命週期及送達仍未閉合。
