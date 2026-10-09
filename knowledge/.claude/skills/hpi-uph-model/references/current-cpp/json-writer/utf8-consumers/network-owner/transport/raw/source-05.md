# 原文：WebBridgeServer::Impl::ReceiveInto

[上層](../index.md)。固定來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `8cd0be50e14294b42774510c7fa8accc51b511d6`；SHA256 `8f5a5140af107cfd90ead6f790256d53434209dd876e55ef9c06b929357ec8da`。
保留原正文與comment；完整函式計數1；未執行程式。

```cpp
<!-- preserved-content:start -->
bool WebBridgeServer::Impl::ReceiveInto(Conn& c)
{
    for (;;) {
        char buf[kRecvChunk];
        const int n = recv(c.s, buf, static_cast<int>(sizeof(buf)), 0);
        if (n > 0) {
            c.in.append(buf, static_cast<size_t>(n));
            c.lastRecvMs = NowMs();
            if (n < static_cast<int>(sizeof(buf))) break;
            continue;
        }
        if (n == 0) return false;                        // peer closed
        const int e = WSAGetLastError();
        if (e == WSAEWOULDBLOCK) break;
        return false;
    }

    if (!c.isWs) {
        if (c.in.size() > kMaxHttpHead) return false;    // head never terminated
        return ProcessHttpHead(c);
    }
    return ProcessWsBytes(c);
}


<!-- preserved-content:end -->
```
