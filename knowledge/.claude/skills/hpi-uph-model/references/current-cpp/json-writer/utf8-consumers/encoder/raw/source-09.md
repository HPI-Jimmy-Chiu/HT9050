# 原文 09：EncodeClose

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `EncodeClose` 定位。
固定來源 `16f1ddb588497d250b9eaad227bc26dce9294ce4`；SHA256 `18cb4dc4deac5007b80ac9c8ff1dec3997f95be7d90c26dbbc608d0ca456ab65`。
完整函式與相鄰原註解；未執行程式／測試。

```cpp
<!-- preserved-content:start -->
std::string EncodeClose(uint16_t code, const std::string& reason) {
    // RFC 6455 section 5.5.1: the payload is a 2-byte big-endian status code
    // followed by an optional UTF-8 reason. An empty payload is also legal and
    // means "no status" -- code 0 is the way to ask for that here.
    std::string p;
    if (code != 0) {
        p.push_back(static_cast<char>((code >> 8) & 0xFF));
        p.push_back(static_cast<char>(code & 0xFF));
        // 125 total - 2 for the code = 123 bytes of reason.
        p += TruncateUtf8(reason, kWsMaxControlPayload - 2);
    }
    return EncodeFrame(kWsClose, p, true, false, 0);
}


<!-- preserved-content:end -->
```
