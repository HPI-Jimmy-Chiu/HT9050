# OneCycle／CleanOut 的暫停清除區段

來源：[V906 csystem.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/0c2eac30b4e56fec64b207f6adcc3ead69711903/HT9011UC_Cpp_V3.33.906.0/csystem.cpp)、[V912 csystem.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/0c2eac30b4e56fec64b207f6adcc3ead69711903/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/csystem.cpp)，pin `0c2eac30b4e56fec64b207f6adcc3ead69711903`。
以 function、ret、Tray／HotPlate／QA 旗標與 tUPH_PauseTime 定位；本頁只讀所選清除語句及附近區段。外層完整流程與所有 callee 未閉合。

## DoOneCycleFinishCheck

每版指定完整 body 文字內有兩個 tUPH_PauseTime=0 語句；兩個區段已讀，完整函式未讀。

| 附近局部條件 | 所選語句附近狀態 |
| --- | --- |
| ret==K_TRAY_FEED | 設 bOnecycleTrayFeed／bNeedTrayFeed，清 pause |
| ret==K_TRAY_END 且 IniConfig.bCleanOutCanTrayEnd | 設上述兩旗標、清 pause，iTrayFeed／iTrayFeedTask設1，並設 CleanOut／QA TrayEnd |

兩版清除文字與局部條件相同；附近 ServoOff 在 V906 用 W7C2 seam、V912 直接呼叫 fHome。沒有追完整 seam／callee，不能稱完整OneCycle／安全／機型流程等價。

## DoCleanOutFinishCheck

每版指定 body 文字內有八個清除語句；下列是已讀區段的局部定位，不能取代完整外層 guard 與分派。

| 區段定位 | 所見局部清除條件／附近行為 |
| --- | --- |
| AutoFeed／AutoSiteMap 入口附近，bTrySuckHotPlateCleanOut且Hot | 清pause、TrayFeed task設1，取消try-suck／need-feed並呼叫Start |
| 同入口的HotPlate嘗試選路附近else | 清pause，SetInitialICCheck、TrayFeed task設1；其他HotPlate branch不由此視為會清pause |
| QA模式結束附近 ret==K_TRAY_FEED | 清pause、TrayFeed task設1、bCleanOutTrayEnd=false |
| 同區 ret==K_TRAY_END | 清pause、TrayFeed task設1、bCleanOutTrayEnd=true |
| QA完成後CleanOut附近，try-suck且Hot | 清pause，取消HotPlate／try-suck旗標並呼叫Start |
| 另一 ret==K_TRAY_FEED 區，bQAModeFinishCleanOut | 清pause、TrayFeed task設1、bMustCleanAllTray=false |
| 同區HotPlate候選條件未進入的else | 清pause、TrayFeed task設1；不能從else局部推定所有外層條件 |
| TrayEnd准許條件且ret==K_TRAY_END | 清pause、TrayFeed task設1、bCleanOutTrayEnd=true；局部准許條件含一般設定或ASE＋ART例外 |

這些 tUPH_PauseTime 清除語句不等於 count／start／平均／全部旗標一併重置。V906附近Start／barcode／tray mapping／yield等動作使用W7C1 seam，V912直接呼叫；本單元不驗證動作或實機狀態。
