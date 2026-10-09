# 原文 01：AppendLength

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `AppendLength` 定位。
固定來源 `16f1ddb588497d250b9eaad227bc26dce9294ce4`；SHA256 `ee4a93799470405e455935b6c7d7c922b47c92ca6315b852a01773f32d06de14`。
完整函式與相鄰原註解；未執行程式／測試。

```cpp
<!-- preserved-content:start -->
// Append `n` as the RFC 6455 section 5.2 payload-length field(s), OR-ing in the
// mask bit. All three forms live here so the choice between them is in one
// place:
//    n <= 125          -> the 7-bit field carries the length directly
//    126 <= n <= 65535 -> field is 126, followed by 16 bits, network order
//    n >= 65536        -> field is 127, followed by 64 bits, network order
// "Network order" is big-endian, written out with explicit shifts so no
// platform header (and no host-endianness assumption) is involved.
static void AppendLength(std::string& f, std::size_t n, unsigned char maskBit) {
    if (n < 126) {
        f.push_back(static_cast<char>(maskBit | static_cast<unsigned char>(n)));
    } else if (n <= 0xFFFFu) {
        f.push_back(static_cast<char>(maskBit | 126));
        f.push_back(static_cast<char>((n >> 8) & 0xFF));
        f.push_back(static_cast<char>(n & 0xFF));
    } else {
        f.push_back(static_cast<char>(maskBit | 127));
        uint64_t v = static_cast<uint64_t>(n);
        for (int i = 7; i >= 0; --i) {
            f.push_back(static_cast<char>((v >> (i * 8)) & 0xFF));
        }
    }
}


<!-- preserved-content:end -->
```
