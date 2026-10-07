# 指定 UPH 全域定義與 extern

來源 pin `0c2eac30b4e56fec64b207f6adcc3ead69711903`：
[V906 cmydef.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/0c2eac30b4e56fec64b207f6adcc3ead69711903/HT9011UC_Cpp_V3.33.906.0/cmydef.cpp)、[V906 cmydef.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/0c2eac30b4e56fec64b207f6adcc3ead69711903/HT9011UC_Cpp_V3.33.906.0/cmydef.h)、
[V912 cmydef.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/0c2eac30b4e56fec64b207f6adcc3ead69711903/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/cmydef.cpp)、[V912 cmydef.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/0c2eac30b4e56fec64b207f6adcc3ead69711903/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/cmydef.h)。
以變數名及全域 namespace／guard 定位；本層的 24 個指定 snippet 都無字面 inactive。

| 變數 | 兩版指定定義 | Header | 意義界線 |
| --- | --- | --- | --- |
| iUPH_LoaderCount | int，初值0 | extern int | 不是直接宣告盤／排／顆的單位 |
| bRecordUPH | bool，初值false | extern bool | 所有開關寫者與取樣時機未閉合 |
| tUPH_StartTime | TDateTime，初值0 | extern TDateTime | 完整日期運算／重置呼叫待查 |
| tUPH_PauseTime | TDateTime，初值0 | extern TDateTime | 累加與部分清除見本層，非全生命週期 |
| tUPH_PauseStartTime | TDateTime，初值0 | extern TDateTime | 暫停開始時間的所有寫者未查 |
| bCalculatePauseTime | bool，初值false | extern bool | MainProc 用此旗標決定累加並清旗標 |

初始值是指定 cpp 的全域定義文字，不是每次開 Lot／Start／Pause 都重置的證據。型別與 extern 對應不驗證 ABI、thread synchronization、快照一致性、持久保存或實機結果。

原有 [RUN_INFO 與時間 helper](../count-time.md) 另釘住前一層來源；本層不將 V906 TDateTime shim 說成 V912 VCL 運算已等價。

## 最新 main 標頭定位補充（20261007）

推前整合 pin `2fc72a860ea28a655e1a3fa5779cd555d94f264f`：V906 [cmydef.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/2fc72a860ea28a655e1a3fa5779cd555d94f264f/HT9011UC_Cpp_V3.33.906.0/cmydef.h) 目前是umbrella，include [cmydef_core.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/2fc72a860ea28a655e1a3fa5779cd555d94f264f/HT9011UC_Cpp_V3.33.906.0/cmydef_core.h)、[cmydef_io.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/2fc72a860ea28a655e1a3fa5779cd555d94f264f/HT9011UC_Cpp_V3.33.906.0/cmydef_io.h) 與 [cmydef_rt.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/2fc72a860ea28a655e1a3fa5779cd555d94f264f/HT9011UC_Cpp_V3.33.906.0/cmydef_rt.h)。本頁上方0c2eac30b來源／24 snippet保留作原查證版本，不覆寫。

上述六個UPH extern現在集中於cmydef_rt.h：依iUPH_LoaderCount、bRecordUPH、tUPH_StartTime／PauseTime／PauseStartTime與bCalculatePauseTime定位；六個指定宣告文字／SHA與原cmydef.h相同，guard改由cmydef_rtH包住。umbrella的三個include在cmydefH guard內；其餘兩子標頭沒有這六個extern形狀。保存見 [標頭更新manifest](header-refresh-manifest.json)。

這裡只讀六宣告與三include／guard，保存四header blob／byte；沒有讀完全部標頭家族語意／caller或測試ABI、build、thread／runtime等價。V912仍沿用上方獨立來源，不套用V906檔案拆分；全域初值與每次重置的界線維持。
