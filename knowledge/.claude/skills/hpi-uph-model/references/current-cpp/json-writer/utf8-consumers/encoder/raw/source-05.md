# 原文 05：EncodeBinary

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `EncodeBinary` 定位。
固定來源 `16f1ddb588497d250b9eaad227bc26dce9294ce4`；SHA256 `7f0d4e838995a6dec44401ae31d2bc706d9592096ac4a18f79e2f9785fb8f90c`。
完整函式與相鄰原註解；未執行程式／測試。

```cpp
<!-- preserved-content:start -->
std::string EncodeBinary(const std::string& bytes) {
    return EncodeFrame(kWsBinary, bytes, true, false, 0);
}


<!-- preserved-content:end -->
```
