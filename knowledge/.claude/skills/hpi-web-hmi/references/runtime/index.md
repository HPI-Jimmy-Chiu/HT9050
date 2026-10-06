# 本批目前main核對

來源為main `b8ea3a511`，只以Git唯讀核對HT9011UC_Cpp_V3.33.906.0與web/page的相關實檔。下列是本批查到的範圍，不是全量WebHMI接線或實機測試報告。

| 問題 | 檔案與function／變數 | 核對結果 |
|---|---|---|
| 舊啟動旗標 | tools/wb_serve.cpp::main，--dry／gAllowSystemWrite | --dry分支印退場並return 2；gAllowSystemWrite=true，--allow-system-write接受但不改預設選擇 |
| 模擬／真機 | main的NODRY分支、SOFT_SIMULTE | 建置期控制；API payload dryRun與已退場啟動參數是兩件事 |
| server節拍 | tools/wb_serve.cpp::kServeTickMs／kIoTickMs | 定義100ms／200ms；不代表所有Timer、畫面、tag都同頻率，舊50／500ms敘述按原日期 |
| screen存在 | WebPageTable.cpp::PageScreenPresent／PageStartAllowed | liveWs為0先判無畫面，再問fMain；未armed直接允許，armed且screen缺席拒START |
| 全關寬限 | WebPageTable.cpp::PageTableTick／kPageNoScreenGraceMs | 寬限10000ms，依systemStart／homingAll／webMotorJob及已安裝host分派；不由hub送整台STOP |
| stale與fShow | WebWindowRegistry.cpp::kStaleAfterMsDefault／WebWindowRegistryFShowConservative | 過期門檻15000ms；conservative對未知／stale有保守語意，與PageScreenPresent不同 |
| hub責任 | web/page/ht9045_link.js::Hub.prototype._releaseHeld／windowHidden | 所持jog各以自己button釋放；不代送btnStop等全機STOP，HOME／Loop另走server關窗／斷線處理 |
| 權限定位 | WebLogin.cpp::W906_Reauth／W906_ReauthHasAnswerFor／W906_NoteAuthVerify／W906_DoPasswordMBox | 個別驗證函式存在；本批未重驗所有caller、客戶分支或實際密碼 |
| log lifecycle | tools/wb_serve.cpp::main，W906_CreateLogObjects／W906_DestroyLogObjects | 開站／關站有實際呼叫；舊未建log說法僅屬原盤點年代，不證明每條CSV事件caller已接通 |

Config的Cowner、editlist.save運轉／開頁guard與Recipe的直接caller以 [Config現況](../../../hpi-config/references/runtime/writers.md)及[LotInfo／Recipe現況](../../../hpi-lotinfo-recipe/references/runtime/recipe.md)為同批查證入口；未把guard擴張套到每條動作。

七個原Skill的計數、review、ctest與真機紀錄完整保留各自日期；沒有重跑generator、FShow_Audit、登入／WS／HTTP、CSV寫入或機台測試。後續main更新時重新核對相關函式，原資料與目前補充保持分開。

最新main `02ed3e735`的設定鏡像與E-043 WIP紀錄差異見 [20261006整合補充](main-update-20261006.md)，來源核對與待答分開。
