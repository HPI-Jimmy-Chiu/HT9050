# 原文 03：WsDecoder::IsValidUtf8

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `WsDecoder::IsValidUtf8` 定位。
固定來源 `b1bbe408a035595c81615cbc22543a326fba12b2`；SHA256 `6d7717af696da2c445d06a41f3f4a1ca52e369e15fd0043bb9c4aa6261f144b0`。
完整函式，保留相鄰原註解。未執行程式／測試。

```cpp
<!-- preserved-content:start -->
// ===========================================================================
//  UTF-8 validation (RFC 3629): used for text messages and close reasons
// ===========================================================================
bool WsDecoder::IsValidUtf8(const std::string& s) {
    std::size_t i = 0, n = s.size();
    while (i < n) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        std::size_t extra;
        uint32_t cp;
        if (c < 0x80) { ++i; continue; }
        else if ((c & 0xE0) == 0xC0) { extra = 1; cp = c & 0x1F; }
        else if ((c & 0xF0) == 0xE0) { extra = 2; cp = c & 0x0F; }
        else if ((c & 0xF8) == 0xF0) { extra = 3; cp = c & 0x07; }
        else return false;                       // 0x80-0xBF stray, or 0xF8+
        // Continuation bytes live at i+1 .. i+extra, so they all exist only if
        // i + extra <= n - 1. A sequence cut short by the end of the string is
        // invalid (this is a whole-message check, not a streaming one).
        if (i + extra >= n) return false;
        for (std::size_t k = 1; k <= extra; ++k) {
            unsigned char cc = static_cast<unsigned char>(s[i + k]);
            if ((cc & 0xC0) != 0x80) return false;
            cp = (cp << 6) | (cc & 0x3F);
        }
        // Reject overlong encodings, UTF-16 surrogates, and > U+10FFFF.
        if (extra == 1 && cp < 0x80) return false;
        if (extra == 2 && cp < 0x800) return false;
        if (extra == 3 && cp < 0x10000) return false;
        if (cp >= 0xD800 && cp <= 0xDFFF) return false;
        if (cp > 0x10FFFF) return false;
        i += extra + 1;
    }
    return true;
}


<!-- preserved-content:end -->
```
