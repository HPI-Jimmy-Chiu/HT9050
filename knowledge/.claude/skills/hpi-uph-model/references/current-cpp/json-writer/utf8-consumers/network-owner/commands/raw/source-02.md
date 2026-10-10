# 原文：WebBridgeServer::Impl::PostAlarm（1）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin`34c8e2e164014a93699f0b0a712366f5da01ebd2`；本頁payload SHA256 `51a4c7febe0f245e7d04c8cda11e1637b4b7f945a164b79eff4d886cbbc7a9a4`。
完整函式完成數見manifest；context／helper／adapter與拆頁不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
void WebBridgeServer::Impl::PostAlarm(const std::string& code, const std::string& text,
                                     const std::string& at)
{
    std::ostringstream os;
    os << "{\"type\":\"alarm\",\"code\":" << sib::QuoteString(code)
       << ",\"text\":" << sib::QuoteString(text)
       << ",\"at\":" << sib::QuoteString(at.empty() ? IsoLocalNow() : at)
       << "}";
    {
        WbGuard ol(outMx_);
        Outgoing o;
        o.connId = 0;              // broadcast
        o.frame  = os.str();
        outQ_.push_back(o);
    }
    {
        WbGuard sl(statsMx);
        ++stats.alarmsSent;
    }
    Wake();  if (g_W906OpLogHook) g_W906OpLogHook("POST", 0, 0, true, "alarm", os.str());   //AI(W906-OPLOG) 20260929: what the operator's screen was shown
}


<!-- preserved-content:end -->
```
