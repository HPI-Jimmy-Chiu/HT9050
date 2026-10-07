# 計數、欄位與時間換算

## AddLoadingCount

來源：[V906 ainarm9045.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/8f213da4f7c211ecdf23e6f88493c1c918f52996/HT9011UC_Cpp_V3.33.906.0/ainarm9045.cpp)、[V912 ainarm9045.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/8f213da4f7c211ecdf23e6f88493c1c918f52996/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/ainarm9045.cpp)，定位AddLoadingCount(iSuckRow,iSuckCol,iTrayRow,iTrayCol)、iUPH_LoaderCount／LotSummary.iLoadTotal／LastSet.SendCT。

所選body到達計數段時iUPH_LoaderCount++一次，沒有在增量乘site或整排吸嘴數。同時更新其他輸入／Jam／Loader計數，不代表全部同口徑。所有呼叫迴圈與其他writer未查，不能直接稱為每盤、每排、每顆或成功測試輸出數。

增量前有提前return：兩版均有LoaderAutoCleanOutByInputCT／loading status與2DCheck路徑；V912的SCK輸入alarm還可返回，V906相應DoChkInputCntAlarm區塊在所選版本是#if 0。完整callee、SCK／2D／retry差異未閉合，只保留body條件位置，不驗證動作成功。

[配置InArm caller子樹](callers/index.md) 已補七個AddLoadingCount call附近區段與V906 2x8_32短stub，完整caller仍未讀完；MoveInArmXYToLoader_9045與asendic_Loader仍待查；[pause／reset局部狀態](state/index.md) 已補24定義／extern與22指定區段，不宣稱全部呼叫鏈完成。

## RUN_INFO

來源：[V906 cprod.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/8f213da4f7c211ecdf23e6f88493c1c918f52996/HT9011UC_Cpp_V3.33.906.0/cprod.h)、[V912 cprod.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/8f213da4f7c211ecdf23e6f88493c1c918f52996/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/cprod.h)，定位struct RUN_INFO的int iUPH與AnsiString iAvgUPH。

兩版欄位型別相同；double乘積寫入int與整數平均的型別界線已定位，沒有測試極值、溢位、作用中ABI或字串consumer。四個宣告不是整個RUN_INFO初始化／保存的驗證。

## V906 DecodeTime

來源：[vclcompat/TDateTime.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/8f213da4f7c211ecdf23e6f88493c1c918f52996/HT9011UC_Cpp_V3.33.906.0/vclcompat/TDateTime.cpp)，定位DecodeTime／splitSerial、dt.Val()／serial／days／frac／totalMs。

splitSerial以floor分出整日與[0,1)小數；DecodeTime只用小數乘86400000、加0.5取毫秒，滿一日限成86399999，再拆ms／秒／分／時，hour取%24。所選body是日內分量，不能當作保留任意多日duration的轉換。負值／多日／pause大於elapsed仍需核對TDateTime運算與caller；未執行數值測試，也未把此shim套作V912 VCL實作。
