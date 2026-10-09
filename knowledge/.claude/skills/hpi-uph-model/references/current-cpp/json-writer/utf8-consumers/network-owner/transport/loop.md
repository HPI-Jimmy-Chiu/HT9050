# Socket thread與retire順序

[上層](index.md)；[ThreadEntry](raw/source-01.md)、[ThreadMain](raw/source-02.md)；[原header契約](../raw/source-07.md)。
ThreadEntry只把void pointer轉Impl並呼叫ThreadMain；實際thread建立、join與物件壽命須讀Start／Stop及Sync.h，不由trampoline保證。
原「Everything below runs here and nowhere else」註解保留為設計意圖；本次有限查讀未證明所有caller的執行緒身分。

ThreadMain在stopFlag未置位時反覆建立read／write fd_set。listener有效且conns_低於maxConnections才列入read；wake_有效也列入。
每個Conn.s列read；out非空才列write。pollIntervalMs切為tv_sec／tv_usec後呼叫select，傳回後再看stopFlag。

| select結果 | 本body行為 |
|---|---|
| SOCKET_ERROR且WSAENOTSOCK／WSAEINVAL | 逆向刪除s已為INVALID_SOCKET的紀錄，continue |
| 其他SOCKET_ERROR | WbSleepMs(10)，continue |
| rc > 0 | drain wake recvfrom；listener readable時AcceptNew；逆向ReceiveInto／Flush，keep=false時CloseConn |
| rc == 0 | 繼續下面pumps與flush／retire |

error分支只辨識本機已標INVALID_SOCKET的紀錄，不能據此宣稱所有外部失效descriptor都會自動復原。
wake drain只看recvfrom > 0，本body不分析其error碼；接受連線後的FD_ISSET及實際WinSock行為仍須平台查證。

先PumpSnapshot(false)、再PumpOutgoing、再PumpLiveness，然後逆向逐Conn：

1. out非空先嘗試Flush。
2. 此後out.size() > maxSendBacklog，slowClientDrops加一後CloseConn。
3. closeAfterFlush且out空，CloseConn。
4. s為INVALID_SOCKET，CloseConn。

maxSendBacklog是在本次flush後檢查，並非Enqueue前的配置上限；read loop與pumps暫時累積量另查。
Flush失敗後out仍非空也可能先命中slowClientDrops，所以該計數不是完整的斷線原因分類。
每處使用逆向index迴圈，再erase該連線；這是本body的索引策略，未測多執行緒重入。
迴圈離開時CloseAllSockets；running、wake與join收尾留給Start／Stop等owner核對。
