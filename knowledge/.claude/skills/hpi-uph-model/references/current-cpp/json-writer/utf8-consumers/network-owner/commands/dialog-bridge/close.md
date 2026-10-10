# complete與關閉回覆生命期

[上層](index.md)；完整[complete](raw/source-01-part-01.md#complete)／[submitCloseResponse](raw/source-01-part-01.md#submitcloseresponse)／[rejectClose](raw/source-01-part-01.md#rejectclose)，來源 `web/page/dialog-bridge.js`。

## 一般回應與畫面關閉

complete沒有active或active.submitting truthy時直接return。kCode與definition.code用Number(...)||0。
沒有closeRequest且active.kind==='alarm'時：mask零只接受selectedCode零；mask非零要求selectedCode非零且mask&selectedCode非零。這個位元判斷沒有要求只選一個bit。
closeRequest truthy會略過上述本地mask限制；不能推論C++也略過驗證。通過後先active.submitting=true，保存kind及displayKind或kind。
channel==='nonstop-local'只closeView、active=null、return，不寫response。其餘先status，再responseFor，再submit(...).then。
submit fulfilled後發ht-dialog-response事件、closeView、組closeResponseFor，追加normalResponse.file，active=null後才送submitCloseResponse。
這表示一般submit回傳成功與close回覆完成分成兩段。後段Promise rejection進catch時若active已null直接return；沒有本body的UI錯誤訊息。
catch時若active仍在，清active.submitting並status(error.message)。responseFor、channels[kind]存取、直接submit的同步throw及then方法不存在，不全在這條catch涵蓋內。
本輪沒有執行這些失敗情況，不把可能的順序差異寫成已確認現場鎖死或丟回覆。

## 關閉回覆佇列與重試

submitCloseResponse在closeResponseSubmitting truthy時，只於backlog.length<16才push；不論是否push都resolve('close-response-queued')。
因此queued回傳不證明已入列，亦不代表送達。滿佇列時本body沒有保留該value，需連呼叫情境另查，不擅改程式。
空閒時設pendingCloseResponse=value、busy=true，呼叫submit後then清pending及busy；catch只清busy再throw，保留pending供後續重試。
submit同步throw或回傳不具then，發生在Promise chain建立之前，本body沒有catch清busy；與正常rejection分開描述。
全文尾端setInterval的context：有pending且不busy先重送；否則pending空、busy false、backlog非空才shift再送。沒有把這個anonymous callback追加完整函式credit。
POLL_MS與closeChannel狀態、inspectClose／recent[]／closeOne完整原文保存，但其完整語意audit及所有C++consumer待續。
rejectClose組CLOSE_REQUEST_REJECTED回覆，dialogWasOpen用!!active，送closeResponse後catch吞rejection；closeResponseFor或submitCloseResponse同步throw並不由後面的catch捕捉。
