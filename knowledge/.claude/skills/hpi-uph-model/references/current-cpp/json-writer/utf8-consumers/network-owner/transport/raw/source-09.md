# 原文：WebBridgeServer::Impl::SendJson

[上層](../index.md)。固定來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `8cd0be50e14294b42774510c7fa8accc51b511d6`；SHA256 `cd702ba30807a25499b308a81bf4cb215a52e5186a5c9f4c7d8ff979438c1bf3`。
保留原正文與comment；完整函式計數1；未執行程式。

```cpp
<!-- preserved-content:start -->
void WebBridgeServer::Impl::SendJson(Conn& c, const std::string& json)
{
    Enqueue(c, sib::EncodeServerFrame(sib::kOpText, json));
}


<!-- preserved-content:end -->
```
