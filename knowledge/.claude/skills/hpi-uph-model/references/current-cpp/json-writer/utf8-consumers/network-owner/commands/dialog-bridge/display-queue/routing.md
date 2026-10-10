# displayKind路由與兩種顯示入口

[上層](index.md)／[resolveDisplay](locators.md#resolvedisplay)／[isNonStop](locators.md#isnonstop)／[routeAndRender](locators.md#routeandrender)／[raiseNonStop](locators.md#raisenonstop)。
來源 `web/page/dialog-bridge.js`；活定位用function／變數，固定pin見[證據](evidence.md)。

## resolveDisplay與isNonStop

沒有global.HT9045NonStop或其route時回傳已resolve的Promise：kind沿用、info=null、why=no-router。
有router則呼叫route(kind, request)。成功取r.kind或原kind；frames[dk]不存在就退回原kind，但不再驗證原kind的frame存在或ready。
info與why仍來自r；原kind fallback不會自行清掉這兩欄。frame存在不代表iframe已載入、consumer採用或runtime停止。
route的Promise rejection由第二個then handler轉成原kind／null／router-failed；呼叫route的同步throw、非Promise回傳及成功handler自身throw不由這個rejection handler涵蓋。
isNonStop只嚴格比對alarmNonStop或messageNonStop；字串名不是StopAllMotor是否被呼叫的證據。
原檔裁決寫瀏覽器只決定畫哪一頁，是否停機在C++送request之前決定；此歷史正文完整沿用，C++實際分派仍須另查。

## routeAndRender：直接顯示

resolveDisplay成功後，若r.info truthy，以Object.assign({}, r.info, request.nonStopInfo || {})合併；request既有同名欄位優先。
按r.kind呼叫renderNonStop或render，傳入原kind／request及解析的displayKind。
本body沒有enqueue、requestId去重、active忙碌檢查，也沒有return該Promise或catch；直接呼叫可重新指定active／activeNS，不能推成具備佇列入口的保護。
外層HTDialogBridge公開routeAndRender／render等入口；有哪些頁面使用與呼叫時序另待載入／caller查證。

## raiseNonStop：入列後drain

msg.kind嚴格為message才用message，否則alarm；由NONSTOP_OF找對應displayKind。
缺frames[dk]時console.warn後return；沒有退回stop頁。frame存在時enqueue(kind, msg.request || {}, dk)再drain。
原註解刻意不以active擋raise，讓停機告警開著時仍能顯示權限提示；這是原設計裁決，不是本輪瀏覽器實測。
本body不驗證msg物件：frame可用時直接讀msg.request，null／undefined msg會同步throw；不能由前面的msg &&檢查推成全body安全。
enqueue回傳false（例如重複requestId）時仍執行drain；去重及不停機丟舊規則見[兩層佇列](queues.md)。
