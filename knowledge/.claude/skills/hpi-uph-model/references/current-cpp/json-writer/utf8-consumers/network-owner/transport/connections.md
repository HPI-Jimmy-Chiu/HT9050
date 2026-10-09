# Accept、CloseConn與CloseAllSockets

[上層](index.md)；[AcceptNew](raw/source-03.md)、[CloseConn](raw/source-04.md)、[CloseAllSockets](raw/source-10.md)；[Conn原文](../raw/source-11.md)；[控制欄位](raw/source-13.md)。

AcceptNew反覆accept，INVALID_SOCKET即return；本body未區分would-block或其他accept失敗。
達到maxConnections時立刻closesocket新socket，continue；未建Conn或增加accepted計數。
SetNonBlocking(s)之後呼叫setsockopt TCP_NODELAY，本body不檢查兩個設定的成功；平台設定helper另續，不宣稱已啟用。
新Conn記s、nextConnId_++、NowMs所得lastRecvMs／lastPingMs，push_back後connectionsAccepted++與liveConnections=size。
HTTP連線也算connectionsAccepted；isWs由Conn預設false，WebSocket upgrade計數沿用[上層](../upgrade.md)。

CloseConn(index)超出conns_範圍直接return。
匹配ctrlOwner_的connId即store(0)；isWs為true才liveWs_.fetch_sub(1)，再關有效s、erase，connectionsClosed++與liveConnections=size。
原控制權隨連線死亡與AI日期comment完整保存；這裡沒有撤銷已排程機台命令、取消PendingAck或回報browser ACK的正文。
ctrlOwner_／liveWs_ atomics對外讀取宣告不能替所有queue、Conn或完整命令owner提供同步證明。

CloseAllSockets逐Conn關有效s、conns_.clear()，**liveWs_.store(0)**；關listener並設INVALID_SOCKET，再令stats.liveConnections=0。
此body**不關wake_、不清ctrlOwner_**，也沒有逐Conn增加connectionsClosed。
前intake把listener/wake清理與liveWs reset寫錯；[證據](evidence.md)保留原註記及更正，不據錯誤註記改程式。
批次清理與逐連線CloseConn的計數／控制權不同，Start／Stop是否另做收尾尚未在本單元完成。
