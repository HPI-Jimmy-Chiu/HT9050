# 原文：WebBridgeServer::Impl::PumpOutgoing（1）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin`d95fc5fc504d4e2b93a68f754874a72cf1166471`；本頁payload SHA256 `5ea4ae29df73f5d30fdb58c01d1e61ab0eca5604c6451236c31a9c3b991173b7`。
完整函式的完成數記於manifest；拆頁與context均不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
// -----------------------------------------------------------------------------
//  Frames handed over by the UI thread (acks and alarms).
// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::PumpOutgoing()
{
    std::deque<Outgoing> batch;
    {
        WbGuard ol(outMx_);
        if (outQ_.empty()) return;
        batch.swap(outQ_);
    }

    for (std::deque<Outgoing>::const_iterator it = batch.begin(); it != batch.end(); ++it) {
        for (size_t i = 0; i < conns_.size(); ++i) {
            Conn& c = conns_[i];
            if (!c.isWs || c.closeAfterFlush) continue;
            if (it->connId != 0 && c.id != it->connId) continue;
            SendJson(c, it->frame);
        }
    }
}


<!-- preserved-content:end -->
```
