# 原文：WebBridgeServer::Impl::Flush

[上層](../index.md)。固定來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `8cd0be50e14294b42774510c7fa8accc51b511d6`；SHA256 `ca29dbab8e1ca4808b3d718547db019c8d1b59baf63aa09351069caa8d1050e5`。
保留原正文與comment；完整函式計數1；未執行程式。

```cpp
<!-- preserved-content:start -->
void WebBridgeServer::Impl::Flush(Conn& c)
{
    while (!c.out.empty()) {
        const int n = send(c.s, c.out.data(), static_cast<int>(c.out.size()), 0);
        if (n > 0) {
            c.out.erase(0, static_cast<size_t>(n));
            continue;
        }
        const int e = WSAGetLastError();
        if (e == WSAEWOULDBLOCK) return;    // stays buffered; write-set retries
        closesocket(c.s);
        c.s = INVALID_SOCKET;
        return;
    }
}


<!-- preserved-content:end -->
```
