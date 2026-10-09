# 原文：WsaAcquire

[上層](../index.md)。來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `bebe1bf379f961899a85cc8376ebf1aacc9cf2df`；SHA256 `36abd5c115e5cc564a3a22b2cc40f8494418d6aea607d317d013f35fcd023de3`。
完整函式計數1；原comment／metadata與正文保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
bool WsaAcquire(std::string* err)
{
    WbGuard lk(g_wsaMx);
    if (g_wsaRefs > 0) { ++g_wsaRefs; return true; }
    WSADATA wsad;
    const int rc = WSAStartup(MAKEWORD(2, 2), &wsad);
    if (rc != 0) {
        if (err) {
            std::ostringstream os;
            os << "WSAStartup failed, rc=" << rc;
            *err = os.str();
        }
        return false;
    }
    g_wsaRefs = 1;
    return true;
}


<!-- preserved-content:end -->
```
