# 原文：WebBridgeServer::Impl::Wake

[上層](../index.md)。來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `bebe1bf379f961899a85cc8376ebf1aacc9cf2df`；SHA256 `c83782e1c84e6a32beee7f2d4f3868f5c2b19d5f80c7b598a9421a76b410e1b5`。
完整函式計數1；原comment／metadata與正文保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
void WebBridgeServer::Impl::Wake()
{
    if (wake_ == INVALID_SOCKET) return;
    const char b = 'w';
    sendto(wake_, &b, 1, 0, reinterpret_cast<sockaddr*>(&wakeAddr_), sizeof(wakeAddr_));
}


<!-- preserved-content:end -->
```
