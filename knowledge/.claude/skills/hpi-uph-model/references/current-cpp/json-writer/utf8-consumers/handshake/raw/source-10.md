# 原文 10：UrlDecode

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `UrlDecode` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `6375a696dbf110d5425d90c80cb5710f4474e72264097031ef8828fa78d87385`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
std::string UrlDecode(const std::string& s, bool plusAsSpace) {
    std::string out;
    out.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '%' && i + 2 < s.size()) {
            int hi = HexVal(s[i + 1]);
            int lo = HexVal(s[i + 2]);
            if (hi >= 0 && lo >= 0) {
                out.push_back(static_cast<char>((hi << 4) | lo));
                i += 2;
                continue;
            }
            // Malformed escape: keep the '%' literally rather than dropping
            // input. The path check below still refuses anything dangerous.
        }
        if (plusAsSpace && c == '+') { out.push_back(' '); continue; }
        out.push_back(c);
    }
    return out;
}


<!-- preserved-content:end -->
```
