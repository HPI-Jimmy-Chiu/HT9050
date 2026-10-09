# PumpOutgoing：queue移交與收件連線

[上層](index.md)；[完整body](raw/source-04.md)；[CompleteCommand](ack.md)；[structs](raw/source-09.md)／[fields與Q30原說明](raw/source-10.md)。

持outMx_時outQ_.empty()即return；否則batch.swap(outQ_)，離開guard後才遍歷local batch與conns_。
只送isWs且!closeAfterFlush的連線；Outgoing.connId==0廣播給這些連線，非0只送id相符的連線。
每份Outgoing若沒有符合的連線，本body不會重新放回outQ_，也沒有caller ACK／retry結果。
新加入outQ_的item不在已交換的batch內；下一次pump取用時機見[已完成ThreadMain](../transport/index.md)，不在此推定deadline。
SendJson／Enqueue／Flush沿用既有transport，本文不重算其body，也不把送入Conn queue等同send成功或peer receipt。

pendingQueryFrame_／pendingQueryQid_的Q30重播理由保留在[fields](raw/source-10.md)，它是額外狀態；不能只看PumpOutgoing便聲稱所有query永久丟失或已有完整retry保證。
本次沒有把PostQuery／SendSnapshot與命令owner計為新增完成。後續接HandleTextMessage、pending容量與重播／browser caller，保持歷史問題與現行正文適用範圍。
