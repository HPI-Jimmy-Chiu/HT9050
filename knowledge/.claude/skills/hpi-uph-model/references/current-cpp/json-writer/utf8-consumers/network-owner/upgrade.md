# DoWebSocketUpgrade：本機Origin與連上後初始狀態

[上層](index.md)；[WebBridgeServer::Impl::DoWebSocketUpgrade原文](raw/source-05.md)；[AcceptKey原文](raw/source-01.md)；[entire WebBridgeServer header original configuration/API/metadata原文](raw/source-07.md)、[entire WebBridgeServer header original configuration/API/metadata原文](raw/source-08.md)。

`ProcessHttpHead`先做GET／`cfg.wsPath`分流；`DoWebSocketUpgrade`的target參數在本體未用。
key缺或空、version字串不等13時，加`wsRejected`、回400、附Sec-WebSocket-Version:13、設`closeAfterFlush`。
本body沒有呼叫`IsValidWebSocketKey`或`CheckWebSocketUpgrade`；非空key不能因此稱為已驗證Base64／16-byte形狀。
`AcceptKey`只轉交已保存的[ComputeAcceptKey](../handshake/responses.md)，與完整opening-handshake合規驗證分清。

`cfg.checkOrigin`開時：缺Origin在此放行；有Origin才進字面比較。
先去尾端slash或空白，再將ASCII A-Z轉小寫，與boundPort下六個http／https、127.0.0.1／localhost／[::1]字串比較。
其他Origin回403並加`wsRejected`，包括字面null。header的原註解與metadata照存；實際cfg、proxy部署、browser與客戶實例未驗證。
這是upgrade的Origin gate，不等於登入／control token／命令授權；那些owner路徑另查。

通過後自行組HTTP/1.1 101、Upgrade／Connection／Sec-WebSocket-Accept並enqueue。
接著設`c.isWs`、加`liveWs_`與`wsAccepted`，更新lastRecvMs／lastPingMs。
`SnapRead`取得map後，`SendJson`先送snapshot形狀，再設lastSent／sentSnapshot與snapshotsSent。
`snapshotBytes`在此加的是JSON字串sf大小，不能直接當socket實際傳送量或UPH量測。

pendingQueryQid非0時，在outMx鎖內複製pendingQueryFrame，鎖外SendJson並加queriesSent。
保留Q30 replay原註解與日期；其模態等待說明是來源註解，不代表本輪驗證了現場告警解除。
query replay排在snapshot後；其sender、qid清除、queue／鎖與SendJson／SnapRead callee完整行為仍待追。
最後，handshake後殘留`c.in`非空就交`ProcessWsBytes`；空時回true。
101與snapshot已enqueue不代表client已收到；WS錯誤與後續flush/drop責任見[訊息處理](messages.md)。
