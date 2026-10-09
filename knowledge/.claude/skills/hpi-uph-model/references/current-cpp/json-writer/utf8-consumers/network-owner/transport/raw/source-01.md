# 原文：WebBridgeServer::Impl::ThreadEntry

[上層](../index.md)。固定來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `8cd0be50e14294b42774510c7fa8accc51b511d6`；SHA256 `9fcce9128104e8997c2979df6810e7eddafa1cf87beca4a3bc3b09e946399a9d`。
保留原正文與comment；完整函式計數1；未執行程式。

```cpp
<!-- preserved-content:start -->
// -----------------------------------------------------------------------------
//  The socket thread. Everything below runs here and nowhere else.
// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::ThreadEntry(void* self)
{
    static_cast<WebBridgeServer::Impl*>(self)->ThreadMain();
}


<!-- preserved-content:end -->
```
