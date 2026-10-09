# 原文 01：Reset

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `Reset` 定位。
固定來源 `fd50c7d93d928a1ed48d9cbf5b668e39d471c758`；SHA256 `ca41d1783348115557631fbcc10f4182bd3b97d30374b4cc1952001593b87081`。
完整函式。保留原註解；未執行程式或測試。

```cpp
<!-- preserved-content:start -->
void WsDecoder::Reset() {
    buf_.clear();
    frag_.clear();
    fragOpcode_ = 0;
    failed_ = false;
    sawClose_ = false;
    closeCode_ = kWsCloseNormal;
    closeReason_.clear();
}

<!-- preserved-content:end -->
```
