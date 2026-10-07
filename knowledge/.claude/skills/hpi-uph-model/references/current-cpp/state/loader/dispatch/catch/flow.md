# 本地取料效果與上層分流

已讀範圍見[manifest](source-manifest.json)。`InitialCatchFromLoader_9050`只把 `iCatchFromLoader9050Task`設1；本函式不重設 `iLoaderLayerCount_9050`，其他初始化／writer待查。

`DoCatchFromLoader_9050`的Task別名為 `iCatchFromLoader9050Task`，以下僅描述所選source的語句／條件；馬達、氣缸、感測器與教導helper的現場語意未驗。

| 定位 | 本地效果 | UPH／實機界線 |
| --- | --- | --- |
| case1，`MTrayXCanSafeMove()==false` | 呼叫StopMotor、Task設1、return3 | 只證程式有此支路，不證安全helper或停止效果 |
| case1，`TrayXMoveCheckEnc()==1` | SECS開關為true時呼叫TrayTestFinish；有盤／無盤兩支都Task10 | EventReport呼叫不證事件已送達 |
| case20 | selector.On、MLoaderZ.ClearTray、`iLoaderLayerCount_9050++`、Task30 | 層數不是`iUPH_LoaderCount`，不直接當每顆計數 |
| case40 | MTrayX的fHasTray設true；cover／TrayID從MMTrayY複製，MMTrayY.ClearTray | 這是狀態欄位移轉，不是已量到IC／盤有效 |
| case60，`TrayArmMotorMove(xTarget)`通過 | InArm X／Y可動旗標設true、CatchFinish旗標false、Task1、return1 | 其他helper、全counter鏈與真機完成仍未驗 |
| 其他未完成分支／函式末端 | return0 | 不推整機未動作、故障或重試策略 |

case60的 `xTarget`依 `bP56TrayArmWaitAtColorTrack`，或已安裝OCR且 `CosFunction.bTrayOCR`，選 `Prod.iXTrayColor`，否則 `Prod.iXTrayEmpty`。教導值／配置來源、真實機構與校正未查。

兩個所選取料body的字面return集合為0／1／3；這兩body內無直接 `bRecordUPH`／`iUPH_LoaderCount`／`CalculateUPH`／`AddLoadingCount`詞，但不是間接writer或機台沒有UPH的證明。前層 `DoLoad_9050`等候catch Task1的本地gate見[補盤分流](../ht9050.md)，完整caller／thread排程仍待補。

`DoCatchTray`短入口使用 `Task=CatchTrayTask`別名；pause／CatchTraySuck條件或 `bEject`可以提前return，並呼叫QueueTaskList[11].CheckTaskChange；這些helper／旗標源頭未閉合。

case250先設CatchFinish／manual旗標，依 `MachineTypeChoice==Type_HT9050`選9050取料，else走一般 `DoCatchFromLoader`。ret1會記MES0650並走客戶／cleanout條件，最後依LoaderToEmptyColor選Task2120／2000；ret2→Task100、ret3→Task5200。ret2分支仍存在，但所選9050取料body沒有直接return2；不據此宣稱一般helper或所有caller已驗。回[界線](limits.md)。
