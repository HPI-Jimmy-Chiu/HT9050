# Loader caller 與 HT9050 分流

來源 pin `ac116483d8f4243339083fe438bda1cd81b96ef1`；三個來源與前 pin `f66919735119286ebad61a8130e43acd097a0a68` 的 byte 相同。此處只查本地控制區段，沒有執行 C++、build、IO、API 或 runtime。

| 問題 | Reference | 已讀範圍 |
| --- | --- | --- |
| 一般 Loader／AutoTeach 何處呼叫旗標函式 | [caller](callers.md) | 三個短入口與三個所選 case；完整 caller body 僅保存 hash |
| HT9050 改走哪個 Task、補盤 true 的含義 | [HT9050](ht9050.md) | 三個完整函式文字已讀；callee／設定／完整執行語意待查 |
| 證據及仍待閉合的範圍 | [界線](limits.md)、[manifest](source-manifest.json) | 三 blob、六 body hash、八 HT9050 所選區段；不是全部 caller／writer 清單 |

不可變來源：[V906 Loader](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/ac116483d8f4243339083fe438bda1cd81b96ef1/HT9011UC_Cpp_V3.33.906.0/asendic_Loader.cpp)、[V912 Loader](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/ac116483d8f4243339083fe438bda1cd81b96ef1/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/asendic_Loader.cpp)、[V912 AutoTeach](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/ac116483d8f4243339083fe438bda1cd81b96ef1/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/AutoTeach/AutoTeach.cpp)。以 function／Task／變數定位，source snippet 保留原始註解；其中原文行號不是活文件的主要定位。

一般 `DoSupplyNewICTray` 的 `case 1300` 取樣旗標見 [前層觸發區段](../trigger.md)。HT9050 在 V906 `DoLoad` 入口先分流並 return，因此不能用一般 case 的旗標作為其作用中取樣鏈證明。兩機型仍在同一 [UPH Skill](../../../../../SKILL.md) 內分層整理。

[HT9050取料子樹](catch/index.md)補兩本地body文字、DoCatchTray短入口／case250、0／1／3返回值與層數／Tray欄位移轉；全部caller、安全helper及UPH計數鏈仍待補。
