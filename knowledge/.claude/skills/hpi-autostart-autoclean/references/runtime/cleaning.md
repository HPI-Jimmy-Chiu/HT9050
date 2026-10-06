# 目前main的清潔入口

唯讀基準main `f57d93f15`，檔案在HT9011UC_Cpp_V3.33.906.0；未執行清潔、測試機台、產生器或runtime寫入。

## 同一本體，不同收件規則

FileRW/TestIF_File_Cleaning.cpp::W906_ShowBinSelect_btnAutoCleanClick呼叫CL4_GoldenBtnAutoCleanClick。Cleaning頁btnStartAutoClean走form.event與設定頁／權限條件，運轉中拒收；Status.ShowBinSelect的act.showBinSelect.autoClean走cShowBinSelect_E023.cpp::E023_AutoClean及g_W906_E023_BtnAutoCleanSeat，依Steven Q67=B可在運轉中按，不能套用Cleaning頁的拒收規則。

本體先擋bRunAutoClean、bIsASMAutoOneCycle、iOneCycle!=0；iOneCycle的早退是E-030按既有裁決保留V912安全條件，906原版沒有。不管有無料都先歸iAutoClean_IndexContactCount並初始化三組Task；其後有料呼叫InitialAutoCleanAllTask，沒料才查fAllMotorHome、MTrayX對Prod.iXTrayEmpty、CheckIndexIsNormal及IndexStatus／bD51UseOnecycleCleanOutFinishTestArmAtRear。

所以後面檢查擋下時，計數／Task可能已初始化；沒料全部通過才設bRunAutoClean與hAutoCleanHangUp。不能寫成「按鈕一定當場運動」或「拒收一定毫無資料副作用」。FileRW_Cleaning_E023Seat持FormLock取得filerw session；Status caller在鎖釋放後顯示訊息，這也不是兩份不同清潔演算法。

## 已有呼叫不等於所有Host已接通

TesterComm/Handler/HandlerGpibMsg.cpp的MSG_CMD_Auto_Clean目前設定bGPIBAutoClean並記錄，但fShowBinSelect->btnAutoCleanClick仍在#if 0的G14內。SECSGEM/uHGemHT9045.cpp的AUTO_CLEAN整支仍在G18內，原碼說明這是使用者裁決，不因表單本體存在就能解除。原D-033／review日期保留，現況按實際caller核對。

## 正常流程的觸發／Task

AutoClean/AutoClean.cpp::InitialAutoCleanAllTask初始化AutoClean、Shuttle、Index task，設bIsAutoOneCycle並呼叫BtnOneCycleClick。EnableAutoclean的Manual分支與interval分支都要求iOneCycle==0；interval依TestIF.iAutoClean_IntervalContact及iAutoClean_IndexContactCount判斷。此helper的條件與UI手動本體不同，不把SKILL的「全部需滿足」摘要當所有caller唯一入口。

正常AutoClean階梯與DoAutoCleanKit／DoIndexAutoClean以Task推進；[Task原文](../autoclean/tasks/index.md)與[HT9050乾跑](../machines/index.md)分開。Smart Auto Clean在Yield統一告警入口目前仍是PC_CHECKSMARTAUTOCLEAN=false，見 [Alarm／Yield](../../../hpi-alarm/SKILL.md)，不可宣稱全部智能清潔呼叫已上線。
