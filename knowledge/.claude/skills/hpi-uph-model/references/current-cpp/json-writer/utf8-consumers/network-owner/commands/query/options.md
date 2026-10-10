# PostQueryOptions：caller選項與最後query槽

[上層](index.md)；[完整body與YESNO原comment](raw/source-01.md)；[PostQuery固定bit表](../events.md)；[quote adapter](../../pumps/raw/source-12.md)／[local時間helper](../raw/source-05.md)。

type=query、qid直接stream，code固定空字串、kcode固定0；kind／text／options／at用sib::QuoteString。
options按vector順序逐項輸出；body沒有白名單、去重或空字串過濾，空vector仍輸出[]。不能套用PostQuery的九bit順序或把kcode=0當沒有query。
at非空沿用caller字串，空值才呼叫IsoLocalNow；不在此body驗字串格式、機台時區或取得時間成功。

持outMx_加入connId=0的broadcast Outgoing，並把同份frame與qid覆寫pendingQueryFrame_／pendingQueryQid_。
這與PostQuery共用最後一份保留槽；新query會取代先前槽。新增outQ_並不取消先前已排隊的frame。
qid=0也會排入broadcast並存frame，但既有DoWebSocketUpgrade只有pendingQueryQid_!=0才補query，不能把qid=0推為新連線可重播。
outMx_釋放後持statsMx增加queriesSent，再Wake並呼叫可選POST hook；計數／hook不是peer收到、畫面已顯示或操作員已回答的證據。

原YESNO comment談單執行緒tick與host/browser按options判答案的模型，全句保留；本輪只核本body，尚未重驗所有caller／宿主等待／browser或race保證。
HT9050與其他Handler可用這份共用V906來源定位；每台實際caller、選項語意與runtime部署仍須另核。
