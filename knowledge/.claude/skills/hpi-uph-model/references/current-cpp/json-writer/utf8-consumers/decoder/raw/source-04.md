# 原文 04：Feed bytes

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `Feed bytes` 定位。
固定來源 `fd50c7d93d928a1ed48d9cbf5b668e39d471c758`；SHA256 `7c048041b21c3377bb62bb80a0cfc3bb665d55f3c930f014e93f528e11cc9712`。
完整函式。保留原註解；未執行程式或測試。

```cpp
<!-- preserved-content:start -->
bool WsDecoder::Feed(const char* data, std::size_t len, std::vector<WsMessage>* out) {
    if (failed_) return false;
    std::vector<WsMessage> sink;
    if (out == 0) out = &sink;

    if (data != 0 && len != 0) buf_.append(data, len);

    // Drain as many whole frames as the buffer now contains. This loop is what
    // makes the decoder chunk-boundary agnostic: it does not care whether a
    // Feed() delivered a fragment of one frame or three and a half frames.
    for (;;) {
        int r = TryOneFrame(out);
        if (r > 0) continue;      // consumed a frame, try for another
        if (r == 0) return true;  // incomplete frame at the front: need more
        return false;             // protocol failure; CloseCode() says why
    }
}

<!-- preserved-content:end -->
```
