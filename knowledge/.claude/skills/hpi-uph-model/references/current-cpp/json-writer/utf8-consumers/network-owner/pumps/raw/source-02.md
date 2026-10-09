# 原文：WebBridgeServer::Impl::SendAck（1）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin`d95fc5fc504d4e2b93a68f754874a72cf1166471`；本頁payload SHA256 `363f6ce7097ac0d16b60eaaa4046e117eb0e4959ca1c1e3c786fb5bda68d4c2b`。
完整函式的完成數記於manifest；拆頁與context均不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
void WebBridgeServer::Impl::SendAck(Conn& c, double id, bool ok, const std::string& error)
{
    SendJson(c, AckJson(id, ok, error));  if (g_W906OpLogHook) g_W906OpLogHook("ACK", c.id, id, ok, error, std::string());   //AI(W906-OPLOG) 20260928
    WbGuard sl(statsMx);
    ++stats.acksSent;
}


<!-- preserved-content:end -->
```
