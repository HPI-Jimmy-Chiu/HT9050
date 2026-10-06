# Bin Display目前main與歷史界線

main `372b91908`，20261006只讀查證，未建置／連COM／測實體面板。

- `tools/wb_serve.cpp`在LoadMachineConfig之後呼叫`W906_BinDispSystemModularBoot_St02`；`database.cpp::InstallColorBinDisplay`現在new `TMyBinDispHT9046`。原「main只用Offline、沒有建立控制器」限舊577c41c5，不是今日main。
- `BinDisplay/BinDispBringUp_St02.cpp`有`TMyBinDispCtrl::Timer1Timer`活body、TDataModule3／接收queue、`W906_BinDispRxPumpAt_St02`、Timer1Fire及shown-scope／BinSel resume。`NUMBER_PANEL_TYPE==3 || ==4`才走bring-up；types0／1／2不能套序列面板diagnosis。
- 接收執行緒排隊，在pump交回golden handler；SIM與ctest由`W906_BinDispUnderCtest_St02`／SetSimMode保護不開真埠。這是程式條件核對，沒有宣稱上機收發通過。
- `BinDisplay/MyBinDisp.cpp`現在有ST02-S22修正：`WriteTargetCount`只在數量變動時標`bCountDirty`並喚醒cycle；`DoCycleTFT`有priority frame、反飢餓與鏡像更新。相關`WriteTargetBin`的TFT iColorNow／iBinNow更新是命令／ack鏡像，不能當實體讀回。
- 新裁決`RULINGS_20261003`第20條明授權D／E／A交St02；原Skill「912修好的TFT都不移植」是第20條之前狀態。保留原文，今日判斷先看新裁決與活body。
- `MainTimersSt02.cpp::BinDispTimer1Slot`按Timer1Interval調度C14控制器；同檔`W906_FastClockOwnsBin`分流的是Timer1BinTick脈衝面板。不能把30ms快時鐘泛稱所有C14 Timer1都是30ms。
- `WebBinDispStatus_St02.cpp::W906_StageBinDispStatus_St02`已有15個binsel.disp tags，從facade與controller只讀取值；wb_serve在`W906_PageStreamWanted("binselect")`才發布。`web/page/ht9045_bindisp_status.js::model`保留unknown為null，`render`讀jumpSeq切頁；原「網頁靜態佔位」限歷史版。

## 來源

- [Bring-up](../../../../../HT9011UC_Cpp_V3.33.906.0/BinDisplay/BinDispBringUp_St02.cpp)：SystemModularBoot／Timer1Timer／RxPump／UnderCtest。
- [控制器](../../../../../HT9011UC_Cpp_V3.33.906.0/BinDisplay/MyBinDisp.cpp)：WriteTargetCount／WriteTargetBin／DoCycleTFT／bCountDirty。
- [調度](../../../../../HT9011UC_Cpp_V3.33.906.0/MainTimersSt02.cpp)／[wb_serve caller](../../../../../HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp)。
- [網頁資料](../../../../../HT9011UC_Cpp_V3.33.906.0/WebBinDispStatus_St02.cpp)／[前端](../../../../../web/page/ht9045_bindisp_status.js)。
- [1003裁決](../../../../../HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20261003.md)：第19／20條；[原移植盤點](../source/references/port-status.md)。
