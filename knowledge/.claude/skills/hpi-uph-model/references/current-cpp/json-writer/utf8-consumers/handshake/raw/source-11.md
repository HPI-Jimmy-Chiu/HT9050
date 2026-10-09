# 原文 11：PathIsSuspicious

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `PathIsSuspicious` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `56152cf895e776169bd56acd684bcee1d747dd624df1c58db2bbf0d942f4176c`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
bool PathIsSuspicious(const std::string& p) {
    if (p.empty() || p[0] != '/') return true;
    if (p.find('\0') != std::string::npos) return true;
    if (p.find('\\') != std::string::npos) return true;   // Windows separator
    // reject any ".." path segment
    std::size_t seg = 1;
    while (seg <= p.size()) {
        std::size_t slash = p.find('/', seg);
        std::string s = (slash == std::string::npos) ? p.substr(seg)
                                                     : p.substr(seg, slash - seg);
        if (s == "..") return true;
        if (slash == std::string::npos) break;
        seg = slash + 1;
    }
    return false;
}


<!-- preserved-content:end -->
```
