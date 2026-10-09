# ProcessWsBytes：訊息交付、control與關閉旗標

[上層](index.md)；[WebBridgeServer::Impl::ProcessWsBytes原文](raw/source-06.md)；[TryDecode契約](adapters.md)；[original HTTP/WS size constants only原文](raw/source-12.md)。

迴圈呼叫TryDecode。rc=0時，回傳PendingBytes是否不超過64 KiB+1024；partial bytes與fragment都在decoder中。
rc<0立刻回false。rc=1後再查f.masked；本adapter交付欄位固定true，底層decoder先驗mask。
此body遇錯回false，沒有在這個分支組CloseCode response；不能宣稱decode failure已向client送close 1002／1007／1009。
實際drop／close與Flush需要server loop／ReceiveInto／CloseConn完整body接續核對。

| 訊息 | 本body動作 |
|---|---|
| Ping | enqueue同payload的server Pong |
| Pong | awaitingPong=false，pongsReceived加一 |
| Close | enqueue空payload的server Close、設closeAfterFlush、回true |
| Binary | 回false，這條協定路徑只接受JSON text |
| Text且FIN | 檢payload不超過64 KiB，交HandleTextMessage |
| Text非FIN／Continuation | 原legacy fragment分支保留；本TryDecode交付FIN=true的完整訊息 |
| 其他opcode | 回false |

Close不在此回顯client close code／reason，也不保證關閉frame已flush。
Text進HandleTextMessage不代表JSON已接受、命令已執行或已通過機台互鎖，後續parser／queue／UI owner另查。
每個訊息處理後，c.s若為INVALID_SOCKET回false；回true只是此處選擇繼續／flush的狀態，非實機測試結果。
