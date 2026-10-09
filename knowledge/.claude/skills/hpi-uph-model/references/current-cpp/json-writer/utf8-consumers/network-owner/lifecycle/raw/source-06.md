# 原文：WebBridgeServer::Impl::Stop

[上層](../index.md)。來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `bebe1bf379f961899a85cc8376ebf1aacc9cf2df`；SHA256 `fc99a9d1921d342ec249557fe4de2e7c19833fea9c95d557534a4e1dcdcf0d82`。
完整函式計數1；原comment／metadata與正文保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::Stop()
{
    WbGuard lk(lifeMx_);
    stopFlag.store(true);
    if (th_.joinable()) {
        Wake();                 // prompt: do not wait out pollIntervalMs
        th_.join();
    }
    const bool wasRunning = running.exchange(false);

    // The socket thread closes the listener and the client sockets on its way
    // out; the wakeup socket is ours to close once nobody can select() on it.
    if (wake_ != INVALID_SOCKET) { closesocket(wake_); wake_ = INVALID_SOCKET; }
    if (listener_ != INVALID_SOCKET) { closesocket(listener_); listener_ = INVALID_SOCKET; }

    {
        WbGuard ol(outMx_);
        outQ_.clear();
    }
    {
        WbGuard pl(pendMx_);
        pending_.clear();
        pendingOrder_.clear();
    }
    boundPort.store(0);
    if (wasRunning) WsaRelease();   // balances the Start() that succeeded
}


<!-- preserved-content:end -->
```
