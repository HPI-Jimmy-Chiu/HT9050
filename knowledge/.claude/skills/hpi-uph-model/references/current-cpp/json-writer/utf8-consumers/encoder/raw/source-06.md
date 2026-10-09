# 原文 06：EncodePing

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `EncodePing` 定位。
固定來源 `16f1ddb588497d250b9eaad227bc26dce9294ce4`；SHA256 `a2c4a7f1790c2100873e8f09fb0cc9b5e35ec2022d4cbf0187edf30ef658c81c`。
完整函式與相鄰原註解；未執行程式／測試。

```cpp
<!-- preserved-content:start -->
std::string EncodePing(const std::string& payload) {
    // Control frames carry at most 125 bytes and are never fragmented
    // (RFC 6455 section 5.5), so an over-long ping payload is clamped rather
    // than split.
    std::string p = payload;
    if (p.size() > kWsMaxControlPayload) p.resize(kWsMaxControlPayload);
    return EncodeFrame(kWsPing, p, true, false, 0);
}


<!-- preserved-content:end -->
```
