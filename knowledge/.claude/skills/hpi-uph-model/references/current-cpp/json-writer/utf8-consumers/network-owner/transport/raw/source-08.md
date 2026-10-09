# 原文：WebBridgeServer::Impl::PumpLiveness

[上層](../index.md)。固定來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `8cd0be50e14294b42774510c7fa8accc51b511d6`；SHA256 `07caab95fc13d454440a57e2c4f3b0d052dd6d91a3134c43d8837420f6252edd`。
保留原正文與comment；完整函式計數1；未執行程式。

```cpp
<!-- preserved-content:start -->
// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::PumpLiveness()
{
    const unsigned long long now = NowMs();

    // AI(W906-FW-W3) 20260819: idle operator forfeits the token (design doc
    // section 3, default 10 min without an accepted command).
    if (ctrlOwner_.load() != 0 && cfg.controlIdleTimeoutMs > 0 &&
        now - ctrlLastCmdMs_ > static_cast<unsigned long long>(cfg.controlIdleTimeoutMs)) {
        ctrlOwner_.store(0);
    }
    for (size_t i = conns_.size(); i-- > 0;) {
        Conn& c = conns_[i];
        if (!c.isWs) continue;

        if (cfg.idleTimeoutMs > 0 &&
            now - c.lastRecvMs > static_cast<unsigned long long>(cfg.idleTimeoutMs)) {
            CloseConn(i);          // dead peer, or one that stopped ponging
            continue;
        }
        if (cfg.pingIntervalMs > 0 &&
            now - c.lastPingMs >= static_cast<unsigned long long>(cfg.pingIntervalMs)) {
            Enqueue(c, sib::EncodeServerFrame(sib::kOpPing, std::string()));
            c.lastPingMs   = now;
            c.awaitingPong = true;
            WbGuard sl(statsMx);
            ++stats.pingsSent;
        }
    }
}


<!-- preserved-content:end -->
```
