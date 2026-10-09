# 原文 04：EncodeText

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `EncodeText` 定位。
固定來源 `16f1ddb588497d250b9eaad227bc26dce9294ce4`；SHA256 `31b756088f67ac81e2420e65795147757f725ccf0c4ed22dce9f4ba660bd1646`。
完整函式與相鄰原註解；未執行程式／測試。

```cpp
<!-- preserved-content:start -->
std::string EncodeText(const std::string& utf8Payload) {
    return EncodeFrame(kWsText, utf8Payload, true, false, 0);
}


<!-- preserved-content:end -->
```
