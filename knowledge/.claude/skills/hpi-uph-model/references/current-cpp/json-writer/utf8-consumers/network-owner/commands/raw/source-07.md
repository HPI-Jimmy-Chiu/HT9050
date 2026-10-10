# 原文：queue, readOnly and control-owner fields with original threading comments（1）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin`34c8e2e164014a93699f0b0a712366f5da01ebd2`；本頁payload SHA256 `b7cef0e6e24c95a7f640c35b2b24c14e6431a187ec436a8386a1bbe963e0f4e9`。
完整函式完成數見manifest；context／helper／adapter與拆頁不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
    CommandQueue*        queue;
    std::atomic<bool>    readOnly;
    // AI(W906-FW-W3) 20260819: single-operator control token. Owned by the
    // socket thread (all writes happen there); atomic so the UI thread's
    // ControlOwner() read is race-free. 0 = nobody holds it.
    std::atomic<unsigned long long> ctrlOwner_{0};  std::atomic<int> liveWs_{0};   // AI(W906-MODAL-WAKE) 20260926: live WebSocket connections (++ at the upgrade :1138, -- in CloseConn :892, 0 in CloseAllSockets :755) -- LiveWebSocketCount()
    unsigned long long              ctrlLastCmdMs_ = 0;   // socket thread only

<!-- preserved-content:end -->
```
