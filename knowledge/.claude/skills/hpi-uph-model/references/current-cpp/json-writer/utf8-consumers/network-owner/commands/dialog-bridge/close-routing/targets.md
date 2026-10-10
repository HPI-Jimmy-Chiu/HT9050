# target比對與兩層關閉

[上層](index.md)／[sameTarget](locators.md#sametarget)／[dropQueued](locators.md#dropqueued)／[closeOne](locators.md#closeone)。來源 `web/page/dialog-bridge.js`。

## sameTarget與dropQueued

sameTarget先要求target與req存在；channel、requestId用嚴格相等，requestSeq與req.seq各經Number再嚴格相等。
沒有在這支body檢查有限數值、正值或schema；NaN不等於NaN，Number的其他轉型仍照原式，不能改述成整數型別驗證。
dropQueued先掃queueStop再掃queueNS；每層從頭找，第一筆sameTarget即splice(i,1)[0]並return，不會清掉所有同target項。
沒有命中回null；本body只移除排隊資料，不呼叫render、不等待一般normal response，也不直接執行機台動作。

## closeOne的順序

先取request.target或空物件。activeNS及其request存在且sameTarget命中，先closeNonStop，再送closeResponseFor(null,request,true,null)。
此不停機分支沒有檢查resolvedAction；回覆沒有normalResponse.file，dialogWasOpen=true由呼叫位置給定。
再比active及active.request；resolvedAction.name缺值或嚴格等NONE就rejectClose，其他name組成definition交complete並直接return。
這支body沒有return或await complete的Promise；complete自身會在active.submitting時return，不能把closeOne呼叫返回當成回覆已成功。
其後dropQueued命中就送close回覆、dialogWasOpen=false；排隊移除發生在送答之前，送答失敗不由本body把項目放回queue。
沒有命中且!active，回No dialog is open；有active則Target does not match the active dialog。activeNS存在但target不符且active空時仍會走前者文字。
nonstop與queued分支的.catch吞Promise rejection；closeNonStop、closeResponseFor與submitCloseResponse同步throw仍可向上冒出。
原20260922「實體IO四種都能解除」裁決及S17D佇列同步移除註解原文沿用保存；本輪沒有由這九body驗證四種IO線路與C++互鎖。
