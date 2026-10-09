# 原文：WsaRelease

[上層](../index.md)。來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `bebe1bf379f961899a85cc8376ebf1aacc9cf2df`；SHA256 `f8af2c7d7b6edcc88e6cabbf10be1797fd7750fd0fdaaa2a8e2d7281502ccbca`。
完整函式計數1；原comment／metadata與正文保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
void WsaRelease()
{
    WbGuard lk(g_wsaMx);
    if (g_wsaRefs <= 0) return;
    if (--g_wsaRefs == 0) WSACleanup();
}


<!-- preserved-content:end -->
```
