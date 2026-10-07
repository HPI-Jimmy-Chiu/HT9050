# Loader 的 UPH 記錄旗標

pin `f66919735119286ebad61a8130e43acd097a0a68`；[V906 asendic_Loader.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f66919735119286ebad61a8130e43acd097a0a68/HT9011UC_Cpp_V3.33.906.0/asendic_Loader.cpp)／[V912 asendic_Loader.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f66919735119286ebad61a8130e43acd097a0a68/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/asendic_Loader.cpp)。定位 `DoSupplyNewICTray`、`iSupplyNewIC_From_LoaderCar`、`case 1300`、`bRecordUPH`。

| 問題 | Reference | 範圍 |
| --- | --- | --- |
| 何時設記錄旗標、事件先後 | [觸發區段](trigger.md) | 短入口與完整所選case文字；helper、其他Task與caller未閉合 |
| Tray／AMR／版本差異 | [差異與界線](differences.md) | fContact／ART條件，不能由旗標推定有效IC或實際UPH |
| 原始證據 | [manifest](source-manifest.json) | 兩blob／byte SHA、兩完整function保存hash、四已讀區段與case結構 |
| 上層caller與HT9050專屬分流 | [dispatch子樹](dispatch/index.md) | 三caller入口／case與三HT9050完整文字；完整派工／取樣鏈待查 |

完整function body僅保存hash，其他case尚未讀完。所選case已讀不代表完整函式／helper或機台語意已驗；沒有執行C++、IO、build、API或runtime。

沿用同一 [UPH Skill](../../../../SKILL.md) 的HT9050／其他Handler、Hot／Ambient／客戶與runtime分流。標準取樣狀態見 [全域](../globals.md)，計算consumer見 [CalculateUPH](../../calculate.md)，配置計數call見 [InArm](../../callers/index.md)；完整生命週期仍待查。
