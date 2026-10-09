# 原文：WebBridgeServer::Impl::CloseAllSockets

[上層](../index.md)。固定來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `8cd0be50e14294b42774510c7fa8accc51b511d6`；SHA256 `3fb200e71eaf3a1cfeac9d34683b03d588db398dbb12a48e305e8800006307e9`。
保留原正文與comment；完整函式計數1；未執行程式。

```cpp
<!-- preserved-content:start -->
// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::CloseAllSockets()
{
    for (size_t i = 0; i < conns_.size(); ++i) {
        if (conns_[i].s != INVALID_SOCKET) closesocket(conns_[i].s);
    }
    conns_.clear();  liveWs_.store(0);   // AI(W906-MODAL-WAKE) 20260926
    if (listener_ != INVALID_SOCKET) { closesocket(listener_); listener_ = INVALID_SOCKET; }
    WbGuard sl(statsMx);
    stats.liveConnections = 0;
}


<!-- preserved-content:end -->
```
