# 原文：WebBridgeServer::Impl::AcceptNew

[上層](../index.md)。固定來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `8cd0be50e14294b42774510c7fa8accc51b511d6`；SHA256 `8ba6c63e41ef5b402b0ffabb0ef261817db0e1e38763029e8557c4152676fe43`。
保留原正文與comment；完整函式計數1；未執行程式。

```cpp
<!-- preserved-content:start -->
// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::AcceptNew()
{
    for (;;) {
        sockaddr_in from;
        int fromLen = sizeof(from);
        const SOCKET s = accept(listener_, reinterpret_cast<sockaddr*>(&from), &fromLen);
        if (s == INVALID_SOCKET) return;

        if (static_cast<int>(conns_.size()) >= cfg.maxConnections) {
            closesocket(s);   // at capacity: refuse now, do not queue work
            continue;
        }

        SetNonBlocking(s);
        // Small frames, latency matters more than packing.
        int one = 1;
        setsockopt(s, IPPROTO_TCP, TCP_NODELAY,
                   reinterpret_cast<const char*>(&one), sizeof(one));

        Conn c;
        c.s          = s;
        c.id         = nextConnId_++;
        c.lastRecvMs = NowMs();
        c.lastPingMs = c.lastRecvMs;
        conns_.push_back(c);

        WbGuard sl(statsMx);
        ++stats.connectionsAccepted;
        stats.liveConnections = static_cast<int>(conns_.size());
    }
}


<!-- preserved-content:end -->
```
