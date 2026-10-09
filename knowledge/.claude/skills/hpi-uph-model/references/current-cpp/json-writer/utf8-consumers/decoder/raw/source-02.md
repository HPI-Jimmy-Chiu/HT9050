# 原文 02：Fail

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `Fail` 定位。
固定來源 `fd50c7d93d928a1ed48d9cbf5b668e39d471c758`；SHA256 `f3588ab30327337bd29bcb0c8b545f4ba936740005eb3a3afe1f4f30627f6a67`。
完整函式。保留原註解；未執行程式或測試。

```cpp
<!-- preserved-content:start -->
bool WsDecoder::Fail(uint16_t code, const char* reason) {
    // First failure wins: once the stream is broken, later observations are
    // just noise and must not overwrite the real cause.
    if (!failed_) {
        failed_ = true;
        closeCode_ = code;
        closeReason_ = reason ? reason : "";
    }
    return false;
}

<!-- preserved-content:end -->
```
