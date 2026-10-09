# 原文：WebBridgeServer::Impl::CloseConn

[上層](../index.md)。固定來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `8cd0be50e14294b42774510c7fa8accc51b511d6`；SHA256 `6a0903ff47653f18f17ee533fadc6657c4b67f39dd5e4a31b479455fa628aa42`。
保留原正文與comment；完整函式計數1；未執行程式。

```cpp
<!-- preserved-content:start -->
void WebBridgeServer::Impl::CloseConn(size_t index)
{
    if (index >= conns_.size()) return;
    // AI(W906-FW-W3) 20260819: the control token dies with its connection
    // (design doc section 3, "持有權隨 ws 連線生命週期").
    if (conns_[index].id == ctrlOwner_.load()) ctrlOwner_.store(0);  if (conns_[index].isWs) liveWs_.fetch_sub(1);   // AI(W906-MODAL-WAKE) 20260926
    if (conns_[index].s != INVALID_SOCKET) closesocket(conns_[index].s);
    conns_.erase(conns_.begin() + static_cast<long>(index));
    WbGuard sl(statsMx);
    ++stats.connectionsClosed;
    stats.liveConnections = static_cast<int>(conns_.size());
}


<!-- preserved-content:end -->
```
