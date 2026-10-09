# 原文：receive and parser constants, reused caps plus kRecvChunk

[上層](../index.md)。固定來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `8cd0be50e14294b42774510c7fa8accc51b511d6`；SHA256 `bc4392540ff71b49946bfb899f24c27634da0923464432c5d1504daa8fb93133`。
保留原正文與comment；context完整函式計數0；未執行程式。

```cpp
<!-- preserved-content:start -->
const size_t kMaxHttpHead    = 32u * 1024u;   // request head before we give up
const size_t kMaxWsMessage   = 64u * 1024u;   // reassembled text message cap
const size_t kMaxPendingAcks = 4096u;         // unanswered CompleteCommand slots
const size_t kRecvChunk      = 8192u;

<!-- preserved-content:end -->
```
