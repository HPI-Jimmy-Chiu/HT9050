# 原文：WebBridgeServer::Impl::Enqueue

[上層](../index.md)。固定來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `8cd0be50e14294b42774510c7fa8accc51b511d6`；SHA256 `bf2e4a798ad7b4e17999c4e23bc08ce40a2ef87d966d188f02dd9068ac589698`。
保留原正文與comment；完整函式計數1；未執行程式。

```cpp
<!-- preserved-content:start -->
void WebBridgeServer::Impl::Enqueue(Conn& c, const std::string& bytes)
{
    c.out += bytes;
}


<!-- preserved-content:end -->
```
