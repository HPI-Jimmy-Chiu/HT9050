# 本單元範圍與下一層

本層的兩個來源、二十九個完整cpp定義、兩個class、宏與DLL名字表可按 [manifest](source-manifest.json) 核對版本與文字。只確認選讀來源的分支／返回／本地狀態，不新增完整UPH語意結案數。

仍待查：

- 全部driver建立者、SetGpibDriver／解除時序、g_driver壽命與所有caller／並行條件。
- 實際NI DLL export、平台ABI、ThreadIb*語意與真正的GPIB／Tester送達、ACK、超時。
- 上游VM／MV字串與buf／cnt契約、site／容量、測試紀錄與實機校正。
- V912對應driver，以及HT9050／其他Handler／客戶和建置組態的實際選擇。
- Sim測試的caller／assertion是否完整涵蓋狀態保留及輸入；本輪未跑ctest或driver。

原註解的golden／單一TesterComm thread敘述保留為來源敘述；歷史及先前 [wrapper界線](../limits.md) 的未查項，不因本單元或舊!321已合main而變成實機已驗證。

沒有執行DLL、C++、build、API或機台；沒有寫runtime、snapshot或來源碼。回 [兩實作索引](index.md)、[共用與差異](versions.md)。
