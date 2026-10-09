# 原文 01：JsonQuote

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `std::string JsonQuote(const std::string& raw) {` 定位，完整CPP正文。
來源 commit `97361abfd5d1a08b51caf707f928b56825f3f056`；摘錄 SHA256 `4f31f93c90842dddc0ee4cc0af2f33da5068c56a40df1f2bb4482539ade96ce6`。
原文、註解、縮排與空行完整保存；只做靜態核對，未執行writer或機台。

```cpp
<!-- preserved-content:start -->
std::string JsonQuote(const std::string& raw) {
    const std::string s = SanitizeToUtf8(raw);
    std::string out;
    out.reserve(s.size() + 2);
    out.push_back('"');
    for (std::size_t i = 0; i < s.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        switch (c) {
            case '"':  out.append("\\\"");  break;
            case '\\': out.append("\\\\");  break;
            case '\b': out.append("\\b");   break;
            case '\f': out.append("\\f");   break;
            case '\n': out.append("\\n");   break;
            case '\r': out.append("\\r");   break;
            case '\t': out.append("\\t");   break;
            default:
                if (c < 0x20u) {
                    // Remaining C0 controls -- RFC 8259 forbids them raw.
                    static const char kHex[] = "0123456789abcdef";
                    out.append("\\u00");
                    out.push_back(kHex[(c >> 4) & 0x0F]);
                    out.push_back(kHex[c & 0x0F]);
                } else {
                    // >= 0x80 bytes are already known-valid UTF-8 at this point.
                    out.push_back(static_cast<char>(c));
                }
                break;
        }
    }
    out.push_back('"');
    return out;
}


<!-- preserved-content:end -->
```
