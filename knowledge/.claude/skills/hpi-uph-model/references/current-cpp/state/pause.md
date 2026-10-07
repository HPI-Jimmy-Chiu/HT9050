# MainProc 的暫停累加區段

來源：[V906 csystem.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/0c2eac30b4e56fec64b207f6adcc3ead69711903/HT9011UC_Cpp_V3.33.906.0/csystem.cpp)、[V912 csystem.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/0c2eac30b4e56fec64b207f6adcc3ead69711903/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/csystem.cpp)，pin `0c2eac30b4e56fec64b207f6adcc3ead69711903`。
定位 MainProc 的 bCalculatePauseTime／tUPH_PauseTime／tUPH_PauseStartTime；只讀兩個指定區段與附近上下文，未讀完整 MainProc。

兩版所選區段相同：bCalculatePauseTime 為 true 才把 Now()-tUPH_PauseStartTime 加到 tUPH_PauseTime，然後把 bCalculatePauseTime 設 false。本段沒有重置 tUPH_PauseStartTime，也沒有在此扣除 UPH 計算 elapsed；扣除另見 [CalculateUPH](../calculate.md)。

此段累加不依 CUSTOMER_CODE、site 數或 Tray 容量再乘倍率。附近有 Fix3／Start 檢查與 ScanColorFixTrayStatus 呼叫，但附近出現不證明完整外層條件、呼叫可達或動作成功。

後續 [ShowRunLabel暫停開始與排程](kernel/index.md) 已補所選writer／刷新與早退條件；兩個DoSystemMessage body完整讀取，完整ShowRunLabel UI與串接仍待查。

所有 bCalculatePauseTime／tUPH_PauseStartTime 的寫者、pause／resume 閘與 caller、clock／負值／跨日、同時執行的資料一致性未查完。不能由這段推定每次暫停一定被計入一次、暫停開始值必定有效或 elapsed 全程正確。
