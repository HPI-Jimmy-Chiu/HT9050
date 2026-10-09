# 原文 06：WebBridgeServer::Impl::ProcessWsBytes

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`；以 `WebBridgeServer::Impl::ProcessWsBytes` 定位。
pin `5eaf921a326691d45eccfcd5f00b6fdda26a9340`；分頁payload SHA256 `95922cfe2dcee3aeda5cb213c447f830dc567452fa4b5db98a14f8d98024d7c8`。
完整函式／相鄰註解或有限context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
// -----------------------------------------------------------------------------
//  WebSocket frames
// -----------------------------------------------------------------------------
bool WebBridgeServer::Impl::ProcessWsBytes(Conn& c)
{
    for (;;) {
        sib::Frame f;
        const int rc = sib::TryDecode(c.dec, c.in, f);
        if (rc == 0) {
            // Guard against a client that dribbles a giant frame header. The
            // decoder holds the partial bytes now, so ask it -- c.in has already
            // been drained into it and is always empty here.
            return sib::PendingBytes(c.dec) <= kMaxWsMessage + 1024u;
        }
        if (rc < 0) return false;                      // protocol error -> drop

        // RFC 6455 5.1: a client-to-server frame MUST be masked.
        if (!f.masked) return false;

        switch (f.opcode) {
            case sib::kOpPing:
                Enqueue(c, sib::EncodeServerFrame(sib::kOpPong, f.payload));
                break;

            case sib::kOpPong:
                c.awaitingPong = false;
                {
                    WbGuard sl(statsMx);
                    ++stats.pongsReceived;
                }
                break;

            case sib::kOpClose:
                Enqueue(c, sib::EncodeServerFrame(sib::kOpClose, std::string()));
                c.closeAfterFlush = true;
                return true;

            case sib::kOpBinary:
                return false;      // this protocol is JSON text only

            case sib::kOpText:
                if (f.fin) {
                    if (f.payload.size() > kMaxWsMessage) return false;
                    HandleTextMessage(c, f.payload);
                } else {
                    c.fragmenting = true;
                    c.fragment = f.payload;
                    if (c.fragment.size() > kMaxWsMessage) return false;
                }
                break;

            case sib::kOpCont:
                if (!c.fragmenting) return false;
                c.fragment += f.payload;
                if (c.fragment.size() > kMaxWsMessage) return false;
                if (f.fin) {
                    const std::string msg = c.fragment;
                    c.fragment.clear();
                    c.fragmenting = false;
                    HandleTextMessage(c, msg);
                }
                break;

            default:
                return false;      // reserved opcode
        }

        if (c.s == INVALID_SOCKET) return false;
    }
}


<!-- preserved-content:end -->
```
