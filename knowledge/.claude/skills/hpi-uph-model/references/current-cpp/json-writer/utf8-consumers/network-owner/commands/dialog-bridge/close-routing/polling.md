# close與一般request的recent輪詢

[上層](index.md)／[inspectClose](locators.md#inspectclose)／[inspect](locators.md#inspect)／[inspectRecent](locators.md#inspectrecent)。來源 `web/page/dialog-bridge.js`。

## inspectClose

closeChannel.loading truthy先return，否則true後loadFresh(closeChannel.request)。load成功一進callback就清loading。
頂層request須pending、有closeRequestId且Number(seq)||0大於lastSeq，否則return；即使recent有其他較新項，頂層不合條件時也不處理。
recent truthy且length非零用slice，否則[request]；若沒有Number(e.seq)與頂層seq相等的一筆，再push頂層request。
按(Number(seq)||0)升序；逐筆只檢查entry、有closeRequestId及s>lastSeq，沒有在這個內層再次檢查entry.state==='pending'。
先closeChannel.lastSeq=s，再try closeOne(entry)；同步throw只warn，不還原lastSeq。下一輪不會單靠同seq再重做，這不是成功送達保證。
forEach不await closeOne返回的Promise；多筆關閉可在一般complete尚未完成時接著進來，實際submitting／backlog結果依相依函式與狀態。
最外catch清loading；loadFresh同步throw或不具then發生在chain建立前，不由末端catch涵蓋。

## inspect單筆路徑

取channels[kind]，loading為true就return；load成功先清loading、算seq。pending且recent.length>1並且!routing先交inspectRecent再return，這個分支在頂層requestId／seq新舊判斷之前。
單筆須pending、有requestId且seq>lastSeq；routing為true先return，否則true後resolveDisplay。
成功callback先routing=false，將r.info與request.nonStopInfo合併，後者同名欄位覆蓋前者；接著lastSeq=seq、enqueue、drain。
lastSeq不因active畫面忙就跳過，原裁決是避免單格request覆寫而漏收；enqueue去重與實際出畫面由相依函式決定。
resolveDisplay rejection只清routing；callback中同步throw會成為返回Promise的rejection，但該then回傳值沒有接入外層loadFresh的chain，本body沒有完整接住所有錯誤。

## inspectRecent逐筆路徑

複製request.recent；沒有相同Number(seq)一筆才push頂層request，再filter pending、有requestId、Number(seq)>channel.lastSeq，最後升序。
filter先建立整份list，不等於每一步重新filter；同seq／重複requestId如何處理由後面的lastSeq與enqueue決定，本body沒有全體去重set。
list空直接return；否則routing=true並以next(i)逐筆resolveDisplay。成功合併nonStopInfo，再Math.max推lastSeq、enqueue、drain，然後next(i+1)。
所有項目走完才routing=false；resolveDisplay rejection清routing並停止本輪後續，lastSeq仍停在已成功處理項。
resolveDisplay同步throw或成功callback在enqueue／drain等位置throw，這支body沒有另接catch清routing；不能推成所有失敗都有相同重試結果。
單筆及recent的lastSeq推進都在resolveDisplay成功後；與inspectClose先推進再closeOne不同。沒有在本輪執行失敗、競態或重送情境。
next是nested callback，本單元將完整inspectRecent body原文保留為一個具名函式，不另加nested function credit。
檔尾POLL_MS輪詢的順序仍為pending close重送／backlog、inspectClose、drain、inspect alarm與message；同一835全文保存件可查，callback不重計完成數。
