# DoSupplyNewICTray 的 case 1300

來源：[V906](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f66919735119286ebad61a8130e43acd097a0a68/HT9011UC_Cpp_V3.33.906.0/asendic_Loader.cpp)、[V912](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f66919735119286ebad61a8130e43acd097a0a68/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/asendic_Loader.cpp)；版本與function／變數定位見 [入口](index.md)。已核對完整所選function的brace邊界，但只讀入口與case 1300區段，保存完整body hash不當成完整語意完成。

兩版以 `int &Task=iSupplyNewIC_From_LoaderCar` 綁定狀態。switch前呼叫MOT[MLoaderY].SetSpeed(100)；若SnLoaderTrackDetect.Enable且CheckLoaderICFloating(0)==false，先return false。這是本處早退條件；沒有驗證sensor、helper或實際馬達。

`bRecordUPH=true` 位於switch(Task)的case 1300，賦值外沒有額外的本地if包住。前面仍有OCR／Tray Mapping／FIFO／AMR／AutoRetest／Alignment等分支與helper呼叫；須正常到達賦值才會設旗標，不能推定每個上料事件都必然執行。

所選case末段的文字順序在兩版相同：

1. bLoaderHasSuck=false，呼叫MOT[MMTrayY_Car].ClearTray(__FUNC__)。
2. bRecordUPH=true。
3. 若IniConfig.bI27_ManualSortMode且bRunManualSortMode為true，呼叫Pause("DoSupplyNewICTray")與EditTray(MMTrayY,2)。
4. bPickUpHomeFinish=false；若IniConfig.bEnable_SECS_GEM，呼叫EventReport(SECS_EVENT.UPHRecordStart)。
5. 再處理bShowHPICCount與OneByOne相關局部條件；Task=1並return true。

旗標賦值沒有以SECS開關或ManualSort開關為條件；事件呼叫另受SECS開關限制。return true在末段，與「確實送出事件／落盤／具有有效IC／UPH計算成功」分開。Pause、EditTray、EventReport及Tray helper內部可能影響狀態；未據此推定無副作用或完整執行順序。

本處沒有直接增加iUPH_LoaderCount、寫tUPH_StartTime／tUPH_PauseTime或呼叫CalculateUPH。其他Task、上層caller、bRecordUPH的全部寫者／消費順序、重入／thread與作用中機型未閉合。這處旗標與 [計算本體](../../calculate.md) 的靜態證據可以相互參照，不能稱完整取樣鏈已驗。
