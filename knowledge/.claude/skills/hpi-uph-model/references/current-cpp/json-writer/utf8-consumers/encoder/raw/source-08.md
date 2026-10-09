# 原文 08：TruncateUtf8

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `TruncateUtf8` 定位。
固定來源 `16f1ddb588497d250b9eaad227bc26dce9294ce4`；SHA256 `5036b7f7dbb6894d0860b5bf942cc37eaaedb67b3b0cfbed40c9838c85167a10`。
完整函式與相鄰原註解；未執行程式／測試。

```cpp
<!-- preserved-content:start -->
// Truncate to at most `limit` bytes without splitting a UTF-8 sequence -- a
// close reason that ends mid-code-point is invalid UTF-8 and the peer is
// entitled to treat it as a 1007 protocol error.
static std::string TruncateUtf8(const std::string& s, std::size_t limit) {
    if (s.size() <= limit) return s;
    std::size_t cut = limit;
    while (cut > 0 && (static_cast<unsigned char>(s[cut]) & 0xC0) == 0x80) --cut;
    return s.substr(0, cut);
}


<!-- preserved-content:end -->
```
