# 原文 15：FindQueryRaw

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `FindQueryRaw` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `4b37e9dc0ba472057fd8c9c875feab8504341cde0ce59535b7ff4b70d97b89b2`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
// Walk "a=1&b=2" looking for `name`. `*value` gets the raw (still encoded)
// value; returns false when the key is absent.
static bool FindQueryRaw(const std::string& query, const std::string& name,
                         std::string* value) {
    std::size_t pos = 0;
    while (pos <= query.size()) {
        std::size_t amp = query.find('&', pos);
        std::string item = (amp == std::string::npos) ? query.substr(pos)
                                                     : query.substr(pos, amp - pos);
        if (!item.empty()) {
            std::size_t eq = item.find('=');
            std::string k = (eq == std::string::npos) ? item : item.substr(0, eq);
            if (UrlDecode(k, true) == name) {
                if (value) {
                    *value = (eq == std::string::npos) ? std::string()
                                                       : item.substr(eq + 1);
                }
                return true;
            }
        }
        if (amp == std::string::npos) break;
        pos = amp + 1;
    }
    return false;
}


<!-- preserved-content:end -->
```
