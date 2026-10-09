# 原文 08：ListContainsToken

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `ListContainsToken` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `95a9f4635da7d8e1b696a5324b122dc38c6fd076d6c9af420bfe4befb046c582`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
// Does a comma-separated header value contain `token` as one of its elements?
// Used for Connection: it legitimately arrives as "keep-alive, Upgrade".
static bool ListContainsToken(const std::string& value, const char* token) {
    std::size_t pos = 0;
    while (pos <= value.size()) {
        std::size_t comma = value.find(',', pos);
        std::string item = (comma == std::string::npos)
                               ? value.substr(pos)
                               : value.substr(pos, comma - pos);
        if (EqualsIgnoreCase(TrimOws(item), token)) return true;
        if (comma == std::string::npos) break;
        pos = comma + 1;
    }
    return false;
}


<!-- preserved-content:end -->
```
