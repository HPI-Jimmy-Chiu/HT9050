# 原文 07：EncodePong

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `EncodePong` 定位。
固定來源 `16f1ddb588497d250b9eaad227bc26dce9294ce4`；SHA256 `c44fe440d2aef6f2fe6379a407e1bf72a31e0ebb713a16d5b8831836d9cbc180`。
完整函式與相鄰原註解；未執行程式／測試。

```cpp
<!-- preserved-content:start -->
std::string EncodePong(const std::string& payload) {
    std::string p = payload;
    if (p.size() > kWsMaxControlPayload) p.resize(kWsMaxControlPayload);
    return EncodeFrame(kWsPong, p, true, false, 0);
}


<!-- preserved-content:end -->
```
