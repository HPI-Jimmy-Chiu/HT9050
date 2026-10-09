# recv、排隊與partial send

[上層](index.md)；[ReceiveInto](raw/source-05.md)、[Enqueue](raw/source-06.md)、[Flush](raw/source-07.md)、[SendJson](raw/source-09.md)；[原常數](raw/source-11.md)；[NowMs context](raw/source-12.md)。

ReceiveInto每次用8192 bytes的kRecvChunk stack buffer recv。n>0先append到c.in、更新lastRecvMs；短於chunk即break，滿chunk繼續recv。
n==0立刻回false；負值遇WSAEWOULDBLOCK才break，其餘回false。本body在EOF/error返回前不將已累積c.in交parser。
因此本次已收到資料不等於已完成HTTP／WS處理；原碼表明的順序與可重現實機現象須分別核對。

recv迴圈離開後，非WS的c.in.size()>32KiB回false，否則ProcessHttpHead；WS則ProcessWsBytes。
32KiB是**離開接收迴圈後**的HTTP input檢查，包含累積input，不是只量CRLF前段；連續滿chunk時append發生在檢查前。
WS64KiB訊息cap與partial decoder檢查在[既有message路由](../messages.md)，不能把它們稱為此recv loop的記憶體硬上限。

Enqueue只做c.out += bytes；SendJson先以EncodeServerFrame(kOpText,json)編碼再Enqueue，沒有在這裡socket send或驗JSON。
相同generic encoder與server角色adapter沿用[既有adapter](../adapters.md)，不重新計完成。

| Flush結果 | c.out與socket |
|---|---|
| send n>0 | 只erase前n bytes並繼續 |
| 其餘，WSAGetLastError()==WSAEWOULDBLOCK | return，out保留待下一輪嘗試 |
| 其他error值 | closesocket、s=INVALID_SOCKET、return，out未清 |

send長度以static_cast<int>(c.out.size())傳入；本body未在轉型前作上界檢查。
n==0也讀WSAGetLastError而未另列分支；僅記原碼，沒有推定OS error值或加新修正。
out清空表示這段程式已移除send正回傳的bytes，不代表peer收到、解析、ACK或機台動作完成。
backlog與closeAfterFlush的最終清理在[ThreadMain](loop.md)，maxSendBacklog是在flush後核，不是Enqueue admission cap。
