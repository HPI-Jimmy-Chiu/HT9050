# CalculateUPH 的公式與狀態

來源：[V906 ainarm9045.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/8f213da4f7c211ecdf23e6f88493c1c918f52996/HT9011UC_Cpp_V3.33.906.0/ainarm9045.cpp)、[V912 ainarm9045.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/8f213da4f7c211ecdf23e6f88493c1c918f52996/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/ainarm9045.cpp)，定位CalculateUPH(bool bReset)、bOneTimes／tTempTime／tUPH_StartTime／tUPH_PauseTime／iUPH_LoaderCount／RunInfo。

## 兩版所選共同運算

bReset或bOneTimes時清bOneTimes、iUPH、pause與count，更新start、初始化grid欄名與資料列；VTEST再處理三個額外欄。此段沒有明確更新RunInfo.iAvgUPH，不能把清grid說成所有平均狀態同步重置。

一般分支只有tTempTime>start時進行一輪：十列歷史往後移，取end與elapsed，先填Start／End／Pause及VTEST原elapsed／count／site，再更新下一輪start、扣pause、清pause。DecodeTime組成秒數；>0用3600除秒數，否則倍率零。RunInfo.iUPH=fTimerMultiple*iUPH_LoaderCount，接著DB呼叫、保存局部iLoaderCount並清全域count；所選公式沒有另乘site、tray容量或良率。

grid十個非空UPH字串以atoi總和／計數；iCount零回零，否則整數除法後轉AnsiString寫iAvgUPH。這是所選列的平均，不是elapsed／count加權。tsUPH與UPHRecordEnd呼叫先於平均更新，完整consumer時序另查。

tTempTime<=start的另一分支只把iUPH設零、start改成tTempTime並清／初始化grid；沒有明確清pause、count或iAvgUPH。全部時間／旗標寫者未核對，不由此推定現場clock回退／重置結果。

VTEST Elaps. Time來源是扣pause之前的tConsumeSecond，兩版body都如此；V906的相反註解不能代替實際順序。見 [計數／時間](count-time.md) 與 [輸出差異](outputs.md)。

本層沒有確認整盤觸發、Lot重置、全部pause累計、日期運算平台等價、counter溢位、callback成功或實機throughput。
