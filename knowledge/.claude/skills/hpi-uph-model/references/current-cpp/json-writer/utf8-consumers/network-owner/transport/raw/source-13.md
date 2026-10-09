# 原文：control ownership, liveWs and loop atomics with original comments

[上層](../index.md)。固定來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `8cd0be50e14294b42774510c7fa8accc51b511d6`；SHA256 `255d02706ce3234f3abeb25b507210d2b5a7f93ab1267613a4913df22346d57e`。
保留原正文與comment；context完整函式計數0；未執行程式。

```cpp
<!-- preserved-content:start -->
    // AI(W906-FW-W3) 20260819: single-operator control token. Owned by the
    // socket thread (all writes happen there); atomic so the UI thread's
    // ControlOwner() read is race-free. 0 = nobody holds it.
    std::atomic<unsigned long long> ctrlOwner_{0};  std::atomic<int> liveWs_{0};   // AI(W906-MODAL-WAKE) 20260926: live WebSocket connections (++ at the upgrade :1138, -- in CloseConn :892, 0 in CloseAllSockets :755) -- LiveWebSocketCount()
    unsigned long long              ctrlLastCmdMs_ = 0;   // socket thread only
    std::atomic<bool>    running;
    std::atomic<bool>    stopFlag;
    std::atomic<unsigned short> boundPort;

<!-- preserved-content:end -->
```
