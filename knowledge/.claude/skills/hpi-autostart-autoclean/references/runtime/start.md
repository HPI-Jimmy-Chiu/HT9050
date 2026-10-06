# 目前main的啟動與通訊

唯讀基準main `f57d93f15`，檔案在HT9011UC_Cpp_V3.33.906.0。未執行Handler、網路啟動、Lot清除或測試；以下為來源碼核對。

## OLP：ProcessBuffer

Automation/automation.cpp::TfAutomation::ProcessBuffer的PAUSE_REQUEST設bLockByServer=true及SoftStop=true；RESUME_REQUEST只清bLockByServer，沒有清SoftStop。START_REQUEST在palMainStatus.Caption==HALT且SystemStart==false時呼叫fMain->Start，條件不符也回START_REPLY，Data[0]仍寫0。

forms/fMain.cpp::TfMain::Start目前是空基底；WebStart.h明確不override它。因此OLP的這條直接呼叫及START_REPLY不能當作目前機台已啟動的證據，也不能把它冒稱7016的seam。程式中第二個PAUSE_REQUEST分支被第一個遮住，保留golden的既有形狀，不在Skill中改程式。

ONECYCLE_REQUEST只在DEBUG_DUTONOFF區段內，設定bOneCycle並呼叫DoOneCycle；這不是所有出貨／SIM組態都開放。CLEAR_REPORT_REQUEST目前每圈讀Data[0]呼叫DoClearReportRequest，回覆也不代表外部Agent的MO／Flow／Ticket已比對。

## 7016：HTSET333／334

Command.cpp的HTSET333受CC_TERAPOWER或CosFunction.bRemoteLotStart、HALT與SystemStart條件限制，呼叫W906_RemoteRunStart；未安裝seam或manual teach不准時回NG。HTSET334另外要求SystemStart及CC_TERAPOWER，呼叫W906_RemoteRunPause。

這是已接通的呼叫入口，並非新的開通需求；然而OK表示依此入口取得的回覆，不代表所有Start守衛已通過、馬達已移動或機台已進RUN。跨通訊細節仍由 [hpi-gpib](../../../hpi-gpib/SKILL.md)管理，不改其owner的文件。

## 外部流程與共同邊界

GTK Ready重試、Agent Info Mismatch、Setting OK屬原協定角色；是否由外部程式實作須有該程式來源，不能由Handler有OLP函式推定。HOME、START、AutoClean、Lot資料寫入都不是文件驗證方式；[HT9050乾跑](../machines/index.md)還可能接管正式流程。
