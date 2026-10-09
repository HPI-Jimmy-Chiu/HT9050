# 原文 16：HttpRequest::QueryParam

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `HttpRequest::QueryParam` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `b78e3ff0a6e935da8712329aacee93e2cdf8ed2750b572c4dda28bab19c41018`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
std::string HttpRequest::QueryParam(const std::string& name,
                                    const std::string& def) const {
    std::string raw;
    if (!FindQueryRaw(query, name, &raw)) return def;
    return UrlDecode(raw, true);
}


<!-- preserved-content:end -->
```
