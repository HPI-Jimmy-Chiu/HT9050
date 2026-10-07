# Loader Tray 與版本差異

來源：[V906](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f66919735119286ebad61a8130e43acd097a0a68/HT9011UC_Cpp_V3.33.906.0/asendic_Loader.cpp)、[V912](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f66919735119286ebad61a8130e43acd097a0a68/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/asendic_Loader.cpp)；pin `f66919735119286ebad61a8130e43acd097a0a68`。僅核對DoSupplyNewICTray短入口與case 1300，並比較兩版完整所選case文字；其他case與helper尚未完成。

| 所選區段 | V906 | V912 | 解讀界線 |
| --- | --- | --- | --- |
| Tray Mapping分支的畫面條件 | W906_FormShowing("fContact",fContact->fShow)==false | fContact->fShow==false | 頁面表helper與實際UI未驗，不能判定等價 |
| TrayForm.bEnableAMR下設NULL_IC的條件 | iLoaderTrayCountCal >= iAMRLDNowTrayCount-iAMRCoverTray | 另要求LastSet.iAutoRetestCount_ART<1 | 這項版本差異在旗標之前，不稱作用中客戶已驗 |
| AMR／ART計數區段的進入條件 | iLoaderTrayCountCal+iAMRCoverTray < iAMRLDSECSTrayCount | 另有OR LastSet.iAutoRetestCount_ART>=1 | Tray／重測計數不同於iUPH_LoaderCount，全部ART流程待查 |
| 入口的額外local static | 未見bSupplyStuckPrev／tSupplyStuckLog | 有兩項宣告 | 只證明入口宣告差異，其後用途尚未讀完 |

所選case能把Tray資料設HAS_IC或NULL_IC，AMR ID／cover tray與Mapping分支也各有條件；bRecordUPH的賦值沒有額外HasIC guard。故此旗標本身不能證明有有效IC、更不能換成單顆／多site／tray容量口徑。OCR／Tray Mapping分支中的fHasTray與EventReport也不等於現場結果已驗。

兩版旗標附近末段文字一致，但case先行分支、helper／機型／客戶／runtime可能不同；[HT9050與其他Handler](../../../machines/index.md) 仍在同一主題中分流。沒有從所選兩版來源推定HT9050作用中派工或能力。

下一步核對DoSupplyNewICTray上層caller／其餘Task與iSupplyNewIC_From_LoaderCar寫者、MoveInArmXYToLoader、所有sampling／count／reset／pause writer、UI／DB／CSV／SECS consumer及保存結果，並分清容量／site／校正。歷史來源／metadata／資源／相容入口／裁決保留；沒有執行函式、機台或runtime。
