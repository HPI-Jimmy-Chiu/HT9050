# 原文 14：HttpRequest::Header

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `HttpRequest::Header` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `1a3a57b119e971b32385afaccb6a9b530f55627704d1611805d1b97b4df2e34c`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
std::string HttpRequest::Header(const std::string& name) const {
    std::string want = ToLowerAscii(name);
    std::string joined;
    bool found = false;
    for (std::size_t i = 0; i < headers.size(); ++i) {
        if (headers[i].first != want) continue;
        if (found) joined += ", ";      // RFC 7230 section 3.2.2
        joined += headers[i].second;
        found = true;
    }
    return joined;
}


<!-- preserved-content:end -->
```
