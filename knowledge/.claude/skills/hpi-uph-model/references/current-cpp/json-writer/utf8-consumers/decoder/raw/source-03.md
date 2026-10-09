# 原文 03：Feed chunk

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `Feed chunk` 定位。
固定來源 `fd50c7d93d928a1ed48d9cbf5b668e39d471c758`；SHA256 `b2d603bd099354e7634479d283cc3e0c7599f662ed3636639a6329355fb4ce4c`。
完整函式。保留原註解；未執行程式或測試。

```cpp
<!-- preserved-content:start -->
bool WsDecoder::Feed(const std::string& chunk, std::vector<WsMessage>* out) {
    return Feed(chunk.data(), chunk.size(), out);
}

<!-- preserved-content:end -->
```
