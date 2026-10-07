# MotorProfiler 設計符號的限定盤點

[原稿架構](../legacy/references/uph-motor-time-table/04.md) 與 [三階段設計](../legacy/references/uph-motor-time-table/01.md) 保留為歷史提案。Service Tab、埋點、Coverage、Smart掃描、Home／CSV與安全敘述沒有因盤點變成目前可執行功能。

以pin8f213da4f盤點V906 C++與V912的追蹤.cpp／.h／.hpp／.inc／.dfm，排除.svn／docs／tests；共2473檔，清單SHA與查詢見 [manifest](source-manifest.json)。這是文字符號盤點，沒有讀全部function body。

tsUPHProfiler／DoUPHScan／BuildScanQueue／bUPHProfilerRunning／uUPHProfileLog／bP11_1_RecordFlowProcessTime六個原稿定位詞在此範圍零命中；檔名包含uUPHProfileLog或MotorProfiler也沒有命中。

零命中只表示限定版本、追蹤檔與名稱沒有對應；不能證明全部改名、外部元件、未追蹤程式或機台功能不存在，也不驗證現有uMotorTest／Home／ContinuousMove安全互鎖。版本更新需重新盤點。

未執行Profiler、移動、Home、測速、CSV讀寫、API或機台；實作、資料producer、容量／site／校正／Coverage仍待核對，不能依舊稿接動作。
