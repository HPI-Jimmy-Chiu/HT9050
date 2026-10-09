# 原文：WebBridgeServer::Impl::CompleteCommand（1）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin`d95fc5fc504d4e2b93a68f754874a72cf1166471`；本頁payload SHA256 `0772f0408987deb4a777ad36ed80977ac78d795939b12aa391b75445f2d3a348`。
完整函式的完成數記於manifest；拆頁與context均不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
// -----------------------------------------------------------------------------
//  UI-thread entry points. Both only touch a mutex-guarded queue, then wake the
//  socket thread. Neither blocks.
// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::CompleteCommand(unsigned long long ticket, bool ok,
                                           const std::string& error)
{
    unsigned long long connId = 0;
    double browserId = 0.0;
    {
        WbGuard pl(pendMx_);
        std::map<unsigned long long, PendingAck>::iterator it = pending_.find(ticket);
        if (it == pending_.end()) return;      // connection already gone
        connId    = it->second.connId;
        browserId = it->second.browserId;
        pending_.erase(it);
    }
    {
        WbGuard ol(outMx_);
        Outgoing o;
        o.connId = connId;
        o.frame  = AckJson(browserId, ok, error);
        outQ_.push_back(o);
    }
    Wake();  if (g_W906OpLogHook) g_W906OpLogHook("DONE", connId, browserId, ok, error, std::string());   //AI(W906-OPLOG) 20260928: tick thread
}


<!-- preserved-content:end -->
```
