# 原文：original WSA ref-counter globals and host-app rationale

[上層](../index.md)。來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `bebe1bf379f961899a85cc8376ebf1aacc9cf2df`；SHA256 `7fad26a9903c6cb469c81e0824530b32b651fbe460c66e1dfc1c8408f95bfd1d`。
完整函式計數0；原comment／metadata與正文保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
// Two servers in one process, or a host app that already called WSAStartup,
// must both keep working. Winsock itself refcounts, but we still balance our
// own calls exactly so our WSACleanup never pulls the rug out from under the
// host application.
WbMutex g_wsaMx;
int        g_wsaRefs = 0;

<!-- preserved-content:end -->
```
