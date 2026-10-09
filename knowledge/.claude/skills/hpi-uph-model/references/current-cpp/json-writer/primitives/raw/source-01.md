# 原文 01：Utf8SequenceLen

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `Utf8SequenceLen`／原header policy定位。
來源 commit `8edc9bdb86d2ece2a83244d9b71f9dfe5837ce86`；摘錄 SHA256 `5f008bcc66b2c56671615013cb97919f3e8cd72dc113ba317abc736798e58173`。
完整CPP函式，全文、註解、縮排與原空行保存；未執行函式或歷史測試。

```cpp
<!-- preserved-content:start -->
std::size_t Utf8SequenceLen(const std::string& s, std::size_t i) {
    const std::size_t n = s.size();
    const unsigned char c0 = static_cast<unsigned char>(s[i]);

    if (c0 < 0x80) {
        return 1;                                   // ASCII
    }
    if (c0 < 0xC2) {
        return 0;                                   // continuation byte, or overlong C0/C1
    }
    std::size_t need;
    unsigned int cp;
    if (c0 < 0xE0)      { need = 1; cp = c0 & 0x1Fu; }
    else if (c0 < 0xF0) { need = 2; cp = c0 & 0x0Fu; }
    else if (c0 < 0xF5) { need = 3; cp = c0 & 0x07u; }
    else                { return 0; }               // F5..FF: beyond U+10FFFF

    if (i + need >= n) {
        return 0;                                   // truncated at end of string
    }
    for (std::size_t k = 1; k <= need; ++k) {
        const unsigned char cc = static_cast<unsigned char>(s[i + k]);
        if ((cc & 0xC0u) != 0x80u) {
            return 0;
        }
        cp = (cp << 6) | (cc & 0x3Fu);
    }
    if (need == 1 && cp < 0x80u)     { return 0; }  // overlong
    if (need == 2 && cp < 0x800u)    { return 0; }  // overlong
    if (need == 3 && cp < 0x10000u)  { return 0; }  // overlong
    if (cp > 0x10FFFFu)              { return 0; }
    if (cp >= 0xD800u && cp <= 0xDFFFu) { return 0; } // surrogate
    return need + 1;
}


<!-- preserved-content:end -->
```
